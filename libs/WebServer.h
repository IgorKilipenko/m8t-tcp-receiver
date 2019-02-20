/*
	WebServer
*/

#ifndef WebServer_h
#define WebServer_h

#ifndef APSSID
#define APSSID "ESP_ap_"
#define APPSK "12345678"
#endif

#define GPS_START_BTN "Start GPS"
#define GPS_STOP_BTN "Stop GPS"

#include "Arduino.h"
#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include "TelnetServer.h"
#include "OTA.h"

class WebServer {
  private:
	const char *softAP_ssid = (APSSID + String(ESP.getChipId())).c_str();
	const char *softAP_password = APPSK;
	const char *myHostname = ("esp8266_" + String(ESP.getChipId())).c_str();
	char ssid[32];
	char password[32];
	char gpsStarted[1];

	// DNS server
	const byte DNS_PORT = 53;
	DNSServer dnsServer;

	// Web server
	ESP8266WebServer server{80};

	/* Soft AP network parameters */
	IPAddress apIP{192, 168, 4, 1};
	IPAddress netMsk{255, 255, 255, 0};

	/** Should I connect to WLAN asap? */
	boolean connect;

	/** Last time I tried to connect to WLAN */
	unsigned long lastConnectTry = 0;

	/** Current WLAN status */
	unsigned int status = WL_IDLE_STATUS;

	TelnetServer telnetServer;

	/** Meta-tags html */
	String meta;

	/** OTA updater */
	OTA ota;

  public:
	WebServer(TelnetServer);
	~WebServer();
	void setup();
	void handleRoot();
	boolean captivePortal();
	void handleWifi();
	void handleWifiSave();
	void handleNotFound();
	void connectWifi();
	void process();
	char *getSsid();

	void handleStartGPS();
	void returnFail(String);

	void saveCredentials();
	void loadCredentials();
};

#endif // WebServer_h