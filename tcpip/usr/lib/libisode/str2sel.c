static char sccsid[] = "@(#)30	1.3  src/tcpip/usr/lib/libisode/str2sel.c, isodelib7, tcpip411, GOLD410 4/5/93 17:03:12";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: str2sel
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2sel.c
 */

/* str2sel.c - string to selector */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2sel.c,v 1.2 93/02/05 17:13:08 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2sel.c,v 1.2 93/02/05 17:13:08 snmp Exp $
 *
 *
 * $Log:	str2sel.c,v $
 * Revision 1.2  93/02/05  17:13:08  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:15:57  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/11/21  11:29:59  mrose
 * sun
 * 
 * Revision 7.0  89/11/23  21:23:37  mrose
 * Release 6.0
 * 
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include <ctype.h>
#include <stdio.h>
#include <isode/general.h>
#include <isode/manifest.h>
#include <isode/tailor.h>


#define	QUOTE	'\\'

/*    STR2SEL */

int	str2sel (s, quoted, sel, n)
char   *s,
       *sel;
int	quoted,
	n;
{
    int     i,
            r;
    register char  *cp;

    if (*s == NULL)
	return 0;

    if (quoted <= 0) {
	for (cp = s; *cp; cp++)
	    if (!isxdigit ((u_char) *cp))
		break;

	if (*cp == NULL && (i = (cp - s)) >= 2 && (i & 0x01) == 0) {
	    if (i > (r = n * 2))
		i = r;
	    i = implode ((u_char *) sel, s, i);
	    if ((r = (n - i)) > 0)
		bzero (sel + i, r);
	    return i;
	}
	if (*s == '#') {	/* gosip style, network byte-order */
	    i = atoi (s + 1);
	    sel[0] = (i >> 8) & 0xff;
	    sel[1] = i & 0xff;

	    return 2;
	}

	DLOG (compat_log, LLOG_EXCEPTIONS, ("invalid selector \"%s\"", s));
    }

    for (cp = sel; *s && n > 0; cp++, s++, n--)
	if (*s != QUOTE)
	    *cp = *s;
	else
	    switch (*++s) {
		case 'b':
		    *cp = '\b';
		    break;
		case 'f':
		    *cp = '\f';
		    break;
		case 'n':
		    *cp = '\n';
		    break;
		case 'r':
		    *cp = '\r';
		    break;
		case 't':
		    *cp = '\t';
		    break;

		case NULL: 
		    s--;
		case QUOTE: 
		    *cp = QUOTE;
		    break;

		default: 
		    if (!isdigit ((u_char) *s)) {
			*cp++ = QUOTE;
			*cp = *s;
			break;
		    }
		    r = *s != '0' ? 10 : 8;
		    for (i = 0; isdigit ((u_char) *s); s++)
			i = i * r + *s - '0';
		    s--;
		    *cp = toascii (i);
		    break;
	    }
    if (n > 0)
	*cp = NULL;

    return (cp - sel);
}
