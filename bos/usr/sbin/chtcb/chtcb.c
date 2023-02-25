static char sccsid[] = "@(#)68  1.12.1.1  src/bos/usr/sbin/chtcb/chtcb.c, cmdspriv, bos411, 9428A410j 2/2/94 13:02:58";

/*
 * COMPONENT_NAME:  (CMDSPRIV) security: privilege management
 *
 * FUNCTIONS: chtcb command
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	<stdio.h>
#include	<sys/tcb.h>
#include 	<errno.h>
#include	<locale.h>

#include "chtcb_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_CHTCB,n,s) 


/* Print syntax message. */
void usage (prog)
char *prog;
{
/*
	(void) fprintf(stderr,
MSGSTR(USAGE, "Usage: %s {on|off|tp|query} <file> ...\n"),prog);
*/
	(void) fprintf(stderr,
MSGSTR(USAGE, "Usage: %s {on|off|query} <file> ...\n"),prog);
	exit ( EINVAL );
}

main (ac, av)
int	ac;
char	*av[];
{
	char	*ProgName;
	int	i, rtn;
	int	cmd;
	int	rc;

	ProgName = av[0];
	rtn = 0;

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_CHTCB, NL_CAT_LOCALE);

	if (ac < 3)
		usage (ProgName);
	if (strcmp (av[1], "on") == 0)
		cmd = TCB_ON;
/*
	else if (strcmp (av[1], "tp") == 0)
		cmd = TP_ON;
*/
	else if (strcmp (av[1], "off") == 0)
		cmd = TCB_OFF;
	else if (strcmp (av[1], "query") == 0)
		cmd = TCB_QUERY;
	else
		usage (ProgName);

	for (i = 2; i < ac; i++)
	{
		if ((rc = tcb ( av[i], cmd )) < 0) {
			rtn = errno;
			perror (av[i]);
		}
		else if (cmd == TCB_QUERY) {
			if (rc == 0) 
				(void) fprintf (stdout,
/*
MSGSTR( NOTINTCB, "%s is not in the TCB or TP\n"), av[i]);
*/
MSGSTR( NOTINTCB, "%s is not in the TCB\n"), av[i]);

/*
			else if ( rc == (TCB_ON|TP_ON) )
				(void) fprintf (stdout,
MSGSTR( ISINBOTH, "%s is in the TCB and TP\n"), av[i]);

			else if ( rc == TP_ON )
				(void) fprintf (stdout,
MSGSTR( ISINTP, "%s is in the TP\n"), av[i]);
*/

			else if ( rc == TCB_ON )
				(void) fprintf (stdout,
MSGSTR( ISINTCB, "%s is in the TCB\n"), av[i]);

			else {
				rtn = EINVAL;
				(void) fprintf (stdout,
MSGSTR( TCBERROR, "tcb() error=%d\n"), rc);
			}

		}
					
	}

	exit(rtn);
}
