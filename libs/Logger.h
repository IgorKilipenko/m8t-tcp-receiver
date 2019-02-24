#ifndef Logger_h
#define Logger_h

#include "Arduino.h"

#ifdef DEBUG
#define log_info(M, ...) Serial.printf("[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
	#define log_info(M, ...)
#endif

class Logger {
  public:
		Logger(HardwareSerial *);
		~Logger();
		template <typename... T> void debug(T...);
		template <typename T> void print(T);
		template <typename... T> void printf(T...);
		template <typename T> void println(T);
		template <typename T> Logger &operator<<(T str);

  private:
		HardwareSerial *lout;
};

#endif