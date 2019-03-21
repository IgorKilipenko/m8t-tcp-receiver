bool AWebServer::_static_init = false;

const char *AWebServer::API_P_GPSCMD = "cmd";

AWebServer::AWebServer(ATcpServer *telnetServer)
	: softAP_ssid{APSSID}, softAP_password{APPSK}, ssid{APSSID}, password{APPSK}, hostName{"GPS IoT "},server{80}, ws{"/ws"}, events{"/events"}, ubxMsgSource{"/ubxmsg"}, telnetServer{telnetServer}, wifiList{}, _ubxDecoder{} ,api{} {
	String id = utils::getEspChipId();
	strcat(softAP_ssid, id.c_str());
	strcat(hostName, id.c_str());
}

AWebServer::~AWebServer() { end(); }

void AWebServer::end() {
	SPIFFS.end();
	wifiList.clear();
	if (telnetServer) {
		telnetServer->stopReceive();
		// delete telnetServer;
		// telnetServer = nullptr;
	}
	ws.closeAll();
	if (ws.enabled()) {
		ws.enable(false);
	}
	server.reset();
}

void AWebServer::restart() {
	end();
	setup();
}

void AWebServer::setup() {
	loadWiFiCredentials();
#ifdef ESP32
	WiFi.setHostname(hostName);
#else
	WiFi.hostname(hostName);
#endif
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(softAP_ssid, softAP_password);

	connectStaWifi(ssid, password);

	addOTAhandlers();

	ArduinoOTA.setHostname(hostName);
	ArduinoOTA.begin();

	addReceiverHandlers();

	MDNS.addService("http", "tcp", 80);

	SPIFFS.begin();

	addServerHandlers();

	int8_t nets = scanWiFi();
	if (nets > 0) {
		wifiList.clear();
	}
	telnetServer->setup();

	initDefaultHeaders();
	server.begin();
}

/** Credentials ================================================ */
/** Load WLAN credentials from EEPROM */
void AWebServer::loadWiFiCredentials() {
	logger.trace("Start load Credential");

	logger.debug("ssid sizeof: ");
	logger.debug("%i\n", sizeof(ssid));
	logger.debug("EEROM start\n");
	EEPROM.begin(512);
	logger.debug("EEROM begin\n");
	EEPROM.get(0, ssid);
	EEPROM.get(0 + sizeof(ssid), password);
	char ok[2 + 1];
	EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
	EEPROM.end();
	if (String(ok) != String("OK")) {
		ssid[0] = 0;
		password[0] = 0;
	}
	logger.println("Recovered credentials:");
	logger.println(ssid);
	logger.println(strlen(password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void AWebServer::saveWiFiCredentials() {
	logger.trace("Start save Credential");

	EEPROM.begin(512);
	EEPROM.put(0, ssid);
	EEPROM.put(0 + sizeof(ssid), password);
	char ok[2 + 1] = "OK";
	EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
	EEPROM.commit();
	EEPROM.end();
}

/** Set default headers */
void AWebServer::initDefaultHeaders() {
	if (!AWebServer::_static_init) {
		DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
		// DefaultHeaders::Instance().addHeader("Access-Control-Expose-Headers","Access-Control-Allow-Origin");
		DefaultHeaders::Instance().addHeader("charset", "ANSI");
		// DefaultHeaders::Instance().addHeader("charset", "Windows-1252");
		DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,HEAD,OPTIONS,POST,PUT");
		DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers",
											 "Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers");
		AWebServer::_static_init = true;
	}
}

/** Scan WiFi Networks */
int8_t AWebServer::scanWiFi() {
	if (wifiList.size() > 0) {
		wifiList.clear();
	}
	int n = WiFi.scanComplete();
	if (n == -2) {
		WiFi.scanNetworks(true);
	} else if (n) {
		for (int i = 0; i < n; ++i) {
			auto wifi = utils::make_unique<WifiItem>();
			wifi->rssi = WiFi.RSSI(i);
			wifi->ssid = WiFi.SSID(i);
			wifi->bssid = WiFi.BSSIDstr(i);
			wifi->channel = WiFi.channel(i);
			wifi->secure = WiFi.encryptionType(i);
#ifdef ESP8266
			wifi->hidden = WiFi.isHidden(i) ? "true" : "false";
#endif
			wifiList.push_back(std::move(wifi));
		}
		WiFi.scanDelete();
		if (WiFi.scanComplete() == -2) {
			WiFi.scanNetworks(true);
		}
	}
	return n;
}

bool AWebServer::connectStaWifi(const char *ssid, const char *password) {
	logger.debug("connectStaWifi -> ssid : %s, password: %s\n", ssid, password);
	logger.debug("connectStaWifi -> start connect WiFi\n");
	if (ssid && password) {
		if (WiFi.isConnected()) {
			logger.debug("connectStaWifi -> disconnect\n");
			delay(100);
			disconnectStaWifi();
			delay(1000);
		}
		logger.debug("connectStaWifi -> begin, ssid : %s, pass: %s\n", ssid, password);
		WiFi.mode(WIFI_AP_STA);
		WiFi.begin(ssid, password);
		logger.debug("connectStaWifi -> wait connectiom status\n");
		delay(1000);
		uint64_t start = millis();
		while (WiFi.waitForConnectResult() != WL_CONNECTED && millis() - start < 6000) {
			// if (WiFi.status() != WL_CONNECTED){
			logger.printf("STA: Failed!\n");
			return false;
		}
		logger.debug("connectStaWifi -> STA connected\n");
		// restart();
	} else {
		logger.println("WiFi connected");
		logger.print("IP address: ");
		logger.println(WiFi.localIP());
		return true;
	}
	return true;
}

void AWebServer::disconnectStaWifi() {
	// WiFi.setAutoConnect(false);
	// WiFi.setAutoReconnect(false);
	if (telnetServer != nullptr && telnetServer->isInProgress()) {
		telnetServer->stopReceive();
	}
	WiFi.disconnect(false);
}

/** Main process */
void AWebServer::process() {
	if (_connect) {
		if (connectStaWifi(ssid, password)) {
			_connect = false;
			logger.debug("Reconnected\n");
			saveWiFiCredentials();
			logger.debug("Saved WiFi credentials\n");
		}
	}

	if (!telnetServer->isInProgress()) {
		ArduinoOTA.handle();
	} else {
		telnetServer->process();
	}
}