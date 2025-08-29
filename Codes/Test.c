#include <stdint.h>
#include "indicator.h"
#include "implement_indicator.h"
#include "PLL.h"
#include "timer.h"

#define OFF 					0
#define ON  					1

#define LEFT_INDICATOR			1
#define RIGHT_INDICATOR			2
#define HAZARD_INDICATOR		3

int main(void)
{
	int hazard_switch = 0;
	int left_switch = 0;
	int right_switch = 1;

    PLL_Init();
    Timer_Init();
    SPI_Init();
	while (1)
	{
		if (hazard_switch == ON)
			Indicator(HAZARD_INDICATOR);
		if (hazard_switch == OFF && left_switch == ON)
			Indicator(LEFT_INDICATOR);
		if (hazard_switch == OFF && right_switch == ON)
			Indicator(RIGHT_INDICATOR);
		
	}
}
