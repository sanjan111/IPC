#include "LPC17xx.h"
#include "timer.h"
#include "pll.h"
#include "pwm.h"

int main(void)
{
    PLL_Init();
    LPC_GPIO2->FIODIR |= (1 << 11);
		Timer_Init();
    PWM_Init();

	while(1)
	{
        /* Keep buzzer active for only reqired seconds, then stop */
        delay_ms(200);
        //NVIC_DisableIRQ(PWM1_IRQn);
        LPC_PWM1->TCR = 0;                 /* stop PWM counter + PWM mode */
        LPC_GPIO2->FIOSET = (1 << 11);     /* ensure buzzer OFF (active-LOW) */
        /* Keep buzzer inactive for reqired seconds, then stop */
        delay_ms(800);
        LPC_PWM1->TCR = 1;
	}

}
