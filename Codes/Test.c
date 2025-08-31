#include <stdint.h>
#include "LPC17xx.h"
#include "indicator.h"
#include "implement_indicator.h"
#include "PLL.h"
#include "timer.h"
#include "led.h"
#include "pwm.h"
#include "buzzer.h"

#define OFF 					0
#define ON  					1

#define LEFT_INDICATOR			1
#define RIGHT_INDICATOR			2
#define HAZARD_INDICATOR		3
#define SEATBELT_INDICATOR		4

int main(void)
{
	int hazard_switch = 1;
	int left_switch = 1;
	int right_switch = 1;
	int seatbelt_switch = 1;

  	PLL_Init();
  	Timer_Init();
 	SPI_Init();
	LED_Init();
	PWM_Init();
	/* Ensure buzzer GPIO P2.11 is output for explicit OFF drive */
	LPC_GPIO2->FIODIR |= (1 << 11);
	while (1)
	{
		if (hazard_switch == ON)
		{
			Indicator(HAZARD_INDICATOR);
			Buzzer(HAZARD_INDICATOR);
		}
		if (hazard_switch == OFF && left_switch == ON)
		{
			Indicator(LEFT_INDICATOR);
			Buzzer(LEFT_INDICATOR);
		}
		if (hazard_switch == OFF && right_switch == ON)
		{
			Indicator(RIGHT_INDICATOR);
			Buzzer(RIGHT_INDICATOR);
		}
		if (seatbelt_switch == ON)
		{
			LED_Status(SEATBELT_INDICATOR);
			Buzzer(SEATBELT_INDICATOR);
		}
	}
}
