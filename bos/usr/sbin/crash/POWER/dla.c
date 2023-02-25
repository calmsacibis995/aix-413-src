static char sccsid[] = "@(#)44        1.7  src/bos/usr/sbin/crash/POWER/dla.c, cmdcrash, bos41J, 9511A_all 3/9/95 15:15:20";
/*
 * COMPONENT_NAME: (CMDCRASH)
 *
 * FUNCTIONS: dla_ci_dlock
 *
 * ORIGINS: 83, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
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
 *      This code is designed both for crash ( User mode ) or for a
 *      kernel debugger ( kdb in Kernel mode ). This is the reason of the
 *      CRASH compile flag, that mainly decides the way to access
 *      memory informations like threads table, ppda and lock structures...
 *
 * FUNCTIONS: dla_init_descs()           CRASH only
 *            dla_new_desc()
 *            dla_init_bitmap()          CRASH only
 *            dla_mark()
 *            dla_marked()
 *            dla_unmark()
 *            kdb_dla_ci_dlock()         KDB   only
 *            dla_ci_dlock()             CRASH only
 *            dla_Dlock()
 *            dla_owner()
 *            dla_owner_simple()
 *            dla_owner_lockl()
 *            dla_owner_complex()
 *            dla_alr_owner()
 *            dla_get_thread()
 *            set_curslot()
 *            dla_print_locks()
 *            dla_print_desc()
 *            dla_waiting()
 *            cpu_running_thread()
 *            dla_get_sleeping_lock()
 *            dla_get_spinning_lock()
 *            dla_spinning_ctxt()
 *            dla_lock_type()
 *            dla_print_stack()          CRASH only
 *            dla_read_kdb()             KDB   only
 *            dla_read()                 CRASH only
 *            dla_findsym()              CRASH only
 *            dla_symaddr()              
 *            dla_is_simple()
 *            dla_is_disable()
 *            dla_is_complex()
 *            dla_is_aix3_2()
 *
 *
 *
 *      These functions provide a way to analyze a deadlock situation.
 *      The research is based on kernel thread structures and kernel lock
 *      structures and primitives. 
 *     
 *      For more informations about algorithm, consult the SDS associated 
 *      to DCR53034.
 *
 */

#include <sys/types.h>
#ifndef _KERNSYS
#define _KERNSYS
#include <sys/lock_def.h>
#include <sys/user.h>
#undef _KERNSYS
#else /* _KERNSYS */
#include <sys/lock_def.h>
#endif  /* _KERNSYS */
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/mstsave.h>
#include <sys/debug.h>
#include <sys/param.h>
#include <syms.h>
#include "dla.h"
#include <values.h>

#ifdef _CRASH
#include <stdio.h>
#include <errno.h>
#include <sys/thread.h>
#include "crash_msg.h"
#else /* _CRASH */
#include "kdb.h"
#endif /* _CRASH */

int dla_stack_debug = 0;

#ifdef _CRASH
static void dla_print_stack(struct mstsave *w_csa, int nb_loop);
static unsigned int *dla_init_bitmap(void);
static void dla_init_descs(int nthread);
static int dla_read(ulong kern_addr, unsigned char *value, unsigned size);
#else /* _CRASH */
static int dla_read_kdb(ulong vad, unsigned char *buf, unsigned len);
static long dla_findsym(ulong val, struct syment **sym_tbl_p, char **str_tbl_p);
#endif /* _CRASH */
static int dla_Dlock(int thp_or_cpuid, owner_t start_type);
static dla_mark(struct thread *thp, unsigned int *bitmap);
static int dla_marked(struct thread *thp, unsigned int *bitmap);
static struct lock_desc *dla_new_desc(void);
static lock_type_t dla_lock_type(struct mstsave *w_csa);
static int dla_owner(ulong waited_lock, owner_t *o_type , lock_type_t l_type);
static int dla_owner_simple(ulong waited_lock, owner_t *o_type);
static int dla_owner_lockl(ulong waited_lock);
static int dla_owner_complex(ulong waited_lock);
static struct lock_desc *dla_alr_owner(int o_id, owner_t o_type,struct lock_desc *ll);
static int dla_get_thread(int tid);
static int dla_print_locks(struct lock_desc *ll, int found, int thp_or_cpuid, int own_type);
static int dla_print_desc(struct lock_desc *ll, int dl);
static void set_curslot(int o_id, owner_t o_type);
static ulong dla_waiting(int o_id,owner_t o_type,struct mstsave **w_csa,lock_type_t *l_type);
static int cpu_running_thread(struct thread *thp);
static ulong dla_get_sleeping_lock(struct thread *thp,struct mstsave **w_csa,lock_type_t *l_type);
static ulong dla_get_spinning_lock(int cpu_id,struct mstsave **w_csa,lock_type_t *l_type);
static ulong dla_spinning_ctxt(struct mstsave *mst,struct mstsave **w_csa,lock_type_t *l_type);
static ulong dla_symaddr(char *symb);
static int dla_is_disable(char *name);
static int dla_is_simple(char *name);
static int dla_is_complex(char *name);
static int dla_is_aix3_2(char *name);

#define DUMMY_SIZE 256 /* should be enough */
#define NOT_IN_MEMORY 0xffffffff
#define INT_LEN ((sizeof(int))*BITSPERBYTE)

#ifdef _CRASH
extern nl_catd scmc_catd;  /* Cat descriptor for scmc conversion */
#define DATA 2
extern int selected_cpu;
char strname[256]; /* for symbol search if crash */
extern struct var v;    /* for dynamicaly defining NTHREAD */
#define N_THREAD ((int)v.ve_thread)
unsigned int *dla_mark_th;
unsigned int *dla_mark_dl;
char symbuf[DUMMY_SIZE]; /* for search */
extern cpu_t ncpus;
#define dla_printf printf
#define dla_readvalue(value, kern_addr) dla_read(kern_addr, &value, sizeof(value))

#else /* _CRASH */

unsigned int dla_mark_th[NTHREAD/INT_LEN + 1] = {0};  /* Bitmap to mark threads */
unsigned int dla_mark_dl[NTHREAD/INT_LEN + 1] = {0};  /* Bitmap to mark threads taken in a deadlock */
#define  dla_printf kdb_printf
#define dla_readvalue(value, kern_addr) dla_read_kdb(kern_addr, &value, sizeof(value))
#define catgets(p1, p2, p3, p4) (p4)
extern struct thread *kdb_get_curthread();
#define ncpus _system_configuration.ncpus

#endif /* _CRASH */

int curslot = 0; /* really useful to specify the context, Segment registers */
#define MAXDESC 256 
#define dla_strcmp strcmp
#define dla_strncpy strncpy
#define dla_strcpy strcpy


struct thread *dla_thread_addr = 0;

/*
  -----------------------                            
*  Lock descriptors management.
*  A descriptor is a structure used to describe a lock (lock_desc).
*  See dla.h for more informations.
*
*  The first step of deadlock search is the constitution of a chain of
*  dependent locks, in order to be able to print this quickly if a 
*  deadlock is found. When the algorithm finds a blocking lock, it
*  will update a descriptor with the lock characteristics, and
*  chain it with the already seen and dependent locks.
*  ----------------------- 
*/

int dla_desc_index = 0;
#ifdef _CRASH
struct lock_desc *dla_desc_tab; /* A table will dynamicaly be allocated */
#else  /* _CRASH */
struct lock_desc dla_desc_tab[MAXDESC] = {0}; /* static table in kernel mode */
#endif /* _CRASH */

/*
* NAME: dla_init_descs()
*
* FUNCTION: allocate a descriptor table, and initialize dla_desc_index. 
*
* RETURNS: NULL if no memory, a pointer to allocated memory else.
*/

#ifdef _CRASH
static void
dla_init_descs(int nthread)
{
	dla_desc_tab = (struct lock_desc *)malloc ((sizeof(struct lock_desc)) * nthread);
	if (dla_desc_tab == NULL) {
		dla_printf( catgets(scmc_catd, 
							MS_crsh, DLA_MSG_1, "0452-1051: Out of memory\n"));
		exit();
	}
	dla_desc_index = 0;
}
#endif /* _CRASH */

/*
* NAME: dla_new_desc()
*
* FUNCTION: return the descriptor designed by dla_desc_index, and
*           increment dla_desc_index. 
*
* RETURNS: NULL if no more descriptors, a pointer to allocated memory else.
*/

static struct lock_desc *
dla_new_desc()
{
	struct lock_desc *new_desc;

	if (dla_stack_debug) 
		dla_printf("dla_new_desc\n");
	if (dla_desc_index >= MAXDESC) {
		dla_printf( catgets(scmc_catd, 
							MS_crsh, DLA_MSG_2, "0452-1052: Out of lock descriptors\n"));
		return NULL;
	}
	return (dla_desc_tab + dla_desc_index++);
}

/*
  -----------------------                            
*  Mark and unmark threads - Bitmap management
*  
*  There are two bitmaps to mark threads regarding different criteria.
*  - dla_mark_th: is used to know which threads have already been parsed.
*  Following dependent lock can lead you to parse threads in
*  a different order than the automatic search ( following the threads
*  table slot by slot). It is unuseful to go on with automatic search
*  from an already parsed thread.
*  - dla_mark_dl: is used to know if a thread has already been
*  analyzed as stuck in a deadlock. There are many ways to follow to come
*  to a deadlock, and there is only one deadlock. So that, during
*  automatic search, if following dependent locks leads to a thread
*  marked in this bitmap, return because the concerned deadlock has
*  already been discovered.
*  These two bitmaps use the same primitives for allocating, initializing the
*  bitmap structure, mark a thread, and test if a thread is marked, 
*  In all cases except dla_init_bitmap(), the considered bitmap is a parameter.
*  ----------------------- 
*/

