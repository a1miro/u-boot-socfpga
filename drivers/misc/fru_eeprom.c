#include <errno.h>
#include <command.h>
#include <i2c.h>
#include <iG_fru.h>
#include <asm/byteorder.h>
#include <stdlib.h>
#include <ctype.h> 
#include <vsprintf.h>

#define DUMP_BOARD     (0x01)
#define DUMP_SUPPLY    (0x02)
#define DUMP_CONNECTOR (0x04)
#define DUMP_I2C       (0x08)

unsigned char fmc_eeprom_buf[1024];
static uchar i2c_fmc_last_bus = 1;
static uint i2c_fmc_last_chip = 0x50;

struct fmc_voltage {
	int nominal_volt;
	int minimum_volt;
	int maximum_volt;
};

void * x_calloc (size_t nmemb, size_t size)
{
        unsigned int *ptr;

        ptr = calloc(nmemb, size);
        if (ptr == NULL)
                printf_err("memory error - calloc returned zero\n");

        return (void *)ptr;
}

/*
 * FRU information uses a zero checksum everywhere.
 * The modulo 256 sum of the preceding bytes (starting with the first byte
 * of the header) plus the checksum byte equals zero.
 * Platform Management FRU Information Storage Definition : section 16.2.[56]
 */
unsigned char calc_zero_checksum (unsigned char *data, size_t len)
{
        size_t i;
        unsigned char tmp = 0;

        for (i = 0; i <= len; i++)
                tmp += data[i];

        return tmp;
}

#ifdef DEBUG
/*
 * Used for debugging
 */
static void dump_str(unsigned char * p, unsigned int size, unsigned int space)
{
        size_t i, j = 0, k, m, shift;
        unsigned char *t, this = 0, last = 0;

        t = p;
        k = 8 - space;
        for (i = 0; i < size; i++) {
                m = 0;
                printf("%02zi: %02x : ", i, *t);
                last = this;
                this = *t;
                for (shift = 0x80; shift > 0; shift >>= 1) {
                        printf("%s", ((*t & shift) == shift) ? "1" : "0");
                        j++, m++;
                        if(k == m) {
                                printf(" ");
                                k = 8 - space + m;
                                if (k >= 8)
                                        k -= space;
                                m = space + 100;
                        }
                }
                if (space == 8) {
                        if (*t)
                                printf(" (%c) %02x", *t, *t - 0x20);
                        else
                                printf(" (term) NULL");
                }
                if (space == 6) {
                        unsigned char x = 0, y = 0;

                        if (k == 2) {
                                y = (this >> 2) & 0x3F;
                                x = (last >> 4) | ((this & 0x3) << 4);
                        } else if (k == 4) {
                                x = this & 0x3F;
                        } else if (k == 6) {
                                x = (last >> 6) | ((this & 0xF) << 2);
                        }

                        if (k == 4 || k == 6)
                                printf(" (%02x) %02x '%c'", x , x + 0x20, x + 0x20);
                        if (k == 2)
                                printf(" (%02x) %02x '%c' | (%02x) %02x '%c'", x , x + 0x20, x + 0x20, y , y + 0x20, y + 0x20);
                }
                ++t;
                printf("\n");
        }
}
#else
static void dump_str(unsigned char * UNUSED(p), unsigned int UNUSED(size), unsigned int UNUSED(space))
{
        return;
}
#endif

/*
 * 6-bit ASCII Packing
 * Platform Management FRU Information Storage Definition:  Section 13.[23]
 */
int ascii2six(unsigned char **dest, unsigned char *src, size_t size)
{
        size_t i = 0;
        ssize_t j;
        unsigned int k, m = 0;
        unsigned char *p, *d, *e;

        if (!src || !size)
                return 0;

        e = d = x_calloc(1, size);

        p = src;
        /*
         * 6-bit requires uppercase chars, between 0 and 0x3f
         */
        while (*p != '\0' && i <= size) {
                j = toupper(*p) - 0x20;
                if (j < 0 || j >= 0x40) {
                        printf_warn("%s : trying to convert a string '%s'\n"
                                        "\t\twhich includes char '%c (0x%x)', which can't be converted\n",
                                        __func__, src, *p, *p);
                        free(e);
                        return -1;
                }
                *d = j;
                ++p, ++d, i++;
        }
        /* dump_str(d, size, 4); */

        /* the length of dest, should be 3/4 of size, it's zero padded at the end */
        *dest = x_calloc(1, size + 1);
        p = *dest;

        for (i = 0; i <= size ; i+= 4) {
                k = e[i];
                m++;
                if ((i + 1) < size) {
                        k |= e[i + 1] << 6;
                        m++;
                }
                if ((i + 2) < size) {
                        k |= e[i + 2] << 12;
                       m++;
                }
                if ((i + 3) < size) {
                        k |= e[i + 3] << 18;
                }
#ifndef __MINGW32__
                k = cpu_to_le32(k);
#endif
                memcpy(p, &k, 3);
                p += 3;
        }

        /* dump_str(*dest, m, 6); */
        free (e);

        return  m;
}

