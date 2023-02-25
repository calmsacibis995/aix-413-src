/* @(#)85	1.9  src/tenplus/include/chardefs.h, tenplus, tenplus411, GOLD410 4/13/94 16:48:01 */
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <ctype.h>

extern	int	MultibyteCodeSet;	/* global code set flag */

#define DIR_SEPARATOR '/'
#define SPACE (' ')
#define WCSPACE (L' ')

#define control(c) ((c) & 037)

#define NORMAL    0       /* output font states */
#define UNDERLINE 0x0100
#define STANDOUT  0x0200
#define GRAPHIC   0x0400
#define DELIMIT   0x0800
#define NUM_ATTRS 3       /* number of non-NORMAL attrs */

/* Every first ATTR of a multi-ATTR character (and of every character
   that consists of just one ATTR) has its FIRST bit set. The only
   exception is the null character, which is encoded as all zero's. */

#define FIRST     0x1000
#define firstbyte(x) (((x) & FIRST) || (x == 0))

#define is_underlined(c) (((c) & UNDERLINE) != 0)
#define de_underline(c)  ((ATTR)((c) & ~UNDERLINE))
#define underline(c)     ((c) | UNDERLINE)
#define de_attr(c)       ((c) & 0377)

#define undelimit(a)    ((is_attrset ((a),DELIMIT)) ? ((a) - DELIMIT) : (a))
#define attr_unset(a,b) if (is_attrset (a,b)) (a) -= (b)
#define attr_set(a,b)   if (!is_attrset (a,b)) (a) += (b)
#define attr_flip(a,b)  if (is_attrset (a,b)) (a) -= (b); \
				       else (a) += (b)

#define get_attr(a)     ((a) & 0xFF00)
#define is_graphic(a)   ((de_attr (a)) < SPACE)
#define is_wcgraphic(a) ((a) < WCSPACE)
#define is_attrset(a,b) ((a) & (b))

#ifndef TMP_WC_LEN_VAR
#define TMP_WC_LEN_VAR
static int tmp_wc_len; /* Needed on a per ".c" file basis */
#endif

#define wc_text_width(a) (is_wcgraphic(a) ? 1 : \
			    ((tmp_wc_len = wcwidth(a)) < 0) ? 1 : tmp_wc_len)

/* definitions for the graphics characters possible in boxes    */
#define G_HLINE control('Q')    /* horiz line                   */
#define G_TOL   control('W')    /* tee on left side             */
#define G_TOR   control('E')    /* tee on right side            */
#define G_ITEE  control('R')    /* inverted tee                 */
#define G_TEE   control('T')    /* tee                          */
#define G_VLINE control('A')    /* vertical line                */
#define G_ULC   control('S')    /* upper lefthand corner        */
#define G_URC   control('D')    /* upper righthand corner       */
#define G_LLC   control('F')    /* lower lefthand corner        */
#define G_LRC   control('G')    /* lower righthand corner       */
#define G_X     control('Z')    /* cross                        */
#define G_BLOT  control('B')    /* blot (region start mark)     */
#define DOTCHAR control('Y')    /* fill char for dotted fields  */


