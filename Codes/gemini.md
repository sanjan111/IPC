# Detailed Report on implement_indicator.c

## 1. Overview

The file `implement_indicator.c` implements a non-blocking state machine to control a series of 8 indicators connected to a 74HC595 shift register. The term "non-blocking" means it performs small updates and returns, allowing other code in the main loop to run without being delayed.

It creates animated visual patterns by changing the state of the indicators over time. The speed of these animations is driven by external timer flags that are expected to be updated in a timer interrupt service routine (ISR).

## 2. Dependencies

### Software Dependencies:
- **`indicator.h`**: Provides the low-level function `HC595_Load()` to send data to the 74HC595 shift register via the SPI protocol.
- **`timer.h`**: Declares the external timer flags `LED1_flag` and `LED2_flag`. These flags are the "heartbeat" of the animations.
- **Timer Module**: A hardware timer (e.g., TIMER0 on the LPC17xx) must be configured to periodically toggle `LED1_flag` and `LED2_flag`. The comments suggest:
    - `LED1_flag` toggles every 1 second.
    - `LED2_flag` toggles every 1.35 seconds.

### Hardware Dependencies:
- **LPC17xx Microcontroller**: The code is written for an NXP LPC17xx series MCU.
- **74HC595 Shift Register**: An 8-bit serial-in, parallel-out shift register that drives the 8 indicators.
- **8 Indicators**: Typically LEDs, connected to the output pins of the 74HC595.

## 3. Core Function: `Indicator(uint8_t direction)`

This is the only function in the file. It must be called repeatedly from the main application loop (`while(1)`).

### Key Characteristics:
- **Stateful**: It uses `static` local variables to remember its state across multiple calls. This includes the current pattern, the animation step/index, and the previous state of the timer flags.
- **Edge-Triggered**: The logic inside the function only runs when it detects a change (an edge) in the `LED1_flag` or `LED2_flag`. This is done by comparing the current flag value with the `prev_flag` stored from the last call.
- **Mode Switching**: If the `direction` parameter changes, the function resets its internal state machine, clears the indicator display, and starts the new pattern from the beginning.

## 4. Indicator Patterns (Directions)

The `direction` parameter selects one of three predefined animation patterns.

---

### Direction 1: Fill Lower Nibble (MSB to LSB)

- **Trigger**: `LED1_flag` (1-second interval).
- **Pattern**: Lights up LEDs from bit 3 down to bit 0, one at a time, holding the previous ones on. After all are lit, it clears them and restarts.

**Visual Sequence:**

Let's represent the 8 indicators as `(7 6 5 4 3 2 1 0)`.

```
Step 1 (1s): (0 0 0 0 1 0 0 0)  // Bit 3 ON
Step 2 (2s): (0 0 0 0 1 1 0 0)  // Bit 2 ON
Step 3 (3s): (0 0 0 0 1 1 1 0)  // Bit 1 ON
Step 4 (4s): (0 0 0 0 1 1 1 1)  // Bit 0 ON
Step 5 (5s): (0 0 0 0 0 0 0 0)  // Clear and Restart
```

---

### Direction 2: Fill Upper Nibble (LSB to MSB)

- **Trigger**: `LED2_flag` (1.35-second interval).
- **Pattern**: Lights up LEDs from bit 4 up to bit 7, one at a time, holding the previous ones on. After all are lit, it clears them and restarts.

**Visual Sequence:**

```
Step 1 (1.35s): (0 0 0 1 0 0 0 0) // Bit 4 ON
Step 2 (2.70s): (0 0 1 1 0 0 0 0) // Bit 5 ON
Step 3 (4.05s): (0 1 1 1 0 0 0 0) // Bit 6 ON
Step 4 (5.40s): (1 1 1 1 0 0 0 0) // Bit 7 ON
Step 5 (6.75s): (0 0 0 0 0 0 0 0) // Clear and Restart
```

---

### Direction 3: Center-Out Pattern

- **Trigger**: `LED1_flag` (1-second interval).
- **Pattern**: Lights up LEDs starting from the center pair (3 and 4) and expanding outwards. After all are lit, it clears them and restarts.

**Visual Sequence:**

```
Step 1 (1s): (0 0 0 1 1 0 0 0)  // Bits 3 & 4 ON
Step 2 (2s): (0 0 1 1 1 1 0 0)  // Add Bits 2 & 5
Step 3 (3s): (0 1 1 1 1 1 1 0)  // Add Bits 1 & 6
Step 4 (4s): (1 1 1 1 1 1 1 1)  // Add Bits 0 & 7 (All ON)
Step 5 (5s): (0 0 0 0 0 0 0 0)  // Clear and Restart
```

## 5. Low-Level Driver: `indicator.c`

The `implement_indicator.c` module relies on `indicator.c` to handle the hardware communication.

- **`SPI_Init()`**: This function must be called once at startup to configure the SSP0 (SPI) peripheral on the LPC17xx. It sets up the necessary pins (SCK, MOSI) and the GPIO for the 74HC595 latch control.
- **`HC595_Load(uint8_t value)`**: This is the key function used by `Indicator()`. It takes an 8-bit pattern, sends it serially to the shift register via SPI, and then pulses the latch pin. This pulse causes the 74HC595 to update its parallel outputs, making the new pattern visible on the indicators.

## 6. How to Use

1.  **Initialization**:
    - In your `main()` function, call `SPI_Init()` to set up the SPI hardware.
    - Configure a hardware timer (e.g., TIMER0) to generate interrupts.
    - In the timer's ISR, toggle the global volatile flags `LED1_flag` and `LED2_flag` at the desired intervals (e.g., 1s and 1.35s).

    ```c
    // In main.c
    #include "timer.h"
    #include "indicator.h"
    #include "implement_indicator.h"

    volatile uint8_t LED1_flag = 0;
    volatile uint8_t LED2_flag = 0;

    void TIMER0_IRQHandler(void) {
        // ... clear interrupt flag ...
        // Logic to toggle flags at correct intervals
        // This is a simplified example
        static int count = 0;
        count++;
        if (count % 100 == 0) { // Assuming 10ms timer tick for 1s
             LED1_flag = !LED1_flag;
        }
        if (count % 135 == 0) { // For 1.35s
             LED2_flag = !LED2_flag;
        }
    }

    int main(void) {
        // ... system initialization ...
        SPI_Init();
        Timer_Init(); // Your function to init TIMER0

        uint8_t current_direction = 1;

        while(1) {
            // Your main application logic here...
            // e.g., read buttons to change direction
            // if (button1_pressed) current_direction = 1;
            // if (button2_pressed) current_direction = 2;

            // Call Indicator() continuously
            Indicator(current_direction);
        }
        return 0;
    }
    ```

2.  **Main Loop**:
    - Call the `Indicator(direction)` function continuously inside your main `while(1)` loop.
    - Pass the desired pattern number (1, 2, or 3) as the `direction` argument. You can change this variable at any time to switch patterns.

## 7. Conclusion

`implement_indicator.c` provides a clean and efficient way to manage complex, timed indicator patterns without blocking the main CPU tasks. Its state machine design makes it easy to manage, and the separation from the low-level SPI driver (`indicator.c`) is good practice. The behavior is entirely dependent on the periodic toggling of external flags, making it highly reliant on a correctly configured timer module.
