#include "timer.h"
#include "led.h"
#include "pll.h"

int main()
{
		int timeout = 5000000U;
    PLL_Init();
    Timer_Init();
    LED_Init();

    while(1)
    {
        LED1_On();
			
				delay_ms(500);
        //while (timeout--);
				//timeout = 5000000U;
			
        LED1_Off();
				//while (timeout--);
				//timeout = 5000000U;
    }
}
