/* @(#)79  1.1  src/bos/usr/sbin/snap/check.h, cmdsnap, bos411, 9432A411a 8/4/94 13:37:12 */

/*
 * COMPONENT_NAME: CMDSNAP 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include  "a.out.h"
#include  "signal.h"
#include  "sys/param.h"
#include  "sys/sysmacros.h"
#include  "sys/types.h"
#include  "sys/dir.h"
#include  "sys/user.h"
#include  "sys/var.h"
#include  "stdio.h"
#include  "sys/seg.h"

#define  DMPRTNS_DIR	"/usr/lib/ras/dmprtns"

#define	CURPROC	-1	/* Use the current or last running process's data */
#undef  CURTHREAD
#define CURTHREAD -1
#define	SHORT	0
#define LONG	1
#define RUNNING 1
#define BADSLOT   -2
#define NONE -2
#define NOTSTORED -2

#define MATCHNAME 1	/* for search() */

#define min(a,b) ((a)<(b)?(a):(b))
#define MAXCDTS 200		/* max num of cdts to look for in a dump */
#define	VIRT_MEM	0xffffffff
#define	SYM_VALUE(ptr)	(ptr->n_value)
#define	FMT	"%8.8x"
#define	SWAPPED	1	/* Returned by getuarea if process is swapped */
#define OFF_MSK 0x0fffffff /* strips the segment number */

struct	tsw	{
	char	*t_nm;
	int	t_sw;
	char	*t_dsc;
} ;

struct	prmode	{
	char	*pr_name;
	int	pr_sw;
} ;

#define MAXI	30

int	mem;
extern	int	kmem;

struct var v;

struct cm
{
	int cm_thread;
	int cm_reg;
	ulong cm_segid;
	tid_t cm_tid;
};

#ifndef u_uid     /* These defines will be removed from user.h */

#define        u_uid      u_cred->cr_uid
#define        u_ruid     u_cred->cr_ruid
#define        u_suid     u_cred->cr_suid
#define        u_luid     u_cred->cr_luid
#define        u_acctid   u_cred->cr_acctid
#define        u_gid      u_cred->cr_gid
#define        u_rgid     u_cred->cr_rgid
#define        u_sgid     u_cred->cr_sgid
#define        u_groups   u_cred->cr_groups
#define        u_epriv    u_cred->cr_epriv
#define        u_ipriv    u_cred->cr_ipriv
#define        u_bpriv    u_cred->cr_bpriv
#define        u_mpriv    u_cred->cr_mpriv

#endif

ulong p_fmodsw; /* address of fmodsw  */
ulong p_dmodsw; /* address of dmodsw */
ulong p_sth_open_streams; /* address of sth_open_streams */
ulong p_streams_runq; /* address of streams_runq */


