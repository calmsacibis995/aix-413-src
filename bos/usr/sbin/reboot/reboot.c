static char sccsid[] = "@(#)26	1.11.1.5  src/bos/usr/sbin/reboot/reboot.c, cmdoper, bos411, 9428A410j 11/22/93 12:32:38";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: reboot
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980,1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syslog.h>
#include <locale.h>
#include <sys/reboot.h>
#include <utmp.h>
#include "reboot_msg.h" 

/**********************************************************************/
/* Global / External Variables                                        */
/**********************************************************************/
char	wtmpf[]	= "/var/adm/wtmp";
struct utmp wtmp;
extern nl_catd catd;

/**********************************************************************/
/* Constant Definition / Macro Function                               */
/**********************************************************************/
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define MSGSTR(n,s) catgets(catd,MS_REBOOT,n,s) 

#define  MIN_mm  01  /* Minimum value of month  */
#define  MAX_mm  12  /* Maximum value of month  */
#define  MIN_dd  01  /* Minimum value of day    */
#define  MAX_dd  31  /* Maximum value of day    */

#define  MIN_HH  00  /* Minimum value of hour   */
#define  MAX_HH  23  /* Maximum value of hour   */
#define  MIN_MM  00  /* Minimum value of minute */
#define  MAX_MM  59  /* Maximum value of minute */

#define  MAX_20yy  38  /* Maximum value of year 20XX */
#define  MIN_19yy  70  /* Minimum value of year 19XX */

#define  NUM_SIZE    7  /* Size of num array */
#define  NUMBER(c1,c2)  ( ((c1)-'0')*10 + ((c2)-'0') )
#define   STR_SIZE   15  /* size of string array */

/**********************************************************************/
/* Prototypes                                                         */
/**********************************************************************/
void formtime(char *idate, char *outdate, char *outtime, char *inyr);
int chk_all_num(char *);
int parse_string(char *, time_t *, struct tm *, time_t);


/*
 * NAME:  Reboot
 * FUNCTION:  restarts the machine.
 *   -l   Do not log the reboot in the system accounting records.
 *        file. The -n and -q options imply -l.
 *   -n   Do not perform the sync.
 *   -q   Reboot quickly and ungracefully, without shutting down running 
 *        processes first.
 *   -t mmddHHMM[yy]
 *        Power on the system at the input time.
 */
