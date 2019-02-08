#ifndef Logger_h
#define Logger_h

#include "Arduino.h"

class Logger {
 public:
  Logger();
  ~Logger();
  template <typename T>
  void debug(T, ...);
  template <typename T>
  void print(T);
  template <typename T>
  void printf(T, ...);
  template <typename T>
  void println(T);

 private:
  bool test();
};

#endif