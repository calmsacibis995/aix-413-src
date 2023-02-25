static char sccsid[] = "@(#)62	1.3  src/tcpip/usr/lib/libisode/testdebug.c, isodelib7, tcpip411, GOLD410 4/5/93 17:10:51";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: testdebug
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/testdebug.c
 */

/* automatically generated by pepy 6.0 #89 (oilers.netmgmt.austin.ibm.com), do not edit! */

#include <isode/psap.h>

static char *pepyid = "pepy 6.0 #89 (oilers.netmgmt.austin.ibm.com) of Mon Apr 01 16:47:34 CST 1991";

#define	advise	PY_advise

void	advise ();

/* Generated from module TESTDEBUG */
# line 27 "testdebug.py"

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/testdebug.c,v 1.2 93/02/05 17:13:33 snmp Exp $";
#endif

/* LINTLIBRARY */


#include <stdio.h>


#ifndef PEPYPARM
#define PEPYPARM char *
#endif /* PEPYPARM */
extern PEPYPARM NullParm;
# line 43 "testdebug.py"

int	testdebug (pe, s)
register PE	pe;
register char	*s;
{
    char  *cp;
    register PS ps;
    static int debug = OK;
    
    switch (debug) {
	case NOTOK:
	    return(0);

	case OK:
	    if ((debug = (cp = getenv ("PEPYDEBUG")) && *cp ? atoi (cp)
							    : NOTOK) == NOTOK)
		return(0);
	    (void) fflush (stdout);
	    fprintf (stderr, "testdebug made with %s\n", pepyid);
	    /* and fall... */

	default:
	    (void) fflush (stdout);
	    fprintf (stderr, "%s\n", s);

	    if ((ps = ps_alloc (std_open)) == NULLPS)
		break;
	    if (std_setup (ps, stderr) != NOTOK)
		(void) pe2pl (ps, pe);
	    fprintf (stderr, "--------\n");
	    ps_free (ps);
	    break;
    }
    return(0);
}
