static char sccsid[] = "@(#)07  1.13  src/bos/usr/sbin/acct/ac/ac.c, cmdacct, bos411, 9428A410j 4/12/94 09:20:29";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: loop, print, upall, update, among, newday, pdate 
 *
 * ORIGINS: 9, 26, 27
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
 * ac [ -w wtmp ] [ -d ] [ -p ] [ people ]
 */
#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <utmp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <locale.h>
#include <stdlib.h>

#include "ac_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_AC,n,s) 
#include <langinfo.h>
#define TSSIZE 128
char buf[TSSIZE];

#define NMAX sizeof(ibuf.ut_name)
#define LMAX sizeof(ibuf.ut_line)

#define TSTART 0     /* starting point for tty's.  D16738 */
#define PSTART 1000  /* starting point for pts's lines. D16738 */

/*
 * Total size of the tbuf.  Added 1 extra space for other terminal
 * devices.
 */
#define TSIZE  6242
#define	USIZE	500
struct  utmp ibuf;

struct ubuf {
	char	uname[NMAX];
	long	utime;
} ubuf[USIZE];

struct tbuf {
	struct	ubuf	*userp;
	long	ttime;
} tbuf[TSIZE];

char	*wtmp;
int	pflag, byday;
long	dtime;
long	midnight;
long	lastime;
long	day	= 86400L;
int	pcount;
char	**pptr;

main(argc, argv) 
char **argv;
{
	int fl;
	int c;
	wchar_t wc;
	register i;
	FILE *wf;
	int mb_cur_max;

	setlocale( LC_ALL, "" );

#ifdef DEBUG
      printf("Total size of tbuf is TSIZE = %d\n", i = TSIZE);
      printf("Starting point for tty### is TSTART = %d\n", i = TSTART);
      printf("Starting point for pts/### is PSTART = %d\n", i = PSTART);
#endif

	mb_cur_max = MB_CUR_MAX;

	catd = catopen(MF_AC, NL_CAT_LOCALE);
	wtmp = "/var/adm/wtmp";
	while ((c = getopt(argc, argv, "dpw:")) != EOF)
	switch(c) {
	case 'd':
		byday++;
		break;
	case 'p':
		pflag++;
		break;
	case 'w':
		wtmp = optarg;
		break;
	default:
		fprintf(stderr,MSGSTR(USAGE, "Usage: ac [ -dp ] [ -w wtmp_file ] [ user ]\n"));
		exit(1);
	}
	pcount = argc - optind;
	pptr = &argv[optind];
	if ((wf = fopen(wtmp, "r")) == NULL) {
		fprintf(stderr, MSGSTR(NOFILE, "No %s\n"), wtmp);
		exit(1);
	}
	for(;;) {
		if (fread((char *)&ibuf, sizeof(ibuf), 1, wf) != 1)
			break;

		if (mb_cur_max == 1) {
			fl = 0;
			for (i=0; i<NMAX; i++) {
				c = ibuf.ut_name[i];
				/* If printable character other than a space */
				if (isprint(c) && c != ' ') {
					/* If prior ' ' or '\0', skip */
					if (fl)
						break;
					/* no prior ' ' or '\0', next char */
					continue;
				}
				if (c==' ' || c=='\0') {
					/* Indicate that we got a ' ' or '\0' */
					fl++;
					ibuf.ut_name[i] = '\0';
				} else
					/* Non printable char */
					break;
			}
			if (i>=NMAX)
				loop();
		} else {  /* Full internationalization */
			int len, j;

			fl = 0;
			len=0;
	 		for (i=0; i<NMAX; i += len) {
				if ((len=mbtowc(&wc, &ibuf.ut_name[i], (size_t) mb_cur_max)) == -1)
					break;
				/* if wc==NULL set len to 1. */
				if((len == 0) && (wc == L'\0')) len = 1;
				if (iswprint(wc) && !iswspace(wc)) {
					if (fl)
						break;
					continue;
				}
				if (iswspace(wc) || wc==L'\0') {
					fl++;
					for (j=0; j<len; j++)
						ibuf.ut_name[i+j] = '\0';
				} else
					break;
			}
			if (i >= NMAX)
				loop();
		}
	}
	ibuf.ut_name[0] = '\0';
	ibuf.ut_line[0] = '~';
	time(&ibuf.ut_time);
	loop();
	print();
	exit(0);
}

