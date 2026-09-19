// SPDX-License-Identifier: GPL-2.0
/*
 * Texas Instruments AFE5828 SPI driver — U-Boot bring-up helpers.
 * See afe5828.h for protocol details.
 */

/*
 * Override the header default before including it: run the AFEs at 10 MHz
 * regardless of what the header suggests.
 */
#define AFE_SPI_HZ         10000000 /* 10 MHz */

#include <command.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <spi.h>
#include <vsprintf.h>

#include "afe5828.h"

#define AFE_FRAME_BYTES  3
#define AFE_FRAME_BITS   (AFE_FRAME_BYTES * 8)
int afe_verbosity = 0;  /* set to 1 for debug prints */

/*
 * Both AFEs sit on the first dw_spi master (spi@10da4000, bus 1) as CS 0 and
 * CS 1. Bus 0 is the Cadence QSPI (NOR flash); bus 2 (spi@10da5000) is
 * currently unpowered/unmuxed and its FIFO poll hangs — skip it.
 */

// /* Helper function to inspect and print out SPI bus hardware properties */
// static void log_spi_bus_properties(struct udevice *bus_dev)
// {
//     printf("\n--- SPI BUS DIAGNOSTICS ---\n");
//     printf("Logical Bus ID: %d\n", dev_seq(bus_dev));
//     printf("Driver Name:    %s\n", bus_dev->driver->name);
//     printf("DT Node Name:   %s\n", bus_dev->name);

//     /* 1. Extract and display the physical base memory address */
//     fdt_addr_t base_addr = dev_read_addr(bus_dev);
//     if (base_addr != FDT_ADDR_T_NONE)
// 	    {
//         printf("Physical Regs:  0x%lx\n", (unsigned long)base_addr);
//         }
//     else
// 	    {
//         printf("Physical Regs:  Unknown / Unmapped\n");
//         }

//     /* 2. Retrieve the active bus maximum frequency property from the DT */
//     u32 max_freq = dev_read_u32_default(bus_dev, "spi-max-frequency", 0);
//     if (max_freq)
// 	    {
//         printf("DT Max Freq:    %u Hz (%u MHz)\n", max_freq, max_freq / 1000000);
//         }

//     /* 3. Probe and query the actual running input hardware clock */
//     // struct clk clk;
//     // int ret = clk_get_by_index(bus_dev, 0, &clk);
//     // if (!ret)
// 	   //  {
//     //     ulong rate = clk_get_rate(&clk);
//     //     if (rate > 0 && rate != -ENOSYS)
// 		  //   {
//     //         printf("Hardware Clock: %lu Hz (%lu MHz)\n", rate, rate / 1000000);
//     //         }
//     //     clk_free(&clk);
//     //     }
// 	   // else
// 	   //  {
//     //     printf("Hardware Clock: Could not read clock provider\n");
//     //     }
//     printf("---------------------------\n\n");
// }

static int afe_open(unsigned int bus_no, unsigned int cs, struct spi_slave **slavep)
{
    struct udevice *bus_dev;
    struct spi_slave *slave;
    char dev_name[24];
    int ret;

    if (afe_verbosity)
        printf("AFE-OPEN: Requesting SPI bus: %d, CS: %u\n", bus_no, cs);

    /*
     * U-Boot has no driver bound to "ti,afe5828" / "linux,spidev", so the
     * DT child nodes for the AFEs never enter the DM tree — the 4-arg
     * spi_get_bus_and_cs() would return -ENODEV. Use the 8-arg form with
     * "spi_generic_drv" so U-Boot auto-binds a generic slave on this CS,
     * matching the pattern used by cmd/spi.c and other Intel boards.
     */
    snprintf(dev_name, sizeof(dev_name), "afe5828_%u_%u", bus_no, cs);
    ret = _spi_get_bus_and_cs(bus_no, cs, AFE_SPI_HZ, SPI_MODE_0,
                              "spi_generic_drv", dev_name, &bus_dev, &slave);
    if (ret)
	    {
        printf("AFE-OPEN: _spi_get_bus_and_cs failed: %d\n", ret);
        return ret;
        }


    ret = spi_claim_bus(slave);
    if (ret)
	    {
        printf("AFE-OPEN: spi_claim_bus(cs=%u) failed: %d\n", cs, ret);
        return ret;
        }

    *slavep = slave;
    return 0;
}

static void afe_close(struct spi_slave *slave)
   {
	spi_release_bus(slave);
   }

/* One 24-bit frame, tx/rx buffers must both hold 3 bytes. */
static int afe_xfer(struct spi_slave *slave, const u8 *tx, u8 *rx)
   {
   return spi_xfer(slave, AFE_FRAME_BITS, tx, rx, SPI_XFER_ONCE);
   }

