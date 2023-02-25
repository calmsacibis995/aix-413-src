static char sccsid[] = "@(#)09  1.3  src/rspc/kernext/disp/pcigga/ksr/vtt_cfl.c, pcigga, rspc41J, 9513A_all 3/22/95 11:14:32";
/* vtt_cfl.c */
/*
based on "@(#)48  1.3  src/bos/kernext/disp/wga/ksr/vtt_cfl.c, bos, bos410 10/28/93 17:47:00";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: vttcfl
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
BUGXDEF (dbg_vttcfl);
GS_MODULE(gga_vttcfl);

/*------------------------------------------------------------------
 *
 * IDENTIFICATION: VTTCFL
 *
 * DESCRIPTIVE name:  Copy full lines of text.
 *
 * FUNCTION: Copies a sequence of one or more full lines
 *             up or down a specified number of lines (clipping
 *             at the presentation space boundaries). In addition,
 *             the cursor can optionally be moved.
 *
 * INPUTS:    Virtual terminal pointer
 *              Source , Destination Row
 *              Number of rows to move
 *              Cursor show flag
 *
 * OUTPUTS:
 *
 * CALLED BY: Mode Processor
 *
 * CALLS:     draw_text
 *            vttmovc
 *
 -------------------------------------------------------------------*/
#define NEWSCAN 1

long
vttcfl (vp, s_row, d_row, num_rows, cursor_show)
        struct vtmstruc *vp;
        long            s_row,         /* source row                 */
                        d_row,         /* destination row            */
                        num_rows;      /* number of rows to copy     */
        ulong           cursor_show;   /* if true cursor is moved to */
                                        /* the position given by *cp  */

{

        ulong          *buf_addr;      /* address of the buffer           */
        long            s_offset,      /* starting offset of the source
                                        * string */
                        d_offset,      /* starting offset of the target
                                        * string */
                        num_words,     /* number of full words to be moved  */
                        factor;        /* move direction indicator          */
        long            buf_offset,
                        start,
                        stop;
        struct gga_data *ld;
        long            src_row,
                        dest_row;
        int             i;
	struct gga_ddf  *ddf ;

/*
 * The following is a debug print block intended to display all input parameters
 */

BUGLPR(dbg_vttcfl,BUGGID,("\n"));
BUGLPR(dbg_vttcfl,BUGGID,("Input parameters follow:\n"));

BUGLPR(dbg_vttcfl,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttcfl,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));

BUGLPR(dbg_vttcfl,BUGGID,("s_row             = %d\n",s_row));

BUGLPR(dbg_vttcfl,BUGGID,("d_row             = %d\n",d_row));

BUGLPR(dbg_vttcfl,BUGGID,("num_rows          = %d\n",num_rows));

