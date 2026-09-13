/* This file contains the iWave Global IG58M board FPGA/HPS DDR memory test as a part of POST
 *
 * The Memory tests will be done for 
 * 1. HPS DDR : Data Line Integrity Test, Address Line Integrity Test, 
 *              Pattern Test[Fixed Range], ECC Single bit and Double Bit Fail Test.
 * 2. FPGA DDR : Data Line Integrity Test, Address Line Integrity Test, 
 *               Pattern Test [Fixed Range]
 */

#include <command.h>
#include <console.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include <linux/string.h>
#include <linux/types.h>
#include <asm/io.h>
#include <stdio.h>

#include <command.h>
#include <linux/string.h>
#include <linux/types.h>
#include <asm/io.h>
#include <stdio.h>
#include <console.h> /* For ctrlc */
#include <vsprintf.h> /* For simple_strtoul */
#include <bootretry.h>
#include <cli.h>
#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif
#include <hash.h>
#include <watchdog.h>
#include <linux/compiler.h>
#include <mapmem.h>

#define DBG 0

#define HPS_START_ADDR        0x81000000
#define HPS_END_ADDR          0xF0000000
#define FPGA_START_ADDR_1     0x40000000
#define FPGA_END_ADDR_1       0x7FFFFFFF
#define FPGA_START_ADDR_2     0x0440000000
#define FPGA_END_ADDR_2       0x047fffffff
#define FPGA_START_ADDR_3     0x0480000000
#define FPGA_END_ADDR_3       0x04ffffffff
#define TEST_DATA_1           0x55555555
#define TEST_DATA_2           0xAAAAAAAA
#define SYS_MEMTEST_SCRATCH   0x81800000
#define VULONG                unsigned long long
#define ULONG                 unsigned long

int do_dl_test(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
    VULONG addr;
    ULONG read_data, write_data;
    ULONG dl_hps_err = 0, dl_fpga_err = 0;

    if (strcmp(argv[0], "dl_test\0") == 0)
        addr = (long long unsigned int) map_sysmem(HPS_START_ADDR, 0);
    else if (strcmp(argv[0], "fpga_dl_test\0") == 0)
        addr = (long long unsigned int) map_sysmem(FPGA_START_ADDR_1, 0);

    /* Perform the walk in 1 test for data bus to identify if any of the data bits are struck at LOW */
    printf("\r Walk in 1 test for data bus\n");
    write_data = 0x0000000000000001;
    while (write_data)
    {
#if DBG 
        printf("\n Writing value %08lx to address 0x%.8llx\n", write_data, addr);
#endif
        *(VULONG *) addr = write_data;
        read_data = *(VULONG *) addr;
#if DBG
        printf("\n Read data %08lx\n", read_data);
#endif
        if (read_data != write_data)
        {
            if (strcmp(argv[0], "dl_test\0") == 0)
                dl_hps_err++;
            else if (strcmp(argv[0], "fpga_dl_test\0") == 0)
                dl_fpga_err++;
        }
        addr += 8;
        write_data <<= 1;
    }

    if (strcmp(argv[0], "dl_test\0") == 0)
        addr = (long long unsigned int) map_sysmem(HPS_START_ADDR, 0);
    else if (strcmp(argv[0], "fpga_dl_test\0") == 0)
        addr = (long long unsigned int) map_sysmem(FPGA_START_ADDR_1, 0);

    /* Perform the walk in 0 test for data bus to identify if any of the data bits are struck at HIGH */
    printf("\r Walk in 0 test for data bus\n");
    write_data = 0xFFFFFFFFFFFFFFFE;
    while (write_data < 0xFFFFFFFFFFFFFFFF)
    {
#if DBG 
        printf("\n Writing value %08lx to address 0x%.8llx\n", write_data, addr);
#endif
        *(VULONG *) addr = write_data;
        read_data = *(VULONG *) addr;
#if DBG
        printf("\n Read data %08lx\n", read_data);
#endif
        if (read_data != write_data)
        {
            if (strcmp(argv[0], "dl_test\0") == 0)
                dl_hps_err++;
            else if (strcmp(argv[0], "fpga_dl_test\0") == 0)
                dl_fpga_err++;
        }
        addr += 8;
        write_data = ((write_data << 1) + 1);
    }

    if (strcmp(argv[0], "dl_test\0") == 0) {
        printf("HPS DDR Data line test is done with %ld errors\r\n", dl_hps_err);    
    }
    else if (strcmp(argv[0], "fpga_dl_test\0") == 0) {
        printf("FPGA DDR Data line test is done with %ld errors\r\n", dl_fpga_err);    
    }

    return 0;
}

