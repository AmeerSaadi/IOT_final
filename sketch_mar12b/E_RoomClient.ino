#include <DHT_U.h>
#include <DHT.h>

#define FAN_PIN D4

#define DHTPIN D7
#define DHTTYPE DHT22

#define MUX_A D5
#define MUX_B D6
#define MUX_C D7
#define MUX_IN A0

#define RED_LED D1
#define GREEN_LED D2
#define BLUE_LED D3

#define RED_BTN D5
#define GREEN_BTN D6
#define BLUE_BTN D7

DHT dht(DHTPIN, DHTTYPE);

extern int secretCode[4];
void SendData(int digit);
void SendCodeDigit(int puzzleNumber);
void puzzleSolved(int puzzleNumber);
void wifi_Setup();
void SendGameOn();

int currentPuzzle = 0;
unsigned long currentMillis = 0;
unsigned long lastSensorReadMillis = 0;

unsigned long lightCorrectStartTime = 0;
int maxLightLevel = 0;
bool inCorrectLightRange = false;
bool samplingMaxLight = true;
unsigned long samplingStartTime = 0;

float currentTemp = 0;
float targetTemp = 0;
bool tempPuzzleSolved = false;
bool tempInCorrectRange = false;
unsigned long tempCorrectStartTime = 0;
static unsigned long lastTempCheckMillis = 0;
