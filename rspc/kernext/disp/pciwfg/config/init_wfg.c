static char sccsid[] = "@(#)81  1.10 src/rspc/kernext/disp/pciwfg/config/init_wfg.c, pciwfg, rspc41J, 9520A_all 5/16/95 02:45:45";
/* init_wfg.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: Initialize the hardware
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

/*=======================================================================*
 *                                                                       *
 *            Western Digital Graphics Adapter (WFG) driver              *
 *                                                                       *
 *                         INITIALIZATION ROUTINES                       *
 *                                                                       *
 *      Returns: -1 if can't find WFG, otherwise 0                       *
 *                                                                       *
 *      Called by: vtt_init                                              *
 *                                                                       *
 *      Calls:  Get_Monitor_type                                         *
 *              Get_Panel_type                                           *
 *              load_palette                                             *
 *              wfg_reset                                                *
 *              enable_wfg                                               *
 *                                                                       *
 *=======================================================================*/

#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/residual.h>
#include <sys/iplcb.h>

#include "wfg_INCLUDES.h"
#include "IO_defs.h"

void Get_Monitor_type();               /* Determine monitor, etc. */
void Get_Panel_type();                 /* Determine lcd panel     */
void load_palette ();                  /* Load RAMDAC             */
V_MODE *lookup_v();

#define  Vendor_Packet    0x84           /* tag for DisplayInfoPack */
#define  Display_Type     0x04           /* type for display        */
#define  NOT_FOUND      0x8888

typedef struct _DispInfoPack {
   unsigned char  Tag;                 /* large tag = 0x84 Vendor specific */
   unsigned char  Count0;              /* = 9                              */
   unsigned char  Count1;              /* = 0                              */
   unsigned char  Type;                /* = 4 (display)                    */
   unsigned char  DisplayMode;         /* Display_Mode enum                */
   unsigned char  DisplayID;           /* Display_ID enum                  */
   unsigned char  Hor_Pixels[2];       /* Horizontal resolution, e.g. 640  */
   unsigned char  Ver_Pixels[2];       /* Vertical Resolution, e.g. 480    */
   unsigned char  BytesPerRow[2];      /* Number of bytes per VRAM row     */
   unsigned char  VRAMAddr[8];         /* Location of video buffer         */
   unsigned char  VRAMSize[8];         /* Size of video buffer             */
} DispInfoPack;

static unsigned char crtc_6_01[] = { /* CRT Controller                       */
   0x5f,                             /* Index 00h - Horizontal Total         */
   0x4f,                             /* Index 01h - Horizontal Display End   */
   0x50,                             /* Index 02h - Start Horizontal Blank   */
   0x82,                             /* Index 03h - End Horizontal Blank     */
   0x53,                             /* Index 04h - Start Horizontal Sync Pos*/
   0x9f,                             /* Index 05h - End Horizontal Sync Pos  */
   0x0b,                             /* Index 06h - Vertical Total           */
   0x3e,                             /* Index 07h - CRTC Overflow            */
   0x00,                             /* Index 08h - Preset Row Scan          */
   0x40,                             /* Index 09h - Maximum Scan Line        */
   0x00,                             /* Index 0ah - Cursor Start Scan Line   */
   0x00,                             /* Index 0bh - Cursor End Scan Line     */
   0x00,                             /* Index 0ch - Start Address High       */
   0x00,                             /* Index 0dh - Start Address Low        */
   0x00,                             /* Index 0eh - Cursor Address High      */
   0x00,                             /* Index 0fh - Cursor Address Low       */
   0xea,                             /* Index 10h - Vertical Retrace Start   */
   0x8c,                             /* Index 11h - Vertical Retrace End     */
   0xdf,                             /* Index 12h - Vertical Display End     */
   0x50,                             /* Index 13h - Offset                   */
   0x40,                             /* Index 14h - Underline Location       */
   0xe7,                             /* Index 15h - Start Vertical Blank     */
   0x04,                             /* Index 16h - End Vertical Blank       */
   0xe3,                             /* Index 17h - CRTC Mode Control        */
   0xff                              /* Index 18h - Line Compare             */
};

static unsigned char crtc_8_01[] = {  /* CRT Controller                       */
   0x7f,                              /* Index 00h - Horizontal Total         */
   0x63,                              /* Index 01h - Horizontal Display End   */
   0x64,                              /* Index 02h - Start Horizontal Blank   */
   0x82,                              /* Index 03h - End Horizontal Blank     */
   0x6B,                              /* Index 04h - Start Horizontal Sync Pos*/
   0x1B,                              /* Index 05h - End Horizontal Sync Pos  */
   0x72,                              /* Index 06h - Vertical Total           */
   0xf0,                              /* Index 07h - CRTC Overflow            */
   0x00,                              /* Index 08h - Preset Row Scan          */
   0x60,                              /* Index 09h - Maximum Scan Line        */
   0x00,                              /* Index 0ah - Cursor Start Scan Line   */
   0x00,                              /* Index 0bh - Cursor End Scan Line     */
   0x00,                              /* Index 0ch - Start Address High       */
   0x00,                              /* Index 0dh - Start Address Low        */
   0x00,                              /* Index 0eh - Cursor Address High      */
   0x00,                              /* Index 0fh - Cursor Address Low       */
   0x58,                              /* Index 10h - Vertical Retrace Start   */
   0x8c,                              /* Index 11h - Vertical Retrace End     */
   0x57,                              /* Index 12h - Vertical Display End     */
   0x64,                              /* Index 13h - Offset                   */
   0x40,                              /* Index 14h - Underline Location       */
   0x58,                              /* Index 15h - Start Vertical Blank     */
   0x71,                              /* Index 16h - End Vertical Blank       */
   0xe3,                              /* Index 17h - CRTC Mode Control        */
   0xff                               /* Index 18h - Line Compare             */
};

