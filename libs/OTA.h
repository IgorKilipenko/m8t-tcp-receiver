/*
	OTA updater
*/

#ifndef OTA_h
#define OTA_h

#include "Arduino.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

class OTA {
    public:
    OTA();
    ~OTA();
    void setup();
    void handle();

    private:
};

 #endif //OTA_h