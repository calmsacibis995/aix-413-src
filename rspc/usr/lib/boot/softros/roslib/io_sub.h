/* @(#)39	1.1  src/rspc/usr/lib/boot/softros/roslib/io_sub.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:29  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: IO_Read
 *		IO_Write
 *		inb
 *		inw
 *		outb
 *		outw
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************************************************************************/
/* FILE NAME:    io_sub.h                                                       */
/*               Public routine prototypes for io_sub.s                         */
/********************************************************************************/

#ifndef _H_IO_SUB
#define _H_IO_SUB
/*------------------------------------------------------------------------------*/
/*               Public function prototypes....                                 */
/*                                                                              */
/*      TEMORARILY ALLOW IO_Read/IO_Write/inb/outb/outw/inw.                    */
/*                                                                              */
/*                                                                              */
/*      The xxxle versions of the routines perform a byteswap after transferring*/
/*      the data.                                                               */
/*------------------------------------------------------------------------------*/

#define IO_Read(a)  in8(a)   
#define IO_Write(a,b) out8(a,b)     

#define inb(a)    in8(a)
#define outb(a,b) out8(a,b)

#define inw(a)    in16le(a)
#define outw(a,b) out16le(a,b)

void out8( unsigned int, unsigned char );       /* Write a byte to I/O memory   */
unsigned char in8( unsigned int );              /* Read a byte from I/O memory  */

void out16( unsigned int, unsigned short );     /* Write a short to I/O memory  */
unsigned short in16( unsigned int );            /* Read a short from I/O memory */

void out16le( unsigned int, unsigned short );   /* Write a short to I/O memory  */
unsigned short in16le( unsigned int );          /* Read a short from I/O memory */

void out32( unsigned int, unsigned long );      /* Write a long to I/O memory   */
unsigned long in32( unsigned int );             /* Read a long from I/O memory  */

void out32le( unsigned int, unsigned long );    /* Write a long to I/O memory   */
unsigned long in32le( unsigned int );           /* Read a long from I/O memory  */

#define IO_PORT_MASK            0x80000000

#endif  /* _H_IO_SUB */
