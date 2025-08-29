#include <stdint.h>
#include "indicator.h"
#include "implement_indicator.h"
#include "PLL.h"
#include "timer.h"

int main(void)
{
	//int i,pattern=0;
    PLL_Init();
    Timer_Init();
    SPI_Init();
	while (1)
	{
		Indicator(1);
	}
    
		/* while(1)
		{
			for (i=3;i>=0;i--)
			{
				pattern |= (1 << i);
        		HC595_Load(pattern);
				delay_ms(1000);
			}

 			 for (i=4;i>0;i--)
			{
 				pattern = pattern >> 2;
        		HC595_Load(pattern);
 				delay_ms(1000);	
			} 
		}  */
 
}
