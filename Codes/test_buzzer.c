#include "LPC17xx.h"
#include "timer.h"
#include "PLL.h"
#include "PWM.h"

extern volatile uint8_t BuzzerOn_flag;
extern volatile uint8_t BuzzerOff_flag;
/* extern volatile uint16_t counter3 = 0;
extern volatile uint16_t counter4 = 0; */

int main(void)
{
    //int dutyCycle;
    PLL_Init();
	Timer_Init();
    LPC_GPIO2->FIODIR |= (1 << 11);
    PWM_Init();

	while(1)
	{
        /* Keep buzzer active for only reqired seconds, then stop */
        if(BuzzerOn_flag)
        {
            //NVIC_DisableIRQ(PWM1_IRQn);
            LPC_PWM1->TCR = 0;                 /* stop PWM counter + PWM mode */
            LPC_GPIO2->FIOSET = (1 << 11);     /* ensure buzzer OFF (active-LOW) */
        }
        /* Keep buzzer inactive for reqired seconds, then stop */
        else if(BuzzerOff_flag)
        {
            LPC_PWM1->TCR = 1;
        }
	}

}
