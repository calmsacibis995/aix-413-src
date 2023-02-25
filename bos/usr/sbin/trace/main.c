static char sccsid[] = "@(#)13  1.32.1.33  src/bos/usr/sbin/trace/main.c, cmdtrace, bos41J, 9513A_all 2/21/95 10:00:25";
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *  
 *  LEVEL 1, 5 Years Bull Confidential Information
 *  
 */

/*
 * command line interface and "subcommand" parser for trace
 * See the usage() routine for command syntax, or type:
 *    trace -H
 */

#define _ILS_MACROS

#include <stdio.h>
#include <locale.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/trcctl.h>
#include <sys/trchkid.h>
#include <sys/trchdr.h>
#include <procinfo.h>
#include "trace.h"

#include <sys/wait.h>
#include <xcoff.h>
#include <sys/shm.h>

#include <sys/systemcfg.h>

static char *Unxfile;
static FILE *Unxfp;
static char *Unxbase;

static char *istracpy();


struct rptsym {
	char          *r_name;
	unsigned       r_value;
	char       r_symclass;
};
typedef struct rptsym rptsym;
rptsym *rptsym_install();

static rptsym *getsp();
static void find_symbol();
static void ldrinit();
static void jread();

#define GETSP_INCR 256
static rptsym *Symtab_base;
static Symtab_size;
static Symtab_count;

static struct filehdr filehdr;
static struct aouthdr aouthdr;
static struct scnhdr  scnhdr;
static struct syment  syment;
static struct ldhdr   ldhdr;
static struct ldsym   ldsym;
static union auxent auxent;

static int filehdr_OFF;
static int aouthdr_OFF;
static int scnhdr_OFF;
static int syment_OFF;
static int ldhdr_OFF;

static char *syment_BASE;
static char *ldsym_BASE;
static char *symflx_BASE;				/* flexnames for symbol table */
static char *ldrflx_BASE;				/* flexnames for loader symbols */
static int   symflx_SIZE;
static int   ldrflx_SIZE;

static int   delta_text;  /* difference between raw and virtual implementation
			    of section .text    */
static int   delta_data;  /* difference between raw and virtual implementation
			    of section .data    */

#define filehdr_SIZE FILHSZ                    /* 20 */
#define aouthdr_SIZE (sizeof(struct aouthdr))  /* 48 */
#define scnhdr_SIZE  SCNHSZ                    /* 40 */
#define syment_SIZE  SYMESZ                    /* 18 */
#define ldhdr_SIZE   LDHDRSZ                   /* 32 */
#define ldsym_SIZE   LDSYMSZ                   /* 24 */
#define auxent_SIZE AUXESZ		      /* 18 */
#define auxent_cpy(i) \
	memcpy(&auxent,syment_BASE + (i) * syment_SIZE, auxent_SIZE)
#define AUXE_TYPE(a) ((a).x_csect.x_smtyp & 0x07)
#define AUXE_CLAS(a) ((a).x_csect.x_smclas)

#define IOFF(STRUCT,IDX) \
	( (STRUCT/**/_OFF) + STRUCT/**/_SIZE * (IDX) )

#define syment_cpy(i) \
	memcpy(&syment,syment_BASE + i * syment_SIZE, syment_SIZE)

#define ldsym_cpy(i) \
	memcpy(&ldsym,ldsym_BASE + i * ldsym_SIZE, ldsym_SIZE)

extern optind;
extern char *optarg;
extern char *getenv();
extern char *basename();
extern char *malloc();
extern char *realloc();
int call_exec();


extern verboseflg;
int forceflg;

extern FILE *Infp;

static char Systracefile[32];
static char Systrctlfile[32];

static char *Infile;
static infileflg;               /* suppress prompt when -F option used */

static Argc;
static char **Argv;

static char *strip();
static parentsignal();
static ignore();
static subtrcon(), subtrcoff(), subquit(), subhelp();
static pid_t initChild();

/*
 * subcommand table, to look up commands.
 */
extern shell();

static struct subcmdlist {
        int (*cmd_func)();
        char *cmd_name;
} subcmdlist[] = {
        { subtrcon, "trcon" },
        { subtrcoff,"trcoff" },
        { subquit,  "q" },
        { subquit,  "quit" },
        { shell,    "!" },
        { subhelp,  "?" },
        { subhelp,  "help" },
        { subhelp,  "hp" },
        { 0,0 }
};

static int logstart_ic;
static int logstart;
static int lsize;
static int nentries;

/*
 * NAME:     main
 * FUNCTION: scan command line options and open log and template files.
 * INPUTS:   argc,argv
 * RETURNS:  none (exits)
 *
 * trcstat, trc_test entries  are removed
 *                            
 * The Progname variable is examined to determine what this routine is
 *   trace, trcstop, trcon, trcoff.
 * Open the message catalog by calling catinit.
 * Catch SIGINT and SIGQUIT to the default signal handler parentsignal.
 * Call getcmdline to parse the command line flags. It will open
 *   /dev/systrace and fill in the Conditional bitmap.
 * Open the logfile if not stdout.  Lock it to avoid collision with trace.
 * Call trace to configure a trace session with the CFGMTR ioctl.
 * Then fork and call child(). This process is the trace daemon.
 * If the -a flag was not given, enter interactive mode and interpret
 *   subcommands.
 * The subcommand line scanning routine will fill
 *  in up to NARG subcommand line arguments, including subcommand name.
 * The name of the command is Arg[0], the arguments are Arg[1] - Arg[9]
 * Narg == 1 if there are no arguments (like main(argc,argv))
 * The '!' command is handled separately by sending the rest of the line
 *  to shell().
 */

/*                 
 *	for load dynamic :
 *		- in the subroutine getcmdline add a call to trc_load
 * 		 this routine is called here because it's only in getcmdline
 *		 that you know the channel.After getcmdline, it's too late
 *		 because some channel could be opened (option -g)
 * 		- in some cases exit is replaced by trc_unload
 *	for option -c :
 * 		add in getcmdline the code to save the logfile in logfile.old
 *		in case of option -c
 *	we delete the entries for trcstat and trctest
 *	for mp :
 *	      - in subroutine prhdr :
 *		- increase the size of structure of ps (two new fields)
 *		- add a new structure wtrc_lv send to the driver for
 *		  ioctl TRC_LOADER  : the driver returns now the full name
 *		  of the loader entry
 *		- add a new structure ls to keep informations about
 *		  the devices available in the system
 *		- add a new structure la for cache and memory space
 *		- add a new structure nm to have for each loader entry the list
 *		  of the symbols
 *	    all these structures (except wtrc_lv) belongs now to the
 *	    header of trcfile when the flag -n is set
 */

