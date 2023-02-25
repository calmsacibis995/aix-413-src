static char sccsid[] = "@(#)96  1.7  src/rspc/usr/bin/pmd/pmd.c, pwrcmd, rspc412, 9445A166791 11/4/94 10:17:15";
/*
 *   COMPONENT_NAME: PWRCMD
 *
 *   FUNCTIONS: main,check_console,check_utmp,getprocdata,get_pm_data
 *              run_xlock,run_LockDisplay,check_program,pmd_lock
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/cfgodm.h>
#include <procinfo.h>
#include <errno.h>
#include <stdio.h>
#include <utmp.h>
#include <pwd.h>

#include <sys/pm.h>

static struct procsinfo *getprocdata(long *);

#define PROCSIZE sizeof(struct procsinfo)
#define U_SIZE sizeof(struct userinfo)

#define LFT_ONLY		1	/* LFT only. No X server. */
#define X_SERVER_RUNNING	2	/* X server is running. */
#define NO_LFT			3	/* No LFT & no X server. */

int lockid;
char *lockfile = "/etc/pmd_lock";
long nproc;
struct procsinfo *mproc;

extern int errno;
extern int odmerrno;

main()
{
    int event, action, value;
    uid_t pid, ppid;
    struct pm_state pms;

    umask(0);
    setpgrp();
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGCLD, SIG_IGN);
    chdir("/");

    if( pmd_lock() != TRUE ) {
	fprintf(stderr, "pmd: cannot create lockfile /etc/pmd_lock.\n");
	exit(1);
    }

    for(;;) {
	/* Query PM event. Returned action is default PM state transition */
	if( pm_event_query(&event, &action) != PM_SUCCESS ) {
	    fprintf(stderr, "pmd: pm_event_query failed.\n");
	    event = action = 0;
	    continue;
	}

	switch( action ) {
	  case PM_SYSTEM_SUSPEND:
	    /* get process data */
	    mproc = getprocdata(&nproc);
	    /* Get console type */
	    switch( check_console(&value) ) {
	      case LFT_ONLY:			/* LFT */
		/* Whether or not LFT session should be terminated. */
		if( get_pm_data("pm_lft") == 1 ) {
		    /* Value is PID of console */
		    kill(value, SIGKILL);
		}
		break;
	      case X_SERVER_RUNNING:		/* X server */
		/* Whether or not xlock should be invoked. */
		if( get_pm_data("pm_password") == 1 ) {
		    /* get uid of parent "dtlogin <:0>" */
		    check_program("dtlogin", "<:0>", 0, &ppid);
		    /* Value is PID of X server */
		    if( check_program("dtgreet", NULL, ppid, NULL) ) {
			; /* do nothing */
		    } else if( check_program("dtsession", NULL, ppid, &pid) ) {
			(void)run_LockDisplay(pid);
		    } else if( !check_program("xlock", NULL, 0, NULL) ) {
			(void)run_xlock(value);
		    }
		}
		break;
	      default:				/* no CRT */
		break;
	    }
	    /* free process data */
	    if(mproc != NULL) {
		free((char *)mproc);
	    }
	    break;

	  case PM_SYSTEM_HIBERNATION:
	  case PM_SYSTEM_SHUTDOWN:
	    system("/usr/sbin/shutdown -F > /dev/console 2> /dev/console");
	    /* if shutdown fails */
	    break;

	  /* PM core is going to terminate. PM daemon terminates, too. */
	  case PM_TERMINATE:
	    unlink(lockfile);
	    exit(0);
	}

	/* Start default PM state transition */
	pms.state = action;
	if( pm_control_state(PM_CTRL_START_STATE, &pms) != PM_SUCCESS ) {
	    fprintf(stderr, "pmd: PM_CTRL_START_STATE, %x failed\n", action);
	}
    }
}


/*
 * NAME:  check_console
 *
 * FUNCTION:  
 *        Check processes running on console and set user id
 *        to argument when X is running, or set process id 
 *        of shell on LFT when X is not running.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *        procsinfo - process info structure
 *
 * RETURN VALUE DESCRIPTION:	
 *        X_SERVER_RUNNING - X Server is running on console
 *        LFT_ONLY - X Server not running
 *        NO_LFT - neither X server nor LFT runs.
 */
