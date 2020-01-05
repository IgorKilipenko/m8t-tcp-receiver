#ifndef WiFiWithEvents_h
#define WiFiWithEvents_h

#include "Arduino.h"

class WiFiWithEvents {
  public:
	WiFiWithEvents() {}
	~WiFiWithEvents() {}

    void connectSta() {
        WiFi.begin(ssid, password);
    }


  private:
	const char *ssid = "Keenetic-9267";
	const char *password = "1234567890";
};

#endif // WiFiWithEvents_h