main(argc,argv)
char *argv[];
{
	(void) setlocale(LC_ALL,"");
        Argc = argc;                                                            /* used by prhdr */
        Argv = argv;                                                            /* used by prhdr */
        setprogname();
        catinit(MCS_CATALOG);                                           /* init cmdtrace.cat */


/* Progname is pointer to character type (declared in trc_sub.c, also
 declared as extern in trace.h) where is stored the name of the in-used
 command */
        if(streq(Progname,"trcsync")) {
                trcsync(argc,argv);
                exit(0);
        }
        if(streq(Progname,"trcdead")) {                         /* trcdead */
                trcdead(argc,argv);
                exit(0);
        }
        if(streq(Progname,"trc_test")) {                        /* trc_test */
                exit(0);
        }
        if(streq(Progname,"lgen")) {                            /* test. generate a logfile */
                lgen(argc,argv);                                                /* from a description file */
                exit(0);
        }
        if(streq(Progname,"snap")) {                            /* read the snap buffer from */
                snap(argc,argv);                                                /* trcdd */
                exit(0);
        }
        if(streq(Progname,"trcstop")) {                         /* trcstop */
		struct tr_stat s;
		pid_t  initChildPid = 0;

                ctlopen(argc,argv);

		/*  Check to make sure we are not stopping an interactive
		** trace.  If we try to do a TRCSYNC to an interactive
		** trace, then the trcstop will hang until the trace 
		** closes the /dev/systrace, which in some cases can
		** never happen.
		**
		** Note: don't check the rc of the TRCSTAT ioctl, because
		** it *will* fail if there is no trace daemon running.
		*/
		ioctl(Systrctlfd,TRCSTAT,&s);
		if (s.trst_iactive && !(initChildPid = initChild())) {
                	cat_eprint(CAT_TRC_STPIACT,
			"Cannot %s an interactive trace session.\nUse the trcoff trace sub-command to stop tracing.\n",
			Progname);
			exit(1);
		}
		if (initChildPid) {
                        kill(initChildPid,SIGUSR2);
                }
                ioctl(Systrctlfd,TRCOFF,0);
                if (ioctl(Systrctlfd,TRCSTOP,0) < 0) {
			perror("trcstop");
			close(Systrctlfd);
			exit(1);
		}
                ioctl(Systrctlfd,TRCSYNC,0);
		close(Systrctlfd);
                exit(0);
        }
        if(streq(Progname,"trcoff")) {                          /* trcoff */
                ctlopen(argc,argv);
                subtrcoff();
		close(Systrctlfd);
                exit(0);
        }
        if(streq(Progname,"trcon")) {                           /* trcon */
                ctlopen(argc,argv);
                subtrcon();
		close(Systrctlfd);
                exit(0);
        }
        if(streq(Progname,"trcstat")) {                         /* trcstat */
                exit(0);
        }
        /*
         * trace
         */
        Progname = "trace";                  /* identify error messages */


        getcmdline(argc,argv);                          /* read command line and init. sizes */
        Debug("Lsize=%d Tbufsize=%d Logfile=%s\n",Logsize,Tracebufsize,Logfile);

        /*
         * open /dev/systrace, /dev/systrctl, and logfile
         */

        if((Systrctlfd = open(Systrctlfile,0,0)) < 0) { /* open trace control */
                perror(Systrctlfile);
		close(Systracefd);
                call_exec(EXIT_NOFILE,0);
        }

        signal(SIGQUIT,parentsignal);           /* terminate child end exit */
        signal(SIGHUP, parentsignal);           /* terminate child end exit */
        signal(SIGTERM,parentsignal);           /* terminate child end exit */
        signal(SIGSEGV,parentsignal);           /* terminate child end exit */

        if(!stdoutflag) {                                               /* if writing log to a file */
                if((Logfd = open(Logfile,O_RDWR|O_TRUNC|O_CREAT,0666)) < 0) {
                        perror(Logfile);                                /* truncate, create if  */
			close(Systracefd);
			close(Systrctlfd);
                        call_exec(EXIT_NOLOGFILE,0);           /* necessary */
                }
		(void)fchmod(Logfd,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
			/* always attempt to set mode to 666 in case we created it...
			 * since mode should be 666 we don't care who created it.
			 */
        } else {                                                                /* otherwise, */
                Logfd = 1;                                                      /* use stdout */
        }
        trace();                                                                /* no return */
}

/*
 * The parent process reads the subcommand line and controls the trace
 *  by issuing ioctls to /dev/systrctl or sending signals to Childpid.
 * The child process reads the /dev/systrace device as a daemon and
 *  writes the Logfile.
 *
 * The shell subcommand ("!") is called with the rest of the subcommand
 *  line, which is exec-ed with a sh -c vargbuf.
 *  If only "!" is type, the shell itself is exec-ed.
 * The other subcommands are looked up in the array 'subcmdlist[]'.
 * The subcommand name is placed in Arg[0].
 * Subcommand arguments are placed in Arg[1],...,Arg[9].
 * Most subcommands do not use any arguments.
 */

#define SIGTOMASK(sig) ( 1 << ((sig) - 1))

static trace()
{
        struct tr_struct tr;
        int cmd;

        /*
         * init tr_struct: buffer size, trace mode and conditional event bitmap
         */
        tr.tr_tbufsize = Tracebufsize;
        memcpy(tr.tr_events,Condhookids,sizeof(tr.tr_events));
        switch(Tracemode) {                                                     /* convert from English name */
        case M_CIRCULAR:  cmd = CFGMTRL; break;         /* to ioctl command, */
        case M_ALTERNATE: cmd = CFGMTRA; break;         /* using a straight switch */
        case M_SINGLE:    cmd = CFGMTRF; break;         /* table. Other Tracemodes */
        case M_BUS:       cmd = CFGBTR;  break;         /* "can't happen" */
        }


        if(ioctl(Systracefd,cmd,&tr) < 0) {           /* set the tracemode, */
                perror("ioctl");                   /* bufsize, and eventlist */
		close(Systracefd);
                close(Systrctlfd);
                call_exec(EXIT_NOFILE,0);      
        }
        sigsetmask(SIGTOMASK(SIGUSR1));     /* block SIGUSR1 until ready */
	if (nmflag) {
		Debug("main : Father header \n");
               	lhdrinit();
	}
        Childpid = fork();                        /* child is the daemon */
        if(Childpid == 0) {                       /* if child process */
		if (!nmflag) {
			Debug("main : child header \n");
                	lhdrinit();
		}
                child();                                                                /* call child() */
        }
        cldwait();                                                                      /* wait for child ready */
        if(asyncflag) {
                close(Systracefd);
                close(Systrctlfd);
                exit(genericflag ? Channel : 0);
        }
        subcmd();
}

static lhdrinit()
{
        int i;
        struct trc_log *logp;

        if(!stdoutflag) {
        	nentries = Logsize / Tracebufsize;
        }

	prhdr();

        logstart = lseek(Logfd,0,1);


        /*
         * initialize log structure
         */
        logp = (struct trc_log *)jalloc(lsize);
        for(i = 0; i < nentries; i++)
                logp->l_data[i].le_offset = logstart + i * (Tracebufsize + TRC_OVF);
        logp->l_magic    = TRC_NEW_LMAGIC;   
        logp->l_fd       = Logfd;
        logp->l_nentries = nentries;
        if(stopflag)
                logp->l_mode |= MDL_STOPONWRAP;
        if(stdoutflag)
                logp->l_mode |= MDL_NOWRAP;
        else {
                logp->l_ic.le_offset = logstart_ic;
                logp->l_ic.le_size   = logstart - logstart_ic;
                lseek(Logfd,0,0);
                write(Logfd,logp,lsize);
                lseek(Logfd,logstart,0);
        }
        if(ioctl(Systracefd,TRCIOC_LOGCFG,logp)) {
                perror("TRCIOC_LOGCFG");
                childexit(1);
        }
}

static subcmd()
{
        int i,n;
        struct subcmdlist *subcmdp;
        char *cp;
        char buf[256];          /* subcommand line */
        char *line;
        char vargbuf[256];      /* copy of subcommand line for shell() */

        signal(SIGINT, parentsignal);                   /* toggle TRCON/TRCOFF */
        sigsetmask(SIGTOMASK(SIGINT));                  /* block SIGINT until ready */
	ioctl(Systrctlfd,TRCIACTIVE,0);		/* set interactive */
        for(;;) {                                                               /* loop. Exit by quit command */
                prompt();                                                       /* printf("-> "); */
                sigsetmask(0);                                          /* allow signals */
                if(fgets(buf,256,Infp) == NULL) {
                        if(feof(Infp))
                                subquit();
                        if(errno == EINTR)
                                continue;
                        perror(Infile);
                        subquit();
                }
                sigsetmask(SIGTOMASK(SIGINT));          /* block SIGINT until ready */
                line = strip(buf);                                      /* remove leading and trailing */
                Debug("line='%s'\n",line);
                if(line[0] == '\0')
                        continue;
                if(line[0] == '!') {                            /* shell command */
                        vargbuf[0] = ' ';                               /* get rest of subcommand line, */
                        strcpy(vargbuf,line+1);                 /* skip over "!", */
                        shell(vargbuf);                                 /* and pass to shell() */
                        continue;                                               /* get next subcommand */
                }
                if((Arg[0] = strtok(line," \t\n")) == NULL)     /* get command */
                        continue;                                                               /* ignore blank lines */
                for(subcmdp = &subcmdlist[0]; subcmdp->cmd_name; subcmdp++)
                        if(streq(subcmdp->cmd_name,Arg[0]))             /* match names */
                                break;                                                          /* match */
                if(subcmdp->cmd_name == NULL) {                         /* name not in table */
                        printf("%s ?\n",Arg[0]);                                /* print message */
                        continue;                                                               /* get next subcommand */
                }
                for(i = 1;i < NARG;i++) {       /* get rest of subcommand line arguments */
                        if((Arg[i] = strtok(0," \t\n")) == NULL)
                                break;  /* delimiters are blanks, tabs, and newline */
                        Debug("arg[%d]=%x '%s'\n",i,Arg[i],Arg[i]);
                }
                Narg = i;               /* pass number of arguments (argc) as a global var. */
                (*subcmdp->cmd_func)();                                         /* call the routune */
        }
}

/*
 * parentexit is called by the parent to turn off the trace and
 * shut down the trace daemon (child).
 * The TRCSTOP ioctl causes tracing to stop, unblocking of a read
 *  on /dev/systrace, and EOF when the kernel trace buffers are emptied.
 * The trace daemon (child) will close the Logfile and exit on its own.
 * If the trace daemon has not been started, as would be the case of a fatal
 *  error early in the parent program, parent exit will just exit.
 */
static parentexit(exitcode)
int exitcode;                            /* return this value as an exitcode */
{
        struct tr_stat trstat;            /* get number of missed traces */

        Debug("parentexit\n");
        sync();
        subtrcoff();
        if(Childpid) {
                if(ioctl(Systrctlfd,TRCSTOP,0) < 0)
                        perror("TRCSTOP");
                close(Systracefd);
                ioctl(Systrctlfd,TRCSYNC,0);
        	close(Systrctlfd);
		wait(0);
        }
	else {
		close(Systracefd);
	        close(Systrctlfd);
	}

        call_exec(exitcode,0);
}

/*
 * NAME:     strip
 * FUNCTION: remove leading and trailing blanks and tabs
 * INPUT:    line   string to strip
 * RETURNS:  pointer to start of stripped line
 */
static char *strip(line)
char *line;
{
        char *cp;

        cp = &line[strlen(line)-1];
        while(cp > line) {
                switch(*cp) {
                case '\n':
                case ' ':
                case '\t':
                        *cp = '\0';
                        cp--;
                        continue;
                }
                break;
        }
        if(cp == line)
                return(line);
        cp = line;
        while(*cp == ' ' || *cp == '\t')
                cp++;
        return(cp);
}

/*
 * NAME:     prompt
 * FUNCTION: write out subcommand line prompt
 * INPUT:    none
 * RETURNS:  none
 *
 * Don't write a prompt if input is taken from a file.
 */
static prompt()
{

        if(!infileflg) {
                printf("-> ");
                fflush(stdout);
        }
}

/*
 * Called once to get the command line arguments.
 * The fill in the default (if not set on the command line)
 * Logfile         read "file = " entry in config object.
 * Logsize         read "size = " entry in config object.
 * Tracebufsize    to TRACESIZE_DFLT
 *
 * The two sizes (Tracebuf, Log) need some explanation because
 *  they interact with each other.
 * The Tracebuf and Log sizes are related by Logfile wraparound.
 *  The trace entries are variable length (4,8,12,20,and 24 bytes),
 *   the data stored is binary, (synchronization on a particular byte sequence
 *   is not always possible), and the length information is in the second byte
 *   of the trace entry. This makes it important to not write on top of
 *   a partial trace entry when the Logfile wraps around. Always keep track of
 *   the start of the next trace entry, because this entry will be the first
 *   entry of the Logfile when it is unwrapped.
 *  This keeping track of the start of the next trace entry is compute
 *   intensive, since it involves byte by byte (or at least event by event)
 *   scanning of all the trace data, and calculating the size of the event
 *   from a 1/2 byte size code.
 *  It is possible to take advantage of the fact that a kernel trace buffer
 *   contains only whole events, so that a whole kernel trace buffer starts
 *   with the beginning of a trace, and ends with the end of a trace. There
 *   is no partial trace. (This means that the size of a kernel trace buffer
 *   varies in size by a few words to accommodate different combinations of
 *   trace events). It is not necessary to maintain "synchronization" within
 *   a trace buffer, just to manage the boundaries of a trace buffer.
 *  The Logfile size becomes important here. If the Logfile size is a
 *   (approximate) multiple of a trace buffer, then is can store whole but
 *   slightly variable length trace buffers, and the trace daemon just has
 *   to manage the start and end boundaries of each whole trace buffer
 *   within the Logfile. The trace buffers within the Logfile are referred
 *   to as 'pieces' within this program.
 *  Therefore, getcmdline() ensures that the Logfile size is at least as large
 *   as Tracebufsize, and truncates it to an (roughly) even multiple of
 *   a trace buffer. It also truncates it to the size of the ulimit so that
 *   Logfile wraparound will always occur before the ulimit is exceeded.
 *  Note that this restriction (Logsize=N*Tracebufsize) does not logically
 *   apply when wraparound is not used (-s option), but is enforced anyway.
 *  In that case, redirect the Logfile to stdout, where this restriction
 *   is not enforced.
 */

/*                   
 * in this routine we call the trc_load routine with the channel chosen
 */ 

static getcmdline(argc,argv)
char *argv[];
{
        int c;
        int i,j;
        char *logfile;
        char *options;
        char *cp;
        char *jhooklist;
        int condflg,kcondflg;
        struct conflist *cfp;
        struct stat statbuf;
        char ext[4];
	char logfile_old[132];
	int openflag;

	openflag = 0;
        logfile = 0;
        condflg = 0;
        kcondflg = 0;
        for(i = 0; i < 16; i++)         /* start with ids 0-15 ON */
                SETHOOKID(Condhookids,i);
/*      options = "Cvg01234567flbacndho:k::j::sm:T:L:HDF:"; */
        options = "vg01234567flbacndho:k::j::sm:T:L:HF:";
        while((c = getopt(argc,argv,options)) != EOF) {
                switch(c) {
/*
                case 'C':
                        forceflg++;
                        break;
*/
                case 'v':
                        verboseflg++;
                        break;
                case 'g':
                        genericflag++;
                        break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                        Channel = c - '0';
                        break;
                case 'F':
                        Infile = optarg;
                        break;
/*
                case 'D':
                        debuginit("Btrace");
                        break;
*/
                case 'f':
			if (Tracemode == M_CIRCULAR) {
				usage();
				break;
			}
                        Tracemode = M_SINGLE;
                        break;
                case 'l':
			if (Tracemode == M_SINGLE) {
				usage();
				break;
			}
                        Tracemode = M_CIRCULAR;
                        break;
                case 'b':
                        Tracemode = M_BUS;
                        break;
                case 's':
                        stopflag++;
                        break;
                case 'a':
                        asyncflag++;
                        break;
                case 'd':
                        delayflag++;
                        break;
                case 'o':
                        if(streq(optarg,"-"))
                                stdoutflag++;
                        else
                                stdoutflag = 0;
                        logfile = optarg;
                        break;
                case 'h':
                        nohdrflag++;
                        break;
                case 'm':
                        Message = optarg;
                        break;
                case 'k':
                        if(!kcondflg++ && !condflg)
                                memset(Condhookids,0xFF,sizeof(Condhookids));
                        bitmap(optarg,0);
                        break;
                case 'j':
                        condflg++;
 			jhooklist = malloc(strlen(optarg) + 1);
 			if(jhooklist==NULL) {
 				perror("malloc");
 				exit(1);
 			}
 			strcpy(jhooklist,optarg);
                        bitmap(jhooklist,1);
                        break;
                case 'T':
                        Tracebufsize = atoi(optarg);
                        break;
                case 'L':
                        Logsize = atoi(optarg);
                        break;
		case 'c':
			saveflag++;
			break;
		case 'n': 
			nmflag++;
			break;
                case '?':
                case 'H':
                default:
                        usage();
                }
        }

	
        if(Infile) {
                infileflg++;
                if((Infp = fopen(Infile,"r")) == NULL) {
                        perror(Infile);
                        exit(EXIT_BADOPT);
                }
        } else {
                Infile = "stdin";
                Infp = stdin;
        }
        if( fstat(fileno(Infp),&statbuf) == 0 &&
           (statbuf.st_mode & S_IFMT) != S_IFCHR)
                infileflg++;
        if(Channel >= TRC_NCHANNELS) {
                cat_eprint(CAT_TRC_CHAN2BIG,
"Channel %d is out of range.\n\
The range of channels is from 0 to %d\n",
                        Channel,TRC_NCHANNELS);
                exit(EXIT_BADOPT);
        }
        if(!kcondflg && !condflg)
                memset(Condhookids,0xFF,sizeof(Condhookids));
        if(genericflag) {
		if (!asyncflag)
			usage();
		lock_sema();
                for(i = 1; i < TRC_NCHANNELS; i++) {
                        sprintf(Systracefile,"%s%d",SYSTRACE,i);
                        sprintf(Systrctlfile,"%s%d",SYSTRCTL,i);
                        if((Systracefd = open(Systracefile,0,0)) < 0) {
				if (errno == EBUSY) {
                      		          continue;
				}
				else  {
					openflag++;
				}
			}
                        Channel = i;
                        break;
                }
                if(i == TRC_NCHANNELS) {
			unlock_sema();
                        exit(EXIT_NOCHANNELS);
		}
		call_exec(Channel,2);
		if ((openflag) && (Systracefd = open(Systracefile,0,0)) < 0) {
			perror("open");
			unlock_sema();
			call_exec(EXIT_NOFILE,0);
		}
		unlock_sema();
        } else {
		call_exec(Channel,1);
                if(Channel > 0) {
                        sprintf(Systracefile,"%s%d",SYSTRACE,Channel);
                        sprintf(Systrctlfile,"%s%d",SYSTRCTL,Channel);
                } else {
                        sprintf(Systracefile,"%s",SYSTRACE);
                        sprintf(Systrctlfile,"%s",SYSTRCTL);
                }
                /*
                 * only 1 open() of systrace allowed
                 */
/*
                if(forceflg) {
                        int fd;

                        if((fd = open("/dev/systrctl",0)) >= 0) {
                                ioctl(fd,TRCIOC_CLOSE,0);
                                close(fd);
                        }
                }
*/
                if((Systracefd = open(Systracefile,0,0)) < 0) {
               		cat_eprint(CAT_TRC_SESS,"\
The trace daemon is already active.  Only one trace session may \n\
be active at a time.\n"); 
                        call_exec(EXIT_NOFILE,0);
                }
        }
        if(Tracebufsize == 0)
                Tracebufsize = TRACESIZE_DFLT;
        if(Logsize == 0)
                Logsize = LOGSIZE_DFLT;
        if(logfile == NULL) {
                logfile = vset(LOGFILE_DFLT);
                if(Channel > 0)
                        sprintf(Logfile,"%s.%d",logfile,Channel);
                else
                        strcpy(Logfile,logfile);
        } else {
                strcpy(Logfile,logfile);
        }


/* At this step, we know the name of the logfile */
	if ((stdoutflag) && (saveflag)) {
		cat_eprint(CAT_TRC_USAGE5,"\
Warning : Flags -c and -o- are mutually exclusive. \n\
Only flag -o- is accepted.\n");
		saveflag--;
	}
	if (saveflag) {
		sprintf(logfile_old, "%s%s", Logfile, ".old");
		if (rename(Logfile, logfile_old) == -1) {
			perror ("save logfile (W)");
		}
	}


	if ((nmflag) && (nohdrflag)) {
		cat_eprint(CAT_TRC_USAGE6,"\
Warning : Option -n and -h are mutually exclusive. \n\
Only option -h is accepted.\n");
		nmflag--;
	}


        if(Tracemode == 0)
                Tracemode = M_ALTERNATE;                /* circular, alternate, single, bus */


        if(!stdoutflag) {
        	Ublimit = ulimit(1,0) * 512;
		/* verify that logfile will fit within the ulimit.  */
	        if (Logsize > Ublimit) {
			Logsize = Ublimit;
                }
                switch(Tracemode) {
                case M_ALTERNATE:
                case M_CIRCULAR:
                        if(Logsize < 2 * Tracebufsize) {
                                cat_eprint(CAT_TRC_USAGE3,"\
Error: Maximum logfile size (%d bytes) must be at least twice\n\
the size of the trace buffer (%d bytes).\n",
                                        Logsize,Tracebufsize);
				close(Systracefd);
				call_exec(EXIT_BADOPT,0);
			}
			break;
                case M_SINGLE:
                        if(Logsize < Tracebufsize) {
                                cat_eprint(CAT_TRC_USAGE4,"\
Error: Maximum logfile size (%d bytes) must be at least\n\
the size of the trace buffer (%d bytes).\n",
                                        Logsize,Tracebufsize);
				close(Systracefd);
				call_exec(EXIT_BADOPT,0);
			}
                }
        }
        if(verboseflg) {
                for(i = 0; i < 16; i++) {
                        for(j = 0; j < 32; j++)
                                vprint("%02X",Condhookids[i*32 + j]);
                        vprint("\n");
                }
        }
}

static unsigned char bitmap0[2] = { 0xFF, 0xFF };       /* bitmap for major id 0 */

#define SETCLR(map,bit,setflg) \
        if(setflg) \
                SETHOOKID(map,b); \
        else \
                CLRHOOKID(map,b);

static bitmap(hookbuf,setflg)
char *hookbuf;
{
        char *cp;
        int len;
        int n;
        int i;
	int j;
        int start,count,b;
        char *delims;
	char *local_hookbuf;

        local_hookbuf = malloc(strlen(hookbuf) + 1);
        if (local_hookbuf == 0) {
                perror("malloc");
                exit(1);
        }
        else {
                strcpy(local_hookbuf, hookbuf);
        }


        if(local_hookbuf == NULL)
                return;
        delims = ", \t\n";
        cp = strtok(local_hookbuf,delims);
        while(cp) {
                len = strlen(cp);
                if((n = strtoid_cmd(cp)) < 0)
                        usage();
                if(len == 2) {
                        start = 16*n;
                        count = 16;
                } else {
                        start = n;
                        count = 1;
                }
		Debug("bitmap: cp=%s count=%d start=%d\n",cp,count,start);
                for(i = 0; i < count; i++) {
                        b = start + i;
                        if(b < 16) {            /* 0-15 need -k TWICE */
                                if(!setflg && ISHOOKID(bitmap0,b)) {
                                        CLRHOOKID(bitmap0,b);
                                        continue;
                                }
                                SETCLR(bitmap0,b,setflg);
                        }
                        SETCLR(Condhookids,b,setflg);
                }
                cp = strtok(0,delims);
        }
}

usage()
{

        cat_eprint(CAT_TRC_USAGE1,"\
usage:  trace -fl -a -g -d -s -h -j event_list -k event_list -m message\n\
		-o outfile -T trace_buffer_size -L log_file_size -c -n\n\
-f         Log only first buffer of trace data collected. Default is all.\n\
-l         Log only last  buffer of trace data collected.\n\
-a         Start tracing and exit. Default is start trace, enter subcommand.\n\
-g         Start generic tracing. Option -a must also be specified.\n\
-d         Delay trace data collection until 'trcon' subcommand\n\
-s         Stop tracing when trace log fills. Default is to wraparound.\n\
-h         Suppress date/sysname/message header to the trace log.\n\
-j e1,,eN  Collect trace data only for the events in list. Default is all.\n");
        cat_eprint(CAT_TRC_USAGE2,"\
-k e1,,eN  Don't collect trace data for the events in list.\n\
-m message Text to be included in trace log header record.\n\
-o outfile Write trace log to outfile. Default is %s\n\
-o -       Write trace log to stdout.\n\
-T size    Trace buffer size = 'size' bytes. Default is %d\n\
-L size    Log file size = 'size' bytes.     Default is %d\n\
-c         Save the old trace log in a .old file.\n\
-n         Write in trace log the nm of all loader entries.\n\
modes f,l   are mutually exclusive.\n",
                LOGFILE_DFLT,TRACESIZE_DFLT,LOGSIZE_DFLT);
        exit(EXIT_BADOPT);
}

static ctlopen(argc,argv)
char *argv[];
{
        int c;
        char filename[32];
        char ext[8];
        int channel;

        channel = 0;
        while((c = getopt(argc,argv,"01234567")) != EOF) {
                switch(c) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                        channel = c - '0';
                        break;
                case '?':
                        cat_eprint(CAT_TRC_STOP,"\
Usage:  %s \n",Progname);
                        exit(EXIT_BADOPT);
                }
        }
        if(channel == 0)
                sprintf(ext,"");
        else
                sprintf(ext,"%d",channel);
        sprintf(Logfile,"/dev/systrace%s",ext);
        sprintf(filename,"/dev/systrctl%s",ext);
        if((Systrctlfd = open(filename,0,0)) < 0) {
                perror(filename);
                exit(EXIT_BADOPT);
        }
}