BUGLPR(dbg_vttcfl,BUGGID,("cursor_show       = %d\n",cursor_show));

	ddf = (struct gga_ddf *) vp->display->free_area;

	GGA_ENTER_TRC0(ddf,gga_vttcfl,2,GGA_VTTCFL);

        s_row--;
        d_row--;                       /* adjust the row numbers based from 0 */

        ld = (struct gga_data *) vp->vttld;

        VDD_TRACE (CFL, ENTRY, vp);

        if (ld->Vttenv.vtt_mode != KSR_MODE)    /* not in character mode */
        {
		GGA_EXIT_TRC1(ddf,gga_vttcfl,2,GGA_VTTCFL,EIO);
                return (EIO);
        }

        /*-----------------------------------------------------------
        | the source address does not equal the destination address
        |----------------------------------------------------------*/
        if (s_row != d_row)
        {
                /*-----------------------------------------------
                | copy the indicated line segments as requested
                |----------------------------------------------*/

                /*--------------------------------
                | update the presentation space
                |-------------------------------*/

                buf_addr = ld->Vttenv.pse;

                /*-------------------------------------------------------
                 | calculate the starting offset of the first line to be
                 | moved in the source and and destination strings
                 |------------------------------------------------------*/

                s_offset = s_row * ld->Vttenv.ps_size.wd;
                d_offset = d_row * ld->Vttenv.ps_size.wd;

                /*------------------------------------------------------------
                | if the destination is above the source, copy top to bottom
                |-----------------------------------------------------------*/
                if (s_row > d_row)
                {
                        BUGLPR (dbg_vttcfl, BUGACT, ("CFL,copy top2bot\n"));
                        factor = 1;
                        if (num_rows > ld->Vttenv.ps_size.ht - s_row)
                        {
                                num_rows = ld->Vttenv.ps_size.ht - s_row;
                        }
                        num_words = num_rows * ld->Vttenv.ps_size.wd;
                }
                /*-------------------------------------------------------------------
                | else copy bottom to top avoiding source and dest overlap problems
                |------------------------------------------------------------------*/
                else
                {
                        BUGLPR (dbg_vttcfl, BUGACT, ("CFL,copy bot2top\n"));
                        factor = -1;
                        if (num_rows > ld->Vttenv.ps_size.ht - d_row)
                        {
                                num_rows = ld->Vttenv.ps_size.ht - d_row;
                        }
                        num_words = num_rows * ld->Vttenv.ps_size.wd;
                        s_offset = s_offset + (num_words - 1);
                        d_offset = d_offset + (num_words - 1);
                }

                /*-------------------------------------------------------------------
                |  for all lines that are to be copied, copy the next line segment
                |         This loop updates the presentation space only
                |------------------------------------------------------------------*/

BUGLPR (dbg_vttcfl, BUGGID, ("Moving data in presentation space \n"));
BUGLPR (dbg_vttcfl, BUGGID, ("Number of words to move is 0x%x\n",num_words));
BUGLPR (dbg_vttcfl, BUGGID, ("Source offset is 0x%x\n",s_offset));
BUGLPR (dbg_vttcfl, BUGGID, ("Destination offset is 0x%x\n",d_offset));
BUGLPR (dbg_vttcfl, BUGGID, ("Source address = 0x%x \n",(buf_addr + ((s_offset + ld->Scroll_offset) % ld->Vttenv.ps_words))));
BUGLPR (dbg_vttcfl, BUGGID, ("Destination address = 0x%x \n",(buf_addr + ((d_offset + ld->Scroll_offset) % ld->Vttenv.ps_words))));

                for (; num_words > 0; num_words--)
                {
                        *(buf_addr + ((d_offset + ld->Scroll_offset) % ld->Vttenv.ps_words)) =
                                *(buf_addr + ((s_offset + ld->Scroll_offset) % ld->Vttenv.ps_words));
                        s_offset += factor;
                        d_offset += factor;
                }
        }

        ld->Vttenv.cursor_show = cursor_show;

        if (!ld->Vttenv.vtt_active)    /* update the row lengths */
        {
                if (factor == 1)
                {
                        src_row = s_row;
                        dest_row = d_row;
                }
                else
                {
                        src_row = s_row + num_rows - 1;
                        dest_row = d_row + num_rows - 1;
                }
                for (i = 0; i < num_rows; i++, src_row += factor, dest_row += factor)
                {
                        ld->Vttenv.rowlen[dest_row] = ld->Vttenv.rowlen[src_row];
                }
        }
        else                           /* If the virtual terminal is active */
        {
		GGA_PARM_TRC0(ddf,gga_vttcfl,2,GGA_VTTCFL,GGA_VTTCFL_UPDATE_FB);

		ddf->IO_in_progress += 1;    /* set flag to indicate IO happening */

                /* copy num_rows lines from row s_row to row d_row */
                BUGLPR (dbg_vttcfl, BUGNTA, ("CFL before copy_scan_lines: s_row_len=%d, d_row_len=%d\n",
                       ld->Vttenv.rowlen[s_row], ld->Vttenv.rowlen[d_row]));
                copy_scan_lines (ld, s_row, d_row, num_rows);

                /* If the cursor must be moved */
		GGA_PARM_TRC0(ddf,gga_vttcfl,2,GGA_VTTCFL,GGA_VTTCFL_CALL_MOVC);

                vttmovc (vp);          /* Call movc to move it */

		ddf->IO_in_progress -= 1;    /* reset flag  */
        }

        VDD_TRACE (CFL, EXIT, vp);

        BUGLPR (dbg_vttcfl, BUGNTA, ("<CFL(0)\n"));

	GGA_EXIT_TRC0(ddf,gga_vttcfl,2,GGA_VTTCFL);

        return (0);
}                                      /* end  of  vttcfl */
