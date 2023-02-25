 static char sccsid[] = "@(#)18  1.23.1.23  src/bos/usr/sbin/crash/main.c, cmdcrash, bos411, 9438B411a 9/20/94 11:57:28";
/*
*/
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: main,prall,sigint,token,prompt,prkfp,validigit
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

/*              include file for message texts          */
#include "crash_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include	"crash.h"
#include	"cmd.h"
#include	<locale.h>
#include	<setjmp.h>
#include	<ctype.h>
#include	<a.out.h>
#include	<sys/proc.h>
#include	<sys/errno.h>

void
usage()
{
	fatal( catgets(scmc_catd, MS_crsh, F_MAIN_MSG_99,
	    "USAGE: crash [-a] [-i includefile] "
	    "[systemimagefile [kernelfile]]\n"));
}

#define	readinto(number)	sscanf(arg, NBPW==4 ? "%x" : "%o", &number)
char	symnamebuf[80];		/* for search() */
void showdump();

int		mem, kmem;
unsigned	Kfp;
char	*namelist = "/unix";
int	nmoff, nmlen;
char	*dumpfile = "/dev/mem";

char	line[256], *p = line;
char	sbuf[BUFSIZ];		/* For setbuf() */
struct	var	v;
int	tok;
jmp_buf	jmp;
struct	nlist	*File, *Inode, *Mount, *Swap, *Core, *Proc, *Sbuf,
 		*Sys, *Time, *Panic,  *V, *Sbrpte, *D_rsav, *Hinode,
		*Buf, *End, *Lbolt, *Dmpstk, *Gpa_vnode, *Nhino,
		*Rootvfs,*g_kxsrval, *Hbuf, *Vmker, *Mbuf, *MbufC;
struct	nlist	*Segid0, *Dmpsegs, *Dumpflag, *_kpchk_psb, *_pcregs,
		*Kernel_heap, *Pinned_heap, *K_anchor, *Ifnet;
struct nlist	*Thread, *Ublock, *Ppd, *Sysconfig, *Dbp_fmodsw, 
		*Dbp_dmodsw, *Dbp_sth_open_streams, *Dbp_streams_runq;

int dumpflag = 0;  /* non zero if we are working on a dump */

#ifdef PORTING
struct exec filehdr;
#endif

int 	strtblsiz;		/* Size of string table (if present) */

struct  cm  cmstr, *cm = &cmstr;
struct proc global_proc;
struct thread global_thread;
struct uthread x_thread;
struct	user	x;
int proc_stored    = NOTSTORED;
int thread_stored  = NOTSTORED;
cpu_t selected_cpu;
cpu_t ncpus;

/*
 * NAME: main
 *                                                                    
 * FUNCTION: This is the main command processing loop for the crash utility
 */  
