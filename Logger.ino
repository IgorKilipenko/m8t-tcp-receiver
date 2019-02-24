/*
	Logger
*/

Logger::Logger(HardwareSerial *serial) : lout{serial} {}
Logger::~Logger() {}

template <typename... T> void Logger::debug(T... args) {
#ifdef DEBUG
	lout->printf(args...);
#endif
}
template <typename T> void Logger::print(T str) {
#ifdef DEBUG
	lout->print(str);
#endif
}
template <typename T> void Logger::println(T str) {
#ifdef DEBUG
	lout->println(str);
#endif
}
template <typename... T> void Logger::printf(T... args) {
#ifdef DEBUG
	lout->printf(args...);
#endif
}
template <typename T> Logger &Logger::operator<<(T str) {
	logger.print(str);
	return (*this);
}