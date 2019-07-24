#define TTGO_BOARD // Use TTGO LORA BOARD

#define DEBUG // Uncomment for enable debug mode
#define LOG_LEVEL 2
#define WEB_LOG_LEVEL 3

#define ALTSSID

/* SD card */
#ifdef ESP32
#ifdef TTGO_BOARD
#define CS_PIN 13
#define RXD2 32
#define TXD2 33
#define RXD1 13
#define TXD1 23
#else
#define CS_PIN 5 // SD card cs_pin (default D5 GPIO5 on ESP32 DevkitV1)
#define RXD2 16
#define TXD2 17
#define RXD1 4
#define TXD1 15
#endif

HardwareSerial *RTCM{&Serial1};
HardwareSerial *Receiver{&Serial2};
#elif defined(ESP8266)
#define CS_PIN D8 // SD card cs_pin (default D8 on ESP8266)
#define MOCK_RECEIVER_DATA
HardwareSerial *Receiver{&Serial};
#else
#error Platform not supported
#endif

/* TCP Client */
#define MAX_TCP_CLIENTS 5 // Default max clients
#define TCP_PORT 7042	 // Default tcp port (GPS receiver communication)

/* Serial */
#define BAUD_SERIAL 115200 // Debug Serial baund rate
#define BAUND_RECEIVER 921600	   // GPS receiver baund rate
#define SERIAL_SIZE_RX 1024*2

#include "libs/utils.h"
#include "libs/Logger.h"
#include "libs/AWebServer.h"
#include "libs/ATcpServer.h"
#include "libs/WiFiManager.h"

Logger logger{&Serial};	// For debug mode
WiFiManager WM{}
ATcpServer telnetServer{}; // GPS receiver communication

AWebServer webServer{&telnetServer};

void setup() {
	Serial.begin(BAUD_SERIAL);
	Receiver->begin(BAUND_RECEIVER, SERIAL_8N1, RXD2, TXD2);
	RTCM->begin(38400, SERIAL_8N1, RXD1, TXD1);
	Receiver->setRxBufferSize(SERIAL_SIZE_RX);
	WM.setup("esp_ap_host");
	webServer.setup();
}

void loop() { webServer.process(); }
