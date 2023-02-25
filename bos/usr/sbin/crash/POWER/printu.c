static char sccsid[] = "@(#)61	1.21.1.10  src/bos/usr/sbin/crash/POWER/printu.c, cmdcrash, bos41J, rgj07 2/10/95 10:34:22";
/*
*/
/*
 * COMPONENT_NAME: (CMDCRASH) Crash
 *
 * FUNCTIONS:  pruthread, pruser, gnodeType, socketType
 *
 * ORIGINS: 3, 27, 83
 
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */
/*
 */
#include        "crash.h"
#include 	<sys/proc.h>
#include 	<sys/user.h>
#include 	<sys/buf.h>
#include 	<sys/pseg.h>		/* needed for pinned/paged constants */
#include	<stdlib.h>		/* needed for call to div	     */
#include	<sys/thread.h>
#include	<sys/uthread.h>
#include	<sys/param.h>
#include 	<sys/vnode.h>
#include 	<sys/socket.h>
#include 	<sys/socketvar.h>

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#undef u_sfirst
#undef u_slast
#undef u_srw
#undef u_segbase
#undef u_segoffs
#undef u_ofirst
#undef u_olast
extern int errno;
extern int dumpflag;


char *
gnodeType( struct gnode *gnodePtr )
{
	static char   staticBuf[ 20 ] ;

	switch ( gnodePtr->gn_type )
	{
		case VNON:
			return( "VNON" );
		case VREG:
			return( "VREG" );
		case VDIR:
			return( "VDIR" );
		case VBLK:
			return( "VBLK" );
		case VCHR:
			return( "VCHR" );
		case VLNK:
			return( "VLNK" );
		case VSOCK:
			return( "VSOCK" );
		case VBAD:
			return( "VBAD" );
		case VFIFO:
			return( "VFIFO" );
		case VMPC:
			return( "VMPC" );
		default :
			sprintf( staticBuf, "%#x", gnodePtr->gn_type );
			return( staticBuf );
	}
}

char *
socketType( struct socket *socketPtr )
{
	static char   staticBuf[ 20 ] ;

	switch ( socketPtr->so_type )
	{
		case SOCK_STREAM :
			return( "STREAM" );
		case SOCK_DGRAM :
			return( "DGRAM" );
		case SOCK_RAW :
			return( "RAW" );
		case SOCK_RDM :
			return( "RDM" );
		case SOCK_SEQPACKET :
			return( "SEQPACKET" );
		default :
			sprintf( staticBuf, "%#x", socketPtr->so_type );
			return( staticBuf );
	}

}


/*
 *  Function:	print uthread area 
 *
 *  Returns:	nothing.
*/
pruthread (x, threadslot, mst_only)
struct uthread *x;
int threadslot;
int mst_only;		/* 1 = print only mst area, not all of uarea */

{
	char *p;
	register int i;

	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_0, "\tUTHREAD AREA FOR SLOT %3d\n\n"), threadslot);
/*
 *		Mstsave structure
 */
	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_1,"SAVED MACHINE STATE \n"));
	printf("    curid:0x%08x  m/q:0x%08x  iar:0x%08x  cr:0x%08x \n",
		x->ut_save.curid,x->ut_save.mq,x->ut_save.iar, x->ut_save.cr);
#ifdef _POWER
	printf("    msr:0x%08x  lr:0x%08x  ctr:0x%08x  xer:0x%08x\n",
		x->ut_save.msr,x->ut_save.lr,x->ut_save.ctr,x->ut_save.xer);
#endif /* _POWER */
   printf("    *prevmst:0x%08x  *stackfix:0x%08x  intpri:0x%08x \n",
		 x->ut_save.prev,x->ut_save.stackfix, x->ut_save.intpri);
   printf("    backtrace:0x%02x  tid:0x%08x  fpeu:0x%02x  ecr:0x%08x\n",
		x->ut_save.backt, x->ut_save.tid, x->ut_save.fpeu,
#ifdef _POWER
		x->ut_save.excp_type);
