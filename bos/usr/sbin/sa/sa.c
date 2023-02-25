static char sccsid[] = "@(#)88  1.8.2.17  src/bos/usr/sbin/sa/sa.c, cmdstat, bos41B, 9504A 12/21/94 13:42:25";
/*
 * COMPONENT_NAME: (CMDSTAT) statistical commands....
 *
 * FUNCTIONS: sa
 *
 * ORIGINS: 26,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
static char *sccsid = "sa.c	4.9 (Berkeley) 12/12/84";
*/

/*
 *  Extensive modifications to internal data structures
 *  to allow arbitrary number of different commands and users added.
 *	
 *  Also allowed the digit option on the -v flag (interactive
 *  threshold compress) to be a digit string, so one can
 *  set the threshold > 9.
 *
 *  Also added the -f flag, to force no interactive threshold
 *  compression with the -v flag.
 *
 *  Robert Henry
 *  UC Berkeley
 *  31jan81
 */

/**********************************************************************/
/* Include File                                                       */
/**********************************************************************/
#define _ILS_MACROS
#include  <stdio.h>
#include  <ctype.h>
#include  <sys/types.h>
#include  <sys/acct.h>
#include  <signal.h>
#include  <utmp.h>
#include  <pwd.h>
#include  <locale.h>
#include  <sys/param.h>    /* for HZ value (100)  */
#include  <nl_types.h>
#include  "sa_msg.h"

/**********************************************************************/
/* Constant Definition / Macro Function                               */
/**********************************************************************/
#define  MSGSTR(Num,Str)  catgets(catd,MS_SA,Num,Str)  

#define  GAP_SIZE         50000 /* arbitrary number used to display message */
#define  UID_DELTA_GROWTH   250 /* 1 Kbyte, used for dynamic table growth */

/*
 *  The passwd file is scanned by getmaxuid(), loading uidtable
 *  with every entry on the passwd file.
 *
 *  The historical "Preposterous" message has been changed to Unknown uid.
 *  This message will now be displayed
 *      for every user not found on the passwd file, INSTEAD OF
 *      for every user bigger than maxuid
 *
 *  The file /var/adm/usracct (USRACCT) has to be written/read from
 *  uid-zero to maxuid. See another usracct paragraph below.
 *   a) If a userid is too large (max FILESIZE is exceeded), the record
 *      will not be written.  A warning message will be displayed.
 *   b) When reading this file, we will read from uid-zero to maxuid.
 *      GAP_SIZE is used to print warning (status) messages while reading
 *      this LARGE file.
 *   c) NOTE FOR THE FUTURE :
 *      In order to support large-uids, the definition of this file
 *      has to change.  We will become INcompatible with ATT and older
 *      versions of AIX.
 *
 *          Beto ( april / 30 / 91 )
 */
 /*
  *	Summary file format is changed to support large uids. 
  *	In new format uid is part of the summary record. So the file
  *	will not have holes as before corresponding to non-existing uids.
  *	From user point of view, the behaviour of program is
  *
  *	if (using old-format)
  *		if (used new flag '-C' instead of '-s')
  *			write new-format of summary file.
  *		else
  *			write old-format.
  *	else if (using new-format)
  *		write new-format.    // irrespective of -C or -s flag
  *	else if (summary file does not exist)
  *		create new-format.
  *	end.
  *
  *	To make this algorithm work, a magic string has been added as first
  *	few bytes of the new summary file. This distinguishes between old
  *	and new file formats.
  *
  *     Changes are made in the old format processing of summary file.
  *     All uids are checked before opening summary file for writing. This
  *     makes sure that we don't come across a large uid while writing
  *     and loose data for that userid.
  */		

#define  AHZ  64  /* 1/AHZ is the granularity of the data
                   * encoded in the various comp_t fields.
                   * This is not equal to hz....
                   */

#define  NC        (sizeof(acctbuf.ac_comm))
#define  NAMELG    (sizeof(utmp.ut_name)+1)
#define  ALLOCQTY  (sizeof(struct allocbox))

#define  us_cnt    oldu.Us_cnt
#define  us_ctime  oldu.Us_ctime
#define  us_io     oldu.Us_io
#define  us_imem   oldu.Us_imem

/*
 *  Table elements are keyed by the name of the file exec'ed.
 *  Because on large systems, many files can be exec'ed,
 *  a static table size may grow to be too large.
 *
 *  Table elements are allocated in chunks dynamically, linked
 *  together so that they may be retrieved sequentially.
 *
 *  An index into the table structure is provided by hashing through
 *  a seperate hash table.
 *  The hash table is segmented, and dynamically extendable.
 *  Realize that the hash table and accounting information is kept
 *  in different segments!
 *
 *  We have a linked list of hash table segments; within each
 *  segment we use a quadratic rehash that touches no more than 1/2
 *  of the buckets in the hash table when probing.
 *  If the probe does not find the desired symbol, it moves to the
 *  next segment, or allocates a new segment.
 *
 *  Hash table segments are kept on the linked list with the first
 *  segment always first (that will probably contain the
 *  most frequently executed commands) and
 *  the last added segment immediately after the first segment,
 *  to hopefully gain something by locality of reference.
 *
 *  We store the per user information in the same structure as
 *  the per exec'ed file information.  This allows us to use the
 *  same managers for both, as the number of user id's may be very
 *  large.
 *  User information is keyed by the first character in the name
 *  being a '\001', followed by four bytes of (long extended)
 *  user id number, followed by a null byte.
 *  The actual user names are kept in a seperate field of the
 *  user structure, and is filled in upon demand later.
 *  Iteration through all users by low user id to high user id
 *  is done by just probing the table, which is gross.
 */
