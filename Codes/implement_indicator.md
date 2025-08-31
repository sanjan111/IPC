# implement_indicator.c — Detailed Report

## Overview

`implement_indicator.c` implements non-blocking LED indicator patterns for an 8-bit output (driven via a 74HC595 shift register through `HC595_Load`). The core entry point is `Indicator(uint8_t direction)`, which advances a visual pattern over time without using blocking delays. Timing is driven by timer interrupt flags provided by `timer.c`:

- `LED1_flag`: toggles once per second (≈1 Hz)
- `LED2_flag`: toggles roughly every 1.35 seconds (≈0.74 Hz)

The function is designed to be called repeatedly in the main loop. It advances exactly one pattern step per toggle edge of the associated flag, achieving precise pacing without busy-wait delays.

## External Dependencies

- `indicator.h` — provides `HC595_Load(uint8_t value)` to shift a new 8-bit pattern to the 74HC595 and latch outputs.
- `timer.h` / `timer.c` — provides and updates:
  - `volatile uint8_t LED1_flag` (toggles every 1 s in `TIMER0_IRQHandler`)
  - `volatile uint8_t LED2_flag` (toggles every 1.35 s in `TIMER0_IRQHandler`)

## Function: Indicator(uint8_t direction)

### Persistent State
The function uses static local variables to retain state across calls:

- `prev_flag1`, `prev_flag2`: last-seen values of `LED1_flag`/`LED2_flag` to detect edges.
- `last_dir`: remembers the previously processed `direction` to reset state on changes.
- `idx1` (dir 1), `idx2` (dir 2): index counters for progressive bit filling.
- `step3` (dir 3): step counter for center-out pattern sequencing.
- `pattern`: the current 8-bit output value sent to `HC595_Load`.

Whenever `direction` changes, the function:
- Resets internal indices and step counters
- Clears `pattern` and immediately loads it (turning all LEDs off)
- Initializes `prev_flag1/prev_flag2` to the current flags to avoid an immediate spurious step

### Edge-Triggered Timing
Rather than checking the raw flag level (which would cause multiple steps per call), the code advances only when the flag’s value changes since the last call:

```
if (curr != prev_flagX) { prev_flagX = curr; /* advance one step */ }
```

This treats each toggle (rising OR falling) as a timing “tick.” Given the flags toggle at fixed intervals, the pattern advances once per interval.

### Direction Behaviors

Assume bit positions `[7 6 5 4 3 2 1 0]` map to two 4-LED indicators:
- Left indicator: bits 3..0
- Right indicator: bits 4..7

1) Direction 1 — Left fill MSB→LSB (bits 3→0)
- Timing: 1 step per `LED1_flag` toggle (≈1/s)
- Sequence (one step per second):
  - 0000 0000 → 0000 1000 → 0000 1100 → 0000 1110 → 0000 1111 → 0000 0000 → repeat
- Implementation details:
  - `idx1` starts at 3 and decrements to 0
  - After reaching 0 and loading, the pattern resets to 0 and `idx1` returns to 3

2) Direction 2 — Right fill LSB→MSB (bits 4→7)
- Timing: 1 step per `LED2_flag` toggle (≈1 per 1.35 s)
- Sequence:
  - 0000 0000 → 0001 0000 → 0011 0000 → 0111 0000 → 1111 0000 → 0000 0000 → repeat
- Implementation details:
  - `idx2` starts at 4 and increments to 7
  - After reaching 7 and loading, the pattern resets to 0 and `idx2` returns to 4

3) Direction 3 — Center-out across both indicators
- Timing: 1 step per `LED1_flag` toggle (≈1/s)
- Behavior: turns on symmetric pairs from the middle towards the ends, then clears
- Sequence:
  - Step 0: 0001 1000 (bits {3,4})
  - Step 1: 0011 1100 (add {2,5})
  - Step 2: 0111 1110 (add {1,6})
  - Step 3: 1111 1111 (add {0,7})
  - Step 4: 0000 0000 (clear)
  - Repeat from Step 0
- Implementation details:
  - `step3` advances from 0→4, then resets to 0 and clears `pattern`
  - Each step ORs in the new pair; the final step clears the pattern

### Output Loading
Every time the pattern changes, `HC595_Load(pattern)` transmits the 8-bit value via SSP0/SPI and toggles the latch (P0.16) so that 74HC595 outputs update atomically.

## Visual Timelines (ASCII)

Below, each row represents the pattern value at successive timer edges.

Direction 1 (1 Hz, left fill 3→0):

```
0000 0000
0000 1000
0000 1100
0000 1110
0000 1111
0000 0000
...
```

Direction 2 (~0.74 Hz, right fill 4→7):

```
0000 0000
0001 0000
0011 0000
0111 0000
1111 0000
0000 0000
...
```

Direction 3 (1 Hz, center-out):

```
0001 1000
0011 1100
0111 1110
1111 1111
0000 0000
...
```

## Design Rationale and Properties

- Non-blocking: No busy-waits or `delay_ms`; main loop remains responsive.
- Edge-driven: One step per timer toggle ensures stable pacing independent of loop speed.
- Direction isolation: Internal state resets on direction change to avoid cross-contamination of patterns.
- Simplicity: 8-bit bitmasks match the presumed wiring of two 4-LED indicators.

## Important Considerations

- Call frequency: If `Indicator()` isn’t called for longer than a period (e.g., >1 s), one or more edges may occur between calls and only a single step will be observed — effectively skipping steps. Ensure the main loop calls `Indicator()` frequently relative to the timer period.
- Flag semantics: The flags toggle each period. The code treats any toggle (rising or falling) as a step. If you prefer stepping only on rising edges, modify the comparison to detect a specific edge (e.g., `prev==0 && curr==1`).
- Direction 2 tempo: It advances per `LED2_flag` (~1.35 s). To make it 1 s, use `LED1_flag` instead or change `counter2` in `timer.c` to 1000.
- Bit mapping: The sequences assume [3..0] and [7..4] map logically to left/right indicators. If your hardware wiring differs, adjust bit indices/masks accordingly.
- Concurrency: `pattern` updates and `HC595_Load` are done in the main context; ISRs only toggle flags and update counters. `LED1_flag/LED2_flag` are `volatile` to ensure visibility across ISR/main contexts.

## Potential Enhancements

- Edge selection: Use rising-edge only for clearer semantics: `if (prev_flag1 == 0U && curr == 1U) { ... }`.
- Parametrized tempo: Accumulate ms tick in main and compute arbitrary periods (e.g., faster animations) without changing timer.
- More patterns: Add bounce, wipe, or alternating effects by composing bitmask sequences similar to direction 3.
- Debounce direction changes: Optional delay or freeze between pattern resets on direction changes for smoother transitions.

## Summary

`implement_indicator.c` implements three smooth, non-blocking LED patterns driven by timer-based toggle flags. It relies on edge detection to advance exactly once per timing interval, maintains pattern state across calls, and updates an external 74HC595 shift register to display the current pattern. The added Direction 3 combines Directions 1 and 2 into a symmetric center-out effect while preserving the original behaviors for Directions 1 and 2.

*** End of Report ***
