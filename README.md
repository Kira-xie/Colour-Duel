# GripToGlow-Piezo
An interactive IoT prototype developed for UCL CASA0014. A squeezable cardboard controller uses an Arduino MKR WiFi 1010 to trigger a colour wave across 72 Vespera LEDs via MQTT.

**Module:** CASA0014 – Prototyping the Internet of Things  
**Institution:** University College London  
**Student:** Jiahan Xie  
**Development:** October 2026

## 1. Project Overview

Grip to Glow is a tangible Internet of Things (IoT) prototype that explores how physical interactions can be translated into real-time visual feedback.

The project involves designing and constructing a handheld cardboard controller containing a piezoelectric disc sensor. When the user squeezes the controller, the sensor detects mechanical deformation and generates an electrical signal.

An Arduino MKR WiFi 1010 processes the sensor data and publishes RGB colour values through MQTT to Vespera, where 72 LEDs display a responsive colour animation.

The project combines physical prototyping, sensor data processing, wireless communication and interactive lighting design.

## 2. Design Concept

The main concept is to transform a simple squeezing gesture into a dynamic visual experience.

The proposed interaction follows five stages:

1. **Idle:** All 72 LEDs display a soft blue-violet colour.
2. **Light squeeze:** A small piezoelectric pulse triggers a subtle purple colour wave.
3. **Stronger squeeze:** A larger pulse produces a more intense pink-purple wave.
4. **Strong pulse:** Deep red spreads across the LED grid.
5. **Recovery:** The colours gradually return to blue-violet after the movement stops.

The lighting effect is designed to move across the 72-LED arrangement, creating a spatial gradient rather than changing all LEDs to the same colour simultaneously.

**Important distinction:** The piezoelectric sensor primarily detects changes in mechanical deformation, rather than sustained grip force. The proposed interaction therefore responds to squeezing events and their relative signal amplitudes, not calibrated force measurements.

## 3. Hardware and Materials

### Electronic Components

- Arduino MKR WiFi 1010
- Piezoelectric disc sensor
- Breadboard
- Jumper wires
- Resistors, capacitors and protection diodes
- USB data cable

The piezoelectric sensor requires a suitable protection and biasing circuit before connection to the Arduino's 3.3V analog input.

### Physical Prototype

- Corrugated cardboard
- Flexible cardboard or card
- Foam padding
- Adhesive tape or glue
- Velcro or rubber grip material
- Basic cutting and measuring tools

The handheld controller will be constructed using layered cardboard and a flexible squeeze surface.

## 4. System Architecture

**Physical interaction → Piezoelectric sensor → Protected analog input → Arduino MKR WiFi 1010 → Wi-Fi → MQTT Broker → Vespera**

The Arduino processes the piezoelectric signal and generates a 216-byte RGB payload for the Vespera lighting system.

### MQTT Configuration

| Parameter | Configuration |
|---|---|
| MQTT Broker | mqtt.cetools.org |
| Port | 1884 |
| Authentication | Username and password |
| MQTT Topic | student/CASA0014/luminaire/24 |
| LED Count | 72 |
| RGB Payload | 216 bytes |

Authentication credentials are stored locally and excluded from the public GitHub repository.

## 5. Current Progress

**Initial development stage — 26 October 2026**

- Project concept and intended interaction defined.
- Arduino MKR WiFi 1010 selected.
- Piezoelectric disc sensor selected.
- Personal Vespera MQTT topic identified.
- Wi-Fi and MQTT RGB publishing code developed.
- Initial cardboard design and component planning prepared.
- Piezoelectric signal-processing code drafted.

**Hardware testing still required:**

- Validate the sensor protection circuit.
- Measure raw piezoelectric sensor signals.
- Test the physical cardboard structure.
- Verify the complete sensor-to-Vespera interaction.
- Refine the colour animation through user testing.

## 6. Expected Outcome

The intended outcome is a functional physical-digital prototype demonstrating how a simple hand gesture can produce expressive visual feedback through an IoT system.

The project aims to investigate the relationship between tangible interaction, sensor behaviour, network communication and visual expression.

## 7. References

- [UCL CASA0014 Vespera Workshop](https://workshops.cetools.org/codelabs/CASA0014-L5-vespera/)
- [Arduino MKR WiFi 1010 Documentation](https://docs.arduino.cc/hardware/mkr-wifi-1010/)
- [PubSubClient MQTT Library](https://pubsubclient.knolleary.net/)

---

*Developed as part of CASA0014 – Prototyping the Internet of Things at University College London.*
