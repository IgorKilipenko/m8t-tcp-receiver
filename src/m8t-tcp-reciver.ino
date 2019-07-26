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
#define BAUD_SERIAL 115200	// Debug Serial baund rate
#define BAUND_RECEIVER 921600 // GPS receiver baund rate
#define SERIAL_SIZE_RX 1024 * 2

#include "libs/utils.h"
#include "libs/Logger.h"
#include "libs/AWebServer.h"
#include "libs/ATcpServer.h"
#include "libs/WiFiManager.h"

TaskHandle_t Task1;
TaskHandle_t Task2;

Logger logger{&Serial}; // For debug mode
WiFiManager WM{};
ATcpServer telnetServer{}; // GPS receiver communication

AWebServer webServer{&telnetServer};

void setup() {
	enableCore0WDT();
	log_v("========= CORE -> [%i]", xPortGetCoreID());
	Serial.begin(BAUD_SERIAL);
	Receiver->begin(BAUND_RECEIVER, SERIAL_8N1, RXD2, TXD2);
	RTCM->begin(38400, SERIAL_8N1, RXD1, TXD1);

	Receiver->setRxBufferSize(SERIAL_SIZE_RX);
	String hostName = String("ESP_GPS_") + utils::getEspChipId();

	WM.setup(hostName.c_str());
	//webServer.setup();

	webServer.run(0);

	//xTaskCreatePinnedToCore(TaskCore0, "TaskCore0", 1024, NULL, 2, &Task1, 0);
	//delay(500);

	//xTaskCreatePinnedToCore(TaskCore1, "TaskCore1", 1024, NULL, 1, &Task2, 1);
	//delay(500);
}

void loop() { /*webServer.process(); */}

void TaskCore0(void *pvParameters) // This is a task.
{
	(void)pvParameters;

	// webServer.setup();

	for (;;) // A Task shall never return or exit.
	{
		Serial.println(xPortGetCoreID());
		// webServer.setup();
		delay(10000);
	}
}

void TaskCore1(void *pvParameters) // This is a task.
{
	(void)pvParameters;

	for (;;) {
		delay(3000);
		// webServer.process();
		Serial.println(xPortGetCoreID());
	}
}