/*
 *  * 6-bit ASCII Unpacking
 *   * Platform Management FRU Information Storage Definition:  Section 13.[23]
 */
unsigned char * six2ascii(unsigned char *buf, size_t size)
{
        unsigned char *p, *dest;
        size_t i;

        if (!size)
                return NULL;

        dump_str(buf, size, 6);
        /* the length of dest, should be 4/3 of size + 1 for null termination char*/
        dest = x_calloc(1, ((size * 4) / 3) + 2);
        p = dest;

        for (i = 0; i < size; i += 3) {
                *dest = (buf[i] & 0x3F) + 0x20;
                dest++;
                if ((i + 1) < size) {
                        *dest = ((buf[i] & 0xC0) >> 6 | (buf[i+1] & 0x0F) << 2) + 0x20;
                        dest++;
                }
                if ((i + 2) < size) {
                        *dest = ((buf[i+1] & 0xF0) >> 4 | (buf[i+2] & 0x03) << 4) + 0x20;
                        dest++;
                }
                if ((i + 3) < size) {
                        *dest = ((buf[i+2] & 0xFC) >> 2) + 0x20;
                        dest++;
                }
        }
        /* make sure strings are null terminated */
        *dest = 0;

        /* Drop trailing spaces & null chars */
        dest--;
        while ((*dest == 0 || *dest == ' ') && size) {
                *dest = 0;
                dest--;
                size--;
        }

        return p;
}


/*
 * Extract strings from fields
 * Section 13 TYPE/LENGTH BYTE FORMAT
 * Platform Management FRU Information Storage Definition
 */
unsigned int parse_string(unsigned char *p, unsigned char **str, const char * field)
{
        size_t len, i, j;

        len = p[0] & 0x3F;

        if (!len) {
                *str = x_calloc(1, 3);
                *str[0] = (FRU_STRING_ASCII << 6);
                return 1;
        }

        switch((p[0] >> 6) & 0x3 ) {
                case FRU_STRING_BINARY:
                        /* binary or unspecified */
                        *str = x_calloc(1, len + 2);
                        memcpy(*str, p, len + 1);
                        break;
                case FRU_STRING_BCD:
                        /* BCD plus */
                        printf_err("BCD - sorry\n");
                        break;
                case FRU_STRING_SIXBIT:
                        /* 6-bit ASCII, packed */
                        {
                                unsigned char *tmp1, *tmp2;
                                size_t tlen;

                                tmp1 = six2ascii(&p[1], p[0]& 0x3F);
                                tlen = strlen((char *)tmp1);
                                *str = x_calloc(1, tlen + 2);

                                tmp2 = *str;
                                tmp2++;
                                memcpy(tmp2, tmp1, tlen + 1);
                                if (tlen > 0x3F)
                                        tlen = 0x3F;

                                *str[0] = (FRU_STRING_ASCII << 6) | tlen;
                                free(tmp1);
                        }
                        break;
                case FRU_STRING_ASCII:
                        /* 8-bit ASCII */
                        *str = x_calloc(1, len + 2);
                        memcpy(*str, p, len + 1);
                        for (i = 1; i < len; i++) {
                                if (p[i] < 0x20 || p[i] == 0x7F) {
                                        printf_warn("Field '%s' marked as ASCII, but contains non-printable "
                                                        "characters:\n", field);
                                        printf_warn("  Length/Type : 0x%x (length:%i; type:ASCII(%i)\n",
                                                        p[0], p[0] & 0x3F, FRU_STRING_ASCII);
                                        printf_warn("  Contents : ");
                                        for (j = 1; j < len + 1; j++)
                                                printf_warn("0x%02x ", p[j]);
                                        printf_warn(" |");
                                        for (j = 0; j < len + 1; j++)
                                                printf_warn("%c", ((p[j] < 32) || (p[j] >= 127)) ? '.': p[j]);
                                        printf_warn("|\n");
                                }
                        }
                        break;
        }
        return len + 1;
}



