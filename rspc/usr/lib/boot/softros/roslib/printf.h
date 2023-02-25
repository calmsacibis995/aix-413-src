/* @(#)40	1.2  src/rspc/usr/lib/boot/softros/roslib/printf.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:50:48  */
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
/*
 * Constant variables and macros user of printf()
 *
 */

#ifndef _H_PRINTF
#define _H_PRINTF

#include "printf_fpd.h"
#include "screen.h"

#define  NoERROR  (0)
#define  ERROR    (1)

#ifndef  FALSE
#define  FALSE    (0)
#endif

#ifndef  TRUE
#define  TRUE     (1)
#endif

#define  LINE_MAX  (80)

/*
#define  putchar(c)          pchar( c, screen.attr )
*/

#define  FONT_X    (16)
#define  FONT_Y    (20)
#define  FONTBASE  ISO88591

#endif  /* _H_PRINTF */