/* Write reg 0 = value while the slave is already claimed. */
static int afe_write_r0(struct spi_slave *slave, u16 value)
    {
	u8 tx[AFE_FRAME_BYTES] = { 0x00, (u8)(value >> 8), (u8)value };
	u8 rx[AFE_FRAME_BYTES] = { 0, 0, 0 };
	return afe_xfer(slave, tx, rx);
    }

int afe5828_reset(unsigned int bus_no, unsigned int cs)
    {
	struct spi_slave *slave;
	int ret;

	ret = afe_open(bus_no, cs, &slave);
	if (ret)
	    {
		return ret;
		}
	if (afe_verbosity)
		printf("AFE-RESET: Open succeded\n");
	ret = afe_write_r0(slave, AFE_R0_RESET);
	afe_close(slave);
	if (ret)
	    {
	    printf("AFE-RESET: Write failed\n");
		return ret;
		}

	if (afe_verbosity)
		printf("AFE-RESET: Write succeeded\n");

	mdelay(200);   /* matches userspace reference */
	return 0;
   }

int afe5828_write_reg(unsigned int bus_no, unsigned int cs, u8 reg, u16 value)
   {
	struct spi_slave *slave;
	u8 tx[AFE_FRAME_BYTES] = { reg, (u8)(value >> 8), (u8)value };
	u8 rx[AFE_FRAME_BYTES] = { 0, 0, 0 };
	int ret;

	ret = afe_open(bus_no, cs, &slave);
	if (ret)
		return ret;

	/* enter write mode, then send the actual write frame */
	ret = afe_write_r0(slave, 0x0000);
	if (!ret)
		ret = afe_xfer(slave, tx, rx);

	afe_close(slave);
	return ret;
}

int afe5828_read_reg(unsigned int bus_no, unsigned int cs, u8 reg, u16 *value)
{
	struct spi_slave *slave;
	u8 tx[AFE_FRAME_BYTES] = { reg, 0x00, 0x00 };
	u8 rx[AFE_FRAME_BYTES] = { 0, 0, 0 };
	int ret;

	if (!value)
		return -EINVAL;

	ret = afe_open(bus_no, cs, &slave);
	if (ret)
		return ret;

	/*
	 * Enter readback mode. The userspace reference sends this frame twice
	 * before the actual read; keeping the same doubled send here so we
	 * behave identically to the known-working path.
	 */
	ret = afe_write_r0(slave, AFE_R0_READBACK);
	if (!ret)
		ret = afe_write_r0(slave, AFE_R0_READBACK);
	if (!ret)
		ret = afe_xfer(slave, tx, rx);

	afe_close(slave);
	if (ret)
		return ret;

	*value = ((u16)rx[1] << 8) | rx[2];
	return 0;
}

int afe5828_probe(unsigned int bus_no,  unsigned int cs, u16 pattern, u16 *readback)
{
	u16 v = 0;
	int ret;

	ret = afe5828_reset(bus_no, cs);
	if (ret)
	    {
		return ret;
		}

	ret = afe5828_write_reg(bus_no, cs, 0x05, pattern);
	if (ret)
	    {
		return ret;
		}

	ret = afe5828_read_reg(bus_no, cs, 0x05, &v);

	if (ret)
	     {
		return ret;
		 }

	if (readback)
	    {
		*readback = v;
		}

	return (v == pattern) ? 0 : -ENODEV;
}

int afe5828_probe_all(void)
    {
	static const struct {
		unsigned int cs;
		u16 pattern;
	} targets[] = {
		{ AFE_CS_A, 0xA5A5 },
		{ AFE_CS_B, 0x5A5A },
	};
	int found = 0;
	unsigned int i;
	unsigned int bus_no = AFE_SPI_BUS;

	printf("AFE5828: probing bus %u (@%u Hz)\n", bus_no, AFE_SPI_HZ);

	for (i = 0; i < ARRAY_SIZE(targets); i++)
		{
		u16 back = 0;
		int ret = afe5828_probe(bus_no, targets[i].cs, targets[i].pattern, &back);
		if (ret == 0)
			{
			printf("AFE5828  CS%u: OK (reg 0x05 wrote 0x%04x, read 0x%04x)\n", targets[i].cs, targets[i].pattern, back);
			found++;
			}
		else if (ret == -ENODEV)
			{
			printf("AFE5828  CS%u: MISMATCH (wrote 0x%04x, read 0x%04x) — no device?\n", targets[i].cs, targets[i].pattern, back);
			found = -1;
			break;
			}
		else
			{
			printf("AFE5828  CS%u: SPI error %d\n", targets[i].cs, ret);
			found = -2;
			break;
			}
		}
	return found;
    }
