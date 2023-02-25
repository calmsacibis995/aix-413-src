static char sccsid[] = "@(#)72	1.21.1.6  src/bos/usr/sbin/crash/proc.c, cmdcrash, bos411, 9439B411b 9/29/94 15:50:32";

/*
 * COMPONENT_NAME: (CMDCRASH) Crash
 *
 * FUNCTIONS: prproc, pr_flags, pr_p_int, pr_p_atomic
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
 * LEVEL 1,5 Years Bull Confidential Information
 */

#include	"crash.h"
#include	<sys/proc.h>
#include	<sys/errno.h>
extern struct user x;
extern int dumpflag;

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 *    Function:	Print a process table slot.  
 *
 *    Returns:	nothing.
 */
prproc(slot, md, run, all)
int	slot;
int	md;		/* 1 if a full listing wanted */
int	run;
int	all;
{
	struct proc proc,*p;		/* proc table pointer */
	char	*cp;
	int	i,tmp;
	struct user *user_area;
	int running;
	ulong proc_addr;

	if(slot == -1)
		return;
	if(slot > (int)v.ve_proc) {
		printf( catgets(scmc_catd, MS_crsh, PROC_MSG_1, 
		"Process slot %d above maximum limit.Specify a value less than %d.\n") , slot,v.ve_proc);
		return;
	}
	proc_addr = SYM_VALUE(Proc) + (ulong)(slot *sizeof(struct proc));
	if (read_proc(proc_addr, &proc) != sizeof(struct proc))
		return;

	p = &proc;

	if (run) {
		running = FALSE;
		if (p->p_stat == SACTIVE) {
			int last_thread = FALSE;
			int thread_slot;
			struct thread thread;
			
			thread_slot = ((int)p->p_threadlist - SYM_VALUE(Thread))/sizeof(struct thread);
			while ((read_thread(thread_slot, &thread) == sizeof (struct thread)) && 
				   (!last_thread) && (!running)) {
				if (thread.t_state == TSRUN)
					running = TRUE;
				else {
					if (thread.t_threadlist.nextthread == p->p_threadlist) 
						last_thread = TRUE;
					else {
						thread_slot = ((int)thread.t_threadlist.nextthread - 
						SYM_VALUE(Thread))/sizeof(struct thread);
					}
				}
			}
		}
		if (!running)
			return;
	}

	if(!all && p->p_stat == 0)
		return;
	if (md) {
		printf( catgets(scmc_catd, MS_crsh, MAIN_MSG_61, "SLT ST    PID   PPID   PGRP   UID  EUID  TCNT  NAME\n") );
	}

	switch(p->p_stat) {
		case 0:      cp = " "; break;
		case SACTIVE:cp =  catgets(scmc_catd, MS_crsh, PROC_MSG_5, "a") ; break;
		case SSWAP:  cp =  catgets(scmc_catd, MS_crsh, PROC_MSG_6, "w") ; break;
		case SIDL:   cp =  catgets(scmc_catd, MS_crsh, PROC_MSG_8, "i") ; break;
		case SZOMB:  cp =  catgets(scmc_catd, MS_crsh, PROC_MSG_9, "z") ; break;
		case SSTOP:  cp =  catgets(scmc_catd, MS_crsh, PROC_MSG_10, "t") ; break;
		default:     cp =  catgets(scmc_catd, MS_crsh, PROC_MSG_11, "?") ; break;
	}
	printf("%3d %s", slot, cp);

	/* CHANGE BELOW mixture of hex and dec - confusing ?? */

	printf(" %7x %6x %6x %5u %5u %5u ",
		p->p_pid, p->p_ppid, p->p_pgrp, p->p_uid, p->p_suid,
		p->p_threadcount);


	/* Get command name */
	if (p->p_stat == 0)
		cp = "        ";
	else if(p->p_stat == SZOMB)
		cp =  catgets(scmc_catd, MS_crsh, PROC_MSG_13, "ZOMBIE") ;
	else if(!p->p_pid)  			/* pid 0 is swapper */
		cp =  catgets(scmc_catd, MS_crsh, PROC_MSG_14, "swapper") ;
	else {
		if(getuarea(slot,&tmp) == -1) {
			cp =  catgets(scmc_catd, MS_crsh, PROC_MSG_15, " Cannot read uarea ") ;
		}
		else {
			cp = x.U_comm;
		}
	}
	if(!p->p_pid) {			/* just to make it all fit on 1 line */
		printf(" %s ", cp);
		}
	else
		printf(" %s  ", cp);
	printf("\n");

	pr_flags(p->p_flag);
	pr_p_int(p->p_int);
	pr_p_atomic(p->p_atomic);

	/* end of short (whole proc table) listing */
	if(!md)
		return;

	/* else do a long listing */
	/* Main proc link pointers */
	printf("\nLinks:  *child:0x%08x  *siblings:0x%08x  *uidl:0x%08x\n",
		p->p_child, p->p_siblings, p->p_uidl);
	printf("    *ganchor:0x%08x\n", p->p_ganchor);

	/* Dispatch fields */
      	printf( catgets(scmc_catd, MS_crsh, PROC_MSG_20, "Dispatch Fields:  pevent:0x%08x \n") ,
		p->p_pevent);
      	printf("    *synch:0x%08x \n", p->p_synch);
      	printf( catgets(scmc_catd, MS_crsh, PROC_MSG_21, "Thread Fields:  *threadlist:0x%08x  threadcount:%5d \n") ,
			p->p_threadlist, p->p_threadcount);
	    printf("   active:%d  suspended:%d  local:%d   terminating:%d\n",
		   p->p_active, p->p_suspended, p->p_local, p->p_terminating);

	/* Scheduler info */
	printf( catgets(scmc_catd, MS_crsh, PROC_MSG_23, "Scheduler Fields:  " )); 
	(p->p_flag & SFIXPRI) ? printf( catgets(scmc_catd, MS_crsh, PROC_MSG_24, " fixed pri:%3u" ) ,p->p_nice) :
				printf(" nice:%3u" ,EXTRACT_NICE(p));
	printf("  repage:0x%08x  scount:0x%08x  sched_pri:%d\n", 
               p->p_repage,p->p_sched_count, p->p_sched_pri);
	printf("   *sched_next:0x%08x  *sched_back:0x%08x cpticks:%d\n", 
                  p->p_sched_next,p->p_sched_back, p->p_cpticks);
        printf("   msgcnt:%d    majfltsec:%d\n", 
                   p->p_msgcnt, p->p_majfltsec);

	/* Misc */
	printf( catgets(scmc_catd, MS_crsh, PROC_MSG_16, "Misc:  adspace:0x%08x  *ttyl:0x%08x\n") ,p->p_adspace,p->p_ttyl);
	printf("       *p_ipc:0x%08x  *p_dblist:0x%08x  *p_dbnext:0x%08x\n",
		p->p_ipc, p->p_dblist, p->p_dbnext);
	printf( "    *lock:0x%08x  kstackseg:0x%08x *pgrpl:0x%08x\n",
		p->p_lock, p->p_kstackseg, p->p_pgrpl);

	/* Zombie or Sig Info */
	if(p->p_stat == SZOMB) {
	  printf(
	   "Zombie:  exit status:0x%08x  user time:0x%08x  sys time:0x%08x\n",
                p->xp_stat, p->xp_utime, p->xp_stime);
		/*	return; 	*/
	}
	else {	
	/* Signal Fields */
	printf( catgets(scmc_catd, MS_crsh, PROC_MSG_25, "Signal Information: \n"));
	printf("    pending:hi 0x%08x,lo 0x%08x \n", p->p_sig.hisigs, p->p_sig.losigs);

     	printf( "    sigcatch:hi 0x%08x,lo 0x%08x  sigignore:hi 0x%08x,lo 0x%08x\n",
			p->p_sigcatch.hisigs, p->p_sigcatch.losigs,
			p->p_sigignore.hisigs, p->p_sigignore.losigs);
	}

	/* Statistics */
       	printf( catgets(scmc_catd, MS_crsh, PROC_MSG_17, "Statistics:  size:0x%08x(pages)  audit:0x%08x\n\n\n") ,
		p->p_size, p->p_auditmask);
        printf("    pctcpu:%d    minflt:%d    majflt:%d\n",
                p->p_pctcpu, p->p_minflt, p->p_majflt);


}  /* end disp_proc */
pr_p_int(flags)
ulong flags;
{
	char buf[128];

	*buf = '\0';
	if(flags & SJUSTBACKIN) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_40, " restarted") );  
	if(flags & SPSMKILL) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_41, " psm_kill") );  
	if(flags & STERM) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_33, " terminating") );  
	if(flags & SSUSP) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_38, " suspended") );  
	if(flags & SSUSPUM) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_42, " suspended_umode") );  
	if(flags & SGETOUT) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_39, " get_out") );  
	if(flags & SSIGSLIH) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_43, " sig_slih") );  
	if (buf[0] != '\0')
		printf("\t      %s\n", buf);
}

