/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 iWave Global 
 * IP Support <support.ip@iwave-global.com>
 */
// use this for searching the Yocto error log: grep -B 3 -A 5 -i "error:" <log_file>

#include <command.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <linux/delay.h>
#include <phy.h>
#include <asm/arch/mailbox_s10.h>
#include <string.h>
#include <asm/io.h>
#include "afe5828.h"
#include "tpm_test.h"
#include <image.h>
#include <env.h>
#include <dm.h>
#include <fpga.h>
#include <net.h>
#include <linux/math64.h> /* Optional helper for log calculations if needed */

#ifdef USE_GM1
//#include "Si5332-cs2-reg.h"
#else
//#include "Si5332-GM1-RevD-AF4-Registers_52MHz.h"
//#include "Si5332-GM3-RevD-AF4X12-Registers_52.083MHz.h"
#include "Si5332-GM3-RevD-AF4X12-Registers_2x20_2x156.h"
#endif

#define I2C_BUS_0			0x0
#define IOEXP2_I2C_BUS			0x2
#define PMIC_I2C_SLAVE_ADDR		0x58
#define SI5332_CB_CS2_I2C_SLAVE_ADDR	0x6A
#define EEPROM_I2C_SLAVE_ADDR		0x52
#define I2CEXP_I2C_SLAVE_ADDR		0x71
#define SFP_DIAG_ADDR            0x51  /* Real-Time Diagnostics Address */
//#define IOEXP2_I2C_SLAVE_ADDR		0x21
#define SFP_EEPROM_ADDR                0x50 /* Standard I2C-address for SFP/SFP+ EEPROM (A0h) */
#define SOM_I2C_BUS                    0x0
#define SOM_EEPROM_ADDR                0x56
#define BSP_VERSION			"meta-iwave-bnj"

/* Offsets for Static Identification (Address 0x50) */
#define SFP_OFF_BITRATE          12    /* 1 byte: Nominal Bit Rate in 100MBits/sec */
#define SFP_OFF_VENDOR_NAME      20    /* 16 bytes: Vendor Name string */
#define SFP_OFF_PART_NUMBER      40    /* 16 bytes: Vendor Part Number string */
#define SFP_OFF_SERIAL_NUMBER    68    /* 16 bytes: Vendor Serial Number string */
#define SFP_OFF_DIAG_TYPE        92    /* 1 byte: Tells us if DOM diagnostics are supported */
/* Offsets for Diagnostic Data (Address 0x51) */
#define SFP_REG_TEMP             96    /* 2 bytes: Internally measured module temperature */
#define SFP_REG_VCC              98    /* 2 bytes: Internally measured supply voltage */
#define SFP_REG_TX_BIAS          100   /* 2 bytes: Internally measured laser bias current */
#define SFP_REG_TX_POWER         102   /* 2 bytes: Internally measured TX output power */
#define SFP_REG_RX_POWER         104   /* 2 bytes: Internally measured RX input power */

#define VT_GREEN "\x1b[1;32m"
#define VT_RED "\x1b[1;31m"
#define VT_RESET "\x1b[0m"
#define VT_REVERSE "\x1b[7m"
#define VT_BRIGHT "\x1b[1m"
#define VT_RESET_STR "\x1b[0m"
#define VT_DEF_COLOR "\x1b[0m"
#define VT_DEF "\x1b[0m"
#define VT_BLUE "\x1b[1;34m"
#define VT_YELLOW "\x1b[1;33m"
#define VT_WHITE "\x1b[37m"
#define VT_CYAN "\x1b[36m"
#define VT_MAGENTA "\x1b[35m"

#define VT_HOME "\x1b[0;0H"
#define VT_CURSOR_OFF "\x1b[?25l"
#define VT_CURSOR_ON "\x1b[?25h"
#define VT_CLRSCR "\x1b[2J"
#define VT_ERASE_DOWN "\x1b[0J"
#define VT_CEOS "\x1b[0J"
#define VT_CEOL "\x1b[K"
#define VT_CLREOL "\x1b[K"


int configure_pmic_ldo2(void);
int configure_pmic_ldo3(void);
int configure_pmic_ldo4(void);
int configure_pmic_vbuck3(void);
int config_si5332_cs(void);
void af4_agilex5_board_info(void);
int board_soft_reset(void);
int print_SOM_version(void);
int SFP_detect(void);


 #define nullptr 0x00

