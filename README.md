# Colour Duel

### Explore Colour. Remember Colour. Challenge Colour.

**An IoT-based RGB colour exploration and two-player matching game**

**Module:** CASA0014 – Prototyping the Internet of Things  
**Institution:** University College London (UCL)  
**Student:** Jiahan Xie  

---

## 1. Project Overview

**Colour Duel** is a tangible Internet of Things (IoT) prototype that explores how physical controls can make digital colour mixing intuitive, playful and social.

Three rotary angle sensors independently control the **red (R), green (G) and blue (B)** channels of a connected 72-pixel lighting display. An Arduino MKR WiFi 1010 reads the physical inputs, processes their values and sends RGB frames over Wi-Fi and MQTT to the Vespera / Lumi display.

The prototype has **two complementary interaction modes**:

1. **RGB Exploration Mode:** With no game running, anyone can freely rotate the three controls and immediately see how different red, green and blue intensities combine into a single colour across all 72 pixels.
2. **Colour Duel Mode:** Pressing the physical button starts a two-player colour-memory challenge. Players take turns reconstructing a briefly displayed target colour using the same three controls. The system evaluates their attempts and displays the winner through spatial lighting feedback.

## 2. Design Concept

### From colour exploration to colour competition

RGB colour is often represented through numerical values or on-screen sliders. Colour Duel replaces these with three tactile rotary controls. The user can explore colour through hand movement and receive immediate visual feedback from a remote LED display.

The button introduces a second layer of interaction: a simple exploratory interface becomes a game of observation, memory and adjustment. Both modes use the same hardware, demonstrating how the meaning of a physical interface can change through software-defined states.

### Mode A — RGB Exploration

This is the default state when the device is powered on and after each game finishes.

- The **R** rotary angle sensor adjusts red intensity from approximately 0 to 255.
- The **G** rotary angle sensor adjusts green intensity from approximately 0 to 255.
- The **B** rotary angle sensor adjusts blue intensity from approximately 0 to 255.
- All **72 LEDs** display the resulting combined colour.
- Users can experiment without a time limit, score or competitive objective.
- Pressing the button switches to Colour Duel Mode.

### Mode B — Colour Duel

Two players use the same set of controls in sequence. Their goal is to reproduce a randomly generated target colour as closely as possible from memory.

| Stage | Interaction | Lighting feedback |
| --- | --- | --- |
| 1. Start | Press the physical button | Game begins |
| 2. Memorise | Observe the random target for 3 seconds | All 72 LEDs show the target |
| 3. Player 1 ready | Wait 1.5 seconds | Left 36 LEDs show white; right side is off |
| 4. Player 1 mixes | Adjust R, G and B; press to save | Left 36 LEDs show the live mixture |
| 5. Player 2 ready | Wait 1.5 seconds | Right 36 LEDs show white; left side is off |
| 6. Player 2 mixes | Adjust R, G and B; press to save | Right 36 LEDs show the live mixture |
| 7. Judging | Automatic 1-second interval | All LEDs off |
| 8. Result | No further input needed | Winning player's half lights up in their saved colour for 3 seconds |
| 9. Reset | Automatic | All 72 LEDs return to live RGB Exploration Mode |

## 3. Hardware and Materials

### Electronic components

| Component | Quantity | Purpose |
| --- | ---: | --- |
| Arduino MKR WiFi 1010 | 1 | Analogue input, game logic and wireless communication |
| Rotary angle sensors | 3 | Independent R, G and B control |
| Momentary push button | 1 | Start game and save each player's colour |
| Breadboard | 1 | Prototyping electrical connections |
| Jumper wires | As required | Signal, power and ground connections |
| USB data/power cable | 1 | Programming and powering the Arduino |
| Cardboard and assembly materials | As required | Planned protective and interactive enclosure |
| Vespera / Lumi virtual LED display | 1 | Connected 72-pixel visual output |

The Arduino MKR WiFi 1010 uses **3.3 V logic**. Sensor wiring must be compatible with the board's analogue input voltage range; do not connect a 5 V analogue output to an MKR analogue pin. 

### Input connections

| Input | Arduino pin | Function |
| --- | --- | --- |
| Red potentiometer wiper | `A0` | Red channel |
| Green potentiometer wiper | `A1` | Green channel |
| Blue potentiometer wiper | `A2` | Blue channel |
| Push button | `D2` and `GND` | Active-low input using `INPUT_PULLUP` |

Wiring details and circuit explanation: [Circuit design](docs/circuit_design.md), including the original three-sensor circuit and the later button connection.

### Physical enclosure

The intended enclosure is a simple cardboard control panel with three clearly labelled rotary controls (**R / G / B**) and a prominent **START / SAVE** button. The structure should protect the Arduino and wiring while leaving the controls accessible, providing USB cable access and allowing the enclosure to be opened for maintenance.

Final enclosure dimensions, mounting details and photographs will be added after physical verification.

### LED arrangement

The Vespera / Lumi display contains **72 pixels** arranged as a **12-column × 6-row** grid. The firmware treats it as two groups of 36 pixels:

- **Player 1:** pixel indices `0–35` (left half in the tested display arrangement).
- **Player 2:** pixel indices `36–71` (right half).
- **Exploration and target display:** all indices `0–71`.

Each pixel is represented by three consecutive bytes: **R, G, B**. One full display frame therefore contains **72 × 3 = 216 binary bytes**.

