/** WebSocket events */
void AWebServer::wsEventHnadler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
	log_d("Start WS EVENT, [CORE ------> [%i]]\n", xPortGetCoreID());
	if (type == WS_EVT_CONNECT) {
		log_d("ws[%s][%u] connect\n", server->url(), client->id());
		client->printf("WS started, client id: %u :)", client->id());
		client->ping();

	} else if (type == WS_EVT_DISCONNECT) {
		log_d("ws[%s] disconnect: %u\n", server->url(), client->id());
		client->close();
	} else if (type == WS_EVT_ERROR) {
		log_e("Ws error\n");
		log_e("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
		if (client) {
			client->close();
		}

	} else if (type == WS_EVT_PONG) {
		log_d("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
	} else if (type == WS_EVT_DATA) {
		// AwsFrameInfo *info = (AwsFrameInfo *)arg;
	}
}

void AWebServer::notFoundHandler(AsyncWebServerRequest *request) {
	log_d("NOT_FOUND: , [CORE ------> [%i]]\n", xPortGetCoreID());
	if (request->method() == HTTP_GET)
		log_d("GET");
	else if (request->method() == HTTP_POST)
		log_d("POST");
	else if (request->method() == HTTP_DELETE)
		log_d("DELETE");
	else if (request->method() == HTTP_PUT)
		log_d("PUT");
	else if (request->method() == HTTP_PATCH)
		log_d("PATCH");
	else if (request->method() == HTTP_HEAD)
		log_d("HEAD");
	else if (request->method() == HTTP_OPTIONS) {
		// request->send(200);
		// return;
	} else
		log_d("UNKNOWN");
	log_d(" http://%s%s\n", request->host().c_str(), request->url().c_str());

	if (request->contentLength()) {
		log_v("_CONTENT_TYPE: %s\n", request->contentType().c_str());
		log_v("_CONTENT_LENGTH: %u\n", request->contentLength());
	}

	int headers = request->headers();
	int i;
	for (i = 0; i < headers; i++) {
		AsyncWebHeader *h = request->getHeader(i);
		log_v("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
	}

	int params = request->params();
	for (i = 0; i < params; i++) {
		AsyncWebParameter *p = request->getParam(i);
		if (p->isFile()) {
			log_v("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
		} else if (p->isPost()) {
			log_v("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
		} else {
			log_v("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
		}
	}

	// NotFount Page in React-Router
	request->send(SPIFFS, "/www/index.html");
}

ApiResultPtr AWebServer::wifiQueryHandler(const char *event, const JsonObject &json, JsonObject &outJson) {
	log_d("Start on wifi QUERY, [CORE ------> [%i]]\n", xPortGetCoreID());
	//logger.debug(":");
	//json.prettyPrintTo(logger);
	//logger.debug("\n");

	const char *cmd = json.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "scan")) {
		log_d("Start scan WIFI, [CORE ------> [%i]]\n", xPortGetCoreID());
		delay(100);
		/*int8_t n = scanWiFi();
		if (n == -1) {
			delay(1000);
			n = scanWiFi();
		}
		JsonArray &arrayJson = outJson.createNestedArray(SGraphQL::RESP_VALUE);
		if (n > 0) {
			for (auto const &item : wifiList) {
				JsonObject &resJson = arrayJson.createNestedObject();
				resJson["rssi"] = item->rssi;
				resJson["ssid"] = item->ssid;
				resJson["bssid"] = item->bssid;
				resJson["channel"] = item->channel;
				resJson["secure"] = item->secure;
				resJson["hidden"] = item->hidden;
			}
			logger.print("\n");
		}
		if (wifiList.size() > 0) {
			wifiList.clear();
		}*/

		JsonArray &arrayJson = outJson.createNestedArray(SGraphQL::RESP_VALUE);

		if (WM.staConnected()) {
			WM.scanWiFiAsync();
			/*/utils::waitAtTime(
				[&]() {
					log_d("Wait.... DONE -> [%s], CORE -----> [%i]\n", WM.lastScanComplete() ? "TRUE" : "FALSE", xPortGetCoreID());
					return WM.lastScanComplete();
				},
				10000, 10);*/
				
			if (WM.lastScanComplete()) {

				ApRecords list{};
				WM.getApRecords(list);
				log_d("Record size = [%i]", list.size());
				for (auto const &item : list) {
					JsonObject &resJson = arrayJson.createNestedObject();
					resJson["rssi"] = item->rssi;
					resJson["ssid"] = item->ssid;
					resJson["bssid"] = item->bssid;
					resJson["channel"] = item->channel;
					resJson["secure"] = item->secure;
					resJson["hidden"] = item->hidden;
				}
			}
		}

		/*
		log_v("========= CORE -> [%i]", xPortGetCoreID());
		WM.onWifiScanDone([&](ApRecords list){
			log_v("========= CORE -> [%i]", xPortGetCoreID());
			if (WM.lastScanComplete()) {
				log_v("========= CORE -> [%i]", xPortGetCoreID());
				//ApRecords list{};
				//WM.getApRecords(list);
				log_d("Record size = [%i]", list.size());
				for (auto const &item : list) {
					//JsonObject &resJson = arrayJson.createNestedObject();
					//resJson["rssi"] = item->rssi;
					//resJson["ssid"] = item->ssid;
					//resJson["bssid"] = item->bssid;
					//resJson["channel"] = item->channel;
					//resJson["secure"] = item->secure;
					//resJson["hidden"] = item->hidden;
					log_d("Record size = [%s]", item->ssid);
				}
			}
		}, true);
		WM.scanWiFiAsync();
		*/

	} else if (utils::streq(cmd, "info")) {
		log_d("Start WIFI info, [CORE ------> [%i]]\n", xPortGetCoreID());

		JsonObject &resJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);

		uint8_t mode = WiFi.getMode();
		if (!mode) {
			log_e("WiFi disconnected, WiFi Mode = [%s]\n", utils::wiFiModeToString(mode).c_str());
			return nullptr;
		}
		resJson["mode"] = utils::wiFiModeToString(mode);
		if (mode == WIFI_AP || mode == WIFI_AP_STA) {
			JsonObject &apJson = resJson.createNestedObject("ap");
			apJson["ip"] = utils::toStringIp(WiFi.softAPIP());
#ifdef ESP32
			apJson["apHostName"] = WiFi.softAPgetHostname();
#else
			apJson["ssid"] = WiFi.softAPSSID();
#endif
			apJson["station_num"] = WiFi.softAPgetStationNum();
		}
		if (mode == WIFI_STA || WIFI_AP_STA) {
			JsonObject &staJson = resJson.createNestedObject("sta");
			staJson["local_ip"] = utils::toStringIp(WiFi.localIP());
			staJson["ssid"] = WiFi.SSID();
			staJson["rssi"] = WiFi.RSSI();
#ifdef ESP32
			staJson["hostname"] = WiFi.getHostname();
#else
			staJson["hostname"] = WiFi.hostname();
#endif
		}

	} else {
		return nullptr;
	}
	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());
	return res_ptr;
}

ApiResultPtr AWebServer::wifiActionHandler(const char *event, const JsonObject &json, JsonObject &outJson) {
	log_d("Start on wifi ACTION, [CORE ------> [%i]]\n", xPortGetCoreID());
	//json.prettyPrintTo(logger);
	//logger.debug("\n");

	const char *cmd = json.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "connect")) {
		if (!json.containsKey("ssid")) {
			log_w("SSID failed\n");
			return nullptr;
		}
		if (!json.containsKey("password")) {
			log_w("PASSWORD failed\n");
			return nullptr;
		}
		const char *new_ssid = json["ssid"];
		strcpy(this->ssid, new_ssid);
		const char *new_password = json["password"];
		strcpy(this->password, new_password);
		JsonObject &resJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);

		resJson["status"] = WiFi.status();
		String ip = utils::toStringIp(WiFi.localIP());
		resJson["sta_ip"] = ip;

		ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

		res_ptr->addAction([&](AsyncWebServerRequest *request) {
			logger.trace("Start Then Action\n");
			logger.debug("SSID: %s\n", ssid);
			logger.debug("PASW: %s\n", password);

			_connect = true;
			return 0;
		});

		return res_ptr;
	}

	return nullptr;
}

