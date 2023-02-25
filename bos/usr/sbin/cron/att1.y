/* @(#)22	1.18  src/bos/usr/sbin/cron/att1.y, cmdcntl, bos41B, 9504A 1/4/95 10:13:27
 *
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS: torder(), dayfirst(), yylex(), lookahd(), word(), getnls(),
 *		strtolower()
 *
 * ORIGINS: 3, 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * @OSF_COPYRIGHT@
static char rcsid[] = "RCSfile: att1.y,v  Revision: 1.5  (OSF) Date: 90/10/07 16:12:47 ";
*/

%{
static char sccsid[] = "@(#)22	1.18  src/bos/usr/sbin/cron/att1.y, cmdcntl, bos41B, 9504A 1/4/95 10:13:27";
#include <stdio.h>
#include <langinfo.h>
#include <locale.h>
#include <time.h>
#include <sys/dir.h>
#include "att1.h"

#include <ctype.h>

extern	int	gmtflag;
static int timedon = 0;
static int daydon = 0;
static int yeartm = 0;
static	int num = 0;
extern	int	mday[];
extern	struct	tm *tp, at, rt;

#include "cron_msg.h"
extern nl_catd catd;
#define	MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
#define STRSIZE	256
%}
%token	NOW
%token	NOON
%token	N
%token	MIDNIGHT
%token	M
%token	MINUTE
%token	HOUR
%token	DAY
%token	DAYW
%token	TODAY
%token	TOMORROW
%token	WEEK
%token	MONTH
%token	YEAR
%token	INC_PERIOD
%token	SUFF
%token	TIMEZONE
%token	AM
%token	A
%token	PM
%token	P
%token	ZULU
%token	NEXT
%token	NUMB
%token	COLON
%token	COMMA
%token	PLUS
%token	DAYN
%token	YEARN
%token	UNKNOWN
%%

/* date and incr are actually optional.  This is handled below. */

args	: time date incr		{ validate_time(); }
	| time date incr UNKNOWN	{ yyerror(); }
	| nowspec			{ validate_time(); }
	| nowspec UNKNOWN		{ yyerror(); }
	;

nowspec	: NOW {
		timedon = 1;   
		memcpy(&at, tp, sizeof(struct tm));
	} incr
	;

time	: hour opt_suff opt_timezone {
		timedon = 1;   
		check_suff($1, $2);
	}
	| hour COLON minute opt_suff opt_timezone {
		torder(&$1, &$3);
		timedon = 1;   
		at.tm_min = $3;
		check_suff($1, $4);
	}
	| hour minute opt_suff opt_timezone {
		torder(&$1, &$2);
		timedon = 1;   
		at.tm_min = $2;
		check_suff($1, $3);
	}
	| NOON {
		timedon = 1;   
		at.tm_hour = 12;
	}
	| MIDNIGHT {
		timedon = 1;   
		at.tm_hour = 0;
	}
	;

date	: /*empty*/ {
		at.tm_mday = tp->tm_mday;
		at.tm_mon = tp->tm_mon;
		at.tm_year = tp->tm_year;
		if ((at.tm_hour < tp->tm_hour)
			|| ((at.tm_hour==tp->tm_hour)&&(at.tm_min<tp->tm_min)))
			rt.tm_mday++;
	}
	| dayn month {
		at.tm_mon = $2;
		at.tm_mday = $1;
		at.tm_year = tp->tm_year;
		if (at.tm_mon < tp->tm_mon)
			at.tm_year++;
	}
	| month dayn {
		at.tm_mon = $1;
		at.tm_mday = $2;
		at.tm_year = tp->tm_year;
		if (at.tm_mon < tp->tm_mon)
			at.tm_year++;
	}
	| dayn month COMMA year {
		at.tm_mon = $2;
		at.tm_mday = $1;
		at.tm_year = $4;
	}
	| month dayn COMMA year {
		at.tm_mon = $1;
		at.tm_mday = $2;
		at.tm_year = $4;
	}
	| DAYW {
		at.tm_mon = tp->tm_mon;
		at.tm_mday = tp->tm_mday;
		at.tm_year = tp->tm_year;
		rt.tm_mday = $1 - tp->tm_wday;
		if (rt.tm_mday < 0)
			rt.tm_mday += 7;
	}
	| TODAY {
		at.tm_mon = tp->tm_mon;
		at.tm_mday = tp->tm_mday;
		at.tm_year = tp->tm_year;
	}
	| TOMORROW {
		at.tm_mon = tp->tm_mon;
		at.tm_mday = tp->tm_mday;
		at.tm_year = tp->tm_year;
		rt.tm_mday += 1;
	}
	;

