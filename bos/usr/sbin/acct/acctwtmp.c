static char sccsid[] = "@(#)17  1.7  src/bos/usr/sbin/acct/acctwtmp.c, cmdacct, bos411, 9428A410j 11/12/93 14:57:41";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: none
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
 *      acctwtmp reason >> /var/adm/wtmp
 *	writes utmp.h record (with current time) to end of std. output
 *      acctwtmp `uname` >> /var/adm/wtmp as part of startup
 *      acctwtmp pm >> /var/adm/wtmp  (taken down for pm, for example)
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <utmp.h>
#include "acctdef.h"
#include "acct_msg.h"
nl_catd catd;
#define	MSGSTR(N,X)	catgets(catd, MS_ACCT,N,X)

struct	utmp	wb;
int	mb_cur_max;
int	num = 0; 
int	tnum = 0;
char *p;
main(int argc, char **argv)
{
	setlocale(LC_ALL, "");
	catd = catopen(MF_ACCT, NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;

	if (argc < 2) {
		fprintf(stderr,MSGSTR(USAGEWTMP,"Usage: %s reason [ >> %s ]\n"),
			argv[0], WTMP_FILE);
		exit(1);
	}
	p = argv[1];
	while (*p != '\0') {
		num = mblen(p,mb_cur_max);
		tnum += num;
		if (tnum < LSZ)
			strncat(wb.ut_line,p,num);
		else
			break;
		p += num;
	}
	wb.ut_line[11] = '\0';
	wb.ut_type = ACCOUNTING;
	time(&wb.ut_time);
	fwrite((void*)&wb, (size_t) sizeof(wb), (size_t)1, stdout);
	exit(0);
}