#define BPR(X,VALUE)   if(X & VALUE) printf("%s ","VALUE");
#define CASE(VALUE)    case VALUE:   printf("%s ","VALUE"); break;

/* This function is used for debugging */
trcstat()
{
        int fd;
        struct trchdr t;

        if((fd = open("/dev/systrctl",0)) < 0 &&
       (fd = open("/dev/sysutil",0)) < 0) {
                perror("/dev/systrctl");
                exit(1);
        }
        if(ioctl(fd,UTRCHDR,&t) < 0) {
                perror("/dev/systrctl ioctl");
                exit(1);
        }
        printf("state: %02x ",t.trc_state);
        BPR(t.trc_state,ST_ISOPEN);
        BPR(t.trc_state,ST_TRCON);
        BPR(t.trc_state,ST_TRCSTOP);
        BPR(t.trc_state,ST_COND);
        BPR(t.trc_state,ST_BUS);
        printf("\n");
        printf("msr:   %04x\n",t.trc_msr);
        printf("lckwd: %08x %d\n", t.trc_lockword, t.trc_lockword);
        printf("slpwd: %08x %d\n", t.trc_sleepword, t.trc_sleepword);
        printf("mode:  %d ",  t.trc_mode);
        switch(t.trc_mode) {
        CASE(MD_OFF);
        CASE(MD_CIRCULAR);
        CASE(MD_CIRCULAR_COND);
        CASE(MD_ALTERNATE);
        CASE(MD_ALTERNATE_COND);
        CASE(MD_SINGLE);
        CASE(MD_SINGLE_COND);
        CASE(MD_BUS);
        CASE(MD_BUS_COND);
        }
        printf("\n");
        printf("fsm:   %d ",  t.trc_fsm);
        switch(t.trc_fsm) {
        CASE(FSM_A_B);
        CASE(FSM_A_BLOCK);
        CASE(FSM_B_A);
        CASE(FSM_B_BLOCK);
        CASE(FSM_SPILL_A);
        CASE(FSM_SPILL_A_OFF);
        CASE(FSM_SPILL_A_OFF2);
        CASE(FSM_SPILL_B);
        CASE(FSM_SPILL_B_OFF);
        CASE(FSM_SPILL_B_OFF2);
        CASE(FSM_SPILL_BLOCK);
        CASE(FSM_SPILL_EOF);
        }
        printf("\n");
        printf("wcnt:  %d\n",t.trc_wrapcount);
        prq(&t.trc_currq);
        prq(&t.trc_Aq);
        prq(&t.trc_Bq);
}

