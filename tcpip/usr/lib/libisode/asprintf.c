static char sccsid[] = "@(#)88	1.4  src/tcpip/usr/lib/libisode/asprintf.c, isodelib7, tcpip411, 9434A411a 8/18/94 14:10:32";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: _asprintf asprintf
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/asprintf.c
 */

/* asprintf.c - sprintf with errno */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/asprintf.c,v 1.2 93/02/05 17:03:37 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/asprintf.c,v 1.2 93/02/05 17:03:37 snmp Exp $
 *
 *
 * $Log:	asprintf.c,v $
 * Revision 1.2  93/02/05  17:03:37  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:14:54  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/02/19  13:07:10  mrose
 * update
 * 
 * Revision 7.0  89/11/23  21:22:54  mrose
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

#include <stdio.h>
#include <varargs.h>
#include <isode/general.h>
#include <isode/manifest.h>

/*    DATA */

extern int errno;

/*  */

void	asprintf (bp, ap)		/* what, fmt, args, ... */
char *bp;
va_list	ap;
{
    char   *what;

    what = va_arg (ap, char *);

    _asprintf (bp, what, ap);
}


void	_asprintf (bp, what, ap)	/* fmt, args, ... */
register char *bp;
char   *what;
va_list	ap;
{
    register int    eindex;
    char   *fmt;

    eindex = errno;

    *bp = NULL;
    fmt = va_arg (ap, char *);

    if (fmt) {
#ifndef	VSPRINTF
	struct _iobuf iob;
#endif

#ifndef	VSPRINTF
#ifdef	pyr
	bzero ((char *) &iob, sizeof iob);
	iob._file = _NFILE;
#endif
	iob._flag = _IOWRT | _IOSTRG;
#if	!defined(vax) && !defined(pyr)
	iob._ptr = (unsigned char *) bp;
#else
	iob._ptr = bp;
#endif
	iob._cnt = BUFSIZ;
	_doprnt (fmt, ap, &iob);
	putc (NULL, &iob);
#else
	(void) vsprintf (bp, fmt, ap);
#endif
	bp += strlen (bp);

    }

    if (what) {
	if (*what) {
	    (void) sprintf (bp, " %s: ", what);
	    bp += strlen (bp);
	}
	(void) strcpy (bp, strerror (eindex));
	bp += strlen (bp);
    }

    errno = eindex;
}
