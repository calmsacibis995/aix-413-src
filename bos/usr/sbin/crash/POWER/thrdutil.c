static char sccsid[] = "@(#)41	1.1  src/bos/usr/sbin/crash/POWER/thrdutil.c, cmdcrash, bos411, 9428A410j 10/15/93 10:25:38";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: read_thread, associated_proc, read_proc, get_slot_curthread
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */


#include "crash_msg.h"
#include "crash.h"
#include <sys/thread.h>
#include <sys/proc.h>
#include <sys/ppda.h>

#define THRDSIZE sizeof (struct thread)
#define PPDSIZE  sizeof(struct ppda)

extern nl_catd scmc_catd;	/* Cat descriptor for scmc conversion */
extern int dumpflag;
extern cpu_t selected_cpu;
extern struct thread global_thread;
extern struct proc   global_proc;
extern int thread_stored;
extern int   proc_stored;

/*
 * NAME : read_thread
 *
 * FUNCTION : reads a thread structure.
 *
 * RETURN:
 * 	-1	if the structure was not read
 *	size of the read structure	otherwise
 */

int 
read_thread(int    threadslot,
			struct thread *tbuf)
{
	ulong 		  segval_kx;
	
	if (dumpflag) {
		if (threadslot != thread_stored) { 
			/* read in the dump file */
			if ((readmem_thread(tbuf, SYM_VALUE(Thread)+threadslot*THRDSIZE, 
			THRDSIZE, threadslot)) != THRDSIZE) {
				printf(catgets(scmc_catd, MS_crsh, THRDUTIL_MSG_1, 
				"0452-1000: Cannot read thread table entry %3d.\n"), threadslot);
				return(-1);
			}
			/* return if the thread was not in the dump */
			if (*(ulong *)tbuf == 0xbad) {
				printf(catgets(scmc_catd, MS_crsh, THRDUTIL_MSG_2, 
				"0452-1001: Thread entry %d is not in the dump.\n"), threadslot);
				return (-1);
			}
			/* store the structure */
			memcpy(&global_thread,tbuf,THRDSIZE);
			thread_stored = threadslot;
		}
		else {
			/* structure was already read */
			memcpy(tbuf, &global_thread, THRDSIZE);
		}
	}
	else {
		/* read the segment value */
		if ((readmem(&segval_kx, SYM_VALUE(g_kxsrval), sizeof g_kxsrval, 0)) 
			!= sizeof g_kxsrval) {
				printf( catgets(scmc_catd, MS_crsh, THRDUTIL_MSG_3, 
				"0452-1002: Cannot read extension segment value from address 0x%8x\n") , SYM_VALUE(g_kxsrval));
				return (-1);
			}
		lseek(mem, (SYM_VALUE(Thread)+threadslot*THRDSIZE)&OFF_MSK, 0);
		/* read in memory */
		if ((readx(mem, (char *)tbuf, THRDSIZE, segval_kx)) != THRDSIZE) {
			printf( catgets(scmc_catd, MS_crsh, THRDUTIL_MSG_4, 
			"0452-1003: Cannot read thread table entry %3d.\n") , threadslot);
			return (-1);
		}
	}
	return (sizeof (*tbuf));
}
/*
 * NAME : associated_proc
 *
 * FUNCTION : seeks the process slot number associated to the thread slot number.
 *
 * RETURN:
 * 	BADSLOT		if no slot was found
 *	process slot 	otherwise
 */

int 
associated_proc (int threadslot)
{
	ulong 	segval_kx;
	struct  thread thread;
	int		procslot;
	
	if ((read_thread(threadslot, &thread)) != sizeof (struct thread))
		procslot = BADSLOT;
	else
		procslot = ((int)thread.t_procp - SYM_VALUE(Proc))/sizeof (struct proc);
	return (procslot);
}


/*
 * NAME : read_proc
 *
 * FUNCTION : reads the process structure indicated by the address given as argument.
 *
 * RETURN:
 * 	-1				if the structure was not read
 *	size of the read structure	otherwise
 */

int 
read_proc(ulong 		proc_addr, 
		  struct proc	*proc)
	
{
	ulong 	segval_kx;
	int		procslot;		
	
	procslot = ((int)proc_addr - (ulong)SYM_VALUE(Proc))/sizeof (struct proc);
	if (dumpflag) {
		if (procslot != proc_stored) {
			/* read in the associated proc structure */
			if(readmem(proc, proc_addr, sizeof (struct proc), procslot) != sizeof(struct proc)) {
				printf( catgets(scmc_catd, MS_crsh, THRDUTIL_MSG_5, 
								"0452-1004: Cannot read process table entry %d.\n") ,procslot);
				return (-1);
			}
			/* return if the process was not in the dump */
			if (*(ulong *)proc == 0xbad)
				return (-1);
			/* store the structure */
			memcpy(&global_proc,proc,sizeof(struct proc));
			proc_stored = procslot;
		}
		else {
			/* structure was already read */
			memcpy(proc, &global_proc, sizeof(struct proc));
		} 
	}
	else {
		/* Find the memory segment for the proc table */
		if(readmem(&segval_kx, SYM_VALUE(g_kxsrval), sizeof g_kxsrval, 
		   CURPROC) != sizeof g_kxsrval) {
			printf( catgets(scmc_catd, MS_crsh, THRDUTIL_MSG_6, 
			"0452-1005: Cannot read extension segment value from address 0x%8x.\n") ,
			SYM_VALUE(g_kxsrval));
			return (-1);
		}
		/* seek to the appropriate process slot in the process table */
		lseek(mem, proc_addr&OFF_MSK, 0);
		
		/* read in the proc structure */
		if (readx(mem, (char *)proc, sizeof(struct proc),segval_kx)
			!= sizeof(struct proc)){
			printf( catgets(scmc_catd, MS_crsh, THRDUTIL_MSG_7, 
			"0452-1006: Cannot read proc table entry %d.\n") ,procslot);
			return (-1);
		}
	}
	return (sizeof (*proc));
}

/*
 * NAME : get_slot_curthread
 *
 * FUNCTION : seeks the thread slot number of the current thread
 *
 * RETURN:
 * 	-1		if no slot was found
 *	thread slot 	otherwise
 */

int 
get_slot_curthread()
{
	int slot;
	ulong curthread_addr;
	struct ppda  ppdata;
	
	/* Get the ppda */
	if (readmem(&ppdata, SYM_VALUE(Ppd)+ selected_cpu * PPDSIZE, PPDSIZE, 0) != PPDSIZE) {
			printf( catgets(scmc_catd, MS_crsh, THRDUTIL_MSG_8, 
			"0452-1007: Cannot read ppda structure from address 0x%8x\n"), SYM_VALUE(Ppd));
			return (-1);
	}
	/* Get the address of the current thread */
	curthread_addr = (ulong)ppdata._curthread;
	if (curthread_addr == NULL)
		return(NONE);
	slot = (curthread_addr - SYM_VALUE(Thread)) / sizeof(struct thread);
	return (slot);
}
		
