static char sccsid[] = "@(#)94	1.3  src/tcpip/usr/lib/libisode/cmd_srch.c, isodelib7, tcpip411, GOLD410 4/5/93 13:46:42";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: cmd_srch
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/cmd_srch.c
 */

/* cmd_srch.c - search a lookup table: return numeric value */

#ifndef lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/cmd_srch.c,v 1.2 93/02/05 17:03:54 snmp Exp $";
#endif

/*
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/cmd_srch.c,v 1.2 93/02/05 17:03:54 snmp Exp $
 *
 *
 * $Log:	cmd_srch.c,v $
 * Revision 1.2  93/02/05  17:03:54  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:06  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:22:58  mrose
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

/* map a string onto a value */

cmd_srch(str, cmd)
register char   *str;
register CMD_TABLE *cmd;
{
	for(;cmd->cmd_key != NULLCP; cmd++)
		if(lexequ(str, cmd->cmd_key) == 0)
			return(cmd->cmd_value);
	return(cmd->cmd_value);
}