static int
check_console(int *val)
{
    int i;
    int pid = 0;
    struct stat devstat;
    char *lft = "/dev/lft0";
    dev_t devno = -1;

    if( stat(lft, &devstat) == 0 ) {
	devno = devstat.st_rdev;
    }

    for( i = 0; i<nproc; i++ ) {
	if( !(mproc[i].pi_flags & SKPROC) ) {
	    /* X server process ? */
	    if( strcmp( mproc[i].pi_comm , "X" ) == 0 ) {
		*val = mproc[i].pi_uid;
		if(mproc != NULL) {
		    free((char *)mproc);
		}
		return X_SERVER_RUNNING;

	    /* LFT session process ? */
	    } else if( (mproc[i].pi_ttyd == devno) &&
					(mproc[i].pi_ppid == 1) ) {
		if( check_utmp(mproc[i].pi_pid) == TRUE ) {
		    pid = mproc[i].pi_pid;
		}
	    }
	}
    }

    if( pid != 0 ) {
	*val = pid;
	return LFT_ONLY;
    }

    return NO_LFT;
}


/*
 * NAME:  check_program
 *
 * FUNCTION:  
 *        Check specified program is running
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *        procsinfo - process info structure
 *
 * RETURN VALUE DESCRIPTION:	
 *        TRUE - program is running
 *        FALSE - program isn't running
 */
static int
check_program(char *progname, char *arg1, int ppid, int *dts_uid)
{
    int i;
    pid_t pid_pmd;
    char args[256];

    for( i = 0; i<nproc; i++ ) {
	if( !(mproc[i].pi_flags & SKPROC) ) {
	    if( strcmp( mproc[i].pi_comm, progname ) == 0 ) {
		if( ppid == 0 || mproc[i].pi_ppid == ppid ) {

		    if( arg1 == NULL ) {	/* no argument check */
			if( dts_uid != NULL ) {
			    *dts_uid = mproc[i].pi_uid;
			}
			return TRUE;
		    				/* argument check */
		    } else if( getargs(&mproc[i], PROCSIZE, args, sizeof(args))
									== 0 ) {
			if( strcmp(arg1, &args[strlen(args)+1]) == 0 ) {
			    if( dts_uid != NULL ) {
				*dts_uid = mproc[i].pi_uid;
			    }
			    return TRUE;
			}
		    }
		}
	    }
	}
    }

    return FALSE;
}


/*
 * NAME:  check_utmp
 *
 * FUNCTION:  
 *        Check that pid exists in utmp entry. 
 *            
 * NOTES:
 *
 * DATA STRUCTURES:
 *        utmp - utmp structure
 *
 * RETURN VALUE DESCRIPTION:	
 *        TRUE - pid is valid.
 *        FALSE - pid is invalid.
 */
static int
check_utmp(pid_t pid)
{
    struct utmp *ut;

    setutent();				/* open/rewind utmp */
    while( (ut = getutent()) != NULL ) {
	if( ut->ut_pid == pid )
	    return TRUE;
    }
    endutent();

    return FALSE;
}


/*
 * NAME: getprocdata
 *
 * FUNCTION: Routine stolen from ps.  Grabs enough memory to read in
 *	     the proc structure, set nprocs and returns a pointer the
 *	     the interesting procs returned from the getproc systm call.
 *           
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *        pointer to the proc structure or FALSE if failed.
 */
