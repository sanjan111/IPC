#include <LPC17xx.h>
#include <stdint.h>
#include "indicator.h"

void SPI_Init(void)
{
    /* 1) Power SSP0 */
    LPC_SC->PCONP |= (1u << 21);                 /* PCSSP0 = bit21 */

    /* 2) PCLK for SSP0 = CCLK/4  (PCLKSEL1[11:10] = 00) */
    LPC_SC->PCLKSEL1 &= ~(3u << 10);

    /* 3) Pin select
       - P0.15 -> SCK0  (PINSEL0[31:30] = 10)
       - P0.18 -> MOSI0 (PINSEL1[5:4]  = 10)
       - P0.16 -> GPIO (ST_CP latch, manual pulse) */
    LPC_PINCON->PINSEL0 &= ~(3u << 30);
    LPC_PINCON->PINSEL0 |=  (2u << 30);          /* SCK0 */

    LPC_PINCON->PINSEL1 &= ~(3u << 4);
    LPC_PINCON->PINSEL1 |=  (2u << 4);           /* MOSI0 */

    LPC_PINCON->PINSEL1 &= ~(3u << 0);           /* P0.16 as GPIO */
    LPC_GPIO0->FIODIR   |=  (1u << 16);          /* output */
    LPC_GPIO0->FIOCLR    =  (1u << 16);          /* start LOW */

    /* (optional) No-pull on SCK/MOSI for cleaner lines */
    LPC_PINCON->PINMODE0 &= ~(3u << 30); LPC_PINCON->PINMODE0 |= (2u << 30); /* P0.15 */
    LPC_PINCON->PINMODE1 &= ~(3u << 4);  LPC_PINCON->PINMODE1 |= (2u << 4);  /* P0.18 */

    /* 4) SSP0 config: disable, set clock & mode, enable
       SCK = PCLK / (CPSR * (SCR+1))
       With PCLK=25 MHz, choose CPSR=12, SCR=216 -> SCK â‰ˆ 25e6/(12*217) â‰ˆ 9600.6 Hz
    */
    LPC_SSP0->CR1  = 0u;                        /* ensure SSE=0 (disabled) */
    LPC_SSP0->CPSR = 12u;                       /* CPSDVSR even, >=2 */
    LPC_SSP0->CR0  =  (7u << 0)                 /* DSS=7 -> 8-bit */
                    | (0u << 4)                 /* FRF=SPI */
                    | (0u << 6)                 /* CPOL=0 */
                    | (0u << 7)                 /* CPHA=0 */
                    | (216u << 8);              /* SCR=216 */
    LPC_SSP0->CR1  = (1u << 1);                 /* SSE=1 (enable, master by default) */
}

/* Sends one byte on SSP0 and returns the simultaneously received byte.
   For 74HC595 the returned value is not meaningfulâ€”safe to ignore. */
uint8_t SPI_Tx_Rx_Byte(uint8_t data)
{
    /* start transfer */
    LPC_SSP0->DR = data;

    /* wait until SSP not busy (SR.BSY = bit4) */
    while ((LPC_SSP0->SR & (1u << 4)) != 0u) {
        /* spin */
    }

    /* read once to drain RX (even if unused) */
    return (uint8_t)(LPC_SSP0->DR & 0xFFu);
}

/* Helper to clock one byte into 74HC595 and latch outputs */
void HC595_Load(uint8_t value)
{
    (void)SPI_Tx_Rx_Byte(value);
    LPC_GPIO0->FIOSET = (1u << 16);  /* ST_CP HIGH */
    LPC_GPIO0->FIOCLR = (1u << 16);  /* ST_CP LOW  */
}