static char sccsid[] = "@(#)11  1.13.1.2  src/bos/usr/sbin/acct/acctdusg.c, cmdacct, bos411, 9428A410j 11/12/93 14:51:12";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: openerr, compare
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
 *	acctdusg [-p file] [-u file] > dtmp-file
 *	-p	get password info from file
 *	-u	file for names of files not charged to anyone
 *	reads input (normally from find / -print)
 *	and compute disk resource consumption by user
 */
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <sys/vmount.h>
#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)
#define PFILE "/etc/passwd"   /* the default passwd file */
 
struct	table {
	char	name[8];	/* login name */
	uid_t   uid;		/* user id of login name */
	ulong	du;		/* disk usage */
} * user;
char    *pfile = PFILE;  /* system passwd file */
struct	stat	statb;
FILE	*pswd, *nchrg;
FILE	*names = stdin;
FILE	*accto  = stdout;
char	fbuf[PATH_MAX + 1];
int	compare(struct table *p1, struct table *p2);
int	count = 0;

main(int argc, char **argv)
{
	int	c, errflag = 0;
	struct table key;
	struct table *found;

	setlocale(LC_ALL,"");
	catd = catopen(MF_ACCT, NL_CAT_LOCALE);

	while((c = getopt(argc, argv, "p:u:")) != EOF)
		switch(c) {
			case 'p':
				pfile = optarg;
				continue;
			case 'u':
				if ((nchrg = fopen(optarg,"w")) == NULL)
					openerr(optarg);
				acl_set(optarg, R_ACC | W_ACC, R_ACC, R_ACC);
				continue;
			default:
				fprintf(stderr, MSGSTR(DUSGUSAGE,
				"Usage: acctdusg [-p file] [-u file]\n"));
				exit(1);
		}

	if ((pswd = fopen(pfile, "r")) == NULL)
		openerr(pfile);

	while ((c = getc(pswd)) != EOF) /* count lines of /etc/passwd */
		if (c == '\n')          /* get number of users        */
			count++;

	if ((user = calloc(count, sizeof (* user))) == NULL) {
		perror("acctdusg");
		exit(1);
	}

	rewind(pswd);
	count = 0;
	while (fscanf(pswd,"%[^:]:%*[^0-9]%lu%*[^\n]\n",user[count].name,&user[count].uid) != EOF)
		count++;

	fclose(pswd);
	qsort(user, count, sizeof(user[0]), compare);

	while(fgets(fbuf, PATH_MAX + 1, names) != NULL) {
		fbuf[strlen(fbuf) - 1] = '\0';   /* remove newline since */
		if (lstat(fbuf, &statb) == -1) { /* lstat can't take it  */
			perror(fbuf);
			errflag++;
			continue;
		}
		if (statb.st_vfstype != MNT_JFS)
			continue;
		key.uid = statb.st_uid;
		if (found = bsearch(&key,user,count,sizeof(user[0]),compare))
			found->du += S_ISLNK(statb.st_mode) ? 1 :
				(S_ISDIR(statb.st_mode) ? statb.st_blocks :
				(S_ISREG(statb.st_mode) ?
				(statb.st_blocks / statb.st_nlink) : 0L));
		else if (nchrg && statb.st_size) /* uid of file not match */
			fprintf(nchrg,"%11lu\t%7lu\t%s\n",
				statb.st_uid,statb.st_blocks,fbuf);
	}
	if (names != stdin)
		fclose(names);

	for(c = 0; c < count; c++) { /* output */
		if(user[c].du)
			fprintf(accto,
				"%011lu\t%-8.8s\t%7lu\n",
				user[c].uid,
				user[c].name,
				user[c].du);
	}

	if (accto != stdout)
		fclose(accto);
	if (nchrg)
		fclose(nchrg);
	exit(errflag ? 1 : 0);
}

openerr(char *file)
{
	fprintf(stderr,MSGSTR(CANTOPEN,"%s: cannot open %s\n"),"acctdusg",file);
	exit(1);
}

compare(struct table *p1, struct table *p2)
{
        if (p1->uid == p2->uid)
                return(0);
        return ((p2->uid > p1->uid) ? -1 : 1);
}
