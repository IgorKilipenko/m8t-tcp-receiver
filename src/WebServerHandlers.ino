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
	log_v("NOT_FOUND: , [CORE ------> [%i]]\n", xPortGetCoreID());
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
	log_v(" http://%s%s\n", request->host().c_str(), request->url().c_str());

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


ApiResultPtr AWebServer::receiverActionHandler(const char *event, const JsonObject &json, JsonObject &outJson) {
	log_d("Start on receiver ACTION, [CORE ------> [%i]]\n", xPortGetCoreID());
	// json.prettyPrintTo(logger);
	// log_d("\n");

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
				log_v("SET -> Start receive\n");
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
	// reqJson.prettyPrintTo(logger);
	// log_d("\n");

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
	// reqJson.prettyPrintTo(logger);
	// log_d("\n");

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
	// json.prettyPrintTo(logger);
	// log_d("\n");

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
	// reqJson.prettyPrintTo(logger);
	// log_d("\n");

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
	log_v("ADD SERVER HANDLERS");
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
		log_v("onRequestBody");
		if (!index)
			log_v("BodyStart: %u\n", total);
		log_v("%s", (const char *)data);
		if (index + len == total)
			log_v("BodyEnd: %u\n", total);
	});

	// REST API handler
	AsyncCallbackJsonWebHandler *apiHandler = new AsyncCallbackJsonWebHandler("/api", [&](AsyncWebServerRequest *request, JsonVariant &json) {
		log_v("API \n");
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
			log_e("OTA Auth Failed");
		else if (error == OTA_BEGIN_ERROR)
			// events.send("Begin Failed", "ota");
			log_e("OTA Begin Failed");
		else if (error == OTA_CONNECT_ERROR)
			// events.send("Connect Failed", "ota");
			log_e("OTA Connect Failed");
		else if (error == OTA_RECEIVE_ERROR)
			// events.send("Recieve Failed", "ota");
			log_e("OTA Recieve Failed");
		else if (error == OTA_END_ERROR)
			// events.send("End Failed", "ota");
			log_e("OTA End Failed");
	});
}

void AWebServer::addReceiverHandlers() { telnetServer->onSerialData(std::bind(&AWebServer::receiverDataHandler, this, std::placeholders::_1, std::placeholders::_2)); }

void AWebServer::receiverDataHandler(const uint8_t *src_buffer, size_t len) {
	if (!isCanSendData()) {
		log_d("Is't can sand data\n");
		delay(1);
		return;
	}
	// if (!ws.count()) {
	//	log_d("Ws not connected\n");
	//	return;
	//}

	uint8_t buffer[len];
	memcpy(buffer, src_buffer, len);

	if (_decodeUbxMsg) {

		unsigned long start = millis();
		Serial.println("=========== START DECODE ============");

		for (uint16_t i = 0; i < len; i++) {
			const int8_t code = _ubxDecoder.inputData(buffer[i]);
			const bool isClsId = (code > 0 && _ubxDecoder.getLength() > 0);

			if (!isClsId) {
				continue;
			}

			const uint8_t *ubxPacket = _ubxDecoder.getBuffer();
			const uint16_t packetLen = _ubxDecoder.getLength();
			bool hasMsg = false;

			log_d("ClassId: [%X], MsgId: [%X]\n", ubxPacket[2], ubxPacket[3]);

			const uint8_t clsId = ubxPacket[2];

			switch (clsId) {
			case static_cast<uint8_t>(ClassIds::NAV):

				switch (ubxPacket[3]) {
				case static_cast<uint8_t>(NavMessageIds::PVT):
					log_v("Has PVT msg\n");
					hasMsg = true;
					break;
				case static_cast<uint8_t>(NavMessageIds::POSLLH):
					log_v("Has NAV POSLLH msg\n");
					hasMsg = true;
					break;
				case static_cast<uint8_t>(NavMessageIds::HPPOSLLH):
					log_v("Has NAV HPPOSLLH msg\n");
					hasMsg = true;
					break;
				}
				break;

			case static_cast<uint8_t>(ClassIds::CFG):
				switch (ubxPacket[3]) {
				case static_cast<uint8_t>(CfgMessageIds::RATE):
					log_v("Has CFG RATE msg\n");
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
				log_d("Send to ws [%ld] bytes", len);
				ws.binaryAll((const char *)ubxPacket, packetLen);
			}
		}

		delay(1);

		Serial.printf("=========== END DECODE ============   TIME  [%ld ms]  =========\n", millis() - start);

	} else {
		if (ws.count() <= 0) {
			//log_d("Ws not connected\n");
			return;
		}

		log_v("WS count: %d", ws.count());

		unsigned long start = millis();
		//Serial.println("=========== START SEND WS NOT DECODE DATA ============");

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

		//Serial.printf("=========== END SEND WS NOT DECODE DATA ==== TIME  [%ld ms]  =========\n", millis() - start);
	}
}