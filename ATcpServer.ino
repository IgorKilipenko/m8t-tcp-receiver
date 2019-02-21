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
		uint8_t buffer[] = "T";
#else
		uint8_t buffer[bytesCount];
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

void ATcpServer::handleError(AsyncClient *client, int8_t error) {
	logger.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

void ATcpServer::handleData(AsyncClient *client, void *data, size_t len) {
	logger.printf("\n data received from client %s \n", client->remoteIP().toString().c_str());
	Serial.write((uint8_t *)data, len);
}

void ATcpServer::handleDisconnect(AsyncClient *client) {
	logger.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
    delete client;
	freeClients();
}

void ATcpServer::handleTimeOut(AsyncClient *client, uint32_t time) { logger.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str()); }

void ATcpServer::handleNewClient(AsyncClient *client) {
	Serial.printf("\n new client has been connected to server, ip: %s\n", client->remoteIP().toString().c_str());

	// add to list
	int i = 0;
	for (; i < MAX_TCP_CLIENTS; i++) {
		if (!clients[i] || clients[i]->free()) {
			clients[i] = client;
			break;
		}
	}

	if (i == MAX_TCP_CLIENTS) {
		// reply to client
		const char reply[] = "403 Forbidden";
		sendMessage(client, reply, strlen(reply));
		logger.debug("server is busy with %d active connections\n", MAX_TCP_CLIENTS);
	}

	// register events
	client->onData([](void *r, AsyncClient* c, void *buf, size_t len){ ((ATcpServer*)(r))->handleData(c, buf, len); }, this);
	client->onError([](void *r, AsyncClient* c, int8_t error){ ((ATcpServer*)(r))->handleError(c, error); }, this);
	client->onDisconnect([](void *r, AsyncClient* c){ ((ATcpServer*)(r))->handleDisconnect(c); }, this);
	client->onTimeout([](void *r, AsyncClient* c, uint32_t time){ ((ATcpServer*)(r))->handleTimeOut(c, time); }, this);
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

	server->setNoDelay(true);
	server->begin();
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

void ATcpServer::freeClients() {
	for (int i=0; i < MAX_TCP_CLIENTS; i++) {
		if (clients[i] && clients[i]->free()) {
			delete clients[i];
            clients[i] = nullptr;
		}
	}
}

void ATcpServer::sendMessage(AsyncClient *client, const char msg[], size_t len) {
	if (client->space() > len && client->canSend()) {
		char reply[32];
		logger.debug("Data has been send to client, Ip: %s", client->remoteIP().toString().c_str());
		client->add(reply, len);
		client->send();
	}
}

void ATcpServer::sendMessage(AsyncClient *client, String str) {
	const char * msg = str.c_str();
	sendMessage(client, msg, strlen(msg));
}

void ATcpServer::setup() {
    store = new SDStore();
	server = new AsyncServer(TCP_PORT);
	server->onClient([](void *s, AsyncClient* c){
       ((ATcpServer*)(s))->handleNewClient(c);
    }, this);
}

void ATcpServer::sendDataToClients(uint8_t buffer[], size_t bytesCount) {
	for (int i = 0; i < MAX_TCP_CLIENTS; i++) {
		if (clients[i] && clients[i]->connected()) {
			sendMessage(clients[i], (char*)buffer, bytesCount);
		}
	}
}