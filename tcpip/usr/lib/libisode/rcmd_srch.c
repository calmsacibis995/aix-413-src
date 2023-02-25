static char sccsid[] = "@(#)16	1.3  src/tcpip/usr/lib/libisode/rcmd_srch.c, isodelib7, tcpip411, GOLD410 4/5/93 15:22:38";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: rcmd_srch
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rcmd_srch.c
 */

/* rcmd_srch.c: search a lookup table: return string value */

#ifndef lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rcmd_srch.c,v 1.2 93/02/05 17:08:33 snmp Exp $";
#endif

/*
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rcmd_srch.c,v 1.2 93/02/05 17:08:33 snmp Exp $
 *
 *
 * $Log:	rcmd_srch.c,v $
 * Revision 1.2  93/02/05  17:08:33  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:42  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:22  mrose
 * Release 6.0
 * 
 */

/*
 *                                NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include <isode/manifest.h>
#include <isode/cmd_srch.h>

/*  */

char   *rcmd_srch(val, cmd)
register int   val;
register CMD_TABLE *cmd;
{
	for(;cmd->cmd_key != NULLCP; cmd++)
		if(val == cmd->cmd_value)
			return(cmd->cmd_key);
	return(NULLCP);
}
