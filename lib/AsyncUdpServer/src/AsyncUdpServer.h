#ifndef AsyncUdpServer_h
#define AsyncUdpServer_h

#include "Arduino.h"
#include <AsyncUDP.h>
#include "WiFiWithEvents.h"

class AsyncUdpServer {

  public:
	AsyncUdpServer(WiFiWithEvents &wifi);
	void send(uint8_t buffer, int len);
    void startRecive()

  private:
	WiFiWithEvents &_wifi;
	AsyncUDP _udp;
    AsyncUDPPacket _packet;
};

#endif // AsyncUdpServer_h