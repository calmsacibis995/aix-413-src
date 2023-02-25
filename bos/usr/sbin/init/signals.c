static char sccsid[] = "@(#)16        1.30.2.7  src/bos/usr/sbin/init/signals.c, cmdoper, bos411, 9428A410j 5/24/94 17:20:22";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<sys/types.h>
#include	<signal.h>
#include	<sys/wait.h>
#include	<utmp.h>
#include	<errno.h>
#include	<termio.h>
#include	<ctype.h>
#include	<sys/dir.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include 	<pwd.h>
#include	<usersec.h>
#include	<sys/inode.h>
#include	<sys/user.h>
#include	<sys/ipc.h>
#include	<sys/ldr.h>
#include	<sys/shm.h>
#include	<sys/NLchar.h>
#ifdef TOTLOGIN
#include	<sys/ipc.h>
#include	<sys/sem.h>
#endif TOTLOGIN
#ifdef CSECURITY
#include	<sys/audit.h>
#endif	CSECURITY
#include	<stdio.h>
#include	<sys/license.h>
#include	"init.h"

#include	"init_msg.h"

extern  void    _FloatingReleaseLicense(pid_t);

/****************************/
/****    init_signals    ****/
/****************************/

/*	Initialize all signals to either be caught or ignored.		*/

init_signals()
{
	extern int siglvl(int), alarmclk(void), childeath(void),powerfail(void);
	extern int danger(void);
#ifdef	UDEBUG
	extern int abort();
#endif
	struct sigaction action;
	int	i;

#ifdef	XDEBUG
	debug("We have entered init_signals().\n");
#endif

	/* let's set up our actions for the sigaction calls */
	action.sa_mask.losigs = 0;	/* for safety's sake */
	action.sa_mask.hisigs = 0;	/* for safety's sake */
	action.sa_flags = 0;		/* no flags */

	for (i = 1; i <= SIGMAX; i++) {
		switch(i) {
		case LVLQ:
		case LVL0:
		case LVL1:
		case LVL5:
		case LVL6:
		case LVL7:
		case LVL8:
		case LVL9:
		case SINGLE_USER:
		case LVLN:
		case LVLa:
		case LVLb:
		case LVLc:
			action.sa_handler = (void (*)(int))siglvl;
			sigaction(i, &action,(struct sigaction *)NULL);
			break;
		case LVL2:
		case LVL3:
		case LVL4:
#ifdef	UDEBUG
			action.sa_handler = SIG_DFL;
#else
			action.sa_handler = (void (*)(int))siglvl;
#endif
			sigaction(i, &action,(struct sigaction *)NULL);
			break;
		case SIGTERM:
#ifdef	UDEBUG
			action.sa_handler = SIG_DFL;
#else
			action.sa_handler = SIG_IGN;
#endif
			sigaction(i, &action,(struct sigaction *)NULL);
			break;
		case SIGUSR1:
		case SIGUSR2:
#ifdef	UDEBUG
			action.sa_handler = (void (*)(int))abort;
#else
			action.sa_handler = SIG_IGN;
#endif
			sigaction(i, &action,(struct sigaction *)NULL);
			break;
		case SIGALRM:
			action.sa_flags = SA_RESTART;
			action.sa_handler = (void (*)(int)) alarmclk;
			sigaction(i, &action,(struct sigaction *)NULL);
			break;
		case SIGCLD:
			action.sa_flags = SA_RESTART;
			action.sa_handler = (void (*)(int)) childeath;
			sigaction(i, &action,(struct sigaction *)NULL);
			break;
		case SIGPWR:
			action.sa_handler = (void (*)(int)) powerfail;
			sigaction(i, &action,(struct sigaction *)NULL);
			break;
		case SIGDANGER:
			action.sa_handler = (void (*)(int)) danger;
			sigaction(i, &action,(struct sigaction *)NULL);
			break;
		default:
			action.sa_handler = SIG_IGN;
			sigaction(i, &action,(struct sigaction *)NULL);
		}
	}
	alarmclk();
}

/**********************/
/****    siglvl    ****/
/**********************/

siglvl(int sig)
{
  extern int new_state,cur_state;
  extern int wakeup_flags;
  register struct PROC_TABLE *process;

#ifdef	XDEBUG
  debug("We have entered siglvl().\n");
#endif
  /* If the signal received is a "LVLQ" signal, do not really */
  /* change levels, just restate the current level. */
  if (sig == LVLQ) new_state = cur_state;

  /* If the signal received is something other than "LVLQ", set */
  /* the new level to the value of the signal received. */
  else new_state = sig;

  /* Clear all times and repeat counts in the process table */
  /* since either the level is changing or the user has editted */
  /* the "/etc/inittab" file and wants us to look at it again.  If */
  /* the user has fixed a typo, we don't want residual timing data */
  /* preventing the fixed command line from executing. */
  for (process= &proc_table[0]; process < &proc_table[NPROC] ; process++)
  {
    process->p_time = 0L;
    process->p_count = 0;
  }

  /* Set the flag saying that a "user signal" was received. */
  wakeup_flags |= W_USRSIGNAL;
  return(0);
}

/************************/
/****    alarmclk    ****/
/************************/

alarmclk(void)
{
  extern int time_up;

#ifdef	XDEBUG
  debug("We have entered alarmclk().\n");
#endif
  time_up = TRUE;
  return(0);
}

/*************************/
/****    childeath    ****/
/*************************/

