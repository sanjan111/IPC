#include "LPC17xx.h"
#include "timer.h"
#include "PLL.h"
#include "PWM.h"

extern volatile uint8_t Buzzer_flag;

int main(void)
{
    /* Replace blocking delays with 20 ms ticker from TIMER0.
       Pattern: 200 ms ON (10 ticks) -> 800 ms OFF (40 ticks) */
    typedef enum { BUZZ_ON = 0, BUZZ_OFF = 1 } buzz_state_t;
    uint8_t prev_tick = 0U;
    uint8_t on_ticks = 0U;
    uint8_t off_ticks = 0U;
    buzz_state_t state = BUZZ_ON; /* start ON like original code */

    PLL_Init();
	Timer_Init();
    LPC_GPIO2->FIODIR |= (1 << 11);
    PWM_Init();

    /* Initialize tick sampler and ensure starting state is ON */
    prev_tick = Buzzer_flag;
    LPC_PWM1->TCR = 1; /* start PWM (buzzer ON) */

	while(1)
	{
        /* Edge detect 20 ms tick */
        uint8_t curr = Buzzer_flag;
        if ((uint8_t)(curr ^ prev_tick) != 0U)
        {
            prev_tick = curr; /* one 20 ms has elapsed */

            if (state == BUZZ_ON)
            {
                on_ticks++;
                if (on_ticks >= 10U) /* 10 * 20 ms = 200 ms */
                {
                    /* Turn OFF for 800 ms */
                    LPC_PWM1->TCR = 0;                 /* stop PWM counter */
                    LPC_GPIO2->FIOSET = (1 << 11);     /* ensure buzzer OFF (active-LOW) */
                    on_ticks = 0U;
                    state = BUZZ_OFF;
                }
            }
            else /* BUZZ_OFF */
            {
                off_ticks++;
                if (off_ticks >= 40U) /* 40 * 20 ms = 800 ms */
                {
                    /* Back to ON for 200 ms */
                    LPC_PWM1->TCR = 1; /* start PWM */
                    off_ticks = 0U;
                    state = BUZZ_ON;
                }
            }
        }
        /* No blocking delay; loop reacts only on 20 ms edges */
	}

}
