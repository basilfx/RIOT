#ifndef _RTL8710_SYS_H_
#define _RTL8710_SYS_H_

#include <stdint.h>

// ~/programming/rtl8710/doc/registers/8195a/fwlib/rtl8195a/rtl8195a_sys_on.h

typedef struct{
	volatile uint16_t PWR_CTRL;               // 0x0000
	volatile uint16_t ISO_CTRL;               // 0x0002
	uint32_t RESERVED1[1];
	volatile uint32_t FUNC_EN;                // 0x0008
	uint32_t RESERVED2[1];
	volatile uint32_t CLK_CTRL0;              // 0x0010
	volatile uint32_t CLK_CTRL1;              // 0x0014
	uint32_t RESERVED3[2];
	volatile uint32_t EFUSE_SYSCFG0;          // 0x0020
	volatile uint32_t EFUSE_SYSCFG1;          // 0x0024
	volatile uint32_t EFUSE_SYSCFG2;          // 0x0028
	volatile uint32_t EFUSE_SYSCFG3;          // 0x002C
	volatile uint32_t EFUSE_SYSCFG4;          // 0x0030
	volatile uint32_t EFUSE_SYSCFG5;          // 0x0034
	volatile uint32_t EFUSE_SYSCFG6;          // 0x0038
	volatile uint32_t EFUSE_SYSCFG7;          // 0x003C
	volatile uint32_t REGU_CTRL0;             // 0x0040
	uint32_t RESERVED4[1];
	volatile uint32_t SWR_CTRL0;              // 0x0048
	volatile uint32_t SWR_CTRL1;              // 0x004C
	uint32_t RESERVED5[4];
	volatile uint32_t XTAL_CTRL0;             // 0x0060
	volatile uint32_t XTAL_CTRL1;             // 0x0064
	uint32_t RESERVED6[2];
	volatile uint32_t SYSPLL_CTRL0;           // 0x0070
	volatile uint32_t SYSPLL_CTRL1;           // 0x0074
	volatile uint32_t SYSPLL_CTRL2;           // 0x0078
	uint32_t RESERVED7[5];
	volatile uint32_t ANA_TIM_CTRL;           // 0x0090
	volatile uint32_t DSLP_TIM_CTRL;          // 0x0094
	volatile uint32_t DSLP_TIM_CAL_CTRL;      // 0x0098
	uint32_t RESERVED8[1];
	volatile uint32_t DEBUG_CTRL;             // 0x00A0
	volatile uint32_t PINMUX_CTRL;            // 0x00A4
	volatile uint32_t GPIO_DSTBY_WAKE_CTRL0;  // 0x00A8
	volatile uint32_t GPIO_DSTBY_WAKE_CTRL1;  // 0x00AC
	uint32_t RESERVED9[3];
	volatile uint32_t DEBUG_REG;              // 0x00BC
	uint32_t RESERVED10[8];
	volatile uint32_t EEPROM_CTRL0;           // 0x00E0
	volatile uint32_t EEPROM_CTRL1;           // 0x00E4
	volatile uint32_t EFUSE_CTRL;             // 0x00E8
	volatile uint32_t EFUSE_TEST;             // 0x00EC
	volatile uint32_t DSTBY_INFO0;            // 0x00F0
	volatile uint32_t DSTBY_INFO1;            // 0x00F4
	volatile uint32_t DSTBY_INFO2;            // 0x00F8
	volatile uint32_t DSTBY_INFO3;            // 0x00FC
	volatile uint32_t SLP_WAKE_EVENT_MSK0;    // 0x0100
	volatile uint32_t SLP_WAKE_EVENT_MSK1;    // 0x0104
	volatile uint32_t SLP_WAKE_EVENT_STATUS0; // 0x0108
	volatile uint32_t SLP_WAKE_EVENT_STATUS1; // 0x010C
	volatile uint32_t SNF_WAKE_EVENT_MSK0;    // 0x0110
	volatile uint32_t SNF_WAKE_EVENT_STATUS;  // 0x0114
	volatile uint32_t PWRMGT_CTRL;            // 0x0118
	uint32_t RESERVED11[1];
	volatile uint32_t PWRMGT_OPTION;          // 0x0120
	volatile uint32_t PWRMGT_OPTION_EXT;      // 0x0124
	uint32_t RESERVED12[2];
	volatile uint32_t DSLP_WEVENT;            // 0x0130
	volatile uint32_t PERI_MONITOR;           // 0x0134
	uint32_t RESERVED13[46];
	volatile uint32_t SYSTEM_CFG0;            // 0x01F0
	volatile uint32_t SYSTEM_CFG1;            // 0x01F4
	volatile uint32_t SYSTEM_CFG2;            // 0x01F8
}__attribute__((packed)) SYS_TypeDef;

#define SYS                                      ((SYS_TypeDef *)0x40000000)

// SYS_PWR_CTRL
#define SYS_PWR_CTRL_PEON_EN                     (((uint16_t)0x01) << 0)
#define SYS_PWR_CTRL_RET_MEM_EN                  (((uint16_t)0x01) << 1)
#define SYS_PWR_CTRL_SOC_EN                      (((uint16_t)0x01) << 2)

// SYS_ISO_CTRL
#define SYS_ISO_CTRL_PEON                        (((uint16_t)0x01) << 0)
#define SYS_ISO_CTRL_RET_MEM                     (((uint16_t)0x01) << 1)
#define SYS_ISO_CTRL_SOC                         (((uint16_t)0x01) << 2)
#define SYS_ISO_CTRL_SYSPLL                      (((uint16_t)0x01) << 7)

// SYS_FUNC_EN
#define SYS_FUNC_EN_FEN_EELDR                    (((uint32_t)0x01) << 0)
#define SYS_FUNC_EN_SOC_SYSPEON_EN               (((uint32_t)0x01) << 4)
#define SYS_FUNC_EN_FEN_SIC                      (((uint32_t)0x01) << 24)
#define SYS_FUNC_EN_FEN_SIC_MST                  (((uint32_t)0x01) << 25)
#define SYS_FUNC_EN_PWRON_TRAP_SHTDN_N           (((uint32_t)0x01) << 30)
#define SYS_FUNC_EN_AMACRO_EN                    (((uint32_t)0x01) << 31)

// SYS_CLK_CTRL0
#define SYS_CLK_CTRL0_CK_SYSREG_EN               (((uint32_t)0x01) << 0)
#define SYS_CLK_CTRL0_CK_EELDR_EN                (((uint32_t)0x01) << 1)
#define SYS_CLK_CTRL0_SOC_OCP_IOBUS_CK_EN        (((uint32_t)0x01) << 2)

// SYS_CLK_CTRL1
#define SYS_CLK_CTRL1_PESOC_EELDR_CK_SEL         (((uint32_t)0x01) << 0)
#define SYS_CLK_CTRL1_PESOC_OCP_CPU_CK_SEL       (((uint32_t)0x07) << 4)

#endif

