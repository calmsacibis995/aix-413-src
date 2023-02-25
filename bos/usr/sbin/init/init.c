static char sccsid[] = "@(#)12  1.61.1.16  src/bos/usr/sbin/init/init.c, cmdoper, bos41J, 9511A_all 3/13/95 18:36:34";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: init
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*	"init" is the general process spawning program.  It reads	*/
/*	/etc/inittab for a script.					*/
/*									*/

#include	<sys/types.h>
#include	<sys/limits.h> 
#include	<signal.h>
#include	<utmp.h>
#include	<errno.h>
#include	<termio.h>
#include	<ctype.h>
#include	<sys/dir.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include 	<pwd.h>
#include	<sys/inode.h>
#include	<sys/user.h>
#include	<stdio.h>
#include	<sys/localedef.h>
#include	<usersec.h>
#include	<locale.h>
#include        <sys/vmker.h>
#include        <nlist.h>
#include        <unistd.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>

#include	"init_msg.h"
nl_catd catd;

#include	"init.h"

#define ENVSIZ	7	/* number of items in the environment */

#ifdef RSTIMER
/* rstimer controls whether or not a timer is needed.
 * this usually occurs when a process (or processes) that init
 * respawns is (are) respawning to rapidly.
 * IF you want to turn on RSTIMER, you'll have to identify where
 * p_count gets cleared.  rstimer should get incremented when 
 * init needs it, and decremented when p_count gets cleared.  
 * There are a few places where p_count gets cleared which have yet to 
 * be identified by RSTIMER.
 */
int rstimer = 0;
#endif

/* function prototypes for init.c */
void single(int defaultlevel);
void remove_proc();
void spawn();
void execcmd(char *command);
void respawn(struct PROC_TABLE *process, struct CMD_LINE *cmd);
struct PROC_TABLE * findpslot(struct CMD_LINE *cmd);
int mask(int lvl);
char level(int state);
void killproc(pid_t pid);
int initialize();
int getlvl();
struct PROC_TABLE * efork(struct PROC_TABLE *process, int modes);
long waitproc(struct PROC_TABLE *process);
struct utmp * account(int state, struct PROC_TABLE *process, char *program);
int error_time(int type);
void userinit(int argc, char **argv);

int NPROC = 2048;
struct PROC_TABLE *proc_table;
int *orphan_tab;

/********************/
/****    main    ****/
/********************/
main(int argc, char **argv)
{
  extern pid_t own_pid;
  extern struct PROC_TABLE dummy;
  extern int prev_state,cur_state,new_state,op_modes;
  extern int wakeup_flags;
  extern int errno;
  extern char level();
  extern unsigned int spawncnt,syncnt,pausecnt;
  extern int rsflag, time_up;
  char utmplock[84];		 /* utmp record string holder */
  int defaultlevel,saved_level;	 /* what the system will boot up into */
  int chg_lvl_flag;		 /* did somebody ask us to change? */
  struct vmkerdata vmker;        /* pager data */
  int mem;
  struct nlist nldata;
  FILE *fp;
  int howto = 0;
#ifdef	XDEBUG
  extern char comment[];
#endif

	(void)setlocale(LC_ALL,"");

#ifdef	UDEBUG
  if (argc == 1) SPECIALPID = getpid();
#endif

  /* Determine if we are process SPECIALPID (the main init) or a */
  /* user invoked init, whose job it is to inform init to change */
  /* levels or perform some other action. */
  if ((own_pid = getpid()) != SPECIALPID) userinit(argc,argv);

  /* Because there is no way that this many processes can be running on a     *
   * small memory system, NPROC should be set according to the memory size.   * 
   * Given that each process always has at least 5 pages in memory (1 pinned, *
   * and 4 pageable), the greatest number of processes running in 8 MB can be *
   * 410 (and this is extremely generous).  One susggestion for the setting   *
   * of NPROC is to increase NPROC by 512 for each 8 MB and cap it at 4096.   *
   *                                                                          *
   * System Size                  NPROC                                       *
   *    16 MB                      1024                                       *
   *    24 MB                      1536                                       *
   *    32 MB                      2048                                       *
   *    48 MB                      3072                                       *
   *  =>64 MB                      4096                                       */

  if ((mem = open("/dev/mem", O_RDONLY)) == -1)
    console(NOLOG, M_OPEN, "Can't open %s. errno: %d\n", "/dev/mem", errno);
  else {
    nldata.n_name = "vmker";
    nldata.n_value = 0;
    (void) knlist(&nldata, 1, sizeof(struct nlist));
    (void) lseek(mem, (off_t) nldata.n_value, SEEK_SET);
    if (read(mem, &vmker, sizeof(vmker)) == sizeof(vmker)) {
      mem = vmker.nrpages - vmker.badpages;
      if (mem <= 4096)
        NPROC = 1024;
      else if (mem <= 6144)
        NPROC = 1536;
      else if (mem <= 8192)
        NPROC = 2048;
      else if (mem <= 12288)
        NPROC = 3072;
      else
        NPROC = 4096;
    }
  }

  proc_table = (struct PROC_TABLE *)malloc(NPROC * sizeof(struct PROC_TABLE));
  orphan_tab = (int *)malloc(NPROC * sizeof(int));

  /* If an argument has been passed when (real) init is first created,
     check the argument for single user mode.       */ 
  if ( argc == 2 ) {
	char c;
	c = argv[1][0];
	if( c == 's' || c == 'S' || c == 'm' || c == 'M' ) {
		int fd;
		char lev[2];

		howto = SINGLE_USER;
		if ((fd = open("/etc/.init.state", O_WRONLY | O_CREAT | O_TRUNC, 0644)) >= 0) {
			lev[0] = 'S';
			lev[1] = '\n';
			write(fd, lev, 2);
			close(fd);
		}
	}
  }

#ifdef	XDEBUG
  debug("We are past userinit().\n");
#endif

  /* Since the machine is just comming up remove the lock on the WTMP file */
#ifndef UDEBUG
  strcpy(UTMP_LCK,UTMP);
  strcat(UTMP_LCK,".lck");
#endif
  unlink(UTMP_LCK);

  /* Set up the initial states and see if there is a default level */
  /* supplied in the "/etc/inittab" file. */
  defaultlevel = initialize();
  chg_lvl_flag = FALSE;

  /* If single user mode is requested, save the current defaultlevel
     in saved_level, and set defaultlevel to -1.  Then new_state will 
     be set to SINGLE_USER later on and the default level will be restored 
     from saved_level.  If the defaultlevel is already -1, it needs 
     to be set to SINGLE_USER later.  This is done by setting saved_level
     to SINGLE_USER. */
  if( howto == SINGLE_USER ) {
	saved_level = (defaultlevel != -1)? defaultlevel: SINGLE_USER;
	defaultlevel = -1;
  }

#ifdef	XDEBUG
  debug("Debug version of init starting-pid = %d\n",SPECIALPID);
#endif

  /* 
  * If there is no default level supplied, ask the user to supply one.
  * Here we will also set up our runlevel indicators new_state (what are
  * we going to) and defaultlevel (where we start at boot time). cur_state
  * is where we are presently at, and prev_state is the last state we were
  * in.
  */ 
  if (defaultlevel == 0)
    new_state = getlvl();
  else if (defaultlevel == -1)
  {
    new_state = SINGLE_USER;
    defaultlevel = (howto == SINGLE_USER) ? saved_level:SINGLE_USER;
  }
  else
    new_state = defaultlevel;
#ifdef	XDEBUG
  debug("We have completed getting new_state.\n");
#endif

  if (new_state == SINGLE_USER)
  {
    single(defaultlevel);
    chg_lvl_flag = TRUE;
  } 
  else 
  {
    int fd;
    char lev[2];

    prev_state = cur_state;
    if(cur_state >= 0)
    {
      n_prev[cur_state]++;
      prior_state = cur_state;
    }
    cur_state = new_state;
    if ((fd = open("/etc/.init.state", O_WRONLY | O_CREAT | O_TRUNC, 0644))
		>= 0) {
	lev[0] = level(cur_state);
	lev[1] = '\n';
	write(fd, lev, 2);
	close(fd);
    }
  }
#ifdef	XDEBUG
  debug("We are preparing for utmplock.\n");
#endif

  /* Initialize the "utmp" file and put in the boot time. */
  /* Set the umask so that the utmp file is created 644. */
  umask((mode_t)022);
  utmpname(UTMP);			/* set the utmp name for all time */
  if ((fp = fopen(UTMP,"w+")) == NULL) { /* truncate utmp file */
    console(LOG, M_CREATE,"Cannot create %s\n",UTMP,0);
    /* Without "utmp" file default to single user mode. */
    cur_state = SINGLE_USER;
  } else {
    fclose(fp);
    account(BOOT_TIME,&dummy,NULL);	/* Put Boot Entry in "utmp" */
    account(RUN_LVL,&dummy,NULL);	/* Make the run level entry */
    endutent();
  }
  umask((mode_t)0);	/* Allow later files to be created normally. */
#ifdef	XDEBUG
  debug("We are at the beginning of the main process loop.\n");
#endif

  /* Here is the beginning of the main process loop. */
  for (;;)
  {
    if (new_state == LVLN) /* if level N then do nothing until recieve a new level */
    {
      setimer(SLEEPTIME);
      pause();
      pausecnt++;
    }
    else 
    {

    /* If "normal" mode, check all living processes and initiate */
    /* kill sequence on those that should not be there anymore. */
    if (op_modes == NORMAL_MODES && cur_state != LVLa
        && cur_state != LVLb && cur_state != LVLc) remove_proc();
#ifdef	XDEBUG
  debug("We have completed remove_proc().\n");
#endif

    /* If a change in run levels is the reason we awoke, now do */
    /* the accounting to report the change in the utmp file. Also */
    /* report the change on the system console. */
    if (chg_lvl_flag)
    {
      int fd;
      char lev[2];

      if ((fd = open("/etc/.init.state", O_WRONLY | O_CREAT | O_TRUNC, 0644))
		>= 0) {
      	lev[0] = level(cur_state);
	lev[1] = '\n';
	write(fd, lev, 2);
	close(fd);
      }
      chg_lvl_flag = FALSE;
      account(RUN_LVL,&dummy,NULL);
      console(NOLOG, M_NEWLVL, "New run level: %c\n",level(cur_state));
    }
#ifdef	XDEBUG
  debug("We have completed account().\n");
#endif

    /* Scan the inittab file and spawn and respawn processes that */
    /* should be alive in the current state. */
    spawn();
    if (rsflag)
    {
      rsflag = 0;
      spawncnt++;
    }
    if (cur_state == SINGLE_USER)
    {
#ifdef	XDEBUG
  debug("We have gone into SINGLE-USER mode.\n");
#endif
      single(defaultlevel);
      if (cur_state != prev_state && cur_state != LVLa && cur_state != LVLb
          && cur_state != LVLc)
      {
        chg_lvl_flag = TRUE;
        continue;
      }
    }

    /* If powerfail signal was received during last sequence, */
    /* set mode to powerfail. When "spawn" is entered the first */
    /* thing it does is check "powerhit".  If it is in PF_MODES */
    /* then it clears "powerhit" and does a powerfail.  If */
    /* it is not in PF_MODES, it puts itself in PF_MODES and */
    /* then clears "powerhit". If "powerhit" sets again while */
    /* "spawn" is working on a powerfail, the following code */
    /* will see that "spawn" tries to execute the sequence */
    /* again.  This ensures that the powerfail sequence will be */
    /* successfully completed before further processing. */
    if (wakeup_flags & W_POWERFAIL)
    {
      op_modes = PF_MODES;

      /* Make sure that cur_state != prev_state so ONCE/WAIT types work. */
      prev_state = 0;

      /* If "spawn" was not just called while in "normal" mode, then */
      /* set the mode to "normal" and call again to check normal states. */
    } 
    else if (op_modes != NORMAL_MODES)
    {

      /* If we have just finished a powerfail sequence(which had the */
      /* prev_state == 0), set the prev_state = cur_state before the */
      /* next pass through. */
      if (op_modes == PF_MODES)
        prev_state = cur_state;
      op_modes = NORMAL_MODES;

      /* "spawn was last called with "normal" modes. */
      /* If it was a change of levels that awakened us and the new */
      /* level is one of the demand levels, LVL[a-c], then reset */
      /* the cur_state to the previous state and do another scan to */
      /* take care of the usual "respawn" actions. */
    } 
    else if (cur_state == LVLa || cur_state == LVLb || cur_state == LVLc)
    {
      if(cur_state >= 0) n_prev[cur_state]++;
      cur_state = prior_state;
      prior_state = prev_state;
      prev_state = cur_state;
      account(RUN_LVL,&dummy,NULL);

      /* At this point "init" is finished with all actions for the */
      /* current wakeup.  */
    } 
    else 
    {
      prev_state = cur_state;

      /* Now pause until there is a signal of some sort.  Signals are */
      /* disallowed until the pause system call actually is performed */
      /* but then all signals are treated until we return from pause. */
      if (wakeup_flags == 0) {
#ifdef RSTIMER
	if (rstimer)
#endif
	    setimer(SLEEPTIME);
        pause();
        pausecnt++;
      }

      setimer(0);

      /*    Now install the new level, if a change in level happened and    */
      /*    then allow signals again while we do our normal processing.     */
      if (wakeup_flags & W_USRSIGNAL && new_state != LVLN)
      {
#ifdef	XDEBUG
        debug("\nmain\tSignal-new: %c cur: %c prev: %c\n",
            level(new_state),level(cur_state), level(prev_state));
#endif
        /* Set flag so that we know to change run level in utmp file */
        /* all the old processes have been removed.  Do not set the flag */
        /* if a "telinit {Q | a | b | c}" was done or a telinit to the */
        /* same level at which init is already running (which is the */
        /* same thing as a "telinit Q"). */
        if (new_state != cur_state)
          if(new_state == LVLa || new_state == LVLb || new_state == LVLc)
          {
            prev_state = prior_state;
            prior_state = cur_state;
            cur_state = new_state;
            account(RUN_LVL,&dummy,NULL);
          }
          else 
          {
            prev_state = cur_state;
            if(cur_state >= 0)
            {
              n_prev[cur_state]++;
              prior_state = cur_state;
            }
            cur_state = new_state;
            chg_lvl_flag = TRUE;
          }
        new_state = 0;
      }

      /*    If we awoke because of a powerfail, change the operating mode   */
      /*    to powerfail mode.                                              */
      if (wakeup_flags & W_POWERFAIL)
        op_modes = PF_MODES;

      /* Clear all wakeup reasons. */
      wakeup_flags = 0;
    }
    }
  }
}

