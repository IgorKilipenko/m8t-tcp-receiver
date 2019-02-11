/*
	TelnetServer
*/

#ifndef TelnetServer_h
#define TelnetServer_h

#include "Arduino.h"

class TelnetServer {
	  private:
		WiFiServer server{7042};
		bool wifiConnected = false;
		static const int MAX_CLIENTS = 5;
		WiFiClient clients[MAX_CLIENTS];

		static const size_t MAX_FNAME_LEN = 13;
		char filename[MAX_FNAME_LEN];

		static const int RXPin = D2, TXPin = D1;

		File myFile;
		bool cdCard = false;

		unsigned long start;

	  public:
		TelnetServer();
		~TelnetServer();

		void getDataFromClients();
		void generateFileName(char *);
		void writeToSD(byte buffer[], size_t bytesCount);
		void sendToClient(byte buffer[], size_t bytesCount);
		bool setFreeClientSpot();

		void setup();
		void process();
		void processEx();
};

#endif