#define  USERKEY        '\001'
#define  ISPROCESS(tp)  (tp->p.name[0] && (tp->p.name[0] != USERKEY))
#define  ISUSER(tp)     (tp->p.name[0] && (tp->p.name[0] == USERKEY))

#define  TABDALLOP      500

#define  NHASH        1103
#define  HASHCLOGGED  (NHASH / 2)

/*
 *  Iterate through all symbols in the symbol table in declaration
 *  order.
 *  struct allocbox  *allocwalk;
 *  cell             *sp, *ub;
 *
 *  sp points to the desired item, allocwalk and ub are there
 *  to make the iteration go.
 */

#define DECLITERATE(allocwalk, walkpointer, ubpointer) \
	for (allocwalk = allochead; \
	    allocwalk != 0; \
	    allocwalk = allocwalk->nextalloc) \
		for (walkpointer = &allocwalk->tabslots[0],\
		    ubpointer = &allocwalk->tabslots[TABDALLOP], \
		    ubpointer = ubpointer > ( (cell *)alloctail) \
			 ? nexttab : ubpointer ;\
		    walkpointer < ubpointer; \
		    walkpointer++ )

#define TABCHUNKS(allocwalk, tabptr, size) \
	for (allocwalk = allochead; \
	    allocwalk != 0; \
	    allocwalk = allocwalk->nextalloc) \
		if ( \
		    (tabptr = &allocwalk->tabslots[0]), \
		    (size = ((&allocwalk->tabslots[TABDALLOP]) \
		        > ((cell *)alloctail)) \
		        ? (nexttab - tabptr) : TABDALLOP ), \
		    1 \
		)

#define  PROCESSITERATE(allocwalk, walkpointer, ubpointer) \
	DECLITERATE(allocwalk, walkpointer, ubpointer) \
	if (ISPROCESS(walkpointer))

#define  USERITERATE(allocwalk, walkpointer, ubpointer) \
	DECLITERATE(allocwalk, walkpointer, ubpointer) \
	if (ISUSER(walkpointer))

#define  pgtok(x)  ((x) * pgdiv)


/*** Definition of the fields in the report ***/
#define  AVIO  "avio"   /* Average number of I/O operations per execution */
#define  CPU   "cpu"    /* Sum of user and system time (in minutes) */
#define  K     "k"      /* Average K-blocks of CPU-time per execution */
#define  KSEC  "k*sec"  /* CPU storage integral in kilo-core seconds */
#define  RE    "re"     /* Minutes of real time */
#define  S     "s"      /* Minutes of system CPU time */
#define  TIO   "tio"    /* Total number of I/O operations */
#define  U     "u"      /* Minutes of user CPU time */
#define  MEM   "mem"    
#define  IO    "io"

/**********************************************************************/
/* Global / External Variables                                        */
/**********************************************************************/
static int    uidtable_size ;
static uid_t  *uidtable ;

static nl_catd  catd;

static struct acct  acctbuf;

static int  lflg;			/* Separate system and user time */
static int  cflg;			/* percentage of total time      */
static int  Dflg;			/* total number of disk i/o oper */
static int  dflg;			/* sort by ave. num. disk i/o's  */
static int  iflg;			/* Don't read in summary file.   */
static int  jflg;			/* Give seconds instead of min.  */
static int  Kflg;			/* Print&sort cpu-storage intgrl */
static int  kflg;			/* sort by cpu time ave mem used */
static int  nflg;			/* sort by num of calls          */
static int  aflg;			/* print all command, even singl */
static int  rflg;			/* Reverse the order of the sort */
static int  oflg;			/* print u/s                     */
static int  tflg;			/* ration of real vs. user+sys   */
static int  vflg;			/* junk out command of n or fewr */
static int  fflg;			/* force no inter threshold comp */
static int  uflg;			/* print the user id of the comm.*/
static int  sflg;			/* Merge the account file w/ save*/
static int  bflg;			/* sort user sum div # of calls  */
static int  mflg;			/* # of proc and cpu min.per/user*/
static int  Cflg;			/* Convert old summary file format*/
static int  wold_format;		/* write old_format summary file */
static size_t	RLenSumF;		/*summary file record length */

static int  debug = 0;		/*  debugging flags  */

static struct  utmp  utmp;
#define MAGIC "NewSumFile"

struct  Olduser {
	int     Us_cnt;
	double  Us_ctime;
	double  Us_io;
	double  Us_imem;
	uid_t	Us_uid;
};

#define OldSumFileSize (sizeof(struct {   \
					int	Us_cnt;\
					double	Us_ctime;\
					double	Us_io;\
					double	Us_imem;\
					}))

struct  user {
	char            name[NC];  /* this is <\001><user id><\000> */
	struct Olduser  oldu;
	char            us_name[NAMELG];
};

struct  process {
	char    name[NC];
	int     count;
	double  realt;
	double  cput;
	double  syst;
	double  imem;
	double  io;
};

union  Tab {
	struct process  p;
	struct user     u;
};

typedef  union Tab cell;

/* interpret command time accounting */

struct  allocbox {
	struct  allocbox  *nextalloc;
	cell              tabslots[TABDALLOP];
};

static struct allocbox  *allochead;	/* head of chunk list */
static struct allocbox  *alloctail;	/* tail of chunk list */
static struct allocbox  *newbox;       /* for creating a new chunk */

static cell  *nexttab;  /* next table element that is free */
static int   tabsleft;  /* slots left in current chunk */
static int   ntabs;
/*
 *  When we have to sort the segmented accounting table, we
 *  create a vector of sorted queues that is merged
 *  to sort the entire accounting table.
 */
