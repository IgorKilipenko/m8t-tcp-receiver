/** WebSocket events */
void AWebServer::wsEventHnadler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
	if (type == WS_EVT_CONNECT) {
		logger.printf("ws[%s][%u] connect\n", server->url(), client->id());
		client->printf("WS started, client id: %u :)", client->id());
		client->ping();

		//              telnetServer->onSerialData([&](const uint8_t *buf, size_t len) {
		//              	logger.trace("Send data to WS\n");
		//              	if (_sendReceiverDataToWs && client != nullptr && client->status() == WS_CONNECTED) {
		//              		client->binary(reinterpret_cast<const char *>(buf), len);
		//              	}
		//              });

	} else if (type == WS_EVT_DISCONNECT) {
		logger.printf("ws[%s] disconnect: %u\n", server->url(), client->id());
	} else if (type == WS_EVT_ERROR) {
		logger.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
	} else if (type == WS_EVT_PONG) {
		logger.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
	} else if (type == WS_EVT_DATA) {
		AwsFrameInfo *info = (AwsFrameInfo *)arg;
		String msg = "";
		if (info->final && info->index == 0 && info->len == len) {
			// the whole message is in a single frame and we got all of it's data
			logger.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

			if (info->opcode == WS_TEXT) {
				for (size_t i = 0; i < info->len; i++) {
					msg += (char)data[i];
				}
			} else {
				char buff[3];
				for (size_t i = 0; i < info->len; i++) {
					sprintf(buff, "%02x ", (uint8_t)data[i]);
					msg += buff;
				}
			}
			logger.printf("%s\n", msg.c_str());

			if (info->opcode == WS_TEXT)
				client->text("I got your text message");
			else
				client->binary("I got your binary message");
		} else {
			// message is comprised of multiple frames or the frame is split into multiple packets
			if (info->index == 0) {
				if (info->num == 0)
					logger.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
				logger.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
			}

			logger.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

			if (info->opcode == WS_TEXT) {
				for (size_t i = 0; i < info->len; i++) {
					msg += (char)data[i];
				}
			} else {
				char buff[3];
				for (size_t i = 0; i < info->len; i++) {
					sprintf(buff, "%02x ", (uint8_t)data[i]);
					msg += buff;
				}
			}
			logger.printf("%s\n", msg.c_str());

			if ((info->index + len) == info->len) {
				logger.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
				if (info->final) {
					logger.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
					if (info->message_opcode == WS_TEXT)
						client->text("I got your text message");
					else
						client->binary("I got your binary message");
				}
			}
		}
	}
}

void AWebServer::notFoundHandler(AsyncWebServerRequest *request) {
	logger.printf("NOT_FOUND: ");
	if (request->method() == HTTP_GET)
		logger.printf("GET");
	else if (request->method() == HTTP_POST)
		logger.printf("POST");
	else if (request->method() == HTTP_DELETE)
		logger.printf("DELETE");
	else if (request->method() == HTTP_PUT)
		logger.printf("PUT");
	else if (request->method() == HTTP_PATCH)
		logger.printf("PATCH");
	else if (request->method() == HTTP_HEAD)
		logger.printf("HEAD");
	else if (request->method() == HTTP_OPTIONS) {
		// request->send(200);
		// return;
	} else
		logger.printf("UNKNOWN");
	logger.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

	if (request->contentLength()) {
		logger.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
		logger.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
	}

	int headers = request->headers();
	int i;
	for (i = 0; i < headers; i++) {
		AsyncWebHeader *h = request->getHeader(i);
		logger.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
	}

	int params = request->params();
	for (i = 0; i < params; i++) {
		AsyncWebParameter *p = request->getParam(i);
		if (p->isFile()) {
			logger.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
		} else if (p->isPost()) {
			logger.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
		} else {
			logger.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
		}
	}

	// NotFount Page in React-Router
	request->send(SPIFFS, "/www/index.html");
}

