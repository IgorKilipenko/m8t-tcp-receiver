
#include "WiFiManager.h"

WiFiManager::WiFiManager() : _eventIds{} {}
WiFiManager::~WiFiManager() {
	log_d("-> Destruct WiFi manager\n");
	removeEvents();
}

bool WiFiManager::disableSoftAp() {
	if (_apEnabled) {
		return WiFi.softAPdisconnect(true);
	}

	return false;
}

bool WiFiManager::disableSta() {
	if (WiFi.isConnected()) {
		return WiFi.disconnect(true);
	}

	return false;
}

size_t WiFiManager::removeEvents(WiFiEventId_t id) {
	bool hasId = (std::find(_eventIds.begin(), _eventIds.end(), id) != _eventIds.end());
	if (!hasId) {
		return 0;
	}

	WiFi.removeEvent(id);
	return 1;
}

size_t WiFiManager::removeEvents() {
	size_t count = _eventIds.size();
	for (int i = 0; i < count; i++) {
		removeEvents(_eventIds[i]);
	}
	_eventIds.clear();
	return count;
}

void WiFiManager::_wifiPrintEvent(WiFiEvent_t event) {
	log_d("[WiFi-event] event: %d\n", (int)event);

	switch (event) {
	case SYSTEM_EVENT_WIFI_READY:
		log_d("WiFi interface ready\n");
		break;
	case SYSTEM_EVENT_SCAN_DONE:
		log_d("Completed scan for access points\n");
		_scanDoneCb();
		break;
	case SYSTEM_EVENT_STA_START:
		log_d("WiFi client started\n");
		break;
	case SYSTEM_EVENT_STA_STOP:
		log_d("WiFi clients stopped\n");
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		log_d("Connected to access point\n");
		if (_ap_hostname.length() > 3) {
			WiFi.softAPsetHostname(_ap_hostname.c_str());
		}
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		_staConnected = false;
		log_d("Disconnected from WiFi access point\n");
		break;
	case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
		log_d("Authentication mode of access point has changed\n");
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		_staConnected = true;
		log_d("Obtained IP address: \n" /*,String{WiFi.localIP()}.c_str()*/);
		break;
	case SYSTEM_EVENT_STA_LOST_IP:
		log_d("Lost IP address and IP address is reset to 0\n");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
		log_d("WiFi Protected Setup (WPS): succeeded in enrollee mode\n");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_FAILED:
		log_d("WiFi Protected Setup (WPS): failed in enrollee mode\n");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
		log_d("WiFi Protected Setup (WPS): timeout in enrollee mode\n");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_PIN:
		log_d("WiFi Protected Setup (WPS): pin code in enrollee mode\n");
		break;
	case SYSTEM_EVENT_AP_START:
		log_d("WiFi access point started\n");
		if (_ap_hostname.length() > 3) {
			WiFi.softAPsetHostname(_ap_hostname.c_str());
		}
		_apEnabled = true;
		break;
	case SYSTEM_EVENT_AP_STOP:
		_apEnabled = false;
		log_d("WiFi access point  stopped\n");
		break;
	case SYSTEM_EVENT_AP_STACONNECTED:
		//_apHasClients = true;
		log_d("Client connected\n");
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		log_d("Client disconnected\n");
		break;
	case SYSTEM_EVENT_AP_STAIPASSIGNED:
		log_d("Assigned IP address to client\n");
		break;
	case SYSTEM_EVENT_AP_PROBEREQRECVED:
		log_d("Received probe request\n");
		break;
	case SYSTEM_EVENT_GOT_IP6:
		log_d("IPv6 is preferred\n");
		break;
	case SYSTEM_EVENT_ETH_START:
		log_d("Ethernet started\n");
		break;
	case SYSTEM_EVENT_ETH_STOP:
		log_d("Ethernet stopped\n");
		break;
	case SYSTEM_EVENT_ETH_CONNECTED:
		log_d("Ethernet connected\n");
		break;
	case SYSTEM_EVENT_ETH_DISCONNECTED:
		log_d("Ethernet disconnected\n");
		break;
	case SYSTEM_EVENT_ETH_GOT_IP:
		log_d("Obtained IP address\n");
		break;
	default:
		break;
		// case SYSTEM_EVENT_MAX:
		//	log_d("SYSTEM_EVENT_MAX\n");
	}
}

WiFiEventId_t WiFiManager::_onWiFiEvent(WifiEventHandler callback, system_event_id_t systemEventId, bool once) {
	log_d("Start\n");
	WiFiEventId_t eventID = WiFi.onEvent(
		[&](WiFiEvent_t event, WiFiEventInfo_t info) {
			// log_d("Start WiFi event handler\n");
			callback(info);
			if (once) {
				log_d("Remove event\n");
				removeEvents(eventID);
				log_d("Event id: [%i] removed\n", (int)eventID);
			}
		},
		systemEventId);
	_eventIds.push_back(eventID);
	return eventID;
}

void WiFiManager::setup(const char *ap_hostname) {
	log_d("Setup");
	_ap_hostname = ap_hostname;

	// delete old config
	WiFi.disconnect(true);
	delay(500);

	// WiFiEventId_t eventID = WiFi.onEvent(std::bind(&WiFiManager::_wifiPrintEvent, this, std::placeholders::_1));
	//_eventIds.push_back(eventID);
}

