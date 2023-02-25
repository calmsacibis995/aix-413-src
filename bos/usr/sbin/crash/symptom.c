 static char sccsid[] = "@(#)37  1.3  src/bos/usr/sbin/crash/symptom.c, cmdcrash, bos41J, 9517B_all 4/27/95 18:48:50";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: symptom
 *
 * ORIGINS: 27
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
#include <sys/types.h>
#include <sys/sysprobe.h>
#define	_SSGENVAR
#include <sys/sstbl.h>
#undef	_SSGENVAR
#include <sys/retids.h>		/* Retain Comp. ids and versions */
#include <sys/probeids.h>	/* Probe ids, */
#include <sys/pseg.h>
#include <sys/errids.h>
#include <errno.h>
#include <assert.h>
#include "crash.h"
#include "crash_msg.h"

/* External symbols/routines */
extern char symnamebuf[];
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern	int dumpflag;
extern char *token();
extern int readmem(char *,ulong,ulong,int);
extern ulong symtoaddr(char *);
extern int errlog(void *,ulong);
extern decode(unsigned long, unsigned long, char *);
extern struct syment * esearch(ulong,char *,int);
unsigned addr;
char *name;
int  matchname;

/* Internal routines */
static void help();
static int get_stack(struct mstsave *);
static void get_info(void *,int);
static void stack_info(int,void *,ulong,char *,ulong,ulong);
static struct stelem *is_panic();
static void cleanup();
static void log_stack(void *);
static void log_led();
static int log_str(char *);
static void dbg_prstack();
static struct stk *nextframe(struct stk *, struct stk *);
static int goodstackp(ulong, ulong);
static int pspace_error();
static void say_error(int,char *);
static int search_stack(int,int [],struct stelem *[]);
static void say_symp(struct probe_rec *);
static int findkwd(int);
static struct stelem *find_caller();
static int valid_inst(struct stelem *);


/* Stack element */
struct	stelem {
	struct stelem *next;	/* Next ptr */
	int attr;		/* Attributes (iar, or, other) */
	void *addr;		/* Address on stack */
	ulong inst;		/* Inst. at the address */
	ulong offset;		/* Offset within function */
	char fname[1];		/* Function name */
};
static struct stelem *stfirst;	/* ptr to First one */
static int logsymp;		/* non-zero if logging data */
static int dbg;			/* non-zero means debug (-d) */
/* A minimal stack frame. */
struct stk {
	struct stk *next;	/* Next frame */
	ulong rsvd;
	void *ret;		/* Return address */
};

#define SEG(n) ((n) >> SEGSHIFT)
#define NOTWORD 3		/* Bits on if not word aligned */

/* LED values */
static ulong ledval, csa;
static char ledval_asc[4];
#define LEDSHIFT 20
#define DSI	0x300		/* data storage interrupt */
#define ISI	0x400		/* Inst. storage interrupt */
#define BADOP	0x700		/* bad op-code */

/* Returns from get_stack */
#define BADADDR		-1	/* Means the iar is bad */
#define BADSTACK	-2	/* the stack is bad */
#define NOMST		-3	/* the mst can't be read */
#define DUMPERR		-4	/* Dump appears bad */

/* Stack frame types */
#define STF_IAR		0x01
#define	STF_LR		0x02
#define STF_OTHER	0x04
#define STF_BADADDR	0x08	/* address is bad. */
#define STF_BADNAME	0x10	/* name is bad. */
#define STF_BAD (STF_BADADDR | STF_BADNAME)
#define IARCALLER	3	/* meta attribute passed to search_stack */

/* Error log record. */
static ERR_REC(ERR_REC_MAX+1) er = {ERRID_SYSDUMP_SYMP,"CMDCRASH"};
static char *er_p;		/* Data pointer. */
#define ER_SIZE	(int)(er_p-(char *)&er)
#define ER_BYTESLEFT (int)((char *)er.detail_data+ERR_REC_MAX-er_p)