struct  chunkdesc {
	cell  *chunk_tp;
	int   chunk_n;
};

/*
 *  Hash table segments and manager
 */
struct  hashdallop {
	int                  h_nused;
	struct  hashdallop  *h_next;
	cell                *h_tab[NHASH];
};

static struct  hashdallop  *htab;  /* head of the list */

static double	treal;
static double	tcpu;
static double	tsys;
static double	tio;
static double	timem;
static char	*sname;
static double	ncom;

/*
 *  usracct saves records of type Olduser.
 *  There is one record for every possible uid less than
 *  the largest uid seen in the previous usracct or in savacct.
 *  uid's that had no activity correspond to zero filled slots;
 *  thus one can index the file and get the user record out.
 *  It would be better to save only user information for users
 *  that the system knows about to save space, but that is not
 *  upward compatabile with the old system.
 *
 *  In the old version of sa, uid's greater than 999 were not handled
 *  properly; this system will do that.
 */

#define  USRACCT  "/var/adm/usracct"
#define  SAVACCT  "/var/adm/savacct"
#define  ACCT     "/var/adm/pacct"

static char *usracct = USRACCT;
static char *savacct = SAVACCT;

static cell  *junkp = 0;
static int    thres = 0;
static int    htabinstall = 1;  /* install the symbol */

/* we assume pagesize is at least 1k */
static int  pgdiv;

/**********************************************************************/
/* Function Prototype Declaration                                     */
/**********************************************************************/
static int    cellcmp();
static int    (*cmp)();  /* compares 2 cells; set to appropriate func */
static cell   *enter();
static float   expand();
static long    expand_int();
static char   *getname();

static int   init();
static void   getnames();
static void   tabinit();
static void   htaballoc();

static struct user  *finduser();
static struct user  *wasuser();

extern  tcmp(), ncmp(), bcmp(), dcmp(), Dcmp(), kcmp(), Kcmp();
extern  double sum();

