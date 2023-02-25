static char sccsid[] = "@(#)82  1.31  src/bos/usr/sbin/sar/sar.c, cmdstat, bos41J, 9521B_all 5/18/95 02:11:45";
/*
*/
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 27 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *  
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */
/*
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

/*
 *
 *  sar.c - It generates a report either
 *          from an input data file or
 *          by invoking sadc to read system activity counters 
 *          at the specified intervals.
 *
 *  usage: sar { -A | [-a][-b][-c][-k][-m][-q][-r][-u][-v][-w][-y] }
 *             [-s hh[:mm[:ss]]] [-e hh[:mm[:ss]]]
 *	       [-P processor_id[,...] | ALL]
 *             [-f file] [-i seconds] [-o file] [interval [number]]
 */

/**********************************************************************/
/* Include File                                                       */
/**********************************************************************/

#define _ILS_MACROS

#include  <stdio.h>
#include  <sys/param.h>
#include  <ctype.h>
#include  <time.h>
#include  <sys/types.h>
#include  <sys/ltypes.h>
#include  <sys/sysinfo.h>
#include  <sys/utsname.h>
#include  <locale.h>
#include  <stdlib.h>
#include  <sys/access.h>
#include  "sa.h"  /* note the structure in sa.h defines namei(), which
                   * really is lookuppn() */
#include  "sar_msg.h"

/**********************************************************************/
/* Constant Definition / Macro Function                               */
/**********************************************************************/

#define  MAX_NUMBER       2147483647
#define  SADC_CMD	"/usr/lib/sa/sadc"
#define  DISP_OPTIONS	"abckmqruvwy"

#define  MSGSTR(Num,Str)  catgets(catd,MS_SAR,Num,Str)


/**********************************************************************/
/* Function Prototype Declaration                                     */
/**********************************************************************/

extern char  *strchr(const char *s, int c);
static void  prpass();
static void  prttim();

int  strlen(), strdmp();
/*
char  *strcpy(), *strncat(), *strncpy(), *strchr();
*/
static int init_cpu();
static void fill_nx();

/**********************************************************************/
/* Global / External Variables                                        */
/**********************************************************************/

static nl_catd  catd;

static struct sa       nx, ox;
static struct sa_float ax, bx;
static struct tm       *curt, args, arge;
static struct utsname  name;

static int    sflg, eflg, iflg, oflg, fflg;
static int    realtime;
static int    passno;
static int    t=0;
static int    n=0;
static int    recsz, tblmap[SINFO];
static int    j,i;
static int    tabflg;
static int    debug=0;
static char   options[16],fopt[16],excl_opts[16];
static float  s_time, e_time, isec;
static int    fin, fout, childid;
static int    pipedes[2];
static char   arg1[30], arg2[10], arg3[10];
static char   tblcpu[N_PROCESSOR + 1];
		/* the value of tblcpu = 1 if the processor must be repported, 
		 *  and = 0 else   */
static struct sa_old nx_old;
static int    cpuflg;
static int    newflg;
static struct sa_head sa_head;
static int    recsz_old;
static float  tdiff[N_PROCESSOR + 1];
static char   inter[16];
static int    pallflg;

extern int   optind;
extern char  *optarg;

