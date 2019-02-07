
//#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include "libs/Logger.h"
#include "libs/TelnetServer.h"
#include "libs/WebServer.h"

#define DEBUG
#define ALTSSID
#define CS_PIN D8

#ifndef ALTSSID
#define SSID "Keenetic-9267"
#define PASS "1234567890"
#else
#define SSID "Redmi5Igor123"
#define PASS "1234567890"
#endif

#define BAUND 115200

Logger logger{};
WebServer webServer{};

void setup()
{

    Serial.begin(BAUND);

    telnetServer.setup();
}

void loop()
{
    telnetServer.process();
}
