#include <iostream>
#include "logger.h"
#include "device.h"
#include "socket.h"
// (quotes tell the compiler to look into the src dir
// and if it isn't there, look into system include paths)
// while <> tell the compiler to only search for it in
// the system include paths, not the src dir first.
// local project headers always use quotes.

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

  if(!adb_forward(selected->serial, log)) {
    log.error("failed to adb forward, will attempt cleanup");
    adb_forward_remove(selected->serial, log);
    free_device_list(devices);
    return 1;
  }

  log.info("ready for socket connection on localhost:27183, connecting...");
  Socket client;
  if(!client.connect("127.0.0.1", 27183)) {
    log.error("failed to connect to sccpp server on device! is adb server running?");
    adb_forward_remove(selected->serial, log);
    free_device_list(devices);
    return 1;
  }

  log.info("connected to sccpp server");

  // do the handshake
  const char handshake[] = "sccpp";
  if(!client.send((const uint8_t*)handshake, 5, log)) {
    log.error("failed to send handshake");
    client.close();
    adb_forward_remove(selected->serial, log);
    free_device_list(devices);
    return 1;
  }

  log.info("sent handshake: \"%s\"", handshake);

  uint8_t name_buf[256] = {0};
  size_t received = 0;
  if(!client.recv(name_buf, sizeof(name_buf) - 1, &received)) {
    log.error("failed to read device name");
  } else {
    name_buf[received] = '\0';
    log.info("device name: \"%s\"", (char*)name_buf);
  }

  log.info("closing socket");
  client.close();

  log.info("cleaning up adb forward");
  adb_forward_remove(selected->serial, log);

  free_device_list(devices);

  // TODO: catch ctrl-C ?

  log.info("sccpp exiting");
  return 0;
}