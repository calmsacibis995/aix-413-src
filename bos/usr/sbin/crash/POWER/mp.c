static char sccsid[] = "@(#)42	1.3  src/bos/usr/sbin/crash/POWER/mp.c, cmdcrash, bos411, 9428A410j 1/17/94 11:15:35";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prppd, prstatus
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */



#include <stdio.h>
#include "crash.h"
#include "crash_msg.h"
#include <sys/ppda.h>
#
#define PPDSIZE	sizeof(struct ppda)

extern nl_catd scmc_catd;
extern cpu_t selected_cpu;
extern cpu_t ncpus;

/*
 * NAME: prppd
 *
 * FUNCTION: this routine displays the ppda structure
 *
 * RETURN VALUE : none
 */

prppd(cpu_t cpu)

{
	int ret, i;
	struct ppda ppda;
	struct ppda *ppdata;

	printf(catgets(scmc_catd, MS_crsh, MP_MSG_1, 
	"Per Processor Data Area for processor %2d\n\n"), cpu);

	if (cpu >= ncpus){
		printf(catgets(scmc_catd, MS_crsh, MP_MSG_2, 
		"0452-900: Processor number not valid. This machine has %2d processors\n"), ncpus);
		return;
	}
	
	if ((ret = readmem(&ppda, SYM_VALUE(Ppd)+ cpu * PPDSIZE, PPDSIZE, CURPROC)) != PPDSIZE) {
		printf(catgets(scmc_catd, MS_crsh, MP_MSG_3, 
		"0452-901: Cannot read ppda structure at address 0x%08x\n"), (SYM_VALUE(Ppd)+cpu*PPDSIZE));
		return;
	}
	
	ppdata = &ppda;
	if (ppdata->_csa == NULL) {
		printf(catgets(scmc_catd, MS_crsh, MP_MSG_4, "This processor is disabled\n"));
		return;
	}

	printf("csa......................%08x\n", ppdata->_csa);
	printf("mstack...................%08x\n", ppdata->mstack);
	printf("fpowner..................%08x\n", ppdata->fpowner);
	printf("curthread................%08x\n", ppdata->_curthread);
	printf("r0.......................%08x\n", ppdata->save[0]);
	printf("r1.......................%08x\n", ppdata->save[1]);
	printf("r2.......................%08x\n", ppdata->save[2]);
	printf("r15......................%08x\n", ppdata->save[3]);
	printf("sr0......................%08x\n", ppdata->save[4]);
	printf("sr2......................%08x\n", ppdata->save[5]);
	printf("iar......................%08x\n", ppdata->save[6]);
	printf("msr......................%08x\n", ppdata->save[7]);
	printf("intr.....................%08x\n", ppdata->intr);
	printf("i_softis.................%08x\n", ppdata->i_softis);
	printf("dar......................%08x\n", ppdata->dar);
	printf("dsisr....................%08x\n", ppdata->dsisr);
	printf("dsi_flag.................%08x\n", ppdata->dsi_flag);
	if (ncpus > 1){
		printf("cpuid....................%08x\n", ppdata->cpuid);
		printf("mfrr_pend................%08x\n", ppdata->mfrr_pend);
		printf("mpc_pend.................%08x\n", ppdata->mpc_pend);
	}

	printf("iodonelist...............%08x\n", ppdata->iodonelist);

	for (i=0; i<8; i++)
		printf("dssave[%1d]................%08x\n", i, ppdata->dssave[i]);

	for ( i=0 ; i <= (INTOFFL3-INTOFFL0) ; i++ )
		printf("schedtail[%d].............%08x\n", i, ppdata->schedtail[i]);

	if (ppdata->alsave[0]) {
		printf("match....................%08x\n", (ppdata->alsave[0] >> 12) & 0xff);
		printf("rt.......................%08x\n", (ppdata->alsave[0] >> 8) & 0xff);
		printf("ra.......................%08x\n", (ppdata->alsave[0] >> 4) & 0xff);
		printf("rb.......................%08x\n", ppdata->alsave[0] & 0xff);
		printf("work1....................%08x\n", ppdata->alsave[1]);
		printf("work2....................%08x\n", ppdata->alsave[2]);
		printf("work3....................%08x\n", ppdata->alsave[3]);
		printf("r25......................%08x\n", ppdata->alsave[4]);
		printf("r26......................%08x\n", ppdata->alsave[5]);
		printf("r27......................%08x\n", ppdata->alsave[6]);
		printf("r28......................%08x\n", ppdata->alsave[7]);
		printf("r29......................%08x\n", ppdata->alsave[8]);
		printf("r30......................%08x\n", ppdata->alsave[9]);
		printf("r31......................%08x\n", ppdata->alsave[10]);
		printf("srr0.....................%08x\n", ppdata->alsave[11]);
		printf("srr1.....................%08x\n", ppdata->alsave[12]);
		printf("lr.......................%08x\n", ppdata->alsave[13]);
		printf("cr.......................%08x\n", ppdata->alsave[14]);
		printf("xer......................%08x\n", ppdata->alsave[15]);
	}
	
	printf("TIMER....................");
	printf("\nt_free...................%08x", ppdata->ppda_timer.t_free);
	printf("\nt_active.................%08x", ppdata->ppda_timer.t_active);
	printf("\nt_freecnt................%08x", ppdata->ppda_timer.t_freecnt);
	printf("\ntrb_called...............%08x", ppdata->ppda_timer.trb_called);
	printf("\nsystimer.................%08x", ppdata->ppda_timer.systimer);
	printf("\nticks_its................%08x", ppdata->ppda_timer.ticks_its);
	printf("\nref_time.tv_sec..........%08x", ppdata->ppda_timer.ref_time.tv_sec);
	printf("\nref_time.tv_nsec.........%08x", ppdata->ppda_timer.ref_time.tv_nsec);
	printf("\ntime_delta...............%08x", ppdata->ppda_timer.time_delta);
	printf("\ntime_adjusted............%08x", ppdata->ppda_timer.time_adjusted);
	printf("\nwtimer.next..............%08x", ppdata->ppda_timer.wtimer.next);
	printf("\nwtimer.prev..............%08x", ppdata->ppda_timer.wtimer.prev);
	printf("\nwtimer.func..............%08x", ppdata->ppda_timer.wtimer.func);
	printf("\nwtimer.count.............%08x", ppdata->ppda_timer.wtimer.count);
	printf("\nwtimer.restart...........%08x", ppdata->ppda_timer.wtimer.restart);
	printf("\nw_called.................%08x", ppdata->ppda_timer.w_called);
	printf("\n\n\n");
}


