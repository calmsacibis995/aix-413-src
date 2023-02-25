/* @(#)24	1.7  src/bos/usr/sbin/audit/audutil.h, cmdsaud, bos411, 9428A410j 3/17/93 11:16:38 */
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: audutil.h for audit commands
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#ifndef _h_AUDUTIL
#define _h_AUDUTIL

#include	<sys/audit.h>

#define	LINE_SIZE	512

#define	MAXCLASSES	32

#define AUDHEADSIZE	(sizeof(struct aud_rec))
#define	MAXTAILSIZE	4096
#define BINHEADSIZE	(sizeof(struct aud_bin))

#define	PROG_AUDITB	"/usr/sbin/auditbin"
#define	PROG_AUDITPR	"/usr/sbin/auditpr"
#define	PATH_EVENT	"/etc/security/audit/events"
#define	PATH_CONFIG	"/etc/security/audit/config"
#define	PATH_OBJECT	"/etc/security/audit/objects"

#define	STZ_AUDIT	"audit"
#define	STZ_BIN		"bin"
#define	STZ_START	"start"
#define	STZ_STREAM	"stream"
#define	STZ_CLASSES	"classes"
#define	 KWD_BIN1	"bin1"
#define	 KWD_BIN2	"bin2"
#define	 KWD_TRAIL	"trail"
#define	 KWD_BINSIZE	"binsize"
#define	 KWD_BINMODE	"binmode"
#define	 KWD_STRMODE	"streammode"
#define	 KWD_CLASSES	"classes"
#define	 KWD_CMDS	"cmds"

#define	STZ_AUDITPR	"auditpr"

/* exported by audutil.c */
extern	int	parseline();
extern	char	*getnext();
extern	void	exit1();
extern	int	getline();
extern	int	bytecopy();
extern	char	*xalloc ();

/* exported by cfgcutil.c */
extern	char	*getattr();

#define	PUBLIC
#define	PRIVATE	static

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#ifdef	DEBUG
	int	Debug;
	extern	char	*list_dprintf();
#	define	DPRINTF(args)		{ if (Debug) dprintf args ; }
#	define	DEBUGNAME(name)		char	*DebugName = name;
#else
#	define	DPRINTF(args)
#	define	DEBUGNAME(name)
#endif

#endif
