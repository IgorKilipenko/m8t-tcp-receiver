#ifndef AWebServer_h
#define AWebServer_h

#define FS_NO_GLOBALS

#include "Arduino.h"
#include "utils.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncWebServer.h>
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWebServer.h>
#else
#error Platform not supported
#endif

#include <EEPROM.h>
#include <ArduinoOTA.h>
//#include <ESP8266WiFi.h>
#include <FS.h>
#include <Hash.h>
//#include <ESPAsyncTCP.h>
#include <SPIFFSEditor.h>
#include "ATcpServer.h"

#ifdef REST_API
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "SGraphQL.h"
#endif

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
	int8_t scanWiFi();
	struct WifiItem;

  private:
	const char *softAP_ssid = (APSSID + String(ESP.getChipId())).c_str();
	const char *softAP_password = APPSK;

	char ssid[32];
	char password[32];
	const char *hostName = "esp-async";
	const char *http_username = "admin";
	const char *http_password = "admin";
	
	static bool _static_init;

	AsyncWebServer server;
	AsyncWebSocket ws;
	AsyncEventSource events;
	ATcpServer* telnetServer;
	std::vector<std::unique_ptr<WifiItem>> wifiList;

	/* API ============================== */
	static const char * API_P_GPSCMD;
	SGraphQL api;

	void onWsEvent(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType , void *, uint8_t *, size_t);
	void initDefaultHeaders();
};

struct AWebServer::WifiItem {
	int32_t rssi;
	String ssid;
	String bssid;
	int32_t channel;
	uint8_t secure;
	bool hidden;
};

#endif // AWebServer_h