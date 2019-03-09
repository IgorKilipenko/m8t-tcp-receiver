#define DEBUG		// Uncomment for enable debug mode
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
//#include "libs/WebServer.h"



Logger logger{&Serial};		// For debug mode
ATcpServer telnetServer{};	// GPS receiver communication
//WebServer webServer{&telnetServer};	// Web interface

AWebServer webServer{&telnetServer};

void setup() {

	Serial.begin(BAUND);

	//webServer.setup();

	webServer.setup();
}

void loop() { webServer.process(); }
