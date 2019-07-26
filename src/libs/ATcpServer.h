#ifndef ATcpServer_h
#define ATcpServer_h

#include "Arduino.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#error Platform not supported
#endif

#include "SDStore.h"
#include <vector>

#ifndef TCP_PORT
#define TCP_PORT 7042
#endif

#ifndef MAX_TCP_CLIENTS
#define MAX_TCP_CLIENTS 5 // Default 5 clients
#endif

#define BUFFER_SIZE 2048

typedef std::function<void(const uint8_t *, size_t)> SerialDataCallback;

class ATcpServer {
  public:
	ATcpServer();
	~ATcpServer();

	// TelnetServer methods:
	void process();
	void stopReceive();
	void startReceive(bool writeToSd = true, bool sendToTcp = true);
	bool isInProgress() const;
	bool isSdInitialize() const;
	unsigned long getTimeReceive() const;
	unsigned long getTimeStart() const;
	void sendDataToClients(const char *buffer, size_t bytesCount);
	void onSerialData(SerialDataCallback cb) { _seralDataCallback = cb; }

	void end();
	size_t sendMessage(AsyncClient *client, const char *msg, size_t len);
	size_t sendMessage(AsyncClient *client, String str);
	void setup();
	size_t availableClientsCount();

	bool sendToTcpEnabled() const { return _sendToTcp; }
	bool writeToSdEnabled() const { return _writeToSd; }
	void runTask();

  private:
	std::vector<AsyncClient *> clients; // a list to hold all clients
	bool receiveData = false;
	SDStore *store = nullptr;
	AsyncServer *server = nullptr;
	AsyncServer *serviceServer = nullptr;
	SerialDataCallback _seralDataCallback = nullptr;
	unsigned long _timeStart;
	unsigned long _timeEnd;

	bool _writeToSd = true;
	bool _sendToTcp = true;

	size_t freeClients();
	void handleError(AsyncClient *client, int8_t error);
	void handleData(AsyncClient *client, void *data, size_t len);
	void handleDisconnect(AsyncClient *client);
	void handleTimeOut(AsyncClient *client, uint32_t time);
	void handleNewClient(AsyncClient *client);
	char _buffer[BUFFER_SIZE]{0};
	void _processData(char *buffer, int len);
};

#endif