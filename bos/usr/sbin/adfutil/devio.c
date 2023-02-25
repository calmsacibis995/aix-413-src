static char sccsid[] = "@(#)07	1.1  src/bos/usr/sbin/adfutil/devio.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:03";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: _devio 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define _NO_PROTO
#include <stdio.h>
#include "errno.h"
#include "fcntl.h"
#include "sys/signal.h"
#include <sys/stat.h>
#ifdef aiws
#include "sys/fd.h"
#endif
#include "symbol.h"
#include "pcdos.h"

/*
 *          _devio - handle DOS abort/retry/ignore semantics.
 *
 * This needs to be enveloped around every system call that might
 * spin a floppy in libdos.
 *
 * Note 1: sync() and fsync() don't spin the disk.
 *
 * Note 2: calls to access UNIX files on a mounted disk need not return
 *         EIO or ENXIO since I/O happens asynchronously in the kernel.
 */

extern int ioctl();
extern int dostrace, doserrno;

_devio(op,arg1,arg2,arg3,arg4)
register int (*op)(),arg1,arg2,arg3,arg4;
{
	register int rc;
	int ch = -1;
	char message[28];
	char response[128], *msg, *opname;

retry:
	if (op)		/* op == 0 if dmount() opened device O_RDONLY */
	{
		rc = (*op)(arg1,arg2,arg3,arg4);
		if (rc >= 0 ) return rc;
	}
	else
		errno = EWRPROTECT;

	switch(errno) {
		case ENOTREADY:		msg = NL_MSG(DEVIO_NOTREADY,
			"Not ready error during %s on drive %s\n"); break;
		case EBUSY:		msg = NL_MSG(DEVIO_DEVBUSY,
			"Device busy error during %s on drive %s\n"); break;
		case EIO:		msg = NL_MSG(DEVIO_DATA,
			"Data error during %s on drive %s\n"); break;
		case EWRPROTECT:	msg = NL_MSG(DEVIO_WRIPROT,
			"Write protect error during %s on drive %s\n"); break;
		case ENXIO:
		case EFORMAT:		msg = NL_MSG(DEVIO_GENFAIL,
			"General Failure error during %s on drive %s\n"); break;

		default: /* shouldn't we do something here? */
			     doserrno = errno;
			     return rc;

	}

	if (op==open)  opname = "open()";
	  else if (op==read)  opname = "read()";
	    else if (op==write)  opname = "write()";
	      else if (op==ioctl)  opname = "ioctl()";
		else if (op==fstat)  opname = "fstat()";
		  else if (op==close)  opname = "close()";
		    else if (op==link)  opname = "link()";
		      else if (op==unlink)  opname = "unlink()";
			else     opname = "write()";

	fprintf(stderr, msg, opname, devicename);

query:
	fprintf(stderr,NL_MSG(DEVIO_ABORT, "Abort, Retry, Ignore?"));
	if ((ch = _pause(" ")) == -1) return -1;  /* can't open tty */
	response[0] = ch;
	response[1] = '\n';
	if (ch =='\n')
		response[1] = '\0';
	else
		if ((ch=='\003')||(ch == '\177'))
		{       response[0] = '^';
			response[1] = 'C';
			response[2] = '\n';
			response[3] = '\0';
			ch = 'a';
		}
		else
			response[2] = '\0';
	fprintf(stderr,"%s",response);

	switch(ch)
	{       case 'A': case 'a':
				    if (op != open && op != close)
					close(arg1);
				    _runmount();
				    kill( getpid(), SIGINT );
				    return(-1);
		case 'R': case 'r': if (op != open && op != ioctl && op != 0)
#ifdef aiws
					ioctl(arg1, FDIOCRESET, 0 );
#endif
				    goto retry;
		case 'I': case 'i': break;
		default:            goto query;
	}
ignore:
	/* ignore option depends on routine called */
	if (op == open) return open("/dev/null", arg2, arg3 );
	if (op == read || op == write || op == 0)
	{        /* to ignore, seek past requested block */
		lseek(arg1,arg3,1);
		return arg3;
	}
	return 0;
}


_devio1(disk, seekp, op,arg1,arg2,arg3,arg4)
DCB *disk;
long seekp;
register int (*op)(),arg1,arg2,arg3,arg4;
{
	register int rc;
	int ch = -1;
	char message[28];
	char response[128], *msg, *opname;

retry:
	if (op)		/* op == 0 if dmount() opened device O_RDONLY */
	{
		rc = (*op)(arg1,arg2,arg3,arg4);
		if (rc >= 0 ) return rc;
	}
	else
		errno = EWRPROTECT;

	switch(errno) {
		case ENOTREADY:		msg = NL_MSG(DEVIO_NOTREADY,
			"Not ready error during %s on drive %s\n"); break;
		case EBUSY:		msg = NL_MSG(DEVIO_DEVBUSY,
			"Device busy error during %s on drive %s\n"); break;
		case EIO:		msg = NL_MSG(DEVIO_DATA,
			"Data error during %s on drive %s\n"); break;
		case EWRPROTECT:	msg = NL_MSG(DEVIO_WRIPROT,
			"Write protect error during %s on drive %s\n"); break;
		case ENXIO:
		case EFORMAT:		msg = NL_MSG(DEVIO_GENFAIL,
			"General Failure error during %s on drive %s\n"); break;

		default: /* shouldn't we do something here? */
			     doserrno = errno;
			     return rc;
	}

	if (op==open)  opname = "open()";
	  else if (op==read)  opname = "read()";
	    else if (op==write)  opname = "write()";
	      else if (op==ioctl)  opname = "ioctl()";
		else if (op==fstat)  opname = "fstat()";
		  else if (op==close)  opname = "close()";
		    else if (op==link)  opname = "link()";
		      else if (op==unlink)  opname = "unlink()";
			else     opname = "write()";

	fprintf(stderr, msg, opname, devicename);

query:
	fprintf(stderr,NL_MSG(DEVIO_ABORT, "Abort, Retry, Ignore?"));
	if ((ch = _pause(" ")) == -1) return -1;  /* can't open tty */
	response[0] = ch;
	response[1] = '\n';
	if (ch =='\n')
		response[1] = '\0';
	else
		if ((ch=='\003')||(ch == '\177'))
		{       response[0] = '^';
			response[1] = 'C';
			response[2] = '\n';
			response[3] = '\0';
			ch = 'a';
		}
		else
			response[2] = '\0';
	fprintf(stderr,"%s",response);

	switch(ch)
	{       case 'A': case 'a':
				    if (op != open && op != close)
					close(arg1);
				    _runmount();
				    kill( getpid(), SIGINT );
				    return(-1);
		case 'R': case 'r': if (op != open && op != ioctl && op != 0)
#ifdef aiws
					ioctl(arg1, FDIOCRESET, 0 );
#else
				    	;
#endif
				    if (errno == EWRPROTECT && op == 0) {
					int tmpfd;

					op = write;
					close(arg1);
					tmpfd = open(disk->dev_name, O_RDWR|020);
					if (tmpfd < 0) {
					    op = 0;
					    goto retry;
					}
					lseek(disk->fd, seekp, 0);
					disk->fd = tmpfd;
					disk->protect = (int)write;
				    }
					
				    goto retry;
		case 'I': case 'i': break;
		default:            goto query;
	}
ignore:
	/* ignore option depends on routine called */
	if (op == open) return open("/dev/null", arg2, arg3 );
	if (op == read || op == write || op == 0)
	{        /* to ignore, seek past requested block */
		lseek(arg1,arg3,1);
		return arg3;
	}
	return 0;
}
