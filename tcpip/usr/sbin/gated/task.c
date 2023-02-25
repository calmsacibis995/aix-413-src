static char sccsid[] = "@(#)77	1.10  src/tcpip/usr/sbin/gated/task.c, tcprouting, tcpip411, GOLD410 12/8/93 10:16:17";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: AUX_LIST
 *		AUX_LIST_END
 *		FD_CLR
 *		FD_ISSET
 *		FD_SET
 *		FD_ZERO
 *		MSGSTR
 *		SIGNAL_LIST
 *		TASK_SAVE
 *		_PROTOTYPE
 *		__PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF5
 *		dosrcpacket
 *		task_aux_register3511
 *		task_crash_add
 *		task_set_option
 *		task_signals_block
 *		task_signals_release
 *		timer_create
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  task.c,v 1.73.2.8 1993/08/27 22:28:50 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_WAIT
#define	INCLUDE_TIME
#define	INCLUDE_SIGNAL
#define	INCLUDE_IOCTL
#define	INCLUDE_FILE
#define	INCLUDE_FILE
#define	INCLUDE_MROUTE

#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */
#include "krt.h"
#ifdef	PROTO_BGP
#include "bgp.h"
#endif	/* PROTO_BGP */
#ifdef	PROTO_DVMRP
#include "dvmrp.h"
#endif	/* PROTO_DVMRP */
#ifdef	PROTO_EGP
#include "egp.h"
#endif	/* PROTO_EGP */
#ifdef	PROTO_OSPF
#include "ospf.h"
#endif	/* PROTO_OSPF */
#ifdef	PROTO_IDPR
#include "idpr.h"
#endif	/* PROTO_IDPR */
#ifdef	PROTO_RIP
#include "rip.h"
#endif	/* PROTO_RIP */
#ifdef	PROTO_HELLO
#include "hello.h"
#endif	/* PROTO_HELLO */
#ifdef	PROTO_ICMP
#include "icmp.h"
#endif	/* PROTO_ICMP */
#ifdef	PROTO_IGMP
#include "igmp.h"
#endif	/* PROTO_IGMP */
#ifdef	PROTO_ISODE_SNMP
#include "snmp_isode.h"
#endif	/* PROTO_ISODE_SNMP */
#ifdef	PROTO_ISIS
#include "isis_includes.h"
#endif	/* PROTO_ISIS */
#ifdef	PROTO_SLSP
#include "slsp.h"
#endif	/* PROTO_SLSP */
#include "parse.h"

/* For systems that do not have FD_SET macros */

#ifndef	FD_SET
#ifndef	NBBY
#define	NBBY	8			/* number of bits in a byte */
#endif	/* NBBY */
typedef long fd_mask;

#define	NFDBITS	(sizeof(fd_mask) * NBBY)/* bits per mask */

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define	FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))
#endif	/* FD_SET */

/* Signal names */
#include "task_sig.h"

#ifdef _POWER
/* Set up message catalog stuff */
#include "gated_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TASK,n,s)
/*
 * The following includes and declarations are for support of the System
 * Resource Controller (SRC) - .
 */
#include <spc.h>

#include "libffdb.h"
#include "libsrc.h"

static  struct srcreq srcpacket;
struct  statrep *srcstatus;
short   srcstatsize;
int     srcreplen;
int     cont = END;             /* for src... */
struct  srchdr *srchdr;
extern  int src_exists;
extern  char progname[];
int rc, addrsz;
struct sockaddr srcaddr;

#else
#define MSGSTR(n,s) s
#endif /* _POWER */

struct gtime timer_time = { 0 };	/* Current time of day */
struct gtime timer_time_start = { 0 };	/* Time gated started */

static timer *timer_active;		/* Pointer to the active timer */
static timer timer_queue_active =
{&timer_queue_active, &timer_queue_active, "activeTimers"};	/* Doubly linked list of active timers */
static timer timer_queue_inactive =
{&timer_queue_inactive, &timer_queue_inactive, "inactiveTimers"};	/* Doubly linked list of inactive timers */

static task task_head =
    {&task_head, &task_head, "taskHead"};	/* Head of doubly linked list of timers */
static fd_set task_select_readbits;
static fd_set task_select_writebits;
static fd_set task_select_exceptbits;
static int task_max_socket = 0;
static int task_n_tasks = 0;		/* Number of tasks */
char *task_path_start = 0;		/* Directory where we were started */
char *task_path_now = 0;		/* Directory where we are now */
char *task_config_file = 0;		/* Config file we read (and reread) */

static task task_task = {		/* Dummy task for the scheduler */
    (task *) 0,
    (task *) 0,
    "Scheduler"
};
task *task_active = &task_task;		/* Task pointer to the active task */

/*
 *	Signals
 */
#ifdef	BSD_SIGNALS
static int task_signal_mask;		/* Signals that are blocked */
static int task_signal_mask_save;	/* Signal mask at init */

#define	task_signals_block(name)	(void) sigblock(task_signal_mask)
#define	task_signals_release(name)	(void) sigsetmask(task_signal_mask_save);
#endif	/* BSD_SIGNALS */

#ifdef	POSIX_SIGNALS
static sigset_t task_signal_mask;	/* Signals that are blocked */

#define	task_signals_block(name) \
    if (sigprocmask(SIG_BLOCK, &task_signal_mask, (sigset_t *) 0) < 0) { \
	int error = errno; \
	trace(TR_ALL, LOG_ERR, "%s: sigprocmask(SIG_BLOCK): %m", name); \
	task_quit(error); \
    }
#define	task_signals_release(name) \
    if (sigprocmask(SIG_UNBLOCK, &task_signal_mask, (sigset_t *) 0) < 0) { \
	int error = errno; \
	trace(TR_ALL, LOG_ERR, "%s: sigprocmask(SIG_UNBLOCK): %m", name); \
	task_quit(error); \
    }
#endif	/* POSIX_SIGNALS */

#ifdef	SYSV_SIGNALS
#define	task_signals_block(name) \
    SIGNAL_LIST(ip) { \
	sighold(*ip); \
    } SIGNAL_LIST_END(ip)
#define	task_signals_release(name) \
	SIGNAL_LIST(ip) { \
	    sigrelse(*ip); \
	} SIGNAL_LIST_END(ip)
#endif	/* SYSV_SIGNALS */

static int task_signals[] = {
    SIGTERM,
    SIGALRM,
    SIGUSR1,
    SIGUSR2,
    SIGINT,
    SIGHUP,
#ifndef	NO_FORK
    SIGCHLD,
#endif				/* NO_FORK */
    0};
#define	SIGNAL_LIST(ip)	{ int *ip; for (ip = task_signals; *ip; ip++)
#define	SIGNAL_LIST_END(ip) }


/*
 * Miscellaneous variables.
 */
char *task_progname = 0;			/* name we were invoked as */
char *task_hostname = 0;			/* Hostname of this system */
int task_pid = 0;				/* my process ID */
int task_mpid = 0;				/* process ID of main process */
char task_pid_str[6] = { 0 };			/* Printable version of PID */
flag_t task_state = TASKS_INIT|TASKS_NOFLASH;	/* Current state */
size_t task_pagesize = 0;			/* System page size */
size_t task_maxpacket = 0;			/* Maximum packet size the kernel supports */
block_t task_block_index = 0;			/* Block allocation index for task structures */

/*
 *	I/O structures - visible
 */

/* Receive buffer and length */
void_t task_recv_buffer = 0;
size_t task_recv_buffer_len = 0;

/* Send buffer and length */
void_t task_send_buffer = 0;
size_t task_send_buffer_len = 0;

sockaddr_un *task_recv_srcaddr = 0;	/* Source address of this packet */
sockaddr_un *task_recv_dstaddr = 0;	/* Destination address of this packet */


static const bits task_state_bits[] =
{
    {TASKS_INIT,	"Init" },
    {TASKS_TEST,	"Test" },
    {TASKS_RECONFIG,	"ReConfig" },
    {TASKS_TERMINATE,	"Terminate" },
    {TASKS_NORECONFIG,	"NoReConfig" },
    {TASKS_NOSEND,	"NoSend" },
    {TASKS_FLASH,	"FlashUpdate" },
    {TASKS_NEWPOLICY,	"NewPolicy" },
    {TASKS_NODAEMON, 	"Foreground" },
    {TASKS_STRICTIFS,	"StrictIfs" },
    {TASKS_NOFLASH,	"NoFlash" },
    {TASKS_NODUMP,	"NoDump" },
    {TASKS_NORESOLV,	"NoResolv" },
    {0}
} ;

static const bits task_flag_bits[] =
{
    {TASKF_ACCEPT,	"Accept"},
    {TASKF_CONNECT,	"Connect"},
    {TASKF_LAST,	"Last"},
    {TASKF_DELETE,	"Delete"},
    {0}
};

static const bits task_socket_options[] =
{
    {TASKOPTION_RECVBUF, "RecvBuffer"},
    {TASKOPTION_SENDBUF, "SendBuffer"},
    {TASKOPTION_LINGER, "Linger"},
    {TASKOPTION_REUSEADDR, "ReUseAddress"},
    {TASKOPTION_BROADCAST, "Broadcast"},
    {TASKOPTION_DONTROUTE, "DontRoute"},
    {TASKOPTION_KEEPALIVE, "KeepAlive"},
    {TASKOPTION_DEBUG, "Debug"},
    {TASKOPTION_NONBLOCKING, "NonBlocking"},
    {TASKOPTION_USELOOPBACK, "UseLoopback"},
    {TASKOPTION_GROUP_ADD, "GroupAdd"},
    {TASKOPTION_GROUP_DROP, "GroupDrop"},
    {TASKOPTION_MULTI_IF, "MulticastInterface"},
    {TASKOPTION_MULTI_LOOP, "MulticastLoop"},
    {TASKOPTION_MULTI_TTL, "MulticastTTL"},
    {TASKOPTION_MULTI_ADD, "MulticastAdd"},
    {TASKOPTION_MULTI_DROP, "MulticastDrop"},
    {TASKOPTION_MULTI_ROUTE, "MulticastRouting"},
    {TASKOPTION_TTL,	"TTL"},
    {TASKOPTION_TOS,	"TOS"},
    {TASKOPTION_RCVDSTADDR,	"RcvDstAddr"},
    {TASKOPTION_IPHEADER_INC,	"IncludeIpHeader"},
    {0, NULL}
};

static const bits task_msg_bits[] =
{
    {MSG_OOB, "MSG_OOB"},
    {MSG_PEEK, "MSG_PEEK"},
    {MSG_DONTROUTE, "MSG_DONTROUTE"},
#ifdef	MSG_EOR
    {MSG_EOR, "MSG_EOR"},
#endif	/* MSG_EOR */
#ifdef	MSG_TRUNC
    {MSG_TRUNC, "MSG_TRUNC"},
#endif	/* MSG_TRUNC */
#ifdef	MSG_CTRUNC
    {MSG_CTRUNC, "MSG_CTRUNC"},
#endif	/* MSG_CTRUNC */
#ifdef	MSG_WAITALL
    {MSG_WAITALL, "MSG_WAITALL"},
#endif	/* MSG_WAITALL */
    {0, NULL}
};


/*
 *	Free any deleted tasks
 */
static void
task_collect __PF0(void)
{
    register task *tp;

    for (tp = task_head.task_forw; tp != &task_head; tp = tp->task_forw) {
	if (BIT_TEST(tp->task_flags, TASKF_DELETE)) {
	    register task *free_tp = tp;

	    /* Point to previous task */
	    tp = tp->task_back;

	    /* Remove from the list */
	    remque((struct qelem *) free_tp);

	    /* And delete this task */
	    task_block_free(task_block_index, (void_t) free_tp);
	}
    }
}


/**/

/* Timer stuff */

static const bits timer_flag_bits[] =
{
    {TIMERF_ABSOLUTE, "Absolute"},
    {TIMERF_DELETE, "Delete"},
    {0}
};

block_t timer_block_index = 0;	/* Allocation index for timer structures */

/*
 * get time of day in seconds and as an ASCII string.
 * Called at each interrupt and saved in external variables.
 */

void
timer_peek __PF0(void)
{
    struct timeval tp;
#ifdef	HAVE_GETSYSTIMES
    struct timeval boot;

    if (getsystimes(&tp, &boot)) {
	trace(TR_ALL, LOG_ERR, "timer_peek: getsystimes: %m");
    }
#else	/* HAVE_GETSYSTIMES */
    struct timezone tzp;

    if (gettimeofday(&tp, &tzp)) {
	trace(TR_ALL, LOG_ERR, "timer_peek: gettimeofday: %m");
    }
#endif	/* HAVE_GETSYSTIMES */

    if (time_sec != tp.tv_sec) {
	/* The time has changed */

	time_sec = tp.tv_sec;

#ifdef	time_boot
	time_boot = boot.tv_sec;
	tp.tv_sec += boot.tv_sec;
	if ((tp.tv_usec + boot.tv_usec) >= 1000000) {
	    tp.tv_sec++;
	}
#endif	/* time_boot */

	(void) strcpy(time_full, (char *) ctime(&tp.tv_sec));
	(void) strncpy(time_string, &time_full[4], 15);
	time_string[15] = (char) 0;
    }
}


/*
 *	Insert a timer on one of the queues.  Inactive timers are
 *	inserted at the beginning of their queue, Active timers are
 *	inserted in order of their expiration.
 */
inline static void
timer_insert  __PF1(insert_tip, timer *)
{
    timer *insert_tip1;

    if (insert_tip->timer_interval) {
	TIMER_ACTIVE(insert_tip1, insert_tip2) {
	    if (insert_tip->timer_next_time < insert_tip1->timer_next_time) {
		break;
	    }
	} TIMER_ACTIVE_END(insert_tip1, insert_tip2);
    } else {
	insert_tip1 = timer_queue_inactive.timer_forw;
    }

    insque((struct qelem *) insert_tip, (struct qelem *) insert_tip1->timer_back);
}


/*
 *	Return a pointer to a string containing the timer name
 */
char *
timer_name __PF1(tip, timer *)
{
    static char name[MAXHOSTNAMELENGTH];

    if (tip->timer_task) {
	if (tip->timer_task->task_addr && tip->timer_task->task_addr) {
	    (void) sprintf(name, "%s_%s.%A",
			   tip->timer_task->task_name,
			   tip->timer_name,
			   tip->timer_task->task_addr);
	} else {
	    (void) sprintf(name, "%s_%s",
			   tip->timer_task->task_name,
			   tip->timer_name);
	}
    } else {
	(void) strcpy(name, tip->timer_name);
    }
    return name;
}


/*
 *	Create a timer - returns pointer to timer structure
 */
timer *
timer_create(tp, indx, name, flags, interval, offset, job, data)
task *tp;
int indx;
const char *name;
flag_t flags;
time_t interval;
time_t offset;
_PROTOTYPE(job,
	   void,
	   (timer *,
	    time_t));
