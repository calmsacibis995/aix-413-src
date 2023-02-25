/* @(#)63  1.4  src/rspc/kernext/disp/pcihga/inc/hga_reg.h, pcihga, rspc411, 9432B411a 8/8/94 14:04:39 */
/* hga_reg.h */
/*
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

#ifndef _HGA_REG
#define _HGA_REG

/****START OF SPECIFICATIONS *******************************************
* FUNCTIONS:         KSR device driver include file
*
* DESCRIPTIVE NAME:  S3 PCI Adapter Register Offsets
*                    from the base address and possible values
*
* REFERENCES:        Homestake Adapter Specification, 8/20/93
*
*                    S3 86C928 GUI Accelerator, September, 1992
*
*                    Brooktree Bt485 Product Description L485001 B
*/
#ifndef EIEIO
#define EIEIO __iospace_eieio() /* Enforce Instruction Execution In Order */
#endif

/******************************************************************************/
/****************** PCI Bus Defines for memory mapping ************************/
/******************************************************************************/
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
/*----------------------------------------------------------------------------*/

#define PCI_BUS_ATT(map_ptr)					\
                (unsigned long)iomem_att((map_ptr));		
#define PCI_BUS_DET(eff_addr)                                   \
                iomem_det((void *)(eff_addr))

#define FB_PIXEL(x, y)   ((volatile unsigned char *)(ld->bus_base_addr) + \
                             (((y)*(ld->vert_stride)) + (x)))

#endif  /* _HGA_REG */