main(argc, argv)
	int	argc;
	char	**argv;
{
	char	*token();
	long	atol();
	int	sigint();
	register  struct  tsw	*tp;
	register  char  *c, *arg;
	char	*strct;
	char	*editor = NULL;
	char	*dbgfile = NULL;
	char	*includes[512];
	char	tty_args[256];
 	char	*iarg[4];
	int	aflg = 0, iflg = 0;
 	int	iblocks;
	int	i, cnt, units, r, mk, prdef = (NBPW==4 ? HEX : OCTAL);
	int	rc;
	unsigned long addr;
	struct	syment	*nmsrch(), *search(), *sp;
	struct	prmode	*prptr;
	struct  cm	cmp;
	extern	unsigned  ttycnt;
	int	mst_only = 0;		/* 1=print only mst,not all of uarea */
	char	*name;
	struct 	nlist 	nlist;
	pid_t 	pid;
	int opt;
	extern int optind;
	extern char *optarg;
	short	format, status;
	int	thread_slot, proc_slot;
	ulong	proc_addr;
	struct thread	thread;
	struct proc 	proc;
	int	go_on;

	/* Open NLS catalogue for this command */
	setlocale(LC_ALL,"");
	scmc_catd=catopen(MF_CRASH,0); 

	/* Get the name by which we were called */
	name = &argv[0][strlen(argv[0]) - 5];

	/* if called as "pstat", use the non-interactive interface. */
	if (strcmp(name,"pstat") == 0) {
		pstat(argc,argv);
		exit(0);
	}

	/* process arguments */
	while ((opt = getopt(argc, argv, "ae:i:")) != -1) {
		switch (opt) {
		case 'a':
			aflg++;
		break;
		case 'i':
			/* include file name */
			includes[iflg++] = optarg;
			break;
		case 'e':
			editor = optarg;
			break;
		case '?':
			usage();
		}
	}
	switch (argc - optind) {
	case 0:
		break;
	case 1:
		dumpfile = argv[optind];
		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_7,
		    "Using /unix as the default namelist file.\n") );
		break;
	case 2:
		dumpfile = argv[optind];
		namelist = argv[optind+1];
		break;
	default:
		usage();
	}

	if (editor == NULL)
		editor = getenv("CRASHED");
	if (editor == NULL)
		editor = "/bin/pg -e";

	/* build debug info if needed */
	if (iflg) {
		int i, rc;
		FILE *fp;
		char c_dbgfile[MAXPATH];
		char cmd[4096];

		dbgfile = tmpnam((char *)0);
		sprintf(c_dbgfile, "%s.c", dbgfile);
		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_100,
			"Processing include files...") );
		fflush(stdout);
		if ((fp = fopen(c_dbgfile, "w")) == NULL) {
			perror(name);
			exit(errno);
		}
		for (i=0; i<iflg; i++)
			fprintf(fp, "#include <%s>\n", includes[i]);
		fprintf(fp, "main() { ; }\n");
		fclose(fp);
		sprintf(cmd, "cc -g -qdbxextra -o %s -I . %s",
			dbgfile, c_dbgfile);
		rc = system(cmd);
		rc >>= 8;
		printf("\n");
		unlink(c_dbgfile);
		if (rc != 0)
			exit(rc);
	}

	setbuf(stdout,sbuf);

	/*
	 *  Initialize the symbol values and open the files
	 *  that crash will use.
	 */
	init();
	cm->cm_thread = -1;

	if (setjmp(jmp))
		exit(0);

	/*
	 * If the -a option was used then print a exaustive set of data
	 * from the various commands to stdout.
	 */
	if (aflg) {
		prall();
		exit(0);
	}

	/* Come back to this point on errors */
	setjmp(jmp);
	for(;;) {
		fflush(stdout);

		/* Get a token from the keyboard */
		if((c = token()) == NULL)
			continue;

		/* Compare the token to the list of valid commands */
		for(tp = t; tp->t_sw != 0; tp++)
			if(strcmp(tp->t_nm, c) == 0)
				break;

		/* Execute the appropriate code for the command */
		switch(tp->t_sw) {

		default:
			/* The token was not a valid command */
			prompt( catgets(scmc_catd, MS_crsh, P_MAIN_MSG_8, "eh?\n") );
			while(token() != NULL);
			continue;

		case MST:
			if((arg = token()) == NULL)
				prmst(0);
			else do {
                            if((addr = symtoaddr(arg)) == -1L) {
                                printf( catgets(scmc_catd, MS_crsh,
                                        MAIN_MSG_35, "Symbol not found.\n") );
                            }
                            else prmst(addr);
			} while((arg = token()) !=NULL);
			continue;

		case FILES:
			/* display the file table */
			printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_59, " SLOT    FILE        REF     TYPE            ADDRESS      FLAGS\n") );
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < (int)v.ve_file; cnt++)
					prfile(cnt, 0);
			else do {
				prfile(atoi(arg), 1);
			} while((arg = token()) != NULL);
			continue;

		case Q:
			/* Display the crash usage and subcommands */
			printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_10, "Dump Analyser\n"));
			printf( catgets(scmc_catd, MS_crsh, F_MAIN_MSG_99, 
			"USAGE: crash [-a] [-i includefile] [systemimagefile [kernelfile]]\n"));
			printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_12, "Available commands:\n\n"));
			for(tp = t; tp->t_sw != 0; tp++)
				if(tp->t_dsc != 0)
					printf("%s\t%s\n",tp->t_nm,tp->t_dsc);
			while(token() != NULL);
			continue;

		case STATS:
			/* Display the system stats */
			prstat();
			while(token() != NULL);
			continue;

		case SYMPTOM:
			symptom();
			continue;

		case VAR:
			/* Display the system variables */
			prvar();
			while(token() != NULL);
			continue;

		case INODE:
			/* Display the inodes on the system hash table */
			printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_13, " ADDRESS   MAJ  MIN INUMB REF LINK  UID  GID   SIZE   MODE  SMAJ SMIN FLAGS\n") );
			iblocks = FALSE;
			if((arg = token()) == NULL) {
				prhinode(iblocks, NULL);
				continue;
			}
			if(*arg == '-') {
				iblocks = TRUE;
				if((arg = token()) == NULL) {
					prhinode(iblocks, NULL);
					continue;
				}
			}
			iarg[0] = arg;
			for (cnt = 1; cnt < 3; cnt++)
				if((iarg[cnt] = token()) == NULL)
					break;
			if(cnt != 3) {
			     printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_14, "USAGE: inode [-] [<maj> <min> <inode>]\n") );
				continue;
			}
			prhinode(iblocks, iarg);
			while(token() != NULL);
			continue;

		case TOUT:
			/* Display the system TRB structures */
			prcallout();
			while(token() != NULL);
			continue;
		case TCBLK:
			/* Display the mstsave structure from the uthread area */
			mst_only = 1;
		case UAREA:
			/* 
			 * Display the user area with formatted output.
			 * 	This command uses the process table slot
			 *	number as input and not a process id.
			 */
			if((arg = token()) == NULL) {
				pruareat(CURTHREAD,mst_only);  /* Use _curthread */
				if (!mst_only)
					pruareap(CURPROC);
			}
			else do {
				pruareat(atoi(arg),mst_only);
				if (!mst_only)
					pruareap(associated_proc(atoi(arg)));
			} while((arg = token()) != NULL);
			mst_only = 0;
			continue;

		case LE:
			/* Display the kernel loader entries 
			 */ 
			if((arg = token()) == NULL)
				call_le(NULL,ALL);
			else {
				if(isdigit((int)arg[0])) {
					if (validigit(arg)) {
						readinto(addr);
						call_le(addr,FIND);
					}
			}
				else
					call_le(arg,FINDTXT);
			}
			continue;
		case DUMPU:
			/* 
			 * Display the user area as a hex/ascii dump.
			 * 	This command uses the thread table slot
			 *	or the process table slot number as input
			 *	and not a thread or a process id.
			 */
			if ((arg = token())== NULL) {
				dumput(CURTHREAD);
				dumpu(CURPROC);
			}
			else do {
				if (isdigit ((int)*arg)) {
					dumput(atoi(arg));
					dumpu(associated_proc(atoi(arg)));
				}
				else {
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_60, 
					"0452-700: Parameter %s not valid. Specify a thread slot number.\n") ,arg);
					continue;
				}
			} while ((arg = token()) != NULL);
			continue;

		case PROC:
			/* Display process table entries */
			if((arg = token()) == NULL) {
				printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_61, 
				"SLT ST    PID   PPID   PGRP   UID  EUID  TCNT  NAME\n") );
				for(cnt = 0; cnt < (int)v.ve_proc; cnt++)
					prproc(cnt, 0, 0, 0);
			}
			else if (*arg == '-') {
				r = (arg[1] == 'r') ? 0 : 1;
				if (r == 0) {
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_61, 
					"SLT ST    PID   PPID   PGRP   UID  EUID  TCNT  NAME\n") );
				}
				if((arg = token()) == NULL) {
					for(cnt=0; cnt < (int)v.ve_proc; cnt++)
						prproc(cnt, r, !r, 0);
				}
				else {
					do {
						prproc(atoi(arg), r, !r, 1);
					} while((arg = token()) != NULL);
				}
			} 
			else {
				printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_61, 
				"SLT ST    PID   PPID   PGRP   UID  EUID  TCNT  NAME\n") );
				do {
					prproc(atoi(arg), 0, 0, 1);
				} while((arg = token()) != NULL);
			}
			continue;

		case VFS:
			/* Display the system's VFS structures. */
			{
				int xflag = 0;

		        printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_17, "VFS ADDRESS   TYPE OBJECT    STUB NUMBER FLAGS\tPATHS\n") );
			vfsnums:
				if((arg = token()) == NULL)
					/* 1st arg value arbitrary, if not -1 */
					prvfs(0, 1, xflag);	
				else if (*arg == '-')
				{
					xflag++;
					goto vfsnums;
				}
				else do {
					prvfs(atoi(arg), 0, xflag);
				} while (arg = token());
				if (!xflag)
					printvfsflags();
				continue;
			}

		case VNODE:
			/* Display the system's vnodes */
			/* Command format: vnode [vnodeaddr] */
			/* where vnodeaddr is the address of memory to format as a vnode */
	                printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_18, "ADDRESS VFS MVFS VNTYPE FSTYPE COUNT ISLOT  DATAPTR FLAGS\n") );
			if ((arg = token()) == NULL)
				prvnode(NULL);
			else do {
				pr_one_vnode(strtoul(arg,NULL,16));
			} while (arg = token());
			continue;

		/* network debugger */
		case NDB:
			ndbmain();
			continue;

		case MBUFS:
			/* Display the system's mbufs */
                   {    
                        int header_printed = 0;
			r = 0;

			if((arg = token()) == NULL) {
			      printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_20, 
                                      "USAGE: mbuf <hexaddr> ...\n") );
			      break;
		       	}

			do {
			   if (sscanf(arg, "%x", &r) != 1) {
			      printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_20, 
                                      "USAGE: mbuf <hexaddr> ...\n") );
			      break;
		       	   }
			   if(!header_printed) {
                               header_printed = 1;
                               printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_19, 
                               "  ADDRESS   SIZE    TYPE     LINK       DATAPTR\n") );
                           }
			   prmbuf(r);
			} while((arg = token()) != NULL);
			continue;
                   }

		case MLIST:
			if ((arg = token()) == NULL ||
			    sscanf(arg, "%x", &r) != 1) {
				printf("USAGE: mlist <hexaddr>\n");
				break;
			}
			mtrack(r);
			continue;

		case SOCKS:
			/* Display the system's sockets */
			if((arg = token()) == NULL)
				prsock(0, 0, 1);
			else if(*arg == '-') {
				if((arg = token()) == NULL)
						prsock(0, 1, 1);
				else do {
						prsock(atoi(arg), 1, 0);
					} while((arg = token()) != NULL);
			} else
				do {
					prsock(atoi(arg), 0, 0);
				} while((arg = token()) != NULL);
			continue;

		case TS:
		case DS:
			/*
			 * Find and Display the symbol that is closest to 
			 * and preceeds the given address.  This search is
			 * done on the symbol table in the namelist file.
			 */
			while((arg = token()) != NULL) { 
				if (!validigit(arg))
					break;
				readinto(addr);
				sp = esearch(addr,symnamebuf,!MATCHNAME);
				if(sp == 0) {
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_22, "\tno match\n") );
					continue;
				}
				else if((int)sp == -1) /* error */
					continue;
				else if(sp->n_zeroes) {
					printf("\t%.8s",sp->n_name);
				}
				else {		/* flexname */
					printf("\t%s",symnamebuf);
				}
				printf(" + 0x%08x ", addr - sp->n_value);
				printf("\n");
			}
			continue;

		case KNLIST:
			/*
			 * Find and display the value of a symbol.  This 
			 * only works on an active system since we use the
			 * knlist system call.
			 */
			while ((arg = token()) != NULL) {
				nlist.n_name = arg;
				srch_knlist(&nlist, 1, sizeof (struct nlist));
				printf("\t%.9s: ",nlist.n_name);
				if(nlist.n_value == 0) {
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_23, "no match\n") );
					continue;
				}
				printf("0x%08x \n",nlist.n_value);
			}
			continue;

		case NM:
			/*
			 * Find and display all symbol table entries associated
			 * with the given symbol.  This search is
			 * done on the symbol table in the namelist file.
			 */
			while((arg = token()) != NULL) {
				sp = search(1,arg,MATCHNAME);
				if(sp == 0) {
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_24, "\t%s\tno match\n"), arg);
					continue;
				}
				else if((int)sp == -1) /* error */
					continue;
			}
			continue;

		case DUMP:
			/*
			 * Display Component dump table entries from the
			 * dump file.
			 */
