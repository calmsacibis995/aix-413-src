static char sccsid[] = "@(#)13  1.2  src/rspc/kernext/disp/pciwfg/ksr/ksr_diag.c, pciwfg, rspc411, 9437B411a 9/8/94 06:31:44";
/* ksr_diag.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: none
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

/****************************************************/
/*  Debug printout action levels:                   */
/*    BUGNFO   Level 1  general info                */
/*    BUGACT   Level 2  Program action              */
/*    BUGNTF   Level 3  Names, Data, rc's           */
/*    BUGNTA   Level 4  interface subroutine routes */
/*    BUGNTX   Level 5  detailed interface data     */
/*    BUGGID   Level 99 Gory Internal Details       */
/****************************************************/
/*  Couse of Events:                                */
/*    BUGVDEF(variable, level_setting);             */
/*      This sets the original debug level for the  */
/*      debug variable.  This is done in diag.c     */
/*      for easy access, change, and recompile.     */
/*    BUGXDEF(variable);                            */
/*      This defines the debug variable as an       */
/*      external.  This is done in the routine in   */
/*      which the variable is used.                 */
/*    BUGLPR(variable, level, ("print statement",   */
/*                              print variables));  */
/*      This is the use of the debug statement.     */
/*      If the level is < the level set in BUGVDEF  */
/*      then the print statement is executed.       */
/****************************************************/
/*  Include Files:                                  */
/*    <sys/syspest.h>   is the main include file    */
/*    <sys/errids.h>    may be needed               */
/*    <sys/sysmacros.h> may be needed               */
/****************************************************/
#include <sys/syspest.h>
#include <sys/errids.h>
#include <sys/sysmacros.h>
#include <sys/types.h>

BUGVDEF(dbg_ChgCurShp, 0);
BUGVDEF(dbg_CopyScanLines, 0);
BUGVDEF(dbg_asyncmask, 0);
BUGVDEF(dbg_clrlines, 0);
BUGVDEF(dbg_colex, 0);
BUGVDEF(dbg_copyps, 0);
BUGVDEF(dbg_define, 0);
BUGVDEF(dbg_draw, 0);
BUGVDEF(dbg_drawch, 0);
BUGVDEF(dbg_drawtxt, 0);
BUGVDEF(dbg_setup_powermgt,0);
BUGVDEF(dbg_disp_powermgt,0);
BUGVDEF(dbg_load_palette,0);
BUGVDEF(dbg_load_vram,0);
BUGVDEF(dbg_reset, 0);
BUGVDEF(dbg_makegp, 0);
BUGVDEF(dbg_unmakegp, 0);

BUGVDEF(dbg_vttact, 0);
BUGVDEF(dbg_vttcfl, 0);
BUGVDEF(dbg_vttclr, 0);
BUGVDEF(dbg_vttcpl, 0);
BUGVDEF(dbg_vttdact, 0);
BUGVDEF(dbg_vttdefc, 0);
BUGVDEF(dbg_vttinit, 0);
BUGVDEF(dbg_vttmovc, 0);
BUGVDEF(dbg_vttrds, 0);
BUGVDEF(dbg_vttscr, 0);
BUGVDEF(dbg_vttsetm, 0);
BUGVDEF(dbg_vttstct, 0);
BUGVDEF(dbg_vttdpm, 0);
BUGVDEF(dbg_vttterm, 0);
BUGVDEF(dbg_vtttext, 0);
BUGVDEF(dbg_wfg_err, 0);
