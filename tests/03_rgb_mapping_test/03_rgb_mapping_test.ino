/*
* Project: Colour Duel
 * File: 03_rgb_mapping_test.ino
 * Module: CASA0014 - Prototyping the Internet of Things
 * Board: Arduino MKR WiFi 1010
 *
 * Description:
 * This sketch tests three rotary angle sensors used to
 * control the Red, Green, and Blue channels of an RGB
 * Colour Mixer.
 *
 * Each sensor is connected to an analog input:
 *   - Red   -> A0
 *   - Green -> A1
 *   - Blue  -> A2
 *
 * The Arduino reads raw analog values (0-1023) and
 * maps them to RGB intensity values (0-255).
 *
 * Both raw sensor readings and mapped RGB values
 * are displayed in the Serial Monitor at 9600 baud.
 *
 * Test Result:
 * SUCCESS - Arduino reads the three analog inputs
 * and converts their values into RGB data.
 */

const int redPin = A0;
const int greenPin = A1;
const int bluePin = A2;

void setup() {
  Serial.begin(9600);
  analogReadResolution(10);

  while (!Serial && millis() < 5000) {
    // Wait for Serial Monitor
  }

  Serial.println("RGB Colour Mixer Started!");
  Serial.println("-------------------------");
}

void loop() {
  // Read three rotary sensors
  int redRaw = analogRead(redPin);
  int greenRaw = analogRead(greenPin);
  int blueRaw = analogRead(bluePin);

  // Convert 0-1023 into RGB 0-255
  int red = map(redRaw, 0, 1023, 0, 255);
  int green = map(greenRaw, 0, 1023, 0, 255);
  int blue = map(blueRaw, 0, 1023, 0, 255);

  // Print raw sensor values
  Serial.print("RAW: ");
  Serial.print(redRaw);
  Serial.print(" | ");
  Serial.print(greenRaw);
  Serial.print(" | ");
  Serial.print(blueRaw);

  // Print RGB values
  Serial.print("    RGB: ");
  Serial.print(red);
  Serial.print(", ");
  Serial.print(green);
  Serial.print(", ");
  Serial.println(blue);

  delay(200);
}

