/*
 * Simple simulation to demonstrate edge detection in Indicator().
 * - Stubs HC595_Load() to print the pattern and timestamp instead of driving hardware.
 * - Simulates TIMER0 toggling LED1_flag and LED2_flag at fixed intervals.
 * - Calls Indicator(direction) every 1 ms to show that steps advance only on edges.
 *
 * Build (host machine with GCC/Clang):
 *   gcc -std=c99 -O2 -o sim Codes/sim_indicator_edge_demo.c
 * Run:
 *   ./sim
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Provide the flags expected by implement_indicator.c */
volatile uint8_t LED1_flag = 0U; /* toggles every 400 ms in this sim */
volatile uint8_t LED2_flag = 0U; /* toggles every 100 ms in this sim */

/* Mock the hardware loader used by implement_indicator.c */
void HC595_Load(uint8_t value)
{
    /* Print as 8-bit binary for clarity */
    char bits[9];
    for (int i = 7; i >= 0; --i) { bits[7 - i] = (char)('0' + ((value >> i) & 1U)); }
    bits[8] = '\0';
    static uint32_t now_ms = 0; /* updated in the simulation loop via a setter */
    extern void __sim_set_time(uint32_t t);
    extern uint32_t __sim_get_time(void);
    printf("t=%4u ms  pattern=%s (0x%02X)\n", __sim_get_time(), bits, value);
}

/* Pull in the real logic under test. This file expects LED1_flag/LED2_flag and HC595_Load(). */
/* NOTE: We include the C file directly to avoid linking hardware drivers. */
#include "implement_indicator.c"

/* Simple time keeper for printing */
static uint32_t g_now_ms = 0U;
void __sim_set_time(uint32_t t) { g_now_ms = t; }
uint32_t __sim_get_time(void) { return g_now_ms; }

static void step_time_ms(uint32_t dt)
{
    /* Advance simulated time in 1 ms increments and run the app loop */
    for (uint32_t i = 0; i < dt; ++i)
    {
        g_now_ms++;
        /* Toggle flags at fixed periods: LED1 every 400 ms, LED2 every 100 ms */
        if ((g_now_ms % 400U) == 0U) { LED1_flag ^= 1U; }
        if ((g_now_ms % 100U) == 0U) { LED2_flag ^= 1U; }

        /* Call the non-blocking Indicator every 1 ms like a main loop would */
        Indicator(1U); /* Try direction 1 first; change below for other demos */
    }
}

int main(void)
{
    printf("\nDemo 1: Direction=1 (lower nibble MSB->LSB), paced by LED1_flag edges (~400 ms)\n");
    g_now_ms = 0U; LED1_flag = 0U; LED2_flag = 0U;
    /* Run for ~4 seconds */
    step_time_ms(4000U);

    printf("\nDemo 2: Direction=2 (upper nibble LSB->MSB), paced by LED2_flag edges (~100 ms)\n");
    /* Reset state inside Indicator by switching direction in-place */
    for (int i = 0; i < 10; ++i)
    {
        /* Switch to direction 2 for this phase */
        g_now_ms++;
        if ((g_now_ms % 400U) == 0U) { LED1_flag ^= 1U; }
        if ((g_now_ms % 100U) == 0U) { LED2_flag ^= 1U; }
        Indicator(2U);
    }
    /* Continue running dir=2 for ~1.2 s */
    for (uint32_t i = 0; i < 1200U; ++i)
    {
        g_now_ms++;
        if ((g_now_ms % 400U) == 0U) { LED1_flag ^= 1U; }
        if ((g_now_ms % 100U) == 0U) { LED2_flag ^= 1U; }
        Indicator(2U);
    }

    printf("\nDemo 3: Direction=3 (center-out), paced by LED1_flag edges (~400 ms)\n");
    /* Small transition calls to reset state to dir=3 */
    for (int i = 0; i < 10; ++i)
    {
        g_now_ms++;
        if ((g_now_ms % 400U) == 0U) { LED1_flag ^= 1U; }
        if ((g_now_ms % 100U) == 0U) { LED2_flag ^= 1U; }
        Indicator(3U);
    }
    /* Run ~2.5 seconds */
    for (uint32_t i = 0; i < 2500U; ++i)
    {
        g_now_ms++;
        if ((g_now_ms % 400U) == 0U) { LED1_flag ^= 1U; }
        if ((g_now_ms % 100U) == 0U) { LED2_flag ^= 1U; }
        Indicator(3U);
    }

    printf("\nNotes:\n");
    printf("- Even though Indicator() runs every 1 ms, it only steps when LEDx_flag toggles.\n");
    printf("- That edge (curr != prev) occurs at the configured periods (400/100 ms here).\n");
    printf("- Therefore the visible step-to-step delay is set by the timer, not by the loop speed.\n");
    return 0;
}

