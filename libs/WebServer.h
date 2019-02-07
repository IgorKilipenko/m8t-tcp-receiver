/*
    WebServer
*/

#ifndef WebServer_h
#define WebServer_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#ifndef APSSID
#define APSSID "ESP_ap"
#define APPSK "12345678"
#endif

class WebServer
{
  private:
    //void handleRoot();
    //boolean captivePortal();
    //void handleWifi();
    //void handleWifiSave();
    //void handleNotFound();
    void setup();

  public:
    WebServer(/* args */);
    ~WebServer();
};

WebServer::WebServer(/* args */)
{
}

WebServer::~WebServer()
{
}

#endif