int board_read_config(void)
     {
	struct gpio_desc rbf_load_tftp;
	struct gpio_desc rbf_load_skip;
	int nodeoffset;
	const void *blob = gd->fdt_blob;

	nodeoffset = fdt_path_offset(blob, "/board_config_load_tftp");
	printf("BOARD_CONFIG\tnode offset:%d\n", nodeoffset);
	if (nodeoffset < 0)
             {
	      printf("BOARD_CONFIG\tFailed to find board_config node in device tree\n");
	      return -1;
	      }

	gpio_request_by_name_nodev(offset_to_ofnode(nodeoffset), "gpios", 0, &rbf_load_tftp, GPIOD_IS_IN);
	if (!dm_gpio_is_valid(&rbf_load_tftp))
             {
	      printf("\trbf-load-tfpt pin not valid\n");
	      return -1;
	      }
	int tftp_state = dm_gpio_get_value(&rbf_load_tftp);
       dm_gpio_free(NULL, &rbf_load_tftp);

	return (tftp_state & 1) ;
     }

int board_soft_reset(void)
     {
	struct gpio_desc eth_reset;
	int nodeoffset;
	const void *blob = gd->fdt_blob;
	/*IG58M: Reset: Same reset is used for both GMAC2 PHY and I2C-MUX*/
 
	nodeoffset = fdt_path_offset(blob, "/soc/ethernet@10830000/mdio0/ethernet-phy@3");
	if (nodeoffset < 0)
        {
	   printf("GMAC2:\tFailed to find gmac2 node in device tree\n");
	   return -1;
	    }
 
	gpio_request_by_name_nodev(offset_to_ofnode(nodeoffset), "reset-gpios", 0, &eth_reset, 0);
	if (!dm_gpio_is_valid(&eth_reset))
        {
	    printf("GMAC2:\tphy-reset-gpio pin not valid\n");
	    return -1;
	    }
 
	dm_gpio_set_dir_flags(&eth_reset, GPIOD_IS_OUT);
	/*IG58M: Assert PHY reset (active-low means set=1 drives low) */
	dm_gpio_set_value(&eth_reset, 1);
	udelay(1000);
 
	/*IG58M: Release PHY reset (set=0 drives high) */
	dm_gpio_set_value(&eth_reset, 0);
	udelay(10000);
 
	return 0;
   }

 void env_print(const char *env_str)
    {
    char *env_value_str = env_get(env_str);
    
    if (env_value_str)
        {
        printf("env:[%-12s] [%s]\n", env_str, env_value_str);
        }
     else
         {
        printf("env:[%-12s] [%s]\n", env_str, "--");
        }
   }

void print_dbm_power(const char *label, uint16_t raw_val)
{
    /* Convert 0.1 uW units to absolute microwatts */
    uint32_t uw = raw_val / 10; 
    
    if (uw == 0) {
        printf("    %s: 0.0000 mW (No Light / Link Down)\n", label);
        return;
    }

    /* Extract whole milliwatts and fractional micro-units */
    uint32_t mw_whole = uw / 1000;
    uint32_t mw_frac  = uw % 1000;

    /* Pads the fraction to match a %04d precision layout */
    printf("    %s: %u.%04u mW\n", label, mw_whole, mw_frac * 10);
}