/*
 * Function:  symptom()
 *
 * This routine has two possible behaviors
 * 1>  Display a symptom string.
 * 2>  Put a symptom entry in the error log
 *
 * Behavior #1 will be used mostly by service personel who want
 * to generate a symptom string manually.
 * The second behavior is how symptom data will be put into the
 * error log for system dumps.  The logging of this data may
 * start automatic phone home activity.
 *
 * The symptom string is generated depending upon the LED code 
 * indicating the type of dump.  In addition, the call stack is
 * put into the error log entry if we're logging.
 *
 * Input:  -e means log the symptom.
 *	   -d debug (internal only)
 *
 * This routine returns no status.
 */
void
symptom()
{
#define RETURN cleanup(); return

	int rc;
	char *parm = (char *)NULL;
	/* Used for search_stack */
	int srch_attr[2], p_id;
	struct stelem *srch_ret[2], *pst;
	/* Declare a probe_rec p with 8 elements (the maximum) */
	PROBE_DATA(p,8);

	/* Initialize local static variables */
	stfirst = (struct stelem *)NULL;
	dbg = logsymp = 0;
	ledval = csa = 0;
	er_p = er.detail_data;	/* error log data pointer. */

	/* If -e specified, log the symptom. */
	for (parm=token(); parm; parm=token()) {
#ifdef DEBUG
		if (parm[0]!='-') break;
#else /* Normal case (DEBUG off) */
		if (parm[0]!='-') {
			help();
			while(token());	/* Flush any remaining parms */
			RETURN;
		}
#endif /* DEBUG */

		switch(parm[1]) {
		case 'd': dbg++; break; /* Internal only */
		case 'e': logsymp++; break;
		default:
			help();
			RETURN;
		}
	}
#ifdef DEBUG
	/* If it's not a switch (-), assume it's an LED code
	 * and a csa value in hex.
	 * This is for internal use (debugging).
	 */
	if (parm) {
		sscanf(parm,"%x",&ledval);
		parm = token();
		if (parm) {
			sscanf(parm,"%x",&csa);
			while(token());	/* Flush any remaining parms */
		}
		else {
			fprintf(stderr,"LED with no CSA.\n");
			RETURN;
		}
	}
#endif /* DEBUG */

	/* If this isn't a dump, quit. */
	if (!dumpflag) {
		fprintf(stderr,catgets(scmc_catd, MS_crsh, SYMP_MSG_2,
		    "This only works on a dump.\n") );
		RETURN;
	}

	/* If logging, the user must be root */
	if (logsymp && (getuid() != 0)) {
		fprintf(stderr,catgets(scmc_catd, MS_crsh, SYMP_MSG_4,
		    "You must be root to log symptom data.\n") );
		logsymp = 0;
	}

	/* Get the abend code and csa */
	if (ledval == 0) {
		if ((readmem_thread(&ledval,(long)SYM_VALUE(abend_code),sizeof(ledval),CURTHREAD) != sizeof(ledval)) ||
		    (readmem_thread(&csa,(long)SYM_VALUE(abend_csa),sizeof(csa),CURTHREAD) != sizeof(csa)) ) {
			fprintf(stderr,catgets(scmc_catd, MS_crsh, SYMP_MSG_1,
		    "can't read abend code or csa.\n") );
			RETURN;
		}
		else /* Fix up the LED value */
		     ledval = ledval >> LEDSHIFT;
	}
	/* Get string representation of LED code */
	sprintf(ledval_asc,"%3x",ledval);

	/* Setup the symptom string header info. */
	p.erecp = (struct err_rec *)&er;
	p.sskwds[0].sskwd_id = SSKWD_PIDS;
	p.sskwds[0].SSKWD_PTR = BOS_COMP;
	p.sskwds[1].sskwd_id = SSKWD_LVLS;
	p.sskwds[1].SSKWD_PTR = BOS_VERS;
	p.sskwds[2].sskwd_id = SSKWD_PCSS;
	p.sskwds[2].SSKWD_PTR = PCSS_SYSDUMP;
	/* sskwds[3] is filled in later */
	p_id = 4;	/* Next array id to use. */

	/* Log the led value */
	log_led();

	/* According to the dump code */
	if (dbg) printf("ledval = %x, csa = %x\n",ledval,csa);
	switch(ledval) {
	case DSI: /* 300 (DSI) - bad data address */
		/* Don't generate if this is a paging i/o error. */
		if ((rc=pspace_error())>0) {
			say_error(SYMP_MSG_6,"Paging device I/O error\n");
			RETURN;
		} else if (rc<0) { /* error reading dump */
			if (dbg) printf("pspace_error failed, rc=%d\n",rc);
			say_error(SYMP_MSG_7,"Couldn't retrieve dump data\n");
			RETURN;
		}

		/* Get stack information from mst at the csa. */
		if (rc=get_stack((struct mstsave *)csa)) {
			if (dbg) printf("get_stack failed, rc=%d\n",rc);
			say_error(SYMP_MSG_7,"Couldn't retrieve dump data\n");
			RETURN;
		}

		/* Log the stack */
		log_stack((void *)csa);

		/* If we got routine and inst. at the IAR, and
		 * routine and offset of the caller, generate string. */
		srch_attr[0] = STF_IAR;
		srch_attr[1] = IARCALLER;
		if (search_stack(2,srch_attr,srch_ret) != 2) {
			if (dbg) printf("search_stack failed\n");
			say_error(SYMP_MSG_7,"Couldn't retrieve dump data\n");
			RETURN;
		}
		/* See if the IAR was ok.
		 * The caller we know was ok. */
		if (srch_ret[0]->attr & STF_BAD) {
			say_error(SYMP_MSG_5,"Can't generate symptom data from this dump\n");
			RETURN;
		}

		/* Generate the symptom string. */
		p.sskwds[p_id].sskwd_id = SSKWD_FLDS;
		if (strlen(srch_ret[0]->fname)>FLDS_LEN)
			srch_ret[0]->fname[FLDS_LEN] = '\0';
		p.sskwds[p_id++].SSKWD_PTR = srch_ret[0]->fname;
		p.sskwds[p_id].sskwd_id = SSKWD_VALU;
		p.sskwds[p_id++].SSKWD_VAL = srch_ret[0]->inst;
		if (strlen(srch_ret[1]->fname)>FLDS_LEN)
			srch_ret[1]->fname[FLDS_LEN] = '\0';
		p.sskwds[p_id].sskwd_id = SSKWD_FLDS;
		p.sskwds[p_id++].SSKWD_PTR = srch_ret[1]->fname;
		p.sskwds[p_id].sskwd_id = SSKWD_VALU;
		p.sskwds[p_id++].SSKWD_VAL = srch_ret[1]->offset;
		break;
	
	case ISI: /* 400 - bad instruction address */
		/* Don't generate if this is a paging i/o error. */
		if ((rc=pspace_error())>0) {
			say_error(SYMP_MSG_6,"Paging device I/O error\n");
			RETURN;
		} else if (rc<0) { /* error reading dump */
			if (dbg) printf("pspace_error failed, rc=%d\n",rc);
			say_error(SYMP_MSG_7,"Couldn't retrieve dump data\n");
			RETURN;
		}

		/* Get stack information from mst at the csa. */
		if (rc=get_stack((struct mstsave *)csa)) {
			if (dbg) printf("get_stack failed, rc=%d\n",rc);
			say_error(SYMP_MSG_7,"Couldn't retrieve dump data\n");
			RETURN;
		}

		/* Log the stack */
		log_stack((void *)csa);

fake_ISI: /* This handles a BADOP (700) if the inst. is invalid */
		/* If LR is ok and got function name then
		 * generate string using routine and offset at LR. */
		srch_attr[0] = STF_LR;
		if (search_stack(1,srch_attr,srch_ret) != 1) {
			if (dbg) printf("search_stack failed\n");
			say_error(SYMP_MSG_7,"Couldn't retrieve dump data\n");
			RETURN;
		}
		/* See if the LR was ok */
		if (srch_ret[0]->attr & STF_BAD) {
			say_error(SYMP_MSG_5,"Can't generate symptom data from this dump\n");
			RETURN;
		}

		/* Generate the symptom string. */
		p.sskwds[p_id].sskwd_id = SSKWD_FLDS;
		if (strlen(srch_ret[0]->fname)>FLDS_LEN)
			srch_ret[0]->fname[FLDS_LEN] = '\0';
		p.sskwds[p_id++].SSKWD_PTR = srch_ret[0]->fname;
		p.sskwds[p_id].sskwd_id = SSKWD_VALU;
		p.sskwds[p_id++].SSKWD_VAL = srch_ret[0]->offset;
		break;
		
	case BADOP: /* 700 - bad inst or trap */
		/* Get stack information from mst at the csa. */
		if (rc=get_stack((struct mstsave *)csa)) {
			if (dbg) printf("get_stack failed, rc=%d\n",rc);
			say_error(SYMP_MSG_7,"Couldn't retrieve dump data\n");
			RETURN;
		}

		/* Log the stack */
		log_stack((void *)csa);

		/* If inst. at the iar is invalid, treat like a 400. */
		srch_attr[0] = STF_IAR;
		if (search_stack(1,srch_attr,srch_ret) != 1) {
			if (dbg) printf("search_stack failed\n");
			say_error(SYMP_MSG_7,"Couldn't retrieve dump data\n");
			RETURN;
		}
		/* Check instruction at iar. */
		if (!valid_inst(srch_ret[0])) {
			/* Invalid, treat like a 400 (ISI) */
			ledval = ISI;
			sprintf(ledval_asc,"%3x",ledval);
			goto fake_ISI;
		}

		/* If is a panic (panic is on the stack),
		 * Report panic's caller, routine and offset. */
		if (pst=is_panic()) {
			/* panic, pst points to saved stack frame of panic */
			if (!(pst=pst->next) || (pst->attr & STF_BAD)) {
				say_error(SYMP_MSG_5,"Can't generate symptom data from this dump\n");
				RETURN;
			}
			/* Data looks ok */
			p.sskwds[p_id].sskwd_id = SSKWD_FLDS;
			if (strlen(pst->fname)>FLDS_LEN)
				pst->fname[FLDS_LEN] = '\0';
			p.sskwds[p_id++].SSKWD_PTR = pst->fname;
			p.sskwds[p_id].sskwd_id = SSKWD_VALU;
			p.sskwds[p_id++].SSKWD_VAL = pst->offset;
		}
		else { /* If not a panic, report routine and inst. at the iar,
		        * and routine and offset for the caller. */
			srch_attr[0] = STF_IAR;
			srch_attr[1] = IARCALLER;
			if (search_stack(2,srch_attr,srch_ret) != 2) {
				if (dbg) printf("search_stack failed\n");
				say_error(SYMP_MSG_7,"Couldn't retrieve dump data\n");
				RETURN;
			}
			/* See if the IAR was ok.
		 	* The caller we know was ok. */
			if (srch_ret[0]->attr & STF_BAD) {
				say_error(SYMP_MSG_5,"Can't generate symptom data from this dump\n");
				RETURN;
			}

			/* Generate the symptom string. */
			p.sskwds[p_id].sskwd_id = SSKWD_FLDS;
			if (strlen(srch_ret[0]->fname)>FLDS_LEN)
				srch_ret[0]->fname[FLDS_LEN] = '\0';
			p.sskwds[p_id++].SSKWD_PTR = srch_ret[0]->fname;
			p.sskwds[p_id].sskwd_id = SSKWD_VALU;
			p.sskwds[p_id++].SSKWD_VAL = srch_ret[0]->inst;
			if (strlen(srch_ret[1]->fname)>FLDS_LEN)
				srch_ret[1]->fname[FLDS_LEN] = '\0';
			p.sskwds[p_id].sskwd_id = SSKWD_FLDS;
			p.sskwds[p_id++].SSKWD_PTR = srch_ret[1]->fname;
			p.sskwds[p_id].sskwd_id = SSKWD_VALU;
			p.sskwds[p_id++].SSKWD_VAL = srch_ret[1]->offset;
		}
		break;

	/* Can't generate symptom string for other LED values. */
	default:
		say_error(SYMP_MSG_5,"Can't generate symptom data from this dump\n");
		RETURN;
	}

	/* Write the symptom data. */
	p.sskwds[3].sskwd_id = SSKWD_MS;
	p.sskwds[3].SSKWD_PTR = ledval_asc;
	p.nsskwd = p_id;	/* Set # of keywords */
	say_symp((struct probe_rec *)&p);

	RETURN;

#undef	RETURN
}

