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

const int sequenceLength = 8;
int sequence[sequenceLength];
int leds[3] = { RED_LED, GREEN_LED, BLUE_LED };
int buttons[3] = { RED_BTN, GREEN_BTN, BLUE_BTN };
int currentStep = 0;
bool displayingSequence = true;
unsigned long lastChangeTime = 0;
int displayState = 0;

unsigned long buttonReleaseTime = 0;
bool waitingForButtonRelease = false;
int lastPressedButton = -1;

bool isBlinking = false;
int blinkCount = 0;
unsigned long blinkTime = 0;
int blinkState = 0;

bool showingVictory = false;
int victoryStep = 0;
unsigned long victoryTime = 0;

bool showingStartSequence = true;
int startSequenceCount = 0;
unsigned long startSequenceTime = 0;

bool joystickChallengeSolved = false;
unsigned long cornerStartTime = 0;
bool inCornerPosition = false;
const int CORNER_TIMEOUT = 2000;

int joystickX;
int joystickY;
const int CORNER_THRESHOLD = 200;
const int CENTER_VALUE = 512;

unsigned long lastMuxReadTime = 0;
unsigned long lastJoystickCheckTime = 0;
const int MUX_STABILIZE_TIME = 10;
bool muxReadPending = false;
byte pendingChannel = 0;
int muxReadResult = 0;

bool gameIsOn = false;
bool gameOver = false;
bool gameOverMessage = false;

void setup() {
  Serial.begin(9600);
  wifi_Setup();

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);

  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);
  pinMode(MUX_IN, INPUT);
  digitalWrite(A0, LOW);

  for (int i = 0; i < 10; i++) {
    analogRead(A0);
  }
  generateRandomSequence();

  samplingStartTime = millis();

  showingStartSequence = true;
  startSequenceCount = 0;
  startSequenceTime = millis();
}

void loop() {
  currentMillis = millis();
  static unsigned long statusPrintTimer = 0;
  if (!gameOver) {
    if (currentMillis - statusPrintTimer >= 5000) {
      statusPrintTimer = currentMillis;
      Serial.print("Current State: Puzzle ");
      Serial.println(currentPuzzle + 1);
    }
  } else {
    if (!gameOverMessage) {
      Serial.print("Game Over! Thank you for playing!");
      gameOverMessage = true;
    }
  }
  if (ReadMuxChannel(0) > 50) {
    if (!gameIsOn) {
      SendGameOn();
    }
    gameIsOn = true;
  }
  if (currentMillis - lastSensorReadMillis >= 1000) {
    lastSensorReadMillis = currentMillis;
  }