#ifdef _CRASH 

/*
* NAME: dla_init_bitmap()
*
* FUNCTION: Allocate size for bitmap in 
* case of user program (crash)
*
* RETURNS: The pointer to the allocated bitmap. Can exit if no memory.
*/

static unsigned int *
dla_init_bitmap()
{
unsigned int *bitmap;
if ((bitmap = (unsigned int *) malloc((sizeof(int)) * ((N_THREAD/INT_LEN) + 1))) == NULL) {
dla_printf( catgets(scmc_catd, 
MS_crsh, DLA_MSG_1, "0452-1051: Out of memory\n"));
exit();
}
return bitmap;
}
#endif /* _CRASH */


/*
* NAME: dla_mark(thp, bitmap)
*
* FUNCTION: Mark the thread in dla_mark_th in the corresponding
*           slot to the thread pointer thp in the specified bitmap
*
* RETURNS: Nothing.
*/


static dla_mark(struct thread *thp, unsigned int *bitmap)
{
int th_slot; 
unsigned bit, word, *ptr;

th_slot = ((int)thp - (int)dla_thread_addr) / sizeof(struct thread);
word = th_slot/INT_LEN;
ptr = (unsigned *)(bitmap + 4*word);
bit = th_slot - word*INT_LEN;
(*ptr) |= (0x1 << ( 31 - bit));
}

/*
* NAME: dla_marked(thp, bitmap)
*
* FUNCTION: looks in the specified  bitmap to answer
*  the value contained in the slot of thp in the specified bitmap.
* RETURNS: the value of the associated bit.
*/

static int dla_marked(struct thread *thp, unsigned int *bitmap)
{
unsigned bit, word, *ptr;
int th_slot;

th_slot = ((int)thp - (int)dla_thread_addr) / sizeof(struct thread);
word = th_slot/INT_LEN;
ptr = (unsigned *)( bitmap + 4*word);
bit = th_slot - word*INT_LEN;
bit = (*ptr >> (31 - bit)) & 0x1;
return bit;
}

/*
* NAME: dla_unmark(bitmap)
*
* FUNCTION: set the specified bitmap to NULL
*
* RETURNS: Nothing
*/
static dla_unmark(unsigned int *bitmap)
{
int i;
#ifdef _CRASH
int top = N_THREAD/INT_LEN + 1;
#else /* CRASH */
int top = NTHREAD/INT_LEN + 1;
#endif /* _CRASH */
	for (i=0; i<= top; i++)
		bitmap[i] = 0;
}

/*
  -----------------------                            
 *    Subcommand driver
 *  ----------------------- 
 */
/* -------------------------------------------------------------- */
/* Analyze a crash subcommand of the following form:              */
/* expression of command is dlock [ tid / -p [ ProcessorNum ] ]   */
/*                                                                */
/*                                                                */
/* if no parameter: searches deadlocks, looping automatically on  */
/*         any as start point threads with first step on current  */
/*         cpu.                                                   */
/*                                                                */
/* if tid parameter, searches deadlock from the specified thread  */
/*                                                                */
/* if -p ProcessorNum parameter, searches deadlock from the       */
/*        specified processor.                                    */
/*                                                                */
/* if -p parameter, searches deadlocks, looping automatically on  */
/*        any processors as start point.                          */
/*                                                                */
/* -------------------------------------------------------------- */

#define HEXA 16

/*
 * NAME: dla_ci_dlock()
 *
 * FUNCTION: Command interpreter for deadlock analyzer
 *
 * RETURNS: Nothing. returns to main driver.
 */
#ifdef _CRASH

dla_ci_dlock()
{
	struct pm_heap	dla_thread_cb, *dla_thread_cb_addr;
	int   err, n_th, found;
	int threadp, cpu_num, given_tid, i;
	char *arg;
	
	if (dla_stack_debug) 
		dla_printf("dla_ci_dlock\n");
	
	dla_thread_addr = (struct thread *)dla_symaddr("thread");
	if (dla_stack_debug) 
		dla_printf("thread_addr %x\n", dla_thread_addr);
	dla_mark_th = dla_init_bitmap();
	dla_mark_dl = dla_init_bitmap();
	dla_unmark(dla_mark_th); /* unmark threads regarding to parse*/
	dla_unmark(dla_mark_dl); /* unmark threads regarding to deadlocks */
	dla_init_descs(MAXDESC);
	curslot = 0; /* never swapped */
	
	/* if no parameter: looping automatically on
	   threads with first step on current cpu and return */
	
	if ((arg = token()) == NULL) {
		dla_Dlock(selected_cpu, o_it_handler); /* start point is the current cpu */

		dla_thread_cb_addr = (struct pm_heap *)dla_symaddr("thread_cb");
		if (!dla_readvalue(dla_thread_cb, dla_thread_cb_addr)){
			dla_printf("invalid thread_cb_addr %x\n", dla_thread_addr);
			return NULL;
		}
		if (dla_stack_debug){ 
			dla_printf("dla_cb_addr %x", dla_thread_cb_addr);
			dla_printf(" highwater %x\n", dla_thread_cb.highwater);
		}
		for (n_th=0, found = 0;
			 (n_th < N_THREAD) /* limit the search under highwater mark */
			 && ((dla_thread_addr + n_th) < (struct thread *)dla_thread_cb.highwater);
			 n_th++) { 
			/* loop on thread table, each unmarked thread
                           is taken as start point for the search */

			/* Need to show an activity, printing recursive chars to one same line */
			if (!dla_stack_debug) {
				if ((n_th % 80) == 0)
					dla_printf("\r");
				if ((n_th % 160) >= 80)
					dla_printf("*");
				else
					dla_printf(".");
				fflush(stdout);
			}
			if (!dla_marked(dla_thread_addr + n_th, dla_mark_th) == 1) {
				found += dla_Dlock(dla_thread_addr + n_th, o_thread);
			}
		}
		if (!dla_stack_debug) {
			for(i=0; i < 80; i++)
				dla_printf(" ");
			dla_printf("\r");
		}
		if (!found)
			dla_printf( catgets(scmc_catd, 
					    MS_crsh, DLA_MSG_3, " No deadlock found\n"));
		return;
	}
	
	/* if there is a -p ProcessorNum sequence, search a deadlock 
	   starting from the cpu identified with ProcessorNum and return */
	
	if (strcmp(arg, "-p") == 0) {
		if ((arg = token()) == NULL) { /* have to loop on any avalaible cpu */
			for(cpu_num = 0, found = 0; cpu_num < ncpus; cpu_num++) {
				if (!dla_stack_debug) { /* has to show activity */
					dla_printf(".");
					fflush(stdout);
				}
				found += dla_Dlock(cpu_num, o_it_handler);
			}
			if (!dla_stack_debug) {
				for(i=0; i < 80; i++)
					dla_printf(" ");
				dla_printf("\r");
			}
			if (!found)
				dla_printf( catgets(scmc_catd, 
						    MS_crsh, DLA_MSG_3, " No deadlock found\n"));
			return ;
		} else {
			cpu_num = atoi(arg);
			if ( (cpu_num < ncpus) && (cpu_num >= 0)) {
				if (!dla_Dlock(cpu_num, o_it_handler))
					dla_printf( catgets(scmc_catd, 
							    MS_crsh, DLA_MSG_4, 
							    " No waited lock on cpu %d\n"), cpu_num);
			} else
				dla_printf( catgets(scmc_catd, 
						    MS_crsh, DLA_MSG_5, 
						    "0452-1055: This is not a running cpu\n"));
			for (;(arg = token()) != NULL;)
				dla_printf( catgets(scmc_catd,
                                                    MS_crsh, DLA_MSG_7,
                                                    "0452-1057: Bad argument for deadlock analyzer\n"));

		}
		return;
	}
	
	/*  else the defect parameter is a tid, search a deadlock
	    starting from this thread */

	if ((given_tid = strtoul(arg, &arg, HEXA)) != ULONG_MAX){ /* correct conversion */
		threadp = dla_get_thread(given_tid);
		if (threadp != NULL) {
			/* given_addr is correct in thread table */
			if (!dla_Dlock(threadp, o_thread))
				dla_printf( catgets(scmc_catd, 
						    MS_crsh, DLA_MSG_6, 
						    " No waited lock for thread %x\n"), threadp);
		} else {
			dla_printf( catgets(scmc_catd, 
					    MS_crsh, DLA_MSG_14, 
					    "0452-1064: Cannot access this thread \n"));
		}
		return;
	}
	dla_printf( catgets(scmc_catd, 
			    MS_crsh, DLA_MSG_7, 
			    "0452-1057: Bad argument for deadlock analyzer\n"));
}
#else /* _CRASH */
kdb_dla_ci_dlock(unsigned char *buff, int *buffix, int linelen)
{
	
	struct var *v_a, v;
	int   err, n_th, found;
	int threadp, cpu_num, given_tid, i;
	char *arg;
	
	kdb_more_on();
	
	dla_thread_addr = kdb_thread_addr;
	dla_unmark(dla_mark_th); /* unmark threads regarding to parse*/
	dla_unmark(dla_mark_dl); /* unmark threads regarding to deadlocks */
	dla_desc_index = 0;
	curslot = kdb_get_curthread() - kdb_thread_addr;
	
	/* if no parameter: looping automatically on
	   threads with first step on current cpu and return */
	
	kdb_deblank(buff, buffix, linelen);
	if (*buffix >= linelen) {
		dla_Dlock(kdb_cpunb, o_it_handler);
		if (kdb_symboladdr(v_a, "v"))
			kdb_readvalue(v, v_a);
		for (n_th=0, found = 0; (kdb_thread_addr + n_th) < (struct thread *)v.ve_thread; n_th++) {
			if (kdb_breakchar())
				break;
			if (!dla_marked(kdb_thread_addr + n_th, dla_mark_th) == 1) {
				found += dla_Dlock(kdb_thread_addr + n_th, o_thread);
			}
		}
		if (!found)
			kdb_printf(" No deadlock found\n");
		kdb_more_off();
		return;
	}
	
	/* if there is a -p ProcessorNum sequence, search a deadlock 
	   starting from the cpu identified with ProcessorNum and return */
	
	if (kdb_strncmp((char *)buff + *buffix, "-p", 2) == 0) {
		*buffix +=2;
		kdb_deblank(buff, buffix, linelen);
		if (*buffix >= linelen) { /* have to loop on any avalaible cpu */
			for(cpu_num = 0, found = 0; cpu_num < ncpus; cpu_num++)
				found += dla_Dlock(cpu_num, o_it_handler);
			if (!found)
				kdb_printf("No deadlock found\n", cpu_num);
			kdb_more_off();
			return ;
		} else {
			if ((err = kdb_expr(10, buff, buffix, linelen, &cpu_num)) == 0) {
				if (cpu_num < ncpus && (cpu_num >= 0)) {
					/* running cpu ? */
					if (!dla_Dlock(cpu_num, o_it_handler))
						kdb_printf("No waited lock on processor %d\n", cpu_num);
				}
				else
					kdb_printf("This is not a running cpu\n");
			}
			kdb_more_off();
			return;
		}
	}
	
	/*  else the defect parameter is a tid, search a deadlock
	    starting from this thread */

	if ((err = kdb_expr(16, buff, buffix, linelen, &given_tid)) == 0) {
		threadp = dla_get_thread(given_tid);
		if (threadp != NULL) {
			/* given_addr is correct in thread table */
			if (!dla_Dlock(threadp, o_thread))
				kdb_printf(" No waited lock for thread %x\n", threadp);
		} else {
			kdb_printf("Cannot access this thread\n");
		}
		kdb_more_off();
		return;
	}
	kdb_printf("Bad argument for deadlock analyzer\n");
	kdb_more_off();
}

