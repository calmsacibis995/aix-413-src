/* @(#)89  1.3  src/rspc/kernext/disp/pcigga/inc/gga_reg.h, pcigga, rspc412, 9444b412 11/1/94 12:36:26 */
/* gga_reg.h */
/*
based on @(#)91       1.1  src/bos/kernext/disp/wga/inc/wga_reg.h, bos, bos410 10/28/93 18:28:47
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
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

#ifndef _H_GGA_REG
#define _H_GGA_REG

/****START OF SPECIFICATIONS *******************************************
* FUNCTIONS:         KSR device driver include file
*
* DESCRIPTIVE NAME:  Weitek PCI Adapter Register Offsets
*                    from the base address and possible values
*
* REFERENCES:        Glacier Adapter Specification, 07/05/94
*
*                    Weitek Power 9100 User Interface Controller
*                       March, 1994
*
*                    RGB525 Product Description
*/
#ifndef EIEIO
#define EIEIO __iospace_eieio() /* Enforce Instruction Execution In Order */
#endif

/******************************************************************************/
/****************** PCI Bus Defines for memory mapping ************************/
/******************************************************************************/
#define PCI_MEM_ATT(map_ptr)                                    \
                (unsigned long)iomem_att((map_ptr));		\
                __iospace_eieio()
#define PCI_MEM_DET(eff_addr)                                   \
                iomem_det((void *)(eff_addr))
 
/******** Start of Glacier Addresses ******************************************/

#define PCI_BASE_ADDR	ld->bus_mem_addr
#define WTK_BASE_ADDR   ((PCI_BASE_ADDR)+(0x00060000L)) /*(byte rev.)*/
#define WTK_PARM_BASE   ((PCI_BASE_ADDR)+(0x00062000L)) /*(byte rev.)*/
#define WTK_VRAM_BASE   ((PCI_BASE_ADDR)+(0x00800000L))
#define RGB525_BASE_ADDR ((PCI_BASE_ADDR)+(0x00000200L)) 

/************************************************** End of Glacier Addresses **/

#define FB_PIXEL(x, y)   ((volatile unsigned char *)(WTK_VRAM_BASE + \
                             (( (y) * (ld->mem_shift) ) + (x))))

/******************************************************************************/
/***************** DISPLAY CONTROLLER ADDRESSES/OFFSETS ***********************/
/******************************************************************************/

/*---------  Weitek P9100 System Control Register Offsets  ------------------*/

#define WTK_SYS_CONFIG (volatile unsigned long *)(WTK_BASE_ADDR + 0x4L)
#define WTK_INTR_STAT  (volatile unsigned long *)(WTK_BASE_ADDR + 0x8L)
#define WTK_INTR_ENBL  (volatile unsigned long *)(WTK_BASE_ADDR + 0xCL)

/*-----------  Weitek P9100 Video Control Register Offsets  ------------------*/

#define WTK_HRZT      (volatile unsigned long *)(WTK_BASE_ADDR + 0x108L)
#define WTK_HRZSR     (volatile unsigned long *)(WTK_BASE_ADDR + 0x10CL)
#define WTK_HRZBR     (volatile unsigned long *)(WTK_BASE_ADDR + 0x110L)
#define WTK_HRZBF     (volatile unsigned long *)(WTK_BASE_ADDR + 0x114L)
#define WTK_PREHRZC   (volatile unsigned long *)(WTK_BASE_ADDR + 0x118L)
#define WTK_VRTT      (volatile unsigned long *)(WTK_BASE_ADDR + 0x120L)
#define WTK_VRTSR     (volatile unsigned long *)(WTK_BASE_ADDR + 0x124L)
#define WTK_VRTBR     (volatile unsigned long *)(WTK_BASE_ADDR + 0x128L)
#define WTK_VRTBF     (volatile unsigned long *)(WTK_BASE_ADDR + 0x12CL)
#define WTK_PREVRTC   (volatile unsigned long *)(WTK_BASE_ADDR + 0x130L)
#define WTK_SRTCTL    (volatile unsigned long *)(WTK_BASE_ADDR + 0x138L)
#define WTK_SRTCTL2   (volatile unsigned long *)(WTK_BASE_ADDR + 0x140L)
#define WTK_ENB_VIDEO       0x00000020      /* Enable Video (bit 26)             */
#define WTK_DSB_VIDEO       0xFFFFFFDF      /* Disable video (bit 26)            */

/*------------  Weitek P9100 VRAM Control Register Offsets  ------------------*/
#define WTK_MEMCFG      (volatile unsigned long *)(WTK_BASE_ADDR + 0x184L)
#define WTK_RFPERIOD    (volatile unsigned long *)(WTK_BASE_ADDR + 0x188L)
#define WTK_RLMAX       (volatile unsigned long *)(WTK_BASE_ADDR + 0x190L)
#define WTK_PU_CONFIG   (volatile unsigned long *)(WTK_BASE_ADDR + 0x198L)
#define WTK_PALCUR_W    (volatile unsigned long *)(WTK_BASE_ADDR + 0x200L)
#define WTK_DAC_PALETTE (volatile unsigned long *)(WTK_BASE_ADDR + 0x204L)
#define WTK_PIXEL_MASK	(volatile unsigned long *)(WTK_BASE_ADDR + 0x208L)
#define WTK_INDEXLOW    (volatile unsigned long *)(WTK_BASE_ADDR + 0x210L)
#define WTK_INDEXHIGH   (volatile unsigned long *)(WTK_BASE_ADDR + 0x214L)
#define WTK_INDEXDATA   (volatile unsigned long *)(WTK_BASE_ADDR + 0x218L)

