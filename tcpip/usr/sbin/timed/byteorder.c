static char sccsid[] = "@(#)49	1.3  src/tcpip/usr/sbin/timed/byteorder.c, tcptimer, tcpip411, GOLD410 3/18/91 11:33:34";
/* 
 * COMPONENT_NAME: TCPIP byteorder.c
 * 
 * FUNCTIONS: bytehostorder, bytenetorder 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
static char sccsid[] = "byteorder.c	2.6 (Berkeley) 6/18/88";
#endif  not lint */

#include <nl_types.h>
#include "timed_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s)  catgets(catd, MS_TIMED, n, s)

#include "globals.h"
#include <protocols/timed.h>

/*
 * Two routines to do the necessary byte swapping for timed protocol
 * messages. Protocol is defined in /usr/include/protocols/timed.h
 */

bytenetorder(ptr)
struct tsp *ptr;
{
	ptr->tsp_seq = htons((u_short)ptr->tsp_seq);
	switch (ptr->tsp_type) {

	case TSP_SETTIME:
	case TSP_ADJTIME:
	case TSP_SETDATE:
	case TSP_SETDATEREQ:
		ptr->tsp_time.tv_sec = htonl((u_long)ptr->tsp_time.tv_sec);
		ptr->tsp_time.tv_usec = htonl((u_long)ptr->tsp_time.tv_usec);
		break;
	
	default:
		break;		/* nothing more needed */
	}
}

bytehostorder(ptr)
struct tsp *ptr;
{
	ptr->tsp_seq = ntohs((u_short)ptr->tsp_seq);
	switch (ptr->tsp_type) {

	case TSP_SETTIME:
	case TSP_ADJTIME:
	case TSP_SETDATE:
	case TSP_SETDATEREQ:
		ptr->tsp_time.tv_sec = ntohl((u_long)ptr->tsp_time.tv_sec);
		ptr->tsp_time.tv_usec = ntohl((u_long)ptr->tsp_time.tv_usec);
		break;
	
	default:
		break;		/* nothing more needed */
	}
}