#endif /* _CRASH */


/*
  -----------------------                            
 *        Deadlock search function
 *
 *  From the parameters, define the search start point ( a cpu or a thread pointer ),
 *  follow the blocking locks and for each, register it in a
 *  descriptor. If this lock has already been seen, a deadlock is
 *  found. The function will then print the list of lock descriptors if not empty.
 *
 *  Algo: from start thread or from start cpu ,
 *      list_of_depending_locks = NULL;
 *      while(waited_lock = dla_waiting(cur_th_or_ithandler))
 *      	if there is a real lock
 *     		cur_th_or_ithandler = dla_owner(waited_lock);
 *      	else 
 *      		insert in list_of_depending_locks a new lock descriptor
 *      		with cur_th_or_ithandler and waited_lock;
 *      		No deadlock is found;
 *      		break;
 *
 *      	check cur_th_or_ithandler: if a correct thread or cpu
 *
 *      		if (cur_th_or_ithandler already in list_of_depending_locks)
 *      			a deadlock is found;
 *      			suppress the uncycled part of list_of_depending_locks;
 *      			break;
 *      		else
 *      			insert in list_of_depending_locks a new lock descriptor 
 *      			with cur_th_or_ithandler and waited_lock;
 *
 *      print formatted list_of_depending_locks;
 *      return if a deadlock is found.
 *
 *
 *  There are adds to this basic algorithm regarding automatic
 *  research and waited events that are not locks, but see the code.
 *  ----------------------- 
 */
/*
 * NAME: dla_Dlock
 *
 * FUNCTION: Search a deadlock from a thread pointer or a cpu_id
 *           If start_type is o_it_handler, thp_or_cpuid is a cpu id,
 *           else                    thp_or_cpuid is a thread pointer.
 *         
 * RETURNS: returns true if a deadlock found, or false else.
 */

static int
dla_Dlock(int thp_or_cpuid, owner_t start_type)
{
	owner_t o_type=start_type;
	struct lock_desc *ll = NULL, *new_ll=0;
	struct mstsave *w_csa=0;
	int i, o_id,  found = 0, lockval;  /* had a deadlock */
	ulong waited_lock=0;
	lock_type_t l_type;
	struct complex_lock_data wk_waited_lock;
	struct thread wk_thread;
	
	o_id = thp_or_cpuid;
	if (dla_stack_debug) 
		dla_printf("dla_Dlock %x\n", thp_or_cpuid);
	/* follow the lock cycle with ll = NULL as start point */
	while ((waited_lock = dla_waiting(o_id,o_type,&w_csa,&l_type))!=NULL) { 
		if (o_type == o_thread) {
			if (dla_marked(o_id, dla_mark_dl)) { /* this deadlock is already printed */
				ll = NULL;
				break;
			}
			dla_mark(o_id, dla_mark_th);
		}
		if (l_type != l_other)
			o_id = dla_owner(waited_lock,&o_type,l_type);
		/* o_id is NULL if not found in memory */
		if ((l_type == l_other) || /* An event waited but not a lock */
		    ((o_type == o_thread) && (o_id == NULL)) || /* no return from dla_owner */
		    ((o_type == o_it_handler) &&
		     ((o_id < 0) || (o_id >= ncpus)))) { /* wrong cpu return */
			if (dla_stack_debug) 
				dla_printf("owner type %d value %d\n",o_type, o_id);
			
			if ((new_ll = dla_new_desc()) != NULL) {
				if (dla_stack_debug) 
					dla_printf("new_ll %x\n", new_ll);
				new_ll->lock = waited_lock;
				new_ll->lock_type = l_type;
				new_ll->status = NOT_IN_MEMORY;
				new_ll->owner_type = o_none;
				new_ll->owner_id = NULL;   
				new_ll->waiting_csa = w_csa;
				new_ll->slot = curslot;
				new_ll->next = ll;
				ll = new_ll;
				found = FALSE; /* will only print a broken chain */
				break; 
			}
			else { /* No more descriptors in pool */
				found = FALSE;
				break;
			}
		}
		if ((new_ll = dla_alr_owner(o_id, o_type, ll)) != NULL) {
			/* Search the first occurence in the list */
			found = TRUE; /* will print a cycle */
			new_ll->next = NULL; /* forget the end of list */
			break;
		}
		/* add a lock desc to ll */
		if ((new_ll = dla_new_desc()) != NULL) {
			if (dla_stack_debug) 
				dla_printf("new_ll %x\n", new_ll);
			new_ll->lock = waited_lock;
			new_ll->lock_type = l_type;
			if (l_type == l_complex) 
				new_ll->status = -1;
			else
				new_ll->status = LOCKBIT;
			if (dla_readvalue(wk_waited_lock,waited_lock)){
				dla_readvalue(lockval,waited_lock);
				if (lockval & INSTR_ON) /* follow the pointer of the structure */
					dla_readvalue(wk_waited_lock, lockval);
				new_ll->status = wk_waited_lock.status;
			}
			new_ll->owner_type = o_type;
			new_ll->owner_id = o_id;   
			new_ll->waiting_csa = w_csa;
			new_ll->slot = curslot;
			new_ll->next = ll;
			ll = new_ll;
		}
		else { /* No more descriptors in pool */
			found = FALSE;
			break;
		}
	}
	
	/* the search from the specified start time is completed, print the result */ 
	
	if (ll != NULL)
		if ((ll->lock_type != l_other) ||
			(ll->next != NULL)){/* Contrary case is a waited event blocking no other thread */
			if (!dla_stack_debug) {
				for(i=0; i < 80; i++)
					dla_printf(" ");
				dla_printf("\r");
			}
			dla_print_locks ( ll, found, thp_or_cpuid, start_type);
			return found;
		}
	if (dla_stack_debug) {
		if (start_type == o_it_handler)
			dla_printf("No waited lock on processor %x \n", thp_or_cpuid);
		else
			dla_printf("No waited lock for thread %x \n", thp_or_cpuid);
	}
	return FALSE;
}
/* 
 ------------------------                            
 *        dla_owner functions
 *    Knowing the type of the lock and its address, just have to 
 *    read the lock control word to know its owner. If the
 *    instrumentation is on, the first thing to do is a dereference.
 *
 *    See sys/lock_def.h for lock structures.
 *  ----------------------- 
 */
/*
 * NAME: dla_owner(waited_lock, o_type, l_type)
 *
 * FUNCTION: finds the owner of a lock, ( a thread or a handler ), reading
 *           the lock structure. 
 *
 * RETURNS: NULL if not found, a pointer the the owning thread or a cpu_id
 *          and updates the type of the owner in o_type (thread or handler).
 */

