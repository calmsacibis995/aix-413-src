static char sccsid[] = "@(#)47        1.15.1.6  src/bos/usr/sbin/crash/POWER/u.c, cmdcrash, bos411, 9428A410j 6/30/94 11:46:53";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: pruareap, dumpu, prstack, prtrace,
 *            pruareat, dumput
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
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#ifndef _KERNSYS
#define _KERNSYS
#include	"crash.h"
#undef _KERNSYS
#else /* _KERNSYS */
#include        "crash.h"
#endif  /* _KERNSYS */
#include	<sys/acct.h>
#include	<sys/file.h>
#include	<sys/lock.h>
#include	<sys/proc.h>
#include	<sys/pseg.h>
#include	<sys/debug.h>
#include	<sys/thread.h>

#define BPLINE	16			/* Bytes per line */
#define ALIGN(x)	(x & (~(BPLINE-1)))
#define	R1	1			/* offset to r1 (frame pointer) */
#define FP	x_thread.ut_save.gpr[R1]
#define INTPRI	x_thread.ut_save.intpri
#define CURID	x_thread.ut_save.curid
#define EXP	x_thread.ut_save.except
#define IAR	x_thread.ut_save.iar
#define LR	x_thread.ut_save.lr

#define RET_OFFSET	(sizeof(ulong))

#define KSTACK_TOP stacktop

#undef BUFSIZ
#define BUFSIZ	512

#define PPDSIZE sizeof(struct ppda)

extern	int errno;
extern	int dumpflag;
extern	struct	user	x;
extern struct uthread x_thread;
extern	unsigned Kfp;
extern cpu_t selected_cpu;

/* 
 * NAME: pruareap
 *
 * FUNCTION:
 * 	prints the user structure of the process (formatted)
 *	
 * RETURNS:
 *	nothing upon successful completion
 *	-1 otherwise
 *
 */

pruareap(procslot,mst_only)
int	procslot;
int	mst_only;	/* 1 = print only the mst area, not all of uarea */
{
	register  int   i;
	register  int	ret;
	int threadslot;
	ulong	u_segval;	/* segval of uarea. needed to retrieve iorb */

	if(procslot > (int)v.ve_proc) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_1, 
		"0452-750: Process slot %d above maximum limit.Specify a value less than %d.\n"),
		procslot,v.ve_proc);
		return(-1);
	}

	if (procslot == CURPROC) {
		threadslot = get_slot_curthread();
		procslot = associated_proc(threadslot);
	}

	if (procslot == BADSLOT) {
		printf ( catgets(scmc_catd, MS_crsh, U_MSG_2, 
		"0452-751: \nSlot of process associated to the thread was not found.\n"));
		return (-1);
	}

	if((ret=getuarea(procslot,&u_segval)) == -1) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_3, 
		"0452-752: Cannot read uarea for this process.\n") );
		return(-1);
	}
	else if(ret == SWAPPED) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_4, 
		"0452-753: Process is currently swapped out.\n") );
		return(-1);
	}
	pruser(&x,procslot);
}


/* 
 * NAME: pruareat
 *
 * FUNCTION:
 * 	prints the uthread structure of a thread (formatted)
 *	
 * RETURNS:
 *	nothing upon successful completion
 *	-1 otherwise
 *
 */

pruareat(threadslot, mst_only)
int	threadslot;
int	mst_only;	/* 1 = print only the mst area,not all of uarea */
{
	register  int  i;
	register  int	ret;
	ulong	u_segval;	/* segval of uarea. needed to retrieve iorb */

	if (threadslot == CURTHREAD) {
		threadslot = get_slot_curthread();
	}

	if(threadslot > (int)v.ve_thread) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_5, 
		"0452-754: Thread slot %d above maximum limit.Specify a value less than %d.\n"),
		threadslot, v.ve_thread);
		return(-1);
	}

	if((ret=getuarea_thread(threadslot,&u_segval)) == -1) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_6, 
		"0452-755: Cannot read uthread area for this  thread.\n"));
		return(-1);
	}
	else if(ret == SWAPPED) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_7, 
		"Thread is currently swapped out.\n") );
		return(-1);
	}
	pruthread(&x_thread, threadslot, mst_only);
}

