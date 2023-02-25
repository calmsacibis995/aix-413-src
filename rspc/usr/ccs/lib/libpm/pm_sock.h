/* @(#)15       1.1  src/rspc/usr/ccs/lib/libpm/pm_sock.h, pwrcmd, rspc41J, 9510A 3/8/95 07:41:17 */
/*
 * COMPONENT_NAME: pwrcmd
 *
 * FUNCTIONS: Power Management Library
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_PM_SOCK
#define _H_PM_SOCK

/* pm_sock.c */
extern int pmlib_create_sock(struct sockaddr_un *, struct sockaddr_un *);
extern void pmlib_close_sock(int, struct sockaddr_un *);
extern int pmlib_recv_reply(struct pmlib_client *, struct pmlib_server *, int);
extern char *mktemp(char *);

#endif /* _H_PM_SOCK */
