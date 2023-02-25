static char sccsid[] = "@(#)07  1.14  src/bos/usr/sbin/acct/acctcon1.c, cmdacct, bos411, 9428A410j 4/12/94 09:06:42";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: bootshut, fixup, iline, loop, nomem, prctmp, printlin,
 *            printrep, sortty, upall, update, valid, comptty, wread,
 *            setblock
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
 *	acctcon1 [-p] [-t] [-l file] [-o file] <wtmp-file >ctmp-file
 *	-p	print input only, no processing
 *	-t	test mode: use latest time found in input, rather than
 *		current time when computing times of lines still on
 *		(only way to get repeatable data from old files)
 *	-l file	causes output of line usage summary
 *	-o file	causes first/last/reboots report to be written to file
 *      reads input (normally /var/adm/wtmp), produces
 *	list of sessions, sorted by ending time in ctmp.h/ascii format
 *	A_TSIZE is max # distinct ttys
 */
#define _ILS_MACROS
#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <utmp.h>
#include <fcntl.h>
#include <locale.h>
#include "ctmp.h"
#include "table.h"
#include <stdlib.h>

#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)
#include  <langinfo.h>
#define TSSIZE 128

char *d_t_format;
int mb_cur_max;
char buf[TSSIZE];


struct  utmp	wb;	/* record structure read into */
struct	ctmp	cb;	/* record structure written out of */

struct tbuf {
	char	tline[LSZ];	/* /dev/ *.* */
	char	tname[NSZ];	/* user name */
	time_t	ttime;		/* start time */
	dev_t	tdev;		/* device */
	int	tlsess;		/* # complete sessions */
	int	tlon;		/* # times on (ut_type of 7) */
	int	tloff;		/* # times off (ut_type != 7) */
	long	ttotal;		/* total time used on this line */
	ushort  tchain;         /* used entries chain */
	ushort  thchain;        /* hash chain */
	ushort  tstatus;        /* status of `user/dead process' occurrences */

} *tbuf, t0;
unsigned short tbufstart;
struct  table tbuftable = INITTABLE(tbuf, A_TSIZE);

#define NSYS	20
int	nsys;
struct sys {
	char	sname[LSZ];	/* reasons for ACCOUNTING records */
	char	snum;		/* number of times encountered */
} sy[NSYS];

time_t	datetime;	/* old time if date changed, otherwise 0 */
time_t	firstime;
time_t	lastime;
int	ndates;		/* number of times date changed */
int	exitcode;
char	*report	= NULL;
char	*replin = NULL;
char    *prog;
int	printonly;
int	tflag;

uid_t	namtouid();
dev_t	lintodev();
unsigned iline();
void 	setblock();

main(int argc, char **argv) 
{
	register int i;

	prog = argv[0];

	setlocale(LC_ALL,"");

	catd = catopen(MF_ACCT, NL_CAT_LOCALE);

	d_t_format = nl_langinfo(D_T_FMT);

	mb_cur_max = MB_CUR_MAX;


	while (--argc > 0 && **++argv == '-')
		switch(*++*argv) {
		case 'l':
			if (--argc > 0)
				replin = *++argv;
			continue;
		case 'o':
			if (--argc > 0)
				report = *++argv;
			continue;
		case 'p':
			printonly++;
			continue;
		case 't':
			tflag++;
			continue;
		}

	setblock();
	if (printonly) {
		while (wread()) {
			if (valid()) {
				printf("%.12s\t%.8s\t%lu",
					wb.ut_line,
					wb.ut_name,
					wb.ut_time);

				strftime(buf, TSSIZE,d_t_format,localtime(&wb.ut_time));
				printf("\t%s\n", buf);
			} else
				fixup(stdout);
			
		}
		exit(exitcode);
	}
	/* Allocate initial tbuf table and clear hash section */
	if (extend(&tbuftable) == NULL) {
		nomem();
	}
	for (i = 0; i <= THASH; i++)
		tbuf[i] = t0;

	while (wread()) {
		if (firstime == 0)
			firstime = wb.ut_time;
		if (valid())
			loop();
		else
			fixup(stderr);
	}
	wb.ut_name[0] = '\0';
	strcpy(wb.ut_line, "acctcon1");
	wb.ut_type = ACCOUNTING;
	if (tflag)
		wb.ut_time = lastime;
	else
		time(&wb.ut_time);
	loop();
	if (report != NULL)
		printrep();
	if (replin != NULL)
		printlin();
	exit(exitcode);
}