int do_al_test(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
    vu_long pattern, anti_pattern;
    vu_long *addr;
    vu_long test_offset;
    vu_long offset;
    vu_long temp;
    ulong al_hps_err = 0, al_fpga_err = 0;

    if (strcmp(argv[0], "al_test\0") == 0)
        addr = map_sysmem(HPS_START_ADDR, 0);
    else if (strcmp(argv[0], "fpga_al_test\0") == 0)
        addr = map_sysmem(FPGA_START_ADDR_1, 0);
    
    pattern = (vu_long) 0xaaaaaaaa;
    anti_pattern = (vu_long) 0x55555555;

    /* Write the pattern to the power-of-two offset locations */
    for (offset = 1; offset < 4; offset <<= 1)
        addr[offset] = pattern;

    test_offset = 0;

    /* Write anti-pattern to the below location */
    addr[test_offset] = anti_pattern;

    /* All the address lines are set to 0 as we written pattern to offset 
     * Read the data ie.pattern from the power-of-two offset locations which has one address bit set to high
     * If the pattern is re-written to anti-pattern to any one of the power-of-two offset locations
     * stuck-at high fault is detected at that address bit */
    printf("\r Walk in 1 test for data bus\n");
    for (offset = 1; offset < 4; offset <<= 1) {
        temp = addr[offset];    
        if (temp != pattern) {
            printf("\nFAILURE: Address bit stuck high"
                   "@ 0x%.8lx: expected 0x%.8lx,"
                   " actual 0x%.8lx\n",
                   (ulong)addr + offset * sizeof(vu_long),
                   pattern, temp);
            if (strcmp(argv[0], "al_test\0") == 0)
                al_hps_err++;
            else if (strcmp(argv[0], "fpga_al_test\0") == 0)
                al_fpga_err++;
        }
    }

    addr[test_offset] = pattern;

    /*
     * Check for addr bits stuck low or shorted.
     */
    printf("\r Walk in 0 test for data bus\n");
    for (test_offset = 1; test_offset < 4; test_offset <<= 1) {
        addr[test_offset] = anti_pattern;

        for (offset = 1; offset < 4; offset <<= 1) {
            temp = addr[offset];
            if ((temp != pattern) && (offset != test_offset)) {
                printf("\nFAILURE: Address bit stuck low or"
                       " shorted @ 0x%.8lx: expected 0x%.8lx,"
                       " actual 0x%.8lx\n",
                       (ulong)addr + offset * sizeof(vu_long),
                       pattern, temp);
                if (strcmp(argv[0], "al_test\0") == 0)
                    al_hps_err++;
                else if (strcmp(argv[0], "fpga_al_test\0") == 0)
                    al_fpga_err++;
            }
        }
        addr[test_offset] = pattern;
    }

    if (strcmp(argv[0], "al_test\0") == 0) {
        printf("HPS DDR Address line test is done with %ld errors\r\n", al_hps_err);
    }
    else if (strcmp(argv[0], "fpga_al_test\0") == 0) {
        printf("FPGA DDR Address line test is done with %ld errors\r\n", al_fpga_err);
    }

    return 0;
}

