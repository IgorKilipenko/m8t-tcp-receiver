
TelnetServer::TelnetServer() {}
TelnetServer::~TelnetServer() { stopReceive(); }

void TelnetServer::process() {

	if (!receiveData) {
		return;
	}

	if (WiFi.status() == WL_CONNECTED) {
		handleClients();
	}

#ifdef DEBUG
	size_t bytesCount = 2;
#else
	size_t bytesCount = Serial.available();
#endif
	if (bytesCount > 0) {
#ifdef DEBUG
		uint8_t buffer[] = "T";
#else
		uint8_t buffer[bytesCount];
		bytesCount = Serial.readBytes(buffer, bytesCount);
#endif

		if (sdFile) {
			writeToSD(buffer, bytesCount);
		}

		if (WiFi.status() == WL_CONNECTED) {
			getDataFromClients();
			sendDataToClients(buffer, bytesCount);
		}

#ifdef DEBUG
		delay(1000);
#else
		delay(1);
#endif
	}
}

/** Find free/disconnected spot */
// bool TelnetServer::setFreeClientSpot() {
//
//	bool clientConnected = false;
//	for (int i = 0; i < MAX_CLIENTS; i++) {
//		WiFiClient &client = clients[i];
//		if (client && client.connected()) {
//			while (client.available()) {
//				Serial.write(client.read());
//				logger.debug("Data has been sent to Serial\n");
//			}
//		} else if (!client || !client.connected()) {
//			if (client) { // Нет такого события
//				client.stop();
//				logger.debug("Close connection to client\n");
//			}
//			if (server.hasClient()) {
//				client = server.available();
//				clientConnected = true;
//				logger.debug("Client connected, ip address: %s\n\t-> Connected clients count: %i\n", client.remoteIP().toString().c_str(), i + 1);
//				break;
//			}
//		}
//		clientConnected = false;
//	}
//
//	if (server.hasClient() && !clientConnected) {
//		server.available().stop();
//		logger.debug("MAX_CONNECTION, connection closed.\n");
//	}
//
//	return clientConnected;
//}

/** Find free/disconnected spot and receive data from clients*/
bool TelnetServer::handleClients() {

	bool clientConnected = false;
	if (server.hasClient()) {
		int i;
		for (i = 0; i < MAX_CLIENTS; i++) {
			if (!clients[i]) { // receive data from clients and send to uart (to GPS)
				clients[i] = server.available();
				clientConnected = true;
				logger.debug("Client connected, ip address: %s\n\t-> Connected clients count: %i\n", clients[i].remoteIP().toString().c_str(), i + 1);
				break;
			}
		}
		if (i == MAX_CLIENTS) {
			server.available().println("403 Forbidden");
			logger.debug("server is busy with %d active connections\n", MAX_CLIENTS);
		}
	}

	return clientConnected;
}

void TelnetServer::sendDataToClients(uint8_t buffer[], size_t bytesCount) {

	// push UART data to all connected telnet clients
	for (int i = 0; i < MAX_CLIENTS; i++){
		// if client.availableForWrite() was 0 (congested)
		// and increased since then,
		// ensure write space is sufficient:
		if (!clients[i]){
			continue;
		}
		if (size_t afw = clients[i].availableForWrite()) {
			if (bytesCount > afw) {
				logger.debug("warn available for write: %zd serial-read:%zd\n", afw, bytesCount);
			}
			afw = std::min(bytesCount, afw);
			size_t tcp_sent = clients[i].write(buffer, afw);
			if (tcp_sent != afw) {
				logger.debug("len mismatch: available:%zd serial-read:%zd tcp-write:%zd\n", afw, bytesCount, tcp_sent);
			}
#ifdef DEBUG
			String msg;
			msg.getBytes(buffer, afw);
			logger.debug(msg.c_str());

			logger.debug("data");
#endif
		}
	}
}

void TelnetServer::getDataFromClients() {
	if (WiFi.status() == WL_CONNECTED) {
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (clients[i]) {
				while (clients[i].available() && Serial.availableForWrite() > 0) {
					Serial.write(clients[i].read());
					logger.debug("Data has been sent to GPS\n");
				}
				delay(1);
			}
		}
	}
}

/** Save GPS data to SD card */
void TelnetServer::writeToSD(uint8_t buffer[], size_t bytesCount) {
	sdFile.write(buffer, bytesCount);
	sdFile.flush();

	delay(1);
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
		SD.end();
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