static char sccsid[] = "@(#)24	1.49.1.4  src/bos/usr/sbin/cron/cron.c, cmdcntl, bos41J, 9521B_all 5/26/95 10:58:59";
/*
 * COMPONENT_NAME: (CMDCNTL) commands needed for basic system needs
 *
 * FUNCTIONS: cron 
 *
 * ORIGINS: 3, 18, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 *
 */
 
/*
 *  cron is a daemon that runs jobs at requested times.
 */                                                                   

#ifdef SEC_BASE
#include <sys/secdefines.h>
#else
#include <usersec.h>
#include <sys/priv.h>
#include <sys/audit.h>
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <sys/mode.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <locale.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <nl_types.h>
#include <langinfo.h>
#include <sys/var.h>
#include <sys/sysconfig.h>

#include "cron.h"
#include "cron_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)

#if SEC_BASE
#include <sys/security.h>
#include <prot.h>
#if SEC_MAC
extern char     *cron_jobname();
#endif
#if SEC_MAC || SEC_NCAV
extern char     *cron_getlevel();
#endif
#endif /* SEC_BASE */

#define AUDIT_CLASS     "cron\0\0"
#define CRON_START      "CRON_Start"
#define CRON_FINISH     "CRON_Finish"
#define AT_JOB          "start at job"
#define CRON_JOB        "start cron job"
#define MAIL             "/usr/bin/mail" /* mail program to use */
#define CONSOLE         "/dev/console"  /* where to write error messages when cron dies */

#define TMPINFILE       "/tmp/crinXXXXXX"  /* file to put stdin in for cmd  */
#define TMPDIR          "/tmp"
#define PFX             "crout"

#define INMODE          00400           /* mode for stdin file  */
#define OUTMODE         00600           /* mode for stdout file */

#define INFINITY        2147483647L     /* upper bound on time  */
#define CUSHION         120L
#define MAXRUN          40              /* default max total jobs allowed */
#define ZOMB            100             /* proc slot used for mailing output */

#define JOBF            'j'
#define NICEF           'n'
#define USERF           'u'
#define WAITF           'w'

#define DEFAULT         0
#define LOAD            1

#define BADCD           "can't change directory to the %s directory."
#define NOREADDIR       "can't read the %s directory."

#define BADJOBOPEN      "unable to read your at job."
#define BADSTAT         "can't access your crontab file.  Resubmit it."
#define CANTCDHOME      "can't change directory to your home directory.\nYour commands will not be executed."
#define CANTEXECSH      "unable to exec the shell for one of your commands."
#define EOLN            "unexpected end of line"
#define NOREAD          "can't read your crontab file.  Resubmit it."
#define NOSTDIN         "unable to create a standard input file for one of your crontab commands.\nThat command was not executed."
#define OUTOFBOUND      "number too large or too small for field"
#define STDERRMSG       "\n\n*************************************************\nCron: The previous message is the standard output\n      and standard error of one of your cron commands.\n"
#define STDOUTERR       "one of your commands generated output or errors, but cron was unable to mail you this output.\nRemember to redirect standard output and standard error for each of your commands."
#define UNEXPECT        "unexpected symbol found"
#undef  sleep

struct event {  
        time_t time;    /* time of the event    */
        short etype;    /* what type of event; CRONEVENT ATEVENT...*/
        char *cmd;      /* command for cron, job name for at    */
        struct usr *u;  /* ptr to the owner (usr) of this event */
        struct event *link;     /* ptr to another event for this user */
        union { 
                struct { /* for crontab events */
                        char *minute;   /*  (these      */
                        char *hour;     /*   fields     */
                        char *daymon;   /*   are        */
                        char *month;    /*   from       */
                        char *dayweek;  /*   crontab)   */
                        char *login;    /* login id     */
                        char *input;    /* ptr to stdin */
                } ct;
                struct { /* for at events */
                        short exists;   /* for revising at events       */
                        int eventid;    /* for el_remove-ing at events  */
                } at;
        } of; 
};

struct usr {    
        char *name;     /* name of user (e.g. "kew")    */
        char *home;     /* home directory for user      */
        int uid;        /* user id      */
        int gid;        /* group id     */
#ifdef ATLIMIT
        int aruncnt;    /* counter for running jobs per uid */
#endif
#ifdef CRONLIMIT
        int cruncnt;    /* counter for running cron jobs per uid */
#endif
        int ctid;       /* for el_remove-ing crontab events */
        short ctexists; /* for revising crontab events  */
        struct event *ctevents; /* list of this usr's crontab events */
        struct event *atevents; /* list of this usr's at events */
        struct usr *nextusr; 
#if SEC_MAC || SEC_NCAV
        char *seclevel_ir;      /* security level of job */
#endif
};      /* ptr to next user     */

struct  queue
{
        int njob;       /* limit */
        int nice;       /* nice for execution */
        int nwait;      /* wait time to next execution attempt */
        int nrun;       /* number running */
}       
        qd = {100, 2, 60},              /* default values for queue defs */
        qt[NQUEUE];

struct  queue   qq;
int     wait_time = 60;

struct  runinfo
{
        pid_t   pid;
        short   que;
        struct  usr *rusr;              /* pointer to usr struct */
        char    *outfile;       /* file where stdout & stderr are trapped */
}       *rt;

int msgfd;              /* file descriptor for fifo queue */
int ecid=1;             /* for giving event classes distinguishable id names 
                           for el_remove'ing them.  MUST be initialized to 1 */
short jobtype;          /* at or batch job */
int delayed;            /* is job being rescheduled or did it run first time */
int notexpired;         /* time for next job has not come */
int cwd;                /* current working directory */
int running;            /* zero when no jobs are executing */
unsigned int chtime;    /* last time crontab, at files were read */
struct event *next_event;       /* the next event to execute    */
struct usr *uhead;      /* ptr to the list of users     */
struct usr *ulast;      /* ptr to last usr table entry */

struct usr *find_usr();
int timeout(void);
time_t init_time,num();
extern char *xmalloc();
char **getarray();
time_t next_time();
char *strchr();
char *getfilenam();
int maxrun = MAXRUN;	/* max jobs allowed - default is MAXRUN */

#ifdef _OSF
extern daemon();        /* From libutil */

/* user's default environment for the shell */
char homedir[100]="HOME=";
char logname[50]="LOGNAME=";
char *envinit[]={
        homedir,logname,"PATH=:/usr/bin","SHELL=/usr/bin/bsh",0};
extern char **environ;
#endif

/* list of directories to open so that their inodes will always be in cache */
char *openlst[] = { "/bin", "/lib", "/usr", "/usr/bin", "/usr/lib", "/etc",
                    "/tmp", "/var/adm", 0};


/*
 * NAME: cron
 *
 * FUNCTION: executes commands automatically 
 *              The jobs which are executed by the cron daemon are:
 *                      at jobs, batch jobs, cron jobs, sync
 *                      ksh at jobs, csh at jobs
 * EXECUTION ENVIRONMENT:
 *      cron daemon is executed at the system startup time and never exit.
 *      If it failed and exit, it will be executed immediately by the inittab.
 *
 * RETURNS: none
 */

main(argc,argv)
char **argv;
{
        time_t t,t_old;
        time_t last_time;
        time_t ne_time;         /* amt of time until next event execution */
        time_t lastmtime = 0L;
        struct usr *u,*u2;
        struct event *e,*e2,*eprev;
        struct stat buf;
        long seconds;
        pid_t rfork;
        int id;                 /* event id */
        int time_zone;          /* time zone */
        struct tm *time_data;
        struct var kerparms;

        if (sysconfig(SYS_GETPARMS, &kerparms, sizeof(struct var)) == 0 )
            maxrun = kerparms.v_maxup;

        if ((rt = calloc( maxrun, sizeof (struct runinfo))) == NULL) {
            perror("cron ");
            exit(1);
        }

        (void) setlocale(LC_ALL,"");
        catd = catopen(MF_CRON,NL_CAT_LOCALE);

begin:                                  /* detach from console */

#ifdef _OSF
        if (daemon(1,1) == -1) {
                sleep((unsigned)30);
                goto begin; 
        }
#endif

#if SEC_BASE
        cron_init(argc, argv);
        cron_secure_mask();
#else
        privilege(PRIV_LAPSE);          /* lapse all privileges */
        umask((mode_t)022);
#endif
        setpgrp();      /* detach cron from console */
        signal(SIGHUP, SIG_IGN);
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);

#ifdef _AIX
        auditproc(0, AUDIT_EVENTS, AUDIT_CLASS, strlen(AUDIT_CLASS)+2);
        auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
#endif

#ifdef DEBUGX
        printf("starting !!!!\n");
