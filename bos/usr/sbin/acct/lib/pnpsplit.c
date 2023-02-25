static char sccsid[] = "@(#)80  1.7  src/bos/usr/sbin/acct/lib/pnpsplit.c, cmdacct, bos411, 9428A410j 2/7/94 13:02:40";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: pnpsplit, checkhol, ssh, inithol, sort
 *
 * ORIGINS: 3, 9, 27
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
 * pnpsplit splits interval into prime & nonprime portions
 * ONLY ROUTINE THAT KNOWS ABOUT HOLIDAYS AND DEFN OF PRIME/NONPRIME
 */
#include "acctdef.h"
#include <stdio.h>
#include <time.h>

#include	"acct_msg.h"
nl_catd catd;
#define MSGSTR(N,X)	catgets(catd,MS_ACCT,N,X)

#define	NHOLIDAYS	40	/* max number of company holidays per year */

/* validate that hours and minutes of prime/non-prime read in
 * from holidays file fall within proper boundaries.
 * Time is expected in the form and range of 0000-2359.
 */
#define	okay(time)	((time/100>=0) && (time/100<=24) \
				 && (time%100>=0) && (time%100<60))

static int	thisyear = 1970;	/* this is changed by holidays file */
static int	holidays[NHOLIDAYS];	/* holidays file day-of-year table */

static char	holfile[] = { "/etc/acct/holidays\0" };

/*
 *	prime(0) and nonprime(1) times during a day
 *	for BTL, prime time is 9AM to 5PM
 */
static struct hours {
	int	h_sec;		/* normally always zero */
	int	h_min;		/* initialized from holidays file (time%100) */
	int	h_hour;		/* initialized from holidays file (time/100) */
	int	h_type;		/* prime/nonprime of previous period */
} h[4];
int	daysend[] = {60, 59, 23};	/* the sec, min, hr of the day's end */

struct tm *localtime();
long	tmsecs();

/*
 * split interval of length etime, starting at start into prime/nonprime
 * values, return as result
 * input values in seconds
 */
pnpsplit(start, etime, result)
long start, etime, result[2];
{
	struct tm cur, end;
	long tcur, tend;
	long tmp;
	register sameday;
	register struct hours *hp;

	if (thisyear)	/* once holidays file is read, this is zero */
		checkhol();

	tcur = start;
	tend = start+etime;
	end=*(localtime(&tend));
	result[P] = 0;
	result[NP] = 0;

	while (tcur < tend) {	/* one iteration per day or part thereof */
		cur=*(localtime(&tcur));
		sameday = cur.tm_yday == end.tm_yday;
		if (ssh(&cur)) {	/* ssh:only NP */
			if (sameday) {
				result[NP] += tend-tcur;
				break;
			} else {
				tmp = tmsecs(&cur, daysend);
				result[NP] += tmp;
				tcur += tmp;
			}
		} else {	/* working day, P or NP */
			for (hp = h; tmless(hp, &cur); hp++);
			for (; hp->h_sec >= 0; hp++) {
				if (sameday && tmless(&end, hp)) {
					/* PTM P9849 @rec@ */
					result[hp->h_type] += tend-tcur;
					tcur = tend;
					break;	/* all done */
				} else {	/* time to next P/NP change */
					tmp = tmsecs(&cur, hp);
					result[hp->h_type] += tmp;
					tcur += tmp;
					cur.tm_sec = hp->h_sec;
					cur.tm_min = hp->h_min;
					cur.tm_hour = hp->h_hour;
				}
			}
		}
	}
}

/*
 *	Starting day after Christmas, complain if holidays not yet updated.
 *	This code is only executed once per program invocation.
 */
checkhol()
{
	register struct tm *tp;
	long t;

	if(inithol() == 0) {
		fprintf(stderr, MSGSTR( PNPHOLTBLERR, "pnpsplit: holidays table setup failed\n"));
		thisyear = 0;
		holidays[0] = -1;
		return;
	}
	time(&t);
	tp = localtime(&t);
	tp->tm_year += 1900;
	if ((tp->tm_year == thisyear && tp->tm_yday > 359)
		|| tp->tm_year > thisyear)
		fprintf(stderr,
			MSGSTR( UPDTHOL, "***UPDATE %s WITH NEW HOLIDAYS***\n"), holfile);
	thisyear = 0;	/* checkhol() will not be called again */
}

/*
 * ssh returns 1 if Sat, Sun, or Holiday
 */
