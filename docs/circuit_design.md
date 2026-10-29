# Circuit Design — Colour Duel

## 1. From RGB Mixer to Colour Duel

My first circuit used **three rotary angle sensors** to control red, green and blue separately. Once this worked, I added a **push button** to turn the mixer into a two-player colour-matching game. I kept the original sensors, so the second version required only one extra input.

## 2. Connections

| Component | Arduino connection | Purpose |
|---|---|---|
| Red rotary sensor | Signal → **A0** | Adjust red |
| Green rotary sensor | Signal → **A1** | Adjust green |
| Blue rotary sensor | Signal → **A2** | Adjust blue |
| All three sensors | VCC → **3.3V**, GND → **GND** | Shared power and ground |
| Push button (added in V2) | **D2 ↔ GND** | Start game / save colour |
| Arduino MKR WiFi 1010 | USB | Power and upload code |

**Important:** The MKR WiFi 1010 uses **3.3V I/O**. I must not send a 5V signal into A0–A2. I also need to check the `VCC`, `GND` and signal labels on each sensor module rather than relying on wire colours.

## 3. How the inputs work

The sensors change their output voltage when rotated. `analogRead()` reads each signal, and my code maps a **10-bit value (0–1023)** to an **RGB intensity (0–255)**. A ten-sample moving average helps reduce small fluctuations.

For the button, I use `pinMode(2, INPUT_PULLUP)`. **Released = HIGH; pressed = LOW.** My program recognises a click on release, with a short debounce delay.

The Arduino sends colour data through **Wi-Fi and MQTT** to **Lumi/Vespera**. The **72 LEDs are the remote visual display**, not LEDs physically wired to this breadboard.

**Related evidence:** `images/circuit_prototype_v1.jpg` and `images/circuit_prototype_v2.jpg`.