void_t data;
{
    timer *tip;

    tip = (timer *) task_block_alloc(timer_block_index);
    tip->timer_name = name;
    tip->timer_task = tp;
    tip->timer_index = indx;
    tip->timer_flags = flags;
    tip->timer_interval = interval;
    tip->timer_job = job;
    tip->timer_data = data;

    /* Link timer to it's task if there is one */
    if (tip->timer_task) {
	/* Too many timers */
	assert(tip->timer_index < tip->timer_task->task_n_timers);

	if (!tip->timer_task->task_timer) {
	    /* Allocate timer array */
	    tip->timer_task->task_timer = (timer **) task_mem_calloc(tip->timer_task,
								     tip->timer_task->task_n_timers,
								     (size_t) (sizeof (tip)));
	}
	tip->timer_task->task_timer[tip->timer_index] = tip;
    }

    /* If this timer is active, set the intervals */
    if (tip->timer_interval) {
	tip->timer_next_time = tip->timer_last_time = time_sec + offset;
	if (BIT_TEST(tip->timer_flags, TIMERF_ABSOLUTE)) {
	    tip->timer_next_time += tip->timer_interval;
	}
    }

    /* Insert in the correct queue */
    timer_insert(tip);

    /* If we have changed the wakeup time cause a wakeup now to recalculate */
    if ((tip == timer_queue_active.timer_forw) &&
	(timer_queue_active.timer_next_time > tip->timer_next_time)) {
	(void) kill(task_pid, SIGALRM);
    }
    TRACE_ON(TR_TIMER) {
	trace(TR_TIMER, 0, "timer_create: created timer %s  flags <%s>  interval %#T at %T",
	      timer_name(tip),
	      trace_bits(timer_flag_bits, tip->timer_flags),
	      tip->timer_interval,
	      tip->timer_next_time);
     }

    return tip;
}


/*
 *	Delete a timer
 */
int
timer_delete __PF1(tip, timer *)
{
    int active = (tip == timer_active);

    TRACE_ON(TR_TIMER) {
	trace(TR_TIMER, 0, "timer_delete: %s",
	      timer_name(tip));
     }

    /* Unlink this timer from it's task if there is one */
    if (tip->timer_task) {
	tip->timer_task->task_timer[tip->timer_index] = (timer *) 0;
	tip->timer_task = (task *) 0;
    }
    if (active) {
	BIT_SET(tip->timer_flags, TIMERF_DELETE);
    } else {
	remque((struct qelem *) tip);
	task_block_free(timer_block_index, (void_t) tip);
    }

    return active;
}


/*
 *	Reset a timer - move it to the inactive queue
 */
void
timer_reset __PF1(tip, timer *)
{
    if (tip->timer_interval) {
	tip->timer_next_time = tip->timer_last_time = tip->timer_interval = (time_t) 0;

	if (tip != timer_active) {
	    remque((struct qelem *) tip);
	    timer_insert(tip);
	}

	TRACE_ON(TR_TIMER) {
	    trace(TR_TIMER, 0, "timer_reset: reset %s",
		  timer_name(tip));
	 }
    }
}


/*
 *	Set a timer to fire in interval seconds from now
 */
void
timer_set __PF3(tip, timer *,
		interval, time_t,
		offset, time_t)
{
    tip->timer_interval = interval;

    if (tip != timer_active) {
	/* If the active timer it will be updated when the routine finishes */

	if (tip->timer_next_time) {
	    /* Timer was active */

	    tip->timer_next_time = time_sec + tip->timer_interval;
	} else {
	    /* Timer was inactive */

	    tip->timer_next_time = (tip->timer_last_time = time_sec + offset) + tip->timer_interval;
	}

	/* Re-insert this timer in the active queue in expiration order */
	remque((struct qelem *) tip);
	timer_insert(tip);

	/* If we have changed the wakeup time cause a wakeup now to recalculate */
	if ((tip == timer_queue_active.timer_forw) &&
	    (tip != timer_active) &&
	    (timer_queue_active.timer_next_time > tip->timer_next_time)) {
	    (void) kill(task_pid, SIGALRM);
	}
    }

    TRACE_ON(TR_TIMER) {
	trace(TR_TIMER, 0, "timer_set: timer %s interval set to %#T at %T",
	      timer_name(tip),
	      tip->timer_interval,
	      tip->timer_next_time);
     }

}

/*
 *	Set a timer to fire in interval seconds from the last time it fired
 */
void
timer_interval __PF2(tip, timer *,
		     interval, time_t)
{
    if (tip->timer_interval != interval) {
	tip->timer_interval = interval;

	if (tip != timer_active) {
	    /* Don't duplicate this calculation on the active timer */
	    
	    if (!tip->timer_next_time) {
		tip->timer_last_time = tip->timer_next_time = time_sec;
	    }
	    tip->timer_next_time = tip->timer_last_time + tip->timer_interval;

	    /* Re-insert this timer in the active queue in expiration order */
	    remque((struct qelem *) tip);
	    timer_insert(tip);

	    /* If we have changed the wakeup time cause a wakeup now to recalculate */
	    if ((tip == timer_queue_active.timer_forw) &&
		(tip != timer_active) &&
		(timer_queue_active.timer_next_time > tip->timer_next_time)) {
		(void) kill(task_pid, SIGALRM);
	    }
	}

	TRACE_ON(TR_TIMER) {
	    trace(TR_TIMER, 0, "timer_interval: timer %s interval set to %#T at %T",
		  timer_name(tip),
		  tip->timer_interval,
		  tip->timer_next_time);
	 }
    }
}


/*
 *	Dump the specified timer
 */
static void
timer_dump  __PF2(fd, FILE *,
		  tip, timer *)
{
    (void) fprintf(fd, "\t\t%s",
		   timer_name(tip));
    if (tip->timer_interval) {
	(void) fprintf(fd, "\tlast: %T\tnext: %T\tinterval: %#T",
		       tip->timer_last_time,
		       tip->timer_next_time,
		       tip->timer_interval);
    }
    if (tip->timer_flags) {
	(void) fprintf(fd, "\t<%s>",
		       trace_bits(timer_flag_bits, tip->timer_flags));
    }
    (void) fprintf(fd, "\n");
}


/*
 * timer control for periodic route-age and interface processing.
 * timer_dispatch() is called when the periodic interrupt timer expires.
 */
static void
timer_dispatch __PF0(void)
{
    time_t late = 0;
    register timer *tip;
    struct itimerval value;

    /* Log a message if the system dispatched us late */
    if (timer_queue_active.timer_last_time) {
	trace(TR_TIMER, 0, "timer_dispatch: requested interval: %#T actual interval: %#T",
	      timer_queue_active.timer_interval,
	      time_sec - timer_queue_active.timer_last_time);

	late = time_sec - timer_queue_active.timer_last_time - timer_queue_active.timer_interval;
	if (late < TIMER_FUZZ) {
	    late = 0;
	}
	if (late) {
	    trace(TR_INT, 0, "timer_dispatch: interval timer interrupt %d seconds late",
		  late);
	}
    } else {
	trace(TR_TIMER, 0, "timer_dispatch: initializing");
    }

    /* Run the queues until all the expired timers have been serviced.  This allows for timers that expire while we are */
    /* working on other timers */
    do {
	TIMER_ACTIVE(tip, tip1) {
	    register task *task_save;
	  
	    /* Timers are in time order so we don't have to scan the whole list */
	    if (time_sec < tip->timer_next_time) {
		break;
	    }
	    /* Log the timer */
	    TRACE_ON(TR_TIMER) {
		trace(TR_TIMER, 0, "timer_dispatch: call %s, due at %T, last at %T, interval %#T",
		      timer_name(tip),
		      tip->timer_next_time,
		      tip->timer_last_time,
		      tip->timer_next_time - tip->timer_last_time);
	     }

	    /* Update last firing time */
	    tip->timer_last_time = time_sec;

	    /* Set the active task pointer */
	    if (tip->timer_task) {
	          task_save = task_active;
		  task_active = tip->timer_task;
	    } else {
	          task_save = (task *) 0;
	    }
	    
	    /* Call the timer routine */
	    timer_active = tip;
	    tip->timer_job(tip, tip->timer_interval);
	    timer_active = (timer *) 0;

	    /* Reset the active task pointer */
	    if (task_save) {
	        task_active = task_save;
	    }

	    TRACE_ON(TR_TIMER) {
		tracef("timer_dispatch: returned from %s, ",
		       timer_name(tip));
	    }

	    if (BIT_TEST(tip->timer_flags, TIMERF_DELETE)) {
		/* Delete a one-shot timer */
		
		(void) timer_delete(tip);

		trace(TR_TIMER, 0, "deletion requested");
	    } else if (tip->timer_interval) {
		/* Timer is still active */

		/* Reschedule again at the next interval after the current time */
	        {
		    register time_t delta = time_sec - tip->timer_next_time;

		    if (delta >= 0) {
			tip->timer_next_time += ((delta / tip->timer_interval) + 1) * tip->timer_interval;
		    }
		}

		/* Remove and re-insert to maintain order in the queue */
		remque((struct qelem *) tip);
		timer_insert(tip);

		TRACE_ON(TR_TIMER) {
		    tracef("rescheduled ");
		    if (tip->timer_interval > late) {
			tracef("in %#T ",
			       tip->timer_interval - late);
		    }
		    trace(TR_TIMER, 0, "at %T",
			  tip->timer_next_time);
		}
	    } else {
		/* Timer is now inactive, put on inactive queue */

		remque((struct qelem *) tip);
		timer_insert(tip);
		
		trace(TR_TIMER, 0, "now inactive");
	    }

	} TIMER_ACTIVE_END(tip, tip1);

	/* Process all the queued flash updates */
	rt_flash_update(TRUE);

	/* Get the current time */
	timer_peek();

    } while (timer_queue_active.timer_forw->timer_next_time <= time_sec);

    /* Calulate when we are supposed to wake up next and how long that is from now */
    timer_queue_active.timer_next_time = timer_queue_active.timer_forw->timer_next_time;
    timer_queue_active.timer_interval = timer_queue_active.timer_next_time - time_sec;
    timer_queue_active.timer_last_time = time_sec;

    trace(TR_TIMER, 0, "timer_dispatch: end, next job: %T delta: %#T",
	  timer_queue_active.timer_next_time,
	  timer_queue_active.timer_interval);

    /* Check for invalid intervals (should not happen) */
    if (timer_queue_active.timer_interval <= 0) {
	trace(TR_INT, 0, "timer_dispatch: timer interval (%#T) invalid, using 1 second",
	      timer_queue_active.timer_interval);
	timer_queue_active.timer_interval = 1;
    }
    /* Set the interval timer */
    value.it_interval.tv_sec = 0;	/* no auto timer reload */
    value.it_interval.tv_usec = 0;
    value.it_value.tv_sec = timer_queue_active.timer_interval;
    value.it_value.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &value, (struct itimerval *) 0)) {
	trace(TR_ALL, LOG_ERR, "timer_dispatch: setitimer: %m");
	task_quit(errno);
    }
}


/*  */

#define	TASK_SAVE(tp, s)		{ task *task_save = task_active; task_active = tp; s; task_active = task_save; }


const bits task_socket_types[] =
{
#ifdef	SOCK_STREAM
    {SOCK_STREAM, "STREAM"},
#endif	/* SOCK_STREAM */
#ifdef	SOCK_DGRAM
    {SOCK_DGRAM, "DGRAM"},
#endif	/* SOCK_DGRAM */
#ifdef	SOCK_RAW
    {SOCK_RAW, "RAW"},
#endif	/* SOCK_RAW */
#ifdef	SOCK_RDM
    {SOCK_RDM, "RDM"},
#endif	/* SOCK_RDB */
#ifdef	SOCK_SEQPACKET
    {SOCK_SEQPACKET, "SEQPACKET"},
#endif	/* SOCK_SEQPACKET */
    { 0, NULL }
};


const bits task_domain_bits[] =
{
#ifdef	AF_UNSPEC
    {AF_UNSPEC, "UNSPEC"},
#endif	/* AF_UNSPEC */
#ifdef	AF_UNIX
    {AF_UNIX, "UNIX"},
#endif	/* AF_UNIX */
#ifdef	AF_INET
    {AF_INET, "INET"},
#endif	/* AF_INET */
#ifdef	AF_ISO
    {AF_ISO, "ISO"},
#endif	/* AF_ISO */
#ifdef	AF_ROUTE
    {AF_ROUTE, "Route"},
#endif	/* AF_ROUTE */
#ifdef	AF_NIT
    {AF_NIT, "NIT"},
#endif	/* AF_NIT */
    { 0, NULL }
};

/**/

static int task_pid_fd = -1;

/* Write and lock PID file */
void
task_pid_open __PF0(void)
{
    char path_pid[MAXPATHLEN];

    /* Open, lock and update the PID file */

    /* Try to open it */
#ifndef	O_SYNC
#define	O_SYNC	0
#endif	/* O_SYNC */
    (void) sprintf(path_pid, _PATH_PID, task_progname);
    task_pid_fd = open(path_pid, O_RDWR | O_CREAT | O_SYNC, 0644);
    if (task_pid_fd < 0) {
	int error = errno;

	trace(TR_ALL, LOG_ERR, "Could not open %s: %m",
	      path_pid);

	task_quit(error);
    } else {
	int len;
	char buf[LINE_MAX];
	
	/* Try to lock it */
	if (flock(task_pid_fd, LOCK_EX|LOCK_NB) < 0) {
	    int error = errno;
	    int pid;

	    switch (error) {
	    case EWOULDBLOCK:
#if	defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	    case EAGAIN:		/* System V style */
#endif	/* EAGAIN */
		len = read(task_pid_fd, buf, sizeof buf);
		if (len > 0 &&
		    (pid = atoi(buf))) {

		    /* Announce PID of gated already running */

		    trace(TR_ALL, LOG_ERR, "Could not obtain a lock on %s, %s[%d] is still running!",
			  path_pid,
			  task_progname,
			  pid);
		} else {
		    trace(TR_ALL, LOG_ERR, "Could not obtain a lock on %s, is another copy of gated running!",
			  path_pid);
		}
		break;

	    default:
		trace(TR_ALL, LOG_ERR, "flock(%s, LOCK_EX): %m",
		      path_pid);
	    }
	    (void) close(task_pid_fd);
	    task_pid_fd = -1;
	    task_quit(error);
	}

	len =  sprintf(buf, "%d\n",
		       task_pid);

#ifndef	SEEK_SET
#define	SEEK_SET	L_SET
#endif	/* SEEK_SET */
	/* Back up to the beginning and truncate the file */
	if (lseek(task_pid_fd, 0L, SEEK_SET) < 0
	    || ftruncate(task_pid_fd, 0L) < 0
	    || write(task_pid_fd, buf, len) != len) {
	    int error = errno;

	    trace(TR_ALL, LOG_ERR, "Could not write %s: %m",
		  path_pid);

	    task_quit(error);
	}

	/* Leave the file open to retain the lock */
    }

    {
	FILE *fp;
	
	/* Write version file */
	(void) sprintf(path_pid, _PATH_VERSION, task_progname);
	fp = fopen(path_pid, "w");
	if (fp) {
	    (void) fprintf(fp, "%s version %s built %s\n\tpid %d, started %s",
			   task_progname,
			   gated_version,
			   build_date,
			   task_pid,
			   time_full);
	    (void) fclose(fp);
	}
    }
}

