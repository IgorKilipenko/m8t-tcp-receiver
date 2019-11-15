#ifndef NtripClientSync_h
#define NtripClientSync_h

#include "Arduino.h"
#include "utils.h"
#include <algorithm>

#ifdef ESP32
#include <WiFi.h>
#else
#error Platform not supported
#endif

#define NTRIP_AGENT "ESP_GPS"
#define NTRIP_CLI_PORT 2101						  /* default ntrip-client connection port */
#define NTRIP_SVR_PORT 80						  /* default ntrip-server connection port */
#define NTRIP_MAXRSP 32768						  /* max size of ntrip response */
#define NTRIP_MAXSTR 256						  /* max length of mountpoint string */
#define NTRIP_RSP_OK_CLI "ICY 200 OK"		  /* ntrip response: client */
#define NTRIP_RSP_OK_SVR "OK\r\n"				  /* ntrip response: server */
#define NTRIP_RSP_SRCTBL "SOURCETABLE 200 OK\r\n" /* ntrip response: source table */
#define NTRIP_RSP_TBLEND "ENDSOURCETABLE"
#define NTRIP_RSP_HTTP "HTTP/"  /* ntrip response: http */
#define NTRIP_RSP_ERROR "ERROR" /* ntrip response: error */

#define NTRIP_BUFFER_LENGTH 512
#define NTRIP_HTTP_OK "HTTP/1.1 200 OK"

class NtripClientSync {
	
  public:
	//const static char* HTTP_OK;

	NtripClientSync(HardwareSerial *uart = Receiver);
	~NtripClientSync();

	bool connect(const char *host, uint16_t port, const char *user, const char *pass, const char *mountPoint, const char *nmea = nullptr);
	void stop();
	bool isEnabled();
	int receiveNtrip();
	int read(uint8_t * buffer, size_t length);
	

  private:
	HardwareSerial *_uart;
	WiFiClient *_client;
	bool _connectedNtrip = false;
	//bool _connect = false;
	char _user[256]{0};
	char _passwd[256]{0};
	uint16_t _port = 2021;
	char _host[256]{0};
	char _mntpnt[256]{0};
	char _connectionString[1024]{0};
	uint8_t _buffer[NTRIP_BUFFER_LENGTH]{0};

	size_t buildConnStr(char* connStr, const char *host, uint16_t port, const char *user, const char *pass, const char *mountPoint, const char *nmea = nullptr);
	bool requestNtrip();
	int readLine(uint8_t *buffer, size_t len);
};

#endif // NtripClientSync_h