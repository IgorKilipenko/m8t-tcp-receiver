
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
	log_v("[WiFi-event] event: %d\n", (int)event);

	switch (event) {
	case SYSTEM_EVENT_WIFI_READY:
		log_d("WiFi interface ready\n");
		break;
	case SYSTEM_EVENT_SCAN_DONE:
		log_d("Completed scan for access points\n");
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
		// log_d("Obtained IP address: [%s]\n", String{WiFi.localIP()}.c_str());
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

WiFiEventId_t WiFiManager::_onWiFiEvent(WifiEventHandler callback, system_event_id_t systemEventId) {
	WiFiEventId_t eventID = WiFi.onEvent(
		[&](WiFiEvent_t event, WiFiEventInfo_t info) {
			log_d("Start WiFi event handler\n");
			callback(info);
		},
		systemEventId);
	_eventIds.push_back(eventID);
	return eventID;
}

void WiFiManager::setup(const char *ap_hostname) {

	_ap_hostname = ap_hostname;

	// delete old config
	WiFi.disconnect(true);
	delay(500);

	WiFiEventId_t eventID = WiFi.onEvent(std::bind(&WiFiManager::_wifiPrintEvent, this, std::placeholders::_1));
	_eventIds.push_back(eventID);
}

bool WiFiManager::connectAp(const char *ap_ssid, const char *ap_password) {
	_ap_ssid = ap_ssid;
	_ap_password = ap_password;
	log_d("AP ssid: [%s], AP pass: [%s]\n", _ap_ssid.c_str(), _ap_password.c_str());
	return WiFi.softAP(_ap_ssid.c_str(), _ap_password.c_str());
}

bool WiFiManager::connectSta(const char *ssid, const char *password) {
	_ssid = ssid;
	_password = password;
	WiFi.mode(WIFI_AP_STA);
	log_d("ssid: [%s], pass: [%s]\n", _ssid.c_str(), _password.c_str());
	return WiFi.begin(_ssid.c_str(), _password.c_str());
}

bool WiFiManager::apDisconnect(int delayTime) {
	bool res = WiFi.softAPdisconnect(true);
	delay(delayTime);
	return res;
}

bool WiFiManager::staDisconnect(int delayTime) {
	bool res = WiFi.disconnect(true);
	delay(delayTime);
	return res;
}

bool WiFiManager::waitEnabledAp(const char *ssid, const char *password) {
	unsigned long start = millis();
	while (!apEnabled() && millis() - start < 5000) {
		connectAp(ssid, password);
		delay(10);
	}
	return apEnabled();
}

bool WiFiManager::waitConnectionSta(const char *ssid, const char *password) {
	unsigned long start = millis();
	while (!staConnected() && millis() - start < 5000) {
		connectSta(ssid, password);
		delay(10);
	}
	return staConnected();
}