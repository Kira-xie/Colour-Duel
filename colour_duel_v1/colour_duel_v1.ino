/*
  ======================================================
  COLOUR DUEL - VERSION 1
  Basic RGB Colour Mixer
  ======================================================

  Module: CASA0014 - Prototyping the Internet of Things
  Board: Arduino MKR WiFi 1010

  INPUT:
  Red rotary sensor   -> A0
  Green rotary sensor -> A1
  Blue rotary sensor  -> A2

  OUTPUT:
  Vespera / Lumi (72 RGB LEDs)

  FEATURES:
  - Free RGB colour mixing
  - Three independent rotary controls
  - Real-time RGB adjustment
  - Analog-to-RGB mapping (0-1023 to 0-255)
  - RGB values displayed in Serial Monitor
  - Moving average sensor smoothing
  - WiFi + MQTT communication
  - Real-time colour display on all 72 LEDs

  DESCRIPTION:
  This initial version explores physical RGB colour
  mixing using three rotary sensors.

  Users can freely adjust the red, green and blue
  channels and observe the resulting colour changes
  on the Vespera / Lumi LED interface.

  MQTT:
  Broker: mqtt.cetools.org
  Port: 1884
  Topic: student/CASA0014/luminaire/24

  SERIAL BAUD RATE: 9600
*/

#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

// =====================================
// 1. TEST MODE
// =====================================

// true  = automatic colour test
// false = control colours with rotary sensors

const bool TEST_MODE = false;

// =====================================
// 2. SENSOR SETTINGS
// =====================================

const int redPin = A0;
const int greenPin = A1;
const int bluePin = A2;

const int sampleCount = 10;

int redSamples[sampleCount] = {0};
int greenSamples[sampleCount] = {0};
int blueSamples[sampleCount] = {0};

int sampleIndex = 0;

// =====================================
// 3. WIFI / MQTT SETTINGS
// =====================================

const char* ssid = SECRET_SSID;
const char* wifiPassword = SECRET_PASS;

const char* mqttUser = SECRET_MQTTUSER;
const char* mqttPassword = SECRET_MQTTPASS;

const char* mqttServer = "mqtt.cetools.org";

// Test authenticated MQTT port
const int mqttPort = 1884;

const char* mqttTopic =
  "student/CASA0014/luminaire/24";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// =====================================
// 4. VESPERA SETTINGS
// =====================================

const int numLeds = 72;
const int payloadSize = numLeds * 3;

byte RGBpayload[payloadSize];

// =====================================
// 5. TIMING SETTINGS
// =====================================

unsigned long lastPublish = 0;
unsigned long lastMQTTAttempt = 0;
unsigned long lastWiFiAttempt = 0;

const unsigned long publishInterval = 200;
const unsigned long refreshInterval = 2000;

const unsigned long mqttRetryInterval = 5000;
const unsigned long wifiRetryInterval = 10000;

int lastRed = -1;
int lastGreen = -1;
int lastBlue = -1;

// Fixed colour test
int testStep = 0;
unsigned long lastTestChange = 0;
const unsigned long testDuration = 3000;

// =====================================
// 6. CONNECT WIFI
// =====================================

void connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Connecting to Wi-Fi...");

  WiFi.begin(ssid, wifiPassword);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 15000) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Wi-Fi connected!");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println("Wi-Fi connection failed!");
  }
}

// =====================================
// 7. CONNECT MQTT
// =====================================

