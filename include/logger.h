#pragma once
#include <cstdio>
#include <cstring>

class Logger {
private:
  char *buffer;    // allocated in ctor, freed in dtor
  size_t capacity; // current buffer size
  FILE *file;      // log file handle (nullptr = disabled)

  void ensure_capacity(size_t needed);

public:
  // ctor (explicit so that a Logger
  // object has to be explicitly created,
  // and nothing can be accidentally converted to it).
  // ex: 
  // void someMethod(Logger logger);
  // someMethod("output.log");
  // without explict, "output.log" is allowed
  // to be converted to a logger, which is not
  // what we want. 
  explicit Logger(const char *filename);

  // dtor
  ~Logger();

  void info(const char *format, ...);
  void error(const char *format, ...);

  // disable copy since raw rsrc owned
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;
};