wread()
{
	return( fread((void *)&wb, (size_t)sizeof(wb), (size_t)1, stdin) == 1 );
	
}

/*
 * valid: check input wtmp record, return 1 if looks OK
 */
valid()
{
	register i;

	int rc;
        wchar_t wc;

        for (i = 0; i < NSZ; i++) {
                rc = mbtowc(&wc,&wb.ut_name[i], (size_t) mb_cur_max);
                if (iswprint(wc))
                        continue;
                else if (wc == L'\0')
                        break;
                else
                        return(0);
        }

	if((wb.ut_type >= EMPTY) && (wb.ut_type <= UTMAXTYPE))
		return(1);

	return(0);
}

/*
 *	fixup assumes that V6 wtmp (16 bytes long) is mixed in with
 *	V7 records (20 bytes each)
 *
 *	Starting with Release 5.0 of UNIX, this routine will no
 *	longer reset the read pointer.  This has a snowball effect
 *	On the following records until the offset corrects itself.
 *	If a message is printed from here, it should be regarded as
 *	a bad record and not as a V6 record.
 */
fixup(stream)
register FILE *stream;
{
	fprintf(stream, MSGSTR( BADWTMP, "bad wtmp: offset %lu.\n"), ftell(stdin)-sizeof(wb));
	fprintf(stream, MSGSTR( WTMPREC, "bad record is:  %.12s\t%.8s\t%lu"),
		wb.ut_line,
		wb.ut_name,
		wb.ut_time);

	strftime(buf, TSSIZE,d_t_format,localtime(&wb.ut_time));
	fprintf( stream, "\t%s\n", buf);

#ifdef	V6
	fseek(stdin, (long)-4, 1);
#endif
	exitcode = 1;
}

loop()
{
	register unsigned ti;
	register timediff;
	register struct tbuf *tp;
	static  time_t  opentime, closetime;

	if( wb.ut_line[0] == '\0' )	/* It's an init admin process */
		return;			/* no connect accounting data here */
	switch(wb.ut_type) {
	case OLD_TIME:
		datetime = wb.ut_time;
		return;
	case NEW_TIME:
		if(datetime == 0)
			return;
		timediff = wb.ut_time - datetime;
		for (ti = tbufstart; ti; ti = tp->tchain) {
			tp = &tbuf[ti];
			tp->ttime += timediff;
		}
		datetime = 0;
		ndates++;
		return;
	case BOOT_TIME:
		upall();
	case ACCOUNTING:
	case RUN_LVL:
                /*
                 * There are two "special" records.  The first is the
                 * "openacct" record.  It is the time the wtmp file was
                 * started for this accounting period.  The second is
                 * the "runacct" record and is the time the wtmp file was
                 * logically closed by /usr/lib/acct/runacct.
                 */

                if (! strncmp (wb.ut_line, "openacct", sizeof wb.ut_line))
                        opentime = wb.ut_time;
                else if (! strncmp (wb.ut_line, "runacct", sizeof wb.ut_line))
                        closetime = wb.ut_time;

		lastime = wb.ut_time;
		bootshut();
		return;
	case LOGIN_PROCESS:
	case INIT_PROCESS:
	case EMPTY:
		return;
	case USER_PROCESS:       /* user logged on */
 	case DEAD_PROCESS:       /* first one indicates user logged off */
                /*
                 * If the current record has a timestamp before the
                 * "earliest" time for this accounting run (the time of
                 * the previous accounting run), change the time to be
                 * that time for accounting purposes.  For logout
                 * records, use the "last" time (the time of the current
                 * accounting run).
                 */
                if (opentime != 0 && wb.ut_type == USER_PROCESS &&
                                wb.ut_time < opentime)
                        wb.ut_time = opentime;

                /*
                 * acctcon1 depends on the ordering of the "runact" record
                 * and the DEAD_PROCESS records.  The runacct command must
                 * add the "runacct" record, then copy the existing UTMP
                 * file with the ut_type field set to DEAD_PROCESS.
                 */

                if (closetime != 0 && wb.ut_type == DEAD_PROCESS &&
                                wb.ut_time < closetime)
                        wb.ut_time = closetime;

		ti = iline();
		update(&tbuf[ti]);
		return;
	default:
		strftime(buf, TSSIZE,d_t_format,localtime(&wb.ut_time));
		fprintf(stderr, MSGSTR( CON1BADTYPE, "acctcon1: invalid type %d for %s %s %s"),
			wb.ut_type,
			wb.ut_name,
			wb.ut_line,
			buf);
			fprintf(stderr,"\n");
	}
}

