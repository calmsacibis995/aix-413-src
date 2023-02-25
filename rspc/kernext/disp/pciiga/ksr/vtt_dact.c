static char sccsid[] = "@(#)64	1.2  src/rspc/kernext/disp/pciiga/ksr/vtt_dact.c, pciiga, rspc41B, 9504A 12/15/94 16:59:51";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
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

#include "iga_INCLUDES.h"
BUGXDEF(dbg_vttdact);
GS_MODULE(iga_vttdact);

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
  struct iga_data *ld;
  struct iga_ddf *ddf;

/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */

{
BUGLPR(dbg_vttdact,BUGGID,("\n"));
BUGLPR(dbg_vttdact,BUGGID,("Input parameters follow:\n"));

BUGLPR(dbg_vttdact,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttdact,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));
BUGLPR(dbg_vttdact,BUGGID,("vp->display       = 0x%x\n",vp->display));
BUGLPR(dbg_vttdact,BUGGID,("vp->display->free_area = 0x%x\n",vp->display->free_area));
}

  ld = (struct iga_data *) vp->vttld;
  ddf = (struct iga_ddf *) vp->display->free_area;

  IGA_ENTER_TRC0(ddf,iga_vttdact,2,IGA_VTTDACT);

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

  IGA_EXIT_TRC0(ddf,iga_vttdact,2,IGA_VTTDACT);

  return(0);
}                                 /* end  of  vttdact                 */