#ifdef NOTDEF
			if(!dumpflag) 
				printf("crash not running on a dump!\n");
			else
#endif
				/* showdump(batch,all,0); */
				showdump(0,0,0);
			continue;


		case BUFHDR:
			/* Display the system buffer headers */
			if((arg = token()) == NULL) {
				printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_25, "BUF MAJ  MIN    BLOCK   FLAGS\n") );
				for(cnt = 0; cnt < v.v_bufhw; cnt++)
					prbufhdr(cnt,SHORT_BUF);
			}
			else do {
				printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_26, "\nBUFFER HEADER %d:\n") ,atoi(arg));
				prbufhdr(atoi(arg),LONG_BUF);
			     }	while((arg = token()) != NULL);
			continue;

		case BUFFER:
			/* Display the system buffers */
			if((arg = token()) == NULL)
				for(cnt = 0; cnt < v.v_bufhw; cnt++)
					prbuffer(cnt, prdef);
			else if(*arg >= '0' && *arg <= '9')
				do prbuffer(atoi(arg), prdef);
				while((arg = token()) != NULL);
			else {
				for(prptr = prm; prptr->pr_sw != 0; prptr++)
					if(!strcmp(arg, prptr->pr_name))
						break;
				if(prptr->pr_sw == 0) {
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_27, "Mode %s not valid.\n") ,arg);
					while(token() != NULL);
					continue;
				}
				prdef = prptr->pr_sw;
				if((arg = token()) == NULL)
					for(cnt = 0; cnt < v.v_bufhw; cnt++)
						prbuffer(cnt, prptr->pr_sw);
				else do prbuffer(atoi(arg), prptr->pr_sw);
					while((arg = token()) != NULL);
			}
			continue;

		case FS:
			/*
			 * Display a stack traceback showing the routine name
			 * for each calling routine and also displaying the
			 * each stack frame in a hex dump format.
			 * This command uses the process table slot
			 * number as input and not a process id.
			 */
			if((arg = token()) == NULL)
				fstack(CURTHREAD);	/* Use _curthread */
			else do {
				if(isdigit((int)*arg))
					fstack(atoi(arg));
				else {
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_62, 
					"0452-702: Parameter %s not valid.Specify a thread slot number.\n") ,arg);
					while((arg = token()) != NULL);
					continue;
				}
			} while((arg = token()) != NULL);
			continue;

		case STACK:
			/*
			 * Display the stack from the current stack pointer
			 * to the top of stack in a hex dump format.
			 * This command uses the process table slot
			 * number as input and not a process id.
			 */
			if((arg = token()) == NULL)
				prstack(CURTHREAD);	/* Use _curthread */
			else do {
				if(isdigit((int)*arg))
					prstack(atoi(arg));
				else {
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_63, 
					"0452-703: Parameter %s not valid.Specify a thread slot number.\n"),arg);
					continue;
				}
			} while((arg = token()) != NULL);
			continue;

		case TRACE:
			/*
			 * Display a stack traceback showing the routine name
			 * for each calling routine.
			 */
			r = mk = 0;
			if((arg=token()) == NULL) {
				prtrace(r, CURTHREAD, mk);
				continue;
			}

			if (arg[0] == '-') {
			    switch (arg[1]) {
			    case 'r':	/* Use kfp for stack trace */
				r = 1;
				arg = token();
				break;
			    case 'm':
				mk |= TR_MST | TR_EXTENDED;
				arg = token();
				break;
			    case 'k':
				mk |= TR_EXTENDED;
				arg = token();
				break;
			    }
			}

			if(arg == NULL)
				prtrace(r, CURTHREAD, mk);
			else do {
				if(isdigit((int)*arg))
					prtrace(r, atoi(arg), mk);
				else {
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_64, 
					"0452-704: Parameter %s not valid.Specify a thread slot number.\n") ,arg);
					continue;
				}
			} while((arg = token()) != NULL);
			continue;

		case KFP:
			/*
			 * Set up or display the current default 
			 * frame pointer
			 */
			if((arg = token()) != NULL) {
				if (!validigit(arg))
					continue;
				readinto(Kfp);
				prkfp(Kfp);
				while(token() != NULL);
			}
			else {
				prkfp(Kfp);
			}
			continue;

		case TTY:
			if((arg = token()) == NULL)
				strcpy(tty_args,"o");
			else {
				strcpy(tty_args,arg);
				while ((arg = token()) != NULL) {
					strcat(tty_args," ");
					strcat(tty_args,arg);
				}
			}
			call_tty(tty_args);
			continue;

		case CM:
			/*
			 * Change the crash map to point to the given segment
			 * of the process associated with the given process
			 * slot number.
			 */
			if((arg = token()) == NULL) {
				cm->cm_thread = -1; /* default back to kernel */
				continue;
			}
			if(!validigit(arg))
				continue;
			cmp.cm_thread = atoi(arg);

			if((arg = token()) == NULL) {
				printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_32, 
				"USAGE: cm [<SlotNum> <SegNum>]\n") );
				continue;
			}
			if(!validigit(arg))
				continue;
			cmp.cm_reg = atoi(arg);
			if (token() != NULL) {
				printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_33, "USAGE: cm [<SlotNum> <SegNum>]\n") );
				while(token() != NULL);
				continue;
			}
			if(chngmap(&cmp))
				cmstr = cmp;
			continue;

		case OD:
			/*
			 * Displays an area of memory at a given address
			 */
			if((arg = token()) == NULL) {
				printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_34, "Missing parameter.\n\tSpecify an address or a symbol.\n") );
				continue;
			}
			if((addr = symtoaddr(arg)) == -1L) {
				printf( catgets(scmc_catd, MS_crsh,
					MAIN_MSG_35, "Symbol not found.\n") );
					while(token() != NULL);
					continue;				} 
			if((arg = token()) == NULL) {
				units = 1;
				arg = NBPW==4 ? "hex" : "octal";
			} else {
				units = atoi(arg);
				if(units == -1) {
					while(token() != NULL);
					continue;
				}
				if((arg = token()) == NULL)
					arg = NBPW==4 ? "hex" : "octal";
				else
					while(token() != NULL);
			}
			prod(addr, units, arg);
			continue;

		case XXMALLOC:
			/*
			 * Display the system's xmalloc tables
			 */
			xmalloc();
			while ((arg = token()) !=NULL);
			continue;

		case PRINT:
			if ((arg = token()) == NULL) {
printerr:
				printf( catgets(scmc_catd, MS_crsh,
					PRINT_MSG_1, 
					"USAGE: print <type> <address>\n") );
				continue;
			}
			strct = arg;

			if ((arg = token()) == NULL)
				goto printerr;
			if ((addr = symtoaddr(arg)) == -1L) {
				printf( catgets(scmc_catd, MS_crsh,
					MAIN_MSG_35, "Symbol not found.\n") );
				while(token() != NULL);
				continue;
			}
			while(token() != NULL);
			stab(strct, addr, dbgfile);
			continue;

                case DLA:
                        dla_ci_dlock();
                        continue;

		case ERRPT:
			if ((arg = token()) == NULL)
				units = 3;
			else {
				units = atoi(arg);
				while(token() != NULL)
				    ;
			}
			errpt(units);
			continue;

		case THREAD:
			
			format = SHORT;
			status = ALL;
			do {
			   go_on = FALSE;
			   arg = token();
			   if (arg == NULL) {
			      if (format == SHORT)
			      	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_65, 
			      	"SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME\n"));
			      for(thread_slot = 0; thread_slot < (int)v.ve_thread; thread_slot++) {
				 prthread(format, status, thread_slot);
			      }
			   } else if (arg[0] == '-') {
			      switch (arg[1]) {
	
			      case '\0' :
				format = LONG;
				go_on = TRUE;
				break;
	
			      case 'r':
				if (format == SHORT)
				   printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_65, 
				   "SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME\n") );
				status = RUNNING;
				for(thread_slot = 0; thread_slot < (int)v.ve_thread; thread_slot++)
					prthread(format, status, thread_slot);
				break;
	
			      case 'p':
				if (format == SHORT)
				   printf(catgets(scmc_catd, MS_crsh, MAIN_MSG_65, 
				   "SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME\n") );
				arg = token();
				if (!isdigit(*arg))
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_66, 
						"0452-706: Specify a process slot\n") );
				else {
				   do {
					if ((!isdigit(*arg)) || (arg[0] == '-')){
                                                        printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_66,
								"0452-706: Specify a process slot\n") );
                                                        break;
                                                }
				   	proc_slot = atoi(arg);
				   	proc_addr = SYM_VALUE(Proc) +
				   	(ulong)(proc_slot *sizeof(struct proc));
				   	if (read_proc(proc_addr, &proc) != sizeof (struct proc)) {
				      		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_78,
						      	"0452-715: Cannot read process entry %d\n"), proc_slot);
				   		}
				   	else {
				      		int last_thread = FALSE;
				      		thread_slot = ((int)proc.p_threadlist - SYM_VALUE(Thread))/sizeof(struct thread);
				      		prthread(format, status, thread_slot);
				      		while ((read_thread(thread_slot, &thread) == sizeof (struct thread)) 
				      		&& (!last_thread))  {
							if (thread.t_threadlist.nextthread == proc.p_threadlist) {
								last_thread = TRUE;
				      			}
				      			else {
					    			thread_slot = ((int)thread.t_threadlist.nextthread - 
					    				SYM_VALUE(Thread))/sizeof(struct thread);
					    			prthread(format, status, thread_slot);
				      				}
				   			}
						}
                                        } while ((arg=token())!=NULL);
			      } 
			      break;
						
			      case 'a':
				if (format == SHORT)
				   printf(catgets(scmc_catd, MS_crsh, MAIN_MSG_65, 
				   "SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME\n") );
				if ((arg=token())== NULL)	
				   printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_67, 
				   "0452-707: Specify a thread address\n"));
				else {
				   do {
                                      if (arg[0] == '-'){
                                           printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_67,
                                                                           "0452-707: Specify a thread address\n"));
                                                        break;
                                                }
                                        if ((addr = (long ) strtoul(arg,NULL,16)) < SYM_VALUE(Thread)){
                                                printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_67,
                                                                "0452-707: Specify a thread address\n"));
                                                break;
                                        }
				      thread_slot=(int)(addr -
						   SYM_VALUE(Thread))/sizeof(struct thread); 
				      prthread(format, status, thread_slot);
				   } while ((arg=token())!=NULL);
				}
				break;
	
			      default :
				 printf ( catgets(scmc_catd, MS_crsh, MAIN_MSG_68, 
				 "0452-708: No such option\n"));
			      }	
			   } else {
				if (format == SHORT){
				   printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_65, 
				   "SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME\n"));
				}
				do {
					prthread(format, status, atoi(arg));
				} while ((arg=token())!=NULL);
			   }
			} while (go_on);
			continue;
		case PPD :
			arg = token();
			if (arg == NULL) {
			   prppd(selected_cpu);
			}
			else {
				int parsed = 0;
			   do {
			      if (isdigit((int)*arg)) {
				if (parsed == 1)
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_70,
                                	"0452-710: Specify a logical processor number. \n"));
				else
					prppd(atoi(arg));
				parsed = 2;
			      }
			      else {
			         if (*arg == '*') {
				   if (parsed > 0)
					printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_70,
                                	"0452-710: Specify a logical processor number. \n"));
				   else
				   	for (i=0; i< ncpus; i++)
				      		prppd(i);
				   parsed = 1;
			         }
			         else {
				   printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_69, 
				   "0452-709: Specify a logical processor number or *.\n"));
				   break;
			         }
			      }
			   } while ((arg=token())!=NULL);
			}
			continue;

		case STATUS:
			arg = token();
			printf(catgets(scmc_catd, MS_crsh, MAIN_MSG_80, 
			"CPU     TID  TSLOT     PID  PSLOT  PROC_NAME\n"));
			if (arg == NULL) {
			   for (i=0; i< ncpus; i++)
			      prstatus(i);
			}   
			else {
			   do {
			      if (isdigit((int)*arg))
			         prstatus(atoi(arg));
			      else {
			         printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_70, 
			         "0452-710: Specify a logical processor number. \n"));
				 break;
			      }
			   } while ((arg=token())!=NULL);
			}
			continue;

		case CPU:
			arg = token();
			if (arg == NULL)
			   printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_71, 
			   "Selected cpu number : %2d\n"), selected_cpu);
			else {
			   if (!isdigit((int)*arg))
		            	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_72, 
			        "0452-712: Specify a logical processor number.\n"));
			   else {
			      int to_select;
			      to_select = atoi(arg);
			      if (to_select >= ncpus) {
			         printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_73, 
				 "0452-713: This machine has got %2d cpus.\n"), 
			   	 ncpus); 
			      }
			      else 
				 selected_cpu = (cpu_t)to_select;
		           }
			}
			continue;
			
		case STREAM:
			/*
			 * The stream subcommand is issued with no parameters.
			 * This subcommand supports the STREAMS framework.
			 */
			stream();
			continue;

		case QUEUE:
			/*
			 * The queue subcommand issued with no parameters
			 * causes crash to list all write queue structures
			 * in the system.
			 * If the queue subcommand is issued with an
			 * address parameter, then crash will print only
			 * the queue structure associated with that address.
			 * This subcommand supports the STREAMS framework.
			 */
			arg = token();
			if (arg == NULL)
				all_queues();
			else 
				if (validigit(arg))
					queue_by_addr(strtoul(arg,NULL,16));
			continue;

		case MBLOCK:
			/*
			 * The mblock subcommand must be issued with an
			 * address parameter.
			 * This subcommand supports the STREAMS framework.
			 */
			arg = token();
			if (arg == NULL) {
				printf(catgets(scmc_catd, MS_crsh, MBLOCK_MSG_1,
				  "0452-1120: This subcommand requires an address.\n"));
				continue;		
			}
			if (validigit(arg))
				mblock(strtoul(arg,NULL,16));
			continue;

		case DBLOCK:
			/*
			 * The dblock subcommand must be issued with an
			 * address parameter.
			 * This subcommand supports the STREAMS framework.
			 */
			arg = token();
			if (arg == NULL) {
				printf(catgets(scmc_catd, MS_crsh, MBLOCK_MSG_1,
				  "0452-1120: This subcommand requires an address.\n"));
				continue;		
			}
			if (validigit(arg))
				dblock(strtoul(arg,NULL,16));
			continue;

		case QRUN:
			/*
			 * The qrun subcommand is issued with no parameters.
			 * This subcommand supports the STREAMS framework.
			 */
			qrun();
			continue;

		case LINKBLK:
			/*
			 * The linkblk subcommand is issued with no parameters.
			 * This subcommand supports the STREAMS framework.
			 */
			linkblk();
			continue;

		case DMODSW:
			/*
			 * The dmodsw subcommand is issued with no parameters.
			 * This subcommand supports the STREAMS framework.
			 */
			dmodsw();
			continue;

		case FMODSW:
			/*
			 * The fmodsw subcommand is issued with no parameters.
			 * This subcommand supports the STREAMS framework.
			 */
			fmodsw();
			continue;

		case QUIT:
			/*
			 * Exit the crash utility
			 */
			exit(0);
		}
	}
}


