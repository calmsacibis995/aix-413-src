static char sccsid[] = "@(#)14  1.8.1.2  src/bos/usr/sbin/acct/accton.c, cmdacct, bos411, 9428A410j 11/12/93 14:57:31";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<errno.h>
#include	<stdio.h>
#include        <locale.h>
#include "acct_msg.h"
nl_catd catd;
#define	MSGSTR(N,X)	catgets(catd, MS_ACCT,N,X)
#define	ERR	(-1)

main(int argc, char **argv) 
{
	setlocale(LC_ALL,"");
	catd= catopen(MF_ACCT, NL_CAT_LOCALE);

	if (argc > 1) {                    /* file name argument */
		if(acct(argv[1]) == ERR) { /* turn cmd accounting on */
			perror(argv[0]);
			fprintf(stderr,MSGSTR(ACCTCANTON,
				"cannot turn accounting ON\n"));
			exit(1);
		}
	}
/*
 * The following else branch currently never returns
 * an ERR.  In other words, you may turn the accounting
 * off to your heart's content.
 */
	else if (acct((char *)0) == ERR) { /* no file name argument */
		perror(argv[0]);           /* turn cmd acct off */
		fprintf(stderr,MSGSTR(ACCTNOOFF,
			"cannot turn accounting OFF\n"));
		exit(1);
	}
	exit(0);
}
