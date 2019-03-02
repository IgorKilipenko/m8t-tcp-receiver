bool AWebServer::_static_init = false;

const char *AWebServer::API_P_GPSCMD = "cmd";

AWebServer::AWebServer(ATcpServer *telnetServer)
	: softAP_ssid{APSSID}, softAP_password{APPSK}, ssid{APSSID}, password{APPSK}, server{80}, ws{"/ws"}, events{"/events"}, telnetServer{telnetServer}, wifiList{}, api{} {
	String id(ESP.getChipId());
	strcat(softAP_ssid, id.c_str());
}

AWebServer::~AWebServer() {
	if (telnetServer) {
		delete telnetServer;
		telnetServer = nullptr;
	}
}

void AWebServer::setup() {
	loadWiFiCredentials();

	// WiFi.hostname(hostName);
	WiFi.mode(WIFI_AP_STA);
	WiFi.softAP(softAP_ssid, softAP_password);

	connectStaWifi(ssid, password);

	// Send OTA events to the browser
	ArduinoOTA.onStart([&]() { events.send("Update Start", "ota"); });
	ArduinoOTA.onEnd([&]() { events.send("Update End", "ota"); });
	ArduinoOTA.onProgress([&](unsigned int progress, unsigned int total) {
		char p[32];
		sprintf(p, "Progress: %u%%\n", (progress / (total / 100)));
		events.send(p, "ota");
	});
	ArduinoOTA.onError([&](ota_error_t error) {
		if (error == OTA_AUTH_ERROR)
			events.send("Auth Failed", "ota");
		else if (error == OTA_BEGIN_ERROR)
			events.send("Begin Failed", "ota");
		else if (error == OTA_CONNECT_ERROR)
			events.send("Connect Failed", "ota");
		else if (error == OTA_RECEIVE_ERROR)
			events.send("Recieve Failed", "ota");
		else if (error == OTA_END_ERROR)
			events.send("End Failed", "ota");
	});
	ArduinoOTA.setHostname(hostName);
	ArduinoOTA.begin();

	MDNS.addService("http", "tcp", 80);

	SPIFFS.begin();

	ws.onEvent(std::bind(&AWebServer::wsEventHnadler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

	server.addHandler(&ws);

	events.onConnect([&](AsyncEventSourceClient *client) { client->send("hello!", NULL, millis(), 1000); });

	server.addHandler(&events);

	server.addHandler(new SPIFFSEditor(http_username, http_password));

	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", String(ESP.getFreeHeap())); });

	server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

	server.onNotFound(std::bind(&AWebServer::notFoundHandler, this, std::placeholders::_1));

	server.onFileUpload([&](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
		if (!index)
			logger.printf("UploadStart: %s\n", filename.c_str());
		logger.printf("%s", (const char *)data);
		if (final)
			logger.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
	});
	server.onRequestBody([&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
		if (!index)
			logger.printf("BodyStart: %u\n", total);
		logger.printf("%s", (const char *)data);
		if (index + len == total)
			logger.printf("BodyEnd: %u\n", total);
	});

	/* API  ================================ */
	server.on("/api/wifi/scan", HTTP_GET, [&](AsyncWebServerRequest *request) {
		logger.debug("WIFI Scan request\n");

		String json = "[";
		int n = WiFi.scanComplete();
		if (n == -2) {
			WiFi.scanNetworks(true);
		} else if (n) {
			for (int i = 0; i < n; ++i) {
				if (i)
					json += ",";
				json += "{";
				json += "\"rssi\":" + String(WiFi.RSSI(i));
				json += ",\"ssid\":\"" + WiFi.SSID(i) + "\"";
				json += ",\"bssid\":\"" + WiFi.BSSIDstr(i) + "\"";
				json += ",\"channel\":" + String(WiFi.channel(i));
				json += ",\"secure\":" + String(WiFi.encryptionType(i));
				json += ",\"hidden\":" + String(WiFi.isHidden(i) ? "true" : "false");
				json += "}";
			}
			WiFi.scanDelete();
			if (WiFi.scanComplete() == -2) {
				WiFi.scanNetworks(true);
			}
		}
		json += "]";

		AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
		request->send(response);
		json = String();
	});

#ifdef REST_API

	api.on(SGraphQL::WIFI, SGraphQL::QUERY, std::bind(&AWebServer::wifiQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	//api.on(SGraphQL::WIFI, SGraphQL::ACTION, std::bind(&AWebServer::wifiActionHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	// REST API handler
	AsyncCallbackJsonWebHandler *apiHandler = new AsyncCallbackJsonWebHandler("/api", [&](AsyncWebServerRequest *request, JsonVariant &json) {
		logger.debug("API \n");
		if (!json) {
			logger.debug("apiHandler -> Json is empty\n");
			request->send(404);
			return;
		}
		const JsonObject &jsonObj = json.as<const JsonObject>();
		if (!jsonObj.success()) {
			logger.debug("apiHandler -> Json parsing failed\n");
			request->send(404);
			return;
		}

		AsyncJsonResponse *response = new AsyncJsonResponse(true);
		JsonArray &root = response->getRoot();
		if (!api.parse(jsonObj, root)) {
			logger.debug("apiHandler -> Json API request parse failed\n");
			request->send(404);
			return;
		}
		logger.debug("apiHandler -> Json API request parse success\n");

		response->setLength();
		request->send(response);
		logger.debug("apiHandler -> Json API response send success\n");
		// delete response;
	});

	server.addHandler(apiHandler);

	AsyncCallbackJsonWebHandler *wifiStaConnectHandler = new AsyncCallbackJsonWebHandler("/api/srvice", [&](AsyncWebServerRequest *request, JsonVariant &jsonReq) {
		logger.debug("/api/srvice \n");
		if (!jsonReq) {
			logger.debug("apiHandler -> Json is empty\n");
			request->send(404);
			return;
		}
		const JsonObject &jsonObj = jsonReq.as<const JsonObject>();
		if (!jsonObj.success()) {
			logger.debug("apiHandler -> Json parsing failed\n");
			request->send(404);
			return;
		}

		AsyncJsonResponse *response = new AsyncJsonResponse(true);
		JsonArray &jsonResp = response->getRoot();

		if (jsonObj.containsKey("cmd")) {
			logger.debug("Contain cmd key, cmd: %s\n", jsonObj.get<const char *>("cmd"));
			const char *cmd = jsonObj.get<const char *>("cmd");
			if (utils::streq(cmd, "connect")) {
				if (!jsonObj.containsKey("ssid")) {
					logger.debug("SSID failed\n");
					return;
				}
				if (!jsonObj.containsKey("password")) {
					logger.debug("PASSWORD failed\n");
					return;
				}
				const char *new_ssid = jsonObj["ssid"];
				strcpy(this->ssid, new_ssid);
				const char *new_password = jsonObj["password"];
				strcpy(this->password, new_password);

				logger.debug("onWiFiActionRequest -> createNestedObject\n ");
				JsonObject &resJson = jsonResp.createNestedObject();

				if (!this->connectStaWifi(ssid, password)) {
					logger.debug("WiFi STA not connected\n");
					return;
				}
				delay(500);
				//resJson["status"] = WiFi.status();
				//String ip = utils::toStringIp(WiFi.localIP());
				//resJson["new_sta_ip"] = ip;
				//resJson["status"] = WiFi.status();
			}
		}

		response->setLength();
		request->send(response);
		logger.debug("apiHandler -> Json API response send success\n");
		// delete response;
	});

	server.addHandler(wifiStaConnectHandler);

#endif // REST_API

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
			wifi->hidden = WiFi.isHidden(i) ? "true" : "false";
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
			WiFi.disconnect(false);
		}
		logger.debug("connectStaWifi -> begin\n");
		WiFi.begin(ssid, password);
		logger.debug("connectStaWifi -> wait connectiom status\n");
		while (WiFi.waitForConnectResult() != WL_CONNECTED) {
			logger.printf("STA: Failed!\n");
			return false;
			// WiFi.disconnect(false);
			// delay(1000);
			// WiFi.begin(ssid, password);
		}
	} else {
		logger.println("WiFi connected");
		logger.print("IP address: ");
		logger.println(WiFi.localIP());
		return true;
	}
	return true;
}

/** Main process */
void AWebServer::process() { ArduinoOTA.handle(); }