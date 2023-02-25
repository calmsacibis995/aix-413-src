static char sccsid[] = "@(#)18  1.3  src/rspc/kernext/disp/pcifga/ksr/load_pal.c, pcifga, rspc411, GOLD410 5/26/94 11:32:04";
/* load_pal.c */
/*
based on "@(#)41  1.4  src/bos/kernext/disp/wga/ksr/load_pal.c, bos, bos410 10/28/93 17:43:15";
 *
 * COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
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

#include "fga_INCLUDES.h"

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
 *              Since FGA    accepts colors in two methods repeating
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
load_palette (ld)
struct fga_data * ld;
{
        int             index;
        struct fga_ddf *ddf;

        BUGLPR (dbg_load_palette, BUGNTX, ("Entering LOADPAL\n"));

        BUGLPR (dbg_load_palette, BUGNTX, ("getting ld->bus_mem_addr\n"));

        /* set addressability for attaching the bus */
        ddf = ld->ddf;
        /* attach (memory map) the bus MEM space */
        ld->bus_mem_addr = PCI_MEM_ATT(&ddf->pci_mem_map);

        BUGLPR (dbg_load_palette, BUGNTX, ("Loading color Palette (0..%d)\n",
                                           ld->ksr_color_table.num_entries));

        /* Load Palette */
        *BT485_RAMDAC_WR_ADDR = 0x0;
        EIEIO;

        /* indirect addr to palette data 0-number of entries in color table */
        /* RAMDAC hardware auto increments the indirect address */

        for (index = 0; index < ld->ksr_color_table.num_entries; index++)
        {
        BUGLPR (dbg_load_palette, BUGNTX, ("Color Index %d = %x\n", index,
                                        ld->ksr_color_table.rgbval[index]));

                *BT485_DAC_PALETTE = (((ld->ksr_color_table.rgbval[index]) >> 16) & 0x00ffl);
                *BT485_DAC_PALETTE = (((ld->ksr_color_table.rgbval[index]) >> 8) & 0x00ffl);
                *BT485_DAC_PALETTE = (((ld->ksr_color_table.rgbval[index]) ) & 0x00ffl);

        	io_delay(1);        /* hold off for a microsecond */
        }

        PCI_MEM_DET (ld->bus_mem_addr);

        BUGLPR (dbg_load_palette, BUGNTX, ("Leaving LOADPAL for real\n"));

}                                      /* end of load_palette */
