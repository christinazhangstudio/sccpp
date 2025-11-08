#include "logger.h"
#include <cstdarg>
#include <cstdlib>
#include <cstdio>

Logger::Logger(const char *filename) : 
  buffer(nullptr), capacity(0), file(nullptr) {
  file = fopen(filename, "w");
  if(!file) {
    fprintf(stderr, "[warning] could not open log file; %s\n", filename);
  }

  // create buffer with default capacity
  capacity = 256;
  buffer = new char[capacity];
  buffer[0] = '\0';
}

Logger::~Logger() {
  if(file) {
    fclose(file);
    file = nullptr;
  }
  delete[] buffer;
  buffer = nullptr;
  capacity = 0;
}

void Logger::ensure_capacity(size_t needed){
  // if capacity actually has enough
  if(needed < capacity) return;

  // otherwise allocate more (chose 1.5x largely arbitrarily)
  // if this new size is less than the requested 
  // needed size, then we use the needed (the bigger one)
  // instead.
  size_t new_cap = capacity * 3 / 2;
  if(new_cap < needed) new_cap = needed;

  char *new_buffer = new char[new_cap];
  if(buffer) {
    // create a new buffer with the new size
    // and delete the old one.
    strcpy(new_buffer, buffer);
    delete[] buffer;
  } else {
    // create buffer here for safety?.
    new_buffer[0] = '\0';
  }

  buffer = new_buffer;
  capacity = new_cap;
}

void Logger::info(const char* format, ...) {
  va_list args;
  va_start(args, format); // access to variadic args

  // format and store chars into buffer, 
  // with a specified maximum size.
  // different than printf in
  // that output is sure not to exceed the buffer size.
  // (if the str is actually longer than capacity-1, 
  // the remaining chars are thrown away 
  // (but still counted for the returned value, WITHOUT
  // the null terminator)).
  // returns number of chars -- only if value is pos
  // and less than capacity means str is completely written.
  int needed = vsnprintf(buffer, capacity, format, args);
  va_end(args); // ends traversal of variadic args/clean up args init by va_start

  // encoding error returns negative
  if(needed < 0) {
    strcpy(buffer, "[log error]"); // size 11 str
    needed = 11;
  }

  // if str was not completely written,
  // we need the null terminator, or we need
  // more size for unwritten chars.
  if((size_t)needed >= capacity) {
    ensure_capacity(needed + 1);
    va_start(args, format);
    // attempt with our new buffer/capacity
    vsnprintf(buffer, capacity, format, args);
    va_end(args);
  }

  printf("[INFO]: %s\n", buffer);

  if(file) {
    fprintf(file, "[INFO]: %s\n", buffer);
    fflush(file);
  }
}

void Logger::error(const char* format, ...) {
  va_list args;
  va_start(args, format); // access to variadic args

  int needed = vsnprintf(buffer, capacity, format, args);
  va_end(args);

  if(needed < 0) {
    strcpy(buffer, "[LOG ERROR]"); // size 11 str
    needed = 11;
  }

  if((size_t)needed >= capacity) {
    ensure_capacity(needed + 1);
    va_start(args, format);
    vsnprintf(buffer, capacity, format, args);
    va_end(args);
  }

  printf("[ERROR]: %s\n", buffer);

  if(file) {
    fprintf(file, "[ERROR]: %s\n", buffer);
    fflush(file);
  }
}