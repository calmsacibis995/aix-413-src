static char sccsid[] = "@(#)03	1.4.1.4  src/bos/usr/samples/kernel/schedtune.c, kersamp, bos411, 9428A410j 6/9/94 09:02:34";

/*
 * COMPONENT_NAME: SYSPROC - Process Management 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <nlist.h>

extern int errno;
extern int optind;
extern char *optarg;

#define V_REPAGE_HI      0
#define V_REPAGE_PROC    1
#define V_WAIT_SECS    	 2
#define V_MIN_PROCESS    3
#define V_EXEMPT_SECS  	 4
#define PACEFORK 	 5
#define SCHED_D		 6
#define SCHED_R		 7
#define TIMESLICE	 8
#define MAXPARMS 	 9

static struct nlist nlst[] = {
        { "v_repage_hi" },
        { "v_repage_proc" },
        { "v_sec_wait" },
	{ "v_min_process" },
        { "v_exempt_secs" },
	{ "pacefork" },
	{ "sched_D" },
	{ "sched_R" },
	{ "timeslice" },
       	NULL 
};

struct  cmdtab {
	char	flag;				/* command flag; see below */
	int	index;				/* index into nlist        */
	void	(*invalid)();			/* validation routine      */
	int	current;			/* current value	   */
	int	vdefault;			/* default value	   */
};

static void v_repage_hi_x();
static void v_repage_proc_x();
static void v_wait_secs_x();
static void v_exempt_secs_x();
static void pacefork_x();
static void sched_d_x();
static void sched_r_x();
static void timeslice_x();

struct	cmdtab cmdtab[] = {
/*	flag,  nlst[i],         validation(),	     current, default    */

	{ 'h', V_REPAGE_HI, 	v_repage_hi_x, 		0,       6  },
	{ 'p', V_REPAGE_PROC, 	v_repage_proc_x, 	0,       4  },
	{ 'w', V_WAIT_SECS,	v_wait_secs_x,		0,	 1  },
	{ 'm', V_MIN_PROCESS, 	NULL, 			0,       2  },
	{ 'e', V_EXEMPT_SECS, 	v_exempt_secs_x, 	0,       2  },
	{ 'f', PACEFORK, 	pacefork_x, 		0,      10  },
	{ 'd', SCHED_D, 	sched_d_x, 		0,      16  },
	{ 'r', SCHED_R, 	sched_r_x, 		0,      16  },
	{ 't', TIMESLICE, 	timeslice_x, 		0,       1  },
	NULL,
};

static void rdwrval();
static void read_values();
static cmd_help();
static display_values();

int     kmem;

#define READ_MODE		0
#define WRITE_MODE		1

main(argc, argv)
        int argc;
        char **argv;
{
	int	i, c, value;

	nlist("/usr/lib/boot/unix", nlst);

	if (nlst[V_REPAGE_HI].n_value == 0) {
		perror("namelist on /usr/lib/boot/unix failed");
		exit(1);
	}

	kmem = open("/dev/kmem", O_RDWR, 0);
	if (kmem < 0) {
		perror("failed open of /dev/kmem");
		exit(1);
	}

	read_values();

	if (argc == 1)			/* display current values */
	{
		display_values();
		exit(0);
	}

	while ((c = getopt(argc, argv, "Dt:r:d:h:p:m:w:e:f:")) != EOF) {
		if (c == '?')
		{
			cmd_help();
			exit(1);
		}
		if (c == 'D')
		{
			for (i=0; i<MAXPARMS; i++)
			{
				cmdtab[i].current = cmdtab[i].vdefault;
				rdwrval(WRITE_MODE,i);
			}
			read_values();
			display_values();
			exit(0);
		}

		for(i=0; cmdtab[i].flag != c; i++);
		
		if (cmdtab[i].flag == c && nlst[i].n_value){
			value = atoi(optarg);

			if (cmdtab[i].invalid) 
				(*cmdtab[i].invalid)(i, value); 
			cmdtab[i].current = value;

			if (nlst[i].n_value)
				rdwrval(WRITE_MODE,i);
		}
	}

	display_values();
	close(kmem);
	exit(0);
}

