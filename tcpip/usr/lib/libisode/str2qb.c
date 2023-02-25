static char sccsid[] = "@(#)73	1.3  src/tcpip/usr/lib/libisode/str2qb.c, isodelib7, tcpip411, GOLD410 4/5/93 17:01:46";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: str2qb
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2qb.c
 */

/* str2qb.c - string to qbuf */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2qb.c,v 1.2 93/02/05 17:13:02 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2qb.c,v 1.2 93/02/05 17:13:02 snmp Exp $
 *
 *
 * $Log:	str2qb.c,v $
 * Revision 1.2  93/02/05  17:13:02  snmp
 * ANSI - D67743
 * 
 * Revision 7.3  91/02/22  09:37:10  mrose
 * Interim 6.8
 * 
 * Revision 7.2  90/04/18  08:51:02  mrose
 * touch-up
 * 
 * Revision 7.1  90/02/19  13:09:48  mrose
 * update
 * 
 * Revision 7.0  89/11/23  22:13:49  mrose
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
#include <isode/psap.h>

/*  */

struct qbuf *str2qb (s, len, head)
char   *s;
int	len,
	head;
{
    register struct qbuf *qb,
			 *pb;

    if ((pb = (struct qbuf *) malloc ((unsigned) (sizeof *pb + len))) == NULL)
	return NULL;

    if (head) {
	if ((qb = (struct qbuf *) malloc (sizeof *qb)) == NULL) {
	    free ((char *) pb);
	    return NULL;
	}
	qb -> qb_forw = qb -> qb_back = qb;
	qb -> qb_data = NULL, qb -> qb_len = len;
	insque (pb, qb);
    }
    else {
	pb -> qb_forw = pb -> qb_back = pb;
	qb = pb;
    }

    pb -> qb_data = pb -> qb_base;
    if ((pb -> qb_len = len) > 0 && s)
	bcopy (s, pb -> qb_data, len);

    return qb;
}
