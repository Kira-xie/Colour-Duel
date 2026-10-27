/*
 * PROJECT OVERVIEW
 * This prototype demonstrates the intended visual response
 * of Echo of Touch before integrating the Piezo sensor.
 *
 * A simulated touch-frequency level controls a spatial
 * colour gradient across 72 Vespera LEDs:
 *
 * Idle          -> All blue
 * Low activity  -> Purple begins spreading from the left
 * Medium        -> Purple, pink and red gradients expand
 * Maximum       -> All red
 * Recovery      -> Gradually returns to blue
 *
 * IMPORTANT
 * - No physical sensor is used in this version.
 * - Touch intensity is simulated using a 16-second cycle.
 * - The RGB colour palette, spatial gradient algorithm and
 *   MQTT publishing logic can be reused in the final version.
 * - Future development will replace the simulated input
 *   with touch-frequency values measured from a Piezo disc.
 *
 * Output: 72 LEDs x 3 RGB channels = 216-byte MQTT payload
 * Luminaire ID: 24
 *
 * Status: Software simulation — sensor integration pending
 * ============================================================
 */

#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

// ======================================
// 1. Wi-Fi Network Configuration
// ======================================

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

// ======================================
// 2. MQTT Broker and Luminaire Settings
// ======================================

const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;

const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;

// Personal Vespera Luminaire ID: 24
const char* mqtt_topic =
  "student/CASA0014/luminaire/24";

// ======================================
// 3. Network and MQTT Client Initialisation
// ======================================

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// ======================================
// 4. LED Array and Animation Configuration
// ======================================

// Vespera contains 72 LEDs with RGB channels
const int num_leds = 72;
const int payload_size = num_leds * 3;

// RGB data buffer: 72 x 3 = 216 bytes
byte RGBpayload[payload_size] = {0};

// Animation update interval: 100 milliseconds
const unsigned long updateInterval = 100;

// ======================================
// 5. Establish Wi-Fi Connection
// ======================================

