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
    static uint8_t started = 0U;    /* ensures init runs only once while active */

    /* Separate state for direction == 3 (20 ms ON, 2000 ms OFF) */
    static uint8_t  prev_tick3 = 0U;
    static uint8_t  on_ticks3  = 0U;
    static uint8_t  off_ticks3 = 0U;
    static beep_state_t state3  = BEEP_OFF;
    static uint8_t started3     = 0U;

    /* Separate state for direction == 4 (200 ms ON, 800 ms OFF) */
    static uint8_t  prev_tick4 = 0U;
    static uint8_t  on_ticks4  = 0U;
    static uint8_t  off_ticks4 = 0U;
    static beep_state_t state4  = BEEP_OFF;
    static uint8_t started4     = 0U;

    uint8_t curr;

    if (direction == 1U || direction == 2U)
    {
        /* Ensure duty (MR1) = 750 and latch */
        LPC_PWM1->MR1 = 750U;
        LPC_PWM1->LER = (PWM_LER_EN_MR0_MASK | PWM_LER_EN_MR1_MASK);

        /* Initialize only once when beeping becomes active */
        if (started == 0U)
        {
            prev_tick = Buzzer_flag;
            LPC_PWM1->TCR = (PWM_TCR_COUNTER_ENABLE_MASK | PWM_TCR_PWM_ENABLE_MASK); /* start PWM */
            state = BEEP_ON;
            on_ticks = 0U;
            off_ticks = 0U;
            started = 1U;
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
                        LPC_PWM1->TCR = (PWM_TCR_COUNTER_ENABLE_MASK | PWM_TCR_PWM_ENABLE_MASK); /* start PWM */
                        off_ticks = 0U;
                        state = BEEP_ON;
                    }
                }
            }
        }
    }
    else if (direction == 3U)
    {
        /* Ensure duty (MR1) = 200 and latch */
        LPC_PWM1->MR1 = 200U;
        LPC_PWM1->LER = (PWM_LER_EN_MR0_MASK | PWM_LER_EN_MR1_MASK);

        /* Initialize only once when this mode becomes active */
        if (started3 == 0U)
        {
            prev_tick3 = Buzzer_flag;
            LPC_PWM1->TCR = (PWM_TCR_COUNTER_ENABLE_MASK | PWM_TCR_PWM_ENABLE_MASK);
            state3 = BEEP_ON;
            on_ticks3 = 0U;
            off_ticks3 = 0U;
            started3 = 1U;
        }

        /* 20 ms edge tick */
        {
            uint8_t curr3 = Buzzer_flag;
            if ((uint8_t)(curr3 ^ prev_tick3) != 0U)
            {
                prev_tick3 = curr3;
                if (state3 == BEEP_ON)
                {
                    on_ticks3++;
                    if (on_ticks3 >= 1U) /* 1 * 20 ms = 20 ms */
                    {
                        LPC_PWM1->TCR = 0;                 /* stop PWM */
                        LPC_GPIO2->FIOSET = (1U << 11);    /* active-LOW off */
                        on_ticks3 = 0U;
                        state3 = BEEP_OFF;
                    }
                }
                else /* BEEP_OFF */
                {
                    off_ticks3++;
                    if (off_ticks3 >= 100U) /* 100 * 20 ms = 2000 ms */
                    {
                        LPC_PWM1->TCR = (PWM_TCR_COUNTER_ENABLE_MASK | PWM_TCR_PWM_ENABLE_MASK);
                        off_ticks3 = 0U;
                        state3 = BEEP_ON;
                    }
                }
            }
        }
    }
    else if (direction == 4U)
    {
        /* Ensure duty (MR1) = 1400 and latch */
        LPC_PWM1->MR1 = 1400U;
        LPC_PWM1->LER = (PWM_LER_EN_MR0_MASK | PWM_LER_EN_MR1_MASK);

        /* Initialize only once when this mode becomes active */
        if (started4 == 0U)
        {
            prev_tick4 = Buzzer_flag;
            LPC_PWM1->TCR = (PWM_TCR_COUNTER_ENABLE_MASK | PWM_TCR_PWM_ENABLE_MASK);
            state4 = BEEP_ON;
            on_ticks4 = 0U;
            off_ticks4 = 0U;
            started4 = 1U;
        }

        /* 20 ms edge tick: 10 ticks ON (200 ms), 40 ticks OFF (800 ms) */
        {
            uint8_t curr4 = Buzzer_flag;
            if ((uint8_t)(curr4 ^ prev_tick4) != 0U)
            {
                prev_tick4 = curr4;
                if (state4 == BEEP_ON)
                {
                    on_ticks4++;
                    if (on_ticks4 >= 10U)
                    {
                        LPC_PWM1->TCR = 0;                 /* stop PWM */
                        LPC_GPIO2->FIOSET = (1U << 11);    /* active-LOW off */
                        on_ticks4 = 0U;
                        state4 = BEEP_OFF;
                    }
                }
                else /* BEEP_OFF */
                {
                    off_ticks4++;
                    if (off_ticks4 >= 40U)
                    {
                        LPC_PWM1->TCR = (PWM_TCR_COUNTER_ENABLE_MASK | PWM_TCR_PWM_ENABLE_MASK);
                        off_ticks4 = 0U;
                        state4 = BEEP_ON;
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
        started = 0U;
        on_ticks3 = 0U;
        off_ticks3 = 0U;
        state3 = BEEP_OFF;
        started3 = 0U;
        on_ticks4 = 0U;
        off_ticks4 = 0U;
        state4 = BEEP_OFF;
        started4 = 0U;
        /* prev_tick left unchanged until next enable */
    }
}
