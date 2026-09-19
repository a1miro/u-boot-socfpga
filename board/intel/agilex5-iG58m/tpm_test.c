// SPDX-License-Identifier: GPL-2.0
/*
 * TPM debug + presence-detect for the iG58m board.
 *
 * Historically tpm_detect() jumped straight from a raw ping into spi_claim_bus()
 * / spi_xfer(). If bus 2 (DW SSI @ 0x10da5000) was held in reset, wasn't
 * clocked, or the wire transfer itself never completed (dead SCLK, no chip
 * responding, bad fabric routing), u-boot's generic SPI driver would spin
 * forever polling the FIFO status register (drivers/spi/designware_spi.c:
 * poll_transfer() has no timeout) and lock up the CPU — no prompt, no way to
 * reload the .rbf.
 *
 * The new flow is a staircase of increasingly intrusive checks. Each stage
 * prints on entry and on completion, so if the CPU ever does wedge again the
 * last printed line names the exact stage that crossed the tripwire.
 *
 *   A. Reset-manager peek     — is SPIM1 out of reset? (informational)
 *   B. DW SSI CSR peek        — do the controller CSRs read plausible values?
 *                                (informational)
 *   C. DM lookup (no probe)   — does UCLASS_SPI bus 2 exist in the DM tree?
 *   D. Bus probe              — device_probe() on the SPI bus, then confirm
 *                                SPIM1's reset is actually released.
 *   E. Bounded raw SPI xfer   — 4-byte DID_VID request via direct DW SSI
 *                                register access (deliberately not
 *                                spi_xfer(), see Stage E's own comment).
 *   F. UCLASS_TPM handoff     — DM TIS-SPI driver + tpm_get_desc().
 *
 * SPIM1's reset is only ever cleared as a side effect of Stage D's
 * device_probe() (dw_spi_probe() -> dw_spi_reset() -> reset_deassert_bulk()).
 * Nothing else probes bus 2 before a human runs tpm_debug/tpm_detect —
 * board_late_init() deliberately skips it (see socfpga-iG58m.c) to avoid the
 * historical hang. That means Stages A and B will read "asserted" /
 * "not clocked" on every cold boot, before Stage D has ever run — that is
 * expected, not a fault, so A and B are informational only and never abort
 * the sequence. The real pass/fail check is done right after Stage D, by
 * re-reading the reset bit: if it's still asserted once probing has
 * actually run, that's a genuine problem.
 *
 * Confirming the controller is out of reset (Stage D) does NOT guarantee the
 * wire transfer itself will complete — that also depends on the TPM chip
 * actually being present/clocked/responding, which stages A-D cannot check.
 * Stage E is where that gets tested, and it's built to fail cleanly
 * (-ETIMEDOUT) rather than hang if the transfer never completes.
 *
 * Stages A–E are all bounded / non-blocking now; only Stage F (via the DM
 * TIS-SPI driver, not our own code) is unguarded, and it's only reached once
 * E has already proven the wire works.
 *
 * Two commands are exposed:
 *   `tpm_debug`  — runs A..D only, purely diagnostic, cannot hang.
 *   `tpm_detect` — runs A..F, aborts at the first failure.
 */

#include <clk.h>
#include <command.h>
#include <dm.h>
#include <dm/device-internal.h> /* device_probe() */
#include <dm/uclass-internal.h> /* uclass_find_device_by_seq() */
#include <errno.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <asm/io.h>
#include <log.h>
#include <tpm-common.h>

#include "tpm_test.h"

/* -------------------------------------------------------------------------
 * Wiring / hardware constants
 * ------------------------------------------------------------------------- */

/* TPM is the second DW SPI master in the DT (spi1). U-Boot enumerates the
 * Cadence QSPI as bus 0, so the DW SSIs are buses 1 and 2. */
#define TPM_SPI_BUS         2
#define TPM_SPI_CS          0
#define TPM_SPI_HZ          10000000        /* 10 MHz, matches DTS */
#define TPM_WAIT_STATES     100             /* upper bound per upstream driver */

/* Agilex5 Reset Manager. Bit 18 of PER0MODRST gates SPIM1
 * (see dt-bindings/reset/altr,rst-mgr-agx5.h: SPIM1_RESET = 18). */
