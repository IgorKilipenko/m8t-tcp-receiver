
WiFiManager::WiFiManager(const char *ssid, const char *password, const char *ap_ssid, const char *ap_password) {
	this->ssid = utils::copynewstr(ssid, 32);
	this->password = utils::copynewstr(password, 32);
	this->ap_ssid = utils::copynewstr(ap_ssid, 32);
	this->ap_password = utils::copynewstr(ap_password, 32);
}
WiFiManager::~WiFiManager() {}

void WiFiManager::setup() {
	stationConnectedHandler = WiFi.onSoftAPModeStationConnected(std::bind(&WiFiManager::onStationConnected, this, std::placeholders::_1));
	stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(std::bind(&WiFiManager::onStationDisconnected, this, std::placeholders::_1));
}

bool WiFiManager::setApModeOnly() { return WiFi.mode(WIFI_AP); }

void WiFiManager::onStationConnected(const WiFiEventSoftAPModeStationConnected &evt) {
	logger.print("Station connected: ");
	logger.println(utils::macToString(evt.mac));
}

void WiFiManager::onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected &evt) {
	logger.print("Station disconnected: ");
	logger.println(utils::macToString(evt.mac));
}

void WiFiManager::WiFiPrintEvent(WiFiEvent_t event) {
	Serial.printf("[WiFi-event] event: %d\n", event);

	switch (event) {
	case SYSTEM_EVENT_WIFI_READY:
		logger.debug("WiFi interface ready\n");
		break;
	case SYSTEM_EVENT_SCAN_DONE:
		logger.debug("Completed scan for access points\n");
		break;
	case SYSTEM_EVENT_STA_START:
		logger.debug("WiFi client started\n");
		break;
	case SYSTEM_EVENT_STA_STOP:
		logger.debug("WiFi clients stopped\n");
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		logger.debug("Connected to access point\n");
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		logger.debug("Disconnected from WiFi access point\n");
		break;
	case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
		logger.debug("Authentication mode of access point has changed\n");
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		logger.debug("Obtained IP address: [%s]\n", String{WiFi.localIP()});
		break;
	case SYSTEM_EVENT_STA_LOST_IP:
		logger.debug("Lost IP address and IP address is reset to 0\n");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
		logger.debug("WiFi Protected Setup (WPS): succeeded in enrollee mode\n");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_FAILED:
		logger.debug("WiFi Protected Setup (WPS): failed in enrollee mode\n");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
		logger.debug("WiFi Protected Setup (WPS): timeout in enrollee mode\n");
		break;
	case SYSTEM_EVENT_STA_WPS_ER_PIN:
		logger.debug("WiFi Protected Setup (WPS): pin code in enrollee mode\n");
		break;
	case SYSTEM_EVENT_AP_START:
		logger.debug("WiFi access point started\n");
		break;
	case SYSTEM_EVENT_AP_STOP:
		logger.debug("WiFi access point  stopped\n");
		break;
	case SYSTEM_EVENT_AP_STACONNECTED:
		logger.debug("Client connected\n");
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		logger.debug("Client disconnected\n");
		break;
	case SYSTEM_EVENT_AP_STAIPASSIGNED:
		logger.debug("Assigned IP address to client\n");
		break;
	case SYSTEM_EVENT_AP_PROBEREQRECVED:
		logger.debug("Received probe request\n");
		break;
	case SYSTEM_EVENT_GOT_IP6:
		logger.debug("IPv6 is preferred\n");
		break;
	case SYSTEM_EVENT_ETH_START:
		logger.debug("Ethernet started\n");
		break;
	case SYSTEM_EVENT_ETH_STOP:
		logger.debug("Ethernet stopped\n");
		break;
	case SYSTEM_EVENT_ETH_CONNECTED:
		logger.debug("Ethernet connected\n");
		break;
	case SYSTEM_EVENT_ETH_DISCONNECTED:
		logger.debug("Ethernet disconnected\n");
		break;
	case SYSTEM_EVENT_ETH_GOT_IP:
		logger.debug("Obtained IP address\n");
		break;
	}
}

void WiFiManager::setup(){
	    // delete old config
    WiFi.disconnect(true);
	delay(1000);

	WiFi.onEvent(std::bind(&WiFiEvent, this, std::placeholders::_1));

}