static ulong mem_test_alt(vu_long *buf, ulong start_addr, ulong end_addr,
                          vu_long *dummy)
{
    vu_long *addr;
    ulong errs = 0;
    ulong val, readback;
    int j;
    vu_long offset;
    vu_long test_offset;
    vu_long pattern;
    vu_long temp;
    vu_long anti_pattern;
    vu_long num_words;
    static const ulong bitpattern[] = {
        0x00000001, /* single bit */
        0x00000003, /* two adjacent bits */
        0x00000007, /* three adjacent bits */
        0x0000000F, /* four adjacent bits */
        0x00000005, /* two non-adjacent bits */
        0x00000015, /* three non-adjacent bits */
        0x00000055, /* four non-adjacent bits */
        0xaaaaaaaa, /* alternating 1/0 */
    };

    num_words = (end_addr - start_addr) / sizeof(vu_long);

    /*
     * Data line test: write a pattern to the first
     * location, write the 1's complement to a 'parking'
     * address (changes the state of the data bus so a
     * floating bus doesn't give a false OK), and then
     * read the value back. Note that we read it back
     * into a variable because the next time we read it,
     * it might be right (been there, tough to explain to
     * the quality guys why it prints a failure when the
     * "is" and "should be" are obviously the same in the
     * error message).
     *
     * Rather than exhaustively testing, we test some
     * patterns by shifting '1' bits through a field of
     * '0's and '0' bits through a field of '1's (i.e.
     * pattern and ~pattern).
     */
    addr = buf;
    for (j = 0; j < sizeof(bitpattern) / sizeof(bitpattern[0]); j++) {
        val = bitpattern[j];
        for (; val != 0; val <<= 1) {
            *addr = val;
            *dummy = ~val; /* clear the test data off the bus */
            readback = *addr;
            if (readback != val) {
                printf("FAILURE (data line): "
                       "expected %08lx, actual %08lx\n",
                       val, readback);
                errs++;
                if (ctrlc())
                    return -1;
            }
            *addr = ~val;
            *dummy = val;
            readback = *addr;
            if (readback != ~val) {
                printf("FAILURE (data line): "
                       "Is %08lx, should be %08lx\n",
                       readback, ~val);
                errs++;
                if (ctrlc())
                    return -1;
            }
        }
    }

    /*
     * Based on code whose Original Author and Copyright
     * information follows: Copyright (c) 1998 by Michael
     * Barr. This software is placed into the public
     * domain and may be used for any purpose. However,
     * this notice must not be changed or removed and no
     * warranty is either expressed or implied by its
     * publication or distribution.
     */

    /*
     * Address line test
     *
     * Description: Test the address bus wiring in a
     *              memory region by performing a walking
     *              1's test on the relevant bits of the
     *              address and checking for aliasing.
     *              This test will find single-bit
     *              address failures such as stuck-high,
     *              stuck-low, and shorted pins. The base
     *              address and size of the region are
     *              selected by the caller.
     *
     * Notes: For best results, the selected base
     *              address should have enough LSB 0's to
     *              guarantee single address bit changes.
     *              For example, to test a 64-Kbyte
     *              region, select a base address on a
     *              64-Kbyte boundary. Also, select the
     *              region size as a power-of-two if at
     *              all possible.
     *
     * Returns:     0 if the test succeeds, 1 if the test fails.
     */
    pattern = (vu_long) 0xaaaaaaaa;
    anti_pattern = (vu_long) 0x55555555;

    pr_debug("%s:%d: length = 0x%.8lx\n", __func__, __LINE__, num_words);
    /*
     * Write the default pattern at each of the
     * power-of-two offsets.
     */
    for (offset = 1; offset < num_words; offset <<= 1)
        addr[offset] = pattern;

    /*
     * Check for address bits stuck high.
     */
    test_offset = 0;
    addr[test_offset] = anti_pattern;

    for (offset = 1; offset < num_words; offset <<= 1) {
        temp = addr[offset];
        if (temp != pattern) {
            printf("\nFAILURE: Address bit stuck high @ 0x%.8lx:"
                   " expected 0x%.8lx, actual 0x%.8lx\n",
                   start_addr + offset * sizeof(vu_long),
                   pattern, temp);
            errs++;
            if (ctrlc())
                return -1;
        }
    }
    addr[test_offset] = pattern;
    schedule();

    /*
     * Check for addr bits stuck low or shorted.
     */
    for (test_offset = 1; test_offset < num_words; test_offset <<= 1) {
        addr[test_offset] = anti_pattern;

        for (offset = 1; offset < num_words; offset <<= 1) {
            temp = addr[offset];
            if ((temp != pattern) && (offset != test_offset)) {
                printf("\nFAILURE: Address bit stuck low or"
                       " shorted @ 0x%.8lx: expected 0x%.8lx,"
                       " actual 0x%.8lx\n",
                       start_addr + offset * sizeof(vu_long),
                       pattern, temp);
                errs++;
                if (ctrlc())
                    return -1;
            }
        }
        addr[test_offset] = pattern;
    }

    /*
     * Description: Test the integrity of a physical
     *              memory device by performing an
     *              increment/decrement test over the
     *              entire region. In the process every
     *              storage bit in the device is tested
     *              as a zero and a one. The base address
     *              and the size of the region are
     *              selected by the caller.
     *
     * Returns:     0 if the test succeeds, 1 if the test fails.
     */
    num_words++;

    /*
     * Fill memory with a known pattern.
     */
    for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
        schedule();
        addr[offset] = pattern;
    }

    /*
     * Check each location and invert it for the second pass.
     */
    for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
        schedule();
        temp = addr[offset];
        if (temp != pattern) {
            printf("\nFAILURE (read/write) @ 0x%.8lx:"
                   " expected 0x%.8lx, actual 0x%.8lx)\n",
                   start_addr + offset * sizeof(vu_long),
                   pattern, temp);
            errs++;
            if (ctrlc())
                return -1;
        }

        anti_pattern = ~pattern;
        addr[offset] = anti_pattern;
    }

    /*
     * Check each location for the inverted pattern and zero it.
     */
    for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
        schedule();
        anti_pattern = ~pattern;
        temp = addr[offset];
        if (temp != anti_pattern) {
            printf("\nFAILURE (read/write): @ 0x%.8lx:"
                   " expected 0x%.8lx, actual 0x%.8lx)\n",
                   start_addr + offset * sizeof(vu_long),
                   anti_pattern, temp);
            errs++;
            if (ctrlc())
                return -1;
        }
        addr[offset] = 0;
    }

    return 0;
}