/*
 * Function:  Get_stack
 *
 * Function:
 *   Get the stack associated with the passed csa,
 *   current save area.
 *
 * Output:
 *   The stack elements will be saved by stack_info.
 *   -1 is returned if the stack can't be traversed.
 */
static int
get_stack(csa)
struct mstsave *csa;
{
	struct mstsave mst;
	struct stk stk, *stkp;
	int rc = 0, first;

	/* Get the mst at csa */
	if (readmem_thread(&mst,csa,sizeof(mst),CURTHREAD) != sizeof(mst))
		return(NOMST);

	/* setup stack info for the iar */
	get_info((void *)mst.iar,STF_IAR);

	/* setup stack info for the lr */
	get_info((void *)mst.lr,STF_LR);

	/* for each frame:
	 * (This will terminate because nextframe demands stack address
	 *  be increasing and in the same segment.
	 */
	for (stkp=(struct stk *)mst.gpr[1],rc=BADSTACK,first=1;
	     goodstackp((ulong)csa,(ulong)stkp);
	     stkp = nextframe(stkp,&stk),rc=0) {
		 if (readmem_thread(&stk,stkp,sizeof(stk),CURTHREAD)!=sizeof(stk)) {
			if (dbg) printf("get_stack: couldn't read stack at %x\n",stkp);
			return(BADSTACK);
		}

		 /* Skip first frame because info belongs to callee if any */
		 if (first-- == 1) continue;

		/* Setup the stack info. */
		get_info(stk.ret,STF_OTHER);
	}
	/* At this point, rc is either BADSTACK if we couldn't read
	 * in any stack frames, or zero if we read at least one.
	 * Note that we'll break right out returning BADSTACK if we
	 * couldn't read the data in.
	 */