static PARADISE_MODE_TABLE Mode_TFT_Table_1 = {

      PR1B,  PR1B_UNLOCK  ,           /* Disable CRT/LCD by PR19              */
      PR19,  PR19_DISABLE ,           /* Flat Panel Control I register        */
      PR1B,  PR1B_LOCK    ,           /* Flat Panel Unlock register           */

      32,                             /* Length of PVGA Sequencer Regs        */
      PR20 , PR20_UNLOCK  ,           /* Unlock Paradise Extended Registers   */
      PR30A, PR30A_S32    ,           /* Write Buffer and FIFO Control Reg    */
      PR31 , PR31_ALL     ,           /* System Interface Control             */
      PR32 , PR32_ALL     ,           /* Miscellaneous Control 4              */
      PR33A, PR33A_ALL    ,           /* DRAM Timing & Zero Wait State Ctl Reg*/
      PR34A, PR34A_ALL    ,           /* Display Memory Mapping Register      */
      PR35A, PR35A_ALL    ,           /* FPUSR0, FPUSR1 Output Select Register*/
      PR57 , PR57_ALL     ,           /* Feature Register I                   */
      PR58 , PR58_ALL     ,           /* Feature Register II                  */
      PR59 , PR59_S32     ,           /* Memory Arbitration Cycle Setup Reg   */
      PR62 , PR62_ALL     ,           /* FR Timing Register                   */
      PR64 , PR64_ALL     ,           /* CRT Lock Control II                  */
      PR66 , PR66_S32     ,           /* Feature Register III                 */
      PR72 , PR72_LOCK    ,           /* Programmable Clock Unlock            */
      PR70 , PR70_ALL     ,           /* Mixed Voltage Override Register      */
      PR20 , PR20_LOCK    ,           /* Unlock Paradise Extended Registers   */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */

      PR5  , PR5_UNLOCK   ,           /* Status Bits/Unlock PR0 through PR4   */
      PR1  , PR1_ALL      ,           /* Memory Size                          */
      PR2  , PR2_S32      ,           /* Video Select Register                */
      PR3  , PR3_ALL      ,           /* CRT Lock Control Register            */
      PR4  , PR4_CRT      ,           /* Video Control Register               */
      PR5  , PR5_LOCK     ,           /* Lock PR0 through PR4                 */

      66,                             /* Length of PVGA CRT Controller Regs   */
      PR10 , PR10_UNLOCK  ,           /* Unlock PR11 through PR17             */
      PR11 , PR11_LOCK    ,           /* EGA Switches     Default is lock     */
      PR14 , PR14_ALL     ,           /* Interace H/2 End                     */
      PR15 , PR15_ALL     ,           /* Miscellaneous Control 1              */
      PR16 , PR16_ALL     ,           /* Miscellaneous Control 2              */
      PR17 , PR17_ALL     ,           /* Miscellaneous Control 3              */
      PR10 , PR10_LOCK    ,           /* Lock PR11 through PR17               */

      PR1B , PR1B_UNLOCK  ,           /* Unlock (Flat Panel Unlock Register)  */
      PR18 , PR18_S32 | 0x80 ,        /* Flat Panel Status Register           */
      PR39 , PR39_S32     ,           /* Color LCD Control Register           */
      PR1A , PR1A_ALL     ,           /* Flat Panel Control II Register       */
      PR36 , PR36_ALL     ,           /* Panel Height Select Register         */
      PR37 , PR37_S32     ,           /* FLat Panel Blinking Control          */
      PR18A, PR18A_ALL    ,           /* CRTC Vertical Timing Overflow        */
      PR44 , PR44_ALL     ,           /* Power-Down and Memory Refresh Ctl Reg*/
      PR1B , PR1B_LOCK    ,           /* Lock PR[18,19,39,1A,36,37,18A,44]    */

      PR30 , PR30_UNLOCK  ,           /* Unlock (Mapping RAM not initialized) */
      PR35 , PR35_ALL     ,           /* Mapping RAM and Power-Down Ctl Reg   */
      PR30 , PR30_LOCK    ,           /* Lock PR35                            */

      PR1B , PR1B_UNLOCK_SHADOW ,     /* Unlock shadow registers              */
      0x11 , CRTC11_S32 & 0x7f ,      /*   Unlock CRTC 0-7                    */
      0x00 , CRTC00_S32   ,           /*   TFT color simultanuous mode        */
      0x02 , CRTC02_S32   ,           /*   TFT color simultanuous mode        */
      0x03 , CRTC03_S32   ,           /*   TFT color simultanuous mode        */
      0x04 , CRTC04_S32   ,           /*   TFT color simultanuous mode        */
      0x05 , CRTC05_S32   ,           /*   TFT color simultanuous mode        */
      0x06 , CRTC06_S32   ,           /*   TFT color simultanuous mode        */
      0x07 , CRTC07_S32   ,           /*   TFT color simultanuous mode        */
      0x10 , CRTC10_S32   ,           /*   TFT color simultanuous mode        */
      0x11 , CRTC11_S32   ,           /*   TFT color simultanuous mode        */
      0x15 , CRTC15_S32   ,           /*   TFT color simultanuous mode        */
      0x16 , CRTC16_S32   ,           /*   TFT color simultanuous mode        */
      PR1B , PR1B_LOCK    ,           /* Lock shadow registers                */

      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00                     /* This is not used.                    */
};

