/*
  ======================================================
  COLOUR DUEL - VERSION 4
  White Preparation and Refined Gameplay
  ======================================================

  Module: CASA0014 - Prototyping the Internet of Things
  Board: Arduino MKR WiFi 1010

  INPUT:
  Red rotary sensor   -> A0
  Green rotary sensor -> A1
  Blue rotary sensor  -> A2
  Push button         -> Digital Pin 2 + GND

  OUTPUT:
  Vespera / Lumi (72 RGB LEDs)

  FEATURES:
  - Free RGB colour exploration
  - Real-time RGB adjustment
  - Moving average sensor smoothing
  - Physical button interaction
  - Random target colour generation
  - Target colour displayed for 3 seconds
  - Two-player colour matching game
  - White preparation display for both players
  - 1.5-second white preparation period
  - Player 1 controls the left 36 LEDs
  - Player 2 controls the right 36 LEDs
  - Single-click player colour saving
  - Automatic weighted RGB colour comparison
  - Automatic winner detection
  - Winner displayed after a 1-second delay
  - Winning player's LEDs remain on for 3 seconds
  - Automatic reset to free RGB mixing
  - WiFi + MQTT communication

  DESCRIPTION:
  This version improves the player transition
  experience by introducing a white preparation
  display before each player's turn.

  Player 1 and Player 2 each begin with a
  1.5-second white LED display before entering
  real-time RGB adjustment.

  This provides a consistent visual starting
  point and reduces the immediate influence
  of the previous player's displayed colour.

  After both players save their colours, the
  system automatically evaluates the results,
  displays the winner and returns to free
  RGB exploration mode.

  DESIGN NOTE:
  The white preparation stage resets the LED
  display visually, but does not physically
  reset the rotary sensors.

  Players may manually return the rotary
  controls to a common starting position
  for a more consistent experience.

  MQTT:
  Broker: mqtt.cetools.org
  Port: 1884
  Topic: student/CASA0014/luminaire/24

  SERIAL BAUD RATE: 9600
*/


#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

// ======================================================
// 1. HARDWARE PINS
// ======================================================

const int RED_PIN = A0;
const int GREEN_PIN = A1;
const int BLUE_PIN = A2;
const int BUTTON_PIN = 2;

// ======================================================
// 2. WIFI / MQTT
// ======================================================

const char* ssid = SECRET_SSID;
const char* wifiPassword = SECRET_PASS;
const char* mqttUser = SECRET_MQTTUSER;
const char* mqttPassword = SECRET_MQTTPASS;

const char* mqttServer = "mqtt.cetools.org";
const int mqttPort = 1884;

const char* mqttTopic =
  "student/CASA0014/luminaire/24";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ======================================================
// 3. RGB DATA
// ======================================================

struct RGBColor {
  int r;
  int g;
  int b;
};

const RGBColor BLACK = {0, 0, 0};
const RGBColor WHITE = {255, 255, 255};

RGBColor currentColor = {0, 0, 0};
RGBColor targetColor = {0, 0, 0};
RGBColor player1Color = {0, 0, 0};
RGBColor player2Color = {0, 0, 0};

int winner = 0;

// ======================================================
// 4. LED CONFIGURATION
// ======================================================

const int NUM_LEDS = 72;
const int HALF_LEDS = 36;
const int PAYLOAD_SIZE = NUM_LEDS * 3;

byte RGBpayload[PAYLOAD_SIZE];

// ======================================================
// 5. GAME STATES
// ======================================================

enum GameState {
  FREE_MIXING,
  SHOW_TARGET,
  PLAYER_1_READY,
  PLAYER_1,
  PLAYER_2_READY,
  PLAYER_2,
  JUDGING,
  SHOW_WINNER
};

GameState gameState = FREE_MIXING;

unsigned long stateStartTime = 0;

// ======================================================
// 6. TIMING
// ======================================================

const unsigned long TARGET_DURATION = 3000;
const unsigned long READY_DURATION = 1500;
const unsigned long JUDGE_DURATION = 1000;
const unsigned long WINNER_DURATION = 3000;

const unsigned long RGB_PRINT_INTERVAL = 500;
const unsigned long PUBLISH_INTERVAL = 150;

const unsigned long DEBOUNCE_DELAY = 40;

const unsigned long WIFI_RETRY_INTERVAL = 10000;
const unsigned long MQTT_RETRY_INTERVAL = 5000;

