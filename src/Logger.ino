/*
	Logger
*/

Logger::Logger(HardwareSerial *serial) : Print(), lout{serial}, _eventSource{nullptr} {}
Logger::~Logger() {}

template <typename... T> void Logger::debug(T... args) {
#if WEB_LOG_LEVEL > 1
	sendToEventSource("logger", args...);
#endif
#if defined(DEBUG) && LOG_LEVEL > 1
	//lout->print("[DEBUG] ");
	lout->printf(args...);
#endif
}

template <typename... T> void Logger::error(T... args) {
#if WEB_LOG_LEVEL > 0
	sendToEventSource("logger", args...);
#endif
#if defined(DEBUG) && LOG_LEVEL > 0
	lout->print("[ERROR] ");
	lout->printf(args...);
#endif
}

template <typename... T> void Logger::trace(T... args) {
#if WEB_LOG_LEVEL > 2
	sendToEventSource("logger", args...);
#endif
#if defined(DEBUG) && LOG_LEVEL > 2
	lout->print("[TRACE] -> ");
	lout->printf(args...);
#endif
}

size_t Logger::write(const uint8_t *buffer, size_t len) {
//#ifdef DEBUG
	return lout->write(buffer, len);
//#endif
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
//#ifdef DEBUG
	return lout->write(c);
#//endif
	return 0;
}

#ifdef ESP8266
void Logger::flush() {
#ifdef DEBUG
	return lout->flush();
#endif
}
#endif

void Logger::sendToEventSource(const char *event, const char * format, ...) {
	if (_eventSource != nullptr && _eventSource->count() > 0) {
		va_list arg;
		va_start(arg, format);
		char temp[64];
		char *buffer = temp;
		size_t len = vsnprintf(temp, sizeof(temp), format, arg);
		va_end(arg);
		if (len > sizeof(temp) - 1) {
			buffer = new char[len + 1];
			if (!buffer) {
				return;
			}
			va_start(arg, format);
			vsnprintf(buffer, len + 1, format, arg);
			va_end(arg);
		}
		_eventSource->send(buffer, event);
		if (buffer != temp) {
			delete[] buffer;
		}
	}
}

Stream& Logger::getStream(){
	return {*lout};
}