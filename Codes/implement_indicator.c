/* Non-blocking indicator pattern using timer interrupt flag.
 * Advances one step per 1-second TIMER0 event (LED1_flag toggles).
 */
#include "indicator.h"
#include <stdint.h>
#include "timer.h"

extern volatile uint8_t LED1_flag; /* toggles every 1 s in TIMER0 IRQ */
extern volatile uint8_t LED2_flag; /* toggles every 1.35 s in TIMER0 IRQ */

void Indicator(uint8_t direction)
{
    /* Persist state across calls */
    static uint8_t prev_flag = 0U;   /* for edge detection */
    static int8_t  idx       = 3;    /* bit index: 3 -> 0 */
    static int8_t  irx       = 4; 
    static uint8_t pattern   = 0U;   /* 74HC595 output pattern */
	
	uint8_t curr_flag;

    if (direction == 1)
    {
        /* Detect any edge (0->1 or 1->0) of the 1 Hz flag.
        The ISR toggles LED1_flag every second, so any change = 1 s elapsed. */
        curr_flag = LED1_flag;
        if (curr_flag != prev_flag)
        {
            prev_flag = curr_flag; /* consume the edge */

            /* Advance exactly one step per second */
            if (idx >= 0)
            {
                pattern |= (uint8_t)(1U << idx);
                HC595_Load(pattern);
                idx--;
            }
            else
            {
                /* Sequence complete: reset to restart the loading effect */
                idx     = 3;
                pattern = 0U;
                HC595_Load(pattern);
            }
        }
    }

    if (direction == 2)
    {
        /* Detect any edge (0->1 or 1->0) of the 1 Hz flag.
        The ISR toggles LED1_flag every second, so any change = 1 s elapsed. */
        curr_flag = LED2_flag;
        if (curr_flag != prev_flag)
        {
            prev_flag = curr_flag; /* consume the edge */

            /* Advance exactly one step per second */
            if (irx <= 7)
            {
                pattern |= (uint8_t)(1U << irx);
                HC595_Load(pattern);
                irx++;
            }
            else
            {
                /* Sequence complete: reset to restart the loading effect */
                irx     = 4;
                pattern = 0U;
                HC595_Load(pattern);
            }
        }
    }

    if (direction == 3)
    {
        
    }
}
