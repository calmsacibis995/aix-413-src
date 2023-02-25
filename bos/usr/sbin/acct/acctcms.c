static char sccsid[] = "@(#)05  1.21  src/bos/usr/sbin/acct/acctcms.c, cmdacct, bos41J, 9510A_all 2/28/95 13:29:23";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: ccmp, dofile, enter, fixjunk, kcmp, ncmp,outputa, outputc,
 *            pcmadd, pprint, print, prnt, squeeze, tccmp, tcmadd, tdofile,
 *            tenter, tfixjunk, tkcmp, tncmp, totprnt, toutpta, toutptc,
 *            tprint, tsqueeze
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
 *	acctcms [-a [-op] | -t] [-cjns] [file...]
 *	summarize per-process accounting
 *	-a	output in ascii, rather than [pt]cms.h format
 *	-c	sort by total cpu, rather than total kcore-minutes
 *	-j	anything used only once -> ***other
 *	-n	sort by number of processes
 *	-s	any following files already in pcms.h format
 *      -p      output prime time command summary (only with -a)
 *      -o      output non-prime time (offshift) command summary (only
 *		with -a option)
 *	-t	process records in total (old) style (tcms.h) format
 *	file	file in [pt]cms.h (if -s seen already) or acct.h (if not)
 *	expected use:
 *	acctcms /var/adm/pacct? > today; acctcms -s old today >new
 *	mv new old
 *	acctcms -a today; acctcms -a old
 */
#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <sys/acct.h>
#define MYKIND(flag)	((flag & ACCTF) == 0)
#define TOTAL(a)        (a[PRIMETM] + a[NONPRIME])

#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

#define CSIZE 1000
int	csize;

#define CMDSIZE  9

/*  Total cms records format */
struct tcms {
	char	tcm_comm[CMDSIZE];	/* command name */
	long	tcm_pc;		/* number of processes */
	float	tcm_cpu;	/* cpu time(min) */
	float	tcm_real;	/* real time(min) */
	float	tcm_kcore;	/* kcore-minutes */
	ulong	tcm_io;		/* chars transferred */
	ulong	tcm_rw;		/* blocks read */
} ;
struct tcms	*tcm;

/* prime/nonprime CMS record format */
struct pcms {
	char	pcm_comm[CMDSIZE];	/* command name */
	long	pcm_pc[2];	/* number of processes */
	float	pcm_cpu[2];	/* cpu time(min) */
	float	pcm_real[2];	/* real time(min) */
	float	pcm_kcore[2];	/* kcore-minutes */
	float	pcm_io[2];	/* chars transferred */
	float	pcm_rw[2];	/* blocks read */
} ;
struct pcms	*pcm;
#define PRIMETM         0
#define NONPRIME	1
struct	acct	ab;
struct	tcms	tcmtmp	= {"***other"};
struct	pcms	pcmtmp	= {"***other"};
int	aflg = 0,
   	cflg = 0,
   	jflg = 0,
   	nflg = 0,
   	sflg = 0,
   	pflg = 0,
   	oflg = 0,
   	tflg = 0,
	errflg = 0;

int	ccmp(), kcmp(), ncmp();
int	tccmp(), tkcmp(), tncmp();
float   expand();
int	csz = 0;	/* variable for CSIZE multiple storage */

/*  Format specification for ASCII printing */

/* begin D42194
 * fix formats to fit nicely on 132 column (MAX) page and to prevent adjacent
 * fields from running into one another. We use f format until we get TOOBIG
 * and then switch to g format. This way our column-oriented reports will
 * maintain perfect alignment until the field is larger than e+99, a
 * VERY big number (in magnitude, i.e. should normally not be a problem). */

#define TOOBIG( x ) ( x ) > 999999.99 ?

char    *fmtcmd =       "%-9.8s",
        *fmtcnt =       "%10ld ",
        *fmtkcore =     "%9.2f ",
        *fmtcpu =       "%9.2f ",
        *fmtreal =      "%9.2f ",
        *fmtmsz =       "%9.2f ",
        *fmtmcpu =      "%9.2f ",
        *fmthog =       "%9.2f ",
        *fmtcharx =     "%9.2f ",
        *fmtblkx =      "%9.2f ",
        *fmtbig =       "%9.4g ";

