#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "EscapeRoom";
const char* pswd = "12345678";

WiFiClient client;
int server_port = 80;
int secretCode[4];
unsigned long lastWifiCheckTime = 0;

void generateRandomCode() {
  long seed = millis() * analogRead(A0) + micros();
  randomSeed(seed);
  Serial.print("New Secret Code: ");
  for (int i = 0; i < 4; i++) {
    secretCode[i] = random(0, 10); 
    Serial.print(secretCode[i]);
    if (i < 3) Serial.print("-");
  }
  Serial.println();
}

void setupWiFi() {
  Serial.println("Connecting to network...");
  WiFi.begin(ssid, pswd);
  lastWifiCheckTime = millis();
  Serial.println("Connected to the network!");
  generateRandomCode(); 
}

void sendGameStart() {
  HTTPClient http;
  String url = "http://192.168.1.1/api?gameOn";
  http.begin(client, url);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
}

void sendDigit(int digit) {
  Serial.print("Sending digit: " + String(digit));
  HTTPClient http;
  String url = "http://192.168.1.1/api?missionCode=" + String(digit);
  http.begin(client, url);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
}
void sendCodeDigit(int puzzleNumber) {
  if (puzzleNumber >= 0 && puzzleNumber < 4) {
    int digit = secretCode[puzzleNumber];
    sendDigit(digit); 
    Serial.print("Sending digit ");
    Serial.print(puzzleNumber + 1);
    Serial.print(" from the code: ");
    Serial.println(digit);
  }
}
void puzzleSolved(int puzzleNumber) {
  Serial.print("Puzzle number ");
  Serial.print(puzzleNumber + 1);
  Serial.println(" solved!");
  sendCodeDigit(puzzleNumber); 
}
void setup() {
  Serial.begin(9600);
  setupWiFi(); 
}

void loop() {}
