static char sccsid[] = "@(#)28	1.31  src/bos/usr/sbin/crash/misc.c, cmdcrash, bos411, 9438B411a 9/20/94 11:59:49";

/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: fatal,error,atoi,init,getuarea,grmpu,
 *            getuarea_thread, grmpu_thread
 *
 * ORIGINS: 27,3,83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */
/*
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include	"crash.h"

#include	<sys/inode.h>
#include	<sys/proc.h>
#include	<sys/file.h>
#include	<errno.h>
#include 	<sys/thread.h>
#include	<sys/systemcfg.h>
#include    <nlist.h>
#include    <storclass.h>

#define DATA 0x2  /* for scnm field in nlist structure */

extern	struct	user	x;
extern  struct   uthread x_thread;
extern  char    *dumpfile;
extern  int	dumpflag;
extern cpu_t ncpus;
extern cpu_t selected_cpu;
extern	char	*namelist;
int	initialized = 0;
int arch_type;
/*
 * NAME: fatal
 *                                                                    
 * FUNCTION: This routine displays an error message and then causes the
 *		crash utility to exit.
 *                                                                    
 */  
fatal(str)
	char	*str;
{
	printf("%s\n", str);
	exit(1);
}


/*
 * NAME: atoi
 *                                                                    
 * FUNCTION: converts an ascii string to an integer.
 *                                                                    
 * RETURNS:
 *	-1	on error
 *	numb	in integer equivalent to the string
 */  
atoi(s)
	register  char  *s;
{
	register  base, numb;

	numb = 0;
	if(*s == '0')
		base = 8;
	else
		base = 10;
	while(*s) {
		if(*s < '0' || *s > '9') {
			printf( catgets(scmc_catd, MS_crsh, MISC_MSG_1, "Parameter supplied is not numeric.\n") );
			return(-1);
		}
		numb = numb * base + *s++ - '0';
	}
	return(numb);
}

/*
 * NAME:  init
 *                                                                    
 * FUNCTION: This routine is called from main and is used to initialize
 *		the symbol information needed by the various crash 
 *		subcommands.  It also opens the SystemImageFile and 
 *		KernelFile that will be used by crash.
 *                                                                    
 */  