static PARADISE_MODE_TABLE Mode_TFT_Table_2 = {

      PR1B,  PR1B_UNLOCK  ,           /* Disable CRT/LCD by PR19              */
      PR19,  PR19_DISABLE ,           /* Flat Panel Control I register        */
      PR1B,  PR1B_LOCK    ,           /* Flat Panel Unlock register           */

      32,                             /* Length of PVGA Sequencer Regs        */
      PR20 , PR20_UNLOCK  ,           /* Unlock Paradise Extended Registers   */
      PR30A, PR30A_S32    ,           /* Write Buffer and FIFO Control Reg    */
      PR31 , PR31_ALL | 0x40 ,        /* System Interface Control             */
      PR32 , PR32_ALL     ,           /* Miscellaneous Control 4              */
      PR33A, PR33A_ALL    ,           /* DRAM Timing & Zero Wait State Ctl Reg*/
      PR34A, PR34A_ALL    ,           /* Display Memory Mapping Register      */
      PR35A, PR35A_ALL    ,           /* FPUSR0, FPUSR1 Output Select Register*/
      PR57 , PR57_ALL     ,           /* Feature Register I                   */
      PR58 , PR58_ALL     ,           /* Feature Register II                  */
      PR59 , PR59_S32     ,           /* Memory Arbitration Cycle Setup Reg   */
      PR62 , PR62_ALL     ,           /* FR Timing Register                   */
      PR64 , PR64_ALL     ,           /* CRT Lock Control II                  */
      PR66 , PR66_S32     ,           /* Feature Register III                 */
      PR72 , PR72_LOCK    ,           /* Programmable Clock Unlock            */
      PR70 , PR70_ALL     ,           /* Mixed Voltage Override Register      */
      PR20 , PR20_LOCK    ,           /* Unlock Paradise Extended Registers   */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */

      PR5  , PR5_UNLOCK   ,           /* Status Bits/Unlock PR0 through PR4   */
      PR1  , PR1_ALL      ,           /* Memory Size                          */
      PR2  , PR2_S32      ,           /* Video Select Register                */
      PR3  , PR3_ALL      ,           /* CRT Lock Control Register            */
      PR4  , PR4_CRT      ,           /* Video Control Register               */
      PR5  , PR5_LOCK     ,           /* Lock PR0 through PR4                 */

      66,                             /* Length of PVGA CRT Controller Regs   */
      PR10 , PR10_UNLOCK  ,           /* Unlock PR11 through PR17             */
      PR11 , PR11_LOCK    ,           /* EGA Switches     Default is lock     */
      PR14 , PR14_ALL     ,           /* Interace H/2 End                     */
      PR15 , PR15_ALL     ,           /* Miscellaneous Control 1              */
      PR16 , PR16_ALL     ,           /* Miscellaneous Control 2              */
      PR17 , PR17_ALL     ,           /* Miscellaneous Control 3              */
      PR10 , PR10_LOCK    ,           /* Lock PR11 through PR17               */

      PR1B , PR1B_UNLOCK  ,           /* Unlock (Flat Panel Unlock Register)  */
      PR18 , PR18_S32 | 0x80 ,        /* Flat Panel Status Register           */
      PR39 , PR39_S32     ,           /* Color LCD Control Register           */
      PR1A , 0x90         ,           /* Flat Panel Control II Register       */
      PR36 , PR36_ALL     ,           /* Panel Height Select Register         */
      PR37 , PR37_S32     ,           /* FLat Panel Blinking Control          */
      PR18A, PR18A_ALL    ,           /* CRTC Vertical Timing Overflow        */
      PR44 , PR44_ALL     ,           /* Power-Down and Memory Refresh Ctl Reg*/
      PR1B , PR1B_LOCK    ,           /* Lock PR[18,19,39,1A,36,37,18A,44]    */

      PR30 , PR30_UNLOCK  ,           /* Unlock (Mapping RAM not initialized) */
      PR35 , PR35_ALL     ,           /* Mapping RAM and Power-Down Ctl Reg   */
      PR30 , PR30_LOCK    ,           /* Lock PR35                            */

      PR1B , PR1B_UNLOCK_SHADOW ,     /* Unlock shadow registers              */
      0x11 , 0x0f         ,           /*   Unlock CRTC 0-7                    */
      0x00 , 0x7f         ,           /*   TFT color simultanuous mode        */
      0x02 , 0x63         ,           /*   TFT color simultanuous mode        */
      0x03 , 0x80         ,           /*   TFT color simultanuous mode        */
      0x04 , 0x69         ,           /*   TFT color simultanuous mode        */
      0x05 , 0x19         ,           /*   TFT color simultanuous mode        */
      0x06 , 0x72         ,           /*   TFT color simultanuous mode        */
      0x07 , 0xf0         ,           /*   TFT color simultanuous mode        */
      0x10 , 0x58         ,           /*   TFT color simultanuous mode        */
      0x11 , 0x8c         ,           /*   TFT color simultanuous mode        */
      0x15 , 0x57         ,           /*   TFT color simultanuous mode        */
      0x16 , 0x6f         ,           /*   TFT color simultanuous mode        */
      PR1B , PR1B_LOCK    ,           /* Lock shadow registers                */

      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00         ,           /* This is not used.                    */
      0x00 , 0x00                     /* This is not used.                    */
};

