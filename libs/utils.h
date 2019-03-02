#ifndef utils_h
#define utils_h

#include "Arduino.h"

namespace utils {
#include <cassert>
#include <memory>
#include <algorithm>

// note: this implementation does not disable this overload for array types
template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args &&... args) { return std::unique_ptr<T>(new T(std::forward<Args>(args)...)); }

bool streq(const char *s1, const char *s2) { return strcmp(s1, s2) == 0; }

String macToString(const unsigned char *mac) {
	char buf[20];
	snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return String(buf);
}

char *copynewstr(const char *src, size_t len) {
	char *dest = new char[len]{};
	size_t maxlen = std::min(len - 1, strlen(src));
	std::copy(src, src + maxlen, dest);
	return dest;
}

char *copynewstr(const char *src) { return copynewstr(src, strlen(src) + 1); }

/** Is this an IP? */
bool isIp(String str) {
	for (size_t i = 0; i < str.length(); i++) {
		int c = str.charAt(i);
		if (c != '.' && (c < '0' || c > '9')) {
			return false;
		}
	}
	return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
	String res = "";
	for (int i = 0; i < 3; i++) {
		res += String((ip >> (8 * i)) & 0xFF) + ".";
	}
	res += String(((ip >> 8 * 3)) & 0xFF);
	return res;
}

} // namespace utils

#endif