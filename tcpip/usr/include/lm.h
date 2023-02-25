/* @(#)64	1.2.1.1  src/tcpip/usr/include/lm.h, tcpip, tcpip411, GOLD410 3/18/94 13:10:32 */
/*
 * COMPONENT_NAME: TCPIP lm.h
 *
 * FUNCTIONS: 
 *            
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef __SRC_USR_SBIN_LM_LM_H_
#define __SRC_USR_SBIN_LM_LM_H_


/*
 * DESCRIPTION:	Login Monitor message descriptions
 */
#include <sys/types.h>

#ifndef _H_LM
#define _H_LM

#define NMAX 				30
#define ENVMAX 				64
#define TTYPATHSIZE 			32
#define HOSTNMSIZE	 		64
#define SPEEDMAX	 		10

/*
 * Types of virtual terminal protocols
 */
#define	TELNETD_VTP			1
#define	RLOGIND_VTP			2

/*
 * Terminal Solicitor required variables when communicating 
 * with the Login Monitor (LM).
 */
typedef struct ts_var_for_lm {
	char term_type[ENVMAX];	 /* terminal type */
	char speed[SPEEDMAX];    /* baud rate of terminal */
} TS_VAR;

/*
 * Telnet required variables when communicating 
 * with the Login Monitor (LM).
 */
typedef struct telnetd_var_for_lm {
	char term_env0[ENVMAX];	/* "TERM=" to set external variable, environ */
	char rhostname[HOSTNMSIZE];     /* host name (client) */
} TELNETD_VAR;

/*
 * Rlogin required variables when communicating 
 * with the Login Monitor (LM).
 */
typedef struct rlogind_var_for_lm {
	char term_env0[ENVMAX];	/* "TERM=" to set external variable, environ */
	char rhostname[HOSTNMSIZE];     /* host name (client) */
	char rusername[NMAX+1];  /* user name (client) */
	char lusername[NMAX+1];  /* user name (server) */
	char speed[SPEEDMAX];    /* baud rate of terminal */
} RLOGIND_VAR;

/*
 * Required variables for all virtual terminal
 * protocols (vtp) when communicating 
 * with the Login Monitor (LM).
 */
typedef struct var_for_lm {
	uchar_t vtp_type;		/* virtual terminal protocol type */
	union{ 				/* variables needed by LM depending */
		TELNETD_VAR tv;		/* on the vtp type.                 */
		RLOGIND_VAR rv;
	} u;
} LM_VTP_VAR;


/* message structure for terminal solicitor */
struct lm_message_hdr {
	uchar_t		lm_size;	/* message size */
	uchar_t		lm_msgtype;	/* message type */
	char		lm_devname[TTYPATHSIZE];	/* ocs tty pathname */
};
 
struct lm_response {
	int	status;		/* can be LM_OPEN_SUCCESS, etc */
};
 
/* types of login monitor requests */
#define TS_OPEN_TERMINAL	1 /* terminal solicitor open terminal request */
#define TN_OPEN_TERMINAL	2 /* telnetd open terminal request */
#define TN_REMOVE_FILE		3 /* remove file name (after close) */

/*
 * Notes:
 *	rtyservclose calls the terminal solicitor after a close to re-open 
 *	the terminal with the appropriate hosts options.
 */


/* types of login monitor responses */
#define LM_ACK			5	/* acknowledgement from login monitor */
#define LM_OPEN_SUCCESS		6	/* file opened succesfully */
#define LM_OPEN_FAILED		7	/* file did not open succesfully */
#define LM_NO_DEVNAME		8	/* could not get devname */
#define LM_READ_MSG_FAILED	9	/* read lm request failed */
#define LM_INVALID_PROTOCOL     10      /* invalid protocol type */
#define LM_DUP_FAILED		11	/* couldn't duplicate file descriptor */
#define LM_EXEC_FAILED		12	/* couldn't exec */
#define LM_INVALID_MSGTYPE      13      /* invalid lm message type */
#define LM_MALLOC_MSG_ERROR     14      /* malloc() lm message error */
#define LM_REQUEST_FAILED       15      /* accept() error for LM req */
 
 
#define TS_PORT		9992
#define LM_PORT		9993
 
/* xxx use MAXHOSTNAMELEN instead */
#define MAX_HOSTNAME_LEN	80
#define MGETTY_TIMEOUT		1
#define MGETTY_TRY_SEND		4
 
#endif /* _H_LM */
 
#endif /* __SRC_USR_SBIN_LM_LM_H_ */
