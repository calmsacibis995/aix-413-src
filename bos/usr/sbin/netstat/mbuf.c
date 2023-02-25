static char sccsid[] = "@(#)88	1.15  src/bos/usr/sbin/netstat/mbuf.c, cmdnet, bos41J, 9513A_all 3/21/95 23:53:33";
/*
 *   COMPONENT_NAME: CMDNET
 *
 *   FUNCTIONS: MSGSTR
 *		banner
 *		defined
 *		kmemstats
 *		mbpr
 *		sizeof
 *		table
 *
 *   ORIGINS: 26,27,85
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: mbuf.c,v $ $Revision: 2.8.2.2 $ (OSF) $Date: 1992/02/17 16:52:56 $";
#endif
/*
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

#ifndef lint
/* Updated to some of: "@(#)mbuf.c	5.9 (Berkeley) 6/18/90"; */
/*** "mbuf.c	5.7 (Berkeley) 9/25/89"; ***/
#endif /* not lint */

#include <stdio.h>
#include <stddef.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <net/net_globals.h>

#include <net/net_malloc.h>

#include <nl_types.h>
#include <locale.h>
#include "netstat_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_NETSTAT,Num,Str)
extern nl_catd catd;

off_t	kbuckaddr;
off_t	kstataddr;
int	ncpus;

#define	TBL_MALLOCS	1
#define	TBL_MALLOCT	2

struct tbl_malloct {
        char    ks_name[12];            /* bucket name                  */
        long    ks_inuse;               /* # packets this type in use   */
        long    ks_calls;               /* # calls for this size        */
        long    ks_memuse;              /* total memory held in bytes   */
        long    ks_limblocks;           /* # times blocked (hit limit)  */
        long    ks_mapblocks;           /* # times blocked (kernel map) */
        long    ks_maxused;             /* max # ever used              */
        long    ks_limit;               /* max # allowed to exist       */
        long    ks_failed;              /* # failed calls this type     */
};

struct tbl_mallocs {
	void *	kb_next;
        long    kb_calls;               /* # calls to malloc this size  */
        long    kb_total;               /* # blocks alloc'ed            */
        long    kb_totalfree;           /* # elements free this bucket  */
        long    kb_elmpercl;            /* # elements/allocation        */
        long    kb_highwat;             /* high water limit             */
        long    kb_couldfree;           /* over high water, could free  */
        long    kb_failed;              /* # failed calls               */
	simple_lock_data_t	kb_lock;
};

struct tbl_kmemstats {
	long	ks_inuse;	/* # of packets of this type currently in use */
	long	ks_calls;	/* total packets of this type ever allocated */
	long 	ks_memuse;	/* total memory held in bytes */
	long	ks_limblocks;	/* number of times blocked for hitting limit */
	long	ks_mapblocks;	/* number of times blocked for kernel map */
	long	ks_maxused;	/* maximum number ever used */
	long	ks_limit;	/* most that are allowed to exist */
	long	ks_failed;	/* total failed allocations */
	simple_lock_data_t	ks_lock;
};

/*
 * This structure yanked from <sys/systemcfg.h> in which the kernel variable
 * _system_configuration is externed.  The struct, however, is not named so
 * we must define it here.  All we need is ncpus.
 */