static int
dla_owner(ulong waited_lock, owner_t *o_type , lock_type_t l_type)
{
	int retval, instr_on;
	
	retval = 0;
	*o_type = o_thread; /* important default return value */
	if (dla_stack_debug)
		dla_printf("dla_owner %x %d %d\n",
			   waited_lock,(int)*o_type,(int)l_type);
	
        /* dereference if an instrumented lock */

	if (!dla_readvalue(instr_on, waited_lock))
		return NULL;
	instr_on &= INSTR_ON;
	
	if (instr_on != 0) /* follow the pointer of the structure */
		if (!dla_readvalue(waited_lock, waited_lock))
			return NULL;
	
	switch (l_type) {
	case l_simple: /* o_type will be changed if a it_handler is owner */
		retval =  dla_owner_simple( waited_lock, o_type );
		break;
	case l_aix3_2:
		retval = dla_owner_lockl( waited_lock);
		break;
	case l_complex:
		retval = dla_owner_complex( waited_lock);
		break;
	default:
		retval = NULL;
	}
	if ((*o_type == o_thread) &&(retval != NULL))
		/* get the thread associated to tid */
		retval = dla_get_thread(retval);
	return retval;
}

/*
 * NAME: dla_owner_simple(waited_lock, o_type)
 *
 * FUNCTION: finds the owner of a simple lock, a thread or a handler. 
 *
 * RETURNS: NULL if not found, a pointer the the owning thread or a cpu_id
 *          and updates the type of the owner in o_type (thread or handler).
 */


static int 
dla_owner_simple(ulong waited_lock, owner_t *o_type)
{
	simple_lock_data wk_lock;
	struct ppda *ppda_addr, wk_ppda;
	int i;
	
	if (dla_stack_debug) 
		dla_printf("dla_owner_simple %x %d\n",waited_lock,(int)o_type);
	
	if (!dla_readvalue(wk_lock, waited_lock))
		return NULL;
	if ( wk_lock & THREAD_BIT)
		*o_type = o_thread;
	else {
		*o_type = o_it_handler;
		/* the owner is a shifted cpu_id, has to transfer to a cpu_num */
		ppda_addr = dla_symaddr("ppda");
		for (i = 0; i < ncpus; i++)
			if (dla_readvalue(wk_ppda, ppda_addr+i))
				if (wk_ppda.cpuid == ((wk_lock & OWNER_MASK) >> 1))
					return i;
		return 0;
	}
	return (wk_lock & OWNER_MASK);
}

static int
dla_owner_lockl(ulong waited_lock)
{
	lock_t wk_lock;
	
	if (dla_stack_debug)
		dla_printf("dla_owner_lockl %x\n",waited_lock);
	if (!dla_readvalue(wk_lock, waited_lock))
		return NULL;
	return (wk_lock & LOCKL_OWNER_MASK);
}

static int
dla_owner_complex(ulong waited_lock)
{
	simple_lock_data wk_lock;
	
	if (dla_stack_debug) 
		dla_printf("dla_owner_complex %x\n",waited_lock);
	if (!dla_readvalue(wk_lock, waited_lock))
		return NULL;
	return (wk_lock & OWNER_MASK);
}

/*
 * NAME: dla_alr_owner(o_id, o_type, ll)
 *
 * FUNCTION: detects if a owner is already in lock list.
 *
 * RETURNS: NULL if not found, a pointer to the descriptor else.
 */

static struct lock_desc *
dla_alr_owner(int o_id, owner_t o_type, struct lock_desc *ll)
{
	struct lock_desc *ll1;
	
	if (dla_stack_debug)
		dla_printf("dla_alr_owner %x %d %x\n",o_id,(int)o_type,(int)ll);
	for(ll1 = ll; ll1!= NULL; ll1 = ll1->next)
		if ((ll1->owner_id == o_id) && (ll1->owner_type == o_type))
			return ll1;
	return NULL;
}

/*
 * NAME: dla_get_thread(tid)
 *
 * FUNCTION: returns the thread pointer associated to a tid.
 *
 * RETURNS: NULL if not found, a pointer to the thread structure with tid.
 */

static int
dla_get_thread(int tid)
{
	struct thread *thread_addr, wk_thread;
	int n_th;
	
	thread_addr = 0;
	if (dla_stack_debug)
		dla_printf("dla_get_thread %x\n",tid);
#ifdef _CRASH
	for (n_th=0; n_th < N_THREAD; n_th++) {
		if (read_thread(n_th,&wk_thread) != sizeof (struct thread))
			continue;
		if (wk_thread.t_tid == tid) {
			thread_addr = dla_thread_addr + n_th;
			break;
		}
	}
	return ((int) thread_addr);
#else /* _CRASH */
	return (kdb_thread_addr + kdb_threadindex(tid));
#endif /* _CRASH */
}


/*
 * NAME: set_curslot(o_id, o_type)
 *
 * FUNCTION: set curslot to the current thread slot value.
 *           essential for accessing addresses thru a dump.
 *           IMPORTANT. The context of a handler is the one
 *           of the interrupted thread.
 *
 * RETURNS: Nothing.
 */

static void 
set_curslot(int o_id, owner_t o_type)
{
	struct ppda *ppda_addr, wk_ppda;
	struct thread *thp;
	if (o_type == o_it_handler) {
		ppda_addr = dla_symaddr("ppda");
		if (!dla_readvalue(wk_ppda, ppda_addr + o_id)) {
#ifdef _CRASH
			dla_printf(catgets(scmc_catd, MS_crsh, DLA_MSG_15,
				"0452-1065: Cannot set curslot, no curthread on cpu %d\n"), o_id);
#else /* _CRASH */
			dla_printf("cannot set curslot, no curthread on cpu %d\n",o_id );
#endif /* _CRASH */
			curslot = 0;
			return;
		}
		thp = wk_ppda._curthread;
	}
	else
		thp = o_id;
	
	curslot = ((int)thp - (int)dla_thread_addr) / sizeof(struct thread);
}

/* 
 ------------------------                            
 *   list and print functions
 *  ----------------------- 
 */

/*
 * NAME: dla_print_locks
 *
 * FUNCTION: print a list of lock descriptors
 *
 * RETURNS: Nothing.
 */

static int
dla_print_locks(struct lock_desc *ll, int dl, int thp_or_cpuid, int start_type)
{
	struct thread wk_thread;
	
	if (start_type == o_thread)
#ifdef _CRASH
		read_thread(((thp_or_cpuid - (long)dla_thread_addr)
					 /(sizeof (struct thread))),&wk_thread) == sizeof (struct thread);
#else /* _CRASH */
		dla_readvalue(wk_thread, thp_or_cpuid);  /* print the tid instead of ptr */
#endif /* _CRASH */
	if (dl == TRUE) {
		if (start_type == o_it_handler)
			dla_printf(catgets(scmc_catd, MS_crsh, DLA_MSG_8, 
					   "Deadlock from cpu %d. This cpu is waiting for the first line lock,\n"), thp_or_cpuid);
		else
			dla_printf(catgets(scmc_catd, MS_crsh, DLA_MSG_9, 
					   "Deadlock from tid %05x. This tid waits for the first line lock,\n"), wk_thread.t_tid);
	} else {
		if (start_type == o_it_handler)
			dla_printf(catgets(scmc_catd, MS_crsh, DLA_MSG_10, 
					   "No deadlock, but chain from cpu %d, that waits for the first line lock,\n"), thp_or_cpuid);
		else
			dla_printf(catgets(scmc_catd, MS_crsh, DLA_MSG_11, 
					   "No deadlock, but chain from tid %05x, that waits for the first line lock,\n"), wk_thread.t_tid);
	}
	dla_printf(catgets(scmc_catd, 
			   MS_crsh, DLA_MSG_12, 
			   "owned by Owner-Id that waits for the next line lock, and so on ...\n"));
	dla_printf(catgets(scmc_catd, 
			   MS_crsh, DLA_MSG_13, 
			   "     LOCK NAME    |   ADDRESS  |  OWNER-ID | LOCK STATUS | WAITING FUNCTION \n"));
	dla_print_desc(ll, dl);
}

/*
 * NAME: dla_print_desc
 *
 * FUNCTION: print recursively a list of lock descriptors ( to reverse the order of the list )
 *
 * RETURNS: Nothing.
 */