main(int argc, char **argv)
{
	int	c;
	int	exit_flag = TRUE;	/* flag for exiting while loop */
	int	strtopt;		/* variable for storing optind value */
	extern	int	optind;
	extern	char	*optarg;

	setlocale(LC_ALL,"");
	catd = catopen(MF_ACCT, NL_CAT_LOCALE);

	while((c = getopt(argc, argv, "acjnopst")) != EOF)
		switch(c) {
		case 'a':
			aflg = tflg ? usage() : 1;
			continue;
		case 'c':
			cflg = 1;
			continue;
		case 'j':
			jflg = 1;
			continue;
		case 'n':
			nflg = 1;
			continue;
		case 's':
			sflg = 1;
			continue;
		case 'p':
			pflg = 1;
			continue;
		case 'o':
			oflg = 1;
			continue;
		case 't':
			tflg = aflg ? usage() : 1;
			continue;
		default:
			usage();
		}

	if((oflg | pflg) && !aflg)
		usage();

	csz = CSIZE;
	strtopt = optind;
	if(tflg) {
		while(1){
			if( (tcm = (struct tcms *)calloc(csz, sizeof(struct tcms))) == NULL) {
				fprintf(stderr, MSGSTR( NOMEM, "%s: Cannot allocate memory\n"), argv[0]);
				exit(5);
			}
			for(; optind < argc; optind++)
				if(tdofile(argv[optind]) == -1)
					exit_flag = FALSE;
			if(!exit_flag){
				optind = strtopt;
				exit_flag = TRUE;
				csz = csz * 2;
				free(tcm);
			} else break;
		}
		if (jflg)
			tfixjunk();
		tsqueeze();
		qsort(tcm, csize, sizeof(tcm[0]), nflg? tncmp: (cflg? tccmp: tkcmp));
		if (aflg)
			toutpta();
		else
			toutptc();
	} else {
		while(1) {
			if( (pcm = (struct pcms *)calloc(csz, sizeof(struct pcms))) == NULL) {
				fprintf(stderr, MSGSTR( NOMEM, "%s: Cannot allocate memory\n"), argv[0]);
				exit(6);
			}
			for(; optind < argc; optind++)
				if(dofile(argv[optind]) == -1)
					exit_flag = FALSE;
			if(!exit_flag){
				optind = strtopt;
				exit_flag = TRUE;
				csz = csz * 2;
				free(pcm);
			} else break;
		}
		if (jflg)
			fixjunk();
		squeeze();
		qsort(pcm, csize, sizeof(pcm[0]), nflg? ncmp: (cflg? ccmp: kcmp));
		if (aflg)
			outputa();
		else
			outputc();
	}

	exit(errflg ? 1 : 0);
}

tdofile(fname)
char *fname;
{
	struct tcms cmt;

	if (freopen(fname, "r", stdin) == NULL) {
		fprintf(stderr, MSGSTR( CANTOPEN, "%s: cannot open %s\n"), "acctcms:", fname);
		errflg = 1;
		return;
	}
	if (sflg){
		while (fread((void *)&cmt, (size_t)sizeof(cmt),(size_t)1, stdin) == 1)
			if(tenter(&cmt)== -1)
				return(-1);
	} else
		while (fread((void *)&ab, (size_t)sizeof(ab), (size_t)1, stdin) == 1) {
			CPYN(cmt.tcm_comm, ab.ac_comm);
			cmt.tcm_comm[CMDSIZE-1]='\0'; /* d80524 */
			cmt.tcm_pc = 1;
			cmt.tcm_cpu = MINS(expand(ab.ac_stime)+expand(ab.ac_utime));
			cmt.tcm_real = MINS(expand(ab.ac_etime));
			cmt.tcm_kcore = MEM(ab.ac_mem) * cmt.tcm_cpu;
			cmt.tcm_io = expand_int(ab.ac_io);
			cmt.tcm_rw = expand(ab.ac_rw);
			if(tenter(&cmt)== -1)
				return(-1);
		}
}