ApiResultPtr AWebServer::receiverActionHandler(const char *event, const JsonObject &json, JsonObject &outJson) {
	log_d("Start on receiver ACTION, [CORE ------> [%i]]\n", xPortGetCoreID());
	//json.prettyPrintTo(logger);
	//logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

	const char *cmd = json.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, SGraphQL::CMD_START) || utils::streq(cmd, SGraphQL::CMD_STOP)) {
		log_v("Start Receiver Action, cmd: {%s}\n", cmd);
		JsonObject &objJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);
		log_v("Json root created\n");
		if (utils::streq(cmd, SGraphQL::CMD_START)) {

			if (telnetServer->isInProgress()) {
				const char *msg = "Already enabled\n";
				log_d("%s", msg);
				outJson[SGraphQL::RESP_MSG] = msg;
				// objJson["timeReceive"] = telnetServer->getTimeReceive();
			} else {
				logger.trace("SET -> Start receive\n");
				bool writeToSd = telnetServer->writeToSdEnabled();
				bool sendToTcp = telnetServer->sendToTcpEnabled();
				if (json.containsKey("writeToSd")) {
					writeToSd = json.get<bool>("writeToSd");
				}
				if (json.containsKey("sendToTcp")) {
					sendToTcp = json.get<bool>("sendToTcp");
				}
				telnetServer->startReceive(writeToSd, sendToTcp);
			}
			objJson["timeStart"] = telnetServer->getTimeStart();

		} else {
			if (!telnetServer->isInProgress()) {
				const char *msg = "Already disabled\n";
				log_d("%s", msg);
				outJson[SGraphQL::RESP_MSG] = msg;
			} else {
				log_v("SET -> Stop receive\n");
				// objJson["timeReceive"] = telnetServer->getTimeReceive();
				telnetServer->stopReceive();
			}
		}
		objJson["enabled"] = telnetServer->isInProgress();
		return res_ptr;
	}

	JsonObject &errJson = outJson.createNestedObject(SGraphQL::RESP_ERR);
	errJson[SGraphQL::RESP_MSG] = String("Not found command : ") + cmd;

	return res_ptr;
}

