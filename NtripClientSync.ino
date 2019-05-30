// const size_t BUFFER_LENGTH = 1024;

NtripClientSync::NtripClientSync(HardwareSerial *uart) : _uart{uart}, _client{new WiFiClient} {
	//_client->setNoDelay(true);
	//_client->onConnect(std::bind(&NtripClientSync::onConnect, this, std::placeholders::_1, std::placeholders::_2));
	//_client->onData(std::bind(&NtripClientSync::handleData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}
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

	//_client->write(_connStr, strlen(_connStr));

	bool connected = false;
	if (_client->connect(host, port)) {
		buildConnStr(_connStr, host, port, user, pass, mntpnt, nmea);
		logger.debug("Ntrip connection string: %s\n", _connStr);
		connected = requestNtrip();
	}

	if (connected) {
		strcpy(_host, host);
		strcpy(_user, user);
		strcpy(_passwd, pass);
		strcpy(_mntpnt, mntpnt);
		_port = port;
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
	const char * res ="HTTP/1.1 200 OK";
	const size_t len = read(_buffer, strlen(res));
	char respMsg[len + 1]{0};
	memcpy(respMsg, _buffer, len);
	if (strcmp(res, respMsg) != 0) {
		logger.debug("Error response from ntrip server. Response: %s\n", respMsg);
		_connectedNtrip = true;
		return false;
	} else {
		logger.debug("Connection to ntrip success. Response: %s\n", respMsg);
	}
	_connectedNtrip = true;
	return _connectedNtrip;
}

size_t NtripClientSync::receiveNtrip() {
	if (!isEnabled()) {
		return 0;
	}
	size_t count = read(_buffer, NTRIP_BUFFER_LENGTH);
	count = _uart->write(_buffer, count);
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
		logger.debug("Count %i\n", count);
	}
	return count;
}

bool NtripClientSync::isEnabled() {
	// logger.debug("Ntrip enabled = %s\n", (_client && _client->connected() && _connectedNtrip) ? "TRUE" : "FALSE");
	_connectedNtrip = _client && _client->connected() && _connectedNtrip;
	return _connectedNtrip;
}

void NtripClientSync::buildConnStr(char *connStr, const char *host, uint16_t port, const char *user, const char *pass, const char *mntpnt, const char *nmea) {

	assert(host && port && user && pass);

	char buff[1024], sec[512], *p = buff;
	p += sprintf(p, "GET /%s HTTP/1.0\r\n", mntpnt);
	p += sprintf(p, "HOST: %s\r\n", mntpnt);
	p += sprintf(p, "Ntrip-Version: Ntrip/2.0\r\n");
	p += sprintf(p, "User-Agent: NTRIP %s/2.0\r\n", NTRIP_AGENT);
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

	//_client->write(connStr, p-buff);

	const char *str = "GET /NVSB3_2 HTTP/1.0\r\n"
					  "HOST: NVSB3_2\r\n"
					  "Ntrip-Version: Ntrip/2.0\r\n"
					  "User-Agent: NTRIP ESP_GPS/2.0\r\n"
					  "Authorization: Basic c2JyNTAzNzo5NDAxNzI=\r\n\r\n";
	size_t count = _client->write((const uint8_t *)str, strlen(str));

	logger.debug("Conn str write count - %i\n", count);

	uint64_t start = millis();
	while (!_client->available() && millis() - start < 1000) {
		delay(10);
	}
}