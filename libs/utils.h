#ifndef utils_h
#define utils_h

#include "Arduino.h"

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error Platform not supported
#endif

#include <cassert>
#include <memory>
#include <algorithm>

namespace utils {


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

/* Source : https://github.com/alanswx/ESPAsyncWiFiManager/blob/00b5c15acab9ef18abf5084cfee88995a8fec1ed/ESPAsyncWiFiManager.cpp */
const char HEX_CHAR_ARRAY[17] = "0123456789ABCDEF";
/**
* convert char array (hex values) to readable string by seperator
* buf:           buffer to convert
* length:        data length
* strSeperator   seperator between each hex value
* return:        formated value as String
*/
String byteToHexString(uint8_t *buf, uint8_t length, String strSeperator = "-") {
	String dataString = "";
	for (uint8_t i = 0; i < length; i++) {
		byte v = buf[i] / 16;
		byte w = buf[i] % 16;
		if (i > 0) {
			dataString += strSeperator;
		}
		dataString += String(HEX_CHAR_ARRAY[v]);
		dataString += String(HEX_CHAR_ARRAY[w]);
	}
	dataString.toUpperCase();
	return dataString;
} // byteToHexString

String getEspChipId() {
#ifdef ESP32
	uint64_t chipid;
	chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
	int chipid_size = 6;
	uint8_t chipid_arr[chipid_size];
	for (uint8_t i = 0; i < chipid_size; i++) {
		chipid_arr[i] = (chipid >> (8 * i)) & 0xff;
	}
	return byteToHexString(chipid_arr, chipid_size, "");
#elif defined(ESP8266)
	return String(ESP.getChipId());
#endif
}

} // namespace utils

#endif