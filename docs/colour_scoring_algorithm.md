# Colour Scoring Algorithm — Colour Duel

*UCL CASA0014 · Technical notes for Mode B (Two-Player Colour Duel)*

## 1. Why did I need a formula?

In **Mode A**, users can freely mix RGB colours, so there is no correct answer. In **Mode B**, two players try to match the same target colour. After both save their selections, I need one consistent rule to decide whose RGB values are closer to that target.

I use a **weighted squared RGB distance**:

```text
Score = 299(R − Rt)² + 587(G − Gt)² + 114(B − Bt)²
```

- **R, G, B:** a player's saved values (0–255).
- **Rt, Gt, Bt:** the target's values (0–255).
- **Lower score = closer match under this game's rule.** Equal scores mean a draw.

The differences are squared so negative and positive errors cannot cancel each other out, and large mistakes count more. I do not need a square root because it would not change which score is smaller.

## 2. Where did 299, 587 and 114 come from?

The coefficients were **adapted from the ITU-R BT.601 video standard** (ITU-R, 2011), which uses these proportions to calculate a *luma* signal:

```text
Y′ = 0.299R′ + 0.587G′ + 0.114B′
```

These numbers sum to 1.000. In this video representation, green contributes most to luma (58.7%), followed by red (29.9%) and blue (11.4%). The coefficients reflect how the video system represents brightness from its RGB primaries, taking human visual sensitivity into account. They do not mean that everyone's eyes always perceive a green error as exactly 58.7% of a colour difference.

![RGB scoring weights](../images/scoring_weights.png)

**My adaptation:** BT.601 uses these weights to *combine colour channels into luma*. Colour Duel uses the same proportions to **weight squared differences between two colours**. That second step is my game-design choice, not a colour-distance formula published in BT.601.

## 3. Why not use different numbers?

I *could*. For example:

| Rule | Weights (R : G : B) | What changes? |
|---|---|---|
| **Colour Duel** | **299 : 587 : 114** | Green errors have the largest effect. |
| Equal weighting | 1 : 1 : 1 | All three channel errors matter equally. |

For a target of `(100,100,100)`, compare a player who is **10 too red** with one who is **10 too blue**:

- Colour Duel: red-error score = `299 × 10² = 29,900`; blue-error score = `114 × 10² = 11,400` → **the blue-error player wins**.
- Equal weighting: both scores are `10² = 100` → **a draw**.

This shows that **changing relative weights can change the winner**. Multiplying *all* weights by the same positive number would only rescale scores, not change the result.

## 4. One example round

The following values are illustrative, not recorded gameplay data:

| | RGB values | Score |
|---|---|---:|
| Target | (180, 100, 60) | — |
| Player 1 | (170, 110, 70) | **100,000** |
| Player 2 | (210, 95, 70) | **295,175** |

```text
Player 1: 299×10² + 587×10² + 114×10² = 100,000
Player 2: 299×30² + 587×5² + 114×10² = 295,175
```

![Worked scoring example](../images/scoring_example.png)

**Player 1 wins** because `100,000 < 295,175`. In the current game, the winner's half of the Lumi/Vespera display flashes three times before returning to Mode A.

## 5. What I learnt

At first, I thought comparing two colours would be straightforward. Writing the scoring system taught me that **an algorithm follows the definition of “closer” that I choose**. This rule is simple to explain and run on Arduino, and it treats both players consistently, but **RGB is not a perceptually uniform colour space**.

## References

- **ITU-R (2011).** *Recommendation ITU-R BT.601-7: Studio encoding parameters of digital television*. [Official standard](https://www.itu.int/rec/r-rec-bt.601/) · [PDF, especially p. 4](https://www.itu.int/dms_pubrec/itu-r/rec/bt/r-rec-bt.601-7-201103-i!!pdf-e.pdf).
- **ISO/CIE (2022).** *ISO/CIE 11664-6:2022 — Colorimetry, Part 6: CIEDE2000 colour-difference formula*. [Official overview](https://www.cie.co.at/publications/colorimetry-part-6-ciede2000-colour-difference-formula-1).