#define AGX5_RSTMGR_BASE        0x10D11000
#define AGX5_RSTMGR_STAT        (AGX5_RSTMGR_BASE + 0x00)
#define AGX5_RSTMGR_PER0MODRST  (AGX5_RSTMGR_BASE + 0x24)
#define AGX5_RSTMGR_SPIM1_BIT   BIT(18)

/* DW APB SSI register layout — CPU-side CSRs, respond as long as SPIM1 is
 * out of reset (they don't need the functional SPI clock). */
#define DW_SSI_BASE_SPI1        0x10DA5000
#define DW_SSI_CTRLR0           0x00
#define DW_SSI_SSIENR           0x08
#define DW_SSI_SER              0x10
#define DW_SSI_BAUDR            0x14
#define DW_SSI_SR               0x28
#define DW_SSI_DR               0x60
#define DW_SSI_VERSION_ID       0x5C        /* ASCII, e.g. 0x3430312A ("*104") */
#define DW_SSI_COMP_TYPE        0x58        /* usually 0x46570001 */

#define DW_SSI_SR_BUSY          BIT(0)
#define DW_SSI_SR_TFNF          BIT(1)      /* tx fifo not full */
#define DW_SSI_SR_TFE           BIT(2)      /* tx fifo empty */
#define DW_SSI_SR_RFNE          BIT(3)      /* rx fifo not empty */

/*
 * CTRLR0 for an 8-bit, Motorola-SPI, mode-0, transmit+receive frame — the
 * one config Stage E ever needs. This DT node is "snps,dw-apb-ssi" (the
 * generic compatible), so u-boot's driver auto-detects the legacy 32-bit-DFS
 * register layout (drivers/spi/designware_spi.c: dw_spi_apb_init() ->
 * dw_spi_dw32_update_cr0()), where DFS lives at bits[20:16] as (frame_size -
 * 1). Confirmed against a live Stage-B CTRLR0 readback (bits[20:16] were the
 * implemented ones). FRF/MODE/TMOD all stay 0 (Motorola SPI, CPOL=CPHA=0,
 * TX&RX), so only the DFS field is non-zero: 7 = 8-bit words minus one.
 */
#define DW_SSI_CTRLR0_XFER_8BIT_MODE0_TR   (7u << 16)

#define TPM_SPI_POLL_TIMEOUT_US 50000       /* bound per register wait */
#define TPM_SPI_POLL_STEP_US    10

/* TIS register offsets, locality 0. */
#define TPM_REG_DID_VID     0x0F00

/* -------------------------------------------------------------------------
 * Stage A — reset-manager peek (informational only, see file header: this
 * always finds SPIM1 in reset pre-probe, so it must not gate the sequence)
 * ------------------------------------------------------------------------- */
static void tpm_dbg_check_reset(void)
{
    u32 per0, stat;

    printf("TPM/A:\treset-manager peek\n");
    stat  = readl(AGX5_RSTMGR_STAT);
    per0  = readl(AGX5_RSTMGR_PER0MODRST);
    printf("TPM/A:\tRSTMGR STAT=0x%08x  PER0MODRST=0x%08x\n", stat, per0);

    if (per0 & AGX5_RSTMGR_SPIM1_BIT)
        printf("TPM/A:\tSPIM1 reset bit is asserted (bit18 set) — expected pre-probe, Stage D releases it\n");
    else
        printf("TPM/A:\tSPIM1 already out of reset\n");
}

/* -------------------------------------------------------------------------
 * Post-Stage-D confirmation — the real reset-manager gate.
 *
 * device_probe() (Stage D) is what actually deasserts SPIM1's reset, via
 * dw_spi_reset() -> reset_deassert_bulk(). Re-read the bit now: if it's
 * still asserted after a successful probe, that's a genuine problem
 * (clocks/resets DT binding, hardware fault) rather than the expected
 * pre-probe state Stage A saw.
 * ------------------------------------------------------------------------- */
static int tpm_dbg_confirm_reset_released(void)
{
    u32 per0 = readl(AGX5_RSTMGR_PER0MODRST);

    if (per0 & AGX5_RSTMGR_SPIM1_BIT) {
        printf("TPM/D:\tSPIM1 reset STILL asserted after probe (PER0MODRST=0x%08x) — controller off\n", per0);
        printf("TPM/D:\thint: reset_deassert for SPIM1 failed — check clocks/resets DT binding\n");
        return -EIO;
    }

    printf("TPM/D:\tSPIM1 confirmed out of reset after probe — OK\n");
    return 0;
}

