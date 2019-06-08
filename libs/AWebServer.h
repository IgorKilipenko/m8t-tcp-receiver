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
#include "NtripClientSync.h"

#ifdef REST_API
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "SGraphQL.h"
#endif

#include "ublox.h"
#include "UbxMessage.h"
//#include <SparkFun_Ublox_Arduino_Library.h>

#define APSSID "ESP_ap_"
#define APPSK "1234567890"
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
	void initializeGpsReceiver();

  private:
	char softAP_ssid[32];
	char softAP_password[64];

	char ssid[32];
	char password[32];
	char hostName[32];
	const char *http_username = "admin";
	const char *http_password = "admin";
	bool _connect = false;
	bool _gpsIsInint = false;
	bool _sendReceiverDataToWs = true;

	static bool _static_init;

	AsyncWebServer server;
	AsyncWebSocket ws;
	unsigned long _ubxWsLastSendTime = 0;
	unsigned long _ubxWsSendInterval = 1000;	// Interval in ms send messages to ubxWS clients
	int32_t _ubxWsWaitResp = -1;
	AsyncEventSource events;
	ATcpServer *telnetServer;
	NtripClientSync *_ntripClient;
	std::vector<std::unique_ptr<WifiItem>> wifiList;
	UbxDecoder _ubxDecoder;
	UBLOX_GPS *_gps;
	UbloxTransport *_transport;
	bool _decodeUbxMsg = true;

	/* API ============================== */
	static const char *API_P_GPSCMD;
	SGraphQL api;

	void wsEventHnadler(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType, void *, uint8_t *, size_t);
	void notFoundHandler(AsyncWebServerRequest *request);
	void initDefaultHeaders();
	void receiverDataHandler(const uint8_t *buffer, size_t len);

	ApiResultPtr wifiQueryHandler(const char *event, const JsonObject &json, JsonObject &outJson);
	ApiResultPtr wifiActionHandler(const char *event, const JsonObject &json, JsonObject &outJson);
	ApiResultPtr receiverActionHandler(const char *event, const JsonObject &json, JsonObject &outJson);
	ApiResultPtr receiverQueryHandler(const char *event, const JsonObject &reqJson, JsonObject &outJson);
	ApiResultPtr serverQueryHandler(const char *event, const JsonObject &reqJson, JsonObject &outJson);
	ApiResultPtr ntripActionHandler(const char *event, const JsonObject &reqJson, JsonObject &outJson);
	ApiResultPtr ntripQueryHandler(const char *event, const JsonObject &reqJson, JsonObject &outJson);

	bool connectStaWifi(const char *ssid, const char *password);
	void disconnectStaWifi();
	void addServerHandlers();
	void addOTAhandlers();
	void addReceiverHandlers();

	bool _autoPVT{false};
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
