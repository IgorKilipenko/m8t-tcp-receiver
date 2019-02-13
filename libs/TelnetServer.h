/*
	TelnetServer
*/

#ifndef TelnetServer_h
#define TelnetServer_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <SD.h>

class TelnetServer {
	  private:
		WiFiServer server{7042};
		bool wifiConnected = false;
		static const int MAX_CLIENTS = 5;
		WiFiClient clients[MAX_CLIENTS];

		static const size_t MAX_FNAME_LEN = 13;
		char filename[MAX_FNAME_LEN];

		static const int RXPin = D2, TXPin = D1;

		File sdFile;
		bool sdCard = false;
		bool receiveData = false;

	  public:
		TelnetServer();
		~TelnetServer();

		void getDataFromClients();
		void generateFileName(char *);
		void writeToSD(byte buffer[], size_t bytesCount);
		void sendToClient(byte buffer[], size_t bytesCount);
		bool setFreeClientSpot();
		void createFile();
		void initSdCard();

		void startReceive();
		void stopReceive();
		void process();
};

#endif