static char sccsid[] = "@(#)26  1.1  src/rspc/kernext/disp/pciwfg/ksr/vtt_scr.c, pciwfg, rspc411, 9434B411a 8/24/94 07:56:19";
/* vtt_scr.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttscr
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


#include "wfg_INCLUDES.h"
#include <memory.h>

BUGXDEF(dbg_vttscr);
BUGXDEF(dbg_CopyScanLines);
/**********************************************************************/
/*                                                                    */
/*   IDENTIFICATION: VTTSCR                                           */
/*                                                                    */
/*   DESCRIPTIVE name:  Scroll screen up or down                      */
/*                                                                    */
/*   FUNCTION:  Scrolls the entire contents of the screen             */
/*                (or presentation space if the Virtual Terminal is   */
/*                not active) up or down a specified number of lines. */
/*                                                                    */
/*                If not in character mode:                           */
/*                - Return EIO                                        */
/*                                                                    */
/*                Set up attributes for area being scrolled out       */
/*                                                                    */
/*                If all the data has been scrolled off the screen:   */
/*                  - Update all entries in the pspace with blanks    */
/*                    and requested attributes.                       */
/*                  - Write contents of presentation space to screen  */
/*                    (clear the screen)                              */
/*                                                                    */
/*                If data is scrolled up:                             */
/*                - Store a blank for every character in each line at */
/*                    bottom of screen                                */
/*                                                                    */
/*                If data is scrolled down:                           */
/*                  - Store a blank for every character in each line at */
/*                    top of screen.                                  */
/*                                                                    */
/*                Write new Presentation space to the adapter         */
/*                                                                    */
/*                Move the cursor if requested                        */
/*                                                                    */
/*   INPUTS:    Number of lines to scroll (>0 up, < 0 down)           */
/*                Attribute for area being cleared                    */
/*                Cursor Structure containing new cursor position     */
/*                Flag to indicate whether to move cursor             */
/*                                                                    */
/*   OUTPUTS:   EIO                                                   */
/*                                                                    */
/*   CALLED BY: Mode Processor                                        */
/*                                                                    */
/*   CALLS:     none                                                  */
/*                                                                    */
/**********************************************************************/


long vttscr(vp, lines, attr, cursor_show)
struct vtmstruc *vp;
long   lines;               /* number of lines to scroll     */
                            /*   >0 ==> scroll data up       */
                            /*   <0 ==> scroll data down     */
ulong  attr;                /* attribute associated with all */
                            /* characters of the new lines   */
                            /* that appear at either the top */
                            /* or bottom of the screen       */
