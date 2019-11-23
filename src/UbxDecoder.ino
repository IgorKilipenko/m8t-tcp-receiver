
UbxDecoder::UbxDecoder() : _buffer{new uint8_t[Ublox::MAX_MSG_LEN]} {}

UbxDecoder::~UbxDecoder() {
	log_v(":: destructor  :: UbxDecoder\n");
	if (_buffer != nullptr) {
		delete[] _buffer;
		_buffer = nullptr;
	}
}

int8_t UbxDecoder::inputData(uint8_t data) {

	// log_v("Start input data...\n");

	if (_nbyte == 0) {
		if (!_syncHeader(data)) {
			return 0;
		} else {
			_nbyte = 2;
			return 0;
		}
	}
	
	if (_nbyte == 2 && (data < static_cast<uint8_t>(ClassIds::NAV) || data > static_cast<uint8_t>(ClassIds::SEC) || data == static_cast<uint8_t>(ClassIds::RXM))) {
		_nbyte = 0;
		return 0;
	}

	_buffer[_nbyte++] = data;

	if (_nbyte == 6) {
		_length = utils::fromBytes<uint16_t>(_buffer + 4, 2) + 8;
		log_v("Payload length = [%d]\n", _length);
		if (_length > Ublox::MAX_MSG_LEN) {
			log_v("Length message error, length = [%d]\n", _length);
			_nbyte = 0;
			return -1;
		}
	}

	if (_nbyte == _length) {
		_nbyte = 0;
		log_v("Ubx message complite, (UbxDecoder)\n");
		if (_testChecksum()) {
			log_v("Ubx message success, length = [%d]\n", _length);
			return _decode();
		}
	}

	return 0;
}

int8_t UbxDecoder::_decode() {
	log_v("Start decode UBX message...\n");

	uint8_t classId = *(_buffer + 2);
	uint8_t msgId = *(_buffer + 3);

	switch (classId) {
	case (uint8_t)ClassIds::NAV:
		if (_msgCallback != nullptr){
			MessageEventPtr e = std::shared_ptr<MessageEvent>(new MessageEvent(classId, msgId, _buffer+4));
			_msgCallback(e);
		}
	}
	return static_cast<int8_t>(classId);
}

bool UbxDecoder::_testChecksum() {
	log_v("Start test checksum...\n");

	// assert(_buffer != nullptr);
	// assert(_length >= 8);
	// assert(_nbyte == 0);

	uint8_t ck_a = 0, ck_b = 0;

	for (uint16_t i = 2; i < _length - 2; i++) {
		ck_a += _buffer[i];
		ck_b += ck_a;
	}

	return ck_a == _buffer[_length - 2] && ck_b == _buffer[_length - 1];
}

bool UbxDecoder::_syncHeader(uint8_t data) {
	_buffer[0] = _buffer[1];
	_buffer[1] = data;

	return _buffer[0] == Ublox::HEADER_BYTES[0] && _buffer[1] == Ublox::HEADER_BYTES[1];
}
