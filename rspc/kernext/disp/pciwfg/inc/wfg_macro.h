/* @(#)89  1.1  src/rspc/kernext/disp/pciwfg/inc/wfg_macro.h, pciwfg, rspc411, 9434B411a 8/24/94 07:34:37 */
/* wfg_macro.h */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS:
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

#ifndef _WFG_MACRO
#define _WFG_MACRO

#include "wfgmisc.h"
/*************************************************************
*  Sets up a change in attributes by:                        *
*     - loading the ld->fgcol and ld->bgcol variables with   *
*       the new foreground and background colors             *
*************************************************************/
#define SET_CHAR_ATTRIB(ld,new,tmp)                          \
{                                                            \
  SELECT_NEW_FONT(ld,new,tmp);                               \
  SELECT_NEW_BGCOL(ld,new);                                  \
  SELECT_NEW_FGCOL(ld,new);                                  \
  ld->Vttenv.current_attr=new;                               \
}

#define SELECT_NEW_FGCOL(ld,char_attr)                       \
 ld->colset = FALSE;                                         \
 ld->fgcol = ((char_attr << 16) >> 28);

#define SELECT_NEW_BGCOL(ld,char_attr)                       \
 ld->colset = FALSE;                                         \
 ld->old_bgcol = ld->bgcol;                                  \
 ld->bgcol = ((char_attr << 20) >> 28);

#define SELECT_NEW_FONT(ld,char_attr,tmp)                    \
 tmp = ((char_attr << 24) >> 29);                            \
 tmp = tmp & 0x00000007;                                     \
 if (tmp != ld->cur_font)                                    \
 {                                                           \
   (void)load_font(vp,tmp);                                  \
   ld->cur_font = tmp;                                       \
 }

/******************************************************************************
*                                                                             *
*       Define the PCI Configuration Space access macros                      *
*                                                                             *
******************************************************************************/
/*----------------------------------------------------------------------
|
|       FUNCTION DESCRIPTION
|
|   These functions are ONLY for accessing the PCI Standard
|   Configuration Space.  All reads are relative to the beginning
|   of the configuration space for the bus/slot.
|
|----------------------------------------------------------------------*/
#define get_pci_cfg_long( bus_num, bus_slot, cfg_offset, data_addr ) \
        rw_pci_cfg_regs((bus_slot),(cfg_offset),(data_addr),(bus_num), 4, 0 )

#define put_pci_cfg_long( bus_num, bus_slot, cfg_offset, data_addr ) \
        rw_pci_cfg_regs((bus_slot),(cfg_offset),(data_addr),(bus_num), 4, 1 )

#define get_pci_cfg_short( bus_num, bus_slot, cfg_offset, data_addr ) \
        rw_pci_cfg_regs((bus_slot),(cfg_offset),(data_addr),(bus_num), 2, 0 )

#define put_pci_cfg_short( bus_num, bus_slot, cfg_offset, data_addr ) \
        rw_pci_cfg_regs((bus_slot),(cfg_offset),(data_addr),(bus_num), 2, 1 )

#endif /* _WFG_MACRO */