/* This function is used for debugging */
static prq(qp)
struct trc_q *qp;
{

        printf("[%08x,%08x]   I: %04x   size=%x ",
                qp->q_start,
                qp->q_end,
                (char *)qp->q_inptr  - (char *)qp->q_start,
                qp->q_size);
        if( qp->q_inptr < qp->q_start ||
           (char *)qp->q_end+qp->q_size < (char *)qp->q_inptr)
                printf("IOUTOFRANGE");
        printf("\n");
}

static ishexnum(cp)
char *cp;
{
        int c;

        while(c = *cp++) {
                if(isxdigit(c))
                        continue;
                if(c == 'x' || c == 'X')
                        continue;
                return(0);
        }
        return(1);
}

/*
 * SUBCOMMANDS:
 * subquit()
 * subhelp()
 * subtrcon()
 * subtrcoff()
 */
static subquit()
{

        parentexit(0);                                                  /* parentexit() handles cleanup */
}

static subhelp()
{

        cat_print(CAT_TRC_HELP,"\
trcon       Start collection of data\n\
trcoff      Stop  collection of data\n\
q,quit      Stop  collection of data and exit trace\n\
! command   Run command 'command'\n\
?           Display summary of trace commands\n");
}

static subtrcon()
{

        if(ioctl(Systrctlfd,TRCON,0) < 0)       /* issue TRCON ioctl */
                perror("trcon: TRCON");
}

static subtrcoff()
{

        if(ioctl(Systrctlfd,TRCOFF,0) < 0)              /* issue TRCOFF ioctl */
                perror("TRCOFF");
}

