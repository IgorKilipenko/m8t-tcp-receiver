/*
	TelnetServer
*/

#ifndef TelnetServer_h
#define TelnetServer_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include <algorithm> // std::min

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

		void generateFileName(char *);
		void writeToSD(uint8_t[], size_t);
		void sendDataToClients(uint8_t[], size_t);
		bool handleClients(); 
		void createFile();
		void initSdCard();
		void getDataFromClients();

		void startReceive();
		void stopReceive();
		void process();

		/** return true if data transfer in progress */
		bool isInProgress();
};

#endif