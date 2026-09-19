
#include "si5332_regs_def.h"


unsigned char s_active_regs_lookup[] =
	{
 DESIGN_ID0,
 DESIGN_ID1,
 DESIGN_ID2,
 DESIGN_ID3,
 DESIGN_ID4,
 DESIGN_ID5,
 IMUX_SEL,
 OMUX0_SEL0,
 OMUX0_SEL1,
 OMUX1_SEL0,
 OMUX1_SEL1,
 OMUX2_SEL0,
 OMUX2_SEL1,
 OMUX3_SEL0,
 OMUX3_SEL1,
 HSDIV0A_DIV,
 HSDIV1A_DIV,
 ID0A_INTG,
 ID0A_RES,
 ID0A_DEN,
 ID0A_SS_ENA,
 ID0A_SS_MODE,
 ID0B_SS_ENA,
 ID0B_SS_MODE,
 ID1A_SS_ENA,
 ID1A_SS_MODE
 ID1B_SS_ENA
 ID1B_SS_MODE,
 IDPA_INTG,
 IDPA_RES,
 IDPA_DEN,
 CLKIN_2_CLK_SEL,
 CLKIN_3_CLK_SEL,
 P_VAL,
 OUT0_MODE,
 OUT0_DIV,
 OUT0_SKEW,
 OUT0_STOP_HIGHZ,
 OUT0_CMOS_INV,
 OUT0_DIFF_INV,
 OUT1_MODE,
 OUT1_SKEW,
 OUT1_STOP_HIGHZ,
 OUT1_CMOS_INV,
 OUT1_DIFF_INV,
 OUT2_MODE,
 OUT2_DIV,
 OUT2_SKEW,
 OUT2_STOP_HIGHZ,
 OUT2_CMOS_INV,
 OUT2_DIFF_INV,
 OUT3_MODE,
 OUT3_SKEW
 OUT3_STOP_HIGHZ,
 OUT3_CMOS_INV,
 OUT3_DIFF_INV,
 OUT4_MODE,
 OUT4_SKEW,
 OUT4_STOP_HIGHZ,
 OUT4_CMOS_INV,
 OUT4_DIFF_INV,
 OUT5_MODE,
 OUT5_DIV,
 OUT5_SKEW,
 OUT5_STOP_HIGHZ,
 OUT5_CMOS_INV,
 OUT5_DIFF_INV,
 OUT6_MODE.
 OUT6_SKEW,
 OUT6_STOP_HIGHZ,
 OUT6_CMOS_INV,
 OUT6_DIFF_INV,
 OUT7_MODE,
 OUT7_DIV,
 OUT7_SKEW,
 OUT7_STOP_HIGHZ,
 OUT7_CMOS_INV,
 OUT7_DIFF_INV,
 OUT8_MODE,
 OUT8_DIV,
 OUT8_SKEW,
 OUT8_STOP_HIGHZ,
 OUT8_CMOS_INV,
 OUT8_DIFF_INV,
 OUT9_MODE,
 OUT9_SKEW,
 OUT9_STOP_HIGHZ,
 OUT9_CMOS_INV,
 OUT9_DIFF_INV,
 OUT10_MODE,
 OUT10_SKEW,
 OUT10_STOP_HIGHZ,
 OUT10_CMOS_INV
 OUT10_DIFF_INV,
 OUT11_MODE,
 OUT11_SKEW,
 OUT11_STOP_HIGHZ,
 OUT11_CMOS_INV,
 OUT11_DIFF_INV,
 OUT0_OE,
 OUT1_OE,
 OUT2_OE,
 OUT3_OE,
 OUT4_OE,
 OUT5_OE,
 OUT6_OE,
 OUT7_OE,
 OUT8_OE,
 OUT9_OE,
 OUT10_OE,
 OUT11_OE,
 XOSC_DIS,
 IBUF0_DIS,
 IBUF1_DIS,
 IMUX_DIS,
 PDIV_DIS,
 PLL_DIS,
 ID0_DIS,
 ID1_DIS,
 HSDIV0_DIS,
 HSDIV1_DIS,
 HSDIV2_DIS,
 HSDIV3_DIS,
 HSDIV4_DIS,
 OMUX0_DIS,
 OMUX1_DIS,
 OMUX2_DIS,
 OMUX3_DIS,
 OMUX4_DIS,
 OMUX5_DIS,
 OUT0_DIS,
 OUT1_DIS,
 OUT2_DIS,
 OUT3_DIS,
 OUT4_DIS,
 OUT5_DIS,
 OUT6_DIS,
 OUT7_DIS,
 OUT8_DIS,
 OUT9_DIS,
 OUT10_DIS,
 OUT11_DIS,
 PLL_MODE,
 XOSC_CINT_ENA,
 XOSC_CTRIM_XIN,
 XOSC_CTRIM_XOUT
 	};