/**********************************************************************/
/* NAME: main                                                         */
/* FUNCTION:  Main routine of sa command.                             */
/* RETURN VALUE:                                                      */
/**********************************************************************/
main(argc, argv)
int  argc;
char *argv[];
{
	FILE  *ff;
	double  ft;
	register struct	allocbox  *allocwalk;
	register cell  *tp, *ub;
	int  c, i, j, size, nchunks, smallest, errflag = 0;
	struct chunkdesc  *chunkvector;
	void  getmaxuid();

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_SA,NL_CAT_LOCALE);

	pgdiv = getpagesize() / 1024;
	if (pgdiv == 0)
		pgdiv = 1;

	getmaxuid();

	tabinit();
	cmp = tcmp;
	while((c = getopt(argc, argv, "abcdDfijJkKlmnorCstuU:v:S:")) != EOF) {
		switch (c) {
		case 'a':
			aflg++;
			break;
		case 'b':
			bflg++;
			cmp = bcmp;
			break;
		case 'c':
			cflg++;
			break;
		case 'd':
			dflg++;
			cmp = dcmp;
			break;
		case 'D':
			Dflg++;
			cmp = Dcmp;
			break;
		case 'f':
			fflg++;	/* force v option; no tty interaction */
			break;
		case 'i':
			iflg++;
			break;
		case 'j':
			jflg++;
			break;
		case 'J':
			debug++;
			break;
		case 'k':
			kflg++;
			cmp = kcmp;
			break;
		case 'K':
			Kflg++;
			cmp = Kcmp;
			break;
		case 'l':
			lflg++;
			break;
		case 'm':
			mflg++;
			break;
		case 'n':
			nflg++;
			cmp = ncmp;
			break;
		case 'o':
			oflg++;
			break;
		case 'r':
			rflg++;
			break;
		case 'C':
			Cflg++; /* convert summary file , falls to case's'*/
		case 's':
			sflg++;
			aflg++;
			break;
		case 'S':
			savacct = optarg;
			break;
		case 't':
			tflg++;
			break;
		case 'u':
			uflg++;
			break;
		case 'U':
			usracct = optarg;
			break;
		case 'v':
			vflg++;
			thres = atoi(optarg);
			break;
		default:
			fprintf(stderr, MSGSTR(INVOPT,"Invalid option %c\n"),
				optopt);
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	if (iflg == 0)   errflag = init();

	/*
	 * When s flag is on, savacct and usracct will be created
	 * if there are no savacct and usracct. When u flag is on,
	 * it needs only the pacct file. So, errflag = 0 in both cases.
	 */
	
	if ((sflg == 1)||(uflg == 1)) errflag = 0;
	if (argc < 1) {
		errflag |= doacct(ACCT);
	}
	else while (argc--) {
		errflag |= doacct(*argv++);
	}
	if (uflg) {
		exit(errflag);
	}

	/*
	 * cleanup pass
	 * put junk together
	 */

	if (vflg) {
		strip();
	}
	if (!aflg) 
	PROCESSITERATE(allocwalk, tp, ub) {
		for (j=0; j<NC; j++) {
			if (tp->p.name[j] == '?')
				goto yes;
		}
		if (tp->p.count != 1) {
			continue;
		}
	yes:
		if (junkp == 0) {
			junkp = enter("***other");
		}
		junkp->p.count += tp->p.count;
		junkp->p.realt += tp->p.realt;
		junkp->p.cput  += tp->p.cput;
		junkp->p.syst  += tp->p.syst;
		junkp->p.imem  += tp->p.imem;
		junkp->p.io    += tp->p.io;
		tp->p.name[0] = 0;
	}

	if (sflg) {
		struct 	user	*up;
		uid_t	uid;
		long MaxOldUid;
		MaxOldUid = (~0UL>>1)/RLenSumF;
		/* if using old format, check before hand if we can 
		   write all the userids successfully */
		if (wold_format)
			for ( j = 0 ; j < uidtable_size ; j++ ) {
				uid = uidtable[j] ;
				if ( (uid > MaxOldUid) && (wasuser(uid) != 0) ){
					fprintf( stderr,
					MSGSTR(BIGUID,"uid %u is too large.\n\tIt can NOT be written in file %s.\n\tYou can use -C instead of -s to write the summary file \n\t in compressed format.  Exiting...\n"),
						         uid, usracct ) ;
					exit(1);
				}
			}
			
		signal(SIGINT, SIG_IGN);
		if ((ff = fopen(usracct, "w")) != NULL) {
			/*
			 *	The old format file can be indexed directly by 
			 *	uid to get the correct record.
			 *	The new format file has uid as part of record.
			 */
			if (!wold_format) 
				fwrite ((void *)MAGIC, 
				  (size_t)strlen(MAGIC), (size_t)1, ff);
			for ( j = 0 ; j < uidtable_size ; j++ ) {
				uid = uidtable[j] ;
				if ( (up = wasuser(uid)) == 0 ) 
					continue;

				if ( wold_format && j && (uid -1) != uidtable[ j -1 ] )
				{
					if ( fseek( ff,(long)(uid) * RLenSumF,0 ))
					{
						/* fseek should never fail */
						fprintf(stderr, 
						 MSGSTR(UNEXP,"Unexpected failure on fseek\n"));
						continue ;
					}
				}
				up->oldu.Us_uid = uid;
				fwrite((void  *)&(up->oldu),
					RLenSumF, (size_t)1, ff);
			}
		}
		if ((ff = fopen(savacct, "w")) == NULL) {
			printf(MSGSTR(CANTSAVE,"Can't save\n"));
			exit(1);
		}
		PROCESSITERATE(allocwalk, tp, ub)
			fwrite((void *)&(tp->p), (size_t)sizeof(struct process), (size_t)1, ff);
		fclose(ff);
		creat(sname, 0644);
		signal(SIGINT, SIG_DFL);
	}
	/*
	 * sort and print
	 */
	if (mflg) {
		printmoney();
		exit(0);
	}
	column(ncom, treal, tcpu, tsys, timem, tio);
	printf("\n");

	/*
	 *  the fragmented table is sorted by sorting each fragment
	 *  and then merging.
	 */
	nchunks = 0;
	TABCHUNKS(allocwalk, tp, size) {
		qsort(tp, size, sizeof(cell), cellcmp);
		nchunks ++;
	}
	chunkvector = (struct chunkdesc *)calloc(nchunks,
		sizeof(struct chunkdesc));
	nchunks = 0;
	TABCHUNKS(allocwalk, tp, size) {
		chunkvector[nchunks].chunk_tp = tp;
		chunkvector[nchunks].chunk_n = size;
		nchunks++;
	}
	for (; nchunks; ) {
		/*
		 *  Find the smallest element at the head of the queues.
		 */
		smallest = 0;
		for (i = 1; i < nchunks; i++) {
			if (cellcmp(chunkvector[i].chunk_tp,
				chunkvector[smallest].chunk_tp) < 0)
					smallest = i;
		}
		tp = chunkvector[smallest].chunk_tp++;
		/*
		 *  If this queue is drained, drop the chunk count,
		 *  and readjust the queues.
		 */
		if (--chunkvector[smallest].chunk_n == 0) {
			nchunks--;
			for (i = smallest; i < nchunks; i++) {
				chunkvector[i] = chunkvector[i+1];
			}
		}
		if (ISPROCESS(tp)) {
			ft = tp->p.count;
			column(ft, tp->p.realt, tp->p.cput,
				tp->p.syst, tp->p.imem, tp->p.io);
			/*** Print process name ***/ 
			printf("   %.14s\n", tp->p.name);
		}
	}	/* iterate to merge the lists */
	exit(errflag);
}

/**********************************************************************/
/* NAME: printmoney                                                   */
/* FUNCTION:                                                          */
/* RETURN VALUE:  none                                                */
/**********************************************************************/
static printmoney()
{
	register uid_t        uid;
	register char         *cp;
	register struct user  *up;
	int                   i;

	getnames();		/* fetches all of the names! */
	for ( i = 0; i < uidtable_size; i++ ) {
		uid = uidtable[i] ;
		if ( (up = wasuser(uid)) != 0 && up->us_cnt ) {
			if (up->us_name[0]) {
				printf("%-8s", up->us_name);
			}
			else {
				printf("%-8d", uid);
			}
			printf("%7u %9.2f%s %10.0f%s %12.0f%s \n",
				up->us_cnt,
				up->us_ctime/60, CPU,
				up->us_io, TIO,
				up->us_imem, KSEC);
		}
	}
}

/**********************************************************************/
/* NAME: column                                                       */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static column(n, a, b, c, d, e)
double  n, a, b, c, d, e;
{
	/*** Print number of called ***/
	printf("%8.0f ", n);
	if (cflg) {
		if (n == ncom) {
			printf("%9s ", "");
		}
		else {
			printf("%8.2f%% ", 100.*n/ncom);
		}
	}
	/*** Print minutes of elapsed real time ***/
	col(n, a, treal, RE);

	if (oflg) {
		col(n, (b/(b+c)), tcpu+tsys, "u/s");
	}
	else if (lflg) {
		/*** Print user CPU time ***/
		col(n, b, tcpu, U);
		/*** Print system CPU time ***/
		col(n, c, tsys, S);
	} 
	else {
		/*** Print minutes of CPU (usr+sys) ***/
		col(n, b+c, tcpu+tsys, CPU);
	}

	if (tflg) {
		/*** Ratio of real time to CPU time ***/
		printf("%8.1f%s/%s ", a/(b+c), RE, CPU);
	}

	if (dflg || !Dflg) {
		/*** Average number of I/O per command ***/
		printf("%10.0f%s ", e/(n?n:1), AVIO);
	}
	else {
		printf("%10.0f%s ", e, TIO);
	}

	if (kflg || !Kflg) {
		/*** k-blocks of memory ***/
		printf("%10.0f%s", ((b+c)!=0.0?((d/(b+c))/HZ):d), K);
	}
	else {
		printf("%10.0f%s ", d, KSEC);
	}
}

/**********************************************************************/
/* NAME:  col                                                         */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static col(n, a, m, cp)
double  n, a, m;
char   *cp;
{
	if (jflg) {
		printf("%11.2f%s ", a, cp); 
	}
	else {
		printf("%11.2f%s ", a/60, cp);
	}

	if (cflg) {
		if (a == m) {
			printf("%9s ", "");
		}
		else {
			printf("%8.2f%% ", 100.*a/m);
		}
	}
}

/**********************************************************************/
/* NAME:  doacct                                                      */
/* FUNCTION:                                                          */
/* RETURN VALUE:  0 - if file f is opened successfully                */
/*                1 - if file f cannot be opened                      */
/**********************************************************************/
static int doacct(f)
char  *f;
{
	FILE *ff;
	float x;
	long y, z;
	struct acct fbuf;
	register char *cp;
	register int c;
	register struct	user *up;
	register cell *tp;
	int	nrecords = 0;

	if (sflg && sname) {
		printf(MSGSTR(ONEFILE,"Only 1 file with -s\n"));
		exit(0);
	}
	if (sflg) {
		sname = f;
	}
	if ((ff = fopen(f, "r")) == NULL) {
		printf(MSGSTR(CANTOPEN,"Can't open %s\n"), f);
		return 1;
	}
	while (fread((void *)&fbuf, (size_t)sizeof(fbuf), (size_t)1, ff) == 1) {
		if (debug)
#if 0
		if (++nrecords % 1000 == 0)
#endif
		{
			printf("Input record from %s number %d\n",
				f, nrecords);
			printf ("ac_comm:%s  ac_rw: %d ac_io: %d ac_mem: %d\n",
				fbuf.ac_comm,fbuf.ac_rw,fbuf.ac_io,
				expand_int(fbuf.ac_mem));
			printf ("etime:%d  stime: %d utime: %d btime: %d\n",
				fbuf.ac_etime,fbuf.ac_stime,fbuf.ac_utime,fbuf.ac_btime);
			printf ("tty:%d  gid: %d uid: %d stat: %d flag: %x\n",
				fbuf.ac_tty,fbuf.ac_gid,fbuf.ac_uid,fbuf.ac_stat,fbuf.ac_flag);
		}
		for (cp = fbuf.ac_comm; *cp && cp < &fbuf.ac_comm[NC]; cp++) {
			if (!isascii((int)*cp) || iscntrl((int)*cp))
				*cp = '?';
		}
		if (cp == fbuf.ac_comm)
			*cp++ = '?';
		if (fbuf.ac_flag&AFORK) {
			if (cp >= &fbuf.ac_comm[NC]) {
				cp = &fbuf.ac_comm[NC-1];
			}
			*cp++ = '*';
		}
		if (cp < &fbuf.ac_comm[NC])
			*cp = '\0';
		x = expand(fbuf.ac_utime) + expand(fbuf.ac_stime);
		y = expand_int(fbuf.ac_mem);
		z = expand_int(fbuf.ac_io);
		if (uflg) {
			printf("%6d %6.2f %s %8d%s %s %6ld %s    %.*s\n",
			    fbuf.ac_uid, x, CPU, y, K, MEM, z, IO, NC, fbuf.ac_comm);
			continue;
		}
		up = finduser(fbuf.ac_uid);
		if (up == 0)
			continue;	/* preposterous user id */
		up->us_cnt++;
		up->us_ctime += (double) x;
		up->us_imem += (double) (x * y);
		up->us_io += (double) z;
		ncom += 1.0;

		tp = enter(fbuf.ac_comm);
		tp->p.imem += (double) (x * y);
		timem += (double) (x * y);
		tp->p.count++;
		x = expand(fbuf.ac_etime);
		tp->p.realt += (double) x;
		treal += (double) x;
		x = expand(fbuf.ac_utime);
		tp->p.cput += (double) x;
		tcpu += (double) x;
		x = expand(fbuf.ac_stime);
		tp->p.syst += (double) x;
		tsys += (double) x;
		tp->p.io += (double) z;
		tio += (double) z;
	}
	fclose(ff);
	return 0;
}

/**********************************************************************/
/* NAME:  cellcmp                                                     */
/* FUNCTION:  Generalized cell compare routine, to cast out users     */
/* RETURN VALUE:  0 -                                                 */
/*                1 -                                                 */
/*               -1 -                                                 */
/*        otherwise -                                                 */
/**********************************************************************/
static cellcmp(p1, p2)
cell  *p1, *p2;
{
	if (ISPROCESS(p1)) {
		if (ISPROCESS(p2)) {
			return((*cmp)(p1, p2));
		}
		return(-1);
	}
	if (ISPROCESS(p2)) {
		return(1);
	}
	return(0);
}

/**********************************************************************/
/* NAME:  ncmp                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static ncmp(p1, p2)
cell  *p1, *p2;
{
	if (p1->p.count == p2->p.count) {
		return(tcmp(p1, p2));
	}
	if (rflg) {
		return(p1->p.count - p2->p.count);
	}
	return(p2->p.count - p1->p.count);
}

/**********************************************************************/
/* NAME:  bcmp                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static bcmp(p1, p2)
cell  *p1, *p2;
{
	double f1, f2;
	double sum();

	f1 = sum(p1)/p1->p.count;    /* (user + system) / (number of calls) */
	f2 = sum(p2)/p2->p.count;    /* (user + system) / (number of calls) */

	if (f1 < f2) {
		if (rflg)  return(-1);
		else       return(1);
	}
	if (f1 > f2) {
		if (rflg)  return(1);
		else       return(-1);
	}
	return(0);
}