static int
dla_print_desc(struct lock_desc *ll, int dl)
{
	struct syment *sym_tbl;
	char *str_tbl;
	int diff, save_curslot;
	char name[18], status[5];
	struct thread wk_thread;
	
	if (ll == NULL)
		return NULL;
	dla_print_desc(ll->next,dl);
#ifdef _CRASH
	if ((ll->lock < dla_symaddr("endcomm")) 
		&& ((diff = dla_findsym(ll->lock, &sym_tbl, &str_tbl)) != -1)) {
		dla_strncpy(name, str_tbl, 17);
		name[17] = '\0';
	} else {
		dla_strcpy(name, "? ? ?");
		diff = 0xffff;
	}
#else /* _CRASH */
	if ((diff = kdb_dla_findsym(ll->lock, &sym_tbl, &str_tbl)) != -1) {
		name[8] = '\0';
		if (sym_tbl->n_zeroes)
			kdb_strncpy(name, sym_tbl->n_name, 8);
		else
			kdb_strncpy(name, (char *)(str_tbl + sym_tbl->n_offset),17);
		name[17] = '\0';
	} else {
		kdb_strcpy(name, "? ? ?");
		diff = 0xffff;
	}
#endif /* _CRASH */
	
	if (ll->lock_type != l_other) {
		if (ll->owner_type == o_it_handler) 
			dla_strcpy(status, "Cpu");
		else {
			if ( dl == TRUE)
				dla_mark(ll->owner_id, dla_mark_dl);
#ifdef _CRASH
			if (read_thread((((long)ll->owner_id - (long)dla_thread_addr)
					 /(sizeof (struct thread))),&wk_thread) == sizeof (struct thread))
#else /* _CRASH */
			if (dla_readvalue(wk_thread, ll->owner_id))  /* print the tid instead of ptr */
#endif /* _CRASH */
				ll->owner_id = wk_thread.t_tid;
			dla_strcpy(status, "Tid");
		}
	} else
		dla_strcpy(status, "   ");
	
	if ((diff != 0) && (diff < 0x1000)) {
		name[12] = '\0'; /* Truncate to fit the display column */
		if (ll->lock_type != l_other)
			dla_printf("%12s+%04x | 0x%08x | %3s %5x |  0x%08x | ",
				   name,diff,ll->lock,status,ll->owner_id,ll->status);
		else
			dla_printf("%12s+%04x | 0x%08x | %3s %5s |  %10s | ",
				   name,diff,ll->lock,status," ","Event");
	} else
		if (diff == 0)
			if (ll->lock_type != l_other)
				dla_printf("%17s | 0x%08x | %3s %5x |  0x%08x | ",
					   name,ll->lock,status,ll->owner_id,ll->status);
			else
				dla_printf("%17s | 0x%08x | %3s %5s |  %10s | ",
					   name,ll->lock,status," ","Event");
		else
			if (ll->lock_type != l_other)
				dla_printf("%17s | 0x%08x | %3s %5x |  0x%08x | ",
					   " ? ? ? ",ll->lock,status,ll->owner_id,ll->status);
			else
				dla_printf("%17s | 0x%08x | %3s %5x |  0x%08x | ",
					   " ? ? ? ",ll->lock,status," ","Event");
#ifdef _CRASH
	save_curslot = curslot;
	curslot = ll->slot;
	dla_print_stack(ll->waiting_csa, 20);
	curslot = save_curslot ;
#else /* _CRASH */
	kdb_print_stack_trace(kdb_thread_addr + ll->slot);
#endif /* _CRASH */
}
/* 
 ------------------------                            
 *   waiting functions
 *  ----------------------- 
 */

/*
 * NAME: dla_waiting()
 *
 * FUNCTION: searches if a thread is waiting for a lock.
 *
 * RETURNS: NULL if no waited lock, the address of a lock, 
 *          a pointer to the waiting context,
 *          and the type of the waited_lock else.
 */


/* 
 *       The best way to implement this function is to analyze in sequence the
 *       two types of locks, regarding to DCR 52995.
 *       This order is important, because we have to follow handlers locks
 *       before thread locks, and handler locks will always be spinning locks.
 *
 *       - spinning locks :
 *       The only solution is to find in the stack the context of the
 *       concerned thread or it-handler, and the lock interface call
 *	for which parameter will be the lock address the thread is 
 *	waiting for. In fact, the first spinning mst will be the 
 *	spinning function : simple_lock with GPR3 = lock address.
 *
 *       simple_lock is optimized and even if GPR3 is volatile, there won't
 *       be any call, because spinning, and GPR3 is saved.
 *
 *       While no spinning lock, try to get back to the interrupted thread/handler
 *       and replay until no csa->prev;
 *
 *       - If no spinning lock, search for sleeping locks with
 *      dla_get_sleeping_lock().
 */

static ulong
dla_waiting(int o_id, owner_t o_type, struct mstsave **w_csa, lock_type_t *l_type)
{
	struct thread *thread_addr, wk_thread;
	ulong lock_addr;
	int nth, cpu_id = o_id;
	
	if (dla_stack_debug) 
		dla_printf("dla_waiting %d %d %x %x\n",
			   o_id,(int)o_type,(int)w_csa,(int)l_type);
	set_curslot(o_id, o_type);
	if (o_type == o_thread) {

                /* try to know if the thread is running on a cpu.
                   In this case, it can be blocked by a spinning it-handler.
                   We need to inspect first if the cpu is spinning or no.
                   If yes, no need to go forward, the blocking lock is the
                   one of the last spinning it-handler on this cpu */

		cpu_id = -1;
#ifdef _CRASH
		if (!read_thread(((o_id - (long)dla_thread_addr)
					 /(sizeof (struct thread))),&wk_thread) == sizeof (struct thread))
#else /* _CRASH */
		if (!dla_readvalue(wk_thread, o_id))  
#endif /* _CRASH */
			return NULL;
		if ((wk_thread.t_state == TSZOMB)||
			(wk_thread.t_state == TSIDL)||
			(wk_thread.t_state == TSNONE))
			return NULL;
		if (wk_thread.t_state == TSRUN)
			cpu_id = cpu_running_thread((struct thread *)o_id);
	}
	if (cpu_id != -1)
		if ((lock_addr = dla_get_spinning_lock(cpu_id, w_csa, l_type)) != NULL) {
			if (dla_stack_debug)
				dla_printf("dla_waiting return value %x", lock_addr);
			return lock_addr;
		}
	if (o_type == o_it_handler)
		return NULL;
	/* The lock must be a sleeping lock */

	if ((wk_thread.t_state == TSRUN) ||
		(wk_thread.t_state == TSSLEEP) ||
		(wk_thread.t_state == TSSWAP) ||
		(wk_thread.t_state == TSSTOP)) 
		return (dla_get_sleeping_lock(&wk_thread, w_csa, l_type));
	else
		return NULL;
}

/*
 * NAME: cpu_running_thread()
 *
 * FUNCTION: searches the cpu running the thread.
 *
 * RETURNS: -1 if not found, the index of cpu else.
 */

static int
cpu_running_thread(struct thread *thp)
{
	struct ppda *ppda_addr, wk_ppda;
	int nb_cpu;
	
	if (dla_stack_debug) 
		dla_printf("cpu_running_thread %x\n", thp);
	if (dla_stack_debug)
		dla_printf("number of cpus %d\n", ncpus);
	ppda_addr = dla_symaddr("ppda");
	if (dla_stack_debug) 
		dla_printf("ppda[] %x\n",ppda_addr);
	for (nb_cpu = 0; nb_cpu < ncpus; nb_cpu++, ppda_addr++) {
		if (dla_stack_debug) 
			dla_printf("ppda[%d] %x\n", nb_cpu, ppda_addr);
		if (!dla_readvalue(wk_ppda, ppda_addr))
			continue;
		if (dla_stack_debug)
			dla_printf("curthread[ppda[%d] %x\n",
				   nb_cpu, wk_ppda._curthread );
		if (wk_ppda._curthread == thp)
			return nb_cpu;
	}
	return -1;
} 

/*
 * NAME: dla_get_sleeping_lock()
 *
 * FUNCTION: searches if a thread is sleeping for a lock.
 *
 * RETURNS: NULL if no waited lock, the address of a lock,
 *          a pointer to the waiting context
 *          and the type of the waited_lock else.
 */

/*
 *       This function scopes on swchan of the concerned thread. If not NULL
 *	this is the address of the blocking lock. Then, find in the
 *	stack the call interface that addressed this lock to decide
 *	the type of this lock. This is important to know how to read the lock
 *	word to decide who is the owner of this blocking lock.
 *
 *	The thread can be blocked by something else than a lock. This
 *	means wchan is not NULL. In this case, the analyze breaks with
 *	a last descriptor with lock_type = l_other. At print time, we
 *	we will decide if there is a good reason to print this special
 *	descriptor.
 *	The criteria to take into account is : if this different type event
 *	blocks some locks, there is a chain to print. Else, there is 
 *	no need to print.
 *	
 */

static ulong
dla_get_sleeping_lock(struct thread *thp, struct mstsave **w_csa, lock_type_t *l_type)
{
	if (dla_stack_debug)
		dla_printf("dla_get_sleeping_lock %d %x %x\n",
			   thp, w_csa, l_type);
	if (thp->t_wchan1 != 0) { /* waiting for a lock */
		*w_csa = (struct mstsave *) thp->t_uaddress.uthreadp;
		*l_type = dla_lock_type(*w_csa);
		return ((ulong)thp->t_wchan1);
	}
	if (thp->t_swchan != 0) { /* waiting for a lock */
		*w_csa = (struct mstsave *) thp->t_uaddress.uthreadp;
		*l_type = dla_lock_type(*w_csa);
		return ((ulong)thp->t_swchan);
	}
	if (thp->t_wchan != 0) { /* waiting for an event */
		*w_csa = (struct mstsave *) thp->t_uaddress.uthreadp;
		*l_type = l_other;
		return ((ulong)thp->t_wchan);
	}
	return NULL;
}

/*
 * NAME: dla_get_spinning_lock()
 *
 * FUNCTION: searches if a thread or a it-handler is spinning for a lock.
 *
 * RETURNS: NULL if no waited lock, the address of a lock, a pointer to the waiting context
 *                                  and the type of the waited_lock else.
 */

