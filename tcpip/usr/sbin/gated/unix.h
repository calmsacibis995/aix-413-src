/* @(#)81	1.4  src/tcpip/usr/sbin/gated/unix.h, tcprouting, tcpip411, GOLD410 12/6/93 18:05:34 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		PROTOTYPEV
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * unix.h,v 1.9 1992/10/15 14:51:04 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* system function type declarations */

PROTOTYPE(free,
	  extern void,
	  (caddr_t ptr));
PROTOTYPE(send,
	  extern int,
	  (int s,
	   caddr_t msg,
	   int len,
	   int flags));
PROTOTYPE(getpeername,
	  extern int,
	  (int s, struct
	   sockaddr * name,
	   int *namelen));
PROTOTYPE(fcntl,
	  extern int,
	  (int fd,
	   int cmd,
	   int arg));
PROTOTYPE(setsockopt,
	  extern int,
	  (int s,
	   int level,
	   int optname,
	   caddr_t optval,
	   int optlen));
PROTOTYPE(close,
	  extern int,
	  (int fd));
PROTOTYPE(connect,
	  extern int,
	  (int s, struct
	   sockaddr * name,
	   int namelen));
PROTOTYPE(accept,
	  extern int,
	  (int s,
	   struct sockaddr * addr,
	   int *addrlen));
PROTOTYPE(bind, 
	  extern int,
	  (int s,
	   struct sockaddr * name,
	   int namelen));
PROTOTYPE(listen,
	  extern int,
	  (int s,
	   int n));
PROTOTYPE(recv,
	  extern int,
	  (int s,
	   caddr_t buff,
	   int len,
	   int flags));
PROTOTYPE(qsort,
	  extern void,
	  (caddr_t base,
	   int nel,
	   int width,
	   _PROTOTYPE(compare,
		      int,
		      (void_t,
		       void_t))));
#ifdef	INCLUDE_TIME
PROTOTYPE(gettimeofday,
	  extern int,
	  (struct timeval * tp,
	   struct timezone * tzp));
#endif
PROTOTYPE(sendto,
	  extern int,
	  (int s,
	   caddr_t msg,
	   int len,
	   int flags,
	   struct sockaddr * to,
	   int tolen));
PROTOTYPE(ioctl,
	  extern int,
	  (int d,
	   unsigned long request,
	   caddr_t argp));
#ifdef	INCLUDE_NLIST
PROTOTYPE(nlist,
	  extern int,
	  (const char *filename,
	   struct nlist * nl));
#endif
PROTOTYPE(open,
	  extern int,
	  (const char *path,
	   int flags,
	   int mode));
PROTOTYPE(read,
	  extern int,
	  (int d,
	   caddr_t buf,
	   int nbytes));
PROTOTYPE(lseek,
	  extern off_t,
	  (int,
	   off_t,
	   int));
PROTOTYPE(gethostname,
	  extern int,
	  (char *name,
	   int namelen));
PROTOTYPE(fork,
	  extern int,
	  (void));
PROTOTYPE(exit,
	  extern void,
	  (int status));
PROTOTYPE(getdtablesize,
	  extern int,
	  (void));
PROTOTYPE(getpid,
	  extern int,
	  (void));
PROTOTYPE(openlog,
	  extern void,
	  (const char *ident,
	   int logopt,
	   int facility));
PROTOTYPE(setlogmask,
	  extern void,
	  (int maskpri));
PROTOTYPE(srandom,
	  extern void,
	  (int seed));
PROTOTYPE(random,
	  extern long,
	  (void));
PROTOTYPE(chdir,
	  extern int,
	  (const char *path));
PROTOTYPE(abort,
	  extern void,
	  (void));
PROTOTYPE(insque,
	  extern void,
	  (struct qelem * elem,
	   struct qelem * pred));
PROTOTYPE(remque,
	  extern void,
	  (struct qelem * elem));
PROTOTYPE(fputs,
	  extern int,
	  (const char *s,
	   FILE * stream));
PROTOTYPE(kill,
	  extern int,
	  (int pid,
	   int sig));
#ifdef	INCLUDE_TIME
PROTOTYPE(setitimer,
	  extern int,
	  (int which,
	   struct itimerval * value,
	   struct itimerval * ovalue));
#endif
PROTOTYPE(recvmsg,
	  extern int,
	  (int s,
	   struct msghdr * msg,
	   int flags));
#ifdef	INCLUDE_TIME
PROTOTYPE(select,
	  extern int,
	  (int nfds,
	   fd_set * readfds,
	   fd_set * writefds,
	   fd_set * exceptfds,
	   struct timeval * timeout));
#endif
PROTOTYPE(sigblock,
	  extern int,
	  (int mask));
PROTOTYPE(sigsetmask,
	  extern int,
	  (int mask));
#ifdef	INCLUDE_WAIT
PROTOTYPE(wait3,
	  extern int,
	  (union wait * status,
	   int options,
	   caddr_t /* XXX */));
#endif
PROTOTYPE(sigvec,
	  extern int,
	  (int sig,
	   struct sigvec * vec,
	   struct sigvec * ovec));
PROTOTYPE(socket,
	  extern int,
	  (int domain,
	   int type,
	   int protocol));
PROTOTYPE(sleep,
	  extern void,
	  (unsigned seconds));
PROTOTYPE(fclose,
	  extern int,
	  (FILE * fp));
#ifdef	INCLUDE_STAT
PROTOTYPE(stat,
	  extern int,
	  (const char *path,
	   struct stat * buf));
#endif
PROTOTYPE(setlinebuf,
	  extern int,
	  (FILE *));
PROTOTYPE(setbuf,
	  extern void,
	  (FILE *,
	   caddr_t));
PROTOTYPE(setvbuf,
	  extern int,
	  (FILE *,
	   caddr_t,
	   int,
	   size_t));
PROTOTYPE(fflush,
	  extern int,
	  (FILE * fp));
PROTOTYPE(fputc,
	  extern int,
	  (char c, FILE * stream));
PROTOTYPEV(syslog,
	   extern int,
	   (int, const char *, ...));
PROTOTYPE(calloc,
	  extern caddr_t,
	  (unsigned nelem,
	   size_t));
PROTOTYPE(malloc,
	  extern caddr_t,
	  (size_t));
#ifdef	notdef
PROTOTYPE(alloca,
	  extern caddr_t,
	  (int size));
#endif
PROTOTYPE(atoi,
	  extern int,
	  (const char *nptr));
#ifdef	notdef
PROTOTYPE(index,
	  extern char *,
	  (char *,
	   char));
PROTOTYPE(rindex,
	  extern char *,
	  (char *,
	   char));
#endif
PROTOTYPE(getwd,
	  extern char *,
	  (char *pathname));
PROTOTYPE(sethostent,
	  extern,
	  (int stayopen));
PROTOTYPE(endhostent,
	  extern,
	  (void));
PROTOTYPE(endnetent,
	  extern,
	  (void));
PROTOTYPE(setnetent,
	  extern,
	  (int stayopen));


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