printf(" catd = %d \n",catd);
#endif
        /*
         * open the basic system directories to assure that they will
         * always be in the inode cache for fast access.
         */
        {
                char **f;
                for (f = openlst; *f; f++)
                        open(*f, 0);
        }

        initialize();
        quedefs(DEFAULT);       /* load default queue definitions */
        msg(MSGSTR(MS_CRSTART, "*** cron started ***   pid = %ld"),getpid());
        timeout();      /* set up alarm clock trap */
        t_old = time((time_t *) 0);
        last_time = t_old;
        time_data = localtime(&t_old);
        time_zone = time_data -> tm_isdst;

        while (TRUE) {                  /* MAIN LOOP    */
#ifdef DEBUGX
        printf("in main while looprime zone = %d\n",time_zone);
printf(" catd = %d \n",catd);
#endif
                t = time((time_t *) 0);
                time_data = localtime(&t);
                if((t_old > t) || (t-last_time > CUSHION) || 
                        ( time_zone != time_data->tm_isdst)){
                        /* the time was set backwards or forward   
                                or changed the daylight saving time */
                        time_zone = time_data->tm_isdst;
                        el_delete();
                        u = uhead;
                        while (u!=NULL) {
                                rm_ctevents(u);
                                e = u->atevents;
                                while (e!=NULL) { /* at events */
                                        free((void *)e->cmd);
                                        e2 = e->link;
                                        free((void *)e);
                                        e = e2; 
                                }
                                u2 = u->nextusr;
                                u = u2; 
                        }
                        close(msgfd);
                        chtime = 0;
                        initialize();
                        t = time((time_t *) 0); 
                        chtime = t;
                }
                t_old = t;
                if (next_event == NULL)
                        if (el_empty()) ne_time = INFINITY;
                        else {  
                                next_event = (struct event *) el_first();
                                ne_time = next_event->time - t; 
                        }
                else ne_time = next_event->time - t;
#ifdef DEBUGX
                if(next_event != NULL)
                        printf("next_time=%ld  %s",
                                next_event->time,ctime(&next_event->time));
#endif
                seconds = (ne_time < (long) 0) ? (long) 0 : ne_time;
                if(ne_time > (long) 0)
                        idle(seconds);
                if(notexpired) {
                        notexpired = 0;
                        last_time = INFINITY;
                        continue;
                }
                if(stat(QUEDEFS,&buf))
                        msg(MSGSTR(MS_NOQUEDEFS,"cannot stat QUEDEFS file"));
                else
                        if(lastmtime != buf.st_mtime) {
                                quedefs(LOAD);
                                lastmtime = buf.st_mtime;
                        }
                last_time = next_event->time;   /* save execution time */
                ex(next_event);
#ifdef DEBUGX
                if(next_event != NULL)
                        printf("out ex    next_time=%ld  %s",
                                next_event->time,ctime(&next_event->time));
#endif
                switch(next_event->etype) {
                /* add cron or sync event back into the main event list */
                case CRONEVENT:
                case SYNCEVENT:
                        if(delayed) {
                                delayed = 0;
                                break;
                        }
                        next_event->time = next_time(next_event, (time_t)0);
                        if (next_event->time == last_time) {
                        /* Don't schedule event for same time twice.    */
                        /* Schedule it for next next time.              */
                        next_event->time = next_time(next_event,
                                (time_t)last_time + 15);
                        }
                        if (next_event->etype == CRONEVENT)
                                id = (next_event->u)->ctid; 
                        else
                                id = 0;
                        el_add(next_event, next_event->time, id);
                        break;
                /* remove at or batch job from system */
                default:
                        eprev=NULL;
                        e=(next_event->u)->atevents;
                        while (e != NULL)
                                if (e == next_event) {
                                        if (eprev == NULL)
                                                (e->u)->atevents = e->link;
                                        else    eprev->link = e->link;
                                        free((void *)e->cmd);
                                        free((void *)e);
                                        break;  
                                }
                                else {  
                                        eprev = e;
                                        e = e->link; 
                                }
                        break;
                }
                chtime = t;
                next_event = NULL; 
        }
}

/*
 * NAME: initialize
 *
 * FUNCTION: initialization of the date areas which are used by the cron.
 *              o initialize event queue.
 *              o add sync event
 *              o check FIFO queue status
 *              o read directories, create user list and add events to the
 *                main event list.
 *              o log file open.
 * EXECUTION ENVIRONMENT:
 * RETURNS: none
 */

initialize()
{

        static int flag = 0;
        char *s1,*s2;

#ifdef DEBUGX
        printf("in initialize\n");
#endif
        init_time = time((time_t *) 0);
        el_init(8,init_time,(long)(60*60*24),10);


         /* Add sync event that will get executed once a minute.
          */
        sync_event();
#if SEC_BASE
        cron_set_communications(FIFO, 0);
#else
        if(access(FIFO,R_ACC)==-1 && mknod(FIFO,S_IFIFO|S_IRUSR|S_IWUSR,0)!=0) 
                crabort(MSGSTR(MS_NOFIFO1,"cannot access %s queue"),FIFO);
#endif
        if((msgfd = open(FIFO, O_RDWR)) < 0) {
                perror("open");
                crabort(MSGSTR(MS_NOFIFO2,"cannot create fifo queue"),"");
        }

        /* read directories, create users list,
           and add events to the main event list        */
        uhead = NULL;
        read_dirs();
        next_event = NULL;
        if(flag)
                return;
#if SEC_BASE
        create_file_securely(ACCTFILE, AUTH_SILENT, "cron event log");
#endif
    /* cron should write to log ONLY if it exists */
        if(freopen(ACCTFILE,"a",stdout) == NULL)
                fprintf(stderr,MSGSTR(MS_NOWRITE,"cannot write to %s\n"),ACCTFILE);
        close((int)fileno(stderr));
        dup(1);
        /* this must be done to make popen work....i dont know why */
        freopen("/dev/null","r",stdin);
        flag = 1;
#ifdef DEBUGX
        printf("leaving initialize\n");
#endif
}


/*
 * NAME: read_dirs
 * FUNCTION: read directories, create user list and add main event list.
 * EXECUTION ENVIRONMENT:
 *              o /usr/spool/cron/crontabs
 *              o /usr/spool/cron/atjobs
 * RETURNS: none
 */

read_dirs()
{
        DIR *dir;
        int mod_ctab(), mod_atjob();
        char *s1,*s2;

#ifdef DEBUGX
        printf("Read Directories\n");
#endif
        if (chdir(CRONDIR) == -1) crabort(MSGSTR(MS_BADCD,BADCD),CRONDIR);
        cwd = CRON;
#if SEC_MAC
        s1 = MSGSTR(MS_NOREADDIR,NOREADDIR);
        s2 = xmalloc(strlen(s1)+strlen(CRONDIR));
        sprintf(s2,s1,CRONDIR);
        cron_existing_jobs(s2, mod_ctab, 1);
        free(s2);
#else
        if ((dir=opendir("."))==NULL)  
                crabort(MSGSTR(MS_NOREADDIR,NOREADDIR),CRONDIR);
        dscan(dir,mod_ctab);
        closedir(dir);
#endif
        if(chdir(ATDIR) == -1) {
                msg(MSGSTR(MS_BADCD,BADCD),ATDIR);
                return;
        }
        cwd = AT;
#if SEC_MAC
        s1 = MSGSTR(MS_NOREADDIR,NOREADDIR);
        s2 = xmalloc(strlen(s1)+strlen(ATDIR));
        sprintf(s2,s1,ATDIR);
        cron_existing_jobs(s2, mod_atjob, 0);
        free(s2);
#else
        if ((dir=opendir("."))==NULL) {
                msg(MSGSTR(MS_NOREADDIR,NOREADDIR),ATDIR);
                return; 
        }
        dscan(dir,mod_atjob);
        closedir(dir);
#endif
#ifdef DEBUGX
        printf("leaving read_dirs\n");
#endif
}

#if !SEC_MAC
/*
 * NAME: dscan
 * FUNCTION: read directories specified by the df, 
 *           create user list and add main event list.
 * RETURNS: none
 */
dscan(df,fp)
DIR     *df;
int     (*fp)();
{

        struct dirent *dn;
        char            name[PATH_MAX+1];

#ifdef DEBUGX
        printf("inside dscan\n");
#endif
/*      rewinddir(df);*/         /* I don't think this is necessary */
        dn = readdir(df);     /* read . */
        dn = readdir(df);     /* read .. */
#ifdef DEBUGX
        printf("dscan reading dir\n");
#endif
        while(( dn = readdir(df) ) != NULL ) {
                if(dn->d_ino == 0)
                        continue;
                strncpy(name,dn->d_name,dn->d_namlen);
                name[dn->d_namlen] = '\0'; /* just in case */
                (*fp)(name);
        }
#ifdef DEBUGX
        printf("leaving dscan\n");
#endif
}
#endif /* !SEC_MAC */

/*
 * NAME: mod_ctab
 * FUNCTION: read crontab file specified by name.
 *              o checks the status of the crontabfile(CRONDIR/name)
 *              o If name is a new user then reserve a user data area.
 *                      and read crontab file.
 *              o If 'name' user already exists, and if he didn't have a 
 *                cronevent entry , create it and read crontab file.
 *              o If 'name' user already exists, and if he have a  cronevents
 *                already, remove the existing crontab enty and read crontab 
 *                file.
 * RETURNS: none
 */
mod_ctab(name)
char    *name;
{

        struct  passwd  *pw;
        struct  stat    buf;
        struct  usr     *u;
        char    namebuf[PATH_MAX];
        char    *pname;

#ifdef DEBUGX
        printf("inside mod_ctab\n");
#endif
        if((pw=getpwnam(name)) == NULL)
                return;
#if SEC_MAC
        pname = cron_jobname(cwd, CRON, CRONDIR, name, namebuf);
#else
        if(cwd != CRON) {
                strcat(strcat(strcpy(namebuf, CRONDIR),"/"),name);
                pname = namebuf;
        } else
                pname = name;
#endif
        if(stat(pname,&buf)) {
                mail(name,MSGSTR(MS_BADSTAT,BADSTAT),2);
                unlink(pname);
                return;
        }
        /*
         * D16930:
         *      The following lines are removed because the
         *      comparison with chtime is invalid.
         */
        /*
        if (buf.st_mtime < chtime)
                return;
        */

        if((u=find_usr(name)) == NULL) {
#ifdef DEBUGX
                printf("new user (%s) with a crontab\n",name);
#endif
                u = (struct usr *) xmalloc(sizeof(struct usr));
                u->name = xmalloc(strlen(name)+1);
                strcpy(u->name,name);
                u->home = xmalloc(strlen(pw->pw_dir)+1);
                strcpy(u->home,pw->pw_dir);
                u->uid = pw->pw_uid;
                u->gid = pw->pw_gid;
#if SEC_MAC || SEC_NCAV
                u->seclevel_ir = cron_getlevel();
#endif
                u->ctexists = TRUE;
                u->ctid = ecid++;
                u->ctevents = NULL;
                u->atevents = NULL;
#ifdef ATLIMIT
                u->aruncnt = 0;
#endif
#ifdef CRONLIMIT
                u->cruncnt = 0;
#endif
                u->nextusr = uhead;
                uhead = u;
                readcron(u);
        }
        else {
                u->uid = pw->pw_uid;
                u->gid = pw->pw_gid;
#if SEC_MAC || SEC_NCAV
                u->seclevel_ir = cron_getlevel();
#endif
                if(strcmp(u->home,pw->pw_dir) != 0) {
                        free(u->home);
                        u->home = xmalloc(strlen(pw->pw_dir)+1);
                        strcpy(u->home,pw->pw_dir);
                }
                u->ctexists = TRUE;
                if(u->ctid == 0) {
#ifdef DEBUGX
                        printf("%s now has a crontab\n",u->name);
#endif
                        /* user didnt have a crontab last time */
                        u->ctid = ecid++;
                        u->ctevents = NULL;
                        readcron(u);
                        return;
                }
#ifdef DEBUGX
                printf("%s has revised his crontab\n",u->name);
#endif
                rm_ctevents(u);
                el_remove(u->ctid,0);
                readcron(u);
        }
}