dofile(char *fname)
{
	struct pcms pcmt;
	double	ratio;
	long	elaps[2];
	long	etime;
	double	dtmp;
	ulong	ltmp;

	if (freopen(fname, "r", stdin) == NULL) {
		fprintf(stderr, MSGSTR( CANTOPEN, "%s: cannot open %s\n"), "acctcms:", fname);
		errflg = 1;
		return;
	}
	if (sflg){
		while (fread((void *)&pcmt, (size_t)sizeof(pcmt), (size_t)1, stdin) == 1)
			if(enter(&pcmt)== -1)
				return(-1);
	} else
		while (fread((void *)&ab, (size_t)sizeof(ab), (size_t)1, stdin) == 1) {
			CPYN(pcmt.pcm_comm, ab.ac_comm);
			pcmt.pcm_comm[CMDSIZE-1]='\0'; /* d80524 */
     /*
	 * Approximate P/NP split as same as elapsed time
 	 */
			if((etime = expand(ab.ac_etime)) == 0)
				etime = 1;
			pnpsplit(ab.ac_btime, etime, elaps);
			ratio = (double)elaps[PRIMETM]/(double)etime;
			if(elaps[PRIMETM] > elaps[NONPRIME]) {
				pcmt.pcm_pc[PRIMETM] = 1;
				pcmt.pcm_pc[NONPRIME] = 0;
			} else {
				pcmt.pcm_pc[PRIMETM] = 0;
				pcmt.pcm_pc[NONPRIME] = 1;
			}
			dtmp = MINS(expand(ab.ac_stime)+expand(ab.ac_utime));
			pcmt.pcm_cpu[PRIMETM] = dtmp * ratio;
			pcmt.pcm_cpu[NONPRIME] = (ratio == 1.0) ? 0.0 : (dtmp - pcmt.pcm_cpu[PRIMETM]);
			dtmp = MINS(expand(ab.ac_etime));
			pcmt.pcm_real[PRIMETM] = dtmp * ratio;
			pcmt.pcm_real[NONPRIME] = (ratio == 1.0) ? 0.0 : (dtmp - pcmt.pcm_real[PRIMETM]);
			dtmp = MINS(expand(ab.ac_stime)+expand(ab.ac_utime))*
				MEM(ab.ac_mem);
			pcmt.pcm_kcore[PRIMETM] = dtmp * ratio;
			pcmt.pcm_kcore[NONPRIME] = (ratio == 1.0) ? 0.0 : (dtmp - pcmt.pcm_kcore[PRIMETM]);
			ltmp = expand_int(ab.ac_io);
			pcmt.pcm_io[PRIMETM] = (double)ltmp * ratio;
			pcmt.pcm_io[NONPRIME] = (ratio == 1.0) ? 0.0 : ((double)ltmp - pcmt.pcm_io[PRIMETM]);
			ltmp = expand(ab.ac_rw);
			pcmt.pcm_rw[PRIMETM] = (double)ltmp * ratio;
			pcmt.pcm_rw[NONPRIME] = (ratio == 1.0) ? 0.0 : ((double)ltmp - pcmt.pcm_rw[PRIMETM]);
			if(enter(&pcmt)== -1)
				return(-1);
		}
}

tenter(p)
register struct tcms *p;
{
	register i;
	register j;
	for (i = j = 0; j < sizeof(p->tcm_comm)-1; j++) {
		if (p->tcm_comm[j] && p->tcm_comm[j] <= 037)
			p->tcm_comm[j] = '?';
		i = i*7 + p->tcm_comm[j];	/* hash function */
	}
	if (i < 0)
		i = -i;
	for (i %= csz, j = 0; tcm[i].tcm_comm[0] && j != csz; i = (i+1)%csz, j++)
		if (strncmp(p->tcm_comm, tcm[i].tcm_comm, sizeof(p->tcm_comm)-1) == 0)
			break;
	if(j == csz)
		return(-1);
	if (tcm[i].tcm_comm[0] == 0)
		CPYN(tcm[i].tcm_comm, p->tcm_comm);
	tcmadd(&tcm[i], p);
	return(i);
}
enter(p)
register struct pcms *p;
{
	register i;
	register j;
	for (i = j = 0; j < sizeof(p->pcm_comm)-1; j++) {
		if (p->pcm_comm[j] && p->pcm_comm[j] <= 037)
			p->pcm_comm[j] = '?';
		i = i*7 + p->pcm_comm[j];	/* hash function */
	}
	if (i < 0)
		i = -i;
	for (i %= csz, j = 0; pcm[i].pcm_comm[0] && j != csz; i = (i+1)%csz, j++)
		if (strncmp(p->pcm_comm, pcm[i].pcm_comm, sizeof(p->pcm_comm)-1) == 0)
			break;
	if(j == csz)
		return(-1);
	if (pcm[i].pcm_comm[0] == 0)
		CPYN(pcm[i].pcm_comm, p->pcm_comm);
	pcmadd(&pcm[i], p);
	return(i);
}
tfixjunk()	/* combine commands used only once */
{
	register i, j;
	j = enter(&tcmtmp);
	for (i = 0; i < csz; i++)
		if (i != j && tcm[i].tcm_comm[0] && tcm[i].tcm_pc <= 1) {
			tcmadd(&tcm[j], &tcm[i]);
			tcm[i].tcm_comm[0] = 0;
		}
}
fixjunk()	/* combine commands used only once */
{
	register i, j;
	j = enter(&pcmtmp);
	for (i = 0; i < csz; i++)
		if (i != j && pcm[i].pcm_comm[0] && (pcm[i].pcm_pc[PRIMETM] + pcm[i].pcm_pc[NONPRIME]) <= 1) {
			pcmadd(&pcm[j], &pcm[i]);
			pcm[i].pcm_comm[0] = 0;
		}
}