/*
 * bootshut: record reboot (or shutdown)
 * bump count, looking up wb.ut_line in sy table
 */
bootshut()
{
	register i;

	for (i = 0; i < nsys && !EQN(wb.ut_line, sy[i].sname); i++)
		;
	if (i >= nsys) {
		if (++nsys > NSYS) {
			fprintf(stderr,
				MSGSTR( CON1TOOSMALL, "acctcon1: recompile with larger NSYS\n"));
			nsys = NSYS;
			return;
		}
		CPYN(sy[i].sname, wb.ut_line);
	}
	sy[i].snum++;
}

/*
 * iline: look up/enter current line name in tbuf, return index
 * (used to avoid system dependencies on naming)
 */
unsigned
iline()
{       register struct tbuf *tp;
	register unsigned t, th;
	static tused = THASH;

	/* Hash line name and look it up */
	for (t = th = 0; t < LSZ; t++) {
		th *= 61;
		th += wb.ut_line[t];
	}
	t = th = th%THASH + 1;
	do {
		tp = &tbuf[t];
		if (EQN(wb.ut_line, tp->tline))
			return(t);
	} while (t = tp->thchain);

	/* If already an entry in this slot (first hash), get another slot.
	 * An empty slot is one that has a null linename.
	 */
	if (tp->tline[0]) {
		if ((tused += 1) >= tbuftable.tb_nel) {
			tbuftable.tb_nel += 32;
			if (extend(&tbuftable) == NULL)
				nomem();
		}
		tp = &tbuf[t = tused];
		*tp = t0;
	} else
		t = th;
	if (t != th) {  /* If not first entry, link on at head of chain */
		tp->thchain = tbuf[th].thchain;
		tbuf[th].thchain = t;
	}
	/* Add this entry to the list of all lines */
	tp->tchain = tbufstart;
	tbufstart = t;
	CPYN(tp->tline, wb.ut_line);
	tp->tdev = lintodev(wb.ut_line);
	return(t);
}

nomem()
{       fprintf(stderr,MSGSTR( NOMEM, "%s: out of memory\n"), prog);
	exit(2);
}

upall()
{
	register unsigned ti;
	register struct tbuf *tp;

	wb.ut_type = DEAD_PROCESS;	/* fudge a logoff for reboot record */
	for (ti = tbufstart; ti; ti = tp->tchain)
		update(tp = &tbuf[ti]);
}

/*
 * update tbuf with new time, write ctmp record for end of session
 */
update(tp)
register struct tbuf *tp;
{
	long	told,	/* last time for tbuf record */
		tnew;	/* time of this record */
			/* Difference is connect time */

	told = tp->ttime;
	tnew = wb.ut_time;
	if (told > tnew) {
		strftime(buf, TSSIZE,d_t_format,localtime(&told));
		fprintf(stderr,MSGSTR( BADTIMEOLD, "%s: bad times: old: %s"), prog, buf);
		fprintf(stderr,"\n");
		strftime(buf, TSSIZE,d_t_format,localtime(&tnew));
		fprintf(stderr,MSGSTR( BADTIMENEW, "new: %s"),buf);
		fprintf(stderr,"\n");
		exitcode = 1;
		tp->ttime = tnew;
		return;
	}
	tp->ttime = tnew;
	switch(wb.ut_type) {
	case USER_PROCESS:
		tp->tlsess++;
		if(tp->tname[0] != '\0') { /* Someone logged in without */
					   /* logging off. Put out record. */
			cb.ct_tty = tp->tdev;
			CPYN(cb.ct_name, tp->tname);
			cb.ct_uid = namtouid(cb.ct_name);
			cb.ct_start = told;
			pnpsplit(cb.ct_start, tnew-told, cb.ct_con);
			prctmp(&cb);
			tp->ttotal += tnew-told;
		}
		else	/* Someone just logged in */
			tp->tlon++;
		tp->tstatus = LOGGED_ON;   /* mark as logged on */
		CPYN(tp->tname, wb.ut_name);
		break;
	case DEAD_PROCESS:
		tp->tloff++;
		if(tp->tname[0] != '\0' && tp->tstatus == LOGGED_ON) { 
		/* Someone logged off */
			/* Set up and print ctmp record */
			cb.ct_tty = tp->tdev;
			CPYN(cb.ct_name, tp->tname);
			cb.ct_uid = namtouid(cb.ct_name);
			cb.ct_start = told;
			pnpsplit(cb.ct_start, tnew-told, cb.ct_con);
			prctmp(&cb);
			tp->ttotal += tnew-told;
			tp->tname[0] = '\0';
			tp->tstatus = LOGGED_OFF;   /* mark as logged off */
		}
	}
}

