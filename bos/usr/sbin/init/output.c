static char sccsid[] = "@(#)15	1.29  src/bos/usr/sbin/init/output.c, cmdoper, bos411, 9428A410j 4/27/94 16:54:32";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include	<sys/limits.h>
#include	<sys/types.h>
#include	<utmp.h>
#include	<signal.h>
#include	<stdio.h>
#include	<termio.h>
#include	<ctype.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<userpw.h>
#include	<sys/inode.h>
#include	<sys/user.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<sys/NLchar.h>
#include	<sys/errids.h>

#include <nl_types.h>
#include	"init_msg.h"
#include	"init.h"

#define MSGSTR(Num,Str) catgets(catd,MS_INIT,Num,Str)

extern long waitproc();
struct termios	curterm;	/* current terminal state */

/*************************/
/****    switchcon    ****/
/*************************/

switchcon(int sig)
{
  extern int own_pid;
  extern int fd_syscon;
  extern int switchcon(int);
  struct sigaction action;

#ifdef	XDEBUG
  debug("We have entered switchcon().\n");
#endif

  action.sa_mask.losigs = 0;	/* for safety's sake */
  action.sa_mask.hisigs = 0;	/* for safety's sake */
  action.sa_flags = 0;		/* no flags */
  action.sa_handler = SIG_IGN;

  sigaction(sig,&action,(struct sigaction *)NULL);

  /* If this is the first time a <del> has been typed on the */
  /* /dev/syscon, then issue the console change command to */
  /* /dev/syscon.  Also reestablish file pointers. */
  if (fd_syscon != -1) 
  {
    reset_console();
    openconsole();

  /* Set fd_syscon to -1 so that we ignore any deletes from it in */
  /* the future as far as changing /dev/console to /dev/syscon. */
    fd_syscon = -1;
  }

  action.sa_handler = (void (*)(int)) switchcon;

  sigaction(sig,&action,(struct sigaction *)NULL);
}


/**************************/
/****    openconsole    ****/
/**************************/

/*	"openconsole" opens stdin, stdout, and stderr, making sure	*/
/*	that their file descriptors are 0, 1, and 2, respectively.	*/

openconsole()
{
  FILE *fp;
  extern struct termios termios;
  extern FILE *fdup();

  fclose(stdin);
  fclose(stdout);
  fclose(stderr);
  close(0);
  close(1);
  close(2);

  fp = fopen(CONSOLE,"r+");
  fdup(fp);
  fdup(fp);
  setbuf(fp,(char *)NULL);
  setbuf(stdout,(char *)NULL);
  setbuf(stderr,(char *)NULL);

  /* Make sure the hangup on last close is off.  Then restore */
  /* the modes that were on console when the signal was sent. */
  tcgetattr(fileno(fp), &curterm);
  termios.c_cflag = curterm.c_cflag;
  termios.c_cflag &= ~HUPCL;
  tcsetattr(fileno(fp), TCSANOW, &termios);
  return;
}


/********************************/
/****    get_ioctl_console    ****/
/********************************/

/*	"get_ioctl_console" retrieves the /dev/console settings from	*/
/*	the file "/etc/.ioctl.console".					*/

get_ioctl_console()
{
  register FILE *fp;
  int iflags,oflags,cflags,lflags,ldisc,cc[NCCS],i;
  extern struct termios dflt_termios,termios;
  int got_it;

  /* Read in the previous modes for /dev/console from .ioctl.console. */
  got_it = FALSE;

  if ( (fp = fopen(IOCTLCONSOLE,"r")) != NULL )
  {

    i = fscanf(fp,"%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x",
        &iflags,&oflags,&cflags,&lflags,
        &cc[0],&cc[1],&cc[2],&cc[3],&cc[4],&cc[5],&cc[6],&cc[7],
        &cc[8],&cc[9],&cc[10],&cc[11],&cc[12],&cc[13],&cc[14],&cc[15]);

  /* If the file is formatted properly, use the values to initialize */
  /* the console terminal condition. */
    if (i == 4+NCCS)
    {
      termios.c_iflag = (ushort) iflags;
      termios.c_oflag = (ushort) oflags;
      termios.c_cflag = (ushort) cflags;
      termios.c_lflag = (ushort) lflags;
      for(i=0; i<NCCS; i++) 
        termios.c_cc[i] = cc[i];

      got_it = TRUE;

    } 
    fclose(fp);
  }

    /* If the file was nonexistent, or badly formatted, */
    /* use the default settings. */
  if ( got_it == FALSE )
  {
    /* get the default internal states and put THEM in the file */
    /*save_ioctl();*/
/**************** These are the "defaults" defined in init.h *****/
    memcpy(&termios,&dflt_termios,sizeof(dflt_termios));
  }
}

/****************************/
/****    reset_console    ****/
/****************************/

