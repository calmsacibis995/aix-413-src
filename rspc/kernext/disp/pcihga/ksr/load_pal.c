static char sccsid[] = "@(#)38  1.2  src/rspc/kernext/disp/pcihga/ksr/load_pal.c, pcihga, rspc411, GOLD410 4/21/94 17:47:52";
/* load_pal.c */
/*
based on "@(#)41  1.4  src/bos/kernext/disp/wga/ksr/load_pal.c, bos, bos410 10/28/93 17:43:15";
 *
 * COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: load_palette
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "hga_INCLUDES.h"
#include "IO_defs.h"

BUGXDEF (dbg_load_palette);
/*---------------------------------------------------------------------
 *
 * IDENTIFICATION: LOAD_PALETTE
 *
 * DESCRIPTIVE NAME: Load Palette into Adapter
 *
 * FUNCTION:  Takes the default color table      and downloads the
 *              RGB information contained in the table into the adapter
 *              palette area. This code will handle either 16 or 256
 *              colors depending on whether 8Bpp mode is ever
 *              implemented in the KSR code. The number of colors sent
 *              to the adapter will be based on a define KSR_8BPP.
 *              Since HGA    accepts colors in two methods repeating
 *              sequences of packed 24 bit RGB values or a fullword
 *              with RBGX (X being junk), this routine will have to
 *              restructure the default color table to a downloadable
 *              format. A static table sky_palette will be built with
 *              the format RBGX and saved for downloading. The variable
 *              palette_converted will be set to 1 so future conversion
 *              isn't necessary unless the palette has been changed by
 *              the application. (See vttstct).
 *
 * INPUTS:    Virtual terminal pointer
 *
 * OUTPUTS:   Table containing skyway oriented rgb values. Adapter
 *              with palette loaded.
 *
 * CALLED BY: vttact when entering KSR mode
 *
 * CALLS:     None.
 *
 ----------------------------------------------------------------------*/

void
load_palette (struct hga_data * ld)
{
        uchar           u;
        int             index;
        struct hga_ddf *ddf;

        BUGLPR (dbg_load_palette, BUGNTX, ("Entering LOADPAL\n"));

        BUGLPR (dbg_load_palette, BUGNTX, ("accessing PCI bus\n"));

        /* set addressability for attaching the bus */
        ddf = ld->ddf;
        /* attach (memory map) the bus I/O space */
        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

        BUGLPR (dbg_load_palette, BUGNTX, ("Loading color Palette (0..%d)\n",
                                           ld->ksr_color_table.num_entries));

        /* Load Palette */

        PCI_PUTC(DAC_MASK,0XFF);         /* Initialize DAC Addressing    */
        PCI_PUTC(DAC_WRITE_INDEX,0X00);

        /* indirect addr to palette data 0-number of entries in color table */
        /* RAMDAC hardware auto increments the indirect address */

        for (index = 0; index < ld->ksr_color_table.num_entries; index++)
        {
            *DAC_PALETTE = (((ld->ksr_color_table.rgbval[index]) >> 16)); /* R */
            *DAC_PALETTE = (((ld->ksr_color_table.rgbval[index]) >>  8)); /* G */
            *DAC_PALETTE = (((ld->ksr_color_table.rgbval[index])      )); /* B */
        }

        PCI_BUS_DET(ld->IO_base_address);

        BUGLPR (dbg_load_palette, BUGNTX, ("Leaving LOADPAL for real\n"));

}                                      /* end of load_palette */
