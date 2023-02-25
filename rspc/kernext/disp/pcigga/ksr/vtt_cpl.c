static char sccsid[] = "@(#)11  1.3  src/rspc/kernext/disp/pcigga/ksr/vtt_cpl.c, pcigga, rspc41J, 9513A_all 3/22/95 11:15:59";
/* vtt_cpl.c */
/*
based on "@(#)50  1.3  src/bos/kernext/disp/wga/ksr/vtt_cpl.c, bos, bos410 10/28/93 17:48:01";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttcpl
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
#include "gga_funct.h"
BUGXDEF(dbg_vttcpl);

GS_MODULE(gga_vttcpl);

/********************************************************************/
/*                                                                  */
/* IDENTIFICATION: VTTCPL                                           */
/*                                                                  */
/* DESCRIPTIVE name:  Copy Line Segment                             */
/*                                                                  */
/* FUNCTION:  Provides a horizontal scroll function by copying a    */
/*       portion of the specified line to another position          */
/*       on that same line. This operation is repeated for          */
/*       the number of consecutive lines requested.                 */
/*                                                                  */
/* INPUTS:    Virtual terminal pointer                              */
/*       Stucture indicating row/column information                 */
/*       Cursor show flag                                           */
/*                                                                  */
/* OUTPUTS:   None.                                                 */
/*                                                                  */
/* CALLED BY: Mode Processor                                        */
/*                                                                  */
/* CALLS:     copy_ps                                               */
/*       vttmovc                                                    */
/*                                                                  */
/********************************************************************/

long vttcpl(vp, rc, cursor_show)
struct vtmstruc *vp;
struct vtt_rc_parms *rc;  /* string position and length */
ulong  cursor_show; /* if true cursor is moved to */
{
  ulong  *buf_addr;  /* address of the buffer   */
  long  s_offset;    /* starting offset of the source string */
  long  d_offset;    /* starting offset of the target string */
  long  length;      /* number of words to be moved   */
  long  slength;     /* number of words to be moved   */
  long  no_lines_m1; /* number of lines to copy minus 1  */
  long  factor;      /* move direction indicator   */
  long  buf_offset;
  long  start;
  long  stop;
  long  shift_cnt;
  struct gga_data *ld;
  struct gga_ddf *ddf;

  ld = (struct gga_data *) vp->vttld;
  ddf = (struct gga_data *) vp->display->free_area;

  GGA_ENTER_TRC0(ddf,gga_vttcpl,2,GGA_VTTCPL);

  VDD_TRACE(CPL , ENTRY, vp);

  if (ld->Vttenv.vtt_mode != KSR_MODE)  /* If not in character mode */
  { 
     GGA_EXIT_TRC1(ddf,gga_vttcpl,2,GGA_VTTCPL,EIO);
     return(EIO);
  }




/*
 * The following is a debug print block intended to display all input parameters
 */


BUGLPR(dbg_vttcpl,BUGGID,("\n"));
BUGLPR(dbg_vttcpl,BUGGID,("Input parameters follow:\n"));

BUGLPR(dbg_vttcpl,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttcpl,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));

BUGLPR(dbg_vttcpl,BUGGID,("rc->string_length = %d\n",rc->string_length));
BUGLPR(dbg_vttcpl,BUGGID,("rc->string_index  = %d\n",rc->string_index));
BUGLPR(dbg_vttcpl,BUGGID,("rc->start_row     = %d\n",rc->start_row));
BUGLPR(dbg_vttcpl,BUGGID,("rc->start_column  = %d\n",rc->start_column));
BUGLPR(dbg_vttcpl,BUGGID,("rc->dest_row      = %d\n",rc->dest_row));
BUGLPR(dbg_vttcpl,BUGGID,("rc->dest_column   = %d\n",rc->dest_column));

