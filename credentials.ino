/** Load WLAN credentials from EEPROM */
void loadCredentials(char ssid[32], char password[32]) {
		EEPROM.begin(512);
		EEPROM.get(0, ssid);
		EEPROM.get(0 + sizeof(ssid), password);
		char ok[2 + 1];
		EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
		EEPROM.end();
		if (String(ok) != String("OK")) {
				ssid[0] = 0;
				password[0] = 0;
		}
		logger.println("Recovered credentials:");
		logger.println(ssid);
		logger.println(strlen(password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void saveCredentials(char ssid[32], char password[32]) {
		EEPROM.begin(512);
		EEPROM.put(0, ssid);
		EEPROM.put(0 + sizeof(ssid), password);
		char ok[2 + 1] = "OK";
		EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
		EEPROM.commit();
		EEPROM.end();
}