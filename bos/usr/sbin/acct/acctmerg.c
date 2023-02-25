static char sccsid[] = "@(#)12  1.15  src/bos/usr/sbin/acct/acctmerg.c, cmdacct, bos41B, 9505A 1/23/95 07:07:39";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: getlast, getnext, parse, prtacct, sumcurr, tacctadd
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
 *      acctmerg [-a] [-v] [-p] [-h] [-i] [-t] [-u] [file...]
 *	-i	input is in tacct.h/ascii (instead of tacct.h)
 *	-a	output in tacct.h/ascii (instead of tacct.h)
 *	-v	output in verbose tacct.h/ascii
 *	-p	print input files with no processing
 *                  -i, -a, -v, -p may be followed by a fieldspec
 *                             e.g., -a1-3,5,7,14-13
 *      -h      (with -a, -v, -p only) print column headings
 *	-t	output single record that totals all input
 *	-u	summarize by uid, rather than uid/name
 *	reads std input and 0-NFILE files, all in tacct.h format,
 *	sorted by uid/name.
 *	merge/adds all records with same uid/name (or same uid if -u,
 *      or all records if -t), writes to std. output
 *	(still in tacct.h format)
 *	note that this can be used to summarize the std input
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <locale.h>
#include "tacct.h"
#include "accrec.h"

#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

#define CSIZE 1000
#define NFILE	10
int	nfile;			/* index of last used in fl */
FILE	*fl[NFILE]	= {stdin};

struct	tacct tb[NFILE];	/* current record from each file */
struct	tacct	tt = {
	0,
	"TOTAL"
};

char    asciiout;
char    asciiinp;
char    printonly;
char    totalonly;
char    uidsum;
char    verbose;
char    headings;
struct tacct	*getleast();

#define NONUID  65535   /* Nonexistent UID */
#define NFIELDS 18      /* Number of printable fields */

char ifields[NFIELDS+1];  /* ASCII input fields, with 0 terminator */

/* Allow space to permit printing fields (e.g., logname) more than once */
char ofields[2*NFIELDS];

char *prog;

main(int argc, char **argv)
{
	register i;
	register struct tacct *tp;
	static char badspec[] = "%s: bad fieldspec `%s'\n";
	static char onespec[] = "%s: only one input fieldspec allowed\n";

	setlocale(LC_ALL,"");

	catd = catopen(MF_ACCT, NL_CAT_LOCALE);

	prog = *argv;

	while (--argc > 0) {
		if (**++argv == '-')
			switch (*++*argv) {
			case 'q':
				print_file(*++argv);
				exit(0);				
			case 'i':
				if (asciiinp++) {
				    eprintf(MSGSTR( ONESPEC, onespec), prog);
				    exit(1);
				}
				if (*++*argv) {
				    if (parse(*argv,ifields,NFIELDS)==0) {
					eprintf(MSGSTR( BADSPEC, badspec), prog, *argv);
					exit(1);
				    }
				}
				else for (i = NFIELDS; i > 0; --i)
				    ifields[i-1] = i;
				continue;
			case 't':
				totalonly++;
				continue;
			case 'u':
				uidsum++;
				continue;
			case 'p':
				printonly++;
				goto fieldspec;
			case 'v':
				verbose++;
				asciiout++;
				goto fieldspec;
			case 'h':
				headings++;     /* implies asciiout */
			case 'a':
				asciiout++;
			fieldspec:
				if (*++*argv) { /* -[avph]fieldspec */
				  if (parse(*argv,ofields,sizeof ofields)==0){
					eprintf(MSGSTR( BADSPEC, badspec), prog, *argv);
					exit(1);
				    }
				}
				continue;
			}
		else {
			if (++nfile >= NFILE) {
				eprintf(MSGSTR( TOOMANYFILES, "%s: >%d files\n"), prog, NFILE);
				exit(1);
			}
			if ((fl[nfile] = fopen(*argv, "r")) == NULL) {
				eprintf(MSGSTR( CANTOPEN, "%s: can't open %s\n"), prog, *argv);
				exit(1);
			}
		}
	}

	/* If no output field specifications, assume all fields */
	if (ofields[0] == 0)
		for (i = NFIELDS; i > 0; --i)
		    ofields[i-1] = i;

	if (headings)
		prtacct(NULL);

	if (printonly) {
		for (i = 0; i <= nfile; i++)
			while (getnext(0))
				prtacct(&tb[0]);
		exit(0);
	}

	for (i = 0; i <= nfile; i++)
		getnext(i);

	while ((tp = getleast()) != NULL)	/* get least uid of all files, */
		sumcurr(tp);			/* sum all entries for that uid, */
	if (totalonly)				/* and write the 'summed' record */
		output(&tt);
	exit(0);
}