/**********************/
/****    single    ****/
/**********************/
void
single(int defaultlevel)
{
  register struct PROC_TABLE *su_process;
  extern struct PROC_TABLE dummy;
  struct PROC_TABLE *efork();
  extern long waitproc();
  extern int errno;
  extern int new_state,cur_state,prev_state;
  extern int wakeup_flags;
  struct passwd *pwd;
  struct sigaction action;
  extern int childeath(void);
  extern char *enterpwd();
  char *password, *crypt();
  void endpwent();
  int state, fd;
  char c;

#ifdef	XDEBUG
  debug("We have entered single().\n");
#endif
  for (;;)
  {
    console(NOLOG, M_SUSER, "SINGLE-USER MODE\n");
    action.sa_mask.losigs = 0;	/* for safety's sake */
    action.sa_mask.hisigs = 0;	/* for safety's sake */
    action.sa_flags = 0;
    action.sa_handler = SIG_DFL;
    sigaction(SIGCLD,&action,(struct sigaction *)NULL);
    while ((su_process = efork(NULLPROC,NOCLEANUP)) == NO_ROOM)
      pause();
    action.sa_flags = SA_RESTART;
    action.sa_handler = (void (*)(int))childeath;
    sigaction(SIGCLD,&action,(struct sigaction *)NULL);
    if (su_process == NULLPROC)
    {
      /* get the root password. We will die and do the whole thing over
	 again if we don't get what we want. */
      if((pwd = getpwnam("root")) == NULL)
      {
        if((pwd = getpwnam("su")) == NULL)
	{
	  console(NOLOG, M_NENTRY,
	    "Cannot find entries for ROOT or SU in /etc/passwd.\n");
          exit(1);
	}
      }
#ifdef	XDEBUG
  debug("Got pwnam. pwd->pw_passwd = %s.\n", pwd->pw_passwd);
#endif

      if( pwd->pw_passwd[0] != '\0' )
      {
        catd = catopen(MF_INIT,NL_CAT_LOCALE);
        password = enterpwd(catgets(catd,MS_INIT, M_PWD, "Password:"));
        catclose(catd);
#ifdef	DEBUG1
  debug("Completed enterpwd(). password = %s.\n", password);
#endif
        if( (strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)) != 0)) 
        {
	  console(NOLOG, M_SORRY, "Sorry.\n");
	  exit(1);
        }
      }
      endpwent();

      openconsole();

      /*
       * Attach to the console device, using the proper tty name. /dev/console
       * may have opened the tty with O_NOCTTY, which is neccesary to prevent
       * daemons from attaching themselves to the console.
       */
      fd = open(ttyname(0), 0);
      if (fd != -1)
	  close(fd);

      /* Execute the "su" program. */
      execlp(SU,SU,"-",0);
      console(NOLOG, M_EXEC,"execlp of %s failed; errno = %d\n", SU, errno);
      timer(5);
      exit(1);
    }

    /* If we are the parent, wait around for the child to die or for */
    /* "init" to be signaled to change levels. */
    while (waitproc(su_process) == FAILURE)
    {

      /* Did we waken because a change of levels?  If so, kill the  */
      /* child and then exit. */
      if (wakeup_flags & W_USRSIGNAL)
      {
        if (new_state != SINGLE_USER && new_state >= LVL0 && new_state <= LVL9)
        {
          kill(su_process->p_pid,SIGKILL);
          prev_state = cur_state;
          if(cur_state >= 0)
          {
            n_prev[cur_state]++;
            prior_state = cur_state;
          }
          cur_state = new_state;
          new_state = 0;
          wakeup_flags = 0;
          su_process->p_flags &= ~NOCLEANUP;
          return;
        }
      }

      /* All other reasons for waking are ignored when in SINGLE_USER */
      /* mode.  The only child we are interested in is being waited */
      /* for explicitly by "waitproc". */

      wakeup_flags = 0;
    }

    /* Since the su user process died and the level hasn't been */
    /* changed by a signal, either request a new level from the user */
    /* if default one wasn't supplied, or use the supplied default */
    /* level. That is, if they really WANT to go! */
    /* openconsole() will close utmp file descriptor, unbeknownst to */
    /* getutid(), etc., so do it first here. */
    endutent();
    openconsole();
    catd = catopen(MF_INIT,NL_CAT_LOCALE);
    fprintf(stdout, catgets(catd,MS_INIT, M_SUPROMPT,"Do you wish to leave SINGLE-USER mode?\nEnter (Y)es,(N)o, or <cr> for the default runlevel: "));
    catclose(catd);
    fflush(stdout);

    /* Get a character from the user which isn't a space or tab. */
    while((fscanf(stdin,"%c",&c) != 1) || (c == '\t') || (c == ' '));
    c &= 0x7f;

    /* If the character is a [Yy<cr>], then YES. Else, put them
     * back into SINGLE-USER mode.
     */
    if (c == '\n')
      state = defaultlevel;
    else if (c == 'Y' || c == 'y')
      state = getlvl();
    else
      state = SINGLE_USER;

    if (state != SINGLE_USER)
    {

      /* If the new level is not SINGLE_USER, then exit, otherwise */
      /* go back and make up a new "su" process. */
      prev_state = cur_state;
      if(cur_state >= 0)
      {
        n_prev[cur_state]++;
        prior_state = cur_state;
      }
      cur_state = state;
      return;
    }
  }
}

