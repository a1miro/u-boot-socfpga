// SPDX-License-Identifier: GPL-2.0
/*
 * U-Boot 'afe' command — interactive access to the AFE5828 driver.
 *
 * Subcommands:
 *   afe detect                  probe CS0 + CS1, print result
 *   afe reset  <cs>             software reset one AFE
 *   afe read   <cs> <reg>       read a register (hex)
 *   afe write  <cs> <reg> <val> write a 16-bit value
 *
 * <cs>  = 0 or 1
 * <reg> = 8-bit register address, hex or dec
 * <val> = 16-bit value, hex or dec
 */

#include <command.h>
#include <errno.h>
#include <log.h>
#include <linux/kernel.h>
#include <vsprintf.h>

#include "afe5828.h"

static int do_afe_detect(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	afe5828_probe_all();
	return CMD_RET_SUCCESS;
}

static int parse_cs(const char *s, unsigned int *cs)
{
	unsigned long v = simple_strtoul(s, NULL, 0);

	if (v != AFE_CS_A && v != AFE_CS_B) {
		printf("afe: cs must be 0 or 1 (got %s)\n", s);
		return -EINVAL;
	}
	*cs = (unsigned int)v;
	return 0;
}

static int do_afe_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int cs;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;
	if (parse_cs(argv[1], &cs))
		return CMD_RET_FAILURE;

	ret = afe5828_reset(AFE_SPI_BUS, cs);
	if (ret) {
		printf("afe: reset cs=%u failed: %d\n", cs, ret);
		return CMD_RET_FAILURE;
	}
	printf("afe: reset cs=%u OK\n", cs);
	return CMD_RET_SUCCESS;
}

static int do_afe_read(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int cs;
	u8 reg;
	u16 val = 0;
	int ret;

	if (argc < 3)
		return CMD_RET_USAGE;
	if (parse_cs(argv[1], &cs))
		return CMD_RET_FAILURE;

	reg = (u8)simple_strtoul(argv[2], NULL, 0);

	ret = afe5828_read_reg(AFE_SPI_BUS, cs, reg, &val);
	if (ret) {
		printf("afe: read cs=%u reg=0x%02x failed: %d\n", cs, reg, ret);
		return CMD_RET_FAILURE;
	}
	printf("afe: cs=%u reg=0x%02x -> 0x%04x\n", cs, reg, val);
	return CMD_RET_SUCCESS;
}

static int do_afe_write(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int cs;
	u8 reg;
	u16 val;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;
	if (parse_cs(argv[1], &cs))
		return CMD_RET_FAILURE;

	reg = (u8)simple_strtoul(argv[2], NULL, 0);
	val = (u16)simple_strtoul(argv[3], NULL, 0);

	ret = afe5828_write_reg(AFE_SPI_BUS, cs, reg, val);
	if (ret) {
		printf("afe: write cs=%u reg=0x%02x val=0x%04x failed: %d\n",
		       cs, reg, val, ret);
		return CMD_RET_FAILURE;
	}
	printf("afe: cs=%u reg=0x%02x <- 0x%04x OK\n", cs, reg, val);
	return CMD_RET_SUCCESS;
}

static struct cmd_tbl afe_subcmds[] = {
	U_BOOT_CMD_MKENT(detect, 1, 0, do_afe_detect, "", ""),
	U_BOOT_CMD_MKENT(reset,  2, 0, do_afe_reset,  "", ""),
	U_BOOT_CMD_MKENT(read,   3, 0, do_afe_read,   "", ""),
	U_BOOT_CMD_MKENT(write,  4, 0, do_afe_write,  "", ""),
};

static int do_afe(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *sub;

	if (argc < 2)
		return CMD_RET_USAGE;

	sub = find_cmd_tbl(argv[1], afe_subcmds, ARRAY_SIZE(afe_subcmds));
	if (!sub)
		return CMD_RET_USAGE;

	argc--;
	argv++;
	return sub->cmd(sub, flag, argc, argv);
}

U_BOOT_CMD(
	afe, 5, 0, do_afe,
	"AFE5828 SPI helper",
	"detect              - probe both AFEs (CS0 and CS1)\n"
	"afe reset  <cs>          - software reset one AFE\n"
	"afe read   <cs> <reg>    - read a register (8-bit addr)\n"
	"afe write  <cs> <reg> <val> - write a 16-bit value\n"
);
