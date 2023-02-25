/* @(#)96	1.5  src/nfs/usr/include/rpcsvc/yppasswd.h, cmdnfs, nfs411, GOLD410 1/17/94 14:33:27 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * (#) from SUN 1.4
 * Sun (and any other copyrights) go right hyar
 * 
 */

/* (#)yppasswd.h	1.1 12/13/88 17:47:54 */
/*	(#)yppasswd.h	1.2 88/05/08 4.0NFSSRC SMI	*/

#ifndef _H_rpcsvc_yppasswd
#define _H_rpcsvc_yppasswd

#define YPPASSWDPROG 100009
#define YPPASSWDPROC_UPDATE 1
#define YPPASSWDVERS_ORIG 1
#define YPPASSWDVERS 1

struct yppasswd {
	char *oldpass;		/* old (unencrypted) password */
	struct passwd newpw;	/* new pw structure */
};

int xdr_yppasswd();

#endif /*_H_rpcsvc_yppasswd*/
