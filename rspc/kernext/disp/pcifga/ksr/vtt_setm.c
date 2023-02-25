static char sccsid[] = "@(#)31  1.1  src/rspc/kernext/disp/pcifga/ksr/vtt_setm.c, pcifga, rspc411, GOLD410 4/15/94 18:42:08";
/* vtt_setm.c */
/*
based on "@(#)59  1.3  src/bos/kernext/disp/wga/ksr/vtt_setm.c, bos, bos410 10/28/93 18:02:15";
 *
 * COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttsetm
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

#include "fga_INCLUDES.h"
BUGXDEF(dbg_vttsetm);
/*********************************************************************/
/*                                                                   */
/*   IDENTIFICATION: VTTSETM                                         */
/*                                                                   */
/*   DESCRIPTIVE name:        Setmode Routine                        */
/*                                                                   */
/*   FUNCTION: This command sets the mode of the VDD to either       */
/*               character or monitored mode.                        */
/*                                                                   */
/*               If the VDD is active and the mode is character      */
/*               this command:                                       */
/*                  - initializes the adapter (if it was previously  */
/*                    in monitored mode or never previously          */
/*                    initialized)                                   */
/*                  - sets the VDD and adapter mode  to character    */
/*                  - copies the current contents of the             */
/*                    VDD's presentation space into the frame buffer */
/*                    of the CGA A/N adapter.                        */
/*                  - establishes the shape and position of the      */
/*                    hardware cursor.                               */
/*                                                                   */
/*               If the VDD is active and the mode is monitored,     */
/*               this command:                                       */
/*                  - copies the current contents of the             */
/*                    frame buffer of the CGA A/N adapter into the   */
/*                    VDD's presentation space.                      */
/*                  - sets the adapter mode to monitored             */
/*                  - detaches the Resource Controller from the      */
/*                    CGA A/N SLIH                                   */
/*                                                                   */
/*   INPUTS:        Virtual terminal structure pointer               */
/*                Mode to set terminal to                            */
/*                                                                   */
/*   OUTPUTS:   EIO                                                  */
/*                                                                   */
/*   CALLED BY: Mode Processor                                       */
/*                                                                   */
/*   CALLS:        vttact                                            */
/*                                                                   */
/*********************************************************************/

long vttsetm(vp,mode)
struct vtmstruc *vp;
long         mode;     /* adapter mode: 0=> monitored,*/
                       /* 1=> character, 2=> APA.     */
{
  struct fga_data *ld;

#if 0
/*
 * The following is a debug print block intended to display all input parameters
 * This block is usually #if 0 out
 */

{
BUGLPR(dbg_vttsetm,BUGGID,("\n"));
BUGLPR(dbg_vttsetm,BUGGID,("Input parameters follow:\n"));
BUGLPR(dbg_vttsetm,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttsetm,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));
}
#endif

  ld = (struct fga_data *) vp->vttld;

  VDD_TRACE(SETM , ENTRY, vp);

  if /************************************************
      * the requested mode is character or monitored *
      ************************************************/
     ((mode == KSR_MODE) || (mode == GRAPHICS_MODE))
  {
     /*****************************************
     * set the VDD state to the specifed mode *
     ******************************************/

     ld->Vttenv.vtt_mode = mode;

     if /*--------------------------------*/
        /* the virtual terminal is active */
        /*--------------------------------*/
         (ld->Vttenv.vtt_active)
     {
        /************************
        * Call activate routine *
        ************************/
        /*BUGLPR(dbg_vttsetm,BUGGID, ("SETM is calling VTTACT\n"));*/
        vttact(vp);
        /*BUGLPR(dbg_vttsetm,BUGGID, ("SETM has returned from VTTACT\n"));*/
     }
  }
                else
                {
                                return(-1);
                }
  VDD_TRACE(SETM , EXIT, vp);
  /*BUGLPR(dbg_vttsetm,BUGGID, ("Leaving SETM\n"));*/
  return(0);
} /* end  of  set mode */