extern int errno;

/*
 * NAME: prall
 *                                                                    
 * FUNCTION: This routine is invoked by the -a option to crash.  It displays
 *		a large set of data to the screen from a broad range of 
 *		crash subcommands
 *                                                                    
 */  
prall()
{
	int cnt;
	ulong u_segval;
	struct proc proc;
	struct thread thread;
	int thread_slot;

	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_36, "SYSTEM STAT:\n\n") ,cnt);
	prstat();
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_37, "\nSYSTEM VARIABLES:\n\n") ,cnt);
	prvar();
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_38, "\nSYSTEM TRB STRUCTURES:\n\n") ,cnt);
	prcallout();
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_39, "\nBUFFER HEADERS :\n\n") ,cnt);
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_40, "BUF MAJ  MIN    BLOCK   FLAGS\n") );
	for(cnt = 0; cnt < v.v_bufhw; cnt++) {
		prbufhdr(cnt,SHORT_BUF);
	}
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_41, "\nHASHED INODES\n\n") );
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_42, "MAJ  MIN INUMB REF LINK  UID  ") );
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_43, "GID   SIZE   MODE  SMAJ SMIN FLAGS\n") );
	prhinode(TRUE, NULL);
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_44, "\nTTY STRUCTURES:\n\n") );
	call_tty("a");
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_45, "\nVFS STRUCTURES:\n\n") );
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_46, 
	"VFS ADDRESS   TYPE OBJECT    STUB NUMBER FLAGS\tPATHS\n") );
	prvfs(0, 1, 1);	
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_47, "\nFILE TABLES:\n\n") );
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_59, 
	" SLOT    FILE        REF     TYPE            ADDRESS      FLAGS\n") );
	for(cnt = 0; cnt < (int)v.ve_file; cnt++)
		prfile(cnt, 0);
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_49, "\nPROCESS TABLE:\n\n") );
	printf( catgets(scmc_catd, MS_crsh, 
		MAIN_MSG_61, "SLT ST    PID   PPID   PGRP   UID  EUID  TCNT  NAME\n") );
	for(cnt = 0; cnt < (int)v.ve_proc; cnt++) {
		prproc(cnt, 0, 0, 0);
	}
	for(cnt = 0; cnt < (int)v.ve_proc; cnt++) {
		if (dumpflag) {
			if(readmem(&proc, (long)SYM_VALUE(Proc) + 
			  cnt * sizeof proc, sizeof proc, cnt) != sizeof proc) {
				printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_50, 
				"0452-149: Cannot read process table entry %3d.\n") , cnt);
				continue;
			}
		}
		else {
			if(readmem(&u_segval, SYM_VALUE(g_kxsrval), 
			   sizeof g_kxsrval, cnt) != sizeof g_kxsrval) {
			  printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_51, 
			"0452-150: Cannot read extension segment value from address 0x%8x.\n") ,
				SYM_VALUE(g_kxsrval));
				continue;
			}
			lseek(mem, (SYM_VALUE(Proc)+ 
			      cnt * sizeof(struct proc)&OFF_MSK),0);
			
			if (readx(mem, (char *)&proc, sizeof(struct proc),
			    u_segval) != sizeof(struct proc)) {
				printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_52, 
				"0452-151: Cannot read process table entry %3d.\n") , cnt);
				continue;
			}
		}
		if (proc.p_stat == SNONE)
			continue;
		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_53, 
		"\nDATA FOR PROCESS TABLE ENTRY %d:") ,cnt);
		printf("\n--------------------------------\n\n");
		prproc(cnt, 1, 0, 1);
		printf("\n\n");
		pruareap(cnt,0);
	}
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_79, "\nTHREAD TABLE:\n\n") );
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_65, 
	"SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME\n") );
	for(thread_slot = 0; thread_slot < (int)v.ve_thread; thread_slot++) {
		prthread(SHORT, ALL, thread_slot);
	}
	for(thread_slot = 0; thread_slot < (int)v.ve_thread; thread_slot++) {
		if ((read_thread(thread_slot, &thread)) != sizeof (struct thread)) {
			continue;
		}
		if (thread.t_state == TSNONE)
			continue;
		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_75, 
		"\nDATA FOR THREAD TABLE ENTRY %d:"),thread_slot);
		printf("\n--------------------------------\n\n");
		prthread(LONG, ALL, thread_slot);
		printf("\n\n");
		pruareat(thread_slot, 0);
		fstack(thread_slot);
	}
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_76, "\nPER PROCESSOR DATA AREAS\n")); 
	for (cnt=0; cnt< ncpus; cnt++)
		prppd(cnt);
	printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_77, "\nSTATUS OF PROCESSORS\n")); 
	for (cnt=0; cnt< ncpus; cnt++)
		prstatus(cnt);
 }

