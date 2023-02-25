/* @(#)02  1.2  src/rspc/kernext/disp/pcifga/inc/fga_reg.h, pcifga, rspc411, GOLD410 4/21/94 18:20:04 */
/* fga_reg.h */
/*
based on @(#)91       1.1  src/bos/kernext/disp/wga/inc/wga_reg.h, bos, bos410 10/28/93 18:28:47
 *
 * COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
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

#ifndef _H_FGA_REG
#define _H_FGA_REG

/****START OF SPECIFICATIONS *******************************************
* FUNCTIONS:         KSR device driver include file
*
* DESCRIPTIVE NAME:  Weitek PCI Adapter Register Offsets
*                    from the base address and possible values
*
* REFERENCES:        Fairway Adapter Specification, 6/24/93
*
*                    Weitek Power 9000 User Interface Controller
*                       October, 1992
*
*                    Brooktree Bt485 Product Description L485001 B
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

/******** Start of Fairway Addresses ******************************************/
#define FGA_CNTL_BASE   ((ld->bus_mem_addr)+(0x00000000L)) /*0x00000000*/
#define BT485_BASE_ADDR ((ld->bus_mem_addr)+(0x00080000L)) /*0x00080000*/
#define WTK_BASE_ADDR   ((ld->bus_mem_addr)+(0x00160000L)) /*0x00160000 (byte rev.)*/
#define WTK_PARM_BASE   ((ld->bus_mem_addr)+(0x001e0000L)) /*0x001e0000 (byte rev.)*/
#define WTK_VRAM_BASE   ((ld->bus_mem_addr)+(0x00200000L)) /*0x00200000*/
/************************************************** End of Fairway Addresses **/

#define WTK_4PIXELS(x, y)  ((volatile unsigned long *)(WTK_VRAM_BASE) + \
                             (((y)<<(ld->mem_shift)) | (x)))  /* 4 pixels word aligned */
#define WTK_2PIXELS(x, y)  ((volatile unsigned short *)(WTK_VRAM_BASE) + \
                             (((y)<<(ld->mem_shift+1)) | (x)))
#define FB_PIXEL(x, y)   ((volatile unsigned char *)(WTK_VRAM_BASE) + \
                             (((y)<<(ld->mem_shift+2)) | (x)))

/******************************************************************************/
/***************** DISPLAY CONTROLLER ADDRESSES/OFFSETS ***********************/
/******************************************************************************/

/*----------  Fairway Adapter Control/Sense Register Offsets  ----------------*/
#define FGA_ADCNTL            (volatile unsigned char *)(FGA_CNTL_BASE + 0x00L)
#define FGA_ADSTAT            (volatile unsigned char *)(FGA_CNTL_BASE + 0x04L)

/*---------  Weitek P9000 System Control Register Offsets  ------------------*/

#define WTK_SYS_CONFIG (volatile unsigned long *)(WTK_BASE_ADDR + 0x4)
#define WTK_INTR_STAT  (volatile unsigned long *)(WTK_BASE_ADDR + 0x8)
#define WTK_INTR_ENBL  (volatile unsigned long *)(WTK_BASE_ADDR + 0xc)

/*-----------  Weitek P9000 Video Control Register Offsets  ------------------*/

#define WTK_HRZT      (volatile unsigned long *)(WTK_BASE_ADDR + 0x108)
#define WTK_HRZSR     (volatile unsigned long *)(WTK_BASE_ADDR + 0x10C)
#define WTK_HRZBR     (volatile unsigned long *)(WTK_BASE_ADDR + 0x110)
#define WTK_HRZBF     (volatile unsigned long *)(WTK_BASE_ADDR + 0x114)
#define WTK_PREHRZC   (volatile unsigned long *)(WTK_BASE_ADDR + 0x118)
#define WTK_VRTT      (volatile unsigned long *)(WTK_BASE_ADDR + 0x120)
#define WTK_VRTSR     (volatile unsigned long *)(WTK_BASE_ADDR + 0x124)
#define WTK_VRTBR     (volatile unsigned long *)(WTK_BASE_ADDR + 0x128)
#define WTK_VRTBF     (volatile unsigned long *)(WTK_BASE_ADDR + 0x12C)
#define WTK_PREVRTC   (volatile unsigned long *)(WTK_BASE_ADDR + 0x130)
#define WTK_SRTCTL    (volatile unsigned long *)(WTK_BASE_ADDR + 0x138)
#define WTK_ENB_VIDEO       0x00000020      /* Enable Video (bit 26)             */
#define WTK_DSB_VIDEO       0xFFFFFFDF      /* Disable video (bit 26)            */

