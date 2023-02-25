static char sccsid[] = "@(#)23  1.8  src/bos/usr/sbin/acct/fwtmp.c, cmdacct, bos411, 9428A410j 11/12/93 14:57:47";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: inp
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

#include <stdio.h>
#include <sys/types.h>
#include <utmp.h>
#include <locale.h>
#include "acctdef.h"
#include "acct_msg.h"
#define TSSIZE 128

char buf[TSSIZE];
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)
struct	utmp	Ut;

main (int argc, char **argv)
{
	int	c,
		cflg = 0,
		iflg = 0;

	setlocale(LC_ALL,"");
	catd = catopen(MF_ACCT, NL_CAT_LOCALE);
	while((c = getopt(argc, argv, "ci")) != EOF)
		switch(c) {
		case 'c':
			cflg = 1;
			continue;
		case 'i':
			iflg = 1;
			continue;
		default:
			fprintf(stderr,MSGSTR(FWTMPUSAGE,
				"Usage: fwtmp [-ci]\n"));
			exit(1);
		}

	for(;;) {
		if(iflg) {
			if (inp(stdin, &Ut) == EOF)
				break;
		} else {
			if (fread(&Ut, sizeof Ut, 1, stdin) != 1)
				break;
		}
		if (cflg)
			fwrite(&Ut, sizeof Ut, 1, stdout);
		else {
			strftime(buf, TSSIZE, "%c", localtime(&Ut.ut_time));
			printf("%-8.8s %-14.14s %-12.12s %2hd %5d %4.4ho %4.4ho %10lu %-16.16s %s\n",
				Ut.ut_name,
				Ut.ut_id,
				Ut.ut_line,
				Ut.ut_type,
				Ut.ut_pid,
				Ut.ut_exit.e_termination,
				Ut.ut_exit.e_exit,
				Ut.ut_time,
				Ut.ut_host,
				buf);
		}
	}
	exit ( 0 );
}

inp(file, u)
FILE *file;
register struct utmp *u;
{
	char	buf[BUFSIZ];
	register char *p;
	register i;
	int len;

	if (fgets((p = buf), BUFSIZ, file) == NULL)
		return EOF;

	for (i = 0; i < NSZ; i++)	/* Allow a space in name field */
		u->ut_name[i] = *p++;

	for (i = NSZ - 1; i >= 0; i--) {
		if (u->ut_name[i] == ' ')
			u->ut_name[i] = '\0';
		else
			break;
	}

	p++;

	for (i = 0; i < 14; i++)	/* Allow a space in name field */
		u->ut_id[i] = *p++;

	for (i = 14 - 1; i >= 0; i--) {
		if (u->ut_id[i] == ' ')
			u->ut_id[i] = '\0';
		else
			break;
	}

	p++;

	for (i = 0; i < LSZ; i++)	/* Allow a space in line field */
		u->ut_line[i] = *p++;

	for (i = LSZ - 1; i >= 0; i--) {
		if (u->ut_line[i] == ' ')
			u->ut_line[i] = '\0';
		else
			break;
	}

	/* %n returns how many characters read by sscanf */
	sscanf(p, "%hd %d %ho %ho %lu%n",
		&u->ut_type,
		&u->ut_pid,
		&u->ut_exit.e_termination,
		&u->ut_exit.e_exit,
		&u->ut_time,
		&len);

	p += len + 1;

	for (i = 0; i < 16; i++)
		u->ut_host[i] = *p++;

	for (i = 16 - 1; i >= 0; i--) {
		if (u->ut_host[i] == ' ')
			u->ut_host[i] = '\0';
		else
			break;
	}

	return((unsigned)u);
}
