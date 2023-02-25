/* @(#)78	1.5  src/tcpip/usr/sbin/gated/task.h, tcprouting, tcpip411, GOLD410 12/6/93 18:04:03 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		PROTOTYPEV
 *		TASK_TABLE
 *		TASK_TIMER
 *		TIMER_ACTIVE
 *		TIMER_INACTIVE
 *		_PROTOTYPE
 *		task_aux_flash
 *		task_aux_initiate
 *		task_aux_newpolicy
 *		task_block_alloc
 *		task_block_free
 *		task_block_free_clear
 *		task_get_recv_buffer
 *		task_get_send_buffer
 *		task_parse_ip
 *		task_parse_ip_hl
 *		task_parse_ip_opt
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
 * task.h,v 1.30.2.5 1993/08/27 22:28:55 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* Task definitions */

struct _task {
    struct _task *task_forw;
    struct _task *task_back;
    const char *task_name;		/* Printable task name */
    flag_t task_flags;			/* Flags */
    int task_proto;			/* Protocol */
    int task_socket;			/* Socket (if applicable) */
    proto_t task_rtproto;		/* Routing table protocol (if applicable) */
    proto_t task_rtfamily;		/* Routing table family (if applicable) */
    u_int task_rtbit;			/* Place to save my bit number */
    int task_priority;			/* Priority of this task in relation to others */
    flag_t task_trace_flags;		/* Tracing flags for this task */

    _PROTOTYPE(task_recv,
	       void,
	       (task *));		/* Routine to receive packets (if applicable) */
    _PROTOTYPE(task_accept,
	       void,
	       (task *));		/* Routine to process accepts (if applicable) */
    _PROTOTYPE(task_write,
	       void,
	       (task *));		/* Routine to write when socket is ready */
    _PROTOTYPE(task_connect,
	       void,
	       (task *));		/* Routine to process connect completions */
    _PROTOTYPE(task_except,
	       void,
	       (task *));		/* Routine to handle exceptions */
    _PROTOTYPE(task_terminate,
	       void,
	       (task *));		/* Routine to terminate task */
    _PROTOTYPE(task_flash,
	       void,
	       (task *,
		rt_list *));		/* Routine to do flash updates */
    _PROTOTYPE(task_newpolicy,
	       void,
	       (task *,
		rt_list *));		/* Routine to re-evaluate policy after reconfig */
    _PROTOTYPE(task_ifachange,
	       void,
	       (task *,
		if_addr *));			/* Routine to call when an address status changes */
    _PROTOTYPE(task_iflchange,
	       void,
	       (task *,
		if_link *));			/* Routine to call when a physical interface status changes */
    _PROTOTYPE(task_cleanup,
	       void,
	       (task *));		/* Routine to cleanup before config file is re-read */
    _PROTOTYPE(task_reinit,
	       void,
	       (task *));		/* Routine to called before normal processing begins */
    _PROTOTYPE(task_dump,
	       void,
	       (task *,
		FILE *));		/* Routine to dump state */
    sockaddr_un *task_addr;		/* Task dependent address */
    void_t task_data;			/* Task dependent pointer */

    /* Aux stuff */
    proto_t	task_aux_proto;		/* Protocol for Aux */
    struct _aux_proto *task_aux;	/* For protocol interaction */
    _PROTOTYPE(task_aux_register,
	       void,
	       (task *,
		struct _aux_proto *));	/* To register aux request */

    int task_pid;			/* PID if this is a child */
    _PROTOTYPE(task_process,
	       void,
	       (task *));		/* Routine to run after fork */
    _PROTOTYPE(task_child,
	       void,
	       (task *));		/* Routine to run after child finishes */

    u_int task_n_timers;		/* Number of timers this task has */
    struct _timer **task_timer;		/* Pointer to array of pointers to this task's timers */
};

#define	TASKF_ACCEPT		BIT(0x01)	/* This socket is waiting for accepts, not reads */
#define	TASKF_CONNECT		BIT(0x02)	/* This socket is waiting for connects, not writes */
#define	TASKF_LAST		BIT(0x04)	/* This task should the last one deleted */
#define	TASKF_DELETE		BIT(0x08)	/* This task has been deleted and is waiting for housekeeping */

