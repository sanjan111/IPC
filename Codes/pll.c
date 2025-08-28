#include <stdint.h>
#include "LPC17xx.h"
#include "pll.h"


static void pll0_feed(void)
{
    LPC_SC->PLL0FEED = PLL0_FEED_SEQ_1;
    LPC_SC->PLL0FEED = PLL0_FEED_SEQ_2;
}

pll_status_t PLL_Init(void)
{
    uint32_t timeout = OSC_READY_TIMEOUT_CYCLES;

    /* Select proper range for your crystal: 0 => 1-20 MHz (typical 12 MHz) */
    LPC_SC->SCS &= ~SCS_OSCRANGE;
    
    /* Enable the main oscillator */
    LPC_SC->SCS |= SCS_OSCEN;

    /* Wait for oscillator to stabilize */
    while (((LPC_SC->SCS & SCS_OSCSTAT) == 0U) && (timeout > 0U))
    {
        timeout --;
    }
    if (timeout == 0U)
    {
        return PLL_ERR_OSC_TIMEOUT;
    }
    /* Resetting timeout for next use*/
    timeout = OSC_READY_TIMEOUT_CYCLES;  

    /* Select main oscillator as pll clock source,: 01 => [1:0] */
    LPC_SC->CLKSRCSEL &= ~(CLKSRCSEL_CLKSRC0 |CLKSRCSEL_CLKSRC1);
    LPC_SC->CLKSRCSEL |= CLKSRCSEL_CLKSRC0;

    /* PLL0 multiplier and predivider value */
    LPC_SC->PLL0CFG = PLL0CFG_MSEL0 | PLL0CFG_NSEL0;
   
    /* Enable PLL */
    LPC_SC->PLL0CON = PLL0CON_PLLE0;

    /* Feed to latch changes done for PLL0 */
    pll0_feed();

    /* Wait for PLL to lock */
    while (((LPC_SC->PLL0STAT & PLL0STAT_PLOCK0) == 0U) && (timeout > 0U))
    {
        timeout --;
    }
    if (timeout == 0U)
    {
        return PLL_ERR_OSC_TIMEOUT;
    }
    /* Set CPU clock divider to 3 */
    LPC_SC->CCLKCFG = CCLKCFG_CCLKSEL;

    /* Connect PLL now that it is locked */
    LPC_SC->PLL0CON |= PLL0CON_PLLC0;

    /* Feed to latch changes done for PLL0 */
    pll0_feed();

    return PLL_OK;

}

