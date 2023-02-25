static char sccsid[] = "@(#)74  1.1  src/rspc/kernext/disp/pcihga/config/configa_diag.c, pcihga, rspc411, GOLD410 4/15/94 18:31:11";
/* config_diag.c */
/*
based on "@(#)69  1.3  src/bos/kernext/disp/wga/config/config_diag.c, bos, bos410 10/28/93 15:37:20";
 *
 *   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
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
/*  Course of Events:                               */
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


BUGVDEF(dbg_cfg, 0);
BUGVDEF(dbg_define, 0);
BUGVDEF(dbg_undefine, 0);
BUGVDEF(dbg_ioctl, 0);
BUGVDEF(dbg_enable, 0);