static ulong
dla_get_spinning_lock(int cpu_id, struct mstsave **w_csa, lock_type_t *l_type)
{
	struct ppda *ppda_addr, wk_ppda;
	ulong l = NULL;
	
	struct mstsave *cur_mst_p, cur_mst;
	if (dla_stack_debug) 
		dla_printf("dla_get_spin_lock %d %x %x\n",cpu_id,w_csa,l_type);
	ppda_addr = dla_symaddr("ppda");
	if (!dla_readvalue(wk_ppda, ppda_addr + cpu_id))
		return NULL;
	if (!dla_readvalue(cur_mst, wk_ppda._csa))
		return NULL;
	cur_mst_p = cur_mst.prev; 
	if (!dla_readvalue(cur_mst, cur_mst_p))
		return NULL;
	if ((l = dla_spinning_ctxt(cur_mst_p, w_csa, l_type)) != NULL) {
		if (dla_stack_debug)
			dla_printf("dla_get_spin_lock return value %x\n", l);
		return l;
	}
	return l;
}
/* 
 ------------------------                            
 *   stack analyze functions
 *  ----------------------- 
 */

/*
 * NAME: dla_spinning_ctxt()
 *
 * FUNCTION: analyze the stack of a context to determine if spinning.
 *
 * RETURNS the address of the lock, the spinning mst address and the type
 * of the lock, depending on the lock function spinning and the LOCK BIT.
 */

static ulong
dla_spinning_ctxt(struct mstsave *mst, struct mstsave **w_csa, lock_type_t *l_type)
{
	struct mstsave *cur_mst = mst, wk_csa;
	char name[DUMMY_SIZE];
	struct syment *sym_tbl;
	char *str_tbl;
	ulong adr_tb, adr;

#ifdef _CRASH
	if (dla_stack_debug) 
		dla_printf("dla_get_spinning_ctxt %x %x %x\n",
			   cur_mst, w_csa, l_type);
		while (cur_mst) {
			*w_csa = cur_mst;
			if (!dla_readvalue(wk_csa, *w_csa))
				return NULL;
			adr = wk_csa.iar;
			if (dla_stack_debug)
				dla_printf("adr %x r0 %x r1 %x r2 %x r3 %x r4 %x r5 %x\n",
					   adr, wk_csa.gpr[0],wk_csa.gpr[1],wk_csa.gpr[2],wk_csa.gpr[3],
					   wk_csa.gpr[4],wk_csa.gpr[5]  );
			if ((adr_tb = adr) == 0)	/* safety */
				break;
			name[0] = '\0';
			if (dla_stack_debug) 
				dla_printf("look for function name in symbol table\n");
			if (dla_findsym(adr, &sym_tbl, &str_tbl) != -1) {
				name[8] = '\0';
				if (sym_tbl->n_zeroes)
					dla_strncpy(name, sym_tbl->n_name, 8);
				else
					dla_strncpy(name, str_tbl, DUMMY_SIZE);
			}
		if (dla_stack_debug) dla_printf("name = %s--\n", name);
		if (dla_is_simple(name)) {
			int lockval;
			*l_type = l_simple;
			if (dla_is_disable(name)){
				if (! dla_readvalue(lockval,wk_csa.gpr[4]))
						return (NULL);
				if (lockval & INSTR_ON){
					if (! dla_readvalue(lockval,lockval))
						return (NULL);
				}
				if (lockval & LOCKBIT)
						return ((ulong)wk_csa.gpr[4]);
				return (NULL);
			}
			if (! dla_readvalue(lockval,wk_csa.gpr[3]))
						return (NULL);
			if (lockval & INSTR_ON){
				if (! dla_readvalue(lockval,lockval))
					return (NULL);
			}
			if (lockval & LOCKBIT)
					return ((ulong)wk_csa.gpr[3]);
				
		}
		if (dla_is_complex(name)) {
			*l_type = l_complex;
			return ((ulong)wk_csa.gpr[3]);
		}
		if (dla_is_aix3_2(name)) {
			*l_type = l_aix3_2;
			return ((ulong)wk_csa.gpr[3]);
		}
		cur_mst = wk_csa.prev;
	}
	return NULL;
#else /* _CRASH */
	ulong kdb_virttophys();
	ulong pt;
	int nb_p, ret;
	unsigned int tb_off;
	struct tbtable_short tbtable_short;
	/*
	 * this is NOT a regular structure. Be carefull about this.
	 * struct tbtable_ext tbtable_ext;
	 */
	char tbtable_ext[DUMMY_SIZE];
	struct tbtable_short *tbsh;
	struct thread *th_p_save;
	struct proc *p_save;
	struct mstsave *mst_save;
	
	ret = NULL;
	th_p_save = kdb_cur_thread;
	p_save = kdb_cur_proc;
	mst_save = kdb_cur_mst;

	if (kdb_virttophys(kdb_thread_addr + curslot) == -1)
		return NULL;
	kdb_readvalue(kdb_wk_thread, kdb_thread_addr + curslot);
	if (kdb_wk_thread.t_state == TSNONE || kdb_wk_thread.t_state == TSZOMB)
		return NULL;
	if (kdb_virttophys(kdb_wk_thread.t_procp) == -1)
		return NULL;
	kdb_readvalue(kdb_wk_proc, kdb_wk_thread.t_procp);
	if ((kdb_wk_proc.p_flag & SLOAD) == 0)
		return NULL;
	/*
	 * init new proc and new mstsave area
	 */
	kdb_cur_thread = &kdb_wk_thread;
	kdb_cur_proc = &kdb_wk_proc;
	while (cur_mst) {
		*w_csa = cur_mst;
		if (kdb_virttophys(*w_csa) == -1) {
			kdb_cur_proc = p_save;
			kdb_cur_thread = th_p_save;
			return NULL;
		}
		kdb_readvalue(kdb_wk_mst, *w_csa);
		kdb_cur_mst = &kdb_wk_mst;
		adr = kdb_wk_mst.iar;
		if ((adr_tb = adr) == 0)	/* safety */
			break;
		name[0] = '\0';
		while (kdb_get_word(adr_tb) != 0)
			adr_tb += 4;
		adr_tb += 4;
		kdb_readvalue(tbtable_short, adr_tb);
		tbsh = &tbtable_short;
		nb_p = (int)tbsh->fixedparms + (int)tbsh->floatparms;
		if (nb_p>16) nb_p = 16; /* limit if no traceback (.s)*/
		kdb_readvalue(tbtable_ext, &(((struct tbtable*)adr_tb)->tb_ext));
		pt = (ulong)&tbtable_ext[0];
		if (nb_p != 0) {
			pt += 4;
		}
		if (tbsh->has_tboff) {
			tb_off = *(unsigned int *)pt + 4;
			pt += 4;
		} else
			tb_off = 0;
		if (tbsh->int_hndl)
			pt += 4;
		if (tbsh->has_ctl) {
			unsigned int nb;
			nb = *(unsigned int *)pt;
			pt += 4;
			pt += 4*nb;
		}
		if ((int)(adr - adr_tb + tb_off) >= 0) {
			if (tbsh->name_present) {
				short len;
				len = *(short *)pt;
				if (len > DUMMY_SIZE -1)
					len = DUMMY_SIZE -1;
				pt = (ulong)((short *)pt + 1);
				kdb_strncpy(name, pt, len);
				name[len] = '\0';
			}
		} else {
			if (kdb_dla_findsym(adr, &sym_tbl, &str_tbl) != -1) {
				name[8] = '\0';
				if (sym_tbl->n_zeroes)
					kdb_strncpy(name, sym_tbl->n_name, 8);
				else
					kdb_strncpy(name, (char *)(str_tbl + sym_tbl->n_offset), DUMMY_SIZE);
			}
		}
		if (dla_is_simple(name)) {
			*l_type = l_simple;
			if (dla_is_disable(name))
				ret = kdb_wk_csa.gpr[7]);
			ret = kdb_wk_mst.gpr[3];
			goto out;
		}
		if (dla_is_complex(name)) {
			*l_type = l_complex;
			ret = kdb_wk_mst.gpr[3];
			goto out;
		}
		if (dla_is_aix3_2(name)) {
			*l_type = l_aix3_2;
			ret = kdb_wk_mst.gpr[3];
			goto out;
		}
		cur_mst = kdb_wk_mst.prev;
	}
 out:
	kdb_cur_mst = mst_save;
	kdb_cur_proc = p_save;
	kdb_cur_thread = th_p_save;
	return ((ulong)ret);
#endif /* _CRASH */
}

/* 
 * NAME: dla_lock_type()
 *
 * FUNCTION: searches which context is spinning for the lock.
 *
 * RETURNS: The lock type of the waited lock
 *                                  
 */

