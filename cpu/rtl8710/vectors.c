/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_rtl8710
 * @{
 *
 * @file
 * @brief       Startup code and interrupt vector definition
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdint.h>
#include "vectors_cortexm.h"

/* get the start of the ISR stack as defined in the linkerscript */
extern uint32_t _estack;

/* define a local dummy handler as it needs to be in the same compilation unit
 * as the alias definition */
void dummy_handler(void) {
    dummy_handler_default();
}

/* Cortex-M3 common interrupt vectors */
WEAK_DEFAULT void isr_svc(void);
WEAK_DEFAULT void isr_pendsv(void);
WEAK_DEFAULT void isr_systick(void);

/* Realtek specific interrupt vector */
WEAK_DEFAULT void isr_0(void);
WEAK_DEFAULT void isr_1(void);
WEAK_DEFAULT void isr_2(void);
WEAK_DEFAULT void isr_3(void);
WEAK_DEFAULT void isr_4(void);
WEAK_DEFAULT void isr_5(void);
WEAK_DEFAULT void isr_6(void);
WEAK_DEFAULT void isr_7(void);
WEAK_DEFAULT void isr_8(void);
WEAK_DEFAULT void isr_9(void);
WEAK_DEFAULT void isr_10(void);
WEAK_DEFAULT void isr_11(void);
WEAK_DEFAULT void isr_12(void);
WEAK_DEFAULT void isr_13(void);
WEAK_DEFAULT void isr_14(void);
WEAK_DEFAULT void isr_15(void);
WEAK_DEFAULT void isr_16(void);
WEAK_DEFAULT void isr_17(void);
WEAK_DEFAULT void isr_18(void);
WEAK_DEFAULT void isr_19(void);
WEAK_DEFAULT void isr_20(void);
WEAK_DEFAULT void isr_21(void);
WEAK_DEFAULT void isr_22(void);
WEAK_DEFAULT void isr_23(void);
WEAK_DEFAULT void isr_24(void);
WEAK_DEFAULT void isr_25(void);
WEAK_DEFAULT void isr_26(void);
WEAK_DEFAULT void isr_27(void);
WEAK_DEFAULT void isr_28(void);
WEAK_DEFAULT void isr_29(void);
WEAK_DEFAULT void isr_30(void);
WEAK_DEFAULT void isr_31(void);
WEAK_DEFAULT void isr_32(void);
WEAK_DEFAULT void isr_33(void);
WEAK_DEFAULT void isr_34(void);
WEAK_DEFAULT void isr_35(void);
WEAK_DEFAULT void isr_36(void);
WEAK_DEFAULT void isr_37(void);
WEAK_DEFAULT void isr_38(void);
WEAK_DEFAULT void isr_39(void);
WEAK_DEFAULT void isr_40(void);
WEAK_DEFAULT void isr_41(void);
WEAK_DEFAULT void isr_42(void);
WEAK_DEFAULT void isr_43(void);
WEAK_DEFAULT void isr_44(void);
WEAK_DEFAULT void isr_45(void);
WEAK_DEFAULT void isr_46(void);
WEAK_DEFAULT void isr_47(void);
WEAK_DEFAULT void isr_48(void);
WEAK_DEFAULT void isr_49(void);
WEAK_DEFAULT void isr_50(void);
WEAK_DEFAULT void isr_51(void);
WEAK_DEFAULT void isr_52(void);
WEAK_DEFAULT void isr_53(void);
WEAK_DEFAULT void isr_54(void);
WEAK_DEFAULT void isr_55(void);
WEAK_DEFAULT void isr_56(void);
WEAK_DEFAULT void isr_57(void);
WEAK_DEFAULT void isr_58(void);
WEAK_DEFAULT void isr_59(void);
WEAK_DEFAULT void isr_60(void);
WEAK_DEFAULT void isr_61(void);
WEAK_DEFAULT void isr_62(void);
WEAK_DEFAULT void isr_63(void);
WEAK_DEFAULT void isr_64(void);
WEAK_DEFAULT void isr_65(void);
WEAK_DEFAULT void isr_66(void);
WEAK_DEFAULT void isr_67(void);
WEAK_DEFAULT void isr_68(void);
WEAK_DEFAULT void isr_69(void);
WEAK_DEFAULT void isr_70(void);
WEAK_DEFAULT void isr_71(void);
WEAK_DEFAULT void isr_72(void);
WEAK_DEFAULT void isr_73(void);
WEAK_DEFAULT void isr_74(void);
WEAK_DEFAULT void isr_75(void);
WEAK_DEFAULT void isr_76(void);
WEAK_DEFAULT void isr_77(void);
WEAK_DEFAULT void isr_78(void);
WEAK_DEFAULT void isr_79(void);
WEAK_DEFAULT void isr_80(void);
WEAK_DEFAULT void isr_81(void);
WEAK_DEFAULT void isr_82(void);
WEAK_DEFAULT void isr_83(void);
WEAK_DEFAULT void isr_84(void);
WEAK_DEFAULT void isr_85(void);
WEAK_DEFAULT void isr_86(void);
WEAK_DEFAULT void isr_87(void);
WEAK_DEFAULT void isr_88(void);
WEAK_DEFAULT void isr_89(void);
WEAK_DEFAULT void isr_90(void);
WEAK_DEFAULT void isr_91(void);
WEAK_DEFAULT void isr_92(void);
WEAK_DEFAULT void isr_93(void);
WEAK_DEFAULT void isr_94(void);
WEAK_DEFAULT void isr_95(void);
WEAK_DEFAULT void isr_96(void);
WEAK_DEFAULT void isr_97(void);
WEAK_DEFAULT void isr_98(void);
WEAK_DEFAULT void isr_99(void);
WEAK_DEFAULT void isr_100(void);
WEAK_DEFAULT void isr_101(void);
WEAK_DEFAULT void isr_102(void);
WEAK_DEFAULT void isr_103(void);
WEAK_DEFAULT void isr_104(void);
WEAK_DEFAULT void isr_105(void);
WEAK_DEFAULT void isr_106(void);
WEAK_DEFAULT void isr_107(void);
WEAK_DEFAULT void isr_108(void);
WEAK_DEFAULT void isr_109(void);
WEAK_DEFAULT void isr_110(void);
WEAK_DEFAULT void isr_111(void);
WEAK_DEFAULT void isr_112(void);
WEAK_DEFAULT void isr_113(void);
WEAK_DEFAULT void isr_114(void);
WEAK_DEFAULT void isr_115(void);
WEAK_DEFAULT void isr_116(void);
WEAK_DEFAULT void isr_117(void);
WEAK_DEFAULT void isr_118(void);
WEAK_DEFAULT void isr_119(void);
WEAK_DEFAULT void isr_120(void);
WEAK_DEFAULT void isr_121(void);
WEAK_DEFAULT void isr_122(void);
WEAK_DEFAULT void isr_123(void);
WEAK_DEFAULT void isr_124(void);
WEAK_DEFAULT void isr_125(void);
WEAK_DEFAULT void isr_126(void);
WEAK_DEFAULT void isr_127(void);
WEAK_DEFAULT void isr_128(void);
WEAK_DEFAULT void isr_129(void);
WEAK_DEFAULT void isr_130(void);
WEAK_DEFAULT void isr_131(void);
WEAK_DEFAULT void isr_132(void);
WEAK_DEFAULT void isr_133(void);
WEAK_DEFAULT void isr_134(void);
WEAK_DEFAULT void isr_135(void);
WEAK_DEFAULT void isr_136(void);
WEAK_DEFAULT void isr_137(void);
WEAK_DEFAULT void isr_138(void);
WEAK_DEFAULT void isr_139(void);
WEAK_DEFAULT void isr_140(void);
WEAK_DEFAULT void isr_141(void);
WEAK_DEFAULT void isr_142(void);
WEAK_DEFAULT void isr_143(void);
WEAK_DEFAULT void isr_144(void);
WEAK_DEFAULT void isr_145(void);
WEAK_DEFAULT void isr_146(void);
WEAK_DEFAULT void isr_147(void);
WEAK_DEFAULT void isr_148(void);
WEAK_DEFAULT void isr_149(void);
WEAK_DEFAULT void isr_150(void);
WEAK_DEFAULT void isr_151(void);
WEAK_DEFAULT void isr_152(void);
WEAK_DEFAULT void isr_153(void);
WEAK_DEFAULT void isr_154(void);
WEAK_DEFAULT void isr_155(void);
WEAK_DEFAULT void isr_156(void);
WEAK_DEFAULT void isr_157(void);
WEAK_DEFAULT void isr_158(void);
WEAK_DEFAULT void isr_159(void);
WEAK_DEFAULT void isr_160(void);
WEAK_DEFAULT void isr_161(void);
WEAK_DEFAULT void isr_162(void);
WEAK_DEFAULT void isr_163(void);
WEAK_DEFAULT void isr_164(void);
WEAK_DEFAULT void isr_165(void);
WEAK_DEFAULT void isr_166(void);
WEAK_DEFAULT void isr_167(void);
WEAK_DEFAULT void isr_168(void);
WEAK_DEFAULT void isr_169(void);
WEAK_DEFAULT void isr_170(void);
WEAK_DEFAULT void isr_171(void);
WEAK_DEFAULT void isr_172(void);
WEAK_DEFAULT void isr_173(void);
WEAK_DEFAULT void isr_174(void);
WEAK_DEFAULT void isr_175(void);
WEAK_DEFAULT void isr_176(void);
WEAK_DEFAULT void isr_177(void);
WEAK_DEFAULT void isr_178(void);
WEAK_DEFAULT void isr_179(void);
WEAK_DEFAULT void isr_180(void);
WEAK_DEFAULT void isr_181(void);
WEAK_DEFAULT void isr_182(void);
WEAK_DEFAULT void isr_183(void);
WEAK_DEFAULT void isr_184(void);
WEAK_DEFAULT void isr_185(void);
WEAK_DEFAULT void isr_186(void);
WEAK_DEFAULT void isr_187(void);
WEAK_DEFAULT void isr_188(void);
WEAK_DEFAULT void isr_189(void);
WEAK_DEFAULT void isr_190(void);
WEAK_DEFAULT void isr_191(void);
WEAK_DEFAULT void isr_192(void);
WEAK_DEFAULT void isr_193(void);
WEAK_DEFAULT void isr_194(void);
WEAK_DEFAULT void isr_195(void);
WEAK_DEFAULT void isr_196(void);
WEAK_DEFAULT void isr_197(void);
WEAK_DEFAULT void isr_198(void);
WEAK_DEFAULT void isr_199(void);
WEAK_DEFAULT void isr_200(void);
WEAK_DEFAULT void isr_201(void);
WEAK_DEFAULT void isr_202(void);
WEAK_DEFAULT void isr_203(void);
WEAK_DEFAULT void isr_204(void);
WEAK_DEFAULT void isr_205(void);
WEAK_DEFAULT void isr_206(void);
WEAK_DEFAULT void isr_207(void);
WEAK_DEFAULT void isr_208(void);
WEAK_DEFAULT void isr_209(void);
WEAK_DEFAULT void isr_210(void);
WEAK_DEFAULT void isr_211(void);
WEAK_DEFAULT void isr_212(void);
WEAK_DEFAULT void isr_213(void);
WEAK_DEFAULT void isr_214(void);
WEAK_DEFAULT void isr_215(void);
WEAK_DEFAULT void isr_216(void);
WEAK_DEFAULT void isr_217(void);
WEAK_DEFAULT void isr_218(void);
WEAK_DEFAULT void isr_219(void);
WEAK_DEFAULT void isr_220(void);
WEAK_DEFAULT void isr_221(void);
WEAK_DEFAULT void isr_222(void);
WEAK_DEFAULT void isr_223(void);
WEAK_DEFAULT void isr_224(void);
WEAK_DEFAULT void isr_225(void);
WEAK_DEFAULT void isr_226(void);
WEAK_DEFAULT void isr_227(void);
WEAK_DEFAULT void isr_228(void);
WEAK_DEFAULT void isr_229(void);
WEAK_DEFAULT void isr_230(void);
WEAK_DEFAULT void isr_231(void);
WEAK_DEFAULT void isr_232(void);
WEAK_DEFAULT void isr_233(void);
WEAK_DEFAULT void isr_234(void);
WEAK_DEFAULT void isr_235(void);
WEAK_DEFAULT void isr_236(void);
WEAK_DEFAULT void isr_237(void);
WEAK_DEFAULT void isr_238(void);
WEAK_DEFAULT void isr_239(void);

/* interrupt vector table */
ISR_VECTORS const void *interrupt_vector[] = {
    /* Exception stack pointer */
    (void*) (&_estack),             /* pointer to the top of the stack */

    /* Cortex M3 handlers */
    (void*) reset_handler_default,  /* entry point of the program */
    (void*) nmi_default,            /* non maskable interrupt handler */
    (void*) hard_fault_default,     /* hard fault exception */
    (void*) mem_manage_default,     /* memory manage exception */
    (void*) bus_fault_default,      /* bus fault exception */
    (void*) usage_fault_default,    /* usage fault exception */

    (void*) (0UL),                  /* Reserved */
    (void*) (0UL),                  /* Reserved */
    (void*) (0UL),                  /* Reserved */
    (void*) (0UL),                  /* Reserved */
    (void*) isr_svc,                /* system call interrupt, in RIOT used for
                                     * switching into thread context on boot */
    (void*) debug_mon_default,      /* debug monitor exception */
    (void*) (0UL),                  /* Reserved */
    (void*) isr_pendsv,             /* pendSV interrupt, in RIOT the actual
                                     * context switching is happening here */
    (void*) isr_systick,            /* SysTick interrupt, not used in RIOT */

    /* RTL8710 specific peripheral handlers */
    (void*) isr_0,                  /* 0 - ISR 0 */
    (void*) isr_1,                  /* 1 - ISR 1 */
    (void*) isr_2,                  /* 2 - ISR 2 */
    (void*) isr_3,                  /* 3 - ISR 3 */
    (void*) isr_4,                  /* 4 - ISR 4 */
    (void*) isr_5,                  /* 5 - ISR 5 */
    (void*) isr_6,                  /* 6 - ISR 6 */
    (void*) isr_7,                  /* 7 - ISR 7 */
    (void*) isr_8,                  /* 8 - ISR 8 */
    (void*) isr_9,                  /* 9 - ISR 9 */
    (void*) isr_10,                 /* 10 - ISR 10 */
    (void*) isr_11,                 /* 11 - ISR 11 */
    (void*) isr_12,                 /* 12 - ISR 12 */
    (void*) isr_13,                 /* 13 - ISR 13 */
    (void*) isr_14,                 /* 14 - ISR 14 */
    (void*) isr_15,                 /* 15 - ISR 15 */
    (void*) isr_16,                 /* 16 - ISR 16 */
    (void*) isr_17,                 /* 17 - ISR 17 */
    (void*) isr_18,                 /* 18 - ISR 18 */
    (void*) isr_19,                 /* 19 - ISR 19 */
    (void*) isr_20,                 /* 20 - ISR 20 */
    (void*) isr_21,                 /* 21 - ISR 21 */
    (void*) isr_22,                 /* 22 - ISR 22 */
    (void*) isr_23,                 /* 23 - ISR 23 */
    (void*) isr_24,                 /* 24 - ISR 24 */
    (void*) isr_25,                 /* 25 - ISR 25 */
    (void*) isr_26,                 /* 26 - ISR 26 */
    (void*) isr_27,                 /* 27 - ISR 27 */
    (void*) isr_28,                 /* 28 - ISR 28 */
    (void*) isr_29,                 /* 29 - ISR 29 */
    (void*) isr_30,                 /* 30 - ISR 30 */
    (void*) isr_31,                 /* 31 - ISR 31 */
    (void*) isr_32,                 /* 32 - ISR 32 */
    (void*) isr_33,                 /* 33 - ISR 33 */
    (void*) isr_34,                 /* 34 - ISR 34 */
    (void*) isr_35,                 /* 35 - ISR 35 */
    (void*) isr_36,                 /* 36 - ISR 36 */
    (void*) isr_37,                 /* 37 - ISR 37 */
    (void*) isr_38,                 /* 38 - ISR 38 */
    (void*) isr_39,                 /* 39 - ISR 39 */
    (void*) isr_40,                 /* 40 - ISR 40 */
    (void*) isr_41,                 /* 41 - ISR 41 */
    (void*) isr_42,                 /* 42 - ISR 42 */
    (void*) isr_43,                 /* 43 - ISR 43 */
    (void*) isr_44,                 /* 44 - ISR 44 */
    (void*) isr_45,                 /* 45 - ISR 45 */
    (void*) isr_46,                 /* 46 - ISR 46 */
    (void*) isr_47,                 /* 47 - ISR 47 */
    (void*) isr_48,                 /* 48 - ISR 48 */
    (void*) isr_49,                 /* 49 - ISR 49 */
    (void*) isr_50,                 /* 50 - ISR 50 */
    (void*) isr_51,                 /* 51 - ISR 51 */
    (void*) isr_52,                 /* 52 - ISR 52 */
    (void*) isr_53,                 /* 53 - ISR 53 */
    (void*) isr_54,                 /* 54 - ISR 54 */
    (void*) isr_55,                 /* 55 - ISR 55 */
    (void*) isr_56,                 /* 56 - ISR 56 */
    (void*) isr_57,                 /* 57 - ISR 57 */
    (void*) isr_58,                 /* 58 - ISR 58 */
    (void*) isr_59,                 /* 59 - ISR 59 */
    (void*) isr_60,                 /* 60 - ISR 60 */
    (void*) isr_61,                 /* 61 - ISR 61 */
    (void*) isr_62,                 /* 62 - ISR 62 */
    (void*) isr_63,                 /* 63 - ISR 63 */
    (void*) isr_64,                 /* 64 - ISR 64 */
    (void*) isr_65,                 /* 65 - ISR 65 */
    (void*) isr_66,                 /* 66 - ISR 66 */
    (void*) isr_67,                 /* 67 - ISR 67 */
    (void*) isr_68,                 /* 68 - ISR 68 */
    (void*) isr_69,                 /* 69 - ISR 69 */
    (void*) isr_70,                 /* 70 - ISR 70 */
    (void*) isr_71,                 /* 71 - ISR 71 */
    (void*) isr_72,                 /* 72 - ISR 72 */
    (void*) isr_73,                 /* 73 - ISR 73 */
    (void*) isr_74,                 /* 74 - ISR 74 */
    (void*) isr_75,                 /* 75 - ISR 75 */
    (void*) isr_76,                 /* 76 - ISR 76 */
    (void*) isr_77,                 /* 77 - ISR 77 */
    (void*) isr_78,                 /* 78 - ISR 78 */
    (void*) isr_79,                 /* 79 - ISR 79 */
    (void*) isr_80,                 /* 80 - ISR 80 */
    (void*) isr_81,                 /* 81 - ISR 81 */
    (void*) isr_82,                 /* 82 - ISR 82 */
    (void*) isr_83,                 /* 83 - ISR 83 */
    (void*) isr_84,                 /* 84 - ISR 84 */
    (void*) isr_85,                 /* 85 - ISR 85 */
    (void*) isr_86,                 /* 86 - ISR 86 */
    (void*) isr_87,                 /* 87 - ISR 87 */
    (void*) isr_88,                 /* 88 - ISR 88 */
    (void*) isr_89,                 /* 89 - ISR 89 */
    (void*) isr_90,                 /* 90 - ISR 90 */
    (void*) isr_91,                 /* 91 - ISR 91 */
    (void*) isr_92,                 /* 92 - ISR 92 */
    (void*) isr_93,                 /* 93 - ISR 93 */
    (void*) isr_94,                 /* 94 - ISR 94 */
    (void*) isr_95,                 /* 95 - ISR 95 */
    (void*) isr_96,                 /* 96 - ISR 96 */
    (void*) isr_97,                 /* 97 - ISR 97 */
    (void*) isr_98,                 /* 98 - ISR 98 */
    (void*) isr_99,                 /* 99 - ISR 99 */
    (void*) isr_100,                /* 100 - ISR 100 */
    (void*) isr_101,                /* 101 - ISR 101 */
    (void*) isr_102,                /* 102 - ISR 102 */
    (void*) isr_103,                /* 103 - ISR 103 */
    (void*) isr_104,                /* 104 - ISR 104 */
    (void*) isr_105,                /* 105 - ISR 105 */
    (void*) isr_106,                /* 106 - ISR 106 */
    (void*) isr_107,                /* 107 - ISR 107 */
    (void*) isr_108,                /* 108 - ISR 108 */
    (void*) isr_109,                /* 109 - ISR 109 */
    (void*) isr_110,                /* 110 - ISR 110 */
    (void*) isr_111,                /* 111 - ISR 111 */
    (void*) isr_112,                /* 112 - ISR 112 */
    (void*) isr_113,                /* 113 - ISR 113 */
    (void*) isr_114,                /* 114 - ISR 114 */
    (void*) isr_115,                /* 115 - ISR 115 */
    (void*) isr_116,                /* 116 - ISR 116 */
    (void*) isr_117,                /* 117 - ISR 117 */
    (void*) isr_118,                /* 118 - ISR 118 */
    (void*) isr_119,                /* 119 - ISR 119 */
    (void*) isr_120,                /* 120 - ISR 120 */
    (void*) isr_121,                /* 121 - ISR 121 */
    (void*) isr_122,                /* 122 - ISR 122 */
    (void*) isr_123,                /* 123 - ISR 123 */
    (void*) isr_124,                /* 124 - ISR 124 */
    (void*) isr_125,                /* 125 - ISR 125 */
    (void*) isr_126,                /* 126 - ISR 126 */
    (void*) isr_127,                /* 127 - ISR 127 */
    (void*) isr_128,                /* 128 - ISR 128 */
    (void*) isr_129,                /* 129 - ISR 129 */
    (void*) isr_130,                /* 130 - ISR 130 */
    (void*) isr_131,                /* 131 - ISR 131 */
    (void*) isr_132,                /* 132 - ISR 132 */
    (void*) isr_133,                /* 133 - ISR 133 */
    (void*) isr_134,                /* 134 - ISR 134 */
    (void*) isr_135,                /* 135 - ISR 135 */
    (void*) isr_136,                /* 136 - ISR 136 */
    (void*) isr_137,                /* 137 - ISR 137 */
    (void*) isr_138,                /* 138 - ISR 138 */
    (void*) isr_139,                /* 139 - ISR 139 */
    (void*) isr_140,                /* 140 - ISR 140 */
    (void*) isr_141,                /* 141 - ISR 141 */
    (void*) isr_142,                /* 142 - ISR 142 */
    (void*) isr_143,                /* 143 - ISR 143 */
    (void*) isr_144,                /* 144 - ISR 144 */
    (void*) isr_145,                /* 145 - ISR 145 */
    (void*) isr_146,                /* 146 - ISR 146 */
    (void*) isr_147,                /* 147 - ISR 147 */
    (void*) isr_148,                /* 148 - ISR 148 */
    (void*) isr_149,                /* 149 - ISR 149 */
    (void*) isr_150,                /* 150 - ISR 150 */
    (void*) isr_151,                /* 151 - ISR 151 */
    (void*) isr_152,                /* 152 - ISR 152 */
    (void*) isr_153,                /* 153 - ISR 153 */
    (void*) isr_154,                /* 154 - ISR 154 */
    (void*) isr_155,                /* 155 - ISR 155 */
    (void*) isr_156,                /* 156 - ISR 156 */
    (void*) isr_157,                /* 157 - ISR 157 */
    (void*) isr_158,                /* 158 - ISR 158 */
    (void*) isr_159,                /* 159 - ISR 159 */
    (void*) isr_160,                /* 160 - ISR 160 */
    (void*) isr_161,                /* 161 - ISR 161 */
    (void*) isr_162,                /* 162 - ISR 162 */
    (void*) isr_163,                /* 163 - ISR 163 */
    (void*) isr_164,                /* 164 - ISR 164 */
    (void*) isr_165,                /* 165 - ISR 165 */
    (void*) isr_166,                /* 166 - ISR 166 */
    (void*) isr_167,                /* 167 - ISR 167 */
    (void*) isr_168,                /* 168 - ISR 168 */
    (void*) isr_169,                /* 169 - ISR 169 */
    (void*) isr_170,                /* 170 - ISR 170 */
    (void*) isr_171,                /* 171 - ISR 171 */
    (void*) isr_172,                /* 172 - ISR 172 */
    (void*) isr_173,                /* 173 - ISR 173 */
    (void*) isr_174,                /* 174 - ISR 174 */
    (void*) isr_175,                /* 175 - ISR 175 */
    (void*) isr_176,                /* 176 - ISR 176 */
    (void*) isr_177,                /* 177 - ISR 177 */
    (void*) isr_178,                /* 178 - ISR 178 */
    (void*) isr_179,                /* 179 - ISR 179 */
    (void*) isr_180,                /* 180 - ISR 180 */
    (void*) isr_181,                /* 181 - ISR 181 */
    (void*) isr_182,                /* 182 - ISR 182 */
    (void*) isr_183,                /* 183 - ISR 183 */
    (void*) isr_184,                /* 184 - ISR 184 */
    (void*) isr_185,                /* 185 - ISR 185 */
    (void*) isr_186,                /* 186 - ISR 186 */
    (void*) isr_187,                /* 187 - ISR 187 */
    (void*) isr_188,                /* 188 - ISR 188 */
    (void*) isr_189,                /* 189 - ISR 189 */
    (void*) isr_190,                /* 190 - ISR 190 */
    (void*) isr_191,                /* 191 - ISR 191 */
    (void*) isr_192,                /* 192 - ISR 192 */
    (void*) isr_193,                /* 193 - ISR 193 */
    (void*) isr_194,                /* 194 - ISR 194 */
    (void*) isr_195,                /* 195 - ISR 195 */
    (void*) isr_196,                /* 196 - ISR 196 */
    (void*) isr_197,                /* 197 - ISR 197 */
    (void*) isr_198,                /* 198 - ISR 198 */
    (void*) isr_199,                /* 199 - ISR 199 */
    (void*) isr_200,                /* 200 - ISR 200 */
    (void*) isr_201,                /* 201 - ISR 201 */
    (void*) isr_202,                /* 202 - ISR 202 */
    (void*) isr_203,                /* 203 - ISR 203 */
    (void*) isr_204,                /* 204 - ISR 204 */
    (void*) isr_205,                /* 205 - ISR 205 */
    (void*) isr_206,                /* 206 - ISR 206 */
    (void*) isr_207,                /* 207 - ISR 207 */
    (void*) isr_208,                /* 208 - ISR 208 */
    (void*) isr_209,                /* 209 - ISR 209 */
    (void*) isr_210,                /* 210 - ISR 210 */
    (void*) isr_211,                /* 211 - ISR 211 */
    (void*) isr_212,                /* 212 - ISR 212 */
    (void*) isr_213,                /* 213 - ISR 213 */
    (void*) isr_214,                /* 214 - ISR 214 */
    (void*) isr_215,                /* 215 - ISR 215 */
    (void*) isr_216,                /* 216 - ISR 216 */
    (void*) isr_217,                /* 217 - ISR 217 */
    (void*) isr_218,                /* 218 - ISR 218 */
    (void*) isr_219,                /* 219 - ISR 219 */
    (void*) isr_220,                /* 220 - ISR 220 */
    (void*) isr_221,                /* 221 - ISR 221 */
    (void*) isr_222,                /* 222 - ISR 222 */
    (void*) isr_223,                /* 223 - ISR 223 */
    (void*) isr_224,                /* 224 - ISR 224 */
    (void*) isr_225,                /* 225 - ISR 225 */
    (void*) isr_226,                /* 226 - ISR 226 */
    (void*) isr_227,                /* 227 - ISR 227 */
    (void*) isr_228,                /* 228 - ISR 228 */
    (void*) isr_229,                /* 229 - ISR 229 */
    (void*) isr_230,                /* 230 - ISR 230 */
    (void*) isr_231,                /* 231 - ISR 231 */
    (void*) isr_232,                /* 232 - ISR 232 */
    (void*) isr_233,                /* 233 - ISR 233 */
    (void*) isr_234,                /* 234 - ISR 234 */
    (void*) isr_235,                /* 235 - ISR 235 */
    (void*) isr_236,                /* 236 - ISR 236 */
    (void*) isr_237,                /* 237 - ISR 237 */
    (void*) isr_238,                /* 238 - ISR 238 */
    (void*) isr_239,                /* 239 - ISR 239 */
};
