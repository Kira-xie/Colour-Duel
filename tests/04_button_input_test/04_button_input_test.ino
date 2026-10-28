/*
 * Project: Colour Duel
 * File: 04_button_input_test.ino
 * Module: CASA0014 - Prototyping the Internet of Things
 * Board: Arduino MKR WiFi 1010
 *
 * Description:
 * This sketch tests a physical momentary push button
 * used to control the interaction flow of Colour Duel.
 *
 * The button is connected to:
 *   - Button terminal 1 -> Digital Pin 2 (D2)
 *   - Button terminal 2 -> GND
 *
 * The Arduino uses INPUT_PULLUP to detect button
 * presses without requiring an external resistor.
 *
 * When the button is pressed, the digital input
 * changes from HIGH to LOW. The button state is
 * monitored and displayed in the Serial Monitor
 * at 9600 baud.
 *
 * Purpose:
 * This test was developed between Colour Duel v1
 * and v2 to verify reliable physical button input
 * before integrating button-controlled gameplay.
 *
 * Test Result:
 * SUCCESS - Arduino detects physical button presses
 * and releases, confirming that the button can be
 * used to start the game and save player RGB colours.
 */

const int buttonPin = 2;

bool lastButton = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  bool currentButton = digitalRead(buttonPin);

  if (lastButton == HIGH && currentButton == LOW) {
    Serial.println("BUTTON PRESSED");
  }

  if (lastButton == LOW && currentButton == HIGH) {
    Serial.println("BUTTON RELEASED");
  }

  lastButton = currentButton;
  delay(10);
}
