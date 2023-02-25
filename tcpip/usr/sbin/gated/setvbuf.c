static char sccsid[] = "@(#)23	1.1  src/tcpip/usr/sbin/gated/setvbuf.c, tcprouting, tcpip411, GOLD410 12/6/93 17:27:31";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		__PF4
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  setvbuf.c,v 1.1 1992/10/15 14:51:11 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

#include "include.h"

/*
 *	Emulation of setvbuf().
 */

#if	__GNUC__ >= 2
PROTOTYPE(setvbuf,
	  int,
	  (FILE *,
	   caddr_t,
	   int,
	   size_t));
#endif	/* __GNUC__ >= 2*/

int
setvbuf __PF4(stream, FILE *,
	      buf, caddr_t,
	      type, int,
	      size, size_t)
{
    int rc = 0;

    switch (type) {
#ifdef	_IOFBF
    case _IOFBF:
	setbuffer(stream, buf, size);
	break;
#endif	/* _IOFBF */

    case _IOLBF:
	(void) setlinebuf(stream);
	break;

    case _IONBF:
	setbuf(stream, NULL);
	break;

    default:
	assert(FALSE);
    }

    return rc;
}
