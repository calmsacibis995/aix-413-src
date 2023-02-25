static char sccsid[] = "@(#)42  1.9  src/bos/usr/sbin/acct/wtmpfix.c, cmdacct, bos411, 9428A410j 4/12/94 09:21:06";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: adjust, err, intr, invalid, mkdtab, setdtab,
 *            winp, wout
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * wtmpfix - adjust wtmp file and remove date changes.
 *
 *	wtmpfix <wtmp1 >wtmp2
 *
 */
#define _ILS_MACROS
# include <stdio.h>
# include <ctype.h>
# include <sys/types.h>
# include <utmp.h>
# include "acctdef.h"
# include <signal.h>
# include <locale.h>
# include <langinfo.h>
#include <stdlib.h>

#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

#define TSSIZE 128

char *format;
char buf[TSSIZE];
int mb_cur_max;

FILE	*Wtmp, *Opw;

char	*Ofile	={ "/tmp/wXXXXXX" };

struct	dtab
{
	long	d_off1;		/* file offset start */
	long	d_off2;		/* file offset stop */
	long	d_adj;		/* time adjustment */
	struct dtab *d_ndp;	/* next record */
};

struct	dtab	*Fdp;		/* list header */
struct	dtab	*Ldp;		/* list trailer */


struct	utmp	wrec, wrec2;


intr()
{

	signal(SIGINT,SIG_IGN);
	unlink(Ofile);
	exit(1);
}

main(int argc, char **argv)
{

	static long	recno = 0;
	register struct dtab *dp;

	setlocale(LC_ALL,"");
	format=nl_langinfo(D_T_FMT);
	mb_cur_max=MB_CUR_MAX;

	if(argc < 2){
		argv[argc] = "-";
		argc++;
	}

#ifndef lint
	if(signal(SIGINT,intr) == SIG_ERR) {
		perror("signal");
		exit(1);
	}
#endif

	catd = catopen(MF_ACCT, NL_CAT_LOCALE);

	mktemp(Ofile);
	if((Opw=fopen(Ofile,"w"))==NULL)
		err( MSGSTR( CANTMKTMP, "cannot make temporary: %s"), Ofile);

	while(--argc > 0){
		argv++;
		if(strcmp(*argv,"-")==0)
			Wtmp = stdin;
		else if((Wtmp = fopen(*argv,"r"))==NULL)
			err(MSGSTR( CANTOPEN, "%s: Cannot open: %s"), "wtmpfix", *argv);
		while(winp(Wtmp,&wrec)){
			if(recno == 0 || wrec.ut_type==BOOT_TIME){
				mkdtab(recno,&wrec);
			}
			if(invalid(wrec.ut_name)) {
				fprintf(stderr,
					MSGSTR( LOGCHG, "wtmpfix: logname \"%8.8s\" changed to \"INVALID\"\n"),
					wrec.ut_name);
				strncpy(wrec.ut_name, "INVALID", NSZ);
			}
			if(wrec.ut_type==OLD_TIME){
				if(!winp(Wtmp,&wrec2))
					err(MSGSTR( INTRUC, "Input truncated at offset %ld"),recno);
				if(wrec2.ut_type!=NEW_TIME)
					err(MSGSTR( NEWDTEXPTED, "New date expected at offset %ld"),recno);
				setdtab(recno,&wrec,&wrec2);
				recno += (2 * sizeof(struct utmp));
				wout(Opw,&wrec);
				wout(Opw,&wrec2);
				continue;
			}
			wout(Opw,&wrec);
			recno += sizeof(struct utmp);
		}
		if(Wtmp!=stdin)
			fclose(Wtmp);
	}
	fclose(Opw);
	if((Opw=fopen(Ofile,"r"))==NULL)
		err(MSGSTR( CATREADTMP, "Cannot read from temp: %s"), Ofile);
	recno = 0;
	while(winp(Opw,&wrec)){
		adjust(recno,&wrec);
		recno += sizeof(struct utmp);
/*		if(wrec.ut_type==OLD_TIME || wrec.ut_type==NEW_TIME)
			continue;	*/
		wout(stdout,&wrec);
	}
	fclose(Opw);
	unlink(Ofile);
	exit(0);
}