/* -------------------------------------------------------------------------
 * Stage B — DW SSI CSR peek (read-only, no side effects; informational
 * only, see file header — while SPIM1 is still in reset these reads are
 * expected to look dead, so this must not gate the sequence either)
 * ------------------------------------------------------------------------- */
static void tpm_dbg_check_csr(void)
{
    u32 ctrlr0, ssienr, sr, ver, comp;

    printf("TPM/B:\tDW SSI CSR peek @ 0x%08x\n", DW_SSI_BASE_SPI1);

    ver    = readl(DW_SSI_BASE_SPI1 + DW_SSI_VERSION_ID);
    comp   = readl(DW_SSI_BASE_SPI1 + DW_SSI_COMP_TYPE);
    ctrlr0 = readl(DW_SSI_BASE_SPI1 + DW_SSI_CTRLR0);
    ssienr = readl(DW_SSI_BASE_SPI1 + DW_SSI_SSIENR);
    sr     = readl(DW_SSI_BASE_SPI1 + DW_SSI_SR);

    /* VERSION_ID is 4 printable ASCII chars in every DW SSI IP release. */
    printf("TPM/B:\tVERSION_ID=0x%08x '%c%c%c%c'  COMP_TYPE=0x%08x\n",
           ver,
           (ver >> 24) & 0x7f ? (ver >> 24) & 0x7f : '.',
           (ver >> 16) & 0x7f ? (ver >> 16) & 0x7f : '.',
           (ver >>  8) & 0x7f ? (ver >>  8) & 0x7f : '.',
           (ver >>  0) & 0x7f ? (ver >>  0) & 0x7f : '.',
           comp);
    printf("TPM/B:\tCTRLR0=0x%08x  SSIENR=0x%08x  SR=0x%08x\n", ctrlr0, ssienr, sr);

    if (ver == 0x00000000 || ver == 0xFFFFFFFF)
        printf("TPM/B:\tVERSION_ID reads %s — expected pre-probe, Stage D releases the reset\n",
               ver ? "0xFFFFFFFF" : "0x00000000");
    else
        printf("TPM/B:\tcontroller CSRs look alive already — OK\n");
}

/* -------------------------------------------------------------------------
 * Stage C — DM lookup, no probe (uclass_find_device_by_seq() never probes)
 * ------------------------------------------------------------------------- */
static int tpm_dbg_check_dm_lookup(struct udevice **bus_out)
{
    struct udevice *bus = NULL;
    int ret;

    printf("TPM/C:\tDM lookup UCLASS_SPI seq=%d (no probe)\n", TPM_SPI_BUS);

    ret = uclass_find_device_by_seq(UCLASS_SPI, TPM_SPI_BUS, &bus);
    if (ret || !bus) {
        printf("TPM/C:\tuclass_find_device_by_seq failed: %d (dev=%p)\n", ret, bus);
        return ret ? ret : -ENODEV;
    }

    printf("TPM/C:\tfound bus '%s' (activated=%d)\n",
           bus->name, device_active(bus) ? 1 : 0);

    if (bus_out)
        *bus_out = bus;
    return 0;
}

/* -------------------------------------------------------------------------
 * Stage D — bus probe. First stage that actually touches the peripheral.
 *
 * Safe to call regardless of what A/B saw: dw_spi_probe()'s setup path
 * (dw_spi_calc_fifo_len() et al.) is a bounded register-poke loop, not an
 * unbounded poll, and it's what calls dw_spi_reset() to deassert SPIM1 in
 * the first place — that's the whole point of reaching this stage.
 * ------------------------------------------------------------------------- */
static int tpm_dbg_probe_bus(struct udevice *bus)
{
    int ret;

    printf("TPM/D:\tdevice_probe('%s')\n", bus->name);

    if (device_active(bus)) {
        printf("TPM/D:\talready active — OK\n");
        return 0;
    }

    ret = device_probe(bus);
    if (ret) {
        printf("TPM/D:\tdevice_probe failed: %d\n", ret);
        return ret;
    }

    printf("TPM/D:\tbus probed — OK\n");
    return 0;
}

