
WiFiManager::WiFiManager(const char *ssid, const char *password, const char *ap_ssid, const char *ap_password) {
    this->ssid = utils::copynewstr(ssid, 32);
    this->password = utils::copynewstr(password, 32);
    this->ap_ssid = utils::copynewstr(ap_ssid, 32);
    this->ap_password = utils::copynewstr(ap_password, 32);
}
WiFiManager::~WiFiManager(){}

void WiFiManager::setup(){
    stationConnectedHandler = WiFi.onSoftAPModeStationConnected(std::bind(&WiFiManager::onStationConnected, this, std::placeholders::_1));
    stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(std::bind(&WiFiManager::onStationDisconnected, this, std::placeholders::_1));
}

bool WiFiManager::setApModeOnly(){
    return WiFi.mode(WIFI_AP);
}

void WiFiManager::onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  logger.print("Station connected: ");
  logger.println(utils::macToString(evt.mac));
}

void WiFiManager::onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  logger.print("Station disconnected: ");
  logger.println(utils::macToString(evt.mac));
}