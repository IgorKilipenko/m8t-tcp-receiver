#include "net_helpers.h"
#include <ESP8266WiFi.h>

int findPriorNetwork(const char *ssid)
{
    byte available_networks = WiFi.scanNetworks();
    for (int network = 0; network < available_networks; network++)
    {
        if (strcmp(WiFi.SSID(network).c_str(), ssid) == 0)
        {
            return WiFi.RSSI(network);
        }
    }
}
