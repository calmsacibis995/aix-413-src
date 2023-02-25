/* @(#)90  1.7  src/rspc/kernext/disp/pcigga/inc/gga_regval.h, pcigga, rspc412, 9444b412 11/1/94 12:35:31 */
/* gga_regval.h */
/*
based on @(#)14       1.8  src/bos/kernext/disp/wga/inc/wga_regval.h, bos, bos410 4/30/93 13:06:10
 *
 *   COMPONENT_NAME: PCIGGA
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

#ifndef _H_GGA_REGVAL
#define _H_GGA_REGVAL

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

#define GGA_PCI_ID      0x0E100091      /* Glacier adapter PCI ID      	*/
#define GGA_MAP_SIZE    0x01000000      /* 4MB Vram			*/
#define GGA_4MB_VRAM	0x7
#define GGA_2MB_VRAM	0x5		/* 110 vs. 101 MEM_CONFIG[2-0]	*/
#define PATTERN1	0x5f5f5f5f	/* memory test pattern */
#define PATTERN2   	0xa0a0a0a0	/* memory test pattern */

/***************************************************************************/
/*                     WEITEK setup values                                 */
/***************************************************************************/

#define WTK_MEM_CONFIG_2     0xC808003E
#define WTK_MEM_CONFIG_4     0xC948003F     
#define WTK_SRTCTL_DATA	     0x00000183
#define WTK_VRAM_SIZE_2	     0x00200000   
#define WTK_VRAM_SIZE_4	     0x00400000   
#define WTK_MEM_SHIFT	     0x00000009
#define WTK_INTR_ENABLE_DATA 0x00000080     /* master enable off */
#define WTK_VBLANK_CLR       0x00000020
#define WTK_VBLANK_SET       0x00000010
#define WTK_PICKED_CLR       0x00000008
#define WTK_PICKED_SET       0x00000004
#define WTK_IDLE_CLR         0x00000002
#define WTK_IDLE_SET         0x00000001
#define WTK_RF_PERIOD_DATA   0x00000188
#define WTK_RL_MAX_DATA      0x000000FA
#define WTK_PREHRZC_DATA     0x00000000     /* Used for ext. sync */
#define WTK_PREVRTC_DATA     0x00000000     /* Used for ext. sync */
#define WTK_PATRN_DISABLE    0x000100F0
#define WTK_PLANE_MASK_DATA  0xFFFFFFFF
#define WTK_DRAW_MODE_DATA   0x000000a0
#define WTK_COOR_OFFS_DATA   0x00000000
#define WTK_VCLK0	     0x0001ac3d
#define WTK_VCLK1	     0x0021ac3d
#define WTK_VCLK2	     0x0041ac3d
#define WTK_MCLK	     0x0061d843
#define WTK_BUSY	     0x40000000



/***************************************************************************/
/************** DISPLAY CONTROLLER REGISTER VALUES  ************************/
/***************************************************************************/

/*----------  Glacier Adapter Status Register Values (GGA_FIFO_STAT) ---------*/
/*                                                                            */
/*       Bit  7 - Reserved                                                    */
/*            6 - Reserved                                                    */
/*            5 - Reserved                                                    */
/*            4 - Reserved                                                    */
/*            3 - Reserved                                                    */
/*            2 - Count in FIFO Bit 2                                         */
/*            1 - Count in FIFO Bit 1                                         */
/*            0 - Count in FIFO Bit 0                                         */

#define GGA_FIFO_Bit_2     Bit2
#define GGA_FIFO_Bit_1     Bit1
#define GGA_FIFO_Bit_0     Bit0

/*----------  Glacier Adapter Control Register Values (GGA_ADCNTL) -----------*/
/*                                                                            */
/*       Bit 15 - Enable Alias                                                */
/*           14 - Spare                                                       */
/*           13 - Spare                                                       */
/*           12 - DAC RD/WR Inactive Pulse Width 270ns                        */
/*           11 - DAC RD/WR Inactive Pulse Width 180ns                        */
/*           10 - DAC RD/WR Inactive Pulse Width 120ns                        */
/*            9 - Enable P9000 writes to be stored in 4-deep FIFO             */
/*            8 - Enable RGB525 writes to be stored in 4-deep FIFO             */

/*       Bit  7 - VSync Select    0 selects it                                */
/*            6 - HSync Select    0 selects it                                */
/*            5 - HSync Polarity  1 for +, 0 for -                            */
/*            4 - VSync Polarity  1 for +, 0 for -                            */
/*            3 - Spare                                                       */
/*            2 - Spare                                                       */
/*            1 - Serial Clock for ICD 2061A                                  */
/*            0 - Serial Data  for ICD 2061A                                  */

#define GGA_VSYNC_off      Bit7
#define GGA_HSYNC_off      Bit6
#define GGA_V_sync_plus    Bit5
#define GGA_V_sync_minus   0x00
#define GGA_H_sync_plus    Bit4
#define GGA_H_sync_minus   0x00
#define GGA_SYNC_select_mask (~(GGA_V_sync_plus | GGA_H_sync_plus ))
#define GGA_ser_clock_on   Bit2
#define GGA_ser_data_on    Bit3
#define GGA_HV_sync_mask   (~( GGA_V_sync_plus | GGA_H_sync_plus ))

/*
 * Define the Monitor id values for each supported monitor
 */

