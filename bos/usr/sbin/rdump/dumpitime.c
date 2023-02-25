static char sccsid[] = "@(#)87	1.22  src/bos/usr/sbin/rdump/dumpitime.c, cmdarch, bos411, 9428A410j 10/20/93 12:08:21";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "(#)dumpitime.c	5.2 (Berkeley) 5/28/86"; */

#include <nl_types.h>
#include "dumpi_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_DUMP,N,S)
nl_catd catd;

#include <time.h>
#include "dump.h"
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/audit.h>
#include <sys/priv.h>

char *prdate(d)
	time_t d;
{
	char *p;
	static char tmbuf[64];
	struct tm *ltm;

	if(d == 0)
		return(MSGSTR(EPOCH, "the epoch"));

    /* assume that d already has something in it which is valid */
	ltm = localtime(&d);
	strftime(tmbuf,64,"%c",ltm);
    p = tmbuf;
	if  (tmbuf[strlen(tmbuf)-1] == '\n')
	  tmbuf[strlen(tmbuf)-1] = '\0';
	return(p);
}

inititimes()
{
	FILE *df;
	struct flock arg;

	if ((df = fopen(increm, "r")) == NULL) {
		perror(increm);
		return;
	}
	arg.l_type = F_RDLCK;
	arg.l_whence = 0;
	arg.l_start = 0;
	arg.l_len = 0;
	(void) fcntl((int)fileno(df), F_SETLKW, &arg);
	readitimes(df);
	fclose(df);
}

readitimes(df)
	FILE *df;
{
	register	int	i;
	register	struct	itime	*itwalk;

	for (;;) {
		itwalk = (struct itime *)calloc(1, sizeof (struct itime));
		if (getrecord(df, &(itwalk->it_value)) < 0)
			break;
		nidates++;
		itwalk->it_next = ithead;
		ithead = itwalk;
	}

	idates_in = 1;
	/*
	 *	arrayify the list, leaving enough room for the additional
	 *	record that we may have to add to the idate structure
	 */
	idatev = (struct idates **)calloc(nidates + 1,sizeof (struct idates *));
	itwalk = ithead;
	for (i = nidates - 1; i >= 0; i--, itwalk = itwalk->it_next)
		idatev[i] = &itwalk->it_value;
}

getitime()
{
	register	struct	idates	*ip;
	register	int	i;
			char	*fname;

	fname = disk;
#ifdef FDEBUG
	msg("Looking for name %s in increm = %s for delta = %c\n",
		fname, increm, incno);
#endif
	spcl.c_ddate = 0;
	lastincno = '0';

	inititimes();
	/*
	 *	Go find the entry with the same name for a lower increment
	 *	and older date
	 */
	ITITERATE(i, ip) {
		if (strncmp(fname, ip->id_name, sizeof (ip->id_name)) != 0)
			continue;
		if (ip->id_incno >= incno)
			continue;
		if (ip->id_ddate <= spcl.c_ddate)
			continue;
		spcl.c_ddate = ip->id_ddate;
		lastincno = ip->id_incno;
	}
}