loop()
{
	register i;
	register struct tbuf *tp;
	register struct ubuf *up;
	int icnt;

	if(ibuf.ut_line[0] == '|') {
		dtime = ibuf.ut_time;
		return;
	}
	if(ibuf.ut_line[0] == '{') {
		if(dtime == 0)
			return;
		for(tp = tbuf; tp < &tbuf[TSIZE]; tp++)
			tp->ttime += ibuf.ut_time-dtime;
		dtime = 0;
		return;
	}
	if (lastime>ibuf.ut_time || lastime+(1.5*day)<ibuf.ut_time)
		midnight = 0;
	if (midnight==0)
		newday();
	lastime = ibuf.ut_time;
	if (byday && ibuf.ut_time > midnight) {
		upall(1);
		print();
		newday();
		for (up=ubuf; up < &ubuf[USIZE]; up++)
			up->utime = 0;
	}
	if (ibuf.ut_line[0] == '~') {
		ibuf.ut_name[0] = '\0';
		upall(0);
		return;
	}
	/*  This was BSD original code
	if (ibuf.ut_line[0]=='t')
		i = (ibuf.ut_line[3]-'0')*10 + (ibuf.ut_line[4]-'0');
	else
		i = TSIZE-1;
	if (i<0 || i>=TSIZE)
		i = TSIZE-1;
	*/

	/* D16738 - commented out in order to fix for pts's
	 *
	 * Correction contributed by Phyllis Kantar @ Rand-unix
	 *
	 * Fixes long standing problem with tty names other than 00-99
	if (ibuf.ut_line[0]=='t') {
		i = (ibuf.ut_line[3]-'0');
		if(ibuf.ut_line[4])
			i = i*79 + (ibuf.ut_line[4]-'0');
	} else
		i = TSIZE-1;
	if (i<0 || i>=TSIZE) {
		i = TSIZE-1;
		fprintf(stderr, MSGSTR(BADTTY, "ac: Bad tty name: %s\n"),
			ibuf.ut_line); 
	}
          -------------- commented out for RISC/6000 machine */

	/*
	 * Space was reserved for 1000 tty devices (tty0-tty999).
	 * The value 1000 was chosen to be an optimal value.  The real
	 * limit for the number of tty lines depends on the available
	 * hardware.  Medium and large size RISC/6000 currently may have
	 * up to 64*8=512 tty's.
	 *
	 * Range:
	 *    tty0-tty999.  i = [0,999].
	 */
	if (ibuf.ut_line[0]=='t') {
		icnt = 3; i = 0;  
		while (ibuf.ut_line[icnt] != '\0') {
			if (icnt < 6)  /* limit of 3 digits, (0-999) */
				if ( (ibuf.ut_line[icnt] >= '0') && 
		  			(ibuf.ut_line[icnt] <= '9') ) {
					i = (i*10) + (ibuf.ut_line[icnt]-'0');
				}
				else {
					i = TSIZE-1;
					break;
				}
			else {
				i = TSIZE-1;
				break;
			}
			icnt++;
		}
		if (icnt == 3) i = TSIZE - 1;
		else if ( (ibuf.ut_line[icnt] == '\0') )
			i += TSTART;
#ifdef DEBUG
		printf("%s = %d\n",ibuf.ut_line,i);
#endif
	}
	/*
	 * Space was reserved for 1000 pts pseudo devices.
	 * The value 1000 was chosen to be an optimal value.  The real
	 * limit of the number of pts lines depends on the amount of
	 * paging space available.
	 *
	 * Range:
	 *    pts/0-pts/999.  i = [PSTART,PSTART+999].
	 */
	else if (ibuf.ut_line[0]=='p') {
		icnt = 4; i = 0; 
		while (ibuf.ut_line[icnt] != '\0') {
			if (icnt < 7)        /* limit of 3 digits, (0-999) */
				if ( (ibuf.ut_line[icnt] >= '0') && 
		  			(ibuf.ut_line[icnt] <= '9') ) {
					i = (i*10) + (ibuf.ut_line[icnt]-'0');
				}
				else {
					i = TSIZE-1;
					break;
				}
			else {
				i = TSIZE-1;
				break;
			}
			icnt++;
		}
		if (icnt == 4) i = TSIZE - 1;
		else if ( (ibuf.ut_line[icnt] == '\0') )
			i += PSTART;
#ifdef DEBUG
		printf("%s = %d\n",ibuf.ut_line,i);
#endif
	}
	/* 
	 * All other terminal devices goes here.
	 */
	else  {
		i = TSIZE-1;
#ifdef DEBUG
		printf("OTHER: %s = %d\n",ibuf.ut_line,i);
#endif
	}

	if (i<0 || i>=TSIZE) {
		i = TSIZE-1;
		fprintf(stderr, MSGSTR(BADTTY, "ac: Bad tty name: %s\n"),				ibuf.ut_line);
	}

	tp = &tbuf[i];
	update(tp, 0);
}

