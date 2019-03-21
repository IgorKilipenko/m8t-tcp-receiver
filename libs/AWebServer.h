#ifndef AWebServer_h
#define AWebServer_h

#define FS_NO_GLOBALS

#include "Arduino.h"
//#include <string>

#ifdef ESP32
#include "WiFi.h"
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include "SPIFFS.h"
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#include <Hash.h> // ESP8266 only
#else
#error Platform not supported
#endif

#include <ESPAsyncWebServer.h>

#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <FS.h>

#include <SPIFFSEditor.h>
#include "ATcpServer.h"

#ifdef REST_API
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "SGraphQL.h"
#endif

#include "ublox.h"
#include "UbxMessage.h"

#define APSSID "ESP_ap_"
#define APPSK "12345678"
#define GPS_START_BTN "Start GPS"
#define GPS_STOP_BTN "Stop GPS"

class AWebServer {
  public:
	AWebServer(ATcpServer *telnetServer);
	~AWebServer();
	void setup();
	void process();
	void loadWiFiCredentials();
	void saveWiFiCredentials();
	int8_t scanWiFi();
	struct WifiItem;
	void end();
	void restart();
	unsigned long getServerTime() const { return millis(); }

  private:
	char softAP_ssid[32];
	char softAP_password[64];

	char ssid[32];
	char password[32];
	char hostName[32];
	const char *http_username = "admin";
	const char *http_password = "admin";
	bool _connect = false;
	bool _sendReceiverDataToWs = true;

	static bool _static_init;

	AsyncWebServer server;
	AsyncWebSocket ws;
	AsyncEventSource events;
	AsyncEventSource ubxMsgSource;
	ATcpServer *telnetServer;
	std::vector<std::unique_ptr<WifiItem>> wifiList;
	UbxDecoder _ubxDecoder;
	bool _decodeUbxMsg = true;

	/* API ============================== */
	static const char *API_P_GPSCMD;
	SGraphQL api;

	void wsEventHnadler(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType, void *, uint8_t *, size_t);
	void notFoundHandler(AsyncWebServerRequest *request);
	void initDefaultHeaders();

	ApiResultPtr wifiQueryHandler(const char *event, const JsonObject &json, JsonObject &outJson);
	ApiResultPtr wifiActionHandler(const char *event, const JsonObject &json, JsonObject &outJson);
	ApiResultPtr receiverActionHandler(const char *event, const JsonObject &json, JsonObject &outJson);
	ApiResultPtr receiverQueryHandler(const char *event, const JsonObject &reqJson, JsonObject &outJson);
	ApiResultPtr serverQueryHandler(const char *event, const JsonObject &reqJson, JsonObject &outJson);

	bool connectStaWifi(const char *ssid, const char *password);
	void disconnectStaWifi();
	void addServerHandlers();
	void addOTAhandlers();
	void addReceiverHandlers();
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
