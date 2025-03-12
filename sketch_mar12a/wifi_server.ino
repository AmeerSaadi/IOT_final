#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DIYables_4Digit7Segment_74HC595.h>

#define CLOCK_PIN D5  
#define LATCH_PIN D6 
#define DATA_PIN D7  

#define LOCK_PIN D4  

DIYables_4Digit7Segment_74HC595 display(CLOCK_PIN, LATCH_PIN, DATA_PIN);

IPAddress localIP(192, 168, 1, 1);

const char* networkSSID = "EscapeRoom";
const char* networkPassword = "12345678";

String secretCode = "";
unsigned long lastWifiCheck = 0;

ESP8266WebServer server(80);

void handleUnknownRequest() {
  String response = "Requested resource not found\n\n";
  response += "URI: ";
  response += server.uri();
  response += "\nMethod: ";
  response += (server.method() == HTTP_GET) ? "GET" : "POST";
  response += "\nArguments: ";
  response += server.args();
  response += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    response += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", response);
}

void handleCodeSubmission() {
  Serial.println("Code submission received");
  if(server.hasArg("unlock")){
    digitalWrite(LOCK_PIN, LOW);
  }
  if (server.hasArg("code") && secretCode.length() < 4) {
    if (server.arg("code").length() == 1) {
      secretCode = secretCode + server.arg("code");
      display.printInt(secretCode.toInt(), true);
      server.send(200, "text/plain", "Code accepted");
    }
    server.send(400, "text/plain", "Code must be exactly 1 character");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void WiFisetup() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(localIP, localIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(networkSSID, networkPassword);

  server.on("/", handleHomePage);
  server.on("/submit", handleCodeSubmission);
  server.onNotFound(handleUnknownRequest);

  Serial.print("Access Point IP: ");
  Serial.println(localIP);
  pinMode(LOCK_PIN, OUTPUT);
  lastWifiCheck = millis();
  display.clear();
  display.printInt(0, true);
  server.begin();
}

void WiFiloop() {
  if (millis() - lastWifiCheck >= 10) {
    lastWifiCheck = millis();
    server.handleClient();
  }
  display.loop();
}