static BASE_MODE_TABLE Mode_800x600_Table = {
       600,                           /* Screen Height                        */
       800,                           /* Screen Width                         */

       0x23,                          /* Miscellaneous Output for ISO Monitor */
       0x23,                          /*                  for Non-ISO Monitor */

                                      /* Sequencer Data                       */
       0x01,                          /* - Clocking Mode                      */
       0x0f,                          /* - Enable Write Plane                 */
       0x00,                          /* - Character Font Select              */
       0x0e,                          /* - Memory Mode Control    56/72Hz     */

                                      /* Attribute Controller Data            */
       0x00,                          /* Index 00h - Palette Reg 00h          */
       0x01,                          /* Index 01h - Palette Reg 01h          */
       0x02,                          /* Index 02h - Palette Reg 02h          */
       0x03,                          /* Index 03h - Palette Reg 03h          */
       0x04,                          /* Index 04h - Palette Reg 04h          */
       0x05,                          /* Index 05h - Palette Reg 05h          */
       0x06,                          /* Index 06h - Palette Reg 06h          */
       0x07,                          /* Index 07h - Palette Reg 07h          */
       0x08,                          /* Index 08h - Palette Reg 08h          */
       0x09,                          /* Index 09h - Palette Reg 09h          */
       0x0a,                          /* Index 0ah - Palette Reg 0ah          */
       0x0b,                          /* Index 0bh - Palette Reg 0bh          */
       0x0c,                          /* Index 0ch - Palette Reg 0ch          */
       0x0d,                          /* Index 0dh - Palette Reg 0dh          */
       0x0e,                          /* Index 0eh - Palette Reg 0eh          */
       0x0f,                          /* Index 0fh - Palette Reg 0fh          */
       0x61,                          /* Index 10h - Attribute Mode Ctl       */
       0x00,                          /* Index 11h - Border Coloer            */
       0x0f,                          /* Index 12h - Color Plane Enable       */
       0x00,                          /* Index 13h - Horizontal Pixel Panning */
       0x00,                          /* Index 14h - Color Secelt             */

                                      /* Graphics Controller                  */
       0x00,                          /* Index 00h - Set/Reset Data           */
       0x00,                          /* Index 01h - Enable Set/Reset Data    */
       0x00,                          /* Index 02h - Color Compare            */
       0x00,                          /* Index 03h - Raster Op/Rotate Count   */
       0x00,                          /* Index 04h - Read Plane Select        */
       0x40,                          /* Index 05h - Graphics Controller Mode */
       0x05,                          /* Index 06h - Memory Map Mode Control  */
       0x0f,                          /* Index 07h - Color Don't Care         */
       0xff                           /* Index 08h - Bit Mask                 */
};

static BASE_MODE_TABLE Mode_640x480_Table = {
       480,                      /* Screen Height                        */
       640,                      /* Screen Width                         */
    
       0xe3,                     /* Miscellaneous Output for ISO Monitor */
       0xe3,                     /*                  for Non-ISO Monitor */
    
                                 /* Sequencer Data                       */
       0x01,                     /* - Clocking Mode                      */
       0x0f,                     /* - Enable Write Plane                 */
       0x00,                     /* - Character Font Select              */
       0x0e,                     /* - Memory Mode Control                */
    
                                 /* Attribute Controller Data            */
       0x00,                     /* Index 00h - Palette Reg 00h          */
       0x01,                     /* Index 01h - Palette Reg 01h          */
       0x02,                     /* Index 02h - Palette Reg 02h          */
       0x03,                     /* Index 03h - Palette Reg 03h          */
       0x04,                     /* Index 04h - Palette Reg 04h          */
       0x05,                     /* Index 05h - Palette Reg 05h          */
       0x06,                     /* Index 06h - Palette Reg 06h          */
       0x07,                     /* Index 07h - Palette Reg 07h          */
       0x08,                     /* Index 08h - Palette Reg 08h          */
       0x09,                     /* Index 09h - Palette Reg 09h          */
       0x0a,                     /* Index 0ah - Palette Reg 0ah          */
       0x0b,                     /* Index 0bh - Palette Reg 0bh          */
       0x0c,                     /* Index 0ch - Palette Reg 0ch          */
       0x0d,                     /* Index 0dh - Palette Reg 0dh          */
       0x0e,                     /* Index 0eh - Palette Reg 0eh          */
       0x0f,                     /* Index 0fh - Palette Reg 0fh          */
       0x41,                     /* Index 10h - Attribute Mode Ctl       */
       0x00,                     /* Index 11h - Border Coloer            */
       0x0f,                     /* Index 12h - Color Plane Enable       */
       0x00,                     /* Index 13h - Horizontal Pixel Panning */
       0x00,                     /* Index 14h - Color Secelt             */
    
                                 /* Graphics Controller                  */
       0x00,                     /* Index 00h - Set/Reset Data           */
       0x00,                     /* Index 01h - Enable Set/Reset Data    */
       0x00,                     /* Index 02h - Color Compare            */
       0x00,                     /* Index 03h - Raster Op/Rotate Count   */
       0x00,                     /* Index 04h - Read Plane Select        */
       0x40,                     /* Index 05h - Graphics Controller Mode */
       0x05,                     /* Index 06h - Memory Map Mode Control  */
       0x0f,                     /* Index 07h - Color Don't Care         */
       0xff                      /* Index 08h - Bit Mask                 */
};

static V_MODE tft_svga = {       /* 800 x 600 x 256                      */
        0x0e,                    /* Sequence 04h                         */
        8,                       /* 8 bits/pel                           */
        &Mode_800x600_Table,     /* Base Mode Table                      */
        &Mode_TFT_Table_2,       /* Paradise Register(CRTC Disable)      */
        crtc_8_01,               /* Table for ISO Monitor        74HZ NI */
        crtc_8_01                /* Table for Non-ISO Monitor    59HZ NI */
};

static V_MODE tft_vga = {        /* 640 x 480 x 256                      */
        0x0e,                    /* Sequence 04h                         */
        8,                       /* 8 bits/pel                           */
        &Mode_640x480_Table,     /* Base Mode Table                      */
        &Mode_TFT_Table_1,       /* Paradise Register(CRTC Disable)      */
        crtc_6_01,               /* Table for ISO Monitor        74HZ NI */
        crtc_6_01                /* Table for Non-ISO Monitor    59HZ NI */
};

