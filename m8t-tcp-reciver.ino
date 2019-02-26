#define DEBUG		// Uncomment for enable debug mode
#define ALTSSID
#define REST_API	// Use REST API

/* SD card */
#define CS_PIN D8	// SD card cs_pin (default D8 on ESP8266)

/* TCP Client */
#define MAX_TCP_CLIENTS 5   // Default max clients
#define TCP_PORT 7042		// Default tcp port (GPS receiver communication)

/* Serial */
#define BAUD_SERIAL 115200	// Not use (for Serial1 logging)
#define BAUND 115200		// GPS receiver baund rate

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