/*
	printf("    o_iar:0x%08x  o_toc:0x%08x   o_arg1:0x%08x \n",
		x->ut_save.o_iar, x->ut_save.o_toc, x->ut_save.o_arg1);
*/
#endif /* _POWER */

	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_2,"    Exception Struct\n"));
	printf("      0x%08x  0x%08x  0x%08x  0x%08x  0x%08x\n",
		x->ut_save.except[0], x->ut_save.except[1],
		x->ut_save.except[2], x->ut_save.except[3],
		x->ut_save.except[4]);

	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_3,"    Segment Regs\n    "));
	for(i=0;i<NSRS;i++) {
		if(i<10)
			printf(" %d:0x%08x  ",i,x->ut_save.as.srval[i]);
		else
			printf("%d:0x%08x  ",i,x->ut_save.as.srval[i]);
		if(!((i+1)%4))
			printf("\n    ");
	}
	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_4,"General Purpose Regs\n    "));
	for(i=0;i<NGPRS;i++) {
		if(i<10)
			printf(" %d:0x%08x  ",i,x->ut_save.gpr[i]);
		else
			printf("%d:0x%08x  ",i,x->ut_save.gpr[i]);
		if(!((i+1)%4))
			printf("\n    ");
	}
	if (i%4)
		printf("\n");

	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_5,"Floating Point Regs\n    "));
#ifdef _POWER
	printf("    Fpscr: 0x%08x \n    ",x->ut_save.fpscr);
#endif /* _POWER */
	for(i=0;i<NFPRS;i++) {
		if(i<10)
			printf(" %d:0x%08x 0x%08x ",i,x->ut_save.fpr[i]);
		else
			printf("%d:0x%08x 0x%08x ",i,x->ut_save.fpr[i]);
		if(!((i+1)%3))
			printf("\n    ");
	}
	if(i%4)		/* we didn't print \n and left margin in loop */
		printf("\n    ");

	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_6,"\n\nKernel stack address: 0x%08x\n"), x->ut_kstack);

	if (!mst_only) {
/*
 *		System Call Info
 */
		printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_7,"\nSYSTEM CALL STATE \n"));
		printf("    error code:0x%02x  *kjmpbuf:0x%08x\n",
			x->ut_error,x->ut_save.kjmpbuf);
		pr_ut_flags(x->ut_flags);
	
/*
 *		Timers info
 */
		printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_8,"\nPER-THREAD TIMER MANAGEMENT \n"));
		printf("    Real/Alarm Timer (ut_timer[TIMERID_ALRM]) = 0x%x\n",
			x->ut_timer[TIMERID_ALRM]);
		printf("    Virtual Timer (ut_timer[TIMERID_VIRTUAL]) = 0x%x\n",
			x->ut_timer[TIMERID_VIRTUAL]);
		printf("    Prof Timer (ut_timer[TIMERID_PROF]) = 0x%x\n",
			x->ut_timer[TIMERID_PROF]);
		for(i = 0; i < NTIMEOFDAY; i++)
		{
			printf("    Posix Timer (ut_timer[POSIX%d]) = 0x%x\n",
				i, x->ut_timer[TIMERID_TOD + i]);
		}
	
/*
 *		Signal management
 */
		printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_9,"\nSIGNAL MANAGEMENT \n"));
		printf("    *sigsp:0x%x  oldmask:hi 0x%x,lo 0x%x  code:0x%x \n",
		     x->ut_sigsp, x->ut_oldmask.hisigs,x->ut_oldmask.losigs,x->ut_code);
/*
 *		Miscellaneous
 */
	
		printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_12,"\nMISCELLANOUS FIELDS:\n"));
		printf("   fstid:0x%08x   ioctlrv:0x%08x   selchn:0x%08x\n\n\n",
			x->ut_fstid, x->ut_ioctlrv,  x->ut_selchn);
	
	}
}

/*
 *  Function:	print uarea 
 *
 *  Returns:	nothing.
*/
pruser(x,procslot)
struct user *x;
int procslot;

{
	register int i;
	char *p;
	struct ucred cred, *c;
	
	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_13, 
	"\tUSER AREA OF ASSOCIATED PROCESS %s (SLOT %3d, PROCTAB 0x%08x)\n\n"), 
	x->U_comm, procslot, x->U_procp); 

	printf ("    handy_lock:0x%08x  timer_lock:0x%08x\n",
		x->U_handy_lock, x->U_timer_lock);
	printf ("    map:0x%08x  *semundo:0x%08x\n",
		x->U_map, x->U_semundo);
	printf ("    compatibility:0x%08x  lock:0x%08x\n",
		x->U_compatibility, x->U_lock);
	printf ("    ulocks:0x%08x",
		x->U_ulocks);