/* 
 * NAME: dumpu
 *
 * FUNCTION:
 * 	dumps the user structure of the process (non formatted)
 *	
 * RETURNS:
 *	nothing upon successful completion
 *	-1 otherwise
 *
 */

dumpu(procslot)
int     procslot;
{
	register  int  i;
	register  int   ret;
	int threadslot;
	ulong	u_segval;	/* segval of uarea. needed to retrieve iorb */
	char *style = "hex";

	if (procslot == CURPROC) {
		threadslot = get_slot_curthread ();
		procslot = associated_proc(threadslot);
	}

	if(procslot > (int)v.ve_proc) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_8, 
		"0452-757: Process slot %d a bove maximum limit.Specify a value less than %d.\n"), 
		procslot,v.ve_proc);
		return(-1);
	}

	printf ("\n");
	if (procslot == BADSLOT) {
		printf ( catgets(scmc_catd, MS_crsh, U_MSG_9, 
		"0452-758: Slot of process associated to the thread was not found.\n"));
		return (-1);
	}
	printf ( catgets(scmc_catd, MS_crsh, U_MSG_10, 
	"Struct user of associated process (slot %3d):\n"), procslot);

	if((ret=getuarea(procslot, &u_segval)) == -1) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_11, "0452-759: Cannot read uarea for this process.\n") );
		return(-1);
	}
	else if(ret == SWAPPED) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_12, "Process is currently swapped out.\n") );
		return(-1);
	}
	/*prod(&x, sizeof(struct user), style);*/
	xdumpx(&x, 0, sizeof(struct user));
}

/* 
 * NAME: dumput
 *
 * FUNCTION:
 * 	prints the uthread structure of the thread (non formatted)
 *	
 * RETURNS:
 *	nothing upon successful completion
 *	-1 otherwise
 *
 */

dumput(int threadslot)
{
	register  int  i;
	register  int   ret;
	ulong	u_segval;	/* segval of uarea. needed to retrieve iorb */

	if (threadslot == CURTHREAD) {
	   threadslot = get_slot_curthread();
	}

	printf ("\n");
	printf ( catgets(scmc_catd, MS_crsh, U_MSG_13, 
	"Uthread structure of thread slot %3d :\n"), threadslot);

	if((threadslot > (int)v.ve_thread) || (threadslot < 0)) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_14, 
		"0452-760: Thread slot %d not in right range. Specify a value less than %d and positive.\n"), 
		threadslot,v.ve_thread);
		return(-1);
	}

	if((ret=getuarea_thread(threadslot, &u_segval)) == -1) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_15, 
		"0452-761: Cannot read uarea for this thread.\n") );
		return(-1);
	}
	else if(ret == SWAPPED) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_16, 
		"0452-762: Thread is currently swapped out.\n") );
		return(-1);
	}
	xdumpx(&x_thread, 0, sizeof(struct uthread));

}

#ifdef _POWER

/* 
 * NAME: prstack
 *
 * FUNCTION:
 * 	prints the stack of the thread (non formatted)
 *	
 * RETURNS:
 *	nothing upon successful completion
 *	-1 otherwise
 *
 */

prstack(threadslot)
int	threadslot;

