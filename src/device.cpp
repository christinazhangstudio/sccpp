#include "device.h"
#include "logger.h"
#include <cstdlib>
#include <cstdio>
#include <cstring> // just to go to show string.h and cstring are both valid, but cstring is preferred.

// _WIN32 macro is always defined when compiling on
// Windows, and this replaces popen(...) with _popen(...) calls etc.
// Windows prefixes these functions with an underscore
// because they are non-standard extensions to the C runtime.
// literally using popen(...) will be OK in Linux, 
// but will fail in Windows.
// (could use CreateProcess/CreatePipe, but we'll opt for simplicity)
#ifdef _WIN32
    #define popen _popen
    #define pclose _pclose
#endif

static void trim(char* str) {
  if (str == NULL || *str == '\0') return;

  char* end = str + strlen(str) - 1; // last char, before null term
  while(end >= str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
    *end = '\0';
    --end;
  }

  char* start = str;
  while(*start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
    ++start;
  }
  if (start != str) {
    // move start (first non whitespace char) + 
    // the rest of the str (and +1 for null term) 
    // into str
    memmove(str, start, strlen(start) + 1);
  }
}

DeviceList* discover_devices(Logger &log){
  log.info("discovering ADB devices...");

  FILE* pipe = popen("adb devices", "r");
  if(!pipe) {
    log.error("failed to run 'adb devices'");
    return nullptr;
  }

  char line[256];
  DeviceList* devices = new DeviceList();

  if(fgets(line, sizeof(line), pipe)) {
    // skip header
  }

  while(fgets(line, sizeof(line), pipe)) {
    trim(line);
    if(strlen(line) == 0) continue;

    // split on spaces and 
    // replace first delimiter with null.
    // passing in nullptr will make strtok
    // continue where it left off from the prev 
    // ex: "12345 active" -> "12345" and "active"
    char* serial = strtok(line, " \t");
    char* state = strtok(nullptr, " \t");
    if (serial && state) {
      log.info("found device; %s (%s)", serial, state);
      // devices->push_back(new Device(serial, state));   // vector alt
      devices->add(new Device(serial, state));
    }
  }

  pclose(pipe);
  log.info("device discovery complete; found %zu devices", devices->count);
  return devices;
}

// vector alt:
// void free_device_list(DeviceList* list) {
//   if(!list) return;
//   for(Device* dev : *list) {
//     delete dev;
//   }
//   delete list;
// }

void DeviceList::add(Device* dev) {
  if(count >= capacity) {
    // start with 2, then x1.5 (this is largely arbitrary)
    size_t new_cap = capacity == 0 ? 2 : capacity * 3 / 2;
    Device** new_array = new Device*[new_cap];

    for(size_t i = 0; i < count; ++i) {
      new_array[i] = devices[i];
    }

    delete[] devices; // free the old array, but not the objects!!
    devices = new_array;
    capacity = new_cap;
  }

  devices[count++] = dev;
}

void DeviceList::clear() {
  for(size_t i = 0; i < count; ++i) {
    delete devices[i];
  }

  delete[] devices;
  devices = nullptr;
  count = capacity = 0;
}

void free_device_list(DeviceList* list) {
  if(!list) return;
  list->clear();
  delete list; // delete struct itself
}

bool adb_forward(const char* serial, Logger& log) {
  char cmd[512];
  // like vsn..., but for non-variadic
  // device connects to Android unix domain socket at localabstract:<name>
  snprintf(cmd, sizeof(cmd), "adb -s \"%s\" forward tcp:27183 localabstract:sccpp", serial);

  log.info("forwarding; %s", cmd);

  FILE* pipe = popen(cmd, "r");
  if(!pipe) {
    log.error("failed to execute adb forward");
    return false;
  }

  char line[128];
  while(fgets(line, sizeof(line), pipe)) {
    trim(line);
    if(strlen(line) > 0 && strcmp(line, "27183") != 0) {
      log.error("adb returned with error code; %s", line);
      return false; // if adb outputs anything (other than the tcp port), this is likely an err
    }
  }

  int exit_code = pclose(pipe);
  if(exit_code != 0) {
    log.error("failed to adb forward; %d", exit_code);
    return false;
  }

  log.info("successfully forwarded tcp:27183 -> localabstract:sccpp");
  return true;
}

void adb_forward_remove(const char* serial, Logger& log) {
  char cmd[512];
  snprintf(cmd, sizeof(cmd), "adb -s \"%s\" forward --remove tcp:27183", serial);

  log.info("removing forward; %s", cmd);

  FILE* pipe = popen(cmd, "r");
  if(pipe) {
    pclose(pipe); // ignore output; this is best effort anyway
  }
}