#ifndef _RTL8710_LOG_UART_H_
#define _RTL8710_LOG_UART_H_

#include <stdint.h>

typedef struct{
	union{
		volatile uint32_t RBR;
		volatile uint32_t THR;
		volatile uint32_t DLL;
		volatile uint32_t RBR_THR_DLL;
	};
	union{
		volatile uint32_t IER;
		volatile uint32_t DLH;
		volatile uint32_t IER_DLH;
	};
	union{
		volatile uint32_t IIR;
		volatile uint32_t FCR;
		volatile uint32_t IIR_FCR;
	};
	volatile uint32_t LCR;
	volatile uint32_t MCR;
	volatile uint32_t LSR;
	volatile uint32_t MSR;
	uint32_t RESERVED1[24];
	volatile uint32_t USR;
}__attribute__((packed)) LOG_UART_TypeDef;

#define LOG_UART               ((LOG_UART_TypeDef *)0x40003000)

// LOG_UART_IER
#define LOG_UART_IER_ERBFI     (((uint32_t)0x01) << 0)
#define LOG_UART_IER_ETBEI     (((uint32_t)0x01) << 1)
#define LOG_UART_IER_ELSI      (((uint32_t)0x01) << 2)
#define LOG_UART_IER_EDSSI     (((uint32_t)0x01) << 3)

// LOG_UART_FCR
#define LOG_UART_FCR_FIFOE     (((uint32_t)0x01) << 0)
#define LOG_UART_FCR_RFIFOR    (((uint32_t)0x01) << 1)
#define LOG_UART_FCR_XFIFOR    (((uint32_t)0x01) << 2)
#define LOG_UART_FCR_DMAM      (((uint32_t)0x01) << 3)
#define LOG_UART_FCR_TET       (((uint32_t)0x03) << 4)
#define LOG_UART_FCR_RT        (((uint32_t)0x03) << 6)

// LOG_UART_LCR
#define LOG_UART_LCR_DLS       (((uint32_t)0x03) << 0)
#define LOG_UART_LCR_STOP      (((uint32_t)0x01) << 2)
#define LOG_UART_LCR_PEN       (((uint32_t)0x01) << 3)
#define LOG_UART_LCR_EPS       (((uint32_t)0x01) << 4)
#define LOG_UART_LCR_STICK_PAR (((uint32_t)0x01) << 5)
#define LOG_UART_LCR_BC        (((uint32_t)0x01) << 6)
#define LOG_UART_LCR_DLAB      (((uint32_t)0x01) << 7)

// LOG_UART_MCR
#define LOG_UART_MCR_DTR       (((uint32_t)0x01) << 0)
#define LOG_UART_MCR_RTS       (((uint32_t)0x01) << 1)
#define LOG_UART_MCR_OUT1      (((uint32_t)0x01) << 2)
#define LOG_UART_MCR_OUT2      (((uint32_t)0x01) << 3)
#define LOG_UART_MCR_LOOPBACK  (((uint32_t)0x01) << 4)
#define LOG_UART_MCR_AFCE      (((uint32_t)0x01) << 5)

// LOG_UART_LSR
#define LOG_UART_LSR_DR        (((uint32_t)0x01) << 0)
#define LOG_UART_LSR_OE        (((uint32_t)0x01) << 1)
#define LOG_UART_LSR_PE        (((uint32_t)0x01) << 2)
#define LOG_UART_LSR_FE        (((uint32_t)0x01) << 3)
#define LOG_UART_LSR_BI        (((uint32_t)0x01) << 4)
#define LOG_UART_LSR_THRE      (((uint32_t)0x01) << 5)
#define LOG_UART_LSR_TEMT      (((uint32_t)0x01) << 6)
#define LOG_UART_LSR_RFE       (((uint32_t)0x01) << 7)
#define LOG_UART_LSR_ADDR_RCVD (((uint32_t)0x01) << 8)

#endif

