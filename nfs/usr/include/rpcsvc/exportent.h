/* @(#)85	1.5   1/17/94 14:30:50 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Sun (and any other copyrights) go right hyar
 * 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.5 88/02/07 (C) 1986 SMI
 * 
 * 
*/

/*	@(#)exportent.h	1.1 88/03/15 4.0NFSSRC SMI	*/

#ifndef _H_rpcsvc_exportent
#define _H_rpcsvc_exportent

/*
 * Exported file system table, see exportent(3)
 */ 

#define TABFILE "/etc/xtab"		/* where the table is kept */

/*
 * Options keywords
 */
#define ACCESS_OPT	"access"	/* machines that can mount fs */
#define ROOT_OPT	"root"		/* machines with root access of fs */
#define RO_OPT		"ro"		/* export read-only */
#define RW_OPT		"rw"		/* export read-mostly */
#define ANON_OPT	"anon"		/* uid for anonymous requests */
#define SECURE_OPT	"secure"	/* require secure NFS for access */
#define WINDOW_OPT	"window"	/* expiration window for credential */

struct exportent {
	char *xent_dirname;	/* directory (or file) to export */
	char *xent_options;	/* options, as above */
};

extern FILE *setexportent();
extern void endexportent();
extern int remexportent();
extern int addexportent();
extern char *getexportopt();
extern struct exportent *getexportent();

#endif /*_H_rpcsvc_exportent*/