struct  _system_configuration {
	int architecture;	/* processor architecture */
	int implementation;	/* processor implementation */
	int version;		/* processor version */
	int width;		/* width (32 || 64) */
	int ncpus;		/* 1 = UP, n = n-way MP */
	int cache_attrib;	/* cache attributes (bit flags)		*/
				/* bit		0/1 meaning   		*/
				/* -------------------------------------*/
				/* 31	 no cache / cache present	*/
				/* 30	 separate I and D / combined    */
	int icache_size;	/* size of L1 instruction cache */
	int dcache_size;	/* size of L1 data cache */
	int icache_asc;		/* L1 instruction cache associativity */
	int dcache_asc;		/* L1 data cache associativity */
	int icache_block;	/* L1 instruction cache block size */
	int dcache_block;	/* L1 data cache block size */
	int icache_line;	/* L1 instruction cache line size */
	int dcache_line;	/* L1 data cache line size */
	int L2_cache_size;	/* size of L2 cache, 0 = No L2 cache */
	int L2_cache_asc;	/* L2 cache associativity */
	int tlb_attrib;		/* TLB attributes (bit flags)		*/
				/* bit		0/1 meaning   		*/
				/* -------------------------------------*/
				/* 31	 no TLB / TLB present		*/
				/* 30	 separate I and D / combined    */
	int itlb_size;		/* entries in instruction TLB */
	int dtlb_size;		/* entries in data TLB */
	int itlb_asc;		/* instruction tlb associativity */
	int dtlb_asc;		/* data tlb associativity */
	int resv_size;		/* size of reservation */
	int priv_lck_cnt;	/* spin lock count in supevisor mode */
	int prob_lck_cnt;	/* spin lock count in problem state */
	int rtc_type;		/* RTC type */
	int virt_alias;		/* 1 if hardware aliasing is supported */
	int cach_cong;		/* number of page bits for cache synonym */
	int model_arch;		/* used by system for model determination */
	int model_impl;		/* used by system for model determination */
	int Xint;		/* used by system for time base conversion */
	int Xfrac;		/* used by system for time base conversion */
};

#define NTBL	100
char buffer[NTBL * sizeof (struct tbl_malloct)];

char kmemnames[M_LAST][KMEMNAMSZ] = INITKMEMNAMES;

#undef	MSGSTR
#define MSGSTR(Num,Str) Str

#define	NSTAT_NOSYMB	0
#define	NSTAT_BDREAD	0
#define	NSTAT_MINUSE	0
#define	NSTAT_FREECNT	0
#define	NSTAT_MBUSE	0
#define	NSTAT_NAGREE	0
#define	NSTAT_MBALL	0
#define	NSTAT_MBTYPE	0
#define	NSTAT_CLUST	0
#define	NSTAT_KBYTES	0
#define	NSTAT_BDELAY	0
#define	NSTAT_CLUSTIN	0
#define	NSTAT_KBALLOC	0
#define	NSTAT_REQDEN	0
#define	NSTAT_PROTDR	0
#define	NSTAT_MSTATS	0
#define	NSTAT_BYSIZE	0
#define	NSTAT_INUSE	0
#define	NSTAT_CALLS	0
#define	NSTAT_FAILED	0
#define	NSTAT_FREE	0
#define	NSTAT_HIWAT	0
#define	NSTAT_FREED	0
#define	NSTAT_BYTYPE	0
#define	NSTAT_INUSE	0
#define	NSTAT_CALLS	0
#define	NSTAT_FAILED	0
#define	NSTAT_MEMUSE	0
#define	NSTAT_MEMMAX	0
#define	NSTAT_MEMLIM	0
#define	NSTAT_LIMB	0
#define	NSTAT_MAPB	0
#define	NSTAT_BUFINUSE	0
#define	NSTAT_ALLCNTS	0

#define	YES	1
typedef int bool;

struct	mbstat mbstat;
extern	int kmem;
extern	int Zflag;

#define	zeroit(base, field, sname) {	\
	(void) lseek(kmem, (caddr_t)base+offsetof(struct sname, field), 0); \
	sname.field = 0; \
	(void) write(kmem, &sname.field, sizeof(sname.field)); \
}


/*
 * Print mbuf statistics.
 */
