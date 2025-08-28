/*
 * Timer configuration header (MISRA C:2012 aligned)
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* Status codes for timer APIs */
typedef enum
{
    TIMER_STATUS_OK = 0,
    TIMER_STATUS_INVALID_PARAM = 1,
    TIMER_STATUS_NOT_READY = 2
} timer_status_t;

/* Selecting the peripheral clock for TIMER0 (PCLKSEL0 bits [3:2]) */
/* Mask for the 2-bit TIMER0 field (both bits) */
#define PCLKSEL0_PCLK_TIMER0_MASK           (3U << 2U)

/* Timer configuration values */
#define PR_VALUE                            (249U)           /* Prescaler value */
#define MR0_VALUE                           (100U)           /* Match register 0 value */

/* Match Control Register bits */
#define MCR_MR0R                            (1U << 1U)       /* Reset on MR0 match */

/* Timer0 Interrupt Register bits */
#define IR_MR0                              (1U << 0U)

/* Timer Control Register bits */
#define TCR_COUNT_RESET                     (1U << 1U)       /* Reset */
#define TCR_COUNT_ENABLE                    (1U << 0U)       /* Enable */

/* Initialize Timer0 for periodic match on MR0 */
timer_status_t Timer_Init(void);
/* Blocking delay using IR.MR0 polling */
timer_status_t delay_ms(uint32_t ms);

#endif /* TIMER_H */
