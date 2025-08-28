#include "LPC17xx.h"
#include "led.h"

led_status_t LED_Init(void)
{
    LPC_GPIO1->FIODIR |= LED1;
    LPC_GPIO1->FIODIR |= LED2;
    LPC_GPIO1->FIOCLR |= LED1;
    LPC_GPIO1->FIOCLR |= LED2;

    return LED_OK;
}
led_status_t LED1_On(void)
{
    LPC_GPIO1->FIOSET |= LED1;

    return LED_OK;
}

led_status_t LED1_Off(void)
{
    LPC_GPIO1->FIOCLR |= LED1;

    return LED_OK;
}

led_status_t LED2_On(void)
{
    LPC_GPIO1->FIOSET |= LED2;

    return LED_OK;
}

led_status_t LED2_Off(void)
{
    LPC_GPIO1->FIOCLR |= LED2;

    return LED_OK;
}