/*	err() writes an error message to the standard error and then
 *	calls the interrupt routine to clean up the temporary file
 *	and exit.  The variable "f" is the format specification that
 *	can contain up to 3 arguments (m1, m2, m3).
 */
err(f,m1,m2,m3)
char *f;
{

	fprintf(stderr,f,m1,m2,m3);
	fprintf(stderr,"\n");
	intr();
}

/*	winp() reads a record from a utmp.h-type file pointed to
 *	by the stream pointer "f" into the structure whose address
 *	is given by the variable "w".
 *	This reading takes place in two stages: first the raw
 *      records from the utmp.h files are read in (usually /var/adm/wtmp)
 *	and written into the temporary file (Opw); then the records
 *	are read from the temporary file and placed on the standard 
 *	output.
 */
winp(f,w)
register FILE *f;
register struct utmp *w;
{

	if(fread(w,sizeof(struct utmp),1,f)!=1)
		return 0;
	if((w->ut_type >= EMPTY) && (w->ut_type <= UTMAXTYPE))
		return ((unsigned)w);
	else {
		strftime(buf, TSSIZE, localtime(&w->ut_time));
		fprintf(stderr,MSGSTR( BADFILOFF, "Bad file at offset %ld\n"),
			ftell(f)-sizeof(struct utmp));
		fprintf(stderr,"%-12s %-8s %lu %s",
			w->ut_line,w->ut_user,w->ut_time,buf);

		intr();
	}
}

/*	wout() writes an output record of type utmp.h.  The
 *	variable "f" is a file descripter of either the temp
 *	file or the standard output.  The variable "w" is an
 *	address of the entry in the utmp.h structure.
 */
wout(f,w)
register FILE *f;
register struct utmp *w;
{

	fwrite(w,sizeof(struct utmp),1,f);
}

mkdtab(p,w)
long	p;
register struct utmp *w;
{

	register struct dtab *dp;

	dp = Ldp;
	if(dp == NULL){
		dp = (struct dtab *)calloc(sizeof(struct dtab),1);
		if(dp == NULL)
			err(MSGSTR( NOCORE, "out of core"));
		Fdp = Ldp = dp;
	}
	dp->d_off1 = p;
}

setdtab(p,w1,w2)
long	p;
register struct utmp *w1, *w2;
{

	register struct dtab *dp;

	if((dp=Ldp)==NULL)
		err("no dtab");
	dp->d_off2 = p;
	dp->d_adj = w2->ut_time - w1->ut_time;
	if((Ldp=(struct dtab *)calloc(sizeof(struct dtab),1))==NULL)
		err(MSGSTR( NOCORE, "out of core"));
	Ldp->d_off1 = dp->d_off1;
	dp->d_ndp = Ldp;
}

adjust(p,w)
long	p;
register struct utmp *w;
{

	long pp;
	register struct dtab *dp;

	pp = p;

	for(dp=Fdp;dp!=NULL;dp=dp->d_ndp){
		if(dp->d_adj==0)
			continue;
		if(pp>=dp->d_off1 && pp < dp->d_off2)
			w->ut_time += dp->d_adj;
	}
}

/*
 *	invalid() determines whether the name field adheres to
 *	the criteria set forth in acctcon1.  If the name violates
 *	conventions, it returns a truth value meaning the name is
 *	invalid; if the name is okay, it returns false indicating
 *	the name is not invalid.
 */
#define	VALID	0
#define	INVALID	1

invalid(name)
char	*name;
{
	register int	i;
	int rc;
        wchar_t wc;

	for(i=0; i<NSZ; i++) {
                rc=mbtowc(&wc, &name[i], (size_t) mb_cur_max);
                if(wc == L'\0')
                        return(VALID);
                if( ! (iswprint(wc) || (wc == L'$')
                        || (wc == L' ') )) {
                        return(INVALID);
		}
	}
	return(VALID);
}
