static char sccsid[] = "@(#)41	1.9  src/bos/usr/sbin/crash/POWER/pstat.c, cmdcrash, bos411, 9428A410j 3/31/94 16:14:40";
/*
 * COMPONENT_NAME: (CMDCRASH) Crash
 *
 * FUNCTIONS: pstat
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

#include	<signal.h>
#include	<sys/vmker.h>
#include	"crash.h"



/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */


extern char *namelist, *dumpfile;
extern char sbuf[];
extern struct var v;
extern cpu_t ncpus;


pstat(argc, argv)
int	argc;
char	**argv;
{
	register int	arg;
	register int	end_arg = argc;
	int	threadslot, procslot;
	int aflg = 0;
	int Aflg = 0;
	int fflg = 0;
	int iflg = 0;
	int pflg = 0;
	int Pflg = 0;
	int sflg = 0;
	int tflg = 0;
	int uflg = 0;
	int Uflg = 0;
	int Tflg = 0;
	int Sflg = 0;
	struct	vmkerdata vmkerdata;
	if (argc == 1) {
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_1, 
		"USAGE: pstat [-a] [-A] [-f] [-i] [-p] [-P] [-s] [-t] [-u procslot] [-U threadslot] [-S] [-T] [[System] CoreFile]\n") );
		exit(0);
	}
	if (*argv[argc-1] != '-' && !isdigit(*argv[argc-1])) {
		dumpfile = argv[argc - 1];
		end_arg -= 1;
		if (argc > 2) {
			if (*argv[argc-2] != '-' && 
			    !isdigit(*argv[argc-2])) {
				namelist = argv[argc - 2];
				end_arg -= 1;
			}
		}
	}	
	for(arg = 1; arg < end_arg;) {
		if ((argv[arg][0] != '-') &&  (argv[arg][2] != '\0')) 
			fatal( catgets(scmc_catd, MS_crsh, PSTAT_MSG_1,
			"USAGE: pstat [-a] [-A] [-f] [-i] [-p] [-P] [-s] [-t] [-u procslot] [-U threadslot] [-S] [-T] [[System] CoreFile]\n") ); 
			switch (argv[arg][1]) {
			case 'T':
				Tflg++;
				arg++;
				break;
			case 'a':
				aflg++;
				arg++;
				break;
			case 'A':
				Aflg++;
				arg++;
				break;
			case 'f':
				fflg++;
				arg++;
				break;
			case 'i':
				iflg++;
				arg++;
				break;
			case 'p':
				pflg++;
				arg++;
				break;
			case 'P':
				Pflg++;
				arg++;
				break;
			case 's':
				sflg++;
				arg++;
				break;
			case 't':
				tflg++;
				arg++;
				break;
			case 'u':
				uflg++;
				arg++;
				if (arg == end_arg)
					fatal( catgets(scmc_catd, MS_crsh, PSTAT_MSG_1, 
					"USAGE: pstat [-a] [-A] [-f] [-i] [-p] [-P] [-s] [-t] [-u procslot] [-U threadslot] [-S] [-T] [[System] CoreFile]\n") );
				if (!isdigit(*argv[arg]))
					fatal( catgets(scmc_catd, MS_crsh, PSTAT_MSG_1, 
				    "USAGE: pstat [-a] [-A] [-f] [-i] [-p] [-P] [-s] [-t] [-u procslot] [-U threadslot] [-S] [-T] [[System] CoreFile]\n") );
				procslot = atoi(argv[arg++]); 
				break;
			case 'U':
				Uflg++;
				arg++;
				if (arg == end_arg)
					fatal( catgets(scmc_catd, MS_crsh, PSTAT_MSG_1, 
					"USAGE: pstat [-a] [-A] [-f] [-i] [-p] [-P] [-s] [-t] [-u procslot] [-U threadslot] [-S] [-T] [[System] CoreFile]\n") );
				if (!isdigit(*argv[arg]))
					fatal( catgets(scmc_catd, MS_crsh, PSTAT_MSG_1, 
					"USAGE: pstat [-a] [-A] [-f] [-i] [-p] [-P] [-s] [-t] [-u procslot] [-U threadslot] [-S] [-T] [[System] CoreFile]\n") );
				threadslot = atoi(argv[arg++]); 
				break;
			case 'S':
				Sflg++;
				arg++;
				break;
			default:
				fatal( catgets(scmc_catd, MS_crsh, PSTAT_MSG_1, 
				"USAGE: pstat [-a] [-A] [-f] [-i] [-p] [-P] [-s] [-t] [-u procslot] [-U threadslot] [-S] [-T] [[System] CoreFile]\n") );
				break;
		}
	}
	setbuf(stdout,sbuf);
	init();
	signal(SIGINT,SIG_DFL);

	if (Tflg) {
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_2, "SYSTEM VARS:\n\n") );
		prvar();
		printf("\n\n");
	}
	if (aflg) {
        printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_3, "PROC TABLE:\n\n") );
		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_61, 
		"SLT ST    PID   PPID   PGRP   UID  EUID  TCNT  NAME\n") );
        for(arg = 0; arg < (int)v.ve_proc; arg++)
                prproc(arg, 0, 0, 0);
        printf("\n\n");
    }
	if (Aflg) {
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_4, "THREAD TABLE:\n\n") );
        printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_65, 
		"SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME\n") );
		for (arg = 0; arg < (int)v.ve_thread; arg++)
			prthread(SHORT, ALL, arg);
		printf("\n\n");
	}
	if (fflg) {	
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_5, "FILE TABLE:\n\n") );
		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_59, " SLOT    FILE        REF     TYPE            ADDRESS      FLAGS\n") );
		for(arg = 0; arg < (int)v.ve_file; arg++)
			prfile(arg, 0);
		printf("\n\n");
	}
	if (iflg) {
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_7, "INODE TABLE:\n\n") );
		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_13, 
		" ADDRESS   MAJ  MIN INUMB REF LINK  UID  GID   SIZE   MODE  SMAJ SMIN FLAGS\n") );
		prhinode(0, NULL);
		printf("\n\n");
	}
	if (pflg) {
        printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_3, "PROC TABLE:\n\n") );
		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_61, 
		"SLT ST    PID   PPID   PGRP   UID  EUID  TCNT  NAME\n") );
        for(arg=0; arg < (int)v.ve_proc; arg++)
                prproc(arg, 0, 1, 0);
        printf("\n\n");
    }
	if (Pflg) {
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_4, "THREAD TABLE:\n\n") );
                printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_65, 
				"SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME\n") );
		for (arg = 0; arg < (int)v.ve_thread; arg++)
			prthread(SHORT, RUNNING, arg);
		printf("\n\n");
	}
	if (sflg) {
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_8, "PAGE SPACE:\n\n") );
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_9, "\t  USED PAGES   FREE PAGES\n") );	
		if (readmem(&vmkerdata,SYM_VALUE(Vmker), 
		    sizeof (struct vmkerdata)) != sizeof (struct vmkerdata)) {
			printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_10, 
			"0452-326: Cannot read vmkerdata structure from address 0x%8x\n") ,
			SYM_VALUE(Vmker));
			return(-1);
		}
		printf("\t%9d    %9d\n",vmkerdata.numpsblks - 
			vmkerdata.psfreeblks, vmkerdata.psfreeblks);
		printf("\n\n");
	}
	if (tflg) {
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_11, "TTY STRUCTS:\n\n") );
		call_tty("o");
		printf("\n\n");
	}
	if (uflg) {
        printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_12, "USER AREA:\n\n") );
        pruareap(procslot,0);
        printf("\n\n");
    }
	if (Uflg) {
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_13, "UTHREAD AREA:\n\n") );
		pruareat(threadslot,0); 
        	printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_12, "USER AREA:\n\n") );
        	pruareap(associated_proc(threadslot),0);
		printf("\n\n");
	}
	if (Sflg) {
		printf( catgets(scmc_catd, MS_crsh, PSTAT_MSG_14, "STATUS OF PROCESSORS:\n\n"));
	printf(catgets(scmc_catd, MS_crsh, MAIN_MSG_80, 
		"CPU     TID  TSLOT     PID  PSLOT  PROC_NAME\n"));
        for (arg=0; arg< ncpus; arg++)
					prstatus(arg);
        printf("\n\n");
	}
}