mbpr(mbaddr, buckaddr, stataddr, cfgaddr)
	off_t mbaddr, buckaddr, stataddr, cfgaddr;
{
	register int i;
	int size;
	struct tbl_malloct *mt;
	int mbufs, clusters;
	struct tbl_kmemstats tbl_kmemstats;
	struct tbl_mallocs tbl_mallocs;
	caddr_t	addr;
	int j;
	struct _system_configuration _system_configuration;

	kbuckaddr = buckaddr;
	kstataddr = stataddr;

	if (mbaddr == 0) {
		printf(MSGSTR(NSTAT_NOSYMB, "mbstat: symbol not in namelist\n"));
		goto out;
	}

	/*
	 * Read in the _system_configuration struct (see sys/systemcfg.h).
	 * This allows us to determine the number of CPUs.
	 */
	if (!kvm_read(cfgaddr, (char *)&_system_configuration, 
		sizeof (struct _system_configuration))) {
		ncpus = 1;
	} else
		ncpus = _system_configuration.ncpus;

	if (Zflag) {
		bzero(&mbstat, sizeof(mbstat));
		(void) lseek(kmem, mbaddr, 0);
		if (write(kmem, &mbstat, sizeof(mbstat)) != sizeof(mbstat))
			perror("write");

		addr = (caddr_t)kstataddr;
		for (i = 0; i < M_LAST; i++) {
			zeroit(addr, ks_calls, tbl_kmemstats);
			zeroit(addr, ks_limblocks, tbl_kmemstats);
			zeroit(addr, ks_mapblocks, tbl_kmemstats);
			zeroit(addr, ks_failed, tbl_kmemstats);
			addr += sizeof(struct tbl_kmemstats);
		}
		addr = (caddr_t)kbuckaddr;
		for (j = 0; j < ncpus; j++)
			for (i = 0; i < MINBUCKET+16; i++) {
				zeroit(addr, kb_calls, tbl_mallocs);
				zeroit(addr, kb_failed, tbl_mallocs);
				zeroit(addr, kb_couldfree, tbl_mallocs);
				addr += sizeof(struct tbl_mallocs);
			}
		return;
	}
	if (!kvm_read(mbaddr, (char *)&mbstat, sizeof (mbstat))) {
		printf(MSGSTR(BAD_READ, "mbstat: bad read\n")); /*MSG*/
		goto out;
	}

	size = table(TBL_MALLOCT, 0, buffer, NTBL, sizeof (struct tbl_malloct));
	if (size < 0)
		/*perror("table TBL_MALLOCT")*/;
	else {
		mt = (struct tbl_malloct *)buffer;

	}
	clusters = mt[M_CLUSTER].ks_inuse;
	mbufs = mt[M_MBUF].ks_inuse;
	printf(MSGSTR(NSTAT_MBUSE, "%u mbufs in use:\n"), mbufs);
	printf(MSGSTR(NSTAT_CLUSTIN, "%u mbuf cluster pages in use\n"),
	    clusters);
	printf(MSGSTR(NSTAT_KBALLOC, "%u Kbytes allocated to mbufs\n"),
		((clusters * MCLBYTES) + (mbufs * MSIZE))/1024);
	printf(MSGSTR(NSTAT_REQDEN, "%u requests for mbufs denied\n"),
	    mbstat.m_drops);
	printf(MSGSTR(NSTAT_PROTDR, "%u calls to protocol drain routines\n"),
	    mbstat.m_drain);
out:
	/*
	 * OSF/1.1 streams and sockets allocate everything from the
	 * kernel mallocator. Not all the following stats are network-
	 * related, but all are printed for now. This function may
	 * migrate later.
	 */
	kmemstats();
}


static void
banner(void) {
	static int didit = 0;
	printf("\n");
	if (didit++ == 0)
		printf(MSGSTR(NSTAT_MSTATS, "Kernel malloc statistics:\n"));
}

