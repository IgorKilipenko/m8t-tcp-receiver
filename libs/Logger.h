#ifndef Logger_h
#define Logger_h

#include "Arduino.h"

#ifdef DEBUG
#define log_info(M, ...) Serial.printf("[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
	#define log_info(M, ...)
#endif

class Logger : public Print {
  public:
		Logger(HardwareSerial *);
		~Logger();
		template <typename... T> void debug(T...);
		template <typename... T> void error(T...);
		template <typename... T> void trace(T...);
		size_t write(const uint8_t*, size_t) override;
		size_t write(const uint16_t*, size_t);
		size_t write(const uint8_t) override;
		size_t write(const uint16_t);
		void flush() override;

  private:
		HardwareSerial *lout;
};

#endif