tcmadd(p1, p2)
register struct tcms *p1, *p2;
{
	p1->tcm_pc += p2->tcm_pc;
	p1->tcm_cpu = p1->tcm_cpu + p2->tcm_cpu;
	p1->tcm_real = p1->tcm_real + p2->tcm_real;
	p1->tcm_kcore = p1->tcm_kcore + p2->tcm_kcore;
	p1->tcm_io += p2->tcm_io;
	p1->tcm_rw += p2->tcm_rw;
}
pcmadd(p1, p2)
register struct pcms *p1, *p2;
{
	p1->pcm_pc[PRIMETM] += p2->pcm_pc[PRIMETM];
	p1->pcm_pc[NONPRIME] += p2->pcm_pc[NONPRIME];
	p1->pcm_cpu[PRIMETM] += p2->pcm_cpu[PRIMETM];
	p1->pcm_cpu[NONPRIME] += p2->pcm_cpu[NONPRIME];
	p1->pcm_real[PRIMETM] += p2->pcm_real[PRIMETM];
	p1->pcm_real[NONPRIME] += p2->pcm_real[NONPRIME];
	p1->pcm_kcore[PRIMETM] += p2->pcm_kcore[PRIMETM];
	p1->pcm_kcore[NONPRIME] += p2->pcm_kcore[NONPRIME];
	p1->pcm_io[PRIMETM] += p2->pcm_io[PRIMETM];
	p1->pcm_io[NONPRIME] += p2->pcm_io[NONPRIME];
	p1->pcm_rw[PRIMETM] += p2->pcm_rw[PRIMETM];
	p1->pcm_rw[NONPRIME] += p2->pcm_rw[NONPRIME];
}

tsqueeze()	/* get rid of holes in hash table */
{
	register i, k;

	for (i = k = 0; i < csz; i++)
		if (tcm[i].tcm_comm[0]) {
			CPYN(tcm[k].tcm_comm, tcm[i].tcm_comm);
			tcm[k].tcm_pc = tcm[i].tcm_pc;
			tcm[k].tcm_cpu = tcm[i].tcm_cpu;
			tcm[k].tcm_real = tcm[i].tcm_real;
			tcm[k].tcm_kcore = tcm[i].tcm_kcore;
			tcm[k].tcm_io = tcm[i].tcm_io;
			tcm[k].tcm_rw = tcm[i].tcm_rw;
			k++;
		}
	csize = k;
}
squeeze()	/* get rid of holes in hash table */
{
	register i, k;

	for (i = k = 0; i < csz; i++)
		if (pcm[i].pcm_comm[0]) {
			CPYN(pcm[k].pcm_comm, pcm[i].pcm_comm);
			pcm[k].pcm_pc[PRIMETM] = pcm[i].pcm_pc[PRIMETM];
			pcm[k].pcm_pc[NONPRIME] = pcm[i].pcm_pc[NONPRIME];
			pcm[k].pcm_cpu[PRIMETM] = pcm[i].pcm_cpu[PRIMETM];
			pcm[k].pcm_cpu[NONPRIME] = pcm[i].pcm_cpu[NONPRIME];
			pcm[k].pcm_real[PRIMETM] = pcm[i].pcm_real[PRIMETM];
			pcm[k].pcm_real[NONPRIME] = pcm[i].pcm_real[NONPRIME];
			pcm[k].pcm_kcore[PRIMETM] = pcm[i].pcm_kcore[PRIMETM];
			pcm[k].pcm_kcore[NONPRIME] = pcm[i].pcm_kcore[NONPRIME];
			pcm[k].pcm_io[PRIMETM] = pcm[i].pcm_io[PRIMETM];
			pcm[k].pcm_io[NONPRIME] = pcm[i].pcm_io[NONPRIME];
			pcm[k].pcm_rw[PRIMETM] = pcm[i].pcm_rw[PRIMETM];
			pcm[k].pcm_rw[NONPRIME] = pcm[i].pcm_rw[NONPRIME];
			k++;
		}
	csize = k;
}

