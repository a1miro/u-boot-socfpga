/*
 * si5332_regs_def.h
 */

#ifndef INCLUDE_SI5332_REGS_DEF_H_
#define INCLUDE_SI5332_REGS_DEF_H_

#include <stdint.h>
#include <stddef.h>

 typedef struct
     {
      unsigned int tag_no;
      const char *tag_name;
      unsigned char reg_no;
      unsigned char bit_ofs;
      unsigned char bit_width;
  } si5332_reg_def_t;


 enum {
     DUMMY,
 // * Location    Setting Name      Decimal Value      Hex Value
 // * ----------  ----------------  -----------------  -----------------
 DESIGN_ID0,     // * 0x17[7:0]   DESIGN_ID0        65                 0x41
 DESIGN_ID1,     // * 0x18[7:0]   DESIGN_ID1        70                 0x46
 DESIGN_ID2,     // * 0x19[7:0]   DESIGN_ID2        52                 0x34
 DESIGN_ID3,     // * 0x1A[7:0]   DESIGN_ID3        88                 0x58
 DESIGN_ID4,     // * 0x1B[7:0]   DESIGN_ID4        49                 0x31
 DESIGN_ID5,     // * 0x1C[7:0]   DESIGN_ID5        50                 0x32
 IMUX_SEL,       // * 0x24[1:0]   IMUX_SEL          1                  0x1
 OMUX0_SEL0,     // * 0x25[1:0]   OMUX0_SEL0        0                  0x0
 OMUX0_SEL1,     // * 0x25[6:4]   OMUX0_SEL1        0                  0x0
 OMUX1_SEL0,     // * 0x26[1:0]   OMUX1_SEL0        0                  0x0
 OMUX1_SEL1,     // * 0x26[6:4]   OMUX1_SEL1        5                  0x5
 OMUX2_SEL0,     // * 0x27[1:0]   OMUX2_SEL0        0                  0x0
 OMUX2_SEL1,     // * 0x27[6:4]   OMUX2_SEL1        1                  0x1
 OMUX3_SEL0,     // * 0x28[1:0]   OMUX3_SEL0        0                  0x0
 OMUX3_SEL1,     // * 0x28[6:4]   OMUX3_SEL1        0                  0x0
 HSDIV0A_DIV,    // * 0x2B[7:0]   HSDIV0A_DIV       26                 0x1A
 HSDIV1A_DIV,    // * 0x2D[7:0]   HSDIV1A_DIV       10                 0x0A
 ID0A_INTG,      // * 0x36[14:0]  ID0A_INTG         2129               0x0851
 ID0A_RES,       // * 0x38[14:0]  ID0A_RES          23                 0x0017
 ID0A_DEN,       // * 0x3A[14:0]  ID0A_DEN          25                 0x0019
 ID0A_SS_ENA,    // * 0x3C[0]     ID0A_SS_ENA       0                  0x0
 ID0A_SS_MODE,   // * 0x3C[2:1]   ID0A_SS_MODE      0                  0x0
 ID0B_SS_ENA,    // * 0x48[0]     ID0B_SS_ENA       0                  0x0
 ID0B_SS_MODE,   // * 0x48[2:1]   ID0B_SS_MODE      0                  0x0
 ID1A_SS_ENA,    // * 0x54[0]     ID1A_SS_ENA       0                  0x0
 ID1A_SS_MODE    // * 0x54[2:1]   ID1A_SS_MODE      0                  0x0
 ID1B_SS_ENA     // * 0x60[0]     ID1B_SS_ENA       0                  0x0
 ID1B_SS_MODE,   // * 0x60[2:1]   ID1B_SS_MODE      0                  0x0
 IDPA_INTG,      // * 0x67[14:0]  IDPA_INTG         13312              0x3400
 IDPA_RES,       // * 0x69[14:0]  IDPA_RES          0                  0x0000
 IDPA_DEN,       // * 0x6B[14:0]  IDPA_DEN          1                  0x0001
 CLKIN_2_CLK_SEL,// * 0x73[1:0]   CLKIN_2_CLK_SEL   0                  0x0
 CLKIN_3_CLK_SEL,// * 0x74[1:0]   CLKIN_3_CLK_SEL   0                  0x0
 P_VAL,          // * 0x75[4:0]   P_VAL             1                  0x01
 OUT0_MODE,      // * 0x7A[3:0]   OUT0_MODE         6                  0x6
 OUT0_DIV,       // * 0x7B[5:0]   OUT0_DIV          1                  0x01
 OUT0_SKEW,      // * 0x7C[2:0]   OUT0_SKEW         0                  0x0
 OUT0_STOP_HIGHZ,// * 0x7D[0]     OUT0_STOP_HIGHZ   0                  0x0
 OUT0_CMOS_INV,  // * 0x7D[5:4]   OUT0_CMOS_INV     0                  0x0
 OUT0_DIFF_INV,  // * 0x7D[6]     OUT0_DIFF_INV     0                  0x0
 OUT1_MODE,      // * 0x7F[3:0]   OUT1_MODE         7                  0x7
 OUT1_SKEW,      // * 0x81[2:0]   OUT1_SKEW         0                  0x0
 OUT1_STOP_HIGHZ,// * 0x82[0]     OUT1_STOP_HIGHZ   0                  0x0
 OUT1_CMOS_INV,  // * 0x82[5:4]   OUT1_CMOS_INV     0                  0x0
 OUT1_DIFF_INV,  // * 0x82[6]     OUT1_DIFF_INV     0                  0x0
 OUT2_MODE,      // * 0x84[3:0]   OUT2_MODE         6                  0x6
 OUT2_DIV,       // * 0x85[5:0]   OUT2_DIV          1                  0x01
 OUT2_SKEW,      // * 0x86[2:0]   OUT2_SKEW         0                  0x0
 OUT2_STOP_HIGHZ,// * 0x87[0]     OUT2_STOP_HIGHZ   0                  0x0
 OUT2_CMOS_INV,  // * 0x87[5:4]   OUT2_CMOS_INV     0                  0x0
 OUT2_DIFF_INV,  // * 0x87[6]     OUT2_DIFF_INV     0                  0x0
 OUT3_MODE,      // * 0x89[3:0]   OUT3_MODE         6                  0x6
 OUT3_SKEW       // * 0x8B[2:0]   OUT3_SKEW         0                  0x0
 OUT3_STOP_HIGHZ,// * 0x8C[0]     OUT3_STOP_HIGHZ   0                  0x0
 OUT3_CMOS_INV,  // * 0x8C[5:4]   OUT3_CMOS_INV     0                  0x0
 OUT3_DIFF_INV,  // * 0x8C[6]     OUT3_DIFF_INV     0                  0x0
 OUT4_MODE,      // * 0x8E[3:0]   OUT4_MODE         6                  0x6
 OUT4_SKEW,      // * 0x90[2:0]   OUT4_SKEW         0                  0x0
 OUT4_STOP_HIGHZ,// * 0x91[0]     OUT4_STOP_HIGHZ   0                  0x0
 OUT4_CMOS_INV,  // * 0x91[5:4]   OUT4_CMOS_INV     0                  0x0
 OUT4_DIFF_INV,  // * 0x91[6]     OUT4_DIFF_INV     0                  0x0
 OUT5_MODE,      // * 0x93[3:0]   OUT5_MODE         6                  0x6
 OUT5_DIV,       // * 0x94[5:0]   OUT5_DIV          5                  0x05
 OUT5_SKEW,      // * 0x95[2:0]   OUT5_SKEW         0                  0x0
 OUT5_STOP_HIGHZ,// * 0x96[0]     OUT5_STOP_HIGHZ   0                  0x0
 OUT5_CMOS_INV,  // * 0x96[5:4]   OUT5_CMOS_INV     0                  0x0
 OUT5_DIFF_INV,  // * 0x96[6]     OUT5_DIFF_INV     0                  0x0
 OUT6_MODE.      // * 0x98[3:0]   OUT6_MODE         7                  0x7
 OUT6_SKEW,      // * 0x9A[2:0]   OUT6_SKEW         0                  0x0
 OUT6_STOP_HIGHZ,// * 0x9B[0]     OUT6_STOP_HIGHZ   0                  0x0
 OUT6_CMOS_INV,  // * 0x9B[5:4]   OUT6_CMOS_INV     0                  0x0
 OUT6_DIFF_INV,  // * 0x9B[6]     OUT6_DIFF_INV     0                  0x0
 OUT7_MODE,      // * 0x9D[3:0]   OUT7_MODE         6                  0x6
 OUT7_DIV,       // * 0x9E[5:0]   OUT7_DIV          5                  0x05
 OUT7_SKEW,      // * 0x9F[2:0]   OUT7_SKEW         0                  0x0
 OUT7_STOP_HIGHZ,// * 0xA0[0]     OUT7_STOP_HIGHZ   0                  0x0
 OUT7_CMOS_INV,  // * 0xA0[5:4]   OUT7_CMOS_INV     0                  0x0
 OUT7_DIFF_INV,  // * 0xA0[6]     OUT7_DIFF_INV     0                  0x0
 OUT8_MODE,      // * 0xA2[3:0]   OUT8_MODE         6                  0x6
 OUT8_DIV,       // * 0xA3[5:0]   OUT8_DIV          5                  0x05
 OUT8_SKEW,      // * 0xA4[2:0]   OUT8_SKEW         0                  0x0
 OUT8_STOP_HIGHZ,// * 0xA5[0]     OUT8_STOP_HIGHZ   0                  0x0
 OUT8_CMOS_INV,  // * 0xA5[5:4]   OUT8_CMOS_INV     0                  0x0
 OUT8_DIFF_INV,  // * 0xA5[6]     OUT8_DIFF_INV     0                  0x0
 OUT9_MODE,      // * 0xA7[3:0]   OUT9_MODE         7                  0x7
 OUT9_SKEW,      // * 0xA9[2:0]   OUT9_SKEW         0                  0x0
 OUT9_STOP_HIGHZ,// * 0xAA[0]     OUT9_STOP_HIGHZ   0                  0x0
 OUT9_CMOS_INV,  // * 0xAA[5:4]   OUT9_CMOS_INV     0                  0x0
 OUT9_DIFF_INV,  // * 0xAA[6]     OUT9_DIFF_INV     0                  0x0
 OUT10_MODE,     // * 0xAC[3:0]   OUT10_MODE        6                  0x6
 OUT10_SKEW,     // * 0xAE[2:0]   OUT10_SKEW        0                  0x0
 OUT10_STOP_HIGHZ,// * 0xAF[0]     OUT10_STOP_HIGHZ  0                  0x0
 OUT10_CMOS_INV   // * 0xAF[5:4]   OUT10_CMOS_INV    0                  0x0
 OUT10_DIFF_INV,  // * 0xAF[6]     OUT10_DIFF_INV    0                  0x0
 OUT11_MODE,      // * 0xB1[3:0]   OUT11_MODE        7                  0x7
 OUT11_SKEW,      // * 0xB3[2:0]   OUT11_SKEW        0                  0x0
 OUT11_STOP_HIGHZ,// * 0xB4[0]     OUT11_STOP_HIGHZ  0                  0x0
 OUT11_CMOS_INV,  // * 0xB4[5:4]   OUT11_CMOS_INV    0                  0x0
 OUT11_DIFF_INV,  // * 0xB4[6]     OUT11_DIFF_INV    0                  0x0
 OUT0_OE,         // * 0xB6[0]     OUT0_OE           1                  0x1
 OUT1_OE,         // * 0xB6[1]     OUT1_OE           0                  0x0
 OUT2_OE,         // * 0xB6[2]     OUT2_OE           1                  0x1
 OUT3_OE,         // * 0xB6[3]     OUT3_OE           0                  0x0
 OUT4_OE,         // * 0xB6[4]     OUT4_OE           0                  0x0
 OUT5_OE,         // * 0xB6[5]     OUT5_OE           1                  0x1
 OUT6_OE,         // * 0xB6[6]     OUT6_OE           0                  0x0
 OUT7_OE,         // * 0xB6[7]     OUT7_OE           1                  0x1
 OUT8_OE,         // * 0xB7[0]     OUT8_OE           1                  0x1
 OUT9_OE,         // * 0xB7[1]     OUT9_OE           0                  0x0
 OUT10_OE,        // * 0xB7[2]     OUT10_OE          0                  0x0
 OUT11_OE,        // * 0xB7[3]     OUT11_OE          0                  0x0
 XOSC_DIS,        // * 0xB9[0]     XOSC_DIS          0                  0x0
 IBUF0_DIS,       // * 0xB9[1]     IBUF0_DIS         1                  0x1
 IBUF1_DIS,       // * 0xB9[2]     IBUF1_DIS         1                  0x1
 IMUX_DIS,        // * 0xB9[3]     IMUX_DIS          0                  0x0
 PDIV_DIS,        // * 0xB9[4]     PDIV_DIS          0                  0x0
 PLL_DIS,         // * 0xB9[5]     PLL_DIS           0                  0x0
 ID0_DIS,         // * 0xBA[5]     ID0_DIS           0                  0x0
 ID1_DIS,         // * 0xBA[6]     ID1_DIS           1                  0x1
 HSDIV0_DIS,      // * 0xBA[0]     HSDIV0_DIS        0                  0x0
 HSDIV1_DIS,      // * 0xBA[1]     HSDIV1_DIS        0                  0x0
 HSDIV2_DIS,      // * 0xBA[2]     HSDIV2_DIS        1                  0x1
 HSDIV3_DIS,      // * 0xBA[3]     HSDIV3_DIS        1                  0x1
 HSDIV4_DIS,      // * 0xBA[4]     HSDIV4_DIS        1                  0x1
 OMUX0_DIS,       // * 0xBB[0]     OMUX0_DIS         0                  0x0
 OMUX1_DIS,       // * 0xBB[1]     OMUX1_DIS         0                  0x0
 OMUX2_DIS,       // * 0xBB[2]     OMUX2_DIS         0                  0x0
 OMUX3_DIS,       // * 0xBB[3]     OMUX3_DIS         0                  0x0
 OMUX4_DIS,       // * 0xBB[4]     OMUX4_DIS         1                  0x1
 OMUX5_DIS,       // * 0xBB[5]     OMUX5_DIS         1                  0x1
 OUT0_DIS,        // * 0xBC[0]     OUT0_DIS          0                  0x0
 OUT1_DIS,        // * 0xBC[1]     OUT1_DIS          1                  0x1
 OUT2_DIS,        // * 0xBC[2]     OUT2_DIS          0                  0x0
 OUT3_DIS,        // * 0xBC[3]     OUT3_DIS          1                  0x1
 OUT4_DIS,        // * 0xBC[4]     OUT4_DIS          1                  0x1
 OUT5_DIS,        // * 0xBC[5]     OUT5_DIS          0                  0x0
 OUT6_DIS,        // * 0xBC[6]     OUT6_DIS          1                  0x1
 OUT7_DIS,        // * 0xBC[7]     OUT7_DIS          0                  0x0
 OUT8_DIS,        // * 0xBD[0]     OUT8_DIS          0                  0x0
 OUT9_DIS,        // * 0xBD[1]     OUT9_DIS          1                  0x1
 OUT10_DIS,       // * 0xBD[2]     OUT10_DIS         1                  0x1
 OUT11_DIS,       // * 0xBD[3]     OUT11_DIS         1                  0x1
 PLL_MODE,        // * 0xBE[7:0]   PLL_MODE          16                 0x10
 XOSC_CINT_ENA,   // * 0xBF[0]     XOSC_CINT_ENA     0                  0x0
 XOSC_CTRIM_XIN,  // * 0xC0[5:0]   XOSC_CTRIM_XIN    15                 0x0F
 XOSC_CTRIM_XOUT ´// * 0xC1[5:0]   XOSC_CTRIM_XOUT   15                 0x0F
 };

extern afe_reg_def_t *si5332_reg_defs_ptr();
extern size_t  si5332_reg_defs_size();
extern const unsigned char * si5332_active_regs_ptr();
extern size_t si5332_active_regs_size();

#endif
