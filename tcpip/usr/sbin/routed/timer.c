static char sccsid[] = "@(#)19  1.5  src/tcpip/usr/sbin/routed/timer.c, tcprouting, tcpip41J, 9511A_all 3/3/95 09:02:35";
/* 
 * COMPONENT_NAME: TCPIP timer.c
 * 
 * FUNCTIONS: hup, timer 
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
 * Copyright (c) 1983, 1988 Regents of the University of California.
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
static char sccsid[] = "@(#)timer.c	5.9 (Berkeley) 6/1/90";
#endif  not lint */

/*
 * Routing Table Management Daemon
 */
#include "defs.h"

int	faketime;

extern	int	garbage_time;

/*
 * Timer routine.  Performs routing information supply
 * duties and manages timers on routing table entries.
 * Management of the RTS_CHANGED bit assumes that we broadcast
 * each time called.
 */
timer()
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	struct rthash *base = hosthash;
	int doinghost = 1, timetobroadcast;
	extern int externalinterfaces;

	(void) gettimeofday(&now, (struct timezone *)NULL);
	faketime += TIMER_RATE;
	if (lookforinterfaces && (faketime % CHECK_INTERVAL) == 0)
		ifinit();
	timetobroadcast = supplier && (faketime % SUPPLY_INTERVAL) == 0;
again:
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
		rt = rh->rt_forw;
		for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
			/*
			 * We don't advance time on a routing entry for
			 * a passive gateway, or any interface if we're
			 * not acting as supplier.
			 */
			if (!(rt->rt_state & RTS_PASSIVE) &&
			    (supplier || !(rt->rt_state & RTS_INTERFACE)))
				rt->rt_timer += TIMER_RATE;
			if (rt->rt_timer >= garbage_time) {
				rt = rt->rt_back;
				rtdelete(rt->rt_forw);
				continue;
			}
			if (rt->rt_timer >= EXPIRE_TIME &&
			    rt->rt_metric < HOPCNT_INFINITY)
				rtchange(rt, &rt->rt_router, HOPCNT_INFINITY);
			rt->rt_state &= ~RTS_CHANGED;
		}
	}
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto again;
	}
	if (timetobroadcast) {
		toall(supply, 0, (struct interface *)NULL);
		lastbcast = now;
		lastfullupdate = now;
		needupdate = 0;		/* cancel any pending dynamic update */
		nextbcast.tv_sec = 0;
	}
}

/*
 * On hangup, let everyone know we're going away.
 */
hup()
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	struct rthash *base = hosthash;
	int doinghost = 1;

	if (supplier) {
again:
		for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
			rt = rh->rt_forw;
			for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw)
				rt->rt_metric = HOPCNT_INFINITY;
		}
		if (doinghost) {
			doinghost = 0;
			base = nethash;
			goto again;
		}
		toall(supply, 0, (struct interface *)NULL);
	}
	exit(1);
}
