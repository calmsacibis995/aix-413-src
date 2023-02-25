/* @(#)43	1.1  src/bos/usr/sbin/crash/POWER/dla.h, cmdcrash, bos411, 9428A410j 10/15/93 10:25:54 */

/*
 * COMPONENT_NAME: (CMDCRASH)
 *
 * FUNCTIONS:
 *
 * ORIGIN: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */

typedef enum { o_none = 0, o_thread = 1, o_it_handler = 2 } owner_t;

typedef enum { l_simple = 0, l_complex, l_aix3_2, l_other} lock_type_t;

struct lock_desc {
	
	int lock;	/* machine address */
	lock_type_t lock_type;  /* simple, complex, aix3_2, other */
	int status; 	/* lock state = R, W ...  (taken from lock design) */
	owner_t owner_type; /* enum {thread or it_handler or NULL} */
	int owner_id; /* a thread id or a cpu_id */
	struct mstsave *waiting_csa; 	/* csa to take in consideration for stack tracing */
	int slot;   /* The context associated with the csa */
	struct lock_desc *next;         /* a pointer to a lock_desc structure */
	
};