static lock_type_t
dla_lock_type(struct mstsave *w_csa)
{
	struct mstsave wk_csa;
	char name[DUMMY_SIZE];
	struct syment *sym_tbl;
	char *str_tbl;
	ulong adr;
#ifdef _CRASH
	if (dla_stack_debug) 
		dla_printf("dla_lock_type %x\n", w_csa);
	if (!dla_readvalue(wk_csa, w_csa))
		return l_other;
	adr = wk_csa.iar;
	if (adr == 0)	/* safety */
		return l_other;
	if (dla_stack_debug) 
		dla_printf("look for function name in symbol table\n");
	if (dla_findsym(adr, &sym_tbl, &str_tbl) != -1) {
		name[8] = '\0';
		if (sym_tbl->n_zeroes)
			dla_strncpy(name, sym_tbl->n_name, 8);
		else
			dla_strncpy(name, str_tbl, DUMMY_SIZE);
	}
	if (dla_stack_debug) dla_printf("name = %s\n", name);
	if (dla_is_simple(name)) {
		return l_simple;
	}
	if (dla_is_complex(name)) {
		return l_complex;
	}
	if (dla_is_aix3_2(name)) {
		return l_aix3_2;
	}
	return l_other;
#else /* _CRASH */
	ulong kdb_virttophys();
	ulong adr_tb, pt;
	int nb_p;
	unsigned int tb_off;
	struct tbtable_short tbtable_short;
	/*
	 * this is NOT a regular structure. Be carefull about this.
	 * struct tbtable_ext tbtable_ext;
	 */
	char tbtable_ext[DUMMY_SIZE];
	struct tbtable_short *tbsh;
	struct thread *th_p_save;
	struct proc *p_save;
	struct mstsave *mst_save;

	th_p_save = kdb_cur_thread;
	p_save = kdb_cur_proc;
	mst_save = kdb_cur_mst;

	if (kdb_virttophys(kdb_thread_addr + curslot) == -1)
		return l_other;
	kdb_readvalue(kdb_wk_thread, kdb_thread_addr + curslot);
	if (kdb_wk_thread.t_state == TSNONE || kdb_wk_thread.t_state == TSZOMB)
		return l_other;
	if (kdb_virttophys(kdb_wk_thread.t_procp) == -1)
		return l_other;
	kdb_readvalue(kdb_wk_proc, kdb_wk_thread.t_procp);
	if ((kdb_wk_proc.p_flag & SLOAD) == 0)
		return l_other;
	/*
	 * init new proc and new mstsave area
	 */
	kdb_cur_thread = &kdb_wk_thread;
	kdb_cur_proc = &kdb_wk_proc;
	if (kdb_virttophys(w_csa) == -1) {
		kdb_cur_proc = p_save;
		kdb_cur_thread = th_p_save;
		return l_other;
	}
	kdb_readvalue(kdb_wk_mst, w_csa);
	kdb_cur_mst = &kdb_wk_mst;
	
	adr = kdb_wk_mst.iar;
	if ((adr_tb = adr) == 0)	/* safety */
		return l_other;
	while (kdb_get_word(adr_tb) != 0)
		adr_tb += 4;
	adr_tb += 4;
	kdb_readvalue(tbtable_short, adr_tb);
	tbsh = &tbtable_short;
	nb_p = (int)tbsh->fixedparms + (int)tbsh->floatparms;
	if (nb_p>16) nb_p = 16; /* limit if no traceback (.s)*/
	kdb_readvalue(tbtable_ext, &(((struct tbtable*)adr_tb)->tb_ext));
	pt = (ulong)&tbtable_ext[0];
	if (nb_p != 0) {
		pt += 4;
	}
	if (tbsh->has_tboff) {
		tb_off = *(unsigned int *)pt + 4;
		pt += 4;
	} else
		tb_off = 0;
	if (tbsh->int_hndl)
		pt += 4;
	if (tbsh->has_ctl) {
		unsigned int nb;
		nb = *(unsigned int *)pt;
		pt += 4;
		pt += 4*nb;
	}
	if (((int)(adr - adr_tb + tb_off) >= 0) && (tbsh->name_present)) {
			short len;
			len = *(short *)pt;
			pt = (ulong)((short *)pt + 1);
			kdb_strncpy(name, pt, len);
			name[len] = '\0';
		} else { 
			kdb_strcpy(name, "? ? ?");
			if ((kdb_dla_findsym(adr, &sym_tbl, &str_tbl)) != -1) {	
				name[8] = '\0';
				if (sym_tbl->n_zeroes)
					kdb_strncpy(name, sym_tbl->n_name, 8);
				else
					kdb_strncpy(name, (char *)(str_tbl + sym_tbl->n_offset),DUMMY_SIZE);
			}
		}
	kdb_cur_mst = mst_save;
	kdb_cur_proc = p_save;
	kdb_cur_thread = th_p_save;

	if (dla_is_simple(name)) {
		return l_simple;
	}
	if (dla_is_complex(name)) {
		return l_complex;
	}
	if (dla_is_aix3_2(name)) {
		return l_aix3_2;
	}
	return l_other;
#endif /* _CRASH */			
}

#ifdef _CRASH
/*
 * NAME: dla_print_stack()
 *
 * FUNCTION: prints the stack(s) from w_stack until finished or nb_loop reached
 *
 * RETURNS: No return.
 */

static void
dla_print_stack(struct mstsave *w_csa, int nb_loop)
{
	int first_time;
	char name[256];
	char filler[80];
	ulong sp, adr, lr;
	struct mstsave *mst = w_csa, wk_mst;

	long diff;
	struct syment *sym_tbl;
	char *str_tbl;
	
	filler[0] = '\0';
	while (mst && (nb_loop>0)) {
		if (!dla_readvalue(wk_mst, mst)) {
			dla_printf(" Bad stack\n");
			return;
		}
		sp = wk_mst.gpr[1]; /* get sp (GPR1) from mst */
		first_time = 1;
		while (sp && (nb_loop>0)) {
			if (first_time) {
				adr = wk_mst.iar;
				first_time = 0;
			}
			if (adr == 0)	/* safety */
				break;
			if (dla_stack_debug)
				dla_printf("adr %x\n", adr);
			/* forget the case of being in a prologue as if in kernel mode : to long */
			if (dla_readvalue(sp,sp)) {
				if (dla_stack_debug)
					dla_printf("read sp %x\n", sp);
				if (!dla_readvalue(lr, (int)sp+2*4))
					sp = NULL; /* stop the trace in this mst */
			} else
				sp = NULL;
			if (dla_stack_debug)
				dla_printf("read lr %x\n", lr);
			if (dla_stack_debug) 
				dla_printf("look for function name in symbol table\n");
			name[0] = '\0';
			if ((diff = dla_findsym(adr, &sym_tbl, &str_tbl)) != -1) {
				name[8] = '\0';
				if (sym_tbl->n_zeroes)
					dla_strncpy(name, sym_tbl->n_name, 8);
				else
					dla_strncpy(name, str_tbl, DUMMY_SIZE);
			} else {
				sprintf(name, "%d", adr);
			}
			if (filler[0] == '\0') { /* The first function of the stack trace */
				if (name[0] != '\0')
					dla_printf ("%s\n", name);
				else
					dla_printf ("%08x\n", adr);
				dla_strcpy(filler, "                   called from : ");
			} else {
				if (name[0] != '\0')
					dla_printf ("%s%s + %08x\n", filler, name, diff);
				else
					dla_printf ("%s%08x\n", filler, adr);
			}	
			if (lr)	/* safety */
				lr -= 4; /* lr points to NEXT instruction */
			adr = lr;
			nb_loop--;
		}
		mst = wk_mst.prev;
		if (mst && (nb_loop>0)) {
			dla_printf("____ Exception (%.08x) ____\n", mst);
		}
	}
}
#endif /* _CRASH */			

#ifndef _CRASH

/* 
 ------------------------                            
 *   KDB memory/symbol interface functions
 *  ----------------------- 
 */


/*
 * dla_read_kdb -- check memory, then call kdb_read_mem
 */
static int
dla_read_kdb(ulong vad, unsigned char *buf, unsigned len)
{
	ulong paddr, kdb_virttophys();
	ulong caddr;
	uint count, resid;
	int ret = FALSE;
	
	
	struct thread *th_p_save;
	struct proc *p_save;
	struct mstsave *mst_save;
	
	if (kdb_is_iospace(vad))
		return 0;
	
	if (kdb_get_curthread() != kdb_thread_addr + curslot) {
		th_p_save = kdb_cur_thread;
		p_save = kdb_cur_proc;
		mst_save = kdb_cur_mst;
		
		if (kdb_virttophys(kdb_thread_addr + curslot) == -1)
			return (NULL);
		kdb_read_mem(kdb_thread_addr + curslot, &kdb_wk_thread, sizeof(kdb_wk_thread));
		if (kdb_wk_thread.t_state == TSNONE || kdb_wk_thread.t_state == TSZOMB)
			return NULL;
		if (kdb_virttophys(kdb_wk_thread.t_procp) == -1)
			return (NULL);
		kdb_read_mem(kdb_wk_thread.t_procp, &kdb_wk_proc, sizeof(kdb_wk_proc));
		if ((kdb_wk_proc.p_flag & SLOAD) == 0)
			return NULL;
		/*
		 * init new proc and new mstsave area
		 */
		kdb_cur_thread = &kdb_wk_thread;
		kdb_cur_proc = &kdb_wk_proc;
		if (kdb_virttophys(kdb_cur_thread->t_uaddress.uthreadp) == -1) {
			kdb_cur_proc = p_save;
			kdb_cur_thread = th_p_save;
			return (NULL);
		}
		kdb_read_mem(kdb_cur_thread->t_uaddress.uthreadp, &kdb_wk_mst, sizeof(kdb_wk_mst)); /* &u.u_save */
		kdb_cur_mst = &kdb_wk_mst;
	}

	resid = len;
	caddr = vad;
	count = ctob(btoc(caddr)) - caddr;
	if (count > resid)
		count = resid;
	if (count) {
		if ((paddr = kdb_virttophys(caddr)) == -1)
			goto out;
		caddr += count;
		resid -= count;
	}
	while (resid >= NBPC) {
		count = NBPC;
		if ((paddr = kdb_virttophys(caddr)) == -1)
			goto out;
		caddr += count;
		resid -= count;
	}
	count = resid;
	if (count) {
		if ((paddr = kdb_virttophys(caddr)) == -1)
			goto out;
		caddr += count;
		resid -= count;
	}
	kdb_read_mem(vad, buf, len);

	ret = TRUE;
 out:
	if (kdb_get_curthread() != kdb_thread_addr + curslot) {
		kdb_cur_mst = mst_save;
		kdb_cur_proc = p_save;
		kdb_cur_thread = th_p_save;
	}
	return ret;
}