static
cmd_help()
{
	printf("schedtune command\n");
	printf("\n");
	printf("PURPOSE\n");
	printf("\n");
	printf("\tSets parameters for the scheduler and process pacing.\n");
	printf("\n");
	printf("SYNTAX\n");
	printf("\n");
	printf("\tschedtune [-D] | [-h n][-p n][-w n][-m n][-e n][-f n][-r n][-d n][-t n]\n");
	printf("\n");
	printf("DESCRIPTION\n");
	printf("\n");

	printf("\tThe system scheduler performs load control by suspending\n");
	printf("\tprocesses when memory is over committed.  The system does not perform\n"); 
	printf("\treal IO swapping, instead pages are stolen as they are needed to fulfill\n");
	printf("\tthe current memory requirements.  Typically, pages are stolen from\n");
	printf("\tsuspended processes, since their pages have the best chance to age.\n");
	printf("\tMemory is considered over committed when the following condition is met:\n\n");

	printf("\t\tp * h > s, where p is the number of pages written to paging\n");
	printf("\t\t                   space in the last second,\n");
	printf("\t\t                 h is an integer specified by the -h flag, and\n");
	printf("\t\t                 s is the number of page steals that have\n"); 
	printf("\t\t                   occurred in the last second.\n\n"); 

	printf("\tA process is suspended when memory is over committed and the following\n");
	printf("\tcriterion is met:\n\n");
	printf("\t\tr * p > f, where r is the number of repages that the process\n");
	printf("\t\t                   has accumulated in the last second,\n");
	printf("\t\t                 p is an integer specified by the -p flag, and\n");
	printf("\t\t                 f is the number of page faults that the process\n");
	printf("\t\t                   has accumulated in the last second.\n\n");
	printf("\t\tIn addition, fixed priority processes and kernel processes are\n");
	printf("\t\texempt from being suspended.\n\n");

	printf("\tThe term 'repages' refers to the number of pages belonging to the\n");
	printf("\tprocess which were written to paging space or file space and are\n");
	printf("\tsoon after referenced again by the process.\n\n");
	
	printf("\tThe user also can specify a minimum multiprogramming level with the -m \n");
	printf("\tflag.  This assures that a minimum number of 'active' processes will\n");
	printf("\tremain throughout the process suspension period.  Active processes are\n");
	printf("\tdefined to be runnable processes and processes waiting for page I/O.\n"); 
	printf("\tProcesses that are waiting for events and processes that are suspended\n");
	printf("\tare not considered part of the active mix.  The wait process is not\n");
	printf("\tcounted as part of the active mix.\n\n");

	printf("\tSuspended processes can be added back into the mix, when the system has\n");
	printf("\tstayed below the first threshold for w seconds, where w is specified by\n");
	printf("\tthe -w flag.  Processes are added back into the system based first on\n");
	printf("\ttheir priority and second on the length of their suspension period.\n\n");

	printf("\tThe schedtune command also allows one to control the pacing of process\n");
	printf("\tcreation.  If not enough paging space is available to fulfill the \n");
	printf("\tcurrent fork request, then the system will delay a specified number of\n");
	printf("\tclock ticks before retrying (-f).  The system retries up to five times\n");
	printf("\tbefore failing the fork() system call with errno set to ENOMEM.\n");
	printf("\n");
	printf("FLAGS\n");
	printf("\n");
	printf("	If no flags are specified, the current values are printed.\n");
	printf("\n");
	printf("-D	Restores the default values.\n");

	printf("-h n   	Part of the system wide criterion used to determine when process\n");
	printf("        suspension begins and ends.\n"); 
	

	printf("-p n	Part of the per process criterion used to determine which processes\n");
	printf("        to suspend.\n");

	printf("-w n    The number of seconds to wait after thrashing ends before adding \n");
	printf("	processes back into the mix.\n");
	printf("-m n    The minimum multprogramming level.\n"); 
	printf("-e n	A recently resumed suspended process is eligible for resuspension,\n"); 
	printf("        when it has been active for n seconds.\n");
	printf("-f n    The number of clock ticks to delay before retrying a failed\n");
	printf("	fork call.  The system retries up to five times.\n");
	printf("-r n    The rate at which to accumulate cpu usage\n");
	printf("-d n    The factor used to decay cpu usage\n");
	printf("-t n	The number of 10ms time slices (SCHED_RR only)\n");
	printf("\n");
}