pr_p_atomic(flags)
ulong flags;
{
	char buf[128];
	buf[0] = '\0';
	if(flags & SWTED)
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_44, " stopped_wh_traced") );  
	if(flags & SFWTED)
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_45, " stopped_af_fork_wh_traced") );  
	if(flags & SEWTED)
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_46, " stopped_af_exec_wh_traced") );  
	if(flags & SLWTED)
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_47, " stopped_af_load_wh_traced") ); 
	if (buf[0] != '\0')
		printf("\t      %s\n", buf);
}

pr_flags(flags)
ulong flags;
{
	char buf[128];

	*buf = '\0';
	if(flags & SLOAD) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_27, " swapped_in") );  
	if(flags & SNOSWAP) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_28, " no_swap") );  
	if(flags & SFORKSTACK) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_29, " special_fork_stack") );  
	if(flags & (STRC|SMPTRACE)) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_30, " trace") );
	if(flags & SFIXPRI) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_31, " fixed_pri") );  
	if(flags & SKPROC) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_32, " kproc") );  
	if(flags & SEXECING) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_34, " execing") );  
	if(flags & SLKDONE) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_35, " locks") );  
	if(flags & (STRACING)) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_36, " debug") );  
	if(flags & SEXIT) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_37, " exiting") );  
	if(flags & SORPHANPGRP) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PROC_MSG_26, " orphanpgrp") );  
	if(flags & SSIGNOCHLD) strcat(buf, " signochld");
	if(flags & SSIGSET) strcat(buf, " sigset");
	if(flags & SNOCNTLPROC) strcat(buf, " nocntlproc");
	if(flags & SPPNOCLDSTOP) strcat(buf, " ppnocldstop");
        if(flags & SJOBSESS) strcat(buf, " jobsess");
        if(flags & SJOBOFF) strcat(buf, " joboff");
	if(flags & SPSEARLYALLOC) strcat(buf, " psearlyalloc");
        printf(catgets(scmc_catd,MS_crsh,PROC_MSG_18, "\tFLAGS:%s\n"), buf);
}
