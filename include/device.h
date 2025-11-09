#pragma once
#include <stdlib.h>
#include <string.h>
// #include <vector>
#include <cstddef>
#include <logger.h>

struct Device {
  char *serial;
  char *state;

  Device(const char* s, const char* st) {
    serial = strdup(s); // strdup = malloc + strcpy; needs to be freed
    state = strdup(st);
  }

  ~Device() {
    free(serial);
    free(state);
    serial = state = nullptr;
  }
};

// alias the data type for defining a vector of Device*
// raw array of Device*, can be replaced with Device** / new[]/delete[]
// using DeviceList = std::vector<Device*>;

// instead of vector, can use raw dynamic array also
struct DeviceList {
  Device** devices; // array of pointers
  size_t count;     // number of valid devices
  size_t capacity;  // allocated size

  DeviceList() : devices(nullptr), count(0), capacity(0) {}

  void add(Device* dev);
  void clear(); // free all
};

// run ADB forward for a device
// return true on success
bool adb_forward(const char* serial, Logger& log);

// remove froward
void adb_forward_remove(const char* serial, Logger &log);