/**********************************************************************/
/* NAME:  main                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/

main (argc,argv)
int   argc;
char  *argv[];
{
	char   flnm[50], ofile[50];
	char   ccc, *ch1, *ch2;
	long   temp;
	int    jj=0;
	float  convtm();
	long   lseek();
	int    t0, n0;  /* Check validity of <Interval> and <Number> */

	char   Popt[N_PROCESSOR], opt_buf[256];
	int    ind;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_SAR,NL_CAT_LOCALE);


	tblcpu[GLOBAL] = 1;
	/* process options with arguments and pack options 
	 * without arguments  */
	sprintf(opt_buf, "%s%s", "ADo:s:e:i:f:P:", DISP_OPTIONS);
	while ((i= getopt(argc,argv,opt_buf)) != EOF) {
		switch (ccc = i) {
		case 'o':
			oflg++;
			sprintf(ofile,"%s",optarg);
			break;
		case 'D':
			debug++;
			break;
		case 's':
			if (sscanf(optarg,"%d:%d:%d",
			&args.tm_hour, &args.tm_min, &args.tm_sec) < 1)
				pillarg(optarg,ccc);
			else {
				sflg++,
				s_time = args.tm_hour*3600.0 +
					args.tm_min*60.0 +
					args.tm_sec;
			}
			break;
		case 'e':
			if (sscanf(optarg,"%d:%d:%d",
			&arge.tm_hour, &arge.tm_min, &arge.tm_sec) < 1)
				pillarg(optarg,ccc);
			else {
				eflg++;
				e_time = arge.tm_hour*3600.0 +
					arge.tm_min*60.0 +
					arge.tm_sec;
			}
			break;
		case 'i':
			if (sscanf(optarg,"%f",&isec) < 1)
				pillarg(optarg,ccc);
			else if (isec > 0.0)
				iflg++;
			break;
		case 'f':
			fflg++;
			sprintf(flnm,"%s",optarg);
			break;
		case 'P':
			sprintf(Popt,"%s",optarg);
			cpuflg++;
			break;
		case '?':
			pmsgexit("");
			exit(2);
			break;
		default:
			strncat (options,&ccc,1);
			break;
		}
	}

	/*  Are starting and ending times consistent?  */

	if ((sflg) && (eflg) && (e_time <= s_time)) {
		pmsgexit(MSGSTR(ETIME, "e_time <= s_time"));
	}


	/*   
	 *  Determine if t and n arguments are given,
	 *  and whether to run in real time or from a file
	 */

	switch (argc - optind) {
	  case 0:  /*   Get input data from file   */
		if (fflg == 0) {
			temp = time((long *) 0);
			curt = localtime(&temp);
			sprintf(flnm,"/var/adm/sa/sa%.2d", curt->tm_mday);
		}
		if ((fin = open(flnm, 0)) == -1) {
			fprintf(stderr, MSGSTR(CANTOPEN, "Can't open %s\n"), flnm);
			fprintf(stderr, MSGSTR(TRYRUNNING, 
			"Try running /usr/lib/sa/sa1 <inc> <num>\n"));
			exit(1);
		}
		break;
	  case 1:  /*   Real time data; one cycle   */
		realtime++;
		t0 = atoi(argv[optind]);
		n0 = 1;
		break;
	  case 2:  /*   Real time data; specified cycles   */
	  default:
		realtime++;
		t0 = atoi(argv[optind]);
		n0 = atoi(argv[optind+1]);
		break;
	}

	/*  "u" is default option to display cpu utilization   */
	if (strlen(options) == 0) {
		strcpy(options, "u");
	}

	/*  'A' means all data options   */
	 if (strchr(options, 'A') != (char *)NULL) {
		if (!cpuflg) {
			strcpy(options, DISP_OPTIONS);
		}
		else {
			 strcpy(options, "acmuw");
		}
	}

	/* if option -P then only options acmuw are accepted */

	if ((cpuflg) && (strchr(options, 'A') == (char *)NULL)) {
		if (strchr(options, 'a') != (char *)NULL)
			strncat(inter,"a",1);
		if (strchr(options, 'c') != (char *)NULL)
			strncat(inter,"c",1);
		if (strchr(options, 'm') != (char *)NULL)
			strncat(inter,"m",1);
		if (strchr(options, 'u') != (char *)NULL)
			strncat(inter,"u",1);
		if (strchr(options, 'w') != (char *)NULL)
			strncat(inter,"w",1);
		strcpy(options,inter);
		if (strlen(options) == 0) {
                	fprintf(stderr,MSGSTR(BADOPTION,
                        "With option P, only options acmuw are accepted. \n"));
                	exit(1);
		}
	}
	if (debug)
		printf("options : %s \n",options);

	if (realtime) {
	/*  Get input data from sadc via pipe   */
		/* Check validity of specified Interval length */
		if ( t0 <= 0 ) {
			pmsgexit(MSGSTR(INV_INTERVAL, 
			"Invalid interval length specified."));
		}
		t = t0;

		/* Check validity of specified Number of intervals */
		if ((n0 < 0) || (n0 >= MAX_NUMBER)) {
			pmsgexit(MSGSTR(INV_NUMBER,
			"Invalid number of interval specified."));
		}
		n = (n0 + 1);

		ch2 = excl_opts;
		for(ch1 = DISP_OPTIONS; *ch1; ch1++)
			if (strchr(options, ch1) == NULL)
				*ch2++ = ch1;
		*ch2 = '\0';
		sprintf(arg1,"-x %s", excl_opts);
		sprintf(arg2,"%d", t);
		sprintf(arg3,"%d", n);
		/* Avoid weird messages from both parent and  */
		/* child about permissions & inconsistencies. */
		if (access(SADC_CMD, X_ACC) < 0)
			perrexit();
		if (pipe(pipedes) == -1)
			perrexit();
		if ((childid = fork()) == 0) {  /*  child:   */
			close(1);  /*  shift pipedes[write] to stdout  */
			dup(pipedes[1]);
			if (execlp (SADC_CMD,SADC_CMD,arg1,arg2,arg3,0) == -1)
				perrexit();
		}  /*   parent:   */
		/* Check for bad fork! */
		if (childid == -1)  perrexit();

		fin = pipedes[0];
		close(pipedes[1]);  /*   Close unused output   */
	}

	if (oflg) {
		if (strcmp(ofile, flnm) == 0) {
			pmsgexit(MSGSTR(OFILE, "ofile same as ffile"));
		}
		fout = creat(ofile, 00644);
	}

	/*  read the header record and compute record size */
	if (read(fin, tblmap, sizeof tblmap) < 0) {
		perrexit ();
	}
	if (read(fin, &sa_head, sizeof(sa_head)) < 0) {
		perrexit ();
	}
	if (sa_head.magic == SAR_MAGIC) 
		newflg++;
	if (debug)
		printf("newflg = %d \n", newflg);
	if ((!newflg) && (realtime)) {
		fprintf(stderr,MSGSTR(COMPAT,
			"Versions of sadc and sar are inconsistent. \n"));
		exit(1);
	}
	if ((!newflg) && (cpuflg)) {
                fprintf(stderr,MSGSTR(BADOPT,
                        "Option P is inconsistent with input file. \n"));
                exit(1);
	}

	for (i=0;i<SINFO;i++) {
		recsz += tblmap[i];
		recsz_old += tblmap[i];
	}
	recsz = sizeof (struct sa) - sizeof nx.devio +
			recsz * sizeof nx.devio[0];
	recsz_old = sizeof (struct sa_old) - sizeof nx_old.sa_old_devio +
			recsz_old * sizeof nx_old.sa_old_devio[0];
	if (!newflg) {
		sa_head.magic = SAR_MAGIC;
		sa_head.nbcpu = 0;
	}
	if ((cpuflg) && (init_cpu(Popt) < 0))
		pillarg(Popt,'P');
	if (debug) {
		printf("nbcpu = %d \n",sa_head.nbcpu);
		for (ind = 0; ind <= N_PROCESSOR; ind++) {
			printf(" %d ",tblcpu[ind]);
		}
		printf (" \n");
	}

	if (oflg) {
	 	write(fout, tblmap, sizeof tblmap);
		write(fout, &sa_head, sizeof sa_head);
	}

	if (realtime) {
		/*   Make single pass, processing all options   */
		strcpy(fopt, options);
		passno++;
		prpass();
		kill(childid, 2);
		wait((int *) 0);
	}
	else {
		/*  Make multiple passes, one for each option   */
		while (strlen(strncpy(fopt,&options[jj++],1)) > 0) {
			lseek(fin, (long)(sizeof tblmap), 0);
			if (newflg)
				lseek(fin, (long)(sizeof sa_head), SEEK_CUR);
			passno++;
			prpass();
		}
	}
	exit(0);
}