printrep()
{
	register i;

	freopen(report, "w", stdout);

	strftime(buf, TSSIZE,d_t_format,localtime(&firstime));
	printf(MSGSTR( CON1FROM, "from %s"), buf);
	printf("\n");
	strftime(buf, TSSIZE,d_t_format,localtime(&lastime));
	printf(MSGSTR( CON1TO, "to   %s"), buf);
	printf("\n");
	if (ndates)
		if (ndates>1)
			printf(MSGSTR( DATECHGS, "%d\tdate changes\n"),ndates);
		else
			printf(MSGSTR( DATECHG, "%d\tdate change\n"),ndates);
	for (i = 0; i < nsys; i++)
		printf("%d\t%.12s\n", sy[i].snum, sy[i].sname);
}

/*
 *	print summary of line usage
 *	accuracy only guaranteed for wtmp file started fresh
 */
printlin()
{
	register struct tbuf *tp;
	register int ti, ntty;
	double timet, timei;
	double ttime;
	int tsess, ton, toff;

	freopen(replin, "w", stdout);
	ttime = 0.0;
	tsess = ton = toff = 0;
	timet = MINS(lastime-firstime);
	printf(MSGSTR( CON1TOT1, "TOTAL DURATION: %.0f MINUTES\n\n"), timet);
	printf(MSGSTR( CON1TOT2, "LINE\tMINUTES\tPERCENT\t# SESS\t# ON\t# OFF\n"));
	ntty = sortty();
	for (ti = 0; ti < ntty; ti++) {
		tp = &tbuf[ti];
		timei = MINS(tp->ttotal);
		ttime += timei;
		tsess += tp->tlsess;
		ton += tp->tlon;
		toff += tp->tloff;
		printf("%.8s\t%.0f\t%.0f\t%d\t%d\t%d\n",
			tp->tline,
			timei,
			(timet > 0.)? 100*timei/timet : 0.,
			tp->tlsess,
			tp->tlon,
			tp->tloff);
	}
	printf(MSGSTR( CON1TOT3, "TOTALS\t%.0f\t--\t%d\t%d\t%d\n"), ttime, tsess, ton, toff);
}

prctmp(t)
register struct ctmp *t;
{

	printf("%u\t%lu\t%.8s\t%lu\t%lu\t%lu",
		t->ct_tty,
		t->ct_uid,
		t->ct_name,
		t->ct_con[0],
		t->ct_con[1],
		t->ct_start);
	strftime(buf, TSSIZE,d_t_format,localtime(&t->ct_start));
	printf("\t%s\n", buf);
}

/* Sort tbuf entries.  Links (tchain, thchain) no longer valid afterwards. */
sortty()
{       register struct tbuf *tempty, *tp;
	register unsigned ti;
	int ntty = 0;
	int comptty();

	tempty = &tbuf[0];

	for (ti = tbufstart; ti; ti = tp->tchain) {
		tp = &tbuf[ti];
		++ntty;

		/* Find next empty slot before this one, if any */
		while (tempty < tp && tempty->tline[0])
			tempty++;

		/* If we found one, transfer this entry */
		if (tempty->tline[0] == '\0') {
			*tempty = *tp;
			tp->tline[0] = '\0';
		}
	}
	qsort(tbuf, ntty, sizeof (*tbuf), comptty);
	return(ntty);
}

comptty(t1, t2)
struct tbuf *t1, *t2;
{       register char *n1 = t1->tline;
	register char *n2 = t2->tline;

	/* No end check necessary as there are no duplicates */
	while (*n1 == *n2) ++n1, ++n2;
	return(*n1 - *n2);
}

void setblock()
{
	int nonblockf;
	int flags;

	if ((flags = fcntl(0, F_GETFL, 0)) == -1)
		perror("acctcon1: fcntl");
	nonblockf = flags | O_NDELAY;
	if (fcntl(0, F_SETFL, nonblockf) == -1)
		perror("acctcon1: fcntl2");
}
