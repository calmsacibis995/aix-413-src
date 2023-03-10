/* @(#)96	1.2  src/bos/usr/sbin/adfutil/genodm.h, cmdadf, bos411, 9428A410j 6/15/90 16:58:11 */
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS:  genodm.h - support structs for dropping info in database
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/* Transaction types */
#define t_ADD     8
#define t_CHANGE  9
#define t_DELETE 10

struct genmap {
    int ClassType;
    int  trans_t;
    char *pEntry;
    struct genmap *pNext;
};

/* 
 * ClassTypes
 */

#define HEAD 1
#define TAIL 2
#define DEVICE 3
#define MAP 4
#define ATTRIB 5
