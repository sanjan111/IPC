#include "LPC17xx.h"
#include "Utilities.h"

void PWM_Init(void)
{
    /* Enable power/clock for PWM1 */
    LPC_SC->PCONP |= (1 << 6);

    /* Ensure P2.11 is GPIO (PINSEL4 bits 23:22 = 00) and do not enable PWM1.1 on P2.0 */
    LPC_PINCON->PINSEL4 &= ~((3U << 22) | (3U << 0));

    /* Set prescaler (before starting timer) */
    LPC_PWM1->PR = 10;

    /* Enable reset on MR0 match (MCR bit 1) */
    LPC_PWM1->MCR = (1 << 1);

    /* Set period (MR0) and duty (MR1) */
    LPC_PWM1->MR0 = 1500;
    //LPC_PWM1->MR1 = 10;
		LPC_PWM1->MR1 = 1400;
		//LPC_PWM1->MR1 = 1400;

    /* Latch both MR0 and MR1 updates */
    LPC_PWM1->LER = (1 << 0) | (1 << 1);

    /* Pure timer/interrupt usage; no PWM output channel needed */

    /* Enable interrupt on MR0 and MR1 match */
    LPC_PWM1->MCR |= ((1 << 0) | (1<<3));

    /* Enable PWM1 interrupt in NVIC */
    NVIC_EnableIRQ(PWM1_IRQn);

    /* Enable PWM1 interrupt in NVIC */
    //__enable_irq();

    /* Start timer and PWM last */
    LPC_PWM1->TCR = (1 << 0) | (1 << 3);
} 

void PWM1_IRQHandler(void) 
{	 
	if (LPC_PWM1->IR & (1 << 0))
	{
		/* MR0: drive LOW (buzzer ON) */
		LPC_GPIO2->FIOCLR = (1 << 11);
		/* Clear only MR0 flag (write-1-to-clear requires assignment, not OR) */
		LPC_PWM1->IR |= (1 << 0);
	}
	else if (LPC_PWM1->IR & (1 << 1))
	{
		/* MR1: drive HIGH (buzzer OFF) */
		LPC_GPIO2->FIOSET = (1 << 11);
		/* Clear only MR1 flag */
		LPC_PWM1->IR |= (1 << 1);
	}
	
	
	
}
