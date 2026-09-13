/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 iWave Global 
 * IP Support <support.ip@iwave-global.com>
 */

#include <command.h>
#include <asm/gpio.h>
#include <i2c.h>
#include <linux/delay.h>
#include <phy.h>
#include <asm/arch/mailbox_s10.h>
#include <string.h>
#include <asm/io.h>
#include <iG_fru.h>
#include "zl30733-cs1-reg.h"
#include "Si5332-cs2-reg.h"

#define I2C_BUS_0			0x0
#define IOEXP2_I2C_BUS			0x2
#define FMC_PLUS_I2C_BUS		0x3
#define PMIC_I2C_SLAVE_ADDR		0x58
#define ZL30733_CB_CS1_I2C_SLAVE_ADDR	0x70
#define SI5332_CB_CS2_I2C_SLAVE_ADDR	0x6A
#define EEPROM_I2C_SLAVE_ADDR		0x52
#define I2CEXP_I2C_SLAVE_ADDR		0x71
#define IOEXP2_I2C_SLAVE_ADDR		0x21
#define FMC_PLUS_I2C_SLAVE_ADDR		0x50
#define SOM_I2C_BUS                     0x0
#define SOM_EEPROM_ADDR                 0x56
#define BSP_VERSION			"iG-PRHSZ-SC-01-R2.0-REL1.0-SD2.0-Q25.1-L6.12.11"



int configure_pmic_ldo2(void);
int configure_pmic_ldo3(void);
int configure_pmic_ldo4(void);
int configure_pmic_vbuck3(void);
int config_zl30733_cs(void);
int config_si5332_cs(void);
int usb_power_enable(void);
void iG58m_agilex5_board_info(void);
int print_carrier_board_version(void);
int board_soft_reset(void);
int io_expander2_init(void);
int fmc_plus_power_sequence(void);
int print_SOM_version(void);