void connectMQTT() {

  if (client.connected()) {
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  Serial.println("Connecting to MQTT...");

  String clientId =
    "RGBMixer24-" +
    String((unsigned long)millis(), HEX);

  bool success = client.connect(
    clientId.c_str(),
    mqttUser,
    mqttPassword
  );

  if (success) {

    Serial.println("MQTT connected!");

    Serial.print("Broker: ");
    Serial.println(mqttServer);

    Serial.print("Port: ");
    Serial.println(mqttPort);

    Serial.print("Topic: ");
    Serial.println(mqttTopic);

    lastRed = -1;
    lastGreen = -1;
    lastBlue = -1;

    lastPublish = millis() - refreshInterval;

  } else {

    Serial.print("MQTT connection failed. State: ");
    Serial.println(client.state());
  }
}

// =====================================
// 8. SENSOR READING
// =====================================

void updateSamples() {

  redSamples[sampleIndex] = analogRead(redPin);
  greenSamples[sampleIndex] = analogRead(greenPin);
  blueSamples[sampleIndex] = analogRead(bluePin);

  sampleIndex = (sampleIndex + 1) % sampleCount;
}

int calculateAverage(int samples[]) {

  long total = 0;

  for (int i = 0; i < sampleCount; i++) {
    total += samples[i];
  }

  return total / sampleCount;
}

// =====================================
// 9. RGB MAPPING
// =====================================

int convertToRGB(int rawValue) {

  rawValue = constrain(rawValue, 0, 1023);

  return constrain(
    map(rawValue, 0, 1023, 0, 255),
    0,
    255
  );
}

// =====================================
// 10. CREATE RGB PAYLOAD
// =====================================

void createRGBPayload(int r, int g, int b) {

  for (int pixel = 0; pixel < numLeds; pixel++) {

    RGBpayload[pixel * 3] = (byte)r;
    RGBpayload[pixel * 3 + 1] = (byte)g;
    RGBpayload[pixel * 3 + 2] = (byte)b;
  }
}

// =====================================
// 11. PUBLISH RGB
// =====================================

bool sendRGB(int r, int g, int b) {

  if (!client.connected()) {

    Serial.println("MQTT disconnected!");
    return false;
  }

  createRGBPayload(r, g, b);

  bool success = client.publish(
    mqttTopic,
    RGBpayload,
    (unsigned int)payloadSize
  );

  if (success) {

    Serial.print("RGB Sent: ");
    Serial.print(r);
    Serial.print(", ");
    Serial.print(g);
    Serial.print(", ");
    Serial.println(b);

    return true;

  } else {

    Serial.println("MQTT publish FAILED!");

    Serial.print("State: ");
    Serial.println(client.state());

    Serial.print("Buffer: ");
    Serial.println(client.getBufferSize());

    Serial.print("Payload: ");
    Serial.println(payloadSize);

    return false;
  }
}

// =====================================
// 12. FIXED COLOUR TEST
// =====================================

void getTestColour(int &r, int &g, int &b) {

  switch (testStep) {

    case 0:
      r = 255;
      g = 0;
      b = 0;
      break;

    case 1:
      r = 0;
      g = 255;
      b = 0;
      break;

    case 2:
      r = 0;
      g = 0;
      b = 255;
      break;

    default:
      r = 255;
      g = 255;
      b = 255;
      break;
  }
}

// =====================================
// 13. SETUP
// =====================================

void setup() {

  Serial.begin(9600);

  while (!Serial && millis() < 5000) {
  }

  analogReadResolution(10);

  Serial.println();
  Serial.println("==========================");
  Serial.println("RGB COLOUR MIXER");
  Serial.println("VESPERA LIGHT ID: 24");
  Serial.println("==========================");

  Serial.print("Test Mode: ");
  Serial.println(TEST_MODE ? "ON" : "OFF");

  // Initialise sensor buffers
  for (int i = 0; i < sampleCount; i++) {
    updateSamples();
    delay(10);
  }

  // Configure MQTT buffer
  bool bufferOK = client.setBufferSize(512);

  Serial.print("Buffer configured: ");
  Serial.println(bufferOK ? "YES" : "NO");

  Serial.print("Buffer size: ");
  Serial.println(client.getBufferSize());

  client.setServer(mqttServer, mqttPort);

  connectWiFi();
  connectMQTT();

  lastTestChange = millis();

  Serial.println("Setup complete.");
}

// =====================================
// 14. MAIN LOOP
// =====================================

void loop() {

  unsigned long now = millis();

  // Wi-Fi reconnection
  if (WiFi.status() != WL_CONNECTED) {

    if (now - lastWiFiAttempt >= wifiRetryInterval) {

      lastWiFiAttempt = now;
      connectWiFi();
    }
  }

  // MQTT reconnection
  if (WiFi.status() == WL_CONNECTED) {

    if (!client.connected()) {

      if (now - lastMQTTAttempt >= mqttRetryInterval) {

        lastMQTTAttempt = now;
        connectMQTT();
      }

    } else {

      client.loop();
    }
  }

  // Read sensors continuously
  updateSamples();

  int red = 0;
  int green = 0;
  int blue = 0;

  // ===================================
  // TEST MODE
  // ===================================

  if (TEST_MODE) {

    if (now - lastTestChange >= testDuration) {

      testStep = (testStep + 1) % 4;
      lastTestChange = now;
    }

    getTestColour(red, green, blue);

  } else {

    // =================================
    // ROTARY SENSOR MODE
    // =================================

    int redRaw = calculateAverage(redSamples);
    int greenRaw = calculateAverage(greenSamples);
    int blueRaw = calculateAverage(blueSamples);

    red = convertToRGB(redRaw);
    green = convertToRGB(greenRaw);
    blue = convertToRGB(blueRaw);
  }

  // ===================================
  // MQTT PUBLISH
  // ===================================

  bool colourChanged =
    red != lastRed ||
    green != lastGreen ||
    blue != lastBlue;

  bool refreshNeeded =
    now - lastPublish >= refreshInterval;

  if (client.connected() &&
      now - lastPublish >= publishInterval &&
      (colourChanged || refreshNeeded)) {

    bool success = sendRGB(red, green, blue);

    if (success) {

      lastRed = red;
      lastGreen = green;
      lastBlue = blue;
    }

    lastPublish = now;
  }

  delay(10);
}