/*
 * NAME: mod_atjob
 * FUNCTION: read at job file specified by name.
 *              o checks the status of the crontabfile(CRONDIR/name)
 *              o If 'name' is a new user then reserve a user data area.
 *                      and read at jobs file.
 *              o If 'name' user already exists,  then add new at jobs entry
 *                      and read at jobs file.
 * RETURNS: none
 */
mod_atjob(name)
char    *name;
{
        char    *tmpptr, *ptr;
        time_t  tim;
        struct  passwd  *pw;
        struct  stat    buf;
        struct  usr     *u;
        struct  event   *e;
        char    namebuf[PATH_MAX];
        char    *pname, *uname;

#ifdef DEBUGX
        printf("inside mod_atjob\n");
#endif
        tmpptr = name;                  /* name = root.61326488.a */
        ptr = strchr(tmpptr,'.');       /* bypass login id in name */
        if (ptr == NULL)
                return;
	uname = getfilenam(name);
	ptr = name + strlen(uname) + 1;
        if(((tim=num(&ptr)) == 0) || (*ptr != '.'))
                return;
        ptr++;
        if(!isalpha((int)*ptr))
                return;
        jobtype = *ptr - 'a';
#if SEC_MAC
        pname = cron_jobname(cwd, AT, ATDIR, name, namebuf);
#else
        if(cwd != AT) {
                strcat(strcat(strcpy(namebuf,ATDIR),"/"),name);
                pname = namebuf;
        } else
                pname = name;
#endif

        if(stat(pname,&buf) || jobtype >= NQUEUE-1) {
                unlink(pname);
                return;
        }
/*      A15513 CONCERNING BATCH JOBS    */
/*      FIXED BY REMOVING THESE 2 LINES */
/*      if (buf.st_mtime < chtime)      */
/*              return;                 */

        if(!(buf.st_mode & ISUID)) {
                unlink(pname);
                return;
        }

        if((pw=getpwuid(buf.st_uid)) == NULL)
                return;
        if((u=find_usr(pw->pw_name)) == NULL) {
#ifdef DEBUGX
                printf("new user (%s) with an at job = %s\n",pw->pw_name,name);
#endif
                u = (struct usr *) xmalloc(sizeof(struct usr));
                u->name = xmalloc(strlen(pw->pw_name)+1);
                strcpy(u->name,pw->pw_name);
                u->home = xmalloc(strlen(pw->pw_dir)+1);
                strcpy(u->home,pw->pw_dir);
                u->uid = pw->pw_uid;
                u->gid = pw->pw_gid;
#if SEC_MAC || SEC_NCAV
                u->seclevel_ir = cron_getlevel();
#endif
                u->ctexists = FALSE;
                u->ctid = 0;
                u->ctevents = NULL;
                u->atevents = NULL;
#ifdef ATLIMIT
                u->aruncnt = 0;
#endif
#ifdef CRONLIMIT
                u->cruncnt = 0;
#endif
                u->nextusr = uhead;
                uhead = u;
                add_atevent(u,name,tim);
        }
        else {
                u->uid = pw->pw_uid;
                u->gid = pw->pw_gid;
#if SEC_MAC || SEC_NCAV
                u->seclevel_ir = cron_getlevel();
#endif
                if(strcmp(u->home,pw->pw_dir) != 0) {
                        free(u->home);
                        u->home = xmalloc(strlen(pw->pw_dir)+1);
                        strcpy(u->home,pw->pw_dir);
                }
                e = u->atevents;
                while(e != NULL)
                        if(strcmp(e->cmd,name) == 0) {
                                e->of.at.exists = TRUE;
                                break;
                        } else
                                e = e->link;
                if (e == NULL) {
#ifdef DEBUGX
                        printf("%s has a new at job = %s\n",u->name,name);
#endif
                        add_atevent(u,name,tim);
                }
        }
}

/*
 * NAME: add_atevent
 * FUNCTION: reserve a event data area and set the data.
 *           adda an event to the main event list(el_add)
 * RETURNS: none
 */
add_atevent(u,job,tim)
struct usr *u;
char *job;
time_t tim;
{
        struct event *e;

#ifdef DEBUGX
        printf("in add_atevent\n");
#endif
        e=(struct event *) xmalloc(sizeof(struct event));
        e->etype = jobtype;
        e->cmd = xmalloc(strlen(job)+1);
        strcpy(e->cmd,job);
        e->u = u;
#ifdef DEBUGX
        printf("add_atevent: user=%s, job=%s, time=%ld\n",
                u->name,e->cmd, e->time);
#endif
        e->link = u->atevents;
        u->atevents = e;
        e->of.at.exists = TRUE;
        e->of.at.eventid = ecid++;
        if(tim < init_time)             /* old job */
                e->time = init_time;
        else
                e->time = tim;
        el_add(e, e->time, e->of.at.eventid); 
#ifdef DEBUGX
        printf("leaving add_atevent\n");
#endif
}


char line[CTLINESIZE];          /* holds a line from a crontab file     */
int cursor;                     /* cursor for the above line    */

/*
 * NAME: readcron
 * FUNCTION: readcron reads in a crontab file for a user (u).
 *           The list of events for user u is built, and 
 *           u->events is made to point to this list.
 *           Each event is also entered into the main event list. 
 * RETURNS: none
 */
readcron(u)
struct usr *u;
{

        FILE *cf;               /* cf will be a user's crontab file */
        struct event *e;
        int start,i;
        char *next_field();
        char *get_login_name(), *login_name;
        char namebuf[PATH_MAX];
        char *pname;
        int line_num;

#ifdef DEBUGX
        printf("in readcron\n");
#endif
        /* read the crontab file */
#if SEC_MAC
        pname = cron_jobname(cwd, CRON, CRONDIR, u->name, namebuf);
#else
        if(cwd != CRON) {
                strcat(strcat(strcpy(namebuf,CRONDIR),"/"),u->name);
                pname = namebuf;
        } else
                pname = u->name;
#endif
        if ((cf=fopen(pname,"r")) == NULL) {
                mail(u->name,MSGSTR(MS_NOREAD, NOREAD),2);
                return; 
        }

        line_num = 0;
        while (fgets(line,CTLINESIZE,cf) != NULL) {
                /* process a line of a crontab file */
                cursor = 0;
                while(line[cursor] == ' ' || line[cursor] == '\t')
                        cursor++;
                if(line[cursor] == '#')
                        continue;
                e = (struct event *) xmalloc(sizeof(struct event));
                e->etype = CRONEVENT;
                if (line_num == 0) {
                        login_name = get_login_name(u->name);
                        e->of.ct.login = login_name;
                        line_num++;
                        /* only login name on 1st line */
                        if (strchr(line,' ') == NULL) 
                                continue;
                }
                else
                        e->of.ct.login = login_name;

                if ((e->of.ct.minute=next_field(0,59,u)) == NULL) goto badline;
                if ((e->of.ct.hour=next_field(0,23,u)) == NULL) goto badline;
                if ((e->of.ct.daymon=next_field(1,31,u)) == NULL) goto badline;
                if ((e->of.ct.month=next_field(1,12,u)) == NULL) goto badline;
                if ((e->of.ct.dayweek=next_field(0,6,u)) == NULL) goto badline;
                if (line[++cursor] == '\0') {
                        mail(u->name,MSGSTR(MS_EOLN,EOLN),1);
                        goto badline; 
                }
                /* get the command to execute   */
                start = cursor;
again:
                while ((line[cursor]!='%')&&(line[cursor]!='\n')
                    &&(line[cursor]!='\0') && (line[cursor]!='\\')) cursor++;
                if(line[cursor] == '\\') {
                        cursor += 2;
                        goto again;
                }
                e->cmd = xmalloc(cursor-start+1);
                strncpy(e->cmd,line+start,cursor-start);
                e->cmd[cursor-start] = '\0';
                /* see if there is any standard input   */
                if (line[cursor] == '%') {
                        e->of.ct.input = xmalloc(strlen(line)-cursor+1);
                        strcpy(e->of.ct.input,line+cursor+1);
                        for (i=0; i<strlen(e->of.ct.input); i++)
                                if (e->of.ct.input[i] == '%') e->of.ct.input[i] = '\n'; 
                }
                else e->of.ct.input = NULL;
                /* have the event point to it's owner   */
                e->u = u;
                /* insert this event at the front of this user's event list   */
                e->link = u->ctevents;
                u->ctevents = e;
                /* set the time for the first occurance of this event   */
                e->time = next_time(e, (time_t)0);
                /* finally, add this event to the main event list       */
                el_add(e,e->time,u->ctid);
#ifdef DEBUGX
                printf("inserting cron event %s at %ld (%s)",
                        e->cmd,e->time,ctime(&e->time));
#endif
                continue;

badline: 
                free((void *)e); 
        }

