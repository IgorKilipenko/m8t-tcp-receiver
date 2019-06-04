#ifndef ublox_h
#define ublox_h

#include "Arduino.h"
#include "utils.h"

enum class ClassIds : uint8_t;
enum class NavMessageIds : uint8_t;
enum class RxmMessageIds : uint8_t;
enum class SecMessageIds : uint8_t;
class MessageEvent;

using MessageEventPtr = std::shared_ptr<MessageEvent>;
using MessageCallback = std::function<void(MessageEventPtr)>;

class Ublox {
  public:
	Ublox();
	~Ublox();

	static const uint8_t HEADER_BYTES[];
	static const uint16_t MAX_MSG_LEN = 8192;
};

enum class ClassIds : uint8_t {
	NAV = 0x01,
	RXM = 0x02,
	INF = 0x04,
	ACK = 0x05,
	CFG = 0x06,
	UPD = 0x09,
	MON = 0x0A,
	TIM = 0x0D,
	MGA = 0x13, // Multiple GNSS Assistance Messages: Assistance data for various GNSS
	LOG = 0x21,
	SEC = 0x27
};

enum class NavMessageIds : uint8_t {
	CLOCK = 0x22,
	DOP = 0x04,
	EOE = 0x61,
	GEOFENCE = 0x39,
	HPPOSECEF = 0x13,
	HPPOSLLH = 0x014,
	ODO = 0x09,
	ORB = 0x34,
	POSECEF = 0x01,
	POSLLH = 0x02,
	PVT = 0x07,
	RELPOSNED = 0x3C,
	RESETODO = 0x10,
	SAT = 0x35,
	SIG = 0x43,
	STATUS = 0x03,
	SVIN = 0x3B,
	TIMEBDS = 0x24,
	TIMEGAL = 0x25,
	TIMEGLO = 0x23,
	TIMEGPS = 0x20,
	TIMELS = 0x26,
	TIMEUTC = 0x21,
	VELECEF = 0x11,
	VELNED = 0x12
};

enum class RxmMessageIds : uint8_t { MEASX = 0x14, PMREQ = 0x41, RAWX = 0x15, RLM = 0x59, RTCM = 0x32, SFRBX = 0x13 };

enum class SecMessageIds : uint8_t { UNIQID = 0x03 };

class UbxDecoder {
  public:
	UbxDecoder();
	~UbxDecoder();
	int8_t inputData(uint8_t data);
	const uint8_t *getBuffer() const { return _buffer; }
	uint16_t getLength() const { return _length; }

  private:
	uint8_t *_buffer;
	uint16_t _nbyte = 0;
	uint16_t _length;

	MessageCallback _msgCallback;

	const ClassIds _allowedClasses = ClassIds::NAV;

	bool _testChecksum();
	bool _syncHeader(uint8_t data);
	int16_t _decode();
};

class MessageEvent {
  public:
	MessageEvent(uint8_t classId, uint8_t msgId, uint8_t *payload) : _classId{classId}, _msgId{msgId}, _payload{payload} {}
	~MessageEvent() {}
	uint8_t getClassId() const { return _classId; }
	uint8_t getMessageId() const { return _msgId; }
	const uint8_t *getPayload() const { return _payload; }

  private:
	uint8_t _classId = 0;
	uint8_t _msgId = 0;
	uint8_t *_payload;
};

class UbloxTransport : public Stream {
  public:
	UbloxTransport(Stream &outStream, size_t buff_len = 1024);
	~UbloxTransport() override;
	int available() override;
	int read() override;
	int peek() override;
	void flush() override;
	size_t write(uint8_t) override;
	size_t setBufferBytes(uint8_t);
	size_t setBufferBytes(const uint8_t *, size_t);
	int availableSet();

  private:
	Stream &_outStream;
	uint8_t *_buffer;
	uint8_t *_curr{nullptr};

	bool _check();
};

#endif // ublox_h