/**********************************************************************/
/* NAME:  prpass                                                      */
/* FUNCTION:  Read records from input, classify, and decide           */
/*            on printing.                                            */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static void  prpass()
{
	int    lines=0;
	int    recno=0;   /* number of record traited  */
	float  tnext=0;
	float  trec;
	int rc;
	int ind;

	if (sflg)  tnext = s_time;

	for (;;) {
		if (newflg)
			rc = read(fin, &nx, (unsigned)recsz);
		else
			rc = read(fin, &nx_old, (unsigned)recsz_old);
		if (rc <= 0 )
			break;
			
		if (!newflg)
			fill_nx();

		if (debug) {
			for (ind=0 ; ind <= N_PROCESSOR ; ind++) {
		      printf ("nx0: %d  %d  %d  %d\n",
 		      nx.si[ind].cpu[0], nx.si[ind].cpu[1],
			 nx.si[ind].cpu[2], nx.si[ind].cpu[3]);
		      printf ("ox0: %d  %d  %d  %d\n",
 		  ox.si[ind].cpu[0], ox.si[ind].cpu[1],
		 ox.si[ind].cpu[2], ox.si[ind].cpu[3]);
		     printf("ind = %d \n",ind);
		}
		}
		curt = localtime(&nx.ts);
		trec = curt->tm_hour * 3600.0
			+ curt->tm_min * 60.0
			+ curt->tm_sec;
		if ((recno == 0) && (trec < s_time))
			continue;
		if ((eflg) && (trec > e_time))
			break;
		if ((oflg) && (passno == 1))
			write(fout, &nx, (unsigned)recsz);
		if (recno == 0) {
			if (passno == 1) {
				uname(&name);
				printf("\n%s %s %s %s %s    %.2d/%.2d/%.2d\n",
					name.sysname,
					name.nodename,
					name.release,
					name.version,
					name.machine,
					curt->tm_mon + 1,
					curt->tm_mday,
					curt->tm_year);
			}
			prthdg();
			recno = 1;
			if ((iflg) && (tnext == 0))
				tnext = trec;
		}
		if (nx.si[GLOBAL].cpu[0]==-300 &&  nx.si[GLOBAL].cpu[1]==-300 &&
		    nx.si[GLOBAL].cpu[2]==-300 &&  nx.si[GLOBAL].cpu[3]==-300 ) {

		/*  This dummy record signifies system restart
		    New initial values of counters follow in next record  */
			prttim();
			printf(MSGSTR(RESTART, "\taix restarts\n"));
			recno = 1;
			continue;
		}
		if ((iflg) && (trec < tnext))
			continue;
		if (recno++ > 1) {
			rc = 0;
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
			   if (!tblcpu[ind])
				continue;
		 	   tdiff[ind] =   nx.si[ind].cpu[0] - ox.si[ind].cpu[0]
				        + nx.si[ind].cpu[1] - ox.si[ind].cpu[1]
					+ nx.si[ind].cpu[2] - ox.si[ind].cpu[2]
				       + nx.si[ind].cpu[3] - ox.si[ind].cpu[3];
			   if ( ind == GLOBAL ) {
				tdiff[ind] = tdiff[ind] / sa_head.nbcpu;
			   }
				if (tdiff[ind] <= 0) {
					if (debug) {
					      printf ("nx0: %d  %d  %d  %d\n",
 					  nx.si[ind].cpu[0], nx.si[ind].cpu[1],
					 nx.si[ind].cpu[2], nx.si[ind].cpu[3]);
					      printf ("ox0: %d  %d  %d  %d\n",
 					  ox.si[ind].cpu[0], ox.si[ind].cpu[1],
					 ox.si[ind].cpu[2], ox.si[ind].cpu[3]);
					     printf("ind = %d \n",ind);
					/* tdiff = 1; */
					/* ox = nx */    /*  Age the data */
					} 
					fprintf(stderr, MSGSTR(TIMECHG, "sar: time change not positive\n"));
					rc = -1;
					break;
			 	}
			}
			if (rc == -1) /* tdiff is negative in the preceding loop */
				break;
			prtopt();	/*  Print a line of data  */
			lines++;
		}
		ox = nx;		/*  Age the data	*/
		if (isec > 0)
			while(tnext <= trec)
				tnext += isec;
	}
	if (lines > 1) {
		n=lines+1;
		prtavg();
	}
	ax = bx;    /* Zero out the accumulators   */
}

/**********************************************************************/
/* NAME:  prttim                                                      */
/* FUNCTION:  Print time label routine.                               */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static void  prttim()
{
	curt = localtime(&nx.ts);
	printf("%.2d:%.2d:%.2d",
		curt->tm_hour,
		curt->tm_min,
		curt->tm_sec);
	tabflg = 1;
}

/**********************************************************************/
/* NAME:  tsttab                                                      */
/* FUNCTION:  Add 8 spaces to the table if flag turned on.            */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static tsttab()
{
	if (tabflg == 0)  printf("        ");
	else              tabflg = 0;
}

/**********************************************************************/
/* NAME:  prthdg                                                      */
/* FUNCTION:  Print report heading.                                   */
/* RETURN VALUE:  none                                                */
/**********************************************************************/
/* the flags u a c m w have been modified */
/* a new header has been added to manage the flag cpuflg */

