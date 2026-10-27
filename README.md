# Echo of Touch
### Translating Physical Gestures into Colour Waves through IoT

**Module:** CASA0014 – Prototyping the Internet of Things  
**Institution:** University College London (UCL)  
**Student:** Jiahan Xie  
**Development Period:** October 2026 – Present  
**Project Status:** Physical prototyping and hardware validation in progress

---

## 1. Project Overview

**Echo of Touch** is a tangible Internet of Things (IoT) prototype that explores how brief physical interactions can be transformed into expressive visual feedback across a connected environment.

The project uses a handmade cardboard interface containing a piezoelectric disc sensor. Physical interactions such as tapping, pressing or gently deforming the controller generate electrical signals through the piezoelectric effect.

An Arduino MKR WiFi 1010 is used to process these signals and communicate with the Vespera lighting system through Wi-Fi and MQTT. The intended output is a responsive colour animation across 72 LEDs.

Rather than simply switching lights on and off, Echo of Touch explores how a momentary physical gesture can leave a temporary visual trace, resembling a ripple or an echo.

The project combines physical prototyping, sensor experimentation, signal processing, wireless communication and interactive lighting design.

## 2. Design Concept

### From Physical Contact to Visual Echo

The central idea is to translate a short physical gesture into a gradually expanding and fading wave of colour.

A user interacts with the cardboard controller through a light tap, press or squeeze. The piezoelectric sensor detects changes in mechanical stress and generates a transient electrical signal.

The Arduino interprets the detected pulse and maps its amplitude to the intensity of the lighting response.

### Proposed Interaction

1. **Idle:** All 72 LEDs display a soft blue-violet colour.
2. **Gentle interaction:** A small detected pulse triggers a subtle purple colour transition.
3. **Moderate interaction:** A larger pulse creates a more noticeable pink-purple wave.
4. **Stronger pulse:** A more intense colour wave spreads across the LED grid, gradually approaching deep red.
5. **Recovery:** The colours smoothly return to their original blue-violet state.

The proposed animation progresses across the 12-column × 6-row LED grid, creating a spatial colour gradient rather than changing all LEDs simultaneously.

### Understanding the Sensor

A piezoelectric disc responds primarily to changes in mechanical stress, such as tapping, vibration or bending.

It is not designed to provide a stable measurement of sustained grip force.

Therefore, Echo of Touch is intended to respond to physical interaction events and their relative signal amplitudes, rather than measure precise force or pressure.

Different gestures may generate similar electrical signals. The prototype does not claim to identify individual gesture types without further signal analysis and testing.

## 3. Hardware and Materials

### Electronic Components

- Arduino MKR WiFi 1010
- Piezoelectric disc sensor
- Breadboard
- Jumper wires
- 100kΩ and 1MΩ resistors
- 100µF capacitor
- Input protection components (under evaluation)
- USB data cable

The Arduino MKR WiFi 1010 uses 3.3V logic. Because piezoelectric discs can generate transient positive and negative voltage spikes, a suitable input protection and biasing circuit is required before the sensor can be safely connected to the analogue input.

### Physical Prototype

- Corrugated cardboard
- Cardboard support layers
- Foam or cushioning material
- Adhesive tape or glue
- Flexible joining materials
- Cutting and measuring tools

The initial controller uses two layers of corrugated cardboard separated by a supporting structure.

The design aims to allow limited and repeatable mechanical deformation while protecting the fragile piezoelectric ceramic and its electrical connections.

The first cardboard prototype has been assembled. Its mechanical sensitivity and suitability for the sensor will be evaluated through physical testing.

## 4. System Architecture

The proposed communication process is:

**Physical gesture → Piezoelectric disc → Input protection and biasing circuit → Arduino MKR WiFi 1010 → Wi-Fi → MQTT Broker → Vespera**

The Arduino is intended to sample the piezoelectric signal, detect transient changes and map their relative amplitude to an RGB animation.

The Vespera system uses 72 LEDs, with three bytes representing the red, green and blue components of each LED. Each complete frame therefore contains 216 bytes.

### MQTT Configuration

| Parameter | Configuration |
|---|---|
| MQTT Broker | `mqtt.cetools.org` |
| Port | `1884` |
| Authentication | MQTT username and password |
| MQTT Topic | `student/CASA0014/luminaire/24` |
| LED Count | 72 |
| RGB Payload | 216 bytes |

Wi-Fi and MQTT connection credentials are stored locally and excluded from the public GitHub repository.

## 5. Prototype Development

### Stage 1 — Concept Development

The project initially explored the relationship between grip strength and lighting intensity under the name *Grip to Glow*.

Following investigation of the piezoelectric disc, the concept was refined to focus on short physical interaction events rather than sustained force measurement.

The revised concept, *Echo of Touch*, better reflects the physical behaviour of the selected sensor and the proposed ripple-like animation.

Initial design sketches have been created to explore the controller's shape, internal support structure and interaction method.

### Stage 2 — Wi-Fi and MQTT Communication

Initial Arduino experiments established Wi-Fi and MQTT communication using the MKR WiFi 1010.

RGB data publishing to the personal Vespera MQTT topic has been tested independently of the piezoelectric sensor.

These tests provide a foundation for integrating real sensor input later.

### Stage 3 — Cardboard Prototype V1

The first physical cardboard prototype has been constructed using layered corrugated cardboard and a central supporting element.

This early version is intended to investigate:

- How the controller responds to pressing and tapping.
- Whether the internal structure allows small mechanical movements.
- How the piezoelectric disc can be supported without excessive force on the ceramic surface.
- How electrical wires can exit the enclosure without strain on solder joints.

The current support material is relatively rigid, and softer cushioning materials may be explored in subsequent iterations.

### Stage 4 — Breadboard Circuit V1

An initial breadboard assembly has been prepared to explore a mid-supply bias voltage for the Arduino's analogue input.

The planned static bias circuit uses two 100kΩ resistors to generate approximately 1.65V from a 3.3V supply, together with a capacitor and an analogue input connection.

**This is an unverified static bias circuit, not a completed piezoelectric protection circuit.**

Power connections, bias voltage, component polarity and analogue input levels require electrical verification before dynamic piezoelectric testing.

### Stage 5 — Sensor Testing and Integration (Planned)

The next development stage will involve:

- Verifying the 3.3V supply and static bias circuit.
- Validating the piezoelectric input protection arrangement.
- Recording raw sensor readings for different physical interactions.
- Identifying transient peaks and comparing signal-processing methods.
- Mapping detected events to colour-wave intensity.
- Testing the complete Arduino–MQTT–Vespera interaction.
- Evaluating the responsiveness and usability of the cardboard controller.

## 6. References

- [UCL CASA0014 Vespera Workshop](https://workshops.cetools.org/codelabs/CASA0014-L5-vespera/)
- [Arduino MKR WiFi 1010 Documentation](https://docs.arduino.cc/hardware/mkr-wifi-1010/)
- [Arduino Piezo Knock Sensor Tutorial](https://docs.arduino.cc/built-in-examples/sensors/Knock/)
- [PubSubClient MQTT Library](https://pubsubclient.knolleary.net/)

---

*Developed as part of CASA0014 – Prototyping the Internet of Things, University College London.*