/*	"reset_console" changes /dev/console to /dev/syscon and puts	*/
/*	the default ioctl setting back into /etc/ioctl.syscon and	*/
/*	the incore arrays.						*/

reset_console()
{

/* FIRST PASS CODE
* This routine will do nothing until we have a /dev/console and
* an associated command that allows changes to where /dev/console
* points. That command will be issued here, in place of this stuff.
******* OLD SYS V CODE FOLLOWS *****
	
#ifdef	XDEBUG
  debug("We have entered reset_console().\n");
#endif
  YET_TO_BE_CODED_CHANGECONS_CALL("/dev/syscon");
  save_ioctl();
  sync();
  umask(0);

********* END OF OLD SYS V CODE **********/
}



/**************************/
/****    save_ioctl    ****/
/**************************/

/*	Get the ioctl state of CONSOLE and write it into IOCTLCONSOLE.	*/

save_ioctl()
{
  register FILE *fp;
  register struct PROC_TABLE *process;
  extern struct termios termios,dflt_termios;
  extern struct PROC_TABLE *efork();
  extern int errno, childeath(void);
  struct sigaction action;

#ifdef	XDEBUG
  debug("We have entered save_ioctl().\n");
#endif
  action.sa_mask.losigs = 0;	/* for safety's sake */
  action.sa_mask.hisigs = 0;	/* for safety's sake */
  action.sa_flags = 0;

  action.sa_handler = SIG_DFL;
  sigaction(SIGCLD,&action,(struct sigaction *)NULL);
  while ((process = efork(NULLPROC,NOCLEANUP)) == NO_ROOM) timer(2);
  action.sa_flags = SA_RESTART;
  action.sa_handler = (void (*)(int)) childeath;

  sigaction(SIGCLD,&action,(struct sigaction *)NULL);

  /* If we are the child, open /dev/console, do an ioctl to get the */
  /* modes, and write them out to "/etc/.ioctl.console". */
  if (process == NULLPROC)
  {
    fp = fopen(CONSOLE,"w");

    /* Turn off the HUPCL bit, so that carrier stays up after the */
    /* shell is killed. */

    if (tcgetattr(fileno(fp),&termios) != FAILURE)
    {
      termios.c_cflag &= ~HUPCL;
      tcsetattr(fileno(fp),TCSANOW, &termios);
    }
    fclose(fp);

    /* Write the state of "/dev/console" into "/etc/.ioctl.console" so */
    /* that it will be remembered across reboots. */
    umask((mode_t)(~0644));
    if ((fp = fopen(IOCTLCONSOLE,"w")) == NULL)
      console(NOLOG, M_OPEN,"Can't open %s. errno: %d\n", IOCTLCONSOLE, errno);
    else 
    {
      fprintf(fp,"%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
          termios.c_iflag,termios.c_oflag,termios.c_cflag, termios.c_lflag, 
          termios.c_cc[0],termios.c_cc[1],termios.c_cc[2],
          termios.c_cc[3],termios.c_cc[4],termios.c_cc[5],
          termios.c_cc[6],termios.c_cc[7],termios.c_cc[8],
          termios.c_cc[9],termios.c_cc[10],termios.c_cc[11],
          termios.c_cc[12],termios.c_cc[13],termios.c_cc[14],
          termios.c_cc[15]);
      fclose(fp);
      sync();
    }
    exit(0);
  }

  /* If we are the parent, wait for the child to die. */
  else while (waitproc(process) == FAILURE);
}

/***********************/
/****    console    ****/
/***********************/

/*	"console" forks a child if it finds that it is the main "init"	*/
/*	and outputs the requested message to the system console.	*/
/*	Note that the number of arguments passed to "console" is	*/
/*	determined by the print format.					*/

