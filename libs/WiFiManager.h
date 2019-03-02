#ifndef WiFiManager_h
#define WiFiManager_h

#include "Arduino.h"
#include "utils.h"

class WiFiManager {
  public:
    WiFiManager(const char *ssid, const char *password, const char *ap_ssid, const char *ap_password);
    ~WiFiManager();
	void connecSoftAP();
	void connectSTA(const char *ssid, const char *password);
	bool setApModeOnly();
	void setup();

  private:
	char *ap_ssid;
	char *ap_password;
	char *ssid;
	char *password;

	WiFiEventHandler stationConnectedHandler;
	WiFiEventHandler stationDisconnectedHandler;
	//WiFiEventHandler probeRequestPrintHandler;
	//WiFiEventHandler probeRequestBlinkHandler;

    void onStationConnected(const WiFiEventSoftAPModeStationConnected&);
    void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected&);
};

#endif // WiFiManager_h