bool WiFiManager::connectAp(const char *ap_ssid, const char *ap_password) {
	log_d("Start\n");
	_ap_ssid = ap_ssid;
	_ap_password = ap_password;
	log_d("AP ssid: [%s], AP pass: [%s]\n", _ap_ssid.c_str(), _ap_password.c_str());
	return WiFi.softAP(_ap_ssid.c_str(), _ap_password.c_str());
}

bool WiFiManager::connectSta(const char *ssid, const char *password) {
	log_d("Start\n");
	_ssid = ssid;
	_password = password;
	WiFi.mode(WIFI_AP_STA);
	log_d("ssid: [%s], pass: [%s]\n", _ssid.c_str(), _password.c_str());
	return WiFi.begin(_ssid.c_str(), _password.c_str());
}

bool WiFiManager::apDisconnect(int delayTime) {
	log_d("Start\n");
	bool res = WiFi.softAPdisconnect(true);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	return res;
}

bool WiFiManager::staDisconnect(int delayTime) {
	log_d("Start\n");
	bool res = WiFi.disconnect(true);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	return res;
}

bool WiFiManager::waitEnabledAp(const char *ssid, const char *password) {
	log_d("Start\n");
	unsigned long start = millis();
	while (!apEnabled() && millis() - start < 100000) {
		connectAp(ssid, password);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	return apEnabled();
}

bool WiFiManager::waitConnectionSta(const char *ssid, const char *password) {
	log_d("Start\n");
	unsigned long start = millis();
	while (!staConnected() && millis() - start < 100000) {
		connectSta(ssid, password);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	return staConnected();
}

int16_t WiFiManager::_scanDoneCb() {
	log_d("Start\n");
	_scanComplete = false;
	//_wifiList.clear();
	int n = WiFi.scanComplete();
	log_d("Availeble networks, scanComplete: [%i]\n", n);
	if (n == -1) {
		log_w("WiFi scaning continue\n");
	} else if (n == -2) {
		log_w("WiFi scan filed, scanComplete: [%i]\n", n);
	} else if (n == 0) {
		log_w("Not availeble networks, scanComplete: [%i]\n", n);
	} else {
		if (n == 0) {
			log_d("Not availeble networks, scanComplete: [%i]\n", n);
		}
		log_d("Not availeble networks, count: [%i]\n", n);
		for (int i = 0; i < n; i++) {
			WifiItem item{};
			item.rssi = WiFi.RSSI(i);
			item.ssid = WiFi.SSID(i);
			item.bssid = WiFi.BSSIDstr(i);
			item.channel = WiFi.channel(i);
			item.secure = WiFi.encryptionType(i);
			_wifiList.push_back(item);
		}
		WiFi.scanDelete();
		_scanComplete = true;
	}

	return n;
}

int16_t WiFiManager::scanWiFiAsync() {
	log_d("Start\n");
	_scanComplete = false;
	_wifiList.clear();
	int16_t n = WiFi.scanNetworks(true);
	if (n == -2) {
		log_e("Wifi sacn filed\n");
	}
	return n;
}

WiFiEventId_t WiFiManager::onWifiScanDone(WifiScanHandler callback, bool once) {
	log_d("Start\n");
	return _onWiFiEvent(
		[&](WiFiEventInfo_t info) {
			log_d("Add Event");
			bool complete = utils::waitAtTime([&]() { return _scanComplete; }, 200, 10);
			if (complete) {
				callback(_wifiList);
			}
		},
		SYSTEM_EVENT_SCAN_DONE, once);
}

WiFiEventId_t WiFiManager::onStationConnected(WifiEventHandler callback, bool once) {
	log_d("Start\n");
	return _onWiFiEvent(
		[&](WiFiEventInfo_t info) {
			log_d("Add Event");
			callback(info);
		},
		SYSTEM_EVENT_STA_CONNECTED, once);
	return
}

WiFiEventId_t WiFiManager::onApConnected(WifiEventHandler callback, bool once) {
	log_d("Start\n");
	return _onWiFiEvent(
		[&](WiFiEventInfo_t info) {
			log_d("Add Event");
			callback(info);
		},
		SYSTEM_EVENT_AP_START, once);
}

bool WiFiManager::connectStaSync(const char *ssid, const char *password) {
	int n = WiFi.scanComplete();
	if (n == -2) {
		WiFi.scanNetworks(true);
	} else if (n) {
		for (int i = 0; i < n; ++i) {
			auto wifi = std::shared_ptr<WifiItem>(new WifiItem);
			wifi->rssi = WiFi.RSSI(i);
			wifi->ssid = WiFi.SSID(i);
			wifi->bssid = WiFi.BSSIDstr(i);
			wifi->channel = WiFi.channel(i);
			wifi->secure = WiFi.encryptionType(i);
			wifiList.push_back(std::move(wifi));
		}
		WiFi.scanDelete();
		if (WiFi.scanComplete() == -2) {
			WiFi.scanNetworks(true);
		}
	}
}