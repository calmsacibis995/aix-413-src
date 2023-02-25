/* @(#)30	1.2.1.1  src/tcpip/usr/include/libffdb.h, tcpip, tcpip411, GOLD410 3/30/94 17:18:30 */
/*
 * COMPONENT_NAME:  TCPIP
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define  MAXRECLEN      512	/* max length of line in database */
#define  MAXRECORDS     4096	/* max number of records in database */
#define	 DELIM          " ,\t\n" /* field delimiters */
#define  SKIPDELIM(bp)  (bp += strspn(bp, DELIM)) /* move past any delimeters */
#define  SKIPTOKEN(bp)  (bp += strcspn(bp, DELIM)) /* move past any non-delimeters */
#define  DELIMITER(ch)  (strchr(DELIM, ch)) /* ch is a delimiter */
#define  PARSE_LIST(list,tokenarray) \
    do { \
	*tokenarray++ = list; \
	SKIPTOKEN(list); \
	if (*list) { \
	    *list++ = '\0'; \
	    SKIPDELIM(list); \
	} \
    } while (*list); \
    *tokenarray = NULL;