tccmp(p1, p2)
register struct tcms *p1, *p2;
{
	if (p1->tcm_cpu == p2->tcm_cpu)
		return(0);
	return ((p2->tcm_cpu > p1->tcm_cpu)? 1 : -1);
}

ccmp(p1, p2)
register struct pcms *p1, *p2;
{
	register int	index;

	if( (pflg && oflg) || (!pflg && !oflg) ) {
		if (p1->pcm_cpu[PRIMETM] + p1->pcm_cpu[NONPRIME] == p2->pcm_cpu[PRIMETM] + p2->pcm_cpu[NONPRIME])
			return(0);
		return ((p2->pcm_cpu[PRIMETM] + p2->pcm_cpu[NONPRIME] > p1->pcm_cpu[PRIMETM] + p1->pcm_cpu[NONPRIME])? 1 : -1);
	}
	index = pflg ? PRIMETM : NONPRIME;
	if (p1->pcm_cpu[index] == p2->pcm_cpu[index])
		return(0);
	return ((p2->pcm_cpu[index] > p1->pcm_cpu[index])? 1 : -1);
}

tkcmp(p1, p2)
register struct tcms *p1, *p2;
{
	if (p1->tcm_kcore == p2->tcm_kcore)
		return(0);
	return ((p2->tcm_kcore > p1->tcm_kcore)? 1 : -1);
}

kcmp(p1, p2)
register struct pcms *p1, *p2;
{
	register int	index;

	if( (pflg && oflg) || (!pflg && !pflg) ){
		if (p1->pcm_kcore[PRIMETM] + p1->pcm_kcore[NONPRIME] == p2->pcm_kcore[PRIMETM] + p2->pcm_kcore[NONPRIME])
			return(0);
		return ((p2->pcm_kcore[PRIMETM] + p2->pcm_kcore[NONPRIME] > p1->pcm_kcore[PRIMETM] + p1->pcm_kcore[NONPRIME])? 1 : -1);
	}
	index = pflg ? PRIMETM : NONPRIME;
	if (p1->pcm_kcore[index] == p2->pcm_kcore[index])
		return(0);
	return ((p2->pcm_kcore[index] > p1->pcm_kcore[index])? 1 : -1);
}

tncmp(p1, p2)
register struct tcms *p1, *p2;
{
	if (p1->tcm_pc == p2->tcm_pc)
		return(0);
	return ((p2->tcm_pc > p1->tcm_pc)? 1 : -1);
}

ncmp(p1, p2)
register struct pcms *p1, *p2;
{
	register int	index;

	if( (pflg && oflg) || (!pflg && !oflg) ) {
		if (p1->pcm_pc[PRIMETM] + p1->pcm_pc[NONPRIME] == p2->pcm_pc[PRIMETM] + p2->pcm_pc[NONPRIME])
			return(0);
		return ((p2->pcm_pc[PRIMETM] + p2->pcm_pc[NONPRIME] > p1->pcm_pc[PRIMETM] + p1->pcm_pc[NONPRIME])? 1 : -1);
	}
	index =  pflg ? PRIMETM : NONPRIME;
	if (p1->pcm_pc[index] == p2->pcm_pc[index])
		return(0);
	return ((p2->pcm_pc[index] > p1->pcm_pc[index])? 1 : -1);
}

