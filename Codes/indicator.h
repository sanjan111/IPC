/*
 * File: indicator.h
 * Purpose: SPI/74HC595 indicator interface (MISRA C:2012 aligned)
 */

#ifndef INDICATOR_H
#define INDICATOR_H

#include <stdint.h>

/*
 * Peripheral power and clocks
 */
#define PCONP_PCSSP0_MASK                 (1UL << 21)  /* Power to SSP0 */
#define PCLKSEL1_SSP0_PCLK_MASK           (3UL << 10)  /* PCLKSEL1[11:10] */

/*
 * Pin function select (PINSEL)
 *  - P0.15 -> SCK0 (function 2)
 *  - P0.18 -> MOSI0 (function 2)
 *  - P0.16 -> GPIO (manual latch pulse)
 */
#define PINSEL0_P0_15_MASK                (3UL << 30)
#define PINSEL0_P0_15_FUNC_SCK0           (2UL << 30)

#define PINSEL1_P0_18_MASK                (3UL << 4)
#define PINSEL1_P0_18_FUNC_MOSI0          (2UL << 4)

#define PINSEL1_P0_16_MASK                (3UL << 0)   /* 00 = GPIO */

/*
 * Pin mode (PINMODE): no pull-up/pull-down on SCK/MOSI
 */
#define PINMODE0_P0_15_MASK               (3UL << 30)
#define PINMODE0_P0_15_NO_PULL            (2UL << 30)

#define PINMODE1_P0_18_MASK               (3UL << 4)
#define PINMODE1_P0_18_NO_PULL            (2UL << 4)

/*
 * GPIO (P0.16 used as ST_CP latch line for 74HC595)
 */
#define GPIO0_P0_16_MASK                  (1UL << 16)

/*
 * SSP0 register fields
 */
#define SSP_CR1_SSE_ENABLE_MASK           (1UL << 1)   /* Enable SSP */
#define SSP_SR_BSY_MASK                   (1UL << 4)   /* Busy flag */

#define SSP_CPSR_DIVISOR                  (12UL)       /* Even, >= 2 */

/* CR0 configuration: 8-bit, SPI frame, CPOL=0, CPHA=0, SCR=216 */
#define SSP_CR0_DSS_8BIT                  (7UL << 0)
#define SSP_CR0_FRF_SPI                   (0UL << 4)
#define SSP_CR0_CPOL_0                    (0UL << 6)
#define SSP_CR0_CPHA_0                    (0UL << 7)
#define SSP_CR0_SCR_216                   (216UL << 8)

/* 8-bit data mask for readback */
#define SSP_DATA_8BIT_MASK                (0xFFUL)

/* Public API */
void SPI_Init(void);
uint8_t SPI_Tx_Rx_Byte(uint8_t data);
void HC595_Load(uint8_t value);

#endif /* INDICATOR_H */
