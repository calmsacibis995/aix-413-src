/* @(#)41	1.2  src/rspc/usr/lib/boot/softros/roslib/printf_fpd.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:52:10  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
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

#include <sys/types.h>

/*=========================================================================*/
/* Description:    Format and print string  (As in libc printf)            */
/* Input:          Format string (const char *)                            */
/* Ouput:          Address of target (void *)                              */
/*                                                                         */
int  printf( const char *format_str, ... );

/*=========================================================================*/
/* Function:     pchar (Character mode)                                    */
/* Description:  display one character on screen                           */
/* Input:        IBM-850 Character code (u_char)                           */
/*               Character attribute (u_char)                              */
/* Ouput:        none                                                      */
/*                                                                         */
void  pchar( u_char chr, u_char attr );
