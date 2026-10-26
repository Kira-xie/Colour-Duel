#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

// ======================================
// 1. WiFi Configuration
// ======================================

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

// ======================================
// 2. MQTT Configuration
// ======================================

const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;

const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;

// Luminaire ID: 24
const char* mqtt_topic =
  "student/CASA0014/luminaire/24";

// ======================================
// 3. WiFi and MQTT Clients
// ======================================

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// ======================================
// 4. LED Configuration
// ======================================

// 72 LEDs, each with R, G, B
const int num_leds = 72;
const int payload_size = num_leds * 3;

// 216-byte RGB buffer
byte RGBpayload[payload_size] = {0};

// Animation speed (milliseconds)
const unsigned long updateInterval = 100;

// ======================================
// 5. WiFi Connection
// ======================================

void startWifi() {

  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {

    WiFi.begin(ssid, password);

    delay(5000);

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi retrying...");
    }
  }

  Serial.println("WiFi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ======================================
// 6. MQTT Connection
// ======================================

void reconnectMQTT() {

  while (!mqttClient.connected()) {

    // Make sure WiFi is connected
    if (WiFi.status() != WL_CONNECTED) {
      startWifi();
    }

    Serial.println("Connecting to MQTT...");

    // Generate MQTT Client ID
    // Does not use WiFi.macAddress()
    String clientId =
      "MKR1010_Random_" +
      String(random(0xffff), HEX);

    if (mqttClient.connect(
          clientId.c_str(),
          mqtt_username,
          mqtt_password
        )) {

      Serial.println("MQTT Connected!");

      Serial.print("Publishing to: ");
      Serial.println(mqtt_topic);

    } else {

      Serial.print("MQTT Failed. State: ");
      Serial.println(mqttClient.state());

      delay(5000);
    }
  }
}

// ======================================
// 7. Generate Random RGB Colours
// ======================================

void generateRandomColors() {

  for (int pixel = 0; pixel < num_leds; pixel++) {

    // Generate random values from 50 to 255
    // byte red   = (byte)random(50, 256);
    // byte green = (byte)random(50, 256);
    // byte blue  = (byte)random(50, 256);

    // Store RGB values in the buffer
    RGBpayload[pixel * 3 + 0] = 0;
    RGBpayload[pixel * 3 + 1] = 0;
    RGBpayload[pixel * 3 + 2] = 255;
  }
}

// ======================================
// 8. Publish Random Colours to MQTT
// ======================================

void send_all_random() {

  if (!mqttClient.connected()) {
    Serial.println("MQTT disconnected!");
    return;
  }

  // Generate a new random frame
  generateRandomColors();

  // Send all 216 bytes
  bool success = mqttClient.publish(
    mqtt_topic,
    RGBpayload,
    payload_size
  );

  if (success) {

    static unsigned long frameCount = 0;
    frameCount++;

    // Display progress every 10 frames
    if (frameCount % 10 == 0) {

      Serial.print("Random frame sent: ");
      Serial.println(frameCount);
    }

  } else {

    Serial.println("MQTT Publish Failed!");
  }
}

// ======================================
// 9. Arduino Setup
// ======================================

void setup() {

  Serial.begin(115200);
  delay(1500);

  Serial.println("==========================");
  Serial.println("Vespera Random Colours");
  Serial.println("Luminaire ID: 24");
  Serial.println("==========================");

  // Initialise random generator
  randomSeed(micros());

  // Connect WiFi
  startWifi();

  // Configure MQTT server
  mqttClient.setServer(
    mqtt_server,
    mqtt_port
  );

  // Buffer must fit MQTT header,
  // topic and 216-byte RGB payload
  mqttClient.setBufferSize(512);

  mqttClient.setKeepAlive(30);

  // Connect MQTT
  reconnectMQTT();

  Serial.println("System Ready!");
}

// ======================================
// 10. Main Loop
// ======================================

void loop() {

  // Reconnect WiFi if necessary
  if (WiFi.status() != WL_CONNECTED) {
    startWifi();
  }

  // Reconnect MQTT if necessary
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }

  // Maintain MQTT connection
  mqttClient.loop();

  // Generate and send random colours
  send_all_random();

  // Wait before the next frame
  delay(updateInterval);
}