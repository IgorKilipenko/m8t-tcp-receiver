
TelnetServer::TelnetServer() {}
TelnetServer::~TelnetServer() {}

void TelnetServer::setup() {
  // WiFi.begin(ssid, password);

  unsigned long start = millis();
  // while (WiFi.status() != WL_CONNECTED && millis() - start < 30000)
  //{
  //    delay(500);
  //    logger.print(".");
  //}
#ifdef DEBUG
  // if (WiFi.status() == WL_CONNECTED)
  //{
  //    logger.debug("WiFi connected on ");
  //    logger.debug("IP -> %s\n", WiFi.localIP().toString().c_str());
  //}
  // else
  //{
  //    logger.print("WiFi not connected");
  //}
#endif

  if (!SD.begin(CS_PIN)) {
    // initialization failed!
    cdCard = false;
    // return;
  } else {
    cdCard = true;
    logger.debug("initialization done.\n");
    generateFileName(filename);
    if (SD.exists(filename)) {
      SD.remove(filename);
      logger.debug("File: %s removed\n", filename);
    }
    myFile = SD.open(filename, FILE_WRITE);
    logger.debug("File: %s created\n", filename);
  }

  // for (unsigned long start = millis(); millis() - start < 1000;)
  //{
  //    //Serial1.print("#");
  //    delay(500);
  //}

  server.begin();
  server.setNoDelay(true);

  start = millis();
}

void TelnetServer::process() {
  if (WiFi.status() == WL_CONNECTED) {
    bool clientConnected = false;
    for (int i = 0; i < MAX_CLIENTS; i++) {
      WiFiClient &client = clients[i];
      if (client && client.connected()) {
        while (client.available()) {
          Serial.write(client.read());
          logger.debug("Data has been sent to Serial\n");
        }
      } else if (!client || !client.connected()) {
        if (client) {  // Нет такого события
          client.stop();
          logger.debug("Close connection to client\n");
        }
        if (server.hasClient()) {
          client = server.available();
          clientConnected = true;
          logger.debug(
              "Client connected, ip address: %s\n\t-> Connected clients count: "
              "%i\n",
              client.remoteIP().toString().c_str(), i + 1);
          break;
        }
      }
      clientConnected = false;
    }

    if (server.hasClient() && !clientConnected) {
      server.available().stop();
      logger.debug("MAX_CONNECTION, connection closed.\n");
    }
  }

#ifdef DEBUG
  size_t bytesCount = 2;
#else
  size_t bytesCount = Serial.available();
#endif
  if (bytesCount > 0) {
    // logger.debug("Start send data...\n");
#ifdef DEBUG
    byte buffer[] = "T";
#else
    byte buffer[bytesCount];
    Serial.readBytes(buffer, bytesCount);
#endif

    if (myFile) {
#ifdef DEBUG
      logger.debug("Write data: (%s) to SD", buffer);
#else
      myFile.write(buffer, bytesCount);
#endif
    }

    if (WiFi.status() == WL_CONNECTED) {
      for (int i = 0; i < MAX_CLIENTS; i++) {
        WiFiClient &client = clients[i];
        if (&client && client && client.connected()) {
          client.write(buffer, bytesCount);
          logger.debug("Data has been sent to client\n");
          delay(1);
        }
      }
    }

#ifdef DEBUG
    delay(1000);
#else
    delay(1);
#endif
  }
  if (myFile) {
    myFile.flush();
  }

  delay(1);
}

void TelnetServer::getDataFromClients() {
  if (WiFi.status() == WL_CONNECTED) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
      WiFiClient &client = clients[i];
      if (&client && client && client.connected()) {
        while (client.available()) {
          Serial.write(client.read());
        }

        logger.debug("Data has been sent to Serial\n");
        delay(1);
      }
    }
  }
}

void TelnetServer::generateFileName(char *name) {
  const size_t max_nums = 999;
  bool reset = false;
  const char prefix[] = "raw_";
  const char ext[] = ".ubx";
  size_t number = 1;
  while (number != 0) {
    if (number > max_nums) {
      number = 1;
      reset = true;
    }
    sprintf(name, "%s%i%s", prefix, number, ext);
    if (SD.exists(name) && !reset) {
      // file exist
      number++;
    } else {
      // next number
      return;
    }
  }
}