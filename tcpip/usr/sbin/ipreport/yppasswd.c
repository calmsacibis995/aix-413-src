static char sccsid[] = "@(#)46	1.1  src/tcpip/usr/sbin/ipreport/yppasswd.c, tcpip, tcpip411, GOLD410 10/22/93 13:55:56";
/*
 * COMPONENT_NAME: TCPIP yppasswd.c
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 24
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * This file contains routines for the proclist, which is a linked list
 * of function pointers. Given any rpc program, and rpc procedure within
 * that program, (i.e. NFSPROGRAM,NFSSTATFS,) we should be able to return
 * a pointer to the xdr routine that decodes that result, (and to the
 * dump/print routine that prints it.)
 * 
 * Created Thu Sep 30 1993 
*/

#include <pwd.h>
#include <rpc/rpc.h>
#include "yppasswd.h"
#include "ipr.h"


/* Prefix for YP password data */
static char *ypp_beg = "YPP: ";


/* XDR for YP password change requests */

int
xdr_yppasswd(xdrsp, pp)
        XDR *xdrsp;
        struct yppasswd *pp;
{
        if (xdr_wrapstring(xdrsp, &pp->oldpass) == 0)
                return (0);
        if (xdr_passwd(xdrsp, &pp->newpw) == 0)
                return (0);
        return (1);
}

static
xdr_passwd(xdrsp, pw)
        XDR *xdrsp;
        struct passwd *pw;
{
        if (xdr_wrapstring(xdrsp,
&pw->pw_name) == 0)
                return (0);
        if (xdr_wrapstring(xdrsp, &pw->pw_passwd) == 0)
                return (0);
        if (xdr_int(xdrsp, &pw->pw_uid) == 0)
                return (0);
        if (xdr_int(xdrsp, &pw->pw_gid) == 0)
                return (0);
        if (xdr_wrapstring(xdrsp, &pw->pw_gecos) == 0)
                return (0);
        if (xdr_wrapstring(xdrsp, &pw->pw_dir) == 0)
                return (0);
        if (xdr_wrapstring(xdrsp, &pw->pw_shell) == 0)
                return (0);
        return (1);
}



/* Print the contents of a YP password change request. */

prt_yppasswd(pp)
        struct yppasswd *pp;
{
	reset_beg_line(ypp_beg);

	printf("%sPassword update request\n", beg_line);
	printf("%sOld password (CLEARTEXT): %s\n", beg_line, pp->oldpass);
	printf("%sUsername: %s\n", beg_line, pp->newpw.pw_name);
	printf("%sNew Password (ENCRYPTED): %s\n", beg_line, pp->newpw.pw_passwd);
	printf("%sUser ID: %lu\n", beg_line, pp->newpw.pw_uid);
	printf("%sGroup ID: %lu\n", beg_line, pp->newpw.pw_gid);
	printf("%sGECOS: %s\n", beg_line, pp->newpw.pw_gecos);
	printf("%sHome directory: %s\n", beg_line, pp->newpw.pw_dir);
	printf("%sLogin shell: %s\n", beg_line, pp->newpw.pw_shell);
}

/* Print the reply to a YP password change request. */

prt_yppasswd_response(pp)
	int *pp;
{
	reset_beg_line(ypp_beg);

	printf("%sPassword update response\n", beg_line);
	printf("%sResponse: %d (%s)\n", beg_line, *pp,
		*pp?"FAILURE":"SUCCESS");
}

