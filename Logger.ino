/*
	Logger
*/

Logger::Logger(HardwareSerial *serial) : Print(), lout{serial} {}
Logger::~Logger() {}

template <typename... T> void Logger::debug(T... args) {
#ifdef DEBUG
	lout->print("[DEBUG] ");
	lout->printf(args...);
#endif
}

template <typename... T> void Logger::error(T... args) {
#ifdef DEBUG
	lout->print("[ERROR] ");
	lout->printf(args...);
#endif
}

template <typename... T> void Logger::trace(T... args) {
#ifdef DEBUG
	lout->print("[TRACE] -> ");
	lout->printf(args...);
#endif
}

size_t Logger::write(const uint8_t *buffer, size_t len) {
#ifdef DEBUG
	return lout->write(buffer, len);
#endif
	return 0;
}

size_t Logger::write(const uint16_t *buffer, size_t len) {
#ifdef DEBUG
	size_t i = 0;
	while (i < len) {
		return lout->write((const uint8_t *)(&buffer[i++]), 2);
	}
#endif
	return 0;
}

size_t Logger::write(const uint16_t wc) {
#ifdef DEBUG
	return write(&wc, 2);
#endif
	return 0;
}

size_t Logger::write(const uint8_t c) {
#ifdef DEBUG
	return lout->write(c);
#endif
	return 0;
}

void Logger::flush() {
#ifdef DEBUG
	return lout->flush();
#endif
}