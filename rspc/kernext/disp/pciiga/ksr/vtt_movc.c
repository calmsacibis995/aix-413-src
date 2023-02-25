static char sccsid[] = "@(#)67	1.3  src/rspc/kernext/disp/pciiga/ksr/vtt_movc.c, pciiga, rspc41J, 9515B_all 3/22/95 17:11:05";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttmovc
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

/* #define CURPOSCHK 1 */

#include "iga_INCLUDES.h"
#include "iga_funct.h"
#include "igamisc.h"
#define SETBITS 0xFF
#define CLRBITS 0x00
BUGXDEF(dbg_vttmovc);
GS_MODULE(iga_vttmovc);

/*************************************************************************/
/*                                                                       */
/*   IDENTIFICATION: VTTMOVC                                             */
/*                                                                       */
/*   DESCRIPTIVE name:  Move Cursor to requested location                */
/*                                                                       */
/*   FUNCTION:      Moves the cursor to the specified                    */
/*                  row and column position on the screen. Row and       */
/*                  column numbers are unity based.                      */
/*                                                                       */
/*                  If not in KSR mode:                                  */
/*                  - Return EIO                                         *
/*                                                                       */
/*                  Update the cursor position in environment structure  */
/*                                                                       */
/*                  If terminal is active:                               */
/*                  - Send commands to the adapter to move the cursor    */
/*                                                                       */
/*   INPUTS:        Virtual terminal structure pointer containing cursor */
/*                  position.                                            */
/*                                                                       */
/*   OUTPUTS:   EIO                                                      */
/*                  INVALID_CURSOR_POSITION                              */
/*                                                                       */
/*   CALLED BY: Mode Processor                                           */
/*                  vttcfl                                               */
/*                  vttclr                                               */
/*                  vttcpl                                               */
/*                  vttdefc                                              */
/*                  vttrep                                               */
/*                  vttscr                                               */
/*                                                                       */
/*   CALLS:         None.                                                */
/*                                                                       */
/*************************************************************************/

long vttmovc(vp)
struct vtmstruc *vp;
{

  union
  { ushort half;
    struct
    { char   hi;
      char   lo;
    } byte;
  } addr;