/* -------------------------------------------------------------------------
 * Stage E — bounded raw SPI transfer.
 *
 * This deliberately does NOT call u-boot's spi_xfer(). Its generic
 * implementation (drivers/spi/designware_spi.c: dw_spi_xfer() ->
 * poll_transfer()) contains an *unbounded* loop that spins until the RX
 * FIFO fills — if the transfer never completes on the wire (dead clock,
 * unresponsive/missing chip, bad fabric routing), that loop never returns
 * and hangs the CPU. That's exactly the historical bug this whole staged
 * diagnostic exists to avoid, and it's exactly what was hit here: Stages
 * A-D above already prove the controller itself is alive and out of
 * reset, so a hang at this point means the actual wire transfer is what's
 * stuck, not the reset/DM plumbing.
 *
 * Instead, every register wait below (tpm_dbg_wait_sr()) is bounded by an
 * explicit timeout, so a non-responding TPM produces a clean -ETIMEDOUT
 * instead of locking up the board. CTRLR0/BAUDR/SER are all configured
 * directly by tpm_dbg_raw_ping() below (via the clock framework on the
 * already-probed bus device, not via _spi_get_bus_and_cs()/spi_claim_bus()
 * — those resolve to the DTS's own tpm@0 node and end up silently running
 * the stock upstream tpm_tis-spi driver's transfer instead of this one).
 * SSIENR must be 0 while writing control registers, per the DW SSI spec.
 *
 * TIS-SPI wire framing (per TCG PTP §7.4):
 *   Request:  [ (rd?0x80:0x00) | (n-1) ] [ 0xD4 ] [ addr_hi ] [ addr_lo ]
 *   After the request the TPM either drives byte-3 LSB high on the same
 *   4-byte frame (immediate ready), or inserts 0x00 bytes ("wait states")
 *   until it can drive LSB high, then n data bytes follow. CS must stay
 *   asserted continuously across the whole exchange, so SSIENR is only
 *   toggled once at the start and once at the end.
 * ------------------------------------------------------------------------- */
static int tpm_dbg_wait_sr(u32 mask, u32 want)
{
    unsigned int i;

    for (i = 0; i < TPM_SPI_POLL_TIMEOUT_US / TPM_SPI_POLL_STEP_US; i++) {
        if ((readl(DW_SSI_BASE_SPI1 + DW_SSI_SR) & mask) == want)
            return 0;
        udelay(TPM_SPI_POLL_STEP_US);
    }
    return -ETIMEDOUT;
}

/* Full-duplex single-byte exchange: in TMOD_TR every byte written to DR
 * simultaneously shifts a byte into the RX FIFO, so a write is always
 * paired with a read of the byte it clocked in. */
static int tpm_dbg_dw_xfer_byte(u8 tx, u8 *rx)
{
    int ret;

    ret = tpm_dbg_wait_sr(DW_SSI_SR_TFNF, DW_SSI_SR_TFNF);
    if (ret)
        return ret;
    writel(tx, DW_SSI_BASE_SPI1 + DW_SSI_DR);

    ret = tpm_dbg_wait_sr(DW_SSI_SR_RFNE, DW_SSI_SR_RFNE);
    if (ret)
        return ret;
    *rx = (u8)readl(DW_SSI_BASE_SPI1 + DW_SSI_DR);
    return 0;
}

static int tpm_dbg_raw_read(u16 addr, u8 *buf, u8 len)
{
    u8 tx[4] = {
        0x80 | (u8)(len - 1),
        0xD4,
        (u8)(addr >> 8),
        (u8)(addr & 0xFF),
    };
    u8 rx[4] = {0, 0, 0, 0};
    int ret, i;

    writel(0, DW_SSI_BASE_SPI1 + DW_SSI_SSIENR);
    writel(DW_SSI_CTRLR0_XFER_8BIT_MODE0_TR, DW_SSI_BASE_SPI1 + DW_SSI_CTRLR0);
    writel(BIT(TPM_SPI_CS), DW_SSI_BASE_SPI1 + DW_SSI_SER);
    writel(1, DW_SSI_BASE_SPI1 + DW_SSI_SSIENR);

    for (i = 0; i < 4; i++) {
        ret = tpm_dbg_dw_xfer_byte(tx[i], &rx[i]);
        if (ret) {
            printf("TPM/E:\trequest xfer timed out at byte %d — SCLK/MISO likely dead\n", i);
            goto done;
        }
    }

    if (!(rx[3] & 0x01)) {
        u8 ws = 0;

        for (i = 0; i < TPM_WAIT_STATES; i++) {
            ret = tpm_dbg_dw_xfer_byte(0xFF, &ws);
            if (ret) {
                printf("TPM/E:\twait-state xfer timed out\n");
                goto done;
            }
            if (ws & 0x01)
                break;
        }
        if (i == TPM_WAIT_STATES) {
            printf("TPM/E:\twait-state timeout (chip not ACKing)\n");
            ret = -ETIMEDOUT;
            goto done;
        }
    }

    for (i = 0; i < len; i++) {
        ret = tpm_dbg_dw_xfer_byte(0xFF, &buf[i]);
        if (ret) {
            printf("TPM/E:\tdata xfer timed out at byte %d\n", i);
            goto done;
        }
    }
    ret = 0;

done:
    /* Disable — this also deasserts CS, ending the exchange. */
    writel(0, DW_SSI_BASE_SPI1 + DW_SSI_SSIENR);
    return ret;
}