{
	ulong	buf[BUFSIZ];
	ulong	*curaddr;
	register  int	i, count, ret;
	ulong	u_segval;	/* segval of uarea */
	ulong	segment;	/* segment value of either uarea or thread kernel stacks */
	struct thread thread;
	struct proc proc;
	ulong	stacktop;

	if (threadslot == CURTHREAD) {
	   threadslot = get_slot_curthread();
	}

	if(threadslot > (int)v.ve_thread) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_17, 
		"0452-763: Thread slot %d above maximum limit. Specify a value less than %d.\n"), 
		threadslot, v.ve_thread);
		return(-1);
	}
	if((ret=getuarea_thread(threadslot,&u_segval)) == -1) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_18, "0452-764: Cannot read uarea for this thread.\n"));
		return(-1);
	}
	else if(ret == SWAPPED) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_19, "Thread is currently swapped out.\n"));
		return(-1);
	}

	printf( catgets(scmc_catd, MS_crsh, U_MSG_20, "KERNEL STACK:\n") );

	curaddr = (ulong *)ALIGN(FP);
	stacktop = (ulong *)x_thread.ut_kstack;

	if ((FP >> SEGSHIFT) == PRIVSEG) {
		/* the stack is in segment 2 */
		segment = u_segval;
	}
	else {
		/* the stack is in segment 1 */
		if ((ret = read_thread(threadslot, &thread)) != sizeof(struct thread)) {
			printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_23, 
							"0452-603: Cannot read thread structure.\n"));
			return(-1);
		}
		if ((ret = read_proc(thread.t_procp, &proc)) != sizeof(struct proc)){
			printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_24, 
							"0452-604: Cannot find associated process.\n") );
			return(-1);
		}
		segment = proc.p_kstackseg;
	}

	if ((int)curaddr < stacktop - KSTACKSIZE) {
		printf( catgets(scmc_catd, MS_crsh, U_MSG_23, 
		"0452-765: Frame pointer not valid.\n") );
		return (-1);
	}
	lseek(mem, curaddr, 0);
	do {
		count = (((int)curaddr+BUFSIZ)<stacktop)?BUFSIZ:stacktop-(int)curaddr;
		if (dumpflag) {
			if (readmem_thread(buf, curaddr, count, threadslot) != count){
				printf( catgets(scmc_catd, MS_crsh, U_MSG_24, 
				"0452-768: Cannot read kernel stack at address 0x%8x.\n") ,curaddr);
				return (-1);
			}
			if (*buf == 0xbad)
				return (-1);
		}
		else {
			if (readx(mem, buf, count, segment) != count){
				printf( catgets(scmc_catd, MS_crsh, U_MSG_25, 
				"0452-769: Cannot read kernel stack at address 0x%8x.\n") ,curaddr);
				return (-1);
			}
		}
		for (i = 0; i < count/sizeof(ulong); i++) {
			if ((int)curaddr % 16 == 0) {
				printf("\n");
				printf(FMT, curaddr);
				printf(":\t");
			}
			curaddr++;
			printf("  ");
			printf("%8x", buf[i]);
		}
	} while ((int)curaddr < stacktop);
	printf("\n");
}

static int
finddebug(addr, endaddr, tbp, name)
uint addr, endaddr;
struct tbtable_ext *tbp;
char *name;
{
#define isident1(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define isident(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_' || ((c) >= '0' && (c) <= '9'))

	static char text[1024 * 64];
	char *tp, *tp2;
	short namelen;
	int cnt, i;

	cnt = endaddr - addr;
	if (cnt > sizeof(text)-2048)
		cnt = sizeof(text)-2048;
	tp = text;
	cnt = memread(addr, tp, cnt);
	if (cnt < 0)
		return -1;
	for (; cnt--; tp++) {
		tp2 = tp + 5 * sizeof(int);
		namelen = *((short *) tp2);
		if (namelen < 3 || namelen > 64)
			continue;
		tp2 += sizeof(short);
		if (!isident1(*tp2))
			continue;
		for (i=1; i<namelen; i++)
			if (!isident(tp2[i]))
				break;
		if (i != namelen)
			continue;
		/* got it! */	
		tp2[i] = '\0';

		if (tbp != NULL)
			*tbp = *(struct tbtable_ext *)tp;
		if (name != NULL)
			strcpy(name, tp2);
		return 0;
	}
	return -1;
}

struct syment *
esearch(addr, name, matchname)
unsigned addr;
char *name;
int  matchname;
{
	extern	char symnamebuf[];
	unsigned ledata = 0, letext = 0;
	static char lename[128];
	static struct tbtable_ext tb;
	static struct syment s;
	struct syment *sp, *search();

	sp = search(addr, name, matchname);
	if (sp != NULL && sp != (struct syment *) -1
	&& strcmp(sp->n_name, "endcomm") == 0) {
		if (getle(addr, lename, &letext, &ledata) != 1)
			return sp;
		*symnamebuf = '[';
		strcpy(symnamebuf+1, lename);
		if (addr < ledata) {
			/* get the name from the debug stuff */
			strcat(symnamebuf, ":");
			if (finddebug(addr, ledata, &tb,
					symnamebuf+strlen(symnamebuf)) < 0)
				strcat(symnamebuf, "???");
			s.n_value = letext;
	}
		else
			s.n_value = ledata;
		strcat(symnamebuf, "]");
		s.n_zeroes = 0;
		sp = &s;
	}
	return sp;
}

