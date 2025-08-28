/*
 * File: pwm.h
 * Purpose: PWM interface and configuration macros (MISRA C:2012 aligned)
 */

#ifndef PWM_H
#define PWM_H

/* Macros moved from source to header per requirement */

/* PCONP bit for PWM1 peripheral clock */
#define PCONP_PCPWM1_MASK             (1UL << 6)

/* PINSEL4 masks: ensure P2.11 is GPIO and do not enable PWM1.1 on P2.0 */
#define PINSEL4_P2_11_MASK            (3UL << 22)
#define PINSEL4_P2_00_MASK            (3UL << 0)

/* PWM1 timing configuration */
#define PWM1_PRESCALE_VALUE           (10UL)
#define PWM1_PERIOD_MR0_TICKS         (1500UL)
#define PWM1_DUTY_MR1_TICKS           (1400UL)

/* PWM1 Match Control Register (MCR) bits */
#define PWM_MCR_INT_ON_MR0_MASK       (1UL << 0)
#define PWM_MCR_RESET_ON_MR0_MASK     (1UL << 1)
#define PWM_MCR_INT_ON_MR1_MASK       (1UL << 3)

/* PWM1 Latch Enable Register (LER) bits */
#define PWM_LER_EN_MR0_MASK           (1UL << 0)
#define PWM_LER_EN_MR1_MASK           (1UL << 1)

/* PWM1 Timer Control Register (TCR) bits */
#define PWM_TCR_COUNTER_ENABLE_MASK   (1UL << 0)
#define PWM_TCR_PWM_ENABLE_MASK       (1UL << 3)

/* PWM1 Interrupt Register (IR) bits */
#define PWM_IR_MR0_MASK               (1UL << 0)
#define PWM_IR_MR1_MASK               (1UL << 1)

/* Buzzer GPIO (P2.11) mask */
#define BUZZER_GPIO_P2_11_MASK        (1UL << 11)

/* Public API */
void PWM_Init(void);
void PWM1_IRQHandler(void);

#endif /* PWM_H */
