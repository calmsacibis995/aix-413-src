static char sccsid[] = "@(#)92	1.3  src/bos/usr/sbin/adfutil/file.c, cmdadf, bos411, 9428A410j 6/15/90 16:57:58";
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (file) yyerror()
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

#include "symbol.h"
#include "externs.h"

char sbError[128];

/*
 * NAME: yyerror
 *                                                                    
 * FUNCTION: 
 *    print error message, file name, and line number on stderr
 *                                                                    
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN: (none)
 *	
 */  

yyerror(sb, w1, w2, w3, w4, w5, w6, w7, w8)
char *sb;
int w1, w2, w3, w4, w5, w6, w7, w8;
{
    sprintf(sbError, sb, w1, w2, w3, w4, w5, w6, w7, w8);
    fprintf(stderr, NL_MSG(YY_ERR, "%s, line %3d: %s\n"),
            sbFile, cLines, sbError);
    cErrors++;
    return;
}