/*
 * NAME: prstatus
 *
 * FUNCTION: this routine displays a description of what is running on a processor
 *
 * RETURN VALUE : none
 */

prstatus(cpu_t cpu)

{
	extern struct user x;

	int ret;
	struct ppda ppdata;
	pid_t pid;
	int procslot;
	struct proc proc;
	tid_t	tid;
	int threadslot;
	struct thread thread;
	int u_segval;
	char *name;
	
	if (cpu >= ncpus){
		printf(catgets(scmc_catd, MS_crsh, MP_MSG_6, 
		"0452-905: Processor number is not valid. This machine has %2d processors\n"), ncpus);
		return;
	}
	
	if ((ret = readmem(&ppdata, SYM_VALUE(Ppd)+ cpu * PPDSIZE, PPDSIZE, CURPROC)) != PPDSIZE) {
		printf(catgets(scmc_catd, MS_crsh, MP_MSG_7, 
		"0452-902: Cannot read ppda structure at address 0x%08x\n"), 
		(SYM_VALUE(Ppd)+cpu*PPDSIZE));
		return;
	}

	if (ppdata._csa == NULL) {
		printf(catgets(scmc_catd, MS_crsh, MP_MSG_8, "Processor %2d is disabled\n"), cpu);
		return;
	}

	threadslot = ((int)ppdata._curthread - SYM_VALUE(Thread))/sizeof (struct thread);
	if (read_thread(threadslot, &thread) != sizeof(struct thread)) {
		printf(catgets(scmc_catd, MS_crsh, MP_MSG_9, "0452-903: Cannot read thread structure.\n"));
		return;
	}
	tid = thread.t_tid;
	procslot = ((int)thread.t_procp - SYM_VALUE(Proc))/sizeof (struct proc);
	if (read_proc(thread.t_procp, &proc) != sizeof(struct proc)) {
		printf(catgets(scmc_catd, MS_crsh, MP_MSG_10, "0452-904: Cannot read process structure.\n"));
		return;
	}
	pid = proc.p_pid;
	/* get process name */
	if (proc.p_stat == SNONE)
		name = "      ";
	else if(proc.p_stat == SZOMB)
		name = catgets(scmc_catd, MS_crsh, MP_MSG_11, "ZOMBIE");
	else if (!proc.p_pid)
		name = catgets(scmc_catd, MS_crsh, MP_MSG_12, "swapper");
	else {
		if ((ret = getuarea(procslot, &u_segval)) == -1) 
			name = catgets(scmc_catd, MS_crsh, MP_MSG_13, "0452-908: Cannot read user structure.\n");
		else
			name = x.U_comm;
	}
		
	printf("%3d %7x %6d", cpu, tid, threadslot);
	printf(" %7x %6d  ",pid, procslot);
	printf("%s\n", name);
}

