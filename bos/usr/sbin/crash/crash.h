/* @(#)71   1.31  src/bos/usr/sbin/crash/crash.h, cmdcrash, bos411, 9438B411a 9/20/94 11:57:20 */

/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27,3,83
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
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

/*
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
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

extern	struct	var	v;
extern	int	mem;
extern	int	kmem;


struct cm
{
	int cm_thread;
	int cm_reg;
	ulong cm_segid;
	tid_t cm_tid;
};

extern	struct	nlist	*File, *Hinode, *Mount, *Swap, *Core, *Proc, *Sbuf,
			*Sys, *Time, *Panic, *Etext, *Text, *V, *Sbrpte,
			*Buf, *End, *Lbolt, *Dmpstk, *Gpa_vnode, *Nhino,
			*D_rsav, *Rootvfs,*g_kxsrval, *Hbuf, *Inode, *Vmker;
extern	struct	nlist	*Segid0, *Dmpsegs, *Dumpflag, *_pcregs, *_kpchk_psb,
			*Kernel_heap, *Pinned_heap, *Mbuf, *MbufC, *K_anchor,
			*Ifnet, *abend_csa, *abend_code;;
extern struct nlist    *Thread, *Ublock, *Ppd, *Sysconfig, *Dbp_fmodsw, *Dbp_dmodsw,
			*Dbp_sth_open_streams, *Dbp_streams_runq;

#define	STACK	1
#define	UAREA	2
#define	FILES	3
#define	TRACE	4
#define	QUIT	5
#define TCBLK	6
#define	INODE	7
#define	MOUNT	8
#define	TTY	9
#define	Q	10
#define	TEXT	11
#define	TS	13
#define	DS	14
#define	PROC	15
#define	STATS	16
#define	KFP	17
#define	BUFHDR	20
#define	BUFFER	21
#define	TOUT	22
#define	NM	23
#define	OD	24
#define	MAP	25
#define	VAR	28
#define VFS     29
#define VFSX    30
#define DVFS    31
#define DVFSX   32
#define VNODE   33
#define RESET   34
#define DUMPU   36
#define FS      37
#define CM      38
#define SOCKS   39
#define MBUFS   40
#define XXMALLOC 41
#define DUMP    42
#define KNLIST  43
#define	MST	44
#define	LE	45
#define MLIST   46
#define	PRINT	47
#define	ERRPT	48
#define NDB     49
#define THREAD	50
#define PPD	51
#define STATUS	52
#define CPU	53
#define SYMPTOM	54
#define DLA	60
#define STREAM  61
#define QUEUE   62
#define MBLOCK  63
#define DBLOCK  64
#define QRUN    65
#define LINKBLK 66
#define DMODSW  67
#define FMODSW  68


#define	DIRECT	2
#define	OCTAL	3
#define	DECIMAL	4
#define	CHAR	5
#define	WRITE	6
#define	INODE	7
#define	BYTE	8
#define	LDEC	9
#define	LOCT	10
#define	HEX	11
#define INST	12
#define	ASCII	13

#define LONG_BUF	0
#define SHORT_BUF	1

#define CLUSTER		1
#define SHOWDATA	2

#define	ALL		0
#define	FIND		1
#define	FINDTXT		2

#define TR_EXTENDED	0x01
#define TR_MST		0x02

/* Invalid instruction from decode() */
#define IVD_INST "       ?"

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


