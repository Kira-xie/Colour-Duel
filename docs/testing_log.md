# Testing and Iteration Log — Colour Duel

I tested Colour Duel in stages, starting with individual inputs and then building the full game. This log records the main changes and what I learnt.

| Stage | Test / problem | Improvement or result |
|---|---|---|
| **Wi-Fi & MQTT tests** | Arduino could connect, but the lights did not always respond as expected. | Checked the network, MQTT topic and LED output separately rather than assuming a connection meant success. |
| **v1 — RGB exploring** | Tested three rotary sensors as a basic colour mixer. | Mapped analogue values (0–1023) to RGB (0–255); added a **10-sample moving average** to reduce small fluctuations. |
| **v2 — Button added** | Added a button and two-player turns. Early gameplay could save colours but did not reliably judge a winner or reset. | Tested the button separately and identified the need for a complete game sequence. |
| **v3 — Scoring and reset** | The game needed to compare two saved colours. | Added weighted RGB colour-distance scoring, winner feedback and automatic reset. |
| **v4 — Player preparation** | An early white-threshold approach could leave the game waiting. | Replaced it with a **1.5-second white preparation display** for each player. |
| **v5 — Winner animation** | The result needed clearer feedback. | Made the winning half of the LEDs flash **three times** before returning to free mixing. |
| **Final — Dark target colours** | Some randomly generated target colours were too pale. | Limited the random RGB range and total channel values to favour darker colours. |

## Remaining Limitations

**Physical knob positions:** The white screen resets the *display*, but the knobs remain where the previous player left them. This needs a better player-handover design.

**Vespera display limitations:** During testing, I noticed that lighter colours were less distinguishable on Vespera, even though the luminaries displayed them correctly. This made me realise that physical display limitations can affect the user experience. I therefore adjusted the random target colours towards darker shades to improve their visibility.
