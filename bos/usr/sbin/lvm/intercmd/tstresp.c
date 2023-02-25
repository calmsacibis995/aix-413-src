static char sccsid[] = "@(#)39	1.5  src/bos/usr/sbin/lvm/intercmd/tstresp.c, cmdlvm, bos41J, 9520B_all 5/18/95 16:33:51";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: chpv.sh
 *
 * ORIGINS: 27
 * 
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * [End of PROLOG]
 */

/*
 * RETURN VALUE DESCRIPTION: 1 for YES response
 *                           0 for NO response
 *                          -1 for error condition
 */

#include <stdio.h>
#include <locale.h>
#include <sys/localedef.h>
extern char	*setlocale();

main(argc, argv)
int argc;
char *argv[];
{
	if(argc ==1) return(-1);
	setlocale(LC_ALL, "");
	return(rpmatch(argv[1])) ;
}