print()
{
	int i;
	long ttime, t;

	ttime = 0;
	for (i=0; i<USIZE; i++) {
		if(!among(i))
			continue;
		t = ubuf[i].utime;
		if (t>0)
			ttime += t;
		if (pflag && ubuf[i].utime > 0) {
			printf("\t%-*.*s%6.2f\n", NMAX, NMAX,
			    ubuf[i].uname, ubuf[i].utime/3600.);
		}
	}
	if (ttime > 0) {
		pdate();
		printf(MSGSTR(TOTAL, "\ttotal%9.2f\n"), ttime/3600.);
	}
}

upall(f)
{
	register struct tbuf *tp;

	for (tp=tbuf; tp < &tbuf[TSIZE]; tp++)
		update(tp, f);
}

update(tp, f)
struct tbuf *tp;
{
	int j;
	struct ubuf *up;
	long t, t1;

	if (f)
		t = midnight;
	else
		t = ibuf.ut_time;
	if (tp->userp) {
		t1 = t - tp->ttime;
		if (t1 > 0)
			tp->userp->utime += t1;
	}
	tp->ttime = t;
	if (f)
		return;
	if (ibuf.ut_name[0]=='\0') {
		tp->userp = 0;
		return;
	}
	for (up=ubuf; up < &ubuf[USIZE]; up++) {
		if (up->uname[0] == '\0')
			break;
		for (j=0; j<NMAX && up->uname[j]==ibuf.ut_name[j]; j++);
		if (j>=NMAX)
			break;
	}
	if(ibuf.ut_type == USER_PROCESS) {
		for (j=0; j<NMAX; j++)
			up->uname[j] = ibuf.ut_name[j];
		tp->userp = up;
	}
}

among(i)
{
	register j, k;
	register char *p;

	if (pcount==0)
		return(1);
	for (j=0; j<pcount; j++) {
		p = pptr[j];
		for (k=0; k<NMAX; k++) {
			if (*p == ubuf[i].uname[k]) {
				if (*p++ == '\0' || k == NMAX-1)
					return(1);
			} else
				break;
		}
	}
	return(0);
}

newday()
{
	long ttime;
	struct timeb tb;
	struct tm *localtime();

	time(&ttime);
	if (midnight == 0) {
		ftime(&tb);
		midnight = 60*(long)tb.timezone;
		if (localtime(&ttime)->tm_isdst)
			midnight -= 3600;
	}
	while (midnight <= ibuf.ut_time)
		midnight += day;
}

pdate()
{
	long x;

	if (byday==0)
		return;
	x = midnight-1;

	strftime(buf, TSSIZE, "%sD", localtime(&x));
	printf("%s", buf);
}
