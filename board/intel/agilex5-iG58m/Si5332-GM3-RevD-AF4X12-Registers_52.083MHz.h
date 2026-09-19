/*
 * Si5332-GM3 Rev D Configuration Register Export Header File
 *
 * This file represents a series of Skyworks Si5332-GM3 Rev D 
 * register writes that can be performed to load a single configuration 
 * on a device. It was created by a Skyworks ClockBuilder Pro
 * export tool.
 *
 * Part:		                                       Si5332-GM3 Rev D
 * Design ID:                                          AF4X12
 * Includes Pre/Post Download Control Register Writes: Yes
 * Created By:                                         ClockBuilder Pro v4.18 [2026-03-23]
 * Timestamp:                                          2026-06-18 07:44:14 GMT+02:00
 *
 * A complete design report corresponding to this export is included at the end 
 * of this header file.
 *
 */

#ifndef SI5332_GM3_REVD_REG_CONFIG_HEADER
#define SI5332_GM3_REVD_REG_CONFIG_HEADER

#define SI5332_GM3_REVD_REG_CONFIG_NUM_REGS				81

const char * Si5332_design_info =
#ifdef HEADER_NAME
  "[" HEADER_NAME "]\n"
#endif
" * Outputs:\n"
" *    OUT0: 100 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low\n"
" *          Power-up state: Enabled\n"
" *    OUT1: Unused\n"
" *    OUT2: 156.25 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low\n"
" *          Power-up state: Enabled\n"
" *    OUT3: 156.25 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low\n"
" *          Power-up state: Enabled\n"
" *    OUT4: Unused\n"
" *    OUT5: 52.0833333333333333... MHz [ 52 + 1/12 MHz ] LVDS Fast 3.3 V, Disabled-State: Stop Low\n"
" *          [ OUT3/3 ]\n"
" *          Power-up state: Enabled\n"
" *    OUT6: Unused\n"
" *    OUT7: 20 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low\n"
" *          Power-up state: Enabled\n"
" *    OUT8: 20 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low\n"
" *          Power-up state: Enabled\n"
" *    OUT9: Unused\n"
" *   OUT10: Unused\n"
" *   OUT11: Unused\n";

typedef struct
{
	unsigned int address; /* 8-bit register address */
	unsigned char value; /* 8-bit register data */

} si5332_gm3_revd_register_t;