	if (dbg) dbg_prstack();

	return(rc);
}

/*
 * Function:  get_info
 *
 * Given an address, get stack information to save, see stelem.
 * The flags tell us what kind of address it is.
 */
static void
get_info(addr,flags)
void *addr;
int flags;
{
	struct syment *sp;
	ulong inst, symplen;
	char *symp;		/* Symbol pointer */

	symp = (char *)NULL;
	symplen = 0;

	/* Attempt to read instruction from the passed address. */
	if (((unsigned)addr & NOTWORD) || 
	    (readmem_thread(&inst,addr,sizeof(inst),CURTHREAD)!=sizeof(inst))) {
		flags |= STF_BADADDR;
		stack_info(flags,addr,0,(char *)NULL,0,0);
	} else { /* inst. address looks ok. */
		/* Get the function name and beginning */
		sp = esearch((ulong)addr,symnamebuf,0);
		if (!sp || (sp == (struct syment *)-1) ||
	            (strcmp(symnamebuf,"endcomm") == 0)) {
			flags |= STF_BADNAME;
		}
		else {	/* Setup the symbol pointer */
			symp = symnamebuf;
			symplen = strlen(symp);
			if (*symp == '.') {
				symp++;
				symplen--;
			}
			else if ((char)*symp == '[') {
				/* Kernel extension, func name is past colon */
				char *p;
				symp++;
				p = (char *)strchr(symp,':');
				if (p) symp = p+1;
				symplen = strlen(symp);
				if (*(symp+symplen-1)==']') symplen--;
			}
		}

		/* Save the data */
		stack_info(flags,addr,inst,symp,symplen,
			   (ulong)((ulong)addr-sp->n_value));
	}
}