static int panel_type;
struct Vddinfo
{
	unsigned int lcd_w;	/* LCD Panel Physical Resolution (Width) */
	unsigned int lcd_h;	/* LCD Panel Physical Resolution (Height)*/
	unsigned int offset_x;	/* overlay offset X                      */
	unsigned int offset_y;	/* overlay offset Y                      */
	unsigned int vram_clk;	/* VRAM Clock timing                     */
	unsigned int vram_offset;/* VRAM offset                          */
	unsigned int v_sync;    /* Vsync                                 */
	unsigned int v_total;   /* Vertical Total lines                  */
};

BUGXDEF (dbg_initw);
/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : init_wfg()                                           *
 *                                                                         *
 * DESCRIPTIVE NAME : Western Digital PCI Graphic Adapter                  *
 *                    device enable routine                                *
 *                                                                         *
 * FUNCTIONS : A part of initialize procedure                              *
 *               1. Detect the attached monitor type                       *
 *               2. Detect the attached LCD panel type                     *
 *               3. Set the initialize data  (display mode)                *
 *                                                                         *
 * INPUTS    : Pointer to vtmstruc structure                               *
 *             Pointer to local defined structure (wfgldat)                *
 *             Pointer to ddf                                              *
 *             Pointer to dds                                              *
 *                                                                         *
 * OUTPUTS   : -1 if can't find WFG, otherwise 0                           *
 * CALLED BY : vtt_init()                                                  *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

int 
init_wfg(vp, ld, ddf, dds)       /* Mainline routine */
    struct vtmstruc *vp;
    struct wfg_data *ld;
    struct wfg_ddf *ddf;
    struct fbdds   *dds;         /* contains config method data-prefilled */
{
    int  i, rc;
    uchar adapter_stat_reg;
    struct phys_displays *pd;

    BUGLPR (dbg_initw, BUGNTA, ("Starting initialization\n"));
    BUGLPR (dbg_initw, BUGNTA, ("Attaching PCI I/O space\n"));

#ifdef	PANEL_PROTECTION
    /* =================================== *
     *    Disable LCD for H/W protection   *
     * =================================== */
    wfg_planar_control( ddf, dds, vp->display->devno, 
			dds->pm_lcd_device_id, PM_PLANAR_OFF );
#endif	/* PANEL_PROTECTION */

    ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

    /* ============================================================== *
     *    Set up for attached monitor and built-in LCD panel type     *
     * ============================================================== */

    /* ------------------------- *
     *    Determine monitor      *
     * ------------------------- */
    BUGLPR (dbg_initw, BUGGID, ("Get_Monitor_type\n"));
    Get_Monitor_type (ld, ddf);      


    /* -------------------------------------------------- *
     *    Get LCD panel type from residual data of ROS    *
     * -------------------------------------------------- */
    BUGLPR (dbg_initw, BUGGID, ("Get_Panel_type\n"));
    Get_Panel_type (ld, ddf);

    /* ------------------------ *
     *    Set display mode      *
     * ------------------------ */
    ld->v_mode = (V_MODE *)lookup_v(ddf);

    if(ld->v_mode == NULL) 
    {
            ld->v_mode = &tft_vga;
    }

    /* ------------------------------- *
     *    Pick up mode's resolution    *
     * ------------------------------- */
    ld->screen_width_pix         = ld->v_mode->Base_Mode_Table->screen_w;
    ld->screen_height_pix        = ld->v_mode->Base_Mode_Table->screen_h;
    ld->vram_size = ddf->fb_size = WFG_RAM_1MEG;
    ld->bpp                      = ld->v_mode->bits_per_pel;

    PCI_GETC(INPUT_STATUS1);     /* Reset attribute address flip flop    */
    PCI_PUTC(ATTR_INDX, 0);      /* Disable video palette (turns off vid)*/

    ld->ksr_color_table.num_entries = 16;
    for (i = 0; i < 16; i++)
    {
        ld->ksr_color_table.rgbval[i] = dds->ksr_color_table[i];

        BUGLPR(dbg_initw,BUGGID,
               ("palette [%2.2d] = 0x%8.8x\n",i,dds->ksr_color_table[i]));

    }
    ld->cursor_x               = CURSOR_X; /* cursor magic numbers */
    ld->cursor_y               = CURSOR_Y;

    /* -------------------------------------- *
     *    Detach addressability to the bus    *
     * -------------------------------------- */
    BUGLPR (dbg_initw, BUGNTA, ("Detaching PCI I/O space\n"));
    PCI_BUS_DET(ld->IO_base_address); 

#ifdef	PANEL_PROTECTION
    /* ------------------------------------------ *
     *          Enable LCD			  *
     * ------------------------------------------ */
    wfg_planar_control( ddf, dds, vp->display->devno, 
			dds->pm_lcd_device_id, PM_PLANAR_ON );
#endif	/* PANEL_PROTECTION */

    return 0;
}

/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : Get_Monitor_type()                                   *
 *                                                                         *
 * DESCRIPTIVE NAME : Detect the attached monitor type                     *
 *                                                                         *
 * FUNCTIONS : Read Composite Display ID, which is defined                 *
 *             as a part of XGA DMQS.                                      *
 *                                                                         *
 * INPUTS    : Pointer to local defined structure (wfgldat)                *
 *             Pointer to ddf                                              *
 *                                                                         *
 * OUTPUTS   : void                                                        *
 * CALLED BY : init_wfg()                                                  *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

