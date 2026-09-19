/* SPDX-License-Identifier: GPL-2.0 */
/*
 * TPM presence-detect and safe-debug helpers for the iG58m board.
 *
 * The TPM (ST33HTPH2XSPI) sits on U-Boot SPI bus 2 (DTS &spi1, DW SSI at
 * 0x10da5000), CS 0. Its SPI clocks are only routed once the FPGA has been
 * configured and the HPS<->FPGA bridges are enabled — calling into the SPI
 * DM stack before that used to lock the CPU in the DW SPI FIFO poll.
 *
 * Both entry points here run a staged diagnostic. Stages A..D are safe under
 * any hardware state (they only read CPU-side CSRs). Stage E actually
 * transfers SPI bytes but is timeout-bounded (it bypasses u-boot's
 * spi_xfer()/poll_transfer(), which has no timeout — see tpm_test.c's Stage E
 * comment). Only Stage F, reached solely once E has proven the wire works,
 * relies on unmodified upstream driver code.
 *
 * Stages:
 *   A. Reset-manager peek     — is SPIM1 out of reset? (informational)
 *   B. DW SSI CSR peek        — do the controller CSRs read plausible values?
 *                                (informational)
 *   C. DM lookup (no probe)   — does UCLASS_SPI bus 2 exist in the DM tree?
 *   D. Bus probe              — device_probe() on the SPI bus, then confirm
 *                                SPIM1's reset is actually released.
 *   E. Bounded raw SPI xfer   — 4-byte DID_VID request via direct DW SSI
 *                                register access, every wait timeout-bounded.
 *   F. UCLASS_TPM handoff     — DM TIS-SPI driver + tpm_get_desc().
 *
 * SPIM1's reset is only cleared as a side effect of Stage D's probe, and
 * nothing else probes bus 2 before a human runs one of these commands, so
 * Stages A/B are expected to look "dead" pre-probe on every cold boot —
 * they're informational only. The real check is after Stage D.
 *
 * Passing Stage D only proves the controller is out of reset — it says
 * nothing about whether the TPM chip itself is present/clocked/responding.
 * Stage E is what actually tests the wire, and a non-responding chip now
 * produces a clean -ETIMEDOUT there instead of hanging the CPU.
 *
 * Each stage prints on entry and exit, so if the CPU still hangs the last
 * visible line names the offending stage.
 */

#ifndef _TPM_TEST_H_
#define _TPM_TEST_H_

/*
 * Non-intrusive check: runs stages A..D only, never touches the SPI wire.
 * Safe to call before the FPGA is configured.
 * Returns 0 if the bus looks safe to talk to, negative errno otherwise.
 */
int tpm_debug(void);

/*
 * Full presence detect: runs stages A..F, aborts on the first failing stage.
 * Prints a one-line vendor/device summary on success.
 * Returns 0 on success, negative errno on failure.
 */
int tpm_detect(void);

#endif /* _TPM_TEST_H_ */