putitime()
{
	FILE		*df;
	register	struct	idates	*itwalk;
	register	int	i;
	int		fd;
	char		*fname;
	struct flock arg;

	if(uflag == 0)
		return;
	if ((df = fopen(increm, "r+")) == NULL) {
		/* If 'increm' does not exist */
		/* then try to create it.     */
		if (errno == ENOENT)
			df = fopen(increm, "w+");

		if (df == NULL) {
			perror(increm);
			dumpabort();
		}
	}
	fd = fileno(df);
	arg.l_type = F_WRLCK;
	arg.l_whence = 0;
	arg.l_start = 0;
	arg.l_len = 0;
	(void) fcntl((int)fileno(df), F_SETLKW, &arg);
	fname = disk;
	free((void *)idatev);
	idatev = 0;
	nidates = 0;
	ithead = 0;
	idates_in = 0;
	readitimes(df);
	if (fseek(df,0L,0) < 0) {   /* rewind() was redefined in dumptape.c */
		perror(MSGSTR(FSEEK, "fseek"));
		dumpabort();
	}
	spcl.c_ddate = 0;
	ITITERATE(i, itwalk){
		if (strncmp(fname, itwalk->id_name,
				sizeof (itwalk->id_name)) != 0)
			continue;
		if (itwalk->id_incno != incno)
			continue;
		goto found;
	}
	/*
	 *	construct the new upper bound;
	 *	Enough room has been allocated.
	 */
	itwalk = idatev[nidates] =
		(struct idates *)calloc(1, sizeof(struct idates));
	nidates += 1;
  found:
	strncpy(itwalk->id_name, fname, sizeof (itwalk->id_name));
	itwalk->id_incno = incno;
	itwalk->id_ddate = spcl.c_date;

	ITITERATE(i, itwalk){
		recout(df, itwalk);
	}
	if (ftruncate(fd, ftell(df))) {
		perror(MSGSTR(DUMPFTRUNC,"ftruncate"));
		dumpabort();
	}
	(void) fclose(df);
	msg(MSGSTR(DUMPL, "level %c backup on %s\n"), incno, prdate(spcl.c_date));
}

recout(file, what)
	FILE	*file;
	struct	idates	*what;
{
	fprintf(file, DUMPOUTFMT,
		what->id_name,
		what->id_incno,
		ctime(&(what->id_ddate))
	);
}

int	recno;
int getrecord(df, idatep)
	FILE	*df;
	struct	idates	*idatep;
{
	char		buf[BUFSIZ];

	recno = 0;
	if ( (fgets(buf, BUFSIZ, df)) != buf)
		return(-1);
	recno++;
	if (makeidate(idatep, buf) < 0)
		msg(MSGSTR(UNKIF, "Unknown intermediate format in %s, line %d\n"),
			increm, recno);

#ifdef FDEBUG
	msg("getrecord: %s %c %s\n",
		idatep->id_name, idatep->id_incno, prdate(idatep->id_ddate));
#endif
	return(0);
}

time_t	unctime();

int makeidate(ip, buf)
	struct	idates	*ip;
	char	*buf;
{
	char	un_buf[128];

	sscanf(buf, DUMPINFMT, ip->id_name, &ip->id_incno, un_buf);
	ip->id_ddate = unctime(un_buf);
	if (ip->id_ddate < 0)
		return(-1);
	return(0);
}

/*
 * This is an estimation of the number of TP_BSIZE blocks in the file.
 */
est(ip)
	struct dinode *ip;
{
	long s, t;

	/*
	 * ip->di_size is the size of the file in bytes.
	 * ip->di_nblocks is the number of fragments in data blocks
	 * in the file (i.e. indirect blocks are not counted).
	 *
	 * If the file is bigger than the number of fragments indicates,
	 * then the file has holes in it.  In this case we must use the
	 * block count to estimate the number of data blocks used, but
	 * we use the actual size for estimating the number of indirect
	 * dump blocks (t vs. s in the indirect block calculation).
	 */
	esize += 3;
	if (iscompressed(ip))
	{
		/* each fragment is at most PAGESIZE after decompression */
		s = howmany(ip->di_nblocks * PAGESIZE, TP_BSIZE);
	}
	else
	{
		s = howmany(ip->di_nblocks*FragSize, TP_BSIZE);
	}

	t = howmany(ip->di_size, TP_BSIZE);
	if ( s > t )
		s = t;
	if (ip->di_size > PAGESIZE * NDADDR) {
		/* calculate the number of indirect blocks on the dump tape */
		s += howmany(t - NDADDR * PAGESIZE / TP_BSIZE,
			TP_NINDIR);
	}
	esize += s;
}

bmapest(map)
	char *map;
{
	register i, n;

	n = -1;
	for (i = 0; i < msiz; i++)
		if(map[i])
			n = i;
	if(n < 0)
		return;
	n++;
	esize++;
	esize += howmany(n * sizeof map[0], TP_BSIZE);
}