main(int argc, char **argv)
{
	int qflag = 0;
	int nflag = 0;
	int tflag = 0;
	int needlog = 1;
	char *user;
	struct passwd *pw;
	int opt;
        time_t tmdiff, ctime, tbuf;
        struct tm *ltim;
        char lt[80], indate[STR_SIZE], fmdate[STR_SIZE], fmtime[STR_SIZE];
        char optyr[STR_SIZE];

	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_REBOOT, NL_CAT_LOCALE);

        /*** Get current date and time ***/
        ctime = time(&tbuf);
        ltim = localtime(&tbuf);

        /*** Format date for printing ***/
        strftime(lt, 80, "%D %H:%M", ltim);
        strftime(optyr, 15, "%y", ltim);

	while((opt = getopt(argc,argv,"lnqt:")) != EOF) {
		switch(opt) {
		case 'n':		/* do not sync, do not log */
			nflag++;
			needlog = 0;
			break;
		case 'q':		/* quick reboot, do not log */
			qflag++;
			needlog = 0;
			break;
		case 'l':		/* do not log */
			needlog = 0;
			break;
                case 't':               /* power on in specified time */
                        /* Call function to validate time */
                        strcpy(indate, optarg);
                        if (chk_all_num(optarg) == 0 ) {
                                if ( parse_string(optarg, &tmdiff, ltim, ctime) == 0) {
				if ( tmdiff >= 0 ) {
                                                tflag++;
                                                break;
                                        }
                                else  {
                                        /*** Put time in printable format ***/

                                                fprintf(stderr,
                                                MSGSTR(USAGE1, "Enter a date \
to power on that is later than %s\n"), lt);
                                        }
                                }
                        }

/* for case -t, if chk_all_num(optarg) fails, then fall through to the default */
		default:		
			fprintf(stderr,
			    MSGSTR(USAGE2,"Usage: reboot [-l][-n][-q]\
[-t mmddHHMM[yy]]\n"));
			exit(1);
			break;
		}
	}

	if (needlog) {
		openlog("reboot", LOG_NDELAY, LOG_AUTH);
		user = getlogin();
		if (user == (char *)0 && (pw = getpwuid(getuid())))
			user = pw->pw_name;
		if (user == (char *)0)
			user = "root";
		syslog(LOG_CRIT,MSGSTR(LOGIT,"rebooted by %s"), user);
		markdown();
	}

	signal(SIGHUP, SIG_IGN);	/* for remote connections */
	if (!qflag) {
		if (!nflag)
			sync();
		killall();
		setalarm(5);
		pause();
		if (!nflag)
			sync();
	}

        /* Pass future power on time to reboot */
        if (tflag) {
                formtime(indate, fmdate, fmtime, optyr);
                printf(MSGSTR(REBOOT1,"Automatic power on scheduled for %s %s.\n"), fmdate, fmtime); 
		printf(MSGSTR(REBOOT,"Rebooting . . .\n"));
                fflush(stdout);
                sleep((unsigned)3);
                reboot(RB_POWIPL, &tmdiff);
                perror(MSGSTR(PERROR,"reboot"));
                exit(1);
        }
        /* Otherwise do immediate reboot  */
        else {
                printf(MSGSTR(REBOOT,"Rebooting . . .\n"));
                fflush(stdout);
                sleep((unsigned)3);
                reboot(RB_SOFTIPL,(time_t *)0);
                perror(MSGSTR(PERROR,"reboot"));
                exit(1);
        }
}

/***********************************************************************
 *  NAME:  formtime
 *
 *  FUNCTION:  Change input date to nice format
 *              mm/dd/yy HH:MM
 *
 *  ARGUMENTS: string pointers to input date, output date(mm/dd/yy),
 *             output time (HH:MM), input year (if entered).
 *
 *  RETURN VALUE:  string pointers passed as arguments above:
 *                 output date and output time.
 *
 **********************************************************************/

void formtime(idate, outdate, outtime, inyr)
char *idate;
char *outdate;
char *outtime;
char *inyr;
{
        int i, j, k;
        for (i = 0, j = 0, k = 0; idate[i] != '\0' ; i++ ) {
/* Setup mm/dd/ */
                if ( i <= 3 ) {
                        outdate[j]=idate[i];
                        j++;
                        if ( i == 1 || i == 3 ) {
                                outdate[j]='/';
                                j++;
                        }
                }
/* Setup HH:MM */
                if ( i > 3 && i <= 7 ) {
                        outtime[k]=idate[i];
                        k++;
                        if ( i == 5 ) {
                                outtime[k]=':';
                                k++;
                        }
                }
        }
/* Setup yy and append to mm/dd/ */
        i=8;
        if ( idate[i] != '\0' ) {
                outdate[j]=idate[i];
                j++;
                i++;
                outdate[j]=idate[i];
        }
        else  {
                outdate[j]=inyr[0];
                j++;
                outdate[j]=inyr[1];
        }
}

/*
 * NAME: dingdong
 * FUNCTION:  catch SIGALRM
 */
dingdong(void)
{
	/* RRRIIINNNGGG RRRIIINNNGGG */
}

/*
 * NAME: setalarm
 * FUNCTION: set up alarm.
 */
setalarm(n)
{
	signal(SIGALRM, (void (*)(int))dingdong);
	alarm((unsigned)n);
}

/*
 * NAME: markdown
 * FUNCTION:  write shutdown entry in /var/adm/wtmp file.
 */
