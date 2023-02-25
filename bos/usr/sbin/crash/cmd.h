/* @(#)62        1.25  src/bos/usr/sbin/crash/cmd.h, cmdcrash, bos411, 9435A411a 8/25/94 17:08:01 */
/*
*/
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27,3,83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information 
*/
/*
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */



struct	tsw	t[] = {
 	"buffer",	BUFFER,	"buffer data",
	"b",		BUFFER,	NULL,
	"buf",		BUFHDR,	"buffer headers",
	"bufhdr",	BUFHDR,	NULL,
	"hdr",		BUFHDR,	NULL,
	"cm",		CM,	"change current segment map, used by od",
	"cpu",		CPU,	"set current cpu",
	"dblock",	DBLOCK,	"STREAMS data block headers",
	"dblk",		DBLOCK,	NULL,
	"dlock",	DLA,	"search for deadlocks",
	"dmodsw",	DMODSW,	"STREAMS driver switch table",
	"ds",		DS,	"data address namelist search",	
	"du",		DUMPU,	"dump user area of process",	
	"dump",		DUMP,	"list components of a dump",
	"errpt",	ERRPT, "error log messages",
	"errlog",	ERRPT, NULL,
	"file",		FILES,	"file table",
	"files",	FILES,	NULL,
	"f",		FILES,	NULL,
	"fmodsw",	FMODSW,	"STREAMS module switch table",
#ifdef _POWER
	"fs",		FS,	"formatted kernel stack dump",
#endif
	"inode",	INODE,	"inode table",
	"inode -",	INODE,	"inode table with block addresses",
	"ino",		INODE,	NULL,
	"i",		INODE,	NULL,
	"knlist",	KNLIST,	"name search from active kernel's knlist",
#ifdef _POWER
  	"kfp",		KFP,	"frame pointer for kernel stack trace start",
	"fp",		KFP,	NULL,
	"r1",		KFP,	NULL,
	"le",		LE,	"load list entries",
#endif
	"linkblk",	LINKBLK, "STREAMS linkblk table",
	"lblk",		LINKBLK, NULL,
 /****	"map",		MAP,	"map structures",
******************************************************************/
	"mblock",	MBLOCK,	"STREAMS message block headers",
	"mblk",		MBLOCK,	NULL,
	"mbuf",		MBUFS,	"mbuf structures",
	"mlist",        MLIST,  "list mbuf and what it points to",
	"mst",		MST,	"mst structures",
  	"ndb",		NDB,    "network debugger",
 	"nm",		NM,	"name search",
 	"od",		OD,	"dump symbol values",
	"rd",		OD,	NULL,
	"ppd",		PPD,	"prints the Per Processor Data Areas",
	"print",	PRINT, "formatted structure dump",
	"struct",	PRINT, NULL,
	"str", 		PRINT, NULL,
	"proc",		PROC,	"process table",
	"proc -",	PROC,	"process table long listing",
	"proc -r",	PROC,	"runnable processes only",
	"ps",		PROC,	NULL,
	"p",		PROC,	NULL,
	"!",		Q,	"escape to shell",
	"?",		Q,	"print this list of available commands",
	"qrun",		QRUN,	"scheduled STREAMS queues",
	"queue",	QUEUE,	"STREAMS queue",
	"que",		QUEUE,	NULL,
	"quit",		QUIT,	"exit",
	"q",		QUIT,	NULL,
#ifdef _POWER
	"kernel",	STACK,	NULL,
	"k",		STACK,	NULL,
#endif
	"socket",	SOCKS,	"socket structures",
	"sock",		SOCKS,	NULL,
#ifdef _POWER
	"stack",	STACK,	"kernel stack",
	"stk",		STACK,	NULL,
	"s",		STACK,	NULL,
#endif
	"stat",		STATS,	"dump statistics",
	"status",	STATUS,	"processor status",
	"stream",	STREAM, "stream head table",
	"symptom",	SYMPTOM,"Generate a symptom string",
	"tcb",		TCBLK,	"thread control block",
	"thread",	THREAD,	"display thread table",
	"thread -",	THREAD,	"thread table long listing",
	"thread -r",	THREAD,	"display runable threads",
	"thread -p",	THREAD, "display threads of specified process",
	"th",		THREAD, NULL,
 	"callout",	TOUT,	"active trb structures",
	"calls",	TOUT,	NULL,
	"call",		TOUT,	NULL,
	"timeout",	TOUT,	NULL,
	"time",		TOUT,	NULL,
	"tout",		TOUT,	NULL,
	"trb",		TOUT,	NULL,
	"c",		TOUT,	NULL,

#ifdef _POWER
	"trace",	TRACE,	"kernel stack trace",
	"trace -k",	TRACE,	"trace with extended information",
	"trace -m",	TRACE,	"trace using mst save area(s)",
	"trace -r",	TRACE,	"trace using saved kernel frame pointer",
	"t",		TRACE,	NULL,
#endif
	"ts",		TS,	"text address namelist search",
	"tty",		TTY,	"tty structures",
	"term",		TTY,	NULL,
	"dz",		TTY,	NULL,
	"dh",		TTY,	NULL,
	"user",		UAREA,	"user area",
	"u_area",	UAREA,	NULL,
	"uarea",	UAREA,	NULL,
	"u",		UAREA,	NULL,
	"var",		VAR,	"system variables",
	"tunables",	VAR,	NULL,
	"tunable",	VAR,	NULL,
	"tune",		VAR,	NULL,
	"v",		VAR,	NULL,
	"vfs",          VFS,    "virtual file systems",
 	"mount",	VFS,	NULL,
	"mnt",		VFS,	NULL,
	"m",		VFS,	NULL,
	"vfs -",        VFSX,   "virtual file systems + vnodes per vfs",
	"vnode",        VNODE,  "vnode (virtual inode)",
	"vn",           VNODE,  NULL,
	"xmalloc",      XXMALLOC, "kernel xmalloc usage",
	"xm",           XXMALLOC, NULL,
	"malloc",       XXMALLOC, NULL,
	0,
};

struct	prmode	prm[] = {
	"ascii",	ASCII,
	"asc",		ASCII,
	"a",		ASCII,
	"decimal",	DECIMAL,
	"dec",		DECIMAL,
	"e",		DECIMAL,
	"octal",	OCTAL,
	"oct",		OCTAL,
	"o",		OCTAL,
	"directory",	DIRECT,
	"direct",	DIRECT,
	"dir",		DIRECT,
	"d",		DIRECT,
	"write",	WRITE,
	"w",		WRITE,
	"character",	CHAR,
	"char",		CHAR,
	"c",		CHAR,
	"inode",	INODE,
	"ino",		INODE,
	"i",		INODE,
	"instruction",	INST,
	"inst",		INST,
	"I",		INST,
	"byte",		BYTE,
	"b",		BYTE,
	"longdec",	LDEC,
	"ld",		LDEC,
	"D",		LDEC,
	"longoct",	LOCT,
	"lo",		LOCT,
	"O",		LOCT,
	"hex",		HEX,
	"h",		HEX,
	"x",		HEX,
	"hexadec",	HEX,
	"hexadecimal",	HEX,
	NULL,		0
} ;