incr	: /* empty */
	| PLUS inc_number INC_PERIOD	{ add_incr($2, $3); }
	| NEXT INC_PERIOD		{ add_incr(1, $2); }
	;

dayn	: DAYN		{ daydon = 1; $$ = $1; }
	;

year	: number	{ $$ = $1; }
	;

hour	: NUMB		{ $$ = $1; }
	;

minute	: NUMB		{ timedon = 1; $$ = $1; }
	;

month	: MONTH	        { timedon = 2;  $$ = $1; }
	;

inc_number	: /* empty */	{ $$ = 1; }
		| number	{ $$ = $1; }
		;

number		: NUMB		{ $$ = $1; }
		| number NUMB	{ $$ = ((num==2) ? 100 * $1 + $2:
						    10 * $1 + $2); }
		;

opt_suff	: /* empty */	{ $$ = 0; }
		| SUFF		{ $$ = $1; }
		;

opt_timezone	: /* empty */
		| TIMEZONE {
			sprintf(tzenv, "TZ=%s", tzbuf);
			putenv(tzenv);
		}
		;

%%
static torder(h, m)
int *h, *m;
{
	char timeord[20];
	int i, tmp;
	char *s;

	if ((s = nl_langinfo(T_FMT)) != NULL) {
		strcpy(timeord, s);
		for (i=0; timeord[i] != '\0'; i++) {
			if (timeord[i] == 'h' || timeord[i] == 'H' ||
				timeord[i] == 'I' ||
				(timeord[i] == 's' && timeord[i+1] == 'H'))
				break;
			if (timeord[i] == 'm' || timeord[i] == 'M') {
				tmp = *m;
				*m = *h;
				*h = tmp;
				break;
			}
		}
	}
}

static int dayfirst()
{
	char dateord[20];
	int i;
	char *s;

	if ((s = nl_langinfo(D_FMT)) != NULL) {
		strcpy(dateord, s);
		for (i=0; dateord[i] != '\0'; i++) {
                        if (dateord[i] == 'm' || dateord[i] == 'B' || 
				dateord[i] == 'b' || dateord[i] == 'h' || 
 				(dateord[i] == 'l' && dateord[i+1] == 'h') || 
				dateord[i] == 'M') {
				return(0);
			}
			if (dateord[i] == 'd') {
				return(1);
			}
		}
	}
	return(0);
}

#define	LL(t, v)	return(yylval = v, t)
#define EQ(s1, s2)	(strcmp(s1, s2) == 0)
#define EQN(s1, s2, n)	(strncmp(s1, s2, n) == 0)

#undef getc
#define	getc()		(*argp ? *argp++ : EOF)

char	*argp = "";

static char timesep;

static char mon[12][8] = {
			"jan",
		   	"feb",
			"mar",
			"apr",
			"may",
			"jun",
			"jul",
			"aug",
			"sep",
			"oct",
			"nov",
			"dec"
			};
static char lgmon[12][26] = {
			"january",
			"february",
			"march",
			"april",
			"may",
			"june",
			"july",
			"august",
			"september",
			"october",
			"november",
			"december"
			};
static char days[7][8] = {
			"sun",
			"mon",
			"tue",
			"wed",
			"thu",
			"fri",
			"sat"
			};