/*------------  Weitek P9000 VRAM Control Register Offsets  ------------------*/
#define WTK_MEMCFG      (volatile unsigned long *)(WTK_BASE_ADDR + 0x184)
#define WTK_RFPERIOD    (volatile unsigned long *)(WTK_BASE_ADDR + 0x188)
#define WTK_RLMAX       (volatile unsigned long *)(WTK_BASE_ADDR + 0x190)

/*---------  Weitek P9000 Parameter Engine Register Offsets  -----------------*/
/*---------------------  Windowing Register Offsets  -------------------------*/
#define WTK_WORIG       (volatile unsigned long *)((WTK_PARM_BASE)+0x0190)
#define WTK_FGROUND     (volatile unsigned long *)((WTK_PARM_BASE)+0x0200)
#define WTK_BGROUND     (volatile unsigned long *)((WTK_PARM_BASE)+0x0204)
#define WTK_PLANE_MASK  (volatile unsigned long *)((WTK_PARM_BASE)+0x0208)
#define WTK_DRAW_MODE   (volatile unsigned long *)((WTK_PARM_BASE)+0x020C)
#define WTK_W_MIN_XY    (volatile unsigned long *)((WTK_PARM_BASE)+0x0220)
#define WTK_W_MAX_XY    (volatile unsigned long *)((WTK_PARM_BASE)+0x0224)

/*---------------------  Brooktree 485 Register Offsets  ---------------------*/

#define BT485_RAMDAC_WR_ADDR  (volatile unsigned char *) (BT485_BASE_ADDR + 0x0000)
#define BT485_DAC_PALETTE     (volatile unsigned char *) (BT485_BASE_ADDR + 0x0004)
#define BT485_PIXEL_MASK      (volatile unsigned char *) (BT485_BASE_ADDR + 0x0008)
#define BT485_RAMDAC_RD_ADDR  (volatile unsigned char *) (BT485_BASE_ADDR + 0x000C)
#define BT485_COLOR_WR_ADDR   (volatile unsigned char *) (BT485_BASE_ADDR + 0x0010)
#define BT485_COLOR_DATA      (volatile unsigned char *) (BT485_BASE_ADDR + 0x0014)
#define BT485_COMMAND_REG_0   (volatile unsigned char *) (BT485_BASE_ADDR + 0x0018)
#define BT485_COLOR_RD_ADDR   (volatile unsigned char *) (BT485_BASE_ADDR + 0x001C)
#define BT485_COMMAND_REG_1   (volatile unsigned char *) (BT485_BASE_ADDR + 0x0020)
#define BT485_COMMAND_REG_2   (volatile unsigned char *) (BT485_BASE_ADDR + 0x0024)
#define BT485_RAMDAC_STATUS   (volatile unsigned char *) (BT485_BASE_ADDR + 0x0028)
#define BT485_COMMAND_REG_3   (volatile unsigned char *) (BT485_BASE_ADDR + 0x0028)
#define BT485_CUR_RAM_ARRAY   (volatile unsigned char *) (BT485_BASE_ADDR + 0x002C)
#define BT485_CUR_X_LOW_REG   (volatile unsigned char *) (BT485_BASE_ADDR + 0x0030)
#define BT485_CUR_X_HIGH_REG  (volatile unsigned char *) (BT485_BASE_ADDR + 0x0034)
#define BT485_CUR_Y_LOW_REG   (volatile unsigned char *) (BT485_BASE_ADDR + 0x0038)
#define BT485_CUR_Y_HIGH_REG  (volatile unsigned char *) (BT485_BASE_ADDR + 0x003C)

#endif /* _H_FGA_REG */