/* Close the PID file */
static void
task_pid_close __PF0(void)
{
    char path_pid[MAXPATHLEN];

    (void) sprintf(path_pid, _PATH_PID, task_progname);
    if (task_pid_fd > -1) {
	if (close(task_pid_fd) == -1
	    || unlink(path_pid) == -1) {
	    trace(TR_ALL, LOG_ERR, "Could not close and remove %S: %m",
		  path_pid);
	}
	task_pid_fd = -1;
    }
}


/**/
/* Exit gated */

void
task_quit __PF1(code, int)
{
#ifdef	PROTO_INET
    /* Try to remove the pseudo default and install another */
    rt_default_reset();
#endif	/* PROTO_INET */

    /* Figure out what time it is */
    timer_peek();

    /* Remove the PID file */
    task_pid_close();
    
    trace(TR_ALL, 0, NULL);

    switch (code) {
    case 0:
    case EDESTADDRREQ:
	tracef("Exit %s[%d] version %s",
	       task_progname,
	       task_pid,
	       gated_version);
	if (code) {
	    errno = code;
	    tracef(": %m");
	}
	trace(TR_ALL, LOG_NOTICE, NULL);
	trace(TR_ALL, 0, NULL);
	trace_close();
	break;

    default:
	errno = code;
	trace(TR_ALL, LOG_NOTICE, "Abort %s[%d] version %s: %m",
	      task_progname,
	      task_pid,
	      gated_version);
	trace(TR_ALL, 0, NULL);
	trace_close();
	abort();
	break;
    };

    exit(code);
}


void
task_assert __PF2(file, const char *,
		  line, int)
{
#ifdef	PROTO_INET
    /* Try to remove the pseudo default and install another */
    rt_default_reset();
#endif	/* PROTO_INET */

    /* Figure out what time it is */
    timer_peek();

    /* Let them know what happened */
    trace(TR_ALL, 0, NULL);
    trace(TR_ALL, LOG_ERR, "Assertion failed %s[%d]: file \"%s\", line %d",
	  task_progname,
	  task_pid,
	  file,
	  line);
    trace(TR_ALL, 0, NULL);

    /* Exit with a core dump */
    task_quit(EINVAL);
}


/* Change state */
void
task_newstate __PF2(set, flag_t,
		    reset, flag_t)
{
    flag_t state = (task_state & ~reset) | set;
    
    tracef("task_newstate: State change from <%s>",
	   trace_bits(task_state_bits, task_state));
    trace(TR_TASK, 0, " to <%s>",
	  trace_bits(task_state_bits, state));

    task_state = state;
}


/*
 *	Call the reinit routine for each task
 */
static void
task_reinit __PF0(void)
{
    task *tp;
    
    trace(TR_TASK, 0, NULL);
    TASK_TABLE(tp) {
	if (tp->task_reinit) {
	    trace(TR_TASK, 0, "task_reinit: Starting reinit for task %s",
		  task_name(tp));
	    TASK_SAVE(tp, tp->task_reinit(tp));
	    trace(TR_TASK, 0, "task_reinit: Finished reinit for task %s",
		  task_name(tp));
	}
    } TASK_TABLE_END(tp);
}


/*
 *	Return a pointer to a string containing the task name
 */
char *
task_name __PF1(tp, task *)
{
    static char name[MAXHOSTNAMELENGTH];

    if (!tp) {
	(void) strcpy(name, "(null task)");
    } else if (BIT_TEST(tp->task_flags, TASKF_DELETE)) {
	(void) sprintf(name, "%s (DELETED)",
		       tp->task_name);
    } else {
	if (tp->task_addr) {
	    (void) sprintf(name, "%s.%#A",
			   tp->task_name,
			   tp->task_addr);
	} else {
	    (void) strcpy(name, tp->task_name);
	}
	
	if (tp->task_pid > 0) {
	    (void) sprintf(&name[strlen(name)], "[%d]",
			   tp->task_pid);
	}
    }
    
    return name;
}


/*
 *	Receive packet and check for errors
 */
int
task_receive_packet __PF2(tp, task *,
			  count, size_t *)
{
    register int rc;
    byte buf[BUFSIZ];
    caddr_t bp = (caddr_t) buf;
    static struct iovec iovec;
    static struct msghdr msghdr = {
	0, 0,		/* Address and length of received address */
	&iovec, 1,	/* Address and length of buffer runtime */
	NULL, 0		/* Address and length of access rights */
    };

    iovec.iov_base = task_recv_buffer;
    iovec.iov_len = task_recv_buffer_len;

    if (task_recv_srcaddr) {
	sockfree(task_recv_srcaddr);
	task_recv_srcaddr = (sockaddr_un *) 0;
    }

    /* Setup to receive address */
    msghdr.msg_name = bp;		/* Set pointer to address */
    msghdr.msg_namelen = 128;		/* Set max size */
    bp += msghdr.msg_namelen;
    bzero(msghdr.msg_name, msghdr.msg_namelen);	/* Clean address buffer */

#ifdef	SCM_RIGHTS
    /* Setup to receive control information */
    msghdr.msg_control = bp;			/* Set max size */
    msghdr.msg_controllen = sizeof buf - (bp - (caddr_t) buf);	/* Set pointer to buffer */
    bzero(msghdr.msg_control, msghdr.msg_controllen);
#endif	/* SCM_RIGHTS */

    while ((rc = recvmsg(tp->task_socket, &msghdr, 0)) < 0) {
	switch (errno) {
	case EINTR:
	    /* The call was interrupted, probably by a signal, */
	    /* silently retry it. */
	    break;
		
	case EHOSTUNREACH:
	case ENETUNREACH:
	    /* These errors are just an indication that an */
	    /* unreachable was received.  When an operation is */
	    /* attempted on a socket with an error pending */
	    /* itdoes not complete.  So we need to retry. */
	    trace(TR_ALL, 0, "task_receive_packet: %s recvmsg: %m",
		  task_name(tp));
	    break;
		
	default:
	    trace(TR_ALL, LOG_ERR, "task_receive_packet: %s recvmsg: %m",
		  task_name(tp));
	    /* Fall through */
	    
	case EWOULDBLOCK:
#if	defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	case EAGAIN:		/* System V style */
#endif	/* EAGAIN */
	    /* Nothing to read */
	    *count = rc;
	    return errno;
	}
    }

    if (!rc) {
	return TASKRC_EOF;
    }

    if (msghdr.msg_namelen) {
	register sockaddr_un *addr = sock2gated((struct sockaddr *) (void_t) msghdr.msg_name,
						msghdr.msg_namelen);
	if (addr) {
		task_recv_srcaddr = sockdup(addr);
	} else {
		/* Kent */
		trace(TR_TASK, 0, "task_receive_packet: task %s sock2gated returned addr = NULL", task_name(tp));
	}
    }

    TRACE_ON(TR_TASK) {
	tracef("task_receive_packet: task %s ",
	       task_name(tp));

	if (task_recv_srcaddr) {
	    tracef("from %#A ",
		   task_recv_srcaddr);
	}

	trace(TR_TASK, 0, "socket %d length %d",
	      tp->task_socket,
	      rc);
    }

#ifdef	MSG_CTRUNC
#define	ENOUGH_CMSG(cmsg, size)	((cmsg)->cmsg_len >= ((size) + sizeof(struct cmsghdr)))

    if (task_recv_dstaddr) {
	sockfree(task_recv_dstaddr);
	task_recv_dstaddr = (sockaddr_un *) 0;
    }
    
    /* Look at the control information */
    if (msghdr.msg_controllen >= sizeof (struct cmsghdr)
	&& !BIT_TEST(msghdr.msg_flags, MSG_CTRUNC)) {
	struct cmsghdr *cmsg;
	
	for (cmsg = CMSG_FIRSTHDR(&msghdr);
	     cmsg && cmsg->cmsg_len >= sizeof (struct cmsghdr);
	     cmsg = CMSG_NXTHDR(&msghdr, cmsg)) {
	    switch (cmsg->cmsg_level) {
#ifdef	PROTO_INET
	    case IPPROTO_IP:
		switch (cmsg->cmsg_type) {

		case IP_RECVDSTADDR:
		    /* Destination address of the packet */

		    if (!ENOUGH_CMSG(cmsg, sizeof(struct in_addr))) {
			trace(TR_INT, LOG_ERR, "task_receive_packet: %s dest address from %#A truncated",
			      task_name(tp),
			      task_recv_srcaddr);
		    } else if (!task_recv_dstaddr) {
		    	task_recv_dstaddr = sockdup(sockbuild_in(0,
								 ((struct in_addr *) CMSG_DATA(cmsg))->s_addr));
		    }
		    break;
		}
		break;
#endif	/* PROTO_INET */
	    }
	}
    }
#undef	ENOUGH_CMSG
#endif	/* MSG_CTRUNC */

    *count = rc;
    
#ifdef	MSG_TRUNC
    if (BIT_TEST(msghdr.msg_flags, MSG_TRUNC)) {
	/* Packet truncated */
	
	trace(TR_INT, LOG_ERR, "task_receive_packet: %s packet from %#A socket %d truncated",
	      task_name(tp),
	      task_recv_srcaddr,
	      tp->task_socket);
	return TASKRC_TRUNC;
    }
#endif	/* MSG_TRUNC */

    return TASKRC_OK;
}


/*
 *	Send a packet
 */
int
task_send_packet __PF5(tp, task *,
		       msg, void_t,
		       len, size_t,
		       flags, flag_t,
		       addr, sockaddr_un *)
{
    int rc = 0;
    flag_t log = TR_TASK;
    int pri = 0;
    int value = 0;
#define	SEND_RETRIES	5
    int retry = SEND_RETRIES;
    const char *errmsg = NULL;
    static struct iovec iovec;
    static struct msghdr msghdr = {
	0, 0,			/* Address and length of target address */
	&iovec, 1,		/* Address and length of buffer - changed at runtime */
	NULL, 0			/* Address and length of access rights */
    };

    if (BIT_TEST(task_state, TASKS_NOSEND)) {
	log = TR_ALL;
	errmsg = ": packet transmission disabled";
	rc = len;
	goto Log;
    }

    iovec.iov_base = msg;
    iovec.iov_len = len;

    if (addr) {
	struct sockaddr *ua = sock2unix(addr, (int *) &msghdr.msg_namelen);
	
	msghdr.msg_name = (caddr_t) ua;
    } else {
	msghdr.msg_name = (caddr_t) 0;
	msghdr.msg_namelen = 0;
    }

    while ((rc = sendmsg(tp->task_socket, &msghdr, (int) flags)) < 0) {
	switch (errno) {
	case EHOSTUNREACH:
	case ENETUNREACH:
	    /* These errors may be just an indication that an */
	    /* unreachable was received.  When an operation is */
	    /* attempted on a socket with an error pending */
	    /* it does not complete.  So we need to retry. */
	    if (retry--) {
		/* Retry the send a few times */
		break;
	    }
	    /* Too many retries - give up */
	    log = TR_ALL;
	    value = SEND_RETRIES;
	    errmsg = ": (%d retries) %m";
	    goto Log;
	    
	case EINTR:
	    /* The system call was interrupted, probably by */
	    /* a signal.  Silently retry */
	    break;

	default:
	    /* Fatal error */
	    log = TR_ALL;
	    pri = LOG_ERR;
	    errmsg = ": %m";
	    goto Log;

	case EWOULDBLOCK:
#if	defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	case EAGAIN:		/* System V style */
#endif	/* EAGAIN */
	    goto Return;
	}
    }

    if (rc != len) {
	value = len - rc;
	log = TR_ALL;
	pri = LOG_ERR;
	errmsg = ": %d bytes not accepted";
    }

 Log:
    tracef("task_send_packet: task %s socket %d length %d",
	   task_name(tp),
	   tp->task_socket,
	   len);
    if (flags) {
	tracef(" flags %s(%X)",
	       trace_bits(task_msg_bits, flags),
	       flags);
    }
    if (addr) {
	tracef(" to %#A",
	       addr);
    }
    trace(log, pri, errmsg, value);

 Return:
    return rc;
}

#ifdef _POWER
/*
 * SRC packet processing - .
 */
int dosrcpacket(msgno, subsys, txt, len)
        int msgno;
        char *subsys;
        char *txt;
        int len;
{
        struct srcrep reply;

        reply.svrreply.rtncode = msgno;
        strcpy(reply.svrreply.objname, subsys);
        strcpy(reply.svrreply.rtnmsg, txt);
        srchdr = srcrrqs(&srcpacket);
        srcsrpy(srchdr, &reply, len, cont);
}
#endif /* _POWER */

/*
 *	Wait for incoming packets
 */