markdown()
{
        char *line;
        char *tmps;

	register f = open(wtmpf, 1);
	if (f >= 0) {
		lseek(f, 0L, 2);
		if ((line = ttyname(1)) == NULL)
			SCPYN(wtmp.ut_line, "~");
		else {
			tmps = (char *) strchr(line,'/');
			tmps++;
			line = (char *)strchr(tmps,'/');
			line++;
			SCPYN(wtmp.ut_line,line);
		}

		SCPYN(wtmp.ut_name, "shutdown");
		SCPYN(wtmp.ut_host, "");
		time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		fsync(f);
		close(f);
	}
}

/*
 * NAME: killall
 * FUNCTION: kill any processes that are running except this one
 */
killall()
{
	if (fork() == 0) {     /* child */
		execl("/usr/sbin/killall","killall","-",0);
		fprintf(stderr,MSGSTR(CAUTION,"CAUTION: some process(es) wouldn't die\n"));
	}
	else
		wait((int *)0);
}

/***********************************************************************
 *  NAME:  chk_all_num
 *
 *  FUNCTION:  Check if string only contains numeric characters '0'-'9'
 *             and its a valid length.
 *
 *  ARGUMENTS:  string pointer. (char *)
 *
 *  RETURN VALUE:  TRUE  - '0'-'9'  only
 *                 FALSE - invalid character or format.
 **********************************************************************/

int  chk_all_num(str)
char  *str;
{
        int  i;
        for ( i=0; str[i] != '\0'; i++ ) {
                if ( !(isdigit(str[i])) )   {
                        return(1);
                        break;
                }
        }
        if ( i != 8 && i != 10 )
                return(1);
        else
                return(0);
}


/***********************************************************************
 *  NAME:  parse_string
 *
 *  FUNCTION:  Convert the date/time format given on the command line
 *             to timbuf.
 *             The given string will be processed on assumption that
 *             it does not contain any invalid character and is the
 *             correct length.
 *  ARGUMENTS:  str (char *) - date/time format string.
 *
 *  RETURN VALUE:  TRUE  - succeeded in conversion
 *                 FALSE - conversion failed
 **********************************************************************/

int  parse_string(str, td, tim, curtime)
char  *str;
time_t *td;
struct tm  *tim;
time_t curtime;
{
        int  i, j;
        int num[NUM_SIZE];
        int  mm, dd, HH, MM, SS, yy;
        time_t timbuf;
        time_t seconds;

/* Add in the current time's seconds, so test of difference between the
   curtime and calculated time doesn't fail because current time is
   greater just because of seconds   */
         seconds = tim->tm_sec;

        /*** Set the month, date, hour, minute and year ***/
        for ( i = 0, j = 0 ; str[i] != '\0'; i += 2, j++ )
                num[j] = NUMBER(str[i],str[i+1]);
        mm = num[0];
/* to adjust for the tm structure that expects a month between 0-11 */
        tim->tm_mon =  mm - 1;
        tim->tm_mday = dd = num[1];
        tim->tm_hour = HH = num[2];
        tim->tm_min = MM = num[3];
        if ( j == 5 )   tim->tm_year = yy = num[4];
        else            tim->tm_year = yy = tim->tm_year % 100;

        /*** Check range of each value ***/
        if ( mm < MIN_mm || mm > MAX_mm ||
             dd < MIN_dd || dd > MAX_dd ||
             HH < MIN_HH || HH > MAX_HH ||
             MM < MIN_MM || MM > MAX_MM ||
           ( yy > MAX_20yy && yy < MIN_19yy ) ) {
                fprintf(stderr,
                MSGSTR(USAGE3,"Enter a valid date where:\n\
mm is a 2-digit number for month between 01 and 12\n\
dd is a 2-digit number for day between 01 and 31\n\
HH is a 2-digit number for minute between 00 and 23\n\
MM is a 2-digit number for hour between 00 and 59\n\
yy is an optional 2-digit number for year between 1970-1999 or 2000-2038\n"));
                return(1);
        }

        /*** Convert to time_t format ***/
        timbuf = mktime(tim);

/* Add in seconds from time generated from system */
        timbuf += seconds;

/* Calculate the number of seconds to reboot in */
        *td = timbuf - curtime;

        return(0);
}