### Wi-Fi and MQTT

| Parameter | Value |
| --- | --- |
| Board | Arduino MKR WiFi 1010 |
| MQTT broker | `mqtt.cetools.org` |
| MQTT port | `1884` |
| Publish topic | `student/CASA0014/luminaire/24` |
| Payload | 216 raw RGB bytes per frame |
| Intended publish interval | Approximately 150 ms |
| MQTT library | PubSubClient |
| Wi-Fi library | WiFiNINA |

**Security:** Wi-Fi and MQTT credentials belong in a local `arduino_secrets.h` file and must not be committed to a public repository. The topic and device identifier above are project-specific and may need changing for another people's setup.

## 4. Colour Matching and Scoring

The target colour and both player colours are stored as RGB triples. After Player 2 presses the button to save, the Arduino compares each player's colour against the target using a **weighted squared RGB distance**:

$$
D^2 = 299(R_p - R_t)^2 + 587(G_p - G_t)^2 + 114(B_p - B_t)^2
$$

Where:

- \(R_p, G_p, B_p\) are the player's saved channel values.
- \(R_t, G_t, B_t\) are the target channel values.
- **Lower score means a closer match** under this metric.

The relative channel weights (0.299, 0.587 and 0.114, scaled by 1000 for integer arithmetic) are adapted from a familiar luminance-weighting convention. This is a **simple prototype scoring heuristic**, not a perceptually uniform colour-difference standard such as CIEDE2000.

The system awards the win to the player with the lower score. Equal scores produce a draw. The outcome is also printed to the Arduino Serial Monitor for inspection.

**Formula explanation:** [Colour scoring algorithm](docs/colour_scoring_algorithm.md).

## 5. Software and Game Logic

The Arduino sketch uses an explicit state machine:

```text
FREE_MIXING
    |
    | Button press
    v
SHOW_TARGET (3 s)
    |
    v
PLAYER_1_READY (1.5 s, white)
    |
    v
PLAYER_1 (live RGB)
    |
    | Button saves P1
    v
PLAYER_2_READY (1.5 s, white)
    |
    v
PLAYER_2 (live RGB)
    |
    | Button saves P2
    v
JUDGING (1 s)
    |
    v
SHOW_WINNER (3 s)
    |
    v
FREE_MIXING (automatic reset)
```

### Key libraries

```cpp
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"
```

### Running the prototype

1. Install **Arduino IDE**, select **Arduino MKR WiFi 1010**, and select the correct serial port.
2. Install the **WiFiNINA** and **PubSubClient** libraries.
3. Create a local `arduino_secrets.h` alongside the `.ino` sketch with `SECRET_SSID`, `SECRET_PASS`, `SECRET_MQTTUSER` and `SECRET_MQTTPASS` defined. Do not publish the real values.
4. Upload the latest **Colour Duel V6** sketch.
5. Open the Serial Monitor at **9600 baud** to inspect RGB values, connection status, saved colours and scores.
6. Open the [Lumi display](https://www.iot.io/projects/lumi/) and configure it for the correct MQTT topic.
7. Turn the knobs to explore colour, then press the button to start a duel.

## 6. Prototype Development and Iteration

The project evolved through **four main software versions**, with focused hardware and communication tests carried out between milestones. These versions document the progression from open-ended RGB exploration to a complete two-player colour-matching experience.
### Version notes

**Colour Duel v1 — Basic RGB Colour Mixer.** The earliest working prototype focused on physical colour exploration. Rotating the R, G and B potentiometers changed the corresponding channels of the remote LED display in real time. This remains the default, non-game mode of the final project.

**Between v1 and v2 — Button Input Test.** A standalone sketch verified that the push button could be read reliably before it was used to start the game and save player choices. It is stored under `tests/`, rather than treated as a Colour Duel software version.

**Colour Duel v2 — Two-Player Colour Duel.** The project introduced its central game concept: players observe a target colour and take turns recreating it. This version established the interaction sequence but could not yet reliably compare the two results or reset the game.

**Colour Duel v3 — Winner Evaluation and Reset.** Weighted RGB scoring was added so that the lower colour-distance score wins. The result could be shown on the corresponding half of the LED display, and the system returned to free RGB mixing after the winner display.

**Colour Duel v4 — White Preparation and Player Handover.** Each player's turn begins with a short white visual cue before live knob control resumes. This makes the transition between players clearer and reduces the impression that Player 2 simply continues Player 1's colour. **The white cue does not physically reset the potentiometers**; the actual RGB values still depend on their positions.

**A lesson from testing**: The white preparation stage resets the display, not the physical positions of the knobs. I found that a clear visual transition does not necessarily mean the physical inputs have reset. This remains an important interaction limitation to improve.

## 7. References

- [Arduino MKR WiFi 1010 — Official Documentation](https://docs.arduino.cc/hardware/mkr-wifi-1010/)
- [WiFiNINA Library](https://docs.arduino.cc/libraries/wifinina/)
- [PubSubClient MQTT Library](https://pubsubclient.knolleary.net/)
- [Vespera / Lumi Display](https://www.iot.io/projects/lumi/)
- [UCL CASA0014 Workshop Resources](https://workshops.cetools.org/)

---

*Developed for CASA0014 – Prototyping the Internet of Things, University College London.*