void
task_main __PF0(void)
{
    int n;
    fd_set read_bits, write_bits, except_bits;
    task *tp;

    /* Make dumps happen in a known place */
    (void) task_chdir(_PATH_DUMPDIR);

    /* Remember when we started */
    timer_time_start = timer_time;	/* struct copy */

    if (!BIT_TEST(task_state, TASKS_TEST)) {
	trace(TR_INT, 0, NULL);
	trace(TR_INT, LOG_NOTICE, "Commence routing updates");
    }
    trace(TR_INT, 0, NULL);

    if (BIT_TEST(task_state, TASKS_INIT)) {
	/* Allocate send and receive buffer now that we know their maximum size */
	if (task_send_buffer_len && !task_send_buffer) {
	    task_send_buffer = (void_t) task_block_malloc(task_send_buffer_len);
	}
	if (task_recv_buffer_len && !task_recv_buffer) {
	    task_recv_buffer = (void_t) task_block_malloc(task_recv_buffer_len);
	}
    }

    task_newstate(0, TASKS_INIT);

    task_reinit();

    /* Notify the protocols about the interfaces */
    if_notify();
    
    /* XXX - This will cause protocols to send update packets, should defer just a bit longer */
    /* XXX - How about TASKS_NOBUF to prevent bufs from being allocated and leaving TASKS_INIT on? */

    /* Re-enable flash updates to the protocols */
    task_newstate(0, TASKS_NOFLASH);

    /* Update the kernel with changes so far and have protocols re-evaluate policy */
    rt_flash_update(FALSE);

    if (BIT_MATCH(task_state, TASKS_TEST)) {
	/* Just testing configuration */
	if (!BIT_TEST(task_state, TASKS_NODUMP)) {
	    trace_dump(TRUE);
	}
	task_quit(0);
    }

    timer_dispatch();
    trace(TR_TASK, 0, NULL);

#ifdef _POWER
    if (src_exists == TRUE) {
        FD_SET(SRC_FD, &task_select_readbits);
        if (SRC_FD > task_max_socket)
            task_max_socket = SRC_FD;
    }
#endif  /* _POWER */
    
    while (TRUE) {

	/* Allow signals back in */
	task_signals_release("task_main");

	trace(TR_TASK, 0, NULL);
	read_bits = task_select_readbits;
	write_bits = task_select_writebits;
	except_bits = task_select_exceptbits;
	n = select(task_max_socket + 1, &read_bits, &write_bits, &except_bits, (struct timeval *) 0);

	if (n < 0) {
	     switch (errno) {
	     case EINTR:
		 /* The system call was interrupted, probably by */
		 /* a signal.  Silently retry */
		 continue;

	     default:
		 trace(TR_ALL, LOG_ERR, "task_main: select: %m");
		 task_quit(errno);
	     }
	}

	/* Block signals */
	task_signals_block("task_main");

	/* Scan the task list in priority order.  This insures that redirects are processed before protocol updates */
	TASK_TABLE(tp) {
	    if (tp->task_socket == -1) {
		continue;
	    }
	    
	    timer_peek();			/* current time */

	    /* Check for ready for read on socket */
	    if (FD_ISSET(tp->task_socket, &read_bits)) {
		if (BIT_TEST(tp->task_flags, TASKF_ACCEPT)) {
		    if (tp->task_accept) {
			trace(TR_TASK, 0, "task_main: accept ready for %s",
			      task_name(tp));
			TASK_SAVE(tp, tp->task_accept(tp));
			if (tp->task_socket == -1) {
			    continue;
			}
		    } else {
			trace(TR_ALL, LOG_ERR, "task_main: no accept method for %s socket %d",
			      task_name(tp),
			      tp->task_socket);
			FD_CLR(tp->task_socket, &task_select_readbits);
		    }
		} else {
		    if (tp->task_recv) {
			trace(TR_TASK, 0, "task_main: recv ready for %s",
			      task_name(tp));
			TASK_SAVE(tp, tp->task_recv(tp));
			if (tp->task_socket == -1) {
			    continue;
			}
		    } else {
			trace(TR_ALL, LOG_ERR, "task_main: no accept method for %s socket %d",
			      task_name(tp),
			      tp->task_socket);
			FD_CLR(tp->task_socket, &task_select_readbits);
		    }
		}
	    }

	    if (tp->task_socket == -1) {
		continue;
	    }
	    timer_peek();
	    
	    /* Check for ready for write on socket */
	    if (FD_ISSET(tp->task_socket, &write_bits)) {
		if (BIT_TEST(tp->task_flags, TASKF_CONNECT)) {
		    if (tp->task_connect) {
			trace(TR_TASK, 0, "task_main: connect ready for %s",
			      task_name(tp));
			TASK_SAVE(tp, tp->task_connect(tp));
			if (tp->task_socket == -1) {
			    continue;
			}
		    } else {
			trace(TR_ALL, LOG_ERR, "task_main: no connect method for %s socket %d",
			      task_name(tp),
			      tp->task_socket);
			FD_CLR(tp->task_socket, &task_select_writebits);
		    }
		} else {
		    if (tp->task_write) {
			trace(TR_TASK, 0, "task_main: write ready for %s",
			      task_name(tp));
			TASK_SAVE(tp, tp->task_write(tp));
			if (tp->task_socket == -1) {
			    continue;
			}
		    } else {
			trace(TR_ALL, LOG_ERR, "task_main: no write method for %s socket %d",
			      task_name(tp),
			      tp->task_socket);
			FD_CLR(tp->task_socket, &task_select_writebits);
		    }
		}
	    }
	    
	    if (tp->task_socket == -1) {
		continue;
	    }
	    timer_peek();
	    
	    /* Check for exception on socket */
	    if (FD_ISSET(tp->task_socket, &except_bits)) {
		if (tp) {
		    trace(TR_TASK, 0, "task_main: exception for %s",
			  task_name(tp));
		    TASK_SAVE(tp, tp->task_except(tp));
		} else {
		    trace(TR_ALL, LOG_ERR, "task_main: no exception method for %s socket %d",
			  task_name(tp),
			  tp->task_socket);
		    FD_CLR(tp->task_socket, &task_select_exceptbits);
		}
	    }
	} TASK_TABLE_END(tp) ;

	/* Clean up any deleted tasks */
	task_collect();
	
	/* Process all the queued flash updates */
	rt_flash_update(TRUE);

#ifdef _POWER
	    /* XXX: this really should have been 'just another task' ... */
	    if (src_exists && FD_ISSET(SRC_FD, &read_bits)) {
		if (rc = recvfrom(SRC_FD, &srcpacket, SRCMSG, 0,
						&srcaddr, &addrsz) < 0) {
			trace(TR_ALL, LOG_ERR,
					"task_main: ERROR: recvfrom: %m\n");
			continue;
		}

    		switch(srcpacket.subreq.action) {
        		case REFRESH:
			    {
				if (BIT_TEST(task_state, TASKS_NORECONFIG)) {
				    trace(TR_ALL, LOG_ERR, "task_main: reinitialization not possible");
				   dosrcpacket(SRC_ICMD,progname, MSGSTR(TASK_1,
				    "task_main: reinitialization not possible"),
							sizeof(struct srcrep));
				} else {
				    static void task_reconfigure();

				    task_reconfigure();
				    dosrcpacket(SRC_OK,progname,NULL,
							sizeof(struct srcrep));
				}
                		break;
			    }
        		case START:
        		case TRACE:
        		case STATUS:
                		dosrcpacket(SRC_SUBMSG,progname,MSGSTR(TASK_2,
				      "gated does not support this option.\n"),
							sizeof(struct srcrep));
                		break;
        		case STOP:
                		if (srcpacket.subreq.object == SUBSYSTEM) {
                        		dosrcpacket(SRC_OK,progname,NULL,
							sizeof(struct srcrep));
                        		if (srcpacket.subreq.parm1 == NORMAL)
                                        	task_quit(0);
                		} else
                        		dosrcpacket(SRC_SUBMSG,progname,
						MSGSTR(TASK_2,
				       "gated does not support this option.\n"),
							sizeof(struct srcrep));
                		break;
        		default:
                		dosrcpacket(SRC_SUBICMD,progname,NULL,
							sizeof(struct srcrep));
                		break;
      		}
	    }
#endif		/* defined(_POWER) */
    }
}


/**/
/*
 *	Call all configured protocols init routines
 */
struct protos {
    _PROTOTYPE(init,
	       void,
	       (void));
    _PROTOTYPE(var_init,
	       void,
	       (void));
    const char *proto;
};

static struct protos proto_inits[] = {
#ifdef	PROTO_INET
    { inet_init,	0,			"INET" },
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
    { iso_init,		0,			"ISO" },
#endif	/* PROTO_ISO */
    { if_init,		0,			"IF" },
#ifdef	PROTO_ISODE_SNMP
    { snmp_init,	snmp_var_init,		"SNMP" },
#endif	/* PROTO_ISODE_SNMP */
#if	defined(PROTO_ICMP) && !defined(KRT_RT_SOCK)
    { icmp_init,	0,			"ICMP" },
#endif	/* defined(PROTO_ICMP) && !defined(KRT_RT_SOCK) */
#if	defined(PROTO_ICMP) || defined(KRT_RT_SOCK)
    { 0,		redirect_var_init,	"Redirects" },
#endif	/* defined(PROTO_ICMP) || defined(KRT_RT_SOCK) */
#ifdef	PROTO_IGMP
    { igmp_init,	igmp_var_init,		"IGMP" },
#endif	/* PROTO_IGMP */
#ifdef	PROTO_EGP
    { egp_init,		egp_var_init,		"EGP" },
#endif	/* PROTO_EGP */
#ifdef	PROTO_BGP
    { bgp_init,		bgp_var_init,		"BGP" },
#endif	/* PROTO_BGP */
#ifdef	PROTO_DVMRP
    { dvmrp_init,	dvmrp_var_init,		"DVMRP" },
#endif	/* PROTO_DVMRP */
#ifdef	PROTO_OSPF
    { ospf_init,	ospf_var_init,		"OSPF" },
#endif	/* PROTO_OSPF */
#ifdef	PROTO_IGRP
    { igrp_init, 	igrp_var_init,		"IGRP" },
#endif	/* PROTO_IGRP */
#ifdef	PROTO_RIP
    { rip_init,		rip_var_init,		"RIP" },
#endif	/* PROTO_RIP */
#ifdef	PROTO_HELLO
    { hello_init,	hello_var_init,		"HELLO" },
#endif	/* PROTO_HELLO */
#ifdef	PROTO_IDPR
    { idpr_init,	idpr_var_init,		"IDPR" },
#endif	/* PROTO_IDPR */
#ifdef	PROTO_ISIS
    { isis_init,	isis_var_init,		"ISIS" },
#endif	/* PROTO_ISIS */
#ifdef	PROTO_SLSP
    { slsp_init,	slsp_var_init,		"SLSP" },
#endif	/* PROTO_SLSP */
        {0}
    };

void
task_proto_inits __PF0(void)
{
    struct protos *proto;

    for (proto = proto_inits; proto->proto; proto++) {
	if (proto->init) {
	    trace(TR_TASK, 0, NULL);
	    trace(TR_TASK, 0, "task_proto_inits: initializing %s",
		  proto->proto);
	    (proto->init)();
	}
	task_active = &task_task;
    }
}


void
task_proto_var_inits __PF0(void)
{
    struct protos *proto;

    for (proto = proto_inits; proto->proto; proto++) {
	if (proto->var_init) {
	    trace(TR_TASK, 0, NULL);
	    trace(TR_TASK, 0, "task_proto_var_inits: initializing %s",
		  proto->proto);
	    (proto->var_init)();
	}
	task_active = &task_task;
    }
}


/**/
/*
 *	Call all task that have posted a reinit routine
 */
static void
task_reconfigure __PF0(void)
{
    int i;
    task *tp;

    /* Block signals */
    task_signals_block("task_reconfigure");

    trace(TR_ALL, 0, NULL);
    trace(TR_ALL, LOG_INFO, "task_reconfigure re-initializing from %s",
	  task_config_file);
    trace(TR_ALL, 0, NULL);

    i = adv_n_allocated;

    task_newstate(TASKS_RECONFIG|TASKS_NOFLASH, 0);

    /* Call all tasks that have posted a cleanup routine */
    trace(TR_TASK, 0, NULL);
    TASK_TABLE(tp) {
	if (tp->task_cleanup) {
	    trace(TR_TASK, 0, "task_reconfigure Starting cleanup for task %s",
		  task_name(tp));
	    TASK_SAVE(tp, tp->task_cleanup(tp));
	    trace(TR_TASK, 0, "task_reconfigure Finished cleanup for task %s",
		  task_name(tp));
	}
    } TASK_TABLE_END(tp);

    if (adv_n_allocated) {
	trace(TR_ALL, LOG_ERR, "task_reconfigure %d of %d adv_entry elements not freed",
	      adv_n_allocated, i);
    }

    /* Reset options */
    krt_install = krt_install_parse;

    /* Reset all protocol configurations */
    task_proto_var_inits();
    
    if (parse_parse(task_config_file)) {
	task_quit(0);
    }

    task_proto_inits();
    task_newstate(0, TASKS_RECONFIG);

    /* Cause tasks to reinit */
    task_reinit();

    /* Remind the protocols about the interfaces */
    if_notify();

    /* Re-enable flash updates to the protocols */
    task_newstate(0, TASKS_NOFLASH);

    /* Update the kernel with changes so far and have protocols re-evaluate policy */
    rt_flash_update(FALSE);

    trace(TR_ALL, 0, NULL);
    trace(TR_ALL, LOG_INFO, "task_reconfigure reinitializing done");
    trace(TR_ALL, 0, NULL);

    /* Allow signals back in */
    task_signals_release("task_reconfigure");
}


/*
 *	Call all tasks that have posted an ifchange routine
 */
void
task_ifachange __PF1(ifap, if_addr *)
{
    task *tp;

    trace(TR_TASK, 0, NULL);
    TASK_TABLE(tp) {
	if (tp->task_ifachange) {
	    trace(TR_TASK, 0, "task_ifachange: Starting ifachange of %A(%s) for task %s",
		  ifap->ifa_addr,
		  ifap->ifa_name,
		  task_name(tp));
	    TASK_SAVE(tp, tp->task_ifachange(tp, ifap));
	    trace(TR_TASK, 0, "task_ifachange: Finished ifachange of %A(%s) for task %s",
		  ifap->ifa_addr,
		  ifap->ifa_name,
		  task_name(tp));
	}
    } TASK_TABLE_END(tp);
}


/*
 *	Call all tasks that have posted an ifchange routine
 */
void
task_iflchange __PF1(ifl, if_link *)
{
    task *tp;

    trace(TR_TASK, 0, NULL);
    TASK_TABLE(tp) {
	if (tp->task_iflchange) {
	    trace(TR_TASK, 0, "task_iflchange: Starting iflchange of %s for task %s",
		  ifl->ifl_name,
		  task_name(tp));
	    TASK_SAVE(tp, tp->task_iflchange(tp, ifl));
	    trace(TR_TASK, 0, "task_iflchange: Finished iflchange of %s for task %s",
		  ifl->ifl_name,
		  task_name(tp));
	}
    } TASK_TABLE_END(tp);
}


static const char *term_names[] = {
    "first",
    "second",
    "third",
    "fourth"
    };

static void
task_terminate __PF0(void)
{
    register task *tp;
    static int terminate = 0;

    trace(TR_INT, LOG_NOTICE, "task_terminate: %s terminate signal received", term_names[terminate]);

    /* Subprocesses terminate immediately for now */
    if (task_pid != task_mpid) {
	exit(0);
    }
    if (++terminate > 2) {
	task_quit(0);
    }
    task_newstate(TASKS_TERMINATE|TASKS_NOFLASH, 0);

    TASK_TABLE(tp) {
	if (tp->task_terminate && !BIT_TEST(tp->task_flags, TASKF_LAST)) {

	    trace(TR_TASK, 0, "task_terminate: terminating task %s",
		  task_name(tp));
	    TASK_SAVE(tp, tp->task_terminate(tp));
	    trace(TR_TASK, 0, NULL);

	}
    } TASK_TABLE_END(tp);
    trace(TR_TASK, 0, "task_terminate: Exiting and waiting for completion");
}


