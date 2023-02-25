static char sccsid[] = "@(#)82	1.1  src/tcpip/usr/sbin/gated/kvm.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:38";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		__PF1
 *		__PF2
 *		__PF4
 *		__PF5
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
 *  kvm.c,v 1.9 1993/03/22 02:41:45 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_NLIST
#define	INCLUDE_FILE

#include "include.h"
#ifndef vax11c
#include <sys/file.h>
#endif				/* vax11c */

/*
 *	Emulation of kvm library for systems that do not have it
 */

struct __kvm {
    int	kvm_fd;
    const char *kvm_nl;
    const char *kvm_core;
    const char *kvm_swap;
    char kvm_errmsg[BUFSIZ];
};
typedef struct __kvm kvm_t;

#if	__GNUC__ > 1
/* To prevent a warning about missing prototypes */

PROTOTYPE(kvm_openfiles,
	  extern kvm_t *,
	  (char *,
	   char *,
	   char *,
	   int,
	   char *));
PROTOTYPE(kvm_close,
	  extern int,
	  (kvm_t *));
PROTOTYPE(kvm_nlist,
	  extern int,
	  (kvm_t *,
	   struct nlist *));
PROTOTYPE(kvm_read,
	  extern int,
	  (kvm_t *,
	   off_t,
	   caddr_t,
	   int));
PROTOTYPE(kvm_write,
	  extern int,
	  (kvm_t *,
	   off_t,
	   caddr_t,
	   int));
PROTOTYPE(kvm_geterr,
	  extern char *,
	  (kvm_t *));
#endif


kvm_t *
kvm_openfiles __PF5(nl, char *,
		    core, char *,
		    swap, char *,
		    flags, int,
		    errbuf, char *)
{
#ifndef	vax11c
    int fd;
    kvm_t *kd = (kvm_t *) 0;
    const char *corefile = core ? core : _PATH_KMEM;

    fd = open(corefile, flags, 0);
    if (fd < 0) {
	sprintf(errbuf, "kvm_openfiles: %m");
    } else {
	kd = (kvm_t *) task_mem_calloc((task *) 0, 1, sizeof (kvm_t));

	kd->kvm_fd = fd;
	kd->kvm_nl = nl ? nl : _PATH_UNIX;
	kd->kvm_core = corefile;
	kd->kvm_swap = NULL;
	*kd->kvm_errmsg = (char) 0;
    }

    return kd;
#else	/* vax11c */
    return (kvm_t *) TRUE;
#endif	/* vax11c */
}


int
kvm_close __PF1 (kd, kvm_t *)
{
#ifndef	vax11c
    int rc = 0;

    if (kd->kvm_fd >= 0) {
	rc = close(kd->kvm_fd);
	if (rc != 0) {
	    sprintf(kd->kvm_errmsg, "kvm_close: %m");
	}
    }

    (void) task_mem_free((task *) 0, (void_t) kd);

#endif	/* vax11c */

    return 0;
}


int
kvm_nlist __PF2(kd, kvm_t *,
		nl, struct nlist *)
{
#ifdef	vax11c
    extern char *Network_Image_File;

    return multinet_kernel_nlist(Network_Image_File, nl);
#else	/* vax11c */
    return nlist(kd->kvm_nl, nl);
#endif	/* vax11c */
}


int
kvm_read __PF4(kd, kvm_t *,
	       addr, off_t,
	       buf, caddr_t,
	       nbytes, int)
{
    off_t rc;
#ifdef	vax11c
    rc = klseek(offset);
    if (rc == (off_t) -1) {
	sprintf(kd->kvm_errmsg, "kvm_read: klseek: %m");
    } else {
	rc = klread(buf, nbytes);
	if (rc == (off_t) -1) {
	    sprintf(kd->kvm_errmsg, "kvm_read: klread: %m");
	}
    }
#else	/* vax11c */

#ifdef	sgi
    addr &= 0x7fffffff;
#endif	/* sgi */

    rc = lseek(kd->kvm_fd, addr, 0);
    if (rc == (off_t) -1) {
	sprintf(kd->kvm_errmsg, "kvm_read: lseek: %m");
    } else {
	rc = read(kd->kvm_fd, buf, nbytes);
	if (rc == (off_t) -1) {
	    sprintf(kd->kvm_errmsg, "kvm_read: read: %m");
	}
    }
#endif	/* vax11c */

    return rc;
}


int
kvm_write __PF4(kd, kvm_t *,
	       addr, off_t,
	       buf, caddr_t,
	       nbytes, int)
{
    off_t rc;
#ifdef	vax11c
    rc = klseek(offset);
    if (rc == (off_t) -1) {
	sprintf(kvm_errmsg, "kvm_write: klseek: %m");
    } else {
	rc = klwrite(buf, nbytes);
	if (rc == (off_t) -1) {
	    sprintf(kvm_errmsg, "kvm_write: klwrite: %m");
	}
    }
#else	/* vax11c */

#ifdef	sgi
    addr &= 0x7fffffff;
#endif	/* sgi */

    rc = lseek(kd->kvm_fd, addr, 0);
    if (rc == (off_t) -1) {
	sprintf(kd->kvm_errmsg, "kvm_write: lseek: %m");
    } else {
	rc = write(kd->kvm_fd, buf, nbytes);
	if (rc == (off_t) -1) {
	    sprintf(kd->kvm_errmsg, "kvm_write: write: %m");
	}
    }
#endif	/* vax11c */

    return rc;
}


char *
kvm_geterr __PF1 (kd, kvm_t *)
{
    return kd->kvm_errmsg;
}


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
