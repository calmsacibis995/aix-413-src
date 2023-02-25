static char sccsid[] = "@(#)71  1.25  src/bos/usr/sbin/trace/child.c, cmdtrace, bos412, 9445C412a 11/3/94 02:28:06";
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: child, childexit
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */

/*
 * trace daemon that reads from /dev/systrace and writes to the logfile
 * /usr/adm/ras/trcfile
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/trcctl.h>
#include <sys/trchkid.h>
#include "trace.h"

extern char *calloc(),*malloc();
extern call_exec();

static childsignal();

extern verboseflg;
int Sigusr2;

/*
 * Reset the Progname to "Child" to distinguish from the parent "trace"
 * Allocate a buffer to read into from /dev/systrace.
 * The first thing the daemon writes to the log is a header. Note that
 *   it is in the form of a trcgen generic trace hook, and has its own
 *   template in the /etc/trcfmt file to format the header.
 * Do a ps -e and for each process, hook the process id and the process
 *   name.
 * Make sure that the logfile will not exceed its ulimit.
 * If output is directed to stdout, do not wrap. This is simpler, but
 *   it generated a lot of data quick if the -k and -j options are not
 *   used to select specific trace hooks.
 * Otherwise, call logio_wrap. This routine will manage logfile wraparound
 *   and unwrap it after trcstop.
 */
child()
{

	Progname = "Child";
	signal(SIGINT, SIG_IGN);		/* ignore INT. It comm's to the parent */
	signal(SIGQUIT,(void(*)(int)) childsignal);	/* terminate, immediately */
	signal(SIGTERM,(void(*)(int)) childsignal);	/* terminate, immediately */
	signal(SIGUSR1,(void(*)(int)) childsignal);	/* terminate by forcing early EOF */
	signal(SIGUSR2,(void(*)(int)) childsignal);     /* terminate and unload
driver */
	signal(SIGHUP, (void(*)(int)) childsignal);	/* terminate, immediately */
	Debug("child\n");
	if(!delayflag) {
		if(ioctl(Systrctlfd,TRCON,0) < 0) {
			perror("trace demon: TRCON");
			childexit(1);
		}
		kill(getppid(),SIGUSR1);	/* signal parent that trace is on */
	}
	if(ioctl(Systracefd,TRCIOC_LOGIO,0)) {
		perror("TRCIOC_LOGIO");
		childexit(1);
	}
	childexit(0);
}

static childsignal(signo)
{
	int rv;

	Debug("childsignal(%d)\n",signo);
	rv = 1;
	switch(signo) {
	case SIGUSR1:
		Signo = SIGUSR1;
		break;
	case SIGUSR2:
                Sigusr2++;
                break;
	case SIGQUIT:
	case SIGTERM:
	case SIGHUP:
	default:
		childexit(rv);
	}
	signal(signo,(void(*)(int)) childsignal);
}

childexit(exitcode)
{

	Debug("childexit(%x)\n",exitcode);
	close(Systrctlfd);
	close(Systracefd);
	if ((asyncflag) || (Sigusr2))
                call_exec(exitcode,0);
        else
		_exit(exitcode);
}

