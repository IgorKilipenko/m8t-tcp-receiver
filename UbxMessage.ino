
////////////////////////////////////////////////////////////////////
/* UbxMessage =================================================== */
////////////////////////////////////////////////////////////////////

UbxMessage::UbxMessage(uint8_t classId, uint8_t msgId, const uint8_t *buffer, size_t len) : _classId{classId}, _msgId{msgId}, _totalLength{len} {
	assert(buffer != nullptr);
	assert(len > 8);
	assert(sizeof(buffer) >= len);

	_checkMsg = buffer[2] == classId && buffer[3] == msgId;

	if (_checkMsg) {
		_payloadLength = utils::fromBytes<uint16_t>(buffer + 4, 2);
		_payload = new uint8_t[_payloadLength]{};
		memcpy(_payload, buffer + 6, _payloadLength);
	}
}

UbxMessage::~UbxMessage() {
	if (_payload != nullptr) {
		delete[] _payload;
	}
}

////////////////////////////////////////////////////////////////////
/* NavPOSLLHMessage ============================================= */
////////////////////////////////////////////////////////////////////

NavPOSLLHMessage::NavPOSLLHMessage(const uint8_t *buffer, size_t len) : UbxMessage::UbxMessage(static_cast<uint8_t>(ClassIds::NAV), static_cast<uint8_t>(NavMessageIds::POSLLH), buffer, len) {
    parsePayload();
}
NavPOSLLHMessage::~NavPOSLLHMessage(){};

void NavPOSLLHMessage::parsePayload()  {
    if (_payload == nullptr && sizeof(_payloadLength != LENGTH-8)){
        return;
    }
	uint8_t *cur_ptr = _payload;
	_itow = utils::fromBytes<uint32_t>(cur_ptr, 4);
	cur_ptr += 4;
	_longitude = utils::fromBytes<int32_t>(cur_ptr, 4);
	cur_ptr += 4;
	_latitude = utils::fromBytes<int32_t>(cur_ptr, 4);
	cur_ptr += 4;
	_height = utils::fromBytes<int32_t>(cur_ptr, 4);
	cur_ptr += 4;
	_heightMSL = utils::fromBytes<int32_t>(cur_ptr, 4);
	cur_ptr += 4;
	_horizontalAccuracy = utils::fromBytes<uint32_t>(cur_ptr, 4);
	cur_ptr += 4;
	_verticalAccuracy = utils::fromBytes<uint32_t>(cur_ptr, 4);
}