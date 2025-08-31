#include "LPC17xx.h"
#include <stdint.h>

void LED_Init(void)
{
    LPC_GPIO1->FIODIR = (1 << 29);
}

void LED_Status(uint8_t status)
{
    if (status == 4)
        LPC_GPIO1->FIOSET = (0x1 << 29);
    else if (status == 0)
        LPC_GPIO1->FIOCLR = (0x1 << 29);
}