void startWifi() {

  Serial.println("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {

    WiFi.begin(ssid, password);

    delay(5000);

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wi-Fi connection retrying...");
    }
  }

  Serial.println("Wi-Fi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ======================================
// 6. Establish and Maintain MQTT Connection
// ======================================

void reconnectMQTT() {

  while (!mqttClient.connected()) {

    // Restore Wi-Fi before reconnecting to MQTT
    if (WiFi.status() != WL_CONNECTED) {
      startWifi();
    }

    Serial.println("Connecting to MQTT Broker...");

    // Generate a unique MQTT client identifier
    String clientId =
      "MKR1010_EchoOfTouch_" +
      String(random(0xffff), HEX);

    if (mqttClient.connect(
          clientId.c_str(),
          mqtt_username,
          mqtt_password
        )) {

      Serial.println("MQTT Connected!");

      Serial.print("Publishing to topic: ");
      Serial.println(mqtt_topic);

    } else {

      Serial.print("MQTT Connection Failed. State: ");
      Serial.println(mqttClient.state());

      delay(5000);
    }
  }
}

// ======================================
// 7. Generate Simulated Touch-Frequency
//    Colour Wave and Spatial RGB Gradient
// ======================================

void generateColourWave() {

  // Simulate different levels of touch activity
  // using a repeating 16-second animation cycle.

  unsigned long cycleTime = millis() % 16000;

  float targetIntensity = 0.0f;

  // 0–3s: Idle — all LEDs remain blue
  if (cycleTime < 3000) {
    targetIntensity = 0.0f;
  }

  // 3–6s: Low activity — subtle purple gradient
  else if (cycleTime < 6000) {
    targetIntensity = 0.25f;
  }

  // 6–9s: Medium activity — gradient expands
  else if (cycleTime < 9000) {
    targetIntensity = 0.55f;
  }

  // 9–12s: Maximum activity — transition to red
  else if (cycleTime < 12000) {
    targetIntensity = 1.0f;
  }

  // 12–16s: Recovery — return gradually to blue
  else {
    targetIntensity = 0.0f;
  }

  // Smooth temporal transition between activity levels
  static float intensity = 0.0f;

  intensity +=
    (targetIntensity - intensity) * 0.12f;

  intensity = constrain(intensity, 0.0f, 1.0f);

  // Five-colour RGB palette:
  // Blue -> Purple -> Pink-purple -> Rose -> Red
  const byte palette[5][3] = {
    {0, 60, 255},     // Blue: idle state
    {112, 46, 235},   // Purple: low activity
    {183, 62, 190},   // Pink-purple: medium activity
    {235, 34, 100},   // Rose red: high activity
    {255, 0, 0}       // Red: maximum activity
  };

  // Generate one complete RGB frame for 72 LEDs
  for (int pixel = 0; pixel < num_leds; pixel++) {

    // Vespera layout: 12 columns x 6 LEDs
    int column = pixel / 6;

    // Normalise horizontal position from 0 to 1
    float position = column / 11.0f;

    // Move the colour transition from left to right
    float localIntensity =
      (intensity - position * 0.70f) / 0.30f;

    localIntensity = constrain(
      localIntensity, 0.0f, 1.0f
    );

    // Identify adjacent RGB palette colours
    float scaled = localIntensity * 4.0f;

    int index = min((int)scaled, 3);

    float fraction = scaled - index;

    // Smoothstep interpolation for softer gradients
    fraction = fraction * fraction *
      (3.0f - 2.0f * fraction);

    // Calculate red, green and blue for each LED
    for (int channel = 0; channel < 3; channel++) {

      float value =
        palette[index][channel] +
        (palette[index + 1][channel] -
         palette[index][channel]) * fraction;

      RGBpayload[pixel * 3 + channel] =
        (byte)value;
    }
  }
}

// ======================================
// 8. Publish the RGB Colour Wave via MQTT
// ======================================

void publishColourWave() {

  // Ensure the MQTT connection is active
  if (!mqttClient.connected()) {
    Serial.println("MQTT disconnected!");
    return;
  }

  // Generate the current simulated RGB frame
  generateColourWave();

  // Publish all 216 RGB bytes to Vespera
  bool success = mqttClient.publish(
    mqtt_topic,
    RGBpayload,
    payload_size
  );

  if (success) {

    static unsigned long frameCount = 0;
    frameCount++;

    // Log progress every 10 successfully sent frames
    if (frameCount % 10 == 0) {
      Serial.print("Colour wave frame sent: ");
      Serial.println(frameCount);
    }

  } else {
    Serial.println("MQTT RGB Publish Failed!");
  }
}

// ======================================
// 9. System Initialisation and Setup
// ======================================

void setup() {

  Serial.begin(115200);
  delay(1500);

  Serial.println("==========================");
  Serial.println("Echo of Touch");
  Serial.println("Colour Wave Simulation");
  Serial.println("Luminaire ID: 24");
  Serial.println("Sensor: Simulated Input");
  Serial.println("==========================");

  // Initialise random values for MQTT client IDs
  randomSeed(micros());

  // Establish Wi-Fi connection
  startWifi();

  // Configure the MQTT broker and port
  mqttClient.setServer(
    mqtt_server,
    mqtt_port
  );

  // Buffer accommodates topic, header and RGB payload
  mqttClient.setBufferSize(512);

  mqttClient.setKeepAlive(30);

  // Establish MQTT connection
  reconnectMQTT();

  Serial.println("Echo of Touch Simulation Ready!");
}

// ======================================
// 10. Main Execution Loop
// ======================================

void loop() {

  // Automatically reconnect Wi-Fi if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    startWifi();
  }

  // Automatically reconnect to MQTT if disconnected
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  // Maintain the MQTT communication session
  mqttClient.loop();

  // Generate and publish the simulated colour wave
  publishColourWave();

  // Control the animation frame rate
  delay(updateInterval);
}
