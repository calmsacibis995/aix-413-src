static char sccsid[] = "@(#)54	1.6  src/bos/usr/sbin/crash/xdumpx.c, cmdcrash, bos411, 9428A410j 5/1/91 14:31:08";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: xdumpx,s_cmp,s_cpy
 *
 * ORIGINS: 27,3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include "crash.h"

/* xdumpx.c	7.1 87/07/08 16:30:52 */

xdumpx(saddr,xaddr,count)
char *xaddr;
char *saddr;
int count;

{

#define LINESIZE     60
#define NHALE1       37
#define ASCIISTRT    38
#define NHALE2       54
#define HEXEND       36

    int i, j, k, hexdigit;
    register int c;
    char *hexchar;
    char linebuf[LINESIZE+1];
    char prevbuf[LINESIZE+1];
    char *linestart;
    int asciistart;
    char asterisk = ' ';
    int astcount = 0;
    void s_cpy ();
    int s_cmp ();


    hexchar = "0123456789abcdef";
    prevbuf[0] = '\0';

    for (i = 0; i < count;) {
	for (j = 0; j < LINESIZE; j++)
	    linebuf[j] = ' ';
	linebuf[NHALE1] = '*';
	linebuf[NHALE2] = '*';

	linestart = xaddr;
	asciistart = ASCIISTRT;
	for (j = 0; j < HEXEND;) {
	    for (k = 0; k < 4; k++) {
		c = *(saddr++) & 0xFF;
		if ((c >= 0x20) && (c <= 0x7e))
		    linebuf[asciistart++] = (char) c;
		else
		    linebuf[asciistart++] = '.';
		hexdigit = c >> 4;
		linebuf[j++] = hexchar[hexdigit];
		hexdigit = c & 0x0f;
		linebuf[j++] = hexchar[hexdigit];
		i++;
	    }
	    if (i >= count)
		break;
	    linebuf[j++] = ' ';
	}
	xaddr += 16;
	linebuf[LINESIZE] = '\0';
	if (((j = s_cmp (linebuf, prevbuf)) == 0) && (i < count)) {
	    if (asterisk == ' ') {
		asterisk = '*';
	        astcount++;
		(void) printf ("    * * * *   ");
	    }
	    else
	        astcount++;
	}
	else {
	    if (astcount != 0) {
		if(astcount > 1)
	         printf ( catgets(scmc_catd, MS_crsh, XDUMPX_MSG_2, "   %d lines same as above\n") ,astcount);
		else
	         printf ( catgets(scmc_catd, MS_crsh, XDUMPX_MSG_3, "   %d line same as above\n") ,astcount);
		astcount = 0;
	    }
	    (void) printf ("    %08x  %s\n",linestart, linebuf);
	    asterisk = ' ';
	    s_cpy (prevbuf, linebuf);
	}
    }

    return;
}

int s_cmp(s1,s2)
register char *s1,*s2;
{
    while ((*s1) && (*s1 == *s2)) {
	s1++;
	s2++;
    }
    if (*s1 || *s2)
	return(-1);
    else
	return(0);
}

void s_cpy(s1,s2)
register char *s1,*s2;
{
    while ((*s1 = *s2) != '\0') {
	s1++;
	s2++;
    }
}

