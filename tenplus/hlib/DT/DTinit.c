#if !defined(lint)
static char sccsid[] = "@(#)48	1.6  src/tenplus/hlib/DT/DTinit.c, tenplus, tenplus411, GOLD410 3/23/93 12:00:03";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	DTinit 
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <stddef.h>
#include <langinfo.h>
#include <string.h>

char *months [12][2] ;

/* DTinit: initialize various DT library globals
   Arguments: none
*/

void DTinit (void)
{

#ifdef DEBUG
    debug ("DTinit");
#endif

    /* Get locale specific information on the names of the
       months and their abbreviations
    */
    months[ 0][0] = strdup(nl_langinfo(MON_1 ));
    months[ 1][0] = strdup(nl_langinfo(MON_2 ));
    months[ 2][0] = strdup(nl_langinfo(MON_3 ));
    months[ 3][0] = strdup(nl_langinfo(MON_4 ));
    months[ 4][0] = strdup(nl_langinfo(MON_5 ));
    months[ 5][0] = strdup(nl_langinfo(MON_6 ));
    months[ 6][0] = strdup(nl_langinfo(MON_7 ));
    months[ 7][0] = strdup(nl_langinfo(MON_8 ));
    months[ 8][0] = strdup(nl_langinfo(MON_9 ));
    months[ 9][0] = strdup(nl_langinfo(MON_10));
    months[10][0] = strdup(nl_langinfo(MON_11));
    months[11][0] = strdup(nl_langinfo(MON_12));
 
    months[ 0][1] = strdup(nl_langinfo(ABMON_1 ));
    months[ 1][1] = strdup(nl_langinfo(ABMON_2 ));
    months[ 2][1] = strdup(nl_langinfo(ABMON_3 ));
    months[ 3][1] = strdup(nl_langinfo(ABMON_4 ));
    months[ 4][1] = strdup(nl_langinfo(ABMON_5 ));
    months[ 5][1] = strdup(nl_langinfo(ABMON_6 ));
    months[ 6][1] = strdup(nl_langinfo(ABMON_7 ));
    months[ 7][1] = strdup(nl_langinfo(ABMON_8 ));
    months[ 8][1] = strdup(nl_langinfo(ABMON_9 ));
    months[ 9][1] = strdup(nl_langinfo(ABMON_10));
    months[10][1] = strdup(nl_langinfo(ABMON_11));
    months[11][1] = strdup(nl_langinfo(ABMON_12));
 
    return;
}
