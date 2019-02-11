String toStringIp(IPAddress ip);
boolean isIp(String str);

// void loadCredentials(char ssid[], char password[]);
// void saveCredentials(char ssid[], char password[]);

WebServer::WebServer(TelnetServer _telnetServer) : ssid{""}, password{""}, gpsStarted{'0'}, telnetServer{_telnetServer} {
	meta = F(				  
			"<meta charset=\"utf-8\">"
				  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
}

WebServer::~WebServer() {}

void WebServer::setup() {
		delay(1000);

		telnetServer.setup();
		// logger.println();
		// logger.println("Configuring access point...");
		/* You can remove the password parameter if you want the AP to be open.
		 */
		WiFi.softAPConfig(apIP, apIP, netMsk);
		WiFi.softAP(softAP_ssid, softAP_password);
		delay(500); // Without delay I've seen the IP address blank
		logger.print("AP IP address: ");
		logger.println(WiFi.softAPIP());

		/* Setup the DNS server redirecting all the domains to the apIP */
		dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
		dnsServer.start(DNS_PORT, "*", apIP);

		/* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
		server.on("/", std::bind(&WebServer::handleRoot, this));
		server.on("/wifi", std::bind(&WebServer::handleWifi, this));
		server.on("/wifisave", std::bind(&WebServer::handleWifiSave, this));
		server.on("/gps", std::bind(&WebServer::handleStartGPS,this));
		server.on("/generate_204", std::bind(&WebServer::handleRoot, this)); // Android captive portal. Maybe not needed. Might be handled by notFound handler.
		server.on("/fwlink", std::bind(&WebServer::handleRoot, this));		 // Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
		server.onNotFound(std::bind(&WebServer::handleNotFound, this));
		server.begin(); // Web server start
		Serial.println("HTTP server started");

		logger.debug("SSID size %i\n", sizeof(ssid));
		logger.debug("SSID %s\n", ssid);
		loadCredentials(/*ssid, password*/); // Load WLAN credentials from network
		connect = strlen(ssid) > 0;			 // Request WLAN connect if there is a SSID

		
}

void WebServer::process() {
		if (connect) {
				Serial.println("Connect requested");
				connect = false;
				connectWifi();
				lastConnectTry = millis();
		}
		{
				unsigned int s = WiFi.status();
				if (s == 0 && millis() > (lastConnectTry + 60000)) {
						/* If WLAN disconnected and idle try to connect */
						/* Don't set retry time too low as retry interfere the softAP operation */
						connect = true;
				}
				if (status != s) { // WLAN status change
						Serial.print("Status: ");
						Serial.println(s);
						status = s;
						if (s == WL_CONNECTED) {
								/* Just connected to WLAN */
								Serial.println("");
								Serial.print("Connected to ");
								Serial.println(ssid);
								Serial.print("IP address: ");
								Serial.println(WiFi.localIP());

								// Setup MDNS responder
								if (!MDNS.begin(myHostname)) {
										Serial.println("Error setting up MDNS responder!");
								} else {
										Serial.println("mDNS responder started");
										// Add service to MDNS-SD
										MDNS.addService("http", "tcp", 80);
								}
						} else if (s == WL_NO_SSID_AVAIL) {
								WiFi.disconnect();
						}
				}
				if (s == WL_CONNECTED) {
						MDNS.update();
				}
		}
		// Do work:
		// DNS
		dnsServer.processNextRequest();
		// HTTP
		server.handleClient();

		if (gpsStarted[0] == '1'){
			telnetServer.process();
		}
		
}

void WebServer::connectWifi() {
		Serial.println("Connecting as wifi client...");
		WiFi.disconnect();
		WiFi.begin(ssid, password);
		int connRes = WiFi.waitForConnectResult();
		Serial.print("connRes: ");
		Serial.println(connRes);
}

/** Handle root or redirect to captive portal */
void WebServer::handleRoot() {
		if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
				return;
		}
		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.sendHeader("Pragma", "no-cache");
		server.sendHeader("Expires", "-1");
		
		String Script = String(F(
				"<script type=\"text/javascript\">\n"
				  "function startGps() {\n"
					  "var xhr = new XMLHttpRequest();\n"
					  "xhr.open('POST', '/gps', true);\n"
					  //"xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');\n\n"
					  
					  "xhr.onreadystatechange = function() {\n"
						  "if(xhr.readyState == XMLHttpRequest.DONE && xhr.status == 200) {\n"
						      "var resp = xhr.responseText.split(String.fromCharCode(10));"
						      "console.log({resp});\n"
							  "button.innerHTML = resp[0];\n"
						  "}\n"

					  "}\n"
					  "var formData = new FormData();\n" 
					  "var val = button.innerHTML == '")) + GPS_START_BTN + F("' ? 1 : 0;\n"
					  "formData.append('gps', val);\n"
					  "xhr.send(formData);\n"
					  "console.log({formData}, {xhr});\n"
				  "}\n\n"
				  "button.innerHTML = '") + (gpsStarted[0] == '0' ? GPS_START_BTN : GPS_STOP_BTN) +  
				  F("';\n"
					  "</script>\n");

		String Page;
		Page += String(F("<html>"
				  "<head>")) + 
				  meta +
				  String(F("</head>"
				  "<body>"
				  "<h1>ESP GPS</h1>"));
		if (server.client().localIP() == apIP) {
				Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
		} else {
				Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
		}

		Page += String(F("<div>"
						 "<button id=\"button\" onclick=\"startGps()\">"
					"</button>" 
					"</div>"));

		Page += String(F("<p>You may want to <a href='/wifi'>config the wifi "
				  "connection</a>.</p>")) +
				  Script +
				  String(F("</body></html>"));

		server.send(200, "text/html", Page);
}

