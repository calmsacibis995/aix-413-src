static char sccsid[] = "@(#)72	1.3  src/tcpip/usr/lib/libisode/str2ps.c, isodelib7, tcpip411, GOLD410 4/5/93 17:01:13";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: str_close str_open str_read str_setup str_write
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2ps.c
 */

/* str2ps.c - string-backed abstraction for PStreams */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2ps.c,v 1.2 93/02/05 17:13:00 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2ps.c,v 1.2 93/02/05 17:13:00 snmp Exp $
 *
 *
 * $Log:	str2ps.c,v $
 * Revision 1.2  93/02/05  17:13:00  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/03/09  11:55:38  mrose
 * update
 * 
 * Revision 7.1  91/02/22  09:37:07  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:48  mrose
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

/* ARGSUSED */

static int str_read (ps, data, n, in_line)
register PS	ps;
PElementData data;
PElementLen n;
int	in_line;
{
    register int    cc;

    if (ps -> ps_base == NULLCP || (cc = ps -> ps_cnt) <= 0)
	return 0;
    if (cc > n)
	cc = n;

    bcopy (ps -> ps_ptr, (char *) data, cc);
    ps -> ps_ptr += cc, ps -> ps_cnt -= cc;

    return cc;
}


/* ARGSUSED */

static int str_write (ps, data, n, in_line)
register PS	ps;
PElementData data;
PElementLen n;
int	in_line;
{
    register int    cc;
    register char  *cp;

    if (ps -> ps_base == NULLCP) {
	if ((cp = (char *)malloc ((unsigned) (cc = n + BUFSIZ))) == NULLCP)
	    return ps_seterr (ps, PS_ERR_NMEM, NOTOK);
	ps -> ps_base = ps -> ps_ptr = cp;
	ps -> ps_bufsiz = ps -> ps_cnt = cc;
    }
    else
	if (ps -> ps_cnt < n) {
	    register int    curlen = ps -> ps_ptr - ps -> ps_base;

	    if (ps -> ps_inline) {
		n = ps -> ps_cnt;
		goto partial;
	    }

	    if ((cp = (char *)realloc (ps -> ps_base,
				      (unsigned) (ps -> ps_bufsiz
						  + (cc = n + BUFSIZ))))
		    == NULLCP)
		return ps_seterr (ps, PS_ERR_NMEM, NOTOK);
	    ps -> ps_ptr = (ps -> ps_base = cp) + curlen;
	    ps -> ps_bufsiz += cc, ps -> ps_cnt += cc;
	}
partial: ;

    bcopy ((char *) data, ps -> ps_ptr, n);
    ps -> ps_ptr += n, ps -> ps_cnt -= n;

    return n;
}


static int str_close (ps)
register PS	ps;
{
    if (ps -> ps_base && !ps -> ps_inline)
	free (ps -> ps_base);

    return OK;
}

/*  */

int	str_open (ps)
register PS	ps;
{
    ps -> ps_readP = str_read;
    ps -> ps_writeP = str_write;
    ps -> ps_closeP = str_close;

    return OK;
}


int	str_setup (ps, cp, cc, in_line)
register PS	ps;
register char  *cp;
register int	cc;
int	in_line;
{
    register char  *dp;

    if (in_line) {
	ps -> ps_inline = 1;
	ps -> ps_base = ps -> ps_ptr = cp;
	ps -> ps_bufsiz = ps -> ps_cnt = cc;
    }
    else
	if (cc > 0) {
	    if ((dp = (char *)malloc ((unsigned) (cc))) == NULLCP)
		return ps_seterr (ps, PS_ERR_NMEM, NOTOK);
	    ps -> ps_base = ps -> ps_ptr = dp;
	    if (cp != NULLCP)
		bcopy (cp, dp, cc);
	    ps -> ps_bufsiz = ps -> ps_cnt = cc;
	}

    return OK;
}