void printf_err (const char * fmt, ...)
{
        va_list ap;
        va_start(ap,fmt);
        vprintf(fmt,ap);
        va_end(ap);
}

void printf_warn (const char * fmt, ...)
{
        va_list ap;

        va_start(ap,fmt);
        vprintf(fmt,ap);
        va_end(ap);
}

void printf_info (const char * fmt, ...)
{
        va_list ap;

        va_start(ap,fmt);
        vprintf(fmt,ap);
        va_end(ap);
}



static void dump_fru_field(const char * description, size_t offset, unsigned char * field)
{
        /* does field exist, and have length? */
        if (field) {
                printf("%s\t: ", description);
                if (FIELD_LEN(field)) {
                        if (TYPE_CODE(field) == FRU_STRING_ASCII || offset) {
                                printf("%s\n", &field[offset + 1]);
                        } else {
                                printf("Non-ASCII\n");
                        }
                } else
                        printf("Empty Field\n");
        }
}

void dump_BOARD(struct BOARD_INFO *fru)
{
        unsigned int i, j;

        dump_fru_field("Manufacturer", 0, fru->manufacturer);
        dump_fru_field("Product Name", 0, fru->product_name);
        dump_fru_field("Serial Number", 0, fru->serial_number);
        dump_fru_field("Part Number", 0, fru->part_number);
        dump_fru_field("FRU File ID", 0, fru->FRU_file_ID);

        if (!strncasecmp((const char *)&fru->manufacturer[1], "Analog Devices", strlen("Analog Devices"))) {
                for (i = 0; i < CUSTOM_FIELDS; i++) {
                        /* These are ADI custom fields */
                        if (fru->custom[i] && fru->custom[i][0] & 0x3F) {
                                switch (fru->custom[i][1]) {
                                        case 0:
                                                dump_fru_field("PCB Rev ", 1, fru->custom[i]);
                                                break;
                                        case 1:
                                                dump_fru_field("PCB ID  ", 1, fru->custom[i]);
                                                break;
                                        case 2:
                                                dump_fru_field("BOM Rev ", 1, fru->custom[i]);
                                                break;
                                        case 3:
                                                dump_fru_field("Uses LVDS", 1, fru->custom[i]);
                                                break;
                                        case 4:
                                                dump_fru_field("Tuning  ", 1, fru->custom[i]);
                                                break;
                                        default:
                                                dump_fru_field("Unknown ", 1, fru->custom[i]);
                                                break;
                                }
                        }
                }
        } else {
                printf("Custom Fields:\n");
                for (i = 0; i < CUSTOM_FIELDS; i++) {
                        if (fru->custom[i] && fru->custom[i][0] & 0x3F) {
                                printf("  Field %i (len=%i):", i, fru->custom[i][0] & 0x3F);
                                for (j = 1 ; j <= (fru->custom[i][0] & 0x3F); j++)
                                        printf(" %02x", fru->custom[i][j] & 0xFF);
                                printf("  |");
                                for (j = 1 ; j <= (fru->custom[i][0] & 0x3F); j++)
                                        printf("%c", ((fru->custom[i][j] < 32) || (fru->custom[i][j] >= 127)) ? '.': fru->custom[i][j]);
                                printf("|\n");
                        }
                }
        }
}

/*
 * DC Load and DC Output Multi-record Definitions
 * Table 8 from the VITA/ANSI 57.1 Spec
 */
const char * DC_Loads[] = {
        "P1 VADJ",                      /* Load   :  0 */
        "P1 3P3V",                      /* Load   :  1 */
        "P1 12P0V",                     /* Load   :  2 */
        "P1 VIO_B_M2C",                 /* Output :  3 */
        "P1 VREF_A_M2C",                /* Output :  4 */
        "P1 VREF_B_M2C",                /* Output :  5 */
        "P2 VADJ",                      /* Load   :  6 */
        "P2 3P3V",                      /* Load   :  7 */
        "P2 12P0V",                     /* Load   :  8 */
        "P2 VIO_B_M2C",                 /* Load   :  9 */
        "P2 VREF_A_M2C",                /* Load   : 10 */
        "P2 VREF_B_M2C",                /* Load   : 11 */
};