/*
 *	getleast returns ptr to least (lowest uid)  element of current 
 *	avail, NULL if none left; always returns 1st of equals
 */
struct tacct *getleast()
{
	register struct tacct *tp, *least;
	register int i;

	least = NULL;
	for (tp = tb, i = 0; i <= nfile; tp++, i++) {
		if (fl[i] == NULL)
			continue;
		if (least == NULL ||
			tp->ta_uid < least->ta_uid ||
			((tp->ta_uid == least->ta_uid) &&
			!uidsum &&
			(strncmp(tp->ta_name, least->ta_name, NSZ) < 0)))
			least = tp;
	}
	return(least);
}

/*
 *	sumcurr sums all entries with same uid/name (into tp->tacct record)
 *	writes it out, gets new entry
 */
sumcurr(tp)
register struct tacct *tp;
{
	register int i = tp - &tb[0];
	struct tacct tc;

	tc = *tp;
	tacctadd(&tt, tp);	/* gets total of all uids */
	getnext(i);             /* get next one in same file */
	while (i <= nfile)
		if (fl[i] != NULL &&
			tp->ta_uid == tc.ta_uid &&
			(uidsum || EQN(tp->ta_name, tc.ta_name))) {
			tacctadd(&tc, tp);
			tacctadd(&tt, tp);
			getnext(i);
		} else {
			tp++;	/* look at next file */
			i++;
		}
	if (!totalonly)
		output(&tc);
}

tacctadd(t1, t2)
register struct tacct *t1, *t2;
{
	t1->ta_cpu[0] += t2->ta_cpu[0];
	t1->ta_cpu[1] += t2->ta_cpu[1];
	t1->ta_kcore[0] += t2->ta_kcore[0];
	t1->ta_kcore[1] += t2->ta_kcore[1];
	t1->ta_con[0] += t2->ta_con[0];
	t1->ta_con[1] += t2->ta_con[1];
	t1->ta_io[0] += t2->ta_io[0];
	t1->ta_io[1] += t2->ta_io[1];
	t1->ta_rw[0] += t2->ta_rw[0];
	t1->ta_rw[1] += t2->ta_rw[1];
	t1->ta_du += t2->ta_du;
	t1->ta_qsys += t2->ta_qsys;
	t1->ta_fee += t2->ta_fee;
	t1->ta_pc += t2->ta_pc;
	t1->ta_sc += t2->ta_sc;
	t1->ta_dc += t2->ta_dc;
}

output(tp)
register struct tacct *tp;
{
	if (asciiout)
		prtacct(tp);
	else
		fwrite((void *)tp,(size_t) sizeof(*tp),(size_t) 1, stdout);
}

/*
 *	getnext reads next record from stream i, returns 1 if one existed
 */
getnext(i)
int i;
{
	register struct tacct *tp;
	register int j, z;
	register FILE *fp;
	register int ret;
	char tmpname[sizeof(tp->ta_name)+1];
	ushort tmpshort;
	unsigned long tmplong;
	static char ufmt[] = " %hu";
	static char efmt[] = " %e";
	static char lufmt[] = " %lu";

	static struct tacct tproto = {NONUID,
				      '*','N','O','N','A','M','E','*'};

	if ((fp = fl[i]) == NULL)
	    return(0);
	tp = &tb[i];
	if (asciiinp) {
	    *tp = tproto;
	    for (j = 0; ; j++) {
		/* Terminates after ifields[j] == 0 */
		/* or when an expected field isn't there. */
		switch(ifields[j]) {
		    case 0:  
					/*
					while (((z = getc(fp)) != EOF) && (z != '\n'));
					*/
					z = getc(fp);
					while ((z != EOF) && (z != '\n'))
					{
						z = getc(fp);
					}
			     	if (z != EOF) 
						ungetc('\n', fp);
			     	return(1);

		    case 1:  if ((ret = fscanf(fp, lufmt, &tmplong)) == 1)
				 tp->ta_uid = tmplong;
			     break;
		    case 2:  if ((ret = fscanf(fp, " %9s", tmpname)) == 1)
				 CPYN(tp->ta_name,tmpname);
			     break;
		    case 3:  ret = fscanf(fp, efmt, &tp->ta_cpu[0]); break;
		    case 4:  ret = fscanf(fp, efmt, &tp->ta_cpu[1]); break;
		    case 5:  ret = fscanf(fp, efmt, &tp->ta_kcore[0]); break;
		    case 6:  ret = fscanf(fp, efmt, &tp->ta_kcore[1]); break;
		    case 7:  ret = fscanf(fp, efmt, &tp->ta_io[0]); break;
		    case 8:  ret = fscanf(fp, efmt, &tp->ta_io[1]); break;
		    case 9:  ret = fscanf(fp, efmt, &tp->ta_rw[0]); break;
		    case 10: ret = fscanf(fp, efmt, &tp->ta_rw[1]); break;
		    case 11: ret = fscanf(fp, efmt, &tp->ta_con[0]); break;
		    case 12: ret = fscanf(fp, efmt, &tp->ta_con[1]); break;
		    case 13: ret = fscanf(fp, efmt, &tp->ta_du); break;
		    case 14: ret = fscanf(fp, lufmt, &tp->ta_qsys); break;
		    case 15: ret = fscanf(fp, efmt, &tp->ta_fee); break;
		    case 16: ret = fscanf(fp, lufmt, &tp->ta_pc); break;
		    case 17: if ((ret = fscanf(fp, ufmt, &tmpshort)) == 1)
				 tp->ta_sc = tmpshort;
			     break;
		    case 18: if ((ret = fscanf(fp, ufmt, &tmpshort)) == 1)
				 tp->ta_dc = tmpshort;
			     break;
		}
		if (ret != 1) break;
	    }
	} else {
		if (fread((void*)tp,(size_t) sizeof(*tp) ,(size_t) 1, fp) == 1)
			return(1);
	}
	fl[i] = NULL;
	return(0);
}

