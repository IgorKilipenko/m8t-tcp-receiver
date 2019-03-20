
////////////////////////////////////////////////////////////////////
/* UbxMessage =================================================== */
////////////////////////////////////////////////////////////////////

UbxMessage::UbxMessage(uint8_t classId, uint8_t msgId, const uint8_t *buffer, size_t len) : _classId{classId}, _msgId{msgId}, _totalLength{len} {
	assert(buffer != nullptr && len > 8 && sizeof(buffer) >= len);
	_buffer = new uint8_t[len]{};
	memcpy(_buffer, buffer, len);
	//_payload = _buffer + 6;
    _payloadLength = utils::fromBytes<uint16_t>(_buffer+4, 2);
    _payload = new uint8_t[_payloadLength]{};
    memcpy(_payload, buffer+6, _payloadLength);
	parsePayload();
}

UbxMessage::~UbxMessage() {
	// logger.trace("::~Destroy UbxMessage\n");
	if (_buffer != nullptr) {
		delete[] _buffer;
	}
    if (_payload != nullptr){
        delete [] _payload;
    }
}


////////////////////////////////////////////////////////////////////
/* NavPOSLLHMessage ============================================= */
////////////////////////////////////////////////////////////////////

NavPOSLLHMessage::NavPOSLLHMessage(const uint8_t * buffer, size_t len) : UbxMessage::UbxMessage(static_cast<uint8_t>(ClassIds::NAV), static_cast<uint8_t>(NavMessageIds::POSLLH), buffer, len) {}
NavPOSLLHMessage::~NavPOSLLHMessage(){};

void NavPOSLLHMessage::parsePayload() { 
    assert(_payload != nullptr);
    uint8_t * cur_ptr = _payload;
    _itow = utils::fromBytes<uint32_t>(cur_ptr, 4);
    cur_ptr += 4;
    _longitude = utils::fromBytes<int32_t>(cur_ptr, 4);
    cur_ptr += 4;
    _latitude = utils::fromBytes<int32_t>(cur_ptr, 4);
    cur_ptr += 4;
    _height = utils::fromBytes<int32_t>(cur_ptr, 4);
    cur_ptr +=4;
    _heightMSL = utils::fromBytes<int32_t>(cur_ptr, 4);
    cur_ptr += 4;
    _horizontalAccuracy = utils::fromBytes<uint32_t>(cur_ptr, 4);
    cur_ptr += 4;
    _verticalAccuracy = utils::fromBytes<uint32_t>(cur_ptr, 4);
}