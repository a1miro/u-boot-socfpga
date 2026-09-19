/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Texas Instruments AFE5828 SPI driver — U-Boot bring-up helpers.
 *
 * Protocol mirrored from software/app_spi_test (Linux userspace reference):
 *   24-bit frames: [reg:8][data_hi:8][data_lo:8], SPI mode 0.
 *   Register 0 controls the mode:
 *     write reg 0 = 0x0001  -> reset (waits ~200 ms afterwards)
 *     write reg 0 = 0x0000  -> normal write mode
 *     write reg 0 = 0x0002  -> readback mode (subsequent {reg,0,0} frames
 *                              return the register value in rx[1..2])
 *
 * Two AFEs live on one SPI bus at CS 0 and CS 1. They are only reachable
 * after the FPGA has been configured and the HPS<->FPGA bridges enabled.
 */

#ifndef _AFE5828_H_
#define _AFE5828_H_

#include <linux/types.h>

/* Hardware wiring — override at build time if needed. */
#ifndef AFE_SPI_BUS
#define AFE_SPI_BUS  1
#endif
#ifndef AFE_SPI_HZ
#define AFE_SPI_HZ   1000        /* 1 kHz — matches DTS placeholder + userspace reference */
#endif

#define AFE_CS_A     0
#define AFE_CS_B     1

/* Register 0 bits. */
#define AFE_R0_RESET     0x0001
#define AFE_R0_READBACK  0x0002

/*
 * Low-level access. Each call opens the SPI bus, does the transfer(s),
 * and releases the bus — safe to interleave with other SPI users.
 * Return 0 on success, negative errno on failure.
 */
int afe5828_reset(unsigned int bus_no, unsigned int cs);
int afe5828_write_reg(unsigned int bus_no, unsigned int cs, u8 reg, u16 value);
int afe5828_read_reg(unsigned int bus_no, unsigned int cs, u8 reg, u16 *value);

/*
 * Presence-detect for a single AFE:
 *   1. reset
 *   2. write reg 0x05 with `pattern`
 *   3. read reg 0x05 back into *readback
 *   4. return 0 iff *readback == pattern
 * On any SPI error the return value is negative errno.
 */
int afe5828_probe(unsigned int bus_no, unsigned int cs, u16 pattern, u16 *readback);

/*
 * Presence-detect both AFEs (CS 0 and CS 1). Prints a per-CS result line
 * and returns the number of AFEs successfully detected (0..2). Never
 * returns a negative value — call sites that want "warn and continue"
 * can ignore the return.
 */
int afe5828_probe_all(void);

#endif /* _AFE5828_H_ */
