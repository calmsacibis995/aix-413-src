/* @(#)03  1.1  src/rspc/kernext/disp/pcifga/inc/fga_regval.h, pcifga, rspc411, GOLD410 4/15/94 18:38:45 */
/* fga_regval.h */
/*
based on @(#)14       1.8  src/bos/kernext/disp/wga/inc/wga_regval.h, bos, bos410 4/30/93 13:06:10
 *
 *   COMPONENT_NAME: PCIFGA
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

#ifndef _H_FGA_REGVAL
#define _H_FGA_REGVAL

/*
* COMPONENT NAME:    Weitek PCI Device Driver include file
*
* FUNCTIONS:         KSR device driver include file
*
* ORGINS:            27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with aggregated modules for this product)
*                  MATERIALS
* (C) COPYRIGHT International Business Machines Corp. 1991
* All Rights Reserved
*
* US Government Users  Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*/


/****START OF SPECIFICATIONS *******************************************
 *    DESCRIPTIVE NAME:  Weitek PCI Graphics Adapter Register Values   *
 *                                                                     *
 *    FUNCTION:  Define register offsets and values                    *
 *                                                                     *
 *    ATTRIBUTES:                                                      *
 *    REFERENCES:  White Oak Graphics Adaptor Specification            *
 ***********************************************************************
 *                                                                     *
 ****END   OF SPECIFICATIONS ******************************************/

#define Bit7    0x80
#define Bit6    0x40
#define Bit5    0x20
#define Bit4    0x10
#define Bit3    0x08
#define Bit2    0x04
#define Bit1    0x02
#define Bit0    0x01
#define NoBits  0x00
#define AllBits 0xff

#define FGA_PCI_ID      0x14101300      /* Fairway adapter PCI ID      */
#define FGA_MAP_SIZE    0x00400000      /* 2MB Vram + 2MB control regs */

/***************************************************************************/
/*                     WEITEK setup values                                 */
/***************************************************************************/

#define WTK_MEM_CONFIG_2     0x00000001     /* 1 x 1 Megapixel Buffer (1MB) */
#define WTK_MEM_CONFIG_3     0x00000002     /* 1 x 2 Megapixel Buffer (2MB) */
#define WTK_MEM_CONFIG_4     0x00000003     /* 2 x 1 Megapixel Buffer (2MB) */
#define WTK_SYS_CONFIG_2     0x00603000     /* fb width 1024, pixel swaps */
#define WTK_SYS_CONFIG_3     0x00703000     /* fb width 2048, pixel swaps */
#define WTK_SRTCTL_2         0x000001F4     /* 1 Meg VRAM */
#define WTK_SRTCTL_3         0x000001F5     /* 2 Meg VRAM */
#define WTK_VRAM_SIZE_2      0x00100000     /* 1 Meg VRAM */
#define WTK_VRAM_SIZE_3      0x00200000     /* 2 Meg VRAM */
#define WTK_MEM_SHIFT_2      0x00000008
#define WTK_MEM_SHIFT_3      0x00000009
#define WTK_INTR_ENABLE_DATA 0x00000080     /* master enable off */
#define WTK_VBLANK_CLR       0x00000020
#define WTK_VBLANK_SET       0x00000010
#define WTK_PICKED_CLR       0x00000008
#define WTK_PICKED_SET       0x00000004
#define WTK_IDLE_CLR         0x00000002
#define WTK_IDLE_SET         0x00000001
#define WTK_RF_PERIOD_DATA   0x00000186
#define WTK_RL_MAX_DATA      0x000000FA
#define WTK_PREHRZC_DATA     0x00000000     /* Used for ext. sync */
#define WTK_PREVRTC_DATA     0x00000000     /* Used for ext. sync */
#define WTK_PLANE_MASK_DATA  0x000000ff
#define WTK_DRAW_MODE_DATA   0x000000a0
#define WTK_COOR_OFFS_DATA   0x00000000

/* Brooktree 485 setup values */

#define BT485_REG_0_DATA      Bit7 | /*  CR07: Command Reg 3 can be addressed*/\
                              Bit5 | /*  CR05: Enable SETUP (7.5 IRE)        */\
                              Bit1   /*  CR01: DAC 8 bit resolution          */
#define BT485_REG_1_DATA      Bit6   /*  CR16: Four 8-bit pixels             */
#define BT485_REG_2_DATA      Bit5 | /*  CR25: PORTSEL nonmasked             */\
                              Bit4   /*  CR24: PCLK1 selected                */
#define BT485_REG_3_DATA      NoBits /*  none selected                       */

