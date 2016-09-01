#ifndef _RTL8710_PERI_ON_H_
#define _RTL8710_PERI_ON_H_

#include <stdint.h>

typedef struct{
	volatile uint32_t PEON_PWR_CTRL;         // 0x0200
	volatile uint32_t PON_ISO_CTRL;          // 0x0204
	uint32_t RESERVED1[2];
	volatile uint32_t SOC_FUNC_EN;           // 0x0210
	volatile uint32_t SOC_HCI_COM_FUNC_EN;   // 0x0214
	volatile uint32_t SOC_PERI_FUNC0_EN;     // 0x0218
	volatile uint32_t SOC_PERI_FUNC1_EN;     // 0x021C
	volatile uint32_t SOC_PERI_DB_FUNC0_EN;  // 0x0220
	uint32_t RESERVED2[3];
	volatile uint32_t PESOC_CLK_CTRL;        // 0x0230
	volatile uint32_t PESOC_PERI_CLK_CTRL0;  // 0x0234
	volatile uint32_t PESOC_PERI_CLK_CTRL1;  // 0x0238
	volatile uint32_t PESOC_CLK_CTRL3;       // 0x023C
	volatile uint32_t PESOC_HCI_CLK_CTRL0;   // 0x0240
	volatile uint32_t PESOC_COM_CLK_CTRL1;   // 0x0244
	volatile uint32_t PESOC_HW_ENG_CLK_CTRL; // 0x0248
	uint32_t RESERVED3[1];
	volatile uint32_t PESOC_CLK_SEL;         // 0x0250
	uint32_t RESERVED4[6];
	volatile uint32_t SYS_ANACK_CAL_CTRL;    // 0x026C
	volatile uint32_t OSC32K_CTRL;           // 0x0270
	volatile uint32_t OSC32K_REG_CTRL0;      // 0x0274
	volatile uint32_t OSC32K_REG_CTRL1;      // 0x0278
	volatile uint32_t THERMAL_METER_CTRL;    // 0x027C
	volatile uint32_t UART_MUX_CTRL;         // 0x0280
	volatile uint32_t SPI_MUX_CTRL;          // 0x0284
	volatile uint32_t I2C_MUX_CTRL;          // 0x0288
	volatile uint32_t I2S_MUX_CTRL;          // 0x028C
	uint32_t RESERVED5[4];
	volatile uint32_t HCI_PINMUX_CTRL;       // 0x02A0
	volatile uint32_t WL_PINMUX_CTRL;        // 0x02A4
	volatile uint32_t BT_PINMUX_CTRL;        // 0x02A8
	volatile uint32_t PWM_PINMUX_CTRL;       // 0x02AC
	uint32_t RESERVED6[4];
	volatile uint32_t CPU_PERIPHERAL_CTRL;   // 0x02C0
	uint32_t RESERVED7[7];
	volatile uint32_t HCI_CTRL_STATUS_0;     // 0x02E0
	volatile uint32_t HCI_CTRL_STATUS_1;     // 0x02E4
	uint32_t RESERVED8[6];
	volatile uint32_t PESOC_MEM_CTRL;        // 0x0300
	volatile uint32_t PESOC_SOC_CTRL;        // 0x0304
	volatile uint32_t PESOC_PERI_CTRL;       // 0x0308
	uint32_t RESERVED9[5];
	volatile uint32_t GPIO_SHTDN_CTRL;       // 0x0320
	volatile uint32_t GPIO_DRIVING_CTRL;     // 0x0324
	uint32_t RESERVED10[2];
	volatile uint32_t GPIO_PULL_CTRL0;       // 0x0330
	volatile uint32_t GPIO_PULL_CTRL1;       // 0x0334
	volatile uint32_t GPIO_PULL_CTRL2;       // 0x0338
	volatile uint32_t GPIO_PULL_CTRL3;       // 0x033C
	volatile uint32_t GPIO_PULL_CTRL4;       // 0x0340
	volatile uint32_t GPIO_PULL_CTRL5;       // 0x0344
	volatile uint32_t GPIO_PULL_CTRL6;       // 0x0348
	uint32_t RESERVED11[5];
	volatile uint32_t PERI_PWM0_CTRL;        // 0x0360
	volatile uint32_t PERI_PWM1_CTRL;        // 0x0364
	volatile uint32_t PERI_PWM2_CTRL;        // 0x0368
	volatile uint32_t PERI_PWM3_CTRL;        // 0x036C
	volatile uint32_t PERI_TIM_EVT_CTRL;     // 0x0370
	volatile uint32_t PERI_EGTIM_CTRL;       // 0x0374
	uint32_t RESERVED12[30];
	volatile uint32_t PEON_CFG;              // 0x03F0
	volatile uint32_t PEON_STATUS;           // 0x03F4
}__attribute__((packed)) PERI_ON_TypeDef;

#define PERI_ON                                       ((PERI_ON_TypeDef *)0x40000200)

