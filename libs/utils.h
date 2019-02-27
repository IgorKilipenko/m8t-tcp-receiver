#ifndef utils_h
#define utils_h

#include <cassert>
#include <memory>


namespace utils {

    // note: this implementation does not disable this overload for array types
    template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args &&... args) { return std::unique_ptr<T>(new T(std::forward<Args>(args)...)); }

    bool streq(const char * s1, const char * s2) {
        return strcmp(s1, s2) == 0;
    }

} // namespace utils

#endif