/*
 *		Signal management
 */
	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_14,"\nSIGNAL MANAGEMENT \n"));
	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_15,
	"    Signals to be blocked (sig#:hi/lo mask,flags,&func)\n    "));
	for(i=1;i<NSIG;i++) {
		if(i<=9)
			printf(" %d:hi 0x%08x,lo 0x%08x,0x%08x,0x%08x  ",i,
				x->U_sigmask[i].hisigs, x->U_sigmask[i].losigs,
				x->U_sigflags[i],x->U_signal[i]);
		else
			printf("%d:hi 0x%08x,lo 0x%08x,0x%08x,0x%08x  ",i,
				x->U_sigmask[i].hisigs, x->U_sigmask[i].losigs,
				x->U_sigflags[i],x->U_signal[i]);
		printf("\n    ");
	}
	printf("\n");

/*
 *		User Info
 */
	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_10, 
					"\nUSER INFORMATION \n"));
	if(!x->U_cred)
		printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_11,
						"    CRED Structure Not Found\n"));
	else {
	     if (readmem(&cred, x->U_cred, sizeof(struct ucred),
             CURPROC) != sizeof(struct ucred)) {
		       printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_27,
                       "0452-204: Cannot read cred structure at address 0x%8x.\n") ,x->U_cred);
             }
	     c=&cred;
	     printf("    euid:0x%04x  egid:0x%04x  ruid:0x%04x  rgid:0x%04x  luid:0x%08x\n",
		      c->cr_uid, c->cr_gid, c->cr_ruid,
		      c->cr_rgid, c->cr_luid);
	     printf("    suid:0x%08x  ngrps:0x%04x  *groups:0x%08x  compat:0x%08x\n",
		      c->cr_suid, c->cr_ngrps, c->cr_groups,
		      x->U_compatibility);
	     printf("    ref:0x%08x\n", c->cr_ref);
	     printf("    acctid:0x%08x   sgid:0x%08x   epriv:0x%08x\n",
		      c->cr_acctid, c->cr_sgid, c->cr_epriv);
	     printf("    ipriv:0x%08x   bpriv:0x%08x   mpriv:0x%08x\n",
		      c->cr_ipriv, c->cr_bpriv, c->cr_mpriv);
	} /* end if(cred) not found */

	printf("\n");
	p = x->U_uinfo;
	while(p < &x->U_uinfo[UINFOSIZ]) {
		if(*p=='\0') 
			*p == ' ';
		p++;
	}
	*(p-1)='\0';
	printf("    u_info: %s\n",x->U_uinfo);

/*
 *		Accounting and Profiling Data
 */
    	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_17,"\nACCOUNTING DATA \n"));
    	printf("    start:0x%08x  ticks:0x%08x  acflag:0x%04x  ",
		x->U_start,x->U_ticks,x->U_acflag);
	printf("pr_base:0x%08x\n", x->U_prof.pr_base);
  	printf("    pr_size:0x%08x  pr_off:0x%08x  pr_scale:0x%08x\n",
		x->U_prof.pr_size, x->U_prof.pr_off, x->U_prof.pr_scale);
    	printf("    process times:\n");
	printf("           user:0x%08xs 0x%08xus\n",
		x->U_ru.ru_utime.tv_sec, x->U_ru.ru_utime.tv_usec);
	printf("            sys:0x%08xs 0x%08xus\n",
		x->U_ru.ru_stime.tv_sec, x->U_ru.ru_stime.tv_usec);
    	printf("    children's times:\n");
	printf("           user:0x%08xs 0x%08xus\n",
		x->U_cru.ru_utime.tv_sec, x->U_cru.ru_utime.tv_usec);
	printf("            sys:0x%08xs 0x%08xus\n",
		x->U_cru.ru_stime.tv_sec, x->U_cru.ru_stime.tv_usec);

