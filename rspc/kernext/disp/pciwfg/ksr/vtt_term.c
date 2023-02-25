static char sccsid[] = "@(#)29  1.3  src/rspc/kernext/disp/pciwfg/ksr/vtt_term.c, pciwfg, rspc41J, 9517A_all 4/24/95 04:12:52";
/* vtt_term.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttterm
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "wfg_INCLUDES.h"

#ifdef  PANEL_PROTECTION
#define pm_planar_control(a,b,c)        wfg_planar_control(ddf,dds,a,b,c)
#endif  /* PANEL_PROTECTION */

BUGXDEF(dbg_vttterm);
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
  struct wfg_data  *ld;
  struct wfg_ddf   *ddf;
  struct pm_handle *pmh;
  struct phys_displays *pd;
  int               rc;

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

  ld = (struct wfg_data *) vp->vttld;
  ddf = (struct wfg_ddf *) vp->display->free_area;
  pmh = (struct pm_handle *) ld->wfg_pmh;
  pd  = (struct phys_displays *) vp->display;

  VDD_TRACE(TERM , ENTRY, vp);

#ifdef  PANEL_PROTECTION
  /* ========================================= *
   *    Disable CRT & LCD for H/W protection   *
   * ========================================= */
  device_power ( pd, CRT, PM_PLANAR_OFF );
  device_power ( pd, LCD, PM_PLANAR_OFF );
#endif /* PANEL_PROTECTION */

#ifdef PM_SUPPORT
  /* Unregister Power Management handle */
  if ( ld->pm_register == REGISTERED ) {
      rc = pm_register_handle( pmh, PM_UNREGISTER );
      if (rc != PM_SUCCESS ) {
            BUGLPR (dbg_setup_powermgt, BUGNTA,
                                   ("pm_register_handle failure\n"));
            wfg_err (NULL, "PCIWFG","SETUP_POWERMGT","PM_REGISTER_HANDLE",
                          pmh, WFG_PM_ERROR, RAS_UNIQUE_1);
      }
      xmfree( pmh, pinned_heap );
  }
#endif /* PM_SUPPORT */

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

  VDD_TRACE(TERM , EXIT, vp);

  /*BUGLPR(dbg_vttterm,BUGNTA, ("Leaving TERM\n"));*/
  return(0);
} /* end  of  vttterm */