static char lgdays[7][26]  =  {
			"sunday",
			"monday",
			"tuesday",
			"wednesday",
			"thursday",
			"friday",
			"saturday"
			};
static char ampm[2][26]  =  {
			"a.m.",
			"p.m."
			};
static char rest[30][26] = {
			"am",
			"pm",
			"zulu",
			"now",
			"yesterday",
			"tomorrow",
			"noon",
			"midnight",
			"next",
			"weekdays",
			"weekend",
			"today",
			"a",
			"p",
			"n",
			"m",
			"minute",
			"minutes",
			"hour",
			"hours",
			"day",
			"days",
			"week",
			"weeks",
			"month",
			"months",
			"year",
			"years",
			"min",
			"mins"
			};

static char nl_mon[12][STRSIZE];
static char nl_lgmon[12][STRSIZE];
static char nl_days[7][STRSIZE];
static char nl_lgdays[7][STRSIZE];
static char nl_ampm[2][STRSIZE];
static char tzbuf[STRSIZE];
/*
 * Holder for the TZ env variable.  This is set from tzbuf, when it is known
 * that tzbuf contains a timezone name.  This variable becomes a part of
 * the environment when putenv() is called.
 */
static char tzenv[STRSIZE+4];

yylex()
{
	int c, j=0;
	char buf[STRSIZE];
	int val;
	
	while ((c = getc()) != EOF) {
		switch (c) {
		case '\t':
		case '\n':
		case ' ':
			break;
		case ':':
                        if (timesep == ':')
                                { return(COLON); }
                        break;
		case '.':
                        if (timesep == '.')
                                { return(COLON); }
                        break;
		case ',':
			yeartm++;
			{ return(COMMA); }
		case '+':
			{ return(PLUS); }
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (lookahd() == 2) {  /* a number */
				val = (c - '0') * 10 + (getc() - '0'); 
				num = 2;
			}
			else {
				val = c - '0';
				num = 1;
			}
			if (lookahd() == 1) {
				if ( num == 2 )
					argp -= 2;
				else
					argp -= 1;
				c = getc();
				buf[j++] = c;
				while (lookahd() != 0)
					buf[j++] = getc();
				buf[j] = '\0';
				j = 0;
				return(word(buf));
			}
			if (timedon == 2 && daydon != 1) {
				/*
				 * The order, month followed by day, should
				 * be passed in all locale environment.
				 */
				return(yylval = val,DAYN);
			}
			else {
				if (timedon == 1 && daydon != 1 && dayfirst()){
				  char *argpsav;
			
				  argpsav = argp;
				  if (yylex() == MONTH)  {
				    argp = argpsav;
				    yylval = val;
				    return(DAYN);
				  }
				  else {
				    argp = argpsav;
				    yylval = val;
				    return(NUMB);
				  }
				}
				return(yylval=val,NUMB);
			}

		default:
			buf[j++] = c;
			while (lookahd() != 0)
				buf[j++] = getc();
			buf[j] = '\0';
			j = 0;
			return(word(buf));
		}
	}
	return(0); /* Logical EOF */
}

static lookahd()
{
	switch (*argp) {
	case '\t':
	case '\n':
	case ' ':
	case ':':
	case '.':
	case ',':
	case '+':
		return(0);
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return(2);
	default:
		return(1);
	}
}

