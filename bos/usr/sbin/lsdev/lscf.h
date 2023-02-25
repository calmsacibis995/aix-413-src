/* @(#)90	1.8  src/bos/usr/sbin/lsdev/lscf.h, cmdcfg, bos41J, 9520A_all 4/27/95 13:26:35 */

/*
 * COMPONENT_NAME: (CMDCFG) Generic config support cmds
 *
 * FUNCTIONS: 
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

#ifndef _H_LSCF
#include "lscfconst.h"

extern int Cflag;		/* -C option was found */
extern int Fflag;		/* -F option was found */
extern int Pflag;		/* -P option was found */
extern int hflag;		/* -h option was found */
extern int rflag;		/* -r option was found */
extern int print_hdr;		/* -H option was found - print header */
extern int colon_fmt;		/* -O option was found - colon format */
extern int read_from_stdin;    /* - option was found */

extern int columnize;		/* with -F, align values in columns */

extern char *entry_class;	/* -c option argument */
extern char *entry_state;	/* -S option argument */
extern char *entry_parent;	/* -p option argument */
extern char *entry_subclass;	/* -s option argument */
extern char *entry_key;	/* -k option argument */
extern char *entry_type;	/* -t option argument */
extern char *entry_name;	/* -l option argument */
extern char *entry_col;	/* -r option argument */
extern char *entry_attrs;	/* -a option argument */
extern char *entry_fmt;	/* -F option argument */

extern struct CuDv *cudv;	/* Customized device list */
extern struct listinfo cu_info;	/* CuDv info struct */

extern struct CuDv *parcudv;	 /* parent Customized device list */
extern struct listinfo par_info; /* parent CuDv info struct */

extern struct PdDv *pddv;	/* Predefined device list */
extern struct listinfo pd_info;	/* PdDv info struct */

extern struct PdCn *conn;	/* Predefined connection list */
extern struct listinfo conn_info;	/* PdCn info struct */

extern struct CombinedAttr {
		struct CuAt *cu;
		struct PdAt *pd;
		} combined_attr[MAX_ATTRS];   /* Attribute list */
extern int nattrs;			/* number of attrs */

char *get_subclass(),*get_nls(),*unescape(),*strchr();
long strtol();

extern char **values;		/* unique values used by -r flag */
extern int nvalues;		/* number of unique values */

extern char msg[BUFSIZ];			/* buffer for strings and messages */

extern char name_chars[BUFSIZ];		/* legal chars for class and col names*/
extern int col_sz[MAX_FCOLS];			/* printed size of each column */

#endif /* _H_LSCF */