init()
{
	extern	char	*namelist, *get_cdtname();
	struct	nlist	*symsrch();
	extern struct nlist nltbl[];
	extern 	int	bos_cdt, proc_cdt;
	extern int	thrd_cdt;
	char	*p;
	int	sigint();
	int	i;
	int rc ; /*Debug*/
	int procslot;
	int threadslot;
	int found;
	ulong	u_segval;
	int arcbuf;
	ulong addr;

	/*
	 * Open the SystemImageFile (dumpfile) that is to be used.  This is
	 * either the file that was designated on the command line or the
	 * default SystemImageFile, /dev/mem.
	 */
	if((mem = open(dumpfile, 0)) < 0) {
		printf( catgets(scmc_catd, MS_crsh, MISC_MSG_2, "0452-178: Cannot open dump file %s.\n") ,dumpfile);
		exit(1);
	}

	/* determine if we are working on a dump */
	for(i=1;i<MAXCDTS;i++) {
		if((p = get_cdtname(i))==0)
			break;
		/* look for bos cdt & save index */
		if(!strcmp(p,"bos"))
			bos_cdt = i;
		/* look for proc cdt & save index */
		else if(!strcmp(p,"proc"))
			proc_cdt = i;
		else if(!strcmp(p,"thrd"))
			thrd_cdt = i;
	}
	/* If it is a dump, set the global dumpflag */
	if(i > 1)
		dumpflag = 1;

	/* Read in the needed symbols from the symbol table or kernel */
	rdsymtab();

	/* Initialize crash's internal copy of the variables */

	V = symsrch("v");
	File = symsrch("file");
	Inode = symsrch("inode");
	Hinode = symsrch("hinode");
	Nhino = symsrch("nhino");
	Mount = symsrch("mount");
	Sys = symsrch("utsname");
	Time = symsrch("time");
	Lbolt = symsrch("lbolt");
	Panic = symsrch("panicstr");
	Rootvfs = symsrch("rootvfs");
	Gpa_vnode = symsrch("gpa_vnode");
	Vmker = symsrch("vmker");
	g_kxsrval = symsrch("g_kxsrval");
	Buf = symsrch("buf_hdr");
	Kernel_heap = symsrch("kernel_heap");
	Pinned_heap = symsrch("pinned_heap");
	Mbuf = symsrch("mbufs");
	MbufC = symsrch("mbclusters");
	K_anchor = symsrch("kernel_anchor");
	Ifnet = symsrch("ifnet");
	Proc = symsrch ("proc");
	Thread = symsrch("thread");
	Ublock = symsrch("__ublock");
	Ppd = symsrch("ppda");
	Sysconfig = symsrch("_system_configuration");
	abend_csa = symsrch("crash_csa");
	abend_code = symsrch("crash_LED");
	Dbp_fmodsw = symsrch("dbp_fmodsw");
	Dbp_dmodsw = symsrch("dbp_dmodsw");
	Dbp_sth_open_streams = symsrch("dbp_sth_open_streams");
	Dbp_streams_runq = symsrch("dbp_streams_runq");


	initialized = 1;

	/* Read in the var structure for the vars subcommand */
	if((rc=readmem(&v,(long)SYM_VALUE(V), sizeof(v), CURPROC)) != sizeof(v)){
		printf( catgets(scmc_catd, MS_crsh, MISC_MSG_3, 
		"0452-179: Cannot read v structure from address 0x%8x.\n") ,SYM_VALUE(V));
	}


	/* Read in the value for p_fmodsw - address of fmodsw structure */
	if((rc=readmem(&p_fmodsw,(long)SYM_VALUE(Dbp_fmodsw), sizeof(p_fmodsw), CURPROC)) 
				!= sizeof(p_fmodsw)){
		printf( catgets(scmc_catd, MS_crsh, MISC_MSG_19, 
			"0452-810: Cannot read %1$s variable from address 0x%2$8x.\n"), "dbp_fmodsw", SYM_VALUE(Dbp_fmodsw));
	}
	
	/* Read in the value for p_dmodsw - address of dmodsw structure */
	if((rc=readmem(&p_dmodsw,(long)SYM_VALUE(Dbp_dmodsw), sizeof(p_dmodsw), CURPROC)) 
				!= sizeof(p_dmodsw)){
		printf( catgets(scmc_catd, MS_crsh, MISC_MSG_19, 
			"0452-810: Cannot read %1$s variable from address 0x%2$8x.\n"), "dbp_dmodsw", SYM_VALUE(Dbp_dmodsw));
	}

	/* Read in the value for p_sth_open_streams - address of sth_open_streams */
	if((rc=readmem(&p_sth_open_streams,(long)SYM_VALUE(Dbp_sth_open_streams), sizeof(p_sth_open_streams), CURPROC)) 
					!= sizeof(p_sth_open_streams)){
		printf( catgets(scmc_catd, MS_crsh, MISC_MSG_19, 
			"0452-810: Cannot read %1$s variable from address 0x%2$8x.\n"), "dbp_sth_open_streams", SYM_VALUE(Dbp_sth_open_streams));
	}

	/* Read in the value for p_streams_runq - address of streams_runq  */
	if((rc=readmem(&p_streams_runq,(long)SYM_VALUE(Dbp_streams_runq), sizeof(p_streams_runq), CURPROC)) != 
				sizeof(p_streams_runq)){
		printf( catgets(scmc_catd, MS_crsh, MISC_MSG_19, 
			"0452-810: Cannot read %1$s variable from address 0x%2$8x.\n"), "dbp_streams_runq", SYM_VALUE(Dbp_streams_runq));
	}

	v.ve_file = (char *)(((unsigned)v.ve_file - File->n_value) /
		sizeof (struct file));
	v.ve_proc = (char *)(((unsigned)v.ve_proc - Proc->n_value) /
		sizeof (struct proc));
	v.ve_thread = (char *)(((unsigned)v.ve_thread - Thread->n_value) /
		sizeof (struct thread));
	signal(SIGINT, (void (*)(int))sigint);

	if (!dumpflag) {
		/* Determine which machine architecture dump was taken */ 
		arch_type = _system_configuration.architecture;

		/* get the number of processors of the machine */
		ncpus = _system_configuration.ncpus;
	}
	else { /* on a dump */
		ulong offset;
		int number_cpus;

		/* Determine which machine architecture dump was taken */
		offset = (ulong)&(_system_configuration.architecture) - (ulong)&(_system_configuration);
		if ((rc = readmem(&arch_type, SYM_VALUE(Sysconfig)+offset, sizeof(int), 0)) != sizeof(int)) {
			printf( catgets(scmc_catd, MS_crsh, MISC_MSG_10,
			"0452-800: Cannot read the machine architecture at address 0x%08x\n"),
			SYM_VALUE(Sysconfig)+offset);
		}
		/* get the number of processors of the machine */
		offset = (ulong)&(_system_configuration.ncpus) - (ulong)&(_system_configuration);
		if ((rc = readmem(&number_cpus, SYM_VALUE(Sysconfig)+offset, sizeof(int), 0)) != sizeof(int)) {
			printf( catgets(scmc_catd, MS_crsh, MISC_MSG_4,
			"0452-180: Cannot read the number of cpus at address 0x%08x\n"),
			SYM_VALUE(Sysconfig)+offset);
		}
		else ncpus = (cpu_t)number_cpus;
	}
	/* get the default selected cpu */
	if (ncpus == 1)
		selected_cpu = 0;
	else {			/* ncpus > 1 */
		if (!dumpflag) {
			do {
				if ((threadslot = get_slot_curthread()) != NONE){
					if ((procslot = associated_proc(threadslot)) != BADSLOT) {
						if ((rc = getuarea(procslot,&u_segval)) != (-1)) {
							/* check if crash or pstat is currenty executing on this processor */
							if (!(strncmp(x.U_comm, "crash", 5)) || !(strncmp(x.U_comm, "pstat", 5)))
								found++;
						}
						if (!found)
							selected_cpu++;
					}
				}
			} while ((!found) && (threadslot != NONE)); 
			if (!found) {
				printf( catgets(scmc_catd, MS_crsh, MISC_MSG_11, 
				"0452-801: Cannot find a default cpu. Use the cpu command to select one\n"));
				selected_cpu = 0;
			}
		}
		else {		/* we are on a dump */
			struct nlist cpuid[2];
			long selected_cpu_addr;
			
			/* try ro read crash_cpuid in the namelist that indicates which
			   cpu caused the dump */
			cpuid[0].n_name = "crash_cpuid";
			cpuid[1].n_name = "";
			if ((rc = nlist(namelist, &cpuid[0])) == 0) {
				if ((cpuid[0].n_sclass == C_HIDEXT) && (cpuid[0].n_scnum == DATA)) {
					/* nlist read the TOC entry, need to be dereferenced */
					if (readmem(&selected_cpu_addr, (long) cpuid[0].n_value, sizeof (long), 0) != sizeof(cpu_t))
						selected_cpu_addr = 0;
				}
				else selected_cpu_addr = (long)cpuid[0].n_value;
                                selected_cpu = 0;
                                if (selected_cpu_addr != NULL)
                                        if (readmem(&selected_cpu, (long) selected_cpu_addr,
						 sizeof (cpu_t), 0) != sizeof(cpu_t))
                                                selected_cpu = 0;
			}
			/* we did not find the variable in the namelist */
			else selected_cpu = 0;
		}
	}
}


