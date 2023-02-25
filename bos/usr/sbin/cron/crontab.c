static char sccsid[] = "@(#)04	1.31  src/bos/usr/sbin/cron/crontab.c, cmdcntl, bos41B, 9504A 1/4/95 10:13:32";
/*
 * COMPONENT_NAME: (CMDCNTL) commands needed for basic system needs
 *
 * FUNCTIONS: crontab
 *
 * ORIGINS: 27,71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

#include <sys/id.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/priv.h>
#include <sys/acl.h>
#include <sys/audit.h>
#include <time.h>
#include "cron.h"
#include <ctype.h>
#include <sys/stat.h>

#include <nl_types.h>
#include "cron_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)

#define DEFAULT_EDITOR "vi"    /* default editor for -e option */

#define AUDIT_CLASS     "cron"
#define BADCREATE       "cannot create your crontab file in the crontab directory.\n"
#define INVALIDUSER     "you are not a valid user (no entry in /etc/passwd).\n"
#define NOTALLOWED      "you are not authorized to use cron.  Sorry.\n"
#define EOLN            "unexpected end of line.\n"
#define UNEXPECT        "unexpected character found in line.\n"
#define OUTOFBOUND      "number out of bounds.\n"
#define CANTOPEN        "cannot open your file in %s directory.\n"
#define BADSTAT         "cannot access %s \n"
#define AUDITER         "%s: auditproc error: %d\n"
#define RFLAG           0x00000001
#define LFLAG           0x00000002
#define VFLAG           0x00000004
#define EFLAG           0x00000008

static int err,cursor;
static char *cf,*tnam,line[CTLINESIZE];
char *xmalloc();
char *getenv(char *name);
static char login[UNAMESIZE];

extern int list_cj();
extern int remove_cj();

extern struct passwd *getpwuid(uid_t uid);
extern uid_t getuid(void);

int usage(void);
void crabort( char *msg ,char *s);
void cerror( char *msg );
int catch(void);
int next_field( int lower, int upper );
void copycron( FILE *fp );
void editcron();

/*
 * NAME: crontab
 *                                                                    
 * FUNCTION:  crontab: Submits a schedule of commands to cron.
 *      description:    This program implements crontab (see cron(1)).
 *                      This program should be set-uid to root.
 *      files:
 *              /var/adm/cron drwxr-xr-x root sys
 *              /var/adm/cron/cron.allow -rw-r--r-- root sys
 *              /var/adm/cron/cron.deny -rw-r--r-- root sys
 */  
