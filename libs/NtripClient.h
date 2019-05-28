#ifndef NtripClient_h
#define NtripClient_h

#include "Arduino.h"
#include "utils.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#else
#error Platform not supported
#endif

#define NTRIP_AGENT "ESP_GPS"
#define NTRIP_CLI_PORT 2101						  /* default ntrip-client connection port */
#define NTRIP_SVR_PORT 80						  /* default ntrip-server connection port */
#define NTRIP_MAXRSP 32768						  /* max size of ntrip response */
#define NTRIP_MAXSTR 256						  /* max length of mountpoint string */
#define NTRIP_RSP_OK_CLI "ICY 200 OK\r\n"		  /* ntrip response: client */
#define NTRIP_RSP_OK_SVR "OK\r\n"				  /* ntrip response: server */
#define NTRIP_RSP_SRCTBL "SOURCETABLE 200 OK\r\n" /* ntrip response: source table */
#define NTRIP_RSP_TBLEND "ENDSOURCETABLE"
#define NTRIP_RSP_HTTP "HTTP/"  /* ntrip response: http */
#define NTRIP_RSP_ERROR "ERROR" /* ntrip response: error */

class NtripClient {
  public:
	NtripClient(HardwareSerial *uart = Receiver);
	~NtripClient();

	void onConnect(void *arg, AsyncClient *client);
	void handleData(void *arg, AsyncClient *client, void *data, size_t len);
	void replyToServer(void *arg);
	bool connect(const char *host, uint16_t port, const char *user, const char *pass, const char *mountPoint, const char *nmea = nullptr);
	bool close();
	bool stop();

  private:
	HardwareSerial *_uart;
	AsyncClient *_client;
	bool _connect = false;
	char _user[256]{0};
	char _passwd[256]{0};
	uint16_t _port = 2021;
	char _host[256]{0};
	char _mntpnt[256]{0};
	char _connStr[1024]{0};
};

#endif // NtripClient_h