ulong  cursor_show;         /* if true cursor is moved to    */
                            /* position given in cp_parms    */
{

  ulong  tmp;
  long   i,start;
  ulong  *buf_addr,         /* address of the buffer          */
         blanks;            /* blanks with the specifed       */
                            /* attribute                      */
  short  row;
  union {
    ulong   full;
    struct {
      char  byte1;
      char  byte_h;
      char  byte_m;
      char  byte_l;
    } bytes;
  } startaddr;

  struct wfg_data *ld;
  struct wfg_ddf *ddf;
BUGLPR(dbg_vttscr,BUGGID,("Entering vtt_scr() \n"));

/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */

{

BUGLPR(dbg_vttscr,BUGGID,("\n"));
BUGLPR(dbg_vttscr,BUGGID,("Input parameters follow:\n"));
BUGLPR(dbg_vttscr,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttscr,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));
BUGLPR(dbg_vttscr,BUGGID,("vp->display->free_area = 0x%x\n",vp->display->free_area));

BUGLPR(dbg_vttscr,BUGGID,("lines             = %d\n",lines));

BUGLPR(dbg_vttscr,BUGGID,("attr              = 0x%x\n",attr));

BUGLPR(dbg_vttscr,BUGGID,("cursor_show       = %d\n",cursor_show));
}

  ld = (struct wfg_data *) vp->vttld;
  ddf = (struct wfg_ddf *) vp->display->free_area;

  if /*-----------------------*/
     /* not in character mode */
     /*-----------------------*/
      (ld->Vttenv.vtt_mode != KSR_MODE)
  {
     /*------------------------------------*/
     /* Only valid in character (KSR) mode */
     /*------------------------------------*/
     BUGLPR(dbg_vttscr,BUGNFO,("Error: Mode is not KSR\n"));
     return(EIO);
  }

  VDD_TRACE(SCR , ENTRY, vp);

  /******************************************
   * will be updating the presentation space *
   ******************************************/
  buf_addr = ld->Vttenv.pse;

  /*******************************************************************
  * set up a word with one blank with the specified pc color attrib  *
  *******************************************************************/
  blanks = BLANKS_NULL_ATTR | attr ;

  /********************************
  * scroll the presentation space *
  ********************************/

  if((lines >= ld->Vttenv.ps_size.ht) || (lines <= -ld->Vttenv.ps_size.ht))
  {
    /******************************************
     * all the data is scrolled off the screen *
     ******************************************/
    BUGLPR(dbg_vttscr,BUGACT,("All Data to be scrolled off the screen\n"));

    for (i = 0; i < ld->Vttenv.ps_words; i++)
    {
      /* store a blank into the PS */
      ld->Vttenv.pse[i] = blanks;
    }

    /* If terminal is active, clear the screen too */
    if (ld->Vttenv.vtt_active == VTT_ACTIVE)
    {
      SET_CHAR_ATTRIB(ld,blanks,tmp);

      /* reset text lenght of all lines to zero */
      for (row=0;row<ld->Vttenv.ps_size.ht; row++)
      { ld->Vttenv.rowlen[row] = 0;
      }

      /* clear the screen */
      clear_lines (vp,ld, 0,
            ld->Vttenv.ps_size.ht * ld->Vttenv.character_box.height,
            ld->Vttenv.ps_size.wd * ld->Vttenv.character_box.width,
              0);
    }
    BUGLPR(dbg_vttscr,BUGNTA,("<SCR return(0)\n"));
    return(0);
  }
  else
  {
    /*********************************************************/
    /* only part of the data has been scrolled out of the PS */
    /*********************************************************/
    BUGLPR(dbg_vttscr,BUGACT,("Part of Data to be scrolled\n"));

    if /**************************
       * the data is scrolled up *
       **************************/
       (lines > 0)
    {
      /************************************************************
      * New method of scrolling is to simply increment a scroll   *
      * pointer into the buffer to indicate where in the buffer   *
      * the current upper left character is stored.               *
      ************************************************************/
      BUGLPR(dbg_vttscr,BUGACT,("Scrolling UP....\n"));

      ld->Scroll_offset = ((ld->Scroll_offset + ld->Vttenv.ps_size.wd *
                                lines) % ld->Vttenv.ps_words);

      for /***********************************************************
          * every character in the new lines at the bottom of screen *
          ************************************************************/
         (i = ((ld->Vttenv.ps_size.ht - lines) * ld->Vttenv.ps_size.wd);
          i < ((ld->Vttenv.ps_size.ht        ) * ld->Vttenv.ps_size.wd);
          i++)
      {
        /**********************************
        * store a blank into the new line *
        **********************************/

        *(buf_addr+((i+ld->Scroll_offset) % ld->Vttenv.ps_words)) = blanks;

      }
    }
    else    /* Scrolling text downward (lines < 0) */
    {
      BUGLPR(dbg_vttscr,BUGACT,("Scrolling DOWN..\n"));
      /************************************************************
      * New method of scrolling is to simply increment a scroll  *
      * pointer into the buffer to indicate where in the buffer  *
      * the current upper left character is stored.              *
      ************************************************************/

      ld->Scroll_offset += ld->Vttenv.ps_size.wd * lines ;

      /* Check to see if we have scrolled off top of p space */

      ld->Scroll_offset = (ld->Scroll_offset < 0 ) ?
               ld->Vttenv.ps_words + ld->Scroll_offset : ld->Scroll_offset;

      BUGLPR(dbg_vttscr,BUGNTA,("Scroll_offset = %d\n",ld->Scroll_offset));

      for /**********************************************************
          * every character in the new lines at the top of the      *
          * screen.                                                 *
          * NOTE: "lines" is negative.                              *
          **********************************************************/
          (i = 0; i < ((-lines) * ld->Vttenv.ps_size.wd); i++)
      {
        /**********************************
        * store a blank into the new line *
        **********************************/

        *(buf_addr+((i+ld->Scroll_offset) % ld->Vttenv.ps_words)) = blanks;
      } /* for */
    } /* data moved down */


    if ( ! ld->Vttenv.vtt_active)  /* update row lengths */
    {
      if ( lines > 0 ) /* adjust for scroll up */
      {
        for( i=0; i < (ld->Vttenv.ps_size.ht - lines); i++ )
        {
          ld->Vttenv.rowlen[i] = ld->Vttenv.rowlen[i+lines];
        }
        for( ; i < ld->Vttenv.ps_size.ht; i++) /* clear bottom lines */
        {
          ld->Vttenv.rowlen[i] = 0;
        }
      }
      else   /* adjust for scroll down */
      {
        lines *= -1;
        for( i = ld->Vttenv.ps_size.ht - 1; i >= lines; i--)
        {
          ld->Vttenv.rowlen[i] = ld->Vttenv.rowlen[i-lines];
        }
        for( i=0; i < lines; i++ )  /* clear top lines */
        {
          ld->Vttenv.rowlen[i] = 0;
        }
      }
    }
    else /* Terminal is active so scroll screen and update row lengths */
    {
      if (lines < 0)
      {
        lines *= -1;
        copy_scan_lines(ld, 0, lines, ld->Vttenv.ps_size.ht - lines);

      /* Clear remaining text at top of screen */
        {
          short maxlen = 0;            /*length of longest row to clear*/
          short boxh = ld->Vttenv.character_box.height;/*pel ht of char*/
          short boxw = ld->Vttenv.character_box.width; /*pel wth of char*/
          /**********************************************************/
          /*              of all the rows to clear,                 */
          /* find the longest row and clear all rows to that length */
          /**********************************************************/
          for (row=0; row<lines; row++)
          { if (ld->Vttenv.rowlen[row] > maxlen)
            { maxlen = ld->Vttenv.rowlen[row];
            }
            /*********************************************************/
             /* now reset the text length of the cleared row to zero  */
          /*********************************************************/
          ld->Vttenv.rowlen[row]=0;
          } /* end for rows to clear */

          clear_lines (vp,ld, 0,          /* top line to clear (in pels)*/
            boxh * lines,             /* bot line to clear (in pels)*/
            boxw * maxlen,              /* # of pels in line to clear */
            0);                       /* starting x pel to clear    */
        }
      } /* if lines is negative */
      else
      {
        copy_scan_lines(ld, lines, 0, ld->Vttenv.ps_size.ht - lines);

        /* Clear remaining text at bottom of screen */
        {
          short maxlen = 0;            /*length of longest row to clear*/
          short row;                                   /*last row in pse */
          short boxh = ld->Vttenv.character_box.height;/*pel ht of char*/
          short boxw = ld->Vttenv.character_box.width; /*pel wth of char*/
          /**********************************************************/
          /*              of all the rows to clear,                 */
          /* find the longest row and clear all rows to that length */
          /**********************************************************/
          for (row=ld->Vttenv.ps_size.ht-lines;
            row<ld->Vttenv.ps_size.ht;row++)
          {
            if (ld->Vttenv.rowlen[row] > maxlen)
            { maxlen = ld->Vttenv.rowlen[row];
            } /* end if rowlen > maxlen */
            /*******************************************************/
            /* now reset the text lenght of the cleared row to zero*/
            /*******************************************************/
            ld->Vttenv.rowlen[row]=0;/* reset text length to zero*/
          } /* end for rows to clear */
          row = ld->Vttenv.ps_size.ht - lines;
          clear_lines (vp,ld,
            row * boxh,                 /* top line to clear (in pels)*/
            (row+lines) * boxh,         /* bot line to clear (in pels)*/
            boxw * maxlen,              /* # of pels in line to clear */
            0);                         /* starting x pel to clear    */
        }
      } /* end  if-else lines + or - */

      if /*--------------------------*/
         /* the cursor must be moved */
         /*--------------------------*/
        (ld->Vttenv.cursor_show = cursor_show /* assignment */)
      {
        /*-----------------*/
        /* move the cursor */
        /*-----------------*/
        vttmovc(vp);
      }
    }  /* end of if-vtt-active code */

    VDD_TRACE(SCR , EXIT, vp);

    BUGLPR(dbg_vttscr, BUGNTA, ("<SCR return(0)\n"));
    return(0);
  } /* end of if full or partial scroll */
} /* end of vttscr  */