ApiResultPtr AWebServer::wifiQueryHandler(const char *event, const JsonObject &json, JsonObject &outJson) {
	logger.trace("Start on wifi QUERY\n");
	logger.trace(":");
	json.prettyPrintTo(logger);
	logger.print("\n");

	const char *cmd = json.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "scan")) {
		logger.debug("Start scan WIFI\n");
		delay(500);
		int8_t n = scanWiFi();
		if (n == -1) {
			delay(1000);
			n = scanWiFi();
		}
		JsonArray &arrayJson = outJson.createNestedArray(SGraphQL::RESP_VALUE);
		logger.debug("Scan end, item count: %i\n", n);
		if (n > 0) {
			uint8_t i = 0;
			for (auto const &item : wifiList) {
				logger.debug("n:%i,", i++);
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
		}
	} else if (utils::streq(cmd, "info")) {
		logger.trace("Start WIFI info\n");

		JsonObject &resJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);

		uint8_t mode = WiFi.getMode();
		if (!mode) {
			logger.error("WiFi disconnected, WiFi Mode = [%s]\n", utils::wiFiModeToString(mode).c_str());
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
	logger.debug("Start on wifi ACTION\n");
	json.prettyPrintTo(logger);
	logger.debug("\n");

	const char *cmd = json.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "connect")) {
		if (!json.containsKey("ssid")) {
			logger.debug("SSID failed\n");
			return nullptr;
		}
		if (!json.containsKey("password")) {
			logger.debug("PASSWORD failed\n");
			return nullptr;
		}
		const char *new_ssid = json["ssid"];
		strcpy(this->ssid, new_ssid);
		const char *new_password = json["password"];
		strcpy(this->password, new_password);
		logger.debug("onWiFiActionRequest -> createNestedObject\n ");
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
	logger.debug("Start on receiver ACTION\n");
	json.prettyPrintTo(logger);
	logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

	const char *cmd = json.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, SGraphQL::CMD_START) || utils::streq(cmd, SGraphQL::CMD_STOP)) {
		logger.trace("Start Receiver Action, cmd: {%s}\n", cmd);
		JsonObject &objJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);
		logger.trace("Json root created\n");
		if (utils::streq(cmd, SGraphQL::CMD_START)) {

			if (telnetServer->isInProgress()) {
				const char *msg = "Already enabled\n";
				logger.debug(msg);
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
				logger.debug(msg);
				outJson[SGraphQL::RESP_MSG] = msg;
			} else {
				logger.trace("SET -> Stop receive\n");
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
	logger.debug("Start on receiver QUERY\n");
	reqJson.prettyPrintTo(logger);
	logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

	const char *cmd = reqJson.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "state")) {
		logger.trace("Start Receiver Query\n");
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
	logger.debug("Start server QUERY\n");
	reqJson.prettyPrintTo(logger);
	logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

	const char *cmd = reqJson.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "info")) {
		logger.trace("Start Server Query\n");
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
	logger.debug("Start on rntrip ACTION\n");
	json.prettyPrintTo(logger);
	logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());
	const char *cmd = json.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, SGraphQL::CMD_START) || utils::streq(cmd, SGraphQL::CMD_STOP)) {
		logger.trace("Start NtripClient Action, cmd: {%s}\n", cmd);
		JsonObject &objJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);
		logger.trace("Json root created\n");

		if (utils::streq(cmd, SGraphQL::CMD_START)) { // Enable Ntrip Client
			if (_ntripClient->isEnabled()) {
				const char *msg = "Ntrip Client already receiving data / enabled\n";
				logger.debug(msg);
				outJson[SGraphQL::RESP_MSG] = msg;
			} else {
				const char *host = json.containsKey("host") ? json.get<const char *>("host") : "192.168.1.50"/*"82.202.202.138"*/;
				uint16_t port = json.containsKey("port") ? json.get<uint16_t>("port") : 2102;
				const char *mntpnt = json.containsKey("mntpnt") ? json.get<const char *>("mntpnt") : "NVSB3_2";
				const char *user = json.containsKey("user") ? json.get<const char *>("user") : "sbr5037";
				const char *passwd = json.containsKey("passwd") ? json.get<const char *>("passwd") : "940172";

				if (_ntripClient->connect(host, port, user, passwd, mntpnt)){
					logger.debug("Ntrip client connected");
				}
			}
		} else {
			if (!_ntripClient->isEnabled()) {
				const char *msg = "Ntrip Client already disabled\n";
				logger.debug(msg);
				outJson[SGraphQL::RESP_MSG] = msg;
			}else{
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
	logger.debug("Start Ntrip QUERY\n");
	reqJson.prettyPrintTo(logger);
	logger.debug("\n");

	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

	const char *cmd = reqJson.get<const char *>(SGraphQL::CMD);
	if (utils::streq(cmd, "state")) {
		logger.trace("Start Ntrip Query\n");
		JsonObject &objJson = outJson.createNestedObject(SGraphQL::RESP_VALUE);
		objJson["enabled"] =  _ntripClient && _ntripClient->isEnabled();

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

#ifdef REST_API

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

		AsyncJsonResponse *response = new AsyncJsonResponse(false);
		JsonObject &root = response->getRoot();

		ApiResultPtr res_ptr = api.parse(jsonObj, root);

		if (!res_ptr) {
			logger.debug("apiHandler -> ApiResultPtr is nullptr\n");
			request->send(404);
			delete response;
			return;
		}
		logger.debug("apiHandler -> Json API request parse success\n");

		response->setLength();
		request->send(response);
		logger.debug("apiHandler -> Json API response send success\n");

		logger.debug("Then action count %i\n", res_ptr->actionCount());

		while (res_ptr->actionCount() > 0) {
			const uint8_t err_code = res_ptr->then(request);
			if (err_code == 0) {
				logger.debug("Then Action success\n");
			} else {
				logger.debug("Then action complite with err code: {%i}");
			}
		}

		// delete response;
	});

	api.on(SGraphQL::WIFI, SGraphQL::QUERY, std::bind(&AWebServer::wifiQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::WIFI, SGraphQL::ACTION, std::bind(&AWebServer::wifiActionHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::GPS, SGraphQL::QUERY, std::bind(&AWebServer::receiverQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::GPS, SGraphQL::ACTION, std::bind(&AWebServer::receiverActionHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::SERVER, SGraphQL::QUERY, std::bind(&AWebServer::serverQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::NTRIP, SGraphQL::ACTION, std::bind(&AWebServer::ntripActionHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::NTRIP, SGraphQL::QUERY, std::bind(&AWebServer::ntripQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	server.addHandler(apiHandler);

#endif // REST_API
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
			//events.send("Auth Failed", "ota");
			logger.error("OTA Auth Failed");
		else if (error == OTA_BEGIN_ERROR)
			//events.send("Begin Failed", "ota");
			logger.error("OTA Begin Failed");
		else if (error == OTA_CONNECT_ERROR)
			//events.send("Connect Failed", "ota");
			logger.error("OTA Connect Failed");
		else if (error == OTA_RECEIVE_ERROR)
			//events.send("Recieve Failed", "ota");
			logger.error("OTA Recieve Failed");
		else if (error == OTA_END_ERROR)
			//events.send("End Failed", "ota");
			logger.error("OTA End Failed");
	});
}

void AWebServer::addReceiverHandlers() { telnetServer->onSerialData(std::bind(&AWebServer::receiverDataHandler, this, std::placeholders::_1, std::placeholders::_2)); }

void AWebServer::receiverDataHandler(const uint8_t *buffer, size_t len) {
	if (_decodeUbxMsg) {
		for (uint16_t i = 0; i < len; i++) {
			//		// logger.trace("Buffer length [%d]\n", len);
			const int16_t code = _ubxDecoder.inputData(buffer[i]);
			if (code > 0 && code == static_cast<int16_t>(ClassIds::NAV) && _ubxDecoder.getLength() > 0) {
				//	// NavPOSLLHMessage navMsg { _ubxDecoder.getBuffer(),  _ubxDecoder.getLength()};

				const uint8_t *buffer = _ubxDecoder.getBuffer();
				const uint16_t len = _ubxDecoder.getLength();
				if (buffer[3] == static_cast<uint8_t>(NavMessageIds::POSLLH)) {
					logger.debug("Has NAV POSLLH msg\n");
					ws.binaryAll((const char *)_ubxDecoder.getBuffer(), _ubxDecoder.getLength());
				}
			}
		}
		delay(1);
	}

	// ws.binaryAll((const char *)buffer, len);
}