/**********************/
/****  remove_proc  ****/
/**********************/
/*	"remove_proc" scans thru "proc_table" and performs cleanup. If  */
/*	there is a process in the table, which shouldn't be here at	*/
/*	the current runlevel, then "remove_proc" kills the processes.	*/
void
remove_proc()
{
  register struct PROC_TABLE *process;
  extern int op_modes,prev_state,cur_state,new_state;
  struct CMD_LINE cmd;
  char cmd_string[MAXCMDL];
  int change_level, i;
  extern int time_up;
  extern char *vetchar();
  sigset_t newset, oldset; 

#ifdef	XDEBUG
  debug("We have entered remove_proc().\n");
#endif
  change_level = (cur_state != prev_state ? TRUE : FALSE);

  /* Clear the TOUCHED flag on all entries so that when we have */
  /* finished scanning /etc/inittab, we will be able to tell if */
  /* we have any processes for which there is no entry in */
  /* /etc/inittab. */

  for (process = proc_table, i = 0; i < NPROC; process++, i++)
    process->p_flags &= ~TOUCHED;

  /* Scan all /etc/inittab entries. */
  while(TRUE == getcmd(&cmd, &cmd_string[0])) {

    /* Scan for process which goes with this entry in /etc/inittab. */
    for (process = proc_table, i = 0; i < NPROC; process++, i++) {

      /* Does this slot contain the process we are looking for? */
      if ((process->p_flags & OCCUPIED) && id_eq(process->p_id,cmd.c_id)) {
#ifdef	XDEBUG
        debug("remove_proc- id:%s pid: %d time: %lo %d %o %o\n",
            vetchar(&process->p_id[0]),process->p_pid,
            process->p_time,process->p_count,
            process->p_flags,process->p_exit);
#endif

        /* Is the cur_state SINGLE_USER or */
        /* is this process marked as "off" or was this process was started */
        /* by some other mechanism than the LVLa, LVLb, LVLc mechanism, */
        /* and the current level does not support this process? */
        if ((cur_state == SINGLE_USER)
            || (cmd.c_action == M_OFF)
            || ((cmd.c_levels & mask(cur_state)) == 0
            && (process->p_flags & DEMANDREQUEST) == 0)) {
          if (process->p_flags & LIVING) {

            /* Touch this entry so that we will know that we've treated it. */
            /* NOTE: Processes which are already dead at this point, but */
            /* should not be restarted are left untouched. This causes */
            /* their slot to be freed later after accounting is performed. */
            process->p_flags |= TOUCHED;

            /* If this process has already been killed before, but for some */
            /* reason hasn't disappeared yet, don't kill it again.  Only kill */
            /* it if the KILLED flag hasn't been set. */
            if ((process->p_flags & KILLED) == 0) {
              /* If this is a change of levels call, don't fork a killing */
              /* process for each process that must die.  Send the first */
              /* warning signal yourself and mark the process as warned. If */
              /* any warned processes fail to die in TWARN seconds, then */
              /* kill them. */
              if (change_level)
              {
              	process->p_flags |= WARNED;
              	kill(process->p_pid,SIGTERM);

              	/* If this isn't change of levels, then fork killing process */
              	/* which will worry about details of killing the specified */
              	/* process.  This allows "init" to continue work instead of */
              	/* pausing for TWARN seconds each pass through this routine. */
              } 
              else killproc(process->p_pid);

              /* Mark the process as having been sent it's kill signals. It */
              /* should show up as dead shortly, but just to be safe.... */
              process->p_flags |= KILLED;
            }
          }

          /* This process can exist at the current level.  If it is also */
          /* still alive or a DEMANDREQUEST, TOUCH it so that will be left */
          /* alone.  If it is dead and not a DEMANDREQUEST, leave it */
          /* untouched so that it will be accounted and cleaned up later */
          /* on in "remove_proc".  Dead DEMANDREQUESTS will be accounted, but */
          /* not freed. */
        } 
        else if (process->p_flags & (LIVING | NOCLEANUP | DEMANDREQUEST))
          process->p_flags |= TOUCHED;

        break;
      }
    }
  }

  /* If this was a change of levels call, scan through the process */
  /* table for processes that were warned to die.  If any are found */
  /* that haven't left yet, sleep for TWARN seconds and then send */
  /* final terminations to any that haven't died yet. */
  if (change_level)
  {

    /* Set the alarm for TWARN seconds on the assumption that there */
    /* will be some that need to be waited for.  This won't harm */
    /* anything except we are guarenteed to wakeup in TWARN seconds */
    /* whether we need to or not. */
    setimer(TWARN);

    /* Scan for processes which should be dying.  We hope they will */
    /* die without having to be sent a SIGKILL signal. */
    for (process = proc_table, i = 0; i < NPROC; process++, i++) {
      /* If this process should die, hasn't yet, and the TWARN time */
      /* hasn't expired yet, wait around for process to die or for */
      /* timer to expire. */
      while ((time_up == FALSE) && (process->p_flags & 
             (WARNED | LIVING | OCCUPIED)) == (WARNED | LIVING | OCCUPIED))
        pause();
    }

    /* If we reached the end of the proc table without the timer */
    /* expiring, then there are no processes which will have to be */
    /* sent the SIGKILL signal.  If the timer has expired, then it is */
    /* necessary to scan the table again and send signals to all */
    /* processes which aren't going away nicely. */
    if (time_up == TRUE) 
      for (process = proc_table, i = 0; i < NPROC; process++, i++) {
        /* Is this a WARNED process that hasn't died yet? */
        if ((process->p_flags & (WARNED | LIVING | OCCUPIED)) == 
            (WARNED | LIVING | OCCUPIED))
          kill(process->p_pid,SIGKILL);
      }
    setimer(0);
  }

  /* Rescan the proc_table for two kinds of entry, those marked */
  /* as LIVING, NAMED, and which don't have an entry in */
  /* /etc/inittab (haven't been TOUCHED by the above scanning), and */
  /* haven't been sent kill signals, and those entries marked as */
  /* not LIVING, NAMED.  The former processes should be killed. */
  /* The latter entries should have DEAD_PROCESS accounting done */
  /* on them and the slot cleared. */
  for (process = proc_table, i = 0; i < NPROC; process++, i++) {
    if ((process->p_flags & (LIVING | NAMED | TOUCHED | KILLED | OCCUPIED)) 
	== (LIVING | NAMED | OCCUPIED)) {
      killproc(process->p_pid);
      process->p_flags |= KILLED;
    } else if ((process->p_flags & (LIVING | NAMED | OCCUPIED)) 
	== (NAMED | OCCUPIED)) {
      struct utmp *uptr;
      uptr = account(DEAD_PROCESS,process,NULL);
      reset_tty(process->p_pid, uptr);

      /* If this named process hasn't been TOUCHED, then free the space. */
      /* It has either died of it's own accord, but isn't respawnable */
      /* or was killed because it shouldn't exit at this level. */
      if ((process->p_flags & TOUCHED) == 0)
        process->p_flags = 0;
    }
  }

    /* block signals (SIGCLD) while we process orphans
     * childeath will modify orphan_tab AND orphan_cnt !!!
     * if childeath finds the orphan table full, it blocks SIGCLD,
     * so you MUST unblock it here.
     */
    sigemptyset(&newset);
    sigaddset(&newset, SIGCLD);
    sigprocmask(SIG_BLOCK, &newset, NULL);
    /*
     * childeath() sets more_dead_children whenever the orphan table
     * is too full to bury the latest dead kiddie.  Running reset_orphan_ttys
     * clears out the orphan table, making room for any more dead orphans.
     */
    while (1) {
      reset_orphan_ttys ();
      orphan_cnt = 0;
      if (more_dead_children)
        childeath ();
      else
        break;
    }

    /* done processing orphans, re-enable SIGCLD. */
    sigprocmask(SIG_UNBLOCK, &newset, NULL);
}

