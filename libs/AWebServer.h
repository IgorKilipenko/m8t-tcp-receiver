#ifndef AWebServer_h
#define AWebServer_h

#define FS_NO_GLOBALS

#include "Arduino.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncWebServer.h>
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#else
#error Platform not supported
#endif

#include <EEPROM.h>
#include <ArduinoOTA.h>
//#include <ESP8266WiFi.h>
#include <FS.h>
#include <Hash.h>
//#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include "ATcpServer.h"

#define APSSID "ESP_ap_"
#define APPSK "12345678"
#define GPS_START_BTN "Start GPS"
#define GPS_STOP_BTN "Stop GPS"

class AWebServer {
  public:
	AWebServer(ATcpServer* telnetServer);
	~AWebServer();
	void setup();
	void process();
	void loadWiFiCredentials();
	void saveWiFiCredentials();

  private:
	const char *softAP_ssid = (APSSID + String(ESP.getChipId())).c_str();
	const char *softAP_password = APPSK;

	char ssid[32];
	char password[32];
	char gpsStarted[1];
	const char *hostName = "esp-async";
	const char *http_username = "admin";
	const char *http_password = "admin";
	
	AsyncWebServer server;
	AsyncWebSocket ws;
	AsyncEventSource events;
	ATcpServer* telnetServer;

	void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
};

#endif // AWebServer_h