static prthdg()
{
	int   jj=0;
	char  ccc;

	printf("\n");
	prttim();
	while ((ccc = fopt[jj++]) != (char)NULL) {
		switch (ccc) {
		  case 'u':
			tsttab();
			if (cpuflg)
				printf(" %3s %7s %7s %7s %7s\n",
					"cpu",
					"%usr",
					"%sys",
					"%wio",
					"%idle");
			else
				printf(" %7s %7s %7s %7s\n",
					"%usr",
					"%sys",
					"%wio",
					"%idle");
			break;
		  case 'y':
			tsttab();
			printf(" %7s %7s %7s %7s %7s %7s\n",
				"rawch/s",
				"canch/s",
				"outch/s",
				"rcvin/s",
				"xmtin/s",
				"mdmin/s");
			break;
		  case 'b':
			tsttab();
			printf(" %7s %7s %7s %7s %7s %7s %7s %7s\n",
				"bread/s",
				"lread/s",
				"%rcache",
				"bwrit/s",
				"lwrit/s",
				"%wcache",
				"pread/s",
				"pwrit/s");
			break;
		  case 'k':
			tsttab();
			printf(" %8s %8s %8s\n",
				"ksched/s",
				"kproc-ov",
				"kexit/s" );
			break;
		  case 'd':
			tsttab();
			printf(" %7s %7s %7s %7s %7s %7s %7s\n",
				"device",
				"%busy",
				"avque",
				"r+w/s",
				"blks/s",
				"avwait",
				"avserv");
			break;
		  case 'v':
			tsttab();
			if (newflg) {
			printf("  %-10s  %-10s  %-10s  %-10s\n",
				"proc-sz",
				"inod-sz",
				"file-sz",
				"thrd-sz");
			}
			else {
			printf("  %-10s   %-10s   %-10s\n",
				"proc-sz",
				"inod-sz",
				"file-sz");
			}
			break;
		  case 'c':
			tsttab();
			if (cpuflg)
				printf(" %3s %7s %7s %7s %7s %7s %7s %7s\n",
					"cpu",
					"scall/s",
					"sread/s",
					"swrit/s",
					"fork/s",
					"exec/s",
					"rchar/s",
					"wchar/s");
			else
				printf(" %7s %7s %7s %7s %7s %7s %7s\n",
					"scall/s",
					"sread/s",
					"swrit/s",
					"fork/s",
					"exec/s",
					"rchar/s",
					"wchar/s");
			break;
		  case 'w':
			tsttab();
			if (newflg){
				if (cpuflg) {
					printf(" %3s %7s\n",
						"cpu",
						"cswch/s");
				}
				else {
					printf(" %7s\n",
						"cswch/s");
				}
			}
			else {
				if (cpuflg) {
					printf(" %3s %7s\n",
						"cpu",
						"pswch/s");
				}
				else {
					printf(" %7s\n",
						"pswch/s");
				}
			}
			break;
		  case 'a':
			tsttab();
			if (cpuflg)
				printf(" %3s %7s %10s %8s\n",
					"cpu",
					"iget/s",
					"lookuppn/s",
					"dirblk/s" );
			else
				printf(" %7s %10s %8s\n",
					"iget/s",
					"lookuppn/s",
					"dirblk/s" );
			break;
		  case 'q':
			tsttab();
			printf(" %7s %7s %7s %7s\n",
				"runq-sz",
				"%runocc",
				"swpq-sz",
				"%swpocc" );
			break;
		  case 'm':
			tsttab();
			if (cpuflg)
				printf(" %3s %7s %7s\n",
					"cpu",
					"msg/s",
					"sema/s");
			else
				printf(" %7s %7s\n",
					"msg/s",
					"sema/s");
			break;
		  case 'r':
			tsttab();
			printf(" %7s %7s %7s %7s\n",
				"slots",
				"cycle/s",
				"fault/s",
				"odio/s");
			break;
		}
	}
	if (jj > 2)  printf("\n");
}

