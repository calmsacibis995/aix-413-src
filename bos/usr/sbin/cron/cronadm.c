static char sccsid[] = "@(#)10  1.7.1.1  src/bos/usr/sbin/cron/cronadm.c, cmdcntl, bos411, 9428A410j 11/9/93 11:56:56";
/*
 * COMPONENT_NAME:  CMDCNTL
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */                                                                   

#include <sys/id.h>
#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>
#include <locale.h>
#include <sys/audit.h>
#include <time.h>
#include <errno.h>
#include <sys/priv.h>
#include "cron.h"

#include <nl_types.h>
#include "cron_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)


/*
 * NAME: cronadm
 *                                                                    
 * FUNCTION: command line processing to call the correct cron/at administrative
 *      function. This function is designed to be used by a system administrator
 *      to manage all the cron/at functions in an environment without a
 *      monolithic superuser. Normal users will use crontab or at to manage
 *      jobs, but anyone with execute permission for cronadm has the 
 *      potential to manipulate jobs for any user.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *      This function will compute the command line arguements and
 *      call the correct function based on the user flags. This is a 
 *      front end to the cron administrative functions.
 *                                                                   
 * (NOTES:) NULL passed as a flag parameter means to list everything
 *          for the administrator.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: nothing
 */  

#define INVALIDUSER     "you are not a valid user (no entry in /etc/passwd)"
#define NONFILE         "cronadm: specify job name or username\n"
#define AUDITER         "%s: auditproc error: %d\n"
#define CRADMUSAGE1 	"Usage: cronadm cron {{-l|-v}[UserName...]|-r UserName...}\n"
#define CRADMUSAGE2 	"       cronadm at {-l UserName...|-r {JobNumber...|UserName...}}\n"

extern int list_cj();
extern int remove_cj();
extern int list_aj();
extern int remove_aj();
extern int errno;


main(argc,argv)
int argc;
char *argv[];
{

	uid_t ruid;
        int i=0;
        struct passwd *pw;

        (void ) setlocale(LC_ALL,"");
        catd = catopen(MF_CRON,NL_CAT_LOCALE);

        /* 
         * suspend auditing for this process - This process is trusted 
         */

        if (auditproc(0,AUDIT_EVENTS,"cron",strlen("cron")+2) < 0) {
                fprintf(stderr,MSGSTR(MS_AUDITER, AUDITER),"cronadm",errno);
        }
        if (auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0) < 0) {
                fprintf(stderr,MSGSTR(MS_AUDITER, AUDITER),"cronadm",errno);
        }

	ruid = getuid();		/* get the real user id */
        setuidx(ID_REAL,ruid);          /* set process id back to real id */
        privilege(PRIV_LAPSE);          /* lapse all privileges */


        if ((pw=getpwuid(ruid)) == NULL)    /* get pw struct */
                cronadm_abort(MSGSTR(MS_INVALIDUSER, INVALIDUSER));

        /*
         * Interpret command line flags;  cronadm cron [-l|-r|-v] user name ... 
                                          cronadm at   [-l user name|-r job name]
         */

        if ((argc < 3) || ((strcmp(argv[2],"-r")) && (strcmp(argv[2],"-v")) 
           && (strcmp(argv[2],"-l")))) 
                usage();


        /*
         * determine if job is crontab or at and then call the 
         * correct routine 
         */

        switch(argv[1][0]) {
          case 'a':                     /* atjobs */
                if (strcmp(argv[2],"-l")==0) {
                        if (argc == 3)          /* cronadm at -l */
                                list_aj(CRON_USER|CRON_SORT_E,NULL);    /* ADMIN */
                        else {                  /* list specific users */
                                for (i=3; i<argc; i++) {
                                      list_aj(CRON_USER|CRON_SORT_E,argv[i]);
                                }
                        }
                }
                else if (strcmp(argv[2],"-r")==0) {     /* remove */
                        if (argc == 3)                  /* specify file */
                		cronadm_abort(MSGSTR(MS_NONFILE, NONFILE));
                        else {
                                for (i=3; i<argc; i++) {
                                        if (*argv[i] == '-')
                                                continue;
				  if (strchr(argv[i],(int)'.')) {
                                        remove_aj(CRON_NOTHING,argv[i]);   
                                        audit_pr(REMOVE_AT,pw->pw_name);
				  } else {
					remove_aj(CRON_USER,argv[i]);
                                        audit_pr(REMOVE_AT,argv[i]);
				  }

                                }
                        }
                }
                break;

          case 'c':                     /* crontabs */

/*
                if ((strcmp(argv[2],"-v")) && (strcmp(argv[2],"-r")) && 
                   (strcmp(argv[2],"-l"))) 
                        usage();
*/
                if (strcmp(argv[2],"-v")==0) {
                        if (argc == 3)          /* cronadm crontabs -v */
                                list_cj(CRON_NON_VERBOSE,NULL);
                        else {                  /* list specific crontabs */
                                for (i=3; i<argc; i++)
                                        list_cj(CRON_NON_VERBOSE,argv[i]);
                        }
                }
                else if (strcmp(argv[2],"-l")==0) {
                        if (argc == 3)          /* cronadm crontabs -l */
                                list_cj(CRON_VERBOSE,NULL);
                        else {                  /* list specific crontabs */
                                for (i=3; i<argc; i++)
                                        list_cj(CRON_VERBOSE,argv[i]);
                        }
                }
                else if (strcmp(argv[2],"-r")==0) {
                        if (argc == 3)          /* no job named */
                                usage();

                        for (i=3; i<argc; i++) {
                                remove_cj(CRON_QUIET,argv[i]);  
                /*              audit_pr("CRON_Adm",REMOVE_CRON,argv[i]); */
                                audit_pr(REMOVE_CRON,argv[i]);
                        }
                }
                break;
          default:
                usage();
        }
        exit(0);
}

