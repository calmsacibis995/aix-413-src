static char sccsid[] = "@(#)22  1.2  src/rspc/kernext/disp/pciwfg/ksr/vtt_defc.c, pciwfg, rspc411, 9440E411a 10/11/94 10:32:04";
/* vtt_defc.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttdefc, change_cursor_shape
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

#include "wfg_INCLUDES.h"
#include "wfgmisc.h"
#define SETBITS 0xFF
#define CLRBITS 0x00
BUGXDEF(dbg_vttdefc);
BUGXDEF(dbg_ChgCurShp);

/**********************************************************************/
/*                                                                    */
/*   IDENTIFICATION: VTTDEFC                                          */
/*                                                                    */
/*   DESCRIPTIVE NAME:        Define Cursor to one of six shapes      */
/*                                                                    */
/*   FUNCTION:        If VT is not in character mode:                 */
/*                - Return EIO                                        */
/*                                                                    */
/*                Set selected cursor shape into Vttenv table.        */
/*                                                                    */
/*                If VT is active, change shape according to following*/
/*                selector values:                                    */
/*                                                                    */
/*                  0 - Invisible cursor                              */
/*                  1 - Single underscore                             */
/*                  2 - Double underscore                             */
/*                  3 - Half Blob                                     */
/*                  4 - Mid character double line                     */
/*                  5 - Full blob                                     */
/*                  Any other value will get double underscore        */
/*                                                                    */
/*                If needed move the cursor by calling VTTMOVC        */
/*                                                                    */
/*   INPUTS:        Selector value                                    */
/*                Vtt_cursor structure                                */
/*                Cursor_show (0 means don't show cursor)             */
/*                Top scan line                                       */
/*                Bottom scan line                                    */
/*                                                                    */
/*   OUTPUTS:   EIO                                                   */
/*                                                                    */
/*   CALLED BY: vttinit                                               */
/*                vttact                                              */
/*                Mode Processor                                      */
/*                                                                    */
/*   CALLS:        Change_Cursor_shape                                */
/*                vttmovc                                             */
/*                                                                    */
/**********************************************************************/