void
Get_Monitor_type (ld, ddf)
struct wfg_data *ld;
struct wfg_ddf *ddf;
{
        int     monitor_id = 0, crt_type;
        unsigned char  misc_val, crtc_val, pr19_val;

#define MONO  0
#define COLOR 1
        ld->disp_type         = COLOR;

#ifndef	PANEL_PROTECTION
        /* --------------------------------------------------- *
         *     Disable LCD panel                               *
         * --------------------------------------------------- */
        PUT_GRF_REG ( PR5, PR5_UNLOCK );
        PUT_GRF_REG ( PR4, PR4_CRT );
        PUT_GRF_REG ( PR5, PR5_LOCK );
#endif	/* PANEL_PROTECTION */

        /* --------------------------------------------------- *
         *     Set to CRT only display mode                    *
         *                                                     *
         *     (Disable LCD to unlock sync polarities )        *
         * --------------------------------------------------- */
        PUT_CRTC_REG(PR1B, PR1B_UNLOCK);
     
        PCI_PUTC(CRTC_INDX, PR19); 
        pr19_val = PCI_GETC(CRTC_DATA);

        /*
         * LCDEN bit of Carrera has been disabled at this moment.
         * We don't want to stop WD's dotclock by disabling
         * Flat Panel Display Enable bit (bit4) of PR19.
         */
/**
        PCI_PUTC(CRTC_DATA, pr19_val & 0xef); 
**/
     
        /* ---------------------------------------------------------- *
         *    Save Sync Polarity and Hardware Reset Field in CRTC     *
         * ---------------------------------------------------------- */
     
        misc_val = PCI_GETC(MISC_OUTPUT_R);
        PCI_PUTC(CRTC_INDX, 0x17); crtc_val = PCI_GETC(CRTC_DATA);
        PCI_PUTC(CRTC_DATA, crtc_val & 0x7f);  /* Inactive retrace signals */
     
        /* --------------------------------------------------- *
         *    Set VSYNC = 0, HSYNC = 1                         *
         * --------------------------------------------------- */
     
        PCI_PUTC(MISC_OUTPUT_W, (misc_val & 0x3f) | 0x40);
        monitor_id |= (did2mid(PCI_GETC(0x8a4) & 0x0f) << 3);
     
        /* --------------------------------------------------- *
         *    Set VSYNC = 1, HSYNC = 0                         *
         * --------------------------------------------------- */
     
        PCI_PUTC(MISC_OUTPUT_W, (misc_val & 0x3f) | 0x80);
        monitor_id |= (did2mid(PCI_GETC(0x8a4) & 0x0f) << 2);
     
        /* --------------------------------------------------- *
         *    Set VSYNC = 0, HSYNC = 0                         *
         * --------------------------------------------------- */
        PCI_PUTC(MISC_OUTPUT_W, (misc_val & 0x3f) | 0x00);
        monitor_id |= (did2mid(PCI_GETC(0x8a4) & 0x0f) << 1);
     
        /* --------------------------------------------------- *
         *    Set VSYNC = 1, HSYNC = 1                         *
         * --------------------------------------------------- */
        PCI_PUTC(MISC_OUTPUT_W, (misc_val & 0x3f) | 0xc0);
        monitor_id |= (did2mid(PCI_GETC(0x8a4) & 0x0f) << 0);
     
        /* ------------------------------------------------------------ *
         *   Restore Miscellaneous Output Register and CRTC Index 17h   *
         * ------------------------------------------------------------ */
     
        PCI_PUTC(MISC_OUTPUT_W, misc_val);
        PCI_PUTC(CRTC_DATA, crtc_val);
     
        /* --------------------------------------------------- *
         *   Restore PR19                                      *
         * --------------------------------------------------- */
        PCI_PUTC(CRTC_INDX, PR19); PCI_PUTC(CRTC_DATA, pr19_val);
        PCI_PUTC(CRTC_INDX, PR1B); PCI_PUTC(CRTC_DATA, PR1B_LOCK);

        /* --------------------------------------------------- *
         *   Enable LCD panel                                  *
         * --------------------------------------------------- */
        PUT_GRF_REG ( PR5, PR5_UNLOCK );
        PUT_GRF_REG ( PR4, PR4_ALL | 0x01 );
        PUT_GRF_REG ( PR5, PR5_LOCK );

        BUGLPR(dbg_initw, BUGNTA, ("    monitor_id = 0x%x\n",monitor_id));
        switch (monitor_id) {
           case 0xffff:             /* No monitor                          */
              crt_type = NO_CRT;
              break;
           case 0xf9f0:             /* IBM 9518                            */
           case 0xf9ff:             /* IBM 9515                            */
           case 0x99f0:             /* IBM 9517                            */
              crt_type = ISO_MON;
              break;
           case 0x0500:             /* VESA 16" 289x216mm, 70Hz 1024x768,  */
                                    /*                     72Hz  800x600,  */
                                    /*                     72Hz  640x480   */
           case 0x0505:             /* VESA 15" 268x201mm, 70Hz 1024x768,  */
                                    /*                     72Hz  800x600,  */
                                    /*                     72Hz  640x480   */
           case 0x0509:             /* VESA 17" 309x232mm, 70Hz 1024x768,  */
                                    /*                     72Hz  800x600,  */
                                    /*                     72Hz  640x480   */
           case 0x050F:             /* VESA 20" 370x277mm, 70Hz 1024x768,  */
                                    /*                     72Hz  800x600,  */
                                    /*                     72Hz  640x480   */
           case 0x055F:             /* VESA 21" 390x293mm, 70Hz 1024x768,  */
                                    /*                     72Hz  800x600,  */
                                    /*                     72Hz  640x480   */
           case 0x5555:             /* VESA 14" 248x186mm, 70Hz 1024x768,  */
                                    /*                     72Hz  800x600,  */
                                    /*                     72Hz  640x480   */
              crt_type = VESA_MON;
              break;
           case 0x0ff0:             /* IBM 9507 (640x480 TFT), 9509        */
           case 0x90f0:             /* IBM 8517                            */
           case 0xf00f:             /* IBM 8507,8604                       */
           case 0xf0f0:             /* IBM 8514,7554,631x,632x,952x        */
           case 0xf0ff:             /* IBM 8515,8516                       */
           case 0xff0f:             /* IBM 8503,8504,4707 (9i Monochrome)  */
           case 0xfff0:             /* IBM 8511,8512,8513,8518             */
           default:
              crt_type = NON_ISO_MON;
              break;
        }