int SFP_detect(void)
{
    int ret;
    struct udevice *bus;
    struct udevice *mux_dev;
    struct udevice *sfp_id_dev;
    struct udevice *sfp_diag_dev;
    int SFP_count = 0;
    
    int sfp_channels[] = {2, 3};

    char vendor_name[17];
    char part_number[17];
    char serial_number[17];
    uint8_t raw_bitrate;
    uint8_t diag_type;

    ret = uclass_get_device_by_seq(UCLASS_I2C, I2C_BUS_0, &bus);
    if (ret)
    {
        printf(VT_RED "ERROR: I2C bus %d not found." VT_RESET "\n", I2C_BUS_0);
        return -1;
    }

    ret = dm_i2c_probe(bus, I2CEXP_I2C_SLAVE_ADDR, 0x0, &mux_dev);
    if (ret)
    {
        printf(VT_RED "ERROR: I2C mux at %02xH not found." VT_RESET "\n", I2CEXP_I2C_SLAVE_ADDR);
        return -1;
    }

    for (int i = 0; i < 2; i++)
    {
        int channel = sfp_channels[i];
        uint8_t mux_mask = (1 << channel);
        
        ret = dm_i2c_write(mux_dev, 0, &mux_mask, 1);
        if (ret)
        {
            printf(VT_RED "ERROR: Mux channel %d switch failed." VT_RESET "\n", channel);
            goto reset_mux;
        }

        ret = dm_i2c_probe(bus, SFP_EEPROM_ADDR, 0x0, &sfp_id_dev);
        if (ret == 0)
        {
            SFP_count++;
            
            memset(vendor_name, 0, sizeof(vendor_name));
            memset(part_number, 0, sizeof(part_number));
            memset(serial_number, 0, sizeof(serial_number));
            
            dm_i2c_read(sfp_id_dev, SFP_OFF_VENDOR_NAME, (uint8_t *)vendor_name, 16);
            dm_i2c_read(sfp_id_dev, SFP_OFF_PART_NUMBER, (uint8_t *)part_number, 16);
            dm_i2c_read(sfp_id_dev, SFP_OFF_SERIAL_NUMBER, (uint8_t *)serial_number, 16);
            dm_i2c_read(sfp_id_dev, SFP_OFF_BITRATE, &raw_bitrate, 1);
            dm_i2c_read(sfp_id_dev, SFP_OFF_DIAG_TYPE, &diag_type, 1);

            printf(VT_GREEN "\n--- SFP Module Detected in Cage %d (Mux Ch %d) ---" VT_RESET "\n", i, channel);
            printf("  Vendor Name : %s\n", vendor_name);
            printf("  Part Number : %s\n", part_number);
            printf("  Serial Num  : %s\n", serial_number);
            printf("  Bit Rate    : %d Gbps\n", raw_bitrate / 10);

            if (diag_type & 0x40)
            {
                ret = dm_i2c_probe(bus, SFP_DIAG_ADDR, 0x0, &sfp_diag_dev);
                if (ret == 0)
                {
                    uint8_t raw_diag[10];
                    ret = dm_i2c_read(sfp_diag_dev, SFP_REG_TEMP, raw_diag, 10);
                    
                    if (ret == 0)
                    {
                        int16_t temp_raw  = (raw_diag[0] << 8) | raw_diag[1];
                        uint16_t vcc_raw   = (raw_diag[2] << 8) | raw_diag[3];
                        uint16_t bias_raw  = (raw_diag[4] << 8) | raw_diag[5];
                        uint16_t tx_pw_raw = (raw_diag[6] << 8) | raw_diag[7];
                        uint16_t rx_pw_raw = (raw_diag[8] << 8) | raw_diag[9];

                        /* --- 1. Temperature Fixed Math (1/256 C LSB) --- */
                        /* Scaled up by 100 to extract 2 decimal positions safely */
                        int32_t temp_scaled = ((int32_t)temp_raw * 100) / 256;
                        int32_t temp_whole  = temp_scaled / 100;
                        int32_t temp_frac   = temp_scaled % 100;
                        if (temp_frac < 0) temp_frac = -temp_frac; /* Absolute value for fractional print */

                        /* --- 2. VCC Voltage Fixed Math (100 uV LSB) --- */
                        /* vcc_raw * 100 uV -> total microvolts. Divide by 10000 for 100uV fraction steps */
                        uint32_t vcc_whole = vcc_raw / 10000;
                        uint32_t vcc_frac  = vcc_raw % 10000;

                        /* --- 3. Laser Bias Current Fixed Math (2 uA LSB) --- */
                        /* bias_raw * 2 -> total microamps. Divide by 1000 to get whole mA */
                        uint32_t bias_total_ua = (uint32_t)bias_raw * 2;
                        uint32_t bias_whole    = bias_total_ua / 1000;
                        uint32_t bias_frac     = bias_total_ua % 1000;

                        printf(VT_BLUE "  Diagnostic Performance Parameters:" VT_RESET "\n");
                        printf("    Temperature : %d.%02d °C\n", temp_whole, temp_frac);
                        printf("    VCC Voltage : %u.%04u V\n", vcc_whole, vcc_frac);
                        printf("    Laser Bias  : %u.%02u mA\n", bias_whole, bias_frac / 10);
                        print_dbm_power("TX Power   ", tx_pw_raw);
                        print_dbm_power("RX Power   ", rx_pw_raw);
                    }
                    else
                    {
                        printf(VT_RED "    Failed to parse diagnostics table registers." VT_RESET "\n");
                    }
                }
                else
                {
                    printf("    DOM address 0x51 did not answer the link query.\n");
                }
            }
            else
            {
                printf("  Diagnostics : DOM Not Supported by this module firmware.\n");
            }
        }
        else
        {
            printf("SFP Cage %d (Mux Ch %d): Empty\n", i, channel);
        }
    }

reset_mux:
    uint8_t clear_mask = 0x00;
    dm_i2c_write(mux_dev, 0, &clear_mask, 1);

    if (ret && SFP_count == 0) 
    {
        return -1;
    }

    return SFP_count;
}


