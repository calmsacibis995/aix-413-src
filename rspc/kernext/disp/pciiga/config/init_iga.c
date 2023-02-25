static char sccsid[] = "@(#)48	1.5  src/rspc/kernext/disp/pciiga/config/init_iga.c, pciiga, rspc41J, 9515B_all 3/22/95 17:00:22";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: enable the hardware
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

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                     S3 Graphics Adapter (IGA) driver                       */
/*                                                                            */
/*                         INITIALIZATION ROUTINES                            */
/*                                                                            */
/*      Needs:  pointer to local data structure                               */
/*                                                                            */
/*      Returns: -1 if can't find IGA, otherwise 0                            */
/*                                                                            */
/*      Called by: vtt_init                                                   */
/*                                                                            */
/*      Calls:  Get_Monitor                                                   */
/*              load_palette                                                  */
/*              iga_reset                                                     */
/*              enable_iga                                                    */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include "iga_INCLUDES.h"
#include "IO_defs.h"

void Get_Monitor();                     /* determine monitor, etc. */
void load_palette ();                   /* load Bt485 */
ulong lookup_v();

BUGXDEF (dbg_initf);
GS_MODULE(iga_init);
GS_MODULE(iga_Get_Monitor);
GS_MODULE(iga_lookup_v);

/* data courtesy Sandalfoot ROS */