/*
int
main(int argc, char *argv[])
*/
kmemstats(void)
{
	int i, size, inuse = 0, calls = 0;
	struct tbl_malloct *mt;
	struct tbl_mallocs *ms;
	int j;

	for (j=0; j<ncpus; j++) {
		size = table(TBL_MALLOCS, 0, buffer, NTBL, 
			sizeof (struct tbl_mallocs));
		if (size < 0)
			/*perror("table TBL_MALLOCS")*/;
		else {
			int header=0;
			ms = (struct tbl_mallocs *)buffer;
			for (i = 0; i < size; i++) {
				if (ms->kb_total || ms->kb_calls ||
				    ms->kb_totalfree || ms->kb_couldfree ||
				    ms->kb_failed) {
					if (!header++) {
						banner();
						printf("\n******* CPU %d *******\n", j);
						printf("%-*s %6s %9s %6s %7s %7s %7s\n",
						sizeof mt->ks_name, 
						MSGSTR(NSTAT_BYSIZE, "By size"),
						MSGSTR(NSTAT_INUSE, "inuse"), 
						MSGSTR(NSTAT_CALLS, "calls"),
						MSGSTR(NSTAT_FAILED, "failed"), 
						MSGSTR(NSTAT_FREE, "free"),
						MSGSTR(NSTAT_HIWAT, "hiwat"), 
						MSGSTR(NSTAT_FREED, "freed"));
					}
					printf( "%-*d %6ld %9ld %6ld %7ld %7ld %7ld\n",
						sizeof mt->ks_name, 1 << i,
						ms->kb_total - ms->kb_totalfree,
						ms->kb_calls, ms->kb_failed,
						ms->kb_totalfree,
						ms->kb_highwat, ms->kb_couldfree);
				}
				inuse += ms->kb_total - ms->kb_totalfree;
				calls += ms->kb_calls;
				ms++;
			}
		}
		kbuckaddr += (size * sizeof (struct tbl_mallocs));
	}
	size = table(TBL_MALLOCT, 0, buffer, NTBL, sizeof (struct tbl_malloct));
	if (size < 0)
		/*perror("table TBL_MALLOCT")*/;
	else {
		banner();
		mt = (struct tbl_malloct *)buffer;
		printf( "%-*s %6s %9s %6s %7s %7s %5s\n",
		    sizeof mt->ks_name, MSGSTR(NSTAT_BYTYPE, "By type"),
		    MSGSTR(NSTAT_INUSE, "inuse"), MSGSTR(NSTAT_CALLS, "calls"),
		    MSGSTR(NSTAT_FAILED, "failed"), 
		    MSGSTR(NSTAT_MEMUSE, "memuse"), 
		    MSGSTR(NSTAT_MEMMAX, "memmax"), 
		    MSGSTR(NSTAT_MAPB, "mapb"));
		for (i = 0; i < size; i++) {
			if (mt->ks_inuse || mt->ks_calls || mt->ks_memuse ||
			    mt->ks_maxused || mt->ks_failed ||
			    mt->ks_limblocks || mt->ks_mapblocks)
				printf(
			    "%-*s %6ld %9ld %6ld %7ld %7ld %5ld\n",
					sizeof mt->ks_name, mt->ks_name,
					mt->ks_inuse, mt->ks_calls,
					mt->ks_failed, mt->ks_memuse,
					mt->ks_maxused, mt->ks_mapblocks);
			inuse -= mt->ks_inuse;
			calls -= mt->ks_calls;
			mt++;
		}
		/* Happens too much - harmlessly
		if (inuse)
			printf(MSGSTR(NSTAT_BUFINUSE,
			    "Buffers in use differ by %d\n"), inuse);
		if (calls)
			printf(MSGSTR(NSTAT_ALLCNTS,
			    "Allocator call counts differ by %d\n"), calls);
		*/
	}
}

int
table(int type, int index, caddr_t buffer, int nel, int element_size)
{
	switch(type) {
		case TBL_MALLOCT: 
			{
			int			i;
			struct tbl_malloct	*mt;
			struct tbl_kmemstats 	kmemstats[M_LAST];

			if (kstataddr == 0) {
				printf("kmemstats: symbol not in namelist\n");
				break;
			}
			if (!kvm_read(kstataddr, kmemstats, sizeof(kmemstats))){
				printf("kmemstats: bad read\n");
				break;
			}
			mt = (struct tbl_malloct *) buffer;
			nel = MIN(nel, M_LAST);
			for (i = 0; i < nel; i++) {
				bcopy(kmemnames[i], (caddr_t) mt, KMEMNAMSZ);
				bcopy(&kmemstats[i], (caddr_t) mt + KMEMNAMSZ,
					sizeof(*mt) - KMEMNAMSZ);
				mt++;
			}
			return(nel);
			}
		case TBL_MALLOCS:
			if (kbuckaddr == 0) {
				printf("bucket: symbol not in namelist\n");
				break;
			}
			if (!kvm_read(kbuckaddr, buffer, nel * element_size)) {
				printf("bucket: bad read\n");
				break;
			}
			return(MINBUCKET+16);
			break;
		default:
			break;
	}
	return(-1);
}
