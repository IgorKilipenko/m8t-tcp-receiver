
TelnetServer::TelnetServer() {}
TelnetServer::~TelnetServer() { stopReceive(); }

void TelnetServer::process() {

	if (!receiveData) {
		return;
	}

	if (WiFi.status() == WL_CONNECTED) {
		setFreeClientSpot();
	}

#ifdef DEBUG
	size_t bytesCount = 2;
#else
	size_t bytesCount = Serial.available();
#endif
	if (bytesCount > 0) {
#ifdef DEBUG
		byte buffer[] = "T";
#else
		byte buffer[bytesCount];
		Serial.readBytes(buffer, bytesCount);
#endif

		if (sdFile) {
			writeToSD(buffer, bytesCount);
		}

		if (WiFi.status() == WL_CONNECTED) {
			sendToClient(buffer, bytesCount);
		}

#ifdef DEBUG
		delay(1000);
#else
		delay(1);
#endif
	}
}

/** Find free/disconnected spot */
bool TelnetServer::setFreeClientSpot() {

	bool clientConnected = false;
	for (int i = 0; i < MAX_CLIENTS; i++) {
		WiFiClient &client = clients[i];
		if (client && client.connected()) {
			while (client.available()) {
				Serial.write(client.read());
				logger.debug("Data has been sent to Serial\n");
			}
		} else if (!client || !client.connected()) {
			if (client) { // Нет такого события
				client.stop();
				logger.debug("Close connection to client\n");
			}
			if (server.hasClient()) {
				client = server.available();
				clientConnected = true;
				logger.debug("Client connected, ip address: %s\n\t-> Connected clients count: %i\n", client.remoteIP().toString().c_str(), i + 1);
				break;
			}
		}
		clientConnected = false;
	}

	if (server.hasClient() && !clientConnected) {
		server.available().stop();
		logger.debug("MAX_CONNECTION, connection closed.\n");
	}

	return clientConnected;
}

void TelnetServer::sendToClient(byte buffer[], size_t bytesCount) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		WiFiClient &client = clients[i];
		if (&client && client && client.connected()) {
			client.write(buffer, bytesCount);
			logger.debug("Data has been sent to client\n");

			delay(1);
		}
	}
}

/** Save GPS data to SD card */
void TelnetServer::writeToSD(byte buffer[], size_t bytesCount) {
#ifdef DEBUG
	logger.debug("Write data: (%s) to SD", buffer);
#else
	sdFile.write(buffer, sizeof(buffer));
#endif
	sdFile.flush();

	delay(1);
}

void TelnetServer::getDataFromClients() {
	if (WiFi.status() == WL_CONNECTED) {
		for (int i = 0; i < MAX_CLIENTS; i++) {
			WiFiClient &client = clients[i];
			if (&client && client && client.connected()) {
				while (client.available()) {
					Serial.write(client.read());
				}

				logger.debug("Data has been sent to Serial\n");
				delay(1);
			}
		}
	}
}

void TelnetServer::generateFileName(char *name) {
	const size_t max_nums = 999;
	bool reset = false;
	const char prefix[] = "raw_";
	const char ext[] = ".ubx";
	size_t number = 1;
	while (number != 0) {
		if (number > max_nums) {
			number = 1;
			reset = true;
		}
		sprintf(name, "%s%i%s", prefix, number, ext);
		if (SD.exists(name) && !reset) {
			// file exist
			number++;
		} else {
			// next number
			return;
		}
	}
}

void TelnetServer::initSdCard() {
	if (!SD.begin(CS_PIN)) {
		// initialization failed!
		logger.debug("initialization failed!.\n");
		sdCard = false;
	} else {
		logger.debug("initialization done.\n");
		sdCard = true;
	}
}

void TelnetServer::createFile() {
	generateFileName(filename);
	if (SD.exists(filename)) {
		SD.remove(filename);
		logger.debug("File: %s removed\n", filename);
	}
	sdFile = SD.open(filename, FILE_WRITE);
	logger.debug("File: %s created\n", filename);
}

void TelnetServer::stopReceive() {
	if (sdCard && sdFile) {
		sdFile.close();
	}
	server.stop(); 
	receiveData = false;
}

void TelnetServer::startReceive() {
	initSdCard();
	if (sdCard) {
		createFile();
	}
	server.begin();
	server.setNoDelay(true);
	receiveData = true;
}