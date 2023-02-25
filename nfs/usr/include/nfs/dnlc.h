/* @(#)01	1.3  src/nfs/usr/include/nfs/dnlc.h, sysxnfs, nfs411, GOLD410 9/21/89 14:16:26 */
/* 
 * COMPONENT_NAME: (SYSXNFS) Network File System Kernel
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1984 Sun Microsystems Inc.
 */

/*	(#)dnlc.h	2.1 88/05/18 4.0NFSSRC SMI;	*/
/*	(#)dnlc.h 2.6 86/07/16 SMI	*/

/*
 * This structure describes the elements in the cache of recent
 * names looked up.
 */

#define	NC_NAMLEN	15	/* maximum name segment length we bother with*/

struct	ncache {
	struct	ncache	*hash_next, *hash_prev;	/* hash chain, MUST BE FIRST */
	struct 	ncache	*lru_next, *lru_prev;	/* LRU chain */
	struct	vnode	*vp;			/* vnode the name refers to */
	struct	vnode	*dp;			/* vno of parent of name */
	char		namlen;			/* length of name */
	char		name[NC_NAMLEN];	/* segment name */
	struct	ucred	*cred;			/* credentials */
};

#define	ANYCRED	((struct ucred *) -1)
#define	NOCRED	((struct ucred *) 0)

int	ncsize;
struct	ncache *ncache;