#ifndef	NO_FORK
static void
task_child __PF0(void)
{
    int pid;
    WAIT_T statusp;

    while (pid = waitpid(-1, &statusp, WNOHANG|WUNTRACED)) {
	if (pid < 0) {
	    /* Error */
	    switch (errno) {
	    case EINTR:
		/* Interrupted, try again */
		continue;

	    case ECHILD:
		/* No Children to wait for */
		return;
		    
	    default:
		trace(TR_ALL, LOG_ERR, "task_child: waitpid() error: %m");
	    }
	} else {
	    task *tp;

	    TASK_TABLE(tp) {
		if (pid == tp->task_pid) {
		    /* Assume my pid */
		    int done = TRUE;
			    
		    if (WIFSTOPPED(statusp)) {
			/* Stopped by a signal */
		    
			trace(TR_ALL, LOG_ERR, "task_child: %s stopped by SIG%s",
			      task_name(tp),
			      trace_state(signal_names, WSTOPSIG(statusp) - 1));
			done = FALSE;
		    } else if (WIFSIGNALED(statusp)) {
			/* Terminated by a signal */
				
			trace(TR_ALL, LOG_ERR, "task_child: %s terminated abnormally by SIG%s%s",
			      task_name(tp),
			      trace_state(signal_names, WTERMSIG(statusp) - 1),
			      WIFCOREDUMP(statusp) ? " with core" : "");
		    } else if (WEXITSTATUS(statusp)) {
			/* Non-zero exit status */
			
			trace(TR_ALL, LOG_ERR, "task_child: %s terminated abnormally with retcode %d",
			      task_name(tp),
			      WEXITSTATUS(statusp));
		    } else {
			/* Normal termination */
			
			trace(TR_TASK, 0, "task_child: %s terminated normally",
			      task_name(tp));
		    }
		    
		    if (done && tp->task_child) {
			TASK_SAVE(tp, tp->task_child(tp));
		    }

		    break;
		}
	    } TASK_TABLE_END(tp) ;
	}
    }

}
#endif	/* NO_FORK */


static SIGTYPE
task_receive_signal __PF1(sig, int)
{
#ifdef	HAVE_SYS_SIGLIST
    extern char *const sys_siglist[];
#endif	/* HAVE_SYS_SIGLIST */

    timer_peek();

    trace(TR_TASK, 0, NULL);
#ifdef	HAVE_SYS_SIGLIST
    trace(TR_TASK, 0, "task_receive_signal: received SIG%s(%d) <%s>",
	  trace_state(signal_names, sig - 1),
	  sig,
	  sys_siglist[sig]);
#else	/* HAVE_SYS_SIGLIST */
    trace(TR_TASK, 0, "task_receive_signal: received SIG%s(%d)",
	  trace_state(signal_names, sig - 1),
	  sig);
#endif	/* HAVE_SYS_SIGLIST */
    switch (sig) {
    case SIGTERM:
	task_terminate();
	break;

    case SIGALRM:
	timer_dispatch();
	break;

    case SIGHUP:
	if (BIT_TEST(task_state, TASKS_NORECONFIG)) {
	    trace(TR_ALL, LOG_ERR, "task_receive_signal: reinitialization not possible");
	} else {
	    task_reconfigure();
	}
	break;

#ifdef	SIGPIPE
    case SIGPIPE:
	break;
#endif	/* SIGPIPE */
	
    case SIGINT:
	trace_dump(FALSE);
	break;

    case SIGUSR1:
	if (!trace_file) {
	    trace(TR_ALL, LOG_ERR, "task_receive_signal: can not toggle tracing to console");
	    break;
	}
	if (trace_flags) {
	    trace_off();
	} else {
	    trace_on(trace_file, TRUE);
	}
	break;

    case SIGUSR2:
	if_check();
	break;

#ifndef	NO_FORK
    case SIGCHLD:
	task_child();
	break;
#endif	/* NO_FORK */

    default:
	trace(TR_INT, LOG_ERR, "task_receive_signal: Ignoring unknown signal SIG%s(%d)",
	      trace_state(signal_names, sig - 1),
	      sig);
    }	

    /* Clean up any deleted tasks */
    task_collect();
    
    trace(TR_TASK, 0, NULL);

    SIGRETURN;
}


/*
 *	close a task's socket and terminate
 */
void
task_close __PF1(close_tp, task *)
{
    int rc;
    int s = close_tp->task_socket;
    
    trace(TR_TASK, 0, "task_close: close socket %d task %s",
	  close_tp->task_socket,
	  task_name(close_tp));

    task_reset_socket(close_tp);
    NON_INTR(rc, close(s));
    if (rc < 0) {
	trace(TR_ALL, LOG_ERR, "task_close: close %s.%d: %m",
	      task_name(close_tp),
	      s);
    }
}


/*
 *  Delete a task block and free allocated storage.  When the last task has been deleted, exit.
 */
void
task_delete __PF1(tp, task *)
{
    int i;

    trace(TR_TASK, 0, "task_delete: deleting task %s",
	  task_name(tp));

    /* Close the socket if present */
    if (tp->task_socket != -1) {
	task_close(tp);
    }

    /* Delete any timers associated with this task */
    if (tp->task_timer) {
	for (i = 0; i < tp->task_n_timers; i++) {
	    timer *tip = TASK_TIMER(tp, i);
	    
	    if (tip) {
		if (timer_delete(tip)) {
		    /* Timer was active, remove pointer to the task */
		    tip->timer_task = (task *) 0;
		}
	    }
	}
	task_mem_free(tp, (void_t) tp->task_timer);
	tp->task_timer = 0;
    }
    

    if (tp->task_forw) {
	task_n_tasks--;
	if (tp->task_addr) {
	    sockfree(tp->task_addr);
	    tp->task_addr = (sockaddr_un *) 0;
	}
	BIT_SET(tp->task_flags, TASKF_DELETE);
    }

    if (!task_n_tasks) {
	task_collect();
	trace(TR_TASK, 0, "task_delete: Removed last task, exiting");
	task_quit(0);
    } else {
	int last_count = 0;
	static term_last = 0;

	if (!term_last) {
	    
	    TASK_TABLE(tp) {
		if (BIT_TEST(tp->task_flags, TASKF_LAST)) {
		    last_count++;
		}
	    } TASK_TABLE_END(tp) ;

	    if (last_count == task_n_tasks) {
		term_last++;

		/* Update the kernel with any changes made */
		rt_flash_update(TRUE);
		
		TASK_TABLE(tp) {
		    if (tp->task_terminate) {
			trace(TR_TASK, 0, "task_terminate: terminating task %s",
			      task_name(tp));
			TASK_SAVE(tp, tp->task_terminate(tp));
			trace(TR_TASK, 0, NULL);
		    } 
		} TASK_TABLE_END(tp);
	    }
	}
    }
}


/**/

/* Deal with flash updates */

static block_t crash_block_index;

struct crash_entry {
    struct crash_entry *crash_next;
    task *crash_task;
    void_t crash_data;
    _PROTOTYPE(crash_job,
	       void,
	       (task *,
		void_t));
};

struct crash_entry *crash_list = 0;

void
task_crash_add(tp, job, data)
task *tp;
_PROTOTYPE(job,
	   void,
	   (task *,
	    void_t));
void_t data;
{
    struct crash_entry *cp = (struct crash_entry *) task_block_alloc(crash_block_index);

    cp->crash_task = tp;
    cp->crash_job = job;
    cp->crash_data = data;

    if (crash_list) {
	struct crash_entry *cp1 = crash_list;

	while (cp1->crash_next) {
	    cp1 = cp1->crash_next;
	}

	cp1->crash_next = cp;
    } else {
	crash_list = cp;
    }
}


/* Process any queued crash requests */
void
task_crash __PF0(void)
{
    register struct crash_entry *cp = crash_list;

    crash_list = (struct crash_entry *) 0;

    while (cp) {
	register struct crash_entry *cp1 = cp->crash_next;
	register task *tp1 = cp->crash_task;

	trace(TR_TASK, 0, "task_crash: calling after flash routine for %s",
	      task_name(tp1));
	TASK_SAVE(tp1, cp->crash_job(tp1, cp->crash_data));
	trace(TR_TASK, 0, "task_crash: return from after flash routine for %s",
	      task_name(tp1));

	task_block_free(crash_block_index, (void_t) cp);

	cp = cp1;
    } ;
}


int
task_flash __PF2(change_list, rt_list *,
		 flash, int)
{
    task *tp;
    const char *type;
    flag_t state;

    if (flash) {
	/* Flash update */

	type = "flash update";
	state = TASKS_FLASH;
    } else {
	/* New policy */

	type = "new policy";
	state = TASKS_NEWPOLICY;
    }

    /* Change state */
    task_newstate(state, 0);

    TASK_TABLE(tp) {
	if (tp->task_flash) {
	    trace(TR_TASK, 0, "task_flash: calling %s routine for %s",
		  type,
		  task_name(tp));
	    if (flash) {
		TASK_SAVE(tp, tp->task_flash(tp, change_list));
	    } else {
		TASK_SAVE(tp, tp->task_newpolicy(tp, change_list));
	    }
	    trace(TR_TASK, 0, "task_flash: return from %s routine for %s",
		  type,
		  task_name(tp));
	}
    } TASK_TABLE_END(tp);

    /* Reset state */
    task_newstate(0, state);

    return crash_list ? TRUE : FALSE;
}


/*
 *	Allocate a task block with the specified name
 */
task *
task_alloc __PF2(name, const char *,
		 priority, int)
{
    task *tp;

    tp = (task *) task_block_alloc(task_block_index);
    tp->task_name = name;
    tp->task_terminate = task_delete;
    tp->task_socket = -1;
    tp->task_priority = priority;

    trace(TR_TASK, 0, "task_alloc: allocated task block for %s priority %d",
	  tp->task_name,
	  tp->task_priority);
    return tp;
}


/*
 *	Allocate an output buffer
 */
void
task_alloc_send __PF2(tp, task *,
		      maxsize, size_t)
{
    if (maxsize > task_send_buffer_len) {
	/* This request is larger than previous requests */

	/* Round it up to a page size */
	maxsize = ROUNDUP(maxsize, task_pagesize);

	if (task_send_buffer) {
	    /* Free the old buffer */
	    task_block_reclaim(task_send_buffer_len, task_send_buffer);
	}

	/* Allocate send buffer */
	task_send_buffer = (void_t) task_block_malloc(maxsize);

	trace(TR_TASK, 0, "task_alloc_send: send buffer size increased to %d from %d by %s",
	      maxsize,
	      task_send_buffer_len,
	      task_name(tp));
	task_send_buffer_len = maxsize;
    }
}


/*
 *	Allocate an input buffer
 */
void
task_alloc_recv __PF2(tp, task *,
		      maxsize, size_t)
{
    if (maxsize > task_recv_buffer_len) {
	/* This request is larger than previous requests */

	/* Round it up to a page size */
	maxsize = ROUNDUP(maxsize, task_pagesize);

	if (task_recv_buffer) {
	    /* Free the old buffer */
	    task_block_reclaim(task_recv_buffer_len, task_recv_buffer);
	}

	/* Allocate recv buffer */
	task_recv_buffer = (void_t) task_block_malloc(maxsize);

	trace(TR_TASK, 0, "task_alloc_recv: recv buffer size increased to %d from %d by %s",
	      maxsize,
	      task_recv_buffer_len,
	      task_name(tp));
	task_recv_buffer_len = maxsize;
    }
}


/*
 *	Build a task block and add to the linked list
 */
int
task_create __PF1(tp, task *)
{

    if (tp->task_socket != -1) {
	task_set_socket(tp, tp->task_socket);
    }

    if (task_head.task_forw == &task_head ||
	tp->task_priority < task_head.task_forw->task_priority) {
	/* This is the only task or a lower priority than the first one */
	insque((struct qelem *) tp,
	       (struct qelem *) &task_head);
    } else if (tp->task_priority > task_head.task_back->task_priority) {
	/* Highest priority */
	insque((struct qelem *) tp,
	       (struct qelem *) task_head.task_back);
    } else {
	task *tp2 = task_head.task_forw;

	while (tp->task_priority > tp2->task_priority) {
	    tp2 = tp2->task_forw;
	}

	/* Insert before this element */
	insque((struct qelem *) tp,
	       (struct qelem *) tp2->task_back);
    }
	
    task_n_tasks++;

    tracef("task_create: %s",
	   task_name(tp));

    if (tp->task_proto) {
	tracef("  proto %d",
	       tp->task_proto);
    }
#ifdef	PROTO_INET
    if (tp->task_addr && tp->task_addr->in.sin_port) {
	tracef("  port %d",
	       ntohs(tp->task_addr->in.sin_port));
    }
#endif	/* PROTO_INET */
    if (tp->task_socket != -1) {
	tracef("  socket %d",
	       tp->task_socket);
    }
    if (tp->task_rtproto) {
	tracef("  rt_proto <%s>",
	       trace_state(rt_proto_bits, tp->task_rtproto));
    }
    trace(TR_TASK, 0, NULL);

    return 1;
}


#ifndef	NO_FORK
/*
 * Terminate a subprocess
 */
static void
task_kill  __PF1(tp, task *)
{
    kill(tp->task_pid, SIGTERM);
}


/*
 * Spawn a process and create a task for it.
 */
int
task_fork __PF1(tp, task *)
{
    int rc = 0;

    switch (tp->task_pid = fork()) {
    case 0:
	/* Child */
	tp->task_pid = task_pid = getpid();
	(void) sprintf(task_pid_str, "%d", task_pid);

	trace(TR_TASK, 0, "task_fork: %s forked",
	      task_name(tp));

	if (tp->task_process) {
	    TASK_SAVE(tp, tp->task_process(tp));
	}

	timer_peek();
	
	trace(TR_TASK, 0, "task_fork: %s exiting",
	      task_name(tp));
	_exit(0);

    case -1:
	/* Error */
	trace(TR_ALL, LOG_ERR, "task_fork: could not fork %s: %m",
	      task_name(tp));
	task_delete(tp);
	break;
	
    default:
	/* Parent */
	tp->task_terminate = task_kill;
	rc = task_create(tp);
    }

    return rc;
}
#endif	/* NO_FORK */


/*  */

int
task_ioctl __PF4(fd, int,
		 cmd, u_int,
		 data, void_t,
		 len, int)
{
    int rc;
    
#ifdef	USE_STREAMIO
    struct strioctl si;
 
    si.ic_cmd = cmd;
    si.ic_timout = 0;
    si.ic_len = len;
    si.ic_dp = (caddr_t) data;
    
    NON_INTR(rc, ioctl(fd, I_STR, &si));
#else	/* USE_STREAMIO */
    NON_INTR(rc, ioctl(fd, cmd, data));
#endif	/* USE_STREAMIO */

    return rc;
}
/*  */