/************************************** D42194 *********************************************************************

                                header strings thd1 & thd2

COMMAND      NUMBER    TOTAL     TOTAL    TOTAL       MEAN     MEAN       HOG       CHARS    BLOCKS
 NAME         CMDS   KCOREMIN   CPU-MIN  REAL-MIN    SIZE-K   CPU-MIN    FACTOR    TRNSFD     READ
11111111b2222222222b333333333b444444444b555555555b666666666b777777777b888888888b999999999b000000000
 "-9.8s"    "10ld "  "9.2f "    "9.2f "   "9.2f "  "9.2f "   "9.2f "   "9.2f"    "9.2f "   "9.2f "

NOTE : This adds up to 99 columns.
********************************************************************************************************************/
char    thd1[] =
"COMMAND      NUMBER    TOTAL     TOTAL    TOTAL       MEAN     MEAN       HOG       CHARS    BLOCKS\n";
char    thd2[] =
" NAME         CMDS   KCOREMIN   CPU-MIN  REAL-MIN    SIZE-K   CPU-MIN    FACTOR    TRNSFD     READ\n";

toutpta()
{
	register i;

	printf( MSGSTR( THEADER1, thd1));
        printf("\n");
	printf( MSGSTR( THEADER2, thd2));
	printf("\n");
	for (i = 0; i < csize; i++)
		tcmadd(&tcmtmp, &tcm[i]);
	CPYN(tcmtmp.tcm_comm, MSGSTR( TOTALS, "TOTALS"));
	tprint(&tcmtmp);
	printf("\n");
	for (i = 0; i < csize; i++)
		tprint(&tcm[i]);
}

tprint(p)
register struct tcms *p;
{
        /* begin D42194 */
        printf("%-9.8s", p->tcm_comm);
        printf("%10ld ", p->tcm_pc);
        printf(TOOBIG(p->tcm_kcore) fmtbig : "%9.2f ", p->tcm_kcore);
        printf(TOOBIG(p->tcm_cpu) fmtbig : "%9.2f ", p->tcm_cpu);
        printf(TOOBIG(p->tcm_real) fmtbig : "%9.2f ", p->tcm_real);
        if(p->tcm_cpu == 0)  p->tcm_cpu = 1;
        printf(TOOBIG(p->tcm_kcore/p->tcm_cpu) fmtbig : "%9.2f ", p->tcm_kcore/p->tcm_cpu);
        if(p->tcm_pc == 0)  p->tcm_pc = 1;
        printf(TOOBIG(p->tcm_cpu/p->tcm_pc) fmtbig : "%9.2f ", p->tcm_cpu/p->tcm_pc);
        if (p->tcm_real == 0)
                p->tcm_real = 1;
        printf(TOOBIG(p->tcm_cpu/p->tcm_real) fmtbig : "%9.2f ", p->tcm_cpu/p->tcm_real);
        printf(TOOBIG(p->tcm_io) fmtbig : "%9.2f ",(double)p->tcm_io);
        printf(TOOBIG(p->tcm_rw) fmtbig : "%9.2f\n",(double)p->tcm_rw);
        /* end D42194 */

}

toutptc()
{
	register i;

	for (i = 0; i < csize; i++)
		fwrite((void *)&tcm[i], (size_t)sizeof(tcm[i]), (size_t)1, stdout);
}

/************************************** D42194 *********************************************************************

                                header strings hd1 & hd2

COMMAND      NUMBER    TOTAL     TOTAL    TOTAL       MEAN     MEAN       HOG       CHARS    BLOCKS
 NAME         CMDS   KCOREMIN   CPU-MIN  REAL-MIN    SIZE-K   CPU-MIN    FACTOR    TRNSFD     READ
11111111b2222222222b333333333b444444444b555555555b666666666b777777777b888888888b999999999b000000000
 "-9.8s"    "10ld "  "9.2f "    "9.2f "   "9.2f "  "9.2f "   "9.2f "   "9.2f"    "9.2f "   "9.2f "

NOTE: This adds up to 99 columns.

                                header strings hd3 & hd4

COMMAND          NUMBER-CMDS      TOTAL          CPU-MIN            REAL-MIN         MEAN     MEAN       HOG       CHARS    BLOCKS
 NAME          (P)       (NP)   KCOREMIN      (P)      (NP)       (P)      (NP)     SIZE-K   CPU-MIN    FACTOR    TRNSFD     READ
11111111b2222222222b3333333333b444444444b555555555b666666666b777777777b888888888b999999999b000000000baaaaaaaaabcccccccccbeeeeeeeee
 "-9.8s"    "10ld "  "10ld "    "9.2f "    "9.2f "  "9.2f "    "9.2f "   "9.2f "    "9.2f "   "9.2f "   "9.2f "  "9.2f "   "9.2f "

NOTE: This adds up to 130 columns.
*******************************************************************************************************************/