unsigned long lastRGBPrintTime = 0;
unsigned long lastPublishTime = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;

// ======================================================
// 7. BUTTON VARIABLES
// ======================================================

bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;

unsigned long lastDebounceTime = 0;

// ======================================================
// 8. MOVING AVERAGE
// ======================================================

const int SAMPLE_COUNT = 10;

int redSamples[SAMPLE_COUNT] = {0};
int greenSamples[SAMPLE_COUNT] = {0};
int blueSamples[SAMPLE_COUNT] = {0};

int sampleIndex = 0;

void updateSensorSamples() {

  redSamples[sampleIndex] = analogRead(RED_PIN);
  greenSamples[sampleIndex] = analogRead(GREEN_PIN);
  blueSamples[sampleIndex] = analogRead(BLUE_PIN);

  sampleIndex = (sampleIndex + 1) % SAMPLE_COUNT;
}

int calculateAverage(int samples[]) {

  long total = 0;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    total += samples[i];
  }

  return total / SAMPLE_COUNT;
}

RGBColor readRotaryRGB() {

  updateSensorSamples();

  RGBColor c;

  c.r = constrain(
    map(calculateAverage(redSamples),
        0, 1023, 0, 255),
    0, 255
  );

  c.g = constrain(
    map(calculateAverage(greenSamples),
        0, 1023, 0, 255),
    0, 255
  );

  c.b = constrain(
    map(calculateAverage(blueSamples),
        0, 1023, 0, 255),
    0, 255
  );

  return c;
}

// ======================================================
// 9. SERIAL RGB OUTPUT
// ======================================================

void printRGBValues() {

  unsigned long now = millis();

  if (
    now - lastRGBPrintTime <
    RGB_PRINT_INTERVAL
  ) {
    return;
  }

  lastRGBPrintTime = now;

  Serial.print("RGB: ");
  Serial.print(currentColor.r);
  Serial.print(", ");
  Serial.print(currentColor.g);
  Serial.print(", ");
  Serial.println(currentColor.b);
}

// ======================================================
// 10. LED CONTROL
// ======================================================

void setPixel(int pixel, RGBColor c) {

  if (pixel < 0 || pixel >= NUM_LEDS) {
    return;
  }

  RGBpayload[pixel * 3] = (byte)c.r;
  RGBpayload[pixel * 3 + 1] = (byte)c.g;
  RGBpayload[pixel * 3 + 2] = (byte)c.b;
}

void fillAll(RGBColor c) {

  for (int i = 0; i < NUM_LEDS; i++) {
    setPixel(i, c);
  }
}

void fillLeft(RGBColor c) {

  for (int i = 0; i < HALF_LEDS; i++) {
    setPixel(i, c);
  }
}

void fillRight(RGBColor c) {

  for (int i = HALF_LEDS; i < NUM_LEDS; i++) {
    setPixel(i, c);
  }
}

// ======================================================
// 11. LED DISPLAY BY GAME STATE
// ======================================================

void updateLEDDisplay() {

  fillAll(BLACK);

  switch (gameState) {

    case FREE_MIXING:
      fillAll(currentColor);
      break;

    case SHOW_TARGET:
      fillAll(targetColor);
      break;

    case PLAYER_1_READY:
      fillLeft(WHITE);
      break;

    case PLAYER_1:
      fillLeft(currentColor);
      break;

    case PLAYER_2_READY:
      fillRight(WHITE);
      break;

    case PLAYER_2:
      fillRight(currentColor);
      break;

    case JUDGING:
      // All LEDs off for one second.
      break;

    case SHOW_WINNER:

      if (winner == 1) {
        fillLeft(player1Color);

      } else if (winner == 2) {
        fillRight(player2Color);

      } else {
        // Draw: both sides light up.
        fillLeft(player1Color);
        fillRight(player2Color);
      }

      break;
  }
}

// ======================================================
// 12. WIFI CONNECTION
// ======================================================

void connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, wifiPassword);
}

// ======================================================
// 13. MQTT CONNECTION
// ======================================================

void connectMQTT() {

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (client.connected()) {
    return;
  }

  String clientId = "RGB-Duel-";
  clientId += String((unsigned long)millis(), HEX);

  if (client.connect(
        clientId.c_str(),
        mqttUser,
        mqttPassword
      )) {

    Serial.println("MQTT Connected!");

    lastPublishTime =
      millis() - PUBLISH_INTERVAL;

  } else {

    Serial.print("MQTT connection failed: ");
    Serial.println(client.state());
  }
}