/* PMIC LDIO2 configuration */
int configure_pmic_ldo2(void)
    {
	u8 data;
	u32 voltage;
	int volt, voltage1, ret, nodeoffset;
	struct udevice *dev;
	const void *blob = gd->fdt_blob;

	nodeoffset = fdt_path_offset(blob, "/pmic");
	ofnode_read_u32(offset_to_ofnode(nodeoffset), "pmic-ldo2-millivolt", &voltage);

	if ((voltage < 1000) || (voltage > 3300) || (voltage % 50))
           {
	    printf("PMIC:\tVoltage Out of range. Supported voltage range 1.0V to 3.3V in steps of 0.05v\n");
	    return -1;
	    }

	ret = i2c_get_chip_for_busnum(0, PMIC_I2C_SLAVE_ADDR, 1, &dev);

	if (ret)
           {
	    printf("PMIC:\tFailed to find PMIC !!!\n");
	    return -1;
	    }

	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

	voltage1 = ((voltage - 900) / 50);
	volt = (int)voltage1;
	volt += 2;
	data = volt;

	dm_i2c_write(dev, 0xAA, &data, 1);
	data = 1;
	dm_i2c_write(dev, 0x27, &data, 1);

	printf("PMIC:\tLD02 (HVIO BANK 5A) set to %d.%2dV\n", voltage / 1000, voltage % 1000);

	return 0;
    }

/* PMIC LDIO3 configuration */
int configure_pmic_ldo3(void)
    {
	u8 data;
	u32 voltage;
	int volt, voltage1, ret, nodeoffset;
	struct udevice *dev;
	const void *blob = gd->fdt_blob;

	nodeoffset = fdt_path_offset(blob, "/pmic");
	ofnode_read_u32(offset_to_ofnode(nodeoffset), "pmic-ldo3-millivolt", &voltage);

	if ((voltage < 1000) || (voltage > 3300) || (voltage % 50))
           {
	    printf("PMIC:\tVoltage Out of range. Supported voltage range 1.0V to 3.3V in steps of 0.05v\n");
	    return -1;
	    }

	ret = i2c_get_chip_for_busnum(0, PMIC_I2C_SLAVE_ADDR, 1, &dev);

	if (ret)
           {
	    printf("PMIC:\tFailed to find PMIC !!!\n");
	    return -1;
	    }

	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

	voltage1 = ((voltage - 900) / 50);
	volt = (int)voltage1;
	volt += 2;
	data = volt;

	dm_i2c_write(dev, 0xAB, &data, 1);
	data = 1;
	dm_i2c_write(dev, 0x28, &data, 1);

	printf("PMIC:\tLD03 (HVIO BANK 6C) set to %d.%2dV\n", voltage / 1000, voltage % 1000);

	return 0;
    }

