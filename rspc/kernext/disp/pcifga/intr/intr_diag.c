static char sccsid[] = "@(#)12  1.1  src/rspc/kernext/disp/pcifga/intr/intr_diag.c, pcifga, rspc411, GOLD410 4/15/94 18:39:24";
/* intr_diag.c */
/*
based on "@(#)74        1.2  src/bos/kernext/disp/wga/intr/intr_diag.c, bos, bos410 3/16/93 20:44:12";
 *
 *   COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
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

BUGVDEF(dbg_intr, 0);