long vttdefc(vp, selector, cursor_show)
struct vtmstruc *vp;
uchar  selector;      /* shape selector              */
ulong  cursor_show;   /* 0 ==> don't show the cursor */
{
  short        top,bottom;  /* must be signed variables */
  struct wfg_data *ld;

  BUGLPR(dbg_vttdefc,BUGNTA,("Entering DEFC\n"));
  ld = (struct wfg_data *) vp->vttld;
  VDD_TRACE(DEFC , ENTRY, vp);
  if  (ld->Vttenv.vtt_mode != KSR_MODE) /* not in character mode */
  { /* Only valid in character (KSR) mode */
    return(EIO);
  }

/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */

{

BUGLPR(dbg_vttdefc,BUGGID,("\n"));
BUGLPR(dbg_vttdefc,BUGGID,("Input parameters follow:\n"));
BUGLPR(dbg_vttdefc,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttdefc,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));

BUGLPR(dbg_vttdefc,BUGGID,("selector          = %d\n",selector));

BUGLPR(dbg_vttdefc,BUGGID,("cursor_show       = %d\n",cursor_show));

}
  /******************************************
  * Make sure global cursor shape is saved, *
  * it is used when new font is downloaded. *
  ******************************************/

  ld->Vttenv.cursor_select = selector;

  if (ld->Vttenv.vtt_active)
  { /* if the virtual terminal is active, update adapter's cursor shape */
    /* otherwise, wfgact() will update the shape with with the selector */
    /* just stored in ld->Vttenv->select                                */

    switch (selector) /* on the cursor shape */
    {
      /* invisible */
      case  0: /* code to set cursor to blank */
BUGLPR(dbg_vttdefc,BUGGID,("  shape = invisible\n"));
              top = -1;
            bottom = -1;
               break;

      /* single underscore */
      case  1: /* set top and bottom to  char_box_ht-1 */
BUGLPR(dbg_vttdefc,BUGGID,("  shape = single underscore\n"));
               top = ld->Vttenv.character_box.height - 1;
               bottom = ld->Vttenv.character_box.height;
               break;

      /* double underscore */
      case 2:  /* set top to charbox_ht-2 and bottom to charbox_ht-1   */
BUGLPR(dbg_vttdefc,BUGGID,("  shape = double underscore\n"));
               top = ld->Vttenv.character_box.height - 2;
               bottom = ld->Vttenv.character_box.height;
               break;

      /* half blob lower half of character */
      case 3:  /* insert code to to set top to charbox_ht/2 and bottom to  */
BUGLPR(dbg_vttdefc,BUGGID,("  shape = half blob lower half of character\n"));
               /* charbox_ht-1                                             */
               top = ld->Vttenv.character_box.height / 2;
               bottom = ld->Vttenv.character_box.height;
               break;

      /* mid character double line */
      case 4:  /* set top to (charbox_ht/2)-1 and bottom to                */
               /* charbox_hgt/2.                                           */
BUGLPR(dbg_vttdefc,BUGGID,("  shape = mid character double line\n"));
               top = (ld->Vttenv.character_box.height/2) - 1;
               bottom = (ld->Vttenv.character_box.height/2) + 1;
               break;

      /* full blob */
      case 5:  /* set top to 1 and bottom to charbox_hgt-1.  */
BUGLPR(dbg_vttdefc,BUGGID,("  shape = full blob\n"));
               top = 0;
               bottom = ld->Vttenv.character_box.height;
               break;

      default:
               /**************************************************************
               * Selectors 6-254 are reserved                                *
               * any value other than 0-5 will get a double underscore!      *
               **************************************************************/
BUGLPR(dbg_vttdefc,BUGGID,("  shape = default\n"));
               top = ld->Vttenv.character_box.height-2;
               bottom = ld->Vttenv.character_box.height;
               break;

    } /* end of switch */

    change_cursor_shape (vp,top,bottom);

  }  /* end if vtt is active */
  if (ld->Vttenv.cursor_show = cursor_show /* assignment */)
  { /* the cursor must be moved */
    vttmovc(vp);
  }
  VDD_TRACE(DEFC , EXIT, vp);
  /*BUGLPR(dbg_vttdefc,BUGNTA,("<DEFC ret(0)\n"));*/
  return(0);
} /* end  of vttdefc */
/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: CHANGE_CURSOR_SHAPE                                 */
/*                                                                     */
/* DESCRIPTIVE NAME: Change cursor to requested shape                  */
/*                                                                     */
/* FUNCTION:  This routine sets the WFG local data environment cursor  */
/*              data area to conform to the requested shape.           */
/*              The cursor will be a software cursor which             */
/*              inverts the forground and background colors            */
/*              The mechanism for this will be to store 1's            */
/*              in the character box positions which are to            */
/*              be inverted, and 0's in those which are not to         */
/*              be inverted.  vtt_movc() will be responsible           */
/*              for calling a routine to XOR the character box         */
/*              and the 1bit/pel,pre expanded character font.          */
/*                                                                     */
/*              NOTE: This routine will NOT be called from             */
/*                    wfgdefc() if the selected cursor shape           */
/*                    is the current cursor shape.                     */
/*                                                                     */
/*                                                                     */
/* INPUTS:    top, bottom -  shorts defining the cursor shape          */
/*                                                                     */
/* OUTPUTS:   NONE.                                                    */
/*                                                                     */
/* CALLED BY: VTTDEFC                                                  */
/*                                                                     */
/* CALLS:     None.                                                    */
/*                                                                     */
/***********************************************************************/

