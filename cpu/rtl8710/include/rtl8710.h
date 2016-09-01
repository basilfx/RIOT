#ifndef _RTL8710_H_
#define _RTL8710_H_

#include "rtl8710_sys.h"
#include "rtl8710_int.h"
#include "rtl8710_peri_on.h"
#include "rtl8710_timer.h"
#include "rtl8710_gpio.h"
#include "rtl8710_log_uart.h"
#include "rtl8710_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __CM3_REV                 0x0001
#define __MPU_PRESENT                  0
#define __NVIC_PRIO_BITS               3
#define __Vendor_SysTickConfig         0
#define __FPU_PRESENT                  0

typedef enum IRQn
{
/******  Cortex-M0+ Processor Exceptions Numbers *****************************************/
  NonMaskableInt_IRQn = -14,                /*!< 2 Cortex-M0+ Non Maskable Interrupt     */
  HardFault_IRQn      = -13,                /*!< 3 Cortex-M0+ Hard Fault Interrupt       */
  SVCall_IRQn         = -5,                 /*!< 11 Cortex-M0+ SV Call Interrupt         */
  PendSV_IRQn         = -2,                 /*!< 14 Cortex-M0+ Pend SV Interrupt         */
  SysTick_IRQn        = -1,                 /*!< 15 Cortex-M0+ System Tick Interrupt     */

/******  EFM32HG Peripheral Interrupt Numbers *********************************************/
  ISR0_IRQn           = 0,
  ISR1_IRQn           = 1,
  ISR2_IRQn           = 2,
  ISR3_IRQn           = 3,
  ISR4_IRQn           = 4,
  ISR5_IRQn           = 5,
  ISR6_IRQn           = 6,
  ISR7_IRQn           = 7,
  ISR8_IRQn           = 8,
  ISR9_IRQn           = 9,
  ISR10_IRQn          = 10,
  ISR11_IRQn          = 11,
  ISR12_IRQn          = 12,
  ISR13_IRQn          = 13,
  ISR14_IRQn          = 14,
  ISR15_IRQn          = 15,
  ISR16_IRQn          = 16,
  ISR17_IRQn          = 17,
  ISR18_IRQn          = 18,
  ISR19_IRQn          = 19,
  ISR20_IRQn          = 20,
  ISR21_IRQn          = 21,
  ISR22_IRQn          = 22,
  ISR23_IRQn          = 23,
  ISR24_IRQn          = 24,
  ISR25_IRQn          = 25,
  ISR26_IRQn          = 26,
  ISR27_IRQn          = 27,
  ISR28_IRQn          = 28,
  ISR29_IRQn          = 29,
  ISR30_IRQn          = 30,
  ISR31_IRQn          = 31,
  ISR32_IRQn          = 32,
  ISR33_IRQn          = 33,
  ISR34_IRQn          = 34,
  ISR35_IRQn          = 35,
  ISR36_IRQn          = 36,
  ISR37_IRQn          = 37,
  ISR38_IRQn          = 38,
  ISR39_IRQn          = 39,
  ISR40_IRQn          = 40,
  ISR41_IRQn          = 41,
  ISR42_IRQn          = 42,
  ISR43_IRQn          = 43,
  ISR44_IRQn          = 44,
  ISR45_IRQn          = 45,
  ISR46_IRQn          = 46,
  ISR47_IRQn          = 47,
  ISR48_IRQn          = 48,
  ISR49_IRQn          = 49,
  ISR50_IRQn          = 50,
  ISR51_IRQn          = 51,
  ISR52_IRQn          = 52,
  ISR53_IRQn          = 53,
  ISR54_IRQn          = 54,
  ISR55_IRQn          = 55,
  ISR56_IRQn          = 56,
  ISR57_IRQn          = 57,
  ISR58_IRQn          = 58,
  ISR59_IRQn          = 59,
  ISR60_IRQn          = 60,
  ISR61_IRQn          = 61,
  ISR62_IRQn          = 62,
  ISR63_IRQn          = 63,
  ISR64_IRQn          = 64,
  ISR65_IRQn          = 65,
  ISR66_IRQn          = 66,
  ISR67_IRQn          = 67,
  ISR68_IRQn          = 68,
  ISR69_IRQn          = 69,
  ISR70_IRQn          = 70,
  ISR71_IRQn          = 71,
  ISR72_IRQn          = 72,
  ISR73_IRQn          = 73,
  ISR74_IRQn          = 74,
  ISR75_IRQn          = 75,
  ISR76_IRQn          = 76,
  ISR77_IRQn          = 77,
  ISR78_IRQn          = 78,
  ISR79_IRQn          = 79,
  ISR80_IRQn          = 80,
  ISR81_IRQn          = 81,
  ISR82_IRQn          = 82,
  ISR83_IRQn          = 83,
  ISR84_IRQn          = 84,
  ISR85_IRQn          = 85,
  ISR86_IRQn          = 86,
  ISR87_IRQn          = 87,
  ISR88_IRQn          = 88,
  ISR89_IRQn          = 89,
  ISR90_IRQn          = 90,
  ISR91_IRQn          = 91,
  ISR92_IRQn          = 92,
  ISR93_IRQn          = 93,
  ISR94_IRQn          = 94,
  ISR95_IRQn          = 95,
  ISR96_IRQn          = 96,
  ISR97_IRQn          = 97,
  ISR98_IRQn          = 98,
  ISR99_IRQn          = 99,
  ISR100_IRQn         = 100,
  ISR101_IRQn         = 101,
  ISR102_IRQn         = 102,
  ISR103_IRQn         = 103,
  ISR104_IRQn         = 104,
  ISR105_IRQn         = 105,
  ISR106_IRQn         = 106,
  ISR107_IRQn         = 107,
  ISR108_IRQn         = 108,
  ISR109_IRQn         = 109,
  ISR110_IRQn         = 110,
  ISR111_IRQn         = 111,
  ISR112_IRQn         = 112,
  ISR113_IRQn         = 113,
  ISR114_IRQn         = 114,
  ISR115_IRQn         = 115,
  ISR116_IRQn         = 116,
  ISR117_IRQn         = 117,
  ISR118_IRQn         = 118,
  ISR119_IRQn         = 119,
  ISR120_IRQn         = 120,
  ISR121_IRQn         = 121,
  ISR122_IRQn         = 122,
  ISR123_IRQn         = 123,
  ISR124_IRQn         = 124,
  ISR125_IRQn         = 125,
  ISR126_IRQn         = 126,
  ISR127_IRQn         = 127,
  ISR128_IRQn         = 128,
  ISR129_IRQn         = 129,
  ISR130_IRQn         = 130,
  ISR131_IRQn         = 131,
  ISR132_IRQn         = 132,
  ISR133_IRQn         = 133,
  ISR134_IRQn         = 134,
  ISR135_IRQn         = 135,
  ISR136_IRQn         = 136,
  ISR137_IRQn         = 137,
  ISR138_IRQn         = 138,
  ISR139_IRQn         = 139,
  ISR140_IRQn         = 140,
  ISR141_IRQn         = 141,
  ISR142_IRQn         = 142,
  ISR143_IRQn         = 143,
  ISR144_IRQn         = 144,
  ISR145_IRQn         = 145,
  ISR146_IRQn         = 146,
  ISR147_IRQn         = 147,
  ISR148_IRQn         = 148,
  ISR149_IRQn         = 149,
  ISR150_IRQn         = 150,
  ISR151_IRQn         = 151,
  ISR152_IRQn         = 152,
  ISR153_IRQn         = 153,
  ISR154_IRQn         = 154,
  ISR155_IRQn         = 155,
  ISR156_IRQn         = 156,
  ISR157_IRQn         = 157,
  ISR158_IRQn         = 158,
  ISR159_IRQn         = 159,
  ISR160_IRQn         = 160,
  ISR161_IRQn         = 161,
  ISR162_IRQn         = 162,
  ISR163_IRQn         = 163,
  ISR164_IRQn         = 164,
  ISR165_IRQn         = 165,
  ISR166_IRQn         = 166,
  ISR167_IRQn         = 167,
  ISR168_IRQn         = 168,
  ISR169_IRQn         = 169,
  ISR170_IRQn         = 170,
  ISR171_IRQn         = 171,
  ISR172_IRQn         = 172,
  ISR173_IRQn         = 173,
  ISR174_IRQn         = 174,
  ISR175_IRQn         = 175,
  ISR176_IRQn         = 176,
  ISR177_IRQn         = 177,
  ISR178_IRQn         = 178,
  ISR179_IRQn         = 179,
  ISR180_IRQn         = 180,
  ISR181_IRQn         = 181,
  ISR182_IRQn         = 182,
  ISR183_IRQn         = 183,
  ISR184_IRQn         = 184,
  ISR185_IRQn         = 185,
  ISR186_IRQn         = 186,
  ISR187_IRQn         = 187,
  ISR188_IRQn         = 188,
  ISR189_IRQn         = 189,
  ISR190_IRQn         = 190,
  ISR191_IRQn         = 191,
  ISR192_IRQn         = 192,
  ISR193_IRQn         = 193,
  ISR194_IRQn         = 194,
  ISR195_IRQn         = 195,
  ISR196_IRQn         = 196,
  ISR197_IRQn         = 197,
  ISR198_IRQn         = 198,
  ISR199_IRQn         = 199,
  ISR200_IRQn         = 200,
  ISR201_IRQn         = 201,
  ISR202_IRQn         = 202,
  ISR203_IRQn         = 203,
  ISR204_IRQn         = 204,
  ISR205_IRQn         = 205,
  ISR206_IRQn         = 206,
  ISR207_IRQn         = 207,
  ISR208_IRQn         = 208,
  ISR209_IRQn         = 209,
  ISR210_IRQn         = 210,
  ISR211_IRQn         = 211,
  ISR212_IRQn         = 212,
  ISR213_IRQn         = 213,
  ISR214_IRQn         = 214,
  ISR215_IRQn         = 215,
  ISR216_IRQn         = 216,
  ISR217_IRQn         = 217,
  ISR218_IRQn         = 218,
  ISR219_IRQn         = 219,
  ISR220_IRQn         = 220,
  ISR221_IRQn         = 221,
  ISR222_IRQn         = 222,
  ISR223_IRQn         = 223,
  ISR224_IRQn         = 224,
  ISR225_IRQn         = 225,
  ISR226_IRQn         = 226,
  ISR227_IRQn         = 227,
  ISR228_IRQn         = 228,
  ISR229_IRQn         = 229,
  ISR230_IRQn         = 230,
  ISR231_IRQn         = 231,
  ISR232_IRQn         = 232,
  ISR233_IRQn         = 233,
  ISR234_IRQn         = 234,
  ISR235_IRQn         = 235,
  ISR236_IRQn         = 236,
  ISR237_IRQn         = 237,
  ISR238_IRQn         = 238,
  ISR239_IRQn         = 239,
  ISR240_IRQn         = 240
} IRQn_Type;

#include <core_cm3.h>

#ifdef __cplusplus
}
#endif

#endif
