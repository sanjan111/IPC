#include <stdint.h>

void delay_ms(uint32_t ms) 
{
    volatile int32_t i, j;
    for(i = 0; i < ms; i++) 
    {    
        for(j = 0; j < 13000; j++); 
    }
}

void delay_s(uint32_t s)
{
  delay_ms(s*1000);
}


#if 0
void delay_ms(uint16 count)
{
  int j=0,i=0;

  for(j=0;j<count;j++)
  {
    for(i=0;i<35;i++);
  }
}
#endif
