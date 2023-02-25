/* @(#)87	1.6  src/tcpip/usr/include/isode/snmp/logging.h, snmp, tcpip411, GOLD410 11/10/93 10:54:51 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE: src/tcpip/usr/include/isode/snmp/logging.h
 */

#ifndef _LOGGING_
#define _LOGGING_

#include <nl_types.h>

struct logging {
    char        *l_filename;
    FILE        *l_logfp;
    int          l_enabled;              /* enabled ? */
    int          l_size;                 /* max filesize */
    int          l_level;                /* debug level */
};

#define SLOG_NONE	0
#define SLOG_EMERG	1
#define SLOG_ALERT	2
#define SLOG_CRIT	3
#define SLOG_FATAL	4
#define SLOG_EXCEPTIONS	5
#define SLOG_WARNING	6
#define SLOG_NOTICE	7
#define SLOG_PDUS	8
#define SLOG_TRACE	9
#define SLOG_DEBUG	10
#define SLOG_DEFAULT	11

/*
** MAXMSGSIZE is limited by syslog() MAXLINE.
*/
#define MAXMSGSIZE	1024	

#define MAXDEBUGLEVEL   3
#if	!defined(TRUE) || !defined(FALSE)
#define TRUE		1
#define FALSE		0
#endif

/* Message Catalog define and variable extern */
#ifdef _POWER
extern nl_catd snmpcatd;
#define MSGSTR(m,n,s) catgets(snmpcatd, (m), (n), (s))
#else
#define MSGSTR(m,n,s) (s)
#endif /* _POWER */

#endif /* _LOGGING_ */
