//#define DEBUG
#define ALTSSID
#define CS_PIN D8

#define BAUD_SERIAL 115200
#define BAUND 115200


#include "libs/Logger.h"
#include "libs/TelnetServer.h"
#include "libs/WebServer.h"

Logger logger{&Serial};
TelnetServer telnetServer{};
WebServer webServer{telnetServer};

void setup() {
	Serial.begin(BAUND);

	webServer.setup();
}

void loop() { webServer.process(); }