static ulong mem_test_quick(vu_long *buf, ulong start_addr, ulong end_addr,
                            vu_long pattern, int iteration)
{
    vu_long *end;
    vu_long *addr;
    ulong errs = 0;
    ulong incr, length;
    ulong val, readback;

    /* Alternate the pattern */
    incr = 1;
    if (iteration & 1) {
        incr = -incr;
        /*
         * Flip the pattern each time to make lots of zeros and
         * then, the next time, lots of ones. We decrement
         * the "negative" patterns and increment the "positive"
         * patterns to preserve this feature.
         */
        if (pattern & 0x80000000)
            pattern = -pattern; /* complement & increment */
        else
            pattern = ~pattern;
    }
    length = (end_addr - start_addr) / sizeof(ulong);
    end = buf + length;
    printf("\rPattern %08lX  Writing..."
           "%12s"
           "\b\b\b\b\b\b\b\b\b\b",
           pattern, "");
    for (addr = buf, val = pattern; addr < end; addr++) {
        schedule();
        *addr = val;
        val += incr;
    }

    puts("Reading...");

    for (addr = buf, val = pattern; addr < end; addr++) {
        schedule();
        readback = *addr;
        if (readback != val) {
            ulong offset = addr - buf;

            printf("\nMem error @ 0x%08X: "
                   "found %08lX, expected %08lX\n",
                   (uint)(uintptr_t)(start_addr + offset * sizeof(vu_long)),
                   readback, val);
            errs++;
            if (ctrlc())
                return -1;
        }
        val += incr;
    }

    return 0;
}