/*
 * Function:  stack_info
 *
 * Allocate a new stelem structure and put the stack info in it.
 * Put the new buffer on the chain of stack elements anchored by stfirst.
 * Reallocate the buffer if too small.
 *
 * Input:
 *	attr - attribute of entry (iar, lr, other)
 *	address - address of the code pointed to.
 *	inst - instruction at the address
 *	func - function at the address
 *	funclen - length of func name to copy (not including '0')
 *	offset - offset within that function
 *
 * Returns false if unsuccessful.
 */
static void
stack_info(attr,addr,inst,func,funclen,offset)
int attr;
void *addr;
ulong inst;
char *func;
ulong funclen;
ulong offset;
{
#define elsize sizeof(struct stelem)+funclen+1
	static struct stelem *stp;	/* Maintaines ned-of-stack */
	struct stelem *s;

	/* Allocate storage for the data */
	if (!func) funclen = 0;
	s = (struct stelem *)malloc(elsize);
	if (!s) {
		fprintf(stderr,catgets(scmc_catd, MS_crsh, DLA_MSG_7,
		    "Out of memory\n") );
		exit(1);
	}
	if (stfirst == (struct stelem *)NULL) {
		/* First one allocated */
		stfirst = stp = s;
	}
	else {	/* put new one on the chain */
		stp->next = s;
		stp = s;
	}
	stp->next = (struct stelem *)NULL;

	/* Put the data in the buffer. */
	stp->attr = attr;
	stp->addr = addr;
	stp->inst = inst;
	stp->offset = offset;
	strncpy(stp->fname,func,funclen);
	stp->fname[funclen] = '\0';
}

