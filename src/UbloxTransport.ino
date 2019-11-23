#include "libs/ublox.h"

const size_t UbloxTransport::MAX_BUFFER_SIZE = Ublox::MAX_MSG_LEN;

UbloxTransport::UbloxTransport(Stream &outStream) : Stream(), _outStream{&outStream} {}

UbloxTransport::~UbloxTransport() {
	log_v(":: destructor  :: UbloxTransport\n");
}

int UbloxTransport::read() {
	int count = available();
	if (count > 0) {
		count = _queue.front();
		_queue.pop();
	}

	return count;
}

int UbloxTransport::available() { return (_queue.size()); }

int UbloxTransport::availableForPush() {
	if (!_queue.empty()) {
		if (_queue.size() >= MAX_BUFFER_SIZE) {
			log_d("Buffer overflow, size: [%ld]\n", _queue.size());
			return -1;
		}
	}
	return MAX_BUFFER_SIZE - _queue.size();
}

size_t UbloxTransport::push(uint8_t c) {
	if (availableForPush() > 0) {
		_queue.push(c);
		return 1;
	} else {
		return 0;
	}
}

size_t UbloxTransport::push(const uint8_t *buffer, size_t size) {
	int count = availableForPush();
	if (count > 0) {
		count = std::min(size, (size_t)count);
		int i = 0;
		for (; i < count; i++) {
			_queue.push(buffer[i]);
		}
		return i;
	} else {
		return count;
	}
}

size_t UbloxTransport::write(uint8_t data) {
	if (_outStream != nullptr) {
		const size_t count = _outStream->write(data);
		return count;
	} else {
		log_e("Error write to stream, Stream is nullptr\n");
		return 0;
	}
}

void UbloxTransport::flush() {
	if (_outStream != nullptr)
		_outStream->flush();
}

int UbloxTransport::peek() {
	if (_outStream != nullptr) {
		return _outStream->peek();
	} else {
		return -1;
	}
}

void UbloxTransport::clear() {
	while (!_queue.empty()) {
		_queue.pop();
	}
}

int UbloxTransport::pushFromOutStream() {
	int available = _outStream->available();

	if (available < 0) {
		log_e("Error read from out stream\n");
		return -1;
	} else if (available == 0) {
		return 0;
	}

	uint8_t buffer[available]{0};
	available = _outStream->readBytes(buffer, available);
	if (available > 0) {
		return push(buffer, available);
	} else {
		return available;
	}
}

void UbloxTransport::setWaitResponse(bool enable) { _waitResponse = enable; }

Stream& UbloxTransport::getOutStream(){
	return {*_outStream};
}
