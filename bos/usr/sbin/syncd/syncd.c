static char sccsid[] = "@(#)62	1.3.1.1  src/bos/usr/sbin/syncd/syncd.c, cmdfs, bos411, 9428A410j 1/10/94 18:00:15";
/*
 * COMPONENT_NAME: (CMDFS) file system commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 3
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

#include <stdio.h>
#include <locale.h>
#include "syncd_msg.h"
nl_catd catd;
#define MSGSTR(num,str) catgets(catd, MS_SYNCD,num,str)

void
main(argc, argv)
int argc;
char **argv;
{
	char *string;
	char c;
	int time;

	(void) setlocale(LC_ALL,"");
	
	catd = catopen(MF_SYNCD, NL_CAT_LOCALE);

	if (argc != 2) {
		fprintf(stderr, MSGSTR(USG, "usage: syncd time\n"));
		exit(2);
	}

	string = argv[1];
	time = 0;

	while(c = *string++) {
		if(c<'0' || c>'9') {
			fprintf(stderr, MSGSTR(BADCHAR,
				"syncd: bad character in argument\n"));
			exit(2);
		}
		time = time * 10 + c - '0';
	}

	while(1) {
		(void) sync();
		(void) sleep(time);
	}

}