#ifdef	STDIO_HACK
/*
 *  A hack to catch stdio and stderr output when gated is running as a daemon.
 */

static void
task_stdio_read __PF1(tp, task *)
{
    register char *cp, *ep;
    register char *sp = task_get_recv_buffer(char *);
    int len = task_recv_buffer_len;

    while (TRUE) {
	/* Read until there is not more to read */
	if ((len = read(tp->task_socket,
			sp,
			len)) < 0) {
	    switch (errno) {
	    case EWOULDBLOCK:
#if	defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	    case EAGAIN:		/* System V style */
#endif	/* EAGAIN */
		break;

	    case EINTR:
		/* The system call was interrupted, probably by */
		/* a signal.  Silently retry */
		continue;
		
	    default:
		trace(TR_ALL, LOG_INFO, "task_stdio_read: read: %m");
	    }
	    return;
	}

	assert(len);
	
	if (len < task_recv_buffer_len) {
	    sp[len] = (char) 0;
	}
	ep = (cp = sp) + len;

	/* Print each line seperately */
	while (cp < ep) {
	    switch (*cp) {
	    case '\n':
		trace(TR_ALL, 0, "stdio: %.*s",
		      cp - sp,
		      sp);
		sp = ++cp;
		break;
		
	    default:
		cp++;
	    }
	}

	/* Print any remaining */
	if (cp > sp) {
	    trace(TR_ALL, 0, "stdio: %.*s",
		  cp - sp,
		  sp);
	}
    }
}
	
	
static void
task_stdio_init __PF0(void)
{
    int fd[2];
    task *tp;

    if (pipe(fd) < 0) {
	trace(TR_ALL, LOG_WARNING, "task_stdio: pipe: %m");
	return;
    }

    tp = task_alloc("stdio", 0);
    tp->task_socket = fd[0];
    tp->task_recv = task_stdio_read;
    if (!task_create(tp)) {
    close_up:
	(void) close(fd[0]);
	(void) close(fd[1]);
	return;
    }

    if (task_set_option(tp,
			TASKOPTION_NONBLOCKING,
			TRUE) < 0) {
	goto close_up;
    }
    
    if (dup2(fd[1], fileno(stdout)) < 0) {
	trace(TR_ALL, LOG_INFO, "task_stdio_init: dup2(%d, stdout): %m",
	      fd[1]);
	goto close_up;
    }
    
    if (dup2(fd[1], fileno(stderr)) < 0) {
	trace(TR_ALL, LOG_INFO, "task_stdio_init: dup2(%d, stderr): %m",
	      fd[1]);
	goto close_up;
    }

    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    task_alloc_recv(tp, BUFSIZ);

}
#endif	/* STDIO_HACK */


/**/
void
task_set_socket __PF2(tp, task *,
		      s, int)
{
    tp->task_socket = s;

    trace(TR_TASK, 0, "task_set_socket: task %s socket %d",
	  task_name(tp),
	  tp->task_socket);

    if (tp->task_socket > task_max_socket) {
	task_max_socket = tp->task_socket;
    }

    if (tp->task_recv || tp->task_accept) {
	FD_SET(tp->task_socket, &task_select_readbits);
    } else {
	FD_CLR(tp->task_socket, &task_select_readbits);
    }

    if (tp->task_write || tp->task_connect) {
	FD_SET(tp->task_socket, &task_select_writebits);
    } else {
	FD_CLR(tp->task_socket, &task_select_writebits);
    }
	
    if (tp->task_except) {
	FD_SET(tp->task_socket, &task_select_exceptbits);
    } else {
	FD_CLR(tp->task_socket, &task_select_exceptbits);
    }

}


void
task_reset_socket __PF1(tp, task *)
{

    trace(TR_TASK, 0, "task_reset_socket: task %s socket %d",
	  task_name(tp),
	  tp->task_socket);

    FD_CLR(tp->task_socket, &task_select_readbits);
    FD_CLR(tp->task_socket, &task_select_writebits);
    FD_CLR(tp->task_socket, &task_select_exceptbits);

    /* Delete from socket to task table if no routines present */

    tp->task_socket = -1;
    BIT_RESET(tp->task_flags, TASKF_CONNECT | TASKF_ACCEPT);
    tp->task_recv = 0;
    tp->task_accept = 0;
    tp->task_write = 0;
    tp->task_connect = 0;
    tp->task_except = 0;
}


/*
 *	task_addr_local - Set local socket address (bind())
 */
int
task_addr_local __PF2(tp, task *,
		      ap, sockaddr_un *)
{
    int rc = 0;
    int len;
    struct sockaddr *addr = sock2unix(ap, &len);
    
    tracef("task_addr_local: task %s address %#A",
	   task_name(tp),
	   ap);

    if (!BIT_TEST(task_state, TASKS_TEST) &&
	(bind(tp->task_socket, addr, len) < 0)) {
	rc = errno;
	trace(TR_ALL, LOG_ERR, ": %m");
    }

    trace(TR_TASK, 0, NULL);

    return errno = rc;	/* Reset errno */
}


/*
 *	task_connect - Set remote socket address (bind())
 */
int
task_connect __PF1(tp, task *)
{
    int rc = 0;
    int len;
    struct sockaddr *addr = sock2unix(tp->task_addr, &len);
    
    tracef("task_connect: task %s addr %#A",
	   task_name(tp),
	   sock2gated(addr, len));

    if (!BIT_TEST(task_state, TASKS_TEST) &&
	(connect(tp->task_socket, addr, len) < 0)) {
	rc = errno;
	trace(TR_TASK, 0, ": %m");
    }

    trace(TR_TASK, 0, NULL);

    return errno = rc;	/* Reset errno */
}


/*
 *	task_socket_options - Sets socket options.  Isolates protocols from system layer.
 */
#ifdef	STDARG
/*VARARGS2*/
int
task_set_option(task *tp, int option, ...)
#else	/* STDARG */
/*ARGSUSED*/
/*VARARGS0*/
int
task_set_option(va_alist)
va_dcl
#endif	/* STDARG */
{
    int opt;
    int rc = 0;
    int value_int;
    int len = sizeof(value_int);
    void_t ptr = (void_t) & value_int;
#ifdef	IP_MULTICAST
    byte value_uchar;
    u_int request;
#endif	/* IP_MULTICAST */
    va_list ap;
#ifndef	LINGER_NO_PARAM
    struct linger linger;
#endif	/* LINGER_NO_PARAM */
    int level = SOL_SOCKET;
#ifdef	STDARG
    va_start(ap, option);
#else	/* STDARG */
    task *tp;
    int option;

    va_start(ap);

    tp = va_arg(ap, task *);
    option = va_arg(ap, int);
#endif	/* STDARG */

    tracef("task_set_option: task %s socket %d option %s(%d)",
	   task_name(tp),
	   tp->task_socket,
	   trace_state(task_socket_options, option),
	   option);

    switch (option) {
    case TASKOPTION_RECVBUF:
#ifdef	SO_RCVBUF
	opt = SO_RCVBUF;
	goto int_value;
#else	/* SO_RCVBUF */
	break;
#endif	/* SO_RCVBUF */

    case TASKOPTION_SENDBUF:
#ifdef	SO_SNDBUF
	opt = SO_SNDBUF;
	goto int_value;
#else	/* SO_SNDBUF */
	break;
#endif	/* SO_SNDBUF */

    case TASKOPTION_LINGER:
	opt = SO_LINGER;
#ifndef	LINGER_NO_PARAM
	linger.l_linger = va_arg(ap, int);
	linger.l_onoff = linger.l_linger ? TRUE : FALSE;
	ptr = (void_t) &linger;
	len = sizeof(struct linger);
	tracef(" value { %d, %d }",
	       linger.l_linger,
	       linger.l_onoff);
#else	/* LINGER_NO_PARAM */
	ptr = 0;
	len = 0;
#endif	/* LINGER_NO_PARAM */
	goto setsocketopt;

    case TASKOPTION_REUSEADDR:
	opt = SO_REUSEADDR;
	goto int_value;

    case TASKOPTION_BROADCAST:
#ifdef	SO_BROADCAST
	opt = SO_BROADCAST;
	goto int_value;
#else	/* SO_BROADCAST */
	break;
#endif	/* SO_BROADCAST */

    case TASKOPTION_DONTROUTE:
	opt = SO_DONTROUTE;
	goto int_value;

    case TASKOPTION_KEEPALIVE:
	opt = SO_KEEPALIVE;
	goto int_value;

    case TASKOPTION_DEBUG:
	opt = SO_DEBUG;
	goto int_value;

    case TASKOPTION_USELOOPBACK:
	opt = SO_USELOOPBACK;
	goto int_value;

#ifdef	IP_MULTICAST
    case TASKOPTION_GROUP_ADD:
	level = IPPROTO_IP;
	opt = IP_ADD_MEMBERSHIP;
	goto ip_mreq_value;

    case TASKOPTION_GROUP_DROP:
	level = IPPROTO_IP;
	opt = IP_DROP_MEMBERSHIP;

    ip_mreq_value:
        {
	    if_addr *ifap;
	    struct ip_mreq mreq;

	    ifap = va_arg(ap, if_addr *);
	    if (ifap) {
		tracef(" interface %A(%s)",
		       ifap->ifa_addr,
		       ifap->ifa_name);
		if (!BIT_TEST(ifap->ifa_state, IFS_MULTICAST)) {
		    tracef(": multicast not supported on this interface");
		    break;
		}
		mreq.imr_interface = sock2in(ifap->ifa_addr);		/* struct copy */
	    } else {
		mreq.imr_interface = sock2in(inet_addr_default);
	    }

	    mreq.imr_multiaddr = sock2in(va_arg(ap, sockaddr_un *));	/* struct copy */
	    tracef(" group %A",
		   sockbuild_in(0, mreq.imr_multiaddr.s_addr));
	    ptr = (void_t) &mreq;
	    len = sizeof (mreq);
	}
	goto setsocketopt;
	    
    case TASKOPTION_MULTI_IF:
        {
	    if_addr *ifap;

	    level = IPPROTO_IP;
	    opt = IP_MULTICAST_IF;
	    ifap = va_arg(ap, if_addr *);
	    ptr = (void_t) &sock2in(ifap->ifa_addr);
	    len = sizeof (sock2in(ifap->ifa_addr));
	    tracef(" interface %A(%s)",
		   ifap->ifa_addr,
		   ifap->ifa_name);
	}
	goto setsocketopt;

    case TASKOPTION_MULTI_LOOP:
	level = IPPROTO_IP;
	opt = IP_MULTICAST_LOOP;
	value_uchar = va_arg(ap, int);
	ptr = (void_t) &value_uchar;
	len = sizeof (value_uchar);
	tracef(" loop %d",
	       value_uchar);
	goto setsocketopt;

    case TASKOPTION_MULTI_TTL:
	level = IPPROTO_IP;
	opt = IP_MULTICAST_TTL;
	value_uchar = va_arg(ap, int);
	ptr = (void_t) &value_uchar;
	len = sizeof (value_uchar);
	tracef(" ttl %d",
	       value_uchar);
	goto setsocketopt;

    case TASKOPTION_MULTI_ADD:
	request = SIOCADDMULTI;
	goto multi_adddrop;

    case TASKOPTION_MULTI_DROP:
	request = SIOCDELMULTI;

    multi_adddrop:
        {
	    int s = task_get_socket(AF_INET, SOCK_DGRAM, 0);	/* Some systems require a protocol socket */
	    if_addr *ifap;
	    sockaddr_un *group;
	    struct ifreq ifr;
	    struct sockaddr_in *addr = (struct sockaddr_in *) ((void_t) &ifr.ifr_addr);

	    bzero ((caddr_t) &ifr, sizeof (ifr));

	    ifap = va_arg(ap, if_addr *);
	    group = va_arg(ap, sockaddr_un *);
	    tracef(" interface %A(%s) group %A",
		   ifap->ifa_addr,
		   ifap->ifa_name,
		   group);

	    /* Add unit name */
	    strncpy(ifr.ifr_name, ifap->ifa_name, IFNAMSIZ);
	    /* Add group */
	    addr->sin_family = AF_INET;
#ifdef	SOCKET_LENGTHS
	    addr->sin_len = sizeof (*addr);
#endif	/* SOCKET_LENGTHS */
	    addr->sin_addr = sock2in(group);

	    rc = task_ioctl(s, request, (caddr_t) &ifr, sizeof (ifr));

	    (void) close(s);
	}
	break;

    case TASKOPTION_MULTI_ROUTE:
#ifdef	IP_MULTICAST_ROUTING
	level = IPPROTO_IP;
	value_int = va_arg(ap, int);
	opt = value_int ? DVMRP_INIT : DVMRP_DONE;
	ptr = (void_t) 0;
	len = 0;
	tracef(" %s",
	       value_int ? "On" : "Off");
	goto setsocketopt;
#else	/* IP_MULTICAST_ROUTING */
	tracef(": not supported");
	break;
#endif	/* IP_MULTICAST_ROUTING */

#else	/* IP_MULTICAST */
    case TASKOPTION_GROUP_ADD:
    case TASKOPTION_GROUP_DROP:
    case TASKOPTION_MULTI_IF:
    case TASKOPTION_MULTI_LOOP:
    case TASKOPTION_MULTI_TTL:
    case TASKOPTION_MULTI_ADD:
    case TASKOPTION_MULTI_DROP:
    case TASKOPTION_MULTI_ROUTE:
	tracef(": not supported");
	break;
	    
#endif	/* IP_MULTICAST */

    case TASKOPTION_TTL:
#ifdef	IP_TTL
	opt = IP_TTL;
	level = IPPROTO_IP;
	goto int_value;
#else	/* IP_TTL */
	tracef(": not supported");
	break;
#endif	/* IP_TTL */

    case TASKOPTION_TOS:
#ifdef	IP_TOS
	opt = IP_TOS;
	level = IPPROTO_IP;
	goto int_value;
#else	/* IP_TOS */
	tracef(": not supported");
	break;
#endif	/* IP_TOS */

    case TASKOPTION_RCVDSTADDR:
#ifdef	IP_RECVDSTADDR
	opt = IP_RECVDSTADDR;
	level = IPPROTO_IP;
	goto int_value;
#else	/* IP_RECVDSTADDR */
	tracef(": not supported");
	break;
#endif	/* IP_RECVDSTADDR */

    case TASKOPTION_IPHEADER_INC:
#ifdef IP_HDRINCL
	opt = IP_HDRINCL;
	level = IPPROTO_IP;
	goto int_value;
#else	/* IP_HDRINCL */
	tracef(": not supported");
	break;
#endif	/* IP_HDRINCL */

    int_value:
	value_int = va_arg(ap, int);
	tracef(" value %d",
	       value_int);
	goto setsocketopt;

    setsocketopt:
	if (!BIT_TEST(task_state, TASKS_TEST)) {
	    rc = setsockopt(tp->task_socket, level, opt, ptr, len);
	}
	break;

    case TASKOPTION_NONBLOCKING:
	value_int = va_arg(ap, int);
	tracef(" value %d",
	       value_int);
	if (!BIT_TEST(task_state, TASKS_TEST)) {
#ifdef	FIONBIO
	    rc = task_ioctl(tp->task_socket, FIONBIO, (caddr_t) &value_int, sizeof (value_int));
#else	/* FIONBIO */
	    NON_INTR(rc, fcntl(tp->task_socket, F_SETFL, O_NDELAY));
#endif	/* FIONBIO */
	}
	break;

    default:
	rc = -1;
	errno = EINVAL;
    }

    if (rc < 0) {
	trace(TR_ALL, LOG_ERR, ": %m");
    } else {
	trace(TR_TASK, 0, NULL);
    }

    va_end(ap);
    return rc;
}


