static char sccsid[] = "@(#)06	1.9  src/bos/usr/sbin/cron/funcs.c, cmdcntl, bos411, 9428A410j 6/8/91 21:22:40";
/*
 * COMPONENT_NAME: (CMDCNTL) commands needed for basic system needs
 *
 * FUNCTIONS: funcs
 *
 * ORIGINS: 3, 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * @OSF_COPYRIGHT@
static char rcsid[] = "RCSfile: funcs.c,v Revision: 1.4 (OSF)Date: 90/10/07 22:06:31";
*/

/*
 * module used for crontab, cron, and at
 */

#include <sys/types.h>
#include <sys/priv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <time.h>
#include <nl_types.h>

#include "cron.h"
#include "cron_msg.h"
extern nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
#define INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)"
#define MALLOC 		"There is not enough memory available now.\n"

/*****************/
time_t num(ptr)
/*****************/
char **ptr;
{
        time_t n=0;
        while (isdigit(**ptr)) {
                n = n*10 + (**ptr - '0');
                *ptr += 1; }
        return(n);
}


int dom[12]={31,28,31,30,31,30,31,31,30,31,30,31};

/*
 * NAME: days_in_mon
 *
 * FUNCTION: returns the number of days in month m of year y
 *
 * EXECUTION ENVIRONMENT:
 *         NOTE: m should be in the range 0 to 11 
 *
 * RETURNS: the number of the days. If the input range is illigal then return 0
 */

int days_in_mon(m,y)
int m,y;
{
	if ( m < 0 || y < 0 || m > 11) return(0);
        return( dom[m] + (((m==1)&&((y%4)==0)) ? 1:0 ));
}

/*
 * NAME: days_btwn
 *
 * FUNCTION: calculate the number of "full" days in between m1/d1/y1 
 * 		and m2/d2/y2.
 *
 * EXECUTION ENVIRONMENT:
 *         NOTE: there should not be more than a year separation in the dates.
 *               also, m should be in 0 to 11, and d should be in 1 to 31. 
 *
 * RETURNS: the number of the days. If the input range is illigal then return 0
 */

int days_btwn(m1,d1,y1,m2,d2,y2)
int m1,d1,y1,m2,d2,y2;
{
        
        int days,m;

        if ((m1==m2) && (d1==d2) && (y1==y2)) return(0);
        if ((m1==m2) && (d1<d2)) return(d2-d1-1);
        /* the remaining dates are on different months */
        days = (days_in_mon(m1,y1)-d1) + (d2-1);
        m = (m1 + 1) % 12;
        while (m != m2) {
                if (m==0) y1++;
                days += days_in_mon(m,y1);
                m = (m + 1) % 12; }
        return(days);
}

/*
 * NAME: xmalloc
 *
 * FUNCTION: malloc the specified area(size).
 *
 * RETURNS: returns the pointer. If failed then exit 55
 */
char *xmalloc(size)
int size;
{
        char *p;

        if((p=(char *) malloc((size_t)size)) == NULL) {
                fprintf(stderr,MSGSTR(MS_MALLOC,MALLOC));
                fflush(stdout);
                exit(55);
        }
        return p;
}

/*
 * NAME: xrealloc
 *
 * FUNCTION: realloc the specified area(size) of p.
 *
 * RETURNS: returns the pointer. If failed then exit 55
 */
char *xrealloc(p, size)
char *p;
int size;
{
        if((p= (char *) realloc((void *)p, (size_t)size)) == NULL) {
                fprintf(stderr,MSGSTR(MS_MALLOC,MALLOC));
                fflush(stdout);
                exit(55);
        }
        return p;
}

/*
 * NAME: cat2
 *
 * FUNCTION: cat s1 and s2 to the malloced area.
 *
 * RETURNS: returns the pointer of the reserved area.
 */
char *cat2(s1, s2)
char *s1, *s2;
{
    return (strcat(strcpy(xmalloc((int)(strlen(s1)+strlen(s2)+1)), s1), s2));
}

/*
 * NAME: sameenv
 *
 * FUNCTION: compare if the environment data is equal or not.
 *
 * RETURNS: 0 - not equal 1 - eaual.
 */
static int sameenv(s, t)
char *s, *t;
{
    while (*s != '\0' && *s != '=')
        if (*s++ != *t++) return (0);
    return (*t == '\0' || *t == '=');
}

/*****************/
/* void putenv(s)               NOT USED */
/*****************/
/* char *s;
{
    extern char **environ;
    static char **envmalloced = NULL;
    static char *nullenv[1] = { NULL };
    register char **ep;
    if (environ == NULL) environ = nullenv;
    ep = environ;
    for (ep = environ; *ep != NULL && !sameenv(*ep, s); ++ep);
    if (*ep == NULL) {
        register char **newenv;
        if (environ == envmalloced) {
            newenv = (char **)xrealloc((char *)environ,
                    (ep-environ+2)*sizeof(char *));
        } else {
            newenv = (char **)memcpy((void *)xmalloc((ep-environ+2)*sizeof(char *)), (void *)environ, (size_t)((ep-environ)*sizeof(char *)));
        }
        ep = newenv + (ep - environ);
        envmalloced = environ = newenv;
        ep[1] = NULL;
    }
    *ep = s;
}
*/
/*
 * NAME: audit_pr
 *                                                                    
 * FUNCTION: audit log of addition and deletion of crontab and at files
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:)  This function logs the information of cron and at jobs are 
 * 	     added or removed.
 *
 * RETURNS:  nothing returned
 */  

audit_pr(action,name)
int action;
char *name;
{
        time_t  t = time ((long *) 0);
        struct passwd *pw;
	char s[40];

        if ((pw=getpwuid(getuid())) == NULL)  
                fprintf(stderr,MSGSTR(MS_INVALIDUSER, INVALIDUSER));

        privilege(PRIV_ACQUIRE);        /* need SET_PROC_AUDIT */

        if (action == REMOVE_CRON)  strcpy(s,"CRON_JobRemove");
        if (action == REMOVE_AT)    strcpy(s,"AT_JobRemove");
        if (action == ADD_CRON)  strcpy(s,"CRON_JobAdd");
        if (action == ADD_AT)    strcpy(s,"AT_JobAdd");

        auditwrite (s,
                0,
                name,                   /* user name or job name */
                strlen (name) + 1,
                pw->pw_name,
                strlen (pw->pw_name) + 1,
                ctime(&t),
                strlen(ctime(&t)),
		0
        );

        privilege(PRIV_LAPSE);          /* lapse all privs */
}