void dump_MULTIRECORD (struct MULTIRECORD_INFO *fru)
{
        unsigned char *p, *n, *z;
        int i;

        z = x_calloc(1, 12);

        for (i= 0; i <= NUM_SUPPLIES - 1; i++) {
                if (!fru->supplies[i])
                        continue;
                p = fru->supplies[i];
                n = p + 5;
                switch(p[0]) {
                        case 1:
                                printf("DC Output\n");
                                printf("  Output Number: %d (%s)\n", n[0] & 0xF, DC_Loads[n[0] & 0xF]);
                                if (memcmp(&n[1], z, 11)) {
                                        printf("  Nominal volts:              %d (mV)\n", (n[ 1] | (n[ 2] << 8)) * 10);
                                        printf("  Maximum negative deviation: %d (mV)\n", (n[ 3] | (n[ 4] << 8)) * 10);
                                        printf("  Maximum positive deviation: %d (mV)\n", (n[ 5] | (n[ 6] << 8)) * 10);
                                        printf("  Ripple and Noise pk-pk:     %d (mV)\n",  n[ 7] | (n[ 8] << 8));
                                        printf("  Minimum current draw:       %d (mA)\n",  n[ 9] | (n[10] << 8));
                                        printf("  Maximum current draw:       %d (mA)\n",  n[11] | (n[12] << 8));
                                } else
                                        printf("  All Zeros\n");
                                break;
                        case 2:
                                printf("DC Load\n");
                                printf("  Output number: %d (%s)\n", n[0] & 0xF, DC_Loads[n[0] & 0xF]);
                                printf("  Nominal Volts:         %04d (mV)\n", (n[ 1] | (n[ 2] << 8)) * 10);
                                printf("  minimum voltage:       %04d (mV)\n", (n[ 3] | (n[ 4] << 8)) * 10);
                                printf("  maximum voltage:       %04d (mV)\n", (n[ 5] | (n[ 6] << 8)) * 10);
                                printf("  Ripple and Noise pk-pk %04d (mV)\n",  n[ 7] | (n[ 8] << 8));
                                printf("  Minimum current load   %04d (mA)\n",  n[ 9] | (n[10] << 8));
                                printf("  Maximum current load   %04d (mA)\n",  n[11] | (n[12] << 8));
                                break;
                }
        }
        free (z);

}

void dump_FMConnector (struct MULTIRECORD_INFO *fru)
{

        unsigned char *p, *n;

        if (!fru->connector) {
                printf("No Connector information\n");
                return;
        }

        p = fru->connector;
        n = p + 5;

        n += 3;
        switch (n[1]>>6) {
                case 0:
                        printf("Single Width Card\n");
                        break;
                case 1:
                        printf("Double Width Card\n");
                        break;
                default:
                        printf("error - not the right size\n");
                        break;
        }
        switch ((n[1] >> 4) & 0x3) {
                case 0:
                        printf("P1 is LPC\n");
                        break;
                case 1:
                        printf("P1 is HPC\n");
                        break;
                default:
                        printf("P1 not legal size\n");
                        break;
        }
        switch ((n[1] >> 2) & 0x3) {
                case 0:
                        printf("P2 is LPC\n");
                        break;
                case 1:
                        printf("P2 is HPC\n");
                        break;
                case 3:
                        if (n[1]>>6 != 0)
                                printf("P2 is not populated\n");
                        break;
                default:
                        printf("P2 not legal size\n");
                        break;
        }
        printf("P1 Bank A Signals needed %d\n", n[2]);
        printf("P1 Bank B Signals needed %d\n", n[3]);
        printf("P1 GBT Transceivers needed %d\n", n[6] >> 4);
        if (((n[1] >> 2) & 0x3) != 3) {
                printf("P2 Bank A Signals needed %d\n", n[4]);
                printf("P2 Bank B Signals needed %d\n", n[5]);
                printf("P2 GBT Transceivers needed %d\n", n[6] & 0xF);
        }
        printf("Max JTAG Clock %d\n", n[7]);

}

