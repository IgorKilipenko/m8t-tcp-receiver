#ifndef UbxMessage_h
#define UbxMessage_h

#include "Arduino.h"
#include "ublox.h"

class UbxMessage {
    public:
    //static UbxMessage Decode();
    ~UbxMessage();
    uint8_t getClassId() const { return _classId;}
    uint8_t getMessageId() const {return _msgId;}
    const uint8_t * getPayload() const {return _payload;}

    protected:
    uint8_t * _buffer;
    uint8_t * _payload;
    const uint8_t _classId;
    const uint8_t _msgId;
    const size_t _totalLength;
    uint16_t _payloadLength;
    UbxMessage(uint8_t classId, uint8_t msgId, const uint8_t * buffer, size_t len);
    virtual void parsePayload();
};

class NavPOSLLHMessage : public UbxMessage {
    public:
    NavPOSLLHMessage(const uint8_t * buffer, size_t len);
    ~NavPOSLLHMessage();
    static const size_t LENGTH = 36;
    uint32_t iTOW() const {return _itow;}
    int32_t longitude() const {return _longitude;}
    int32_t latitude() const {return _latitude;}
    int32_t height() const {return _height;}
    int32_t heightMSL() const {return _heightMSL;}
    uint32_t horizontalAccuracy() const {return _horizontalAccuracy;}
    uint32_t verticalAccuracy() const {return _verticalAccuracy;}

    private:
    uint32_t _itow;
    int32_t _longitude;
    int32_t _latitude;
    int32_t _height;
    int32_t _heightMSL;
    uint32_t _horizontalAccuracy;
    uint32_t _verticalAccuracy;

    void parsePayload() override;
};

#endif      //UbxMessage_h