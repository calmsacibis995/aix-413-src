static char sccsid[] = "@(#)40	1.7  src/bos/usr/sbin/crash/POWER/thread.c, cmdcrash, bos41J, rgj07 2/10/95 10:36:06";
/*
 *   COMPONENT_NAME: (CMDCRASH) 
 *
 *   FUNCTIONS: prthread, print_t_flags 
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   LEVEL 1,5 Years Bull Confidential Information
 */

#include	"crash.h"
#include	<sys/thread.h>
#include	<sys/proc.h>
#include    <sys/sched.h>
extern struct user x;
extern int dumpflag;

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/* 
 * NAME: prthread
 *
 * FUNCTION: Print a thread table slot.  
 *
 * RETURNS : nothing.
 */

prthread (short format, short status, int slot)
{
	struct thread thread;
	struct thread	*t;
	struct proc process;
	struct proc	*proc;
	int	procslot;
	ulong	u_segval;
	char	*cp;
	int	ret;

	if(slot > (int)v.ve_thread) {
		printf(catgets(scmc_catd, MS_crsh, THREAD_MSG_1, 
		"0452-950: Thread slot %d above maximum limit.Specify a value less than %d.\n") , slot,v.ve_thread);
		return;
	}
	if (read_thread(slot,&thread) != sizeof (struct thread))
		return;

	t = &thread;

	if ((status == RUNNING) && (t->t_state != TSRUN))
		return;
	if (t->t_state == TSNONE)
		return;
	if (format == LONG) {
		printf( catgets(scmc_catd, MS_crsh, THREAD_MSG_2, 
		"SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME\n") );
    }

	switch(t->t_state) {
		case TSNONE:  cp = " "; break;
		case TSIDL:   cp =  catgets(scmc_catd, MS_crsh, THREAD_MSG_3,  "i") ; break;
		case TSRUN:   cp =  catgets(scmc_catd, MS_crsh, THREAD_MSG_4,  "r") ; break;
		case TSSWAP:  cp =  catgets(scmc_catd, MS_crsh, THREAD_MSG_5,  "w") ; break;
		case TSSLEEP: cp =  catgets(scmc_catd, MS_crsh, THREAD_MSG_6,  "s") ; break;
		case TSSTOP:  cp =  catgets(scmc_catd, MS_crsh, THREAD_MSG_7,  "t") ; break;
		case TSZOMB:  cp =  catgets(scmc_catd, MS_crsh, THREAD_MSG_8,  "z") ; break;
		default:      cp =  catgets(scmc_catd, MS_crsh, THREAD_MSG_10, "?") ; break;
	}
	printf("%3d %s", slot, cp);

	read_proc(t->t_procp, &process);

	proc = &process;

	printf("%8x %8x ", t->t_tid, proc->p_pid);
	
	if (t->t_cpuid == 0xffffffff)   /* thread is not bound */
		printf(" unbound ");
	else
		printf("%8x ", t->t_cpuid);

	switch(t->t_policy) {
	case SCHED_FIFO :   printf("   FIFO"); break;
	case SCHED_RR	:   printf("     RR"); break;
	default		    :   printf("  other"); break;
	}

	printf(" %3x %3x", t->t_pri & PMASK,
			(t->t_cpu < T_CPU_MAX) ? t->t_cpu : T_CPU_MAX);
	t->t_wchan == 0 ? printf("          ") : printf(" %08x ", t->t_wchan);

	/* Get command name */
	if(proc->p_stat == SNONE)
		cp = "       ";
	else if(proc->p_stat == SZOMB)
		cp = "ZOMBIE";
	else if(!proc->p_pid)  			/* pid 0 is swapper */
		cp = " swapper ";
	else {
		/* get the uarea of the process */
		procslot = ((int)t->t_procp - SYM_VALUE(Proc))/sizeof (struct proc);
		if ((ret=getuarea(procslot,&u_segval)	) == -1) {
			printf( catgets(scmc_catd, MS_crsh, THREAD_MSG_11, "0452-951: Cannot read uarea for the process associated to this thread.\n") );
			return;
		}
		cp = x.U_comm;
	}
	if(!proc->p_pid)	/* just to make it all fit on 1 line */
		printf(" %s \n", cp);
	else
		printf(" %8s  \n", cp);

	print_t_flags(t); /* now print t_flags */
	print_t_atomic(t); /* now print t_atomic flags */

	/* end of short listing */
	if(format == SHORT)
		return;
	/* else do a long listing */

/* Main thread link pointers */
	printf("\nLinks:  *procp:0x%08x  *uthreadp:0x%08x  *userp:0x%08x\n",
		t->t_procp, t->t_uthreadp, t->t_userp);
	printf("    *prevthread:0x%08x  *nextthread:0x%08x,  *stackp:0x%08x\n",
		t->t_prevthread, t->t_nextthread, t->t_stackp);

	printf("    *wchan1(real):0x%08x  *wchan2(VMM):0x%08x *swchan:0x%08x\n",
		t->t_wchan1, t->t_wchan2,t->t_swchan);
	printf("    wchan1sid:0x%08x  wchan1offset:0x%08x\n",
		t->t_wchan1sid, t->t_wchan1offset);
	printf("    pevent:0x%08x  wevent:0x%08x  *slist:0x%08x\n",
		t->t_pevent, t->t_wevent, t->t_slist);

/* Dispatch fields */
      printf("Dispatch Fields:  *prior:0x%08x  *next:0x%08x\n",
		t->t_prior,t->t_next);
      printf("    polevel:0x%08x  ticks:0x%04x  *synch:0x%08x  result:0x%08x\n",
		t->t_polevel, t->t_ticks, t->t_synch, t->t_result);
      printf("    *eventlst:0x%08x  *wchan(hashed):0x%08x  suspend:0x%04x\n",
		t->t_eventlist,t->t_wchan,t->t_suspend);
      printf("    thread waiting for: %s%s%s%s%s%s%s%s%s%s%s%s\n",  
				/* be sure number of "%s" entries */
			      /* matches number of expressions below */
		t->t_wtype == TNOWAIT ? " nothing " : "",
		t->t_wtype == TWEVENT ? " event(s) " : "",
		t->t_wtype == TWLOCK ? " serial lock " : "",
		t->t_wtype == TWLOCKREAD ? " serial read lock " : "",
		t->t_wtype == TWTIMER ? " timer " : "",
		t->t_wtype == TWCPU ? " cpu (ready) " : "",
		t->t_wtype == TWPGIN ? " page in " : "",
		t->t_wtype == TWPGOUT ? "page out" : "",
		t->t_wtype == TWPLOCK ? " physical lock " : "",
		t->t_wtype == TWFREEF ? " free page frame " : "",
		t->t_wtype == TWMEM ? " memory " : "",
		t->t_wtype == TWUEXCEPT ? " exception " : "");

/* Scheduler info */
	printf("Scheduler Fields:  cpuid:0x%04x  scpuid:0x%04x  pri:%3u",
		t->t_cpuid, t->t_scpuid, t->t_pri);

	printf("  policy:");
	switch(t->t_policy) {
	case SCHED_FIFO :   printf("FIFO\n"); break;
	case SCHED_RR	:   printf("RR\n"); break;
	default	:   printf("other\n"); break;
	}

	printf("    affinity:0x%04x  cpu:0x%04x",
		t->t_affinity, t->t_cpu);
	printf("    lpri:%3u  wpri:%3u", t->t_lockpri, t->t_wakepri);
	printf("    time:0x%02x  sav_pri:0x%02x\n", t->t_time, t->t_sav_pri);

/* Misc */
	printf("Misc:  lockcount:0x%08x  ulock:0x%08x  *graphics:0x%08x\n",
		t->t_lockcount, t->t_ulock, t->t_graphics);
	printf("    dispct:0x%08x  fpuct:0x%08x  *cancel:0x%08x\n",
		t->t_dispct, t->t_fpuct, t->t_cancel);

/* Signal Fields */
	printf("Signal Information:  cursig:0x%02x  *scp:0x%08x\n", t->t_scp);
	printf("    pending:hi 0x%08x,lo 0x%08x  sigmask:hi 0x%08x,lo 0x%08x\n",
			t->t_sig.hisigs, t->t_sig.losigs,
			t->t_sigmask.hisigs, t->t_sigmask.losigs);

	printf("\n\n");
}  /* end prthread()*/

