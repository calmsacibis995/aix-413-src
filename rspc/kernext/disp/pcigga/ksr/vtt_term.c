static char sccsid[] = "@(#)20  1.3  src/rspc/kernext/disp/pcigga/ksr/vtt_term.c, pcigga, rspc41B, 9504A 1/9/95 13:40:31";
/* vtt_term.c */
/*
based on "@(#)61  1.3  src/bos/kernext/disp/wga/ksr/vtt_term.c, bos, bos410 10/28/93 18:03:27";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttterm
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
BUGXDEF(dbg_vttterm);
GS_MODULE(gga_vttterm);

/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: VTTTERM                                             */
/*                                                                     */
/* DESCRIPTIVE NAME: Terminate virtual terminal                        */
/*                                                                     */
/* FUNCTION:  This procedure deallocates the resources associated with */
/*              a given virtual terminal.                              */
/*                                                                     */
/* INPUTS:    Virtual terminal pointer                                 */
/*                                                                     */
/* OUTPUTS:   None                                                     */
/*                                                                     */
/* CALLED BY: Mode processor                                           */
/*                                                                     */
/* CALLS:     None.                                                    */
/*                                                                     */
/***********************************************************************/

long vttterm(vp)

struct vtmstruc *vp;
{
  struct gga_data *ld;
  struct gga_ddf *ddf;

/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */

{
BUGLPR(dbg_vttterm,BUGGID,("\n"));
BUGLPR(dbg_vttterm,BUGGID,("Input parameters follow:\n"));
BUGLPR(dbg_vttterm,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttterm,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));
BUGLPR(dbg_vttterm,BUGGID,("vp->display       = 0x%x\n",vp->display));
BUGLPR(dbg_vttterm,BUGGID,("vp->display->free_area = 0x%x\n",vp->display->free_area));
}

  ld = (struct gga_data *) vp->vttld;
  ddf = (struct gga_ddf *) vp->display->free_area;

  GGA_ENTER_TRC0(ddf,gga_vttterm,2,GGA_VTTTERM);

  VDD_TRACE(TERM , ENTRY, vp);

  ld->resv = 0x0;

  /*******************************
  * free the presentation space *
  ******************************/
  xmfree(ld->Vttenv.pse, pinned_heap);

  /*-----------------------------
     free cursor structures
    -----------------------------*/
  xmfree( ld->Vttenv.cursor.data, pinned_heap );
  xmfree( ld->Vttenv.cursor.ichar, pinned_heap );

  /*******************************
  * free the local data area    *
  ******************************/
  xmfree(ld, pinned_heap);

  vp->vttld = NULL;
  vp->display->usage -= 1;

  vp->display->current_dpm_phase = 0;   /* Display Power Management */

  VDD_TRACE(TERM , EXIT, vp);

  GGA_EXIT_TRC0(ddf,gga_vttterm,2,GGA_VTTTERM);

  /*BUGLPR(dbg_vttterm,BUGGID, ("Leaving TERM\n"));*/
  return(0);
} /* end  of  vttterm */
