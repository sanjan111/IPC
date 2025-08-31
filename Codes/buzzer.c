#include "LPC17xx.h"
#include "timer.h"
#include "PWM.h"

/* 20 ms ticker driven in TIMER0 IRQ */
extern volatile uint8_t Buzzer_flag;

void Buzzer(uint8_t direction)
{
    /* Beep only for direction 1 or 2: ON 40 ms, OFF 320 ms, MR1 = 750 */
    typedef enum { BEEP_OFF = 0, BEEP_ON = 1 } beep_state_t;
    static uint8_t  prev_tick = 0U;
    static uint8_t  on_ticks  = 0U;  /* 20 ms units while ON */
    static uint8_t  off_ticks = 0U;  /* 20 ms units while OFF */
    static beep_state_t state = BEEP_OFF;

    uint8_t curr;

    if (direction == 1U || direction == 2U)
    {
        /* Ensure duty (MR1) = 750 and latch */
        LPC_PWM1->MR1 = 750U;
        LPC_PWM1->LER = (PWM_LER_EN_MR0_MASK | PWM_LER_EN_MR1_MASK);

        /* Initialize prev_tick on first invocation after inactivity */
        if (on_ticks == 0U && off_ticks == 0U && state == BEEP_OFF)
        {
            prev_tick = Buzzer_flag;
            /* Start with ON phase for 40 ms */
            LPC_PWM1->TCR = 1; /* start PWM */
            state = BEEP_ON;
        }

        /* Edge detect 20 ms tick and advance state machine */
        {
            curr = Buzzer_flag;
            if ((uint8_t)(curr ^ prev_tick) != 0U)
            {
                prev_tick = curr;
                if (state == BEEP_ON)
                {
                    on_ticks++;
                    if (on_ticks >= 2U) /* 2 * 20 ms = 40 ms */
                    {
                        /* Transition to OFF: stop PWM and drive pin HIGH (active-LOW off) */
                        LPC_PWM1->TCR = 0;
                        LPC_GPIO2->FIOSET = (1U << 11);
                        on_ticks = 0U;
                        state = BEEP_OFF;
                    }
                }
                else /* BEEP_OFF */
                {
                    off_ticks++;
                    if (off_ticks >= 16U) /* 16 * 20 ms = 320 ms */
                    {
                        /* Transition to ON */
                        LPC_PWM1->TCR = 1; /* start PWM */
                        off_ticks = 0U;
                        state = BEEP_ON;
                    }
                }
            }
        }
    }
    else
    {
        /* Not a beeping direction: ensure buzzer is OFF and reset counters */
        LPC_PWM1->TCR = 0;
        LPC_GPIO2->FIOSET = (1U << 11);
        on_ticks = 0U;
        off_ticks = 0U;
        state = BEEP_OFF;
        /* prev_tick left unchanged until next enable */
    }
}