int board_soft_reset(void)
{
	struct gpio_desc eth_reset;
	int nodeoffset;
	const void *blob = gd->fdt_blob;
	/*IG58M: Reset: Same reset is used for both GMAC2 PHY and I2C-MUX*/
 
	nodeoffset = fdt_path_offset(blob, "/soc/ethernet@10830000/mdio0/ethernet-phy@4");
	if (nodeoffset < 0) {
		printf("GMAC2:\tFailed to find gmac2 node in device tree\n");
		return -1;
	}
 
	gpio_request_by_name_nodev(offset_to_ofnode(nodeoffset), "phy-reset-gpio", 0, &eth_reset, 0);
	if (!dm_gpio_is_valid(&eth_reset)) {
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

int board_late_init(void)
{
	const char *rbffile = "IG58m.core.rbf";   
	ulong rbf_addr = 0x87000000;
	char cmd[128];
	int rbf_load_success = 0;
	u32 reset_val;

	/* PMIC and Clock configs */
	udelay(500000);
	configure_pmic_ldo2();
	configure_pmic_ldo3();
	configure_pmic_ldo4();
	configure_pmic_vbuck3();
	config_zl30733_cs();
	config_si5332_cs();
	udelay(2000000);

	/* IO Expander 2 init */
	if (io_expander2_init()) {
	printf("ERROR: IO Expander 2 init failed. Skipping FMC power sequence.\n");
	} else {
	fmc_plus_power_sequence();

	}

	board_soft_reset();

	/* Read Reset value */
	reset_val = readl(0x10D11000);
	printf("RESET STATUS: 0x%08x\n", reset_val);

	/* Skip if already configured */
	if (reset_val & 0x1) {
	printf("Cold Reset Detected: FPGA already configured - Skipping\n");
	rbf_load_success = 1;
	goto post_rbf_load;
	}

	printf("Proceeding with RBF load: %s\n", rbffile);
	udelay(300000);

	/* Load RBF */
	snprintf(cmd, sizeof(cmd),"fatload mmc 0:1 0x%lx %s", rbf_addr, rbffile);

	if (run_command(cmd, 0) == 0) {
	printf("Loaded %s at 0x%lx\n", rbffile, rbf_addr);
	udelay(300000);

	/* Program FPGA */
	snprintf(cmd, sizeof(cmd),"fpga load 0 0x%lx ${filesize}", rbf_addr);
	if (run_command(cmd, 0) == 0) {
	printf("FPGA programming successful.\n");
	rbf_load_success = 1;
	}

	} else {
	printf("ERROR: %s not found on MMC.\n", rbffile);

	}

	post_rbf_load:

	if (rbf_load_success) {
	run_command("bridge enable 0x3", 0);
	} else {
	printf("FPGA load FAILED. Bridges NOT enabled.\n");

	}

	iG58m_agilex5_board_info();
	usb_power_enable();

	return 0;
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

	if ((voltage < 1000) || (voltage > 3300) || (voltage % 50)) {
		printf("PMIC:\tVoltage Out of range. Supported voltage range 1.0V to 3.3V in steps of 0.05v\n");
		return -1;
	}

	ret = i2c_get_chip_for_busnum(0, PMIC_I2C_SLAVE_ADDR, 1, &dev);

	if (ret) {
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

	if ((voltage < 1000) || (voltage > 3300) || (voltage % 50)) {
		printf("PMIC:\tVoltage Out of range. Supported voltage range 1.0V to 3.3V in steps of 0.05v\n");
		return -1;
	}

	ret = i2c_get_chip_for_busnum(0, PMIC_I2C_SLAVE_ADDR, 1, &dev);

	if (ret) {
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

	if ((voltage < 1000) || (voltage > 3300) || (voltage % 50)) {
		printf("PMIC:\tVoltage Out of range. Supported voltage range 1.0V to 3.3V in steps of 0.05v\n");
		return -1;
	}

	ret = i2c_get_chip_for_busnum(0, PMIC_I2C_SLAVE_ADDR, 1, &dev);

	if (ret) {
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

	if ((voltage < 1200) || (voltage > 1300) || (voltage % 20)) {
		printf("PMIC:\tVoltage Out of range. Supported voltage range 1.2V to 1.3V in steps of 0.02v\n");
		return -1;
	}

	ret = i2c_get_chip_for_busnum(0, PMIC_I2C_SLAVE_ADDR, 1, &dev);

	if (ret) {
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

int config_zl30733_cs(void)
{
	struct udevice *dev;
	int ret;
	int counter = 0;
	u8 data;

	ret = i2c_get_chip_for_busnum(I2C_BUS_0, ZL30733_CB_CS1_I2C_SLAVE_ADDR, 1, &dev);
	if (ret) {
		printf("CLK:\tCarrierboard Clk synthesizer-1 configuration failed\r\n");
		return ret;
	}

	for (counter = 0; counter < ZL30733_REG_CONFIG_NUM_COMMANDS; counter++) {
		if (zl30733_registers[counter].cmd != W) {
			data = (zl30733_registers[counter].address & 0xFF80) >> 7;
			ret = dm_i2c_write(dev, 0x7F, &data, 1);
			if (ret) {
				printf("CLK:\tCarrierboard Clk synthesizer-1 configuration failed\r\n");
				return ret;
			}

			data = zl30733_registers[counter].value;
			ret = dm_i2c_write(dev, zl30733_registers[counter].address & 0x007F, &data, 1);
			if (ret) {
				printf("CLK:\tCarrierboard Clk synthesizer-1 configuration failed\r\n");
				return ret;
			}
		} else {
			mdelay(zl30733_registers[counter].address / 1000);
		}
	}
	printf("CLK:\tCarrierboard Clk synthesizer-1 configured\r\n");

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
	si5332_gm1_revd_register_t si5332_gm1_reg_2;

	ret = i2c_get_chip_for_busnum(0, SI5332_CB_CS2_I2C_SLAVE_ADDR, 1, &dev);
	if (ret) {
		printf("CLK:\tCarrierboard Clk synthesizer-2 configuration failed\r\n");
		return ret;
	}

	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

	mdelay(300);

	for (counter = 0; counter < SI5332_GM1_REVD_REG_CONFIG_NUM_REGS; counter++) {
		si5332_gm1_reg_2 = si5332_gm1_revd_registers[counter];
		page = si5332_gm1_reg_2.address / 256;
		addr = si5332_gm1_reg_2.address % 256;
		page_buf[0] = page & 0xFF;
		value_buf[0] = si5332_gm1_reg_2.value & 0xFF;
		dm_i2c_write(dev, 0x01, (const uint8_t *)&page_buf, 1);
		dm_i2c_write(dev, addr, (const uint8_t *)&value_buf, 1);
	}
	printf("CLK:\tCarrierboard Clk synthesizer-2 configured\r\n");

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
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

int usb_power_enable(void)
{
	struct gpio_desc usb_en;
	int nodeoffset;
	const void *blob = gd->fdt_blob;

	nodeoffset = fdt_path_offset(blob, "/usb_en");
	if (nodeoffset < 0) {
		printf("USB_PWR:\tFailed to find USB enable node in device tree\n");
		return -1;
	}

	gpio_request_by_name_nodev(offset_to_ofnode(nodeoffset), "usb-en", 0, &usb_en, 0);
	if (!dm_gpio_is_valid(&usb_en)) {
		printf("USB_PWR:\tusb-en pin not valid\n");
		return -1;
	}

	dm_gpio_set_dir_flags(&usb_en, GPIOD_IS_OUT);
	dm_gpio_set_value(&usb_en, 1);

	return 0;
}
int print_SOM_version()
{
    char bsp_ver[54], som_serial_ver[20], som_num[20];
    int ret;
    struct udevice *dev;

    ret = i2c_get_chip_for_busnum(SOM_I2C_BUS, SOM_EEPROM_ADDR, 1, &dev);

    if (ret) {
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

    printf("\nBoard Info:\n");
    printf("\tSOM Version\t: %s-%s\n", som_serial_ver, som_num);
    printf("\tBSP Version (Shipped)\t: %s\n", bsp_ver);
    return 0;
}

void iG58m_agilex5_board_info(void)
{
	printf("\nBSP Info:\n");
	printf ("\tBSP Version\t: %s\n", BSP_VERSION);
	print_SOM_version();
	print_carrier_board_version();
}

int print_carrier_board_version(void)
{
	char crb_ver[20], crb_num[20];
	int ret;
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(I2C_BUS_0, EEPROM_I2C_SLAVE_ADDR, 1, &dev);

	if (ret) {
		printf("I2C:\tFailed to find EEPROM !!!\n");
		printf("Unable to read Carrier board information from EEPROM !!!\n\n");
		return -1;
	}

	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

	dm_i2c_read(dev, 0x20, crb_ver, 16);
	dm_i2c_read(dev, 0x30, crb_num, 12);

	crb_ver[16] = '\0';
	crb_num[12] = '\0';

	printf("\tCarrier Serial Number\t: %s-%s\n", crb_ver, crb_num);

	return 0;
}

int io_expander2_init(void)
{
	int ret;
	struct udevice *bus, *dev;
	ret = uclass_get_device(UCLASS_I2C, I2C_BUS_0, &bus);
	if (ret)
		return -1;

	ret = dm_i2c_probe(bus, I2CEXP_I2C_SLAVE_ADDR, 0x0, &dev);
	if (ret)
		return -1;

	/* iG58M: I2C: No register address will be sent. So, set to 0 */
	i2c_set_chip_offset_len(dev, 0);

	ret = uclass_get_device(UCLASS_I2C, IOEXP2_I2C_BUS, &bus);
	if (ret)
		return -1;

	ret = dm_i2c_probe(bus, IOEXP2_I2C_SLAVE_ADDR, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS, &dev);
	if (ret)
		return -1;

	/* iG58M: I2C: 8bit register address will be sent. So, set to 1 */
	i2c_set_chip_offset_len(dev, 1);

	return 0;
}

int fmc_plus_power_sequence(void)
{
	int ret;
	struct gpio_desc prsnt_m2c, vcc_adj, vcc_3v3, vcc_12v, pg_c2m;
	int nodeoffset;
	const void *blob = gd->fdt_blob;
	u32 vadj_volt;

	nodeoffset = fdt_path_offset(blob, "/fmc_plus");

	ofnode_read_u32(offset_to_ofnode(nodeoffset), "vadj-millivolt", &vadj_volt);

	gpio_request_by_name_nodev(offset_to_ofnode(nodeoffset), "prsnt-m2c", 0, &prsnt_m2c, 0);
	if (!dm_gpio_is_valid(&prsnt_m2c)) {
		printf("FMC+:\tPRSNT_M2C Pin not valid\n");
		goto end;
	}

	gpio_request_by_name_nodev(offset_to_ofnode(nodeoffset), "vcc-adj", 0, &vcc_adj, 0);
	if (!dm_gpio_is_valid(&vcc_adj)) {
		printf("FMC+:\tVCC_ADJ Pin not valid\n");
		goto end;
	}

	gpio_request_by_name_nodev(offset_to_ofnode(nodeoffset), "vcc-3v3", 0, &vcc_3v3, 0);
	if (!dm_gpio_is_valid(&vcc_3v3)) {
		printf("FMC+:\tVCC_3V3 Pin not valid\n");
		goto end;
	}

	gpio_request_by_name_nodev(offset_to_ofnode(nodeoffset), "vcc-12v", 0, &vcc_12v, 0);
	if (!dm_gpio_is_valid(&vcc_12v)) {
		printf("FMC+:\tVCC_12V Pin not valid\n");
		goto end;
	}

	gpio_request_by_name_nodev(offset_to_ofnode(nodeoffset), "pg-c2m", 0, &pg_c2m, 0);
	if (!dm_gpio_is_valid(&pg_c2m)) {
		printf("FMC+:\tPG_C2M Pin not valid\n");
		goto end;
	}

	dm_gpio_set_dir_flags(&prsnt_m2c, GPIOD_IS_IN);
	dm_gpio_set_dir_flags(&vcc_12v, GPIOD_IS_OUT);
	dm_gpio_set_dir_flags(&vcc_3v3, GPIOD_IS_OUT);
	dm_gpio_set_dir_flags(&pg_c2m, GPIOD_IS_OUT);
	dm_gpio_set_dir_flags(&vcc_adj, GPIOD_IS_OUT);

	if (dm_gpio_get_value(&prsnt_m2c)) {
		printf("FMC+:\tFMC+ Not connected\n");
		goto end;
	}

	ret = fmc_vadj_support(FMC_PLUS_I2C_BUS, FMC_PLUS_I2C_SLAVE_ADDR, vadj_volt);
	if (ret) {
		dm_gpio_set_value(&vcc_12v, 1);
		dm_gpio_set_value(&vcc_3v3, 1);
		dm_gpio_set_value(&pg_c2m, 1);
		dm_gpio_set_value(&vcc_adj, 1);
		printf("FMC+:\tFMC+ Vadj Voltage set to %d.%dV\n", vadj_volt / 1000, (vadj_volt % 1000) / 100);
	} else {
		printf("FMC+:\tFMC+ connected does not support %d.%dV\n", vadj_volt / 1000, (vadj_volt % 1000) / 100);
		goto end;
	}

	if (!(dm_gpio_get_value(&vcc_adj) && dm_gpio_get_value(&vcc_3v3) && dm_gpio_get_value(&vcc_12v) && dm_gpio_get_value(&pg_c2m))) {
		printf("FMC+:\tError on FMC+ power up ..!!\n");
		goto end;
	}

	printf("FMC+:\tFMC+ Powered up\n");
	return 0;

end:
	printf("FMC+:\tFMC+ Power up failed ...!!!\n");
	return -1;
}