/** Redirect to captive portal if we got a request for another domain. Return
 * true in that case so the page handler do not try to handle the request again.
 */
boolean WebServer::captivePortal() {
		if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
				logger.println("Request redirected to captive portal");
				server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
				server.send(302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
				server.client().stop();				// Stop is needed because we sent no content length
				return true;
		}
		return false;
}

/** Wifi config page handler */
void WebServer::handleWifi() {
		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.sendHeader("Pragma", "no-cache");
		server.sendHeader("Expires", "-1");		

		String Page;
		Page += F("<html><head></head><body>"
				  "<h1>Wifi config</h1>");
		if (server.client().localIP() == apIP) {
				Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
		} else {
				Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
		}
		Page += String(F("\r\n<br />"
						 "<table><tr><th align='left'>SoftAP config</th></tr>"
						 "<tr><td>SSID ")) +
				String(softAP_ssid) +
				F("</td></tr>"
				  "<tr><td>IP ") +
				toStringIp(WiFi.softAPIP()) +
				F("</td></tr>"
				  "</table>"
				  "\r\n<br />"
				  "<table><tr><th align='left'>WLAN config</th></tr>"
				  "<tr><td>SSID ") +
				String(ssid) +
				F("</td></tr>"
				  "<tr><td>IP ") +
				toStringIp(WiFi.localIP()) +
				F("</td></tr>"
				  "</table>"
				  "\r\n<br />"
				  "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");

		logger.println("scan start");
		int n = WiFi.scanNetworks();
		logger.println("scan done");
		if (n > 0) {
				for (int i = 0; i < n; i++) {
						Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
				}
		} else {
				Page += F("<tr><td>No WLAN found</td></tr>");
		}
		Page += F("</table>"
				  "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to "
				  "network:</h4>"
				  "<input type='text' placeholder='network' name='n'/>"
				  "<br /><input type='password' placeholder='password' name='p'/>"
				  "<br /><input type='submit' value='Connect/Disconnect'/></form>"
				  "<p>You may want to <a href='/'>return to the home page</a>.</p>"
				  "</body></html>");
		server.send(200, "text/html", Page);
		server.client().stop(); // Stop is needed because we sent no content length
}

void WebServer::handleStartGPS() {
		logger.debug("gps start\n");
		logger.debug("gpsStarted = %c\n", gpsStarted[0]);
		logger.debug("Argument count %i\n", server.args());
		if (server.args() == 0){
			return returnFail("BAD ARGS");
		}
		logger.debug("Server has gps atr %s\n)", server.hasArg("gps") ? "true" : "false");
		logger.debug("Argument (0) name : %s, argument gps value : %s\n", server.argName(0).c_str(), server.arg("gps").c_str());
		memcpy(gpsStarted, server.arg("gps").c_str(), sizeof(gpsStarted));
		logger.debug("arg g: %c\n", gpsStarted[0]);
		String res  = gpsStarted[0] == '1' ? String(GPS_STOP_BTN) + String(F("\nGPS Started")) : String(GPS_START_BTN) + String(F("\nGPS not started"));
		server.send(200, "text/plain", res);
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void WebServer::handleWifiSave() {
		logger.println("wifi save");
		server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
		server.arg("p").toCharArray(password, sizeof(password) - 1);
		server.sendHeader("Location", "wifi", true);
		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.sendHeader("Pragma", "no-cache");
		server.sendHeader("Expires", "-1");
		server.send(302, "text/plain",
					"");		// Empty content inhibits Content-length header so we
								// have to close the socket ourselves.
		server.client().stop(); // Stop is needed because we sent no content length
		saveCredentials(/*ssid, password*/);
		connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void WebServer::handleNotFound() {
		if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
				return;
		}
		String message = F("File Not Found\n\n");
		message += F("URI: ");
		message += server.uri();
		message += F("\nMethod: ");
		message += (server.method() == HTTP_GET) ? "GET" : "POST";
		message += F("\nArguments: ");
		message += server.args();
		message += F("\n");

		for (uint8_t i = 0; i < server.args(); i++) {
				message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
		}
		server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
		server.sendHeader("Pragma", "no-cache");
		server.sendHeader("Expires", "-1");
		server.send(404, "text/plain", message);
}

void WebServer::returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}