/*
 * Go to the next stack frame.
 *
 * Input:
 *   The inputs are the stack address in the dump and the address of the
 *   crash memory containing the stack frame.
 *
 * Return the next frame pointer or null if none or invalid.
 * goodstackp provides more checking.
 */
static struct stk *
nextframe(stkp,stk)
struct stk *stkp,*stk;
{
	/* Stack addresses must be increasing and in the same segment. */
	if ((ulong)stk->next <= (ulong)stkp) return((struct stk *)NULL);
	if (SEG((ulong)stkp) != SEG((ulong)stk->next)) return((struct stk *)NULL);

	/* Return next frame ptr */
	return(stk->next);
}
	

/*
 * Function:  goodstackp
 *
 * Test to see if the stack pointer is ok.
 * Return true if yes, false (0) if not.
 */
static int
goodstackp(csa,stkp)
ulong csa, stkp;
{
	if (dbg) printf("goodstackp: stkp=%x,csa=%x\n",stkp,csa);
	/* Don't allow a null pointer */
	if (stkp == (ulong)NULL) return(0);

	/* Must be word aligned */
	if (stkp & NOTWORD) return(0);

#if 0 /* Don't do all this */
	
	/* Stack must be in same address space as the csa. */
	if (SEG(csa) != SEG(stkp)) return(0);

	/* if in the Kernel, the stack must be below the mst
	 * and in the same frame.
	 */
	if (SEG(stkp) == KERNELSEG) {
		if ((stkp >= csa) || 
		    (stkp < (csa+sizeof(struct mstsave)-FRAMESIZE)))
			return(0);
		else return(1); /* It's ok */
	}
	
	/* If in PRIVSEG (2) it must be no more than KSTACKSIZE below
	 * the csa.
	 */
	/* We'll assume if it's not in Kernel segment, the private
	 * segment test is ok?
	 */
	if ((stkp >= csa) || 
	    (stkp < (csa+sizeof(struct mstsave)-KSTACKSIZE)))
		return(0);
#endif /* 0 */
	return(1); /* It's ok */
}

/*
 * See if this dump is a panic.
 *
 * Returns address of stelem structure for panic, or NULL if not found.
 */
static struct stelem *
is_panic()
{
	struct stelem *s = stfirst;

	/* For each saved stack element */
	for (; s; s=s->next) {
		if (strcmp(s->fname,"panic")==0) return(s);
	}
	return((struct stelem *)NULL);	/* Not a panic */
}

/*
 * Clean up the stack storage.
 */
static void
cleanup()
{
	for (; stfirst; stfirst=stfirst->next) free(stfirst);
	stfirst = (struct stelem *)NULL;
}

/*
 * Function:  pspace_error
 *
 * Check for a paging i/o error.
 * Return non-zero if true.
 */
static int
pspace_error()
{
#define	RETC_OFST 0x20
	
	void *addr;
	ulong code;

	/* Read in the vmm error log. */
	if ((addr = (void *)symtoaddr("vmmerrlog")) == (void *)-1L) return(DUMPERR);
	if (readmem_thread(&code,(ulong)addr+RETC_OFST,sizeof(code),CURTHREAD) != sizeof(code))
		return(DUMPERR);

	/* Check the return code. */
	if (code == EIO) return(1);
	return(0);	/* No paging error */
}

