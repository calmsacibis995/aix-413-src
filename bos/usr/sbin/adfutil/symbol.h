/* @(#)04	1.5  src/bos/usr/sbin/adfutil/symbol.h, cmdadf, bos411, 9428A410j 6/19/91 13:18:25 */
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS:  symbol.h - hash table, symbol table def's
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

#include <stdio.h>
#include <sys/types.h>
#include <nl_types.h>
#include "dtype.h"
#include "adf_msg.h"

#define true 1
#define false 0

#define iheMax 100

struct he {		/* hash table entry */
    char *sb;		/* name of symbol */
    struct he *phe;	/* next entry */
    int  tokid;		/* token number id */
};

#define YYDEBUG 1

#define NL_MSG(n,s) catgets(ADF_ERR, ADF_MSG, n, s)
extern nl_catd ADF_ERR;