// ======================================================
// 14. MQTT PUBLISH
// ======================================================

void publishRGB() {

  if (!client.connected()) {
    return;
  }

  updateLEDDisplay();

  bool success = client.publish(
    mqttTopic,
    RGBpayload,
    (unsigned int)PAYLOAD_SIZE
  );

  if (!success) {
    Serial.println("MQTT PUBLISH FAILED!");
  }
}

// ======================================================
// 15. WEIGHTED RGB DISTANCE
// ======================================================

long calculateColourDistance(
  RGBColor player,
  RGBColor target
) {

  long dr = (long)player.r - target.r;
  long dg = (long)player.g - target.g;
  long db = (long)player.b - target.b;

  return
    299L * dr * dr +
    587L * dg * dg +
    114L * db * db;
}

// ======================================================
// 16. JUDGE GAME
// ======================================================

void judgeGame() {

  long score1 = calculateColourDistance(
    player1Color,
    targetColor
  );

  long score2 = calculateColourDistance(
    player2Color,
    targetColor
  );

  Serial.println();
  Serial.println("========== RESULTS ==========");

  Serial.print("Target RGB: ");
  Serial.print(targetColor.r);
  Serial.print(", ");
  Serial.print(targetColor.g);
  Serial.print(", ");
  Serial.println(targetColor.b);

  Serial.print("Player 1 RGB: ");
  Serial.print(player1Color.r);
  Serial.print(", ");
  Serial.print(player1Color.g);
  Serial.print(", ");
  Serial.println(player1Color.b);

  Serial.print("Player 2 RGB: ");
  Serial.print(player2Color.r);
  Serial.print(", ");
  Serial.print(player2Color.g);
  Serial.print(", ");
  Serial.println(player2Color.b);

  Serial.print("Player 1 weighted distance: ");
  Serial.println(score1);

  Serial.print("Player 2 weighted distance: ");
  Serial.println(score2);

  if (score1 < score2) {

    winner = 1;
    Serial.println("PLAYER 1 WINS!");

  } else if (score2 < score1) {

    winner = 2;
    Serial.println("PLAYER 2 WINS!");

  } else {

    winner = 0;
    Serial.println("DRAW!");
  }

  Serial.println("=============================");
}

// ======================================================
// 17. CHANGE GAME STATE
// ======================================================

void changeGameState(GameState newState) {

  gameState = newState;
  stateStartTime = millis();

  // Force a new LED frame immediately.
  lastPublishTime =
    millis() - PUBLISH_INTERVAL;
}

// ======================================================
// 18. START NEW GAME
// ======================================================

void startGame() {

  targetColor.r = random(40, 256);
  targetColor.g = random(40, 256);
  targetColor.b = random(40, 256);

  player1Color = BLACK;
  player2Color = BLACK;

  winner = 0;

  Serial.println();
  Serial.println("========== NEW GAME ==========");

  Serial.print("Target RGB: ");
  Serial.print(targetColor.r);
  Serial.print(", ");
  Serial.print(targetColor.g);
  Serial.print(", ");
  Serial.println(targetColor.b);

  Serial.println("Memorise the target colour!");

  changeGameState(SHOW_TARGET);
}

// ======================================================
// 19. BUTTON ACTION
// ======================================================

void handleButtonClick() {

  switch (gameState) {

    case FREE_MIXING:

      Serial.println("BUTTON: START GAME");
      startGame();
      break;

    case PLAYER_1:

      player1Color = currentColor;

      Serial.print("PLAYER 1 SAVED: ");
      Serial.print(player1Color.r);
      Serial.print(", ");
      Serial.print(player1Color.g);
      Serial.print(", ");
      Serial.println(player1Color.b);

      Serial.println("PLAYER 2 WHITE READY");

      changeGameState(PLAYER_2_READY);
      break;

    case PLAYER_2:

      player2Color = currentColor;

      Serial.print("PLAYER 2 SAVED: ");
      Serial.print(player2Color.r);
      Serial.print(", ");
      Serial.print(player2Color.g);
      Serial.print(", ");
      Serial.println(player2Color.b);

      Serial.println("BOTH PLAYERS SAVED");
      Serial.println("JUDGING IN 1 SECOND...");

      changeGameState(JUDGING);
      break;

    default:
      // Ignore clicks during transitions.
      break;
  }
}

