static char sccsid[] = "@(#)11	1.15.1.3  src/bos/usr/sbin/cron/cronsub.c, cmdcntl, bos41J, 9521B_all 5/26/95 10:47:04";
/*
 * COMPONENT_NAME:  (CMDCNTL) - cronsub.c
 *
 * ORIGIN: 27 18
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1988, 1995
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 *
 */

#ifdef SEC_BASE
#include <sys/secdefines.h>
#endif

#include <sys/types.h>
#include <sys/param.h>        /* for BSIZE needed in dirent.h */
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>	      /* Required for send_msg() */
#include <sys/priv.h>
#include <pwd.h>
#include "cron.h"

#include <ctype.h>
#include <nl_types.h>
#include <langinfo.h>

#include "cron_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)

#define ATRMDEL         "only jobs belonging to user : %s may be deleted\n"
#define BADCD           "can't change directory to the %s directory\n"
#define CANTOPEN        "can't open yourfile in %s directory.\n"
#define NOREADDIR       "can't read the %s directory\n"
#define BADSTAT         "%s : stat failed\n"
#define BADUNLINK       "unlink of %s file failed\n"
#define BADSCANDIR      "scandir of /usr/spool/cron/atjobs failed\n"
#define BADJOBOPEN      "unable to read your at job.\n"
#define NONEXIST        "Job name %s does not exist.\n"
#define NOTOWNER        "You must be the owner of job %s.\n"

#define TIMESIZE	256

struct dirent *dp;
DIR *dir;
char *getfilenam();
int list_aj();
int list_cj();
int remove_cj();
int remove_aj();
int send_msg(char etype, char action, char *fname, char *logname);


/*
 * NAME: rm_at_jobs
 *                                                                    
 * FUNCTION: remove at jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *      This function will find the specified job and delete it.
 *      It sends a message to cron.
 *                                                                   
 * (NOTES:) flag : CRONUSER (If not then name is job name)
 *	           CRON_PROMPT prompt when the job is removed.
 *		   CRON_QUIET  not prompt anythig when the job is removed.
 * 	    ujname :  jobname or user name .
 *	             not support now:(if NULL then delete all of the at jobs)
 *
 * RETURNS: 0 - successful or 1 - (BADCD), 2 - (BADJOBOPEN), 3 - (BADUNLINK)
 *	    4 - (NONEXIST), 5-(NOTOWNER)
 *          exit(1) ATRMDEL
 */  


