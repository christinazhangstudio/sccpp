#include <iostream>
#include "logger.h"
#include "device.h"
#include "socket.h"
#include "decoder.h"
#include "renderer.h"
// (quotes tell the compiler to look into the src dir
// and if it isn't there, look into system include paths)
// while <> tell the compiler to only search for it in
// the system include paths, not the src dir first.
// local project headers always use quotes.

// (don't redefine these here)
extern DeviceList* discover_devices(Logger &log);
extern void free_device_list(DeviceList* list);

Logger* g_log = nullptr;

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

  // ------ ffmpeg decoding start ------

  VideoDecoder decoder;
  g_log = &log;
  
  if(!decoder.init()) {
    log.error("failed to init decoder");
    client.close();
    adb_forward_remove(selected->serial, log);
    free_device_list(devices);
    return 1;
  }

  log.info("waiting for video stream");
  
  Renderer sdl;
  boolean first_frame = true;
  while(sdl.poll_event()) {
    uint64_t timestamp;
    uint32_t packet_size;

    if(!client.recv((uint8_t*)&timestamp, 8)) break;
    if(!client.recv((uint8_t*)&packet_size, 4)) break;

    // network to host
    packet_size = ntohl(packet_size);
    std::vector<uint8_t> nal(packet_size);
    size_t received = 0;
    if(!client.recv(nal.data(), packet_size, &received) || received != packet_size) {
      log.info("break received");
      break;
    }

    log.info("received NAL; %u bytes", (uint32_t)packet_size);

    if (decoder.decode(nal.data(), packet_size)) {
      if(first_frame) {
        log.info("first frame decoded, initing SDL...");
        if(!sdl.init(decoder.get_width(), decoder.get_height())) {
          log.error("failed to init SDL");
          return 1;
        }
        
        // [INFO]: pixel format: yuv420p (0)
        log.info("pixel format: %s (%d)", 
          av_get_pix_fmt_name((AVPixelFormat)decoder.get_frame()->format), 
          decoder.get_frame()->format
        );

        // [INFO]: SDL window opened: 1088x2208
        log.info("SDL window opened: %dx%d", decoder.get_width(), decoder.get_height());
        first_frame = false;
      }

      sdl.render(decoder.get_frame());
    }
  }

  //  ------ ffmpeg decoding end ------

  log.info("closing socket");
  client.close();

  log.info("cleaning up adb forward");
  adb_forward_remove(selected->serial, log);

  free_device_list(devices);

  // TODO: catch ctrl-C ?

  log.info("sccpp exiting");
  return 0;
}