/*
 * All signals are sent to this routine, instead of one routine per signal.
 * This is because many of the signals are set to have the same result
 *  anyway, and it's easier to manage the one signal routine.
 * The signal() 'system call' calls signal() with the signal number (signo)
 *  and an argument (arg), usually the address of the segment violation
 *  (if any).
 */
static parentsignal(signo,arg)
int signo;              /* signal number of signal which caused this routine */
int arg;                /* not always meaningful */
{
        struct tr_stat trstat;  /* trace status structure */

        Debug("parentsignal(%d)\n",signo);
        switch(signo) {                 /* determine which signal */
        case SIGINT:                    /* keyboard interrupt causes a toggle */
                ioctl(Systrctlfd,TRCSTAT,&trstat);      /* get current setting from */
                if(trstat.trst_cmd == TRCON) {          /* systrctl device. */
                        ioctl(Systrctlfd,TRCOFF,0);             /* if ON, turn OFF */
                        cat_print(CAT_TRC_OFF,"tracing is now OFF\n");
                } else {                                                        /* otherwise */
                        ioctl(Systrctlfd,TRCON,0);              /* turn ON */
                        cat_print(CAT_TRC_ON,"tracing is now ON\n");
                }
                break;
        case SIGSEGV:                                                   /* program error */
                Debug("bus error\n");            /* probably better to core dump */
        case SIGQUIT:                                                   /* SIGQUIT, SIGTERM, and SIGHUP */
        case SIGTERM:                                                   /* cause graceful termination */
        case SIGHUP:                                                    /* by shutting down trace daemon */
                parentexit(0);                                          /* and exiting */
        case SIGCLD:                                                    /* someone issued trcstop command */
                parentexit(0);
        default:
                Debug("signal %d. arg=%x\n",signo,arg);/* unexpected signal */
                parentexit(EXIT_SIGNAL);                                        /* prinf message and exit */
        }
        signal(signo,parentsignal);
}

/*
 * Convert the arg 'cp' into a traceid.
 * Return -1 if the traceid is invalid (non-numeric or too big).
 */
strtoid_cmd(id)
char *id;
{
        int c;
        int n;
        char *cp;

        switch(strlen(id)) {
        case 2:
        case 3:
                break;
        default:
                goto bad;
        }
        cp = id;
        while(c = *cp++) {
                if(isxdigit(c))
                        continue;
                if(c == 'x' || c == 'X')
                        continue;
                goto bad;
        }
        n = strtol(id,0,16);
        if((unsigned)n >= NHOOKIDS)
                goto bad;
	Debug("strtoid_cmd: n=%d\n",n);
        return(n);
bad:
        cat_eprint(CAT_TPT_TFMT_C,
"Bad format for traceid %s\n\
A traceid consists of 2 or 3 hexadecimal digits.\n\
A 2 digit traceid represents a range of 16 traceids.\n",id);
        return(-1);
}

static cldwait()
{

        signal(SIGALRM,ignore);         /* turn on SIGALRM */
        signal(SIGUSR1,ignore);         /* turn on SIGUSR1 */
        alarm(5); 
        sigpause(0);                            /* allow SIGUSR1 to cause return from pause */
        alarm(0);                                       /* turn off SIGALRM */
        signal(SIGUSR1,SIG_IGN);        /* turn off SIGUSR1 */
}

static ignore(signo)
{
}

/*
 * This header looks like a trcgen event to trcrpt
 */
/*                     
 * in all the structure an integer field is added, and this because 
 * the hooks in release 4.1 have 4 bytes more (size of tid)
 */

struct header {
        unsigned h_hkid;
        unsigned h_mode;
        time_t   h_time;
        struct utsname h_uts;
        int  h_msglen;
        char h_message[512];
	int h_field;
};
static struct header Header;

struct gen_hdr0 {
        unsigned h_hkid;
        unsigned h_mode;
};

struct gen_hdr {
        struct gen_hdr0 h_0;
        char   h_message[1];
	int    h_field;
};

struct dv {
        unsigned dv_hkid;
        int      dv_mode;
        int      dv_rdev;                       /* start of variable part */
        char     dv_name[16];
	int      dv_field;
};

struct baseconversion {
        unsigned b_hkid;
        int      b_Xint;
	int      b_Xfrac;
	int	 b_rtc_type;
	int	 b_notused[3];
};
struct currp {
        unsigned cp_hkid;
        int      cp_pid;
	int      cp_field;
};

struct ps {
        unsigned p_hkid;
        int      p_mode;
        int      p_pid;                 /* start of variable part */
        int      p_ppid;
        char     p_name[16];
	int	p_tid;			/* thread id */
	int 	p_cpuid;	/* cpu on which thread is currently running */
	int	p_pri;		/* priority of the thread */
	int	p_uid;
	int 	p_gid;
	char	p_tty[10];
	int     p_field;
};

struct lv {
        unsigned lv_hkid;
        int      lv_mode;
        int      lv_start;                      /* start of variable part */
        int      lv_size;
        char     lv_name[16];
	int      lv_field;
};

struct ls {
	unsigned ls_hkid;
	int 	 ls_mode;
	char     ls_name[16];
	char     ls_location[16];
	char     ls_desc[50];
	int	 ls_rdev;
	int      ls_field;
};

struct la {
	unsigned la_hkid;
	int      la_mode;
	char     la_name[16];
	int	 la_value;
	int      la_field;
};

struct nm {
	unsigned nm_hkid;
	int	 nm_mode;
	int	 nm_st_mod;
	int	 nm_value;
	int	 nm_class;
	char	 nm_name[32];
	int      nm_field;
};

struct ldb {
	unsigned ldb_hkid;
	int	 ldb_mode;
	int	 ldb_binname;
	char	 ldb_symname[28];
	int      ldb_field;
};

struct lset {
	unsigned lset_hkid;
	int	 lset_mode;
	int	 lset_id;
	char	 lset_name[16];
	int      lset_field;
};

struct lprec {
	unsigned lprec_hkid;
	int	 lprec_mode;
	int	 lprec_binname;
	int	 lprec_setid;
	int	 lprec_classno;
	int      lprec_field;
};


#define VCPY2(nbbeg,to_array,nbend) \
        memset(to_array,0,sizeof(to_array)); \
        strncpy(to_array,&line[nbbeg],nbend);


#define VCPY(from,to_array) \
        memset(to_array,0,sizeof(to_array)); \
        strncpy(to_array,from,sizeof(to_array));

static char *rbuf;
static rsize;
static ridx;

static rwrite(buf,size)
char *buf;
{

        if(rbuf == 0) {
                rbuf = malloc(1024);
                ridx = 0;
                rsize = 1024;
        }
        while(size > rsize-ridx) {
                rsize += 1024;
                rbuf   = realloc(rbuf,rsize);
        }
        memcpy(rbuf + ridx,buf,size);
        ridx += size;
}

/*
 * NAME:     prhdr
 * FUNCTION: write the header and the ps -ek data to the logfile.
 *	     if nohdrflag, write only the baseconversion structure 
 *           we always need it for timestamps in trcrpt
 * INPUTS:   none
 * RETURNS:  none
 */