/*********************/
/****    spawn    ****/
/*********************/
/*	"spawn" scans /etc/inittab for entries which should be run at	*/
/*	this mode.  If a process which should be running is found not	*/
/*	to be running, then it is started.				*/
void
spawn()
{
  extern struct PROC_TABLE *findpslot();
  register struct PROC_TABLE *process;
  struct CMD_LINE cmd;
  char cmd_string[MAXCMDL];
  short lvl_mask;
  extern int cur_state,prev_state,op_modes;
  extern int wakeup_flags;
  extern long waitproc();
#ifdef	XDEBUG
  extern char level();
  extern char *ctime(),*vetchar();
#endif

#ifdef	XDEBUG
  debug("We have entered spawn().\n");
#endif
  /* First check the "powerhit" flag.  If it is set, make sure */
  /* the modes are PF_MODES and clear the "powerhit" flag. */
  /* Avoid the possible race on the "powerhit" flag by disallowing */
  /* a new powerfail interupt between the test of the powerhit */
  /* flag and the clearing of it. */
  if (wakeup_flags & W_POWERFAIL)
  {
    wakeup_flags &= ~W_POWERFAIL;
    op_modes = PF_MODES;
  }
  lvl_mask = mask(cur_state);

#ifdef	DEBUG1
  debug("spawn\tSignal-new: %c cur: %c prev: %c\n",level(new_state), level(cur_state),level(prev_state));
  debug("spawn- lvl_mask: %o op_modes: %o\n",lvl_mask,op_modes);
#endif

  /* Scan through all the entries in /etc/inittab. */
  while (getcmd(&cmd, cmd_string) == TRUE)
  {
    /* Find out if there is a process slot for this entry already. */
    if ((process = findpslot(&cmd)) == NULLPROC)
    {
      /* Only generate an error message once every WARNFREQUENCY seconds */
      /* when the internal process table is full. */
      if (error_time(FULLTABLE))
    	console(NOLOG, M_PTAB,"Internal process table is full.\n");
      continue;
    }

    /* If there is an entry, and it is marked as DEMANDREQUEST, one */
    /* of the levels a,b, or c is in its levels mask, and the action */
    /* field is ONDEMAND and ONDEMAND is a permissable mode, and */
    /* the process is dead, then respawn it. */
    if (((process->p_flags & (LIVING | DEMANDREQUEST)) == DEMANDREQUEST)
        && (cmd.c_levels & (MASKa | MASKb | MASKc))
        && (cmd.c_action & op_modes) == M_ONDEMAND)
    {
      respawn(process,&cmd);
      /* Now finished with this entry. */
      continue;	
    }

#ifdef	DEBUG1
    debug("process:\t%s\t%05d\n%s\t%d\t%o\t%o\n",
        vetchar(&process->p_id[0]),process->p_pid,
        ctime(&process->p_time),process->p_count,
        process->p_flags,process->p_exit);
    debug("cmd:\t%s\t%o\t%o\n\"%s\"\n",vetchar(&cmd.c_id[0]),
        cmd.c_levels,cmd.c_action,cmd.c_command);
#endif

    /* If the action is not an action we are interested in, skip the entry. */
    if ((cmd.c_action & op_modes) == 0)
    {
      continue;
    }
    if (process->p_flags & LIVING)
    {
      continue;
    }
    if ((cmd.c_levels & lvl_mask) == 0)
    {
      continue;
    }

    /* If the modes are the normal modes (ONCE, WAIT, RESPAWN, OFF, */
    /* ONDEMAND) and the action field is either OFF or the action */
    /* field is ONCE or WAIT and the current level is the same as the */
    /* last level, then skip this entry.  ONCE and WAIT only get run */
    /* when the level changes. */
    if ((op_modes == NORMAL_MODES)
        && (cmd.c_action == M_OFF || cmd.c_action == M_HOLD || (cmd.c_action & (M_ONCE | M_WAIT))
        && cur_state == prev_state))
    {
      continue;
    }

    /* At this point we are interested in performing the action for */
    /* this entry.  Actions fall into two categories, spinning off */
    /* a process and not waiting, and spinning off a process and */
    /* waiting for it to die. */
    /* If the action is ONCE, RESPAWN, ONDEMAND, POWERFAIL, or BOOT */
    /* then spin off a process, but don't wait. */
    if (cmd.c_action & (M_ONCE | M_RESPAWN | M_PF | M_BOOT))
      respawn(process,&cmd);

      /* The action must be WAIT, BOOTWAIT, or POWERWAIT, therefore */
      /* spin off the process, but wait for it to die before continuing. */
    else 
    {
      respawn(process,&cmd);
      while (waitproc(process) == FAILURE);
      account(DEAD_PROCESS,process,NULL);
      process->p_flags = 0;
    }
  }
}

/***********************/
/****    execcmd    ****/
/***********************/
/* get the environment out of /etc/environment and exec the current command*/
void
execcmd(char *command)
{
	char	buf[512], *temp, *bptr;
	FILE	*fd;
	int 	i = 0;
        int     j = 0;
	int 	count=ENVSIZ;
	char 	**env;	/* environment pointer */

	env = (char **) malloc(sizeof(char *) *ENVSIZ);
	if ((fd = fopen("/etc/environment","r")) != NULL) 
	{
	  while(fgets(buf,sizeof(buf),fd) != NULL)
	  {
	    if ((buf[0] == '#') || (strchr (buf, '=') == 0)) 
		continue;   /* skip comments and lines without an = */
	    buf[strlen(buf)-1] = '\0';     /* remove newline */
	    bptr = buf;
	    while (iswspace(bptr[0]))    /* skip leading whitespaces */
	      bptr++;
	    if ((strlen(bptr) == 0) || (bptr[0] == '=')) 
		 continue;  /* skip blank lines and lines starting with an = */

            if (((char *)strstr(bptr,"= ") != NULL) || 
				((char *)strstr(bptr," =") != NULL))
		    continue;

	    temp = (char *) malloc(strlen(bptr)+1);
	    strcpy(temp,bptr);
	    if (i >= count-1)
	    {
	      env = (char **) realloc(env,sizeof(char*) * (count+5));
	      count += 5;
	    }
	    env[i++] = temp;
	  } /* end while */
	  env[i] = NULL;
	  fclose(fd);
	}	/* end if */
	else
	  env[0] = NULL;
	execle("/bin/sh","sh","-c",command,0,&env[0]);
}

