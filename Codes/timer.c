#include "LPC17xx.h"
#include "timer.h"

volatile uint8_t LED1_flag = 0;
volatile uint8_t LED2_flag = 0;
volatile uint16_t counter1 = 0;
volatile uint16_t counter2 = 0;

timer_status_t Timer_Init(void)
{
    /* Power up TIMER0 (PCONP bit 1) */
    LPC_SC->PCONP |= (1U << 1U);

    /* Set Timer0 PCLK to CCLK/4 => 00 */
    LPC_SC->PCLKSEL0 &= ~PCLKSEL0_PCLK_TIMER0_MASK;
    
    /* Setting prescaler */
    LPC_TIM0->PR = PR_VALUE;

    /* Match value for 1ms period */
    LPC_TIM0->MR0 = MR0_VALUE;

    /* Enable interrupt flag and reset on MR0 match */
    LPC_TIM0->MCR = (MCR_MR0I | MCR_MR0R);

    /* Clear any pending match flags just in case */
    LPC_TIM0->IR = IR_MR0;

    /* Reset time counter, then enable timer */
    LPC_TIM0->TCR = TCR_COUNT_RESET;
    LPC_TIM0->TCR = TCR_COUNT_ENABLE;

    /* Enabling the interruptter*/
    NVIC_EnableIRQ(TIMER0_IRQn);
    
    return TIMER_STATUS_OK;
}

/* Blocking delay in milliseconds using TC polling only.
 * Compatible with NVIC-driven MR0 interrupts. Does not read/write IR/MCR.
 * Assumes MR0R is configured for a 1 ms period (TC wraps every 1 ms).
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

    /* Count TC wraps (each wrap = 1 ms when MR0R used) */
    {
        uint32_t prev = LPC_TIM0->TC;
        while (ms > 0U)
        {
            uint32_t curr = LPC_TIM0->TC;
            if (curr < prev)
            {
                /* TC wrapped due to MR0R => elapsed 1 ms */
                ms--;
            }
            prev = curr;
        }
    }
    return TIMER_STATUS_OK;
}


void TIMER0_IRQHandler(void)
{
    /* Check and clear MR0 interrupt */
    if ((LPC_TIM0->IR & IR_MR0) != 0U)
    {
        LPC_TIM0->IR = IR_MR0; /* write-1-to-clear */
        counter1++;
        counter2++;
    }
    if (counter1 == 400)
    {
        LED1_flag ^= 1;
        counter1 = 0;
    }
    if (counter2 == 450)
    {
        LED2_flag ^= 1;
        counter2 = 0;
    }
}