#define	TASKOPTION_RECVBUF	0	/* Set receive buffer size */
#define	TASKOPTION_SENDBUF	1	/* Set send buffer size */
#define	TASKOPTION_LINGER	2	/* Set TCP linger on close */
#define	TASKOPTION_REUSEADDR	3	/* Enable/disable address reuse */
#define	TASKOPTION_BROADCAST	4	/* Enable/disable broadcast use */
#define	TASKOPTION_DONTROUTE	5	/* Enable/disable don't route */
#define	TASKOPTION_KEEPALIVE	6	/* Enable/disable keepalives */
#define	TASKOPTION_DEBUG	7	/* Enable/disable socket level debugging */
#define	TASKOPTION_NONBLOCKING	8	/* Enable/disable non-blocking I/O */
#define	TASKOPTION_USELOOPBACK	9	/* Use loopback */
#define	TASKOPTION_GROUP_ADD	10	/* Join a multicast group */
#define	TASKOPTION_GROUP_DROP	11	/* Leave a multicast group */
#define	TASKOPTION_MULTI_IF	12	/* Set multicast interface */
#define	TASKOPTION_MULTI_LOOP	13	/* Set multicast loopback */
#define	TASKOPTION_MULTI_TTL	14	/* Set multicast TTL */
#define	TASKOPTION_MULTI_ADD	15	/* Add this group to list to match on interface */
#define	TASKOPTION_MULTI_DROP	16	/* Drop this group from list to match on interface */
#define	TASKOPTION_MULTI_ROUTE	17	/* Enable Multicast routing in kernel */
#define	TASKOPTION_TTL		18	/* Set the IP TTL */
#define	TASKOPTION_TOS		19	/* Set the IP TOS */
#define	TASKOPTION_RCVDSTADDR	20	/* Receive destination address */
#define	TASKOPTION_IPHEADER_INC	21	/* IP Header is included */

#define	TASKPRI_INTERFACE	10	/* Interface task */
#define	TASKPRI_FAMILY		15	/* Address family */
#define	TASKPRI_RT		20	/* Routing table */
#define	TASKPRI_ICMP		30	/* ICMP */
#define	TASKPRI_IGMP		35	/* IGMP */
#define	TASKPRI_PROTO		40	/* Protocol tasks */
#define TASKPRI_EXTPROTO	50	/* External protocols */
#define	TASKPRI_REDIRECT	60	/* Redirects */
#define	TASKPRI_KERNEL		TASKPRI_REDIRECT
#define	TASKPRI_NETMGMT		70	/* SNMP et all */
#define	TASKPRI_SCRAM		TASKPRI_NETMGMT

extern task *task_active;		/* Pointer to the active task */
extern char *task_progname;			/* name we were invoked as */
extern int task_pid;			/* my process ID */
extern int task_mpid;			/* process ID of main process */
extern char task_pid_str[];		/* Printable process ID */
extern char *task_hostname;
extern flag_t task_state;		/* State of things to come */
extern size_t task_pagesize;		/* System page size */
extern size_t task_maxpacket;		/* Maximum packet size the kernel supports (set by KRT) */
extern char *task_path_start;		/* Directory where we were started */
extern char *task_path_now;		/* Directory where we are now */
extern char *task_config_file;		/* Configuration file to read */

extern const bits task_domain_bits[];
extern const bits task_socket_types[]; 

#define	TASKS_INIT		BIT(0x01)	/* Initializing */
#define	TASKS_TEST		BIT(0x02)	/* In configuration test mode */
#define	TASKS_RECONFIG		BIT(0x04)	/* Reconfiguration active */
#define	TASKS_TERMINATE		BIT(0x08)	/* Terminating */
#define	TASKS_NORECONFIG	BIT(0x10)	/* Reinits disabled */
#define	TASKS_NOSEND		BIT(0x20)	/* Packet transmission disabled */
#define	TASKS_FLASH		BIT(0x40)	/* Flash update in progress */
#define	TASKS_NEWPOLICY		BIT(0x80)	/* New Policy in progress */
#define	TASKS_NODAEMON		BIT(0x0100)	/* Do not daemonize */
#define	TASKS_STRICTIFS		BIT(0x0200)	/* Allow references to interfaces that do not (yet) exist */
#define	TASKS_NOFLASH		BIT(0x0400)	/* Flash updates suspended */
#define	TASKS_NODUMP		BIT(0x0800)	/* Do not create gated_dump in test mode */
#define	TASKS_NORESOLV		BIT(0x1000)	/* Do not use gethostbyname() or getnetbyname() */

#define	TASK_TIMER(tp, n)	((tp)->task_timer ? (tp)->task_timer[n] : (timer *) 0)


#define	TASK_PACKET_LIMIT	10	/* Max number of packets to read at one time */


