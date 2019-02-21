#ifndef ATcpServer_h
#define ATcpServer_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include "SDStore.h"
#include <vector>

#ifndef TCP_PORT
#define TCP_PORT 7042
#endif

#ifndef MAX_TCP_CLIENTS
#define MAX_TCP_CLIENTS 5 // Default 5 clients
#endif

class ATcpServer {
  public:
	ATcpServer();
	~ATcpServer();

	// TelnetServer methods:
	void process();
	void stopReceive();
	void startReceive();
	bool isInProgress();
    void sendDataToClients(uint8_t buffer[], size_t bytesCount);

	void handleError(AsyncClient *client, int8_t error);
    void handleData(AsyncClient *client, void *data, size_t len);
    void handleDisconnect(AsyncClient *client);
    void handleTimeOut(AsyncClient *client, uint32_t time);
    void handleNewClient(AsyncClient *client);
    void end();
    void freeClients();
    void sendMessage(AsyncClient *client, const char msg[], size_t len);
    void sendMessage(AsyncClient *client, String str);
    void setup();



  private:
	std::vector<AsyncClient*> clients; // a list to hold all clients
	bool receiveData = false;
    SDStore * store = nullptr;
    AsyncServer* server = nullptr;
};

#endif