/*
 *	Gather the uarea together and put it into the structure x
 *
 *	The particular uarea is selectable by the process table slot.
*/
getuarea(slot,segval)
ulong	*segval;	/* segid returned so iorb can be retrieved */
{
	struct proc pbuf;
	int	i;
	ulong	addr, p_segval;
	int	threadslot;
	int ret;


	/* Check to see if we are defaulting to the current process */
	if(slot == CURPROC) {
		/* Get the address of curproc either from memory or the dump */
		/* get the current thread slot */
		if ((threadslot = get_slot_curthread()) == -1) {
			printf( catgets(scmc_catd, MS_crsh, MISC_MSG_15, "0452-805: Cannot read current thread slot\n"));
			return(-1);
		}
		/* get the slot of the process associated to this thread */
		if ((slot = associated_proc(threadslot)) == BADSLOT) {
			printf( catgets(scmc_catd, MS_crsh, MISC_MSG_16, "0452-806: Cannot read current process slot\n"));
			return (-1);
		}
	}

	addr = SYM_VALUE(Proc) + slot*sizeof(struct proc);
	if ((ret = read_proc(addr, &pbuf)) != sizeof (struct proc)) {
		printf( catgets(scmc_catd, MS_crsh, MISC_MSG_5, "0452-181: Cannot read process table entry %d.\n") ,slot);
		return (-1);
	}
	
	*segval = pbuf.p_adspace;

	/* read in the user area associated with this process, then return */
	return(grmpu(&pbuf, slot));
}


