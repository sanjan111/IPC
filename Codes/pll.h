/*
 * PLL configuration for LPC17xx
 * Adds include guards and corrects bit definitions to align with user manual.
 */

#ifndef PLL_H
#define PLL_H

#ifdef __cplusplus
extern "C" {
#endif

#define OSC_READY_TIMEOUT_CYCLES    (1000000UL)     /* Generate wait time */

#define SCS_OSCRANGE        (1U << 4U)              /* 0: 1-20 MHz, 1:15-25 MHz */
#define SCS_OSCEN           (1U << 5U)              /* Main Oscillator Enable */
#define SCS_OSCSTAT         (1U << 6U)              /* Main Oscillator Status */

#define CLKSRCSEL_CLKSRC0   (1U << 0U)               /* Selecting pll clock source*/
#define CLKSRCSEL_CLKSRC1   (1U << 1U)               

#define PLL0CFG_MSEL0       (24U << 0U)               /* PLL0 mutiplier value */
#define PLL0CFG_NSEL0       (1U << 16U)               /* PLL0 predivider rule*/

/* PLL0 control and status */
#define PLL0CON_PLLE0       (1U << 0U)                /* Enable PLL0 */
#define PLL0CON_PLLC0       (1U << 1U)                /* Connect PLL0 */
#define PLL0STAT_PLOCK0     (1U << 26U)               /* PLL0 lock status */

#define CCLKCFG_CCLKSEL     (0x02U)                 /* CPU clock divider = 3 */

/* PLL0 feed sequence constants (avoid magic numbers) */
#define PLL0_FEED_SEQ_1     (0xAAU)
#define PLL0_FEED_SEQ_2     (0x55U)

typedef enum
{
    PLL_OK = 0,
    PLL_ERR_OSC_TIMEOUT
} pll_status_t;

/* Function initializes the CCLK to target frequency */
pll_status_t PLL_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* PLL_H */