        fclose(cf);
#ifdef DEBUGX
        printf("leaving readcron\n");
#endif
}


/*
 * NAME: mail
 * FUNCTION:  mail mails a user a message.
 * NOTES:     format - 1 error message, 2 - normal message
 * RETURNS: none
 */
mail(usrname,mmsg,format)
char *usrname,*mmsg;
int format;
{
        int     p[2]; 

        FILE    *pip; 
        char    *temp, *i,*strrchr ();
        int     child_pid;
        struct runinfo  *rp;

#ifdef DEBUGX
        printf("in mail\n");
#endif
        for (rp = rt; rp < rt + maxrun; rp++) {
                if (rp->pid == 0)
                        break;
        }
        if (rp >= rt + maxrun)
                return;
#if SEC_BASE
        child_pid = cron_mail_setup(0);
        if (child_pid < 0)
                return;
        
        if (child_pid > 0) {
                rp->pid = child_pid;
                rp->que = ZOMB;
                rp->rusr = (struct usr *) 0;
                rp->outfile = (char *) 0;

                /* decremented in idle() */
                running++;

                return;
        }
#endif /* SEC_BASE */
#ifdef _OSF
        temp = xmalloc(strlen(MAIL)+strlen(usrname)+strlen(S_DAEMONCHK)+10);
	sprintf(temp, "%s -r %s %s", MAIL, S_DAEMONCHK, usrname);
        pip =  popen(temp, "w");
#else
/* AIX security enhancement  */
        if (pipe (p) == -1)
                goto    fail;
        if ((child_pid = fork ()) == 0)
        {
                setuid ((uid_t)1);
                setgid ((gid_t)1);
                close (0);
                dup (p[0]);
                close (p[0]);
                close (p[1]);
		/**********************************************
		 * SECURITY : use -r to disable tilde commands.
		 *          : DANGER: uid=1, gid=1
		 **********************************************/
                execl (MAIL, MAIL, "-r", S_DAEMONCHK, usrname, 0);
		msg(MSGSTR(MS_NOEXEC, "cannot exec, errno= %d"),errno);
                exit (1);
        }

        if (child_pid  < 0 )
		msg(MSGSTR(MS_NOFORK, "cannot fork"));

	/*
	 * Add the mail job to the zombie queue so that idle() is
	 * expecting the child to die.
	 */

	rp->pid = child_pid;
	rp->que = ZOMB;
	rp->rusr = (struct usr *) 0;
	rp->outfile = (char *) 0;

        close (p[0]);
        pip = fdopen (p[1], "w"); 
#endif
/* TCSEC Division C class C2 */
        if (pip==NULL) goto    fail;
        if (format == 1) {
                fprintf(pip,MSGSTR(MS_BADTAB, 
                        "Your crontab file has an error in it.\n"));
                i = strrchr(line,'\n');
                if (i != NULL) *i = ' ';
                fprintf(pip, "\t%s\n\t%s\n",line,mmsg);
                fprintf(pip, MSGSTR(MS_IGNORED,
                        "This entry has been ignored.\n")); 
        }
        else fprintf(pip, "Cron: %s\n",msg);
                /* don't want to do pclose because pclose does a wait */
        fclose(pip); 
                /* decremented in idle() */
        running++;
#ifdef _OSF
        free(temp);
#endif
#if SEC_BASE
        cron_mail_finish();
#endif
fail:
#ifdef DEBUGX
        printf("leaving mail\n");
#endif
        return;
}

/*
 * NAME: next_field
 * FUNCTION: next_field returns a pointer to a string which holds 
 *           the next field of a line of a crontab file.
 *           if (numbers in this field are out of range (lower..upper),
 *             or there is a syntax error) then
 *                 NULL is returned, and a mail message is sent to
 *                 the user telling him which line the error was in.
 * RETURNS: none
 */
char *next_field(lower,upper,u)
int lower,upper;
struct usr *u;
{
        char *s;
        int num,num2,start;

#ifdef DEBUGX
        printf("in next_field\n");
#endif
        while ((line[cursor]==' ') || (line[cursor]=='\t')) cursor++;
        start = cursor;
        if (line[cursor] == '\0') {
                mail(u->name,MSGSTR(MS_EOLN,EOLN),1);
                return(NULL); 
        }
        if (line[cursor] == '*') {
                cursor++;
                if ((line[cursor]!=' ') && (line[cursor]!='\t')) {
                        mail(u->name,MSGSTR(MS_UNEXPECT, UNEXPECT),1);
                        return(NULL); 
                }
                s = xmalloc(2);
                strcpy(s,"*");
                return(s); 
        }
        while (TRUE) {
                if (!isdigit(line[cursor])) {
                        mail(u->name,MSGSTR(MS_UNEXPECT, UNEXPECT),1);
                        return(NULL); 
                }
                num = 0;
                do { 
                        num = num*10 + (line[cursor]-'0'); 
                }                       while (isdigit(line[++cursor]));
                if ((num<lower) || (num>upper)) {
                        mail(u->name,MSGSTR(MS_OUTOFBOUND,OUTOFBOUND),1);
                        return(NULL); 
                }
                if (line[cursor]=='-') {
                        if (!isdigit(line[++cursor])) {
                                mail(u->name,MSGSTR(MS_UNEXPECT, UNEXPECT),1);
                                return(NULL); 
                        }
                        num2 = 0;
                        do { 
                                num2 = num2*10 + (line[cursor]-'0'); 
                        }                               while (isdigit(line[++cursor]));
                        if ((num2<lower) || (num2>upper)) {
                                mail(u->name,MSGSTR(MS_OUTOFBOUND,OUTOFBOUND),1);
                                return(NULL); 
                        }
                }
                if ((line[cursor]==' ') || (line[cursor]=='\t')) break;
                if (line[cursor]=='\0') {
                        mail(u->name,MSGSTR(MS_EOLN, EOLN),1);
                        return(NULL); 
                }
                if (line[cursor++]!=',') {
                        mail(u->name,MSGSTR(MS_UNEXPECT, UNEXPECT),1);
                        return(NULL); 
                }
        }
        s = xmalloc(cursor-start+1);
        strncpy(s,line+start,cursor-start);
        s[cursor-start] = '\0';
        return(s);
#ifdef DEBUGX
        printf("leaving next_field\n");
#endif
}

/*
 * NAME: next_time
 * FUNCTION: returns the integer time for the next occurance of event e.
 *           the following fields have ranges as indicated:
 *        PRGM  | min     hour    day of month    mon     day of week
 *        ------|-------------------------------------------------------
 *        cron  | 0-59    0-23        1-31        1-12    0-6 (0=sunday)
 *        time  | 0-59    0-23        1-31        0-11    0-6 (0=sunday)
 * NOTE: this routine is hard to understand.
 * RETURNS: none
 */
