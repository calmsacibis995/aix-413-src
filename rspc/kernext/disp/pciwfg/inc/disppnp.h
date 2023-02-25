/* @(#)79  1.1  src/rspc/kernext/disp/pciwfg/inc/disppnp.h, pciwfg, rspc41J, 9515B_all 4/11/95 06:19:37 */
/* disppnp.h */
/*
 *
 * COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS:     27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _DISPPNP_
#define _DISPPNP_

#define  Vendor_Packet    0x84           /* tag for DisplayInfoPack */
#define  Display_Type     0x04           /* type for display        */
#define  NOT_FOUND      0x8888

#ifdef _PANEL_TYPE

typedef enum _Display_Mode {
   Console_Text      = 0,              /* Video in VGA text mode             */
   Console_Video     = 1,              /* Video in 8 bpp graphics            */
   Console_Video16   = 2,              /* Video in 5,6,5 graphics            */
   Console_Video24   = 3,              /* Video in 8,8,8 graphics            */
   Console_Video32   = 4
   } Display_Mode;


typedef enum _Display_ID {
   UnknownDisplay    = 0x00,           // dxxxx
   IBM_F8532         = 0xfc,           /* IBM F8532 10.4" Color TFT SVGA     */
   TOSHIBA_STNC      = 0xfd,           /* Tosh TLX-8133S-C3X 10.4" D-S STN   */
   IBM_F8515         = 0xfe,           /* IBM F8515 10.4" Color TFT          */
   NoDisplay         = 0xff,           // dxxxx
   } Display_ID;

#endif /* _PANEL_TYPE */

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

#endif  /* ndef _DISPPNP_ */
