typedef enum
{
    LED_OK = 0,
    LED_ERROR = 1
} led_status_t;

#define LED1            (1U << 14U)
#define LED2            (1U << 15U)

led_status_t LED_Init(void);
led_status_t LED1_On(void);
led_status_t LED1_Off(void);
led_status_t LED2_On(void);
led_status_t LED2_Off(void);