/*
 * NAME:  cronadm_abort
 *                                                                    
 * FUNCTION: print the error message with the prefix of "cronadm: "
 *                                                                    
 * EXECUTION ENVIRONMENT: none
 *
 * RETURNS:  exit(1)
 */

cronadm_abort(msg)
char *msg;
{
        fprintf(stderr,"cronadm: %s\n",msg);
        exit(1);
}

/*
 * NAME:  usage
 *                                                                    
 * FUNCTION: print the usage message.
 *                                                                    
 * EXECUTION ENVIRONMENT: none
 *
 * RETURNS:  1
 */
usage()
{
        fprintf(stderr, MSGSTR(MS_CRADMUSAGE3,CRADMUSAGE1)); 
        fprintf(stderr, MSGSTR(MS_CRADMUSAGE4,CRADMUSAGE2)); 
        exit(1);
}

/*
 * NAME: audit_pr
 *                                                                    
 * FUNCTION: audit deletion of crontab and at files
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) More detailed description of the function, down to
 *          what bits / data structures, etc it manipulates. 
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS:  nothing returned
 */  

/*
audit_pr(action,name)
int action;
char *name;
{
        extern time_t   time ();
        time_t  t = time ((long *) 0);
        struct passwd *pw;

        if ((pw=getpwuid(getuid())) == NULL)  
                fprintf(stderr,MSGSTR(MS_INVALIDUSER, INVALIDUSER));

        privilege(PRIV_ACQUIRE);        ?* need SET_PROC_AUDIT *?

        if (action == REMOVE_CRON) {
                ?* Audit successful crontab deletion *?
                auditwrite ("CRON_Adm",
                        0,
                        "CRON_JobRemove",
                        strlen ("CRON_JobRemove") + 1,
                        name,                   ?* job name *?
                        strlen (name) + 1,
                        pw->pw_name,
                        strlen (pw->pw_name) + 1,
                        ctime(&t),
                        strlen(ctime(&t)),
			0
                   );
        }
        else {
                ?* Audit successful atjob deletion *?
                auditwrite ("CRON_Adm",
                        0,
                        "AT_JobRemove",
                        strlen ("AT_JobRemove") + 1,
                        name,                   ?* job name *?
                        strlen (name) + 1,
                        pw->pw_name,
                        strlen (pw->pw_name) + 1,
                        ctime(&t),
                        strlen(ctime(&t)),
			0
                   );
        }

        privilege(PRIV_LAPSE);          ?* lapse all privs *?
}

*/
