// const size_t BUFFER_LENGTH = 1024;
// const char *NtripClientSync::HTTP_OK = "HTTP/1.1 200 OK";

NtripClientSync::NtripClientSync(HardwareSerial *uart) : _uart{uart}, _client{new WiFiClient} {}

NtripClientSync::~NtripClientSync() {
	logger.trace(":: destructor  :: NtripClientSync\n");
	if (_client) {
		_client->stop();
		delete _client;
	}
}

bool NtripClientSync::connect(const char *host, uint16_t port, const char *user, const char *pass, const char *mntpnt, const char *nmea) {

	if (isEnabled()) {
		logger.debug("Client already connected to Ntrip\n");
		return false;
	}

	bool connected = false;
	char connStr[sizeof(_connectionString)]{0};

	if (_client->connect(host, port)) {

		const size_t cstrlen = buildConnStr(connStr, host, port, user, pass, mntpnt, nmea);
		if (cstrlen == 0) {
			logger.error("Connection string is empty");
		} else {
			size_t count = _client->write((const uint8_t *)connStr, cstrlen);
			logger.debug("Ntrip connection string write: [%i] bytes\n", count);
			logger.debug("Ntrip connection string: %s\n", connStr);
			uint64_t start = millis();
			while (!_client->available() && millis() - start < 1000) {
				delay(10);
			}
			connected = requestNtrip();
		}
	}

	if (connected) {
		strcpy(_host, host);
		strcpy(_user, user);
		strcpy(_passwd, pass);
		strcpy(_mntpnt, mntpnt);
		strcpy(_connectionString, connStr);
		_port = port;
	} else if (_client->connected()) {
		_client->stop();
	}

	_connectedNtrip = connected;
	return connected;
}

void NtripClientSync::stop() {
	if (_client && _client->connected()) {
		_client->stop();
	}
}

bool NtripClientSync::requestNtrip() {

	// const char *httpok = "ICY 200 OK\r\n" /*"HTTP/1.1 200 OK"*/;

	uint8_t buffer[NTRIP_BUFFER_LENGTH]{0};
	size_t len = readLine(buffer, 128);

	if (len >= strlen(NTRIP_RSP_OK_CLI) && memcmp((uint8_t *)NTRIP_RSP_OK_CLI, buffer, strlen(NTRIP_RSP_OK_CLI)) == 0) {
		logger.debug("Connection to ntrip success. Response: %s\n", buffer);
	} else if (len >= strlen(NTRIP_HTTP_OK) && memcmp((uint8_t *)NTRIP_HTTP_OK, buffer, strlen(NTRIP_HTTP_OK)) == 0) {
		// FIX Next -> will by ICY 200 OK\r\n in response body FOR FIX
		logger.debug("Connection to ntrip success. Response: %s\n", buffer);
	} else {
		logger.debug("\nError response from ntrip server. Response: %s\n", buffer);
		_connectedNtrip = false;
		return false;
	}

	read(buffer + len, sizeof(buffer) - len);
	logger.debug("\nError response from ntrip server. Response: %s\n", buffer);

	_connectedNtrip = true;
	return _connectedNtrip;
}

size_t NtripClientSync::receiveNtrip() {
	if (!isEnabled()) {
		return 0;
	}
	size_t count = read(_buffer, NTRIP_BUFFER_LENGTH);
	if (count) {
		logger.debug("Count %i\n", count);
		// count = Receiver->write(_buffer, count);
		size_t i = 0;
		logger.debug("\n=======================\n");
		for (; i < count; i++) {
			_uart->write(_buffer[i]);
#ifdef DEBUG
			logger.write(_buffer[i]);
#endif
		}
		logger.debug("\n=======================\n");
	}

	return count;
}

size_t NtripClientSync::read(uint8_t *buffer, size_t len) {
	if (!_client->connected()) {
		logger.debug("Read err ntrip\n");
		return 0;
	}
	size_t count = 0;
	size_t byteCount = _client->available();
	if (byteCount > 0) {
		len = std::min(byteCount, len);
		count = _client->read(buffer, len);
		// utils::ethernetDechunk((char*)buffer);
		logger.debug("Count %i\n", count);
	}
	return count;
}

size_t NtripClientSync::readLine(uint8_t *buffer, size_t len) {
	if (!_client->connected()) {
		logger.debug("Read err ntrip\n");
		return 0;
	}
	return _client->readBytesUntil('\r', buffer, len);
}

bool NtripClientSync::isEnabled() {
	// logger.debug("Ntrip enabled = %s\n", (_client && _client->connected() && _connectedNtrip) ? "TRUE" : "FALSE");
	_connectedNtrip = _client && _client->connected() && _connectedNtrip;
	return _connectedNtrip;
}

size_t NtripClientSync::buildConnStr(char *connStr, const char *host, uint16_t port, const char *user, const char *pass, const char *mntpnt, const char *nmea) {

	assert(host && port && user && pass);

	char buff[1024]{0}, sec[512], *p = buff;
	p += sprintf(p, "GET /%s HTTP/1.0\r\n", mntpnt);
	p += sprintf(p, "HOST: %s\r\n", host);
	p += sprintf(p, "Ntrip-Version: Ntrip/1.0\r\n");
	p += sprintf(p, "User-Agent: NTRIP %s/1.0\r\n", NTRIP_AGENT);
	if (nmea) {
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

	std::copy(buff, p, connStr);
	logger.debug("Build connection string [%s]\n", connStr);
	return p - buff;
}