// PERI_ON_SOC_FUNC_EN
#define PERI_ON_SOC_FUNC_EN_FUN                       (((uint32_t)0x01) << 0)
#define PERI_ON_SOC_FUNC_EN_OCP                       (((uint32_t)0x01) << 1)
#define PERI_ON_SOC_FUNC_EN_LXBUS                     (((uint32_t)0x01) << 2)
#define PERI_ON_SOC_FUNC_EN_FLASH                     (((uint32_t)0x01) << 4)
#define PERI_ON_SOC_FUNC_EN_MEM_CTRL                  (((uint32_t)0x01) << 6)
#define PERI_ON_SOC_FUNC_EN_CPU                       (((uint32_t)0x01) << 8)
#define PERI_ON_SOC_FUNC_EN_LOG_UART                  (((uint32_t)0x01) << 12)
#define PERI_ON_SOC_FUNC_EN_GDMA0                     (((uint32_t)0x01) << 13)
#define PERI_ON_SOC_FUNC_EN_GDMA1                     (((uint32_t)0x01) << 14)
#define PERI_ON_SOC_FUNC_EN_GTIMER                    (((uint32_t)0x01) << 16)
#define PERI_ON_SOC_FUNC_EN_SECURITY_ENGINE           (((uint32_t)0x01) << 20)

// PERI_ON_SOC_PERI_FUNC1_EN
#define PERI_ON_SOC_PERI_FUNC1_EN_ADC0                (((uint32_t)0x01) << 0)
#define PERI_ON_SOC_PERI_FUNC1_EN_DAC0                (((uint32_t)0x01) << 4)
#define PERI_ON_SOC_PERI_FUNC1_EN_DAC1                (((uint32_t)0x01) << 5)
#define PERI_ON_SOC_PERI_FUNC1_EN_GPIO                (((uint32_t)0x01) << 8)

// PERI_ON_PESOC_CLK_CTRL
#define PERI_ON_CLK_CTRL_CKE_OCP                      (((uint32_t)0x01) << 0)
#define PERI_ON_CLK_CTRL_CKE_PLFM                     (((uint32_t)0x01) << 2)
#define PERI_ON_CLK_CTRL_ACTCK_TRACE_EN               (((uint32_t)0x01) << 4)
#define PERI_ON_CLK_CTRL_SLPCK_TRACE_EN               (((uint32_t)0x01) << 5)
#define PERI_ON_CLK_CTRL_ACTCK_VENDOR_REG_EN          (((uint32_t)0x01) << 6)
#define PERI_ON_CLK_CTRL_SLPCK_VENDOR_REG_EN          (((uint32_t)0x01) << 7)
#define PERI_ON_CLK_CTRL_ACTCK_FLASH_EN               (((uint32_t)0x01) << 8)
#define PERI_ON_CLK_CTRL_SLPCK_FLASH_EN               (((uint32_t)0x01) << 9)
#define PERI_ON_CLK_CTRL_ACTCK_SDR_EN                 (((uint32_t)0x01) << 10)
#define PERI_ON_CLK_CTRL_SLPCK_SDR_EN                 (((uint32_t)0x01) << 11)
#define PERI_ON_CLK_CTRL_ACTCK_LOG_UART_EN            (((uint32_t)0x01) << 12)
#define PERI_ON_CLK_CTRL_SLPCK_LOG_UART_EN            (((uint32_t)0x01) << 13)
#define PERI_ON_CLK_CTRL_ACTCK_TIMER_EN               (((uint32_t)0x01) << 14)
#define PERI_ON_CLK_CTRL_SLPCK_TIMER_EN               (((uint32_t)0x01) << 15)
#define PERI_ON_CLK_CTRL_ACTCK_GDMA0_EN               (((uint32_t)0x01) << 16)
#define PERI_ON_CLK_CTRL_SLPCK_GDMA0_EN               (((uint32_t)0x01) << 17)
#define PERI_ON_CLK_CTRL_ACTCK_GDMA1_EN               (((uint32_t)0x01) << 18)
#define PERI_ON_CLK_CTRL_SLPCK_GDMA1_EN               (((uint32_t)0x01) << 19)
#define PERI_ON_CLK_CTRL_ACTCK_GPIO_EN                (((uint32_t)0x01) << 24)
#define PERI_ON_CLK_CTRL_SLPCK_GPIO_EN                (((uint32_t)0x01) << 25)
#define PERI_ON_CLK_CTRL_ACTCK_BTCMD_EN               (((uint32_t)0x01) << 28)
#define PERI_ON_CLK_CTRL_SLPCK_BTCMD_EN               (((uint32_t)0x01) << 29)

// PERI_ON_CPU_PERIPHERAL_CTRL
#define PERI_ON_CPU_PERIPHERAL_CTRL_SPI_FLASH_PIN_EN  (((uint32_t)0x01) << 0)
#define PERI_ON_CPU_PERIPHERAL_CTRL_SPI_FLASH_PIN_SEL (((uint32_t)0x03) << 1)
#define PERI_ON_CPU_PERIPHERAL_CTRL_SDR_PIN_EN        (((uint32_t)0x01) << 4)
#define PERI_ON_CPU_PERIPHERAL_CTRL_SWD_PIN_EN        (((uint32_t)0x01) << 16)
#define PERI_ON_CPU_PERIPHERAL_CTRL_TRACE_PIN_EN      (((uint32_t)0x01) << 17)
#define PERI_ON_CPU_PERIPHERAL_CTRL_LOG_UART_PIN_EN   (((uint32_t)0x01) << 20)
#define PERI_ON_CPU_PERIPHERAL_CTRL_LOG_UART_IR_EN    (((uint32_t)0x01) << 21)
#define PERI_ON_CPU_PERIPHERAL_CTRL_LOG_UART_PIN_SEL  (((uint32_t)0x03) << 22)

#endif