#define TAG_CFG(TAG) TAG,#TAG

	 // typedef struct
  //    {
  //     unsigned int tag_no;
  //     const char *tag_name;
  //     unsigned char reg_no;
  //     unsigned char bit_ofs;
  //     unsigned char bit_width;
  // } si5332_reg_def_t;
  //

si5332_reg_def_t s_si5332_reg_defs[] =
 	{
 // *                   Location    Setting Name      Decimal Value      Hex Value
 // *                   ----------  ----------------  -----------------  -----------------
 // DESIGN_ID0,     // * 0x17[7:0]   DESIGN_ID0        65                 0x41
	{ TAG_CFG(DESIGN_ID0), 0x17, 0, 8 },
 // DESIGN_ID1,     // * 0x18[7:0]   DESIGN_ID1        70                 0x46
	{ TAG_CFG(DESIGN_ID1), 0x18, 0, 8 },
 // DESIGN_ID2,     // * 0x19[7:0]   DESIGN_ID2        52                 0x34
	{ TAG_CFG(DESIGN_ID2), 0x19, 0, 8 },
 // DESIGN_ID3,     // * 0x1A[7:0]   DESIGN_ID3        88                 0x58
	{ TAG_CFG(DESIGN_ID3), 0x1A, 0, 8 },
 // DESIGN_ID4,     // * 0x1B[7:0]   DESIGN_ID4        49                 0x31
	{ TAG_CFG(DESIGN_ID4), 0x1B, 0, 8 },
 // DESIGN_ID5,     // * 0x1C[7:0]   DESIGN_ID5        50                 0x32
	{ TAG_CFG(DESIGN_ID5), 0x1C, 0, 8 },
 // IMUX_SEL,       // * 0x24[1:0]   IMUX_SEL          1                  0x1
	{ TAG_CFG(IMUX_SEL), 0x24, 0, 2 },
 // OMUX0_SEL0,     // * 0x25[1:0]   OMUX0_SEL0        0                  0x0
 	{ TAG_CFG(OMUX0_SEL0),0x25, 0, 2 },
 // OMUX0_SEL1,     // * 0x25[6:4]   OMUX0_SEL1        0                  0x0
 	{ TAG_CFG(OMUX0_SEL1),0x25, 3, 2 },
 // OMUX1_SEL0,     // * 0x26[1:0]   OMUX1_SEL0        0                  0x0
 	{ TAG_CFG(OMUX1_SEL0),0x25, 0, 2 },
 // OMUX1_SEL1,     // * 0x26[6:4]   OMUX1_SEL1        5                  0x5
 	{ TAG_CFG(OMUX1_SEL1),0x26, 4, 3 },
 // OMUX2_SEL0,     // * 0x27[1:0]   OMUX2_SEL0        0                  0x0
 	{ TAG_CFG(OMUX2_SEL0),0x26, 0, 1 },
 // OMUX2_SEL1,     // * 0x27[6:4]   OMUX2_SEL1        1                  0x1
 	{ TAG_CFG(OMUX2_SEL1),0x27, 4, 3 },
 // OMUX3_SEL0,     // * 0x28[1:0]   OMUX3_SEL0        0                  0x0
 	{ TAG_CFG(OMUX3_SEL0),0x28, 0, 1 },
 // OMUX3_SEL1,     // * 0x28[6:4]   OMUX3_SEL1        0                  0x0
 	{ TAG_CFG(OMUX3_SEL1),0x28, 4, 3 },
 // HSDIV0A_DIV,    // * 0x2B[7:0]   HSDIV0A_DIV       26                 0x1A
 	{ TAG_CFG(HSDIV0A_DIV),0x2B, 0, 8 },
 // HSDIV1A_DIV,    // * 0x2D[7:0]   HSDIV1A_DIV       10                 0x0A
 	{ TAG_CFG(HSDIV1A_DIV),0x2D, 0, 8 },
 // ID0A_INTG,      // * 0x36[14:0]  ID0A_INTG         2129               0x0851
 	{ TAG_CFG(ID0A_INTG),0x36, 0, 15 },
 // ID0A_RES,       // * 0x38[14:0]  ID0A_RES          23                 0x0017
 	{ TAG_CFG(ID0A_RES),0x38, 0, 15 },
 // ID0A_DEN,       // * 0x3A[14:0]  ID0A_DEN          25                 0x0019
 	{ TAG_CFG(ID0A_DEN),0x3A, 0, 15 },
 // ID0A_SS_ENA,    // * 0x3C[0]     ID0A_SS_ENA       0                  0x0
 	{ TAG_CFG(ID0A_SS_ENA),0x3C, 0, 1 },
 // ID0A_SS_MODE,   // * 0x3C[2:1]   ID0A_SS_MODE      0                  0x0
 	{ TAG_CFG(ID0A_SS_MODE),0x3C, 1, 2 },
 // ID0B_SS_ENA,    // * 0x48[0]     ID0B_SS_ENA       0                  0x0
 	{ TAG_CFG(ID0B_SS_ENA), 0x48, 0, 1 },
 // ID0B_SS_MODE,   // * 0x48[2:1]   ID0B_SS_MODE      0                  0x0
 	{ TAG_CFG(ID0B_SS_MODE),0x48, 1, 2 },
 // ID1A_SS_ENA,    // * 0x54[0]     ID1A_SS_ENA       0                  0x0
 	{ TAG_CFG(ID1A_SS_ENA), 0x54, 0, 1 },
 // ID1A_SS_MODE    // * 0x54[2:1]   ID1A_SS_MODE      0                  0x0
 	{ TAG_CFG(ID1A_SS_MODE),0x54, 1, 2 },
 // ID1B_SS_ENA     // * 0x60[0]     ID1B_SS_ENA       0                  0x0
 	{ TAG_CFG(ID1B_SS_ENA), 0x60, 0, 1 },
 // ID1B_SS_MODE,   // * 0x60[2:1]   ID1B_SS_MODE      0                  0x0
 	{ TAG_CFG(ID1B_SS_MODE),0x60, 1, 2 },
 // IDPA_INTG,      // * 0x67[14:0]  IDPA_INTG         13312              0x3400
 	{ TAG_CFG(IDPA_INTG),0x67, 0, 15 },
 // IDPA_RES,       // * 0x69[14:0]  IDPA_RES          0                  0x0000
 	{ TAG_CFG(IDPA_RES),0x69, 0, 15 },
 // IDPA_DEN,       // * 0x6B[14:0]  IDPA_DEN          1                  0x0001
 	{ TAG_CFG(IDPA_DEN),0x6B, 0, 15 },
 // CLKIN_2_CLK_SEL,// * 0x73[1:0]   CLKIN_2_CLK_SEL   0                  0x0
 	{ TAG_CFG(CLKIN_2_CLK_SEL),0x73, 0, 2 },
 // CLKIN_3_CLK_SEL,// * 0x74[1:0]   CLKIN_3_CLK_SEL   0                  0x0
 	{ TAG_CFG(CLKIN_3_CLK_SEL),0x74, 0, 2 },
 // P_VAL,          // * 0x75[4:0]   P_VAL             1                  0x01
 	{ TAG_CFG(P_VAL),0x75, 0, 5 },
 // OUT0_MODE,      // * 0x7A[3:0]   OUT0_MODE         6                  0x6
 	{ TAG_CFG(OUT0_MODE),0x7A, 0, 4 },
 // OUT0_DIV,       // * 0x7B[5:0]   OUT0_DIV          1                  0x01
 // OUT0_SKEW,      // * 0x7C[2:0]   OUT0_SKEW         0                  0x0
 // OUT0_STOP_HIGHZ,// * 0x7D[0]     OUT0_STOP_HIGHZ   0                  0x0
 // OUT0_CMOS_INV,  // * 0x7D[5:4]   OUT0_CMOS_INV     0                  0x0
 // OUT0_DIFF_INV,  // * 0x7D[6]     OUT0_DIFF_INV     0                  0x0
 // OUT1_MODE,      // * 0x7F[3:0]   OUT1_MODE         7                  0x7
 	{ TAG_CFG(OUT1_MODE),0x7F, 0, 4 },
 // OUT1_SKEW,      // * 0x81[2:0]   OUT1_SKEW         0                  0x0
 // OUT1_STOP_HIGHZ,// * 0x82[0]     OUT1_STOP_HIGHZ   0                  0x0
 // OUT1_CMOS_INV,  // * 0x82[5:4]   OUT1_CMOS_INV     0                  0x0
 // OUT1_DIFF_INV,  // * 0x82[6]     OUT1_DIFF_INV     0                  0x0
 // OUT2_MODE,      // * 0x84[3:0]   OUT2_MODE         6                  0x6
 	{ TAG_CFG(OUT2_MODE),0x84, 0, 4 },
 // OUT2_DIV,       // * 0x85[5:0]   OUT2_DIV          1                  0x01
 // OUT2_SKEW,      // * 0x86[2:0]   OUT2_SKEW         0                  0x0
 // OUT2_STOP_HIGHZ,// * 0x87[0]     OUT2_STOP_HIGHZ   0                  0x0
 // OUT2_CMOS_INV,  // * 0x87[5:4]   OUT2_CMOS_INV     0                  0x0
 // OUT2_DIFF_INV,  // * 0x87[6]     OUT2_DIFF_INV     0                  0x0
 // OUT3_MODE,      // * 0x89[3:0]   OUT3_MODE         6                  0x6
 	{ TAG_CFG(OUT3_MODE),0x89, 0, 4 },
 // OUT3_SKEW       // * 0x8B[2:0]   OUT3_SKEW         0                  0x0
 // OUT3_STOP_HIGHZ,// * 0x8C[0]     OUT3_STOP_HIGHZ   0                  0x0
 // OUT3_CMOS_INV,  // * 0x8C[5:4]   OUT3_CMOS_INV     0                  0x0
 // OUT3_DIFF_INV,  // * 0x8C[6]     OUT3_DIFF_INV     0                  0x0
 // OUT4_MODE,      // * 0x8E[3:0]   OUT4_MODE         6                  0x6
 // OUT4_SKEW,      // * 0x90[2:0]   OUT4_SKEW         0                  0x0
 // OUT4_STOP_HIGHZ,// * 0x91[0]     OUT4_STOP_HIGHZ   0                  0x0
 // OUT4_CMOS_INV,  // * 0x91[5:4]   OUT4_CMOS_INV     0                  0x0
 // OUT4_DIFF_INV,  // * 0x91[6]     OUT4_DIFF_INV     0                  0x0
 // OUT5_MODE,      // * 0x93[3:0]   OUT5_MODE         6                  0x6
 	{ TAG_CFG(OUT5_MODE),0x93, 0, 4 },
 // OUT5_DIV,       // * 0x94[5:0]   OUT5_DIV          5                  0x05
 // OUT5_SKEW,      // * 0x95[2:0]   OUT5_SKEW         0                  0x0
 // OUT5_STOP_HIGHZ,// * 0x96[0]     OUT5_STOP_HIGHZ   0                  0x0
 // OUT5_CMOS_INV,  // * 0x96[5:4]   OUT5_CMOS_INV     0                  0x0
 // OUT5_DIFF_INV,  // * 0x96[6]     OUT5_DIFF_INV     0                  0x0
 // OUT6_MODE.      // * 0x98[3:0]   OUT6_MODE         7                  0x7
 // OUT6_SKEW,      // * 0x9A[2:0]   OUT6_SKEW         0                  0x0
 // OUT6_STOP_HIGHZ,// * 0x9B[0]     OUT6_STOP_HIGHZ   0                  0x0
 // OUT6_CMOS_INV,  // * 0x9B[5:4]   OUT6_CMOS_INV     0                  0x0
 // OUT6_DIFF_INV,  // * 0x9B[6]     OUT6_DIFF_INV     0                  0x0
 // OUT7_MODE,      // * 0x9D[3:0]   OUT7_MODE         6                  0x6
 // OUT7_DIV,       // * 0x9E[5:0]   OUT7_DIV          5                  0x05
 // OUT7_SKEW,      // * 0x9F[2:0]   OUT7_SKEW         0                  0x0
 // OUT7_STOP_HIGHZ,// * 0xA0[0]     OUT7_STOP_HIGHZ   0                  0x0
 // OUT7_CMOS_INV,  // * 0xA0[5:4]   OUT7_CMOS_INV     0                  0x0
 // OUT7_DIFF_INV,  // * 0xA0[6]     OUT7_DIFF_INV     0                  0x0
 // OUT8_MODE,      // * 0xA2[3:0]   OUT8_MODE         6                  0x6
 // OUT8_DIV,       // * 0xA3[5:0]   OUT8_DIV          5                  0x05
 // OUT8_SKEW,      // * 0xA4[2:0]   OUT8_SKEW         0                  0x0
 // OUT8_STOP_HIGHZ,// * 0xA5[0]     OUT8_STOP_HIGHZ   0                  0x0
 // OUT8_CMOS_INV,  // * 0xA5[5:4]   OUT8_CMOS_INV     0                  0x0
 // OUT8_DIFF_INV,  // * 0xA5[6]     OUT8_DIFF_INV     0                  0x0
 // OUT9_MODE,      // * 0xA7[3:0]   OUT9_MODE         7                  0x7
 // OUT9_SKEW,      // * 0xA9[2:0]   OUT9_SKEW         0                  0x0
 // OUT9_STOP_HIGHZ,// * 0xAA[0]     OUT9_STOP_HIGHZ   0                  0x0
 // OUT9_CMOS_INV,  // * 0xAA[5:4]   OUT9_CMOS_INV     0                  0x0
 // OUT9_DIFF_INV,  // * 0xAA[6]     OUT9_DIFF_INV     0                  0x0
 // OUT10_MODE,     // * 0xAC[3:0]   OUT10_MODE        6                  0x6
 // OUT10_SKEW,     // * 0xAE[2:0]   OUT10_SKEW        0                  0x0
 // OUT10_STOP_HIGHZ,// * 0xAF[0]     OUT10_STOP_HIGHZ  0                  0x0
 // OUT10_CMOS_INV   // * 0xAF[5:4]   OUT10_CMOS_INV    0                  0x0
 // OUT10_DIFF_INV,  // * 0xAF[6]     OUT10_DIFF_INV    0                  0x0
 // OUT11_MODE,      // * 0xB1[3:0]   OUT11_MODE        7                  0x7
 // OUT11_SKEW,      // * 0xB3[2:0]   OUT11_SKEW        0                  0x0
 // OUT11_STOP_HIGHZ,// * 0xB4[0]     OUT11_STOP_HIGHZ  0                  0x0
 // OUT11_CMOS_INV,  // * 0xB4[5:4]   OUT11_CMOS_INV    0                  0x0
 // OUT11_DIFF_INV,  // * 0xB4[6]     OUT11_DIFF_INV    0                  0x0
 // OUT0_OE,         // * 0xB6[0]     OUT0_OE           1                  0x1
 // OUT1_OE,         // * 0xB6[1]     OUT1_OE           0                  0x0
 // OUT2_OE,         // * 0xB6[2]     OUT2_OE           1                  0x1
 // OUT3_OE,         // * 0xB6[3]     OUT3_OE           0                  0x0
 // OUT4_OE,         // * 0xB6[4]     OUT4_OE           0                  0x0
 // OUT5_OE,         // * 0xB6[5]     OUT5_OE           1                  0x1
 // OUT6_OE,         // * 0xB6[6]     OUT6_OE           0                  0x0
 // OUT7_OE,         // * 0xB6[7]     OUT7_OE           1                  0x1
 // OUT8_OE,         // * 0xB7[0]     OUT8_OE           1                  0x1
 // OUT9_OE,         // * 0xB7[1]     OUT9_OE           0                  0x0
 // OUT10_OE,        // * 0xB7[2]     OUT10_OE          0                  0x0
 // OUT11_OE,        // * 0xB7[3]     OUT11_OE          0                  0x0
 // XOSC_DIS,        // * 0xB9[0]     XOSC_DIS          0                  0x0
 // IBUF0_DIS,       // * 0xB9[1]     IBUF0_DIS         1                  0x1
 // IBUF1_DIS,       // * 0xB9[2]     IBUF1_DIS         1                  0x1
 // IMUX_DIS,        // * 0xB9[3]     IMUX_DIS          0                  0x0
 // PDIV_DIS,        // * 0xB9[4]     PDIV_DIS          0                  0x0
 // PLL_DIS,         // * 0xB9[5]     PLL_DIS           0                  0x0
 // ID0_DIS,         // * 0xBA[5]     ID0_DIS           0                  0x0
 // ID1_DIS,         // * 0xBA[6]     ID1_DIS           1                  0x1
 // HSDIV0_DIS,      // * 0xBA[0]     HSDIV0_DIS        0                  0x0
 // HSDIV1_DIS,      // * 0xBA[1]     HSDIV1_DIS        0                  0x0
 // HSDIV2_DIS,      // * 0xBA[2]     HSDIV2_DIS        1                  0x1
 // HSDIV3_DIS,      // * 0xBA[3]     HSDIV3_DIS        1                  0x1
 // HSDIV4_DIS,      // * 0xBA[4]     HSDIV4_DIS        1                  0x1
 // OMUX0_DIS,       // * 0xBB[0]     OMUX0_DIS         0                  0x0
 // OMUX1_DIS,       // * 0xBB[1]     OMUX1_DIS         0                  0x0
 // OMUX2_DIS,       // * 0xBB[2]     OMUX2_DIS         0                  0x0
 // OMUX3_DIS,       // * 0xBB[3]     OMUX3_DIS         0                  0x0
 // OMUX4_DIS,       // * 0xBB[4]     OMUX4_DIS         1                  0x1
 // OMUX5_DIS,       // * 0xBB[5]     OMUX5_DIS         1                  0x1
 // OUT0_DIS,        // * 0xBC[0]     OUT0_DIS          0                  0x0
 // OUT1_DIS,        // * 0xBC[1]     OUT1_DIS          1                  0x1
 // OUT2_DIS,        // * 0xBC[2]     OUT2_DIS          0                  0x0
 // OUT3_DIS,        // * 0xBC[3]     OUT3_DIS          1                  0x1
 // OUT4_DIS,        // * 0xBC[4]     OUT4_DIS          1                  0x1
 // OUT5_DIS,        // * 0xBC[5]     OUT5_DIS          0                  0x0
 // OUT6_DIS,        // * 0xBC[6]     OUT6_DIS          1                  0x1
 // OUT7_DIS,        // * 0xBC[7]     OUT7_DIS          0                  0x0
 // OUT8_DIS,        // * 0xBD[0]     OUT8_DIS          0                  0x0
 // OUT9_DIS,        // * 0xBD[1]     OUT9_DIS          1                  0x1
 // OUT10_DIS,       // * 0xBD[2]     OUT10_DIS         1                  0x1
 // OUT11_DIS,       // * 0xBD[3]     OUT11_DIS         1                  0x1
 // PLL_MODE,        // * 0xBE[7:0]   PLL_MODE          16                 0x10
 // XOSC_CINT_ENA,   // * 0xBF[0]     XOSC_CINT_ENA     0                  0x0
 // XOSC_CTRIM_XIN,  // * 0xC0[5:0]   XOSC_CTRIM_XIN    15                 0x0F
		{ TAG_CFG(XOSC_CTRIM_XIN), 0xc0, 0, 6 },
 // XOSC_CTRIM_XOUT ´// * 0xC1[5:0]   XOSC_CTRIM_XOUT   15                 0x0F
		{ TAG_CFG(XOSC_CTRIM_XOUT), 0xc1, 0, 6 }

};

const unsigned char *si5332_active_regs_ptr()
  {
  return s_active_regs_lookup;
  }

size_t si5332_active_regs_size()
  {
  return sizeof(s_active_regs_lookup);
  }

si5332_reg_def_t *si5332_reg_defs_ptr()
  {
  return s_si5332_reg_defs;
  }

size_t  si5332_reg_defs_size()
  {
  return sizeof(s_si5332_reg_defs) / sizeof(si5332_reg_def_t);
  }