static prhdr()
{
        int msglen, hlen;
        FILE *fp;
        int i, n, j;
        char *cp;
        char line[256];
        struct stat statbuf;
        struct baseconversion baseconversion;
        struct currp currp;
        struct dv dv;
        struct ps ps;
        struct lv lv;
        struct gen_hdr *gp;
	struct wtrc_lv trc_lv[TRC_NLV];
	struct wtrc_lv *trc_lvp;
	struct ls ls;
	struct la la;
	struct nm nm;
	struct ldb ldb;
	struct lset lset;
	struct lprec lprec;
	char w_name[21];
	int writeflg;
	struct {
		int w_field;
	} field;

	/*
	 * This is required for conversion the timestamp from tic to
	 * nanosecond
	 */

#define SUBHKID_TBL	604
	baseconversion.b_hkid = HKWD_TRACE_UTIL | HKTY_G | SUBHKID_TBL;
	baseconversion.b_Xint = (_system_configuration.Xint?_system_configuration.Xint:1);
	baseconversion.b_Xfrac = (_system_configuration.Xfrac?_system_configuration.Xfrac:1);
	baseconversion.b_rtc_type = _system_configuration.rtc_type;
        rwrite(&baseconversion,sizeof(baseconversion));

	if (nohdrflag) {
		goto h;
	}

        /*
         * Current host id 
         */
        currp.cp_hkid = HKWD_TRACE_UTIL | HKTY_L | 5;
        currp.cp_pid  = gethostid();
        currp.cp_field = 0;
        rwrite(&currp,sizeof(currp));

        /*
         * Trace header
         */
        time(&Header.h_time);
        uname(&Header.h_uts);                           /* system info */
        msglen = MIN(strlen(Message),sizeof(Header.h_message));
        Header.h_msglen = msglen;
        hlen = OFFC(header,h_message) + msglen;
        strncpy(Header.h_message,Message,msglen);
        Header.h_hkid = HKWD_TRACE_HEADER | HKTY_V | hlen-4-4;
        Header.h_mode = 1;
        rwrite(&Header,HKWDTOWLEN(hlen)*4);
	rwrite(&field,sizeof(field));

        n = 0;
        for(i = 0; i < Argc; i++)
                n += strlen(Argv[i]) + 1;       /* 1 = delimiter */
        n = ALIGN(4,MIN(n,GENBUFSIZE));
        gp = (struct gen_hdr *)jalloc(sizeof(struct gen_hdr0) + n);
        gp->h_0.h_hkid = HKWD_TRACE_HEADER | HKTY_V | n;
        gp->h_0.h_mode = 2;
        rwrite(gp,sizeof(struct gen_hdr0));
        for(i = 0; i < Argc && n > 0; i++) {
                int m;

                m = MIN(strlen(Argv[i]),n);
                rwrite(Argv[i],m);
                rwrite(" ",1);
                n -= m+1;
        }
        while(--n >= 0)
                rwrite(" ",1);

	rwrite(&field,sizeof(field));

        /*
         * Process names and ids
         */
       if((fp = popen("/bin/ps -ekmo pid,ppid,comm,tid,bnd,pri,uid,gid,tty","r")) == 0) {
                perror("ps");
                goto a;
        }
        hlen = sizeof(ps);
        Debug("hlen=%d\n",hlen);
        ps.p_hkid = HKWD_TRACE_UTIL | HKTY_V | hlen-4-4-4;
        ps.p_mode = 1;
	ps.p_field = 0;
        fgets(line,sizeof(line),fp);                    /* drop ps header */
        while(fgets(line,sizeof(line),fp)) {
		writeflg = 0;
                cp = strtok(line," ");
                if (*cp != '-')
                        ps.p_pid = atoi(cp);
                cp = strtok(0," ");
                if (*cp != '-')
                        ps.p_ppid = atoi(cp);
                cp = strtok(0," ");
                if (*cp != '-') {
                        VCPY(cp,ps.p_name);
                        VCPY(cp,w_name);
                }
                else
                        VCPY(w_name,ps.p_name);
                cp = strtok(0," ");
                if (*cp != '-') {
                        ps.p_tid = atoi(cp);
                        writeflg++;
                }
                cp = strtok(0," ");
                if (*cp != '-')
                        ps.p_cpuid = atoi(cp);
                 else
                        ps.p_cpuid = -1;
		cp = strtok(0," ");
		ps.p_pri = atoi(cp);
		cp = strtok(0," ");
                if (*cp != '-') {
                        ps.p_uid = atoi(cp);
		}
		cp = strtok(0," ");
                if (*cp != '-') {
                        ps.p_gid = atoi(cp);
		}
		cp = strtok(0," ");
                if (*cp != '-') {
                        VCPY(cp,ps.p_tty);
		}
		else {
			if (writeflg == 0) {
				VCPY("-",ps.p_tty);
			}
		}

                if (writeflg) {
                	Debug("ps: name=%s pid=%d ppid=%d tid=%d cpuid=%d pri=%d\n", ps.p_name,ps.p_pid,ps.p_ppid,ps.p_tid,ps.p_cpuid,ps.p_pri);
                	Debug("ps: uid=%d gid=%d tty=%s\n", ps.p_uid,ps.p_gid,ps.p_tty);
                	rwrite(&ps,HKWDTOWLEN(hlen)*4);
		} 
        }
        pclose(fp);
a:
        /*
         * Loaded kernel extensions
         */
        if(ioctl(Systracefd,TRC_LOADER,trc_lv)) {
                perror("TRC_LOADER");
                goto b;
        }
        hlen = sizeof(lv);
        Debug("hlen=%d\n",hlen);
        lv.lv_hkid = HKWD_TRACE_UTIL | HKTY_V | hlen-4-4-4;
        lv.lv_mode = 2;
	lv.lv_field = 0;
        for(i = 0; i < TRC_NLV; i++) {
                trc_lvp = &trc_lv[i];
                /* if(trc_lvp->lv_size == 0) */
                        /* break; */
                if((trc_lvp->lv_size == 0) && (trc_lvp->lv_flags == 0))
                        break;
                lv.lv_start = (int)trc_lvp->lv_start;
                lv.lv_size  = trc_lvp->lv_size;
                trc_lvp->lv_name[sizeof(trc_lvp->lv_name)-1] = '\0';
                cp = trc_lvp->lv_name[0] ? trc_lvp->lv_name : trc_lvp->lv_name + 1;
                Debug("loader: %08x %04x %.16s\n",lv.lv_start,lv.lv_size,cp);
                VCPY(cp,lv.lv_name);
                rwrite(&lv,HKWDTOWLEN(hlen)*4);
        }
b:
        /*
         * /dev/directory
         */
        if((fp = popen("/bin/find /dev -print 2> /dev/null","r")) == 0) {
                perror("find");
                goto c;
        }
        hlen = sizeof(dv);
        Debug("hlen=%d\n",hlen);
        dv.dv_hkid = HKWD_TRACE_UTIL | HKTY_V | hlen-4-4-4;
        dv.dv_mode = 4;
	dv.dv_field = 0;
        while(fgets(line,sizeof(line),fp)) {
                line[strlen(line) - 1] = '\0';  /* get rid of '\n' */
                if(stat(line,&statbuf))
                        continue;
                switch(statbuf.st_mode & S_IFMT) {
                case S_IFCHR:
                case S_IFBLK:
                        break;
                default:
                        continue;
                }
                dv.dv_rdev = statbuf.st_rdev;
                VCPY(line+5,dv.dv_name);
                Debug("dv: name='%.16s' rdev=%08x\n",dv.dv_name,dv.dv_rdev);
                rwrite(&dv,HKWDTOWLEN(hlen)*4);
        }
        pclose(fp);
c:
        /*
         * Current process is trace (this one)
         */
        currp.cp_hkid = HKWD_TRACE_UTIL | HKTY_L | 3;
        currp.cp_pid  = getpid();
	currp.cp_field = 0;
        rwrite(&currp,sizeof(currp));

	if (!nmflag)
		goto h;
d:
	/*
	 *  List all the devices available in the system
	 *
	 * in commamd lsdev between name and location there are a tabulation
	 * and a "%" : these two characters are there to allow the use
	 * of strtok() routine (because sometime there is no information in
	 * location ).
	 * between location and description there is only one tabulation
	 */
	 if ((fp = popen("lsdev -C -Sa -F'name	#location	description'","r")) == 0) {   
		perror ("lsdev");
		goto e;
	}
	hlen = sizeof(ls);
	ls.ls_hkid = HKWD_TRACE_UTIL | HKTY_V | hlen-4-4-4;
	ls.ls_mode = 10;
	ls.ls_field = 0;
	while(fgets(line,sizeof(line),fp)) {
                if((cp = strtok(line,"\t")) == 0)     /* name */ 
                        continue; 
                VCPY(cp,ls.ls_name); 
                if ((cp = strtok(0,"\t")) == 0)	    /* location */
			continue;
                VCPY(cp+1,ls.ls_location);
                cp = strtok(0,"\n");		   /* description */ 
                VCPY(cp,ls.ls_desc); 
		sprintf(w_name,"%s%s","/dev/",ls.ls_name);
                if(stat(w_name,&statbuf) != 0)
                	ls.ls_rdev = 0;
		else
                	ls.ls_rdev = statbuf.st_rdev;
		Debug("ls : name = %.16s , location = %.16s , desc = %.50s , \
	      dev = %08x \n", ls.ls_name,ls.ls_location,ls.ls_desc,ls.ls_rdev);
		rwrite(&ls,HKWDTOWLEN(hlen)*4);
	}
	pclose(fp);
	
e:
	/* 
	 * size of dcache,icache and realmem
	 */
	hlen = sizeof(la);
	la.la_hkid = HKWD_TRACE_UTIL | HKTY_V | hlen-4-4-4;
	la.la_mode = 11;
	la.la_field = 0;

	/* dcache and icache no more accessible with lsattr */
	VCPY("dcache",la.la_name);
	la.la_value = _system_configuration.dcache_size/1024;
	Debug("la : name = %.16s , value = %d \n",
		la.la_name,la.la_value);
	rwrite(&la,HKWDTOWLEN(hlen)*4);

	VCPY("icache",la.la_name);
	la.la_value = _system_configuration.icache_size/1024;
	Debug("la : name = %.16s , value = %d \n",
		la.la_name,la.la_value);
	rwrite(&la,HKWDTOWLEN(hlen)*4);

	/* realmem accessed with lsattr */
	if ((fp = popen("lsattr -E -lsys0 -a realmem","r"))
			== 0)  {
		perror("lsattr");
		goto f;
	}
	while(fgets(line,sizeof(line),fp)) {
		if ((cp = strtok(line," \t\n")) == 0)
			continue;
		VCPY(cp,la.la_name);
		if ((cp = strtok(0," K\t\n")) == 0)
			continue;
		la.la_value = atoi(cp);
		Debug("la : name = %.16s , value = %d \n",
			la.la_name,la.la_value);
		rwrite(&la,HKWDTOWLEN(hlen)*4);
	}

	pclose(fp);
	VCPY("clock_test",la.la_name);
	la.la_value = clock_test();
	rwrite(&la,HKWDTOWLEN(hlen)*4);
f:
	/*
         * for each module of the loader entry list of all the symbols
         */

	hlen = sizeof(nm);
	nm.nm_hkid = HKWD_TRACE_UTIL | HKTY_V | hlen-4-4-4;
	nm.nm_mode = 12;
	nm.nm_field = 0;
	for(i=0; i< TRC_NLV; i++) {
 		cp = trc_lv[i].lv_fullname[0] ? trc_lv[i].lv_fullname :
						 trc_lv[i].lv_fullname + 1;
		Debug("nm : name = %s, size = %d, flags = %d \n",
		         cp, trc_lv[i].lv_size, trc_lv[i].lv_flags);
		 if ((trc_lv[i].lv_size == 0) && (trc_lv[i].lv_flags == 0)) 
			break;
		if ((trc_lv[i].lv_flags & 0x02) == 0)
			continue;
		find_symbol(cp);
		nm.nm_st_mod = trc_lv[i].lv_start;
		for(j= 0; j < Symtab_count; j++) {
			nm.nm_value = Symtab_base[j].r_value;
			nm.nm_class = (int)Symtab_base[j].r_symclass;
			VCPY(Symtab_base[j].r_name, nm.nm_name);
			rwrite(&nm,HKWDTOWLEN(hlen)*4);
		}
	}

g:
		
	/*
	 *  three structures for lock
	 */
	 if ((fp = popen("/usr/bin/cat /usr/lpp/perfagent/lockstat/lockname.db","r")) == 0) {   
		perror ("lockname.db");
		goto h;
	}
	hlen = sizeof(ldb);
	ldb.ldb_hkid = HKWD_TRACE_UTIL | HKTY_V | hlen-4-4-4;
        ldb.ldb_mode = 13;
	ldb.ldb_field = 0;
	while(fgets(line,sizeof(line),fp)) {
		if ((*line == ' ') || (*line == '\n')){
			continue;
		}
                cp = strtok(line," ");
		if (*cp == '#') {
                        continue; 
		}
                VCPY(cp,ldb.ldb_symname); 
                cp = strtok(0,"\n");
		ldb.ldb_binname = atoi(cp);
		Debug("ldb : binname =%d symname = %s \n",
			ldb.ldb_binname,ldb.ldb_symname);
        	rwrite(&ldb,HKWDTOWLEN(hlen)*4);
	}

	hlen = sizeof(lset);
	lset.lset_hkid = HKWD_TRACE_UTIL | HKTY_V | hlen-4-4-4;
        lset.lset_mode = 14;
	lset.lset_id = 0;
	lset.lset_name[0] = '\0';
	lset.lset_field = 0;
	Debug("lset : id =%d name = %s \n",lset.lset_id,lset.lset_name);
        rwrite(&lset,HKWDTOWLEN(hlen)*4);

	hlen = sizeof(lprec);
	lprec.lprec_hkid = HKWD_TRACE_UTIL | HKTY_V | hlen-4-4-4;
        lprec.lprec_mode = 15;
	lprec.lprec_binname = 0;
	lprec.lprec_setid = 0;
	lprec.lprec_classno = 0;
	lprec.lprec_field = 0;
	Debug("lprec : binname =%d setid = %d  classno=%d \n",
		lprec.lprec_binname,lprec.lprec_setid,lprec.lprec_classno);
        rwrite(&lprec,HKWDTOWLEN(hlen)*4);

h:

	if(!stdoutflag) {
		while ((sizeof(struct trc_log_hdr) +
	       		nentries * (sizeof(struct trc_log_entry) + Tracebufsize + TRC_OVF) + ridx) > Ublimit) {
			  nentries--;
		}
		lsize = (nentries * sizeof(struct trc_log_entry)) + sizeof(struct trc_log_hdr);
		logstart_ic = lseek(Logfd,lsize,0);
	}
	else {
		logstart_ic = lseek(Logfd,0,0);
	}

        write(Logfd,rbuf,ridx);
}