/*
 * Read in the file of the disk, or from the EEPROM
 */
int read_fmc_eeprom(int i2c_bus, int chip_addr)
{
        size_t tmp;
        int i = 0, ret;
	struct udevice *dev;


        ret = i2c_get_chip_for_busnum(i2c_bus, chip_addr, 1, &dev);
        if(ret) {
                return -1;
        }

	tmp = dm_i2c_read (dev, 0, fmc_eeprom_buf, 1024); 

        /*
         * If an error  occurs,  or the  end-of-file is reached,
         * the return value is a short item count (or zero).
         */
        if (tmp == 0)
                for (i = 1023; fmc_eeprom_buf[i] == 0; i--);
        return i+1;
}

struct BOARD_INFO * parse_board_area(unsigned char *data)
{

	 struct BOARD_INFO *fru;
        unsigned char *p;
        unsigned int len, i, j;

	fru = x_calloc(1, sizeof(struct BOARD_INFO));

        if (data[0] != 0x01) {
                printf_err("Board Area Format Version mismatch: 0x%02x should be 0x01\n", data [0]);
                goto err;
        }

        len = (data[1] * 8) - 1;
        if (calc_zero_checksum(data, len)) {
                printf_err("Board Area Checksum failed\n");
                goto err;
        }

        if (data[2] != 0 && data[2] != 25) {
                printf_err("Board Area is non-English - sorry: Lang code = %i\n", data[2]);
                goto err;
        }

        len--;
        while ((data[len] == 0x00) && (len != 0))
                len--;
        if (len == 0 || data[len] != 0xC1) {
                printf_err("BOARD INFO not terminated properly, walking backwards len: "
                                "%i:0x%02x should be 0xC1\n", len, data[len]);
                goto err;
        }

        fru->mfg_date = data[3] | (data[4] << 8) | (data[5] << 16);

        p = &data[6];
        len -= 6;

        i = parse_string(p, &fru->manufacturer, "Manufacturer");
        p += i, len -= i;

        i = parse_string(p, &fru->product_name, "Product Name");
        p += i, len -= i;

        i = parse_string(p, &fru->serial_number, "Serial Number");
        p += i, len -= i;

        i = parse_string(p, &fru->part_number, "Part Number");
        p += i, len -= i;

        i = parse_string(p, &fru->FRU_file_ID, "FRU File ID");
        p += i, len -= i;

        j = 0;
        while (len != 0 && j < CUSTOM_FIELDS) {
                i = parse_string(p, &fru->custom[j], "Custom Field");
                p += i, len -= i, j++;
        }

        if (*p != 0xC1) {
                printf_err("BOARD INFO not terminated properly, "
                                "offset %02i(0x%02x) : %02i(0x%02x) should be 0xC1\n",
                                p - data, p - data, *p, *p);
                goto err;
        }

        return fru;

err:
        free(fru->manufacturer);
        free(fru->product_name);
        free(fru->serial_number);
        free(fru->part_number);
        free(fru->FRU_file_ID);
        for( j = 0; j < CUSTOM_FIELDS; j++)
                free(fru->custom[j]);
        free(fru);
        return NULL;
}

/*
 * Each record in this area begins with a pre-defined header as specified in the
 * section 16 in the Platform Management FRU Information Storage Definition.
 * This header contains a Type field that identifies what information is
 * contained in the record.  * There are some FMC specific headers, defined
 * in section 5.5.1 of the FMC specification "IPMI Support". These FMC specific
 * sections have a 1 byte sub-type, and a 3 byte Unique Organization Identifier
 */
struct MULTIRECORD_INFO * parse_multiboard_area(unsigned char *data)
{
        int i = 0, tmp, type;
        unsigned char *p;
	struct MULTIRECORD_INFO *multi;

        multi = x_calloc(1, sizeof(struct MULTIRECORD_INFO));

        p = data;