print_t_atomic(t)
struct thread *t;
{
	unsigned int flags;
	char buf[128];
	
	buf[0] = '\0';
	flags = t->t_atomic;
	if (flags & TSIGDELIVERY)
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_16, " sigdelivery") );  
	if (flags & TSELECT) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_18, " sel") );  
	if (buf[0] != '\0')
		printf("\t         %s\n", buf);
}

print_t_flags(t)
struct thread *t;
{
	unsigned int flags;
	char buf[128];
	
	buf[0] = '\0';
	flags = t->t_flags;
	if (flags & TLOCAL) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_12, " local") );  
	if (flags & TSIGSLIH)
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_13, " sigslih") );  
	if (flags & TTRCSIG)
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_14, " trcsig") );  
	if (flags & TOMASK)
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_15, " omask") );  
        if (flags & TWAKEONSIG) 
		strcat(buf, " wakeonsig");  
        if (flags & TPRIOBOOST) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_19, " prioboost") );  
	flags &= ~(TTRCSIG | TSIGSLIH | TOMASK | TWAKEONSIG | TLOCAL | TPRIOBOOST);

	/* we don't want to print this one */
	if (flags & SWAKEONSIG) flags &= ~(SWAKEONSIG);
	
	if (flags & TTERM) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_17, " term") );  
	if (flags & TSUSP) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_20, " susp") );  
	flags &= ~( TSUSP | TTERM);

	if (flags & TSIGAVAIL) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_21, " sig_avail") );  
	flags &= ~(TSIGAVAIL);

	if (flags & TASSERTSIG) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_22, " assert_sig") );  
	if (flags & TFUNNELLED) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_23, " funnel") );  
	if (flags & TKTHREAD) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_24, " kthread") );  
	if (flags & TSETSTATE) 
		strcat(buf, catgets(scmc_catd, MS_crsh, THREAD_MSG_25, " setstate") );  
	flags &= ~(TASSERTSIG | TFUNNELLED | TKTHREAD | TSETSTATE);
	printf("\tt_flags: %s\n", buf);
 

	if (flags) printf(" unknown: 0x%x\n",flags);
}