/* PMIC LDIO4 configuration */
int configure_pmic_ldo4(void)
    {
	u8 data;
	u32 voltage;
	int volt, voltage1, ret, nodeoffset;
	struct udevice *dev;
	const void *blob = gd->fdt_blob;

	nodeoffset = fdt_path_offset(blob, "/pmic");
	ofnode_read_u32(offset_to_ofnode(nodeoffset), "pmic-ldo4-millivolt", &voltage);

	if ((voltage < 1000) || (voltage > 3300) || (voltage % 50))
           {
	    printf("PMIC:\tVoltage Out of range. Supported voltage range 1.0V to 3.3V in steps of 0.05v\n");
	    return -1;
	    }

	ret = i2c_get_chip_for_busnum(0, PMIC_I2C_SLAVE_ADDR, 1, &dev);

	if (ret)
           {
	    printf("PMIC:\tFailed to find PMIC !!!\n");
	    return -1;
	    }

	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

	voltage1 = ((voltage - 900) / 50);
	volt = (int)voltage1;
	volt += 2;
	data = volt;
	dm_i2c_write(dev, 0xAC, &data, 1);
	data = 1;
	dm_i2c_write(dev, 0x29, &data, 1);
	printf("PMIC:\tLD04 (HVIO BANK 5B and 6D) set to %d.%2dV\n", voltage / 1000, voltage % 1000);

	return 0;
    }

/* PMIC VBUCK3 configuration */
int configure_pmic_vbuck3(void)
    {
    u8 data;
    u32 voltage;
    int volt, voltage1, ret, nodeoffset;
    struct udevice *dev;
    const void *blob = gd->fdt_blob;

    nodeoffset = fdt_path_offset(blob, "/pmic");
    ofnode_read_u32(offset_to_ofnode(nodeoffset), "pmic-vbuck3-millivolt", &voltage);

    if ((voltage < 1200) || (voltage > 1300) || (voltage % 20))
       {
       printf("PMIC:\tVoltage Out of range. Supported voltage range 1.2V to 1.3V in steps of 0.02v\n");
       return -1;
       }

    ret = i2c_get_chip_for_busnum(0, PMIC_I2C_SLAVE_ADDR, 1, &dev);

    if (ret)
      {
      printf("PMIC:\tFailed to find PMIC !!!\n");
      return -1;
      }

    i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

    voltage1 = ((voltage - 800) / 20);
    volt = (int)voltage1;
    data = volt;
    dm_i2c_write(dev, 0xA7, &data, 1);
    data = 1;
    dm_i2c_write(dev, 0x24, &data, 1);

    printf("PMIC:\tBUCK3 (HSIO BANK 2A) set to %d.%2dV\n", voltage / 1000, voltage % 1000);
    return 0;
    }

int config_si5332_cs(void)
    {
    struct udevice *dev;
    int ret;
    int counter = 0;
    unsigned char value_buf[1];
    unsigned char page_buf[1];
    unsigned int page;
    unsigned int addr;
#ifdef USE_GM1
    si5332_gm1_revd_register_t si5332_gm1_reg_2;
#else
    si5332_gm3_revd_register_t si5332_gm3_reg_2;
#endif

    printf("*************************************************************************\r\n");
#ifdef USE_GM1
    printf("CLK:\tConfiguring carrierboard Clk GM1 , %u registers\r\n", SI5332_GM1_REVD_REG_CONFIG_NUM_REGS    );
#else
    printf("CLK:\tConfiguring carrierboard Clk GM3 , %u registers\r\n", SI5332_GM3_REVD_REG_CONFIG_NUM_REGS    );
#endif

    ret = i2c_get_chip_for_busnum(0, SI5332_CB_CS2_I2C_SLAVE_ADDR, 1, &dev);
    if (ret)
      {
      printf("CLK:\tCarrierboard Clk configuration failed\r\n");
      return ret;
      }

    i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);
    mdelay(300);
#ifdef USE_GM1
    for (counter = 0; counter < SI5332_GM1_REVD_REG_CONFIG_NUM_REGS; counter++)
       {
       si5332_gm1_reg_2 = si5332_gm1_revd_registers[counter];
       page = si5332_gm1_reg_2.address / 256;
       addr = si5332_gm1_reg_2.address % 256;
       page_buf[0] = page & 0xFF;
       value_buf[0] = si5332_gm1_reg_2.value & 0xFF;
       dm_i2c_write(dev, 0x01, (const uint8_t *)&page_buf, 1);
       dm_i2c_write(dev, addr, (const uint8_t *)&value_buf, 1);
       }
#else
    for (counter = 0; counter < SI5332_GM3_REVD_REG_CONFIG_NUM_REGS; counter++)
       {
       si5332_gm3_reg_2 = si5332_gm3_revd_registers[counter];
       page = si5332_gm3_reg_2.address / 256;
       addr = si5332_gm3_reg_2.address % 256;
       page_buf[0] = page & 0xFF;
       value_buf[0] = si5332_gm3_reg_2.value & 0xFF;
       dm_i2c_write(dev, 0x01, (const uint8_t *)&page_buf, 1);
       dm_i2c_write(dev, addr, (const uint8_t *)&value_buf, 1);
       }
