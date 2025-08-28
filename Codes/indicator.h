#include <stdint.h>

void SPI_Init(void);
uint8_t SPI_Tx_Rx_Byte(uint8_t data);
void HC595_Load(uint8_t value);