/*  */
/*
 *	Memory allocation
 */

void_t
task_mem_malloc __PF2(tp, task *,
		      size, size_t)
{
    void_t p;

    p = (void_t) malloc(size);
    if (!p) {
	trace(TR_ALL, LOG_ERR, "task_mem_malloc: Can not malloc(%d) for %s",
	      size,
	      task_name(tp));
	task_quit(ENOMEM);
    }

    return p;
}


void_t
task_mem_calloc __PF3(tp, task *,
		      number, u_int,
		      size, size_t)
{
    void_t p;

    p = (void_t) calloc(number, size);
    if (!p) {
	trace(TR_ALL, LOG_ERR, "task_mem_calloc: Can not calloc(%d, %d) for %s",
	      number,
	      size,
	      task_name(tp));
	task_quit(ENOMEM);
    }

    return p;
}


void_t
task_mem_realloc __PF3(tp, task *,
		       p, void_t,
		       size, size_t)
{
    p = (void_t) realloc(p, size);
    if (!p) {
	trace(TR_ALL, LOG_ERR, "task_mem_realloc: Can not realloc( ,%d) for %s",
	      size,
	      task_name(tp));
	task_quit(ENOMEM);
    }

    return p;
}


char *
task_mem_strdup __PF2(tp, task *,
		      str, const char *)
{
    return strcpy((char *) task_mem_malloc(tp, (size_t) (strlen(str) + 1)), str);
}


/*ARGSUSED*/
void
task_mem_free __PF2(tp, task *,
		    p, void_t)
{
    if (p) {
	free((caddr_t) p);
    }
}


/**/

#ifdef	MEMORY_ALIGNMENT
#define	TASK_BLOCK_ALIGN	MEMORY_ALIGNMENT
#else	/* MEMORY_ALIGNMENT */
#define	TASK_BLOCK_ALIGN	sizeof (unsigned long)
#endif	/* MEMORY_ALIGNMENT */

static struct task_block task_blocks;		/* List of block sizes */
static struct task_block task_block_blocks;	/* list entries */
static struct task_block task_block_page;	/* Page size */

block_t task_block_pagesize = 0;		/* Index for pagesize blocks */

block_t
task_block_init __PF1(size, register size_t)
{
    register block_t tbp = task_blocks.forw;
    register int count;
    size_t runt;

    /* Round up to the minimum size */
    size = ROUNDUP(size, TASK_BLOCK_ALIGN);

    count = task_pagesize / size;
    if (count < 1) {
    	/* Greater than a page, allocate one at a time */
	count = 1;
    }
    runt = (task_pagesize - ((size * count) % task_pagesize)) % task_pagesize;

    /* Look for this size in current list */
    do {
	if (tbp->size == size) {
	    /* Found the block */

	    goto Return;
	} else if (size < tbp->size) {
	    /* Insert before this one */

	    break;
	}
    } while ((tbp = tbp->forw) != &task_blocks) ;

    tbp = tbp->back;

    /* Allocate a block before this element */
    insque((struct qelem *) task_block_alloc(&task_block_blocks),
	   (struct qelem *) tbp);
    tbp = tbp->forw;

    /* Fill in the info */
    tbp->size = size;
    tbp->count = count;

    /* Allocate a block for the runt */
    if (runt) {
	tbp->runt = task_block_init(runt);
    }
    
 Return:
    tbp->n_req_init++;

    return tbp;
}


static void_t
task_block_sbrk __PF1(size, size_t)
{
    size_t my_size = ROUNDUP(size, task_pagesize);
    void_t vp = (void_t) sbrk(my_size);

    if (vp == (caddr_t) -1) {
	int error = errno;
	
	trace(TR_ALL, LOG_ERR, "task_block_sbrk: %m");
	task_quit(error);
    }

    bzero(vp, my_size);
    return vp;
}


void_t
task_block_malloc __PF1(size, size_t)
{
    if (size > task_pagesize) {
	return task_block_sbrk(size);
    } else {
	return task_block_alloc(task_block_pagesize);
    }
}


void
task_block_reclaim __PF2(size, size_t,
			 p, void_t)
{
    int count = ROUNDUP(size, task_pagesize) / task_pagesize;

    while (count--) {
	task_block_free(task_block_pagesize, p);
	p = (void_t) ( (caddr_t) p + task_pagesize);
    }
}


struct qelem *
task_block__alloc __PF1(tbp, register block_t)
{
    register struct qelem *cp;

    /* Allocate another block of structures */
    register u_int n = tbp->count;
    register size_t size = n*tbp->size;
    register union {
	struct qelem *qp;
	caddr_t cp;
    } np;

    if (size > task_pagesize ||
	tbp == task_block_pagesize ||
	!task_block_pagesize->free) {
	/* Trying to get more pagesize blocks, just sbrk them */

	np.cp = (caddr_t) task_block_sbrk(size);
    } else {
	/* Getting one page, recurse */
	np.cp = (caddr_t) task_block_alloc(task_block_pagesize);
    }
    cp = np.qp;

    tbp->n_free += n;

    while (--n) {
	register struct qelem *qp = np.qp;
	    
	np.cp += tbp->size;
	qp->q_forw = np.qp;
    }

    if (tbp->runt) {
	/* Find a home for the runt */

	np.cp += tbp->size;
	np.qp->q_forw = tbp->runt->free;
	tbp->runt->free = np.qp;
	tbp->runt->n_free++;
    }

    return cp;
}


static void
task_block_init_all __PF0(void)
{
    size_t runt;

    task_block_page.size = task_pagesize;
    task_block_page.count = 1;
    task_block_page.n_req_init++;
    task_block_pagesize = &task_block_page;

    task_block_blocks.count = task_pagesize / (task_block_blocks.size = ROUNDUP(sizeof (struct task_block), TASK_BLOCK_ALIGN));
    task_block_blocks.n_req_init++;

    /* Init the linked list */
    task_blocks.forw = &task_block_blocks;
    task_blocks.back = &task_block_page;
    task_block_blocks.forw = &task_block_page;
    task_block_blocks.back = &task_blocks;
    task_block_page.forw = &task_blocks;
    task_block_page.back = &task_block_blocks;

    if (runt = task_pagesize - (task_block_blocks.size * task_block_blocks.count)) {
	task_block_blocks.runt = task_block_init(runt);
    }

}


static void
task_block_dump __PF1(fp, FILE *)
{
    register block_t tbp = task_blocks.forw;

    fprintf(fp, "Task Blocks:\n\tAllocation size: %5d\n\n",
	    task_pagesize);

    do {
	fprintf(fp, "\tSize: %5d\tLastAlloc: %08X",
		tbp->size,
		tbp->tmp);
	fprintf(fp, "\tN_free: %d\tInitReq: %d\tAllocReq: %d\tFreeReq: %d\n",
		tbp->n_free,
		tbp->n_req_init,
		tbp->n_req_alloc,
		tbp->n_req_free);
    } while ((tbp = tbp->forw) != &task_blocks) ;

    fprintf(fp, "\n");
}


/**/


/*
 * task_get_socket gets a socket, retries later if no buffers at present
 */

int
task_get_socket __PF3(domain, int,
		      type, int,
		      proto, int)
{
    int retry, get_socket, error;

    tracef("task_get_socket: domain AF_%s  type SOCK_%s  protocol %d",
	  trace_value(task_domain_bits, domain),
	  trace_value(task_socket_types, type),
	  proto);

    if (BIT_TEST(task_state, TASKS_TEST)) {
	if (task_max_socket) {
	    get_socket = ++task_max_socket;
	} else {
	    /* Skip first few that may be used for logging */
	    get_socket = (task_max_socket = 2);
	}
    } else {
	retry = 2;			/* if no buffers a retry might work */
	while ((get_socket = socket(domain, type, proto)) < 0 && retry--) {
	    error = errno;
	    trace(TR_ALL, LOG_ERR, ": %m");
	    if (error == ENOBUFS) {
		(void) sleep(5);
	    } else if (error == EINTR) {
		/* The system call was interrupted, probably by */
		/* a signal.  Silently retry */
		retry++;
		continue;
	    } else {
		break;
	    }
	}
    }

    if (get_socket >= 0) {
	trace(TR_TASK, 0, "  socket %d",
	      get_socket);
    }

    return get_socket;
}


/*
 *	Who are we connected to?
 */
sockaddr_un *
task_get_addr_remote __PF1(tp, task *)
{
    caddr_t name[MAXPATHLEN+2];
    int namelen = sizeof name;

    if (getpeername(tp->task_socket,
		    (struct sockaddr *) name,
		    &namelen) < 0) {
	switch (errno) {
	case ENOTCONN:
	    /* Ignore */
	    break;

	default:
	    trace(TR_ALL, LOG_ERR, "task_get_addr_remote: getpeername(%s): %m",
		  task_name(tp));
	}
	return (sockaddr_un *) 0;
    }

    return sock2gated((struct sockaddr *) name, namelen);
}


sockaddr_un *
task_get_addr_local __PF1(tp, task *)
{
    caddr_t name[MAXPATHLEN+2];
    int namelen = sizeof name;

    if (getsockname(tp->task_socket,
		    (struct sockaddr *) name,
		    &namelen) < 0) {
	switch (errno) {
	case ENOTCONN:
	    /* Ignore */
	    break;

	default:
	    trace(TR_ALL, LOG_ERR, "task_get_addr_remote: getpeername(%s): %m",
		  task_name(tp));
	}
	return (sockaddr_un *) 0;
    }

    return sock2gated((struct sockaddr *) name, namelen);
}


/*
 *	Path names
 */
#ifndef	vax11c
char *
task_getwd __PF0(void)
{
    static char path_name[MAXPATHLEN];
    
    if (!getwd(path_name)) {
	trace(TR_ALL, LOG_ERR, "task_getwd: getwd: %s",
	      path_name);
	/* In case of system error, pick a default */
	(void) strcpy(path_name, "/");
    }

    return path_name;
}


int
task_chdir __PF1(path_name, const char *)
{
    int rc;
    
    if (rc = chdir(path_name)) {
	trace(TR_ALL, LOG_ERR, "task_cwd: chdir: %m");
    }

    task_mem_free((task *) 0, task_path_now);
    task_path_now = task_mem_strdup((task *) 0, task_getwd());

    return rc;
}
#endif	/* vax11c */

/**/
/* IGP-EGP interaction */

/* A BGP will register an AUX connection indicating the protocol it wishes */
/* to interact with.  If a task for that IGP exists, the register routine */
/* is called.  If not, the IGP will call task_aux_lookup when it starts. */

static aux_proto aux_head = { &aux_head, &aux_head };

static block_t task_aux_index = 0;

#define	AUX_LIST(auxp)	for (auxp = aux_head.aux_forw; auxp != &aux_head; auxp = auxp->aux_forw)
#define	AUX_LIST_END(auxp)	if (auxp == &aux_head) auxp = (aux_proto *) 0

/* EGP is ready, register it's connection and make the connection if the */
/* IGP exists */    
void
task_aux_register(tp, proto, initiate, terminate, flash, newpolicy)
task *tp;
u_int proto;
_PROTOTYPE(terminate,
	   void,
	   (task *));
_PROTOTYPE(initiate,
    void,
    (task *,
     proto_t,
     u_int));
_PROTOTYPE(flash,
    void,
    (task *,
     rt_list *));
_PROTOTYPE(newpolicy,
    void,
    (task *,
     rt_list *));
{
    register aux_proto *auxp;
    register task *atp;

    /* Duplicate check */
    AUX_LIST(auxp) {
	/* Duplicate registration */
	assert(auxp->aux_task_egp != tp &&
	       auxp->aux_proto_igp != proto);
    } AUX_LIST_END(auxp) ;

    /* Allocate, fill in and add to list */
    auxp = (aux_proto *) task_block_alloc(task_aux_index);

    auxp->aux_task_egp = tp;
    auxp->aux_proto_igp = proto;
    auxp->aux_initiate = initiate;
    auxp->aux_terminate = terminate;
    auxp->aux_flash = flash;
    auxp->aux_newpolicy = newpolicy;

    insque((struct qelem *) auxp,
	   (struct qelem *) aux_head.aux_back);

    /* Find the task and notify */
    TASK_TABLE(atp) {
	if (atp->task_aux_proto == proto) {
	    atp->task_aux = auxp;
	    auxp->aux_task_igp = atp;
	    assert(atp->task_aux_register);
	    atp->task_aux_register(atp, auxp);
	    break;
	}
    } TASK_TABLE_END(atp) ;
}


/* EGP is going away, sever the connection */
void
task_aux_unregister __PF1(tp, task *)
{
    register aux_proto *auxp;

    AUX_LIST(auxp) {
	if (auxp->aux_task_egp == tp) {
	    task *atp = auxp->aux_task_igp;
	    
	    atp->task_aux_register(atp, (aux_proto *) 0);

	    auxp->aux_task_igp = (task *) 0;
	    atp->task_aux = (aux_proto *) 0;

	    /* Remove from queue and free */
	    remque((struct qelem *) auxp);

	    task_block_free(task_aux_index, (void_t) auxp);

	    return;
	}
    } AUX_LIST_END(auxp) ;

    /* Aux not found for task */
    assert(FALSE);
}


/* IGP is operational and looking for an EGP to interact with */
void
task_aux_lookup __PF1(tp, task *)
{
    register aux_proto *auxp;

    assert(tp->task_aux_register);
    
    AUX_LIST(auxp) {
	if (tp->task_aux_proto == auxp->aux_proto_igp) {
	    tp->task_aux = auxp;
	    auxp->aux_task_igp = tp;
	    tp->task_aux_register(tp, auxp);
	    break;
	}
    } AUX_LIST_END(auxp) ;
}