int
showframe(fptr, iar, flags, prefix)
int fptr, iar;
unsigned int flags;
char *prefix;
{
	extern	char symnamebuf[];
	struct	syment	*sp;
	char *instruction, instmess[128];
	char fps[16];

	sp = esearch(iar, symnamebuf, !MATCHNAME);
	if (sp == 0) {
		printf(catgets(scmc_catd, MS_crsh, U_MSG_26, "\tno match\n"));
			return(-1);
		}
	else if ((int)sp == -1) /* error */
			return(-1);
	instruction = "";
	switch (fptr) {
	case -1L:
	    if (flags & TR_EXTENDED) {
		int op;
		memread(iar, &op, sizeof(op));
		strcpy(instmess, ":  ");
		decode(op, iar, instmess+strlen(instmess));
		instruction = instmess;
		}
	    sprintf(fps, "%sIAR:", prefix);
	    break;
	case -2L:
	    sprintf(fps, "%sLR:", prefix);
	    break;
	default:
	    sprintf(fps, "%s%08x:", prefix, fptr);
	    break;
		}
	if (sp->n_zeroes)
		sprintf(symnamebuf, "%.8s", sp->n_name);
	if ((*symnamebuf != '.' && *symnamebuf != '[') || iar == 0) {
		if (flags & TR_EXTENDED)
			printf("\t%-10.10s  %08x  %s%s\n",
				fps, iar, "<invalid>", instruction);
		else
			printf("\t%8.8x\n", iar);
		}
	else {
		if (flags & TR_EXTENDED)
			printf("\t%-10.10s  %08x  %s + %x%s\n",
				fps, iar, symnamebuf,
				iar - sp->n_value, instruction);
		else
			printf("\t%s ()\n", symnamebuf);
			}
	return (0);
}


static int
dotrace(id,ptr,threadslot,flags,segment,stacktop)
int	id;
int	ptr;
int	threadslot;
unsigned int 	flags;
ulong	segment;	/* segval of uarea. needed to retrieve stack */
ulong	stacktop;

