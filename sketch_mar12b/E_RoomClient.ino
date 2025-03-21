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

 if (gameIsOn == true) {
    switch (currentPuzzle) {
      case 0:
        pinMode(A0, INPUT);
        pinMode(MUX_A, OUTPUT);
        pinMode(MUX_B, OUTPUT);
        pinMode(MUX_C, OUTPUT);
        pinMode(MUX_IN, INPUT);
        digitalWrite(A0, LOW);
        handleLightPuzzle();
        break;
      case 1:
        dht.begin();
        pinMode(FAN_PIN, OUTPUT);
        digitalWrite(FAN_PIN, LOW);
        handleTemperaturePuzzle();
        break;
      case 2:
        pinMode(RED_BTN, INPUT_PULLUP);
        pinMode(GREEN_BTN, INPUT_PULLUP);
        pinMode(BLUE_BTN, INPUT_PULLUP);
        handleLEDSequencePuzzle();
        break;
      case 3:
        pinMode(MUX_A, OUTPUT);
        pinMode(MUX_B, OUTPUT);
        pinMode(MUX_C, OUTPUT);
        pinMode(MUX_IN, INPUT);
        digitalWrite(A0, LOW);
        handleJoystickChallenge();
        break;
    }
  }
}

int ReadMuxChannel(byte chnl) {
  int a = (bitRead(chnl, 0) > 0) ? HIGH : LOW;
  int b = (bitRead(chnl, 1) > 0) ? HIGH : LOW;
  int c = (bitRead(chnl, 2) > 0) ? HIGH : LOW;

  digitalWrite(MUX_A, a);
  digitalWrite(MUX_B, b);
  digitalWrite(MUX_C, c);

  int ret = analogRead(MUX_IN);
  return ret;
}

void handleLightPuzzle() {
  static unsigned long lastLightCheckMillis = 0;

  int lightLevel = ReadMuxChannel(0);

  if (samplingMaxLight) {
    if (lightLevel > maxLightLevel) {
      maxLightLevel = lightLevel;
    }

    if (currentMillis - samplingStartTime >= 5000) {
      samplingMaxLight = false;
    }
    return;
  }

  if (lightLevel > maxLightLevel) {
    maxLightLevel = lightLevel;
  }

Serial.print("Light Level: ");
  Serial.print(lightLevel);
  Serial.print(" Max Light Level: ");
  Serial.print(maxLightLevel);
  Serial.print(" Target Light Level (80%): ");
  Serial.println(maxLightLevel * 0.8);

  if (lightLevel <= maxLightLevel * 0.8) {
    if (!inCorrectLightRange) {
      inCorrectLightRange = true;
      lightCorrectStartTime = currentMillis;
      Serial.println("Entered correct light range");
    } else if (currentMillis - lightCorrectStartTime >= 2000) {
      Serial.println("Puzzle Solved! Light was in the correct range for 2 seconds");
      puzzleSolved(0);
      currentPuzzle++;
      inCorrectLightRange = false;
      maxLightLevel = 0;
    }
  } else {
    if (inCorrectLightRange) {
      Serial.println("Exited correct light range - Restarting");
      inCorrectLightRange = false;
    }
  }
}

void handleTemperaturePuzzle() {
  pinMode(FAN_PIN, OUTPUT);

  if (currentMillis - lastTempCheckMillis >= 2000) {
    lastTempCheckMillis = currentMillis;

    float t = dht.readTemperature();

    currentTemp = t;
    Serial.print("Temperature = ");
    Serial.println(currentTemp);

    if (targetTemp == 0) {
      targetTemp = currentTemp - 0.1;
      Serial.print("Target Temperature: ");
      Serial.println(targetTemp);
    }

    if (currentTemp <= targetTemp) {
      if (!tempInCorrectRange) {
        tempInCorrectRange = true;
        tempCorrectStartTime = currentMillis;
        Serial.println("Reached target temperature!");
      } else if (currentMillis - tempCorrectStartTime >= 2000) {
        Serial.println("Puzzle Solved! Temperature dropped enough for 2 seconds");
        puzzleSolved(1);
        digitalWrite(FAN_PIN, HIGH);
        currentPuzzle++;
        tempPuzzleSolved = true;
      }
    } else {
      if (tempInCorrectRange) {
        Serial.println("Temperature rose above target - Restarting");
        tempInCorrectRange = false;
      }
    }
  }
}