/* Return codes from task_receive_packet - positive values are errno */
#define	TASKRC_OK		0		/* Read OK */
#define	TASKRC_EOF		-1		/* End-of-file */
#define	TASKRC_TRUNC		-2		/* Truncated */

/*
 *	I/O structures
 */

#ifdef	IP_HL_MASK
#define	task_parse_ip_hl(ip)	(((ip)->ip_hl_v & IP_HL_MASK) << 2)
#else	/* IP_HL_MASK */
#define	task_parse_ip_hl(ip)	((ip)->ip_hl << 2)
#endif	/* IP_HL_MASK */

#define	task_parse_ip(ip, dp, type) \
{ \
    ip = task_get_recv_buffer(struct ip *); \
    dp = (type) ((void_t) ((caddr_t) ip + task_parse_ip_hl(ip))); \
}

#define	task_parse_ip_opt(ip, op, dp, type) { \
    task_parse_ip(ip, dp, type); \
    op = ((caddr_t) dp > (caddr_t) ip + sizeof (struct ip)) ? (byte *) ip + sizeof (struct ip) : (byte *) 0; \
}

/* Receive buffer and length */
extern void_t task_recv_buffer;
extern size_t task_recv_buffer_len;

#define	task_get_recv_buffer(type)	(type) task_recv_buffer

/* Send buffer and length */
extern void_t task_send_buffer;
extern size_t task_send_buffer_len;

#define	task_get_send_buffer(type)	(type) task_send_buffer

extern sockaddr_un *task_recv_srcaddr;	/* Source address of received packet */
extern sockaddr_un *task_recv_dstaddr;	/* Destination address of received packet */

PROTOTYPE(task_pid_open,
	  extern void,
	  (void));
PROTOTYPE(task_quit,
	  extern void,
	  (int));
PROTOTYPE(task_assert,
	  extern void,
	  (const char *,
	   int));
PROTOTYPE(task_newstate,
	  void,
	  (flag_t,
	   flag_t));
PROTOTYPE(task_alloc,
	  extern task *,
	  (const char *,
	   int));
PROTOTYPE(task_create,
	  extern int,
	  (task *));
PROTOTYPE(task_alloc_send,
	  extern void,
	  (task *,
	   size_t));
PROTOTYPE(task_alloc_recv,
	  extern void,
	  (task *,
	   size_t));
PROTOTYPE(task_fork,
	  extern int,
	  (task *));
PROTOTYPE(task_ioctl,
	  extern int,
	  (int,
	   u_int,
	   void_t,
	   int));
PROTOTYPE(task_delete,
	  extern void,
	  (task *));
PROTOTYPE(task_close,
	  extern void,
	  (task *));
PROTOTYPE(task_init1,
	  extern void,
	  (char *));
PROTOTYPE(task_init2,
	  extern void,
	  (void));
PROTOTYPE(task_signal_ignore,
	  extern void,
	  (int));
PROTOTYPE(task_main,
	  extern void,
	  (void));
PROTOTYPE(task_flash,
	  extern int,
	  (rt_list *,
	   int));
PROTOTYPE(task_ifachange,
	  extern void,
	  (if_addr *));
PROTOTYPE(task_iflchange,
	  extern void,
	  (if_link *));
PROTOTYPEV(task_set_option,
	   extern int,
	   (task *,
	    int,
	    ...));
PROTOTYPE(task_get_socket,
	  extern int,
	  (int,
	   int,
	   int));
PROTOTYPE(task_set_socket,
	  extern void,
	  (task *,
	   int));
PROTOTYPE(task_addr_local,
	  extern int,
	  (task *,
	   sockaddr_un *));
PROTOTYPE(task_connect,
	  extern int,
	  (task *));
PROTOTYPE(task_get_addr_local,
	  extern sockaddr_un *,
	  (task *));
PROTOTYPE(task_get_addr_remote,
	  extern sockaddr_un *,
	  (task *));
PROTOTYPE(task_reset_socket,
	  extern void,
	  (task *));
PROTOTYPE(task_name,
	  extern char *,
	  (task *));
PROTOTYPE(task_dump,
	  extern void,
	  (FILE *));
PROTOTYPE(task_receive_packet,
	  extern int,
	  (task * tp,
	   size_t *));
PROTOTYPE(task_send_packet,
	  extern int,
	  (task * tp,
	   void_t,
	   size_t,
	   flag_t,
	   sockaddr_un *));
PROTOTYPE(task_proto_inits,
	  extern void,
	  (void));
PROTOTYPE(task_proto_var_inits,
	  extern void,
	  (void));
