#include "LPC17xx.h"
#include "timer.h"

timer_status_t Timer_Init(void)
{
    /* Set Timer0 PCLK to CCLK/4 => 00 */
    LPC_SC->PCLKSEL0 &= ~PCLKSEL0_PCLK_TIMER0_MASK;
    
    /* Setting prescaler */
    LPC_TIM0->PR = PR_VALUE;

    /* Match value for 1ms period */
    LPC_TIM0->MR0 = MR0_VALUE;

    /* Enable reset on MR0 match */
    LPC_TIM0->MCR |= MCR_MR0R; 

    /* Clear any pending match flags just in case */
    LPC_TIM0->IR = IR_MR0;

    /* Reset time counter, clear reset and enable timer */
    LPC_TIM0->TCR = TCR_COUNT_RESET;
    LPC_TIM0->TCR = TCR_COUNT_ENABLE;

    return TIMER_STATUS_OK;
}

/* Blocking delay in milliseconds using MR0 match flag.
 * Each loop waits for one MR0 event (1 ms), clears it, and repeats.
 */
timer_status_t delay_ms(uint32_t ms)
{
    if (ms == 0U)
    {
        return TIMER_STATUS_INVALID_PARAM;
    }

    /* Ensure timer is enabled */
    if ((LPC_TIM0->TCR & TCR_COUNT_ENABLE) == 0U)
    {
        return TIMER_STATUS_NOT_READY;
    }

    while (ms > 0U)
    {
        /* Ensure we wait for a fresh 1 ms tick */
        LPC_TIM0->IR = IR_MR0;

        /* Wait for the MR0 match flag */
        while ((LPC_TIM0->IR & IR_MR0) == 0U)
        {
            /* busy wait */
        }

        ms--;
    }
    return TIMER_STATUS_OK;
}