/**********************************************************************/
/* NAME:  prtopt                                                      */
/* FUNCTION:  Print options routine                                   */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static prtopt()
{
	register int  ii,kk,mm;
	int    jj=0;
	char   ccc;
	float  hr, hw;
	char   buf1[20], buf2[10], buf3[20], buf4[20];
	int    ind;
	float  div;

	if (strcmp(fopt, "d") == 0)   printf("\n");
	prttim();
	for (ind = 0; ind <= N_PROCESSOR; ind++) {
		for (ii=0;ii<4;ii++) {
			ax.si[ind].cpu[ii] += nx.si[ind].cpu[ii] 
						- ox.si[ind].cpu[ii];
		}
	}

	while ((ccc = fopt[jj++]) != (char)NULL) {
		switch (ccc) {
		  case 'u':
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
					printf("  %c ",'-');
				}
				if (ind == GLOBAL){
					div = tdiff[ind] * sa_head.nbcpu;
				}
				else {
					div = tdiff[ind] ;
				}
				printf(" %7.0f %7.0f %7.0f %7.0f\n",
					(float)(nx.si[ind].cpu[1] 
				       - ox.si[ind].cpu[1])/div * 100.0,
					(float)(nx.si[ind].cpu[2] 
				       - ox.si[ind].cpu[2])/div * 100.0,
					(float)(nx.si[ind].cpu[3] 
				       - ox.si[ind].cpu[3])/div * 100.0,
					(float)(nx.si[ind].cpu[0] 
				      - ox.si[ind].cpu[0])/div * 100.0);
			}

			break;
		  case 'y':
			tsttab();
			printf(" %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)(nx.si[GLOBAL].rawch - ox.si[GLOBAL].rawch)/tdiff[GLOBAL] * HZ,
			(float)(nx.si[GLOBAL].canch - ox.si[GLOBAL].canch)/tdiff[GLOBAL] * HZ,
			(float)(nx.si[GLOBAL].outch - ox.si[GLOBAL].outch)/tdiff[GLOBAL] * HZ,
			(float)(nx.si[GLOBAL].rcvint - ox.si[GLOBAL].rcvint)/tdiff[GLOBAL] * HZ,
			(float)(nx.si[GLOBAL].xmtint - ox.si[GLOBAL].xmtint)/tdiff[GLOBAL] * HZ,
			(float)(nx.si[GLOBAL].mdmint - ox.si[GLOBAL].mdmint)/tdiff[GLOBAL] * HZ);
	
			ax.si[GLOBAL].rawch += nx.si[GLOBAL].rawch - ox.si[GLOBAL].rawch;
			ax.si[GLOBAL].canch += nx.si[GLOBAL].canch - ox.si[GLOBAL].canch;
			ax.si[GLOBAL].outch += nx.si[GLOBAL].outch - ox.si[GLOBAL].outch;
			ax.si[GLOBAL].rcvint += nx.si[GLOBAL].rcvint - ox.si[GLOBAL].rcvint;
			ax.si[GLOBAL].xmtint += nx.si[GLOBAL].xmtint - ox.si[GLOBAL].xmtint;
			ax.si[GLOBAL].mdmint += nx.si[GLOBAL].mdmint - ox.si[GLOBAL].mdmint;
			break;
		  case 'b':
			tsttab();
			if (nx.si[GLOBAL].lread - ox.si[GLOBAL].lread)
				hr = (((float)(nx.si[GLOBAL].lread - ox.si[GLOBAL].lread) -
			  (float)(nx.si[GLOBAL].bread - ox.si[GLOBAL].bread))/
			  (float)(nx.si[GLOBAL].lread - ox.si[GLOBAL].lread) * 100.0);
			else
				hr = 0.0;
			if (nx.si[GLOBAL].lwrite - ox.si[GLOBAL].lwrite)
				hw = (((float)(nx.si[GLOBAL].lwrite - ox.si[GLOBAL].lwrite) -
			  (float)(nx.si[GLOBAL].bwrite - ox.si[GLOBAL].bwrite))/
			  (float)(nx.si[GLOBAL].lwrite - ox.si[GLOBAL].lwrite) * 100.0);
			else
				hw = 0.0;
			printf(" %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
			(float)(nx.si[GLOBAL].bread - ox.si[GLOBAL].bread)/tdiff[GLOBAL] * HZ,
			(float)(nx.si[GLOBAL].lread - ox.si[GLOBAL].lread)/tdiff[GLOBAL] * HZ,
			hr,
			(float)(nx.si[GLOBAL].bwrite - ox.si[GLOBAL].bwrite)/tdiff[GLOBAL] * HZ,
			(float)(nx.si[GLOBAL].lwrite - ox.si[GLOBAL].lwrite)/tdiff[GLOBAL] * HZ,
			hw,  
			(float)(nx.si[GLOBAL].phread - ox.si[GLOBAL].phread)/tdiff[GLOBAL] * HZ,
			(float)(nx.si[GLOBAL].phwrite - ox.si[GLOBAL].phwrite)/tdiff[GLOBAL] * HZ);

			ax.si[GLOBAL].bread   += nx.si[GLOBAL].bread 
							  - ox.si[GLOBAL].bread;
			ax.si[GLOBAL].bwrite  += nx.si[GLOBAL].bwrite 
							 - ox.si[GLOBAL].bwrite;
			ax.si[GLOBAL].lread   += nx.si[GLOBAL].lread 
							  - ox.si[GLOBAL].lread;
			ax.si[GLOBAL].lwrite  += nx.si[GLOBAL].lwrite
							 - ox.si[GLOBAL].lwrite;
			ax.si[GLOBAL].phread  += nx.si[GLOBAL].phread  
							- ox.si[GLOBAL].phread;
			ax.si[GLOBAL].phwrite += nx.si[GLOBAL].phwrite 
							- ox.si[GLOBAL].phwrite;
			break;
		  case 'k':
			tsttab();
			printf(" %8.0f %8ld %8.0f\n",
			      (float)(nx.si[GLOBAL].ksched - ox.si[GLOBAL].ksched) / tdiff[GLOBAL] * HZ,
			      (nx.si[GLOBAL].koverf - ox.si[GLOBAL].koverf),
			      (float)(nx.si[GLOBAL].kexit - ox.si[GLOBAL].kexit)   / tdiff[GLOBAL] * HZ );
	
			/*
			 *  a3298
			 */
			ax.si[GLOBAL].ksched += nx.si[GLOBAL].ksched - ox.si[GLOBAL].ksched;
			ax.si[GLOBAL].koverf += nx.si[GLOBAL].koverf - ox.si[GLOBAL].koverf;
			ax.si[GLOBAL].kexit  += nx.si[GLOBAL].kexit  - ox.si[GLOBAL].kexit;
			break;
		  case 'v':
			tsttab();
			sprintf( buf1, "%d/%d", nx.szproc,  nx.mszproc  );
			sprintf( buf2, "%d/%d", nx.szinode, nx.mszinode );
			sprintf( buf3, "%d/%d", nx.szfile,  nx.mszfile  );
			if (newflg) {
			  sprintf( buf4, "%d/%d", nx.szthread,  nx.mszthread  );
			  printf("  %-10s  %-10s  %-10s  %-10s\n",
				buf1, buf2, buf3, buf4);
			}
			else {
			  printf("  %-10s   %-10s   %-10s\n",
				buf1, buf2, buf3);
			}
			break;
		  case 'c':
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
                                        printf("  %c ",'-');
                                }
				printf(" %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
				(float)(nx.si[ind].syscall - ox.si[ind].syscall)/tdiff[ind] *HZ,
				(float)(nx.si[ind].sysread - ox.si[ind].sysread)/tdiff[ind] *HZ,
				(float)(nx.si[ind].syswrite - ox.si[ind].syswrite)/tdiff[ind] *HZ,
				(float)(nx.si[ind].sysfork - ox.si[ind].sysfork)/tdiff[ind] *HZ,
				(float)(nx.si[ind].sysexec - ox.si[ind].sysexec)/tdiff[ind] *HZ,
				(float)(nx.si[ind].readch - ox.si[ind].readch)/tdiff[ind] * HZ,
				(float)(nx.si[ind].writech - ox.si[ind].writech)/tdiff[ind] * HZ);

				ax.si[ind].syscall += nx.si[ind].syscall - ox.si[ind].syscall;
				ax.si[ind].sysread += nx.si[ind].sysread - ox.si[ind].sysread;
				ax.si[ind].syswrite += nx.si[ind].syswrite - ox.si[ind].syswrite;
				ax.si[ind].sysfork += nx.si[ind].sysfork - ox.si[ind].sysfork;
				ax.si[ind].sysexec += nx.si[ind].sysexec - ox.si[ind].sysexec;
				ax.si[ind].readch += nx.si[ind].readch - ox.si[ind].readch;
				ax.si[ind].writech += nx.si[ind].writech - ox.si[ind].writech;
			}
			break;
		  case 'w':
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
                                        printf("  %c ",'-');
                                }
				printf(" %7.0f\n",
					(float)(nx.si[ind].pswitch - ox.si[ind].pswitch)/tdiff[ind] * HZ);
				ax.si[ind].pswitch += nx.si[ind].pswitch - ox.si[ind].pswitch;
			}
			break;
		  case 'a':
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
                                        printf("  %c ",'-');
                                }
				printf(" %7.0f %10.0f %8.0f\n",
					(float)(nx.si[ind].iget   - ox.si[ind].iget)   / tdiff[ind] * HZ,
					(float)(nx.si[ind].namei  - ox.si[ind].namei)  / tdiff[ind] * HZ,
					(float)(nx.si[ind].dirblk - ox.si[ind].dirblk) / tdiff[ind] * HZ );
		
				ax.si[ind].iget   += nx.si[ind].iget   - ox.si[ind].iget;
				ax.si[ind].namei  += nx.si[ind].namei  - ox.si[ind].namei;
				ax.si[ind].dirblk += nx.si[ind].dirblk - ox.si[ind].dirblk;
			}
			break;
		  case 'q':
			tsttab();
			if ((nx.si[GLOBAL].runocc - ox.si[GLOBAL].runocc) == 0)
				printf(" %7s %7s", "  ", "  ");
			else {
				printf(" %7.1f %7.0f",
					(float)(nx.si[GLOBAL].runque - ox.si[GLOBAL].runque) /
					(float)(nx.si[GLOBAL].runocc - ox.si[GLOBAL].runocc),
					/* runque is always occupied */
					MIN((float)(nx.si[GLOBAL].runocc - ox.si[GLOBAL].runocc) /
					(nx.ts - ox.ts)* 100.0, 100.0));
				ax.si[GLOBAL].runque += nx.si[GLOBAL].runque - ox.si[GLOBAL].runque;
				ax.si[GLOBAL].runocc += nx.si[GLOBAL].runocc - ox.si[GLOBAL].runocc;
			}

			if ((nx.si[GLOBAL].swpocc - ox.si[GLOBAL].swpocc) == 0)
				printf(" %7s %7s\n","  ","  ");
			else {
				printf(" %7.1f %7.0f\n",
					(float)(nx.si[GLOBAL].swpque - ox.si[GLOBAL].swpque) /
					(float)(nx.si[GLOBAL].swpocc - ox.si[GLOBAL].swpocc),
					MIN((float)(nx.si[GLOBAL].swpocc - ox.si[GLOBAL].swpocc) /
					(nx.ts - ox.ts)* 100.0, 100.0));
				ax.si[GLOBAL].swpque += nx.si[GLOBAL].swpque - ox.si[GLOBAL].swpque;
				ax.si[GLOBAL].swpocc += nx.si[GLOBAL].swpocc - ox.si[GLOBAL].swpocc;
			}
			break;
		  case 'm':
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
                                        printf("  %c ",'-');
                                }
				printf(" %7.2f %7.2f\n",
					(float)(nx.si[ind].msg - ox.si[ind].msg)/tdiff[ind] * HZ,
					(float)(nx.si[ind].sema - ox.si[ind].sema)/tdiff[ind] * HZ);

				ax.si[ind].msg += nx.si[ind].msg - ox.si[ind].msg;
				ax.si[ind].sema += nx.si[ind].sema - ox.si[ind].sema;
			}
			break;
		  case 'r':
			tsttab();
			printf(" %7ld %7.2f %7.2f %7.2f\n",
				nx.pi.freeslots,
				(float)(nx.pi.cycles - ox.pi.cycles)/tdiff[GLOBAL] * HZ,
				(float)(nx.pi.faults - ox.pi.faults)/tdiff[GLOBAL] * HZ,
				(float)(nx.pi.otherdiskios - ox.pi.otherdiskios)/tdiff[GLOBAL] * HZ);
			ax.pi.freeslots += nx.pi.freeslots;
			ax.pi.cycles += nx.pi.cycles - ox.pi.cycles;
			ax.pi.faults += nx.pi.faults - ox.pi.faults;
			ax.pi.otherdiskios += nx.pi.otherdiskios - ox.pi.otherdiskios;
			break;
		}
	}
	if (jj > 2)  printf("\n");
}