/**********************************************************************/
/* NAME:  Kcmp                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static Kcmp(p1, p2)
cell  *p1, *p2;
{
	if (p1->p.imem < p2->p.imem) {
		if (rflg)  return(-1);
		else       return(1);
	}
	if (p1->p.imem > p2->p.imem) {
		if (rflg)  return(1);
		else       return(-1);
	}
	return(0);
}

/**********************************************************************/
/* NAME:  kcmp                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static kcmp(p1, p2)
cell  *p1, *p2;
{
	double a1, a2;

	a1 = p1->p.imem / ((p1->p.cput+p1->p.syst)?(p1->p.cput+p1->p.syst):1);
	a2 = p2->p.imem / ((p2->p.cput+p2->p.syst)?(p2->p.cput+p2->p.syst):1);

	if (a1 < a2) {
		if (rflg)  return(-1);
		else       return(1);
	}
	if (a1 > a2) {
		if (rflg)  return(1);
		else       return(-1);
	}
	return(0);
}

/**********************************************************************/
/* NAME:  dcmp                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static dcmp(p1, p2)
cell  *p1, *p2;
{
	double a1, a2;

	a1 = p1->p.io / (p1->p.count?p1->p.count:1);
	a2 = p2->p.io / (p2->p.count?p2->p.count:1);

	if (a1 < a2) {
		if (rflg)  return(-1);
		else       return(1);
	}
	if (a1 > a2) {
		if (rflg)  return(1);
		else       return(-1);
	}
	return(0);
}

/**********************************************************************/
/* NAME:  Dcmp                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static Dcmp(p1, p2)
cell  *p1, *p2;
{
	if (p1->p.io < p2->p.io) {
		if (rflg)  return(-1);
		else       return(1);
	}
	if (p1->p.io > p2->p.io) {
		if (rflg)  return(1);
		else       return(-1);
	}
	return(0);
}

/**********************************************************************/
/* NAME:  tcmp                                                        */
/* FUNCTION:  Compare total CPU time.                                 */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static tcmp(p1, p2)
cell  *p1, *p2;
{
	extern double sum();
	double f1, f2;

	f1 = sum(p1);  /* Get total CPU time for p1 */
	f2 = sum(p2);  /* Get total CPU time for p2 */

	if (f1 < f2) {
		if (rflg)  return(-1);
		else       return(1);
	}
	if (f1 > f2) {
		if (rflg)  return(1);
		else       return(-1);
	}
	return(0);
}