/*
 *		Controlling TTY info
 */
    	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_18,"\nCONTROLLING TTY \n"));
    	printf("    *ttysid:0x%08x  *ttyp(pgrp):0x%08x\n",
	       x->U_ttysid, x->U_ttyp);
    	printf("    ttyd(evice):0x%08x  ttympx:0x%08x  *ttys(tate):0x%08x\n",
	       x->U_ttyd, x->U_ttympx, x->U_ttys);
	printf("    tty id: 0x%08x  *query function: 0x%08x\n",
		x -> U_ttyid, x -> U_ttyf);

	/* Pinned profiling buffer */
	printf ( catgets(scmc_catd, MS_crsh, PRINTU_MSG_19,"\nPINNED PROFILING BUFFER\n"));
	printf("    *pprof: 0x%08x  *mem desc: 0x%08x\n",x->U_pprof, x->U_dp);

/*
 *		Resource Limits data
 */
    	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_20,"\nRESOURCE LIMITS AND COUNTERS\n"));
	printf("    ior:0x%08x  iow:0x%08x  ioch:0x%08x\n",
		x->U_ior,x->U_iow, x->U_ioch);
	printf("    text:0x%08x  data:0x%08x  stk:0x%08x\n",
		x->U_tsize, x->U_dsize, x->U_ssize);
	printf("    max data:0x%08x  max stk:0x%08x  max file:0x%08x\n",
		x->U_dmax, x->U_smax,x->U_limit);
	printf("    soft core dump:0x%08x  hard core dump:0x%08x\n",
		x->U_rlimit[RLIMIT_CORE].rlim_cur,
		x->U_rlimit[RLIMIT_CORE].rlim_max);
	printf("    soft rss:0x%08x  hard rss:0x%08x\n",
		x->U_rlimit[RLIMIT_RSS].rlim_cur,
		x->U_rlimit[RLIMIT_RSS].rlim_max);
	printf("    cpu soft:0x%08x  cpu hard:0x%08x\n",
		x->U_rlimit[RLIMIT_CPU].rlim_cur,
		x->U_rlimit[RLIMIT_CPU].rlim_max);
	printf("    hard ulimit:0x%08x\n", x->U_rlimit[RLIMIT_FSIZE].rlim_max);
	printf("    minflt:0x%08x   majflt:0x%08x\n",
		x->U_minflt, x->U_majflt);

/*
 *		auditing data
 */
	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_21,"\nAUDITING INFORMATION \n"));
	printf("    auditstatus:0x%08x\n", x->U_auditstatus);

	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_22,"\nSEGMENT REGISTER INFORMATION \n"));
     	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_23,"    Reg        Flag      Fileno     Pointer  \n"));
	for(i=0;i<NSEGS;i++) {
		if(i<10) 	/* for alignment */
			printf(" ");
		printf("    %d    %8x   %8x   %8x\n", i,x->U_segst[i].segflag,
				x->U_segst[i].segfileno,x->U_segst[i].u_ptrs);
	}
	printf("    *adspace:0x%08x\n", x->U_adspace);

/*
  *		File System Info
 */
	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_24,"FILE SYSTEM STATE \n"));
    	printf("    *curdir:0x%08x  *rootdir:0x%08x\n",
		x->U_cdir,x->U_rdir);
    	printf("    cmask:0x%04x  maxindex:0x%04x\n",x->U_cmask,x->U_maxofile);

	printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_25,"FILE DESCRIPTOR TABLE\n"));
	printf("    *ufd: 0x%08x\n", x->U_ufd);	
	for (i = 0; i < OPEN_MAX; i++) {
		if(dumpflag && (x->U_ofile(i) == (struct file *)0xBAD)) {
			printf( catgets(scmc_catd, MS_crsh, PRINTU_MSG_26, "Rest of user area paged out. \n") );
			return;
		}
		if (x->U_ofile(i))
			printf("    fd %d:  fp = 0x%08x   flags = 0x%08x\n",i, 
				x->U_ofile(i),x->U_pofile(i));
	}
}


pr_ut_flags(flags)
short flags;
{
	char buf[128];

	*buf = '\0';
	if(flags & UTSTILLUSED) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PRINTU_MSG_28, " still_used") );  
	if(flags & UTSCSIG) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PRINTU_MSG_29, " sc_to_sigslih") );  
	if(flags & UTNOULIMIT) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PRINTU_MSG_30, " no_ulimit") );  
	if(flags & UTSIGONSTACK) 
		strcat(buf, catgets(scmc_catd, MS_crsh, PRINTU_MSG_31, " sig_on_stack") ); 
	if (buf[0] != '\0')
		printf("\tut_flags: %s\n", buf);
}

