#include "AWebServer.h"

bool AWebServer::_static_init = false;

const char *AWebServer::API_P_GPSCMD = "cmd";

AWebServer::AWebServer(ATcpServer *telnetServer, HardwareSerial* receiver)
	: softAP_ssid{APSSID}, softAP_password{APPSK}, ssid{APSSID}, password{APPSK}, hostName{"GPS IoT "}, server{80}, ws{"/ubx"}, events{"/events"}, telnetServer{telnetServer},
	  _ubxDecoder{}, _transport{new UbloxTransport(*receiver)}, api{}, _receiver{receiver} {
	String id = utils::getEspChipId();
	strcat(softAP_ssid, id.c_str());
	strcat(hostName, id.c_str());

	//_gps = new UBLOX_GPS(_transport);
}

AWebServer::~AWebServer() {
	end();
	if (_transport != nullptr) {
		delete _transport;
	}
}

void AWebServer::end() {
	SPIFFS.end();
	if (telnetServer) {
		telnetServer->stopReceive();
		// delete telnetServer;
		// telnetServer = nullptr;
	}
	ws.closeAll();
	if (ws.enabled()) {
		ws.enable(false);
	}
	//logger.clearEventSource();
	server.reset();
	vTaskDelete(_runTasks);
}

void AWebServer::restart() {
	end();
	setup();
}

void AWebServer::setup() {

	try {
		throw runtime_error("Test error");
	} catch (const runtime_error &e) {
		log_e("Catch errr [%s]", e.what());
	}

	//loadWiFiCredentials();

	addOTAhandlers();

	ArduinoOTA.setHostname(hostName);
	ArduinoOTA.begin();

	addReceiverHandlers();

	MDNS.addService("http", "tcp", 80);

	SPIFFS.begin();

	addServerHandlers();

	telnetServer->setup();

	_ntripClient = new NtripClientSync(_receiver);

	initDefaultHeaders();
	server.begin();
}

/** Credentials ================================================ */
/** Load WLAN credentials from EEPROM */
void AWebServer::loadWiFiCredentials() {
	log_v("Start load Credential");

	log_d("ssid sizeof: ");
	log_d("%i\n", sizeof(ssid));
	log_d("EEROM start\n");
	EEPROM.begin(512);
	log_d("EEROM begin\n");
	EEPROM.get(0, ssid);
	EEPROM.get(0 + sizeof(ssid), password);
	char ok[2 + 1];
	EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
	EEPROM.end();
	if (String(ok) != String("OK")) {
		ssid[0] = 0;
		password[0] = 0;
	}
	log_v("Recovered credentials:");
	log_v("SSID -> %s", ssid);
	log_v("PASSWORD -> %s", strlen(password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void AWebServer::saveWiFiCredentials() {
	log_v("Start save Credential");

	EEPROM.begin(512);
	EEPROM.put(0, ssid);
	EEPROM.put(0 + sizeof(ssid), password);
	char ok[2 + 1] = "OK";
	EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
	EEPROM.commit();
	EEPROM.end();
}

/** Set default headers */
void AWebServer::initDefaultHeaders() {
	if (!AWebServer::_static_init) {
		DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
		// DefaultHeaders::Instance().addHeader("Access-Control-Expose-Headers","Access-Control-Allow-Origin");
		DefaultHeaders::Instance().addHeader("charset", "ANSI");
		// DefaultHeaders::Instance().addHeader("charset", "Windows-1252");
		DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,HEAD,OPTIONS,POST,PUT");
		DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers",
											 "Access-Control-Allow-Headers, Origin,Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers");
		AWebServer::_static_init = true;
	}
}



void AWebServer::initializeGpsReceiver() {
	//log_v("Start GPS BEGIN\n");
	//_gps->enableDebugging(logger.getStream());
	// if (_gps->begin()) {
	//	log_d("GPS Receiver connected\n");
	//	_gpsIsInint = true;
	//} else {
	//	logger.error("GPS Receiver not connected\n");
	//	_gpsIsInint = false;
	//}
}

/** Main process */
void AWebServer::process() {

	if (!telnetServer->isInProgress()) {
		ArduinoOTA.handle();
	}

	telnetServer->process();
	//if (!_gpsIsInint) {
	//	initializeGpsReceiver();
	//}

	if (_ntripClient->isEnabled()) {
		_ntripClient->receiveNtrip();
	}

	delay(1);
}

/** Main process */
void AWebServer::process(std::vector<char> buf) {

	if (!telnetServer->isInProgress()) {
		ArduinoOTA.handle();
	}

	telnetServer->processData(buf.data(), buf.size());

	if (_ntripClient->isEnabled()) {
		_ntripClient->receiveNtrip();
	}

	delay(1);
}

/** Main process */
void AWebServer::_process(void *arg) {
	// AWebServer *_this = static_cast<AWebServer*>(arg);
	// micros();
	// vTaskDelay(10);
	//_this->setup();
	//_this->setup();
	// for (;;) {
	//	//_this->process();
	//	//vTaskDelay(1 / portTICK_PERIOD_MS);
	//	delay(1);
	//}

	log_d("xCore -> [%i]", xPortGetCoreID());
	AWebServer *_this = static_cast<AWebServer *>(arg);
	_this->setup();
	for (;;) {
		_this->process();
		// vTaskDelay(1 / portTICK_PERIOD_MS);
		delay(1);
		// log_d("xCore -> [%i]", xPortGetCoreID());
	}
}

void AWebServer::run(BaseType_t coreId) {
	delay(500);
	xTaskCreatePinnedToCore(&_process, "_process", 1024 * 8, this, 1, &_runTasks, coreId);
}

bool AWebServer::isCanSendData() { return /* (WM.apEnabled() || WM.staConnected()); */ true;}