/**********************************************************************/
/* NAME:  sum                                                         */
/* FUNCTION:  Add user and system CPU time.                           */
/* RETURN VALUE:  Return CPU time of user and system.                 */
/**********************************************************************/
static double sum(p)
cell  *p;
{
	if (p->p.name[0] == 0) {
		return(0.0);
	}
	return( p->p.cput + p->p.syst );
}

/**********************************************************************/
/* NAME:  init                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:  0 - if savacct and usracct are opened successfully  */
/*                1 - otherwise                                       */
/**********************************************************************/
static int  init()
{
	struct user     userbuf;
	struct process	tbuf;
	register cell  *tp;
	register struct user  *up;
	int    uid;
	FILE  *f;
	int    gap = 0;
	int    returnflag = 0;
	char dummy[20];

	if ((f = fopen(savacct, "r")) == NULL) {
		returnflag = 1;
		goto gshm;
	}
	while (fread((void *)&tbuf, (size_t)sizeof(struct process), (size_t)1, f) == 1) {
		tp = enter(tbuf.name);
		ncom += tbuf.count;
		tp->p.count = tbuf.count;
		treal += tbuf.realt;
		tp->p.realt = tbuf.realt;
		tcpu += tbuf.cput;
		tp->p.cput = tbuf.cput;
		tsys += tbuf.syst;
		tp->p.syst = tbuf.syst;
		tio += tbuf.io;
		tp->p.io = tbuf.io;
		timem += tbuf.imem;
		tp->p.imem = tbuf.imem;
	}
	fclose(f);
 gshm:
	/* If summary file does not exist, create in new format */
	wold_format = 0;
	RLenSumF = sizeof(struct Olduser);
	if ((f = fopen(usracct, "r")) == NULL) {
		return 1;
	}
 	/* Check if the file has new format */
	fread(dummy, strlen(MAGIC), 1, f);
	if (memcmp(dummy, MAGIC, strlen(MAGIC)) != 0)	{
		/* oldformat, backout the bytes read */
		fseek(f, 0L ,0);
		wold_format = 1;
		RLenSumF = OldSumFileSize ;
	}
	for (uid = 0;
	    fread((void *)&(userbuf.oldu), RLenSumF, (size_t)1, f) == 1;
	    uid++) {
		if (!wold_format)
			uid=userbuf.oldu.Us_uid;
		if ( userbuf.us_cnt && (up = finduser(uid)) ) {
			up->oldu = userbuf.oldu;
		}
		else if ( ++gap >= GAP_SIZE ) {
			fprintf( stderr, MSGSTR(LARGEGAP,
				"sa: (WARNING) large uid gap while reading file %s, still processing.\n\tCurrent id is %u (%d), Maxid is %u (%d)\n"),
				usracct, uid, uid,
				uidtable[ uidtable_size - 1 ], uidtable[ uidtable_size - 1 ] );
			gap = 0 ;
		}
	}
	fclose(f);
	if (Cflg) {
		/* While writing use the new format "wold_format=0"*/
		wold_format = 0;
		RLenSumF = sizeof(struct Olduser);
	}
	return returnflag;
}

