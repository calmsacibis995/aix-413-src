static char sccsid[] = "@(#)24	1.1  src/rspc/usr/lib/boot/softros/roslib/io_sub.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:04";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: in16
 *		in16le
 *		in32
 *		in32le
 *		in8
 *		out16
 *		out16le
 *		out32
 *		out32le
 *		out8
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

#pragma title("io_sub.c : I/O port subroutines")
#pragma subtitle("IBM Confidential")
#pragma options source
#pragma options list
/*****************************************************************************/
/* FILE NAME:    io_sub.c                                                    */
/* DESCRIPTION:  This file contains routines for reading and writing I/O     */
/*               ports.                                                      */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/*               Include files....                                           */

#pragma options nosource

#include "io_sub.h"             /* Include external function prototypes... */

#pragma options source


/*---------------------------------------------------------------------------*/
/*               Common equate definitions.....                              */

#define IO_PORT_MASK    0x80000000

// This is a built-in XLC "macro".
// I <rb> called Toronto to find this out.

void __iospace_eieio(void);

/*---------------------------------------------------------------------------*/
/*               Public routine definitions.....                             */

/*---------------------------------------------------------------------------*/
/*  Function: out8()                                                         */
/*            This routine will write a byte value to the specified I/O      */
/*            port.                                                          */
/*                                                                           */
/*  Input:    port -     I/O address to write to                             */
/*            port_val - value to write to I/O port                          */
/*                                                                           */
/*  Output:   No return value                                                */
/*---------------------------------------------------------------------------*/
void out8(unsigned int port, unsigned char port_val) {

  __iospace_eieio();
  *(unsigned char *)(IO_PORT_MASK | port) = port_val;
}

/*---------------------------------------------------------------------------*/
/*  Function: in8()                                                          */
/*            This routine will read a byte from the I/O port specified.     */
/*                                                                           */
/*  Input:    port - Address of I/O port to read from                        */
/*                                                                           */
/*  Output:   Value read from I/O port                                       */
/*---------------------------------------------------------------------------*/
unsigned char in8(unsigned int port) {

  __iospace_eieio();
  return *(unsigned char *)(IO_PORT_MASK | port);
}

/*---------------------------------------------------------------------------*/
/*  Function: out16()                                                        */
/*            This routine will write a 16 bit value to the specified I/O    */
/*            port.                                                          */
/*                                                                           */
/*  Input:    port -     I/O address to write to (Must be 16 bit aligned)    */
/*            port_val - value to write to I/O port                          */
/*                                                                           */
/*  Output:   No return value                                                */
/*---------------------------------------------------------------------------*/
void out16(unsigned int port, unsigned short port_val) {

  __iospace_eieio();
  *(unsigned short *)(IO_PORT_MASK | port) = port_val;
}

/*---------------------------------------------------------------------------*/
/*  Function: out16le()                                                      */
/*            This routine will write a 16 bit value to the specified I/O    */
/*            port.                                                          */
/*                                                                           */
/*  Input:    port -     I/O address to write to (Must be 16 bit aligned)    */
/*            port_val - value to write to I/O port                          */
/*                                                                           */
/*  Output:   No return value                                                */
/*---------------------------------------------------------------------------*/
void out16le(unsigned int port, unsigned short port_val) {

  __iospace_eieio();
  *(unsigned short *)(IO_PORT_MASK | port) =
     (port_val << 8) + (port_val >> 8);
}

/*---------------------------------------------------------------------------*/
/*  Function: in16()                                                         */
/*            This routine will read a 16 bit value from the specified I/O   */
/*            port.                                                          */
/*                                                                           */
/*  Input:    port - Address of I/O port to read from                        */
/*                                                                           */
/*  Output:   Value read from I/O port                                       */
/*---------------------------------------------------------------------------*/
unsigned short in16(unsigned int port) {

  __iospace_eieio();
  return *(unsigned short *)(IO_PORT_MASK | port);
}

/*---------------------------------------------------------------------------*/
/*  Function: in16le()                                                       */
/*            This routine will read a 16 bit value from the specified I/O   */
/*            port.                                                          */
/*                                                                           */
/*  Input:    port - Address of I/O port to read from                        */
/*                                                                           */
/*  Output:   Value read from I/O port                                       */
/*---------------------------------------------------------------------------*/
unsigned short in16le(unsigned int port) {
unsigned short temp_val;

  __iospace_eieio();
  temp_val = *(unsigned short *)(IO_PORT_MASK | port);
  return ((temp_val << 8) + (temp_val >> 8));
}

/*---------------------------------------------------------------------------*/
/*  Function: out32()                                                        */
/*            This routine will write a 32 bit value to the specified I/O    */
/*            port.                                                          */
/*                                                                           */
/*  Input:    port -     I/O address to write to (Must be 32 bit aligned)    */
/*            port_val - value to write to I/O port                          */
/*                                                                           */
/*  Output:   No return value                                                */
/*---------------------------------------------------------------------------*/
void out32(unsigned int port, unsigned long port_val) {

  __iospace_eieio();
  *(unsigned long *)(IO_PORT_MASK | port) = port_val;
}

/*---------------------------------------------------------------------------*/
/*  Function: out32le()                                                      */
/*            This routine will write a 32 bit value to the specified I/O    */
/*            port.                                                          */
/*                                                                           */
/*  Input:    port -     I/O address to write to (Must be 32 bit aligned)    */
/*            port_val - value to write to I/O port                          */
/*                                                                           */
/*  Output:   No return value                                                */
/*---------------------------------------------------------------------------*/
void out32le(unsigned int port, unsigned long port_val) {

  __iospace_eieio();
  *(unsigned long *)(IO_PORT_MASK | port) =
     ((port_val & 0x000000ff) << 24) +
     ((port_val & 0x0000ff00) << 8 ) +
     ((port_val & 0x00ff0000) >> 8 ) +
     ((port_val & 0xff000000) >> 24);
}

/*---------------------------------------------------------------------------*/
/*  Function: in32()                                                         */
/*            This routine will read a 32 bit value from the specified I/O   */
/*            port.                                                          */
/*                                                                           */
/*  Input:    port - Address of I/O port to read from (Must be 32 bit aligned*/
/*                                                                           */
/*  Output:   Value read from I/O port                                       */
/*---------------------------------------------------------------------------*/
unsigned long in32(unsigned int port) {

  __iospace_eieio();
  return *(unsigned long *)(IO_PORT_MASK | port);
}

/*---------------------------------------------------------------------------*/
/*  Function: in32le()                                                         */
/*            This routine will read a 32 bit value from the specified I/O   */
/*            port.                                                          */
/*                                                                           */
/*  Input:    port - Address of I/O port to read from (Must be 32 bit aligned*/
/*                                                                           */
/*  Output:   Value read from I/O port                                       */
/*---------------------------------------------------------------------------*/
unsigned long in32le(unsigned int port) {
unsigned long  port_val;

  __iospace_eieio();
  port_val = *(unsigned long *)(IO_PORT_MASK | port);
  return ((port_val & 0x000000ff) << 24) +
         ((port_val & 0x0000ff00) << 8 ) +
         ((port_val & 0x00ff0000) >> 8 ) +
         ((port_val & 0xff000000) >> 24);
}
