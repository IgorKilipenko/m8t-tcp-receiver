#ifndef utils_h
#define utils_h

#include "Arduino.h"
#include <array>

#ifdef ESP32
#include <WiFi.h>
#else
#error Platform not supported
#endif

#include <cassert>
#include <memory>
#include <algorithm>

template <typename T, size_t len> using SharedBytesPtr = std::shared_ptr<T *>;

template <typename T, size_t len> using SharedArrayPtr = std::shared_ptr<std::array<T, len>>;

template <size_t len> using SharedByteArrayPtr = SharedArrayPtr<uint8_t, len>;

typedef std::function<bool()> Trigger;

class utils {
  public:
	// note: this implementation does not disable this overload for array types
	template <typename T, typename... Args> static std::unique_ptr<T> make_unique(Args &&... args) { return std::unique_ptr<T>(new T(std::forward<Args>(args)...)); }

	static bool streq(const char *s1, const char *s2) { return strcmp(s1, s2) == 0; }

	static String macToString(const unsigned char *mac) {
		char buf[20];
		snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		return String(buf);
	}

	static char *copynewstr(const char *src, size_t len) {
		char *dest = new char[len]{};
		size_t maxlen = std::min(len - 1, strlen(src));
		std::copy(src, src + maxlen, dest);
		return dest;
	}

	static char *copynewstr(const char *src) { return copynewstr(src, strlen(src) + 1); }

	/** Is this an IP? */
	static bool isIp(String str) {
		for (size_t i = 0; i < str.length(); i++) {
			int c = str.charAt(i);
			if (c != '.' && (c < '0' || c > '9')) {
				return false;
			}
		}
		return true;
	}

	/** IP to String? */
	static String toStringIp(IPAddress ip) {
		String res = "";
		for (int i = 0; i < 3; i++) {
			res += String((ip >> (8 * i)) & 0xFF) + ".";
		}
		res += String(((ip >> 8 * 3)) & 0xFF);
		return res;
	}

	/* Source : https://github.com/alanswx/ESPAsyncWiFiManager/blob/00b5c15acab9ef18abf5084cfee88995a8fec1ed/ESPAsyncWiFiManager.cpp */
	static String byteToHexString(uint8_t *buf, uint8_t length, String strSeperator = "-") {
		const char HEX_CHAR_ARRAY[17]{"0123456789ABCDEF"};
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

	static String getEspChipId() {
		uint64_t chipid;
		chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
		int chipid_size = 6;
		uint8_t chipid_arr[chipid_size];
		for (uint8_t i = 0; i < chipid_size; i++) {
			chipid_arr[i] = (chipid >> (8 * i)) & 0xff;
		}
		return byteToHexString(chipid_arr, chipid_size, "");
	}

	static String wiFiModeToString(uint8_t mode) {
		switch (mode) {
		case 0:
			return "WIFI_OFF";
		case 1:
			return "WIFI_STA";
		case 2:
			return "WIFI_AP";
		case 3:
			return "WIFI_AP_STA";
		default:
			return "unknown";
		}
	}

	template <typename T> static T fromBytes(const uint8_t *buffer, size_t len) {
		T res;
		memcpy(&res, buffer, len);
		return res;
	}

	/* Source: https://github.com/tomojitakasu/RTKLIB/blob/71db0ffa0d9735697c6adfd06fdf766d0e5ce807/src/stream.c */
	static int encbase64(char *str, const unsigned char *byte, int n) {
		const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		int i, j, k, b;

		for (i = j = 0; i / 8 < n;) {
			for (k = b = 0; k < 6; k++, i++) {
				b <<= 1;
				if (i / 8 < n)
					b |= (byte[i / 8] >> (7 - i % 8)) & 0x1;
			}
			str[j++] = table[b];
		}
		while (j & 0x3)
			str[j++] = '=';
		str[j] = '\0';
		return j;
	}

	static int ethernetDechunk(char *pChunk) {
		int bytes_removed = -1;
		int chunk_size_length = 0;

		if (pChunk) {
			while (((*pChunk != '\n') && (*pChunk != '\r')) && *pChunk) {
				pChunk++;
				chunk_size_length++;
			}

			if ((*pChunk == '\r') || (*pChunk == '\n')) {
				while ((*pChunk == '\r') || (*pChunk == '\n')) {
					pChunk++;
					chunk_size_length++;
				}

				// Do a memmove on the string, length is +1 to ensure we get a termninating NULL
				memmove(pChunk - chunk_size_length, pChunk, strlen(pChunk) + 1);
				bytes_removed = chunk_size_length;

				// If there is a final \r\n, remove it
				pChunk += (strlen(pChunk) - 2);
				if (((*pChunk == '\r') && (*(pChunk + 1) == '\n')) || ((*pChunk == '\n') && (*(pChunk + 1) == '\r'))) {
					*pChunk++ = 0;
					*pChunk = 0;
					bytes_removed += 2;
				}
			}
		}

		return bytes_removed;
	}

	static char *copy_s(char *dest, const char *src, size_t len) {
		size_t size = std::min(len, sizeof(src));
		assert(size <= sizeof(dest));
		std::copy(src, src + size, dest);
		return dest;
	}

	static bool waitAtTime(Trigger trigger, uint64_t waitTime, int16_t delayTime=10){
		uint64_t start = millis();
		while (!trigger()){
			vTaskDelay(delayTime / portTICK_PERIOD_MS);
			if (millis() - start >= waitTime){
				return false;
			}
		}
		return trigger();
	}
};

#endif