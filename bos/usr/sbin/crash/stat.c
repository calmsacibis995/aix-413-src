static char sccsid[] = "@(#)05  1.19  src/bos/usr/sbin/crash/stat.c, cmdcrash, bos411, 9435A411a 8/25/94 12:58:26";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prstat,prvar
 *
 * ORIGINS: 27 ,3, 83
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
 *
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
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include	"crash.h"
#include	"sys/inode.h"
#include	"sys/proc.h"
#include	"sys/file.h"
#include	"sys/utsname.h"
#include	<sys/param.h>

extern int dumpflag;		/* true if on a dump */
struct nlist *abend_csa, *abend_code;

prstat()
{
	long	toc, lbolt;
	unsigned  panicstr;
	struct	utsname	utsname;
	char	panic[30];
	ulong ledval, csaval;
        struct tm *ptm;
        char *pbuf, buf[40];

        pbuf = buf;
#define FIXLED (ledval >> 20)

	if(readmem(&utsname, (long)SYM_VALUE(Sys), sizeof utsname, CURPROC) 
		!= sizeof utsname)
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_1, "0452-338: Cannot read uname structure from address 0x%8x\n") ,
			SYM_VALUE(Sys));
	else {
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_2, "\tsysname: %s\n") , utsname.sysname);
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_3, "\tnodename: %s\n") , utsname.nodename);
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_4, "\trelease: %s\n") , utsname.release);
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_5, "\tversion: %s\n") , utsname.version);
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_6, "\tmachine: %s\n") , utsname.machine);
	}
	if(readmem(&toc, (long)SYM_VALUE(Time), sizeof toc, CURPROC) 
	   != sizeof toc)
		toc = 0;
        ptm = localtime(&toc);
        strftime(pbuf, 39, "%c\n", ptm); 
	printf( catgets(scmc_catd, MS_crsh, STAT_MSG_7, "\ttime of crash: %s") , pbuf);
	if(readmem(&lbolt, (long)SYM_VALUE(Lbolt),sizeof lbolt, CURPROC) 
	   != sizeof lbolt)
		lbolt = 0;
	printf( catgets(scmc_catd, MS_crsh, STAT_MSG_8, "\tage of system: ") );
	lbolt = (lbolt + (long) (60 * HZ -1)) / (long) (60 * HZ);
	if(lbolt / (long)(60 * 24))
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_9, "%lu day, ") , lbolt / (long)(60 * 24));
	lbolt %= (long)(60 * 24);
	if(lbolt / (long)60)
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_10, "%lu hr., ") , lbolt / (long)60);
	lbolt %= (long) 60;
	if(lbolt)
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_11, "%lu min.") , lbolt);
	printf("\n");

	/* Show the abend (LED) code and csa for a dump. */
	if (dumpflag) {
		if(readmem(&ledval, (long)SYM_VALUE(abend_code),sizeof ledval, CURPROC) 
	   	  == sizeof ledval)
			/* FIXLED shifts ledval into position */
			printf( catgets(scmc_catd, MS_crsh, STAT_MSG_13, "\tdump code: %x\n") , FIXLED);
		if(readmem(&csaval, (long)SYM_VALUE(abend_csa),sizeof csaval, CURPROC) 
	   	  == sizeof csaval)
			printf( catgets(scmc_catd, MS_crsh, STAT_MSG_14, "\tcsa: 0x%x\n") , csaval);
	}

	if(readmem(&panicstr, (long)SYM_VALUE(Panic), sizeof panicstr, CURPROC) 
	   == sizeof panicstr && panicstr != 0) {
			if(readmem(panic, (long)panicstr,
				sizeof(panic), CURPROC) == sizeof(panic))
					printf("\tpanic: %.30s\n\n", panic);
	}

	return(0);
}

prvar()
{
	extern	struct	var	v;

	if(readmem(&v, (long)SYM_VALUE(V), sizeof v, CURPROC) != sizeof v) {
		printf( catgets(scmc_catd, MS_crsh, STAT_MSG_12, "0452-349: Cannot read v structure from address 0x%8x\n") ,
			SYM_VALUE(V));
		return;
	}
	v.ve_file = (char *)(((unsigned)v.ve_file - File->n_value) /
		sizeof (struct file));
    v.ve_proc = (char *)(((unsigned)v.ve_proc - Proc->n_value) /
		sizeof (struct proc));
    v.ve_thread = (char *)(((unsigned)v.ve_thread - Thread->n_value) /
		sizeof (struct thread));
	printf("buffers	  %3d\n",v.v_bufhw);
	printf("files	  %3d\n",v.v_file);
	printf("e_files	  %3d\n",v.ve_file);
	printf("procs	  %3d\n",v.v_proc);
    printf("e_procs	  %3d\n",v.ve_proc);
    printf("threads	  %3d\n",v.v_thread);
	printf("e_threads %3d\n",v.ve_thread);
	printf("clists	  %3d\n",v.v_clist);
	printf("maxproc	  %3d\n",v.v_maxup);
	printf("iostats	  %3d\n",v.v_iostrun);
	printf("locks     %3d\n",v.v_lock);
	printf("e_locks   %3d\n",v.ve_lock);
	return(0);
}