#define BT485_PXL_READ_MSK_DATA 0xff

/***************************************************************************/
/************** DISPLAY CONTROLLER REGISTER VALUES  ************************/
/***************************************************************************/

/*----------  Fairway Adapter Status Register Values (FGA_FIFO_STAT) ---------*/
/*                                                                            */
/*       Bit  7 - Reserved                                                    */
/*            6 - Reserved                                                    */
/*            5 - Reserved                                                    */
/*            4 - Reserved                                                    */
/*            3 - Reserved                                                    */
/*            2 - Count in FIFO Bit 2                                         */
/*            1 - Count in FIFO Bit 1                                         */
/*            0 - Count in FIFO Bit 0                                         */

#define FGA_FIFO_Bit_2     Bit2
#define FGA_FIFO_Bit_1     Bit1
#define FGA_FIFO_Bit_0     Bit0

/*----------  Fairway Adapter Control Register Values (FGA_ADCNTL) -----------*/
/*                                                                            */
/*       Bit 15 - Enable Alias                                                */
/*           14 - Spare                                                       */
/*           13 - Spare                                                       */
/*           12 - DAC RD/WR Inactive Pulse Width 270ns                        */
/*           11 - DAC RD/WR Inactive Pulse Width 180ns                        */
/*           10 - DAC RD/WR Inactive Pulse Width 120ns                        */
/*            9 - Enable P9000 writes to be stored in 4-deep FIFO             */
/*            8 - Enable Bt485 writes to be stored in 4-deep FIFO             */

/*       Bit  7 - VSync Select    0 selects it                                */
/*            6 - HSync Select    0 selects it                                */
/*            5 - HSync Polarity  1 for +, 0 for -                            */
/*            4 - VSync Polarity  1 for +, 0 for -                            */
/*            3 - Spare                                                       */
/*            2 - Spare                                                       */
/*            1 - Serial Clock for ICD 2061A                                  */
/*            0 - Serial Data  for ICD 2061A                                  */

#define FGA_VSYNC_off      Bit7
#define FGA_HSYNC_off      Bit6
#define FGA_V_sync_plus    Bit5
#define FGA_V_sync_minus   0x00
#define FGA_H_sync_plus    Bit4
#define FGA_H_sync_minus   0x00
#define FGA_SYNC_select_mask (~(FGA_V_sync_plus | FGA_H_sync_plus ))
#define FGA_ser_clock_on   Bit1
#define FGA_ser_data_on    Bit0
#define FGA_HV_sync_mask   (~( FGA_V_sync_plus | FGA_H_sync_plus ))

/*----------  Fairway Adapter Sense Register Values (FGA_ADSTAT) -------------*/
/*                                                                            */
/*       Bit  7 - VRAM size 1= 1 MB, 0= 2 MB                                  */
/*            6 - Monitor Sense from Bt485                                    */
/*            5 - VSYNC                                                       */
/*            4 - HSYNC                                                       */
/*            3 - Monitor ID Bit 3 (from connector)                           */
/*            2 - Monitor ID Bit 2 (from connector)                           */
/*            1 - Monitor ID Bit 1 (from connector)                           */
/*            0 - Monitor ID Bit 0 (from connector)                           */

#define FGA_1MB_VRAM       Bit7
#define FGA_Monitor_Sense  Bit6
#define FGA_V_SYNC         Bit5
#define FGA_H_SYNC         Bit4
#define FGA_Mon_ID_3       Bit3
#define FGA_Mon_ID_2       Bit2
#define FGA_Mon_ID_1       Bit1
#define FGA_Mon_ID_0       Bit0

/*
 * Define the Monitor id values for each supported monitor
 */

#define MT_0 0x000a             /* 1024x768, 76Hz, 86MHz    */
                                /* IBM 6319                 */

#define MT_1 0x010a             /* 1024x 768, 70Hz, 78 MHz. */
                                /* IBM - 8517               */

#define MT_2 0x0004             /* 1280x1024, 60Hz, 112MHz  */
                                /* IBM 6091, 6317, vendor   */

#define MT_3 0x0007             /* 1280x1024, 67Hz, 128MHz  */
                                /* IBM 8508 - Mono          */

#define MT_4 0x020f             /* 1280x1024, 72Hz, 128MHz  */
                                /* 1091-51                  */

#define MT_5 0x000f             /* 1024x768, 60Hz, 64MHz    */
                                /* Galaxy X, vendor         */

#define MT_END 0
#endif /* _H_FGA_REGVAL */
