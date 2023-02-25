/* @(#)18	1.1  src/rspc/usr/lib/boot/softros/lib/libstand/mono.h, rspc_softros, rspc411, GOLD410 4/18/94 15:49:44  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: HW_WRRB
 *		MONO_OFF
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Mono display adapter I/O routines support data structures                 */
/*                                                                           */
/* Description of monochrome display adapter internal registers:             */
/*   Reg  Default  Description                                               */
/*   r00   0x61    Horizontal total (Characters)                             */
/*   r01   0x50    Horizontal displayed (Characters)                         */
/*   r02   0x52    Horizontal sync position (Characters)                     */
/*   r03   0x0f    Horizontal sync width (Characters)                        */
/*   r04   0x19    Vertical total (Character rows)                           */
/*   r05   0x06    Vertical total adjust (Scan lines)                        */
/*   r06   0x19    Vertical displayed (Character rows)                       */
/*   r07   0x19    Vertical sync position (Character rows)                   */
/*   r08   0x02    Interlace mode                                            */
/*   r09   0x0D    Maximum scan line address (Scan lines)                    */
/*   r10   0x06    Cursor start scan line                                    */
/*   r11   0x07    Cursor stop scan line                                     */
/*   r12   0x00    Start address high                                        */
/*   r13   0x00    Start address low                                         */
/*   r14   0x00    Cursor address high                                       */
/*   r15   0x00    Cursor address low                                        */
/*   r16   0x00    Reserved                                                  */
/*   r17   0x00    Reserved                                                  */
/*                                                                           */
/* Description of monochrome display adapter visable external registers:     */
/*  Addrs Description                                                        */
/*   3B0  Not used                                                           */
/*   3B1  Not used                                                           */
/*   3B2  Not used                                                           */
/*   3B3  Not used                                                           */
/*   3B4  6845 Index register                                                */
/*   3B5  6845 Data register                                                 */
/*   3B6  Not used                                                           */
/*   3B7  Not used                                                           */
/*   3B8  CRT Control Port 1                                                 */
/*   3B9  Reserved                                                           */
/*   3BA  CRT Status Port                                                    */
/*   3BB  Reserved                                                           */
/*   3BC  Parallel Data Port                                                 */
/*   3BD  Parallel Status Port                                               */
/*   3BE  Parallel Control Port                                              */
/*   3BF  Not used                                                           */
/*                                                                           */
/* Initialization sequence for 80x25 mode:                                   */
/*   out(0x3B8,0x01)               Initialize high resolution mode           */
/*   for (i=0;i < sizeof(init_data);i++)                                     */
/*     {                                                                     */
/*     out(0x3B4,i)                Set index register to value to init       */
/*     out(0x3B5,init_data[i])     Send the initial value to register        */
/*     }                                                                     */
/*   out(0x3B8,0x29)               Enable video & correct port settings      */
/*                                                                           */
/*                                                                           */
/* Base I/O Address: 0x800003B4                                              */
/* Base memory address: 0x000B0000                                           */
/*                                                                           */


/* mono - This structure holds all the information, static and dynamic,*/
/*        for the display adapter.                                     */
struct mono
  {
  unsigned char  rows;           /* Number of rows                     */
  unsigned char  cols;           /* Number of columns                  */
  unsigned char  currow;         /* Current row (0 based)              */
  unsigned char  curcol;         /* Current column (0 based)           */
  unsigned char *membase;        /* Pointer to display memory          */
  unsigned short cursor;         /* Current cursor value               */
  };

/* Define the constants that describe the monochrome display adapter         */
#define M_IOBASE  0x80000000       /* SANDALFOOT Physical addr I/O base      */
#define M_MEMBASE 0xC0000000       /* SANDALFOOT Physical addr adapter mem   */

#define MONO_INDEX M_IOBASE+0x3B4  /* I/O address of mono adapter index reg  */
#define MONO_DATA  M_IOBASE+0x3B5  /* I/O address of mono adataer data reg   */
#define MONO_CNTRL M_IOBASE+0x3B8  /* I/O address of mono control port 1     */

#define MONO_BASE  0x000B0000      /* Base of MONO adapter display memory    */
#define MONO_MEM   M_MEMBASE+MONO_BASE /* Phys addr of adapter display mem   */

#define MONO_CURSIZE 10            /* Internal cursor size register          */
#define MONO_ORIGIN  12            /* Internal origin register (Scroll base) */
#define MONO_CURPOS  14            /* Internal cursor position register      */

#define CURSOR_SHAPE 0x0706        /* Initial cursor shape value             */
#define MODE_HIRES   0x01          /* Initial value for MONO_CNTRL port      */
#define MODE_SET     0x29          /* Final MONO_CNTRL port value            */
#define MROWS 25                   /* Number of rows on the display          */
#define MCOLS 80                   /* Number of columns                      */


/* Macros                                                              */
/*   MONO_OFF - This macro will return the address in display memory   */
/*              for an (x,y) position.  Note that x is in [0..MROWS-1] */
/*              and y is in [0..MCOLS-1].                              */
#define MONO_OFF(x,y) (((monodata.cols * 2 * X) + (y * 2)) + monodata.membase)

/*   HW_WRRB - This macro will do what ever is needed to write a one   */
/*            byte value to an I/O register.                           */
/*#define HW_WRRB(xxr, xxv) printf("  Wrote %2.2X to %8.8X\r\n",xxv,xxr)*/
#define HW_WRRB(xxr, xxv) outbyte(xxr, xxv)

extern void  outbmem( unsigned *addr, unsigned char value );
extern unsigned char inbmem( unsigned char *addr );
void delay( long milliseconds );