char    hd1[] =
"COMMAND      NUMBER    TOTAL     TOTAL    TOTAL       MEAN     MEAN       HOG       CHARS    BLOCKS\n";
char    hd2[] =
" NAME         CMDS   KCOREMIN   CPU-MIN  REAL-MIN    SIZE-K   CPU-MIN    FACTOR    TRNSFD     READ\n";
char    hd3[] =
"COMMAND          NUMBER-CMDS      TOTAL          CPU-MIN            REAL-MIN         MEAN     MEAN       HOG       CHARS    BLOCKS\n";
char    hd4[] =
" NAME          (P)       (NP)   KCOREMIN      (P)      (NP)       (P)      (NP)     SIZE-K   CPU-MIN    FACTOR    TRNSFD     READ\n";
char	hdprime[] =
"                                   PRIME TIME COMMAND SUMMARY\n";
char	hdnonprime[] =
"                                  NON-PRIME TIME COMMAND SUMMARY\n";
char	hdtot[] =
"                                     TOTAL COMMAND SUMMARY\n";
char	hdp[] =
"                                PRIME/NON-PRIME TIME COMMAND SUMMARY\n";

outputa()
{
	register i;

	if( pflg && oflg ) printf( MSGSTR( HDP, hdp));
	else if(pflg) printf( MSGSTR( HDPRIME, hdprime));
	else if(oflg) printf( MSGSTR( HDNONPRIME, hdnonprime));
	else printf( MSGSTR( HDTOT, hdtot));
	if( (!pflg && !oflg) || (pflg ^ oflg)) {
		printf(MSGSTR( HD1, hd1));
		printf( MSGSTR( HD2, hd2));
	}
	else {
		printf( MSGSTR( HD3, hd3));
		printf( MSGSTR( HD3, hd4));
	}
	printf("\n");
	for (i = 0; i < csize; i++)
		pcmadd(&pcmtmp, &pcm[i]);
	CPYN(pcmtmp.pcm_comm, MSGSTR( TOTALS, "TOTALS"));
	print(&pcmtmp);
	printf("\n");
	for (i = 0; i < csize; i++)
		print(&pcm[i]);
}

print(p)
register struct pcms *p;
{
	if(pflg && oflg) pprint(p);
	else if(pflg || oflg) prnt(p, pflg ? PRIMETM : NONPRIME);
	else totprnt(p);
}

prnt(p, hr)
register struct pcms *p;
register int	hr;
{
	float hog_factor,mean_cpu;

        if(p->pcm_pc[hr] == 0) return;
        printf(fmtcmd, p->pcm_comm );
        printf(fmtcnt, p->pcm_pc[hr]);
        /* begin D41294 */
        printf(TOOBIG(p->pcm_kcore[hr]) fmtbig : fmtkcore, p->pcm_kcore[hr]);
        printf(TOOBIG(p->pcm_cpu[hr]) fmtbig : fmtcpu, p->pcm_cpu[hr]);
        printf(TOOBIG(p->pcm_real[hr]) fmtbig : fmtreal, p->pcm_real[hr]);

        if (p->pcm_real[hr] == 0)
           hog_factor = 0;
        else
           hog_factor = ((p->pcm_cpu[hr]/p->pcm_real[hr])*HZ);

        mean_cpu = p->pcm_pc[hr] ? p->pcm_cpu[hr]/p->pcm_pc[hr] : p->pcm_cpu[hr] ;

        if(p->pcm_cpu[hr] == 0)  p->pcm_cpu[hr] = 1;
        printf(TOOBIG(p->pcm_kcore[hr]/p->pcm_cpu[hr]) fmtbig : fmtmsz, p->pcm_kcore[hr]/p->pcm_cpu[hr]);
        printf(TOOBIG(mean_cpu) fmtbig : fmtmcpu, mean_cpu);
        printf(TOOBIG(hog_factor) fmtbig : fmthog, hog_factor);
        printf(TOOBIG(p->pcm_io[hr]) fmtbig : fmtcharx,p->pcm_io[hr]);
        printf(TOOBIG(p->pcm_rw[hr]) fmtbig : fmtblkx,p->pcm_rw[hr]);
        /* end D41294 */
        printf("\n");
}

