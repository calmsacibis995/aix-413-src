/* @(#)61	1.4  src/tcpip/usr/sbin/gated/parse.h, tcprouting, tcpip411, GOLD410 12/6/93 17:54:41 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PARSE_METRIC_ALTERNATE
 *		PARSE_METRIC_CLEAR
 *		PARSE_METRIC_INFINITY
 *		PARSE_METRIC_ISALTERNATE
 *		PARSE_METRIC_ISINFINITY
 *		PARSE_METRIC_ISRESTRICT
 *		PARSE_METRIC_ISSET
 *		PARSE_METRIC_RESTRICT
 *		PARSE_METRIC_SET
 *		PROTOTYPE
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
 * parse.h,v 1.12 1992/12/19 03:49:02 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	PS_INITIAL	0
#define	PS_OPTIONS	1
#define	PS_INTERFACE	2
#define	PS_DEFINE	3
#define	PS_PROTO	4
#define	PS_ROUTE	5
#define	PS_CONTROL	6
#define	PS_MIN		PS_OPTIONS
#define	PS_MAX		PS_CONTROL

#define	LIMIT_AS		1, 65534
#define	LIMIT_PREFERENCE	0, 255
#define	LIMIT_PORT		0, 65535
#define	LIMIT_NATURAL		0, (u_int) -1
#define	LIMIT_MINUTES		0, 59
#define	LIMIT_SECONDS		0, 59

typedef struct {
    char *fi_name;			/* File name */
    FILE *fi_FILE;			/* File pointer */
    int fi_lineno;			/* Line number */
} fi_info;

#define	FI_MAX	10			/* Maxiumum %include nesting level */

typedef struct {
    metric_t metric;			/* Actual metric */
    int state;				/* Metric state */
} pmet_t;

typedef struct {
    byte *ptr;			/* Pointer to the byte string */
    size_t len;				/* Length of string */
    flag_t type;			/* Original format of string */
} bytestr;

typedef struct {
    char *ptr;				/* Pointer to the byte string */
    size_t len;				/* Length of string */
    flag_t type;			/* Original format of string */
} charstr;

#define	PARSE_METRIC_CLEAR(x)		(x)->metric = (metric_t) 0, (x)->state = PARSE_METRICS_UNSET
#define	PARSE_METRIC_SET(x, y)		(x)->metric = y, (x)->state = PARSE_METRICS_SET
#define	PARSE_METRIC_INFINITY(x)	(x)->state = PARSE_METRICS_INFINITY
#define	PARSE_METRIC_RESTRICT(x)	(x)->state = PARSE_METRICS_RESTRICT
#define	PARSE_METRIC_ALTERNATE(x, y)	(x)->metric = y, (x)->state = PARSE_METRICS_ALTERNATE
#define	PARSE_METRIC_ISSET(x)		((x)->state != PARSE_METRICS_UNSET)
#define	PARSE_METRIC_ISINFINITY(x)	((x)->state == PARSE_METRICS_INFINITY)
#define	PARSE_METRIC_ISRESTRICT(x)	((x)->state == PARSE_METRICS_RESTRICT)
#define	PARSE_METRIC_ISALTERNATE(x)	((x)->state == PARSE_METRICS_ALTERNATE)

#define	PARSE_METRICS_UNSET		0		/* Metric has not yet been set */
#define	PARSE_METRICS_SET		1		/* Metric has been set */
#define	PARSE_METRICS_INFINITY		2		/* Metric set to infinity */
#define	PARSE_METRICS_RESTRICT		3		/* Metric is set to restrict (policy) */
#define	PARSE_METRICS_ALTERNATE		4		/* Alternate metric */

extern u_int parse_state;

PROTOTYPE(parse_where,
	  extern char *,
	  (void));
PROTOTYPE(parse_open,
	  extern int,
	  (char *));
PROTOTYPE(yylex,
	  extern int,
	  (void));

extern int yylineno;
extern char parse_error[];
extern char *parse_filename;
extern char *parse_directory;
extern proto_t protos_seen;
extern sockaddr_un parse_addr;

PROTOTYPE(yyparse,
	  extern int,
	  (void));
PROTOTYPE(parse_keyword,
	  extern bits *,
	  (char *));			/* Lookup a token given a keyword */
PROTOTYPE(parse_keyword_lookup,
	  extern bits *,
	  (int));			/* Lookup a keyword given a token */
PROTOTYPE(parse_parse,
	  extern int,
	  (const char *));		/* Parse the config file */
PROTOTYPE(parse_strdump,
	  char *,
	  (char *));			/* Return a pointer to a duplicate string */
PROTOTYPE(parse_where,
	  char *,
	  (void));				/* Return pointer to a string  giving current file and line */
PROTOTYPE(parse_limit_check,
	  int,
	  (const char *type,
	   u_int value,
	   u_int lower,
	   u_int upper));					/* Limit check an integer */
PROTOTYPE(parse_addr_hostname,
	  sockaddr_un *,
	  (char *));				/* Lookup a string as a host name */
PROTOTYPE(parse_addr_netname,
	  sockaddr_un *,
	  (char *));				/* Lookup a string as a network name */
PROTOTYPE(parse_adv_append,
	  int,
	  (adv_entry **,
	   adv_entry *,
	   int));						/* Append one advlist to another */
PROTOTYPE(parse_adv_dm_append,
	  adv_entry *,
	  (adv_entry *,
	   adv_entry *));
PROTOTYPE(parse_gw_flag,
	  int,
	  (adv_entry *,
	   proto_t,
	   flag_t));			/* Set flag in gw_entry for each element in list */
PROTOTYPE(parse_new_state,
	  int,
	  (int));			/* Switch to a new state if it is a logical progression */
								/* from the current state */
PROTOTYPE(parse_metric_check,
	  int,
	  (proto_t,
	   pmet_t *));	/* Verify a specified metric */
PROTOTYPE(parse_adv_propagate_metric,
	  adv_entry *,
	  (adv_entry *,
	   proto_t,
	   pmet_t *,
	   adv_entry *));			/* Set metric in list for elements without metrics */
PROTOTYPE(parse_adv_propagate_preference,
	  adv_entry *,
	  (adv_entry *,
	   proto_t,
	   pmet_t *,
	   adv_entry *));			/* Set preference in list for elements without metrics */
PROTOTYPE(parse_adv_propagate_config,
	  adv_entry *,
	  (adv_entry *,
	   config_list *,
	   proto_t));
PROTOTYPE(parse_adv_preference,
	  void,
	  (adv_entry *,
	   proto_t,
	   pref_t));			/* Set preference in list for elements without preference */
PROTOTYPE(parse_adv_dup,
	  adv_entry *,
	  (adv_entry *));	/* Return a pointer to a duplicate of this adv_entry */
PROTOTYPE(parse_adv_as,
	  int,
	  (adv_entry **,
	   adv_entry *));		/* Append this list to the list for the specified exterior protocol */
PROTOTYPE(parse_args,
	  int,
	  (int,
	   char **));


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
