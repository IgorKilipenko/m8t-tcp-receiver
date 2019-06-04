#include "libs/ublox.h"

UbloxTransport::UbloxTransport(Stream &outStream, size_t buff_len) : Stream(), _outStream{outStream}, _buffer{new uint8_t[buff_len]{0}} { _curr = &_buffer[0]; }

UbloxTransport::~UbloxTransport() {
	if (_buffer) {
		delete[] _buffer;
	}
}

int UbloxTransport::read() {
	const int count = available();
	if (count > 0) {
		int res = *_curr;
		_curr--;
		return res;
	} else {
		return count;
	}
}

int UbloxTransport::available() {
	if (!_check()) {
		return -1;
	} else {
		return (_curr - _buffer);
	}
}

int UbloxTransport::availableSet() {
	if (!_check()) {
		return -1;
	} else {
		return (sizeof(_buffer) - (_curr - _buffer));
	}
}

size_t UbloxTransport::setBufferBytes(uint8_t c) {
	const int count = availableSet();
	if (count > 0) {
		_curr++;
		*_curr = c;
		return 1;
	} else if (count == 0) {
		logger.debug("Buffer overflow, size: [%i]\n", sizeof(_buffer));
		_curr = &_buffer[0];

		return 0;
	} else {
		return -1;
	}
}

size_t UbloxTransport::setBufferBytes(const uint8_t *buffer, size_t size) {
	int count = availableSet();
	if (count > 0) {
		size_t n = 0;
		count = std::min(size, (size_t)count);
		while (count-- > 0) {
			*_curr++ = *buffer++;
			n++;
		}
		return n;
	} else if (count == 0) {
		logger.debug("Buffer overflow, size: [%i]\n", sizeof(_buffer));
		_curr = &_buffer[0];

		return 0;
	} else {
		return -1;
	}
}

size_t UbloxTransport::write(uint8_t data) { return _outStream.write(data); }

bool UbloxTransport::_check() {
	if (_buffer && _curr) {
		return true;
	} else {
		return false;
	}
}

void UbloxTransport::flush(){
    _outStream.flush();
}

int UbloxTransport::peek(){
    return _outStream.peek();
}