/**********************************************************************/
/* NAME:  prtavg                                                      */
/* FUNCTION:  Print average routine.                                  */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static prtavg()
{
	register int  ii, kk;
	int    jj=0;
	char   ccc;
	float  hw, hr;
	int ind;
	float div;

	for (ind = 0; ind <= N_PROCESSOR ; ind++) {
		if (!tblcpu[ind])
			continue;
		tdiff[ind] = ax.si[ind].cpu[0] + ax.si[ind].cpu[1]
				 + ax.si[ind].cpu[2] + ax.si[ind].cpu[3];
		if (ind == GLOBAL ) {
			tdiff[ind] = tdiff[ind] /sa_head.nbcpu;
		}
		if (tdiff[ind] <= 0.0)
			return;
	}
	printf("\n");

	while ((ccc = fopt[jj++]) != (char)NULL) {
		switch (ccc) {
		  case 'u':
			printf("%-8s",MSGSTR(AVERAGE, "Average"));
			tabflg = 1;
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
                                        printf("  %c ",'-');
                                }
				if (ind == GLOBAL){
                                        div = tdiff[ind] * sa_head
.nbcpu;
                                }
                                else {
                                        div = tdiff[ind];
                                }
				printf(" %7.0f %7.0f %7.0f %7.0f\n",
					(float)ax.si[ind].cpu[1]/div * 100.0,
					(float)ax.si[ind].cpu[2]/div * 100.0,
					(float)ax.si[ind].cpu[3]/div * 100.0,
					(float)ax.si[ind].cpu[0]/div * 100.0);
			}
			break;
		  case 'y':
			printf("%-8s %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				MSGSTR(AVERAGE, "Average"),
				(float)ax.si[GLOBAL].rawch/tdiff[GLOBAL] *HZ,
				(float)ax.si[GLOBAL].canch/tdiff[GLOBAL] *HZ,
				(float)ax.si[GLOBAL].outch/tdiff[GLOBAL] *HZ,
				(float)ax.si[GLOBAL].rcvint/tdiff[GLOBAL] *HZ,
				(float)ax.si[GLOBAL].xmtint/tdiff[GLOBAL] *HZ,
				(float)ax.si[GLOBAL].mdmint/tdiff[GLOBAL] *HZ);
			break;
		  case 'b':
			if (ax.si[GLOBAL].lwrite)
				hw = (float)(ax.si[GLOBAL].lwrite - ax.si[GLOBAL].bwrite) / 
					(float)(ax.si[GLOBAL].lwrite) * 100.0;
			else
				hw = 0.0;
			if (ax.si[GLOBAL].lread)
				hr = (float)(ax.si[GLOBAL].lread - ax.si[GLOBAL].bread) / 
					(float)(ax.si[GLOBAL].lread) * 100.0;
			else
				hr = 0.0;
			ax.si[GLOBAL].lread ? (hw = ax.si[GLOBAL].lwrite) : (hw = 0);
			printf("%-8s %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f %7.0f\n",
				MSGSTR(AVERAGE, "Average"),
				(float)ax.si[GLOBAL].bread/tdiff[GLOBAL] *HZ,
				(float)ax.si[GLOBAL].lread/tdiff[GLOBAL] *HZ,
				hr,
				(float)ax.si[GLOBAL].bwrite/tdiff[GLOBAL] *HZ,
				(float)ax.si[GLOBAL].lwrite/tdiff[GLOBAL] *HZ,
				hw,
				(float)ax.si[GLOBAL].phread/tdiff[GLOBAL] *HZ,
				(float)ax.si[GLOBAL].phwrite/tdiff[GLOBAL] *HZ);
			break;
		  case 'k':
			printf("%-8s %8.0f %8ld %8.0f\n",
				MSGSTR(AVERAGE, "Average"),
				(float)ax.si[GLOBAL].ksched / tdiff[GLOBAL] * HZ,
				ax.si[GLOBAL].koverf,
				(float)ax.si[GLOBAL].kexit  / tdiff[GLOBAL] * HZ );
			break;
		  case 'r':
			printf("%-8s %7.0f %7.0f %7.0f %7.0f\n",
				MSGSTR(AVERAGE, "Average"),
				(float)ax.pi.freeslots/(n-1),
				(float)ax.pi.cycles/tdiff[GLOBAL] *HZ,
				(float)ax.pi.faults/tdiff[GLOBAL] *HZ,
				(float)ax.pi.otherdiskios/tdiff[GLOBAL] *HZ);
			break;
		  case 'v':
			break;
		  case 'c':
			printf("%-8s",MSGSTR(AVERAGE, "Average"));
			tabflg = 1;
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
                                        printf("  %c ",'-');
                                }
				printf(" %7.0f %7.0f %7.0f %7.2f %7.2f %7.0f %7.0f\n",
					(float)ax.si[ind].syscall  / tdiff[ind] * HZ,
					(float)ax.si[ind].sysread  / tdiff[ind] * HZ,
					(float)ax.si[ind].syswrite / tdiff[ind] * HZ,
					(float)ax.si[ind].sysfork  / tdiff[ind] * HZ,
					(float)ax.si[ind].sysexec  / tdiff[ind] * HZ,
					(float)ax.si[ind].readch   / tdiff[ind] * HZ,
					(float)ax.si[ind].writech  / tdiff[ind] * HZ );
			}
			break;
		  case 'w':
			printf("%-8s",MSGSTR(AVERAGE, "Average"));
			tabflg = 1;
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
                                        printf("  %c ",'-');
                                }
				printf(" %7.0f\n",
					(float)ax.si[ind].pswitch/tdiff[ind] * HZ);
			}
			break;
		  case 'a':
			printf("%-8s",MSGSTR(AVERAGE, "Average"));
			tabflg = 1;
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
                                        printf("  %c ",'-');
                                }
				printf( " %7.0f %10.0f %8.0f\n",
					(float)ax.si[ind].iget   / tdiff[ind] * HZ,
					(float)ax.si[ind].namei  / tdiff[ind] * HZ,
					(float)ax.si[ind].dirblk / tdiff[ind] * HZ );
			}
			break;
		  case 'q':
			if (ax.si[GLOBAL].runocc == 0)
				printf("%-8s %7s %7s ", MSGSTR(AVERAGE, "Average"), "  ","  ");
			else {
				printf("%-8s %7.1f %7.0f ",
				MSGSTR(AVERAGE, "Average"),
				(float)((float)ax.si[GLOBAL].runque / (float)ax.si[GLOBAL].runocc),
				(float)ax.si[GLOBAL].runocc / tdiff[GLOBAL] * HZ * 100.0);
			}
			if (ax.si[GLOBAL].swpocc == 0)
				printf("%7s %7s\n","  ","  ");
			else {
				printf("%7.1f %7.0f\n",
				(float)((float)ax.si[GLOBAL].swpque / (float)ax.si[GLOBAL].swpocc),
				(float)ax.si[GLOBAL].swpocc / tdiff[GLOBAL] * HZ  * 100.0);
	
			}
			break;
		  case 'm':
			printf("%-8s",MSGSTR(AVERAGE, "Average"));
			tabflg = 1;
			for (ind = 0; ind <= N_PROCESSOR; ind++) {
				if (!tblcpu[ind])
					continue;
				tsttab();
				if (ind != GLOBAL)
					printf("  %d ",ind);
				if ((ind == GLOBAL) && (pallflg)) {
                                        printf("  %c ",'-');
                                }
				printf(" %7.2f %7.2f\n",
					(float)ax.si[ind].msg/tdiff[ind] * HZ,
					(float)ax.si[ind].sema/tdiff[ind] * HZ);
			}
			break;
		}
	}
}