time_t next_time(e, tflag)
struct event *e;
time_t tflag;
{

	struct tm *tm, now, later;
        int tm_mon,tm_mday,tm_wday,wday,m,min,h,hr,carry,day,days,
        d1,day1,carry1,d2,day2,carry2,daysahead,mon,yr,db,wd,today;
        time_t t, delta;

#ifdef DEBUGX
        printf("in next_time\n");
#endif
        /* If tflag is non_zero,  this value is used as the current     */
        /* time.  This allows scheduling of events according to what    */
        /* time cron thinks it is,  and prevents events from being      */
        /* scheduled twice for the same time.                           */
        if (tflag)
                t = tflag;
        else
                t = time((long *) 0);
        tm = localtime(&t);

        tm_mon = next_ge(tm->tm_mon+1,e->of.ct.month) - 1;      /* 0-11 */
        tm_mday = next_ge(tm->tm_mday,e->of.ct.daymon);         /* 1-31 */
        tm_wday = next_ge(tm->tm_wday,e->of.ct.dayweek);        /* 0-6  */
        today = TRUE;
        if ( (strcmp(e->of.ct.daymon,"*")==0 && tm->tm_wday!=tm_wday)
            || (strcmp(e->of.ct.dayweek,"*")==0 && tm->tm_mday!=tm_mday)
            || (tm->tm_mday!=tm_mday && tm->tm_wday!=tm_wday)
            || (tm->tm_mon!=tm_mon)) today = FALSE;

        m = tm->tm_min+1;
        min = next_ge(m%60,e->of.ct.minute);
        carry = (min < m) ? 1:0;
        h = tm->tm_hour+carry;
        hr = next_ge(h%24,e->of.ct.hour);
        carry = (hr < h) ? 1:0;
        if ((!carry) && today) {
                /* this event must occur today  */
                if (tm->tm_min>min)
                        delta =(time_t)(hr-tm->tm_hour-1)*HOUR + 
                            (time_t)(60-tm->tm_min+min)*MINUTE;
                else delta = (time_t)(hr-tm->tm_hour)*HOUR +
                        (time_t)(min-tm->tm_min)*MINUTE;

		goto dst_chk;
        }

        min = next_ge(0,e->of.ct.minute);
        hr = next_ge(0,e->of.ct.hour);

        /* calculate the date of the next occurance of this event,
           which will be on a different day than the current day.       */

        /* check monthly day specification      */
        d1 = tm->tm_mday+1;
        day1 = next_ge((d1-1)%days_in_mon(tm->tm_mon,tm->tm_year)+1,e->of.ct.daymon);
        carry1 = (day1 < d1) ? 1:0;

        /* check weekly day specification       */
        d2 = tm->tm_wday+1;
        wday = next_ge(d2%7,e->of.ct.dayweek);
        if (wday < d2) daysahead = 7 - d2 + wday;
        else daysahead = wday - d2;
        day2 = (d1+daysahead-1)%days_in_mon(tm->tm_mon,tm->tm_year)+1;
        carry2 = (day2 < d1) ? 1:0;

        /* based on their respective specifications,
           day1, and day2 give the day of the month
           for the next occurance of this event.        */

        if ((strcmp(e->of.ct.daymon,"*")==0) && (strcmp(e->of.ct.dayweek,"*")!=0)) {
                day1 = day2;
                carry1 = carry2; 
        }
        if ((strcmp(e->of.ct.daymon,"*")!=0) && (strcmp(e->of.ct.dayweek,"*")==0)) {
                day2 = day1;
                carry2 = carry1; 
        }

        yr = tm->tm_year;
        if ((carry1 && carry2) || (tm->tm_mon != tm_mon)) {
                /* event does not occur in this month   */
                m = tm->tm_mon+1;
                mon = next_ge(m%12+1,e->of.ct.month)-1;         /* 0..11 */
                carry = (mon < m) ? 1:0;
                yr += carry;
                /* recompute day1 and day2      */
                day1 = next_ge(1,e->of.ct.daymon);
                db = days_btwn(tm->tm_mon,tm->tm_mday,tm->tm_year,mon,1,yr) + 1;
                wd = (tm->tm_wday+db)%7;
                /* wd is the day of the week of the first of month mon  */
                wday = next_ge(wd,e->of.ct.dayweek);
                if (wday < wd) day2 = 1 + 7 - wd + wday;
                else day2 = 1 + wday - wd;
                if ((strcmp(e->of.ct.daymon,"*")!=0) && (strcmp(e->of.ct.dayweek,"*")==0))
                        day2 = day1;
                if ((strcmp(e->of.ct.daymon,"*")==0) && (strcmp(e->of.ct.dayweek,"*")!=0))
                        day1 = day2;
                day = (day1 < day2) ? day1:day2; 
        }
        else { /* event occurs in this month    */
                mon = tm->tm_mon;
                if (!carry1 && !carry2) day = (day1 < day2) ? day1 : day2;
                else if (!carry1) day = day1;
                else day = day2;
        }

        /* now that we have the min,hr,day,mon,yr of the next
           event, figure out what time that turns out to be.    */

        days  = days_btwn(tm->tm_mon,tm->tm_mday,tm->tm_year,mon,day,yr);
        delta = (time_t)(23-tm->tm_hour)*HOUR + (time_t)(60-tm->tm_min)*MINUTE
              + (time_t)hr*HOUR + (time_t)min*MINUTE + (time_t)days*DAY;

    dst_chk:

	/*
	 * normally, (later - now) == (t+delta)-t == delta.
	 * when we cross a Daylight Savings Time change, this is
	 * no longer true.  Thus, we need to determine the value of t, t',
	 * for which later(t') - now(t) == delta.
	 */
	now   = *localtime(&t);
	t    += delta;
	later = *localtime(&t);

	if (now.tm_isdst != later.tm_isdst) {
		/*
		 * we've crossed a daylight savings change.
		 * adjust for it.
		 */
		static int once, dst_delta, abs_dst_delta;

		if (!once) {
			struct tm *gm;
			register gmt_delta;
			int zero = 0;

			++once;

			gm = localtime(&zero);		/* the epoch	*/
			gmt_delta = mktime(gm);		/* GMT delta	*/
			gm->tm_isdst = 1;		/* lie a bit	*/

			dst_delta = gmt_delta - mktime(gm);
			abs_dst_delta = (dst_delta < 0) ?
				-dst_delta : dst_delta;
		}

		/*
		 * take dst_delta into account unless the old delta was
		 * <= the dst change.
		 * Otherwise, things that run hourly or less get confused.
		 */
		if (delta > abs_dst_delta) {
			t += dst_delta * (later.tm_isdst ? -1 : 1);
		}
	}

	return t - now.tm_sec;
}

#define DUMMY   100
/*
 * NAME: next_ge
 * FUNCTION: list is a character field as in a crontab file;
 *                for example: "40,20,50-10"
 *           next_ge returns the next number in the list that is
 *           greater than or equal to current.
 *           if no numbers of list are >= current, the smallest
 *           element of list is returned.
 * NOTE: current must be in the appropriate range.   
 * RETURNS: none
 */
next_ge(current,list)
int current;
char *list;
{

        char *ptr;
        int n,n2,min,min_gt;

#ifdef DEBUGX
/*
        printf("in next_ge\n");
*/
#endif
        if (strcmp(list,"*") == 0) return(current);
        ptr = list;
        min = DUMMY; 
        min_gt = DUMMY;
        while (TRUE) {
                if ((n=(int)num(&ptr))==current) return(current);
                if (n<min) min=n;
                if ((n>current)&&(n<min_gt)) min_gt=n;
                if (*ptr=='-') {
                        ptr++;
                        if ((n2=(int)num(&ptr))>n) {
                                if ((current>n)&&(current<=n2))
                                        return(current); 
                        }
                        else {  /* range that wraps around */
                                if (current>n) return(current);
                                if (current<=n2) return(current); 
                        }
                }
                if (*ptr=='\0') break;
                ptr += 1; 
        }
        if (min_gt!=DUMMY) return(min_gt);
        else return(min);
#ifdef DEBUGX
        printf("leaving next_ge\n");
#endif
}

/*
 * NAME: del_atjobs
 * FUNCTION:      delete at jobs event.
 *                If the user (usrname) doesn't have any events, then delete
 *               the data area of the user and remove the user entry from the
 *               user list.
 * RETURNS: none
 */
del_atjob(name,usrname)
char    *name;
char    *usrname;
{

        struct  event   *e, *eprev;
        struct  usr     *u;

#ifdef DEBUGX
        printf("in del_atjob\n");
#endif
        if((u = find_usr(usrname)) == NULL)
                return;
        e = u->atevents;
        eprev = NULL;
        while(e != NULL)
                if(strcmp(name,e->cmd) == 0) {
                        if(next_event == e)
                                next_event = NULL;
                        if(eprev == NULL)
                                u->atevents = e->link;
                        else
                                eprev->link = e->link;
                        el_remove(e->of.at.eventid, 1);
                        free((void *)e->cmd);
                        free((void *)e);
                        break;
                } else {
                        eprev = e;
                        e = e->link;
                }
        if(!u->ctexists && u->atevents == NULL) {
#ifdef DEBUGX
                printf("%s removed from usr list\n",usrname);
#endif
                if(ulast == NULL)
                        uhead = u->nextusr;
                else
                        ulast->nextusr = u->nextusr;
                free((void *)u->name);
                free((void *)u->home);
#if SEC_MAC || SEC_NCAV
                cron_release_ir(u->seclevel_ir);
#endif
                free((void *)u);
        }
#ifdef DEBUGX
        printf("leaving del_atjob\n");
#endif
}

/*
 * NAME: del_ctab
 * FUNCTION:     delete crontabs event.
 *               If the user (usrname) doesn't have any events, then delete
 *               the data area of the user and remove the user entry from the
 *               user list.
 * RETURNS: none
 */
del_ctab(name)
char    *name;
{

        struct  usr     *u;

#ifdef DEBUGX
        printf("in del_ctab\n");
#endif
        if((u = find_usr(name)) == NULL)
                return;
        rm_ctevents(u);
        el_remove(u->ctid, 0);
        u->ctid = 0;
        u->ctexists = 0;
        if(u->atevents == NULL) {
#ifdef DEBUGX
                printf("%s removed from usr list\n",name);
#endif
                if(ulast == NULL)
                        uhead = u->nextusr;
                else
                        ulast->nextusr = u->nextusr;
                free((void *)u->name);
                free((void *)u->home);
#if SEC_MAC || SEC_NCAV
                cron_release_ir(u->seclevel_ir);
#endif
                free((void *)u);
        }
#ifdef DEBUGX
        printf("leaving del_ctab\n");
#endif
}

/*
 * NAME: rm_ctevents
 * FUNCTION:     delete crontabs event.
 *               free the crontab events entry
 * RETURNS: none
 */
rm_ctevents(u)
struct usr *u;
{
        struct event *e2,*e3;

#ifdef DEBUGX
        printf("in rm_ctevents\n");
#endif
        /* see if the next event (to be run by cron)
           is a cronevent owned by this user.           */
        if ( (next_event!=NULL) && 
            (next_event->etype==CRONEVENT) &&
            (next_event->u==u) )
                next_event = NULL;
        e2 = u->ctevents;
        while (e2 != NULL) {
                free((void *)e2->cmd);
                free((void *)e2->of.ct.login);
                free((void *)e2->of.ct.minute);
                free((void *)e2->of.ct.hour);
                free((void *)e2->of.ct.daymon);
                free((void *)e2->of.ct.month);
                free((void *)e2->of.ct.dayweek);
                if (e2->of.ct.input != NULL) free((void *)e2->of.ct.input);
                e3 = e2->link;
                free((void *)e2);
                e2 = e3; 
        }
        u->ctevents = NULL;
#ifdef DEBUGX
        printf("leaving  rm_ctevents\n");
#endif
}
/*
 * NAME: find_user
 * FUNCTION: find the user entry(uname) from the user list.(uhead)
 * RETURNS: none
 */
struct usr *find_usr(uname)
char *uname;
{
        struct usr *u;

#ifdef DEBUGX
        printf("in find_usr\n");
#endif
        u = uhead;
        ulast = NULL;
        while (u != NULL) {
#if SEC_MAC
                if (cron_id_match(u->name, uname, u->seclevel_ir))
                        return u;
#else
                if (strcmp(u->name,uname) == 0) return(u);
#endif
                ulast = u;
                u = u->nextusr; 
        }
        return(NULL);
#ifdef DEBUGX
        printf("leaving find_usr\n");
#endif
}

/*
 * NAME: ex
 * FUNCTION: execute the events
 *           o check the limits
 *           o fork the child process
 *           o Parent process (return to the main )
 *           o child process
 *              If at event, open jobfile for stdin for input the shell script.
 *              If cron event, create a temporary input file for stdin.
 *              create a temporary output file for stdout(croutxxx)
 *              If at event, execute the job(setpenv).(read the cmd from stdin)
 *              If cron event, execute the job(setpenv).(get the cmd from event)
 * RETURNS: none
 */