console(logerr, msg_id,format,arg1,arg2,arg3,arg4)
int logerr;
int msg_id;
char *format;
int arg1,arg2,arg3,arg4;
{
  nl_catd catd;
  char outbuf[BUFSIZ], *cp;
  ERR_REC(BUFSIZ) console_err;
  int fd;

#ifdef	XDEBUG
  debug("We have entered console().\n");
#endif
  catd = catopen(MF_INIT,NL_CAT_LOCALE);
  /* Output the message to the console. */
  strcpy(outbuf, "\nINIT: ");
  cp = outbuf + 7;
  sprintf(cp, MSGSTR(msg_id, format), arg1, arg2, arg3, arg4);
  catclose(catd);

  fd = open(CONSOLE, O_NDELAY|O_NOCTTY|O_WRONLY);
  if (fd >= 0) {
    int rc, sc = strlen(outbuf);
    cp = outbuf;
    while (sc > 0) {
	if (0 > (rc = write(fd, cp, sc)))
	    break;
	cp += rc;
	sc -= rc;
    }
    close(fd);
  }

  if (logerr) {
      char *ncp;
      bzero(&console_err, sizeof(console_err));
      strcpy(console_err.resource_name, "init");
      cp = outbuf + 7;
      if (ncp = index(cp, '\n'))
	  *ncp++ = '\0';
      strncpy(console_err.detail_data, cp, 80);
      switch (msg_id) {
      case M_RAPID: /* this err entry has a 2nd line of data */
	  console_err.error_id = ERRID_INIT_RAPID;
	  cp = ncp;
	  if (ncp = index(cp, '\n'))
	      *ncp++ = '\0';
	  strncpy(console_err.detail_data+80, cp, 80);
	  break;
      case M_OPEN:
	  console_err.error_id = ERRID_INIT_OPEN;
	  break;
      case M_UTMP:
	  console_err.error_id = ERRID_INIT_UTMP;
	  break;
      case M_CREATE:
	  console_err.error_id = ERRID_INIT_CREATE;
	  break;
      default:
	  console_err.error_id = ERRID_INIT_UNKNOWN;
	  break;
      }
      errlog(&console_err, sizeof(console_err));
  }

#ifdef	ACCTDEBUG
  debug(format,arg1,arg2,arg3,arg4);
#endif
}

/*****************************/
/******    enterpwd   *****/
/*****************************/

char *enterpwd(prompt)
char *prompt;
{
  struct termios ttyb;
  char     savel;
  unsigned short flags;
  register char *p;
  register int c;
  FILE	*fp;
  static char pbuf[MAX_PASS+1];

#ifdef	XDEBUG
  debug("We have entered enterpwd().\n");
#endif
  openconsole();

  fp = fopen(CONSOLE,"r+");
  tcgetattr(fileno(fp), &ttyb);
  flags = ttyb.c_lflag;
  ttyb.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
  tcsetattr(fileno(fp), TCSAFLUSH, &ttyb);
  (void) fputs(prompt, stderr);

  for(p=pbuf; (c = getc(fp)) != '\n' && c != EOF; ) 
  {
    if(p < &pbuf[MAX_PASS])
    *p++ = c;
  }
  *p = '\0';
  (void) putc('\n', stderr);
  ttyb.c_lflag = flags;
  tcsetattr(fileno(fp), TCSADRAIN, &ttyb);
  return(pbuf);
}


#ifdef DEBUGGER

/*********************/
/****    debug    ****/
/*********************/

debug(format,arg1,arg2,arg3,arg4,arg5,arg6)
char *format;
int arg1,arg2,arg3,arg4,arg5,arg6;
{
  FILE *fp;
  int errnum;
  extern int errno;

  if ((fp = fopen(DBG_FILE,"a+")) == NULL)
  {
    errnum = errno;
    console(NOLOG, M_OPEN, "Can't open \"%s\".  errno: %d\n",DBG_FILE,errnum);
    return;
  }
  fprintf(fp,format,arg1,arg2,arg3,arg4,arg5,arg6);
  fclose(fp);
}

/*****************/
/**** vetchar ****/
/*****************/
/* This little routine takes a char pointer input and makes */
/* sure that each character is printable (non-control). */

char *vetchar(id)
register char *id;

{
  static char answer[12];
  register char *ptr;
  register int i;

#ifdef	XDEBUG
  debug("We have entered vetchar().\n");
#endif
  for (i=6,ptr = &answer[0]; --i >= 0;id++)
  {
    if ( isprint(*id) == 0 )
    {
      *ptr++ = '^';
      *ptr++ = *id + 0100;
    } 
    else *ptr++ = *id;
  }
  *ptr++ = '\0';
  return(&answer[0]);
}
#endif /* DEBUGGER */


/********************/
/****    fdup    ****/
/********************/

FILE *fdup(fp)
register FILE *fp;

{
  register int newfd;
  register char *mode;

#ifdef	XDEBUG
  debug("We have entered fdup().\n");
#endif
  /* Dup the file descriptor for the specified stream and then */
  /* convert it to a stream pointer with the modes of the original */
  /* stream pointer. */
  if ((newfd = dup(fileno(fp))) != FAILURE)
  {

    /* Determine the proper mode.  If the old file was _IORW, then */
    /* use the "r+" option, if _IOREAD, the "r" option, or if _IOWRT */
    /* the "w" option.  Note that since none of these force an lseek */
    /* by "fdopen", the dupped file pointer will be at the same spot */
    /* as the original. */
    if (fp->_flag & _IORW) mode = "r+";
    else if (fp->_flag & _IOREAD) mode = "r";
    else if (fp->_flag & _IOWRT) mode = "w";

    /* Something is wrong, close dupped descriptor and return NULL. */
    else 
    {
      close(newfd);
      return(NULL);
    }

    /* Now have fdopen finish the job of establishing a new file */
    /* pointer. */
    return(fdopen(newfd,mode));
  } 
  else return(NULL);
}

