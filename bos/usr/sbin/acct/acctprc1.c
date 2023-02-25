static char sccsid[] = "@(#)15  1.18  src/bos/usr/sbin/acct/acctprc1.c, cmdacct, bos411, 9436D411a 8/26/94 15:28:30";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: addurec, getmem, getnamc, getty, readctmp, addsrec
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	acctprc1 [ctmpfile]
 *	reads std. input (acct.h format), adds login names
 *	writes std. output (ptmp.h/ascii format)
 *      if ctmpfile is given, it is expected have ctmp.h/ascii data, sorted
 *      by terminal and time.
 *      it is used to make better guesses at login names
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <pwd.h>
#include <locale.h>
#include <sys/acct.h>
#include "ctmp.h"
#include "ptmp.h"
#include "tacct.h"
#include "table.h"

#include "acct_msg.h"
nl_catd catd;
#define	MSGSTR(N,X)	catgets(catd, MS_ACCT,N,X)

#define MYKIND(flag)	((flag & ACCTF) == 0)

struct	acct	ab;
struct	ctmp	cb;
struct	ptmp	pb;

struct urec {				/* 1 for each distinct uid/name */
	uid_t	ur_uid;			/* sorted by uid/name */
	char	ur_name[NSZ];
	ushort  ur_chain;               /* common hash chain index */
} *ur;                                  /* NULL ur means no (usable) "ctmp" */

struct srec {				/* 1 for each distinct session */
	dev_t	sr_tty;			/* dev, used to connect with process*/
	ushort  sr_urec;                /* index of urec for user */
	time_t	sr_start;		/* start time of session */
} *sr;

struct trec {                           /* 1 for each distinct tty */
	dev_t   tr_tty;                 /* device id */
	ushort  tr_first;               /* index of first srec for tty */
	ushort  tr_last;                /* index of last srec for tty */
	ushort  tr_cur;                 /* current - to speed search */
	ushort  tr_chain;               /* hash chain */
} *tr, *getty();

char *malloc();

char    *getnamc(), *uidtonam();
static  char *prog;     /* program name for error messages */

main(int argc, char **argv)
{
	register char *p;
	long	elaps[2];
	long	etime;		/* Real time, rounded to whole seconds */
	float   expand();
	double  ratio;
	double  stime;		/* CPU time in seconds to nearest 1/AHZ */

	setlocale(LC_ALL,"");
	catd = catopen(MF_ACCT, NL_CAT_LOCALE);

	prog = argv[0];

	if (argc > 1)
		readctmp(argv[1]);

	while (fread((void*)&ab,(size_t) sizeof(ab),(size_t) 1, stdin) == 1) {
		if (!MYKIND(ab.ac_flag))
			continue;
		pb.pt_uid = ab.ac_uid;
		/* Use name from session if available; otherwise, passwd */
		if (ur==NULL ||
		    (p = getnamc(ab.ac_uid, ab.ac_tty, ab.ac_btime))==NULL)
			p = uidtonam(ab.ac_uid);

		if ( !p || !*p )   /* no one seems to know who this was, */
			p = "?";   /* but SOMETHING must be passed       */

		CPYN(pb.pt_name, p);

		/*
		 * Approximate cpu P/NP split same as elapsed time.  If
		 * "real" is 0, use 1 second so pnpsplit says when the
		 * command was run.  But DO NOT use 1 as the amount of
		 * time the process really ran!!!
		 */

		etime = expand(ab.ac_etime);
		stime = expand(ab.ac_stime) + expand(ab.ac_utime);
		pnpsplit(ab.ac_btime, etime > 0 ? etime:1, elaps);

		/*
		 * Make certain the ratio reflects when the command was
		 * run.  This is difficult when the elapsed time is 0, so
		 * take care to use the same dummy time as used in the
		 * call to pnpsplit().
		 */

		ratio = (double) elaps[0] / (etime > 0 ? etime:1);
		pb.pt_cpu[0] = stime * ratio;
		pb.pt_cpu[1] = (stime > pb.pt_cpu[0]) ?
					    stime - pb.pt_cpu[0] : 0;
		pb.pt_io = expand_int(ab.ac_io);
		pb.pt_rw = expand(ab.ac_rw);

		/*
		 * Memory usage is reported in MEAN-SIZE-K.  ACCTPRC2 will
		 * convert from MEAN-SIZE-K to KCORE-MIN.
		 */

		pb.pt_mem = MEM(ab.ac_mem);

		/*
		 * Since acctprc2 uses the CPU P/NP split to determine
		 * I/O P/NP split, use an impossible CPU value as a
		 * flag.  See acctprc2 for the other side of this
		 * handshake.
		 */

		printf("%-11lu%-.8s\t%14.6f\t%14.6f\t%lu\t%lu\t%u\n",
			pb.pt_uid,
			pb.pt_name,
			stime ? pb.pt_cpu[0]:ratio * 0.000001,
			stime ? pb.pt_cpu[1]:(1.0 - ratio) * 0.000001,
			pb.pt_io, pb.pt_rw,
			pb.pt_mem);
	}
}