// ======================================================
// 20. BUTTON DEBOUNCING
// ======================================================

void updateButton() {

  unsigned long now = millis();

  bool rawState = digitalRead(BUTTON_PIN);

  if (rawState != lastRawButtonState) {
    lastDebounceTime = now;
  }

  if (now - lastDebounceTime >= DEBOUNCE_DELAY) {

    if (rawState != stableButtonState) {

      stableButtonState = rawState;

      // Detect click when released.
      if (stableButtonState == HIGH) {
        handleButtonClick();
      }
    }
  }

  lastRawButtonState = rawState;
}

// ======================================================
// 21. AUTOMATIC GAME TIMING
// ======================================================

void updateGameTiming() {

  unsigned long elapsed =
    millis() - stateStartTime;

  switch (gameState) {

    case SHOW_TARGET:

      if (elapsed >= TARGET_DURATION) {

        Serial.println("TARGET DISPLAY FINISHED");
        Serial.println("PLAYER 1 WHITE READY");

        changeGameState(PLAYER_1_READY);
      }
      break;

    case PLAYER_1_READY:

      if (elapsed >= READY_DURATION) {

        Serial.println("PLAYER 1 START!");
        Serial.println("Adjust RGB and press to save.");

        changeGameState(PLAYER_1);
      }
      break;

    case PLAYER_2_READY:

      if (elapsed >= READY_DURATION) {

        Serial.println("PLAYER 2 START!");
        Serial.println("Adjust RGB and press to save.");

        changeGameState(PLAYER_2);
      }
      break;

    case JUDGING:

      if (elapsed >= JUDGE_DURATION) {

        judgeGame();

        Serial.println(
          "SHOWING WINNER FOR 3 SECONDS"
        );

        changeGameState(SHOW_WINNER);
      }
      break;

    case SHOW_WINNER:

      if (elapsed >= WINNER_DURATION) {

        Serial.println("GAME FINISHED");
        Serial.println("RESET TO FREE RGB MIXING");

        winner = 0;
        player1Color = BLACK;
        player2Color = BLACK;

        changeGameState(FREE_MIXING);
      }
      break;

    default:
      break;
  }
}

// ======================================================
// 22. SETUP
// ======================================================

void setup() {

  Serial.begin(9600);

  unsigned long serialStart = millis();

  while (
    !Serial &&
    millis() - serialStart < 3000
  ) {
    delay(10);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  analogReadResolution(10);

  // Initialise moving-average buffers.
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    updateSensorSamples();
    delay(10);
  }

  randomSeed(analogRead(A0) ^ micros());

  // 72 RGB LEDs = 216 bytes.
  client.setBufferSize(512);

  client.setServer(
    mqttServer,
    mqttPort
  );

  connectWiFi();
  lastWiFiAttempt = millis();

  changeGameState(FREE_MIXING);

  Serial.println();
  Serial.println("RGB COLOUR DUEL V6 READY");
  Serial.println("FREE RGB MIXING ACTIVE");
  Serial.println("Press button to start game.");
}

// ======================================================
// 23. MAIN LOOP
// ======================================================

void loop() {

  unsigned long now = millis();

  // 1. Read three rotary sensors.
  currentColor = readRotaryRGB();

  // 2. Print RGB values every 500 ms.
  printRGBValues();

  // 3. Detect physical button.
  updateButton();

  // 4. Update automatic game states.
  updateGameTiming();

  // 5. Maintain Wi-Fi connection.
  if (WiFi.status() != WL_CONNECTED) {

    if (
      now - lastWiFiAttempt >=
      WIFI_RETRY_INTERVAL
    ) {

      lastWiFiAttempt = now;
      connectWiFi();
    }

  } else {

    // 6. Maintain MQTT connection.
    if (!client.connected()) {

      if (
        now - lastMQTTAttempt >=
        MQTT_RETRY_INTERVAL
      ) {

        lastMQTTAttempt = now;
        connectMQTT();
      }

    } else {

      client.loop();
    }
  }

  // 7. Publish LED data to Vespera.
  if (
    now - lastPublishTime >=
    PUBLISH_INTERVAL
  ) {

    lastPublishTime = now;
    publishRGB();
  }

  delay(10);
}