static int tpm_dbg_raw_ping(struct udevice *bus, u32 *vend_dev_out)
{
    struct clk clk;
    ulong rate;
    u16 baudr;
    u8 buf[4] = {0, 0, 0, 0};
    u32 vend_dev;
    int ret;

    printf("TPM/E:\traw ping bus=%u cs=%u @%u Hz (direct register access)\n",
           TPM_SPI_BUS, TPM_SPI_CS, TPM_SPI_HZ);

    /*
     * Deliberately NOT using _spi_get_bus_and_cs()/spi_claim_bus() here.
     * The DTS already declares a real device at (bus 2, cs 0) — the
     * upstream tpm_tis-spi node — so spi_find_chip_select() inside
     * _spi_get_bus_and_cs() finds THAT device and probes it instead of
     * creating a generic slave, meaning it silently runs the stock
     * driver's own spi_xfer()-based transfer instead of ours. Get the
     * bus clock straight from the already-probed controller device
     * (from Stage C/D) instead, and set BAUDR ourselves — this
     * guarantees the bounded register-level code below is what actually
     * runs.
     */
    ret = clk_get_by_index(bus, 0, &clk);
    if (ret) {
        printf("TPM/E:\tclk_get_by_index failed: %d\n", ret);
        return ret;
    }
    ret = clk_enable(&clk);
    if (ret && ret != -ENOSYS && ret != -ENOTSUPP) {
        printf("TPM/E:\tclk_enable failed: %d\n", ret);
        return ret;
    }
    rate = clk_get_rate(&clk);
    if (!rate) {
        printf("TPM/E:\tclk_get_rate returned 0\n");
        return -EINVAL;
    }

    /* Matches dw_spi_set_speed(): clk_div doesn't support odd numbers. */
    baudr = (u16)(rate / TPM_SPI_HZ);
    baudr = (baudr + 1) & 0xFFFE;
    printf("TPM/E:\tbus clk=%lu Hz -> BAUDR=%u\n", rate, baudr);

    writel(0, DW_SSI_BASE_SPI1 + DW_SSI_SSIENR);
    writel(baudr, DW_SSI_BASE_SPI1 + DW_SSI_BAUDR);
    writel(1, DW_SSI_BASE_SPI1 + DW_SSI_SSIENR);

    ret = tpm_dbg_raw_read(TPM_REG_DID_VID, buf, sizeof(buf));
    if (ret)
        return ret;

    vend_dev = (u32)buf[0]         | ((u32)buf[1] << 8) |
               ((u32)buf[2] << 16) | ((u32)buf[3] << 24);

    printf("TPM/E:\tDID_VID raw = %02x %02x %02x %02x -> 0x%08x\n",
           buf[0], buf[1], buf[2], buf[3], vend_dev);

    if (vend_dev_out)
        *vend_dev_out = vend_dev;

    if (vend_dev == 0x00000000 || vend_dev == 0xFFFFFFFF) {
        printf("TPM/E:\tno chip responding (bus alive but MISO stuck)\n");
        return -ENODEV;
    }

    printf("TPM/E:\tVendorID 0x%04x DeviceID 0x%04x — OK\n",
           vend_dev & 0xFFFF, (vend_dev >> 16) & 0xFFFF);
    return 0;
}

/* -------------------------------------------------------------------------
 * Stage F — UCLASS_TPM DM handoff (only reached if E succeeded)
 * ------------------------------------------------------------------------- */