prtacct(tp)
register struct tacct *tp;
{       register int i;
        register int pass;
	register char *efmt;
	register char *lfmt;
	register char *sfmt;
        register char *bigfmt; /* D41294 */

#define TOOBIG( x ) ( ( x ) > 999999.99 ) ?   /* D41294 D43200 */

	if (tp == NULL) {   /* Print headings */
            efmt = "%-10s";  /* D41294 */
            lfmt = "%-12s";
            sfmt = "%-7s";
        /* D43200 Split Title into two Rows ***/
          for (pass = 1; pass >= 0; pass--)
          {
	    for (i = 0; i < NFIELDS; i++) {
	      switch(ofields[i]) {
                case 0:  break;
                case 1:  printf(lfmt,   pass ? "" : MSGSTR( ACCTUID, "UID")); break; /* D41294 D43200 */
                case 2:  printf("%-9s", pass ? MSGSTR( ACCTLOGNAME2, "LOGIN") : MSGSTR( ACCTLOGNAME3, "NAME"));  break; /* D41294 D43200 */
                case 3:  printf(efmt,   pass ? MSGSTR( ACCT_CPU, "CPU")   : MSGSTR( ACCT_PRIME, "PRIME")); break;
                case 4:  printf(efmt,   pass ? MSGSTR( ACCT_CPU, "CPU")  : MSGSTR( ACCT_NPRIME, "NPRIME")); break;
                case 5:  printf(efmt,   pass ? MSGSTR( ACCT_KCORE, "KCORE") : MSGSTR( ACCT_PRIME, "PRIME")); break;
                case 6:  printf(efmt,   pass ? MSGSTR( ACCT_KCORE, "KCORE"): MSGSTR( ACCT_NPRIME, "NPRIME")); break;
                case 7:  printf(efmt,   pass ? MSGSTR( ACCT_BLKIO, "BLKIO") : MSGSTR( ACCT_PRIME, "PRIME")); break;
                case 8:  printf(efmt,   pass ? MSGSTR( ACCT_BLKIO, "BLKIO"): MSGSTR( ACCT_NPRIME, "NPRIME")); break;
                case 9:  printf(efmt,   pass ? MSGSTR( ACCT_RD_WR, "RD/WR"): MSGSTR( ACCT_PRIME, "PRIME")); break;
                case 10: printf(efmt,   pass ? MSGSTR( ACCT_RD_WR, "RD/WR"): MSGSTR( ACCT_NPRIME, "NPRIME")); break;
                case 11: printf(efmt,   pass ? MSGSTR( ACCT_CONNECT, "CONNECT") : MSGSTR( ACCT_PRIME, "PRIME")); break;
                case 12: printf(efmt,   pass ? MSGSTR( ACCT_CONNECT, "CONNECT"): MSGSTR( ACCT_NPRIME, "NPRIME")); break;
                case 13: printf(efmt,   pass ? MSGSTR( ACCTDSK_BLOCKS2, "DISK") : MSGSTR( ACCTDSK_BLOCKS3, "BLOCKS")); break;
                case 14: printf(lfmt,   pass ? MSGSTR( ACCTPRINT, "PRINT") : ""); break;
                case 15: printf(efmt,   pass ? MSGSTR( ACCTFEES, "FEES") : ""); break;
                case 16: printf(lfmt,   pass ? MSGSTR( ACCT_NUM, "# OF") : MSGSTR( ACCTPROCESSES2, "PROCS")); break;
                case 17: printf(sfmt,   pass ? MSGSTR( ACCT_NUM, "# OF") : MSGSTR( ACCTSESS, "SESS")); break;
                case 18: printf(sfmt,   pass ? MSGSTR( ACCTDSAMPS2, "# DISK") : MSGSTR( ACCTDSAMPS3, "SAMPLES")); break;
              }
            }
            printf("\n");
	    }
	} else {
            efmt = (verbose ? "%-9.3e " : "%-9.0f ");   /* D42194 D43200 */
            bigfmt = "%-9.3e ";
            lfmt = "%-11lu ";
            sfmt = "%-6u ";
	    for (i = 0; i < NFIELDS; i++) {
	      switch(ofields[i]) {
		case 0:  break;
		case 1:  printf(lfmt,   tp->ta_uid); break;
		case 2:  printf("%-9.8s", tp->ta_name); break;
                /* begin D41294 */
                case 3:  printf(TOOBIG( tp->ta_cpu[0]) bigfmt : efmt,   tp->ta_cpu[0]); break;
                case 4:  printf(TOOBIG(tp->ta_cpu[1]) bigfmt : efmt,   tp->ta_cpu[1]); break;
                case 5:  printf(TOOBIG(tp->ta_kcore[0]) bigfmt : efmt,   tp->ta_kcore[0]); break;
                case 6:  printf(TOOBIG(tp->ta_kcore[1]) bigfmt : efmt,   tp->ta_kcore[1]); break;
                case 7:  printf(TOOBIG(tp->ta_io[0]) bigfmt : efmt,   tp->ta_io[0]); break;
                case 8:  printf(TOOBIG(tp->ta_io[1]) bigfmt : efmt,   tp->ta_io[1]); break;
                case 9:  printf(TOOBIG(tp->ta_rw[0]) bigfmt : efmt,   tp->ta_rw[0]); break;
                case 10: printf(TOOBIG(tp->ta_rw[1]) bigfmt : efmt,   tp->ta_rw[1]); break;
                case 11: printf(TOOBIG(tp->ta_con[0]) bigfmt : efmt,   tp->ta_con[0]); break;
                case 12: printf(TOOBIG(tp->ta_con[1]) bigfmt : efmt,   tp->ta_con[1]); break;
                case 13: printf(TOOBIG(tp->ta_du) bigfmt : efmt,   tp->ta_du); break;
                case 14: printf(lfmt, tp->ta_qsys); break;
                case 15: printf(TOOBIG(tp->ta_fee) bigfmt : efmt,   tp->ta_fee); break;
                /* end D41294 */
		case 16: printf(lfmt, tp->ta_pc); break;
		case 17: printf(sfmt, tp->ta_sc); break;
		case 18: printf(sfmt, tp->ta_dc); break;
	      }
	    }
	}
	printf("\n");
}



