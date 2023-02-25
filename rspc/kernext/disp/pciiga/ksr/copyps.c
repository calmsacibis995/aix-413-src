static char sccsid[] = "@(#)51	1.3  src/rspc/kernext/disp/pciiga/ksr/copyps.c, pciiga, rspc41B, 9504A 12/20/94 17:18:22";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
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

#include "iga_INCLUDES.h"
#include "iga_funct.h"
BUGXDEF (dbg_copyps);
GS_MODULE(iga_copy_ps);

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
        struct iga_data *ld;
        struct iga_ddf *ddf;

	ddf = (struct iga_ddf *)  vp->display->free_area; 

	IGA_ENTER_TRC(ddf,iga_copy_ps,2,IGA_COPY_PS,start_row,end_row,draw_blanks,0,0);

        BUGLPR (dbg_copyps, BUGNTA, (">copyps\n"));
        ld = (struct iga_data *) vp->vttld;

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

	IGA_EXIT_TRC0(ddf,iga_copy_ps,2,IGA_COPY_PS);

        BUGLPR (dbg_copyps, BUGNTA, ("<copyps(0)\n"));
} /* end of copy_ps */