/**********************************************************************/
/* NAME:  strip                                                       */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static strip()
{
	int c;
	register struct allocbox *allocwalk;
	register cell *tp, *ub, *junkp;

	if (fflg) {
		printf(MSGSTR(CATEGOR,
		"Categorizing commands used %d times or fewer as **junk**\n"),
		thres);
	}
	junkp = enter("**junk**");
	PROCESSITERATE(allocwalk, tp, ub) {
		if (tp->p.name[0] && tp->p.count <= thres) {
			if (!fflg) {
				printf("%.14s--", tp->p.name);
			}
			if (fflg || ((c=getchar())=='y')) {
				tp->p.name[0] = '\0';
				junkp->p.count += tp->p.count;
				junkp->p.realt += tp->p.realt;
				junkp->p.cput += tp->p.cput;
				junkp->p.syst += tp->p.syst;
				junkp->p.imem += tp->p.imem;
				junkp->p.io += tp->p.io;
			}
			if (!fflg) {
				while (c && c!='\n') {
					c = getchar();
				}
			}
		}
	}
}

/**********************************************************************/
/* NAME:  expand                                                      */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static float  expand(t)
unsigned  t;
{
	register long  nt;
	float  e;

	if (debug) {
		printf ("input: %d ",t);
	}
	nt = t&017777;
	t >>= 13;
	while (t!=0) {
		t--;
		nt <<= 3;
	}
	e = (float) (nt/AHZ) + (((float)(nt%AHZ))/AHZ);
	if (debug) {
		printf (" outpuf %f\n",e);
	}
	return(e);
}

/**********************************************************************/
/* NAME:  expand_int                                                  */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static long  expand_int(t)
unsigned  t;
{
	register long  nt;

	if (debug) {
		printf ("input: %d ",t);
	}
	nt = t&017777;
	t >>= 13;
	while (t!=0) {
		t--;
		nt <<= 3;
	}
	if (debug) {
		printf (" outpuf %d\n",nt);
	}
	return(nt);
}

static char  UserKey[NAMELG + 2];

/**********************************************************************/
/* NAME:  makekey                                                     */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static char  *makekey(uid)
uid_t  uid;
{
	sprintf(UserKey+1, "%04x", uid);
	UserKey[0] = USERKEY;
	return(UserKey);
}

/**********************************************************************/
/* NAME:  wasuser                                                     */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static struct user  *wasuser(uid)
uid_t  uid;
{
	struct user *tp;

	htabinstall = 0;
	tp = finduser(uid);
	htabinstall = 1;
	return(tp);
}

/**********************************************************************/
/* NAME:  finduser                                                    */
/* FUNCTION:                                                          */
/*    Only call this if you really want to insert it in the table!    */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static struct user  *finduser(uid)
uid_t  uid;
{
	int  j;

	for ( j = 0 ; j < uidtable_size ; j++ ) {
		if ( uidtable[ j ] >= uid ) {
			break ;
		}
	}

	if ( j >= uidtable_size || uidtable[ j ] != uid ) {
		fprintf(stderr, MSGSTR(BADID,
			"sa: The user id %u (%d) is not recognized. It is ignored.\n"),
			uid, uid );
		return(0);
	}
	return((struct user*)enter(makekey(uid)));
}

/**********************************************************************/
/* NAME:  getnames                                                    */
/* FUNCTION:                                                          */
/*
 *  Set the names of all users in the password file.
 *  We will later not print those that didn't do anything.
 */
/* RETURN VALUE:  none                                                */
/**********************************************************************/
static void  getnames()
{
	register struct user *tp;
	register struct passwd *pw;
	struct passwd *getpwent();

	setpwent();
	while (pw = getpwent()) {
		/* use first name in passwd file for duplicate uid's */
		if ((tp = wasuser(pw->pw_uid)) != 0 && 
			!isalpha((int)tp->us_name[0])) {
			strncpy(tp->us_name, pw->pw_name, NAMELG);
		}
	}
	endpwent();
}

