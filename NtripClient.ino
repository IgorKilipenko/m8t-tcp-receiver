

NtripClient::NtripClient(HardwareSerial *uart) : _uart{uart}, _client{new AsyncClient} {
	_client->setNoDelay(true);
	_client->onConnect(std::bind(&NtripClient::onConnect, this, std::placeholders::_1, std::placeholders::_2));
	_client->onData(std::bind(&NtripClient::handleData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}
NtripClient::~NtripClient() {
	logger.trace(":: destructor  :: NtripClient\n");
	if (_client && close()) {
		delete _client;
	}
}

bool NtripClient::connect(const char *host, uint16_t port, const char *user, const char *pass, const char *mntpnt, const char *nmea ) {

	assert(host && port && user && pass);

	if (_client->connected()) {
		logger.debug("Client already connected to Ntrip\n");
		return false;
	}

	char buff[1024], sec[512], *p = buff;
	p += sprintf(p, "GET /%s HTTP/1.0\r\n", mntpnt);
	p += sprintf(p, "HOST: %s\r\n", mntpnt);
	p += sprintf(p, "Ntrip-Version: Ntrip/2.0\r\n");
	p += sprintf(p, "User-Agent: NTRIP %s/2.0\r\n", NTRIP_AGENT);
	if (nmea){
		p += sprintf(p, "Ntrip-GGA: %s\r\n", nmea);
	}
	

	if (!user) {
		p += sprintf(p, "Accept: */*\r\n");
		p += sprintf(p, "Connection: close\r\n");
	} else {
		sprintf(sec, "%s:%s", user, pass);
		p += sprintf(p, "Authorization: Basic ");
		p += utils::encbase64(p, (unsigned char *)sec, strlen(sec));
		p += sprintf(p, "\r\n");
	}

	p += sprintf(p, "\r\n");

	std::copy(buff, p, _connStr);

	if (!_client->connected()) {
		_connect = _client->connect(host, port);
	}

	if (_connect) {
		strcpy(_host, host);
		strcpy(_user, user);
		strcpy(_passwd, pass);
		strcpy(_mntpnt, mntpnt);
		_port = port;
	}

	return _connect;
}

bool NtripClient::close() {
	if (!_client) {
		return false;
	} else if (!_client->freeable()) {
		_client->stop();
	}
	_client->close(true);
	return true;
}

bool NtripClient::stop() {
	if (_client && _client->connected()) {
		_client->stop();
		return true;
	} else {
		return false;
	}
}

void NtripClient::onConnect(void *arg, AsyncClient *client) {
	logger.debug("Ntrip Client connected to %s\n", client->remoteIP().toString().c_str());
	if (_connect && _client->canSend()) {
		client->add(_connStr, strlen(_connStr));
		client->send(); // Sent connection ntrip string
		logger.debug("Nterip clent send connection string, [%s]", _connStr);
	}
}

void NtripClient::handleData(void *arg, AsyncClient *client, void *data, size_t len) {
	uint8_t *buffer = (uint8_t *)data;
	if (_connect) {
		char respMsg[len + 1]{0};
		memcpy(respMsg, buffer, len);
		if (strcmp(NTRIP_RSP_OK_CLI, respMsg) != 0) {
			logger.debug("Error response from ntrip server. Response: %s\n", respMsg);
		} else {
			logger.debug("Connection to ntrip success. Response: %s\n", respMsg);
		}
		_connect = false;
	}
	char msg[len + 1]{0};
	memcpy(msg, buffer, len);
	logger.debug("-> NTRIP: %s\n", msg);
	// uart->write((uint8_t*)data, len);
}