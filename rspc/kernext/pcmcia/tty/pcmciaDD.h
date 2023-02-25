/* @(#)25       1.1  src/rspc/kernext/pcmcia/tty/pcmciaDD.h, pcmciafax, rspc411, 9437A411a 9/7/94 16:06:35 */
/*
 * COMPONENT_NAME: pcmciafax
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

typedef struct {
    int    offset;
    int    length;
    char  *contents;
} chk_tpl_pkt_t;

#define RC_MATCHED            0x00
#define RC_NOT_MATCH          0x01
#define RC_NO_TUPLE           0x02
#define RC_FAILD_READ         0x03
#define RC_FAILD_GET          0x04