static ulong
dla_symaddr(char *symbol)
{
	ulong value;
	kdb_symboladdr(value, symbol);
	return (value);
}

#else /* _CRASH*/

/* 
 ------------------------                            
 *   CRASH memory/symbol interface functions
 *  ----------------------- 
 */
/*
 * NAME: dla_read
 *
 * FUNCTION: memory/dump access thru a dump (taken by readmem_thread)
 *           returns in value the data at kern_addr with max size
 *
 * RETURNS: 0  if not in memory/dump
 */

static int
dla_read(ulong kern_addr, unsigned char *value, unsigned size)
{
	return readmem_thread(value, kern_addr, size, curslot);
}


/*
 * NAME: dla_findsym()
 *
 * FUNCTION: searches the corresponding symbol table entry to an address
 *
 * RETURNS: -1 if no symbol entry found.
 */

static long
dla_findsym(ulong val, struct syment **sym_tbl_p, char **str_tbl_p)
{
	if ((*sym_tbl_p = esearch(val, symbuf, 0)) == 0)
		return -1;
	if (*sym_tbl_p == -1)
		return -1;
	*str_tbl_p = symbuf;
	return (val - (*sym_tbl_p)->n_value );
}

/*
 * NAME: dla_symaddr
 *
 * FUNCTION: searches the corresponding address to a symbol name
 *
 * RETURNS: NOT_IN_MEMORY if not found.
 */

static ulong
dla_symaddr(char *symbol)
{
	extern struct nlist *Thread, *Ppd;
	struct syment *sym_tbl_p;
	ulong value;
	if (strcmp(symbol,"thread") == 0)
		return Thread->n_value;
	if (strcmp(symbol,"ppda") == 0)
		return Ppd->n_value;
	if ((sym_tbl_p = search(0, symbol, 1)) == 0)
		return NOT_IN_MEMORY;
	if (sym_tbl_p == -1)
		return NOT_IN_MEMORY;
	if ((sym_tbl_p->n_sclass == C_HIDEXT) && (sym_tbl_p->n_scnum == DATA)) {
				if (!dla_readvalue(value, sym_tbl_p->n_value))
					return NOT_IN_MEMORY;
				return value;
			}
	return sym_tbl_p->n_value;
}

#endif /* _CRASH */

static int
dla_is_disable(char *name)
{
	if ((dla_strcmp(name, "disable_lock") == 0) ||
	    (dla_strcmp(name, ".disable_lock") == 0) ||
	    (dla_strcmp(name, "disable_lock_ppc") == 0) ||
	    (dla_strcmp(name, ".disable_lock_ppc") == 0) ||
	    (dla_strcmp(name, "disable_lock_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".disable_lock_instr_ppc") == 0) ||
	    (dla_strcmp(name, "disable_lock_pwr") == 0) ||
	    (dla_strcmp(name, ".disable_lock_pwr") == 0))
		return 1;
	return 0;
}
	    
static int
dla_is_simple(char *name)
{
	if ((dla_strcmp(name, "simple_lock") == 0) ||
	    (dla_strcmp(name, ".simple_lock") == 0) ||
	    (dla_strcmp(name, "slock") == 0) ||
	    (dla_strcmp(name, ".slock") == 0) ||
	    (dla_strcmp(name, "disable_lock") == 0) ||
	    (dla_strcmp(name, ".disable_lock") == 0) ||
	    (dla_strcmp(name, "rsimple_lock") == 0) ||
	    (dla_strcmp(name, ".rsimple_lock") == 0) ||
	    
	    (dla_strcmp(name, "simple_lock_ppc") == 0) ||
	    (dla_strcmp(name, ".simple_lock_ppc") == 0) ||
	    (dla_strcmp(name, "disable_lock_ppc") == 0) ||
	    (dla_strcmp(name, ".disable_lock_ppc") == 0) ||
	    (dla_strcmp(name, "slock_ppc") == 0) ||
	    (dla_strcmp(name, ".slock_ppc") == 0) ||
	    (dla_strcmp(name, "rsimple_lock_ppc") == 0) ||
	    (dla_strcmp(name, ".rsimple_lock_ppc") == 0) ||
	    
	    (dla_strcmp(name, "simple_lock_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".simple_lock_instr_ppc") == 0) ||
	    (dla_strcmp(name, "slock_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".slock_instr_ppc") == 0) ||
	    (dla_strcmp(name, "disable_lock_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".disable_lock_instr_ppc") == 0) ||
	    (dla_strcmp(name, "rsimple_lock_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".rsimple_lock_instr_ppc") == 0) ||
	    
	    (dla_strcmp(name, "simple_lock_pwr") == 0) ||
	    (dla_strcmp(name, ".simple_lock_pwr") == 0) ||
	    (dla_strcmp(name, "disable_lock_pwr") == 0) ||
	    (dla_strcmp(name, ".disable_lock_pwr") == 0) ||
	    (dla_strcmp(name, "rsimple_lock_pwr") == 0) ||
	    (dla_strcmp(name, ".rsimple_lock_pwr") == 0) ||
	    (dla_strcmp(name, "slock_pwr") == 0) ||
	    (dla_strcmp(name, ".slock_pwr") == 0)) {
		return 1;
	}
	return 0;
}

static int
dla_is_complex(char *name)
{
	if ((dla_strcmp(name, "lock_write") == 0) ||
	    (dla_strcmp(name, ".lock_write") == 0) ||
	    (dla_strcmp(name, "complex_lock_sleep") == 0) ||
	    (dla_strcmp(name, ".complex_lock_sleep") == 0) ||
	    (dla_strcmp(name, "lock_read") == 0) ||
	    (dla_strcmp(name, ".lock_read") == 0) ||
	    (dla_strcmp(name, "lock_read_to_write") == 0) ||
	    (dla_strcmp(name, ".lock_read_to_write") == 0) ||
	    (dla_strcmp(name, "lock_write_to_read") == 0) ||
	    (dla_strcmp(name, ".lock_write_to_read") == 0) ||
	    
	    (dla_strcmp(name, "lock_write_ppc") == 0) ||
	    (dla_strcmp(name, ".lock_write_ppc") == 0) ||
	    (dla_strcmp(name, "complex_lock_sleep_ppc") == 0) ||
	    (dla_strcmp(name, ".complex_lock_sleep_ppc") == 0) ||
	    (dla_strcmp(name, "lock_read_ppc") == 0) ||
	    (dla_strcmp(name, ".lock_read_ppc") == 0) ||
	    (dla_strcmp(name, "lock_read_to_write_ppc") == 0) ||
	    (dla_strcmp(name, "lock_write_to_read_ppc") == 0) ||
	    (dla_strcmp(name, ".lock_write_to_read_ppc") == 0) ||
	    (dla_strcmp(name, ".lock_read_to_write_ppc") == 0) ||
	    
	    (dla_strcmp(name, "lock_read_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".lock_read_instr_ppc") == 0) ||
	    (dla_strcmp(name, "complex_lock_sleep_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".complex_lock_sleep_instr_ppc") == 0) ||
	    (dla_strcmp(name, "lock_write_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".lock_write_instr_ppc") == 0) ||
	    (dla_strcmp(name, "lock_read_to_write_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".lock_read_to_write_instr_ppc") == 0) ||
	    (dla_strcmp(name, "lock_write_to_read_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".lock_write_to_read_instr_ppc") == 0) ||
	    
	    (dla_strcmp(name, ".lock_write_pwr") == 0) ||
	    (dla_strcmp(name, ".lock_write_pwr") == 0) ||
	    (dla_strcmp(name, "complex_lock_sleep_pwr") == 0) ||
	    (dla_strcmp(name, ".complex_lock_sleep_pwr") == 0) ||
	    (dla_strcmp(name, "lock_read_pwr") == 0) ||
	    (dla_strcmp(name, ".lock_read_pwr") == 0) ||
	    (dla_strcmp(name, "lock_read_to_write_pwr") == 0) ||
	    (dla_strcmp(name, ".lock_read_to_write_pwr") == 0) ||
	    (dla_strcmp(name, "lock_write_to_read_pwr") == 0) ||
	    (dla_strcmp(name, ".lock_write_to_read_pwr") == 0)) {
		return 1;
	}
	return 0;
}

static int
dla_is_aix3_2(char *name)
{
	if ((dla_strcmp(name, "lockl") == 0) ||
	    (dla_strcmp(name, ".lockl") == 0) ||
		    
	    (dla_strcmp(name, "lockl_ppc") == 0) ||
	    (dla_strcmp(name, ".lockl_ppc") == 0) ||
		    
	    (dla_strcmp(name, "lockl_instr_ppc") == 0) ||
	    (dla_strcmp(name, ".lockl_instr_ppc") == 0) ||
		    
	    (dla_strcmp(name, "lockl_pwr") == 0) ||
	    (dla_strcmp(name, ".lockl_pwr") == 0)) {
		return 1;
	}
	return 0;
}