#endif
    printf(VT_GREEN "CLK:\tCarrierboard Clk configured successfully" VT_RESET "\r\n");
    printf("*************************************************************************\r\n");
    printf(VT_YELLOW );
    printf("Design info:\r\n");
    printf("------------------\r\n");
    printf("%s", Si5332_design_info);
    printf("------------------\r\n");
    printf(VT_RESET );

#ifdef USE_GM1
     printf(" si5332_gm1 register dump:\r\n");
     for (counter = 0; counter < SI5332_GM1_REVD_REG_CONFIG_NUM_REGS; counter++)
        {
        si5332_gm1_reg_2 = si5332_gm1_revd_registers[counter];
        printf("\t%03d Addr:%02x Val:%02x\r\n", counter, si5332_gm1_reg_2.address, si5332_gm1_reg_2.value);
        }
#endif
    return 0;
    }

int board_phy_config(struct phy_device *phydev)
    {
    printf("PHY:\tConfig\n");
    if (phydev->drv->config)
       phydev->drv->config(phydev);

       /* Enable RGMII RXC skew and PHY mode select to RGMII copper */
       phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
       phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

       /* set IO voltage to 1.8V */
       phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
       phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
       phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
       phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);
       return 0;
       }

int print_SOM_version()
    {
    char bsp_ver[54], som_serial_ver[20], som_num[20];
    int ret;
    struct udevice *dev;

    ret = i2c_get_chip_for_busnum(SOM_I2C_BUS, SOM_EEPROM_ADDR, 1, &dev);
    printf("Reading from I2C EEPROM at address:%02xH. St:%d\n", SOM_EEPROM_ADDR, ret);

    if (ret)
        {
        printf("I2C:\tFailed to find EEPROM !!!\n");
        printf("Unable to read SOM information from EEPROM !!!\n\n");
        return -1;
        }

    i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

    dm_i2c_read(dev, 0x40, bsp_ver, 54);
    dm_i2c_read(dev, 0x20, som_serial_ver, 16);
    dm_i2c_read(dev, 0x30, som_num, 12);

    bsp_ver[54] = '\0';
    som_serial_ver[16] = '\0';
    som_num[12] = '\0';
    printf("SOM Version\t: %s-%s\n", som_serial_ver, som_num);
    printf("BSP Version\t: %s\n", BSP_VERSION);
    return 0;
    }

void af4_agilex5_board_info(void)
    {
    printf(VT_YELLOW);
    printf("***********************************\n");
    printf("  AF4 U-BOOT V52\n");
    printf("  Date: 2026-07-22 22:36\n");
    printf("  using meta-iwave-bnj\n");
    printf("***********************************\n");
    printf("Board Info:\n");
    print_SOM_version();
    printf(VT_RESET "\n");
    }

