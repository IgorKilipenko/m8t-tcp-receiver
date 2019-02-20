

OTA::OTA() {}
OTA::~OTA() {}

void OTA::setup() {

	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		} else { // U_SPIFFS
			type = "filesystem";
		}

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		logger.println("Start updating " + type);
	});

	ArduinoOTA.onEnd([]() { logger.println("\nEnd"); });

	ArduinoOTA.onError([](ota_error_t error) {
		logger.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			logger.println("Auth Failed");
		} else if (error == OTA_BEGIN_ERROR) {
			logger.println("Begin Failed");
		} else if (error == OTA_CONNECT_ERROR) {
			logger.println("Connect Failed");
		} else if (error == OTA_RECEIVE_ERROR) {
			logger.println("Receive Failed");
		} else if (error == OTA_END_ERROR) {
			logger.println("End Failed");
		}
	});
	ArduinoOTA.begin();
	logger.println("Ready");
	logger.print("IP address: ");
	logger.println(WiFi.localIP());
}

void OTA::handle(){
    ArduinoOTA.handle();
}