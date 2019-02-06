#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "ESP_ap"
#define APPSK "12345678"
#endif

class WebServer
{
  private:
    const char *softAP_ssid = APSSID;
    const char *softAP_password = APPSK;
    const char *myHostname = "esp8266";
    char ssid[32] = "";
    char password[32] = "";

    // DNS server
    const byte DNS_PORT = 53;
    DNSServer dnsServer;

    // Web server
    ESP8266WebServer server(80);

    /* Soft AP network parameters */
    IPAddress apIP(192, 168, 4, 1);
    IPAddress netMsk(255, 255, 255, 0);

    /** Should I connect to WLAN asap? */
    boolean connect;

    /** Last time I tried to connect to WLAN */
    unsigned long lastConnectTry = 0;

    /** Current WLAN status */
    unsigned int status = WL_IDLE_STATUS;

  public:
    WebServer(/* args */);
    ~WebServer();
    void handleRoot();
    boolean captivePortal();
    void handleWifi();
    void handleWifiSave();
    void handleNotFound();
    void setup();
};

WebServer::WebServer(/* args */)
{
}

WebServer::~WebServer()
{
}

void WebServer::setup()
{
    delay(1000);
    //Serial.println();
    //Serial.println("Configuring access point...");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(softAP_ssid, softAP_password);
    delay(500); // Without delay I've seen the IP address blank
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    /* Setup the DNS server redirecting all the domains to the apIP */
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", apIP);

    /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
    server.on("/", handleRoot);
    server.on("/wifi", handleWifi);
    server.on("/wifisave", handleWifiSave);
    server.on("/generate_204", handleRoot); //Android captive portal. Maybe not needed. Might be handled by notFound handler.
    server.on("/fwlink", handleRoot);       //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    server.onNotFound(handleNotFound);
    server.begin(); // Web server start
    Serial.println("HTTP server started");
    loadCredentials();          // Load WLAN credentials from network
    connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
}