{
	register  int	ret;
	int	count;
	ulong frame_ptr, last_frame_ptr;
	ulong ret_addr;

	if (ptr)
		frame_ptr = Kfp;
	else {
		if (flags &TR_EXTENDED)
			printf("    %#x (excpt=%x:%x:%x:%x:%x) (intpri=%x)\n",
				id, EXP[0], EXP[1], EXP[2], EXP[3], EXP[4],
				INTPRI);
		showframe(-1, IAR, flags, "");
		if (flags & TR_EXTENDED)
			showframe(-2, LR, flags, "*");
		frame_ptr = FP;
		ret_addr = 0;
	}
	last_frame_ptr = frame_ptr - 1;
	count = 0;
	while (frame_ptr <= KSTACK_TOP || count == 0 || flags & TR_MST) {
		if (frame_ptr == 0 ||
		((flags & TR_EXTENDED) && (frame_ptr <= last_frame_ptr
		|| frame_ptr > last_frame_ptr + KSTACKSIZE)))
			break;
		last_frame_ptr = frame_ptr;

		if (!(flags & TR_EXTENDED)
		&& (frame_ptr < KSTACK_TOP - KSTACKSIZE)) {
			printf( catgets(scmc_catd, MS_crsh, U_MSG_27,
				"0452-770: Frame pointer not valid.\n") );
			return (-1);
		}
		if (dumpflag) {
			if (count > 0) {
			if (readmem_thread(&ret_addr, frame_ptr+2*RET_OFFSET, 
				sizeof ret_addr, threadslot) != sizeof ret_addr) {
					if (!(flags & TR_EXTENDED)) {
						printf( catgets(scmc_catd, MS_crsh, U_MSG_28, 
						"0452-771: Cannot read return address at address 0x%8x.\n"),
						frame_ptr+2*RET_OFFSET);
				return (-1);
			}
					else
						ret_addr = 0;
				}
			}
			else
				ret_addr = 0;
			if (ret_addr == 0xbad)
				return (-1);
			if (readmem_thread(&frame_ptr, frame_ptr, sizeof frame_ptr, threadslot) != sizeof frame_ptr){
				if (flags &TR_EXTENDED)
					break;
				printf( catgets(scmc_catd, MS_crsh, U_MSG_29, 
				"0452-772: Cannot read frame pointer at address 0x%8x.\n") ,frame_ptr);
				return (-1);
			}
			if (frame_ptr == 0xbad)
				return (-1);
		}
		else {
			lseek(mem, frame_ptr, 0);
			if (readx(mem, &frame_ptr, sizeof frame_ptr, segment)
			    != sizeof frame_ptr) {
				if (flags &TR_EXTENDED)
					break;
				printf( catgets(scmc_catd, MS_crsh, U_MSG_30, 
				"0452-773: Cannot read frame pointer at address 0x%8x.\n") ,frame_ptr);
				return (-1);
			}
			lseek(mem, RET_OFFSET, 1);
			if (readx(mem, &ret_addr, sizeof ret_addr, segment)
			    != sizeof ret_addr){
				if (!(flags & TR_EXTENDED)) {
					printf( catgets(scmc_catd, MS_crsh, U_MSG_31, 
					"0452-774: Cannot read return address at address 0x%8x.\n") ,frame_ptr+2*RET_OFFSET);
				return (-1);
			}
				else
					ret_addr = 0;
		}
		}
		if (count > 0 && ((ret_addr >> SEGSHIFT)!= KERNELSEG)) {
			printf( catgets(scmc_catd, MS_crsh, U_MSG_32,
				"IAR not in kernel segment.\n") );
			return(-1);
		}
		if (count > 0 || (flags & TR_EXTENDED))
			showframe(frame_ptr, ret_addr, flags, count? "": "*");
		count++;
	}
	printf("\n");
	return(0);
}


