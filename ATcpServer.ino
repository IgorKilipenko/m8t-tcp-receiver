ATcpServer::ATcpServer() : clients{MAX_TCP_CLIENTS, nullptr} {}
ATcpServer::~ATcpServer() { end(); }

void ATcpServer::ATcpServer::process() {
	if (!receiveData) {
		return;
	}

#if defined(DEBUG) && defined(MOCK_RECEIVER_DATA)
	size_t bytesCount = 2;
#else
	size_t bytesCount = Receiver->available();
#endif
	if (bytesCount > 0) {
#if defined(DEBUG) && defined(MOCK_RECEIVER_DATA)
		char buffer[] = "T";
#else
		char buffer[bytesCount];
		bytesCount = Receiver->readBytes(buffer, bytesCount);
#endif

		if (_writeToSd && store && store->isInitialize() && store->isOpenFile()) {
			store->writeToSD(buffer, bytesCount);
			delay(1);
		}

		if (_sendToTcp && WiFi.status() == WL_CONNECTED) {
			sendDataToClients(buffer, bytesCount);
		}

		if (_seralDataCallback != nullptr) {
			_seralDataCallback((const uint8_t *)buffer, bytesCount);
		}

//#if defined(DEBUG) && defined(MOCK_RECEIVER_DATA)
//		delay(1000);
//#else
//		delay(1);
//#endif
	}
}

void ATcpServer::handleError(AsyncClient *client, int8_t error) { logger.printf("\n Connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str()); }

void ATcpServer::handleData(AsyncClient *client, void *data, size_t len) {
	logger.debug("\n Data from client to receiver %s: \n", client->remoteIP().toString().c_str());
	#ifdef DEBUG
	logger.printf("=========================\n");
	logger.write((uint8_t *)data, len);
	logger.printf("\n-- packet count : [%i] bytes--\n", len);
	logger.printf("=========================\n");
	#endif
	Receiver->write((uint8_t *)data, len);
}

void ATcpServer::handleDisconnect(AsyncClient *client) {
	logger.debug("\n Client %s disconnected \n", client->remoteIP().toString().c_str());
	// delete client;
	// client = nullptr;
	const size_t availableClients = freeClients();
	logger.debug("Available clints count: %i \n", availableClients);
}

void ATcpServer::handleTimeOut(AsyncClient *client, uint32_t time) { logger.printf("\n Client ACK timeout ip: %s \n", client->remoteIP().toString().c_str()); }

void ATcpServer::handleNewClient(AsyncClient *client) {
	logger.printf("\n New client connecting to server, ip: %s\n", client->remoteIP().toString().c_str());

	// add to list
	int i = 0;
	for (; i < MAX_TCP_CLIENTS; i++) {
		if (!clients[i] || clients[i]->free()) {
			clients[i] = client;
			logger.printf("Client connected, ip address: %s\n\t-> Connected clients count: %i \n", client->remoteIP().toString().c_str(), availableClientsCount());
			break;
		}
	}

	if (i == MAX_TCP_CLIENTS) {
		// reply to client
		const char reply[] = "403 Forbidden";
		sendMessage(client, reply, strlen(reply));
		logger.printf("Client not connected, server is busy with %d active connections\n", MAX_TCP_CLIENTS);
		client->close(true);
	}

	// register events
	client->onData([](void *r, AsyncClient *c, void *buf, size_t len) { ((ATcpServer *)(r))->handleData(c, buf, len); }, this);
	client->onError([](void *r, AsyncClient *c, int8_t error) { ((ATcpServer *)(r))->handleError(c, error); }, this);
	client->onDisconnect([](void *r, AsyncClient *c) { ((ATcpServer *)(r))->handleDisconnect(c); }, this);
	client->onTimeout([](void *r, AsyncClient *c, uint32_t time) { ((ATcpServer *)(r))->handleTimeOut(c, time); }, this);

	serviceServer->onClient(
		[](void *s, AsyncClient *client) {
			const char msg[] = "Service server\n";
			if (client) {
				client->write(msg);
			}
		},
		this);
}

bool ATcpServer::isInProgress() const { return receiveData; }
bool ATcpServer::isSdInitialize() const { return store != nullptr && store->isInitialize(); }

void ATcpServer::stopReceive() {
	if (store) {
		store->end();
	}
	if (server) {
		server->end();
	}
	receiveData = false;
	_timeEnd = millis();
}

void ATcpServer::startReceive(bool writeToSd, bool sendToTcp) {
	logger.trace("Receiver starting...\n");
	logger.trace("Write to SD card : [%s]\n", writeToSd ? "ENABLED" : "DISABLED");
	logger.trace("Send to TCP : [%s]\n", sendToTcp ? "ENABLED" : "DISABLED");

	_writeToSd = writeToSd;
	_sendToTcp = sendToTcp;

	assert(store != nullptr);
	if (_writeToSd && store && (store->isInitialize() || store->initSdCard())) {
		logger.trace("Stored initialized\n");
		store->createFile();
		logger.trace("File created\n");
	}

	// server->setNoDelay(true);
	if (_sendToTcp) {
		server->begin();
		logger.trace("TCP Server started\n");
	}

	// serviceServer->begin();

	receiveData = true;
	_timeEnd = 0;
	_timeStart = millis();

	logger.trace("Receive data started\n");
}

unsigned long ATcpServer::getTimeReceive() const {
	if (!isInProgress()) {
		return 0;
	}
	return millis() - _timeStart;
}

unsigned long ATcpServer::getTimeStart() const {
	if (!isInProgress()) {
		return 0;
	}
	return _timeStart;
}

void ATcpServer::end() {
	if (store) {
		delete store;
	}
	store = nullptr;

	freeClients();
	clients.clear();

	if (server) {
		delete server;
	}
	server = nullptr;
}

size_t ATcpServer::freeClients() {
	size_t clientCount = 0;
	for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
		if (!clients[i] || clients[i]->free()) {
			if (clients[i]) {
				delete clients[i];
			}
			clients[i] = nullptr;
		} else {
			clientCount++;
		}
	}
	return clientCount;
}

size_t ATcpServer::sendMessage(AsyncClient *client, const char *msg, size_t len) {
	const size_t will_send = client->add(msg, len);
	if (!will_send || !client->send()) {
		return 0;
	}
	return will_send;
}

size_t ATcpServer::sendMessage(AsyncClient *client, String str) {
	const char *msg = str.c_str();
	return sendMessage(client, msg, strlen(msg));
}

void ATcpServer::setup() {
	store = new SDStore();
	if (_writeToSd) {
		store->initSdCard();
	}
	server = new AsyncServer(TCP_PORT);
	serviceServer = new AsyncServer(TCP_PORT + 1);
	server->onClient([](void *s, AsyncClient *c) { ((ATcpServer *)(s))->handleNewClient(c); }, this);
}

void ATcpServer::sendDataToClients(const char *buffer, size_t bytesCount) {
	logger.trace("Send data to clients\n");
	for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
		if (clients[i] && clients[i]->connected()) {
			sendMessage(clients[i], buffer, bytesCount);
		}
	}
}

size_t ATcpServer::availableClientsCount() {
	size_t clientCount = 0;
	for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
		if (clients[i]) {
			clientCount++;
		}
	}
	return clientCount;
}