/*
 * wait for the trace demon to close /dev/systrace
 */
static trcsync(argc,argv)
char *argv[];
{
        int fd;

        if((fd = open("/dev/systrctl",0)) < 0) {
                perror("/dev/systrctl");
                exit(1);
        }
        if(ioctl(fd,TRCSYNC,0)) {
                perror("TRCSYNC");
                exit(1);
        }
        exit(0);
}

/*
	Program to measure the clock speed of a machine.
	Note the result is not directly in Mhz.
	It is a number that should be compared with a database that
	has been formed by running this program on different machines
	with different clock speeds.
*/

#define ARRAY_MAX	1000	/* max # of times to sample the nano register */
#define MAXCYCLETIME	100	/* nanoseconds for a 'slow' 10 Mhz */
#define MAXUPDATEFREQ	100	/* nano register max update freq */
#define MAXNANOALLOW	(MAXCYCLETIME * MAXUPDATEFREQ)

int clock_test()
{
	extern int read_nano() ;
	extern int read_tic() ;
	int test_Xint,test_Xfrac;
	int count = ARRAY_MAX;
	int i ;
	int nano[ARRAY_MAX + 1] ;
	int diff[ARRAY_MAX] ;
	int totdif = 0 ;
	int dcnt = 0 ;
	int zcnt = 0 ;

	if ( _system_configuration.rtc_type == RTC_POWER_PC ) {
		read_tic(nano, count) ;
		test_Xint = (_system_configuration.Xint?
				_system_configuration.Xint:1);
		test_Xfrac = (_system_configuration.Xfrac?
				_system_configuration.Xfrac:1);
		for(i=1; i < count+1 ; i++ ) {
			diff[i-1] = nano[i] - nano[i-1] ;
                	if ( test_Xfrac != 1 )
                        	diff[i-1] /= test_Xfrac;
                	diff[i-1] *= test_Xint;
			if(diff[i-1] > 0 && diff[i-1] < MAXNANOALLOW) {
				dcnt++ ;
				totdif += diff[i-1] ;
			}
			if(diff[i-1] == 0)
				zcnt++ ;
		}
	}
	else {
		read_nano(nano, count) ;

		for(i=1; i < count+1 ; i++ ) {
			diff[i-1] = nano[i] - nano[i-1] ;
			if(diff[i-1] > 0 && diff[i-1] < MAXNANOALLOW) {
				dcnt++ ;
				totdif += diff[i-1] ;
			}
			if(diff[i-1] == 0)
				zcnt++ ;
		}
	}
	Debug("\nAverage time is %d on %d dcnt & %d zcnt\n", 
			totdif/(dcnt+zcnt), dcnt, zcnt) ;
	return(totdif/(dcnt+zcnt));
}



static void find_symbol(filename)
char *filename;
{
	struct stat statbuf;

	vprint("Initializing namelist symbol tables\n");
	Symtab_count = 0;
	Unxfile = filename;
	Debug("Unxfile = %s \n", Unxfile);
	if((Unxfp = fopen(Unxfile,"r")) == 0) {
		vprint("Cannot open %s. %s\n",Unxfile,errstr());
		return;
	}
	if(stat(Unxfile,&statbuf)) {
		vprint("Cannot stat %s. %s\n",Unxfile,errstr());
		return;
	}
	if((statbuf.st_mode & S_IFMT) != S_IFREG) {
		vprint("%s is not a regular file\n",Unxfile);
		return;
	}

	Unxbase = shmat(fileno(Unxfp),0,SHM_MAP|SHM_RDONLY);
	if((int)Unxbase == -1) {
		vprint("map %s: %s\n",Unxfile,errstr());
		Unxbase = 0;
	}

		rptsym_init_aout(filename);
}


static rptsym_init_aout(filename)
char *filename;
{
	int i;

	filehdr_OFF = 0;
	aouthdr_OFF = filehdr_SIZE;
	jread(&filehdr,filehdr_OFF,sizeof(filehdr));
	jread(&aouthdr,aouthdr_OFF,sizeof(aouthdr));
	scnhdr_OFF  = filehdr_SIZE + aouthdr_SIZE;
	syment_OFF  = filehdr.f_symptr;

	if(filehdr.f_nsyms > 0)
		jread(&symflx_SIZE,IOFF(syment,filehdr.f_nsyms),4);

	Debug("nscns=%d\n",filehdr.f_nscns);
	delta_text = 0;
	delta_data = 0;
	for(i = 0; i < filehdr.f_nscns; i++) {	/* look for loader section */
		jread(&scnhdr,IOFF(scnhdr,i),sizeof(scnhdr));
		Debug("sections=%8.8s\n",scnhdr.s_name);
		Debug("vaddr=%x\n",scnhdr.s_vaddr);
		Debug("scnptr=%x\n",scnhdr.s_scnptr);
		if((scnhdr.s_flags & (STYP_LOADER|STYP_TYPCHK)) == STYP_LOADER) {
			ldhdr_OFF  = scnhdr.s_scnptr;
			jread(&ldhdr,ldhdr_OFF,sizeof(ldhdr));
			ldrflx_SIZE = ldhdr.l_stlen;
		}
		if (strcmp(filename,"/unix") != 0) {
			if(strcmp(scnhdr.s_name, _TEXT) == 0)
				delta_text = scnhdr.s_scnptr - scnhdr.s_vaddr;

			if(strcmp(scnhdr.s_name, _DATA) == 0)
				delta_data = scnhdr.s_scnptr - scnhdr.s_vaddr;
		}
	}
	/*
	 * Allocate symbol table
	 */
	base_init();	/* set _BASE and (conditionally) allocate memory */
	syminit();			/* install symbol table */
	ldrinit();			/* install ldr symbols */
	base_term();		/* free Unxfile allocations or mapped file */
	fclose(Unxfp);
}

