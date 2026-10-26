#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

// Wi-Fi credentials
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println("CASA0014 WiFi Test");
  Serial.println("Arduino MKR WiFi 1010");

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("WiFi module not detected!");
    return;
  }

  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WiFi Connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal Strength (RSSI): ");
    Serial.println(WiFi.RSSI());

    delay(5000);

  } else {

    Serial.println("WiFi not connected.");

    // Retry Wi-Fi connection
    WiFi.begin(ssid, password);

    delay(5000);
  }
}

