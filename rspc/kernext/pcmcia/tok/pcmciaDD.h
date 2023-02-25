/* @(#)69       1.5  src/rspc/kernext/pcmcia/tok/pcmciaDD.h, pcmciatok, rspc41J, 9517A_all 4/24/95 20:54:57 */
/*
 * COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS: none
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

#define MAX_CARD    3
#define MAX_ID      3
#define MAX_ID_LEN  10

static struct _card_info {
    int     	  offset[MAX_ID];
    int     	  length[MAX_ID];
    unsigned char contents[MAX_ID][MAX_ID_LEN];
    int	    	  result;
} card_info[MAX_CARD] = {
        /*********** Tuple information for PCMCIA 092F7193 **************/
        {{ 0x11, 0x00, 0x00 },                  /* Offset               */
         { 0x08, 0x04, 0x02 },                  /* Length               */
         {{"092F7193"},                         /* Parts Number         */
         { 0xa4, 0x00, 0x1e ,0x00 },            /* Manufacturer ID      */
         { 0x06, 0x03 }},                       /* Function ID          */
         { 0x00 }},                             /* Result               */
        /*********** Tuple information for PCMCIA 0933929 ***************/
        {{ 0x11, 0x00, 0x00 },                  /* Offset               */
         { 0x07, 0x04, 0x02 },                  /* Length               */
         {{"0933929"},                          /* Parts Number         */
         { 0xa4, 0x00, 0x1e, 0x00 },            /* Manufacturer ID      */
         { 0x06, 0x03 }},                       /* Function ID          */
         { 0x00 }},                             /* Result               */
        /*********** Tuple information for PCMCIA 0933030 ***************/
        {{ 0x11, 0x00, 0x00 },                  /* Offset               */
         { 0x07, 0x04, 0x02 },                  /* Length               */
         {{"0933030"},                          /* Parts Number         */
         { 0x00, 0x00, 0x00, 0x00 },            /* Manufacturer ID      */
         { 0x00, 0x00 }},                       /* Function ID          */
         { 0x14 }}                              /* Result               */
};

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
#define MSK_MATCH_ALL1        0x14 /* VERS_1's  matched, (for 0933030)	*/
                                   /* MANFID's  wasn't there		*/
                                   /* FUNCID's  wasn't there		*/
#define MSK_MATCH_ALL2        0x00 /* VERS_1's  matched, (for 0933929)	*/
                                   /* MANFID's  matched,		*/
                                   /* FUNCID's  matched,		*/

#define TOK_PRIMARY_ADAPTER   0x00
#define TOK_SECONDARY_ADAPTER 0x01
#define DELAYMS(ms)           delay((int)(((ms)*HZ+999)/1000))

static struct _mask_info {
    int     	  cistpl[MAX_ID];
    int     	  vers[MAX_ID];
    int     	  mid[MAX_ID];
    int     	  fid[MAX_ID];
} mask_info[] = {
	 CISTPL_VERS_1,        CISTPL_MANFID,        CISTPL_FUNCID,
	 MSK_VERS_1_NOT_MATCH, MSK_MANFID_NOT_MATCH, MSK_FUNCID_NOT_MATCH,
	 MSK_VERS_1_NO_TUPLE,  MSK_MANFID_NO_TUPLE,  MSK_FUNCID_NO_TUPLE,
	 MSK_VERS_1_FAILD,     MSK_MANFID_FAILD,     MSK_MANFID_FAILD        
};


#ifdef DEBUG
#define TRACE_PCMCIA(hook, tag, arg1, arg2, arg3) \
        tok_save_trace(hook, tag, arg1, arg2, arg3)
#ifdef DEBUG_PCMCIA
#define TRACE_PCMCIA(hook, tag, arg1, arg2, arg3)  \
        printf("  <<%s>> %8x : %8x : %8x\n", tag, arg1, arg2, arg3)
#endif
#else
#define TRACE_PCMCIA(hook, tag, arg1, arg2, arg3)  
#endif