/*
 *	read ctmp file, build up urec-srec data structures for
 *	later use by getnamc
 */
readctmp(fname)
char *fname;
{
	register FILE *fp;
	register  int splast = 0;
	register int up;
		 long devtemp;
		 int    flag = 0;

	if ((fp = fopen(fname, "r")) == NULL) {
		eprintf(MSGSTR( CANTREAD, "%s: Can't read %s\n"), prog, fname);
		return;
	}

	if (getmem() != 0)
		ur = NULL;
	else
	while (fscanf(fp, "%lu\t%lu\t%9s\t%*lu\t%*lu\t%lu\t%*[^\n]",
		&devtemp,
		&cb.ct_uid,
		cb.ct_name, /* 8 chars wide, but ct_start is next field! */
		&cb.ct_start) != EOF) {

		cb.ct_tty = (dev_t)devtemp;
		flag = 1;   /* Remember that we've been thru here   */

		/* Get user record - create if necessary */
		if ((up = addurec(cb.ct_uid, cb.ct_name)) < 0) {
			ur = NULL;
			break;
		}
		/* Add session record if this is not just a "continuation",
		 * the same user on the same tty at the next login.
		 * This relies on the sort order established above.
		 */
		if (splast <= 0 ||
		    cb.ct_tty != sr[splast].sr_tty ||
		    (ushort) up != sr[splast].sr_urec) {
			if ((splast = addsrec(&cb, (ushort)up)) < 0) {
				ur = NULL;
				break;
			}
		}
	}
	if (ur == NULL)
		eprintf(MSGSTR( OUTOFMEM, "%s: Out of memory -- %s not used\n"), prog, fname);

	if (flag == 0)          /* Treat empty ctmp file as if it wasn't */
		ur = NULL;      /* ... specified at all.                 */

	fclose(fp);
}

/*
 *	using urec-srec data (if any), make best guess at login name
 *	corresponding to uid, return ptr to the name.
 *	must match on tty; use start time to help guess
 *      for any urec having same uid as uid.
 *      give it to person of same uid who last used that terminal
 */
char *
getnamc(uid, tty, start)
register uid_t uid;
dev_t	tty;
time_t	start;
{
	register struct srec *sp, *lim;
	register struct trec *tp;

	if ((tp = getty(tty,0)) == NULL)
		return(NULL);

	/* Search all session records, beginning with current one, for
	 * the last session for uid beginning before the given start time.
	 * Update tr_cur accordingly.  Return pointer to associated name.
	 */
	sp = &sr[tp->tr_cur];

	/* Scan forward in time to beyond process start time */
	for (lim = &sr[tp->tr_last]; ; ++sp, ++tp->tr_cur) {
		if (sp->sr_start > start || sp >= lim)
			break;
	}

	/* Scan backward to position at or before process start time */
	for (lim = &sr[tp->tr_first]; ; --sp, --tp->tr_cur) {
		if (sp->sr_start <= start || sp <= lim)
			break;
	}

	if (sp->sr_start > start)
		return(NULL);

	/* Scan backward for first record with matching uid */
	for ( ; ; --sp, --tp->tr_cur) {
		if (uid == ur[sp->sr_urec].ur_uid)
			return(ur[sp->sr_urec].ur_name);
		if (sp <= lim) /* Can't be less */
			return(NULL);
	}
}

/* Space for all tables is allocated dynamically.  The terminal and user
 * tables are hashed with chaining on collisions.  The session table is
 * linear.  Indexes rather than pointers are used both to save space
 * where pointers are longer than shorts) and to permit relocation.
 */

