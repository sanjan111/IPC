#include "LPC17xx.h"
#include "timer.h"
#include "pwm.h"

void PWM_Init(void)
{
    /* Enable power/clock for PWM1 */
    LPC_SC->PCONP |= PCONP_PCPWM1_MASK;

    /* Ensure P2.11 is GPIO (PINSEL4 bits 23:22 = 00) and do not enable PWM1.1 on P2.0 */
    LPC_PINCON->PINSEL4 &= ~(PINSEL4_P2_11_MASK | PINSEL4_P2_00_MASK);

    /* Set prescaler (before starting timer) */
    LPC_PWM1->PR = PWM1_PRESCALE_VALUE;

    /* Enable reset on MR0 match (MCR bit 1) */
    LPC_PWM1->MCR = PWM_MCR_RESET_ON_MR0_MASK;

    /* Set period (MR0) and duty (MR1) */
    LPC_PWM1->MR0 = PWM1_PERIOD_MR0_TICKS;
	LPC_PWM1->MR1 = PWM1_DUTY_MR1_TICKS;
	

    /* Latch both MR0 and MR1 updates */
    LPC_PWM1->LER = (PWM_LER_EN_MR0_MASK | PWM_LER_EN_MR1_MASK);

    /* Pure timer/interrupt usage; no PWM output channel needed */

    /* Enable interrupt on MR0 and MR1 match */
    LPC_PWM1->MCR |= (PWM_MCR_INT_ON_MR0_MASK | PWM_MCR_INT_ON_MR1_MASK);

    /* Enable PWM1 interrupt in NVIC */
    NVIC_EnableIRQ(PWM1_IRQn);

    /* Enable PWM1 interrupt in NVIC */
    //__enable_irq();

    /* Start timer and PWM last */
    LPC_PWM1->TCR = (PWM_TCR_COUNTER_ENABLE_MASK | PWM_TCR_PWM_ENABLE_MASK);
} 

void PWM1_IRQHandler(void) 
{	 
	if ((LPC_PWM1->IR & PWM_IR_MR0_MASK) != 0U)
	{
		/* MR0: drive LOW (buzzer ON) */
		LPC_GPIO2->FIOCLR = BUZZER_GPIO_P2_11_MASK;
		/* Clear only MR0 flag */
		LPC_PWM1->IR |= PWM_IR_MR0_MASK;
	}
	else if ((LPC_PWM1->IR & PWM_IR_MR1_MASK) != 0U)
	{
		/* MR1: drive HIGH (buzzer OFF) */
		LPC_GPIO2->FIOSET = BUZZER_GPIO_P2_11_MASK;
		/* Clear only MR1 flag */
		LPC_PWM1->IR |= PWM_IR_MR1_MASK;
	}	
}