        do {
                if (i != 0){
                        p += 5 + p[2];
                }
                if (p[0] >= 0x06 && p[0] <= 0xBF) {
                        printf_err("MultiRecord Area %i: Invalid Record Header\n", i);
                        return NULL;
                }
                if (calc_zero_checksum(p, 4)) {
                        printf_err("MultiRecord Area %i (Record Type 0x%x): "
                                        "Header Checksum failed\n", i, p[0]);
                        return NULL;
                }

                if (!p[2] || ((calc_zero_checksum(p+5, p[2] - 1) + p[3]) & 0xFF)) {
                        printf_err("MultiRecord Area %i (Record Type 0x%x): "
                                        "Record Checksum failed\n", i, p[0]);
                        return NULL;
                }

                /*
                 * Record Type ID
                 */
                switch(p[0]) {
                        case MULTIRECORD_DC_OUTPUT:
                        case MULTIRECORD_DC_INPUT:
                                tmp = p[5] & 0xF;
                                if ((tmp) >= NUM_SUPPLIES)
                                        printf_err("Too many Supplies defined in Multirecords\n");

                                multi->supplies[tmp] = x_calloc(1, p[2] + 6);
                                memcpy (multi->supplies[tmp], p, p[2] + 6);
                                multi->supplies[tmp][1] = multi->supplies[tmp][1] & 0x7F;
                                break;
                        case MULTIRECORD_FMC:
                                /*
                                 * Use VITA's OUI: 0x0012a2 is specified in the FMC spec - Rule 5.77
                                 */
                                if ((p[5] | p[6] << 8 | p[7] << 16) != VITA_OUI) {
                                        printf_err("OUI Doesn't match : is 0x%06X, "
                                                "should be 0x%06x\n", p[5] | p[6] << 8 | p[7] << 16, VITA_OUI);
                                }
                                /* type field is located: Header + Manufacturer ID = 5 + 3 */
                                type = p[8] >> 4;

                                switch (type) {
                                        case MULTIRECORD_CONNECTOR:
                                                /* see table 7 in FMC spec */
                                                multi->connector = x_calloc(1, p[2] + 6);
                                                memcpy (multi->connector, p, p[2] + 6);
                                                /* This isn't the end 'til we re-assemble things */
                                                multi->connector[1] = multi->connector[1] & 0x7F;
                                                break;
                                        case MULTIRECORD_I2C:
                                                if (p[2] <= 5) {
                                                        printf_warn("I2C MultiRecord is too short (len:%i)\n"
                                                                "       (at least 4 is needed for OUI and subtype)\n",
                                                                p[2]);
                                                } else {
                                                        /* see table 9 in FMC spec */
                                                        unsigned char *foo2;
                                                        foo2 = six2ascii(&p[9], p[2] - 4);

                                                        multi->i2c_devices = x_calloc(1, strlen((char *)foo2) + 1);
                                                        strcpy ((char *)multi->i2c_devices, (char *)foo2);
                                                        free(foo2);
                                                        /* This isn't the end 'til we re-assemble things */
                                                        multi->i2c_devices[1] = multi->i2c_devices[1] & 0x7F;
                                                }
                                                break;
                                        default:
                                                printf_err("Unknown multirecord type : %i\n", type);
                                                break;
                                }

                                if (type == 1) {

                                }
                                break;
                        default:
                                printf_err("Unknown MultiRecord Area\n");
                }

                i++;
        } while (!(p[1] & 0x80));

        return multi;
}

/*
 * Common Header Format
 * Section 8 in the Platform Management FRU Information Storage Definition
 */
struct FRU_DATA * parse_FRU (unsigned char *data)
{
	struct FRU_DATA *fru;

        fru = x_calloc (1, sizeof(struct FRU_DATA));

        /* Check FRU version */
        if (data[0] != 0x01) {
                printf_err("FRU Version number mismatch 0x%02x should be 0x01\n", data[0]);
                goto err;
        }

        /* Check Padding */
        if (data[6] != 0x00) {
                printf_err("FRU byte 6 should be PAD, and be zero -- but it's not\n");
                goto err;
        }

        /* Check header checksum */
        if (calc_zero_checksum(data, 7)) {
                printf_err("Common Header Checksum failed\n");
                goto err;
        }

        /* Parse Internal Use Area */
        if (data[1]) {
                printf_err("Internal Use Area not yet implemented - sorry\n");
                goto err;
        }

        /* Parse Chassis Info Area */
        if (data[2]) {
                printf_err("Chassis Info Area not yet implmented - sorry\n");
                goto err;
        }

