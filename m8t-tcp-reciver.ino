//#define DEBUG
#define ALTSSID
#define CS_PIN D8

#define BAUND 115200

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <SD.h>

#include "libs/Logger.h"
#include "libs/TelnetServer.h"
#include "libs/WebServer.h"

Logger logger{};
TelnetServer telnetServer{};
WebServer webServer{telnetServer};

void setup() {
	Serial.begin(BAUND);

	webServer.setup();
}

void loop() { webServer.process(); }
