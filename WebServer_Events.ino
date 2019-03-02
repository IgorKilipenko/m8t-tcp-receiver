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

void AWebServer::wifiQueryHandler(const char *event, const JsonObject &json, JsonArray &outJson) {
	logger.debug("Start on wifi QUERY\n");
	json.prettyPrintTo(logger);
	logger.debug("\n");
	if (json.containsKey("cmd")) {
		logger.debug("Contain cmd key, cmd: %s\n", json.get<const char *>("cmd"));
		const char *cmd = json.get<const char *>("cmd");
		if (utils::streq(cmd, "scan")) {
			logger.debug("Start scan WIFI\n");
			int8_t n = scanWiFi();
			logger.debug("Scan end, item count: %i\n", n);
			if (n > 0) {
				uint8_t i = 0;
				for (auto const &item : wifiList) {
					logger.debug("n:%i,", i++);
					JsonObject &resJson = outJson.createNestedObject();
					resJson["rssi"] = item->rssi;
					resJson["ssid"] = item->ssid;
					resJson["bssid"] = item->bssid;
					resJson["channel"] = item->channel;
					resJson["secure"] = item->secure;
					resJson["hidden"] = item->hidden;
				}
			}
			if (wifiList.size() > 0) {
				wifiList.clear();
			}
		}
	}
}

void AWebServer::wifiActionHandler(const char *event, const JsonObject &json, JsonArray &outJson) {
	logger.debug("Start on wifi ACTION\n");
	json.prettyPrintTo(logger);
	logger.debug("\n");
	if (json.containsKey("cmd")) {
		logger.debug("Contain cmd key, cmd: %s\n", json.get<const char *>("cmd"));
		const char *cmd = json.get<const char *>("cmd");
		if (utils::streq(cmd, "connect")) {
			if (!json.containsKey("ssid")) {
				logger.debug("SSID failed\n");
				return;
			}
			if (!json.containsKey("password")) {
				logger.debug("PASSWORD failed\n");
				return;
			}
			const char *new_ssid = json["ssid"];
			strcpy(this->ssid, new_ssid);
			const char *new_password = json["password"];
			strcpy(this->password, new_password);

			logger.debug("onWiFiActionRequest -> createNestedObject\n ");
			JsonObject &resJson = outJson.createNestedObject();

			if (!this->connectStaWifi(ssid, password)) {
				logger.debug("WiFi STA not connected\n");
				return;
			}
			delay(500);
			resJson["status"] = WiFi.status();
			String ip = utils::toStringIp(WiFi.localIP());
			resJson["new_sta_ip"] = ip;
			resJson["status"] = WiFi.status();
		}
	}
}