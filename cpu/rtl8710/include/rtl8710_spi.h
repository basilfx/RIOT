#ifndef _RTL8710_SPI_H_
#define _RTL8710_SPI_H_

#include <stdint.h>

typedef struct{
	volatile uint32_t CTRLR0;
	volatile uint32_t CTRLR1;
	volatile uint32_t SSIENR;
	volatile uint32_t MWCR;
	volatile uint32_t SER;
	volatile uint32_t BAUDR;
	volatile uint32_t TXFTLR;
	volatile uint32_t RXFTLR;
	volatile uint32_t TXFLR;
	volatile uint32_t RXFLR;
	volatile uint32_t SR;
	volatile uint32_t IMR;
	volatile uint32_t ISR;
	volatile uint32_t RISR;
	volatile uint32_t TXOICR;
	volatile uint32_t RXOICR;
	volatile uint32_t RXUICR;
	volatile uint32_t MSTICR;
	volatile uint32_t ICR;
	volatile uint32_t DMACR;
	volatile uint32_t DMATDLR;
	volatile uint32_t DMARDLR;
	volatile uint32_t IDR;
	volatile uint32_t SSI_COMP_VERSION;
	union{
		struct{
			union{
				volatile uint8_t DR;
				volatile uint8_t DR8;
			};
			uint8_t RESERVED1[3];
		}__attribute__((packed));
		struct{
			volatile uint16_t DR16;
			uint16_t RESERVED2[1];
		}__attribute__((packed));
		volatile uint32_t DR32;
	};
	uint32_t RESERVED3[31];
	volatile uint32_t READ_FAST_SINGLE;
	volatile uint32_t READ_DUAL_DATA;
	volatile uint32_t READ_DUAL_ADDR_DATA;
	volatile uint32_t READ_QUAD_DATA;
	union{
		volatile uint32_t READ_QUAD_ADDR_DATA;
		volatile uint32_t RX_SAMPLE_DLY;
	};
	volatile uint32_t WRITE_SIGNLE;
	volatile uint32_t WRITE_DUAL_DATA;
	volatile uint32_t WRITE_DUAL_ADDR_DATA;
	volatile uint32_t WRITE_QUAD_DATA;
	volatile uint32_t WRITE_QUAD_ADDR_DATA;
	volatile uint32_t WRITE_ENABLE;
	volatile uint32_t READ_STATUS;
	volatile uint32_t CTRLR2;
	volatile uint32_t FBAUDR;
	volatile uint32_t ADDR_LENGTH;
	volatile uint32_t AUTO_LENGTH;
	volatile uint32_t VALID_CMD;
	volatile uint32_t FLASE_SIZE;
	volatile uint32_t FLUSH_FIFO;
}__attribute__((packed)) SPI_TypeDef;

#define SPI_FLASH                  ((SPI_TypeDef *)0x40006000)

// SPI_CTRLR0
#define SPI_CTRLR0_FRF             (((uint32_t)0x03) << 4)
#define SPI_CTRLR0_SCPH            (((uint32_t)0x01) << 6)
#define SPI_CTRLR0_SCPOL           (((uint32_t)0x01) << 7)
#define SPI_CTRLR0_TMOD            (((uint32_t)0x03) << 8)
#define SPI_CTRLR0_SLV_OE          (((uint32_t)0x01) << 10)
#define SPI_CTRLR0_SRL             (((uint32_t)0x01) << 11)
#define SPI_CTRLR0_CFS             (((uint32_t)0x0F) << 12)
#define SPI_CTRLR0_ADDR_CH         (((uint32_t)0x03) << 16)
#define SPI_CTRLR0_DATA_CH         (((uint32_t)0x03) << 18)
#define SPI_CTRLR0_CMD_CH          (((uint32_t)0x03) << 20)
#define SPI_CTRLR0_FAST_RD         (((uint32_t)0x01) << 22)
#define SPI_CTRLR0_SHIFT_CK_MTIMES (((uint32_t)0x1F) << 23)

// SPI_SER
#define SPI_SER_SS0                (((uint32_t)0x01) << 0)
#define SPI_SER_SS1                (((uint32_t)0x01) << 1)
#define SPI_SER_SS2                (((uint32_t)0x01) << 2)

// SPI_SR
#define SPI_SR_SSI                 (((uint32_t)0x01) << 0)
#define SPI_SR_TFNF                (((uint32_t)0x01) << 1)
#define SPI_SR_TFE                 (((uint32_t)0x01) << 2)
#define SPI_SR_RFNE                (((uint32_t)0x01) << 3)
#define SPI_SR_RFF                 (((uint32_t)0x01) << 4)
#define SPI_SR_TXE                 (((uint32_t)0x01) << 5)

#endif