BUGLPR(dbg_vttcpl,BUGGID,("cursor_show       = %d\n",cursor_show));

  if (rc->dest_column != rc->start_column)
  { /************************************************************
    * the source address does not equal the destination address *
    * copy the indicated line segments as requested             *
    ************************************************************/

    buf_addr = ld->Vttenv.pse;

    /**************************************************************
    * calculate the starting offset of the first character to be *
    * moved in the source and and destination strings     *
    *************************************************************/
    s_offset = (rc->start_row - 1) * ld->Vttenv.ps_size.wd +
                 (rc->start_column - 1) ;
    d_offset = (rc->start_row - 1) * ld->Vttenv.ps_size.wd +
                 (rc->dest_column - 1) ;

    BUGLPR(dbg_vttcpl,BUGNTA,("CPL:s_offset=%d, d_offset=%d\n",s_offset,d_offset));
    /****************************************************************/
    /* calculate the number of characters to be copied taking into  */
    /* account truncation when the destination is to the right of   */
    /* the source. NOTE: truncation is not possible when the        */
    /* destination is to the left of the source because the minimum */
    /* column number is 1            */
    /****************************************************************/

    slength = (ld->Vttenv.ps_size.wd - rc->dest_column) + 1;

    if (slength > rc->string_length)
    {
      slength = rc->string_length;
    }

    /*********************************************
    * calculate the number of lines to be copied *
    *********************************************/

    no_lines_m1 = rc->dest_row - rc->start_row;
    start = rc->start_row;
    stop = start + no_lines_m1;


    if (rc->dest_column < rc->start_column)
    { /***********************************************
      * the destination is to the left of the source *
      * therefore copy left                          *
      ***********************************************/
      factor = 1;
      shift_cnt = (rc->start_column - rc->dest_column);
      BUGLPR(dbg_vttcpl,BUGACT,("CPL:copy left\n"));
    }
    else
    {
      /***************************************************************
      * copy right to left avoiding source and dest overlap problems *
      ***************************************************************/
      factor = -1;
      shift_cnt = (rc->dest_column - rc->start_column);
      s_offset = s_offset + (slength - 1);
      d_offset = d_offset + (slength - 1);
      BUGLPR(dbg_vttcpl,BUGACT,("CPL:copy right\n"));
    }

    for (; no_lines_m1 >= 0; no_lines_m1--)
    {
    /**************************************
    * for all lines that are to be copied *
    * copy the next line segment          *
    **************************************/
      for (length = slength; length > 0; length--)
      {
      /***********************************************************
       * for all characters in the line segment                  *
       * copy the character and its corresponding attribute code *
       **********************************************************/
         *(buf_addr + ((d_offset + ld->Scroll_offset)  %
           ld->Vttenv.ps_words)) =
         *(buf_addr + ((s_offset + ld->Scroll_offset)  %
           ld->Vttenv.ps_words));
         d_offset = (d_offset + factor) ;
         s_offset = (s_offset + factor) ;
      }
      s_offset = s_offset + ld->Vttenv.ps_size.wd - (slength * factor);
      d_offset = d_offset + ld->Vttenv.ps_size.wd - (slength * factor);
    }
  }

  /********************************************************************/
  /* Reset the affected lines text length                             */
  /********************************************************************/
  for (;start <= stop; start++)
  {
    if ( factor == 1 )
    { ld->Vttenv.rowlen[start-1] -= shift_cnt;
      if (ld->Vttenv.rowlen[start-1] < 0 )
      { ld->Vttenv.rowlen[start-1] = 0;
      }
    }
    else
    { if ( ld->Vttenv.rowlen[start-1] >= rc->start_column )
      { ld->Vttenv.rowlen[start-1] += shift_cnt;
        if ( ld->Vttenv.rowlen[start-1] > ld->Vttenv.ps_size.wd )
        { ld->Vttenv.rowlen[start-1] = ld->Vttenv.ps_size.wd;
        }
      }
    }
  }

  /* If the virtual terminal is active draw scrolled text */
  if (ld->Vttenv.vtt_active)
  {
    GGA_PARM_TRC0(ddf,gga_vttcpl,2,GGA_VTTCPL,GGA_VTTCPL_UPDATE_FB);

    ddf->IO_in_progress += 1;   /* set flag to indicate IO happening */

    no_lines_m1 = (rc->dest_row) - (rc->start_row);
    start = rc->start_row;
    stop = start + no_lines_m1;
    buf_offset = ((start-1) * ld->Vttenv.ps_size.wd);
    for (;start <= stop; start++)
    {
      draw_text(vp,(long)(ld->Vttenv.pse + ((buf_offset +
        ld->Scroll_offset) % ld->Vttenv.ps_words)),
        (ushort)ld->Vttenv.ps_size.wd,1,(ushort)start,FALSE, TRUE);
      buf_offset += ld->Vttenv.ps_size.wd;
      BUGLPR(dbg_vttcpl,BUGNTA,("CPL<DTXT\n"));
    }

    ddf->IO_in_progress -= 1;   /* reset flag  */

  }

  ld->Vttenv.cursor_show = cursor_show;
  if (cursor_show)     /* If the cursor must be moved  */
  {
    GGA_PARM_TRC0(ddf,gga_vttcpl,2,GGA_VTTCPL,GGA_VTTCPL_CALL_MOVC);
    vttmovc(vp);       /* call movc to move the cursor */
  }

  VDD_TRACE(CPL , EXIT, vp);
  BUGLPR(dbg_vttcpl,BUGNTA,("<CPL(0)\n"));

  GGA_EXIT_TRC0(ddf,gga_vttcpl,2,GGA_VTTCPL);

  return(0);

} /* end of vttcpl */
