#include "logger.h"
#include "stdarg.h"

template <typename T>
void Logger::debug(T format, ...)
{
    if (!test())
        return;
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 255, format, args);
    va_end(args);

    Serial.print(buffer);
}
template <typename T>
void Logger::print(T str)
{
    if (!test())
        return;
    Serial.print(str);
}
template <typename T>
void Logger::println(T str)
{
    if (!test())
        return;
    Serial.println(str);
}
template <typename T>
void Logger::printf(T format, ...)
{
    if (!test())
        return;

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 255, format, args);
    va_end(args);

    Serial.print(buffer);
}

bool Logger::test()
{
#ifdef DEBUG
    return true;
#else
    return false;
#endif
}