PROTOTYPE(task_mem_malloc,
	  extern void_t,
	  (task *,
	   size_t));
PROTOTYPE(task_mem_calloc,
	  extern void_t,
	  (task *,
	   u_int,
	   size_t));
PROTOTYPE(task_mem_realloc,
	  extern void_t,
	  (task *,
	   void_t,
	   size_t));
PROTOTYPE(task_mem_strdup,
	  extern char *,
	  (task *,
	   const char *));
PROTOTYPE(task_mem_free,
	  extern void,
	  (task *,
	   void_t));
PROTOTYPE(task_crash_add,
	  extern void,
	  (task *,
	   _PROTOTYPE(job,
		      void,
		      (task *,
		       void_t)),
	   void_t));
PROTOTYPE(task_crash,
	  extern void,
	  (void));	  
#ifndef	vax11c
PROTOTYPE(task_getwd,
	  extern char *,
	  (void));
PROTOTYPE(task_chdir,
	  extern int,
	  (const char *));
#endif	/* vax11c */

#define	TASK_TABLE(tp) \
    for (tp = task_head.task_forw; tp != &task_head; tp = tp->task_forw) { \
	if (!BIT_TEST(tp->task_flags, TASKF_DELETE))
#define	TASK_TABLE_END(tp)	}


/*  */

/* Blocks */

struct task_block {
    struct task_block *forw;
    struct task_block *back;
    u_int	size;		/* Size of this block */
    u_int	count;		/* Count of blocks per unit */
    u_int	n_free;		/* Number on the free list */
    block_t	runt;		/* Entry to hold the runt */
    u_int	n_req_alloc;	/* Number of alloc requests */
    u_int	n_req_free;	/* Number of free requests */
    u_int	n_req_init;	/* Number of inits for this size */
    struct qelem *free;		/* Free list */
    struct qelem *tmp;		/* For allocation macro */
};


extern block_t task_block_pagesize;	/* For obtaining pagesize blocks */

PROTOTYPE(task_block_init,
	  extern block_t,
	  (size_t));
PROTOTYPE(task_block__alloc,
	  extern struct qelem *,
	  (block_t));
PROTOTYPE(task_block_malloc,
	  extern void_t,
	  (size_t));
PROTOTYPE(task_block_reclaim,
	  extern void,
	  (size_t,
	   void_t));

/* Free a clear block */
#define	task_block_free_clear(tbp, p) \
    { \
	/* Put this element on the head of the list */ \
	((struct qelem *) ((void_t) p))->q_forw = (tbp)->free; \
	(tbp)->free = (struct qelem *) ((void_t) p); \
	(tbp)->n_free++, (tbp)->n_req_free++; \
    }

/* Clear a block and return it */
#define	task_block_free(tbp, p)	{ bzero((caddr_t) p, (tbp)->size); task_block_free_clear(tbp, p); }

/* Allocate a block */
#define	task_block_alloc(tbp) \
    ((tbp)->tmp = ((tbp)->free ? (tbp)->free : task_block__alloc(tbp)), \
     (tbp)->free = (tbp)->tmp->q_forw, \
     (tbp)->tmp->q_forw = (struct qelem *) 0, \
     (tbp)->n_free--, (tbp)->n_req_alloc++, \
     (void_t) (tbp)->tmp)

/*  */
/* Timer definitions */

struct _timer {
    struct _timer *timer_forw;
    struct _timer *timer_back;
    const char *timer_name;		/* Printable name for this timer */
    flag_t timer_flags;			/* Flags */
    time_t timer_next_time;		/* Timer job wakeup time */
    time_t timer_last_time;		/* Last time job was called */
    time_t timer_interval;		/* Time to sleep between timer jobs */
    _PROTOTYPE(timer_job,
	       void,
	       (timer *,
		time_t));		/* Timer job (if applicable) */
    task *timer_task;			/* Task which owns this timer */
    int timer_index;			/* Index of this timer in task's table */
    void_t timer_data;			/* Timer specific data */
};

/* Timer flags */

#define	TIMERF_ABSOLUTE		BIT(0x01)	/* Timer is relative to start time, not to last time */
#define	TIMERF_DELETE		BIT(0x02)	/* Delete timer after it fires */

#define	TIMER_FUZZ		2	/* How forgiving to be before bitching about the system clock */

/* Current time definition */

struct gtime {
    time_t gt_sec;
#ifdef	HAVE_GETSYSTIMES
    time_t gt_boot;
#endif	/* HAVE_GETSYSTIMES */
    char gt_str[16];
    char gt_ctime[26];
};

extern struct gtime timer_time;
extern struct gtime timer_time_start;

#define	time_sec timer_time.gt_sec
#ifdef	HAVE_GETSYSTIMES
#define	time_boot	timer_time.gt_boot
#endif	/* HAVE_GETSYSTIMES */
#define time_string timer_time.gt_str
#define	time_full timer_time.gt_ctime

PROTOTYPE(timer_name, extern char *, (timer * tip));	/* Return a string containing the name of a timer */
PROTOTYPE(timer_create,		/* Create a timer */
	  extern timer *,
	  (task * tp,
	   int timer_index,
	   const char *name,
	   flag_t flags,
	   time_t interval,
	   time_t offset,
	   _PROTOTYPE(job,
		      void,
		      (timer *,
		       time_t)),
	   void_t data));
PROTOTYPE(timer_delete,
	  extern int,
	  (timer * tip));	/* Delete a timer */
PROTOTYPE(timer_set,
	  extern void,
	  (timer * tip,
	   time_t interval,
	   time_t offset));	/* Set a timer */
PROTOTYPE(timer_reset,
	  extern void,
	  (timer * tip));	/* Reset a timer (clear it) */
PROTOTYPE(timer_interval,
	  extern void,
	  (timer * tip,
	   time_t interval));	/* Change a timer interval */
PROTOTYPE(timer_peek,
	  extern void,
	  (void));

#define	TIMER_ACTIVE(tip, tip1) { timer *tip1; \
				for (tip = timer_queue_active.timer_forw; tip1 = tip->timer_back, tip != &timer_queue_active; \
				     tip = (tip == tip1->timer_forw) ? tip->timer_forw : tip1->timer_forw)
#define TIMER_ACTIVE_END(tip, tip1)	}

#define	TIMER_INACTIVE(tip, tip1) { timer *tip1; \
				for (tip = timer_queue_inactive.timer_forw; tip1 = tip->timer_back, tip != &timer_queue_inactive; \
				     tip = (tip == tip1->timer_forw) ? tip->timer_forw : tip1->timer_forw)
#define TIMER_INACTIVE_END(tip, tip1) }


/**/

/* IGP-EGP interaction */

/*
 * Auxiliary routing protocol structure.  An auxiliary routing protocol
 * is one which operates in parallel with, or as a part of, an IGP to carry
 * additional routing information which the IGP cannot manage itself.
 * Examples of an auxiliary routing protocol are internal BGP and
 * internal IDRP, when run with an IGP such as OSPF or IS-IS.
 */
typedef struct _aux_proto {
    struct _aux_proto	*aux_forw;
    struct _aux_proto	*aux_back;
    proto_t aux_proto_igp;	/* protocol of aux protocol */
    task *aux_task_egp;		/* pointer to aux task */
    task *aux_task_igp;		/* pointer to other task */
    _PROTOTYPE(aux_initiate,
	       void,
	       (task *,
		proto_t,
		u_int));	/* Initiate interaction */
    _PROTOTYPE(aux_flash,
	       void,
	       (task *,
		rt_list *));	/* Flash update done */
    _PROTOTYPE(aux_newpolicy,
	       void,
	       (task *,
		rt_list *));	/* New policy done */
    _PROTOTYPE(aux_terminate,
	       void,
	       (task *));	/* Terminate interaction */
} aux_proto;


#define	task_aux_flash(aux, rtl)	(auxp)->aux_flash((auxp)->aux_task_egp, rtl)
#define	task_aux_newpolicy(auxp, rtl)	(auxp)->aux_newpolicy((auxp)->aux_task_egp, rtl)
#define	task_aux_initiate(auxp, rtbit)	(auxp)->aux_initiate((auxp)->aux_task_egp, (auxp)->aux_proto_igp, rtbit)

PROTOTYPE(task_aux_register,
	  void,
	  (task *,
	   u_int,
	  _PROTOTYPE(aux_initiate,
		      void,
		      (task *,
		       proto_t,
		       u_int)),
	   _PROTOTYPE(aux_terminate,
		      void,
		      (task *)),
	   _PROTOTYPE(aux_flash,
		      void,
		      (task *,
		       rt_list *)),
	   _PROTOTYPE(aux_newpolicy,
		      void,
		      (task *,
		       rt_list *))));
PROTOTYPE(task_aux_unregister,
	  extern void,
	  (task *));
PROTOTYPE(task_aux_lookup,
	  extern void,
	  (task *));
PROTOTYPE(task_aux_terminate,
	  extern void,
	  (task *));
	   


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
