static char sccsid[] = "@(#) 77 1.2 src/bos/usr/sbin/assist/install_assist.c, cmdassist, bos411, 9428A410j 94/04/05 14:20:30";
/*
 * COMPONENT_NAME: cmdassist 
 *
 * FUNCTIONS: install_assist
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:	install_assist.c
 *
 * FUNCTION:  Utility to start the install assistant from an /etc/inittab line.
 *	      It ensures that the install assistant has a controlling terminal
 * 	      by becoming session leader. It can also be executed from the 
 * 	      command line. 
 *
 * EXECUTION ENVIRONMENT:
 *
 *	/etc/inittab
 *	Command line environment.
 *
 *	Syntax:	init_assist 
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/signal.h>
#include <sys/lft_ioctl.h>
#include <sys/termio.h>
#include <fcntl.h>
#include <sys/console.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/wait.h>

main(int argc,char *argv[])
/*
 *           If cannot open /dev/tty then we have no controlling terminal
 *	     so this should be  being invoked from inittab, so assume that
 *           console is the terminal and force it to be a session leader.
 *	        
 */
{
  int		fd;
  uint		rc;
  char		*contty = "/dev/tty";
  char		*portname = "/dev/console";
  char		*assist = "/usr/lib/assist/assist_main";
  struct  stat    stat_buf;
  struct  qry_devsw qdevsw;
  struct  cons_config config;
  struct  cfg_dd cfgdata;        /* Config DD   */
  static char cpath[256];

	if ((fd = open (contty, O_RDWR)) < 0)
	{
	/* If open fails, we have no controlling terminal, so assume 
	 * we are running from inittab and that the i/o device is 
	 * /dev/console. Find its redirected target, and force this
	 * process to be the session leader, so that we have a
	 * controlling terminal.
	 */
	 /* If this is a character device, and is directed to the console 
 	  * driver, then request the name of the final console device from
	  * the console redirecting device driver.
	  */
		if (( stat(portname, &stat_buf) == 0) &&
			S_ISCHR(stat_buf.st_mode))
		{
			qdevsw.devno = stat_buf.st_rdev;
			sysconfig (SYS_QDVSW, &qdevsw, sizeof(qdevsw));
			if (qdevsw.status & DSW_CONSOLE)
			{
				cfgdata.kmid = 0;
				cfgdata.devno = stat_buf.st_rdev;
				cfgdata.cmd = CONSOLE_CFG;
				cfgdata.ddsptr = (char *) (&config);
				cfgdata.ddslen = sizeof(config);
				config.cmd = CONSGETDFLT;
				config.path = cpath;
				if(sysconfig( SYS_CFGDD, &cfgdata,
				   sizeof(struct cfg_dd)) ==0 )
					portname = cpath;
#ifdef _DEBUG
		fprintf(stderr,"Terminal is %s\n",portname);
#endif
       		        /*
               		 * Start a new session ID.  Re-open the
                 	 * login port and then close it.  This
                 	 * -should- make the current process the
                 	 * session leader and the login terminal
                 	 * the controlling terminal for =this=
                 	 * process.
			 		 */
				close (0);
				close (1);
				close (2);
                		setsid();
			/* Open the underlying device for standard in and
			 * dup standard out and error to it.
			 */
                		fd = open (portname , O_RDWR);
                		dup (0);
				dup (0);
			/*
               		 * Somtimes the default process group id
                 	 * is not being set correctly
			 * (has  to do with console always being
	                 * open).  So we will always set it
	                 */
				tcsetpgrp (0, getpid ());
			}
		}	
	}
	else
	{
#ifdef _DEBUG
		fprintf(stderr,"Controlling Terminal already set\n");
#endif
		close (fd);
	}
	/* Invoke the install assistant shell script */
	rc = system (assist);
	rc = WEXITSTATUS(rc);
#ifdef _DEBUG
		fprintf(stderr,"Exit code is %x\n",rc);
#endif
	exit(rc);
}