static syminit()
{
	int i;
	int ind;
	char *name;
	char *short_name;
	char buf[16];

	Debug("syminit\n");
	if (filehdr.f_nsyms != 0)
		ldhdr_OFF = 0;
	for(i = 0; i < filehdr.f_nsyms; i++) {
		syment_cpy(i);
		if(syment.n_zeroes == 0) {
			name = symflx_BASE + syment.n_offset;
		} else {
			strncpy(buf,syment.n_name,8);
			buf[8] = '\0';
			name = buf;
		}
		i += syment.n_numaux;
		 if((syment.n_sclass != C_EXT)&& (syment.n_sclass != C_HIDEXT)
			&& (syment.n_sclass != C_FILE)) 
			 continue; 
		if (syment.n_sclass == C_FILE) {
			if ((strcmp(name,"noname") == 0) ||
			   		(strcmp(name," ") == 0)) {
				continue;
			}
			if (strcmp(name,".file") != 0) {
				rptsym_install(name,syment.n_value,syment.n_sclass);
				continue;
			}
			if( syment.n_numaux  == '0') {
				continue;
			}
			auxent_cpy(i - syment.n_numaux + 1);
			if (auxent.x_file._x.x_zeroes != 0) {
				strncpy(buf, basename(auxent.x_file.x_fname), 14);
				buf[14] = '\0';
				name = buf;
			}
			else {
				name = basename(symflx_BASE + auxent.x_file._x.x_offset);
			}
			rptsym_install(name,syment.n_value,syment.n_sclass);
			continue;
		}
		if( syment.n_numaux  == '0')
			continue;
		auxent_cpy(i);
	     if ((AUXE_TYPE(auxent) == XTY_LD) && (AUXE_CLAS(auxent) == XMC_PR))
		{
		rptsym_install(name,syment.n_value + delta_text,syment.n_sclass);
		}
	     if ((AUXE_TYPE(auxent) == XTY_CM) && (AUXE_CLAS(auxent) == XMC_TD))
		{
		rptsym_install(name,syment.n_value + delta_data,syment.n_sclass);
		}
	     if ((AUXE_CLAS(auxent) == XMC_RW)
	     		|| ((AUXE_TYPE(auxent) == XTY_SD) && (AUXE_CLAS(auxent) == XMC_DS))){
			if  ((short_name=(strrchr(name,'/'))) != NULL)
                		name = short_name + 1; 		/* short_name points to last / character */
			rptsym_install(name,syment.n_value + delta_data,syment.n_sclass);
		}
	}
	Debug("return from syminit\n");
}

static void ldrinit()
{
	int i;
	char *name;
	rptsym *sp;
	char buf[10];

	Debug("ldrinit\n");
	if(ldhdr_OFF == 0)
		return;
	for(i = 0; i < ldhdr.l_nsyms; i++) {
		ldsym_cpy(i);
		if (ldsym.l_value == 0)
			continue;
		if(ldsym.l_zeroes == 0) {
			name = ldrflx_BASE + ldsym.l_offset;
		} else {
			strncpy(buf,ldsym.l_name,8);
			buf[8] = '\0';
			name = buf;
		}
		rptsym_install(name,ldsym.l_value,(unsigned)(LDR_TYPE(ldsym)));
	}
	Debug("return from ldrinit\n");
}


rptsym *rptsym_install(name,value,class)
char *name;
{
	register rptsym *sp;

	sp = getsp();
	sp->r_value = value;
	sp->r_symclass = class;
	sp->r_name  = istracpy(name);
	return(sp);
}

#define RSYMSIZE sizeof(rptsym)

static rptsym *getsp()
{
	rptsym *sp;

	if(Symtab_base == 0) {
		Symtab_base = (rptsym *)jalloc(GETSP_INCR * RSYMSIZE);
		Symtab_size = GETSP_INCR;
	} else if(Symtab_count == Symtab_size) {
		Symtab_size += GETSP_INCR;
		Symtab_base = (rptsym *)realloc(Symtab_base,Symtab_size * RSYMSIZE);
	}
	return(&Symtab_base[Symtab_count++]);
}

#define ASTRCPY_INCR 128
static char *istracpy_base;
static istracpy_idx;

static char *istracpy(str)
char *str;
{
	int len;
	char *cp;

	len = strlen(str) + 1;
	if(len > ASTRCPY_INCR) {
		str[ASTRCPY_INCR-1] = 0;
		len = ASTRCPY_INCR;
	}
	if(istracpy_base == 0 || len + istracpy_idx > ASTRCPY_INCR) {
		istracpy_idx  = 0;
		istracpy_base = jalloc(ASTRCPY_INCR);
	}
	cp = &istracpy_base[istracpy_idx];
	istracpy_idx += len;
	strcpy(cp,str);
	return(cp);
}

static void jread(buf,offset,count)
char *buf;
int offset;
int count;
{

	Debug("jread(%x,%x,%x)\n",buf,offset,count);
	if(count == 0)
		return;
	if(Unxbase) {
		memcpy(buf,Unxbase + offset,count);
		return;
	}
	fseek(Unxfp,offset,0);
	while(--count >= 0)
		*buf++ = getc(Unxfp);
}

static base_init()
{

	if (Symtab_base != 0) {
		jfree(Symtab_base);
		Symtab_base = 0;
	}

	if(Unxbase) {
		syment_BASE = Unxbase + IOFF(syment,0);
		symflx_BASE = Unxbase + IOFF(syment,filehdr.f_nsyms);
		ldsym_BASE  = Unxbase + IOFF(ldhdr,1);
		ldrflx_BASE = Unxbase + ldhdr_OFF + ldhdr.l_stoff;
	} else {
		vprint("reading symbol table\n");
		syment_BASE = jalloc(filehdr.f_nsyms * syment_SIZE);
		jread(syment_BASE,IOFF(syment,0),filehdr.f_nsyms * syment_SIZE);

		vprint("reading flexname strings\n");
		symflx_BASE = jalloc(symflx_SIZE+4);
		jread(symflx_BASE,IOFF(syment,filehdr.f_nsyms),symflx_SIZE);

		/*
		 * Allocate loader symbol table
		 */
		vprint("reading ldr symbol table\n");
		ldsym_BASE = jalloc(ldhdr.l_nsyms * ldsym_SIZE);
		jread(ldsym_BASE,IOFF(ldhdr,1),ldhdr.l_nsyms * ldsym_SIZE);

		vprint("reading ldr flexname strings\n");
		ldrflx_BASE = jalloc(ldhdr.l_stlen + 4);
		jread(ldrflx_BASE,ldhdr_OFF + ldhdr.l_stoff,ldrflx_SIZE);
	}
}

static base_term()
{

	if(Unxbase) {
		shmdt(Unxbase);
		Unxbase = 0;
	} else {
		if(syment_BASE)
			free(syment_BASE);
		if(symflx_BASE)
			free(symflx_BASE);
		if(ldsym_BASE)
			free(ldsym_BASE);
		if(ldrflx_BASE)
			free(ldrflx_BASE);
		syment_BASE = 0;
		symflx_BASE = 0;
		ldsym_BASE  = 0;
		ldrflx_BASE = 0;
	}
}

call_exec (arg,load_flag)
int arg;
int load_flag;
{
pid_t pid;
int status;
int rc;
int excode;
char inter[80];
char inter2[80];

	pid = fork();
	if (pid < 0)
		exit(EXIT_EXEC);
	if (pid > 0) {
		rc = waitpid(pid,&status,NULL);
		excode = (signed char)WEXITSTATUS(status);
		if (rc <= 0 ) {
			Debug("call_exec : error in exec \n");
			exit(EXIT_EXEC);
		}
		if (excode < 0) {
			Debug("call_exec : return code = %d and load_flag = %d , arg = %d \n", excode,load_flag,arg);
			exit(excode);
		}
		if (load_flag)
			return(0);
		else
			exit(0);
	}
	sprintf (inter, "%d", (char)arg);
	sprintf (inter2, "%d", (char)load_flag);
       	execl("/usr/lib/trcload","trcload",inter,inter2,0);
}
	


/****************************************************************
 *
 * ROUTINE: initChild()
 *          returns  pid  of process "trace" whose parent is init
 *          returns    0  otherwise
 *
 ****************************************************************/
static pid_t
initChild()
{
        long              nproc = 0;
        struct procinfo  *procTable;


        /**********************************
         * Extract the kernel process table
         **********************************/
        procTable = (struct procinfo *) malloc ((size_t)sizeof(unsigned long));
        while ( (( nproc = getproc( procTable,
                                    nproc, sizeof(struct procinfo))) == -1) &&
                errno == ENOSPC)
        {

                nproc = *((long *) procTable) + 10;
                if ( procTable = (struct procinfo *) realloc ((void *)procTable,
                                   (size_t)(sizeof (struct procinfo) * nproc)) )
                    continue ;
                perror ("realloc: ");
                exit (1);
        }


        /*********************************************
         * Locate "trace" process whose parent is init
         *********************************************/
        while ( --nproc >= 0 )
            if ( procTable[ nproc ].pi_ppid == 1 ) {
               struct userinfo u_info;

               if ( getuser( &(procTable[ nproc ]), sizeof(struct procinfo),
                             &u_info, sizeof(struct userinfo)) ||
                    strcmp( "trace", u_info.ui_comm ))
                       continue ;
               return( procTable[ nproc ].pi_pid ) ;
            }

        return 0;
}