/*
 * Perform a memory test. A more complete alternative test can be
 * configured using CONFIG_SYS_ALT_MEMTEST. The complete test loops until
 * interrupted by ctrl-c or by a failure of one of the sub-tests.
 */
static int do_ptrn_test(struct cmd_tbl *cmdtp, int flag, int argc,
                        char * const argv[])
{
    ulong start, end;
    vu_long *buf, *dummy;
    int iteration_limit;
    int ret, iteration;
    ulong errs = 0; /* number of errors, or -1 if interrupted */
    ulong pattern;

#if defined(CONFIG_SYS_ALT_MEMTEST)
    const int alt_test = 1;
#else
    const int alt_test = 0;
#endif
    if (argc <= 3) {
        printf("--help\r\nptrn_test start end pattern\r\nEx: ptrn_test 0x81000000 0xF0000000  0xAAAABBBB\r\n");
        return 1;
    }

    start = simple_strtoul(argv[1], NULL, 16);
    end = simple_strtoul(argv[2], NULL, 16);
    pattern = (ulong)simple_strtoul(argv[3], NULL, 16);
    iteration_limit = 1;

    if (start >= HPS_START_ADDR && end <= HPS_END_ADDR)
        printf("Pattern test for HPS DDR memory..");
    else if (start >= FPGA_START_ADDR_1 && end <= FPGA_END_ADDR_1)
        printf("Pattern test for FPGA DDR memory..");
    else if (start >= FPGA_START_ADDR_2 && end <= FPGA_END_ADDR_2)
        printf("Pattern test for FPGA DDR memory..");
    else if (start >= FPGA_START_ADDR_3 && end <= FPGA_END_ADDR_3)
        printf("Pattern test for FPGA DDR memory..");
    else {
        errs = -1UL;
        return -1;
    }    
    
    printf("\nTesting %08lx ... %08lx:\n", (ulong)start, (ulong)end);
    pr_debug("%s:%d: start %#08lx end %#08lx\n", __func__, __LINE__,
             start, end);

    buf = map_sysmem(start, end - start);
    dummy = map_sysmem(SYS_MEMTEST_SCRATCH, sizeof(vu_long));
    for (iteration = 0;
         !iteration_limit || iteration < iteration_limit;
         iteration++) {
        if (ctrlc()) {
            errs = -1UL;
            break;
        }
#if DBG
        printf("Iteration: %6d\r", iteration + 1);
        pr_debug("\n");
#endif
        if (alt_test) {
            errs = mem_test_alt(buf, start, end, dummy);
        } else {
            errs = mem_test_quick(buf, start, end, pattern,
                                  iteration);
        }
        if (errs == -1UL)
            break;
    }

    /*
     * Work-around for eldk-4.2 which gives this warning if we try to
     * case in the unmap_sysmem() call:
     * warning: initialization discards qualifiers from pointer target type
     */
    {
        void *vbuf = (void *)buf;
        void *vdummy = (void *)dummy;

        unmap_sysmem(vbuf);
        unmap_sysmem(vdummy);
    }

    if (errs == -1UL) {
        /* Memory test was aborted - write a newline to finish off */
        putc('\n');
        ret = 1;
    } else {
        if (errs > 0)
            printf("Tested %d iteration(s) with %lu errors.\n",
                   iteration, errs);
        else
            printf("Test is done with %lu errors.\n", errs);
        ret = errs != 0;
    }

    return ret; /* not reached */
}

U_BOOT_CMD(dl_test, 1, 1, do_dl_test, "data line integrity test for HPS DDR", "");

U_BOOT_CMD(fpga_dl_test, 1, 1, do_dl_test, "data line integrity test for FPGA DDR", "");

U_BOOT_CMD(al_test, 1, 1, do_al_test, "Address line integrity test for HPS DDR", "");

U_BOOT_CMD(fpga_al_test, 1, 1, do_al_test, "Address line integrity test for FPGA DDR", "");

U_BOOT_CMD(ptrn_test, 4, 1, do_ptrn_test, "simple RAM pattern test", "start end pattern");