static word(bufin)
char *bufin;
{
	int i;
	char buf[STRSIZE];

	strtolower(buf, bufin);
	for (i = 0; i < 12; i++) {
		if (EQ(mon[i], buf) || EQ(lgmon[i], buf) ||
			EQ(nl_mon[i], buf) || EQ(nl_lgmon[i], buf)) {
			if (timedon != 2)
				LL(MONTH,i);
			else
				LL(NUMB,i);
		}
	}

	for (i = 0; i < 2; i++) {
		if (EQ(ampm[i], buf) || EQ(nl_ampm[i], buf))
			if (i == 0)
				LL(SUFF, AM);
			else
				LL(SUFF, PM);
	}

	for (i = 0; i < 30; i++) {
		if (EQ(rest[i], buf))
			break;
	}

	switch (i) {
	case 11:
		/*  "today"  */
		return(TODAY);
	case 5:
		/*  "tomorrow"  */
		return(TOMORROW);
	case 14:
		/* "n"  */
  		return(NOON);
	case 6:
		/* "noon"  */
  		return(NOON);
	case 15:
		/*   "m"  */
		return(MIDNIGHT);
	case 7:
		/*   "midnight"  */
		return(MIDNIGHT);
	case 3:
		/*  "now" */
		return(NOW);
	case 0:  /*  "am"  */
	case 12: /*  "a"   */
		LL(SUFF, AM);
	case 1:  /*  "pm"  */
	case 13: /*  "p"   */
		LL(SUFF, PM);
	case 2:
		/*  "zulu"   */
		LL(SUFF, ZULU);
	case 8:
		/*  "next"   */
		return(NEXT);
	case 16:
		/*  "minute"	*/
		LL(INC_PERIOD, MINUTE);
	case 17:
		/*  "minutes"	*/
		LL(INC_PERIOD, MINUTE);
	case 28:
		/*  "min"	*/
		LL(INC_PERIOD, MINUTE);
	case 29:
		/*  "mins"	*/
		LL(INC_PERIOD, MINUTE);
	case 18:
		/*   "hour"   */
		LL(INC_PERIOD, HOUR);
	case 19:
		/*   "hours"   */
		LL(INC_PERIOD, HOUR);
	case 20:
		/*   "day"   */
		LL(INC_PERIOD, DAY);
	case 21:
		/*   "days"   */
		LL(INC_PERIOD, DAY);
	case 22:
		/*   "week"   */
		LL(INC_PERIOD, WEEK);
	case 23:
		/*   "weeks"   */
		LL(INC_PERIOD, WEEK);
	case 24:
		/*   "month"   */
		LL(INC_PERIOD, MONTH);
	case 25:
		/*   "months"   */
		LL(INC_PERIOD, MONTH);
	case 26:
		/*   "year"   */
		LL(INC_PERIOD, YEAR);
	case 27:
		/*   "years"   */
		LL(INC_PERIOD, YEAR);
	default:
		for (i = 0; i < 7; i++)
		    if (EQN(days[i],buf,strlen(buf)) ||
		   	EQN(lgdays[i],buf,strlen(buf) <= 2 ? 2:strlen(buf)) ||
			EQN(nl_days[i],buf,strlen(buf)) ||
			EQN(nl_lgdays[i],buf,strlen(buf) <= 2 ? 2:strlen(buf)))
				LL(DAYW,i);
		}

	/*
	 * This may not be a timezone, but the grammar will
	 * catch that case.
	 */
	strcpy(tzbuf, bufin);
	return(TIMEZONE);
}

getnls()
{
	char *s;
	int	i;
	static nl_item	abmon_item[12] = { ABMON_1, ABMON_2, ABMON_3, ABMON_4,
			ABMON_5, ABMON_6, ABMON_7, ABMON_8, ABMON_9, ABMON_10,
			ABMON_11, ABMON_12 };
	static nl_item	mon_item[12] = { MON_1, MON_2, MON_3, MON_4, MON_5,
			MON_6, MON_7, MON_8, MON_9, MON_10, MON_11, MON_12 };
	static nl_item	abday_item[7] = { ABDAY_1, ABDAY_2, ABDAY_3, ABDAY_4,
			ABDAY_5, ABDAY_6, ABDAY_7 };
	static nl_item	day_item[7] = { DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
			DAY_6, DAY_7 };
	static nl_item	ampm_item[2] = { AM_STR, PM_STR };

	for (i=0; i < 12; i++) {
		s = nl_langinfo(abmon_item[i]);
		if (*s != '\0')
			strtolower(nl_mon[i], s);
		else
			nl_mon[i][0] = '\0';
		s = nl_langinfo(mon_item[i]);
		if (*s != '\0')
			strtolower(nl_lgmon[i], s);
		else
			nl_lgmon[i][0] = '\0';
	}

	for (i=0; i < 7; i++) {
		s = nl_langinfo(abday_item[i]);
		if (*s != '\0')
			strtolower(nl_days[i], s);
		else
			nl_days[i][0] = '\0';
		s = nl_langinfo(day_item[i]);
		if (*s != '\0')
			strtolower(nl_lgdays[i], s);
		else
			nl_lgdays[i][0] = '\0';
	}

	for (i=0; i < 2; i++) {
		s = nl_langinfo(ampm_item[i]);
		if (*s != '\0')
			strtolower(nl_ampm[i], s);
		else
			nl_ampm[i][0] = '\0';
	}

        if ((s = nl_langinfo(T_FMT)) != NULL && strcmp(s,"")) {
                while (*s != '\0' && (isalnum(*s) || *s == '%' ))
                        s++;
                if (*s == '\0')
                        timesep = ':';
                else
                        timesep = *s;

	}
}