/***********************/
/****    respawn    ****/
/***********************/
/*	"respawn" spawns a shell, inserts the information about the	*/
/*	process into the proc_table, and does the startup accounting.	*/
void
respawn(struct PROC_TABLE *process, struct CMD_LINE *cmd)
{
  int modes;
  extern int childeath(void);
  extern int cur_state,errno;
  extern char *strrchr();
  extern struct PROC_TABLE *efork();
  struct PROC_TABLE tmproc,*oprocess;
  time_t now;
  static char *envp[] = { "PATH=/bin:/etc:/usr/bin",0 };
  extern int rsflag;
  struct sigaction action;
  char *cmd_name;

#ifdef	DEBUG1
extern char *vetchar();
debug("**  respawn  **  id:%s\n",vetchar(&process->p_id[0]));
#endif
#ifdef	XDEBUG
debug("We have entered respawn().\n");
#endif

/* The modes to be sent to "efork" are 0 unless we are spawning */
/* a LVLa, LVLb, or LVLc entry or we will be waiting for the */
/* death of the child before continuing. */
modes = NAMED;
if ((process->p_flags & DEMANDREQUEST) || cur_state == LVLa
   || cur_state == LVLb || cur_state == LVLc)
modes |= DEMANDREQUEST;
if ((cmd->c_action & (M_SYSINIT | M_WAIT | M_BOOTWAIT | M_PWAIT)) != 0)
modes |= NOCLEANUP;

/* If this is a respawnable process, check the threshold */
/* information to avoid excessive respawns. */
if (cmd->c_action & M_RESPAWN)
{
    /* Add the NOCLEANUP to all respawnable commands so that the  */
    /* information about the frequency of respawns isn't lost. */
    modes |= NOCLEANUP;
    time(&now);

    /* If no time is assigned, then this is the first time this */
    /* command is being processed in this series. Assign the current time. */
    if (process->p_time == 0L)
      process->p_time = now;

    /* Have we just reached the respawn limit? */
    if (process->p_count++ == SPAWN_LIMIT) {
	/* If so, have we been respawning it too rapidly? */
        if (now > process->p_time) {
	    if ((now - process->p_time) < SPAWN_INTERVAL) {
		/* If so, generate an error message */
		console(LOG, M_RAPID, 
		    "Command is respawning too rapidly. Check for possible errors.\nid:%6s \"%s\"\n", cmd->c_id, &cmd->c_command[EXEC]);
		process->p_time = now + SLEEPTIME-1;	/* delay a short period */
#ifdef RSTIMER
		rstimer++;	/* we need a timer */
#endif 
		return;
	    } 
	} else
	    return; /* process is still respawning, don't print a message for now ... */
	process->p_time = now;
	process->p_count = 0;
#ifdef RSTIMER
	rstimer--;	/* no need for timer */
#endif 

      /* If this process has been respawning too rapidly and the */
      /* inhibit time limit hasn't expired yet, refuse to respawn. */
    } else if (process->p_count > SPAWN_LIMIT) {
	/* p_time is now a delta in the future to delay before respawning */
	if (0 > (now - process->p_time))
	    return;
	process->p_time = now + SPAWN_INTERVAL;
	process->p_count = 0;
#ifdef RSTIMER
	rstimer--;	/* no need for timer */
#endif 
    }

    rsflag = TRUE;
  }

  /* Spawn a child process to execute this command. */
  action.sa_mask.losigs = 0;	/* for safety's sake */
  action.sa_mask.hisigs = 0;	/* for safety's sake */
  action.sa_flags = 0;
  action.sa_handler = SIG_DFL;
  sigaction(SIGCLD,&action,(struct sigaction *)NULL);
  oprocess = process;
  while ((process = efork(oprocess,modes)) == NO_ROOM) pause();

  /* If we are the child, close up all the open files and set up the */
  /* default standard input and standard outputs. */
  if (process == NULLPROC)
  {
    int i;
    FILE *fp;

    /* Perform the accounting for the beginning of a process. */
    /* Note that all processes are initially "INIT_PROCESS"es.  Getty */
    /* will change the type to "LOGIN_PROCESS" and login will change */
    /* it to "USER_PROCESS" when they run. */
    strncpy(tmproc.p_id, cmd->c_id, IDENT_LEN);
    tmproc.p_pid = getpid();
    tmproc.p_exit = 0;
/*  if ((cmd_name = strrchr(&cmd->c_command[EXEC], '/')))
      ++cmd_name;
    else
      cmd_name = &cmd->c_command[EXEC]; ====== D16732 ===== */
    account(INIT_PROCESS,&tmproc,cmd->c_id);  /*  D16732  */

    /* Now "exec" a shell with -c and the QUOTED command from /etc/inittab. */
    for (i = 0, fp = stdin; i < LCLOSE; i++, fp++)
      fclose(fp);
    /*
     * execle(SH,"INITSH","-c", cmd->c_command, 0,&envp[0]);
     *
     * replace the above exec with a call to execcmd() so
     * the environment variables from /etc/environment will
     * be available to the commands in inittab.  The main
     * use will be so that cron will know about the TimeZone...
     */

    execcmd(cmd->c_command);

    /* If the "exec" fails, print an error message. */
    console(NOLOG, M_CMDFAIL, "Command\n\"%s\"\n failed to execute.  errno = %d (exec of shell failed)\n", cmd->c_command, errno);

    /* Don't come back so quickly that "init" hasn't had a chance to */
    /* complete putting this child in "proc_table". */
    timer(20);
    exit(1);

  } 
  else 
  {
    /* We are the parent, therefore insert the necessary information */
    /* in the proc_table. */
    strncpy(process->p_id, cmd->c_id, IDENT_LEN);
  }
  action.sa_flags = SA_RESTART;
  action.sa_handler = (void (*)(int))childeath;
  sigaction(SIGCLD,&action,(struct sigaction *)NULL);
  return;
}

/************************/
/****    findpslot    ****/
/************************/
/*	findpslot() finds the old slot in the process table for the	*/
/*	command with the same id, or it finds an empty slot.		*/
struct PROC_TABLE *
findpslot(register struct CMD_LINE *cmd)
{
  register struct PROC_TABLE *process,*empty;
  int i;

#ifdef	XDEBUG
  debug("We have entered findpslot().\n");
#endif
  for(empty= NULLPROC,process= &proc_table[0] ; process < &proc_table[NPROC];process++)
  {
    if ((process->p_flags & OCCUPIED) && id_eq(process->p_id,cmd->c_id)) break;

    /* If the entry is totally empty and "empty" is still 0, remember */
    /* where this hole is and make sure the slot is zeroed out. */
    if (empty == NULLPROC && (process->p_flags & OCCUPIED) == 0)
    {
      empty = process;
      for (i = 0; i < IDENT_LEN; i++)
	      process->p_id[i] = '\0';
      process->p_pid = 0;
      process->p_time = 0L;
      process->p_count = 0;
      process->p_flags = 0;
      process->p_exit = 0;
    }
  }

  /* If there is no entry for this slot, then there should be */
  /* an empty slot.  If there is no empty slot, then we've run out */
  /* of proc_table space.  If the latter is true, empty will be NULL */
  /* and the caller will have to complain. */
  if (process == &proc_table[NPROC])
  {
    process = empty;
  }
  return(process);
}

/********************/
/****    mask    ****/
/********************/
int
mask(int lvl)
{
  register int answer;
#ifdef	XDEBUG
  debug("We have entered mask().\n");
#endif
  switch (lvl)
  {
    case LVLQ :
      answer = 0;
      break;
    case LVL0 :
      answer = MASK0;
      break;
    case LVL1 :
      answer = MASK1;
      break;
    case LVL2 :
      answer = MASK2;
      break;
    case LVL3 :
      answer = MASK3;
      break;
    case LVL4 :
      answer = MASK4;
      break;
    case LVL5 :
      answer = MASK5;
      break;
    case LVL6 :
      answer = MASK6;
      break;
    case LVL7 :
      answer = MASK7;
      break;
    case LVL8 :
      answer = MASK8;
      break;
    case LVL9 :
      answer = MASK9;
      break;
    case SINGLE_USER :
      answer = MASKSU;
      break;
    case LVLN :
      answer = MASKN;
      break;
    case LVLa :
      answer = MASKa;
      break;
    case LVLb :
      answer = MASKb;
      break;
    case LVLc :
      answer = MASKc;
      break;
    default :
      answer = FAILURE;
      break;
  }
  return (answer);
}

/*********************/
/****    level    ****/
/*********************/
char
level(int state)
{
  register char answer;

#ifdef	XDEBUG
  debug("We have entered level().\n");
#endif
  switch(state)
  {
    case LVL0 :
      answer = '0';
      break;
    case LVL1 :
      answer = '1';
      break;
    case LVL2 :
      answer = '2';
      break;
    case LVL3 :
      answer = '3';
      break;
    case LVL4 :
      answer = '4';
      break;
    case LVL5 :
      answer = '5';
      break;
    case LVL6 :
      answer = '6';
      break;
    case LVL7 :
      answer = '7';
      break;
    case LVL8 :
      answer = '8';
      break;
    case LVL9 :
      answer = '9';
      break;
    case SINGLE_USER :
      answer = 'S';
      break;
    case LVLN :
      answer = 'N';
      break;
    case LVLa :
      answer = 'a';
      break;
    case LVLb :
      answer = 'b';
      break;
    case LVLc :
      answer = 'c';
      break;
    default :
      answer = '?';
      break;
  }
  return(answer);
}