/*
 * NAME: grmpu
 *                                                                    
 * FUNCTION: Reads the user area associated with the given process
 * 		into the global structure x
 *                                                                    
 * RETURNS:
 *	-1	on an error
 *	 0	on successful completion
 */  
grmpu(p, slot)
register struct proc *p;
int slot;
{
	long usegid;

	/* if this is not a dump */
	if(!dumpflag) {
  		/* get seg info of uarea */
		if(p->p_stat==SNONE) {
			return (-1);
		}
		usegid = p->p_adspace;

		/* Seek in memory to the u area */
		lseek(mem, (SYM_VALUE(Ublock)+sizeof(struct uthread)), 0);

		/* read in the uarea */
		if (readx(mem, (char *)&x, sizeof(struct user), usegid) 
			!= sizeof(struct user)){
			printf( catgets(scmc_catd, MS_crsh, MISC_MSG_8, "0452-184: Cannot read uarea for this process.\n") );
			return(-1);
		}
	}
	/* else if this is a dump */
	else {
		/* read the uarea in from the dump file */
		if (readmem(&x, SYM_VALUE(Ublock)+sizeof(struct uthread), sizeof(struct user), slot)
		    != sizeof(struct user)) {
			printf( catgets(scmc_catd, MS_crsh, MISC_MSG_9, "0452-185: Cannot read uarea for this process.\n") );
			return(-1);
		}

		/* return an error if the uarea was not in the dump */
		if (*(ulong *)&x == 0xbad)
			return (SWAPPED);
	}
	return(0);
}

/*
 *	Gather the uthread together and put it into the structure x_thread
 *
 *	The particular uthread area is selectable by the thread table slot.
 */
getuarea_thread(tslot,segval)
int	tslot;
ulong	*segval;	/* segid returned so iorb can be retrieved */
{
	struct thread tbuf;
	struct proc   pbuf;
    int  pslot;
	int  ret;

	/* Check to see if we are defaulting to the current thread */
	if(tslot == CURTHREAD) {
		if ((tslot = get_slot_curthread()) == (-1)) 
			return(-1);
	}
	/* get the slot of the process associated to this thread */
	pslot = associated_proc(tslot);

	/* read in the thread structure */
	if ((ret = read_thread(tslot, &tbuf)) != sizeof (struct thread))
		return (-1);
	/* read in the associated proc structure */
	if ((ret = read_proc((ulong)tbuf.t_procp, &pbuf)) != sizeof (struct proc))
		return (-1);

	/* get the segment register value */
	*segval = pbuf.p_adspace;

	/* read in the user area associated with this thread, then return */
	return(grmpu_thread(&tbuf, segval, tslot));
}


/*
 * NAME: grmpu_thread
 *                                                                    
 * FUNCTION: Reads the user area associated with the given thread
 * 		into the global structure x_thread
 *                                                                    
 * RETURNS:
 *	-1	on an error
 *	 0	on successful completion
 */  
grmpu_thread(t, segval, threadslot)
register struct thread *t;
ulong   *segval;
int threadslot;
{
	long usegid;

	/* if this is not a dump */
	if(!dumpflag) {
  		/* get seg info of uarea */
		if(t->t_state==SNONE) 
			return (-1);

		usegid = *segval;

		/* Seek in memory to the u area */
		lseek(mem, t->t_uaddress.uthreadp, 0);

		/* read in the uarea */
		if (readx(mem, (char *)&x_thread, sizeof(struct uthread), usegid) 
		  != sizeof(struct uthread)){
			printf( catgets(scmc_catd, MS_crsh, MISC_MSG_17, "0452-807: Cannot read uthread area for this thread.\n") );
			return(-1);
		}
	}
	/* else if this is a dump */
	else {
		/* read the uthread area in from the dump file */
		if (readmem_thread(&x_thread, t->t_uaddress.uthreadp, sizeof(struct uthread), threadslot)
		    != sizeof(struct uthread)) {
			printf( catgets(scmc_catd, MS_crsh, MISC_MSG_18, "0452-808: Cannot read uthread area for this thread.\n") );
			return(-1);
		}

		/* return an error if the uthread area was not in the dump */
		if (*(ulong *)&x_thread == 0xbad)
			return (SWAPPED);
	}
	return(0);
}