ApiResultPtr AWebServer::receiverQueryHandler(const char *event, const JsonObject &reqJson, JsonObject &outJson) {
	log_d("Start on receiver QUERY, [CORE ------> [%i]]\n", xPortGetCoreID());
	//reqJson.prettyPrintTo(logger);
	//logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

	const char *cmd = reqJson.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "state")) {
		log_v("Start Receiver Query\n");
		JsonObject &objJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);
		objJson["enabled"] = telnetServer->isInProgress() ? true : false;
		objJson["writeToSd"] = telnetServer->writeToSdEnabled();
		objJson["sendToTcp"] = telnetServer->sendToTcpEnabled();
		objJson["sendToWs"] = _sendReceiverDataToWs;
		if (telnetServer->isInProgress()) {
			objJson["timeStart"] = telnetServer->getTimeStart();
			objJson["timeReceive"] = telnetServer->getTimeReceive();
		}

		return res_ptr;
	}

	JsonObject &errJson = outJson.createNestedObject("error");
	errJson["message"] = String("Not found command : ") + cmd;

	return res_ptr;
}

ApiResultPtr AWebServer::serverQueryHandler(const char *event, const JsonObject &reqJson, JsonObject &outJson) {
	log_d("Start server QUERY, [CORE ------> [%i]]\n", xPortGetCoreID());
	//reqJson.prettyPrintTo(logger);
	//logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

	const char *cmd = reqJson.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "info")) {
		log_v("Start Server Query\n");
		JsonObject &objJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);
		objJson["serverTime"] = getServerTime();
		objJson["sdSuccess"] = telnetServer->isSdInitialize();
		return res_ptr;
	}

	JsonObject &errJson = outJson.createNestedObject(SGraphQL::RESP_ERR);
	errJson["message"] = String("Not found command : ") + cmd;

	return res_ptr;
}

