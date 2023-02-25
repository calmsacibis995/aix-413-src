/* @(#)91  1.2 src/rspc/kernext/disp/pciwfg/inc/wfg_reg.h, pciwfg, rspc41J, 9507C 1/27/95 03:10:10 */
/* wfg_reg.h */
/*
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
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _WFG_REG
#define _WFG_REG

/****START OF SPECIFICATIONS *******************************************
* FUNCTIONS:         KSR device driver include file
*
* DESCRIPTIVE NAME:  Western Digital PCI Adapter Register Offsets
*                    from the base address and possible values
*
* REFERENCES:        Woodfield Specification, 3/31/94
*
*                    Western Digital 90C24A2 LCD Controller, 10/25/93
*
*/
#define EIEIO __iospace_eieio() /* Enforce Instruction Execution In Order */

/******************************************************************************/
/****************** PCI Bus Defines for memory mapping ************************/
/******************************************************************************/
/*                    Sandalfoot                                              */
/*----------------------------------------------------------------------------*/
/* PowerPC addresses of 0x80000000 -- 0x807fffff are in PCI/ISA I/O space     */
/*    with addresses of 0x00000000 -- 0x007fffff on the PCI bus               */
/*----------------------------------------------------------------------------*/
/* PowerPC addresses of 0x80000000 -- 0xbf7fffff are in PCI I/O space         */
/*    with addresses of 0x00000000 -- 0x3f7fffff on the PCI bus               */
/*----------------------------------------------------------------------------*/
/* PowerPC addresses of 0x80800000 -- 0x80ffffff are in PCI Config space      */
/*    with addresses of 0x00800000 -- 0x00ffffff on the PCI bus               */
/*----------------------------------------------------------------------------*/
/* PowerPC addresses of 0xc0000000 -- 0xfeffffff are in PCI Mem space         */
/*    with addresses of 0x00000000 -- 0x3effffff on the PCI bus               */
/******************************************************************************/
/*                    Woodfield                                               */
/*----------------------------------------------------------------------------*/
/* PowerPC addresses of 0x80000000 -- 0x807fffff are in PCI/ISA I/O space     */
/*    with addresses of 0x00000000 -- 0x007fffff on the PCI bus               */
/*----------------------------------------------------------------------------*/
/* PowerPC addresses of 0x80800000 -- 0xbfffffff are in PCI I/O space         */
/*    with addresses of 0x00000000 -- 0x3fffffff on the PCI bus               */
/*----------------------------------------------------------------------------*/
/* PowerPC addresses of 0x80800000 -- 0x80ffffff are in PCI Config space      */
/*    with addresses of 0x00800000 -- 0x00ffffff on the PCI bus               */
/*----------------------------------------------------------------------------*/
/* PowerPC addresses of 0xc0000000 -- 0xc0ffffff are in PCI/ISA Mem space     */
/*    with addresses of 0x00000000 -- 0x00ffffff on the PCI bus               */
/*----------------------------------------------------------------------------*/
/* PowerPC addresses of 0xc1000000 -- 0xfeffffff are in PCI Mem space         */
/*    with addresses of 0x01000000 -- 0x3effffff on the PCI bus               */
/*----------------------------------------------------------------------------*/

#define PCI_BUS_ATT(map_ptr)				\
                (unsigned long)iomem_att((map_ptr));	\
                EIEIO;
#define PCI_BUS_DET(eff_addr)                           \
                iomem_det((void *)(eff_addr));                  

#define FB_PIXEL(x, y)   ((volatile unsigned char *)                           \
	       ((ld->bus_base_addr) + ((y + ld->offset_y)*(ld->vert_stride)) + \
	       (unsigned long)(x + ld->offset_x)) )
#endif  /* _WFG_REG */