int
main(int argc, char **argv )
{
        char *pp;
        FILE *fp;
        struct passwd *nptr;
        int funct=0;
        int flag,rstat;
        char *s;
        (void ) setlocale(LC_ALL,"");

        catd = catopen(MF_CRON,NL_CAT_LOCALE);

        /* 
         * suspend auditing for this process - This process is trusted 
         */

        if (auditproc(0,AUDIT_EVENTS,AUDIT_CLASS,strlen(AUDIT_CLASS)+2) < 0) {
                fprintf(stderr,MSGSTR(MS_AUDITER, AUDITER),"crontab",errno);
        }
        if (auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0) < 0) {
                fprintf(stderr,MSGSTR(MS_AUDITER, AUDITER),"crontab",errno);
        }
        /*
         * only 2 arguments allowed except that when "--" is used, then 3 arguments
         */

        if ((argc > 2 && strcmp(argv[1], "--") != 0) || (argc > 3))
		usage();

        if ((nptr=getpwuid(getuid())) == NULL)  /* check the user id */
                crabort(MSGSTR(MS_INVALIDUSER, INVALIDUSER),"");
        else
                pp = nptr->pw_name;             /* user's crontab file name */

        strcpy(login,pp);                       /* save login name */

        /*
         * determine function: list, verbose list, remove or add
         */

        while ((flag = getopt (argc, argv, "rlve")) != EOF) {
                switch (flag) {
                        case 'r':
				if (!funct)
                                	funct = RFLAG;
				else
					usage();
                                break;
                        case 'l':
				if (!funct)
                                	funct = LFLAG;
				else
					usage();
                                break;
                        case 'v':
				if (!funct)
                                	funct = VFLAG;
				else
					usage();
                                break;
                        case 'e':
				if (!funct)
                                	funct = EFLAG;
				else
					usage();
                                break;
                        default:
                                usage ();
                }
        }
	/* adjust argument pointer after parsing */
	argv += optind;
	argc -= optind;
        /*
         * check the .permit and .deny files (part of SVID)
         */

        if (!allowed(login,CRONALLOW,CRONDENY)) 
                crabort(MSGSTR(MS_CRNOTALLOWED, NOTALLOWED),"");

        switch(funct) {
                case VFLAG:                     /* vebose listing */
                        list_cj(CRON_NON_VERBOSE,pp);
                        break;

                case LFLAG:                     /* list */
                        list_cj(CRON_VERBOSE,pp);
                        break;

                case RFLAG:                     /* remove */
                        remove_cj(CRON_QUIET,pp);
                        audit_pr(REMOVE_CRON,pp);  /* audit crontab removal */
                        break;

                case EFLAG:                     /* edit */
                        editcron();
                        rstat = send_msg(CRON,ADD,login,pp);
			if (rstat) exit(1);
                        audit_pr(ADD_CRON,pp);  /* audit crontab add */
                        break;
                default:                        /* add */
                        if (argc==0)   {
                                copycron(stdin);
                        } else {                  /* open the crontab file */
				setegid(getgid());
				seteuid(getuid());
				if ((fp=fopen(argv[0],"r"))==NULL)  {
					crabort(MSGSTR(MS_BADSTAT, BADSTAT),argv[1]);
				} else  {
					seteuid(getuidx(ID_SAVED));
					setegid(getgidx(ID_SAVED));
					copycron(fp);
				}
			}
                       	rstat = send_msg(CRON,ADD,login,pp);
			if (rstat) exit(1);
                        	audit_pr(ADD_CRON,pp);  /* audit crontab add */
        	}
        exit(0);
}

/*
 * NAME: copycron
 *
 * FUNCTION: copy cron event to cron directory from the file(fp)
 *
 * EXECUTION ENVIRONMENT:
 *      This function overrides the contents of the old crontab file.
 *      If there is no crontab file then it will be created.
 * 	The crontab file name is the login name.
 *
 * (NOTES:)  This function uses a temporary file.
 *
 * RETURNS:  none
 */

static void
copycron( FILE *fp )
{
        FILE *tfp;
        char pid[6];
	int t;
        struct passwd *pw;
        int line_num=0;
	char tnam[] = CRONDIR"/crontabXXXXXX";

        /*
         * create the new crontab file  and temporary file name
         */
        cf = xmalloc(strlen(CRONDIR)+strlen(login)+2);
        sprintf(cf,"%s/%s",CRONDIR,login);
        
        /* catch SIGINT, SIGHUP, SIGQUIT signals */
        if (signal(SIGINT,(void (*)(int))catch) == SIG_IGN) signal(SIGINT,SIG_IGN);
        if (signal(SIGHUP,(void (*)(int))catch) == SIG_IGN) signal(SIGHUP,SIG_IGN);
        if (signal(SIGQUIT,(void (*)(int))catch) == SIG_IGN) signal(SIGQUIT,SIG_IGN);
        if (signal(SIGTERM,(void (*)(int))catch) == SIG_IGN) signal(SIGTERM,SIG_IGN);
 	/* mkstemp will call mktemp() and open() with mode O_CREAT|O_EXCL|O_RDWR) */
	if ((t=mkstemp(tnam)) == -1) 
                crabort(MSGSTR(MS_BADCREATE, BADCREATE),"");
        
	if ((tfp=fdopen(t,"w"))==NULL) {
                unlink(tnam);
                free(tnam);
                crabort(MSGSTR(MS_BADCREATE, BADCREATE),"");
        }

        err=0;  /* if errors found, err set to 1 */
        while (fgets(line,CTLINESIZE,fp) != NULL) {
                cursor=0;
                line_num++;
                while(isspace(line[cursor]) || line[cursor] == '\t')
                        cursor++;
                if ((line[cursor] == '\n') || (line[cursor] == '\0'))
			continue;
                        
                if (strchr(line,' ') == NULL) 
                        goto cont;
                if (line[cursor] == '#')
                        goto cont;
                if (next_field(0,59)) continue;
                if (next_field(0,23)) continue;
                if (next_field(1,31)) continue;
                if (next_field(1,12)) continue;
                if (next_field(0,06)) continue;
                if (line[++cursor] == '\0') {
                        cerror(MSGSTR(MS_EOLN, EOLN));
                        continue; 
                }
cont:
                if (fputs(line,tfp) == EOF) {
                        unlink(tnam);
			free(tnam);
                        crabort(MSGSTR(MS_BADCREATE, BADCREATE),""); 
                }
        }
        fclose(fp);
        if (fclose(tfp) == EOF) {
		perror(strcat("crontab: ", tnam));
		exit(1);
	}
        if (!err) {
                /* make file tfp the new crontab */
                unlink(cf);   /* remove the old crontab file */
                if (link(tnam,cf)==-1) {
			unlink(tnam);
			free(tnam);
                        crabort(MSGSTR(MS_BADCREATE, BADCREATE),""); 
		}
	}  else {
        	unlink(tnam);
		free(tnam);
		exit(1);
	}
        unlink(tnam);
	free(tnam);
}

