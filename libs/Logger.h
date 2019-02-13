#ifndef Logger_h
#define Logger_h

#include "Arduino.h"

class Logger {
	  public:
		Logger(HardwareSerial *);
		~Logger();
		template <typename... T> void debug(T...);
		template <typename T> void print(T);
		template <typename... T> void printf(T...);
		template <typename T> void println(T);

	  private:
		#ifdef DEBUG
		bool ndebug = false;
		#else
		bool ndebug = true;
		#endif
		HardwareSerial * lout;
};

#endif