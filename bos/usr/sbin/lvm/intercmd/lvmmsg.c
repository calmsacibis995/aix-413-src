static char sccsid[] = "@(#)75	1.3  src/bos/usr/sbin/lvm/intercmd/lvmmsg.c, cmdlvm, bos411, 9428A410j 1/18/94 10:06:53";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*
 * FILE NAME: lvmmsg.c
 *
 * FILE DESCRIPTION: Prints to stdout the NLS message string referenced
 *                   by the input parameter.
 *
 * RETURN VALUE DESCRIPTION:
 *			0   Command Successfully completed
 *                      1   Command Unsuccessful
 *
 * EXTERNAL PROCEDURES CALLED: lvm_msg
 *
 */

/* Header and Include files */
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include "cmdlvm_msg.h"         /* include files for message tests */

extern char *lvm_msgstr();
nl_catd  scmc_catd;   /* NL message Cat descriptor */

main(argc,argv)

int argc;
char **argv;
{
	int
	    msg_num,
	    rc;

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);
 
	if (argc != 2)
		rc=1;
	else {
		sscanf(argv[1], "%d", &msg_num);
		printf("%s",lvm_msgstr(msg_num));
		rc=0;
	}
	exit(rc);
}