int board_late_init(void)
    {
    printf("Board late init.\n");
    int rbf_load_tftp = 0;
    int rbf_load_skip = 0;
    int bc = board_read_config();
    if(bc >= 0)
        {
	 printf("Board config switches:%04xH\n", bc);
        rbf_load_tftp = (bc & 1);  
 //         rbf_load_skip = (bc & 2);  
        }
     else
        {
        printf("Board config error. %d\n", bc);
        }
        
    env_set("tftprbf","bridge disable; tftpboot ${rbf_addr} ${rbffile}; fatwrite mmc 0:1 ${rbf_addr} ${rbffile} ${filesize} ; fpga load 0 ${rbf_addr} ${filesize} ; bridge enable");
    printf("tftprbf command set. Execute: 'run tftprbf' to load rbf file from tftp.\n");

    if(rbf_load_tftp )
        {
        printf("-----------------------------\n");
        printf(VT_CYAN "**** TFTP Load active. ****" VT_RESET "\n");
        printf("Will attempt to load .rbf from tftp and then boot from tftp\n");
        printf("-----------------------------\n");
        env_set("ipaddr", "192.168.7.17");
        env_set("ethaddr", "fa:07:9b:13:58:f8"); 
        env_set("nfsroot", "/srv/tftp/rootfs");
        env_set("bootdelay", "99");
        env_set("serverip", "192.168.7.1");
        env_set("bootcmd", "echo 'TFTP MODE';run tftprbf; run tftpload;run tftpboot");
        }
    printf("Displaying relevant environment\n");
    printf("-----------------------------\n");
    env_print("ethaddr");
    env_print("ipaddr");
    env_print("serverip");
    env_print("nfsroot");
    env_print("rbffile");
    env_print("bootdelay");
    env_print("bootcmd");
    env_print("tftpload");
    env_print("tftpboot");
    printf("-----------------------------\n");

    int rbf_load_success = 0;
    u32 reset_val;

    /* PMIC and Clock configs */
    udelay(500000);
    configure_pmic_ldo2();
    configure_pmic_ldo3();
    configure_pmic_ldo4();
    configure_pmic_vbuck3();
    config_si5332_cs();
    udelay(2000000);

    int SFP_detect_ret = SFP_detect();

    if (SFP_detect_ret < 0)
       {
  	   printf(VT_RED "ERROR: SFP detect failed." VT_RESET "\n");
	   }
    else
       {
       printf(VT_GREEN "SFP detect successful." VT_RESET "\n");
       }

    board_soft_reset();

     /* Read Reset value */
    reset_val = readl(0x10D11000);
    printf("RESET STATUS: 0x%08x\n", reset_val);

    if (reset_val & 0x1)
        {
        printf("Cold reset detected: FPGA already configured. Reloading anyway :-)\n");
        }

    if(rbf_load_skip )
        {
        printf("SKIPPING RBF LOAD\n");
        }
    else
        {
        const char *rbffile = "af4_fpga.core.rbf";
        ulong rbf_addr = 0x87000000;
        char cmd[128];
        printf("Proceeding with RBF load: %s\n", rbffile);
        udelay(300000);

        printf(VT_YELLOW "FPGA programming from EMMC." VT_RESET "\n");
        snprintf(cmd, sizeof(cmd),"fatload mmc 0:1 0x%lx %s", rbf_addr, rbffile);
        printf("***************************\n");
        printf("Using rbf load command: %s\n", cmd);
        printf("***************************\n");
        if (run_command(cmd, 0) == 0)
            {
	        printf("Loaded %s at 0x%lx\n", rbffile, rbf_addr);
	        udelay(300000);
          /* Program FPGA */
            snprintf(cmd, sizeof(cmd),"fpga load 0 0x%lx ${filesize}", rbf_addr);
            if (run_command(cmd, 0) == 0)
                {
                printf(VT_GREEN "FPGA programming successful." VT_RESET "\n");
                rbf_load_success = 1;
                }
	        }
        else
           {
           printf(VT_RED "FPGA programming error. RBF file %s not found." VT_RESET "\n", rbffile);
           }

        if (rbf_load_success)
            {
            int cmd_st = run_command("bridge enable 0x3", 0);
            if (cmd_st == 0)
                {
                printf(VT_GREEN "FPGA bridge enable was successful." VT_RESET "\n");
                uint32_t fpga_version = readl(0x20000000);
                 printf("FPGA rbf version: Type:%d Ver:%d.%d.%d\n",
                        (fpga_version >> 24) & 0xFF,
                        (fpga_version >> 16) & 0xFF,
                        (fpga_version >> 8)  & 0xFF,
                        fpga_version        & 0xFF);

                /* AFEs are only reachable once the FPGA is configured and the
                * HPS<->FPGA bridges are enabled. Warn and continue on failure. */
                if(afe5828_probe_all() == 2)
                    {
                    printf(VT_GREEN "AFEs detected successfully." VT_RESET "\n");
                    }

                /*
                 * TPM detect intentionally NOT called here — bus 2 currently
                 * hangs the CPU and prevents reaching the U-Boot prompt,
                 * which blocks reloading the .rbf. Run `tpm_detect` from the
                 * prompt once the FPGA image is confirmed to route SPI to
                 * spi@10da5000.
                 */
                }
            }
        else
            {
   	       printf(VT_RED "FPGA load FAILED. Bridges NOT enabled." VT_RESET "\n");
  	       }
       }

  af4_agilex5_board_info();
  return 0;
  }