static struct procsinfo *
getprocdata(long *nproc)
{
	long multiplier;
	struct procsinfo *Proc;

	struct procsinfo *P;
	pid_t	index;
	int	count;
	int	ret;
	short	again;

	*nproc = 0;
	again = 1;
        multiplier = 5;
	index = 0;
	count = 1000000;

	Proc = NULL;
	P = NULL;
	do {
		if ((ret = getprocs(P, PROCSIZE, NULL, 0,&index, count)) != -1){
			if (P == NULL) {		/* first call */
				/* Get extra in case busy system*/
				count = ret + (multiplier <<= 1);
                		Proc = (struct procsinfo *)
					malloc ((size_t)(PROCSIZE * count));
				P = Proc;
                		if ( Proc == NULL) {
        				/* We ran out of space before we could*/					/* read in the entire proc structure. */
                       			 perror ("malloc: ");
                       			 exit (1);
				}
				index = 0;	/* reset proc slot index */
			}
			else {
				*nproc += ret;
				/* Not all entries were retrieved */ 
				if (ret >=  count) {
					count = (multiplier <<= 1);
                			Proc = (struct procsinfo *)
						realloc ((void *)Proc,
							(size_t)(PROCSIZE *
							(*nproc + count)));
                			if ( Proc == NULL) {
						/* We ran out of space before */
						/* we could read in the entire*/
						/* proc structure.            */
                       				 perror ("realloc: ");
                       				 exit (1);
                			}
					else 
						P = Proc + (*nproc);
				}
				else		/* All entries were retrieved */
					again = 0;
			}
        	}
	} while (again && (ret != -1));
	return (Proc);
}


/*
 * NAME:  get_pm_data
 *
 * FUNCTION:  
 *        Get PM data from ODM.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *        Class - ODM class structure
 *
 * RETURN VALUE DESCRIPTION:	
 *        ODM data value (int) is returned.
 *        When an error occurred, -1 is returned.
 */
static int
get_pm_data(char *att)
{
    int flag;

    if( odm_initialize() == -1 ) {
	fprintf(stderr, "pmd: Error in odm_initialize\n");
	odm_terminate();
	return -1;
    }

    if( (flag = get_pmmi_data(att, 0)) == -1 ) {
	fprintf(stderr, "pmd: Error in get_pmmi_data\n");
	odm_terminate();
	return -1;
    }

    odm_terminate();
    return flag;
}


/*
 * NAME:  run_xlock
 *
 * FUNCTION:  
 *        invoke xlock for the user using X on console 
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *        pid of invoked chile process is returned.
 *        When an error occurred, -1 is returned.
 */
static int
run_xlock(uid_t xs_uid)
{
    char *xs_username;
    int ret;
    struct passwd *xs_pwent;
    char logname[32],login[32],home[PATH_MAX+8];

    xs_username = (char *)IDtouser(xs_uid);

    /* LOGIN environment */
    sprintf(login, "LOGIN=%s", xs_username);

    /* LOGNAME environment */
    sprintf(logname, "LOGNAME=%s", xs_username);

    /* HOME environment */
    xs_pwent = getpwuid(xs_uid);
    if( xs_pwent == NULL ) {
	return -1;
    }
    sprintf(home, "HOME=%s", xs_pwent->pw_dir);

    if( (ret = fork()) == 0 ) {		/* child process */
	putenv("AUTHSTATE=compat");
	putenv("DISPLAY=:0.0");
	putenv(logname);
	putenv(login);
	putenv(home);
	setuid(xs_uid);
	exit(system("/usr/lpp/X11/bin/xlock"));		/* run xlock */
    }

    return ret;
}


/*
 * NAME:  run_LockDisplay
 *
 * FUNCTION:  
 *        invoke dtaction LockDisplay for the user using X on console 
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *        pid of invoked child process is returned.
 *        When an error occurred, -1 is returned.
 */
static int
run_LockDisplay(uid_t dts_uid)
{
    char *dts_username;
    int ret;
    char cmd[256];

    /* user name for su - */
    dts_username = (char *)IDtouser(dts_uid);

    /* DISPLAY environment */
    sprintf(cmd, "su - %s -c \"/bin/ksh -c 'export DISPLAY=:0;/usr/dt/bin/dtaction LockDisplay'\"", dts_username);

    if( (ret = fork()) == 0 ) {		/* child process */
	exit(system(cmd));		/* run xlock */
    }

    return ret;
}


/*
 * NAME: pmd_lock 
 *
 * FUNCTION: lock pmd_lock file for only pmd can issue 
 *           pm_control_state(PM_CTRL_START_STATE,) 
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *        TRUE - pmd lock file successfully locked.
 *        FALSE - failed to lock pmd lock file.
 */
static int
pmd_lock()
{
    int fd;

    fd = open(lockfile, O_RDWR|O_CREAT, 0600);
    if( lockf(fd, F_TLOCK, 0) != 0 ) {
	return FALSE;
    }
    return TRUE;
}