childeath(void)
{
    extern int wakeup_flags;
    register struct PROC_TABLE *process;
    register int pid;
    sigset_t newset;
    int status, i;

    /* Assume that childeath can handle all of the dead children in the world. */

    more_dead_children = 0;

#ifdef	XDEBUG
  debug("We have entered childeath().\n");
#endif
  /* Perform wait to get the process id of the child who died and */
  /* then scan the process table to see if we are interested in */
  /* this process.  Bail when there are no slots left in orphan table */
  /* to avoid losing any orphaned kiddies */
    pid = 0;
    while (orphan_cnt < NPROC &&
                0 < (pid = waitpid(-1, &status, WNOHANG))) {
#ifdef	XDEBUG
    debug("childeath: pid- %d status- %x\n",pid,status);
#endif

    for (process = proc_table, i=0; i < NPROC; process++, i++) {
	if ((process->p_flags & (OCCUPIED | LIVING )) == (OCCUPIED | LIVING) 
	    && process->p_pid == pid) {

        /* Mark this process as having died and store the exit status. */
        /* Also set the wakeup flag for a dead child and break out of */
        /* loop. */
	    process->p_flags &= ~LIVING;
	    process->p_exit = status;
	    wakeup_flags |= W_CHILDEATH;
	    break;
	}
    }

    if (NPROC == i && orphan_cnt < NPROC)       /* didn't find process */
        orphan_tab[orphan_cnt++] = pid;

    _FloatingReleaseLicense(pid);

#ifdef	XDEBUG
    if (process == &proc_table[NPROC]) debug("Didn't find process %d.\n", pid);
#endif
    } /* end while */
    if (orphan_cnt == NPROC) {
        /* Get the current signal set and remove SIGCLD.  We cannot put */
        /* any more orphans in the table because there is no room left. */
        sigemptyset(&newset);
        sigaddset(&newset, SIGCHLD);
        sigprocmask(SIG_BLOCK, &newset, NULL);
        more_dead_children = 1;
    }
    return 0;
}

/*************************/
/****    powerfail    ****/
/*************************/

powerfail(void)
{
  extern int wakeup_flags;

#ifdef	XDEBUG
  debug("We have entered powerfail().\n");
#endif
  wakeup_flags |= W_POWERFAIL;
  return(0);
}

/*************************/
/****    danger    ****/
/*************************/

danger(void)
{
        if (own_pid == SPECIALPID) {
		console(NOLOG, M_DANGER, "Paging space low!\n");
		unload(L_PURGE);
	}
	return(0);
}


/*********************/
/****    timer    ****/
/*********************/

/*	"timer" is a substitute for "sleep" which uses "alarm" and	*/
/*	"pause".							*/

timer(waitime)
register int waitime;

{
  extern int time_up;

  setimer(waitime);
  while (time_up == FALSE) pause();
}

/***********************/
/****    setimer    ****/
/***********************/

int time_up;	/* Flag set to TRUE by alarm interupt routine
     * each time an alarm interupt takes place.
     */

setimer(timelimit)
int timelimit;
{
  alarmclk();
  alarm(timelimit);
  time_up = (timelimit ? FALSE : TRUE);
}


/*
**   Set the tty ownership and mode
**  DON'T CALL reset_tty() FROM A SIGNAL HANDLER
**  init calls UTMP routines and can mess up the UTMP if its modified
**  in the SIGNAL HANDLER!
**
** remove_proc will mark user process dead if spawned by init.
** remove_proc will pass in uptr if proc was spawned by init.
*/
void
reset_tty(int pid, struct utmp *u)
{
    struct stat s;
    char dev[132];
    struct acl *acl;
    struct acl *acl_get();
    int owner = 0, group = 0;

    sprintf(dev, "/dev/%s", u->ut_line);
    if (0 > stat(dev,&s))                /*  not a getty  */
	return;
    if (S_IFCHR != (s.st_mode & S_IFMT))   /*  also not a getty  */
	return;

    if (strncmp(u->ut_line, "pts/", 4) != 0) {
        /* set tty to owner uucp, group uucp.
         * default to root, system if not found.
         */
        if (0 > getuserattr("uucp", S_ID, &owner, SEC_INT)
            && 0 > getuserattr("nuucp", S_ID, &owner, SEC_INT))
	    owner = 0;
        if (0 > getgroupattr("uucp", S_ID, &group, SEC_INT))
            group = 0;
    }
    if (0 > chown(dev, owner, group))
	return;

    if (acl = acl_get (dev)) {
	acl->u_access = 6;
	acl->g_access = 6;
	acl->o_access = owner ? 2 : 6;
	acl->acl_len = ACL_SIZ;
	(void) acl_put(dev, acl, 1);
    }
}

/*
 * NAME: reset_orphan_ttys
 *
 * DESCRIPTION:
 *      reset_orphan_ttys calls reset_tty to change the permissions on
 *      all TTY devices which were owned by orphan processes.  It also
 *      updates the /etc/utmp file to remove the utmp file entry for
 *      the orphan, should one exist.
 */

void
reset_orphan_ttys (void)
{
        int     i;
        struct  utmp    *u;

        /*
         * Rewind the utmp file.  This is needed so the entire file
         * is scanned.
         */

        setutent();

        /*
         * For every entry in the utmp file, compare the pid of the
         * entry to every entry in the orphan_tab array.  Because scanning
         * the array is much faster, step through the file an entry at a
         * time, not through the orphan table.
         */

        while ((u = (struct utmp *)getutent())) {
                for (i = 0;i < orphan_cnt;i++) {

                        /*
                         * See if this utmp entry matches the current
                         * orphan.  Change the permissions back, and clear
                         * out the utmp file if so.
                         */

                        if (u->ut_pid == orphan_tab[i]) {
                                reset_tty (orphan_tab[i], u);

                                if (USER_PROCESS == u->ut_type) {
                                    u->ut_type = DEAD_PROCESS;
                                    pututline(u);
                                }
                        }
                }
        }
}

