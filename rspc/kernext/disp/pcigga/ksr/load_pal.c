static char sccsid[] = "@(#)04  1.2  src/rspc/kernext/disp/pcigga/ksr/load_pal.c, pcigga, rspc41B, 9504A 1/9/95 13:30:49";
/* load_pal.c */
/*
based on "@(#)41  1.4  src/bos/kernext/disp/wga/ksr/load_pal.c, bos, bos410 10/28/93 17:43:15";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
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

#include "gga_INCLUDES.h"

BUGXDEF (dbg_load_palette);
GS_MODULE(gga_load_pal);

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
 *              Since GGA    accepts colors in two methods repeating
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
struct gga_data * ld;
{
        int             i=0;
        struct gga_ddf *ddf;
	ulong	TEMP;

        BUGLPR (dbg_load_palette, BUGNTX, ("Entering LOADPAL\n"));

        BUGLPR (dbg_load_palette, BUGNTX, ("getting ld->bus_mem_addr\n"));

        /* set addressability for attaching the bus */
        ddf = ld->ddf;

	GGA_ENTER_TRC0(ddf,gga_load_pal,2,GGA_LOAD_PALETTE);

        /* attach (memory map) the bus MEM space */
        ld->bus_mem_addr = PCI_MEM_ATT(&ddf->pci_mem_map);

        BUGLPR (dbg_load_palette, BUGNTX, ("Loading color Palette (0..%d)\n",
                                           ld->ksr_color_table.num_entries));

	TEMP = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000200);

        /* Load Palette */
        *WTK_PALCUR_W = 0x0;
        EIEIO;

        /* indirect addr to palette data 0-number of entries in color table */
        /* RAMDAC hardware auto increments the indirect address */
        BUGLPR (dbg_load_palette, BUGNTX, ("Color Index %d = %x\n", i,
                                        ld->ksr_color_table.rgbval[i]));

        for (i = 0; i < ld->ksr_color_table.num_entries; i++)
        {
        BUGLPR (dbg_load_palette, BUGNTX, ("Color Index %d = %x\n", i,
                                        ld->ksr_color_table.rgbval[i]));

                *WTK_DAC_PALETTE = (((ld->ksr_color_table.rgbval[i]) >> 16) & 0x00ffl);
                *WTK_DAC_PALETTE = (((ld->ksr_color_table.rgbval[i]) >> 8) & 0x00ffl);
                *WTK_DAC_PALETTE = (((ld->ksr_color_table.rgbval[i]) ) & 0x00ffl);

        	io_delay(1);        /* hold off for a microsecond */
        }

        PCI_MEM_DET (ld->bus_mem_addr);

        BUGLPR (dbg_load_palette, BUGNTX, ("Leaving LOADPAL for real\n"));

	GGA_EXIT_TRC0(ddf,gga_load_pal,2,GGA_LOAD_PALETTE);

}                                      /* end of load_palette */