/* Parse field specifications with a state machine */

/* States */
#define START   (0<<2)
#define NUM1    (1<<2)
#define BETW    (2<<2)
#define NUM2    (3<<2)

/* Character classes */
#define DIGIT   0
#define DASH    1
#define COMMA   2
#define END     3

/* Actions */
#define STNUM   (0<<4)      /* Start number */
#define CNUM    (1<<4)      /* Continue number */
#define ASUM1   (2<<4)      /* Assume 1 for first number */
#define SAVE1   (3<<4)      /* Save as first number */
#define ERR     (4<<4)      /* Too bad */
#define NOP     (5<<4)      /* Nothing to do (check for END)*/
#define FILL1   (6<<4)      /* Fill in one number and check for END */
#define FILLEND (7<<4)      /* Fill through last field and check for END */
#define FILL    (8<<4)      /* Fill range and check for END */

#define ACTION  (~0xF)      /* Mask for action part */
#define STATE   (0xC)       /* Mask for state */
#define INPUT   (0x3)       /* Mask for input character class */

/* Action/Nextstate Table */
static unsigned char nxtact[] =
{             /*  DIGIT           DASH            COMMA           END  */
/* START */     STNUM|NUM1,     ASUM1|BETW,     NOP|START,      NOP,
/* NUM1  */     CNUM|NUM1,      SAVE1|BETW,     FILL1|START,    FILL1,
/* BETW  */     STNUM|NUM2,     NOP|BETW,       FILLEND|START,  FILLEND,
/* NUM2  */     CNUM|NUM2,      ERR,            FILL|START,     FILL,
};