rm_at_jobs(flags,ujname)
int flags;
char *ujname;
{
        FILE *fp;
        char *uname;
        time_t t, num();
        uid_t uid;
        char *ptr;
        struct passwd *pwinfo;
        struct stat buf;
	int rstat;

#if SEC_BASE && defined(BUILDING_AT)
        uid = getluid();
#else
        uid = getuid();
#endif
        pwinfo = getpwuid(uid);

        /* permission check !!! don't remove !!! */
#if SEC_BASE && defined(BUILDING_AT)
        if (!at_authorized() && (strcmp(ujname,pwinfo->pw_name))
#else
        if (uid && (strcmp(ujname,pwinfo->pw_name))
#endif
                && (strcmp(getfilenam(ujname),pwinfo->pw_name))) {
                        /* don't have permission to remove file(s) */
                fprintf(stderr,MSGSTR(MS_ATRMDEL,ATRMDEL),pwinfo->pw_name);
                exit(1);
        }
        if (!strcmp(ujname,".") || !strcmp(ujname,".."))
                return(3);

        /* permission granted  !!! pass through !!! */
        if (chdir(ATDIR)==-1) 
                return(1);
        if (flags & CRON_USER) {  /* remove all of the jobs of user */
                if((dir=opendir(ATDIR)) == NULL)
                        return(2);
                while ((dp = readdir(dir)) != NULL ) {
                        if (!(strcmp(getfilenam(dp->d_name),ujname)))  {
                                if (flags & CRON_PROMPT)
                                        if (prompt_del(dp->d_name)==1)
                                                continue;
                                if (unlink(dp->d_name) != 0) {
                                        return(3);
                                }
                                rstat = send_msg(AT,DELETE,dp->d_name,dp->d_name);
                                if (!rstat && !(flags & CRON_QUIET))
                                        printf(MSGSTR(MS_ATDELTD,"at file: %s deleted\n"), dp->d_name);
                        }
                }
                closedir(dir);
        }
        else {      /* remove a job */
                if(ujname) {
                        if (stat(ujname, &buf))
                                return(4);      /* job file does not exist */
#if SEC_BASE && defined(BUILDING_AT)
                        if ((uid = getluid()) != buf.st_uid && !at_authorized())
#else
                        if (((uid=getuid()) != buf.st_uid) && (uid != ROOT))
#endif
                                return(5);      /* they don't own this one... */
                        /* should we prompt for confirmation */
                        if ((flags & CRON_PROMPT)&&(prompt_del(ujname)!=0))
                                return(0);
                        else {
                                if (unlink(ujname) != 0) {
                                        return(3);
                                }
                                rstat = send_msg(AT,DELETE,ujname,ujname);
                                if (!rstat && !(flags & CRON_QUIET))
                                        printf(MSGSTR(MS_ATDELTD,"at file: %s deleted\n"), ujname);
                        }
                }
                else { /* root is the only one to get this far */
                        if((dir=opendir(ATDIR)) == NULL)
                                return(2);
                        while ((dp = readdir(dir)) != NULL ) {
                                uname = getfilenam(dp->d_name);

                                if (*uname == '\0')
                                        continue;
                                if (flags & CRON_PROMPT)
                                         if (prompt_del(dp->d_name)!=0)
                                                continue;
                                if (unlink(dp->d_name) != 0)  {
                                        return(3);
                                }
                                rstat = send_msg(AT,DELETE,dp->d_name,dp->d_name);
                                if (!rstat && !(flags & CRON_QUIET))
                                        printf(MSGSTR(MS_ATDELTD,"at file: %s deleted\n"), dp->d_name);
                        }
                        closedir(dir);
                }
        }
}

/*
 * NAME: prompt_del
 *                                                                    
 * FUNCTION: verify file deletion
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *      This function will prompt the user to make sure he really wants
 *      to delete the file, user responds yes or no.
 *
 * RETURNS: 0 - yes or 1 -  no
 */  

prompt_del(user)
char *user;
{
        register junk;
        char    *ans;
        char    *yp,*ystr;
        int     in, i;

/* "y" is the default yes string */
        yp = MSGSTR(MS_YESLOW,"y");
        in = strcspn(yp, "\n");
        ystr = strncpy((char *)calloc(in+1, 1), yp, in);
        ans = (char *)malloc(BUFSIZ);

        printf(MSGSTR(MS_DELETE,"delete %s? (%s) "), user, ystr);
	free(ystr);
        
	fgets(ans,BUFSIZ,stdin);

        /* else normal check */
        if ( rpmatch(ans) == 1 ) { free(ans); return(0);}
        /* check the default case when YESSTR is not set anywhere */
        else if ( ans[0] == 'y') { free(ans); return(0);}
        else { free(ans); return(1);}

}

/*
 * NAME: ls_at_jobs
 *                                                                    
 * FUNCTION: list at jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *      This function will list all files for a user or administrator in
 *      the /usr/spool/cron/atjobs directory.
 *                                                                   
 * (NOTES:) flag : CRON_SORT_E , CRON_SORT_M ,CRON_COUNT_JOBS
 * 		   CRON_USER (ujname is user name  else
 * 				ujname is a job name  if NULL then liat all)
 *
 * RETURNS: 0 - successful or 1 - BADCD, 2 - NOREADDIR, 3 - BADSCANDIR
 */  

ls_at_jobs(flags, ujname)
int flags;
char *ujname;
{
        time_t t, num();
        char *ptr;
        int count=0;
        int i,alphasort();              /* sort jobs by date of execution */
        int numentries;
        struct dirent **queue;          /* the queue itself */
        char *dotptr,c, *uname;
        char timestr[TIMESIZE+1];

        if (chdir(ATDIR)==-1) 
                return(1); 
        if (flags & CRON_SORT_E) { /* sort in order of submission */
                if ((numentries = scandir(".",&queue,NULL,NULL)) < 0) 
                        return(3);
        }
        else {                          /* sort in order of execution */
                if ((numentries = scandir(".",&queue,NULL,alphasort)) < 0) 
                        return(3);
        }

        for (i=0;numentries>i;i++)  {
                if (!strcmp(queue[i]->d_name,"."))
                        continue;
                if (!strcmp(queue[i]->d_name,".."))
                        continue;
                /* all valid at job filenames must contain 2 .'s */
                if ((dotptr = (strchr(queue[i]->d_name,'.'))) == NULL)
                        continue;
                /* all valid at job filenames must contain 2 .'s */
                if ((strchr(dotptr+1,'.')) == NULL)
                        continue;
		uname = getfilenam(queue[i]->d_name);
		ptr = queue[i]->d_name + strlen(uname) + 1;
		if (flags & CRON_USER) {
                  if ((ujname == NULL) || 
                     !(strcmp(uname,ujname)))  {
                          t = num(&ptr);
			  c = *(++ptr);
		  	  if ((flags & CRON_QUE_A) && ( c != 'a')) continue;
		  	  if ((flags & CRON_QUE_B) && ( c != 'b')) continue;
			  if ((flags & CRON_QUE_E) && ( c != 'e')) continue;
			  if ((flags & CRON_QUE_F) && ( c != 'f')) continue;
                          if ((ujname == NULL) || 
                             (!(strcmp(getfilenam(queue[i]->d_name),ujname))))
                                  count++;
                          if (flags & CRON_COUNT_JOBS)
                                  continue;
		          strftime(timestr, TIMESIZE, nl_langinfo(D_T_FMT), localtime(&t));
                          printf("%s\t%s\n",queue[i]->d_name, timestr);
			}
		} else { /* job name */
			if (!(strcmp(queue[i]->d_name,ujname)))	{
                          t = num(&ptr);
		          strftime(timestr, TIMESIZE, nl_langinfo(D_T_FMT), localtime(&t));
                          printf("%s\t%s\n",queue[i]->d_name, timestr);
			}
                }
        }
        if (flags & CRON_COUNT_JOBS)
                printf(MSGSTR(MS_CRONCNT,"%d files in the queue\n"),count);
        return(0);
}

/*
 * NAME: ls_cron_tabs
 *                                                                    
 * FUNCTION: list cron jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *      This function will find the specified job and delete it.
 *                                                                   
 * (NOTES:) flag : CRON_VERVOSE -l list crontab file contents.
 *	           CRON_NON_VERBOSE -v list the information of the file.
 * 		      -l and -v option chnged because of the POSIX conformance.
 * 	    user : user name , if NULL then list all of the files.
 *
 * RETURNS: 0 - successful or 1 - BADCD, 2 - NOREADDIR
 * 	    3 - CANTOPEN, 4 - BADSTAT 
 */  

ls_cron_tabs(flags,user)
int flags;
char *user;
{
        FILE *fp;
        char line[CTLINESIZE];
        struct stat buf;
        int found=0;
	char   timestr[TIMESIZE];

        if (chdir(CRONDIR)==-1) 
                return(1); 
        if((dir=opendir(CRONDIR)) == NULL)
                return(2);
        while ((dp = readdir(dir)) != NULL ) {
                if (!strcmp(dp->d_name,"..") || !strcmp(dp->d_name,"."))
                        continue;
                if ((user == NULL) || !(strcmp(dp->d_name,user))) {
                        found++;
                        if (flags & CRON_VERBOSE) {
                                if ((fp = fopen(dp->d_name,"r")) == NULL)
                                        return(3);
                                if (user == NULL)
                                        fprintf(stdout,MSGSTR(MS_FILENM,"\ncrontab: filename %s\n"), dp->d_name);
                                while(fgets(line,CTLINESIZE,fp) != NULL) {
                                        fputs(line,stdout);
                                }
                                fclose(fp);
                        }
                        else {
                                if (stat(dp->d_name,&buf))
                                        return(4);
                                else {
		          strftime(timestr, TIMESIZE, nl_langinfo(D_T_FMT),localtime(&buf.st_mtime));
                        printf(MSGSTR(MS_SUBTIME,"crontab file: %s\tsubmission time: %s\n"),dp->d_name, timestr);
                                }
                        }
                }
        }
        closedir(dir);
        if ((!found) && (user != NULL)) {
		errno=2;
                return(3);
	}
        return(0);
}

/*
 * NAME: rm_cron_tabs
 *                                                                    
 * FUNCTION: remove cron jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *      This function will find the specified job and delete it.
 *      The administrator can delete anyone's job
 *      This function send a message to cron requesting to 
 *     remove the crontab file entry under CRONDIR.
 *                                                                   
 * (NOTES:) flag : CRON_QUIET and others
 *
 * RETURNS: 0 - successful or 1 - BADCD, 2 - BADUNLINK
 */  

rm_cron_tabs(flags,user)
int flags;
char *user;
{
	int rstat;
        if (chdir(CRONDIR)==-1) 
                return(1);

        if (unlink(user) != 0) 
                return(2);

        rstat = send_msg(CRON,DELETE,user,user);
        if (!rstat && !(flags & CRON_QUIET))
                printf(MSGSTR(MS_CRDELETD,"crontab file: %s deleted\n"),user);
        return(0);
}

/*
 * NAME: getfilenam
 *                                                                    
 * FUNCTION: parses the file name to return just the username portion
 *           of an at job.  Since usernames may contain periods, the
 *           function searched backwards for the second-to-last period.
 *           That is where to place the NULL to terminate the username.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (DATA STRUCTURES:) set the user name to the static variable tmpptr.
 *
 * RETURNS: pointer of tmpptr that contains the user name.
 */  

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

/*
 * NAME: list_cj
 *                                                                    
 * FUNCTION: list cron jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *      call ls_at_jobs and check the exit code.
 *                                                                   
 * (NOTES:) flag : CRON_VERVOSE -l list crontab file contents.
 *	           CRON_NON_VERBOSE -v list the information of the file.
 * 		      -l and -v option chnged because of the POSIX conformance.
 * 	    user : user name , if NULL then list all of the files.
 *
 * RETURNS: 0 - successful or exit 1 - failure
 */  
int
list_cj(flags,user)
int flags;
char *user;
{
	errno=0;  /* reset to 0 */
        switch(ls_cron_tabs(flags,user)) {
           case 1: 
                fprintf(stderr,(MSGSTR(MS_BADCD, BADCD)),CRONDIR);
                exit(1); 
           case 2: 
                fprintf(stderr, (MSGSTR(MS_NOREADDIR, NOREADDIR)),CRONDIR);
                exit(1); 
           case 3: 
                fprintf(stderr,(MSGSTR(MS_CANTOPEN, CANTOPEN)),CRONDIR);
		perror("");
                exit(1); 
           case 4: 
                fprintf(stderr, (MSGSTR(MS_BADSTAT, BADSTAT)),"crontab");
                exit(1); 
        }
        return(0); 
}

/*
 * NAME: remove_cj
 *                                                                    
 * FUNCTION: remove cron jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *      This function will find the specified job and delete it.
 *      The administrator can delete anyone's job
 *      call ls_at_jobs and check the exit code.
 *                                                                   
 * (NOTES:) flag : CRON_QUIET and others
 *
 * RETURNS: 0 - successful or exit 1 - failure
*/
int
remove_cj(flags,user)
char *user;
{
        switch(rm_cron_tabs(flags,user)) {
           case 1: 
                fprintf(stderr,(MSGSTR(MS_BADCD, BADCD)),CRONDIR);
                exit(1); 
           case 2: 
               fprintf(stderr,(MSGSTR(MS_BADUNLINK,BADUNLINK)),"cron");
                exit(1); 
        }
        return(0); 
}
                
/*
 * NAME: list_aj
 *                                                                    
 * FUNCTION: list at jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *      call ls_at_jobs and check the exit code.
 *                                                                   
 * (NOTES:) flag : CRON_SORT_E , CRON_SORT_M ,CRON_COUNT_JOBS
 * 		   CRON_USER (ujname is user name  else
 * 				ujname is a job name  if NULL then liat all)
 * RETURNS: 0 - successful or  exit 1 - failure
 */  

int
list_aj(flags,user)
int flags;
char *user;
{
        switch(ls_at_jobs(flags,user)) {
           case 1: 
                fprintf(stderr,(MSGSTR(MS_BADCD, BADCD)),ATDIR);
                exit(1);
           case 2: 
                fprintf(stderr,(MSGSTR(MS_NOREADDIR, NOREADDIR)),ATDIR);
                exit(1);
           case 3: 
                fprintf(stderr,(MSGSTR(MS_BADSCANDIR, BADSCANDIR)));
                exit(1);
        }
        return(0);
}

/*
 * NAME: remove_aj
 *                                                                    
 * FUNCTION: remove at jobs for a user or the administrator
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *           call rm_at_jobs and check the exit code.
 *                                                                   
 * (NOTES:) flag : CRONUSER (If not then name is job name)
 *	           CRON_PROMPT prompt when the job is removed.
 *		   CRON_QUIET  not prompt anythig when the job is removed.
 * 	    ujname : if NULL then delete all of the at jobs by root user
 *
 * RETURNS: 0 - successful or exit 1 - failure
 */  
int
remove_aj(flags,name)
int flags;
char *name;
{
        switch(rm_at_jobs(flags,name)) {
           case 1: 
                fprintf(stderr,(MSGSTR(MS_BADCD, BADCD)),ATDIR);
                exit(1); 
           case 2: 
                fprintf(stderr,(MSGSTR(MS_BADJOBOPEN,BADJOBOPEN)));
                exit(1); 
           case 3: 
                fprintf(stderr,(MSGSTR(MS_BADUNLINK,BADUNLINK)),"at");
                exit(1); 
           case 4:
                fprintf(stderr, (MSGSTR(MS_NONEXIST, NONEXIST)), name);
                exit(1);
           case 5:
                fprintf(stderr, (MSGSTR(MS_NOTOWNER, NOTOWNER)), name);
                exit(1);
        }
        return(0); 
}

/*
 * NAME: send_msg
 *                                                                    
 * FUNCTION: Send message to cron daemon
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * (NOTES:) etype   : event type CRON or AT
 *	    action  : request action DELETE
 *	    fname   : file name (username  or jobname(at))
 *	    logname : user name
 *
 * RETURNS: 0 - successful or 1 - failure
 */  

int
send_msg(char etype, char action, char *fname, char *logname)
{

	static	int	msgfd = -2;
	struct	message	*pmsg;

	pmsg = &msgbuf;
	/* get permission to open /var/adm/cron/FIFO file */
	privilege(PRIV_ACQUIRE);
	
	if(msgfd == -2)
		if((msgfd = open(FIFO,O_WRONLY|O_NDELAY)) < 0) {
			if(errno == ENXIO || errno == ENOENT)
				fprintf(stderr,MSGSTR(MS_NOCRON, "cron may not be running - call your system administrator\n"));
			else
				fprintf(stderr,MSGSTR(MS_MSGQERROR, "error in message queue open\n"));
			privilege(PRIV_LAPSE);
			return(1);
		}
	pmsg->etype = etype;
	pmsg->action = action;
	strncpy(pmsg->fname,fname,(size_t)FLEN);
	strncpy(pmsg->logname,logname,(size_t)LLEN);
	if(write(msgfd,(char *)pmsg,sizeof(struct message)) != sizeof(struct message)) {
		 fprintf(stderr,MSGSTR(MS_MSGSERROR, 
				"error in message send\n"));
		 privilege(PRIV_LAPSE);
		 return(1);
	}
	/* job done returns to normal privilege */
	privilege(PRIV_LAPSE);

	return(0);
	
}