	ddf->monitor_type = ld->monitor = crt_type;

        return;
}

/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : did2mid()                                            *
 *                                                                         *
 * DESCRIPTIVE NAME : Display ID to Monitor ID                             *
 *                                                                         *
 * FUNCTIONS : Convert the data from display id to monitor id              *
 *                                                                         *
 * INPUTS    : display id                                                  *
 *                                                                         *
 * OUTPUTS   : monitor id                                                  *
 * CALLED BY : init_wfg()                                                  *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

static int 
did2mid(int did) {
   unsigned int   mid = 0;

   if (did & 0x01) mid |= 0x0001;
   if (did & 0x02) mid |= 0x0010;
   if (did & 0x04) mid |= 0x0100;
   if (did & 0x08) mid |= 0x1000;

   return mid;
}


/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : lookup_v()                                           *
 *                                                                         *
 * DESCRIPTIVE NAME : Look up the pointer                                  *
 *                                                                         *
 * FUNCTIONS : Look up pointer to video parameters                         *
 *                                                                         *
 * INPUTS    : Pointer to ddf                                              *
 *                                                                         *
 * OUTPUTS   : Pointer to Video mode structure                             *
 * CALLED BY : init_wfg()                                                  *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

V_MODE *
lookup_v (ddf)
        struct wfg_ddf *ddf;
{
	switch ( ddf->panel_type ){
        case IBM_F8515:
                return &tft_vga; break;
		break;
	case IBM_F8532:
                return &tft_svga; break;
	default:
                return &tft_vga; break;
	}
}


/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : Get_Panel_type()                                     *
 *                                                                         *
 * DESCRIPTIVE NAME : SET VIDEO PARMS ACCORDING TO LCD PANEL USED          *
 *                                                                         *
 * FUNCTIONS : Read residual data of soft ROS, and get panel type data     *
 *                                                                         *
 * INPUTS    : Pointer to local defined structure (wfgldat)                *
 *             Pointer to ddf                                              *
 *             Pointer to dds                                              *
 *                                                                         *
 * OUTPUTS   : void                                                        *
 * CALLED BY : init_wfg()                                                  *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

void
Get_Panel_type (ld, ddf, dds)
    struct wfg_data *ld;
    struct wfg_ddf *ddf;
    struct fbdds   *dds;          /* contains config method data-prefilled */
{
    int   panel_id, model_id, wfg_type, vsync, hsync, size, panel_type=0;
    unsigned char  misc_val, crtc_val, pr19_val;
    RESIDUAL                 residual;

    get_residual(&residual, &size);

    panel_type = get_panel_type(&residual);

    switch ( panel_type ){
    case IBM_F8515:      /* IBM F8515 10.4" Color TFT */
         ld->panel_scrn_width_pix  = 640;
         ld->panel_scrn_height_pix = 480;
         ld->panel_scrn_width_mm   = 210;
	 break;
    case IBM_F8532:      /* IBM F8532 10.4" Color TFT SVGA */
         ld->panel_scrn_width_pix  = 800;
         ld->panel_scrn_height_pix = 600;
         ld->panel_scrn_width_mm   = 211;
	 break;
    default:
         ld->panel_scrn_width_pix  = 640;
         ld->panel_scrn_height_pix = 480;
         ld->panel_scrn_width_mm   = 210;
	 break;
    }
    ld->panel_scrn_height_mm    = 158;
    ddf->panel_type = ld->panel = panel_type;
}

/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : i_get_VDD_info()                                     *
 *                                                                         *
 * DESCRIPTIVE NAME : callback routine for device specific data            *
 *                                                                         *
 * FUNCTIONS : The exported function from vdd for panel resolution and     *
 *             device specific information.                                *
 *                                                                         *
 * INPUTS    : Pointer to VDD information structure                        *
 *                                                                         *
 * OUTPUTS   : void                                                        *
 * CALLED BY : Video Capture Device Driver                                 *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

void 
i_get_VDD_info( vddinfo )
        struct Vddinfo *vddinfo;
{
        RESIDUAL                 residual;
        int                      size, panel_type=0;

        get_residual(&residual, &size);

        panel_type = get_panel_type(&residual);

        vddinfo->v_sync      = 60;

	switch ( panel_type ){
	case IBM_F8515:      /* 640x480  VGA panel */
		vddinfo->lcd_w       = 640; 
		vddinfo->lcd_h       = 480;
	        vddinfo->v_total     = 523;
	        vddinfo->offset_x    = 42;
	        vddinfo->offset_y    = 32;
	        vddinfo->vram_clk    = 43;
	        vddinfo->vram_offset = 32;
		break;
	case IBM_F8532:      /* 800x600 SVGA panel */
		vddinfo->lcd_w       = 800; 
		vddinfo->lcd_h       = 600;
	        vddinfo->v_total     = 626;
	        vddinfo->offset_x    = 98;
	        vddinfo->offset_y    = 23;
	        vddinfo->vram_clk    = 99;
	        vddinfo->vram_offset = 23;
		break;
	default:
		vddinfo->lcd_w       = 640; 
		vddinfo->lcd_h       = 480;
	        vddinfo->v_total     = 0x20b;
	        vddinfo->offset_x    = 0x2a;
	        vddinfo->offset_y    = 0x20;
	        vddinfo->vram_clk    = 0x2b;
	        vddinfo->vram_offset = 32;
		break;
	}
}