change_cursor_shape(vp, top, bottom)
struct vtmstruc *vp;
short           top,      /* must be signed variables */
                bottom;   /* top and bottom line numbers in cursor */
{
  int bytecnt,count1,count2; /* work variables */
  ushort width,height; /* character box dimensions */
  short x,y;                   /* counters for cursor data position */
  short area,bytenum;          /* counters for cursor data position */
  union
  { ulong full;
    uchar bdat[4];
  } workword;

  char volatile *rptr,*dptr;
  struct wfg_data *ld;
  struct fbdds *dds;
  BUGLPR(dbg_ChgCurShp,BUGNTA,(">ChgCurShp\n"));

  dds = (struct fbdds *) vp->display->odmdds;
  ld = (struct wfg_data *) vp->vttld;

/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */
{
BUGLPR(dbg_vttdefc,BUGGID,("\n"));
BUGLPR(dbg_vttdefc,BUGGID,("Input parameters follow:\n"));
BUGLPR(dbg_vttdefc,BUGGID,("  top    = %d\n",top));
BUGLPR(dbg_vttdefc,BUGGID,("  bottom = %d\n",bottom));
}

  /*****************************************************/
  /*        Begin changing the cursor shape            */
  /*****************************************************/

  /*****************************************************/
  /*   cursor data will be written 8 bits at a time   */
  /*                (see width/=8)                    */
  /*   and all non byte boundry character box widths  */
  /*   will be written to the next byte boundry.      */
  /*                (see width+=8)                    */
  /*   The routine which XORs character  and cursor   */
  /*   will fit the non byte cases exactly by ignoring*/
  /*   the extraneous bits.                           */
  /*****************************************************/

  width = ld->Vttenv.font_box_width;
  height= ld->Vttenv.character_box.height;

  /* ensure that last font size is large enough */
  if (ld->Vttenv.cursor.lastmalloc < (width * height / BYTELEN))
  { if (ld->Vttenv.cursor.data != NULL)
    { /* release old storage, if any */
      xmfree((char *)ld->Vttenv.cursor.data,pinned_heap);
      xmfree((char *)ld->Vttenv.cursor.ichar,pinned_heap);
      ld->Vttenv.cursor.lastmalloc = 0;   /* show no storage is avialable */
    }
    /* create space for cursor bit map */
    ld->Vttenv.cursor.data=(char *)xmalloc(width*height/BYTELEN,3,pinned_heap);
    if (ld -> Vttenv.pse == NULL)
    { wfg_err(vp,"PCIWFG","VTT_DEFC","XMALLOC",ld->Vttenv.cursor.data,WFG_BAD_XMALLOC,
       RAS_UNIQUE_3);
      return(ENOMEM);
    }

    /* create space for cursor-inverted-character bit map */
    ld->Vttenv.cursor.ichar=(char *)xmalloc(width*height/BYTELEN,3,pinned_heap);
    if (ld -> Vttenv.cursor.ichar == NULL)
    { wfg_err(vp,"PCIWFG","VTT_DEFC","XMALLOC",ld->Vttenv.cursor.ichar,WFG_BAD_XMALLOC,
       RAS_UNIQUE_3);
      return(ENOMEM);
    }

    /* record the amount of space stored */
    ld->Vttenv.cursor.lastmalloc = (width * height / BYTELEN);
  }
  if (top < 0) top = height;
  if (bottom < 0) bottom = height;
  bytenum=0;
  /**********************************************************************/
  /*                   Write above Cursor Settings                      */
  /**********************************************************************/
  area = (width * (top));  /* # bits to clear */ 
  if (area > 0)
  {
    for (y=0;y < area/8; y++)
    { /* write as many non-set bytes as possible above the set bytes */
      *((ld->Vttenv.cursor.data)+(bytenum)) = CLRBITS;
      bytenum++;
    }
    /*clear remaining bits and set initial bits for set bytes */
    if (area % 8)
    { *((ld->Vttenv.cursor.data)+(bytenum)) = SETBITS >> (area % 8);
      bytenum++;
    }
    else area=0;

    /**********************************************************************/
    /*                      Write Cursor Settings                         */
    /**********************************************************************/

    /* get number of bits to set (note 8-area%8 bits were already set */
    if (area%8)
    { area = (width * (bottom-top)) - (8 - (area%8));
    }
    else
    { area = (width * (bottom-top));
    }
    if (area > 0)
    {
      for (y=0;y < area/8; y++)
      { /* write as many set bytes as possible above the non-set bytes */
        *((ld->Vttenv.cursor.data)+(bytenum)) = SETBITS;
        bytenum++;
      }
      /*set remaining bits and clear initial bits for clear bytes */
      if (area % 8)
      { *((ld->Vttenv.cursor.data)+(bytenum)) = SETBITS << (8-area% 8);
        bytenum++;
      }
    }
    else area=0;

    /**********************************************************************/
    /*                   Write below Cursor Settings                      */
    /**********************************************************************/

    /* get number of bits to clear (note area%8 bits were already cleared */
    area = (width * (height-bottom)) - (area%8);
    if (area > 0)
    {
      for (y=0;y < area/8; y++)
      { /* write as many non-set bytes as possible before cursor ends  */
        *((ld->Vttenv.cursor.data)+(bytenum)) = CLRBITS;
        bytenum++;
      }
      /*CLR remaining bits and any that exceed the cursor, within byte boundry*/
      if (area % 8)
      { *((ld->Vttenv.cursor.data)+(bytenum)) = CLRBITS;
      }
    }
  }
  BUGLPR(dbg_ChgCurShp,BUGACT,("<ChgCurShp\n"));
} /* End change cursor shape */
