#ifndef _RTL8710_GPIO_H_
#define _RTL8710_GPIO_H_

#include <stdint.h>

typedef struct{
	volatile uint32_t SWPORTA_DR;
	volatile uint32_t SWPORTA_DDR;
	volatile uint32_t SWPORTA_CTRL;
	volatile uint32_t SWPORTB_DR;
	volatile uint32_t SWPORTB_DDR;
	volatile uint32_t SWPORTB_CTRL;
	volatile uint32_t SWPORTC_DR;
	volatile uint32_t SWPORTC_DDR;
	volatile uint32_t SWPORTC_CTRL;
	uint32_t RESERVED1[3];
	volatile uint32_t INTEN;
	volatile uint32_t INTMASK;
	volatile uint32_t INTTYPE_LEVEL;
	volatile uint32_t INT_POLARITY;
	volatile uint32_t INTSTATUS;
	volatile uint32_t RAW_INTSTATUS;
	volatile uint32_t DEBOUNCE;
	volatile uint32_t PORTA_EOI;
	volatile uint32_t EXT_PORTA;
	volatile uint32_t EXT_PORTB;
	volatile uint32_t EXT_PORTC;
	uint32_t RESERVED2[1];
	volatile uint32_t LS_SYNC;
}__attribute__((packed)) GPIO_TypeDef;

#define GPIO           ((GPIO_TypeDef *)0x40001000)

#define GPIO_PORTA_GC4 (((uint32_t)1) << 8)

#endif

