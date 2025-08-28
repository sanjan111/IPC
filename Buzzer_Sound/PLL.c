#include <LPC17xx.h>
#include "PLL.h"

void PLL_Init(void)
{
    // Selecting Main Oscillator Range 1- 20 MHz
    LPC_SC->SCS &= ~(1<<4);
    // Enabling Main Oscillator
    LPC_SC->SCS |= (1<<5);
    // Selecting Clock Oscillator Source as Main Oscillator
    LPC_SC->CLKSRCSEL &= ~((1<<0) | (1<<1));
    LPC_SC->CLKSRCSEL |=(1<<0);   
    // Setting values for N and M
    LPC_SC->PLL0CFG = ((1<<16) | (24<<0));
    LPC_SC->PLL0CON =(1<<0);
    // Make PPL0 values loaded to shadow register that affect the actual PLL0 Operation
    LPC_SC->PLL0FEED = 0xAA;
    LPC_SC->PLL0FEED = 0x55;
    // Wait for PLL0 to get locked
    while(!(LPC_SC->PLL0STAT & (1<<26)));
    // Set CPU clock divider to 3 (for 100 MHz CCLK from 300 MHz FCCO) BEFORE connect
    LPC_SC->CCLKCFG = 0x02;
    // Enable and connect PLL
    LPC_SC->PLL0CON = 0x03;
    // Feed to latch connect
    LPC_SC->PLL0FEED = 0xAA;
    LPC_SC->PLL0FEED = 0x55;
}
