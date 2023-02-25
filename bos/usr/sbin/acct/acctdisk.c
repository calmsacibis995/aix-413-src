static char sccsid[] = "@(#)10 1.7  src/bos/usr/sbin/acct/acctdisk.c, cmdacct, bos411, 9428A410j 1/13/94 18:10:44";
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

/*
 *      acctdisk <dtmp >dacct
 *	reads std.input & converts to tacct.h format, writes to output
 *	input:
 *	uid	name	#blocks
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <locale.h>
#include "tacct.h"

struct	tacct	tb;
char	ntmp[NSZ+1];

#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(N,X)     catgets(catd, MS_ACCT,N,X)

#define M_BADRECORD  "%s: An Invalid record is detected at line %lu in the input file. Quitting.\n"

char *prog;

main(argc, argv)
char **argv;
{
	int nread;
	unsigned long count=0; /* Count of no. of records */
	setlocale(LC_ALL,"");
	catd = catopen(MF_ACCT, NL_CAT_LOCALE);
	prog = argv[0];

	tb.ta_dc = 1;
	while((nread=scanf("%lu\t%s\t%f",
		&tb.ta_uid,
		ntmp,
		&tb.ta_du)) == 3) {

		CPYN(tb.ta_name, ntmp);
		fwrite((void *)&tb,(size_t) sizeof(tb), (size_t)1, stdout);
		count++;
	}
	if (nread != EOF) {
		fprintf(stderr, MSGSTR( BADRECORD, M_BADRECORD), prog, count);
		exit(1);
	}
	exit(0);
}