prtrace(ptr,threadslot,flags)
int	ptr;	/* If ptr = 1, use Kfp */
int threadslot;
unsigned int 	flags;
{
	int i, ret;
	ulong	segment;	/* segment value of either uarea or thread kernel stacks */
	ulong u_segval;		/* segment value of uarea */
	struct thread thread;
	struct proc proc;
	ulong	stacktop, curaddr;
	struct ppda ppdata;

        if (threadslot != CURTHREAD) {
           struct thread *t;
           if(threadslot > (int)v.ve_thread) {
                printf(catgets(scmc_catd, MS_crsh, THREAD_MSG_1,
                "0452-950: Thread slot %d above maximum limit.Specify a value less than %d.\n") , threadslot,v.ve_thread);
                return;
           }
           if (read_thread(threadslot,&thread) != sizeof (struct thread)) {
                printf( catgets(scmc_catd, MS_crsh, U_MSG_36,
                "0452-777: Cannot read uarea for this thread.\n") );
                return(-1);
           }

           if (thread.t_state == TSNONE) {
                printf( catgets(scmc_catd, MS_crsh, U_MSG_36,
                "0452-777: Cannot read uarea for this thread.\n") );
                return(-1);
           }
        }

	if (flags & TR_MST) {
		struct mstsave mst, *cur_mst;
		printf( catgets(scmc_catd, MS_crsh, U_MSG_33,
			"MST STACK TRACE:\n") );

		if (threadslot != CURTHREAD) {
                   read_thread(threadslot, &thread);
                   cur_mst = (struct mstsave *)(&thread.t_uaddress.uthreadp->ut_save);
                }
		else {
			/*
			 * unwind mst stack starting at csa 
			 */
      			 /* Get the ppda */
		     	if (readmem( &ppdata, SYM_VALUE(Ppd)+selected_cpu*PPDSIZE, PPDSIZE, 0) != PPDSIZE) {
				 printf( catgets(scmc_catd, MS_crsh, U_MSG_34,
				 "0452-775: Cannot read ppda structure from address 0x%08x\n"), SYM_VALUE(Ppd)+selected_cpu*PPDSIZE);
                		 return (-1);
        		}
			cur_mst = ppdata._csa;
		}

		if((ret=getuarea_thread(threadslot,&u_segval)) == -1) {
			printf( catgets(scmc_catd, MS_crsh, U_MSG_36, 
			"0452-777: Cannot read uarea for this thread.\n") );
			return(-1);
		}
		else if(ret == SWAPPED) {
			printf( catgets(scmc_catd, MS_crsh, U_MSG_37,
			"0452-778: Thread is currently swapped out.\n") );
			return(-1);
		}

		stacktop = (ulong *)x_thread.ut_kstack;

		if ((FP >> SEGSHIFT) == PRIVSEG) {
			/* the stack is in segment 2 */
			segment = u_segval;
		}
		else {
			/* the stack is in segment 1 */
			if ((ret = read_thread(threadslot, &thread)) != sizeof(struct thread)) {
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_23, 
								"0452-603: Cannot read thread structure.\n"));
				return(-1);
			}
			if ((ret = read_proc(thread.t_procp, &proc)) != sizeof(struct proc)){
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_24, 
								"0452-604: Cannot find associated process.\n") );
				return(-1);
			}
			segment = proc.p_kstackseg;
		}

		for ( ; cur_mst; cur_mst = mst.prev) {
			/* Read in the mst structure */
			if ((readmem(&mst, (long)(cur_mst), sizeof(mst),
			CURPROC)) != sizeof(mst)) {
				printf(catgets(scmc_catd, MS_crsh, U_MSG_40,
					"0452-781: Mst at 0x%08x paged out \n"),cur_mst);
				continue;
			}
			if (mst.prev == 0xbad){
				printf(catgets(scmc_catd,MS_crsh,U_MSG_41,
					"0452-782: Cannot read mst at 0x%08x \n"),
					cur_mst);
				continue;
			}
			x_thread.ut_save = mst;
			ret = dotrace(cur_mst,ptr,threadslot, flags, segment, stacktop);
		}
	}
	else {
		
		if (ptr)
			printf( catgets(scmc_catd, MS_crsh, U_MSG_42,
				"KFP STACK TRACE:\n") );
		else
			printf( catgets(scmc_catd, MS_crsh, U_MSG_43,
				"STACK TRACE:\n") );

		if (threadslot == CURTHREAD) {
			threadslot = get_slot_curthread();
		}
		if(threadslot > (int)v.ve_thread) {
			printf( catgets(scmc_catd, MS_crsh, U_MSG_44, 
			"0452-783: Thread slot %d above maximum limit. Specify a value less than %d.\n"), 
			threadslot, v.ve_thread);
			return(-1);
		}
		if((ret=getuarea_thread(threadslot,&u_segval)) == -1) {
			printf( catgets(scmc_catd, MS_crsh, U_MSG_45, 
			"0452-784: Cannot read uarea for this thread.\n") );
			return(-1);
		}
		else if(ret == SWAPPED) {
			printf( catgets(scmc_catd, MS_crsh, U_MSG_46, 
			"0452-770: Thread is currently swapped out.\n") );
			return(-1);
		}

		stacktop = (ulong *)x_thread.ut_kstack;

		if ((FP >> SEGSHIFT) == PRIVSEG) {
			/* the stack is in segment 2 */
			segment = u_segval;
		}
		else {
			/* the stack is in segment 1 */
			if ((ret = read_thread(threadslot, &thread)) != sizeof(struct thread)) {
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_23, 
								"0452-603: Cannot read thread structure.\n"));
				return(-1);
			}
			if ((ret = read_proc(thread.t_procp, &proc)) != sizeof(struct proc)){
				printf( catgets(scmc_catd, MS_crsh, FSTACK_MSG_24, 
								"0452-604: Cannot find associated process.\n") );
				return(-1);
			}
			segment = proc.p_kstackseg;
		}
		ret = dotrace(CURID,ptr,threadslot,flags,segment,stacktop);
	}
	return ret;
	printf("\n");
}

#endif /* _POWER */

