static char sccsid[] = "@(#)33  1.1  src/rspc/kernext/disp/pcihga/ksr/copyps.c, pcihga, rspc411, GOLD410 4/15/94 18:22:40";
/* copyps.c */
/*
based on "@(#)34  1.3  src/bos/kernext/disp/wga/ksr/copyps.c, bos, bos410 10/28/93 17:37:52";
 *
 *   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: copy_ps
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
#include "hga_funct.h"
BUGXDEF (dbg_copyps);

/*---------------------------------------------------------------------
 *
 * IDENTIFICATION: COPY_PS
 *
 * DESCRIPTIVE NAME: Copy contents of the presentation space to screen
 *
 * FUNCTION: Copies the presentation space of the Virtual Display
 * Driver (VDD) into the refresh buffer of the adapter.
 * It also initializes the adapter's cursor shape,
 * and cursor postion.
 *
 * INPUTS:   Virtual terminal pointer
 * Flag indicating whether ok to skip draw of blanks
 *
 * OUTPUTS:   NONE.
 *
 * CALLED BY: vttcpl
 * vttcfl
 * vttinit
 *
 * CALLS:     draw_text
 *
 ----------------------------------------------------------------------*/


void
copy_ps (vp, start_row, end_row, draw_blanks, constant_attributes)
        struct vtmstruc *vp;
        long            start_row,
                        end_row;
        char            draw_blanks;
        char            constant_attributes;
{

        ulong          *buf_addr;      /* presentation space address */
        long            buf_offset;    /* offset into the output buffer */
        struct hga_data *ld;
        BUGLPR (dbg_copyps, BUGNTA, (">copyps\n"));
        ld = (struct hga_data *) vp->vttld;

        /*----------------------------------------------------------------
         * calculate frame buffer and presentation buffer start addresses
         ----------------------------------------------------------------*/
        buf_addr = ld->Vttenv.pse;


        for (; start_row <= end_row; start_row++)
        {
                buf_offset = (start_row - 1) * ld->Vttenv.ps_size.wd;

                /* send the string */
                /*
                 * NOTE:  the (offsets % ps_words) ensures that if the
                 * offsets sum to larger than the presentation space, then
                 * they will "wrap around"  to be within the pres. space
                 * (i.e.  offsets sum in modulo "presentation space")
                 */

                BUGLPR (dbg_copyps, BUGNTX, ("Calling draw text\n"));

                draw_text (vp, (buf_addr + ((buf_offset + ld->Scroll_offset) %
                                            ld->Vttenv.ps_words)),
                      (ushort) ld->Vttenv.ps_size.wd, 1, (ushort) start_row,
                           constant_attributes, draw_blanks);
                BUGLPR (dbg_copyps, BUGNTX, ("Returned from draw text\n"));

        }

        BUGLPR (dbg_copyps, BUGNTA, ("<copyps(0)\n"));
} /* end of copy_ps */