ApiResultPtr AWebServer::ntripActionHandler(const char *event, const JsonObject &json, JsonObject &outJson) {
	log_d("Start on rntrip ACTION, [CORE ------> [%i]]\n", xPortGetCoreID());
	//json.prettyPrintTo(logger);
	//logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());
	const char *cmd = json.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, SGraphQL::CMD_START) || utils::streq(cmd, SGraphQL::CMD_STOP)) {
		log_v("Start NtripClient Action, cmd: {%s}\n", cmd);
		JsonObject &objJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);
		log_v("Json root created\n");

		if (utils::streq(cmd, SGraphQL::CMD_START)) { // Enable Ntrip Client
			if (_ntripClient->isEnabled()) {
				const char *msg = "Ntrip Client already receiving data / enabled\n";
				log_d("%s", msg);
				outJson[SGraphQL::RESP_MSG] = msg;
			} else {
				const char *host = json.containsKey("host") ? json.get<const char *>("host") : /*"192.168.1.50"*/ "82.202.202.138";
				uint16_t port = json.containsKey("port") ? json.get<uint16_t>("port") : /*7048*/ 2102;
				const char *mntpnt = json.containsKey("mountPoint") ? json.get<const char *>("mountPoint") : "NVSB3_2";
				const char *user = json.containsKey("user") ? json.get<const char *>("user") : "sbr5037";
				const char *passwd = json.containsKey("password") ? json.get<const char *>("password") : "pass";

				if (_ntripClient->connect(host, port, user, passwd, mntpnt)) {
					log_d("Ntrip client connected");
				}
			}
		} else {
			if (!_ntripClient->isEnabled()) {
				const char *msg = "Ntrip Client already disabled\n";
				log_d("%s", msg);
				outJson[SGraphQL::RESP_MSG] = msg;
			} else {
				_ntripClient->stop();
			}
		}

		objJson["enabled"] = _ntripClient->isEnabled();
		return res_ptr;
	}

	/* Not Found Command */
	char msg[256]{0};
	sprintf(msg, "Ntrip Client Not found command: [%s]\n", cmd);
	JsonObject &errJson = outJson.createNestedObject(SGraphQL::RESP_ERR);
	errJson["message"] = msg;
	return res_ptr;
}

ApiResultPtr AWebServer::ntripQueryHandler(const char *event, const JsonObject &reqJson, JsonObject &outJson) {
	log_d("Start Ntrip QUERY, [CORE ----> [%i]]\n", xPortGetCoreID());
	//reqJson.prettyPrintTo(logger);
	//logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

	const char *cmd = reqJson.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "state")) {
		log_v("Start Ntrip Query State\n");
		JsonObject &objJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);
		objJson["enabled"] = _ntripClient && _ntripClient->isEnabled();

		return res_ptr;
	}

	JsonObject &errJson = outJson.createNestedObject("error");
	errJson["message"] = String("Not found command : ") + cmd;

	return res_ptr;
}

