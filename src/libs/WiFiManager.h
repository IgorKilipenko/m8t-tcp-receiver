#ifndef WiFiManager_h
#define WiFiManager_h

#include "Arduino.h"
#include "utils.h"
#include <WiFi.h>
#include <algorithm>
#include <vector>
#include <string>
#include "Logger.h"

/*
* WiFi Events
0  SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
1  SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
2  SYSTEM_EVENT_STA_START                < ESP32 station start
3  SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
4  SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
5  SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
6  SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
7  SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
8  SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
9  SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
10 SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
11 SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
12 SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
13 SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
14 SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
15 SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
16 SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
17 SYSTEM_EVENT_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
18 SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
19 SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
20 SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
21 SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
22 SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
23 SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
24 SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
25 SYSTEM_EVENT_MAX
*/

struct WifiItem {
	int32_t rssi;
	String ssid;
	String bssid;
	int32_t channel;
	uint8_t secure;
	bool hidden;
};

typedef std::vector<std::shared_ptr<WifiItem>> ApRecords;
typedef std::function<void(WiFiEventInfo_t info)> WifiEventHandler;
typedef std::function<void(const std::vector<std::shared_ptr<WifiItem>> wifiList)> WifiScanHandler;

class WiFiManager {
  public:
	WiFiManager();
	~WiFiManager();
	bool setup(const char *ap_hostname);
	bool connectSta(const char *ssid, const char *password);
	bool connectAp(const char *ap_ssid, const char *ap_password);
	WiFiEventId_t onStationConnected(WifiEventHandler, bool = false);
	// WiFiEventId_t onStationDisconnected(WiFiEvent_t event);
	WiFiEventId_t onApConnected(WifiEventHandler, bool = false);
	// WiFiEventId_t onApDisconnected(WiFiEvent_t event);
	size_t removeEvents(WiFiEventId_t id);
	size_t removeEvents();
	bool ApStarted() const { return _apEnabled; }
	bool staConnected() const { return _staConnected; }
	bool apEnabled() const { return _apEnabled; }
	bool disableSoftAp();
	bool disableSta();
	bool apDisconnect(int delayTime = 100);
	bool staDisconnect(int delayTime = 100);
	bool waitEnabledAp(const char *ssid, const char *password);
	bool waitConnectionSta(const char *ssid, const char *password);
	int16_t scanWiFiAsync();
	WiFiEventId_t onWifiScanDone(WifiScanHandler callback, bool once = false);
	const ApRecords &getApRecords() const { return _wifiList; }
	void getApRecords(ApRecords &list) const { list = _wifiList; }
	bool lastScanComplete() const { return _scanDone; }

  private:
	WiFiClass *_wifi;
	std::string _ap_ssid{32, '\0'};
	std::string _ap_password{64, '\0'};
	std::string _ssid{32, '\0'};
	std::string _password{64, '\0'};
	std::string _ap_hostname{64, '\0'};

	WiFiEventId_t _onWiFiEvent(WifiEventHandler callback, system_event_id_t systemEventId, bool once = false);

	void _wifiPrintEvent(WiFiEvent_t event);
	std::vector<WiFiEventId_t> _eventIds;
	bool _staConnected = false;
	bool _apEnabled = false;
	bool _scanDone = false;
	ApRecords _wifiList;
	Logger *_logger = nullptr;
	static bool _lock;
	static void lock() { WiFiManager::_lock = true; }
	static void unlock() { WiFiManager::_lock = false; }
	static bool isLocked() { return WiFiManager::_lock; }
	unsigned long _lastScanTime;
	int16_t _scanDoneCb();
};

#endif // WiFiManager_h