/*
 * Function:  say_error
 *
 * print or log an error.
 * The error is printed if we're not logging, otherwise it's put
 * into the error log entry.
 *
 * Input:
 *	rc is a return code.
 *	msg is the error message number from the catalog.
 *	deflt is the default message text.
 */
static void
say_error(msg,deflt)
int msg;
char *deflt;
{
	char *msgp;
	int len;

	msgp = catgets(scmc_catd, MS_crsh, msg, deflt);
	fprintf(stderr,msgp);

	if (logsymp) {
		/* See if message will fit in error log. */
		if ((len=strlen(msgp))>ER_BYTESLEFT) len = ER_BYTESLEFT;
		strncpy(er_p,msgp,len);
		er_p += len;	/* go past trailing '\0' too */
		errlog(&er,ER_SIZE);
		/* Note:  If that +1 pushes us over the max for an
		 * error log record, the record gets truncated, so
		 * it's not a problem.
		 */
	}
}

/*
 * Function:  search_stack
 *
 * Look for the specified stack elements.  If one element isn't
 * found, none of the following elements will be.
 *
 * Input:
 *   An array of ints giving the attributes (stelem.attr)
 *   to return.  These must be in the order they'll appear on the stack.
 *
 * Output:
 *   The srch_ret array will contain the address of the elements found.
 *   The number found is returned.
 */
static int
search_stack(n,srch_attr,srch_ret)
int n, srch_attr[];
struct stelem *srch_ret[];
{
	int ipos,opos;	/* input and output positions (indexes) */
	struct stelem *s;

	/* Look thru the stack. */
	for (ipos=opos=0, s=stfirst; ipos<n && s; ) {
		/* If the IAR's caller is wanted, look for it and go on */
		if (srch_attr[ipos] == IARCALLER) {
			if ((srch_ret[opos] = find_caller()) != (struct stelem *)NULL)
				opos++;

			/* Look for next item */
			ipos++;
			continue;
		}

		/* Normal search argument, a bit flag */
		if (s->attr & srch_attr[ipos]) {
			/* Use the first match, this one. */
			srch_ret[opos++] = s;
			ipos++;
		}

		s = s->next;
	}

	/* opos will contain the number of elements returned. */
	return(opos);
}

/*
 * Function:  find_caller
 *
 * Find who called the function at the IAR.
 * It will most likely be the 2nd stack frame (the first
 * was bought by the current function.  I may, however, be the
 * lr if no calls have been issued from the current function.
 *
 * Input:  none
 *
 * Output:
 *   The address of the stelem structure is returned,
 *   or NULL if no match.
 */
static struct stelem *
find_caller()
{
	struct stelem *s = stfirst;
	char *iarfunc;

	/* Look for the IAR */
	while (s && !(s->attr & STF_IAR)) s = s->next;
	if (!s) return((struct stelem *)NULL);
	iarfunc = s->fname; /* NULL if name was bad */

	/* Go to the LR now */
	if (!(s = s->next) || !(s->attr & STF_LR))
		return((struct stelem *)NULL);
	
	/* if the function at the LR is different than the IAR's
	 * function, and the LR's not null, then we'll assume 
	 * the LR is the caller.
	 */
	if (!(s->attr & STF_BAD) && (s->addr != (void *)NULL) &&
	    (strcmp(iarfunc,s->fname) != 0))
		return(s);

	/* Now we're ready to traverse the real stack.
	 * Ignore the first frame because the ret. addr (if any) would be
	 * saved by a callee, not a caller.
	 */
	/* Note:  the first real stack frame is junk and ignored by get_stack.*/
	if (!(s=s->next) || (s->addr = (void *)NULL) ||
	    (s->attr & STF_BAD))
		return((struct stelem *)NULL);
	return(s);
}

/*
 * Function:  say_symp
 *
 * Print or log a symptom string.
 *
 * Input:
 *	A probe_rec structure
 *
 * Output:
 *   A symptom string is displayed and logged if logsymp.
 */
static void
say_symp(p)
struct probe_rec *p;
{
	int i,kwdid;