/**********************************************************************/
/* NAME:  pillarg                                                     */
/* FUNCTION:  Error exit routines.                                    */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static pillarg(list,opt)
char *list;
char  opt;
{
	fprintf(stderr,MSGSTR(ILLEGAL, 
		"%s -- illegal argument for option  %c\n"), list, opt);
	exit(1);
}

/**********************************************************************/
/* NAME:  perrexit                                                    */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static perrexit()
{
	perror("sar");
	exit(1);
}

/**********************************************************************/
/* NAME:  pmsgexit                                                    */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/
static pmsgexit(s)
char  *s;
{
	fprintf( stderr, "%s\n", s );
	fprintf( stderr, MSGSTR(SARUSAGE1,
	"usage: sar { -A | [-a][-b][-c][-k][-m][-q][-r][-u][-v][-w][-y] } \n") );
	fprintf( stderr, MSGSTR(SARUSAGE2,
	"           [-s hh[:mm[:ss]]] [-e hh[:mm[:ss]]] \n") );
	fprintf( stderr, MSGSTR(SARUSAGE3,
	"           [-P processor_id[,...] | ALL] \n") );
	fprintf( stderr, MSGSTR(SARUSAGE4,
	"           [-f file] [-i seconds] [-o file] [interval [number]] \n") );
	exit(1);
}


