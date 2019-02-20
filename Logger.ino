/*
	Logger
*/

Logger::Logger(HardwareSerial *serial) : lout{serial} {}
Logger::~Logger() {}

template <typename... T> void Logger::debug(T... args) {
	if (ndebug)
		return;
	lout->printf(args...);
}
template <typename T> void Logger::print(T str) {
	if (ndebug)
		return;
	lout->print(str);
}
template <typename T> void Logger::println(T str) {
	if (ndebug)
		return;
	lout->println(str);
}
template <typename... T> void Logger::printf(T... args) {
	if (ndebug)
		return;

	lout->printf(args...);
}
template <typename T> Logger &Logger::operator<<(T str) {
	logger.print(str);
	return (*this);
}