static int tpm_dbg_dm_handoff(void)
{
    struct udevice *dev = NULL;
    char desc[128];
    int ret;

    printf("TPM/F:\thanding off to UCLASS_TPM driver\n");

    ret = uclass_find_first_device(UCLASS_TPM, &dev);
    if (ret || !dev) {
        printf("TPM/F:\tno UCLASS_TPM child bound (ret=%d, dev=%p)\n", ret, dev);
        return ret ? ret : -ENODEV;
    }

    ret = device_probe(dev);
    if (ret) {
        printf("TPM/F:\t%s DM probe failed: %d\n", dev->name, ret);
        return ret;
    }

    ret = tpm_get_desc(dev, desc, sizeof(desc));
    if (ret < 0) {
        printf("TPM/F:\t%s: tpm_get_desc failed: %d\n", dev->name, ret);
        return ret;
    }

    printf("TPM/F:\t%s — OK\n", desc);
    return 0;
}

/* -------------------------------------------------------------------------
 * Public entry points
 * ------------------------------------------------------------------------- */

/*
 * Non-intrusive diagnostic: stages A..D only. Never touches the SPI wire, so
 * it cannot hang the CPU regardless of FPGA/bridge state.
 */
int tpm_debug(void)
{
    struct udevice *bus = NULL;
    int ret;

    printf("TPM:\t---- tpm_debug: staged diagnostic ----\n");

    /* A and B are informational only — see file header. */
    tpm_dbg_check_reset();
    tpm_dbg_check_csr();

    ret = tpm_dbg_check_dm_lookup(&bus);
    if (ret)
        goto out;

    ret = tpm_dbg_probe_bus(bus);
    if (ret)
        goto out;

    /* This is the real gate: confirm the probe actually released SPIM1. */
    ret = tpm_dbg_confirm_reset_released();
    if (ret)
        goto out;
    tpm_dbg_check_csr();

    printf("TPM:\tstages A..D passed — bus is safe to talk to\n");
out:
    printf("TPM:\t---- tpm_debug done (ret=%d) ----\n", ret);
    return ret;
}

/*
 * Full detect: A..F. Bails out on the first failing stage, so calling this
 * before the FPGA is loaded prints where we stopped instead of locking up.
 */
int tpm_detect(void)
{
    struct udevice *bus = NULL;
    u32 vend_dev = 0;
    int ret;

    printf("TPM:\t---- tpm_detect: full staged probe ----\n");

    /* A and B are informational only — see file header. */
    tpm_dbg_check_reset();
    tpm_dbg_check_csr();

    ret = tpm_dbg_check_dm_lookup(&bus);
    if (ret)
        goto out;

    ret = tpm_dbg_probe_bus(bus);
    if (ret)
        goto out;

    /* This is the real gate: confirm the probe actually released SPIM1. */
    ret = tpm_dbg_confirm_reset_released();
    if (ret)
        goto out;
    tpm_dbg_check_csr();

    ret = tpm_dbg_raw_ping(bus, &vend_dev);
    if (ret)
        goto out;

    ret = tpm_dbg_dm_handoff();
out:
    printf("TPM:\t---- tpm_detect done (ret=%d) ----\n", ret);
    return ret;
}

/* -------------------------------------------------------------------------
 * U-Boot commands
 * ------------------------------------------------------------------------- */

static int do_tpm_debug(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    int ret = tpm_debug();
    return ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static int do_tpm_detect(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    int ret = tpm_detect();
    return ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

U_BOOT_CMD(tpm_debug, 1, 0, do_tpm_debug,
           "Non-intrusive TPM/SPI-bus diagnostic (stages A..D, never hangs)",
           "\n"
           "    - reset-manager peek (SPIM1 out of reset?) [informational]\n"
           "    - DW SSI CSR peek at 0x10da5000 (VERSION_ID/CTRLR0/SR) [informational]\n"
           "    - DM lookup of UCLASS_SPI bus 2 (no probe)\n"
           "    - device_probe() of the bus, then confirm SPIM1's reset released\n"
           "  Safe to run before the FPGA has routed SPI pins.\n");

U_BOOT_CMD(tpm_detect, 1, 0, do_tpm_detect,
           "Full TPM detect: staged probe on SPI bus 2, aborts on first fail",
           "\n"
           "    - runs stages A..D as tpm_debug does,\n"
           "    - then raw-SPI DID_VID ping via direct DW SSI register\n"
           "      access — bounded/timeout-protected, does not use\n"
           "      spi_xfer() (which has no timeout upstream),\n"
           "    - then hands off to the DM TPM2 TIS-SPI driver\n");