/**********************************************************************/
/* NAME:  init_cpu                                                    */
/* FUNCTION: initialize the list of the CPU slected                   */
/* RETURN VALUE: -2 if the CPU number is not a digit                  */
/*		 -3 if bad CPU number ; 0 else                        */
/**********************************************************************/

static int init_cpu (list)
char *list;
{
	char *cp;
	int   i;
	int   cpu;
	int rc;
	char inter[N_PROCESSOR];

	rc = 0;
	tblcpu[GLOBAL] = 0;
	if (debug)
		printf("- init_cpu - list = %s \n",list);
	strcpy(inter,list);
	for(cp = (char *)strtok(inter,", \n"); cp != NULL ; cp = (char *)strtok(0,", \n")) {
		if (strcmp(cp,"ALL") == 0) {
			for (i = 0; i < sa_head.nbcpu ; i++) {
				tblcpu [i] = 1;
			}
    			tblcpu [GLOBAL] = 1;  
			pallflg++;
			break;
		}
		 if (isdigit((int)*cp) == 0) {
			rc = -2;
			break;
		}
		cpu = atoi(cp);
		if ((cpu < 0) || (cpu > (sa_head.nbcpu -1))) {
			rc = -3;
			break;
		}
		else 
			tblcpu[cpu] = 1;
	}
	if (debug) {
		printf("- init_cpu - rc= %d \n",rc);
	}
	return(rc);
}

/**********************************************************************/
/* NAME:  fill_nx                                                     */
/* FUNCTION: fill the structure nx with the old structure nx          */
/* RETURN VALUE: none                                                 */
/**********************************************************************/

static void fill_nx()
{
	int i;

	for (i=0; i<N_PROCESSOR; i++)
		nx.si[i].cpu[0] = CPU_STOPPED;

	nx.si[GLOBAL] = nx_old.sa_old_si;
	nx.pi         = nx_old.sa_old_pi; 
	nx.szinode    = nx_old.sa_old_szinode;
	nx.szfile     = nx_old.sa_old_szfile;
	nx.sztext     = nx_old.sa_old_sztext;
	nx.szproc     = nx_old.sa_old_szproc;
	nx.szthread   = 0;
	nx.mszinode   = nx_old.sa_old_mszinode;
	nx.mszfile    = nx_old.sa_old_mszfile;
	nx.msztext    = nx_old.sa_old_msztext;
	nx.mszproc    = nx_old.sa_old_mszproc;
	nx.mszthread  = 0;
	nx.ts         = nx_old.sa_old_ts;
}
	