/********************************************************************/
/*                                                                  */
/* IDENTIFICATION: COPY_SCAN_LINES                                  */
/*                                                                  */
/* DESCRIPTIVE name:  Copy full lines of text.                      */
/*                                                                  */
/* FUNCTION: Copies a sequence of one or more full lines            */
/*             up or down a specified number of lines               */
/*             This is done by copying scan lines from VRAM         */
/*             source locations to VRAM destination locations.      */
/*             To save time, only the text length of each line      */
/*             is actually copied (plus any to overwrite previous   */
/*             text)                                                */
/*                                                                  */
/* INPUTS:      Local Data pointer                                  */
/*              Source , Destination Row                            */
/*              Number of rows to move                              */
/*                                                                  */
/* OUTPUTS:                                                         */
/*                                                                  */
/* CALLED BY: Mode Processor                                        */
/*                                                                  */
/* CALLS:     NONE                                                  */
/*                                                                  */
/********************************************************************/

copy_scan_lines(ld, s_row, d_row, num_rows)
struct wfg_data *ld;
long   s_row,         /* source row                 */
       d_row,         /* destination row            */
       num_rows;      /* number of rows to copy     */
{
  long factor;         /* move direction indicator */
  short boxw, boxh, bpp;
  short *rowlen;
  long len=0L;         /* length of row/scanline or number of pels/bits/words*/
  ulong  yinc;          /* address increment values */
  short w_scan, r_scan;  /* scan line numbers */
  struct wfg_ddf *ddf;


  BUGLPR(dbg_CopyScanLines,BUGNFO,("CopyScanLines>\n"));
  BUGLPR(dbg_CopyScanLines,BUGNTX,("srow%d, drow%d, #rows%d\n",
      s_row, d_row, num_rows));

  /********************************************************************
  *********************************************************************
  *  for all lines that are to be copied, copy the next line segment  *
  *         This loop updates the the WFG VRAM                        *
  *********************************************************************
  ********************************************************************/
  if ((ld->Vttenv.vtt_active)  /* If the virtual terminal is active         */
      &&  (num_rows > 0))    /* and there are more than zero rows to copy */
  {
    rowlen = ld->Vttenv.rowlen; /* array to track amount of text on each row */
    boxh = ld->Vttenv.character_box.height;/* height of character to display */
    boxw = ld->Vttenv.character_box.width; /* width  of character to display */

    /*******************************************************/
    /*   Copy Scanlines from the Source to Destination    */
    /*******************************************************/

    /* for each row in the presentation space, copy the read row        */
    /* to the write row                                                 */
    /* Also, copy the lower row text length to the above row's @loop bot*/

    if (s_row < d_row)            /* copy bottom to top */
    { s_row += (num_rows - 1);
      d_row += (num_rows - 1);
      factor = -1;
    }
    else                          /* copy top to bottom */
    { factor = 1;
    }

    /* set addressability for attaching the bus */
    ddf = ld->ddf;
    /* attach (memory map) the bus */
    ld->bus_base_addr = PCI_BUS_ATT(&ddf->pci_mem_map);

    yinc = (FB_PIXEL(0,1) - FB_PIXEL(0,0));   /* calc y interline dist. */

    /****************Check Cursor Position*******************************/
    /* if the cursor is on this source row, relocate it on the dest row */
    /* if the cursor is on destination row that gets scrolled           */
    /* off the screen(i.e. never on a source row, it will end up in the */
    /*                                                                  */
    /* Also ensure the the source row is long enough to get the cursor  */
    /* copied and erased (usually the cursor will be 1 character beyond */
    /* the text length of the row).                                     */
    /********************************************************************/

    bpp = boxw * (ushort)ld->bpp;

    BUGLPR(dbg_vttscr,BUGACT,("Cursor on row %d, s_row+1=%d\n",
            ld->Vttenv.cursor_pos.cursor_row,s_row+1));
    for (; num_rows > 0; num_rows--, d_row+=factor, s_row+=factor)
    {

      if (ld->Vttenv.cursor_pos.cursor_row == s_row+1)
      {
        ld->Vttenv.cursor_pos.cursor_row = d_row+1;
        if (ld->Vttenv.rowlen[d_row+1] < ld->Vttenv.cursor_pos.cursor_col)
        {
          ld->Vttenv.rowlen[d_row+1] = ld->Vttenv.cursor_pos.cursor_col;
        }
      }

      /**********************************************************************/
      /* if the source row's text is longer,                                */
      /*  copy scan lines of the source row's text length to the dest row   */
      /* else                                                               */
      /*  copy scan lines of the dest row's text length to the dest row     */
      /**********************************************************************/
      if ( !(len = (rowlen[d_row] <= rowlen[s_row]) ? rowlen[s_row] : rowlen[d_row]) )
         continue;  /* if both lines have no text, no need to copy */
      len *= bpp;
      /* ensure that len is a multiple of WORDLEN */
      if ( len % WORDLEN )
         len += (WORDLEN - (len % WORDLEN));
      len /= WORDLEN;
      BUGLPR(dbg_CopyScanLines,BUGNTX,("len = %d\n", len));

      /* set the text length of the destination row equal to the source row */
      rowlen[d_row] = rowlen[s_row];

      /************************************************/
      /*    COPY BY SCAN LINES                        */
      /************************************************/
      w_scan = boxh * d_row;
      r_scan = boxh * s_row;

      BUGLPR(dbg_CopyScanLines,BUGNTX,("moveit (%x,%x,%d,%d,%d,%d)\n",
              FB_PIXEL(0,w_scan),
              FB_PIXEL(0,r_scan),
              len, 4, yinc,  boxh));
      moveit( FB_PIXEL(0,w_scan),      /* Address of target string            */
              FB_PIXEL(0,r_scan),      /* Address of source string            */
              len,                     /* Length of target string             */
              4,                       /* x addr increment value (4 bytes)    */
              yinc,                    /* y addr increment value              */
              boxh);                   /* number of scan lines per row to copy*/
    }

    PCI_BUS_DET(ld->bus_base_addr);

    BUGLPR(dbg_CopyScanLines,BUGNFO,("<CopyScanLines\n"));
  }
}