/* IGP is going down, notify the EGP and break the connection */
void
task_aux_terminate __PF1(tp, task *)
{
    aux_proto *auxp = tp->task_aux;

    if (auxp) {
	assert(tp == auxp->aux_task_igp);
	
	auxp->aux_terminate(auxp->aux_task_egp);
	auxp->aux_task_igp = (task *) 0;
	tp->task_aux = (aux_proto *) 0;
    }
}


static void
task_aux_dump __PF1(fp, FILE *)
{
    register aux_proto *auxp;

    if (aux_head.aux_forw == &aux_head) {
	return;
    }

    (void) fprintf(fp, "\t\tIGP-EGP interactions:\n\n");
    
    AUX_LIST(auxp) {
	(void) fprintf(fp, "\t\tEGP task: %s\tIGP proto: %s",
		       task_name(auxp->aux_task_egp),
		       trace_state(rt_proto_bits, auxp->aux_proto_igp));
	if (auxp->aux_task_igp) {
	    (void) fprintf(fp, "\tIGP task: %s\n",
			   task_name(auxp->aux_task_igp));
	} else {
	    (void) fprintf(fp, "\n");
	}
    } AUX_LIST_END(auxp) ;

    (void) fprintf(fp, "\n");
}


/**/


/*
 * task_init()   set up for receiving signals and other initialization
 */
void
task_init1 __PF1(name, char *)
{
    char 	hostname[MAXHOSTNAMELENGTH + 1];

    if (getppid() == 1) {
	/* We were probably started from /etc/inittab */
	task_newstate(TASKS_NODAEMON, 0);
    }

#ifdef	_SC_PAGE_SIZE
    task_pagesize = sysconf(_SC_PAGE_SIZE);
#else	/* _SC_PAGE_SIZE */
    task_pagesize = getpagesize();
#endif	/* _SC_PAGE_SIZE */
    
    if (gethostname(hostname, MAXHOSTNAMELENGTH + 1)) {
	trace(TR_ALL, LOG_ERR, "main: gethostname: %m");
	task_quit(errno);
    }
    task_hostname = task_mem_strdup((task *) 0, hostname);

    /* check arguments for turning on tracing and a trace file */

    task_progname = name;
    if (name = (char *) rindex(task_progname, '/')) {
	task_progname = name + 1;
    }

    /* Remember directory we were started from */
    task_path_start = task_mem_strdup((task *) 0, task_getwd());
    task_path_now = task_mem_strdup((task *) 0, task_path_start);

    task_block_init_all();
    task_block_index = task_block_init(sizeof (task));
    timer_block_index = task_block_init(sizeof (timer));
    crash_block_index = task_block_init(sizeof (struct crash_entry));
    task_aux_index = task_block_init(sizeof (aux_proto));
}


void
task_signal_ignore __PF1(sig, int)
{
#ifdef	POSIX_SIGNALS
    struct sigaction act;

    /* Setup signal processing */
    sigemptyset(&act.sa_mask);
    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;

    if (sigaction(sig, &act, (struct sigaction *) 0) < 0) {
	int error = errno;

	trace(TR_ALL, LOG_ERR, "task_signal_ignore: sigaction(SIG%s): %m",
	      trace_state(signal_names, sig));
	task_quit(error);
    }
#else	/* POSIX_SIGNALS */
    signal(sig, SIG_IGN);
#endif	/* POSIX_SIGNALS */
}


void
task_init2 __PF0(void)
{
#ifdef	BSD_SIGNALS
    struct sigvec vec, ovec;
#endif	/* BSD_SIGNALS */
#ifdef	POSIX_SIGNALS
    struct sigaction act;
#endif	/* POSIX_SIGNALS */

    if (!BIT_TEST(task_state, TASKS_TEST)) {
	if (getuid()) {
	    fprintf(stderr, "%s: must be root\n",
		    task_progname);
	    exit(2);
	}
	
	if (!trace_flags_save || trace_file) {
	    /* This code rearranged after reading "Unix Network Programming" */
	    /* by W. Richard Stevens */
	    int t;

	    /*
	     * On System V if we were started by init from /etc/inittab there
	     * is no need to detach.
	     * There is some ambiguity here if we were orphaned by another
	     * process.
	     */
	    if (!BIT_TEST(task_state, TASKS_NODAEMON)) {
		/* Ignore terminal signals */
#ifdef	SIGTTOU
		task_signal_ignore(SIGTTOU);
#endif	/* SIGTTOU */
#ifdef	SIGTTIN
		task_signal_ignore(SIGTTIN);
#endif	/* SIGTTIN */
#ifdef	SIGTSTP
		task_signal_ignore(SIGTSTP);
#endif	/* SIGTSTP */

		switch (fork()) {
		case 0:
		    /* Child */
		    break;

		case -1:
		    /* Error */
		    perror("task_init2: fork");
		    exit(1);

		default:
		    /* Parent */
		    exit(0);
		}

		/* Remove our association with a controling tty */
#ifdef	USE_SETPGRP
		t = setpgrp();
		if (t < 0) {
		    perror("task_init2: setpgrp");
		    exit(1);
		}

		task_signal_ignore(SIGHUP);

		/* Fork again so we are not a process group leader */
		switch (fork()) {
		case 0:
		    /* Child */
		    break;

		case -1:
		    /* Error */
		    perror("task_init2: fork");
		    exit(1);

		default:
		    /* Parent */
		    exit(0);
		}
#else	/* USE_SETPGRP */
		t = setpgrp(0, getpid());
		if (t < 0) {
		    perror("task_init2: setpgrp");
		    exit(1);
		}

		NON_INTR(t, open(_PATH_TTY, O_RDWR, 0));
		if (t >= 0) {
		    if (ioctl(t, TIOCNOTTY, (caddr_t) 0) < 0) {
			perror("task_init2: ioctl(TIOCNOTTY)");
			exit(1);
		    }
		    (void) close(t);
		}
#endif	/* USE_SETPGRP */
	    }

	    /* Close all open files */
#ifndef	NOFILE
#ifdef	_NFILE
#define	NOFILE	_NFILE
#else	/* _NFILE */
#ifdef	OPEN_MAX
#define	NOFILE	OPEN_MAX
#else	/* OPEN_MAX */
#define	NOFILE	20
#endif	/* OPEN_MAX */
#endif	/* _NFILE */
#endif	/* NOFILE */	    
	    t = NOFILE;
#ifdef _POWER
            for (t--; t >= 0; t--) {
                 /* don't close SRC_FD, or we can't receive SRC packets - */
                 if (src_exists == FALSE)
                     (void) close(t);
                 else if (t != SRC_FD)
                     (void) close(t);
	    }
#else
	    do {
		(void) close(--t);
	    } while (t);
#endif  /* _POWER */

	    /* Inform tracing that stderr no longer exists */
	    trace_close();
	    
	    /* Reset umask */
	    umask(022);

#ifdef	STDIO_HACK
	    /* Setup to catch I/O to stdout and stderr */
	    task_stdio_init();
#endif	/* STDIO_HACK */
	}
    }

#ifdef	SIGPIPE
    task_signal_ignore(SIGPIPE);
#endif	/* SIGPIPE */

#ifdef	BSD_SIGNALS
    /* Set up signals to block */
    SIGNAL_LIST(ip) {
	BIT_SET(task_signal_mask, sigmask(*ip));
    } SIGNAL_LIST_END(ip) ;

    /* Block all signals during init */
    task_signal_mask_save = sigblock(task_signal_mask);

    /* Setup signal processing */
    bzero((char *) &vec, sizeof(vec));
    vec.sv_mask = task_signal_mask;
    vec.sv_handler = task_receive_signal;
#endif	/* BSD_SIGNALS */
#ifdef	POSIX_SIGNALS
#ifdef	SA_FULLDUMP
    /* Some systems (AIX) don't make a useful dump by default */
    {
	static int dumpsigs[] = {
#ifdef	SIGQUIT
	    SIGQUIT,
#endif
#ifdef	SIGILL
	    SIGILL,
#endif
#ifdef	SIGTRAP
	    SIGTRAP,
#endif
#ifdef	SIGIOT
	    SIGIOT,
#endif
#ifdef SIGABRT
	    SIGABRT,
#endif
#ifdef	SIGFPE
	    SIGFPE,
#endif
#ifdef SIGBUS
	    SIGBUS,
#endif
#ifdef SIGSEGV
	    SIGSEGV,
#endif
#ifdef	SIGSYS
	    SIGSYS,
#endif
	    0 };
	register int *sp;

	sigemptyset(&act.sa_mask);
	act.sa_handler = SIG_DFL;
	act.sa_flags = SA_FULLDUMP;
 
	for (sp = dumpsigs; *sp; sp++) {
	    if (sigaction(*sp, &act, (struct sigaction *) 0) < 0) {
		int error = errno;
#ifdef _POWER
		trace(TR_ALL, LOG_ERR, "task_signal_ignore: sigaction(%d): %m", act.sa_handler);
#else /* _POWER */
		trace(TR_ALL, LOG_ERR, "task_signal_ignore: sigaction(SIG%s): %m",
		      gd_upper(signal_names[*sp]));
#endif /* _POWER */
		task_quit(error);
	    }
	}
    }
#endif	SA_FULLDUMP
    /* Set up signals to block */
    sigemptyset(&task_signal_mask);
    SIGNAL_LIST(ip) {
	if (sigaddset(&task_signal_mask, *ip) < 0) {
	    int error = errno;

	    trace(TR_ALL, LOG_ERR, "task_init2: sigaddset(SIG%s): %m",
		  trace_state(signal_names, *ip));
	    task_quit(error);
	}
    } SIGNAL_LIST_END(ip) ;

    /* Block all signals during init */
    if (sigprocmask(SIG_BLOCK, &task_signal_mask, (sigset_t *) 0) < 0) {
	int error = errno;

	trace(TR_ALL, LOG_ERR, "task_init2: sigprocmask(SIG_BLOCK): %m");
	task_quit(error);
    }

    /* Setup signal processing */
    act.sa_mask = task_signal_mask;
    act.sa_handler = task_receive_signal;
    act.sa_flags = 0;
#endif	/* POSIX_SIGNALS */

    SIGNAL_LIST(ip) {
#ifdef	SYSV_SIGNALS
	sigset(*ip, task_receive_signal);
#endif	/* SYSV_SIGNALS */
#ifdef	BSD_SIGNALS
	if (sigvec(*ip, &vec, &ovec)) {
	    int error = errno;

	    trace(TR_ALL, LOG_ERR, "task_init: sigvec(SIG%s): %m",
		  trace_state(signal_names, *ip));
	    task_quit(error);
	}
#endif	/* BSD_SIGNALS */
#ifdef	POSIX_SIGNALS
	if (sigaction(*ip, &act, (struct sigaction *) 0) < 0) {
	    int error = errno;

	    trace(TR_ALL, LOG_ERR, "task_init: sigaction(SIG%s): %m",
		  trace_state(signal_names, *ip));
	    task_quit(error);
	}
#endif	/* POSIX_SIGNALS */
    } SIGNAL_LIST_END(ip) ;

    task_pid = task_mpid = getpid();
}


/*
 *	Dump task information to dump file
 */
void
task_dump __PF1(fd, FILE *)
{
    int i;
    int first;
    task *tp;
    timer *tip;

    (void) fprintf(fd,
		   "Task State: <%s>\n",
		   trace_bits(task_state_bits, task_state));

    (void) fprintf(fd,
		   "\tSend buffer size %u at %X\n",
		   task_send_buffer_len,
		   task_send_buffer);

    (void) fprintf(fd,
		   "\tRecv buffer size %u at %X\n\n",
		   task_recv_buffer_len,
		   task_recv_buffer);

    /* Print out task blocks */
    (void) fprintf(fd,
		   "Tasks (%d) and Timers:\n\n",
		   task_n_tasks);
    TASK_TABLE(tp) {
	(void) fprintf(fd, "\t%s\tPriority %d",
		       task_name(tp),
		       tp->task_priority);

	if (tp->task_proto) {
	    (void) fprintf(fd, "\tProto %3d",
			   tp->task_proto);
	}
#ifdef	PROTO_INET
	if (tp->task_addr && ntohs(tp->task_addr->in.sin_port)) {
	    (void) fprintf(fd, "\tPort %5u",
			   ntohs(tp->task_addr->in.sin_port));
	}
#endif	/* PROTO_INET */
	if (tp->task_socket != -1) {
	    (void) fprintf(fd, "\tSocket %2d",
			   tp->task_socket);
	}
	if (tp->task_rtproto) {
	    (void) fprintf(fd, "\tRtProto %s",
			   trace_state(rt_proto_bits, tp->task_rtproto));
	}
	if (tp->task_rtbit) {
	    (void) fprintf(fd, "\tRtBit %d",
			   tp->task_rtbit);
	}
	if (tp->task_flags) {
	    (void) fprintf(fd, "\t<%s>",
			   trace_bits(task_flag_bits, tp->task_flags));
	}
	(void) fprintf(fd, "\n");
	if (tp->task_trace_flags) {
	    char *buf = trace_string(tp->task_trace_flags);
	    (void) fprintf(fd, "\t\tTrace options: %s\n",
			   buf);
	    task_block_free(task_block_pagesize, buf);
	}
	first = TRUE;
	if (tp->task_timer) {
	    for (i = 0; i < tp->task_n_timers; i++) {
		if (TASK_TIMER(tp, i)) {
		    if (first) {
			(void) fprintf(fd, "\n");
			first = FALSE;
		    }
		    timer_dump(fd, TASK_TIMER(tp, i));
		}
	    }
	}
	(void) fprintf(fd, "\n");
    } TASK_TABLE_END(tp);
    (void) fprintf(fd, "\n");

    /* Print timers that are not associated with tasks */
    first = TRUE;
    TIMER_ACTIVE(tip, tip1) {
	if (!tip->timer_task) {
	    if (first) {
		(void) fprintf(fd, "\tTimers without tasks:\n\n");
		first = FALSE;
	    }
	    timer_dump(fd, tip);
	}
    } TIMER_ACTIVE_END(tip, tip1);

    TIMER_INACTIVE(tip, tip1) {
	if (!tip->timer_task) {
	    if (first) {
		(void) fprintf(fd, "\tTimers without tasks:\n\n");
		first = FALSE;
	    }
	    timer_dump(fd, tip);
	}
    } TIMER_INACTIVE_END(tip, tip1);

    if (!first) {
	(void) fprintf(fd, "\n");
    }

    /* Print memory block usage */
    task_block_dump(fd);
    
    /* Do task-specific dumps */
    TASK_TABLE(tp) {
	if (tp->task_dump) {
	    fprintf(fd, "Task %s:\n",
		    task_name(tp));
	    TASK_SAVE(tp, tp->task_dump(tp, fd));
	    fprintf(fd, "\n");
	}
    } TASK_TABLE_END(tp);

    /* Print out aux information */
    task_aux_dump(fd);
}


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
