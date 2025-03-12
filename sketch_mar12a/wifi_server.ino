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
