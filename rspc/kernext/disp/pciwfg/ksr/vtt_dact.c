static char sccsid[] = "@(#)21  1.1  src/rspc/kernext/disp/pciwfg/ksr/vtt_dact.c, pciwfg, rspc411, 9434B411a 8/24/94 07:52:42";
/* vtt_dact.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttdact
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
BUGXDEF(dbg_vttdact);
/**********************************************************************/
/*                                                                    */
/*  IDENTIFICATION: VTTDACT                                           */
/*                                                                    */
/*  DESCRIPTIVE NAME:  Deactivate Command for the                     */
/*                       Virtual Display Driver (VDD)                 */
/*                                                                    */
/*  FUNCTION:     Set the Vttenv mode to inactive.                    */
/*                                                                    */
/*                If adapter has a FIFO interface wait for it to be   */
/*                empty.                                              */
/*                                                                    */
/*                If VT is currently in monitored mode:               */
/*                  - Detach the paths from the Mode Processor and    */
/*                    SLIH                                            */
/*                                                                    */
/*  INPUTS:       None.                                               */
/*                                                                    */
/*  OUTPUTS:      DETACH_FAILED                                       */
/*                                                                    */
/*  CALLED BY:    Mode Processor                                      */
/*                                                                    */
/*  CALLS:        detach                                              */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

long vttdact(vp)
struct vtmstruc *vp;

{

  ushort i;
  struct wfg_data *ld;
  struct wfg_ddf *ddf;

/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */

{
BUGLPR(dbg_vttdact,BUGNTA,("\n"));
BUGLPR(dbg_vttdact,BUGNTA,("Input parameters follow:\n"));

BUGLPR(dbg_vttdact,BUGNTA,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttdact,BUGNTA,("vp->vttld         = 0x%x\n",vp->vttld));
BUGLPR(dbg_vttdact,BUGNTA,("vp->display       = 0x%x\n",vp->display));
BUGLPR(dbg_vttdact,BUGNTA,("vp->display->free_area = 0x%x\n",vp->display->free_area));
}

  ld = (struct wfg_data *) vp->vttld;
  ddf = (struct wfg_ddf *) vp->display->free_area;

  VDD_TRACE(DACT , ENTRY, vp);

  /*****************************************************
  * set the state of the virtual terminal to inactive *
  ****************************************************/

  ld->Vttenv.vtt_active = VTT_INACTIVE;
  ld->Vttenv.current_attr = 0xffff;
  ld->cur_font = -1;
  ld->resv = 0x0;


  VDD_TRACE(DACT , EXIT, vp);
  /*BUGLPR(dbg_vttdact,BUGNTA,("<DACT(0)\n"));*/
  return(0);
}                                 /* end  of  vttdact                 */