si5332_gm3_revd_register_t const si5332_gm3_revd_registers[SI5332_GM3_REVD_REG_CONFIG_NUM_REGS] =
{

	/* Start configuration preamble */
	/*    Set device in Ready mode */
	{ 0x0006, 0x01 },
	/* End configuration preamble */

	/* Start configuration registers */
	{ 0x0017, 0x41 },
	{ 0x0018, 0x46 },
	{ 0x0019, 0x34 },
	{ 0x001A, 0x58 },
	{ 0x001B, 0x31 },
	{ 0x001C, 0x32 },
	{ 0x0024, 0x01 },
	{ 0x0025, 0x00 },
	{ 0x0026, 0x10 },
	{ 0x0027, 0x10 },
	{ 0x0028, 0x00 },
	{ 0x002B, 0x19 },
	{ 0x002D, 0x10 },
	{ 0x003C, 0x00 },
	{ 0x0048, 0x00 },
	{ 0x0054, 0x00 },
	{ 0x0060, 0x00 },
	{ 0x0067, 0x32 },
	{ 0x0068, 0x00 },
	{ 0x0069, 0x00 },
	{ 0x006A, 0x00 },
	{ 0x006B, 0x00 },
	{ 0x006C, 0x01 },
	{ 0x0073, 0x00 },
	{ 0x0074, 0x00 },
	{ 0x0075, 0x01 },
	{ 0x007A, 0x06 },
	{ 0x007B, 0x01 },
	{ 0x007C, 0x00 },
	{ 0x007D, 0x00 },
	{ 0x007F, 0x07 },
	{ 0x0081, 0x00 },
	{ 0x0082, 0x00 },
	{ 0x0084, 0x06 },
	{ 0x0085, 0x01 },
	{ 0x0086, 0x00 },
	{ 0x0087, 0x00 },
	{ 0x0089, 0x06 },
	{ 0x008A, 0x01 },
	{ 0x008B, 0x00 },
	{ 0x008C, 0x00 },
	{ 0x008E, 0x06 },
	{ 0x0090, 0x00 },
	{ 0x0091, 0x00 },
	{ 0x0093, 0x06 },
	{ 0x0094, 0x03 },
	{ 0x0095, 0x00 },
	{ 0x0096, 0x00 },
	{ 0x0098, 0x07 },
	{ 0x009A, 0x00 },
	{ 0x009B, 0x00 },
	{ 0x009D, 0x06 },
	{ 0x009E, 0x05 },
	{ 0x009F, 0x00 },
	{ 0x00A0, 0x00 },
	{ 0x00A2, 0x06 },
	{ 0x00A3, 0x05 },
	{ 0x00A4, 0x00 },
	{ 0x00A5, 0x00 },
	{ 0x00A7, 0x07 },
	{ 0x00A9, 0x00 },
	{ 0x00AA, 0x00 },
	{ 0x00AC, 0x06 },
	{ 0x00AE, 0x00 },
	{ 0x00AF, 0x00 },
	{ 0x00B1, 0x07 },
	{ 0x00B3, 0x00 },
	{ 0x00B4, 0x00 },
	{ 0x00B6, 0xAD },
	{ 0x00B7, 0x01 },
	{ 0x00B9, 0x06 },
	{ 0x00BA, 0x7C },
	{ 0x00BB, 0x30 },
	{ 0x00BC, 0x52 },
	{ 0x00BD, 0x0E },
	{ 0x00BE, 0x10 },
	{ 0x00BF, 0x00 },
	{ 0x00C0, 0x0F },
	{ 0x00C1, 0x0F },
	/* End configuration registers */

	/* Start configuration postamble */
	/*    Set device in Active mode */
	{ 0x0006, 0x02 },
	/* End configuration postamble */

};