/************************/
/****    killproc    ****/
/************************/
/*	"killproc" sends the SIGTERM signal to the specified process	*/
/*	and then after TWARN seconds, the SIGKILL signal.		*/
void
killproc(register pid_t pid)
{
  extern int childeath(void), alarmclk(void);
  struct PROC_TABLE *efork();
  register struct PROC_TABLE *process;
  struct sigaction action;

#ifdef	XDEBUG
  debug("We have entered killproc().\n");
#endif
  action.sa_mask.losigs = 0;	/* for safety's sake */
  action.sa_mask.hisigs = 0;	/* for safety's sake */
  action.sa_flags = 0;
  action.sa_handler = SIG_DFL;
  sigaction(SIGCLD,&action,(struct sigaction *)NULL);
  while ((process = efork(NULLPROC,0)) == NO_ROOM) pause();
  action.sa_flags = SA_RESTART;
  action.sa_handler = (void (*)(int))childeath;
  sigaction(SIGCLD,&action,(struct sigaction *)NULL);
  action.sa_handler = (void (*)(int)) alarmclk;
  sigaction(SIGALRM,&action,(struct sigaction *)NULL);

  /* If we are the child, send the signals to the process we are to kill. */
  if (process == NULLPROC)
  {
    kill(pid,SIGTERM);	/* Warn the process to quit.    */
#ifdef  XDEBUG
    debug("SIGTERM sent to pid: %d\n", pid);
#endif
    timer(TWARN);	/* Sleep TWARN seconds */
    kill(pid,SIGKILL);	/* Kill the process if still alive. */
#ifdef  XDEBUG
    debug("SIGKILL sent to pid: %d\n", pid);
#endif
    exit(0);
  }
}

/**************************/
/****    initialize    ****/
/**************************/
/*	Perform the initial state setup and look for an initdefault	*/
/*	entry in the "inittab" file.					*/
int 
initialize()
{
  struct CMD_LINE cmd;
  char command[MAXCMDL];
  extern int cur_state,op_modes;
  extern int childeath(void);
  register int msk,i;
  static int states[] = { LVL0,LVL1,LVL2,LVL3,LVL4,LVL5,LVL6,LVL7,LVL8,LVL9,SINGLE_USER };
  FILE *fp_syscon;
  char device[sizeof("/dev/")+MAXNAMLEN];
  int initstate,flag;
  register struct PROC_TABLE *process,*oprocess;
  extern struct PROC_TABLE *efork(),*findpslot();
  struct sigaction action;

  /* Initialize state to "SINGLE_USER" "BOOT_MODES" */
  /* At conception we are cur_state = -1 for safety */
  if(cur_state >= 0)
  {
    n_prev[cur_state]++;
    prior_state = cur_state;
  }
  cur_state = SINGLE_USER;
  op_modes = BOOT_MODES;

  /* Set up all signals to be caught or ignored as is appropriate. */
  init_signals();
  initstate = 0;

#ifdef	UDEBUG
  save_ioctl();
#endif

  /* Get the ioctl settings for /dev/console so that it can be */
  /* brought up in the state it was in when the system went down. */
  get_ioctl_console();
  console(NOLOG, M_DEFINED,"Initialized. Console defined.\n");

  /* Look for an "initdefault" entry in "/etc/inittab", which */
  /* specifies the initial level to which "init" is to go at */
  /* startup time. */
  while((flag = getcmd(&cmd,&command[0])) == TRUE)
  {
    if (cmd.c_action == M_INITDEFAULT)
    {

      /* Look through the "c_levels" word, starting at the highest */
      /* level.  The assumption is that there will only be one level */
      /* specified, but if there is more than one, the system will */
      /* come up at the highest possible level. */
      for(msk=MASKSU,i=(sizeof(states)/sizeof(int)) - 1; msk > 0; msk >>= 1,i--)
      {
        if (msk & cmd.c_levels)
        {
          initstate = states[i];
        }
      }

      /* If the entry is for a system initialization command, execute */
      /* it at once, and wait for it to complete. */
    } 
/* FIRST PASS CODE
*  There will be an "elseif" installed here in the second pass that will
*  look for a "CONSOLE" entry in /etc/inittab. If there is one, the
*  system console is not the default one and a flag will be set.
*/
    else if (cmd.c_action == M_SYSINIT)
    {
      if (process = findpslot(&cmd))
      {
        action.sa_mask.losigs = 0;	/* for safety's sake */
        action.sa_mask.hisigs = 0;	/* for safety's sake */
        action.sa_flags = 0;
        action.sa_handler = SIG_DFL;
        sigaction(SIGCLD,&action,(struct sigaction *)NULL);
        for (oprocess=process; 
             (process = efork(oprocess,(NAMED|NOCLEANUP))) == NO_ROOM;);
	action.sa_flags = SA_RESTART;
        action.sa_handler = (void (*)(int))childeath;
        sigaction(SIGCLD,&action,(struct sigaction *)NULL);
        if (process == NULLPROC)
        {
	  FILE	*fp;

          /* Notice no bookkeeping is performed on these entries.  This is */
          /* to avoid doing anything that would cause writes to the file */
          /* system to take place.  No writing should be done until the */
          /* operator has had the chance to decide whether the file system */
          /* needs checking or not. */
    	  for (i = 0, fp = stdin; i < LCLOSE; i++, fp++)
            fclose(fp);
    	    execcmd(cmd.c_command);
          exit(1);
        } 
        else
	  while (waitproc(process) == FAILURE);
#ifdef	ACCTDEBUG
        debug("SYSINIT- id: %.6s term: %o exit: %o\n",
            &cmd.c_id[0],(process->p_exit&0xff),
            (process->p_exit&0xff00)>>8);
#endif
        process->p_flags = 0;
      }
    }
  }
  if (initstate) return(initstate);

/* FIRST PASS CODE
*  We assume in the first pass that the console is always /dev/console.
*  Future passes will include have the /dev/console driver remember
*  what the console is (inittab) and change to that console.
*    Essentially, nothing happens here UNLESS the console device is
*  supposed to be remote (not the trusted console), in which case we notify
*  the trusted console of the impending change.
*  BY THE WAY: I need to determine if /dev/console -> /dev/syscon, which
*  is a case that makes this code redundant.
*/
  /* If the system console is remote, put a message on the */
  /* system tty warning anyone there that console is elsewhere. */
/*
  action.sa_mask.losigs = 0;
  action.sa_mask.hisigs = 0;
  action.sa_flags = 0;
  action.sa_handler = SIG_DFL;
  sigaction(SIGCLD,&action,(struct sigaction *)NULL);
  while ((process = efork(NULLPROC,NOCLEANUP)) == NO_ROOM);
  action.sa_flags = SA_RESTART;
  action.sa_handler = (void *) childeath;
  sigaction(SIGCLD,&action,(struct sigaction *)NULL);
  if (process == NULLPROC)
  {
    if ((fp_syscon = fopen(SYSCON,"r+")) != (FILE*)NULL)
    {
      catd = catopen(MF_INIT,NL_CAT_LOCALE);
      fprintf(fp_syscon, catgets(catd, MS_INIT, M_REMOTE, 
		"\nInit: system console is remote:\
                %s Type <DEL> to regain control.\n"), &device[0]);
      fflush(fp_syscon);
      fclose(fp_syscon);
      catclose(catd);
    }

    exit(0);
  }

  while(waitproc(process) == FAILURE);
*/

  /* Since no "initdefault" entry was found, return 0.  This will */
  /* have "init" ask the user at /dev/console to supply a level. */
  return(flag);
}

/**********************/
/****    getlvl    ****/
/**********************/
/*	Get the new run level from /dev/console. */
int fd_syscon;