	for (i=0; i<p->nsskwd; i++) {
		kwdid = findkwd(p->sskwds[i].sskwd_id);
		assert(kwdid != -1);

		/* Print the keyword. */
		printf("%s/",kwdtbl[kwdid].kwdn);

		/* print the data according to its type. */
		switch(kwdtbl[kwdid].type) {
		case DT_STR:
			printf("%s ",p->sskwds[i].SSKWD_PTR);
			break;
		case DT_SHORT:
		case DT_INT:
			printf("%d ",p->sskwds[i].SSKWD_VAL);
			break;
		case DT_USHORT:
		case DT_ULONG:
			printf("%x ",p->sskwds[i].SSKWD_VAL);
			break;
		}
	}
	printf("\n");

	/* Only log if desired */
	if (logsymp) {
		p->erecl = ER_SIZE;
		if (dbg) printf("error record length is %d\n",p->erecl);
		assert(probe(p) == 0);
	}
}

/*
 * Function:  findkwd
 *
 * Look up the keyword in kwdtbl (see sstbl.h).
 *
 * Input:
 *   The keyword.
 *
 * Output:
 *   The keyword's table index or -1 if not found.
 */
static int
findkwd(kwd)
int kwd;
{
	int i;

	for (i=0; kwdtbl[i].kwd; i++)
		if (kwdtbl[i].kwd == kwd) return(i);
	return(-1);
}

/*
 * Test to see if the instruction is valid.
 *
 * Input:  stelem pointer
 *
 * Returns 1 if valid, 0 if not or if it can't read the memory.
 */
static int
valid_inst(stk)
struct stelem *stk;
{
	char ascii[64];

	/* Return false if the address is bad */
	if (stk->attr & STF_BADADDR) return(0);

	/* Address must be word aligned */
	if ((ulong)stk->addr & 0x3) return(0);
	
	/* Decode the instruction. */
	decode((ulong)stk->inst,(ulong)stk->addr,ascii);
	if (dbg) printf("inst:  %s\n",ascii);

	/* Check for validity */
	if (strcmp(ascii,IVD_INST)==0) return(0);
	return(1); /* Looks good */
}

/*
 * Log the LED code
 */
static void
log_led()
{
	char str[11];

	sprintf(str,"LED:%3s\n",ledval_asc);
	log_str(str);
}

/*
 * log stack data
 *
 * This routine assumes it's ok to log the stack (i.e.), we're
 * dealing with a valid stack from a valid abend.
 */
static void
log_stack(csa)
void *csa;
{
	struct stelem *s = stfirst;
	char str[80];

	/* Put the csa @ in the log */
	sprintf(str,"csa:%08x\n",csa);
	if (!log_str(str)) return;

	/* Put the stack in */
	for (; s; s=s->next) {
		sprintf(str,"%s %x\n",s->fname,s->offset);
		if (!log_str(str)) return;
	}
}

/*
 * Put a string in the error log buffer.
 *
 * Returns non-zero (true) if it worked, 0 (false) if not enough space.
 */
static int
log_str(char *s)
{
	ulong len = strlen(s);

	if (len>ER_BYTESLEFT) return(0);
	strcpy(er_p,s);
	er_p += len;
	return(1);
}

/*
 * helo - print help text to stderr.
 */
static void
help()
{
	fprintf(stderr,catgets(scmc_catd, MS_crsh, SYMP_MSG_3,
	    "Usage:  symptom [-e] (-e puts the symptom in the error log.)\n") );
}

/*
 * Print stored stack data
 */
static void
dbg_prstack()
{
	struct stelem *s = stfirst;
	char atr[10];

	printf("\ntype addr inst ofst name\n");
	for (; s; s=s->next) {
		atr[0]='\0';
		if (s->attr & STF_IAR) strcat(atr,"IAR ");
		if (s->attr & STF_LR) strcat(atr,"LR ");
		if (s->attr & STF_OTHER) strcat(atr,"OTHER ");
		if (s->attr & STF_BADADDR) strcat(atr,"IVDADDR");
		if (s->attr & STF_BADNAME) strcat(atr,"IVDNAME");

		printf("%s %x %x %x %s\n",atr,s->addr,s->inst,s->offset,s->fname);
	}
}
