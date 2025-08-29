#include "indicator.h"
#include <stdint.h>
#include "timer.h"

extern volatile uint8_t LED1_flag;
extern volatile uint8_t counter1;
void Indicator(uint8_t direction)
{
    int16_t i,pattern=0;
    if (direction == 1)
    {
        
        for (i=3;i>=0;i--)
			{
                if (LED1_flag == 1)
                {
                    pattern |= (1 << i);
                    HC595_Load(pattern);
                }
			}
    }
}