ex(e)
struct event *e;
{

        register i,j;
        short sp_flag;
        int fd;
        pid_t rfork;
        char *at_cmdfile, *cron_infile;
        char *mktemp();
#if SEC_MAC || SEC_NCAV
        char *cron_tempnam();
#endif
        struct stat buf;
        struct queue *qp;
        struct runinfo *rp;
        char cmd[BUFSIZ];
        char tmpbuf[BUFSIZ];
        long    t = time ((long *) 0);
        char **pcred, **penv;

#ifdef DEBUGX
        printf("in ex\n");
#endif
        if (e->etype == SYNCEVENT)
        {
#ifdef DEBUGX
                msg("sync-ing disks");
#endif 
                /*
		 * We don't do syncs every minute any more.
		sync();
		 */
                return;
        }

        qp = &qt[e->etype];     /* set pointer to queue defs */
        jobtype = e->etype;
        if(qp->nrun >= qp->njob) {
                msg(MSGSTR(MS_QLIMAX, "%c queue max run limit reached"),e->etype+'a');
                resched(qp->nwait);
                return;
        }
        for(rp=rt; rp < rt+maxrun; rp++) {
                if(rp->pid == 0)
                        break;
        }
        if(rp >= rt+maxrun) {
                msg(MSGSTR(MS_MAXRUN, "MAXRUN (%d) procs reached"),maxrun);
                resched(qp->nwait);
                return;
        }
#ifdef ATLIMIT
        if((e->u)->uid != 0 && (e->u)->aruncnt >= ATLIMIT) {
                msg(MSGSTR(MS_ATLIMIT, "ATLIMIT (%d) reached for uid %d"),
                    ATLIMIT,(e->u)->uid);
                resched(qp->nwait);
                return;
        }
#endif
#ifdef CRONLIMIT
        if((e->u)->uid != 0 && (e->u)->cruncnt >= CRONLIMIT) {
                msg(MSGSTR(MS_CRONLIMIT, "CRONLIMIT (%d) reached for uid %d"),
                    CRONLIMIT,(e->u)->uid);
                resched(qp->nwait);
                return;
        }
#endif
#if SEC_MAC || SEC_NCAV
        rp->outfile = cron_tempnam(TMPDIR, PFX, e->u->seclevel_ir);
#else
        rp->outfile = tempnam(TMPDIR,PFX);
#endif
        if((rfork = fork()) == -1) {
                msg(MSGSTR(MS_NOFORK, "cannot fork"));
                resched(wait_time);
                sleep((unsigned)30);
                return;
        }
        if(rfork) {     /* parent process */
                ++qp->nrun;
                ++running;
                rp->pid = rfork;
                rp->que = e->etype;
#ifdef ATLIMIT
                if(e->etype != CRONEVENT)
                        (e->u)->aruncnt++;
#endif
#if ATLIMIT && CRONLIMIT
                else
                        (e->u)->cruncnt++;
#else
#ifdef CRONLIMIT
                if(e->etype == CRONEVENT)
                        (e->u)->cruncnt++;
#endif
#endif
                rp->rusr = (e->u);
                return;
        }
#if SEC_BASE
        cron_close_files();
#else
        endpwent();
        kleenup (0, 0, 0);
#endif
        if (e->etype != CRONEVENT ) {
#ifdef DEBUGX
        printf("in ex not CRONEVENT\n");
#endif
                /* open jobfile as stdin to shell */
#if SEC_MAC
                /* the +16 below reflects the space needed to hold a
                 * mld subdirectory name plus trailing slash */
                at_cmdfile = xmalloc(strlen(ATDIR)+strlen(e->cmd)+16+2);
                (void) cron_jobname(cwd, "", ATDIR, e->cmd, at_cmdfile);
#else
                at_cmdfile = xmalloc(strlen(ATDIR)+strlen(e->cmd)+2);
                strcat(strcat(strcpy(at_cmdfile,ATDIR),"/"),e->cmd);
                chdir(ATDIR);
#endif
                if (stat(at_cmdfile,&buf)) exit(1);
                if (!(buf.st_mode&ISUID)) {
                        /* if setuid bit off, original owner has 
                           given this file to someone else      */
                        unlink(at_cmdfile);
                        exit(1); 
                }
                if (open(at_cmdfile,O_RDONLY) == -1) {
                        mail((e->u)->name,MSGSTR(MS_BADJOBOPEN, BADJOBOPEN),2);
                        unlink(at_cmdfile);
                        exit(1); 
                }
                setbuf (stdin, NULL);   /* Don't buffer */
                unlink(at_cmdfile);     /* remove at job file */
        }
#if SEC_MAC || SEC_NCAV
        cron_set_user_environment(e->u->seclevel_ir, e->u->name, e->u->uid);
#else
#if SEC_BASE
        cron_set_user_environment(e->u->name, e->u->uid);
#endif
#endif
#ifdef _OSF
        /* set correct user and group identification    */
        if ((setgid((e->u)->gid)<0) || (setuid((e->u)->uid)<0))
                exit(1);
#endif
#if SEC_BASE
        cron_adjust_privileges();
#endif
        sp_flag = FALSE;
        if (e->etype == CRONEVENT) {
#ifdef DEBUGX
        printf("in ex CRONEVENT\n");
#endif
                /* check for standard input to command  */
                if (e->of.ct.input != NULL) {
                        cron_infile = mktemp(TMPINFILE);
                        if ((fd=creat(cron_infile,(mode_t)INMODE)) == -1) {
                                mail((e->u)->name,MSGSTR(MS_NOSTDIN, NOSTDIN),2);
                                exit(1); 
                        }
                        if (write(fd,e->of.ct.input,(unsigned)strlen(e->of.ct.input))
                            != strlen(e->of.ct.input)) {
                                mail((e->u)->name,MSGSTR(MS_NOSTDIN, NOSTDIN),2);
                                unlink(cron_infile);
                                exit(1); 
                        }
                        close(fd);
                        /* open tmp file as stdin input to sh   */
                        if (open(cron_infile,O_RDONLY)==-1) {
                                mail((e->u)->name,MSGSTR(MS_NOSTDIN, NOSTDIN),2);
                                unlink(cron_infile);
                                exit(1); 
                        }
                        unlink(cron_infile); 
                } /* if stdin for this event is not NULL */
                else if (open("/dev/null",O_RDONLY)==-1) {
                        open("/",O_RDONLY);     /* can't fail */
                        sp_flag = TRUE;         /* close it later */
                }
        } /* if CRONEVENT */
#if SEC_MAC
        forcepriv(SEC_MULTILEVELDIR);
#endif
        if (creat(rp->outfile,(mode_t)OUTMODE)!=-1) {
				/* Give ownership of output file to user */
		fchown(1, (e->u)->uid, (e->u)->gid);
                dup(1);         /* stdout, stderr go to outfle */
	} else if (open("/dev/null",O_WRONLY)!=-1) 
                dup(1);         /* stdout, stderr go to null   */
        if (sp_flag)
                close(0);       /* stdin got opened on "/", so close it */

#ifdef SEC_BASE
        strcat(homedir,(e->u)->home);
        strcat(logname,(e->u)->name);
        environ = envinit;
        if (chdir((e->u)->home)==-1) {
                mail((e->u)->name,MSGSTR(MS_CANTCDHOME, CANTCDHOME),2);
                exit(1); 
        }
        if((e->u)->uid != 0)
                nice(qp->nice);
        if (e->etype == CRONEVENT) {
                execl(SHELL,"sh","-c",e->cmd,0);
		msg(MSGSTR(MS_NOEXEC, "cannot exec, errno= %d"),errno);
		}
        else /* type == ATEVENT */ {
                execl(SHELL,"sh",0);
		msg(MSGSTR(MS_NOEXEC, "cannot exec, errno= %d"),errno);
		}
#else            /* AIX security enhancement */
        privilege(PRIV_ACQUIRE);
	nice(qp->nice);
        if (e->etype != CRONEVENT) {
#ifdef DEBUGX
        printf("in ex not CRONEVENT\n");
#endif
                /* make sure you point to the pcred line */
                pcred = getarray();
		/*
		 * Change the process credentials to those of the cronjob's
		 * owner.
		 */

		if (setpcred((e->u)->name, NULL)) {
			char    buf[BUFSIZ];

			/*
			 * The call to setpcred() failed.  Report this error
			 * to "root" as the most likely cause is a corrupt user
			 * database or non-existent user with an active crontab
			 * file.
			 */
 
			sprintf (buf, MSGSTR (MS_SETPCRED,
				"Cannot set the process credentials for user %s.\n"),
				e->u->name);
			mail ("root", buf, 2);
			exit (1);
		}
                auditproc(0, AUDIT_EVENTS, AUDIT_CLASS,strlen(AUDIT_CLASS)+2);
                auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
                penv = getarray();
                setpriv(PRIV_SET|PRIV_BEQUEATH,NULL,sizeof(priv_t));
                auditwrite(
                        CRON_START,
                        0,
                        AT_JOB,
                        strlen(AT_JOB)+1,
                        at_cmdfile, 
                        strlen(at_cmdfile)+1,
                        ctime(&t),
                        strlen(ctime(&t)),
                        0
                );
		if (chdir((e->u)->home)==-1) {
			mail((e->u)->name,MSGSTR(MS_CANTCDHOME, CANTCDHOME),2);
			exit(1); 
		}
                setpenv(NULL,PENV_RESET, penv, "$SHELL");
                mail ((e->u)->name, CANTEXECSH, 2);
                exit (1);
        }

	/*
	 * Change the process credentials to those of the cronjob's
	 * owner.
	 */