/*
 * NAME: sigint
 *                                                                    
 * FUNCTION: Signal handler for the intr signal.  Catches the signal and
 *		indicates an unknown command to the user by displaying
 *		"eh?".  It then jumps back to the beginning of crash
 * RETURNS:
 *	NONE
 */  
sigint(sig)
int sig;
{
	char  *token();

	signal(sig, (void (*)(int))sigint);
	p = line;
	tok = 1;
	if (sig == SIGINT)
		prompt(catgets(scmc_catd, MS_crsh, P_MAIN_MSG_56, "\neh?\n"));
	line[0] = '\0';
	while(token() != NULL);
	longjmp(jmp, 1);
}

/*
 * NAME: token
 *                                                                    
 * FUNCTION: prompts the user for a token and then gets the token 
 *		from the input stream.
 *                                                                    
 * RETURNS:
 *	NULL	if no token is found
 *	addr	a pointer to the token if one was found
 */  
char	*
token()
{
	register  char  *cp;
	char	prmtstr[20];

	for(;;) 
		switch(*p) {
		case '\n':
			p++;
		case '\0':
			if(tok != 0) {
				tok = 0;
				return(NULL);
			}

			if(cm->cm_thread != -1)
			{       sprintf(prmtstr, "t%d,%d >> ",
						cm->cm_thread, cm->cm_reg);
				prompt(prmtstr);
			}
			else
				prompt( catgets(scmc_catd, MS_crsh, P_MAIN_MSG_57, "> ") );

			p = line;
			if(fgets(line, 256, stdin) == NULL)
				return("quit");
			if(line[0] == '!') {
				system(&line[1]);
				line[0] = '\0';
			}
			continue;

		case ' ':
		case '\t':
			p++;
			continue;

		default:
			tok++;
			cp = p;
			while(*p!=' ' && *p!='\t' && *p!='\n')
				p++;
			*p++ = '\0';
			return(cp);
	}
}

/*
 * NAME: prompt
 *                                                                    
 * FUNCTION: Displays a prompt to the screen
 *                                                                    
 * RETURNS:
 *	NONE
 */  
prompt(str)
	char *	str;
{
	while(*str)
		putc(*str++, stderr);
}

/*
 * NAME: prkfp
 *                                                                    
 * FUNCTION: Displays crash's default frame pointer to the screen.
 *                                                                    
 * RETURNS:
 *	NONE
 */  
prkfp(fp)
{
	printf("kfp:  ");
	printf(FMT, fp);
	printf("\n");
	fflush(stdout);
	return;
}


/*
 * NAME: validigit
 *                                                                    
 * FUNCTION: Take a string argument and checks to see that all digits
 *		in the string are valid hex digits.
 *
 * RETURNS:
 *	1	if all digits are valid
 *	0	on error
 */  
validigit(arg)	
char *arg;
{
	register char *c;

	for (c=arg; *c; c++)  
		if (isxdigit((int)*c) == 0) { 
			printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_58, "\t%c is not a valid hexadecimal digit.\n") ,*c);
			while (token() != NULL)
				;
			return(0);
		};
	return(1);
}
