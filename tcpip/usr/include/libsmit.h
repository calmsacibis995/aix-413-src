/* @(#)63	1.2  src/tcpip/usr/include/libsmit.h, tcpip, tcpip411, GOLD410 12/11/92 10:26:23 */
/*
 * libsmit.h -  Header file for TCP/IP SMIT routines.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1988
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   

#define	BLANK		" "		/* pointer to blank string */

/* defines for ??PRTSV configuration stanza names */
#define	QUEUE		"queue"
#define	DEVICE		"device"

/* defines for low-level command names */
#define	NAMERSLV	"/usr/sbin/namerslv"
#define	RUSER		"/usr/bin/ruser"		

/* command line parameter flags used by the various low-level commands */
#define	TCP_QUOTE	"\""		/* pointer to imbedded quote mark */

#define	TCPFLG_X	" -X "		/* show/delete all flag  */
#define	TCPFLG_Z	" -Z "		/* colon format flag */
#define	TCPFLG_a	" -a "		/* add flag */
#define	TCPFLG_c	" -c "		/* change flag */
#define	TCPFLG_d	" -d "		/* delete flag */
#define	TCPFLG_s	" -s "		/* show flag */

#define	TCPFLG_D	" -D "		/* domain flag for NAMERSLV command */
#define	TCPFLG_E	" -E "		/* filename flag for NAMERSLV command */
#define	TCPFLG_I	" -I "		/* IP addr flag for NAMERSLV command */
#define	TCPFLG_e	" -e "		/* filename flag for NAMERSLV command */
#define	TCPFLG_i	" -i "		/* IP addr flag for NAMERSLV command */
#define	TCPFLG_n	" -n "		/* domain flag for NAMERSLV command */

#define	TCPFLG_P	" -P "		/* hosts.lpd flag for RUSER command */
#define	TCPFLG_p	" -p "		/* hosts.lpd flag for RUSER command */

#define	TCPFLG_q	" -q "		/* queue name flag for qconfig file */

/* used to determine attribute value parameters for parsing commands */
#define	NO_ATTR		-1 		/* undefined attribute index */

/* used by ??NAMSV commands to setup parameters for NAMERSLV command call */
#define	NAMESERVER	0		/* nameserver attribute index */
#define	DOMAIN		1		/* domain attribute index */
char	*namerslv_attr[] = 
	{
	"nameserver=",
	"domain=",
	NULL
	}; 
