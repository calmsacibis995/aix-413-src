/* @(#)39	1.1  src/bos/usr/sbin/pse/utils/mtest/osmtest.h, cmdpse, bos411, 9428A410j 5/7/91 13:45:27 */
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** osmtest.h 1.1, last change 10/31/90
 **/

/* System V.3 definitions for MTEST */

#include <signal.h>
#include <sys/times.h>

#define	stream_read		read
#define	stream_write		write
#define	EnterCritSec(a)
#define	ExitCritSec(a)
#define	ExitThread(a,b)		exit(a)
#define	SET_TIME(t)		(t)->timer_ret = times(&(t)->timer_tms)

static	boolean	child_does_work = true;		/* Phooey! */

#define	parent_does_work	!child_does_work

typedef	struct timer_s {
	long		timer_ret;
	struct tms	timer_tms;
} TIMER;
