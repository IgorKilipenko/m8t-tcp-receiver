#define DEBUG		// Uncomment for enable debug mode
#ifndef ESP32
#define MOCK_RECEIVER_DATA
#endif
#define WEB_LOG_LEVEL 3
#define ALTSSID
#define REST_API	// Use REST API

/* SD card */
#ifdef ESP32
#define CS_PIN 5			// SD card cs_pin (default D5 GPIO5 on ESP32 DevkitV1)
#elif defined(ESP8266)
#define CS_PIN D8			// SD card cs_pin (default D8 on ESP8266)
#else 
#error Platform not supported
#endif

/* TCP Client */
#define MAX_TCP_CLIENTS 5   // Default max clients
#define TCP_PORT 7042		// Default tcp port (GPS receiver communication)

/* Serial */
#define BAUD_SERIAL 115200	// Not use (for Serial1 logging)
#define BAUND 115200		// GPS receiver baund rate

#include "libs/utils.h"
#include "libs/Logger.h"
#include "libs/AWebServer.h"
#include "libs/ATcpServer.h"

#ifdef ESP32
HardwareSerial * Receiver{&Serial1};
#else
HardwareSerial * Receiver{&Serial};
#endif


Logger logger{&Serial};		// For debug mode
ATcpServer telnetServer{};	// GPS receiver communication

AWebServer webServer{&telnetServer};

void setup() {
#ifndef ESP32
	Serial.begin(BAUND);
#endif
	Receiver->begin(BAUND);
	webServer.setup();
}

void loop() { webServer.process(); }
