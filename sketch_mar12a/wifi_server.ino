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