pprint(p)
register struct pcms *p;
{
	float hog_factor,mean_cpu;

        printf(fmtcmd, p->pcm_comm);
        printf(fmtcnt, p->pcm_pc[PRIMETM]);
        printf(fmtcnt, p->pcm_pc[NONPRIME]);
        /* begin D41294 */
        printf(TOOBIG(TOTAL(p->pcm_kcore)) fmtbig : fmtkcore, TOTAL(p->pcm_kcore));
        printf(TOOBIG(p->pcm_cpu[PRIMETM]) fmtbig : fmtcpu, p->pcm_cpu[PRIMETM]);
        printf(TOOBIG(p->pcm_cpu[NONPRIME]) fmtbig : fmtcpu, p->pcm_cpu[NONPRIME]);
        printf(TOOBIG(p->pcm_real[PRIMETM]) fmtbig : fmtreal, p->pcm_real[PRIMETM]);
        printf(TOOBIG(p->pcm_real[NONPRIME]) fmtbig : fmtreal, p->pcm_real[NONPRIME]);

        if (TOTAL(p->pcm_real) == 0)
           hog_factor = 0.0;
        else
           hog_factor = (TOTAL(p->pcm_cpu)/TOTAL(p->pcm_real))*HZ;

        mean_cpu = TOTAL(p->pcm_pc) ? TOTAL(p->pcm_cpu)/TOTAL(p->pcm_pc) : TOTAL(p->pcm_cpu);

        if(TOTAL(p->pcm_cpu) == 0)  p->pcm_cpu[PRIMETM] = 1;
        printf(TOOBIG(TOTAL(p->pcm_kcore)/TOTAL(p->pcm_cpu)) fmtbig : fmtmsz, TOTAL(p->pcm_kcore)/TOTAL(p->pcm_cpu));
        printf(TOOBIG(mean_cpu) fmtbig : fmtmcpu, mean_cpu);
        printf(TOOBIG(hog_factor) fmtbig : fmthog, hog_factor);
        printf(TOOBIG(TOTAL(p->pcm_io)) fmtbig : fmtcharx,TOTAL(p->pcm_io));
        printf(TOOBIG(TOTAL(p->pcm_rw)) fmtbig : fmtblkx, TOTAL(p->pcm_rw));
        /* end D41294 */
        printf("\n");

}

totprnt(p)
register struct pcms *p;
{
	float hog_factor,mean_cpu;

        printf(fmtcmd, p->pcm_comm);
        /* begin D41294 */
        printf(fmtcnt, TOTAL(p->pcm_pc));
        printf(TOOBIG(TOTAL(p->pcm_kcore)) fmtbig : fmtkcore, TOTAL(p->pcm_kcore));
        printf(TOOBIG((TOTAL(p->pcm_cpu))) fmtbig : fmtcpu, (TOTAL(p->pcm_cpu)));
        printf(TOOBIG(TOTAL(p->pcm_real)) fmtbig : fmtreal, TOTAL(p->pcm_real));

        if (TOTAL(p->pcm_real) == 0)
                hog_factor = 0.0;
        else
                hog_factor = (TOTAL(p->pcm_cpu)/TOTAL(p->pcm_real))*HZ;

        mean_cpu = TOTAL(p->pcm_pc) ? TOTAL(p->pcm_cpu)/TOTAL(p->pcm_pc) : TOTAL(p->pcm_cpu);

        if(TOTAL(p->pcm_cpu) == 0)  p->pcm_cpu[PRIMETM] = 1;
        printf(TOOBIG(TOTAL(p->pcm_kcore)/TOTAL(p->pcm_cpu)) fmtbig : fmtmsz, TOTAL(p->pcm_kcore)/TOTAL(p->pcm_cpu));
        printf(TOOBIG(mean_cpu) fmtbig : fmtmcpu, mean_cpu);
        printf(TOOBIG(hog_factor) fmtbig : fmthog, hog_factor);
        printf(TOOBIG(TOTAL(p->pcm_io)) fmtbig : fmtcharx,TOTAL(p->pcm_io));
        printf(TOOBIG(TOTAL(p->pcm_rw)) fmtbig : fmtblkx,TOTAL(p->pcm_rw));
        /* end D41294 */
        printf("\n");

}
outputc()
{
	register i;

	for (i = 0; i < csize; i++)
		fwrite((void *)&pcm[i], (size_t)sizeof(pcm[i]), (size_t)1, stdout);
}

usage()
{
	fprintf(stderr,MSGSTR(USAGE,"Usage: acctcms [-a [-op] | -t] [-cjns] [file ...]\n"));
	exit(1);
}
