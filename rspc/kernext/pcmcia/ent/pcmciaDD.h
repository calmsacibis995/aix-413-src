/* @(#)93       1.2  src/rspc/kernext/pcmcia/ent/pcmciaDD.h, pcmciaent, rspc41J, 9511A_all 3/9/95 03:05:39 */
/*
 * COMPONENT_NAME: PCMCIAENT - IBM PCMCIA Ethernet Device Driver
 *
 * FUNCTIONS: none.
 *           
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
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

#define MSK_VERS_1_NOT_MATCH  0x02
#define MSK_VERS_1_NO_TUPLE   0x01
#define MSK_VERS_1_FAILD      0x03
#define MSK_MANFID_NOT_MATCH  0x08
#define MSK_MANFID_NO_TUPLE   0x04
#define MSK_MANFID_FAILD      0x0c
#define MSK_FUNCID_NOT_MATCH  0x20
#define MSK_FUNCID_NO_TUPLE   0x10
#define MSK_FUNCID_FAILD      0x30
#define MSK_MATCH_ALL         0x00	/* VERS_1's  matched,		*/
                                   	/* MANFID's  matched,    	*/
                                   	/* FUNCID's  matched,    	*/

#ifdef DEBUG
#define TRACE_PCMCIA(hook, tag, arg1, arg2, arg3) \
        ient_trace(hook, tag, arg1, arg2, arg3)
#ifdef DEBUG_PCMCIA
#define TRACE_PCMCIA(hook, tag, arg1, arg2, arg3)  \
        printf("  <<%s>> %8x : %8x : %8x\n", tag, arg1, arg2, arg3)
#endif
#else
#define TRACE_PCMCIA(hook, tag, arg1, arg2, arg3)  
#endif
