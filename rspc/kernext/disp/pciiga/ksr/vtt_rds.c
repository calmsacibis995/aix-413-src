static char sccsid[] = "@(#)68	1.2  src/rspc/kernext/disp/pciiga/ksr/vtt_rds.c, pciiga, rspc41B, 9504A 12/15/94 17:37:46";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttrds
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
BUGXDEF (dbg_vttrds);
GS_MODULE(iga_vttrds);

/*********************************************************************/
/*                                                                   */
/*   IDENTIFICATION: VTTRDS                                          */
/*                                                                   */
/*   DESCRIPTIVE name: Read Screen Segment                           */
/*                                                                   */
/*   FUNCTION:  Read the entire or partial content of the            */
/*                presentation space and convert each entry into a   */
/*                two-byte display code and a two-byte attribute     */
/*                code.                                              */
/*                                                                   */
/*   INPUTS:    Virtual terminal structure pointer containing cursor */
/*                position.                                          */
/*                Pointer and length of display symbol array         */
/*                Pointer and length of display attributes array     */
/*                Row and columns of requested area to read          */
/*                                                                   */
/*   OUTPUTS:   EIO                                                  */
/*                INVALID_CURSOR_POSITION                            */
/*                                                                   */
/*   CALLED BY: Mode Processor                                       */
/*                                                                   */
/*   CALLS:     None.                                                */
/*                                                                   */
/*********************************************************************/


long vttrds(vp,ds,ds_size,attr,attr_size,rc)
struct vtmstruc *vp;
ushort *ds;               /* array of display symbols     */
                          /* returned by this procedure   */
long   ds_size;           /* size of ds array             */
ushort *attr;             /* array of attributes returned */
                          /* by this procedure            */
long   attr_size;         /* size of attr array           */
struct vtt_rc_parms *rc;  /* string position and length   */

{

  ulong  *buf_addr,tmp_full;
  long   i,
         buf_offset;
  ushort tmp_attr,tmp_char;
  struct iga_data *ld;
  struct iga_ddf *ddf;

/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */

{
BUGLPR(dbg_vttrds,BUGGID,("\n"));
BUGLPR(dbg_vttrds,BUGGID,("Input parameters follow:\n"));
BUGLPR(dbg_vttrds,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttrds,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));
}

  ddf = (struct iga_ddf *) vp->display->free_area;

  IGA_ENTER_TRC0(ddf,iga_vttrds,2,IGA_VTTRDS);

  ld = (struct iga_data *) vp->vttld;

  if /*-----------------------*/
     /* not in character mode */
     /*-----------------------*/
      (ld->Vttenv.vtt_mode != KSR_MODE)
  {
     /*------------------------------------*/
     /* Only valid in character (KSR) mode */
     /*------------------------------------*/
     IGA_EXIT_TRC1(ddf,iga_vttrds,2,IGA_VTTRDS,EIO);
     return(EIO);
  }

  VDD_TRACE(RDS , ENTRY, vp);

  /******************************
   * read the presentation space *
   ******************************/

  buf_addr = ld->Vttenv.pse;

  /*****************************************************************
   * calculate the address of the first character that must be read *
   *****************************************************************/
  buf_offset = ((rc->start_row - 1) * ld->Vttenv.ps_size.wd +
                    (rc->start_column - 1));

  for /*********************************************************
      * all characters that must be read from the frame buffer *
      *********************************************************/

      (i=0; i < (rc->string_length); i++)
  {
      /*************************************************************
       * read a character from the buffer and convert the character *
       * code into a two-byte display code and the attribute into a *
       * two-byte attribute code                                      *
       *************************************************************/

     tmp_full = *(ulong *)(buf_addr +
             ((buf_offset + ld->Scroll_offset) % ld->Vttenv.ps_words));
     buf_offset++;

     /******************************************************
      * transform the pcm character code into a 2-byte code *
      ******************************************************/

     /***************************************
      * the display code equals the pcm code *
      ***************************************/
     ds[i] = tmp_full >> 16;

     /******************************
      * set the attribute bytes (2) *
      ******************************/
     attr[i] = tmp_full & 0x0000ffff;
  }                /* end of for loop */

  VDD_TRACE(RDS , EXIT, vp);

  IGA_ENTER_TRC0(ddf,iga_vttrds,2,IGA_VTTRDS);

  return(0);
} /* end of vttrds  */
