/*
 * File: indicator.c
 * Purpose: Configure SSP0 and drive 74HC595 shift register for indicators.
 * Notes: MISRA C:2012 aligned; no logic changes from original implementation.
 */

#include <LPC17xx.h>
#include <stdint.h>
#include "indicator.h"

void SPI_Init(void)
{
    /* 1) Power SSP0 */
    LPC_SC->PCONP |= PCONP_PCSSP0_MASK; /* Enable power to SSP0 peripheral */

    /* 2) PCLK for SSP0 = CCLK/4  (PCLKSEL1[11:10] = 00) */
    LPC_SC->PCLKSEL1 &= ~PCLKSEL1_SSP0_PCLK_MASK; /* Clear to select CCLK/4 */

    /* 3) Pin select and GPIO direction
       - P0.15 -> SCK0  (function 2)
       - P0.18 -> MOSI0 (function 2)
       - P0.16 -> GPIO  (ST_CP latch, manual pulse) */
    LPC_PINCON->PINSEL0 &= ~PINSEL0_P0_15_MASK;
    LPC_PINCON->PINSEL0 |=  PINSEL0_P0_15_FUNC_SCK0;

    LPC_PINCON->PINSEL1 &= ~PINSEL1_P0_18_MASK;
    LPC_PINCON->PINSEL1 |=  PINSEL1_P0_18_FUNC_MOSI0;

    LPC_PINCON->PINSEL1 &= ~PINSEL1_P0_16_MASK; /* P0.16 as GPIO */
    LPC_GPIO0->FIODIR   |=  GPIO0_P0_16_MASK;    /* output */
    LPC_GPIO0->FIOCLR    =  GPIO0_P0_16_MASK;    /* start LOW */

    /* Optional: no pull-up/pull-down on SCK/MOSI for cleaner lines */
    LPC_PINCON->PINMODE0 &= ~PINMODE0_P0_15_MASK; 
    LPC_PINCON->PINMODE0 |=  PINMODE0_P0_15_NO_PULL; /* P0.15 */
    LPC_PINCON->PINMODE1 &= ~PINMODE1_P0_18_MASK;  
    LPC_PINCON->PINMODE1 |=  PINMODE1_P0_18_NO_PULL; /* P0.18 */

    /* 4) SSP0 config: disable, set clock & mode, enable
       SCK = PCLK / (CPSR * (SCR + 1))
       With PCLK=25 MHz, CPSR=12, SCR=216 -> SCK ≈ 25e6/(12*217) ≈ 9600.6 Hz
    */
    LPC_SSP0->CR1  = 0UL;                        /* Ensure SSE=0 (disabled) */
    LPC_SSP0->CPSR = SSP_CPSR_DIVISOR;           /* CPSDVSR even, >= 2 */
    LPC_SSP0->CR0  =  SSP_CR0_DSS_8BIT           /* 8-bit frame */
                    | SSP_CR0_FRF_SPI            /* SPI frame */
                    | SSP_CR0_CPOL_0             /* CPOL=0 */
                    | SSP_CR0_CPHA_0             /* CPHA=0 */
                    | SSP_CR0_SCR_216;           /* Serial clock rate */
    LPC_SSP0->CR1  = SSP_CR1_SSE_ENABLE_MASK;    /* SSE=1 (enable, master by default) */
}

/*
 * Sends one byte on SSP0 and returns the simultaneously received byte.
 * Note: For 74HC595 the returned value is not meaningful; safe to ignore.
 */
uint8_t SPI_Tx_Rx_Byte(uint8_t data)
{
    /* start transfer */
    LPC_SSP0->DR = data;

    /* wait until SSP not busy (SR.BSY = bit4) */
    while ((LPC_SSP0->SR & SSP_SR_BSY_MASK) != 0UL) {
        /* spin */
    }

    /* read once to drain RX (even if unused) */
    return (uint8_t)(LPC_SSP0->DR & SSP_DATA_8BIT_MASK);
}

/* Helper to clock one byte into 74HC595 and latch outputs */
void HC595_Load(uint8_t value)
{
    (void)SPI_Tx_Rx_Byte(value);
    LPC_GPIO0->FIOSET = GPIO0_P0_16_MASK;  /* ST_CP HIGH */
    LPC_GPIO0->FIOCLR = GPIO0_P0_16_MASK;  /* ST_CP LOW  */
}