ssh(ltp)
register struct tm *ltp;
{
	register i;

	if (ltp->tm_wday == 0 || ltp->tm_wday == 6)
		return(1);
	for (i = 0; holidays[i] >= 0; i++) {
		if (ltp->tm_yday > holidays[i])
			continue;
		else if (ltp->tm_yday == holidays[i])
			return(1);
		else if (ltp->tm_yday < holidays[i])
			break;	/* holidays is sorted */
	}
	return(0);
}

/*
 * inithol - read from an ascii file and initialize the "thisyear"
 * variable, the times that prime and non-prime start, and the
 * holidays array.
 */
inithol()
{
	FILE		*fopen(), *holptr;
	char		*fgets(), holbuf[128];
	register int	line = 0,
			holindx = 0,
			errflag = 0;
	void		sort();
	int		pstart, npstart;
	int		doy;	/* day of the year */

	if((holptr=fopen(holfile, "r")) == NULL) {
		perror(holfile);
		fclose(holptr);
		return(0);
	}
	while(fgets(holbuf, sizeof(holbuf), holptr) != NULL) {
		if(holbuf[0] == '*')	/* Skip over comments */
			continue;
		else if(++line == 1) {	/* format: year p-start np-start */
			if(sscanf(holbuf, "%4d %4d %4d",
				&thisyear, &pstart, &npstart) != 3) {
				fprintf(stderr,
					MSGSTR( BADYEARCONV, "%s: bad {yr ptime nptime} conversion\n"),
					holfile);
				errflag++;
				break;
			}

			/* validate year */
			if(thisyear < 1970 || thisyear > 2000) {
				fprintf(stderr, MSGSTR( BADYEAR, "pnpsplit: invalid year: %d\n"),
					thisyear);
				errflag++;
				break;
			}

			/* validate prime/nonprime hours */
			if((! okay(pstart)) || (! okay(npstart))) {
				fprintf(stderr,
					MSGSTR( PNPINVAL, "pnpsplit: invalid p/np hours\n"));
				errflag++;
				break;
			}

			/* Set up start of prime time; 2400 == 0000 */
			h[0].h_sec = 0;
			h[0].h_min = pstart%100;
			h[0].h_hour = (pstart/100==24) ? 0 : pstart/100;
			h[0].h_type = NP;

			/* Set up start of non-prime time; 2400 == 0000 */
			h[1].h_sec = 0;
			h[1].h_min = npstart%100;
			h[1].h_hour = (npstart/100==24) ? 0 : npstart/100;
			h[1].h_type = P;

			/* This is the end of the day */
			h[2].h_sec = 60;
			h[2].h_min = 59;
			h[2].h_hour = 23;
			h[2].h_type = NP;

			/* The end of the array */
			h[3].h_sec = -1;

			continue;
		}
		else if(holindx >= NHOLIDAYS) {
			fprintf(stderr, MSGSTR( PNPHOL1, "pnpsplit: too many holidays, "));
			fprintf(stderr, MSGSTR( PNPHOL2, "recompile with larger NHOLIDAYS\n"));
			errflag++;
			break;
		}

		/* Fill up holidays array from holidays file */
		sscanf(holbuf, "%d	%*s %*s	%*[^\n]\n", &doy);
		if(doy < 1 || doy > 366) {
			fprintf(stderr,
				MSGSTR( PNPBADAY, "pnpsplit: invalid day of year %d\n"), doy);
			errflag++;
			break;
		}
		holidays[holindx++] = (doy - 1);
	}
	fclose(holptr);
	if(!errflag && holindx < NHOLIDAYS) {
		sort(holidays, holindx - 1);
		holidays[holindx] = -1;
		return(1);
	}
	else
		return(0);
}

/*
 * sort - a version of the bubblesort algorithm from BAASE, Alg 2.1
 *
 * sorts the holidays array into nondecreasing order
 */
void
sort(array, nitems)
int	*array;		/* a pointer to the holidays array */
int	nitems;		/* the number of elements in the array */
{
	register int	index,	/* index going though holidays array */
			flag,	/* flag > 0 if more sorting is needed */
			k;

	flag = nitems;
	while(flag > 0) {
		k = flag - 1;
		flag = 0;
		for(index=0; index <= k; ++index) {
			if(array[index] > array[index+1]) {
				flag = array[index];	/* briefly use "flag"*/
				array[index] = array[index+1];
				array[index+1] = flag;
				flag = index;
			}
		}
	}

}
