String toStringIp(IPAddress ip);
boolean isIp(String str);

void loadCredentials(char ssid[32], char password[32]);
void saveCredentials(char ssid[32], char password[32]);

WebServer::WebServer(TelnetServer _telnetServer)
    : ssid{""}, password{""}, telnetServer{_telnetServer} {}

WebServer::~WebServer() {}

void WebServer::setup() {
  delay(1000);
  // logger.println();
  // logger.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);
  delay(500);  // Without delay I've seen the IP address blank
  logger.print("AP IP address: ");
  logger.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
}

/** Handle root or redirect to captive portal */
void WebServer::handleRoot() {
  if (captivePortal()) {  // If caprive portal redirect instead of displaying
                          // the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page +=
      F("<html><head></head><body>"
        "<h1>HELLO WORLD!!</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) +
            softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) +
            ssid + F("</p>");
  }
  Page +=
      F("<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>"
        "</body></html>");

  server.send(200, "text/html", Page);
}

/** Redirect to captive portal if we got a request for another domain. Return
 * true in that case so the page handler do not try to handle the request again.
 */
boolean WebServer::captivePortal() {
  if (!isIp(server.hostHeader()) &&
      server.hostHeader() != (String(myHostname) + ".local")) {
    logger.println("Request redirected to captive portal");
    server.sendHeader("Location",
                      String("http://") + toStringIp(server.client().localIP()),
                      true);
    server.send(302, "text/plain",
                "");  // Empty content inhibits Content-length header so we have
                      // to close the socket ourselves.
    server.client().stop();  // Stop is needed because we sent no content length
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
  Page +=
      F("<html><head></head><body>"
        "<h1>Wifi config</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) +
            softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) +
            ssid + F("</p>");
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
            "<table><tr><th align='left'>WLAN list (refresh if any "
            "missing)</th></tr>");
  logger.println("scan start");
  int n = WiFi.scanNetworks();
  logger.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) +
              ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) +
              F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  Page +=
      F("</table>"
        "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to "
        "network:</h4>"
        "<input type='text' placeholder='network' name='n'/>"
        "<br /><input type='password' placeholder='password' name='p'/>"
        "<br /><input type='submit' value='Connect/Disconnect'/></form>"
        "<p>You may want to <a href='/'>return to the home page</a>.</p>"
        "</body></html>");
  server.send(200, "text/html", Page);
  server.client().stop();  // Stop is needed because we sent no content length
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
              "");  // Empty content inhibits Content-length header so we have
                    // to close the socket ourselves.
  server.client().stop();  // Stop is needed because we sent no content length
  saveCredentials(ssid, password);
  connect = strlen(ssid) >
            0;  // Request WLAN connect with new credentials if there is a SSID
}