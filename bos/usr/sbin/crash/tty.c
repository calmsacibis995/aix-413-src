static char sccsid[] = "@(#)37	1.7  src/bos/usr/sbin/crash/tty.c, cmdcrash, bos411, 9428A410j 5/1/91 14:31:54";
/*
 * COMPONENT_NAME: (CMDCRASH) Crash
 *
 * FUNCTIONS: call_tty
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include	"crash.h"

extern int errno;
extern int dumpflag;

/*
 * Name: 	call_tty
 * Function:	fork and exec the tty display formatting routines
 *		for a running system
 */

call_tty(uargs)
char *uargs;
{
	register int i = 0;     /* current arg number */
	char *args[6];		/* args for exec */
	char  fmt_rtn[MAXPATHLEN]; /* full pathname of routine to exec */	
	char  fdbuf[5];		/* file desc in ascii */
	int   fpid;	
	int   status;

#ifdef Debug
	printf("call_fmt: index %d, mem:0x%.8, tell:0x%.8x\n",index,mem,
		lseek(mem,0,1));
#endif
	/* construct tty formatting routine full pathname */
	strcpy(fmt_rtn,DMPRTNS_DIR);
	strcat(fmt_rtn,"/tty");

	/*
	 * args are set up as follows:
	 * tty <!0=running system,0=dump> <-f filedesc> <users args>
	 */
	args[i++] = fmt_rtn;		/* pathname */
	if (!dumpflag)
		args[i++] = "-r";
	args[i++] = "-f";		/* file desc flag */
	sprintf(fdbuf,"%d",mem);
	args[i++] = fdbuf;		/* file desc value */
	if (uargs)
		args[i++] = uargs;	/* first user arg */
	args[i] = '\0';			/* null terminate */

	errno = 0;
#ifdef Debug
	printf("about to fork %s:0:%s,1:%d,2:%s,3:%s,4:0x%8x,5:0x%8x,6:0x%8x\n",
	       fmt_rtn,args[0],args[1],args[2],args[3],args[4],args[5],args[6]);
#endif
	if((fpid=fork())==0) {		/* child */
		if(execve(fmt_rtn,args,NULL)) {
			printf( catgets(scmc_catd, MS_crsh, TTY_MSG_3, "0452-376: Cannot execute %s, errno = %d.\n") ,fmt_rtn,errno);
			return ;
		}
	}
	else if(fpid == -1) {		/* error */
		printf( catgets(scmc_catd, MS_crsh, TTY_MSG_4, "0452-377: Cannot fork %s, errno = %d.\n") ,fmt_rtn,errno);
		return ;
	}

	/* parent here */
	while (wait(&status) != fpid);
}