void generateRandomSequence() {
  long randSeed = millis() * analogRead(A0);
  randomSeed(randSeed);

  for (int i = 0; i < sequenceLength; i++) {
    sequence[i] = random(0, 3);
  }

  Serial.println("New random sequence:");
  for (int i = 0; i < sequenceLength; i++) {
    Serial.print(sequence[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void handleLEDSequencePuzzle() {
  if (showingStartSequence) {
    handleStartSequence();
    return;
  }

  if (isBlinking) {
    handleErrorBlink();
    return;
  }

  if (showingVictory) {
    handleVictoryAnimation();
    return;
  }

  if (waitingForButtonRelease) {
    if (digitalRead(buttons[lastPressedButton]) == HIGH) {
      digitalWrite(leds[lastPressedButton], LOW);
      waitingForButtonRelease = false;
      buttonReleaseTime = millis();
    }
    return;
  }

  if (lastPressedButton != -1 && millis() - buttonReleaseTime < 200) {
    return;
  }

  lastPressedButton = -1;

  if (displayingSequence) {
    unsigned long currentTime = millis();
    if (currentTime - lastChangeTime > 400) {
      lastChangeTime = currentTime;

digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, LOW);

      if (displayState == 0) {
        if (currentStep < sequenceLength) {
          digitalWrite(leds[sequence[currentStep]], HIGH);
        }
        displayState = 1;
      } else {
        currentStep++;
        displayState = 0;

        if (currentStep >= sequenceLength) {
          displayingSequence = false;
          currentStep = 0;
        }
      }
    }
  } else {
    for (int i = 0; i < 3; i++) {
      if (digitalRead(buttons[i]) == LOW) {
        digitalWrite(leds[i], HIGH);
        lastPressedButton = i;
        waitingForButtonRelease = true;

        if (i == sequence[currentStep]) {
          currentStep++;
          if (currentStep >= sequenceLength) {
            Serial.println("Puzzle Solved!");
            showingVictory = true;
            victoryStep = 0;
            victoryTime = millis();
            puzzleSolved(2);
          }
        } else {
          generateRandomSequence();
          isBlinking = true;
          blinkCount = 0;
          blinkState = 0;
          blinkTime = millis();
        }

        break;
      }
    }
  }
}

void handleStartSequence() {
  if (millis() - startSequenceTime > 300) {
    startSequenceTime = millis();

    if (blinkState == 0) {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(BLUE_LED, HIGH);
      blinkState = 1;
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, LOW);
      blinkState = 0;
      startSequenceCount++;

      if (startSequenceCount >= 3) {
        showingStartSequence = false;
        displayingSequence = true;
        currentStep = 0;
        displayState = 0;
        lastChangeTime = millis();
      }
    }
  }
}

void handleErrorBlink() {
  if (millis() - blinkTime > 200) {
    blinkTime = millis();

    if (blinkState == 0) {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(BLUE_LED, HIGH);
      blinkState = 1;
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, LOW);
      blinkState = 0;
      blinkCount++;

      if (blinkCount >= 3) {
        isBlinking = false;
        currentStep = 0;
        displayingSequence = true;
        displayState = 0;
        lastChangeTime = millis();
      }
    }
  }
}

void handleVictoryAnimation() {
  if (millis() - victoryTime > 150) {
    victoryTime = millis();

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);

    if (victoryStep < 9) {
      int ledIndex = victoryStep % 3;
      digitalWrite(leds[ledIndex], HIGH);
    }

    victoryStep++;

    if (victoryStep >= 9) {
      showingVictory = false;

      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, LOW);

      generateRandomSequence();

      showingStartSequence = true;
      startSequenceCount = 0;
      startSequenceTime = millis();

      currentPuzzle++;
    }
  }
}

void handleJoystickChallenge() {
  static unsigned long lastPrintTime = 0;
  unsigned long currentTime = millis();

  static unsigned long lastJoystickReadTime = 0;
  if (currentTime - lastJoystickReadTime >= 50) {
    lastJoystickReadTime = currentTime;

    joystickX = ReadMuxChannel(4);
    joystickY = ReadMuxChannel(5);
  }

  if (currentTime - lastPrintTime >= 1000) {
    lastPrintTime = currentTime;
    Serial.print("X: ");
    Serial.print(joystickX);
    Serial.print(" Y: ");
    Serial.println(joystickY);
  }

  if (isInCorner()) {
    if (!inCornerPosition) {
      inCornerPosition = true;
      cornerStartTime = currentTime;
      Serial.println("Joystick in corner! Hold for 2 seconds");
    } else if (currentTime - cornerStartTime >= CORNER_TIMEOUT) {
      if (!joystickChallengeSolved) {
        joystickChallengeSolved = true;
        inCornerPosition = false;
        Serial.println("Puzzle Solved! Joystick was in corner long enough");
        currentPuzzle++;
        puzzleSolved(3);
        gameOver = true;
      }
    }
  } else {
    if (inCornerPosition) {
      inCornerPosition = false;
      Serial.println("Joystick left corner - Try again");
    }
  }
}

bool isInCorner() {
  bool topRight = (joystickX > CENTER_VALUE + CORNER_THRESHOLD) && (joystickY > CENTER_VALUE + CORNER_THRESHOLD);

  bool bottomRight = (joystickX > CENTER_VALUE + CORNER_THRESHOLD) && (joystickY < CENTER_VALUE - CORNER_THRESHOLD);

  bool topLeft = (joystickX < CENTER_VALUE - CORNER_THRESHOLD) && (joystickY > CENTER_VALUE + CORNER_THRESHOLD);

  bool bottomLeft = (joystickX < CENTER_VALUE - CORNER_THRESHOLD) && (joystickY < CENTER_VALUE - CORNER_THRESHOLD);

  return topRight || bottomRight || topLeft || bottomLeft;
}