/*
 * NAME: next_field
 *
 * FUNCTION:  check the next field for invalid input
 *
 * EXECUTION ENVIRONMENT:  
 *
 * RETURNS:  0 - success, 1 - failure
 */  
static int
next_field( int lower, int upper )
{
        int num,num2;

        while ((line[cursor]==' ') || (line[cursor]=='\t')) cursor++;
        if (line[cursor] == '\0') {
                cerror(MSGSTR(MS_EOLN, EOLN));
                return(1); 
        }
        if (line[cursor] == '*') {
                cursor++;
                if ((line[cursor]!=' ') && (line[cursor]!='\t')) {
                        cerror(MSGSTR(MS_UNEXPECT, UNEXPECT));
                        return(1); 
                }
                return(0); 
        }
        while (TRUE) {
                if (!isdigit(line[cursor])) {
                        cerror(MSGSTR(MS_UNEXPECT, UNEXPECT));
                        return(1); 
                }
                num = 0;
                do { 
                        num = num*10 + (line[cursor]-'0'); 
                        ++cursor;       /* for KANJI */
                } while (isdigit(line[cursor]));
                if ((num<lower) || (num>upper)) {
                        cerror(MSGSTR(MS_OUTOFBOUND, OUTOFBOUND));
                        return(1); 
                }
                if (line[cursor]=='-') {
                        ++cursor;       /* for KANJI */
                        if (!isdigit(line[cursor])) {
                                cerror(MSGSTR(MS_UNEXPECT, UNEXPECT));
                                return(1); 
                        }
                        num2 = 0;
                        do { 
                                num2 = num2*10 + (line[cursor]-'0'); 
                                ++cursor;       /* for KANJI */
                        } while (isdigit(line[cursor]));
                        if ((num2<lower) || (num2>upper)) {
                                cerror(MSGSTR(MS_OUTOFBOUND, OUTOFBOUND));
                                return(1); 
                        }
                }
                if ((line[cursor]==' ') || (line[cursor]=='\t')) break;
                if (line[cursor]=='\0') {
                        cerror(MSGSTR(MS_EOLN, EOLN));
                        return(1); 
                }
                if (line[cursor++]!=',') {
                        cerror(MSGSTR(MS_UNEXPECT, UNEXPECT));
                        return(1); 
                }
        }
        return(0);
}


/*
 * NAME: cerror
 *
 * FUNCTION:  print out error message for cron event
 *
 * EXECUTION ENVIRONMENT:  
 *
 * RETURNS:  set err 1.
 */  
static void
cerror( char *msg )
{
        char message[256];
        strcpy(message,msg);    /* save message */
        fprintf(stderr,MSGSTR(MS_LINERR, "%scrontab: error on previous line; %s\n"),line,message);
        err=1;
}


/*
 * NAME: catch
 *
 * FUNCTION: unlink if crontab object is invalid
 *
 * EXECUTION ENVIRONMENT: user process
 *
 * RETURNS: does not return exit(1)
 */  

static int
catch(void)
{
        unlink(tnam);
	free(tnam);
        exit(1);
}

/*
 * NAME: crabort
 *
 * FUNCTION:  print error message if crontab failed
 *
 * EXECUTION ENVIRONMENT: user process
 *
 * RETURNS: does not return exit(1)
 */  

