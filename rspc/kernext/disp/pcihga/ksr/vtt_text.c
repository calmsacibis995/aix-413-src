static char sccsid[] = "@(#)54  1.1  src/rspc/kernext/disp/pcihga/ksr/vtt_text.c, pcihga, rspc411, GOLD410 4/15/94 18:23:36";
/* vtt_text.c */
/*
based on "@(#)62        1.2  src/bos/kernext/disp/wga/ksr/vtt_text.c, bos, bos410 3/16/93 20:45:38";
 *
 *   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vtttext
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
BUGXDEF(dbg_vtttext);
/*********************************************************************/
/*                                                                   */
/*   IDENTIFICATION: VTTTEXT                                         */
/*                                                                   */
/*   DESCRIPTIVE name:        Draw Text routine                      */
/*                                                                   */
/*   FUNCTION: Draw a string of code base qualified ASCII            */
/*               characters into the refresh buffer of the           */
/*               display when a virtual terminal is active.          */
/*               If the terminal is inactive the characters are      */
/*               drawn into the presentation space that is managed   */
/*               by the Virtual Display Driver.                      */
/*                                                                   */
/*   INPUTS:   Virtual terminal structure pointer containing cursor  */
/*               position.                                           */
/*               Pointer to string to draw                           */
/*               Pointer and length of display attributes array      */
/*               Row and columns of requested area to write          */
/*               Flag for cursor show.                               */
/*                                                                   */
/*   OUTPUTS:    EIO                                                 */
/*                                                                   */
/*   CALLED BY: Mode Processor                                       */
/*              defc.c
/*              drawt.c
/*                                                                   */
/*   CALLS:      draw_text                                           */
/*               vttmovc                                             */
/*                                                                   */
/*********************************************************************/

long vtttext(vp, string, rc, cp, cursor_show)
struct vtmstruc *vp;
char   *string;                  /* array of ascii characters    */
struct vtt_rc_parms *rc;         /* string position and length   */
struct vtt_cp_parms *cp;         /* code point base/mask and     */
                                 /* new cursor position              */
ulong  cursor_show;              /* if true cursor is moved to   */
                                 /* the pos given in cp_parms    */
{

  ushort  tmp_attr,                /* Color character attribute         */
          len;
  ulong   *buf_addr;               /* output buffer offset              */
  long    buf_offset,              /* offset into the output buffer */
          save_off,                /* offset into the output buffer saved */
          this_char,               /* offset of the char to be displayed  */
          last_char;               /* offset of last char to be displayed */
  uchar   temp_color;              /* temp area used for rev video  */
  struct hga_data  *ld;
  struct hga_ddf *ddf;
  short   pse_wth;                 /* presentation space width */

  BUGLPR(dbg_vtttext,BUGNTA,("Entering VTT TEXT\n"));

  ld = (struct hga_data *) vp->vttld;
  ddf = (struct hga_ddf *) vp->display->free_area;

  if (ld->Vttenv.vtt_mode != KSR_MODE)/* not in character mode */
  { /* Only valid in character (KSR) mode */
    return(EIO);
  }

  VDD_TRACE(TEXT , ENTRY, vp);

  SET_ATTRIBUTES(cp->attributes,tmp_attr);

 /********************************
  * update the presentation space *
  ********************************/
  buf_addr = ld->Vttenv.pse;
  pse_wth  = ld->Vttenv.ps_size.wd;

 /*****************************************************************
  * calculate the address of the first character that must be read *
  *****************************************************************/
  save_off = buf_offset = ((rc->start_row - 1) * pse_wth +
                          (rc->start_column - 1));

 /**********************************************************
  * calculate starting and ending offsets into input string *
  **********************************************************/
  this_char = rc->string_index - 1;
  last_char = this_char + rc->string_length;


/*
 * The following is a debug print block intended to display all input parameters
 */

BUGLPR(dbg_vtttext,BUGGID,("\n"));
BUGLPR(dbg_vtttext,BUGGID,("Input parameters follow:\n"));

BUGLPR(dbg_vtttext,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vtttext,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));
BUGLPR(dbg_vtttext,BUGGID,("vp->display       = 0x%x\n",vp->display));
BUGLPR(dbg_vtttext,BUGGID,("vp->display->free_area = 0x%x\n",vp->display->free_area));