/*
 * Design Report
 *
 * Overview
 * ========
 * 
 * Part:               Si5332ABCD-GM3 Rev D
 * Project File:       C:\Users\tbj\OneDrive - Fortifi Food Processing\Proj\AF4\Evaluation\Agilex\iWave\Si5332-GM3-RevD-AF4X12-Project.slabtimeproj
 * Design ID:          AF4X12
 * Created By:         ClockBuilder Pro v4.18 [2026-03-23]
 * Timestamp:          2026-06-18 07:44:14 GMT+02:00
 * 
 * Design Rule Check
 * =================
 * 
 * Errors:
 * - No errors
 * 
 * Warnings:
 * - No warnings
 * 
 * Device Grade
 * ============
 * Maximum Output Frequency: 156.25 MHz
 * Frequency Synthesis Mode: Integer
 * Frequency Plan Grade:     D
 * Minimum Base OPN:         Si5332D*
 * 
 * Base       Output Clock         Supported Frequency Synthesis Modes
 * OPN Grade  Frequency Range      (Typical Jitter)
 * ---------  ------------------  --------------------------------------------
 * Si5332A    5 MHz to 334 MHz    Integer (~230 fs) and fractional (~500 fs)
 * Si5332B    5 MHz to 200 MHz    "
 * Si5332C    5 MHz to 334 MHz    Integer only (~230 fs)
 * Si5332D*   5 MHz to 200 MHz    "
 * 
 * * Based on your calculated frequency plan, a Si5332D grade device is
 * sufficient for your design. For more in-system configuration flexibility
 * (higher frequencies and/or to enable fractional synthesis), consider
 * selecting device grade Si5332A when specifying an ordering part number (OPN)
 * for your application. See the datasheet Ordering Guide for more information.
 * 
 * Design
 * ======
 * Base I2C Address: 0x6A
 * 
 * Universal Hardware Input Pins:
 *    INPUT1 (P10): None                          
 *    INPUT2 (P11): None                          
 *    INPUT3 (P12): None                          
 *    INPUT4 (P23): None                          
 *    INPUT5 (P38): None                          
 *    INPUT6 (P42): None                          
 *    INPUT7 (P43): None                          
 * 
 * Inputs:
 *    XAXB: 25 MHz
 *          Crystal Mode
 *          Adjusted Capacitance: 6 pF
 *  CLKIN2: Unused
 *  CLKIN3: Unused
 * 
 * Outputs:
 *    OUT0: 100 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low
 *          Power-up state: Enabled
 *    OUT1: Unused
 *    OUT2: 156.25 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low
 *          Power-up state: Enabled
 *    OUT3: 156.25 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low
 *          Power-up state: Enabled
 *    OUT4: Unused
 *    OUT5: 52.0833333333333333... MHz [ 52 + 1/12 MHz ] LVDS Fast 3.3 V, Disabled-State: Stop Low
 *          [ OUT3/3 ]
 *          Power-up state: Enabled
 *    OUT6: Unused
 *    OUT7: 20 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low
 *          Power-up state: Enabled
 *    OUT8: 20 MHz LVDS Fast 3.3 V, Disabled-State: Stop Low
 *          Power-up state: Enabled
 *    OUT9: Unused
 *   OUT10: Unused
 *   OUT11: Unused
 * 
 * Frequency Plan
 * ==============
 * 
 * Fpfd = 25 MHz
 * Fvco = 2.5 GHz
 * 
 * P divider = 1
 * M = 100
 * N dividers:
 *    N0:
 *       Unused
 *    N1:
 *       Unused
 * 
 * O dividers:
 *    O0:
 *       Value: 25
 *       OUT0: 100 MHz, Error: 0 ppm
 *       OUT7: 20 MHz, Error: 0 ppm
 *       OUT8: 20 MHz, Error: 0 ppm
 *    O1:
 *       Value: 16
 *       OUT2: 156.25 MHz, Error: 0 ppm
 *       OUT3: 156.25 MHz, Error: 0 ppm
 *       OUT5: 52.08333... MHz [ 52 + 1/12 MHz ], Error: 0 ppm
 *    O2:
 *       Unused
 *    O3:
 *       Unused
 *    O4:
 *       Unused
 * 
 * R dividers:
 *    R0 = 1
 *    R1 = Unused
 *    R2 = 1
 *    R3 = 1
 *    R4 = Unused
 *    R5 = 3
 *    R6 = Unused
 *    R7 = 5
 *    R8 = 5
 *    R9 = Unused
 *    R10 = Unused
 *    R11 = Unused
 * 
 * Estimated Power
 * ===============
 * Assumptions:
 * 
 * VDD:      1.8 V
 * Ta:       25 °C
 * Theta-JA: 21.80 °C/W (JEDEC Board with 2 m/s airflow)
 * 
 *                               Overall  On Chip
 * Condition                     Power    Power    Ta    Tj
 * ----------------------------  -------  -------  ----  ----
 * Typical Ta, Voltage, Current  259 mW   249 mW   25 C  30 C
 * 
 *                                               -----------------------
 *                                                       Typical        
 *                                               -----------------------
 *                                               Voltage  Current  Power
 *           Output  Frequency         Format      (V)     (mA)    (mW) 
 *           ------  ----------------  --------  -------  -------  -----
 * VDDA                                             1.80       21     38
 * VDD Dig                                          1.80        5      8
 * VDD Xtal                                         1.80        2      4
 * VDDO0     OUT0             100 MHz  LVDSFast     3.30       11     35
 * VDDO1     OUT1              Unused               3.30       11     37
 *           OUT2          156.25 MHz  LVDSFast
 * VDDO2     OUT3          156.25 MHz  LVDSFast     3.30       21     71
 *           OUT4              Unused          
 *           OUT5    52.083333... MHz  LVDSFast
 * VDDO3     OUT6              Unused               3.30       20     66
 *           OUT7              20 MHz  LVDSFast
 *           OUT8              20 MHz  LVDSFast
 * VDDO4     OUT9              Unused          
 * VDDO5     OUT10             Unused          
 *           OUT11             Unused          
 *                                               -------  -------  -----
 *                                                  1.80       28     51
 *                                                  3.30       63    208
 *                                               -------  -------  -----
 *                                                 Total             259
 *                                               -------  -------  -----
 * 
 * Note:
 * 
 * - Tj is junction temperature. Tj must be less than 125 °C (on Si5332-GM3
 *   Revision D) for device to comply with datasheet specifications. Tj = Ta +
 *   Theta_JA*On_Chip_Power.
 * - Overall power includes on-chip power dissipation and adds differential load
 *   power dissipation to estimate total power requirements.
 * - Above are estimates only: power and temperature should be measured on your
 *   PCB.
 * 
 * Settings
 * ========
 * 
 * Location    Setting Name      Decimal Value      Hex Value        
 * ----------  ----------------  -----------------  -----------------
 * 0x17[7:0]   DESIGN_ID0        65                 0x41             
 * 0x18[7:0]   DESIGN_ID1        70                 0x46             
 * 0x19[7:0]   DESIGN_ID2        52                 0x34             
 * 0x1A[7:0]   DESIGN_ID3        88                 0x58             
 * 0x1B[7:0]   DESIGN_ID4        49                 0x31             
 * 0x1C[7:0]   DESIGN_ID5        50                 0x32             
 * 0x24[1:0]   IMUX_SEL          1                  0x1              
 * 0x25[1:0]   OMUX0_SEL0        0                  0x0              
 * 0x25[6:4]   OMUX0_SEL1        0                  0x0              
 * 0x26[1:0]   OMUX1_SEL0        0                  0x0              
 * 0x26[6:4]   OMUX1_SEL1        1                  0x1              
 * 0x27[1:0]   OMUX2_SEL0        0                  0x0              
 * 0x27[6:4]   OMUX2_SEL1        1                  0x1              
 * 0x28[1:0]   OMUX3_SEL0        0                  0x0              
 * 0x28[6:4]   OMUX3_SEL1        0                  0x0              
 * 0x2B[7:0]   HSDIV0A_DIV       25                 0x19             
 * 0x2D[7:0]   HSDIV1A_DIV       16                 0x10             
 * 0x3C[0]     ID0A_SS_ENA       0                  0x0              
 * 0x3C[2:1]   ID0A_SS_MODE      0                  0x0              
 * 0x48[0]     ID0B_SS_ENA       0                  0x0              
 * 0x48[2:1]   ID0B_SS_MODE      0                  0x0              
 * 0x54[0]     ID1A_SS_ENA       0                  0x0              
 * 0x54[2:1]   ID1A_SS_MODE      0                  0x0              
 * 0x60[0]     ID1B_SS_ENA       0                  0x0              
 * 0x60[2:1]   ID1B_SS_MODE      0                  0x0              
 * 0x67[14:0]  IDPA_INTG         12800              0x3200           
 * 0x69[14:0]  IDPA_RES          0                  0x0000           
 * 0x6B[14:0]  IDPA_DEN          1                  0x0001           
 * 0x73[1:0]   CLKIN_2_CLK_SEL   0                  0x0              
 * 0x74[1:0]   CLKIN_3_CLK_SEL   0                  0x0              
 * 0x75[4:0]   P_VAL             1                  0x01             
 * 0x7A[3:0]   OUT0_MODE         6                  0x6              
 * 0x7B[5:0]   OUT0_DIV          1                  0x01             
 * 0x7C[2:0]   OUT0_SKEW         0                  0x0              
 * 0x7D[0]     OUT0_STOP_HIGHZ   0                  0x0              
 * 0x7D[5:4]   OUT0_CMOS_INV     0                  0x0              
 * 0x7D[6]     OUT0_DIFF_INV     0                  0x0              
 * 0x7F[3:0]   OUT1_MODE         7                  0x7              
 * 0x81[2:0]   OUT1_SKEW         0                  0x0              
 * 0x82[0]     OUT1_STOP_HIGHZ   0                  0x0              
 * 0x82[5:4]   OUT1_CMOS_INV     0                  0x0              
 * 0x82[6]     OUT1_DIFF_INV     0                  0x0              
 * 0x84[3:0]   OUT2_MODE         6                  0x6              
 * 0x85[5:0]   OUT2_DIV          1                  0x01             
 * 0x86[2:0]   OUT2_SKEW         0                  0x0              
 * 0x87[0]     OUT2_STOP_HIGHZ   0                  0x0              
 * 0x87[5:4]   OUT2_CMOS_INV     0                  0x0              
 * 0x87[6]     OUT2_DIFF_INV     0                  0x0              
 * 0x89[3:0]   OUT3_MODE         6                  0x6              
 * 0x8A[5:0]   OUT3_DIV          1                  0x01             
 * 0x8B[2:0]   OUT3_SKEW         0                  0x0              
 * 0x8C[0]     OUT3_STOP_HIGHZ   0                  0x0              
 * 0x8C[5:4]   OUT3_CMOS_INV     0                  0x0              
 * 0x8C[6]     OUT3_DIFF_INV     0                  0x0              
 * 0x8E[3:0]   OUT4_MODE         6                  0x6              
 * 0x90[2:0]   OUT4_SKEW         0                  0x0              
 * 0x91[0]     OUT4_STOP_HIGHZ   0                  0x0              
 * 0x91[5:4]   OUT4_CMOS_INV     0                  0x0              
 * 0x91[6]     OUT4_DIFF_INV     0                  0x0              
 * 0x93[3:0]   OUT5_MODE         6                  0x6              
 * 0x94[5:0]   OUT5_DIV          3                  0x03             
 * 0x95[2:0]   OUT5_SKEW         0                  0x0              
 * 0x96[0]     OUT5_STOP_HIGHZ   0                  0x0              
 * 0x96[5:4]   OUT5_CMOS_INV     0                  0x0              
 * 0x96[6]     OUT5_DIFF_INV     0                  0x0              
 * 0x98[3:0]   OUT6_MODE         7                  0x7              
 * 0x9A[2:0]   OUT6_SKEW         0                  0x0              
 * 0x9B[0]     OUT6_STOP_HIGHZ   0                  0x0              
 * 0x9B[5:4]   OUT6_CMOS_INV     0                  0x0              
 * 0x9B[6]     OUT6_DIFF_INV     0                  0x0              
 * 0x9D[3:0]   OUT7_MODE         6                  0x6              
 * 0x9E[5:0]   OUT7_DIV          5                  0x05             
 * 0x9F[2:0]   OUT7_SKEW         0                  0x0              
 * 0xA0[0]     OUT7_STOP_HIGHZ   0                  0x0              
 * 0xA0[5:4]   OUT7_CMOS_INV     0                  0x0              
 * 0xA0[6]     OUT7_DIFF_INV     0                  0x0              
 * 0xA2[3:0]   OUT8_MODE         6                  0x6              
 * 0xA3[5:0]   OUT8_DIV          5                  0x05             
 * 0xA4[2:0]   OUT8_SKEW         0                  0x0              
 * 0xA5[0]     OUT8_STOP_HIGHZ   0                  0x0              
 * 0xA5[5:4]   OUT8_CMOS_INV     0                  0x0              
 * 0xA5[6]     OUT8_DIFF_INV     0                  0x0              
 * 0xA7[3:0]   OUT9_MODE         7                  0x7              
 * 0xA9[2:0]   OUT9_SKEW         0                  0x0              
 * 0xAA[0]     OUT9_STOP_HIGHZ   0                  0x0              
 * 0xAA[5:4]   OUT9_CMOS_INV     0                  0x0              
 * 0xAA[6]     OUT9_DIFF_INV     0                  0x0              
 * 0xAC[3:0]   OUT10_MODE        6                  0x6              
 * 0xAE[2:0]   OUT10_SKEW        0                  0x0              
 * 0xAF[0]     OUT10_STOP_HIGHZ  0                  0x0              
 * 0xAF[5:4]   OUT10_CMOS_INV    0                  0x0              
 * 0xAF[6]     OUT10_DIFF_INV    0                  0x0              
 * 0xB1[3:0]   OUT11_MODE        7                  0x7              
 * 0xB3[2:0]   OUT11_SKEW        0                  0x0              
 * 0xB4[0]     OUT11_STOP_HIGHZ  0                  0x0              
 * 0xB4[5:4]   OUT11_CMOS_INV    0                  0x0              
 * 0xB4[6]     OUT11_DIFF_INV    0                  0x0              
 * 0xB6[0]     OUT0_OE           1                  0x1              
 * 0xB6[1]     OUT1_OE           0                  0x0              
 * 0xB6[2]     OUT2_OE           1                  0x1              
 * 0xB6[3]     OUT3_OE           1                  0x1              
 * 0xB6[4]     OUT4_OE           0                  0x0              
 * 0xB6[5]     OUT5_OE           1                  0x1              
 * 0xB6[6]     OUT6_OE           0                  0x0              
 * 0xB6[7]     OUT7_OE           1                  0x1              
 * 0xB7[0]     OUT8_OE           1                  0x1              
 * 0xB7[1]     OUT9_OE           0                  0x0              
 * 0xB7[2]     OUT10_OE          0                  0x0              
 * 0xB7[3]     OUT11_OE          0                  0x0              
 * 0xB9[0]     XOSC_DIS          0                  0x0              
 * 0xB9[1]     IBUF0_DIS         1                  0x1              
 * 0xB9[2]     IBUF1_DIS         1                  0x1              
 * 0xB9[3]     IMUX_DIS          0                  0x0              
 * 0xB9[4]     PDIV_DIS          0                  0x0              
 * 0xB9[5]     PLL_DIS           0                  0x0              
 * 0xBA[5]     ID0_DIS           1                  0x1              
 * 0xBA[6]     ID1_DIS           1                  0x1              
 * 0xBA[0]     HSDIV0_DIS        0                  0x0              
 * 0xBA[1]     HSDIV1_DIS        0                  0x0              
 * 0xBA[2]     HSDIV2_DIS        1                  0x1              
 * 0xBA[3]     HSDIV3_DIS        1                  0x1              
 * 0xBA[4]     HSDIV4_DIS        1                  0x1              
 * 0xBB[0]     OMUX0_DIS         0                  0x0              
 * 0xBB[1]     OMUX1_DIS         0                  0x0              
 * 0xBB[2]     OMUX2_DIS         0                  0x0              
 * 0xBB[3]     OMUX3_DIS         0                  0x0              
 * 0xBB[4]     OMUX4_DIS         1                  0x1              
 * 0xBB[5]     OMUX5_DIS         1                  0x1              
 * 0xBC[0]     OUT0_DIS          0                  0x0              
 * 0xBC[1]     OUT1_DIS          1                  0x1              
 * 0xBC[2]     OUT2_DIS          0                  0x0              
 * 0xBC[3]     OUT3_DIS          0                  0x0              
 * 0xBC[4]     OUT4_DIS          1                  0x1              
 * 0xBC[5]     OUT5_DIS          0                  0x0              
 * 0xBC[6]     OUT6_DIS          1                  0x1              
 * 0xBC[7]     OUT7_DIS          0                  0x0              
 * 0xBD[0]     OUT8_DIS          0                  0x0              
 * 0xBD[1]     OUT9_DIS          1                  0x1              
 * 0xBD[2]     OUT10_DIS         1                  0x1              
 * 0xBD[3]     OUT11_DIS         1                  0x1              
 * 0xBE[7:0]   PLL_MODE          16                 0x10             
 * 0xBF[0]     XOSC_CINT_ENA     0                  0x0              
 * 0xC0[5:0]   XOSC_CTRIM_XIN    15                 0x0F             
 * 0xC1[5:0]   XOSC_CTRIM_XOUT   15                 0x0F
 * 
 *
 */

#endif