#define MT_0 0x201		/* 640x480x8(default)	    */
#define MT_1 0x205		/* 1024x768x8		    */
#define MT_2 0x107		/* 1280x1024x8		    */
#define MT_3 0x6		/* 640x480		    */

#define MT_END 0

static struct {
      ushort addr;
      uchar  data;                      /* Reg */
      } rgb525_init_tab[] = {           /* Num  - Function used */

        RGB525_MISC_CLK    , 0x27,      /* 0x02 - SLCK,DCLK,Disable PLL */
        RGB525_SYNC_CTL    , 0x80,      /* 0x03 - Do not add delay.  */
        RGB525_SYNC_POS    , 0x00,      /* 0x04 */
        RGB525_PWR_MGT     , 0x00,      /* 0x05 - Full up power -> HOG Mode */
        RGB525_DAC_OP      , 0x02,      /* 0x06 - Set Fast Slew Rate */
        RGB525_PALETTE_CTL , 0x00,      /* 0x07 */
        RGB525_PIXEL_FMT   , 0x03,      /* 0x0a - Set 8BPP Mode */
        RGB525_8BPP_CTL    , 0x00,      /* 0x0b - Normal Palette Mode */
        RGB525_16BPP_CTL   , 0x00,      /* 0x0c - Normal 16bpp mode */
        RGB525_24BPP_CTL   , 0x01,      /* 0x0d - Direct Color */
        RGB525_32BPP_CTL   , 0x00,      /* 0x0e - Direct Color */
        RGB525_PLL_CTL1    , 0x02,      /* 0x10 - Use REFCLK, and Internal FS */
        RGB525_PLL_CTL2    , 0x00,      /* 0x11 - Set same as S3 CR42[3:0] */
        RGB525_PLL_DIV     , (50/2),    /* 0x14 - Set Reference Divider MHZ/2 */

		/* Even though the 16 clock values are loaded during init, */
                /* entry F0 is the only one used.  The ecrtc table has the */
                /* exact clock value to load into the F0 register. Ctl1 and */
                /* ctl2 will always point to internal FS:0.  */

        RGB525_F0          , 0x3d,      /* 0x20 - 25.175 (act 25.25) */
        RGB525_F1          , 0x30,      /* 0x21 - 28.332 (act 28.25) */
        RGB525_F2          , 0x4f,      /* 0x22 - 40.00 */
        RGB525_F3          , 0xcf,      /* 0x23  160.00 */
        RGB525_F4          , 0x63,      /* 0x24   50.00 */
        RGB525_F5          , 0x8c,      /* 0x25   77.00 */
        RGB525_F6          , 0x47,      /* 0x26   36.00 */
        RGB525_F7          , 0x59,      /* 0x27   44.889 (act 45.00) */
        RGB525_F8          , 0xc0,      /* 0x28  130.00 */

        RGB525_F9          , 0xb7,      /* 0x29  120.00 */
        RGB525_F10         , 0x8f,      /* 0x2a   80.00 */
        RGB525_F11         , 0x3d,      /* 0x2b   31.50 */
        RGB525_F12         , 0xad,      /* 0x2c  110.00 */
        RGB525_F13         , 0x80,      /* 0x2d   65.00 */
        RGB525_F14         , 0x8a,      /* 0x2e   75.00 */
        RGB525_F15         , 0x9e,      /* 0x2f   94.50  (act 95.00) */
        RGB525_CUR_CTL     , 0x00,      /* 0x30 */
        RGB525_CUR_X_LOW   , 0x00,      /* 0x31 */
        RGB525_CUR_X_HIGH  , 0x00,      /* 0x32 */
        RGB525_CUR_Y_LOW   , 0x00,      /* 0x33 */
        RGB525_CUR_Y_HIGH  , 0x00,      /* 0x34 */
        RGB525_CUR_X_HOT   , 0x00,      /* 0x35 */
        RGB525_CUR_Y_HOT   , 0x00,      /* 0x36 */
        RGB525_CUR_1_RED   , 0x00,      /* 0x40 */
        RGB525_CUR_1_GREEN , 0x00,      /* 0x41 */
        RGB525_CUR_1_BLUE  , 0x00,      /* 0x42 */
        RGB525_CUR_2_RED   , 0x00,      /* 0x43 */
        RGB525_CUR_2_GREEN , 0x00,      /* 0x44 */
        RGB525_CUR_2_BLUE  , 0x00,      /* 0x45 */
        RGB525_CUR_3_RED   , 0x00,      /* 0x46 */
        RGB525_CUR_3_GREEN , 0x00,      /* 0x47 */
        RGB525_CUR_3_BLUE  , 0x00,      /* 0x48 */
        RGB525_BORDER_RED  , 0x00,      /* 0x60 */
        RGB525_BORDER_GREEN, 0x00,      /* 0x61 */
        RGB525_BORDER_BLUE , 0x00,      /* 0x62 */
        RGB525_MISC_CTL1   , 0x01,      /* 0x70 - Select 64 bit SD bus, PLL */
        RGB525_MISC_CTL2   , 0x45,      /* 0x71 - Select 8 bit color, SID port */
        RGB525_MISC_CLT3   , 0x00,      /* 0x72 */
        RGB525_VRAM_MASKH  , 0x00,      /* 0x90 */
        RGB525_VRAM_MASKL  , 0x01,      /* 0x91 */

        0x00,0x00               /* Terminate the list */
                            };

#endif /* _H_GGA_REGVAL */
