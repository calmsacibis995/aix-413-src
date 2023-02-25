static char sccsid[] = "@(#)62	1.3  src/rspc/kernext/disp/pciiga/ksr/vtt_clr.c, pciiga, rspc41J, 9515B_all 3/22/95 17:09:19";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttclr
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
BUGXDEF(dbg_vttclr);
GS_MODULE(iga_vttclr);

/********************************************************************/
/*                                                                  */
/* IDENTIFICATION: VTTCLR                                           */
/*                                                                  */
/* DESCRIPTIVE name:  Clear rectangle                               */
/*                                                                  */
/* FUNCTION:  Clear a specified upright rectangular area of the     */
/*              frame buffer (if the Virtual Terminal is active)    */
/*              or presentation space (if the VT is not active).    */
/*              A space code is stored in each character position   */
/*              along with a specified attribute (e.g., normal,     */
/*              reverse video, underline, blink, etc.).             */
/*                                                                  */
/* INPUTS:    Virtual terminal pointer                              */
/*              Structure indicating size of box to clear           */
/*              Attribute to be stored into presentation space      */
/*              Cursor show flag                                    */
/*                                                                  */
/* OUTPUTS:   None.                                                 */
/*                                                                  */
/* CALLED BY: Mode Processor                                        */
/*                                                                  */
/* CALLS:     vttmovc                                               */
/*                                                                  */
/********************************************************************/


long vttclr(vp, sp, attr, cursor_show)
struct vtmstruc *vp;
struct vtt_box_rc_parms *sp; /* upper-left and lower-right       */
                             /* corners of the rectangle         */
ulong  attr;                 /* character attribute              */
ulong  cursor_show;          /* if true cursor is moved to       */
                             /* pos specified in cp_parms        */

{

  ulong  pcm_ch_attr,        /* Color char and attr codes           */
         *buf_addr;          /* output buffer offset                */
  long   buf_offset,         /* offset into the output buffer       */
         buf_begin,          /* starting offset into the output buf */
         buf_end,            /* ending offset into the output buf   */
         height,             /* height of the rectangle             */
         i,tempfull;

  long   sx,sy,xdim,ydim;

  ushort my_attrib, tmp_attr;/* attribute work area */
  ushort buf_len;
  struct iga_data *ld;
  struct iga_ddf *ddf;

  ddf = (struct iga_ddf *) vp->display->free_area;

  IGA_ENTER_TRC0(ddf,iga_vttclr,2,IGA_VTTCLR);

  ld = (struct iga_data *) vp->vttld;

  if /*-----------------------*/
     /* not in character mode */
     /*-----------------------*/
      (ld->Vttenv.vtt_mode != KSR_MODE)
  {
     /*------------------------------------*/
     /* Only valid in character (KSR) mode */
     /*------------------------------------*/
     IGA_EXIT_TRC1(ddf,iga_vttclr,2,IGA_VTTCLR,EIO);
     return(EIO);
  }


/*
 * The following is a debug print block intended to display all input parameters
 */


BUGLPR(dbg_vttclr,BUGGID,("\n"));
BUGLPR(dbg_vttclr,BUGGID,("Input parameters follow:\n"));

BUGLPR(dbg_vttclr,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttclr,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));

BUGLPR(dbg_vttclr,BUGGID,("sp->row_ul        = %d\n",sp->row_ul));
BUGLPR(dbg_vttclr,BUGGID,("sp->column_ul     = %d\n",sp->column_ul));
BUGLPR(dbg_vttclr,BUGGID,("sp->row_lr        = %d\n",sp->row_lr));
BUGLPR(dbg_vttclr,BUGGID,("sp->column_lr     = %d\n",sp->column_lr));

BUGLPR(dbg_vttclr,BUGGID,("attr              = 0x%x\n",attr));

BUGLPR(dbg_vttclr,BUGGID,("cursor_show       = %d\n",cursor_show));

 /********************************************************************
  * calculate the attribute that should be stored with each character *
  * of the text string                                                 *
  ********************************************************************/
  SET_CHAR_ATTRIB(ld,attr,tmp_attr);

  pcm_ch_attr = SPACE_A | attr ;

  /******************************************************
   * clear a rectangular area in the presentation space *
   *****************************************************/

  buf_addr = ld->Vttenv.pse;

  /***********************************************************
   * calculate beginning and ending offsets within each line *
   **********************************************************/
  buf_begin =  (sp->row_ul    - 1) * ld->Vttenv.ps_size.wd +
               (sp->column_ul - 1) ;
  buf_end   = buf_begin + sp->column_lr - sp->column_ul;
  buf_len = (sp->column_lr - sp->column_ul) + 1;

  /*****************************************
   * calculate the height of the rectangle *
   ****************************************/
  height = (sp->row_lr - sp->row_ul) + 1;

  for (i = 0; i<height; i++)
  {
    /************************
    * set the row length    *
    ************************/
    if ( ld->Vttenv.rowlen[i+(sp->row_ul-1)] <= sp->column_lr &&
          ld->Vttenv.rowlen[i+(sp->row_ul-1)] > sp->column_ul)
    {
      ld->Vttenv.rowlen[i+(sp->row_ul-1)] = sp->column_ul -1;
      BUGLPR(dbg_vttclr,BUGACT,("reset row %d to %d\n",
            i + sp->row_ul,ld->Vttenv.rowlen[i + sp->row_ul]));
    }

    /****************
    * clear the row *
    ****************/

    /* for each character in the row */
    for (buf_offset = buf_begin; buf_offset <= buf_end; buf_offset++)
    { /****************************************************************
       * display the character as a space with the specified attribute *
       ****************************************************************/

      *(buf_addr + ((buf_offset + ld->Scroll_offset)
                   % ld->Vttenv.ps_words)) = pcm_ch_attr;
    }
    /* if the virtual terminal is active */
    if (ld->Vttenv.vtt_active )
    {
      IGA_PARM_TRC0(ddf,iga_vttclr,2,IGA_VTTCLR,IGA_VTTCLR_UPDATE_FB);

      ddf->IO_in_progress += 1;     /* set flag to indicate IO happening */

      draw_text(vp,
         (buf_addr+((buf_begin+ld->Scroll_offset) % ld->Vttenv.ps_words)),
         buf_len, sp->column_ul,(ushort)sp->row_ul + i, TRUE,TRUE);

      ddf->IO_in_progress -= 1;     /* reset flag */
    }

    /********************************
     * set up to clear the next row *
     *******************************/
     buf_begin += ld->Vttenv.ps_size.wd;
     buf_end += ld->Vttenv.ps_size.wd;
  }

  /* if the cursor must be moved */
  if (ld->Vttenv.cursor_show = cursor_show /* assignment */)
  {
    IGA_PARM_TRC0(ddf,iga_vttclr,2,IGA_VTTCLR,IGA_VTTCLR_CALL_MOVC);
    vttmovc(vp);
  }

  IGA_EXIT_TRC0(ddf,iga_vttclr,2,IGA_VTTCLR);

  return(0);
} /* end of vttclr */