	/* Parse Board Area */
	if (data[3]) {
                fru->Board_Area = parse_board_area(&data[data[3] * 8]);
                if (!fru->Board_Area)
                        goto err;
	}
        /* Parse Chassis Info Area */
        if (data[4]) {
                printf_err("Chassis Info Area parsing not yet implemented - sorry\n");
                goto err;
        }

        /* Parse MultiRecord Area */
        if (data[5]){
		fru->MultiRecord_Area = parse_multiboard_area(&data[data[5] * 8]);
        }

        return fru;

err:
        free(fru);
        return NULL;

}

void free_FRU(struct FRU_DATA *fru)
{
        int j;

        free(fru->Board_Area->manufacturer);
        free(fru->Board_Area->product_name);
        free(fru->Board_Area->serial_number);
        free(fru->Board_Area->part_number);
        free(fru->Board_Area->FRU_file_ID);
        for(j = 0; j < CUSTOM_FIELDS; j++)
                free(fru->Board_Area->custom[j]);
        free(fru->Board_Area);

        for(j = 0; j < NUM_SUPPLIES; j++)
                free(fru->MultiRecord_Area->supplies[j]);
        free(fru->MultiRecord_Area->i2c_devices);

        free(fru->MultiRecord_Area->connector);
        free(fru->MultiRecord_Area);

        free(fru);

}

void parse_fmc_volt (struct MULTIRECORD_INFO *fru, struct fmc_voltage *fmc_volt)
{
	unsigned char *p, *n, *z;
	int i;

	z = x_calloc(1, 12);

	for (i= 0; i <= NUM_SUPPLIES - 1; i++) {
		if (!fru->supplies[i])
			continue;
		p = fru->supplies[i];
		n = p + 5;
		if (p[0] == 2 ){
			if ((n[0] & 0xF) == 0){
				fmc_volt->nominal_volt = (n[ 1] | (n[ 2] << 8)) * 10;
				fmc_volt->minimum_volt = (n[ 3] | (n[ 4] << 8)) * 10;
				fmc_volt->maximum_volt = (n[ 5] | (n[ 6] << 8)) * 10;
			}
		}
	}
	free (z);

}


int fmc_vadj_support (int i2c_bus, int i2c_address, u32 vadj_volt)
{
	struct FRU_DATA *fru = NULL;
	struct fmc_voltage fmc_volt;

	if (!read_fmc_eeprom(i2c_bus, i2c_address)){
		printf (" FMC EEPROM Read Failed...\n");
		return -1;
	}

	fru = parse_FRU(fmc_eeprom_buf);

	if (fru) {
		parse_fmc_volt (fru->MultiRecord_Area, &fmc_volt);
		free_FRU(fru);

		if ((fmc_volt.minimum_volt < vadj_volt) && (fmc_volt.maximum_volt > vadj_volt)){
			return 1;
		} 
	}
	return 0;
}

/**
 * do_fru_dump() - Handle the "frudump" command-line command
 * @cmdtp:      Command data struct pointer
 * @flag:       Command flag
 * @argc:       Command-line argument count
 * @argv:       Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_fru_dump(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct FRU_DATA *fru = NULL;
	u_char  bus;
	uint    chip;

	/* We use the last specified parameters, unless new ones are
	 * entered.
	 */
	bus   = i2c_fmc_last_bus;
	chip   = i2c_fmc_last_chip;

	if (argc != 3)
		return CMD_RET_USAGE;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/*
		 * New command specified.
		 */

		/*
		 * I2C Bus Number
		 */
		bus = simple_strtoul(argv[1], NULL, 16);

		/*
		 * I2C Chip address 
		 */
		chip = simple_strtoul(argv[2], NULL, 16);

	}

	if (!read_fmc_eeprom(bus, chip)){
		printf (" FMC EEPROM Read Failed...\n");
		return -1;
	}

	fru = parse_FRU(fmc_eeprom_buf);

	if (fru) {

		dump_BOARD(fru->Board_Area);

		dump_MULTIRECORD(fru->MultiRecord_Area);

		dump_FMConnector(fru->MultiRecord_Area);

		free_FRU(fru);
	}

	i2c_fmc_last_bus  = bus;
	i2c_fmc_last_chip = chip;

	return 0;
}

U_BOOT_CMD(frudump, 3, 1, do_fru_dump, "Dump FMC Card Details", "fru-dump <i2c Bus No> <Chip Address>");
