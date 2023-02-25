static char sccsid[] = "@(#)82  1.16  src/bos/usr/sbin/trace/data.c, cmdtrace, bos41B, 412_41B_sync 11/29/94 10:19:48";
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: global data for trace
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
 * LEVEL 1, 5 Years Bull Confidential Information
*/


#include <stdio.h>
#include "trace.h"

/*
 * file descriptors 
 */
char *Progname;
char *Message = "";
FILE *Infp  = stdin;	/* subcommand input */
FILE *Outfp = stderr;	/* subcommand output */
int   Systracefd;		/* /dev/systrace */
int   Systrctlfd;		/* /dev/systrctl */
int   Logfd;			/* logfile */
char  Logfile[128];
int   Logsize;
int   Tracebufsize;		/* size in bytes */
int   Channel;			/* generic trace channel number */

int Signo;
int Ublimit;
int Childpid;			/* pid of tracebuf-reading process */

/*
 * conditional event bitmap
 */
unsigned char Condhookids[NHOOKIDS/8];

/*
 * internal mode flags
 */
int Tracemode;		/* circular, alternate, single, bus */
int stdoutflag;		/* logfile output to stdout */
int stopflag;		/* stop trace on logfile full (no wraparound) */
int asyncflag;		/* configure and start trace and exit. */
int delayflag;		/* configure, but don't start trace */
int nohdrflag;		/* no header at start of logfile */
int genericflag;	/* allocate the first generic channel */
int saveflag;           /* rename the logfile to logfile.old */
int nmflag;		/* print nm for all loader entries */
int sid;

/*
 * The subcommand line scanning routine will fill
 *  in up to NARG subcommand line arguments, including subcommand name.
 * The name of the command is Arg[0], the arguments are Arg[1] - Arg[9]
 * Narg == 1 if there are no arguments (like main(argc,argv))
 */
int Narg = NARG;
char *Arg[NARG];