  int i;
  short x,y;
  struct iga_data *ld;
  struct iga_ddf *ddf;
  ulong cursor_char;
  ulong tmp;
  long *buf_addr;
  ulong offset;
  struct fbdds *dds;


/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */


{

BUGLPR(dbg_vttmovc,BUGGID,("\n"));
BUGLPR(dbg_vttmovc,BUGGID,("Input parameters follow:\n"));
BUGLPR(dbg_vttmovc,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttmovc,BUGGID,("vp->mparms.cursor.x = 0x%x\n",vp->mparms.cursor.x));
BUGLPR(dbg_vttmovc,BUGGID,("vp->mparms.cursor.y = 0x%x\n",vp->mparms.cursor.y));
BUGLPR(dbg_vttmovc,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));
BUGLPR(dbg_vttmovc,BUGGID,("vp->display         = 0x%x\n",vp->display));
BUGLPR(dbg_vttmovc,BUGGID,("vp->display->free_area  = 0x%x\n",vp->display->free_area));
BUGLPR(dbg_vttmovc,BUGGID,("vp->display->odmdds = 0x%x\n",vp->display->odmdds));

}

  ld = (struct iga_data *) vp->vttld;
  ddf = (struct iga_ddf *) vp->display->free_area;
  dds = (struct fbdds *)   vp->display->odmdds;

  IGA_ENTER_TRC0(ddf,iga_vttmovc,2,IGA_VTTMOVC);

  BUGLPR(dbg_vttmovc,BUGNTA,(">MOVC: show=%d  shape=%d\n",
      ld->Vttenv.cursor_show,ld->Vttenv.cursor_select));

  if (ld->Vttenv.vtt_mode != KSR_MODE)/* not in character mode */
  {
    /*------------------------------------*/
    /* Only valid in character (KSR) mode */
    /*------------------------------------*/

    IGA_ENTER_TRC1(ddf,iga_vttmovc,2,IGA_VTTMOVC,EIO);

    return(EIO);
  }

  VDD_TRACE(MOVC , ENTRY, vp);

  if (ld->Vttenv.vtt_active) /* the virtual terminal is active */
  {

    ddf->IO_in_progress += 1;    /* set flag to indicate IO is going to happen */

    /********************************
    * update the presentation space *
    ********************************/
    buf_addr = (long *)ld->Vttenv.pse;

    /*****************************************************************
    * remove the software cursor from its current position           *
    *****************************************************************/
    x=ld->Vttenv.cursor_pos.cursor_col;
    y=ld->Vttenv.cursor_pos.cursor_row;
BUGLPR(dbg_vttmovc,BUGGID,("Cursor current position\n"));
BUGLPR(dbg_vttmovc,BUGGID,("x = 0x%x\n",x));
BUGLPR(dbg_vttmovc,BUGGID,("y = 0x%x\n",y));
    offset = ((y-1) * ld->Vttenv.ps_size.wd + (x-1));
    cursor_char = *(buf_addr + ((offset + ld->Scroll_offset) % ld->Vttenv.ps_words));
      /* reset the chars attributes */
BUGLPR(dbg_vttmovc,BUGGID,("buf_addr = 0x%x\n",buf_addr));
BUGLPR(dbg_vttmovc,BUGGID,("ld->Vttenv.ps_size.wd = 0x%x\n",ld->Vttenv.ps_size.wd));
BUGLPR(dbg_vttmovc,BUGGID,("ld->Scroll_offset = 0x%x\n",ld->Scroll_offset));
BUGLPR(dbg_vttmovc,BUGGID,("ld->Vttenv.ps_words = 0x%x\n",ld->Vttenv.ps_words));
BUGLPR(dbg_vttmovc,BUGGID,("offset = 0x%x\n",offset));
BUGLPR(dbg_vttmovc,BUGGID,("cursor addr = 0x%x\n",(buf_addr + ((offset + ld->Scroll_offset) % ld->Vttenv.ps_words))));
BUGLPR(dbg_vttmovc,BUGGID,("cursor_char = 0x%x\n",cursor_char));
    SET_CHAR_ATTRIB(ld,cursor_char,tmp);

    /**********************************************************************
    *by redrawing the character at the current cursor position,noninverted*
    **********************************************************************/

    IGA_PARM_TRC0(ddf,iga_vttmovc,2,IGA_VTTMOVC,IGA_VTTMOVC_CALL_DRAW_CHAR);

    draw_char(vp,x,y,(ushort)(cursor_char >> 16),NOCURSOR);

    /*****************************************************************
    * Then put the software cursor in its new position               *
    *****************************************************************/
    x = vp->mparms.cursor.x;
    y = vp->mparms.cursor.y;
BUGLPR(dbg_vttmovc,BUGGID,("Cursor new position\n"));
BUGLPR(dbg_vttmovc,BUGGID,("x = 0x%x\n",x));
BUGLPR(dbg_vttmovc,BUGGID,("y = 0x%x\n",y));
    offset = ((y-1) * ld->Vttenv.ps_size.wd + (x-1));

    cursor_char = *(buf_addr + ((offset + ld->Scroll_offset) % ld->Vttenv.ps_words));

BUGLPR(dbg_vttmovc,BUGGID,("buf_addr = 0x%x\n",buf_addr));
BUGLPR(dbg_vttmovc,BUGGID,("ld->Vttenv.ps_size.wd = 0x%x\n",ld->Vttenv.ps_size.wd));
BUGLPR(dbg_vttmovc,BUGGID,("ld->Scroll_offset = 0x%x\n",ld->Scroll_offset));
BUGLPR(dbg_vttmovc,BUGGID,("ld->Vttenv.ps_words = 0x%x\n",ld->Vttenv.ps_words));
BUGLPR(dbg_vttmovc,BUGGID,("offset = 0x%x\n",offset));
BUGLPR(dbg_vttmovc,BUGGID,("cursor addr = 0x%x\n",(buf_addr + ((offset + ld->Scroll_offset) % ld->Vttenv.ps_words))));
BUGLPR(dbg_vttmovc,BUGGID,("cursor_char = 0x%x\n",cursor_char));
    /******************************************************************
    * redraw the character at the New     cursor position-   inverted *
    ******************************************************************/
      /* reset the chars attributes */
    SET_CHAR_ATTRIB(ld,cursor_char,tmp);

    IGA_PARM_TRC0(ddf,iga_vttmovc,2,IGA_VTTMOVC,IGA_VTTMOVC_CALL_DRAW_CHAR);
    draw_char(vp,x,y,(ushort)(cursor_char >> 16),SETCURSOR);

    /******************************************************************
    * update the variable that tracks the virtual terminal's current *
    * cursor position with the new specified position                *
    *****************************************************************/
    ld->Vttenv.cursor_pos.cursor_col = vp->mparms.cursor.x;
    ld->Vttenv.cursor_pos.cursor_row = vp->mparms.cursor.y;

    ddf->IO_in_progress -= 1;    /* reset flag */

  } /* end if vtt is active */

  VDD_TRACE(MOVC , EXIT, vp);
  BUGLPR(dbg_vttmovc,BUGNTA,("<MOVC ret(0)\n"));

  IGA_EXIT_TRC0(ddf,iga_vttmovc,2,IGA_VTTMOVC);

  return(0);
} /* end  of  vttmovc */
