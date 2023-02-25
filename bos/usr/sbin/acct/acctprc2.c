static char sccsid[] = "@(#)16  1.16  src/bos/usr/sbin/acct/acctprc2.c, cmdacct, bos41J, 9510A_all 2/28/95 13:49:13";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: getuser, ucmp
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	acctprc2 <ptmp1 >ptacct
 *	reads std. input (in ptmp.h/ascii format)
 *	hashes items with identical uid/name together, sums times
 *	sorts in uid/name order, writes tacct.h records to output
 */

#include <sys/types.h>
#include <sys/limits.h>
#include "acctdef.h"
#include <stdio.h>
#include <locale.h>
#include "ptmp.h"
#include "tacct.h"
#include "table.h"

#include "acct_msg.h"
nl_catd catd;
#define	MSGSTR(N,X)	catgets(catd, MS_ACCT,N,X)

struct	ptmp	pb;
struct	tacct	tb;

struct	utab	{
	ushort  ut_chain;       /* chain for hash table */
	uid_t	ut_uid;
	char	ut_name[NSZ];
	float	ut_cpu[2];	/* cpu time (mins) */
	float	ut_kcore[2];	/* kcore-mins */
	float   ut_io[2];       /* chars/1024 */
	float   ut_rw[2];       /* blocks */
	long	ut_pc;		/* # processes */
} *ub, *getuser(), clrtab;

struct table utable = INITTABLE(ub, A_USIZE);
ushort uused = UHASH;

int	ucmp();
char    *extend();
unsigned hashuser();

char *prog;

main(int argc, char **argv)
{       register int i;

	setlocale(LC_ALL,"");

    catd = catopen(MF_ACCT, NL_CAT_LOCALE);
	prog = argv[0];

	if (extend(&utable) == NULL)
		nomem();

	/* Clear hash table portion */
	for (i = UHASH; --i >= 0; ub[i] = clrtab) ;

	while (scanf("%lu\t%9s\t%f\t%f\t%lu\t%lu\t%u",
		&pb.pt_uid,
		pb.pt_name, /* May extend into pt_cpu, but OK */
		&pb.pt_cpu[0], &pb.pt_cpu[1],
		&pb.pt_io, &pb.pt_rw,
		&pb.pt_mem) != EOF)
			enter(&pb);
	squeeze();
	qsort(ub, uused, sizeof(struct utab), ucmp);
	output();
	exit(0);
}

enter(p)
register struct ptmp *p;
{
	register struct utab *utp;
	double memk;
	double prfrac;
	ulong prime, tot;

	/*
	* Determine the prime/non-prime split for this command.  acctprc1
	* will output a bogus CPU time to indicate that a zero CPU time
	* command has been run in either prime or non-prime time.
	*/

	prfrac = p->pt_cpu[1] ?
		((double) p->pt_cpu[0]) / (p->pt_cpu[0] + p->pt_cpu[1]) : 1.0;

	if (p->pt_cpu[0] + p->pt_cpu[1] < (1.0/HZ))
		p->pt_cpu[0] = p->pt_cpu[1] = 0.0;
  
	utp = getuser(p->pt_uid, p->pt_name);
	utp->ut_cpu[0] += MINS(p->pt_cpu[0]);   /* A15618 */
	utp->ut_cpu[1] += MINS(p->pt_cpu[1]);   /* A15618 */
	memk = KCORE(pb.pt_mem);
	utp->ut_kcore[0] += memk * MINS(p->pt_cpu[0]);  /* A15618 */
	utp->ut_kcore[1] += memk * MINS(p->pt_cpu[1]);  /* A15618 */
	/* tot = (p->pt_io + 255) >> 9;*/    /* I/O rounded to 512-blocks */
	tot = (p->pt_io + 511) >> 10;    /* I/O rounded to 1024-blocks */
	utp->ut_io[0] += (prime = prfrac * tot);
	utp->ut_io[1] += tot - prime;
	utp->ut_rw[0] += (prime = prfrac * p->pt_rw);
	utp->ut_rw[1] += p->pt_rw - prime;
	utp->ut_pc++;
}

squeeze()		/*eliminate holes in hash table*/
{
	register struct utab *p, *q, *lim;

	lim = &ub[uused];
	for (p = q = ub; p <= lim; p++)
		if (p->ut_name[0]) {
			*q = *p;
			++q;
		}
	uused = q - ub;
}

ucmp(p1, p2)
register struct utab *p1, *p2;
{
	if (p1->ut_uid != p2->ut_uid) {
		if ((p1->ut_uid <= INT_MAX && p2->ut_uid  <= INT_MAX) ||
	   		(p1->ut_uid > INT_MAX && p2->ut_uid  > INT_MAX))
			return(p1->ut_uid - p2->ut_uid);
		if (p1->ut_uid <= INT_MAX && p2->ut_uid > INT_MAX) 
			return(-1);
		if (p1->ut_uid > INT_MAX && p2->ut_uid <= INT_MAX) 
			return(1);
	}
	else
		return(strncmp(p1->ut_name, p2->ut_name, NSZ));
}

output()
{
	register i;
	register struct utab *p, *lim;

	for (p = ub, lim = &ub[uused]; p < lim; p++) {
		tb.ta_uid = p->ut_uid;
		CPYN(tb.ta_name, p->ut_name);
		tb.ta_cpu[0] = p->ut_cpu[0];
		tb.ta_cpu[1] = p->ut_cpu[1];
		tb.ta_kcore[0] = p->ut_kcore[0];
		tb.ta_kcore[1] = p->ut_kcore[1];
		tb.ta_io[0] = p->ut_io[0];
		tb.ta_io[1] = p->ut_io[1];
		tb.ta_rw[0] = p->ut_rw[0];
		tb.ta_rw[1] = p->ut_rw[1];
		tb.ta_pc = p->ut_pc;
		fwrite((void*)&tb,(size_t) sizeof(tb),(size_t) 1, stdout);
	}
}

struct utab *
getuser(uid, name)
uid_t uid;
char *name;
{       register ushort u, uh;
	register struct utab *up;

	u = uh = hashuser(uid, name);
	do {
	    up = &ub[u];
	    if (up->ut_uid == uid && EQN(up->ut_name,name))
		    return(up);
	} while (u = up->ut_chain);

	/* If already an entry in this slot (first hash), get another slot. */
	if (up->ut_name[0]) {
		if ((uused += 1) >= utable.tb_nel) {
			utable.tb_nel += utable.tb_nel/3;
			if (extend(&utable) == NULL)
				nomem();
		}
		up = &ub[u = uused];
		*up = clrtab;   /* Clear entry */
	} else
		u = uh;

	up->ut_uid = uid;
	CPYN(up->ut_name, name);
	if (u != uh) {  /* If not first entry, link on at head of chain */
		up->ut_chain = ub[uh].ut_chain;
		ub[uh].ut_chain = uused;
	}
	return(up);
}

nomem()
{       eprintf(MSGSTR( NOMEM, "%s: Out of memory\n"), prog);
	exit(1);
}