/*
 * NAME: strtolower()
 *
 * FUNCTION: Convert upper case to lower case.
 *
 * RETURNS: N/A
 */
strtolower(s1, s2)
char *s1, *s2;
{
	int	rec;
	wchar_t nlc, nlcl;

	while (*s2 != '\0') {
		if ((rec = mbtowc(&nlc, s2, strlen(s2))) == -1)
			break;
		s2 += rec;
		nlcl = towlower(nlc);
		if ((rec = wctomb(s1, nlcl)) == -1)
			break;
		s1 += rec;
	}
	if (rec == -1)
		strcpy(s1, s2);
	else
		*s1 = '\0';
}

static validate_time()
{
	if (at.tm_min >= 60 || at.tm_hour >= 24)
		atabort(MSGSTR(MS_BTIME,"bad time\n"));
	if (at.tm_year < 38)
		at.tm_year += 100;
	if (at.tm_year >= 1900)
		at.tm_year -= 1900;
	if (at.tm_year < 70 || at.tm_year >= 138)
		/* need valid year 			*/
		/* The valid year is 1970 - 2037.	*/
		atabort(MSGSTR(MS_BYEAR,"bad year\n"));
	if (leap(at.tm_year))                        
		/* to check for leap year */
		mday[1]=29;
	if (at.tm_mon >= 12 || at.tm_mday > mday[at.tm_mon])  
		/* to check for day */
		atabort(MSGSTR(MS_BDATE,"bad date\n"));
	if ( *argp != NULL )
		 atabort(MSGSTR(MS_BADDATE,"bad date specification\n"));
}

static check_suff(int hour, int suff)
{
	at.tm_hour = hour;
	switch (suff) {
		case P:
		case PM:
			if (at.tm_hour < 1 || at.tm_hour > 12)
				atabort(MSGSTR(MS_BHOUR,"bad hour\n"));
			at.tm_hour %= 12;
			at.tm_hour += 12;
			break;
		case A:
		case AM:
			if (at.tm_hour < 1 || at.tm_hour > 12)
				atabort(MSGSTR(MS_BHOUR,"bad hour\n"));
			at.tm_hour %= 12;
			break;
		case ZULU:
			if (at.tm_hour == 24 && at.tm_min != 0)
				atabort(MSGSTR(MS_BTIME,"bad time\n"));
			at.tm_hour %= 24;
			gmtflag = 1;
	}
}

static add_incr(int amount, int period)
{
	switch (period) {
		case MINUTE:
			rt.tm_min += amount;
			break;
		case HOUR:
			rt.tm_hour += amount;
			break;
		case DAY:
			rt.tm_mday += amount;
			break;
		case WEEK:
			rt.tm_mday += amount * 7;
			break;
		case MONTH:
			rt.tm_mon += amount;
			break;
		case YEAR:
			rt.tm_year += amount;
			break;
	}
}
