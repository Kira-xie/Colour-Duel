
/*
  ======================================================
  COLOUR DUEL - VERSION 2
  Two-Player Colour Matching Game
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
  - Free RGB colour mixing
  - Real-time RGB adjustment
  - Moving average sensor smoothing
  - Physical button interaction
  - Random target colour generation
  - Target colour displayed for 3 seconds
  - Two-player colour matching game
  - Player 1 controls the left 36 LEDs
  - Player 2 controls the right 36 LEDs
  - Button-controlled game progression
  - Player RGB colour saving
  - WiFi + MQTT communication

  DESCRIPTION:
  This version extends the basic RGB mixer into
  a two-player colour matching game.

  After observing a randomly generated target
  colour, two players take turns adjusting RGB
  values using the same physical rotary controls.

  Each player's colour is saved using the
  physical push button.

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
// 1. HARDWARE
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
// 3. RGB / LED DATA
// ======================================================

struct RGBColor {
  int r;
  int g;
  int b;
};

const RGBColor BLACK = {0, 0, 0};

RGBColor currentColor = {0, 0, 0};
RGBColor targetColor = {0, 0, 0};
RGBColor player1Color = {0, 0, 0};
RGBColor player2Color = {0, 0, 0};

const int NUM_LEDS = 72;
const int PAYLOAD_SIZE = NUM_LEDS * 3;

byte RGBpayload[PAYLOAD_SIZE];

int winner = 0;

// ======================================================
// 4. GAME STATES
// ======================================================

enum GameState {
  FREE_MIXING,
  SHOW_TARGET,
  PLAYER_1,
  PLAYER_2,
  WAIT_JUDGEMENT,
  JUDGING,
  SHOW_WINNER
};

GameState gameState = FREE_MIXING;

unsigned long stateStartTime = 0;

// ======================================================
// 5. TIMING
// ======================================================

const unsigned long TARGET_DURATION = 3000;
const unsigned long JUDGE_DURATION = 1000;
const unsigned long WINNER_DURATION = 3000;

const unsigned long RGB_PRINT_INTERVAL = 500;
const unsigned long PUBLISH_INTERVAL = 150;

const unsigned long DEBOUNCE_DELAY = 40;
const unsigned long DOUBLE_CLICK_WINDOW = 450;

const unsigned long WIFI_RETRY_INTERVAL = 10000;
const unsigned long MQTT_RETRY_INTERVAL = 5000;

unsigned long lastRGBPrintTime = 0;
unsigned long lastPublishTime = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;

// ======================================================
// 6. BUTTON VARIABLES
// ======================================================

bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;

unsigned long lastDebounceTime = 0;

int clickCount = 0;
unsigned long firstClickTime = 0;

// ======================================================
// 7. MOVING AVERAGE SENSOR FILTER
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

  sampleIndex =
    (sampleIndex + 1) % SAMPLE_COUNT;
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

  c.r = map(
    calculateAverage(redSamples),
    0, 1023, 0, 255
  );

  c.g = map(
    calculateAverage(greenSamples),
    0, 1023, 0, 255
  );

  c.b = map(
    calculateAverage(blueSamples),
    0, 1023, 0, 255
  );

  c.r = constrain(c.r, 0, 255);
  c.g = constrain(c.g, 0, 255);
  c.b = constrain(c.b, 0, 255);

  return c;
}

// ======================================================
// 8. SERIAL RGB OUTPUT
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
// 9. LED PIXEL FUNCTIONS
// ======================================================

void setPixel(int pixel, RGBColor c) {

  if (pixel < 0 || pixel >= NUM_LEDS) {
    return;
  }

  RGBpayload[pixel * 3] =
    (byte)c.r;

  RGBpayload[pixel * 3 + 1] =
    (byte)c.g;

  RGBpayload[pixel * 3 + 2] =
    (byte)c.b;
}

void fillAll(RGBColor c) {

  for (int i = 0; i < NUM_LEDS; i++) {
    setPixel(i, c);
  }
}

void fillLeft(RGBColor c) {

  for (int i = 0; i < 36; i++) {
    setPixel(i, c);
  }
}

void fillRight(RGBColor c) {

  for (int i = 36; i < 72; i++) {
    setPixel(i, c);
  }
}

void clearLEDs() {
  fillAll(BLACK);
}

// ======================================================
// 10. DISPLAY LED ACCORDING TO GAME STATE
// ======================================================

void updateLEDDisplay() {

  clearLEDs();

  switch (gameState) {

    case FREE_MIXING:

      fillAll(currentColor);
      break;

    case SHOW_TARGET:

      fillAll(targetColor);
      break;

    case PLAYER_1:

      fillLeft(currentColor);
      break;

    case PLAYER_2:

      fillRight(currentColor);
      break;

    case WAIT_JUDGEMENT:

      // Both answers saved.
      // LEDs remain off.
      break;

    case JUDGING:

      // One-second suspense.
      // LEDs remain off.
      break;

    case SHOW_WINNER:

      if (winner == 1) {

        fillLeft(player1Color);

      } else if (winner == 2) {

        fillRight(player2Color);

      } else {

        // Draw: both players light up.
        fillLeft(player1Color);
        fillRight(player2Color);
      }

      break;
  }
}

// ======================================================
// 11. WIFI CONNECTION
// ======================================================

void connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, wifiPassword);
}

// ======================================================
// 12. MQTT CONNECTION
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

  bool connected = client.connect(
    clientId.c_str(),
    mqttUser,
    mqttPassword
  );

  if (connected) {

    Serial.println("MQTT Connected!");

    lastPublishTime =
      millis() - PUBLISH_INTERVAL;

  } else {

    Serial.print("MQTT Connection Failed: ");
    Serial.println(client.state());
  }
}

// ======================================================
// 13. MQTT RGB PUBLISH
// ======================================================

void publishRGB() {

  static unsigned long lastDebugTime = 0;

  if (!client.connected()) {

    if (millis() - lastDebugTime >= 2000) {
      lastDebugTime = millis();
      Serial.println("MQTT NOT CONNECTED");
    }

    return;
  }

  updateLEDDisplay();

  bool success = client.publish(
    mqttTopic,
    RGBpayload,
    (unsigned int)PAYLOAD_SIZE
  );

  if (millis() - lastDebugTime >= 2000) {

    lastDebugTime = millis();

    if (success) {

      Serial.println("RGB MQTT SENT SUCCESSFULLY");

      Serial.print("Topic: ");
      Serial.println(mqttTopic);

      Serial.print("Payload bytes: ");
      Serial.println(PAYLOAD_SIZE);

      Serial.print("RGB: ");
      Serial.print(currentColor.r);
      Serial.print(", ");
      Serial.print(currentColor.g);
      Serial.print(", ");
      Serial.println(currentColor.b);

    } else {

      Serial.println("MQTT PUBLISH FAILED");

      Serial.print("MQTT state: ");
      Serial.println(client.state());
    }
  }
}

// ======================================================
// 14. COLOUR DISTANCE
// ======================================================

long calculateColourDistance(
  RGBColor a,
  RGBColor b
) {

  long dr = a.r - b.r;
  long dg = a.g - b.g;
  long db = a.b - b.b;

  // Squared Euclidean distance
  return dr * dr + dg * dg + db * db;
}

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
  Serial.println("===== GAME RESULTS =====");

  Serial.print("Target: ");
  Serial.print(targetColor.r);
  Serial.print(", ");
  Serial.print(targetColor.g);
  Serial.print(", ");
  Serial.println(targetColor.b);

  Serial.print("Player 1 distance: ");
  Serial.println(score1);

  Serial.print("Player 2 distance: ");
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

  Serial.println("========================");
}

// ======================================================
// 15. GAME STATE CHANGE
// ======================================================

void changeGameState(GameState newState) {

  gameState = newState;
  stateStartTime = millis();

  // Send new state immediately on next loop.
  lastPublishTime =
    millis() - PUBLISH_INTERVAL;
}

// ======================================================
// 16. START NEW GAME
// ======================================================

void startGame() {

  targetColor.r = random(40, 256);
  targetColor.g = random(40, 256);
  targetColor.b = random(40, 256);

  player1Color = BLACK;
  player2Color = BLACK;

  winner = 0;
  clickCount = 0;

  Serial.println();
  Serial.println("===== NEW GAME =====");

  Serial.print("Target RGB: ");
  Serial.print(targetColor.r);
  Serial.print(", ");
  Serial.print(targetColor.g);
  Serial.print(", ");
  Serial.println(targetColor.b);

  Serial.println("Memorise target colour!");

  changeGameState(SHOW_TARGET);
}

// ======================================================
// 17. SINGLE CLICK
// ======================================================

void handleSingleClick() {

  switch (gameState) {

    case FREE_MIXING:

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

      Serial.println("PLAYER 2 TURN");

      changeGameState(PLAYER_2);
      break;

    case PLAYER_2:

      player2Color = currentColor;

      Serial.print("PLAYER 2 SAVED: ");
      Serial.print(player2Color.r);
      Serial.print(", ");
      Serial.print(player2Color.g);
      Serial.print(", ");
      Serial.println(player2Color.b);

      Serial.println("DOUBLE CLICK TO JUDGE");

      changeGameState(WAIT_JUDGEMENT);
      break;

    default:
      break;
  }
}

// ======================================================
// 18. DOUBLE CLICK
// ======================================================

void handleDoubleClick() {

  if (gameState == WAIT_JUDGEMENT) {

    Serial.println("JUDGING...");

    changeGameState(JUDGING);
  }
}

// ======================================================
// 19. BUTTON DEBOUNCE AND CLICK DETECTION
// ======================================================

void updateButton() {

  unsigned long now = millis();

  bool rawButtonState =
    digitalRead(BUTTON_PIN);

  if (rawButtonState != lastRawButtonState) {

    lastDebounceTime = now;
  }

  if (
    now - lastDebounceTime >= DEBOUNCE_DELAY
  ) {

    if (
      rawButtonState != stableButtonState
    ) {

      stableButtonState = rawButtonState;

      // Count click on release.
      if (stableButtonState == HIGH) {

        if (gameState == WAIT_JUDGEMENT) {

          if (clickCount == 0) {

            clickCount = 1;
            firstClickTime = now;

          } else if (
            now - firstClickTime <=
            DOUBLE_CLICK_WINDOW
          ) {

            clickCount = 0;
            handleDoubleClick();

          } else {

            clickCount = 1;
            firstClickTime = now;
          }

        } else {

          clickCount = 0;
          handleSingleClick();
        }
      }
    }
  }

  lastRawButtonState = rawButtonState;

  if (
    clickCount == 1 &&
    now - firstClickTime >
    DOUBLE_CLICK_WINDOW
  ) {

    clickCount = 0;
    Serial.println("Double click not detected.");
  }
}

// ======================================================
// 20. GAME TIMER
// ======================================================

void updateGameTiming() {

  unsigned long elapsed =
    millis() - stateStartTime;

  switch (gameState) {

    case SHOW_TARGET:

      if (elapsed >= TARGET_DURATION) {

        Serial.println("PLAYER 1 TURN");

        changeGameState(PLAYER_1);
      }

      break;

    case JUDGING:

      if (elapsed >= JUDGE_DURATION) {

        judgeGame();

        changeGameState(SHOW_WINNER);
      }

      break;

    case SHOW_WINNER:

      if (elapsed >= WINNER_DURATION) {

        Serial.println("GAME RESET");
        Serial.println("FREE RGB MIXING");

        changeGameState(FREE_MIXING);
      }

      break;

    default:
      break;
  }
}

// ======================================================
// 21. SETUP
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

  // Initialise smoothing buffers.
  for (int i = 0; i < SAMPLE_COUNT; i++) {

    updateSensorSamples();
    delay(10);
  }

  randomSeed(
    analogRead(A0) ^ micros()
  );

  client.setBufferSize(512);

  client.setServer(
    mqttServer,
    mqttPort
  );

  connectWiFi();

  lastWiFiAttempt = millis();

  changeGameState(FREE_MIXING);

  Serial.println();
  Serial.println("RGB COLOUR DUEL V3 READY");
  Serial.println("Free RGB mixing enabled.");
  Serial.println("Press button to start game.");
}

// ======================================================
// 22. MAIN LOOP
// ======================================================

void loop() {

  unsigned long now = millis();

  // 1. Read RGB sensors
  currentColor = readRotaryRGB();

  // 2. Print RGB values every 500 ms
  printRGBValues();

  // 3. Detect button clicks
  updateButton();

  // 4. Update game timers
  updateGameTiming();

  // 5. Maintain WiFi connection
  if (WiFi.status() != WL_CONNECTED) {

    if (
      now - lastWiFiAttempt >=
      WIFI_RETRY_INTERVAL
    ) {

      lastWiFiAttempt = now;
      connectWiFi();
    }

  } else {

    // 6. Maintain MQTT connection
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

  // 7. Send RGB payload to Vespera
  if (
    now - lastPublishTime >=
    PUBLISH_INTERVAL
  ) {

    lastPublishTime = now;
    publishRGB();
  }

  delay(10);
}
