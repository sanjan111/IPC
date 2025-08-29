/* Non-blocking indicator patterns using timer interrupt flags.
 * Direction 1: fill lower nibble from MSB->LSB (bits 3..0).
 * Direction 2: fill upper nibble from LSB->MSB (bits 4..7).
 * Direction 3: center-out across both (3&4 -> 2&5 -> 1&6 -> 0&7 -> clear).
 */
#include "indicator.h"
#include <stdint.h>
#include "timer.h"

extern volatile uint8_t LED1_flag; /* toggles every 1 s in TIMER0 IRQ */
extern volatile uint8_t LED2_flag; /* toggles every 1.35 s in TIMER0 IRQ */

void Indicator(uint8_t direction)
{
    /* Persist state across calls */
    static uint8_t prev_flag1 = 0U;  /* edge detect for LED1_flag */
    static uint8_t prev_flag2 = 0U;  /* edge detect for LED2_flag */
    static uint8_t last_dir   = 0U;  /* reset state on change */
    static int8_t  idx1       = 3;   /* dir1 index: 3 -> 0 */
    static int8_t  idx2       = 4;   /* dir2 index: 4 -> 7 */
    static uint8_t step3      = 0U;  /* dir3 step: 0..3 then clear */
    static uint8_t pattern    = 0U;  /* 74HC595 output pattern */

    /* Reset per-direction state if direction changed */
    if (direction != last_dir)
    {
        last_dir   = direction;
        prev_flag1 = LED1_flag; /* avoid immediate step on change */
        prev_flag2 = LED2_flag;
        pattern    = 0U;
        HC595_Load(pattern);
        idx1 = 3; idx2 = 4; step3 = 0U;
    }

    if (direction == 1U)
    {
        uint8_t curr = LED1_flag;
        if (curr != prev_flag1)
        {
            prev_flag1 = curr;
            if (idx1 >= 0)
            {
                pattern |= (uint8_t)(1U << idx1);
                HC595_Load(pattern);
                idx1--;
            }
            else
            {
                idx1    = 3;
                pattern = 0U;
                HC595_Load(pattern);
            }
        }
    }
    else if (direction == 2U)
    {
        uint8_t curr = LED2_flag;
        if (curr != prev_flag2)
        {
            prev_flag2 = curr;
            if (idx2 <= 7)
            {
                pattern |= (uint8_t)(1U << idx2);
                HC595_Load(pattern);
                idx2++;
            }
            else
            {
                idx2    = 4;
                pattern = 0U;
                HC595_Load(pattern);
            }
        }
    }
    else if (direction == 3U)
    {
        /* Use 1-second pace on center-out combined pattern */
        uint8_t curr = LED1_flag;
        if (curr != prev_flag1)
        {
            prev_flag1 = curr;
            switch (step3)
            {
                case 0U:
                    /* middle pair: bits 3 and 4 */
                    pattern |= (uint8_t)((1U << 3) | (1U << 4));
                    HC595_Load(pattern);
                    step3 = 1U;
                    break;
                case 1U:
                    /* extend: add bits 2 and 5 */
                    pattern |= (uint8_t)((1U << 2) | (1U << 5));
                    HC595_Load(pattern);
                    step3 = 2U;
                    break;
                case 2U:
                    /* extend: add bits 1 and 6 */
                    pattern |= (uint8_t)((1U << 1) | (1U << 6));
                    HC595_Load(pattern);
                    step3 = 3U;
                    break;
                case 3U:
                    /* extend: add bits 0 and 7 -> all on */
                    pattern |= (uint8_t)((1U << 0) | (1U << 7));
                    HC595_Load(pattern);
                    step3 = 4U;
                    break;
                default:
                    /* clear and restart */
                    pattern = 0U;
                    HC595_Load(pattern);
                    step3 = 0U;
                    break;
            }
        }
    }
    else
    {
        /* Unknown direction: keep outputs as-is */
        (void)0;
    }
}
