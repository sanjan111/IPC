#include <stdint.h>
#include "indicator.h"
#include "PLL.h"
#include "timer.h"

int main(void)
{
	int i,pattern=0;
    PLL_Init();
    Timer_Init();
    SPI_Init();

		while(1)
		{
			for (i=1;i<=4;i++)
			{
				pattern = pattern << 2;
				pattern |= 01;
        		HC595_Load(pattern);
				delay_ms(1000);
			}

 			for (i=4;i>0;i--)
			{
 				pattern = pattern >> 2;
        		HC595_Load(pattern);
 				delay_ms(1000);	
			}
		}
 
}