/*---------  Weitek P9100 Parameter Engine Register Offsets  -----------------*/
/*---------------------  Windowing Register Offsets  -------------------------*/
#define WTK_STATUS      (volatile unsigned long *)(WTK_PARM_BASE)
#define WTK_WORIG       (volatile unsigned long *)((WTK_PARM_BASE)+0x0190L)
#define WTK_W_MIN       (volatile unsigned long *)((WTK_PARM_BASE)+0x0194L)
#define WTK_W_MAX       (volatile unsigned long *)((WTK_PARM_BASE)+0x0198L)
#define WTK_BGROUND     (volatile unsigned long *)((WTK_PARM_BASE)+0x0200L)
#define WTK_FGROUND     (volatile unsigned long *)((WTK_PARM_BASE)+0x0204L)
#define WTK_PLANE_MASK  (volatile unsigned long *)((WTK_PARM_BASE)+0x0208L)
#define WTK_DRAW_MODE   (volatile unsigned long *)((WTK_PARM_BASE)+0x020CL)
#define WTK_RASTER	(volatile unsigned long *)((WTK_PARM_BASE)+0x0218L)
#define WTK_P_W_MIN     (volatile unsigned long *)((WTK_PARM_BASE)+0x0220L)
#define WTK_P_W_MAX     (volatile unsigned long *)((WTK_PARM_BASE)+0x0224L)
#define WTK_B_W_MIN     (volatile unsigned long *)((WTK_PARM_BASE)+0x02A0L)
#define WTK_B_W_MAX     (volatile unsigned long *)((WTK_PARM_BASE)+0x02A4L)

/*---------------------  RGB525 Register Indices  ---------------------*/

#define RGB525_MISC_CLK     0x0002
#define RGB525_SYNC_CTL     0x0003
#define RGB525_SYNC_POS     0x0004
#define RGB525_PWR_MGT      0x0005
#define RGB525_DAC_OP       0x0006
#define RGB525_PALETTE_CTL  0x0007
#define RGB525_PIXEL_FMT    0x000a
#define RGB525_8BPP_CTL     0x000b
#define RGB525_16BPP_CTL    0x000c
#define RGB525_24BPP_CTL    0x000d
#define RGB525_32BPP_CTL    0x000e
#define RGB525_PLL_CTL1     0x0010
#define RGB525_PLL_CTL2     0x0011
#define RGB525_PLL_DIV      0x0014
#define RGB525_F0           0x0020
#define RGB525_F1           0x0021
#define RGB525_F2           0x0022
#define RGB525_F3           0x0023
#define RGB525_F4           0x0024
#define RGB525_F5           0x0025
#define RGB525_F6           0x0026
#define RGB525_F7           0x0027
#define RGB525_F8           0x0028
#define RGB525_F9           0x0029
#define RGB525_F10          0x002a
#define RGB525_F11          0x002b
#define RGB525_F12          0x002c
#define RGB525_F13          0x002d
#define RGB525_F14          0x002e
#define RGB525_F15          0x002f
#define RGB525_CUR_CTL      0x0030
#define RGB525_CUR_X_LOW    0x0031
#define RGB525_CUR_X_HIGH   0x0032
#define RGB525_CUR_Y_LOW    0x0033
#define RGB525_CUR_Y_HIGH   0x0034
#define RGB525_CUR_X_HOT    0x0035
#define RGB525_CUR_Y_HOT    0x0036
#define RGB525_CUR_1_RED    0x0040
#define RGB525_CUR_1_GREEN  0x0041
#define RGB525_CUR_1_BLUE   0x0042
#define RGB525_CUR_2_RED    0x0043
#define RGB525_CUR_2_GREEN  0x0044
#define RGB525_CUR_2_BLUE   0x0045
#define RGB525_CUR_3_RED    0x0046
#define RGB525_CUR_3_GREEN  0x0047
#define RGB525_CUR_3_BLUE   0x0048
#define RGB525_BORDER_RED   0x0060
#define RGB525_BORDER_GREEN 0x0061
#define RGB525_BORDER_BLUE  0x0062
#define RGB525_MISC_CTL1    0x0070
#define RGB525_MISC_CTL2    0x0071
#define RGB525_MISC_CLT3    0x0072
#define RGB525_DAC_SENSE    0x0082
#define RGB525_MISR_RED     0x0084
#define RGB525_MISR_GREEN   0x0086
#define RGB525_MISR_BLUE    0x0088
#define RGB525_PLL_VCO      0x008e
#define RGB525_PLL_REF      0x008f
#define RGB525_VRAM_MASKH   0x0090
#define RGB525_VRAM_MASKL   0x0091
#define RGB525_CURSOR_ARRAY 0x0100

#endif /* _H_GGA_REG */
