#include "timer.h"
#include "led.h"
#include "pll.h"

extern volatile uint8_t LED1_flag;
extern volatile uint8_t LED2_flag;

int main()
{
    PLL_Init();
    Timer_Init();
    LED_Init();
    delay_ms(1000); /* Testing the delay_ms function */

    while(1)
    {
        if (LED1_flag)
        {
        LED1_On();	
		//delay_ms(500);
        }
        else
        {
        LED1_Off();
        //delay_ms(500);
        }

        if (LED2_flag)
        {
        LED2_On();	
		//delay_ms(500);
        }
        else
        {
        LED2_Off();
        //delay_ms(500);
        }

    }
}