parse(line, flds, fldsize)
register char *line;
char *flds;
{
	register int i;
	register int state;
	register int nmbr, firstnum, secnum, fieldno;

	/* Set up for no fields */
	for (i = fldsize; --i >= 0; flds[i] = 0) ;

	fieldno = 0;

	for (state = START; ; ++line) {
	    /* Form state/char-class combo */
	    state |= ((*line >= '0' && *line <= '9') ? DIGIT :
		      (*line == ','                  ? COMMA :
		      (*line == '-'                  ? DASH  : END)));

	    switch(nxtact[state]&ACTION) {

		case STNUM:     /* Start number */
		    nmbr = *line - '0'; break;
		case CNUM:      /* Continue number */
		    nmbr = 10*nmbr + *line - '0'; break;
		case ASUM1:     /* Assume 1 for first number */
		    firstnum = 1; break;
		case SAVE1:     /* Save as first number */
		    firstnum = nmbr; break;
		case ERR:       /* Too bad */
		    return(0);
		case FILL1:     /* Fill in one number and check for END */
		    firstnum = secnum = nmbr; goto fill;
		case FILLEND:   /* Fill thru last field and check for END */
		    secnum = NFIELDS; goto fill;
		case FILL:      /* Fill range and check for END */
		    secnum = nmbr;
		fill:
		    if (firstnum > NFIELDS || secnum > NFIELDS)
			    return(0);
		    for (i = firstnum; ;i += (i < secnum ? 1 : -1)) {
			if (i > 0 && i <= NFIELDS && fieldno < fldsize)
			    flds[fieldno++] = i;
			else return(0);
			if (i == secnum) break;
		    }
		    /* Fall thru */
		case NOP:       /* (check for END) */
		    if ((state&INPUT) == END)
			return(*line == '\0');
	    }
	    state = nxtact[state]&STATE;
	}
}

#include <pwd.h>
#define NAMELEN 8
struct ent {
	uid_t uid;
	char name[NAMELEN + 1];
	int pages;
	char key[20];
};

struct ent *rec;
struct ent *begin_rec;
int n_rec;
	
print_file(file_name)
char *file_name ;
{
FILE *acctfile;
struct acctrec	a_rec;
struct passwd *p;
struct passwd *getpwnam();
long end_file;
int compkey(struct ent *v1, struct ent *v2);
char buf[20];
	
	if (!file_name)
		acctfile=stdin;
 	else if ((acctfile= fopen(file_name, "r")) == NULL) {
			eprintf(MSGSTR( CANTOPEN, "%s: can't open %s\n"), prog, file_name);
			exit(1);
	}

	if (fseek(acctfile, 0L, 2) != 0)
		perror("fseek");
	if ((end_file=ftell(acctfile)) == -1L)
		perror("ftell");
	rewind(acctfile);
	n_rec=(int) (end_file / sizeof(struct acctrec)) + 1;

	rec=calloc((size_t) n_rec, (size_t)sizeof(struct ent));
	if(rec== NULL) {
		eprintf(MSGSTR(NOMEM, "%s: Cannot allocate memory\n"), prog);
		exit(1);
	}
	begin_rec=rec;

	while (fread((void *)&a_rec, (size_t)sizeof(struct acctrec), (size_t)1, acctfile) == 1) {
		strncpy(rec->name, a_rec.from, NAMELEN+1); 
		if ((p = getpwnam(rec->name)) == (struct passwd *) NULL) 
			rec->uid=ULONG_MAX;
		else 
			rec->uid=p->pw_uid;

		sprintf(buf,"%lu",rec->uid);
		strcat(buf,rec->name);
		strcpy(rec->key,buf);
		rec->pages= a_rec.pages;
		rec++;

	}
		n_rec=rec - begin_rec;
		qsort((void *)begin_rec, n_rec, sizeof(struct ent), (int(*)(void *, void*))compkey); 
		print_rec();
}

compkey(struct ent *v1, struct ent *v2)
{
	return(strcmp(v1->key, v2->key));
}

print_rec()
{
struct ent *i;
	for(i=begin_rec; i<begin_rec + n_rec; i++) {
		printf("%-11lu %-9s %d\n", i->uid, i->name, i->pages);
		free(rec);
	}
}