static void
crabort( char *msg , char *s1)
{
	char *s;
	s = xmalloc(strlen(msg)+12);
        sprintf(s,"crontab: %s",msg);
        fprintf(stderr,s,s1);
        exit(1);
}

/*
 * NAME: usage
 *
 * FUNCTION: print usage message and exit
 *
 * EXECUTION ENVIRONMENT: user process
 *
 * RETURNS: does not return exit(1)
 */  

static int
usage(void)
{
        fprintf(stderr, MSGSTR(MS_CRBADUSAGE, 
        "Usage: crontab [-l|-r|-v|-e|File]\n"));
        exit(1);
}

/*
 * NAME: editcron
 *
 * FUNCTION: edit cron event in the file under cron directory
 *
 * EXECUTION ENVIRONMENT:
 *      This function overrides the contents of the old crontab file.
 *      If there is no crontab file then it will be created.
 * 	The crontab file name is the login name.
 *
 * (NOTES:)  This function uses a temporary file.
 *
 * RETURNS:  none
 */

static void
editcron()
{
        FILE *tfp,*tfp2,*fp;
        char pid[6];
        char *edit,*s,*etmp;
        int wpid,fid,st;
	int j,t,line_num=0;
	int i;
	int fd;
	char dnam[] = "/var/spool/cron/crontabs/ctabXXXXXX";
	char tnam[] = "/tmp/crontabXXXXXX";
        struct stat fst;
        char *aclp=NULL;

        /*
         * create the crontab file  and temporary file name
         */
        cf = xmalloc(strlen(CRONDIR)+strlen(login)+2);
        sprintf(cf,"%s/%s",CRONDIR,login);
	if ((t=mkstemp(tnam)) == -1)
            crabort(MSGSTR(MS_BADCREATE, BADCREATE),"");
	mktemp(dnam);

        /* catch SIGINT, SIGHUP, SIGQUIT signals */
        if (signal(SIGINT,(void (*)(int))catch) == SIG_IGN) signal(SIGINT,SIG_IGN);
        if (signal(SIGHUP,(void (*)(int))catch) == SIG_IGN) signal(SIGHUP,SIG_IGN);
        if (signal(SIGQUIT,(void (*)(int))catch) == SIG_IGN) signal(SIGQUIT,SIG_IGN);
        if (signal(SIGTERM,(void (*)(int))catch) == SIG_IGN) signal(SIGTERM,SIG_IGN);

	/* if the crontab exist? then open for read */
        if ((fid = stat(cf,&fst)) != -1) { 
        	if ((fp=fopen(cf,"r"))==NULL) {
			free(cf);
       			crabort(MSGSTR(MS_CANTOPEN, CANTOPEN),CRONDIR);
		}
        	if ((tfp=fdopen(t,"w"))== NULL) {
			fclose(fp);
			free(cf);
       			crabort(MSGSTR(MS_BADCREATE, BADCREATE),"");
		}
		setvbuf(fp, (char *) 0, _IOLBF, 0);
		/* copy the content of current crontab file to temp file for editing */
        	while (fgets(line,CTLINESIZE,fp) != NULL) 
               		if (fputs(line,tfp) == EOF) {
        			fclose(fp);
        			fclose(tfp);
				unlink(tnam);
				free(cf);
        			crabort(MSGSTR
					(MS_BADCREATE,BADCREATE),"");
			}
		/*
		 * Get the access control list from the original crontab
		 * file.  This will be used on the new crontab file after
		 * it has been created.
		 */
		aclp = acl_fget (fileno (fp));
        	fclose(fp);
        	if (fclose(tfp) == EOF) {
			perror("fclose");
		exit(1);
		}
        }

        wpid = fork();
        if (wpid <0) { 
		free(cf);
                crabort(MSGSTR(MS_BADCREATE, BADCREATE),""); 
        }
        if (wpid == 0) { /* child process */
		/* 
		 * Setup environment for executing as the submittor
		 * of the cron file.  I have the real crontab file 
		 * descriptor open at this point.  From here on out, 
	   	 * I'll run with the users uid and copy my file
		 */
                /* give the file to the user */
                chmod(tnam, S_IRUSR|S_IWUSR);
                chown(tnam, getuid(), getgid());

		setegid(getgid());
		seteuid(getuid());

		/* Keep at least the default data length in case the 
		 * EDITOR is NULL 
		 */
                etmp = getenv("EDITOR");
                if (etmp == NULL ) {
                	edit=xmalloc(strlen(tnam)+9);
			strcpy(edit,DEFAULT_EDITOR);
		} else {
                	edit=xmalloc(strlen(etmp)+strlen(tnam)+2);
			strcpy(edit,etmp);
		}
                strcat(edit," ");
                strcat(edit,tnam);
                execlp(SHELL,"sh","-c",edit,'\0');
		free(edit);
                exit(0);
        }
        if ((wait(&st) != wpid) || (st)) {
		unlink(tnam);
		free(cf);
                crabort(MSGSTR(MS_BADCREATE, BADCREATE),""); 
        } 

	setegid(getgid());
	seteuid(getuid());
        if ((tfp=fopen(tnam,"r"))==NULL) {
		unlink(tnam);
		free(cf);
                crabort(MSGSTR(MS_BADCREATE, BADCREATE),""); 
	} else {
		seteuid(getuidx(ID_SAVED));
		setegid(getgidx(ID_SAVED));
	}

        err=0;  /* if errors found, err set to 1 */
        while (fgets(line,CTLINESIZE,tfp) != NULL) {
                cursor=0;
                line_num++;
                while(isspace(line[cursor]) || line[cursor] == '\t')
                        cursor++;
                if ((line[cursor] == '\n') || (line[cursor] == '\0'))
			continue;
                if (strchr(line,' ') == NULL) 
                        continue;
                if (line[cursor] == '#')
                        continue;
                if (next_field(0,59)) continue;
                if (next_field(0,23)) continue;
                if (next_field(1,31)) continue;
                if (next_field(1,12)) continue;
                if (next_field(0,06)) continue;
                if (line[++cursor] == '\0') {
                        cerror(MSGSTR(MS_EOLN, EOLN));
                        continue; 
                }
        }
        if (!err) {
		/* 
		 * Rewind the temporary file pointer to the beginning;
		 * open the target file for writing (effectively clearing
		 * the file); read/write the temporary file into the target.
		 */
        	if (fseek(tfp, SEEK_SET, 0)) {
                	unlink(tnam);
			free(cf);
       			crabort(MSGSTR(MS_BADCREATE, BADCREATE),"");
		}
		if ((fd = open (dnam, O_WRONLY|O_CREAT|O_EXCL, 0600)) < 0 ||
				(fp = fdopen (fd, "w")) == NULL) {
			fclose(tfp);
                	unlink(tnam);
			free(cf);
       			crabort(MSGSTR(MS_BADCREATE, BADCREATE),"");
		}
		/*
		 * Set the new crontab file ownership and permissions from
		 * the original crontab file.
		 */
		if (aclp) {
			fchown (fileno (fp), fst.st_uid, fst.st_gid);
			acl_fput (fileno (fp), aclp, 1);
		}
        	while (fgets(line,CTLINESIZE,tfp) != NULL) {
			/*
			 * Output each line and test for failure.  Abort
			 * if the line cannot be written.  Most common
			 * cause is lack of free space.
			 */
                	if (fputs(line,fp) == EOF) {
        			fclose(fp);
        			fclose(tfp);
				unlink(tnam);
				unlink(dnam);
				free(cf);
        			crabort(MSGSTR (MS_BADCREATE,BADCREATE),"");
			}
		}
        } else {
		fclose(tfp);
		unlink(tnam);
		unlink(dnam);
		free(cf);
		exit(1);
	}
	/*
	 * Flush any buffered I/O, testing for failure.  Some output
	 * lines may not have been written.
	 */

	if (fflush (fp)) {
		fclose(fp);
		fclose(tfp);
		unlink(tnam);
		unlink(dnam);
		free(cf);
		crabort(MSGSTR (MS_BADCREATE,BADCREATE),"");
	}

	/*
	 * Clean up -- need to close files and rename the new crontab
	 * file to its original name.  rename() will leave original file
	 * intact if it fails.
	 */
	fclose(tfp);
	fclose(fp);
        unlink(tnam);
        if (rename(dnam, cf))
		crabort(MSGSTR (MS_BADCREATE,BADCREATE),"");
	free(cf);
}