void AWebServer::addServerHandlers() {
	ws.onEvent(std::bind(&AWebServer::wsEventHnadler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

	server.addHandler(&ws);

	/*logger.setEventSource(&events);  ERROR!!!!!!*/

	events.onConnect([&](AsyncEventSourceClient *client) { client->send("EventService ESP GPS", NULL, millis(), 1000); });

	server.addHandler(&events);

#ifdef ESP32
	server.addHandler(new SPIFFSEditor(SPIFFS, http_username, http_password));
#else
	server.addHandler(new SPIFFSEditor(http_username, http_password, SPIFFS));
#endif

	// server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", String(ESP.getFreeHeap())); });

	server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

	server.onNotFound(std::bind(&AWebServer::notFoundHandler, this, std::placeholders::_1));

	server.onRequestBody([&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
		if (!index)
			logger.printf("BodyStart: %u\n", total);
		logger.printf("%s", (const char *)data);
		if (index + len == total)
			logger.printf("BodyEnd: %u\n", total);
	});

	// REST API handler
	AsyncCallbackJsonWebHandler *apiHandler = new AsyncCallbackJsonWebHandler("/api", [&](AsyncWebServerRequest *request, JsonVariant &json) {
		log_d("API \n");
		if (!json) {
			log_d("apiHandler -> Json is empty\n");
			request->send(404);
			return;
		}
		const JsonObject &jsonObj = json.as<const JsonObject>();
		if (!jsonObj.success()) {
			log_w("apiHandler -> Json parsing failed\n");
			request->send(404);
			return;
		}

		AsyncJsonResponse *response = new AsyncJsonResponse(false);
		JsonObject &root = response->getRoot();

		ApiResultPtr res_ptr = api.parse(jsonObj, root);

		if (!res_ptr) {
			log_w("apiHandler -> ApiResultPtr is nullptr\n");
			request->send(404);
			delete response;
			return;
		}
		log_v("apiHandler -> Json API request parse success\n");

		response->setLength();
		request->send(response);
		log_v("apiHandler -> Json API response send success\n");

		log_v("Then action count %i\n", res_ptr->actionCount());

		while (res_ptr->actionCount() > 0) {
			const uint8_t err_code = res_ptr->then(request);
			if (err_code == 0) {
				log_v("Then Action success\n");
			} else {
				log_e("Then action complite with err code: {%i}");
			}
		}
	});

	api.on(SGraphQL::WIFI, SGraphQL::QUERY, std::bind(&AWebServer::wifiQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::WIFI, SGraphQL::ACTION, std::bind(&AWebServer::wifiActionHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::GPS, SGraphQL::QUERY, std::bind(&AWebServer::receiverQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::GPS, SGraphQL::ACTION, std::bind(&AWebServer::receiverActionHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::SERVER, SGraphQL::QUERY, std::bind(&AWebServer::serverQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::NTRIP, SGraphQL::ACTION, std::bind(&AWebServer::ntripActionHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::NTRIP, SGraphQL::QUERY, std::bind(&AWebServer::ntripQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	server.addHandler(apiHandler);
}

void AWebServer::addOTAhandlers() {
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
			// events.send("Auth Failed", "ota");
			logger.error("OTA Auth Failed");
		else if (error == OTA_BEGIN_ERROR)
			// events.send("Begin Failed", "ota");
			logger.error("OTA Begin Failed");
		else if (error == OTA_CONNECT_ERROR)
			// events.send("Connect Failed", "ota");
			logger.error("OTA Connect Failed");
		else if (error == OTA_RECEIVE_ERROR)
			// events.send("Recieve Failed", "ota");
			logger.error("OTA Recieve Failed");
		else if (error == OTA_END_ERROR)
			// events.send("End Failed", "ota");
			logger.error("OTA End Failed");
	});
}

void AWebServer::addReceiverHandlers() { telnetServer->onSerialData(std::bind(&AWebServer::receiverDataHandler, this, std::placeholders::_1, std::placeholders::_2)); }

void AWebServer::receiverDataHandler(const uint8_t *buffer, size_t len) {
	if (!isCanSendData()) {
		log_d("Is't can sand data\n");
		delay(1);
		return;
	}
	//if (!ws.count()) {
	//	log_d("Ws not connected\n");
	//	return;
	//}

	if (_decodeUbxMsg) {
		for (uint16_t i = 0; i < len; i++) {
			const int8_t code = _ubxDecoder.inputData(buffer[i]);
			const bool isClsId = (code > 0 && _ubxDecoder.getLength() > 0);

			if (!isClsId) {
				continue;
			}

			const uint8_t *ubxPacket = _ubxDecoder.getBuffer();
			const uint16_t packetLen = _ubxDecoder.getLength();
			bool hasMsg = false;

			logger.debug("ClassId: [%X], MsgId: [%X]\n", ubxPacket[2], ubxPacket[3]);

			const uint8_t clsId = ubxPacket[2];

			switch (clsId) {
			case static_cast<uint8_t>(ClassIds::NAV):

				switch (ubxPacket[3]) {
				case static_cast<uint8_t>(NavMessageIds::PVT):
					logger.trace("Has PVT msg\n");
					hasMsg = true;
					break;
				case static_cast<uint8_t>(NavMessageIds::POSLLH):
					logger.trace("Has NAV POSLLH msg\n");
					hasMsg = true;
					break;
				case static_cast<uint8_t>(NavMessageIds::HPPOSLLH):
					logger.trace("Has NAV HPPOSLLH msg\n");
					hasMsg = true;
					break;
				}
				break;

			case static_cast<uint8_t>(ClassIds::CFG):
				switch (ubxPacket[3]) {
				case static_cast<uint8_t>(CfgMessageIds::RATE):
					logger.trace("Has CFG RATE msg\n");
					hasMsg = true;
					break;
				}
				break;
			case static_cast<uint8_t>(ClassIds::RXM):
				hasMsg = false; // Not send raw data to ws
				break;

			default:
				hasMsg = true;
				break;
			}

			if (hasMsg) {
				logger.debug("Send to ws [%ld] bytes", len);
				ws.binaryAll((const char *)ubxPacket, packetLen);
			}
		}

		delay(1);
	} else {
		for (int i = 0; i < len; i++) {
			_ubxWsBuffer.push(buffer[i]);
		}
		const size_t size = _ubxWsBuffer.size();
		// const unsigned long waitTime = millis() - _lastUbxWsSendTime;
		if (size >= 1024 * 2 || (size > 0 && (millis() - _lastUbxWsSendTime > 1000))) {
			uint8_t buff[size]{0};
			for (int i = 0; i < size; i++) {
				buff[i] = _ubxWsBuffer.front();
				_ubxWsBuffer.pop();
			}
			ws.binaryAll((const char *)buff, size);
			_lastUbxWsSendTime = millis();
		}
	}
}