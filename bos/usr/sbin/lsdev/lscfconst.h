/* @(#)91	1.5  src/bos/usr/sbin/lsdev/lscfconst.h, cmdcfg, bos411, 9428A410j 5/19/93 08:55:41 */

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

#ifndef _H_LSCFCONST

#define CONFIG_LOCK		"/etc/objrepos/config_lock"
#define LOCK_TIMEOUT		5

#define isoct(a) ((a)>='0' && (a)<='7')

#define BUFSIZE 2048
#define MAX_FCOLS 1024		/* maximum number of columns which can
				   be named in the -F format */
#define MAX_ATTRS 1024		/* maximum number of attrs which
				   can be named or listed */

/*-----------------------------  function selects for Fparse -------------- */

#define GETHDRSZ 0
#define GETVALSZ 1
#define PRINTHDR 2
#define PRINTVAL 3

/*-----------------------------  modes for lscf routines ------------------- */
#define CuDv_MODE 1
#define PdDv_MODE 2
#define PdCn_MODE 3
#define Attr_MODE 4

/*-----------------------------  warning and error macros ----------------- */

#define error(a) {{fprintf(stderr, ERRSTR(a),cmdname);fclean();exit(-1);}}
#define error1(a,b) {{fprintf(stderr, ERRSTR(a),cmdname,b);fclean();exit(-1);}}
#define error2(a,b,c) {{fprintf(stderr, ERRSTR(a),cmdname,b,c);fclean();exit(-1);}}
#define error3(a,b,c,d) {{fprintf(stderr, ERRSTR(a),cmdname,b,c,d);fclean();exit(-1);}}
#define error4(a,b,c,d,e) {{fprintf(stderr, ERRSTR(a),cmdname,b,c,d,e);fclean();exit(-1);}}
#define warning(a) {fprintf(stderr, ERRSTR(a),cmdname);}
#define warning1(a,b) {fprintf(stderr, ERRSTR(a),cmdname,b);}

#define usage_error(a,b) {{fprintf(stderr, ERRSTR(a),cmdname);\
	fprintf(stderr,MSGSTR(b));fclean();exit(-1);}}

#define usage_error1(a,b,c) {{fprintf(stderr, ERRSTR(a),cmdname,b);\
	fprintf(stderr,MSGSTR(c));fclean();exit(-1);}}

#define usage_error2(a,b,c,d) {{fprintf(stderr, ERRSTR(a),cmdname,b,c);\
	fprintf(stderr,MSGSTR(d));fclean();exit(-1);}}

#define usage_error3(a,b,c,d,e) {{fprintf(stderr, ERRSTR(a),cmdname,b,c,d);\
	fprintf(stderr,MSGSTR(e));fclean();exit(-1);}}

#define usage_error4(a,b,c,d,e,f) {{fprintf(stderr, ERRSTR(a),cmdname,b,c,d,e);\
	fprintf(stderr,MSGSTR(f));fclean();exit(-1);}}


#endif /* _H_LSCFCONST */

