/** WebSocket events */
void AWebServer::wsEventHnadler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
	if (type == WS_EVT_CONNECT) {
		logger.printf("ws[%s][%u] connect\n", server->url(), client->id());
		client->printf("WS started, client id: %u :)", client->id());
		client->ping();
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
		request->send(200);
		return;
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

	request->send(404);
}

ApiResultPtr AWebServer::wifiQueryHandler(const char *event, const JsonObject &json, JsonObject &outJson) {
	logger.trace("Start on wifi QUERY\n");
	logger.trace(":");
	json.prettyPrintTo(logger);
	logger.print("\n");

	const char *cmd = json.get<const char *>("cmd");
	if (utils::streq(cmd, "scan")) {
		logger.debug("Start scan WIFI\n");
		int8_t n = scanWiFi();
		JsonArray &arrayJson = outJson.createNestedArray("value");
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
	}
	ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());
	return res_ptr;
}

ApiResultPtr AWebServer::wifiActionHandler(const char *event, const JsonObject &json, JsonObject &outJson) {
	logger.debug("Start on wifi ACTION\n");
	json.prettyPrintTo(logger);
	logger.debug("\n");

	const char *cmd = json.get<const char *>("cmd");
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
		JsonObject &resJson = outJson.createNestedObject("value");

		resJson["status"] = WiFi.status();
		String ip = utils::toStringIp(WiFi.localIP());
		resJson["new_sta_ip"] = ip;
		resJson["status"] = WiFi.status();

		ApiResultPtr res_ptr = std::shared_ptr<ApiResult>(new ApiResult());

		res_ptr->addAction([&](AsyncWebServerRequest *request) {
			logger.debug("Start Then Action\n");
			disconnectStaWifi();
			delay(2000);
			uint8_t connRes = WiFi.begin(ssid, password);
			// if (!connectStaWifi(ssid, password)) {
			//	logger.debug("WiFi STA not connected\n");
			//	return 1;
			//}
			delay(500);
			if (connRes == WL_CONNECTED) {
				logger.debug("Save WiFi Credentials\n");
				saveWiFiCredentials();
			}

			logger.debug("WiFi STA connected\n");
			return 0;
		});

		return res_ptr;
	}

	return nullptr;
}

void AWebServer::addServerHandlers() {
	ws.onEvent(std::bind(&AWebServer::wsEventHnadler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

	server.addHandler(&ws);

	events.onConnect([&](AsyncEventSourceClient *client) { client->send("hello!", NULL, millis(), 1000); });

	server.addHandler(&events);

	server.addHandler(new SPIFFSEditor(http_username, http_password));

	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", String(ESP.getFreeHeap())); });

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

		while (res_ptr->actionCount()) {
			const uint8_t err_code = res_ptr->then(request);
			if (err_code == 0) {
				logger.debug("Then Action success\n");
			} else {
				logger.debug("Then action complite with err code: {%i}");
			}
		}

		// delete response;
	});

	// AsyncCallbackJsonWebHandler *wifiStaConnectHandler = new AsyncCallbackJsonWebHandler("/api/service", [&](AsyncWebServerRequest *request, JsonVariant &jsonReq) {
	//              logger.debug("/api/srvice \n");
	//              if (!jsonReq) {
	//              	logger.debug("apiHandler -> Json is empty\n");
	//              	request->send(404);
	//              	return;
	//              }
	//              const JsonObject &jsonObj = jsonReq.as<const JsonObject>();
	//              if (!jsonObj.success()) {
	//              	logger.debug("apiHandler -> Json parsing failed\n");
	//              	request->send(404);
	//              	return;
	//              }

	//              AsyncJsonResponse *response = new AsyncJsonResponse(true);
	//              JsonArray &jsonResp = response->getRoot();

	//              if (jsonObj.containsKey("cmd")) {
	//              	logger.debug("Contain cmd key, cmd: %s\n", jsonObj.get<const char *>("cmd"));
	//              	const char *cmd = jsonObj.get<const char *>("cmd");
	//              	if (utils::streq(cmd, "connect")) {
	//              		if (!jsonObj.containsKey("ssid")) {
	//              			logger.debug("SSID failed\n");
	//              			return;
	//              		}
	//              		if (!jsonObj.containsKey("password")) {
	//              			logger.debug("PASSWORD failed\n");
	//              			return;
	//              		}
	//              		const char *new_ssid = jsonObj["ssid"];
	//              		strcpy(this->ssid, new_ssid);
	//              		const char *new_password = jsonObj["password"];
	//              		strcpy(this->password, new_password);

	//              		logger.debug("onWiFiActionRequest -> createNestedObject\n ");
	//              		JsonObject &resJson = jsonResp.createNestedObject();

	//              		//saveWiFiCredentials();
	//              		server.reset();
	//              		if (!this->connectStaWifi(ssid, password)) {
	//              			logger.debug("WiFi STA not connected\n");
	//              			return;
	//              		}

	//

	//              		//delay(1000);
	//              		//ESP.reset();
	//              		//resJson["status"] = WiFi.status();
	//              		//String ip = utils::toStringIp(WiFi.localIP());
	//              		//resJson["new_sta_ip"] = ip;
	//              		//resJson["status"] = WiFi.status();
	//              	}
	//              }

	//              response->setLength();
	//              request->send(response);
	//              logger.debug("apiHandler -> Json API response send success\n");
	//              // delete response;
	//});

	// server.addHandler(wifiStaConnectHandler);

	api.on(SGraphQL::WIFI, SGraphQL::QUERY, std::bind(&AWebServer::wifiQueryHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	api.on(SGraphQL::WIFI, SGraphQL::ACTION, std::bind(&AWebServer::wifiActionHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	server.addHandler(apiHandler);

#endif // REST_API

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
}


void AWebServer::addOTAhandlers(){
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
}