        if (setpcred((e->u)->name, NULL)) {
		char	buf[BUFSIZ];

		/*
		 * The call to setpcred() failed.  Report this error
		 * to "root" as the most likely cause is a corrupt user
		 * database or non-existent user with an active crontab
		 * file.
		 */

		sprintf (buf, MSGSTR (MS_SETPCRED,
			"Cannot set the process credentials for user %s.\n"),
			e->u->name);
		mail ("root", buf, 2);
		exit (1);
	}
        auditproc(0, AUDIT_EVENTS, AUDIT_CLASS,strlen(AUDIT_CLASS)+2);
        auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
        setpriv(PRIV_SET|PRIV_BEQUEATH,NULL, sizeof(priv_t));
        auditwrite(
                CRON_START,              
                0,
                CRON_JOB,
                strlen(CRON_JOB)+1,
                e->cmd,
                strlen(e->cmd)+1,
                ctime(&t),
                strlen(ctime(&t)),
                0
        );
        sprintf(cmd,"/usr/bin/bsh -c %s", e->cmd);
        setpenv(NULL,PENV_INIT | PENV_NOPROF, NULL, cmd);
#endif
        mail ((e->u)->name, CANTEXECSH, 2);
#ifdef DEBUGX
        printf("leaving ex\n");
#endif
        exit(1);
}

/*
 * NAME: idle
 * FUNCTION: idle for a specified period.
 *           o wait for the specified time. This function divides 
 *             the specified time( tyme) and set the alarm.
 *              If some job is running then wait for until it is finised.
 *                If not, wait for the message(queue)
 *              If no job is running wait for the message(queue), too.  
 * RETURNS: none
 */
idle(tyme)
long tyme;
{

        long t;
        time_t  now;
        pid_t   pid;
        int     prc;
        long    alm;
        struct  runinfo *rp;

        t = tyme;
        while(t > 0L) {
#ifdef DEBUGX
		printf("in idle before msg_wait 1\n");
#endif
		if (msg_wait())
                        return;
                if(running) {
                        if(t > wait_time)
                                alm = wait_time;
                        else
                                alm = t;
#ifdef DEBUGX
                        printf("in idle - setting alarm for %ld sec\n",alm);
#endif
                        alarm((unsigned) alm);
                        pid = wait(&prc);
                        alarm((unsigned)0);
#ifdef DEBUGX
                        printf("in idle after wait pid = %d",pid);
#endif
                        if(pid == -1) {
#ifdef DEBUGX
                        printf("in idle before msg_wait 2\n");
#endif
                                if(msg_wait())
                                        return;
                        } else {
#ifdef DEBUGX
                        printf("in idle  pid != -1\n");
#endif
                                for(rp=rt;rp < rt+maxrun; rp++)
                                        if(rp->pid == pid)
                                                break;
                                if(rp >= rt+maxrun) {
                                        msg(MSGSTR(MS_BADPID, "unexpected pid returned %d (ignored)"),pid);
                                        /* incremented in mail() */
                                        running--;
                                }
                                else
                                        if(rp->que == ZOMB) {
                                                running--;
                                                rp->pid = 0;
#if SEC_BASE
                                                if (rp->outfile) {
                                                        unlink(rp->outfile);
                                                        free(rp->outfile);
                                                }
#else
                                                unlink(rp->outfile);
                                                free(rp->outfile);
#endif
                                        }
                                        else
                                                cleanup(rp,prc);
                        }
                }
                now = time((time_t *) 0);
                t = next_event->time - now;
        }
}

/*
 * NAME: cleanup
 * FUNCTION: cleanup the standard output of the executed event.
 * RETURNS: none
 */
cleanup(pr,rc)
struct  runinfo *pr;
{

        int     fd;
        struct  usr     *p;
        struct  stat    buf;
        long    t = time ((long *) 0);
        char    *s,ss[20];
        int n;

#ifdef DEBUGX
        printf("in cleanup rc = %d \n",rc);
        printf("pid = %ld \n",pr->pid);
#endif
        sprintf(ss,"%ld\0",pr->pid);
        auditwrite(
                CRON_FINISH,
                /* return code from the child process */
                rc = ((rc << 16) >> 8),
                (pr->rusr)->name,               /* process owner name */
                strlen((pr->rusr)->name)+1,
                ss,                        /* process id */
                strlen(ss)+1,
                ctime(&t),                      /* current time */
                strlen(ctime(&t)),
                0
        );
        --qt[pr->que].nrun;
        pr->pid = 0;
        --running;
        p = pr->rusr;
#if SEC_MAC || SEC_NCAV
        cron_setlevel(p->seclevel_ir);
#endif
#ifdef ATLIMIT
        if(pr->que != CRONEVENT)
                --p->aruncnt;
#endif
#if ATLIMIT && CRONLIMIT
        else
                --p->cruncnt;
#else
#ifdef CRONLIMIT
        if(pr->que == CRONEVENT)
                --p->cruncnt;
#endif
#endif
        if(!stat(pr->outfile,&buf)) {
                if(buf.st_size > 0) {
                        if((fd=open(pr->outfile,O_WRONLY)) == -1)
                                mail(p->name,MSGSTR(MS_STDOUTERR,STDOUTERR),2);
                        else {
                                lseek(fd,(long) 0,2);
                                /* there may be a bug in catgets
                                        to process MS_STDERRMSG 
                                */
				n = strlen(MSGSTR(MS_STDERRMSG, STDERRMSG))+1;
				s = xmalloc(n);
				strcpy(s,(MSGSTR(MS_STDERRMSG, STDERRMSG)));
#ifdef DEBUGX
printf("print stdout\n");
printf(" catd = %d \n",catd);
printf("ss = %s length = %d\n",s,strlen(s));
                                printf(MSGSTR(MS_STDERRMSG, STDERRMSG));
#endif
                                write(fd,s,n);
                                close(fd);
                                free(s);
/* AIX security enhancement  */
                                /* mail user stdout and stderr */
#if SEC_MAC
                                cron_mail_line(line, MAIL, p->name,
                                                pr->outfile);
#else
                                sprintf(line,"%s -r %s %s < \"%s\"\n",
                                        MAIL,S_DAEMONCHK,p->name,pr->outfile);
#endif
                                if((pr->pid = fork()) == 0) {
#if SEC_BASE
                                        (void) cron_mail_setup(1);
#endif
#ifdef _OSF
                                        execl(SHELL,"sh","-c",line,0);
			msg(MSGSTR(MS_NOEXEC, "cannot exec, errno= %d"),errno);
                                        exit(127);
#else
                                        close (0);
                                        open (pr->outfile, 0);
                                        setuid ((uid_t)1);
                                        setgid ((gid_t)1);
					/**************************************
					 * SECURITY :
					 *     use -r to disable tilde commands
					 *     DANGER: uid=1, gid=1
					 **************************************/
                                        execl (MAIL, MAIL, "-r", S_DAEMONCHK,
						p->name, 0);
			msg(MSGSTR(MS_NOEXEC, "cannot exec, errno= %d"),errno);
                                        exit (1);
#endif
                                }
				/*
				 * Verify that the "mail" process has been
				 * started.  Issue a message if the fork
				 * has failed, otherwise increment the count
				 * of outstanding children.
				 */
				if (pr->pid < 0) {
					msg(MSGSTR(MS_NOFORK,"cannot fork"));
					pr->pid = 0;
				} else {
					pr->que = ZOMB;
					running++;
				}
                        }
                }
        }
	/*
	 * If "pr->que" has been set to "ZOMB", then the following
	 * unlink() and free() will be done later in idle().
	 */
	if(pr->que != ZOMB) {
		unlink(pr->outfile);
		free((void *)pr->outfile);
	}
}

#define MSGSIZE sizeof(struct message)
/*
 * NAME: msg_wait
 * FUNCTION: wait for the message to the FIFO queue.
 * RETURNS: none
 */
msg_wait()
{

        long    t;
        time_t  now;
        struct  stat msgstat;
        struct  message *pmsg;
        int     cnt;
        char    *s1,*s2;

#ifdef DEBUGX
        printf("in msg_wait\n");
#endif
        if(fstat(msgfd,&msgstat) != 0) 
                crabort(MSGSTR(MS_NOFIFO1,"cannot stat fifo queue"),FIFO);
        if(msgstat.st_size == 0 && running)
                return(0);
        if(next_event == NULL)
                t = INFINITY;
        else {
                now = time((long *) 0);
                t = next_event->time - now;
                if(t <= 0L)
                        t = 1L;
        }
#ifdef DEBUGX
        printf("in msg_wait - setting alarm for %ld sec\n", t);
#endif
#if SEC_MAC || SEC_NCAV
        pmsg = (struct message *) cron_set_message(sizeof *pmsg);
        if (pmsg == (struct message *) 0) {
                msg(MSGSTR(MS_MSGALLOC, "can't allocate message buffer"));
                notexpired = 1;
                return 1;
        }
#endif
        alarm((unsigned) t);
#if SEC_MAC || SEC_NCAV
        if (!cron_read_message(msgfd, (char *) pmsg, sizeof *pmsg, &cnt))
#else
        pmsg = &msgbuf;
        if((cnt=read(msgfd,(char *) pmsg,MSGSIZE)) != MSGSIZE)
#endif
        {
                if(errno != EINTR) {
                        perror("read");
                        notexpired = 1;
                }
                if(next_event == NULL)
                        notexpired = 1;
                return(1);
        }
        alarm((unsigned)0);
        if(pmsg->etype != '\0') {
                switch(pmsg->etype) {
                case AT:
                        if(pmsg->action == DELETE)
                                del_atjob(pmsg->fname,pmsg->logname);
                        else
                                mod_atjob(pmsg->fname);
                        break;
                case CRON:
                        if(pmsg->action == DELETE)
                                del_ctab(pmsg->fname);
                        else
                                mod_ctab(pmsg->fname);
                        break;
                default:
                        msg(MSGSTR(MS_BADFMT, "message received - bad format"));
                        break;
                }
                if (next_event != NULL) {
                        if (next_event->etype == CRONEVENT)
                                el_add(next_event,next_event->time,(next_event->u)->ctid);
                        else /* etype == ATEVENT */
                                el_add(next_event,next_event->time,next_event->of.at.eventid);
                        next_event = NULL;
                }
                fflush(stdout);
                pmsg->etype = '\0';
                notexpired = 1;
                return(1);
        }
}