/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : iplcb_get()                                          *
 *                                                                         *
 * DESCRIPTIVE NAME : Get the device specific information                  *
 *                                                                         *
 * FUNCTIONS : Read Composite Display ID, which is defined                 *
 *             as a part of XGA DMQS.                                      *
 *                                                                         *
 * INPUTS    : below                                                       *
 *                                                                         *
 * OUTPUTS   : Returns: -1 if can't get the data, otherwise 0              *
 * CALLED BY : get_residual()                                              *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

int
iplcb_get(dest, address, num_bytes, iocall)
        void *dest;
	int address;
	int num_bytes;
	int iocall;
{
        struct file                *fd;
	int rc;
        MACH_DD_IO        mdd;

        rc = fp_open("/dev/nvram",O_RDONLY,0,0,SYS_ADSPACE,&fd);
	if (rc){
                return(-1);
        }

        mdd.md_addr = (long)address;
        mdd.md_data = dest;
        mdd.md_size = (long)num_bytes;
        mdd.md_incr = MV_BYTE;

        if(fp_ioctl(fd,iocall,&mdd,0)){
                return -1;
        }

        fp_close(fd);
        return 0;
}

/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : get_residual()                                       *
 *                                                                         *
 * DESCRIPTIVE NAME : Get the device specific information                  *
 *                                                                         *
 * FUNCTIONS : Get the residual data from soft ROS                         *
 *                                                                         *
 * INPUTS    : Pointer to RESIDUAL structure                               *
 *             Pointer to residual data size                               *
 *                                                                         *
 * OUTPUTS   : void                                                        *
 * CALLED BY : Get_Panel_type() and i_get_VDD_info()                       *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

int
get_residual(rp, size)
        RESIDUAL *rp;
	int      *size;
{
        IPL_DIRECTORY        iplcb_dir;

        if(iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB)){
                return -1;
        }

        if(iplcb_get(rp, iplcb_dir.residual_offset, iplcb_dir.residual_size,
                                                                MIOIPLCB)){
                return -1;
        }
        *size = iplcb_dir.residual_size;
        return 0;
}

/***************************************************************************
 *                                                                         *
 * IDENTIFICATION   : get_panel_type()                                     *
 *                                                                         *
 * DESCRIPTIVE NAME : Gets the panel type                                  *
 *                                                                         *
 * FUNCTIONS : This function gets the panel type information from residual *
 *             data of soft ROS.                                           *
 *                                                                         *
 * INPUTS    : Pointer to RESIDUAL structure                               *
 *                                                                         *
 * OUTPUTS   : panel type                                                  *
 * CALLED BY : init_wfg()                                                  *
 * CALLS     : None                                                        *
 *                                                                         *
 ***************************************************************************/

int
get_panel_type(rp)
    RESIDUAL *rp;
{
    struct _DispInfoPack     *WD_Packet;
    int                      size, i, panel_type=0;

    for (i=0;i<rp->ActualNumDevices;i++ ){
         if ( (rp->Devices[i].DeviceId.BusId    == PCIDEVICE ) &&
              (rp->Devices[i].DeviceId.BaseType == DisplayController )){

                if (rp->Devices[i].AllocatedOffset) {
                    WD_Packet = (struct _DispInfoPack  *) 
			    &rp->DevicePnPHeap[rp->Devices[i].AllocatedOffset];
                        
                    while ( WD_Packet->Tag  != Vendor_Packet  ||
                            WD_Packet->Type != Display_Type ) {

                    /* ------------------------------------------- *
                     *   Look through all packets until            *
                     *        the 1st WD_packet is reached.        *
                     * ------------------------------------------- */

                        if ( WD_Packet->Tag & 0x80 ) {
                                WD_Packet = (struct _DispInfoPack  *)
                                        ((caddr_t) WD_Packet +
                                        (((struct _L1_Pack *) WD_Packet)->
                                            Count1 << 8) +
                                        ((struct _L1_Pack *) WD_Packet)->
                                            Count0 +3);
                        } else if ((WD_Packet->Tag & 0xf8) == 0x78) {
                                break;
                        } else {
                                WD_Packet = (struct _DispInfoPack  *) 
                                        ((caddr_t)WD_Packet +
                                        (WD_Packet->Tag & 0x07) +1);
                        }
                    }
                    if (WD_Packet->Tag != Vendor_Packet) {

                                /* ------------------------------------ *
                                 *  No packet found, logs the error     *
                                 * ------------------------------------ */

                                panel_type = NOT_FOUND;
                                wfg_err (NULL, "PCIWFG", "INIT_WFG", 
					  "GET_PANEL_TYPE", panel_type,
					  WFG_RESIDUAL_DATA_NOT_FOUND, 
					  RAS_UNIQUE_1);
                    } else {
                                panel_type = WD_Packet->DisplayID;
                    }
                } else {
                        /* ------------------------------------ *
                         *  No packet found, logs the error     *
                         * ------------------------------------ */
                        panel_type = NOT_FOUND;
                        wfg_err (NULL, "PCIWFG", "INIT_WFG", "GET_PANEL_TYPE", 
				 panel_type, WFG_RESIDUAL_DATA_NOT_FOUND, 
				RAS_UNIQUE_1);
                }
         }
    }
    return ( panel_type );
}