/**********************************************************************/
/* NAME:  getmaxuid                                                   */
/* FUNCTION:                                                          */
/* RETURN VALUE:  none                                                */
/**********************************************************************/
static void  getmaxuid()
{
	register struct user    *tp;
	register struct passwd  *pw;
	struct passwd *getpwent();
	int  i, where, uidtable_max;

	uidtable_size = 0;
	uidtable_max  = UID_DELTA_GROWTH;
	uidtable = (uid_t *)malloc( (int)(uidtable_max * sizeof(uid_t)) );
	if ( ! uidtable ) {
		fprintf( stderr, MSGSTR(MALLOCFAIL,
			"sa: (FATAL) malloc() failed\n"));
		exit( -1 ) ;
	}

	setpwent();
	while (pw = getpwent()) {
		/************************************************************
		*                       INSERT  SORT                       *
		************************************************************/
		if ( !uidtable_size || pw->pw_uid > uidtable[uidtable_size - 1] )
			where = uidtable_size;
		else {
			for ( i = 0; i < uidtable_size; i++ )
				if ( pw->pw_uid <= uidtable[ i ] )
					break;
			if ( pw->pw_uid == uidtable[ where = i ] )
				continue;
		}
		if ( uidtable_size >= uidtable_max ) {
			uidtable_max  += UID_DELTA_GROWTH;
			uidtable = (uid_t *)realloc(uidtable,
				(int)(uidtable_max * sizeof(uid_t)) );
			if ( ! uidtable ) {
				fprintf( stderr, MSGSTR(REALLOCFAIL,
					"sa: (FATAL) realloc() failed\n"));
				exit( -1 );
			}
		}
		if ( uidtable_size != where )
			for ( i = uidtable_size; i > where; i-- )
				uidtable[ i ] = uidtable[ i-1 ];
		uidtable[ where ] = pw->pw_uid;
		uidtable_size++;
	}
	endpwent();

	if ( uidtable_size == 0 ) {
		fprintf( stderr, MSGSTR( EMPTYPASSWD,
			"sa: (FATAL) passwd file is empty\n"));
		exit( -1 ) ;
	}
}

/**********************************************************************/
/* NAME:  tabinit                                                     */
/* FUNCTION:                                                          */
/* RETURN VALUE:  none                                                */
/**********************************************************************/
static void  tabinit()
{
	allochead = 0;
	alloctail = 0;
	nexttab   = 0;
	tabsleft  = 0;
	htab      = 0;
	ntabs     = 0;
	htaballoc();    /* get the first part of the hash table */
}

/**********************************************************************/
/* NAME:  taballoc                                                    */
/* FUNCTION:                                                          */
/* RETURN VALUE: (cell *)                                             */
/**********************************************************************/
static cell  *taballoc()
{
	if (tabsleft == 0) {
		newbox = (struct allocbox *)calloc(1, ALLOCQTY);
		tabsleft = TABDALLOP;
		nexttab = &newbox->tabslots[0];
		if (alloctail == 0) {
			allochead = alloctail = newbox;
		} 
		else {
			alloctail->nextalloc = newbox;
			alloctail = newbox;
		}
	}
	--tabsleft;
	++ntabs;
#ifdef DEBUG_IT
	if (ntabs % 100 == 0) {
		printf("##Accounting table slot # %d\n", ntabs);
	}
#endif DEBUG_IT
	return(nexttab++);
}

/**********************************************************************/
/* NAME:  htaballoc                                                   */
/* FUNCTION:                                                          */
/* RETURN VALUE:  none                                                */
/**********************************************************************/
static void  htaballoc()
{
	register struct hashdallop *new;
#ifdef DEBUG_IT
	static int ntables = 0;

	printf("%%%New hash table chunk allocated, number %d\n", ++ntables);
#endif DEBUG_IT
	new = (struct hashdallop *)calloc(1, sizeof (struct hashdallop));
	if (htab == 0) {
		htab = new;
	}
	else {		/* add AFTER the 1st slot */
		new->h_next = htab->h_next;
		htab->h_next = new;
	}
}

/**********************************************************************/
/* NAME:  enter                                                       */
/* FUNCTION:                                                          */
/*
 *  Lookup a symbol passed in as the argument.
 *
 *  We take pains to avoid function calls; this function
 *  is called quite frequently, and the calling overhead
 *  contributes significantly to the overall execution speed of sa.
 */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static cell  *enter(name)
char  *name;	
{
	static int     initialprobe;
	register cell  **hp;
	register char  *from, *to;
	register int   len, nprobes;
	static struct  hashdallop  *hdallop, *emptyhd;
	static cell    **emptyslot, **hp_ub;

	emptyslot = 0;
	for (nprobes = 0, from = name, len = 0;
	     *from && len < NC;
	     nprobes <<= 2, nprobes += *from++, len++) {
		continue;
	}
	nprobes += from[-1] << 5;
	nprobes %= NHASH;
	if (nprobes < 0) {
		nprobes += NHASH;
	}

	initialprobe = nprobes;
	for (hdallop = htab; hdallop != 0; hdallop = hdallop->h_next) {
		for (hp = &(hdallop->h_tab[initialprobe]),
			nprobes = 1,
			hp_ub = &(hdallop->h_tab[NHASH]);
			(*hp) && (nprobes < NHASH);
			hp += nprobes,
			hp -= (hp >= hp_ub) ? NHASH:0,
			nprobes += 2) {
			from = name;
			to = (*hp)->p.name;

			for (len = 0; (len<NC) && *from; len++) {
				if (*from++ != *to++)
					goto nextprobe;
			}
			if (len >= NC) {  /*both are maximal length*/
				return(*hp);
			}
			if (*to == 0) {  /*assert *from == 0*/
				return(*hp);
			}
	nextprobe: ;
		}
		if (*hp == 0 && emptyslot == 0 &&
			hdallop->h_nused < HASHCLOGGED) {
			emptyslot = hp;
			emptyhd = hdallop;
		}
	}
	if (emptyslot == 0) {
		htaballoc();
		hdallop = htab->h_next;		/* aren't we smart! */
		hp = &hdallop->h_tab[initialprobe];
	} 
	else {
		hdallop = emptyhd;
		hp = emptyslot;
	}
	if (htabinstall) {
		*hp = taballoc();
		hdallop->h_nused++;
		for (len = 0, from = name, to = (*hp)->p.name; (len<NC); len++)
			if ((*to++ = *from++) == '\0')
				break;
		return(*hp);
	}
	return(0);
}