BUGLPR(dbg_vtttext,BUGGID,("string            = %s\n",string));

BUGLPR(dbg_vtttext,BUGGID,("rc->string_length = %d\n",rc->string_length));
BUGLPR(dbg_vtttext,BUGGID,("rc->string_index  = %d\n",rc->string_index));
BUGLPR(dbg_vtttext,BUGGID,("rc->start_row     = %d\n",rc->start_row));
BUGLPR(dbg_vtttext,BUGGID,("rc->start_column  = %d\n",rc->start_column));
BUGLPR(dbg_vtttext,BUGGID,("rc->dest_row      = %d\n",rc->dest_row));
BUGLPR(dbg_vtttext,BUGGID,("rc->dest_column   = %d\n",rc->dest_column));

BUGLPR(dbg_vtttext,BUGGID,("cp->cp_mask       = 0x%x\n",cp->cp_mask));
BUGLPR(dbg_vtttext,BUGGID,("cp->cp_base       = %d\n",cp->cp_base));
BUGLPR(dbg_vtttext,BUGGID,("cp->attributes    = 0x%x\n",cp->attributes));
BUGLPR(dbg_vtttext,BUGGID,("cp->cursor.x      = %d\n",cp->cursor.x));
BUGLPR(dbg_vtttext,BUGGID,("cp->cursor.y      = %d\n",cp->cursor.y));

BUGLPR(dbg_vtttext,BUGGID,("cursor_show       = %d\n",cursor_show));

  /******************************************/
  /*        Update Presentation Space       */
  /******************************************/
  for (; this_char < last_char; this_char++)
  { /* for all chars in the input string */
    /* copy them into the output buffer (Presentation Space) */

    *(buf_addr + ((buf_offset + ld->Scroll_offset) % ld->Vttenv.ps_words))
       = (ulong)
         (((string[this_char] & cp->cp_mask) + cp->cp_base) << 16) | tmp_attr;
    buf_offset++;
  }

  /* for the case were the text gets to the end of the pse_wth set it to wth */
  if ((buf_offset%pse_wth == 0) && (buf_offset/pse_wth != 0))
  { ld->Vttenv.rowlen[buf_offset/pse_wth-1] = pse_wth;
    BUGLPR(dbg_vtttext,BUGNTA,("text: ld->Vttenv.rowlen[%d]=%d\n",
           buf_offset/pse_wth-1,pse_wth));
  }
  else  /* set it to the length of the text line (if less than the current) */
  if (ld->Vttenv.rowlen[buf_offset/pse_wth] < buf_offset%pse_wth)
  { ld->Vttenv.rowlen[buf_offset/pse_wth] = buf_offset%pse_wth;
    BUGLPR(dbg_vtttext,BUGNTA,("text: ld->Vttenv.rowlen[%d]=%d\n",
           buf_offset/pse_wth,buf_offset%pse_wth));
  }

  if (ld->Vttenv.vtt_active) /* the virtual terminal is active */
  {
    /******************************************/
    /*        Update HGA VRAM (the Screen)    */
    /******************************************/
    ld->resv = 0xff;

    len = ((rc->string_length + rc->start_column-1)> ld->Vttenv.ps_size.wd)
        ? ld->Vttenv.ps_size.wd - rc->start_column - 1
        : rc->string_length ;

    draw_text(
            vp,
            (buf_addr + ((save_off + ld->Scroll_offset) % ld->Vttenv.ps_words)),
            len,
            (ushort)rc->start_column,
            (ushort)rc->start_row,
            TRUE,TRUE);
  }

  /* Update cursor show variable */
  ld->Vttenv.cursor_show = cursor_show;

  if (ld->Vttenv.cursor_show)      /* If the cursor must be moved */
  {
  BUGLPR(dbg_vtttext,BUGNTA,("Moving the cursor from vtt_text\n"));
    vttmovc(vp);   /* Call movc to move the cursor */
  }

  VDD_TRACE(TEXT , EXIT, vp);
  BUGLPR(dbg_vtttext,BUGNTA,("Leaving VTT TEXT\n"));
  return(0);
} /* end of vtttext */