struct  table urtable = INITTABLE(ur, A_USIZE);
struct  table trtable = INITTABLE(tr, A_TSIZE);
struct  table srtable = INITTABLE(sr, A_SSIZE);

/* Initialize tables and clear hash areas */
getmem()
{       register int i;
	static struct urec u0;
	static struct trec t0;

	if (extend(&urtable) == NULL ||
	    extend(&trtable) == NULL ||
	    extend(&srtable) == NULL)
		return(1);

	for (i = 0; i < UHASH; i++)
		ur[i] = u0;

	for (i = 0; i < THASH; i++)
		tr[i] = t0;

	return(0);
}

/* Return index of user with given uid and name.  If no such user is in
 * table, add him, extending tables as necessary.  Maintain table using
 * hashing of initial size with chained extensions so the extensions can
 * grow contiguously.
 */
addurec(uid, name)
uid_t uid;
char *name;
{       register ushort u, uh;
	register struct urec *up;
	ushort hashuser();
	static uused = UHASH;

	u = uh = hashuser(uid, name);
	do {
	    up = &ur[u];
	    if (up->ur_uid == uid && EQN(up->ur_name,name))
		    return((int) u);
	} while (u = up->ur_chain);

	/* If already an entry in this slot (first hash), get another slot. */
	if (up->ur_name[0]) {
		if ((uused += 1) >= urtable.tb_nel) {
			urtable.tb_nel += urtable.tb_nel/3;
			if (extend(&urtable) == NULL)
				return(-1);
		}
		up = &ur[u = uused];
	} else
		u = uh;
	up->ur_uid = uid;
	CPYN(up->ur_name, name);
	if (u != uh) {  /* If not first entry, link on at head of chain */
		up->ur_chain = ur[uh].ur_chain;
		ur[uh].ur_chain = uused;
	}
	return((int) u);
}

/* Add session record for session given by cp, with user index in hash table
 * of up.  Extend tr (terminal record) and sr (session record) tables as
 * needed.  Return index of record.
 */
addsrec(cp, uindex)
struct ctmp *cp;
ushort uindex;
{       register struct trec *tp;
	register struct srec *sp;
	static int sused = 0;    /* 0 isn't used */

	if ((tp = getty(cp->ct_tty,1)) == NULL)
		return(-1);

	/* Assure that there's space for one more session */
	if ((sused+1) <= sused)         /* Check for wraparound */
		return(-1);
	sused += 1;
	if (sused != (ushort) sused)    /* Make sure it fits in a ushort */
		return(-1);
	if (sused >= srtable.tb_nel) {
		srtable.tb_nel += srtable.tb_nel/3;
		/* Check for overflow */
		if (srtable.tb_nel < sused || extend(&srtable) == NULL)
			return(-1);
	}
	sp = &sr[sused];
	sp->sr_tty = cp->ct_tty;
	sp->sr_urec = uindex;
	sp->sr_start = cp->ct_start;
	if (tp->tr_first == 0)  /* first record for this tty */
	    tp->tr_first = tp->tr_cur = sused;
	return((int) (tp->tr_last = sused));
}

/* Obtain pointer to trec table entry for dev tty.  If record doesn't already
 * exist, create if create!=0 (with tr_first==0), else return NULL.
 */
struct trec *
getty(tty, create)
register dev_t tty;
int create;
{       register struct trec *tp;
	register ushort t, th;
	static tused = THASH;

#define hashtty(x) (((ushort) tty) % THASH)

	t = th = hashtty(tty);
	do {
	    tp = &tr[t];
	    if (tp->tr_tty == tty)
		    return(tp);
	} while (t = tp->tr_chain);

	if (create == 0) return(NULL);

	/* If already an entry in this slot (first hash), get another slot.
	 * An empty slot is one that has tr_first non-zero, since sessions
	 * don't use index 0.
	 */
	if (tp->tr_first) {
		if ((tused += 1) >= trtable.tb_nel) {
			trtable.tb_nel += 32;
			if (extend(&trtable) == NULL)
				return(NULL);
		}
		tp = &tr[t = tused];
	} else
		t = th;
	tp->tr_tty = tty;
	tp->tr_first = 0;
	if (t != th) {  /* If not first entry, link on at head of chain */
		tp->tr_chain = tr[th].tr_chain;
		tr[th].tr_chain = tused;
	}
	return(tp);
}