static unsigned char crtc_6c_00[] = {   /* CRT Controller                       */
   0x99,                                /* Index 00h - Horizontal Total         */
   0x7f,                                /* Index 01h - Horizontal Display End   */
   0x7f,                                /* Index 02h - Start Horizontal Blank   */
   0x9c,                                /* Index 03h - End Horizontal Blank     */
   0x83,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x19,                                /* Index 05h - End Horizontal Sync Pos  */
   0x97,                                /* Index 06h - Vertical Total           */
   0x1f,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x40,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0x81,                                /* Index 10h - Vertical Retrace Start   */
   0x03,                                /* Index 11h - Vertical Retrace End     */
   0x7f,                                /* Index 12h - Vertical Display End     */
   0x80,                                /* Index 13h - Offset                   */
   0x00,                                /* Index 14h - Underline Location       */
   0x80,                                /* Index 15h - Start Vertical Blank     */
   0x96,                                /* Index 16h - End Vertical Blank       */
   0xe3,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_6c_01[] = {   /* CRT Controller                       */
   0xa3,                                /* Index 00h - Horizontal Total         */
   0x7f,                                /* Index 01h - Horizontal Display End   */
   0x80,                                /* Index 02h - Start Horizontal Blank   */
   0x86,                                /* Index 03h - End Horizontal Blank     */
   0x84,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x95,                                /* Index 05h - End Horizontal Sync Pos  */
   0x25,                                /* Index 06h - Vertical Total           */
   0xf5,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x60,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0x02,                                /* Index 10h - Vertical Retrace Start   */
   0x07,                                /* Index 11h - Vertical Retrace End     */
   0xff,                                /* Index 12h - Vertical Display End     */
   0x80,                                /* Index 13h - Offset                   */
   0x60,                                /* Index 14h - Underline Location       */
   0xff,                                /* Index 15h - Start Vertical Blank     */
   0x21,                                /* Index 16h - End Vertical Blank       */
   0xeb,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_6c_10[] = {   /* CRT Controller                       */
   0xa1,                                /* Index 00h - Horizontal Total         */
   0x7f,                                /* Index 01h - Horizontal Display End   */
   0x80,                                /* Index 02h - Start Horizontal Blank   */
   0x84,                                /* Index 03h - End Horizontal Blank     */
   0x84,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x95,                                /* Index 05h - End Horizontal Sync Pos  */
   0x24,                                /* Index 06h - Vertical Total           */
   0xf5,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x60,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0x02,                                /* Index 10h - Vertical Retrace Start   */
   0x08,                                /* Index 11h - Vertical Retrace End     */
   0xff,                                /* Index 12h - Vertical Display End     */
   0x80,                                /* Index 13h - Offset                   */
   0x60,                                /* Index 14h - Underline Location       */
   0xff,                                /* Index 15h - Start Vertical Blank     */
   0x24,                                /* Index 16h - End Vertical Blank       */
   0xeb,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_77_00[] = {   /* CRT Controller                       */
   0x4f,                                /* Index 00h - Horizontal Total         */
   0x3f,                                /* Index 01h - Horizontal Display End   */
   0x40,                                /* Index 02h - Start Horizontal Blank   */
   0x12,                                /* Index 03h - End Horizontal Blank     */
   0x42,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x0a,                                /* Index 05h - End Horizontal Sync Pos  */
   0x97,                                /* Index 06h - Vertical Total           */
   0x1f,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x40,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0x81,                                /* Index 10h - Vertical Retrace Start   */
   0x03,                                /* Index 11h - Vertical Retrace End     */
   0x7f,                                /* Index 12h - Vertical Display End     */
   0x00,                                /* Index 13h - Offset                   */
   0x00,                                /* Index 14h - Underline Location       */
   0x80,                                /* Index 15h - Start Vertical Blank     */
   0x96,                                /* Index 16h - End Vertical Blank       */
   0xe3,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_77_10[] = {   /* CRT Controller                       */
   0x4f,                                /* Index 00h - Horizontal Total         */
   0x3f,                                /* Index 01h - Horizontal Display End   */
   0x40,                                /* Index 02h - Start Horizontal Blank   */
   0x12,                                /* Index 03h - End Horizontal Blank     */
   0x42,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x0a,                                /* Index 05h - End Horizontal Sync Pos  */
   0x24,                                /* Index 06h - Vertical Total           */
   0xf5,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x60,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0x02,                                /* Index 10h - Vertical Retrace Start   */
   0x08,                                /* Index 11h - Vertical Retrace End     */
   0xff,                                /* Index 12h - Vertical Display End     */
   0x00,                                /* Index 13h - Offset                   */
   0x00,                                /* Index 14h - Underline Location       */
   0xff,                                /* Index 15h - Start Vertical Blank     */
   0x21,                                /* Index 16h - End Vertical Blank       */
   0xe3,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_49_00[] = {   /* CRT Controller                       */
   0x5f,                                /* Index 00h - Horizontal Total         */
   0x4f,                                /* Index 01h - Horizontal Display End   */
   0x50,                                /* Index 02h - Start Horizontal Blank   */
   0x82,                                /* Index 03h - End Horizontal Blank     */
   0x54,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x80,                                /* Index 05h - End Horizontal Sync Pos  */
   0x0b,                                /* Index 06h - Vertical Total           */
   0x3e,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x40,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0xea,                                /* Index 10h - Vertical Retrace Start   */
   0x0c,                                /* Index 11h - Vertical Retrace End     */
   0xdf,                                /* Index 12h - Vertical Display End     */
   0x80,                                /* Index 13h - Offset                   */
   0x60,                                /* Index 14h - Underline Location       */
   0xe7,                                /* Index 15h - Start Vertical Blank     */
   0x04,                                /* Index 16h - End Vertical Blank       */
   0xab,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_49_01[] = {   /* CRT Controller                       */
   0x63,                                /* Index 00h - Horizontal Total         */
   0x4f,                                /* Index 01h - Horizontal Display End   */
   0x50,                                /* Index 02h - Start Horizontal Blank   */
   0x86,                                /* Index 03h - End Horizontal Blank     */
   0x53,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x98,                                /* Index 05h - End Horizontal Sync Pos  */
   0x07,                                /* Index 06h - Vertical Total           */
   0x3e,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x40,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0xe8,                                /* Index 10h - Vertical Retrace Start   */
   0x0b,                                /* Index 11h - Vertical Retrace End     */
   0xdf,                                /* Index 12h - Vertical Display End     */
   0x80,                                /* Index 13h - Offset                   */
   0x60,                                /* Index 14h - Underline Location       */
   0xdf,                                /* Index 15h - Start Vertical Blank     */
   0x07,                                /* Index 16h - End Vertical Blank       */
   0xab,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_6e_00[] = {   /* CRT Controller                       */
   0x2e,                                /* Index 00h - Horizontal Total         */
   0x27,                                /* Index 01h - Horizontal Display End   */
   0x28,                                /* Index 02h - Start Horizontal Blank   */
   0x91,                                /* Index 03h - End Horizontal Blank     */
   0x2a,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x90,                                /* Index 05h - End Horizontal Sync Pos  */
   0x0b,                                /* Index 06h - Vertical Total           */
   0x3e,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x40,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0xea,                                /* Index 10h - Vertical Retrace Start   */
   0x0c,                                /* Index 11h - Vertical Retrace End     */
   0xdf,                                /* Index 12h - Vertical Display End     */
   0x00,                                /* Index 13h - Offset                   */
   0x00,                                /* Index 14h - Underline Location       */
   0xe7,                                /* Index 15h - Start Vertical Blank     */
   0x04,                                /* Index 16h - End Vertical Blank       */
   0xe3,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_72_00[] = {   /* CRT Controller                       */
   0xc1,                                /* Index 00h - Horizontal Total         */
   0x9f,                                /* Index 01h - Horizontal Display End   */
   0x9f,                                /* Index 02h - Start Horizontal Blank   */
   0x83,                                /* Index 03h - End Horizontal Blank     */
   0xa4,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x1f,                                /* Index 05h - End Horizontal Sync Pos  */
   0x1b,                                /* Index 06h - Vertical Total           */
   0xb2,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x60,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0x01,                                /* Index 10h - Vertical Retrace Start   */
   0x05,                                /* Index 11h - Vertical Retrace End     */
   0xff,                                /* Index 12h - Vertical Display End     */
   0x80,                                /* Index 13h - Offset                   */
   0x00,                                /* Index 14h - Underline Location       */
   0x00,                                /* Index 15h - Start Vertical Blank     */
   0x19,                                /* Index 16h - End Vertical Blank       */
   0xe3,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_72_10[] = {   /* CRT Controller                       */
   0xce,                                /* Index 00h - Horizontal Total         */
   0x9f,                                /* Index 01h - Horizontal Display End   */
   0xa0,                                /* Index 02h - Start Horizontal Blank   */
   0x11,                                /* Index 03h - End Horizontal Blank     */
   0xa5,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x12,                                /* Index 05h - End Horizontal Sync Pos  */
   0x33,                                /* Index 06h - Vertical Total           */
   0x52,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x40,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0x01,                                /* Index 10h - Vertical Retrace Start   */
   0x06,                                /* Index 11h - Vertical Retrace End     */
   0xff,                                /* Index 12h - Vertical Display End     */
   0xa0,                                /* Index 13h - Offset                   */
   0x00,                                /* Index 14h - Underline Location       */
   0x00,                                /* Index 15h - Start Vertical Blank     */
   0x31,                                /* Index 16h - End Vertical Blank       */
   0xa3,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char crtc_72_11[] = {   /* CRT Controller                       */
   0xce,                                /* Index 00h - Horizontal Total         */
   0x9f,                                /* Index 01h - Horizontal Display End   */
   0xa0,                                /* Index 02h - Start Horizontal Blank   */
   0x11,                                /* Index 03h - End Horizontal Blank     */
   0xa5,                                /* Index 04h - Start Horizontal Sync Pos*/
   0x12,                                /* Index 05h - End Horizontal Sync Pos  */
   0x33,                                /* Index 06h - Vertical Total           */
   0x52,                                /* Index 07h - CRTC Overflow            */
   0x00,                                /* Index 08h - Preset Row Scan          */
   0x40,                                /* Index 09h - Maximum Scan Line        */
   0x00,                                /* Index 0ah - Cursor Start Scan Line   */
   0x00,                                /* Index 0bh - Cursor End Scan Line     */
   0x00,                                /* Index 0ch - Start Address High       */
   0x00,                                /* Index 0dh - Start Address Low        */
   0xff,                                /* Index 0eh - Cursor Address High      */
   0x00,                                /* Index 0fh - Cursor Address Low       */
   0x01,                                /* Index 10h - Vertical Retrace Start   */
   0x06,                                /* Index 11h - Vertical Retrace End     */
   0xff,                                /* Index 12h - Vertical Display End     */
   0xa0,                                /* Index 13h - Offset                   */
   0x00,                                /* Index 14h - Underline Location       */
   0x00,                                /* Index 15h - Start Vertical Blank     */
   0x31,                                /* Index 16h - End Vertical Blank       */
   0xa3,                                /* Index 17h - CRTC Mode Control        */
   0xff                                 /* Index 18h - Line Compare             */
};

static unsigned char ecrtc_addrs[] = {  /* Extended CRT Controller Addresses    */
   0x42,                                /* Index 42h - Mode Control             */
   0x3b,                                /* Index 3bh - Data Transfer Execute Pos*/
   0x3c,                                /* Index 3ch - Interface Retrace Start  */
   0x31,                                /* Index 31h - Memory Configuration     */
   0x3a,                                /* Index 3ah - Miscellaneous 1          */
   0x40,                                /* Index 40h - System Configuration     */
   0x50,                                /* Index 50h - Extended System Control 1*/
   0x54,                                /* Index 54h - Extended Memory Control 2*/
   0x5d,                                /* Index 5dh - Ext Horizontal Overflow  */
   0x60,                                /* Index 60h - Extended Memory Control 3*/
   0x61,                                /* Index 61h - Extended Memory Control 4*/
   0x62,                                /* Index 62h - Extended Memory Control 5*/
   0x58,                                /* Index 58h - Linear Address Window Ctl*/
   0x33,                                /* Index 33h - Backward Compatibility 2 */
   0x43,                                /* Index 43h - Extended Mode            */
   0x13,                                /* Index 13h - Offset                   */
   0x5e,                                /* Index 5eh - Extended Vertical Overflw*/
   0x51,                                /* Index 51h - Extended System Control 2*/
   0x5c,                                /* Index 5ch - General Output Port      */
   0x34,                                /* Index 34h - Backward Compatibility 3 */
   0x00                                 /* List Terminator                      */
};

static unsigned char ecrtc_6c_00[] = {  /* Extended CRT Controller              */
         0x27,  /* 45.00 Mhz */         /* Index 42h - Mode Control             */
         0x94,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x4f,                          /* Index 3ch - Interface Retrace Start  */
         0x89, /*8b*/                   /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x00,                          /* Index 50h - Extended System Control 1*/
         0x08,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x17,                          /* Index 60h - Extended Memory Control 3*/
         0x81,                          /* Index 61h - Extended Memory Control 4*/
         0x01,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x40,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_6c_10[] = {  /* Extended CRT Controller              */
         0x0d, /* 65.00 Mhz */          /* Index 42h - Mode Control             */
         0x9d,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x9b,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x00,                          /* Index 50h - Extended System Control 1*/
         0x08,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x1f,                          /* Index 60h - Extended Memory Control 3*/
         0x80,                          /* Index 61h - Extended Memory Control 4*/
         0x80,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x40,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_6c_11[] = {  /* Extended CRT Controller              */
         0x0e,  /* 75.00 Mhz */         /* Index 42h - Mode Control             */
         0x9d,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x40,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x00,                          /* Index 50h - Extended System Control 1*/
         0x08,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x1f,                          /* Index 60h - Extended Memory Control 3*/
         0x80,                          /* Index 61h - Extended Memory Control 4*/
         0x80,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x40,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_6d_00[] = {  /* Extended CRT Controller              */
         0x27,   /* 45.00 Mhz */        /* Index 42h - Mode Control             */
         0x94,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x4f,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x00,                          /* Index 50h - Extended System Control 1*/
         0x18,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x0f,                          /* Index 60h - Extended Memory Control 3*/
         0x81,                          /* Index 61h - Extended Memory Control 4*/
         0x00,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x80,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_6d_10[] = {  /* Extended CRT Controller              */
         0x0d, /* 65.00 Mhz */          /* Index 42h - Mode Control             */
         0x9d,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x80,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x00,                          /* Index 50h - Extended System Control 1*/
         0x08,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x1f,                          /* Index 60h - Extended Memory Control 3*/
         0x81,                          /* Index 61h - Extended Memory Control 4*/
         0x00,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x80,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_6d_11[] = {  /* Extended CRT Controller              */
         0x0a,   /* 80.00 Mhz */        /* Index 42h - Mode Control             */
         0x9b,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x80,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x00,                          /* Index 50h - Extended System Control 1*/
         0x08,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x1f,                          /* Index 60h - Extended Memory Control 3*/
         0x81,                          /* Index 61h - Extended Memory Control 4*/
         0x01,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x80,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_77_00[] = {  /* Extended CRT Controller              */
         0x27,  /* 45.00 Mhz */         /* Index 42h - Mode Control             */
         0x3e,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x2a,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x10,                          /* Index 50h - Extended System Control 1*/
         0x00,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x3f,                          /* Index 60h - Extended Memory Control 3*/
         0x82,                          /* Index 61h - Extended Memory Control 4*/
         0x00,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x20,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x00,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x50,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static char ecrtc_77_10[] = {           /* Extended CRT Controller              */
         0x0d,  /* 65.00 Mhz */         /* Index 42h - Mode Control             */
         0x4b,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x20,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x10,                          /* Index 50h - Extended System Control 1*/
         0x00,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x2f,                          /* Index 60h - Extended Memory Control 3*/
         0x82,                          /* Index 61h - Extended Memory Control 4*/
         0x00,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x20,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x00,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x50,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_49_00[] = {  /* Extended CRT Controller              */
         0x00, /* see 3c2 */            /* Index 42h - Mode Control             */
         0x5a,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x91,                          /* Index 3ch - Interface Retrace Start  */
         0x88,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x40,                          /* Index 50h - Extended System Control 1*/
         0x38,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x07,                          /* Index 60h - Extended Memory Control 3*/
         0x81,                          /* Index 61h - Extended Memory Control 4*/
         0x01,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x50,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_49_10[] = {  /* Extended CRT Controller              */
         0x00, /* see 3c2 */            /* Index 42h - Mode Control             */
         0x5a,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x91,                          /* Index 3ch - Interface Retrace Start  */
         0x88,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x40,                          /* Index 50h - Extended System Control 1*/
         0x38,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x07,                          /* Index 60h - Extended Memory Control 3*/
         0x81,                          /* Index 61h - Extended Memory Control 4*/
         0x01,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x50,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_49_11[] = {  /* Extended CRT Controller              */
         0x0b,   /* 31.5 Mhz */         /* Index 42h - Mode Control             */
         0x5e,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x40,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x40,                          /* Index 50h - Extended System Control 1*/
         0x00,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x0f,                          /* Index 60h - Extended Memory Control 3*/
         0x80,                          /* Index 61h - Extended Memory Control 4*/
         0xa1,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0x50,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_6e_10[] = {  /* Extended CRT Controller              */
         0x02,    /* 40.00 Mhz */       /* Index 42h - Mode Control             */
         0xbe,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x40,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0x40,                          /* Index 50h - Extended System Control 1*/
         0x18,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x0f,                          /* Index 60h - Extended Memory Control 3*/
         0x81,                          /* Index 61h - Extended Memory Control 4*/
         0x40,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x20,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0xa0,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x00                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_72_00[] = {  /* Extended CRT Controller              */
         0x2e,    /* 75.00 Mhz */       /* Index 42h - Mode Control             */
         0xc0,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x63,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0xc0,                          /* Index 50h - Extended System Control 1*/
         0x08,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x2f,                          /* Index 60h - Extended Memory Control 3*/
         0x81,                          /* Index 61h - Extended Memory Control 4*/
         0x40,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0xa0,                          /* Index 13h - Offset                   */
         0x00,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x10                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_72_10[] = {  /* Extended CRT Controller              */
         0x0c,    /* 110.00 Mhz */      /* Index 42h - Mode Control             */
         0xbb,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x60,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0xc0,                          /* Index 50h - Extended System Control 1*/
         0x88,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x1f,                          /* Index 60h - Extended Memory Control 3*/
         0x80,                          /* Index 61h - Extended Memory Control 4*/
         0xa0,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0xa0,                          /* Index 13h - Offset                   */
         0x55,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x01                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char ecrtc_72_11[] = {  /* Extended CRT Controller              */
         0x08,    /* 110.00 Mhz */      /* Index 42h - Mode Control             */
         0xbb,                          /* Index 3bh - Data Transfer Execute Pos*/
         0x60,                          /* Index 3ch - Interface Retrace Start  */
         0x89,                          /* Index 31h - Memory Configuration     */
         0x15,                          /* Index 3ah - Miscellaneous 1          */
         0x01,                          /* Index 40h - System Configuration     */
         0xc0,                          /* Index 50h - Extended System Control 1*/
         0x88,                          /* Index 54h - Extended Memory Control 2*/
         0x00,                          /* Index 5dh - Ext Horizontal Overflow  */
         0x1f,                          /* Index 60h - Extended Memory Control 3*/
         0x80,                          /* Index 61h - Extended Memory Control 4*/
         0xa0,                          /* Index 62h - Extended Memory Control 5*/
         0x12,                          /* Index 58h - Linear Address Window Ctl*/
         0x00,                          /* Index 33h - Backward Compatibility 2 */
         0x00,                          /* Index 43h - Extended Mode            */
         0xa0,                          /* Index 13h - Offset                   */
         0x55,                          /* Index 5eh - Extended Vertical Overflw*/
         0x00,                          /* Index 51h - Extended System Control 2*/
         0x00,                          /* Index 5ch - General Output Port      */
         0x01                           /* Index 34h - Backward Compatibility 3 */
};

static unsigned char s3_864_49_10[] = { /* 864 Graphics Controller extensions   */

   0x36, 0x8e,
   0x37, 0x0f,
   0x54, 0xd8,                          /* M                                    */
   0x60, 0xff,                          /* N                                    */
   0x61, 0x80,                          /* L                                    */
   0x62, 0x50,                          /* L part 2                             */
   0x68, 0xac,                          /* Memory Timings                       */
   0x00, 0x00,                          /* List Terminator  for VXP extensions  */
};

static unsigned char s3_864_49_11[] = { /* 864 Graphics Controller extensions   */

   0x36, 0x8e,
   0x37, 0x0f,
   0x54, 0xb8,                          /* M                                    */
   0x60, 0xff,                          /* N                                    */
   0x61, 0x80,                          /* L                                    */
   0x62, 0x50,                          /* L part 2                             */
   0x68, 0xac,                          /* Memory Timings                       */
   0x00, 0x00,                          /* List Terminator  for VXP extensions  */
};

static unsigned char s3_864_6d_00[] = { /* 864 Graphics Controller extensions   */

   0x36, 0x8e,
   0x37, 0x0f,
   0x54, 0xb0,                          /* M                                    */
   0x60, 0xff,                          /* N                                    */
   0x61, 0x80,                          /* L                                    */
   0x62, 0x80,                          /* L part 2                             */
   0x68, 0xac,                          /* Memory Timings                       */
   0x00, 0x00,                          /* List Terminator  for VXP extensions  */
};

static unsigned char s3_864_6d_10[] = { /* 864 Graphics Controller extensions   */

   0x36, 0x8e,
   0x37, 0x0f,
   0x54, 0xa0,                          /* M                                    */
   0x60, 0xff,                          /* N                                    */
   0x61, 0x80,                          /* L                                    */
   0x62, 0x80,                          /* L part 2                             */
   0x68, 0xac,                          /* Memory Timings                       */
   0x00, 0x00,                          /* List Terminator  for VXP extensions  */
};

static unsigned char s3_864_6d_11[] = { /* 864 Graphics Controller extensions   */

   0x36, 0x8e,
   0x37, 0x0f,
   0x54, 0x88,                          /* M                                    */
   0x60, 0xff,                          /* N                                    */
   0x61, 0x80,                          /* L                                    */
   0x62, 0x80,                          /* L part 2                             */
   0x68, 0xac,                          /* Memory Timings                       */
   0x00, 0x00,                          /* List Terminator  for VXP extensions  */
};

static unsigned char s3_864_72_00[] = { /* 864 Graphics Controller extensions   */

   0x36, 0x8e,
   0x37, 0x0f,
   0x54, 0xa8,                          /* M                                    */
   0x60, 0xff,                          /* N                                    */
   0x61, 0x80,                          /* L                                    */
   0x62, 0xa0,                          /* L part 2                             */
   0x67, 0x10,                          /* Set as two 8-bit pels/VCLK           */
   0x68, 0xac,                          /* Memory Timings                       */
   0x00, 0x00,                          /* List Terminator  for VXP extensions  */
};


static unsigned char s3_864_72_10[] = { /* 864 Graphics Controller extensions   */

   0x36, 0x8e,
   0x37, 0x0f,
   0x54, 0x98,                          /* M                                    */
   0x60, 0xff,                          /* N                                    */
   0x61, 0x80,                          /* L                                    */
   0x62, 0xa0,                          /* L part 2                             */
   0x67, 0x10,                          /* Set as two 8-bit pels/VCLK           */
   0x68, 0xac,                          /* Memory Timings                       */
   0x00, 0x00,                          /* List Terminator  for VXP extensions  */
};

static unsigned char s3_864_72_11[] = { /* 864 Graphics Controller extensions   */

   0x36, 0x8e,
   0x37, 0x0f,
   0x54, 0x88,                          /* M                                    */
   0x60, 0xff,                          /* N                                    */
   0x61, 0x80,                          /* L                                    */
   0x62, 0xa0,                          /* L part 2                             */
   0x67, 0x10,                          /* Set as two 8-bit pels/VCLK           */
   0x68, 0xac,                          /* Memory Timings                       */
   0x00, 0x00,                          /* List Terminator  for VXP extensions  */
};

static CRT_MODE_TABLE Mode_6C_Table_0[] = {
  crtc_6c_00,
  ecrtc_6c_00,
  dev_ecrtc_none,
  0,0,0,0,
};

static CRT_MODE_TABLE Mode_6C_Table_2[] = {
  crtc_6c_01,
  ecrtc_6c_10,
  dev_ecrtc_none,
  0,0,0,0,
};

static CRT_MODE_TABLE Mode_6C_Table_3[] = {
  crtc_6c_10,
  ecrtc_6c_11,
  dev_ecrtc_none,
  0,0,0,0,
};

static CRT_MODE_TABLE Mode_6D_Table_0[] = {
  crtc_6c_00,
  ecrtc_6d_00,
  s3_864_6d_00,
  0x00,                         /* CMD = 0, 8 bit color (LUT)           */
  0x56,                         /* M = 86           45.0000Mhz          */
  0x45,                         /* N1 = 5 , N2 = 2                      */
  0,                            /* S3 DAC unique data                   */
};

static CRT_MODE_TABLE Mode_6D_Table_2[] = {
  crtc_6c_01,
  ecrtc_6d_10,
  s3_864_6d_10,
  0x00,                         /* CMD = 0, 8 bit color (LUT)           */
  0x6b,                         /* M = 107          65.0284Mhz          */
  0x2a,                         /* N1 = 10, N2 = 1                      */
  0,                            /* S3 DAC unique data                   */
};

static CRT_MODE_TABLE Mode_6D_Table_3[] = {
  crtc_6c_10,
  ecrtc_6d_11,
  s3_864_6d_11,
  0x00,                         /* CMD = 0, 8 bit color (LUT)           */
  0x41,                         /* M = 65           79.9432Mhz          */
  0x24,                         /* N1 = 4, N2 = 1                       */
  0,                            /* S3 DAC unique data                   */
};

static CRT_MODE_TABLE Mode_77_Table_0[] = {
  crtc_77_00,
  ecrtc_77_00,
  dev_ecrtc_none,
  0,0,0,0,
};

static CRT_MODE_TABLE Mode_77_Table_2[] = {
  crtc_77_10,
  ecrtc_77_10,
  dev_ecrtc_none,
  0,0,0,0,
};

static CRT_MODE_TABLE Mode_49_Table_2[] = {
  crtc_49_00,
  ecrtc_49_10,
  s3_864_49_11,
  0x00,                         /* CMD = 0, 8 bit color (LUT)           */
  0x7d,                         /* M = 125              25.256Mhz       */
  0x50,                         /* N1 = 16, N2 = 2                      */
  0,                            /* S3 DAC unique data                   */
};

static CRT_MODE_TABLE Mode_49_Table_3[] = {
  crtc_49_01,
  ecrtc_49_11,
  s3_864_49_11,
  0x00,                         /* CMD = 0, 8 bit color (LUT)           */
  0x56,                         /* M = 86               31.50Mhz        */
  0x48,                         /* N1 = 8, N2 = 2                       */
  0,                            /* S3 DAC unique data                   */
};

static CRT_MODE_TABLE Mode_6e_Table_2[] = {
  crtc_6e_00,
  ecrtc_6e_10,
  dev_ecrtc_none,
  0,0,0,0,
};

static CRT_MODE_TABLE Mode_4f_Table_0[] = {
  crtc_49_00,
  ecrtc_49_00,
  dev_ecrtc_none,
  0,0,0,0,
};

static CRT_MODE_TABLE Mode_4f_Table_2[] = {
  crtc_49_00,
  ecrtc_49_10,
  dev_ecrtc_none,
  0,0,0,0,
};

static CRT_MODE_TABLE Mode_4f_Table_3[] = {
  crtc_49_00,
  ecrtc_49_10,
  dev_ecrtc_none,
  0,0,0,0,
};

static CRT_MODE_TABLE Mode_72_Table_0[] = {
  crtc_72_00,
  ecrtc_72_00,
  s3_864_72_00,
  0x10,                         /* CMD = 0, two 8 bit color (LUT)       */
  0x71,                         /* M = 113           74.8450Mhz         */
  0x29,                         /* N1 =  9, N2 = 1                      */
  0,                            /* S3 DAC unique data                   */
};

static CRT_MODE_TABLE Mode_72_Table_2[] = {
  crtc_72_10,
  ecrtc_72_10,
  s3_864_72_10,
  0x10,                         /* CMD = 0, two 8 bit color (LUT)       */
  0x79,                         /* M = 121         110.0710Mhz          */
  0x0e,                         /* N1 = 14, N2 = 0                      */
  0,                            /* S3 DAC unique data                   */
};

static CRT_MODE_TABLE Mode_72_Table_3[] = {
  crtc_72_11,
  ecrtc_72_11,
  s3_864_72_11,
  0x10,                         /* CMD = 0, two 8 bit color (LUT)       */
  0x6b,                         /* M = 107          130.056Mhz          */
  0x0a,                         /* N1 = 10, N2 = 0                      */
/*0x40, */                      /* M = 64           135.000Mhz          */
/*0x05, */                      /* N1 =  5, N2 = 0                      */
  0,                            /* S3 DAC unique data                   */
};

static BASE_MODE_TABLE Mode_1280x1024_Table = {
   1024,                                /* Screen Height                        */
   1280,                                /* Screen Width                         */
   65536,                               /* Regen Buffer Size                    */

   0x2f,                                /* Miscellaneous Output for Monitor = 0 */
   0x2f,                                /* Miscellaneous Output for Monitor = 1 */
   0x2f,                                /* Miscellaneous Output for Monitor = 2 */
   0x2f,                                /* Miscellaneous Output for Monitor = 3 */

   16,                                  /* xfudge for Monitor = 0               */
   16,                                  /* xfudge for Monitor = 1               */
   8,                                   /* xfudge for Monitor = 2               */
   8,                                   /* xfudge for Monitor = 3               */

   3,                                   /* yfudge for Monitor = 0               */
   3,                                   /* yfudge for Monitor = 1               */
   3,                                   /* yfudge for Monitor = 2               */
   3,                                   /* yfudge for Monitor = 3               */

                                        /* Sequencer Data                       */
   0x01,                                /* - Clocking Mode                      */
   0x0f,                                /* - Enable Write Plane                 */
   0x00,                                /* - Character Font Select              */
   0x06,                                /* - Memory Mode Control                */

                                        /* Attribute Controller Data            */
   0x00,                                /* Index 00h - Palette Reg 00h          */
   0x01,                                /* Index 01h - Palette Reg 01h          */
   0x02,                                /* Index 02h - Palette Reg 02h          */
   0x03,                                /* Index 03h - Palette Reg 03h          */
   0x04,                                /* Index 04h - Palette Reg 04h          */
   0x05,                                /* Index 05h - Palette Reg 05h          */
   0x14,                                /* Index 06h - Palette Reg 06h          */
   0x07,                                /* Index 07h - Palette Reg 07h          */
   0x38,                                /* Index 08h - Palette Reg 08h          */
   0x39,                                /* Index 09h - Palette Reg 09h          */
   0x3a,                                /* Index 0ah - Palette Reg 0ah          */
   0x3b,                                /* Index 0bh - Palette Reg 0bh          */
   0x3c,                                /* Index 0ch - Palette Reg 0ch          */
   0x3d,                                /* Index 0dh - Palette Reg 0dh          */
   0x3e,                                /* Index 0eh - Palette Reg 0eh          */
   0x3f,                                /* Index 0fh - Palette Reg 0fh          */
   0x01,                                /* Index 10h - Attribute Mode Ctl            */
   0x00,                                /* Index 11h - Border Coloer            */
   0x0f,                                /* Index 12h - Color Plane Enable       */
   0x00,                                /* Index 13h - Horizontal Pixel Panning */

                                        /* Graphics Controller                  */
   0x00,                                /* Index 00h - Set/Reset Data           */
   0x00,                                /* Index 01h - Enable Set/Reset Data    */
   0x00,                                /* Index 02h - Color Compare            */
   0x00,                                /* Index 03h - Raster Op/Rotate Count   */
   0x00,                                /* Index 04h - Read Plane Select        */
   0x00,                                /* Index 05h - Graphics Controller Mode */
   0x05,                                /* Index 06h - Memory Map Mode Control  */
   0x0f,                                /* Index 07h - Color Don't Care         */
   0xff                                 /* Index 08h - Bit Mask                 */
};

static BASE_MODE_TABLE Mode_1024x768_Table = {
       768,                           /* Screen Height                        */
       1024,                          /* Screen Width                         */
       65536,                         /* Regen Buffer Size                    */

       0x2f,                          /* Miscellaneous Output for Monitor = 0 */
       0x2f,                          /* Miscellaneous Output for Monitor = 1 */
       0xef,                          /* Miscellaneous Output for Monitor = 2 */
       0x2f,                          /* Miscellaneous Output for Monitor = 3 */

       8,                             /* xfudge for Monitor = 0               */
       8,                             /* xfudge for Monitor = 1               */
       8,                             /* xfudge for Monitor = 2               */
       8,                             /* xfudge for Monitor = 3               */

       3,                             /* yfudge for Monitor = 0               */
       3,                             /* yfudge for Monitor = 1               */
       5,                             /* yfudge for Monitor = 2               */
       1,                             /* yfudge for Monitor = 3               */

                                      /* Sequencer Data                       */
       0x01,                          /* - Clocking Mode                      */
       0x0f,                          /* - Enable Write Plane                 */
       0x00,                          /* - Character Font Select              */
       0x06,                          /* - Memory Mode Control                */

                                      /* Attribute Controller Data            */
       0x00,                          /* Index 00h - Palette Reg 00h          */
       0x01,                          /* Index 01h - Palette Reg 01h          */
       0x02,                          /* Index 02h - Palette Reg 02h          */
       0x03,                          /* Index 03h - Palette Reg 03h          */
       0x04,                          /* Index 04h - Palette Reg 04h          */
       0x05,                          /* Index 05h - Palette Reg 05h          */
       0x14,                          /* Index 06h - Palette Reg 06h          */
       0x07,                          /* Index 07h - Palette Reg 07h          */
       0x38,                          /* Index 08h - Palette Reg 08h          */
       0x39,                          /* Index 09h - Palette Reg 09h          */
       0x3a,                          /* Index 0ah - Palette Reg 0ah          */
       0x3b,                          /* Index 0bh - Palette Reg 0bh          */
       0x3c,                          /* Index 0ch - Palette Reg 0ch          */
       0x3d,                          /* Index 0dh - Palette Reg 0dh          */
       0x3e,                          /* Index 0eh - Palette Reg 0eh          */
       0x3f,                          /* Index 0fh - Palette Reg 0fh          */
       0x01,                          /* Index 10h - Attribute Mode Ctl            */
       0x00,                          /* Index 11h - Border Coloer            */
       0x0f,                          /* Index 12h - Color Plane Enable       */
       0x00,                          /* Index 13h - Horizontal Pixel Panning */

                                      /* Graphics Controller                  */
       0x00,                          /* Index 00h - Set/Reset Data           */
       0x00,                          /* Index 01h - Enable Set/Reset Data    */
       0x00,                          /* Index 02h - Color Compare            */
       0x00,                          /* Index 03h - Raster Op/Rotate Count   */
       0x00,                          /* Index 04h - Read Plane Select        */
       0x00,                          /* Index 05h - Graphics Controller Mode */
       0x05,                          /* Index 06h - Memory Map Mode Control  */
       0x0f,                          /* Index 07h - Color Don't Care         */
       0xff                           /* Index 08h - Bit Mask                 */
       };

static BASE_MODE_TABLE Mode_640x480_Table = {
   480,                                 /* Screen Height                        */
   640,                                 /* Screen Width                         */
   65536,                               /* Regen Buffer Size                    */

   0xef,                                /* Miscellaneous Output for Monitor = 0 */
   0xef,                                /* Miscellaneous Output for Monitor = 1 */
   0xef,                                /* Miscellaneous Output for Monitor = 2 */
   0xef,                                /* Miscellaneous Output for Monitor = 3 */

   8,                                   /* xfudge for Monitor = 0               */
   8,                                   /* xfudge for Monitor = 1               */
   8,                                   /* xfudge for Monitor = 2               */
   8,                                   /* xfudge for Monitor = 3               */

   8,                                   /* yfudge for Monitor = 0               */
   8,                                   /* yfudge for Monitor = 1               */
   8,                                   /* yfudge for Monitor = 2               */
   1,                                   /* yfudge for Monitor = 3               */

                                        /* Sequencer Data                       */
   0x01,                                /* - Clocking Mode                      */
   0x0f,                                /* - Enable Write Plane                 */
   0x00,                                /* - Character Font Select              */
   0x0e,                                /* - Memory Mode Control                */

                                        /* Attribute Controller Data            */
   0x00,                                /* Index 00h - Palette Reg 00h          */
   0x01,                                /* Index 01h - Palette Reg 01h          */
   0x02,                                /* Index 02h - Palette Reg 02h          */
   0x03,                                /* Index 03h - Palette Reg 03h          */
   0x04,                                /* Index 04h - Palette Reg 04h          */
   0x05,                                /* Index 05h - Palette Reg 05h          */
   0x06,                                /* Index 06h - Palette Reg 06h          */
   0x07,                                /* Index 07h - Palette Reg 07h          */
   0x10,                                /* Index 08h - Palette Reg 08h          */
   0x11,                                /* Index 09h - Palette Reg 09h          */
   0x12,                                /* Index 0ah - Palette Reg 0ah          */
   0x13,                                /* Index 0bh - Palette Reg 0bh          */
   0x14,                                /* Index 0ch - Palette Reg 0ch          */
   0x15,                                /* Index 0dh - Palette Reg 0dh          */
   0x16,                                /* Index 0eh - Palette Reg 0eh          */
   0x17,                                /* Index 0fh - Palette Reg 0fh          */
   0x41,                                /* Index 10h - Attribute Mode Ctl            */
   0x00,                                /* Index 11h - Border Coloer            */
   0x0f,                                /* Index 12h - Color Plane Enable       */
   0x00,                                /* Index 13h - Horizontal Pixel Panning */

                                        /* Graphics Controller                  */
   0x00,                                /* Index 00h - Set/Reset Data           */
   0x00,                                /* Index 01h - Enable Set/Reset Data    */
   0x00,                                /* Index 02h - Color Compare            */
   0x00,                                /* Index 03h - Raster Op/Rotate Count   */
   0x00,                                /* Index 04h - Read Plane Select        */
   0x40,                                /* Index 05h - Graphics Controller Mode */
   0x05,                                /* Index 06h - Memory Map Mode Control  */
   0x0f,                                /* Index 07h - Color Don't Care         */
   0xff                                 /* Index 08h - Bit Mask                 */
};

static VESA_MODE vesa_204 = {           /* 1024 x 768 x 4                       */
        0x4c,                           /* Internal Mode Number                 */
        &Mode_1024x768_Table,           /* Base Mode Table                      */
        0x06,                           /* Sequence 04h                         */
        0x07,                           /* Advanced Function Register           */
        0x00,                           /* DAC Mode                             */
        4,                              /* 4 bits/pel                           */
        Mode_6C_Table_0,                /* Table for Monitor type 0 43HZ I      */
        Mode_6C_Table_0,                /* Table for Monitor type 1 56HZ NI     */
        Mode_6C_Table_2,                /* Table for Monitor type 2 60HZ NI     */
        Mode_6C_Table_3 };              /* Table for Monitor type 3 72HZ NI     */

static VESA_MODE vesa_205 = {           /* 1024 x 768 x 8                       */
        0x4d,                           /* Internal Mode Number                 */
        &Mode_1024x768_Table,           /* Base Mode Table                      */
        0x0e,                           /* Sequence 04h                         */
        0x07,                           /* Advanced Function Register           */
        0x00,                           /* DAC Mode                             */
        8,                              /* 8 bits/pel                           */
        Mode_6D_Table_0,                /* Table for Monitor type 0 43HZ I      */
        Mode_6D_Table_0,                /* Table for Monitor type 1 56HZ NI     */
        Mode_6D_Table_2,                /* Table for Monitor type 2 60HZ NI     */
        Mode_6D_Table_3 };              /* Table for Monitor type 3 72HZ NI     */

static VESA_MODE vesa_107 = {           /* 1280 x1024 x 8                       */
        0x72,                           /* Internal Mode Number                 */
        &Mode_1280x1024_Table,          /* Base Mode Table                      */
        0x0e,                           /* Sequence 04h                         */
        0x03,                           /* Advanced Function Register           */
        0x00,                           /* DAC Mode                             */
        8,                              /* 16 bits/pel                          */
        Mode_72_Table_0,                /* Table for Monitor type 0 43HZ I      */
        Mode_72_Table_0,                /* Table for Monitor type 1 56HZ NI     */
        Mode_72_Table_2,                /* Table for Monitor type 2 60HZ NI     */
        Mode_72_Table_3 };              /* Table for Monitor type 3 72HZ NI     */

static VESA_MODE vesa_3ff = {           /* 1280 x1024 x 8                       */
        0x72,                           /* Internal Mode Number                 */
        &Mode_1280x1024_Table,          /* Base Mode Table                      */
        0x0e,                           /* Sequence 04h                         */
        0x03,                           /* Advanced Function Register           */
        0x00,                           /* DAC Mode                             */
        8,                              /* 16 bits/pel                          */
        Mode_72_Table_0,                /* Table for Monitor type 0 43HZ I      */
        Mode_72_Table_0,                /* Table for Monitor type 1 56HZ NI     */
        Mode_72_Table_2,                /* Table for Monitor type 2 60HZ NI     */
        Mode_72_Table_3 };              /* Table for Monitor type 3 72HZ NI     */

static VESA_MODE vesa_177 = {           /* 1024 x 768 x 16                      */
        0x77,                           /* Internal Mode Number                 */
        &Mode_1024x768_Table,           /* Base Mode Table                      */
        0x0e,                           /* Sequence 04h                         */
        0x03,                           /* Advanced Function Register           */
        0x00,                           /* DAC Mode                             */
        16,                             /* 16 bits/pel                          */
        Mode_77_Table_0,                /* Table for Monitor type 0 43HZ I      */
        Mode_77_Table_0,                /* Table for Monitor type 1 56HZ NI     */
        Mode_77_Table_2,                /* Table for Monitor type 2 60HZ NI     */
        Mode_77_Table_2 };              /* Table for Monitor type 3 72HZ NI     */

static VESA_MODE vesa_201 = {           /* 640 x 480 x 8                        */
        0x49,                           /* Internal Mode Number                 */
        &Mode_640x480_Table,            /* Base Mode Table                      */
        0x0e,                           /* Sequence 04h                         */
        0x03,                           /* Advanced Function Register           */
        0x00,                           /* DAC Mode                             */
        8,                              /* 8 bits/pel                           */
        Mode_49_Table_2,                /* Table for Monitor type 0 43HZ I      */
        Mode_49_Table_2,                /* Table for Monitor type 1 56HZ NI     */
        Mode_49_Table_2,                /* Table for Monitor type 2 60HZ NI     */
        Mode_49_Table_3 };              /* Table for Monitor type 3 72HZ NI     */

static VESA_MODE vesa_111 = {           /* 640 x 480 x 16                       */
        0x6e,                           /* Internal Mode Number                 */
        &Mode_640x480_Table,            /* Base Mode Table                      */
        0x0e,                           /* Sequence 04h                         */
        0x03,                           /* Advanced Function Register           */
        0x00,                           /* DAC Mode                             */
        16,                             /* 8 bits/pel                           */
        Mode_6e_Table_2,                /* Table for Monitor type 0 43HZ I      */
        Mode_6e_Table_2,                /* Table for Monitor type 1 56HZ NI     */
        Mode_6e_Table_2,                /* Table for Monitor type 2 60HZ NI     */
        Mode_6e_Table_2 };              /* Table for Monitor type 3 72HZ NI     */

static VESA_MODE vesa_211 = {           /* 640 x 480 x 16                       */
        0x4f,                           /* Internal Mode Number                 */
        &Mode_640x480_Table,            /* Base Mode Table                      */
        0x06,                           /* Sequence 04h                         */
        0x03,                           /* Advanced Function Register           */
        0x00,                           /* DAC Mode                             */
        16,                             /* 16 bits/pel                          */
        Mode_4f_Table_0,                /* Table for Monitor type 0 43HZ I      */
        Mode_4f_Table_0,                /* Table for Monitor type 1 56HZ NI     */
        Mode_4f_Table_2,                /* Table for Monitor type 2 60HZ NI     */
        Mode_4f_Table_3 };              /* Table for Monitor type 3 72HZ NI     */

static  ulong vesa_modes[8][2] = {
        0x201,(ulong)&vesa_201,         /*  640 x  480 x  8 */
        0x111,(ulong)&vesa_111,         /*  640 x  480 x 16 */
        0x211,(ulong)&vesa_211,         /*  640 x  480 x 16 */

        0x204,(ulong)&vesa_204,         /* 1024 x  768 x  4 */
        0x205,(ulong)&vesa_205,         /* 1024 x  768 x  8 */
        0x177,(ulong)&vesa_177,         /* 1024 x  768 x 16 */

        0x107,(ulong)&vesa_107,         /* 1280 x 1024 x  8 */
        0x3ff,(ulong)&vesa_3ff          /* 1280 x 1024 x  8 */
};

static  char monitor_type[16][16] = {


/*------------------------------------------------------------------------------*/
/*                                                                              */
/* The default monitor type is 0, or 43HZ Interlace displays.  This table       */
/* contains those ID' recognized for different requirements.  An entry in the   */
/* in the table can be one of:                                                  */
/*                                                                              */
/*        0 == Slow, 43Hz, Interlaced                                           */
/*        1 ==       56Hz, Non-Interlaced                                       */
/*        2 ==       60Hz, Non-Interlaced                                       */
/*        3 ==       70Hz, Non-Interlaced                                       */
/*                                                                              */
/*------------------------------------------------------------------------------*/
/*      0 1 2 3 4 5 6 7 8 9 a b c d e f                                         */
/*      -------------------------------                                         */
        3,3,3,3,3,3,2,3,3,3,3,0,3,3,3,3,/* HSYNC 0                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC 1                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC 2                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC 3                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC 4                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC 5                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC 6                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC 7                              */
        3,3,0,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC 8                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC 9                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC a                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC b                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC c                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC d                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,/* HSYNC e                              */
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 /* HSYNC f                              */
                                     };

int init_iga(vp, ld, ddf, dds)          /* Mainline routine */
struct vtmstruc *vp;
struct iga_data *ld;
struct iga_ddf *ddf;
struct fbdds   *dds;                    /* contains config method
                                         * data-prefilled */
{
    int  i, rc;
    uchar adapter_stat_reg;
    struct phys_displays *pd;

    BUGLPR (dbg_initf, BUGNTA, ("Starting initialization\n"));

    BUGLPR (dbg_initf, BUGNTA, ("Attaching PCI I/O space\n"));

    IGA_ENTER_TRC(ddf,iga_init,2,IGA_INIT,vp,ld,ddf,0,0); 

    ddf->IO_in_progress += 1;     /* set flag to indicate IO happening */

    ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

    PCI_GETC(INPUT_STATUS1);     /* Reset attribute address flip flop    */
    PCI_PUTC(ATTR_INDX,  0);    /* Disable video palette (turns off vid)*/

        BUGLPR (dbg_initf, BUGGID, ("-PRE Get_Monitor\n"));

    /*-----------------------------------------------------*/
    /*        Set up for attached monitor                  */
    /*-----------------------------------------------------*/
    Get_Monitor (ld, ddf);      /* determine monitor, etc. */

        BUGLPR (dbg_initf, BUGGID, ("POST Get_Monitor\n"));

    /* set display mode */
    if(ddf->monitor_type == 6)		/* is it the flat panel ? */
    {
	IGA_PARM_TRC0(ddf,iga_init,2,IGA_INIT,IGA_FLAT_PANNEL);
	dds->display_mode = 0x0201;
    }
    
    ld->v_mode = (VESA_MODE *)lookup_v(dds->display_mode,ddf);
    if(ld->v_mode == NULL)
    {
        BUGLPR (dbg_initf, BUGNFO, ("Invalid display mode: 0x%8.8x\n", dds->display_mode));
        ld->v_mode = &vesa_201;     /*  640x480x8 */
        dds->display_mode = 0x0201;

	IGA_PARM_TRC0(ddf,iga_init,2,IGA_INIT,IGA_INVAL_DSP_MODE);

        BUGLPR (dbg_initf, BUGNFO, ("\tmode 0x%8.8x assumed.\n", dds->display_mode));
    }

    /* pick up mode's resolution */
    ld->screen_width_pix  = ld->v_mode->Base_Mode_Table->screen_w;
    ld->screen_height_pix = ld->v_mode->Base_Mode_Table->screen_h;

    ld->bpp = ld->v_mode->bits_per_pel;

        BUGLPR (dbg_initf, BUGNTA, ("Detaching PCI I/O space\n"));

    PCI_BUS_DET(ld->IO_base_address);  /* detach addressability to the bus */

    ddf->IO_in_progress -= 1;          /* reset set */

    IGA_EXIT_TRC0(ddf,iga_init,2,IGA_INIT); 

    return 0; /* end of init_iga() */
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*               SET VIDEO PARMS ACCORDING TO MONITOR USED                    */
/*                                                                            */
/*----------------------------------------------------------------------------*/
void
Get_Monitor (ld, ddf)
struct iga_data *ld;
struct iga_ddf *ddf;
{
        int     j;
        uchar   t, u, v, w;
        int     monitor_requested = -1;

	IGA_ENTER_TRC(ddf,iga_Get_Monitor,2,IGA_GET_MONITOR,ld,ddf,0,0,0);

/*---------------------------------------------------------------------------*/
/*   NOTE: - At this point, one should test the DAC, and perform a test for  */
/*           color or monochrome monitor. For the time being, color is the   */
/*           winner.                                                         */
/*---------------------------------------------------------------------------*/
#define MONO  0
#define COLOR 1
        ld->disp_type         = COLOR;

        GET_CRTC_REG(EX_DAC_CT, u);      /* get Extended DAC Control reg (55) */
        u |= ENB_GIR;                    /* Disable DAC Reads - use gpio port */
        PCI_PUTC(CRTC_DATA, u);

        u = PCI_GETC(DAC_GENERAL_RD);

        t = PCI_GETC(MISC_OUTPUT_R);    /* Read misc output register         */
        v = t & (unsigned char) 0x3f;   /* Force syncs to active low         */

        PCI_PUTC(MISC_OUTPUT_W, v);     /* Accumulate all possible 0's in ID */
                                        /* bits.  HSYNC's will be low most of*/
        u = 0x0f;                       /* the time.                         */
        for (j=0;j<256;j++)
           u &= (PCI_GETC(DAC_GENERAL_RD) & (unsigned char)  0x0f);

        v |= 0xc0;                      /* Force syncs to active high         */
        PCI_PUTC(MISC_OUTPUT_W, v);     /* Accumulate all possible 1's in ID. */
        w = 0;                          /* HSYNC's will be high most of the time*/
        for (j=0;j<256;j++)
           w |= (PCI_GETC(DAC_GENERAL_RD) & (unsigned char) 0x0f);

        w = w ^ u;                      /* w contains the bits that are HSYNC */

        if (monitor_requested == -1)
          ld->monitor = monitor_type[w][u];/* Get the monitor type            */
        else                               /* 0 == Slow, 43Hz, Interlaced     */
          ld->monitor = monitor_requested; /* 1 ==       56Hz, Non Interlaced */
                                           /* 2 ==       60Hz, Non Interlaced */
                                           /* 3 ==       72Hz, Non Interlaced */
        ddf->monitor_type = (ulong)(w * 16 + u); /* save monitor id info */

	IGA_PARM_TRC(ddf,iga_Get_Monitor,2,IGA_GET_MONITOR,IGA_MONITOR_ID,w,u,0,0,0);
	IGA_PARM_TRC(ddf,iga_Get_Monitor,2,IGA_GET_MONITOR,IGA_MONITOR_ID,ld->monitor,ddf->monitor_type,0,0,0);

	BUGLPR (dbg_initf, BUGGID,("Monitor type is %x\n", ddf->monitor_type));

        PCI_PUTC(MISC_OUTPUT_W, t);        /* Restore misc output port */

        GET_CRTC_REG(EX_DAC_CT, u);      /* get Extended DAC Control reg (55) */
        u &= ~ENB_GIR;                   /* Re-enable DAC Reads               */
        PCI_PUTC(CRTC_DATA, u);

	IGA_EXIT_TRC0(ddf,iga_Get_Monitor,2,IGA_GET_MONITOR);

        return;  /* end of Get_Monitor () */
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*               LOOK UP POINTER TO VESA PARMS                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
ulong lookup_v (mode,ddf)
short mode;
struct iga_ddf *ddf;
{
        int     i;

	IGA_ENTER_TRC1(ddf,iga_lookup_v,2,IGA_LOOKUP_V,mode);
        for(i=0; i<(sizeof(vesa_modes)/8); i++)
        {
                if (mode == vesa_modes[i][0])
                {
                            BUGLPR (dbg_initf, BUGGID, \
                            ("Vesa Mode %x selected.\n", vesa_modes[i][0]));

			IGA_PARM_TRC1(ddf,iga_lookup_v,2,IGA_LOOKUP_V,IGA_LOOKUP_V_FOUND_MODE,i);
			IGA_EXIT_TRC0(ddf,iga_lookup_v,2,IGA_LOOKUP_V);

                        return vesa_modes[i][1];
                } /* endif */
        }

	IGA_PARM_TRC0(ddf,iga_lookup_v,2,IGA_LOOKUP_V,IGA_LOOKUP_V_NOT_FOUND_MODE);

	IGA_EXIT_TRC0(ddf,iga_lookup_v,2,IGA_LOOKUP_V);

        return NULL;
}
