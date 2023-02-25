/* @(#)55     1.20  src/bos/usr/sbin/trace/trace.h, cmdtrace, bos41B, 412_41B_sync 11/29/94 10:23:00 */
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: header file for trace
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

/*
 * exit codes
 * The -ga option cause trace to find the first available channel
 * exit with the channel as the exit code. These error codes were
 * chosen so that they do not conflict with the channel and the exec-ing
 * program (usually the trcstart() subroutine, can determine if an error 
 * (> 0x80 or < 0, depending on if the status code is unsigned) or successful
 * trace start occurred.
 */
#define EXIT_NOFILE     0x81
#define EXIT_BADOPT     0x82
#define EXIT_SIGNAL     0x83
#define EXIT_NOLOGFILE  0x84
#define EXIT_NOCHANNELS 0x85
#define EXIT_EXEC       0x86
#define EXIT_ODMINIT    0x87
#define EXIT_ODMLOCK    0x88
#define EXIT_MKSPECIAL  0x89
#define EXIT_LOADEXT    0x8A
#define EXIT_CFGINIT    0x8B

#define SYSTRACE "/dev/systrace"
#define SYSTRCTL "/dev/systrctl"

extern Logfd;
extern Systracefd;
extern Systrctlfd;

extern Signo;

extern Debugflg;
extern nohdrflag;
extern stdoutflag;
extern delayflag;
extern stopflag;
extern nohdrflag;
extern asyncflag;
extern genericflag;
extern verboseflag;
extern saveflag;
extern nmflag;
extern sid;

extern char *Progname;
extern Logsize;
extern Tracebufsize;
extern Channel;

extern char *Message;
extern char Logfile[];
extern Tracemode;
extern Ublimit;
extern Childpid;

/*
 * Defaults
 */
#define TRACESIZE_DFLT (128 * 1024)
#define LOGSIZE_DFLT   (10 * TRACESIZE_DFLT)
#define LOGFILE_DFLT   "/usr/adm/ras/trcfile"

#define M_CIRCULAR  1
#define M_ALTERNATE 2
#define M_SINGLE    3
#define M_BUS       4

/*
 * conditional hookid bitmap
 */
#define NHOOKIDS 4096
#define SETHOOKID(cp,i) (cp[(i)/8] |=  (1 << (i)%8))
#define CLRHOOKID(cp,i) (cp[(i)/8] &= ~(1 << (i)%8))
#define ISHOOKID(cp,i)  (cp[(i)/8] &   (1 << (i)%8))
#define HOOKTOID(hw)    (((hw) >> 20) & 0xFFF)
extern unsigned char Condhookids[NHOOKIDS/8];

/*
 * The subcommand line scanning routine will fill
 *  in up to NARG subcommand line arguments, including subcommand name.
 * The name of the command is Arg[0], the arguments are Arg[1] - Arg[9]
 * Narg == 1 if there are no arguments (like main(argc,argv))
 */
#define NARG 10
extern int Narg;
extern char *Arg[NARG];

#define MCS_CATALOG "cmdtrace.cat"
#include "cmdtrace_msg.h"

#include <ras.h>