static
void
read_values()
{
	rdwrval(READ_MODE,V_REPAGE_HI);
	rdwrval(READ_MODE,V_REPAGE_PROC);
	rdwrval(READ_MODE,V_WAIT_SECS);
	rdwrval(READ_MODE,V_MIN_PROCESS);
	rdwrval(READ_MODE,V_EXEMPT_SECS);
	rdwrval(READ_MODE,PACEFORK);
	rdwrval(READ_MODE,SCHED_D);
	rdwrval(READ_MODE,SCHED_R);
	rdwrval(READ_MODE,TIMESLICE);
}

static
void
rdwrval(int mode, int index)
{
        if (lseek(kmem, nlst[index].n_value, SEEK_SET) == -1)
	{
		perror("rdwrval: second lseek");
		exit(1);
	}
	if (mode == READ_MODE)
	{
            if (read(kmem, &cmdtab[index].current, sizeof(int)) != sizeof(int))
	    {
		perror("rdwrval: second read");
		exit(1);
	    }
	}
	else 
	{
	    if (write(kmem, &cmdtab[index].current, sizeof(int)) != sizeof(int))
	    {
		perror("rdwrval: write");
		exit(1);
	    }
	}
}

static
display_values()
{
	char	outbuf[120];

	printf("\n"); 
printf("     THRASH           SUSP       FORK             SCHED\n");
printf("-h    -p    -m      -w    -e      -f       -d       -r        -t\n");
printf("SYS  PROC  MULTI   WAIT  GRACE   TICKS   SCHED_D  SCHED_R  TIMESLICE\n");

	sprintf( outbuf, 
       " %-d   %-d     %-d        %-d      %-d      %-d       %d       %d         %d\n", 
		 cmdtab[V_REPAGE_HI].current,
		 cmdtab[V_REPAGE_PROC].current,
		 cmdtab[V_MIN_PROCESS].current,
		 cmdtab[V_WAIT_SECS].current,
		 cmdtab[V_EXEMPT_SECS].current,
		 cmdtab[PACEFORK].current,
		 cmdtab[SCHED_D].current,
		 cmdtab[SCHED_R].current,
		 cmdtab[TIMESLICE].current);

	printf(outbuf);

	return(0);
}

/* validation routines */

static
void
v_repage_hi_x(int i, int value)
{
	if (value < 0) 
	{
		printf("Invalidating -p %d.\n", value);
		printf("Must be greater than or equal to zero\n");
		exit(1);
	}
}

static
void
v_repage_proc_x(int i, int value)
{
	if (value < 0)
	{
		printf("Invalidating -%c %d.\n", cmdtab[i].flag, value);
		printf("Must be greater than or equal to zero.\n");
		exit(1);
	}
}

static
void
v_wait_secs_x(int i, int value)
{
	if (value < 0)
	{
		printf("Invalidating -%c %d.\n", cmdtab[i].flag, value);
		printf("Must be greater than or equal to zero.\n");
		exit(1);
	}
}

static
void
v_exempt_secs_x(int i, int value)
{
	if (value < 0)
	{
		printf("Invalidating -%c %d.\n", cmdtab[i].flag, value);
		printf("Must be greater than or equal to zero.\n");
		exit(1);
	}
}

static
void
pacefork_x(int i, int value)
{
	if (value < 0)
	{
		printf("Invalidating -%c %d.\n", cmdtab[i].flag, value);
		printf("New value must be greater than or equal to zero\n");
		exit(1);
	}
}

static
void
sched_d_x(int i, int value)
{
	if (value < 0 || value > 32)
	{
		printf("Invalidating -%c %d.\n", cmdtab[i].flag, value);
		printf("New value must be greater than or equal to zero and less or equal to thirty-two\n");
		exit(1);
	}
}
static
void
sched_r_x(int i, int value)
{
	if (value < 0 || value > 32)
	{
		printf("Invalidating -%c %d.\n", cmdtab[i].flag, value);
		printf("New value must be greater than or equal to zero and less or equal to thirty-two\n");
		exit(1);
	}
}

static
void
timeslice_x(int i, int value)
{
	if (value < 0)
	{
		printf("Invalidating -%c %d.\n", cmdtab[i].flag, value);
		printf("New value must be greater than or equal to zero\n");
		exit(1);
	}
}