int
getlvl()
{
  char c;
  int status;
  int flag;
#ifdef	UDEBUG
  extern int abort();
#endif
  FILE *fp_tmp;
  pid_t process;
  pid_t childpid;
  extern int fd_syscon;
  extern int switchcon(int);
  static char levels[] = { LVL0,LVL1,LVL2,LVL3,LVL4,LVL5,LVL6,LVL7,LVL8,LVL9,SINGLE_USER };
  struct sigaction action;

  /* Fork a child who will request the new run level from */
  /* /dev/console. */
#ifdef XDEBUG
  debug("We are entering getlvl()\n");
#endif

  action.sa_mask.losigs = 0;	/* for safety's sake */
  action.sa_mask.hisigs = 0;	/* for safety's sake */
  action.sa_flags = 0;
  action.sa_handler = SIG_DFL;
  sigaction(SIGCLD,&action,(struct sigaction *)NULL);
  while ((process = fork()) == -1);
  if (process == 0)
  {
    endutent();				/* child needs seperate fd */
    action.sa_handler = SIG_IGN;
    sigaction(SIGHUP,&action,(struct sigaction *)NULL); /* ignore telinit q */

  /* Open /dev/syscon so that if someone types a <del>, we can be */
  /* informed of the fact. */
    if ((fp_tmp = fopen(SYSCON,"r+")) != NULL) 
    {

  /* Make sure the file descriptor is greater than 2 so that it */
  /* won't interfere with the standard descriptors. */
      fd_syscon = fcntl((int)fileno(fp_tmp),F_DUPFD,3);
      fdopen(fd_syscon, "r+");
      fclose(fp_tmp);

  /* Prepare to catch the interupt signal if <del> typed at */
  /* /dev/syscon. */
    action.sa_handler = (void (*)(int))switchcon;
    sigaction(SIGINT,&action,(struct sigaction *)NULL);
    sigaction(SIGQUIT,&action,(struct sigaction *)NULL);
    }
#ifdef	UDEBUG
    action.sa_handler = (void (*)(int))abort;
    sigaction(SIGUSR1,&action,(struct sigaction *)NULL);
    sigaction(SIGUSR2,&action,(struct sigaction *)NULL);
#endif
    for (;;)
    {

      /* Close the current descriptors and open ones to /dev/console. */
      openconsole();

      /* Print something unimportant and pause, since reboot may be */
      /* taking place over a line coming in over a modem or */
      /* something stranger... */
      fprintf(stdout,"\n");
      timer(2);

      flag = TRUE;
      while(flag)
      {
        /* Now read in the user response. */
        catd = catopen(MF_INIT,NL_CAT_LOCALE);
    	fprintf(stdout, catgets(catd, MS_INIT, M_LVLPROMPT, "Enter run level (0-9, s, S, m, or M): "));
        catclose(catd);
        fflush(stdout);

        /* Get a character from the user which isn't a space, tab or <cr>. */
        while((fscanf(stdin,"%c",&c) != 1)
                || (c == '\n') || (c == '\t') || (c == ' '));
        c &= 0x7f;

        /* If the character is a digit between 0 and 9 or the letter S, */
        /* fine, exit with the level equal to the new desired state. */
        if (c >= '0' && c <= '9')
        {
          catd = catopen(MF_INIT,NL_CAT_LOCALE);
    	  fprintf(stdout, catgets(catd, MS_INIT, M_CHANGE, 
		"will change to state %c\n"), c);
          catclose(catd);
          exit(levels[c - '0']);
        }
        else if (c == 'S' || c == 's' || c == 'm' || c == 'M')
        {
          catd = catopen(MF_INIT,NL_CAT_LOCALE);
    	  fprintf(stdout, catgets(catd, MS_INIT, M_CHANGE, 
		"will change to state %c\n"), c);
          catclose(catd);
          exit(levels[10]);
        }
        else 
        {
          catd = catopen(MF_INIT,NL_CAT_LOCALE);
    	  fprintf(stdout, catgets(catd, MS_INIT, M_BADCHAR, 
		"\nbad character <%3.3o>\n\n"), c);
          catclose(catd);
          while ((fscanf(stdin,"%c",&c) != 1) || (c != '\n'));
        }
      }
    }
  }

  /* Wait for the child to die and return it's status. */
  while ((childpid = waitpid(process, &status, 0)) != process);

  /* Return the new run level to the caller. */
#ifdef XDEBUG
  debug("getlvl: status: %o exit: %o termination: %o\n", status,(status&0xff00)>>8,(status&0xff));
#endif
  return((status&0xff00)>>8);
}

/*********************/
/****    efork    ****/
/*********************/
/*	"efork" forks a child and the parent inserts the process in	*/
/*	its table of processes that are directly a result of forks	*/
/*	that it has performed.  The child just changes the "global"	*/
/*	with the process id for this process to it's new value.		*/
/*									*/
/*	If "efork" is called with a pointer into the proc_table		*/
/*	it uses that slot, otherwise it searches for a free slot.	*/
/*	Whichever way it is called, it returns the pointer to the	*/
/*	proc_table entry.						*/
struct PROC_TABLE *
efork(register struct PROC_TABLE *process, int modes)
{
  register pid_t childpid;
  struct PROC_TABLE *procptr;
  extern pid_t own_pid;
  extern int errno;
  int i;
  extern int childeath(void);
  extern char *vetchar();
  struct sigaction action, oaction;
#ifdef	UDEBUG
  static int (*oldsigs[NPROC])();
#endif

#ifdef	XDEBUG
  debug("We have entered efork().\n");
#endif
  /* Freshen up the proc_table, removing any entries for dead */
  /* processes that don't have the NOCLEANUP set.  Perform the */
  /* necessary accounting. */
  for (procptr= &proc_table[0]; procptr < &proc_table[NPROC]; procptr++)
  {
    if ((procptr->p_flags & (OCCUPIED | LIVING | NOCLEANUP)) == (OCCUPIED))
    {
#ifdef	XDEBUG
      debug("efork- id:%s pid: %d time: %lo %d %o %o\n",
          vetchar(&procptr->p_id[0]),procptr->p_pid, procptr->p_time,
          procptr->p_count, procptr->p_flags,procptr->p_exit);
#endif

      /* Is this a named process?  If so, do the necessary bookkeeping. */
      if (procptr->p_flags & NAMED)
	account(DEAD_PROCESS,procptr,NULL);

      /* Free this entry for new usage. */
      procptr->p_flags = 0;
    }
  }

  while((childpid = fork()) == FAILURE)
  {
    /* Shorten the alarm timer in case someone else's child dies and */
    /* free up a slot in the process table. */
    setimer(5);

    /* Wait for some children to die.  Since efork() is normally */
    /* called with SIGCLD in the default state, reset it to catch */
    /* so that child death signals can come in. */

    action.sa_mask.losigs = 0;	/* for safety's sake */
    action.sa_mask.hisigs = 0;	/* for safety's sake */
    action.sa_flags = SA_RESTART;
    action.sa_handler = (void (*)(int))childeath;
    sigaction(SIGCLD,&action,(struct sigaction *)NULL);
    pause();
    action.sa_handler = SIG_DFL;
    sigaction(SIGCLD,&action,(struct sigaction *)NULL);
    setimer(0);
  }
  if (childpid != 0)
  {

#ifdef  XDEBUG
      debug("efork's kid pid: %d\n", childpid);
#endif
    /* If a pointer into the process table was not specified, then */
    /* search for one. */
    if (process == NULLPROC)
    {
      for (process= &proc_table[0]; process->p_flags != 0 &&
	   process < &proc_table[NPROC];process++ );
      if (process == &proc_table[NPROC])
      {
        if (error_time(FULLTABLE))
    	  console(NOLOG, M_PTAB,"Internal process table is full.\n");
        return(NO_ROOM);
      }
      process->p_time = 0L;
      process->p_count = 0;
    }
    for (i = 0; i < IDENT_LEN; i++)
    	process->p_id[i] = '\0';
    process->p_pid = childpid;
    process->p_flags = (LIVING | OCCUPIED | modes);
    process->p_exit = 0;
  }
  else 
  {
    own_pid = getpid();	/* Reset child's concept of its own process id.  */
    setsid();		/* start session and become process group leader */
    process = NULLPROC;

    endutent();				/* child need seperate fd */
    /* Reset all signals to the system defaults. */
    action.sa_handler = SIG_DFL;
    action.sa_mask.losigs = 0;	/* for safety's sake */
    action.sa_mask.hisigs = 0;	/* for safety's sake */
    action.sa_flags = 0;
    for (i=SIGHUP; i <= SIGMAX;i++) {
      sigaction(i,&action,&oaction);
#ifdef	UDEBUG
      oldsigs[i] = oaction.sa_handler;
#endif
    }
    /* job control shells reset these to SIG_DFL before exec'ing commands */
    action.sa_handler = SIG_IGN;
    sigaction(SIGTSTP,&action,(struct sigaction *)NULL);
    sigaction(SIGTTIN,&action,(struct sigaction *)NULL);
    sigaction(SIGTTOU,&action,(struct sigaction *)NULL);
  }
  return(process);
}

