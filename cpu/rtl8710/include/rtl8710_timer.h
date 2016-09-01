#ifndef _RTL8710_TIMER_H_
#define _RTL8710_TIMER_H_

#include <stdint.h>

typedef struct{
	volatile uint32_t TIM0_LOAD_COUNT;
	volatile uint32_t TIM0_CURRENT_VALUE;
	volatile uint32_t TIM0_CONTROL;
	volatile uint32_t TIM0_EOI;
	volatile uint32_t TIM0_IS;
	volatile uint32_t TIM1_LOAD_COUNT;
	volatile uint32_t TIM1_CURRENT_VALUE;
	volatile uint32_t TIM1_CONTROL;
	volatile uint32_t TIM1_EOI;
	volatile uint32_t TIM1_IS;
	uint32_t RESERVED1[30];
	volatile uint32_t TIMS_IS;
	volatile uint32_t TIMS_EOI;
	volatile uint32_t TIMS_RAW_IS;
	volatile uint32_t TIMS_COMP_VERSION;
	volatile uint32_t TIM0_LOAD_COUNT2;
	volatile uint32_t TIM1_LOAD_COUNT2;
}__attribute__((packed)) TIMER_TypeDef;

#define TIMER                      ((TIMER_TypeDef *)0x40002000)

// TIMER_CONTROL
#define TIMER_CONTROL_ENABLE       (((uint32_t)0x01) << 0)
#define TIMER_CONTROL_MODE         (((uint32_t)0x01) << 1)
#define TIMER_CONTROL_IM           (((uint32_t)0x01) << 2)
#define TIMER_CONTROL_PWM          (((uint32_t)0x01) << 3)

#endif

