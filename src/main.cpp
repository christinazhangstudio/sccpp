#include <iostream>
#include "logger.h"
#include <device.h>

// (don't redefine these here)
extern DeviceList* discover_devices(Logger &log);
extern void free_device_list(DeviceList* list);

int main(int argc, char **argv) {
  // parse args (ex. --help)
  if (argc > 1 && std::strcmp(argv[1], "--help") == 0) {
    std::cout << "sccpp: Mirror Android screen.\nUsage: scrcpy_cpp [options]\n";
    return 0;
  }

  Logger log("sccpp.log"); // create file

  log.info("sccpp starting... (args count : %d)", argc);

  DeviceList* devices = discover_devices(log);
  // if(devices && !devices->empty()) {
  if(!devices || devices->count == 0) {
    log.info("no devices found! is adb running?");
    free_device_list(devices);
    return 1;
  }

  // get the first online device
  Device* selected = nullptr;
  for(size_t i = 0; i < devices->count; ++i) {
    if(strcmp(devices->devices[i]->state, "device") == 0) {
      selected = devices->devices[i];
      break;
    }
  }

  if (!selected) {
    log.info("no online device found");
    free_device_list(devices);
    return 1;
  }

  log.info("selected device: %s (%s)", selected->serial, selected->state);

  free_device_list(devices);

  log.info("sccpp exiting");
  return 0;
}