/************************/
/****    waitproc    ****/
/************************/
/*	"waitproc" waits for a specified process to die.  For this	*/
/*	routine to work, the specified process must already in		*/
/*	the proc_table.  "waitproc" returns the exit status of the	*/
/*	specified process when it dies.					*/
long
waitproc(register struct PROC_TABLE *process)
{
  int answer;
  struct sigaction action, oaction;
  int pid, status;

#ifdef	XDEBUG
  debug("We have entered waitproc().\n");
#endif

  /* Wait around until the process dies. */
  if (process->p_flags & LIVING) {
	pid = waitpid(process->p_pid, &status, 0);
	if ((pid != -1) || (errno != ECHILD)) {
		if ( pid != process->p_pid )
			return (FAILURE);
		/* Same as in childeath() */
		process->p_flags &= ~LIVING;
		process->p_exit = status;
		wakeup_flags |= W_CHILDEATH;
	}
  }
  childeath();

  /* Make sure to only return 16 bits so that answer will always */
  /* be positive whenever the process of interest really died. */
  answer = (process->p_exit & 0xffff);

  /* Free the slot in the proc_table. */
  process->p_flags = 0;
  return(answer);
}

/***********************/
/****    account    ****/
/***********************/
/*	"account" updates entries in /etc/utmp and appends new entries	*/
/*	to the end of /etc/wtmp (assuming /etc/wtmp exists).		*/
struct utmp *
account(int state, register struct PROC_TABLE *process, char *program)
/* char *program: Program Name in the case of INIT_PROCESSes, else NULL */
{
  extern cur_state;
  static struct utmp utmpbuf;
  register struct utmp *uptr,*oldu;
  extern struct utmp *getutid(),*pututline();
  char level();

#ifdef	ACCTDEBUG
  extern char *vetchar();
  debug("** account ** state: %d id:%s\n",state,vetchar(&process->p_id[0]));
#endif

#ifdef	XDEBUG
  debug("We have entered account().\n");
#endif
  /* Set up the prototype for the utmp structure we want to write. */
  uptr = &utmpbuf;
  bzero(&utmpbuf, sizeof(utmpbuf));

  /* Fill in the various fields of the utmp structure. */
  strncpy(uptr->ut_id, process->p_id, IDENT_LEN);
  uptr->ut_pid = process->p_pid;

  /* Fill the "ut_exit" structure. */
  uptr->ut_exit.e_termination = (process->p_exit & 0xff);
  uptr->ut_exit.e_exit = ((process->p_exit >> 8) & 0xff);
  uptr->ut_type = state;
  time(&uptr->ut_time);

  /* See if there already is such an entry in the "utmp" file. */
  setutent();	/* Start at beginning of utmp file. */
  if ((oldu = getutid(uptr)) != NULL)
  {

    /* Copy in the old "user" and "line" fields to our new structure. */
    memcpy(&uptr->ut_user[0],&oldu->ut_user[0],sizeof(uptr->ut_user));
    memcpy(&uptr->ut_line[0],&oldu->ut_line[0],sizeof(uptr->ut_line));
#ifdef	ACCTDEBUG
    debug("New entry in utmp file.\n");
#endif
  }
#ifdef	ACCTDEBUG
  else debug("Replacing old entry in utmp file.\n");
#endif

  /* Preform special accounting. Insert the special string into the */
  /* ut_line array. For INIT_PROCESSes put in the name of the */
  /* program in the "ut_user" field. */
  switch(state)
  {
    case RUN_LVL :
      uptr->ut_exit.e_termination = level(cur_state);
      uptr->ut_exit.e_exit = level(prior_state);
      uptr->ut_pid = n_prev[cur_state];
      sprintf(uptr->ut_line,RUNLVL_MSG,level(cur_state));
      break;
    case BOOT_TIME :
      strncpy(uptr->ut_line, BOOT_MSG, 12);
      break;
    case INIT_PROCESS :
      strncpy(uptr->ut_user, program, sizeof(uptr->ut_user));
      break;
    case DEAD_PROCESS :
      bzero(uptr->ut_user, sizeof(uptr->ut_user));
      break;
    default :
      break;
  }

  /* Write out the updated entry to utmp file. */
  if (pututline(uptr) == (struct utmp *)NULL)
    console(LOG, M_UTMP,"failed write of utmp entry: \"%14s\"\n", uptr->ut_id);

  (void) append_wtmp(WTMP, uptr);
  return uptr;
}

/**************************/
/****    error_time    ****/
/**************************/
/*	"error_time" keeps a table of times, one for each time of	*/
/*	error that it handles.  If the current entry is 0 or the	*/
/*	elapsed time since the last error message is large enough,	*/
/*	"error_time" returns TRUE, otherwise it returns FALSE.		*/
error_time(register int type)
{
  time_t curtime;
  extern struct ERRORTIMES err_times[];

#ifdef	XDEBUG
  debug("We have entered error_time().\n");
#endif
  time(&curtime);
  if (err_times[type].e_time == 0
      || (curtime - err_times[type].e_time >= err_times[type].e_max))
  {
    err_times[type].e_time = curtime;
    return(TRUE);
  } 
  else return(FALSE);
}


/************************/
/****    userinit    ****/
/************************/
/*	Routine to handle requests from users to main init running as	*/
/*	process 1.							*/
void
userinit(int argc, char **argv)
{
  FILE *fp;
  char *ln;
  extern char *ttyname();
  int init_signal;
  extern int errno;
  extern char *CONSOLE;
  extern pid_t own_pid;

  /* We are a user invoked init.  Is there an argument and is it */
  /* a single character?  If not, print usage message and quit. */
#ifdef	XDEBUG
  debug("We have entered userinit().\n");
#endif
  if (argc != 2 || *(*++argv +1) != '\0')
  {
    catd = catopen(MF_INIT,NL_CAT_LOCALE);
    fprintf(stderr, catgets(catd, MS_INIT, M_USAGE, 
	"Usage: init [0123456789SsMmQqabcN]\n"));
    catclose(catd);
    exit(1);
  } 
  else switch (**argv)
  {
    case 'Q' :
    case 'q' :
      init_signal = LVLQ;
      break;
    case '0' :
      init_signal = LVL0;
      break;
    case '1' :
      init_signal = LVL1;
      break;
    case '2' :
      init_signal = LVL2;
      break;
    case '3' :
      init_signal = LVL3;
      break;
    case '4' :
      init_signal = LVL4;
      break;
    case '5' :
      init_signal = LVL5;
      break;
    case '6' :
      init_signal = LVL6;
      break;
    case '7' :
      init_signal = LVL7;
      break;
    case '8' :
      init_signal = LVL8;
      break;
    case '9' :
      init_signal = LVL9;
      break;

    /* If the request is to switch to single user mode, make sure */
    /* that this process is talking to a legal teletype (yes, gentle- */
    /* persons, this code is THAT old) line and that /dev/console is */
    /* linked to this line. */

    case 'S' :
    case 's' :
    case 'M' :
    case 'm' :
      ln = ttyname(0);	/* Get the name of tty */
      if (*ln == '\0')
      {
        catd = catopen(MF_INIT,NL_CAT_LOCALE);
	fprintf(stderr, catgets(catd, MS_INIT, M_NOTTY, 
		"Standard input not a tty line\n"));
        catclose(catd);
        exit(1);
      }
      if (strcmp(ln,CONSOLE) != 0)
      {

      /* Leave a message if possible on system console saying where */
      /* /dev/console is currently connected. */
/* FIRST PASS CODE
*  This pass sends all i/o to /dev/console, without moving the console
*  to the calling device. In the second pass, it will issue the
*  proper command moving the console to the caller's device.
*/

        catd = catopen(MF_INIT,NL_CAT_LOCALE);
	fprintf(stderr, catgets(catd, MS_INIT, M_MOVCNTL, 
		"Moving control to CONSOLE device.\n"));
        catclose(catd);
        if ((fp = fopen(CONSOLE,"r+")) != NULL)
        {
          catd = catopen(MF_INIT,NL_CAT_LOCALE);
	  fprintf(fp, catgets(catd, MS_INIT, M_SUENTER, 
		"\n**** SINGLE-USER MODE ENTERED FROM %s ****\n"), ln);
          catclose(catd);
          fclose(fp);
        }
      }
      init_signal = SINGLE_USER;
      break;

    case 'N' :
      init_signal = LVLN;
      break;
    case 'a' :
      init_signal = LVLa;
      break;
    case 'b' :
      init_signal = LVLb;
      break;
    case 'c' :
      init_signal = LVLc;
      break;

      /* If the argument was invalid, print the usage message. */
    default :
      catd = catopen(MF_INIT,NL_CAT_LOCALE);
      fprintf(stderr, catgets(catd, MS_INIT, M_USAGE, 
	"Usage: init [0123456789SsMmQqabcN]\n"));
      catclose(catd);
      exit(1);
  }

  /* Now send signal to main init and then exit. */
  if (kill(SPECIALPID,init_signal) == FAILURE)
  {
    catd = catopen(MF_INIT,NL_CAT_LOCALE);
    fprintf(stderr, catgets(catd, MS_INIT, M_NOSIG, 
	"Could not send signal to \"init\".\n"));
    catclose(catd);
    exit(1);
  } 
  else 
    exit(0);
}
