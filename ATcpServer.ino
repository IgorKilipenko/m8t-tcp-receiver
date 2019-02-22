ATcpServer::ATcpServer() : clients{MAX_TCP_CLIENTS, nullptr} {}
ATcpServer::~ATcpServer() { end(); }

void ATcpServer::ATcpServer::process() {
	if (!receiveData) {
		return;
	}

#ifdef DEBUG
	size_t bytesCount = 2;
#else
	size_t bytesCount = Serial.available();
#endif
	if (bytesCount > 0) {
#ifdef DEBUG
		char buffer[] = "T";
#else
		char buffer[bytesCount];
		bytesCount = Serial.readBytes(buffer, bytesCount);
#endif

		if (store->isInitialize()) {
			store->writeToSD(buffer, bytesCount);
		}

		if (WiFi.status() == WL_CONNECTED) {
			sendDataToClients(buffer, bytesCount);
		}

#ifdef DEBUG
		delay(1000);
#else
		delay(1);
#endif
	}
}

void ATcpServer::handleError(AsyncClient *client, int8_t error) { logger.printf("\n Connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str()); }

void ATcpServer::handleData(AsyncClient *client, void *data, size_t len) {
	logger.printf("\n Data received from client %s\n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t *)data, len);
}

void ATcpServer::handleDisconnect(AsyncClient *client) {
	logger.printf("\n Client %s disconnected \n", client->remoteIP().toString().c_str());
	// delete client;
	// client = nullptr;
	const size_t availableClients = freeClients();
	logger.printf("Available clints count: %i \n", availableClients);
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
}

bool ATcpServer::isInProgress() { return receiveData; }

void ATcpServer::stopReceive() {
	if (store) {
		store->end();
	}
	if (server) {
		server->end();
	}
	receiveData = false;
}

void ATcpServer::startReceive() {
	if (store->initSdCard()) {
		store->createFile();
	}

	server->begin();
	server->setNoDelay(true);
	receiveData = true;
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

size_t ATcpServer::sendMessage(AsyncClient *client, const char msg[], size_t len) {
	/* Not work, see AsyncPrinter.cpp */
	//size_t toWrite = 0;
	//size_t toSend = len;
	//while (client->space() < toSend) {
	//	toWrite = client->space();
	//	client->add(msg, toWrite);
	//	while (!client->canSend())
	//		delay(0);
	//	client->send();
	//	toSend -= toWrite;
	//}
	//client->add(msg + (len - toSend), toSend);
	//while (!client->canSend())
	//	delay(0);
	//client->send();
	//return len;

	if (client->space() > len /* && client->canSend()*/) {
		// logger.debug("Data has been send to client, Ip: %s\n", client->remoteIP().toString().c_str());
		const size_t will_send = client->add(msg, len);
		if (client->send()) {
			return will_send;
		}
	}
	return 0;
}

size_t ATcpServer::sendMessage(AsyncClient *client, String str) {
	const char *msg = str.c_str();
	return sendMessage(client, msg, strlen(msg));
}

void ATcpServer::setup() {
	store = new SDStore();
	server = new AsyncServer(TCP_PORT);
	server->onClient([](void *s, AsyncClient *c) { ((ATcpServer *)(s))->handleNewClient(c); }, this);
}

void ATcpServer::sendDataToClients(char buffer[], size_t bytesCount) {
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