/*
 * NAME: timeout
 * FUNCTION: reset the alarm
 * RETURNS: none
 */
timeout(void)
{
#ifdef DEBUGX
        printf("in timeout\n");
#endif
        signal(SIGALRM, (void (*)(int))timeout);
}

/*
 * NAME: crabort
 * FUNCTION: crabort handles exits out of cron 
 * RETURNS: none
 */
crabort(mssg,s1)
char *mssg;
char *s1;
{
        int c;
        char *s,*ss;
#ifdef DEBUGX
        printf("in crabort\n");
#endif
        s = xmalloc(strlen(mssg)+strlen(s1));
        sprintf(s,mssg,s1);

        /* write error msg to console */
        if ((c=open(CONSOLE,O_WRONLY))>=0) {
                ss = xmalloc(strlen(MSGSTR(MS_ABORT, "cron aborted: "))+1);
                strcpy(ss,MSGSTR(MS_ABORT, "cron aborted: "));
                write(c,ss,(unsigned)strlen(ss));
                write(c,s,(unsigned)strlen(s));
                write(c,"\n",(unsigned)1);
                close(c); 
                free(ss);
        }
        msg(s);
        msg(MSGSTR(MS_ABORTHDR, "******* CRON ABORTED ********"));
        free(s);
        exit(1);
}

/*
 * NAME: msg
 * FUNCTION: print out the message with time stamp
 * RETURNS: none
 */
msg(fmt,a,b)
char *fmt;
{

        time_t  t;
        char    timestr[128];

#ifdef DEBUGX
        printf("in msg fmt %s \n",fmt);
#endif
        t = time((long *) 0);
        printf("! ");
        printf(fmt,a,b);
/*      printf(" %s",ctime(&t));        */
        strftime(timestr,128,nl_langinfo(D_T_FMT),localtime(&t));
        printf(" %s\n",timestr);
        fflush(stdout);
}

/*
 * NAME: resched
 * FUNCTION: reschedule the job to run at a later time(delay time)
 * RETURNS: none
 */
resched(delay)
int     delay;
{
        time_t  nt;

#ifdef DEBUGX
        printf("in resched\n");
#endif
        /* run job at a later time */
        nt = next_event->time + delay;
        if(next_event->etype == CRONEVENT) {
                next_event->time = next_time(next_event, (time_t)0);
                if(nt < next_event->time)
                        next_event->time = nt;
                el_add(next_event,next_event->time,(next_event->u)->ctid);
                delayed = 1;
                msg(MSGSTR(MS_CRRESCHED,"rescheduling a cron job"));
                return;
        }
        add_atevent(next_event->u, next_event->cmd, nt);
        msg(MSGSTR(MS_ATRESCHED, "rescheduling at job"));
}

#define QBUFSIZ         80
/*
 * NAME: quedefs
 * FUNCTION: read or check the quedefs file.
 * ENVIRONMENT:    action DEFAULT - set up default queuedefinitions.
 *                        other   - read the quedef file and set up the queue
 *                                  def data.(qbuf) 
 * RETURNS: none
 */
quedefs(action)
int     action;
{
        register i;
        int     j;
        char    name[PATH_MAX+1];
        char    qbuf[QBUFSIZ];
        FILE    *fd;

#ifdef DEBUGX
        printf("in quedefs\n");
#endif
        /* set up default queue definitions */
        for(i=0;i<NQUEUE;i++) {
                qt[i].njob = qd.njob;
                qt[i].nice = qd.nice;
                qt[i].nwait = qd.nwait;
        }
        if(action == DEFAULT)
                return;
        if((fd = fopen(QUEDEFS,"r")) == NULL) {
                msg(MSGSTR(MS_NOQUEDEFS, "cannot open quedefs file"));
                msg(MSGSTR(MS_NOQDEFS2, "using default queue definitions"));
                return;
        }
        while(fgets(qbuf, QBUFSIZ, fd) != NULL) {
                if((j=qbuf[0]-'a') < 0 || j >= NQUEUE || qbuf[1] != '.')
                        continue;
                i = 0;
                while(qbuf[i] != '\0')
                {
                        name[i] = qbuf[i];
                        i++;
                }
                parsqdef(&name[2]);
                qt[j].njob = qq.njob;
                qt[j].nice = qq.nice;
                qt[j].nwait = qq.nwait;
        }
        fclose(fd);
}

/*
 * NAME: parsqdef
 * FUNCTION:       set the qbuf date from the quedef entry.
 * ENVIRONMENT:    name - format EventType.[(Jobs)j][(Nice)n][(Wait)w]
 *                              ex.  a.2j2n90w
 * RETURNS: none
 */
parsqdef(name)
char *name;
{
        register i;

#ifdef DEBUGX
        printf("in parsqdef\n");
#endif
        qq = qd;
        while(*name) {
                i = 0;
                while(isdigit(*name)) {
                        i *= 10;
                        i += *name++ - '0';
                }
                switch(*name++) {
                case JOBF:
                        qq.njob = i;
                        break;
                case NICEF:
                        qq.nice = i;
                        break;
                case WAITF:
                        qq.nwait = i;
                        break;
                }
        }
}

/*
 * NAME: sync_event
 * FUNCTION: add a special event for doing syncs
 *           to cron's main event list. This guarantees that a sync
 *            will occur once per minute.
 *            This code obviates 'sync' entry in root's crontab file.
 * RETURNS: none
 */
sync_event()
{
    struct event *e;

#ifdef DEBUGX
        printf("in sync_event\n");
#endif
    e = (struct event *) xmalloc(sizeof(struct event));
    e->etype = SYNCEVENT;

     /* Set up a once-a-minute event.
      */
    e->of.ct.minute  = "*";
    e->of.ct.hour    = "*";
    e->of.ct.daymon  = "*";
    e->of.ct.month   = "*";
    e->of.ct.dayweek = "*";

     /* Since sync event is handled specially,
      * many elements of the event structure are unused.
      * In particular, there is no usr structure because
      * the event isn't associated with any particular user.
      */
    e->u = NULL;                /* no user structure */
    e->cmd = NULL;              /* no command */
    e->of.ct.input = NULL;      /* no standard input */
    e->link = NULL;             /* no other events */

     /* Set the time for the first occurence of sync event,
      * then add it to the main event list.
      */
    e->time = next_time(e, (time_t)0);
    el_add(e, e->time, 0);      /* event id not used: just set to '0' */
}


/*
 * NAME: get_login_name
 * FUNCTION: process the first line of crontab file whether or not there
 *           is a login id there
 * RETURNS: none
 */
char *get_login_name(name)
char *name;
{
        char *s;
        
#ifdef DEBUGX
        printf("in get_login_name\n");
#endif
        /*
         * process login id  - strchr will return NULL if only 1 
         * token on line. Meaning login id else login id left off
         * so use the name from the usr struct.
         */

        if (strchr(line,' ') == NULL) {  /* only 1 token */
                s = xmalloc(strlen(line)+1);
                strcpy(s,line);
                strcat(s,NULL);

        }
        else {
                s = xmalloc(strlen(name)+1);
                strcpy(s,name);
                strcat(s,NULL);
        }
        return(s);
}

/*
 * NAME: getarray
 * FUNCTION: get a null separated, double-null terminated array
 *           used for pcred and penv lists
 * RETURNS: none
 */
char ** 
getarray()
{       
        register char  **listp,**retp;
        char    line[ARG_MAX], *linep, *lastnull;
        int     offset=0;
        register int argcount=0;
        int c;
        register int i;

#ifdef DEBUGX
        printf("in getarray\n");
#endif
        for (i=0; i < ARG_MAX; i++)
                line[i] = '\0';
        linep = &line[0];

        setbuf(stdin,NULL);
        while ((c = fgetc(stdin)) != EOF)
        {
                if (c == '\n')  /* end of list */
                {   

        /*                                                      */
        /* D18069 - Each line in the jobfile created by at has  */
        /* a \0\n as the terminating characters. Look for       */
        /* both to determine the end-of-line because the user   */
        /* could have an environment variable with an embedded  */
        /* new-line                                             */
        /*                                                      */
                if (argcount && (*(linep-1)=='\0'))
                        break;
                }

                if (offset >= ARG_MAX)
                {
                        if ((lastnull - linep) < (ARG_MAX - 2))
                                *(++lastnull)='\0';
                        else
                                *(--lastnull)='\0';
                        break;
                }

                if (c == '\0')  /* found null separator */
                {
                        lastnull=linep;
                        argcount++;  /* update count for malloc below */
                }
                offset++;
                *linep++ = c;   /* store the character, and go on */
        }

        if (c == EOF)   /* we hit end of file before end of list, error */
                return(NULL);

        /* allocate space for list of char pointers */
        if ( (listp = (char **)xmalloc(argcount * sizeof(char *))) == NULL)
                return(NULL);

        retp = listp;

        /* reset pointer to the list (in string form)*/
        linep = &line[0];

        /* loop until double-null */
        while ( '\0' != *linep )
        {
                *listp = (char *)xmalloc(strlen(linep)+1);
                strcpy(*listp++,linep); /* copy the next string */
                while (*linep++);       /* skip to next string (null) */
        }

        *listp = NULL;  /* terminate the pointer array */

        return(retp);
}

static char tmpptr[UNAMESIZE];

char *getfilenam(base)
char *base;
{
	char *ptr;
	strcpy(tmpptr,base);
	if ((ptr = strrchr(tmpptr, '.')) != NULL) {
		*ptr = '\0';
		if ((ptr = strrchr(tmpptr, '.')) != NULL)
			*ptr = '\0';
	}
	return(tmpptr);
}

