static char sccsid[] = "@(#)53  1.3  src/bos/usr/sbin/crash/streams.c, cmdcrash, bos411, 9428A410j 5/16/94 16:29:00";
/*
 * COMPONENT_NAME: (CMDCRASH)
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 
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
 *
 */

#include "crash.h"
#include <pse/str_stream.h>

#include "crash_msg.h"
extern nl_catd  scmc_catd;	/* Cat descriptor for scmc conversion */

extern int dumpflag;		/* system dump or running system? */

void stream();
void all_queues();
void queue_by_addr();
int get_queue();
void print_queue();
int valid_queue();
void mblock();
void dblock();
void qrun();
void linkblk();
void dmodsw();
void fmodsw();
void get_modsw();


/*
 * stream()
 * Print the stream head table.
 */
void
stream()
{
	STH     sth;                /* stream head structure */
	STHP    sth_open_streams[STH_HASH_TBL_SIZE]; /* Stream head table */
	STHP    s;
	int     rflag = 0;          /* procslot for readmem() */
	int     j;
	int     num_streams = 0;    /* count of streams open */
	POLLQ	pollq, sigsq;       /* POLLQ and SIGSQ are same */

	if (dumpflag == 0)
		rflag = 1;

	/* 
	 * Find the memory location of sth_open_streams.
	 */
	if (!get_sth(sth_open_streams, rflag))
		return;

	/*
	 * Walk the entire stream head array and look for cling-ons.
	 * readmem() if a cling-on is found.  It's possible that not
	 * all hash entries will have been filled.
	 */
	printf(" ADDRESS   WRITEQ MAJ/MIN RERR WERR  FLAGS IOCID  WOFF PCNT POLQNEXT SIGQNEXT\n");

	for (j=0; j<STH_HASH_TBL_SIZE; j++) {
		if (sth_open_streams[j] == NULL)
			continue;

		/* 
		 * There can be more than one stream head per array index.
		 */
		s = sth_open_streams[j];
		do {
			if (readmem(&sth, s, sizeof(sth), rflag) !=
					sizeof(sth)) {
				printf(catgets(scmc_catd, MS_crsh, STREAM_MSG_2,
					"0452-1100: Cannot read the stream head table.\n"));
				return;
			}

			/*
			 * We have to do a readmem() on the sthq_s structure
			 * to determine if there is an active poll.
			 */
			if (readmem(&pollq, sth.sth_pollq.next, sizeof(POLLQ),
					rflag) != sizeof(POLLQ)) {
				printf(catgets(scmc_catd, MS_crsh, STREAM_MSG_4,
					"0452-1102: Cannot read sthq_s structure.\n"));
				return;
			}
			
			/*
			 * We have to do a readmem() on the sthq_s structure
			 * to determine if there is an active signal.
			 */
			if (readmem(&sigsq, sth.sth_sigsq.next, sizeof(SIGSQ),
					rflag) != sizeof(SIGSQ)) {
				printf(catgets(scmc_catd, MS_crsh, STREAM_MSG_4,
					"0452-1102: Cannot read sthq_s structure.\n"));
				return;
			}
			
			printf("%8x %8x %3d,%3d %4d %4d 0x%04x %5d %5u %4d %8x %8x\n",
				s, sth.sth_wq, major(sth.sth_dev),
				minor(sth.sth_dev), sth.sth_read_error,
				sth.sth_write_error, sth.sth_flags,
				sth.sth_ioc_id, sth.sth_wroff,
				sth.sth_push_cnt,
				((pollq.next == sth.sth_pollq.next)?0:
				sth.sth_pollq.next),
				((sigsq.next == sth.sth_sigsq.prev)?0:
				sth.sth_sigsq.next));

			num_streams ++;
			s = sth.sth_next;
		} while (sth.sth_next != NULL);
	}

	/* Sanity check - there should always be a stream open. */
	if (num_streams == 0)
		printf( catgets(scmc_catd, MS_crsh, STREAM_MSG_1,
			"No open streams are available on the system.\n"));
}


/*
 * get_sth()
 * Find the sth_open_streams array.  If it is found, a 1 is returned.
 * Otherwise, a 0 is returned.
 */
int
get_sth(sth_open_streams, rflag)
STHP    sth_open_streams[];	/* Stream head table */
int     rflag;          	/* procslot for readmem() */
{
	/*
	 * Get the sth_open_stream array.
	 */
	if (readmem(sth_open_streams, p_sth_open_streams,
		STH_HASH_TBL_SIZE*sizeof(STHP), rflag) !=
			STH_HASH_TBL_SIZE*sizeof(STHP)) {
		printf(catgets(scmc_catd, MS_crsh, STREAM_MSG_2,
			"0452-1100: Cannot read the stream head table.\n"));
		return 0;
	}

	return 1;
}


/*
 * all_queues()
 * List all the write queues on the system.  If the user wants the details
 * of the read queue, the user will need to specify the read queue address
 * as a parameter to the queue subcommand.
 */
void
all_queues()
{
	STH     sth;                /* stream head structure */
	STHP    sth_open_streams[STH_HASH_TBL_SIZE]; /* Stream head table */
	STHP    s;
	int     rflag = 0;          /* procslot for readmem() */
	int     j;
	int     num_queues = 0;	    /* count of queues */
	struct queue queue;
	struct queue *q;

	if (dumpflag == 0)
		rflag = 1;

	/* 
	 * Find the memory location of sth_open_streams.
	 */
	if (!get_sth(sth_open_streams, rflag))
		return;

	/* Header. */
	printf("  WRITEQ    QINFO     NEXT  PRIVATE  FLAGS     HEAD    READQ      COUNT\n");

	/*
	 * Walk the entire stream head array and look for cling-ons.
	 * readmem() if a cling-on is found.  It's possible that not
	 * all hash entries will have been filled.
	 */
	for (j=0; j<STH_HASH_TBL_SIZE; j++) {
		if (sth_open_streams[j] == NULL)
			continue;

		/* 
		 * There can be more than one stream head per array index.
		 */
		s = sth_open_streams[j];
		do {
			if (readmem(&sth, s, sizeof(sth), rflag) != 
					sizeof(sth)) {
				printf(catgets(scmc_catd, MS_crsh, STREAM_MSG_2,
					"0452-1100: Cannot read the stream head table.\n"));
				return;
			}

			if (!get_queue(&queue, sth.sth_wq, rflag)) {
				s = sth.sth_next;
				continue;
			}
			num_queues++;
			print_queue(&queue, sth.sth_wq);

			q = queue.q_next;
			while (q != NULL) {
				if (!get_queue(&queue, q, rflag))
					break;
				num_queues++;
				print_queue(&queue, q);
				q = queue.q_next;
			}
			s = sth.sth_next;
		} while (sth.sth_next != NULL);
	}

	if (num_queues == 0)
		printf( catgets(scmc_catd, MS_crsh, QUEUE_MSG_2,
			"No queues are found in the system.\n"));
}


/*
 * queue_by_addr()
 * Find a specific queue.
 */
void
queue_by_addr(address)
queue_t  *address;
{
	queue_t  queue;
	int      rflag = 0;          /* procslot for readmem() */
	int      rc;

	if (dumpflag == 0)
		rflag = 1;

	rc = valid_queue(address, rflag);
	if (rc == -1)
		return;
	if (!rc) {
		printf(catgets(scmc_catd, MS_crsh, QUEUE_MSG_1,
		"0452-1110: The specified address is not a valid queue address.\n"));
		return;
	}

	if (get_queue(&queue, address, rflag)) {
		printf("   QUEUE    QINFO     NEXT  PRIVATE  FLAGS     HEAD   OTHERQ      COUNT\n");
		print_queue(&queue, address);
		return;
	}

	/*
	 * Sanity check.  The queue is valid, but cannot find it.
	 */
	printf(catgets(scmc_catd, MS_crsh, QUEUE_MSG_1,
	       "0452-1110: The specified address is not a valid queue address.\n"));
}


/*
 * get_queue()
 * Read a specific queue structure from kernel memory.  If the queue is found,
 * a 1 is returned.  Otherwise a 0 is returned.
 */
int
get_queue(queue, memory_loc, flag)
struct queue *queue;
caddr_t memory_loc;	/* where to read in kernel memory */
int     flag;           /* procslot for readmem() */
{
	if (readmem(queue, memory_loc, sizeof(struct queue), flag) !=
			sizeof(struct queue)) {
		printf(catgets(scmc_catd, MS_crsh, QUEUE_MSG_3,
			("0452-1111: Cannot read the queue structure: %x.\n",
			 memory_loc)));
		return 0;
	}

	return 1;
}


/*
 * print_queue()
 * Print out the interesting queue information.
 */
void
print_queue(queue, memory_loc)
struct queue *queue;
caddr_t memory_loc;
{
	printf("%8x %8x %8x %8x 0x%04x %8x %8x %10u\n", memory_loc,
		queue->q_qinfo, queue->q_next, queue->q_ptr, queue->q_flag,
		queue->q_first, queue->q_other, queue->q_count);
}


/*
 * valid_queue()
 * Determine if this address is really a queue address.
 * Returns 1 if the address is a valid queue address.
 * Returns 0 if the address is not a valid queue address.
 * Returns -1 if the sth_open_streams array cannot be located or read.
*/
int
valid_queue(address, flag)
queue_t *address;
int      flag;
{
	STH     sth;                /* stream head structure */
	STHP    sth_open_streams[STH_HASH_TBL_SIZE]; /* Stream head table */
	STHP    s;
	int     j;
	struct queue queue;
	struct queue *q;

	/* 
	 * Find the memory location of sth_open_streams.
	 */
	if (!get_sth(sth_open_streams, flag))
		return -1;

	/*
	 * Walk the entire stream head array and look for cling-ons.
	 * readmem() if a cling-on is found.  It's possible that not
	 * all hash entries will have been filled.
	 */
	for (j=0; j<STH_HASH_TBL_SIZE; j++) {
		if (sth_open_streams[j] == NULL)
			continue;

		/* 
		 * There can be more than one stream head per array index.
		 */
		s = sth_open_streams[j];
		do {
			if (readmem(&sth, s, sizeof(sth), flag) !=
					sizeof(sth)) {
				printf(catgets(scmc_catd, MS_crsh, STREAM_MSG_2,
					"0452-1100: Cannot read the stream head table.\n"));
				return -1;
			}

			/*
			 * If this address is a read queue or write queue
			 * associated with the stream head, it's valid.
			 */
			if ((address == sth.sth_rq) || (address == sth.sth_wq))
				return 1;

			if (!get_queue(&queue, sth.sth_wq, flag))
				continue;
			q = queue.q_next;
			while (q != NULL) {
				if (!get_queue(&queue, q, flag)) 
					continue;
				if ((address == q) || 
						(address == queue.q_other))
					return 1;
				q = queue.q_next;
			}
			s = sth.sth_next;
		} while (sth.sth_next != NULL);
	}

	/* Queue not found. */
	return 0;
}


/*
 * mblock()
 * Print the allocated STREAMS message block headers for the specified
 * address.
 */
void
mblock(address)
mblk_t  *address;
{
	mblk_t  mblock;
	int     rflag = 0;          /* procslot for readmem() */

	if (dumpflag == 0)
		rflag = 1;

	/*
	 * The mblk_t address must be on a 128 byte boundary.
	 */
	if ((ulong)address & (ulong)0x7F) {
		printf(catgets(scmc_catd, MS_crsh, MBLOCK_MSG_2,
			"0452-1121: The specified address is not a valid msgb address.\n"));
		return;
	}

	if (readmem(&mblock, address, sizeof(mblk_t), rflag) !=
			sizeof(mblk_t)) {
		printf(catgets(scmc_catd, MS_crsh, MBLOCK_MSG_2,
			"0452-1121: The specified address is not a valid msgb address.\n"));
	}

	printf(" ADDRESS     NEXT PREVIOUS     CONT     RPTR     WPTR DATABLOCK\n");
	printf("%8x %8x %8x %8x %8x %8x  %8x\n", address, mblock.b_next,
		mblock.b_prev, mblock.b_cont, mblock.b_rptr,  mblock.b_wptr,
		mblock.b_datap);
}


/*
 * dblock()
 * Print the allocated STREAMS data block headers for the specified address.
 */
void
dblock(address)
dblk_t  *address;
{
	dblk_t  dblock;
	int     rflag = 0;          /* procslot for readmem() */

	if (dumpflag == 0)
		rflag = 1;

	if (readmem(&dblock, address, sizeof(dblk_t), rflag) !=
			sizeof(dblk_t)) {
		printf(catgets(scmc_catd, MS_crsh, DBLOCK_MSG_1,
			"0452-1130: The specified address is not a valid datab address.\n"));
	}

	printf(" ADDRESS    FREEP     BASE      LIM   REFCNT     TYPE     SIZE\n");
	printf("%8x %8x %8x %8x %8x %8x %8x\n", address, dblock.db_f.freep,
		dblock.db_base, dblock.db_lim, dblock.db_ref, dblock.db_type,
		dblock.db_size);
}


/*
 * qrun()
 * Print the list of scheduled STREAMS queues.
 */
void
qrun()
{
	SQH   runq;
	SQHP  runq_ptr;
	SQ    schedq;
	SQP   schedq_ptr;
	int   rflag = 0;          /* procslot for readmem() */
	int   num_queues = 0;

	if (dumpflag == 0)
		rflag = 1;

	/*
	 * Get the streams_runq.
	 */
	if (readmem(&runq, p_streams_runq, sizeof(SQH), rflag) != sizeof(SQH)) {
		printf(catgets(scmc_catd, MS_crsh, QRUN_MSG_3,
			"0452-1141: Cannot read streams_runq.\n"));
		return;
	}

	runq_ptr = &runq;
	if (runq_ptr->sqh_next == NULL) {
		printf(catgets(scmc_catd, MS_crsh, QRUN_MSG_1,
			"No queues are scheduled for service.\n"));
		return;
	}

	schedq_ptr = runq_ptr->sqh_next;
	while (schedq_ptr != NULL) {
		if (readmem(&schedq, schedq_ptr, sizeof(SQ), rflag)
				!= sizeof(SQ)) {
			printf(catgets(scmc_catd, MS_crsh, QRUN_MSG_4,
				"1452-1142: Cannot read the sqe_s structure.\n"));
			return;
		}

		/*
		 * If the next pointer points to itself, then this is
		 * the end of the list.
		 */
		if (schedq.sq_next == schedq_ptr) {
			if (num_queues == 0)
				printf(catgets(scmc_catd, MS_crsh, QRUN_MSG_1,
				     "No queues are scheduled for service.\n"));
			return;
		}

		num_queues++;
		if (num_queues == 1)
			printf("   QUEUE\n");
		printf("%8x\n", schedq.sq_queue);
		schedq_ptr = schedq.sq_next;
	};
}


/*
 * linkblk()
 * Print the linkblk table.
 */
void
linkblk()
{
	STH      sth, lsth;                /* stream head structure */
	STHP     sth_open_streams[STH_HASH_TBL_SIZE]; /* Stream head table */
	STHP     s, mux;
	int      j;
	queue_t *l_qtop;
	queue_t *l_qbot;
	int      index;
	queue_t *q;
	queue_t  queue;
	int      rflag = 0;         /* procslot for readmem() */
	int      num_mplex = 0;

	if (dumpflag == 0)
		rflag = 1;

	/* 
	 * Find the memory location of sth_open_streams.
	 */
	if (!get_sth(sth_open_streams, rflag))
		return;

	/*
	 * Walk the entire stream head array and look for cling-ons.
	 * readmem() if a cling-on is found.  It's possible that not
	 * all hash entries will have been filled.
	 */
	for (j=0; j<STH_HASH_TBL_SIZE; j++) {
		if (sth_open_streams[j] == NULL)
			continue;
		s = sth_open_streams[j];
		do {
			if (readmem(&sth, s, sizeof(sth), rflag) !=
					sizeof(sth)) {
				printf(catgets(scmc_catd, MS_crsh, STREAM_MSG_2,
					"0452-1100: Cannot read the stream head table.\n"));
				return;
			}

			/* 
		 	 * If there is no sth_mux_top or sth_pmux_top, then
		 	 * there is no stream multiplexor for this stream head.
		 	 */
			if ((sth.sth_mux_top == NULL) &&
					(sth.sth_pmux_top == NULL)) {
				s = sth.sth_next;
				if (s == NULL)
					break;
				continue;
			}

			/*
			 * We've got a multiplexor.
			 */
			if (sth.sth_mux_top != NULL)
				mux = sth.sth_mux_top;
			if (sth.sth_pmux_top != NULL)
				mux = sth.sth_pmux_top;

			/*
			 * We need the muxid from the lower stream head, so
			 * we have to get it from kernel memory.
			 */
			if (readmem(&lsth, mux, sizeof(sth), rflag) !=
					sizeof(sth)) {
				printf(catgets(scmc_catd, MS_crsh, STREAM_MSG_2,
					"0452-1100: Cannot read the stream head table.\n"));
				return;
			}
				
			/* Defaults. */
			l_qtop = NULL;
			l_qbot = lsth.sth_wq;

			/*
		 	 * Get l_qtop -- the lowest level write queue of the
		 	 * stream which has a non-NULL sth_mux_top or
		 	 * sth_pmux_top.  If sth.sth_pmux_top is non-NULL,
		 	 * then l_qtop is NULL.  Walk the queue linked list to
		 	 * get l_qtop for non-NULL sth.sth_mux_top.  We've got
		 	 * the lowest queue when the write queue q_next is NULL.
		 	 */
			if ((sth.sth_mux_top != NULL) &&
					(sth.sth_pmux_top == NULL)) {
				q = sth.sth_wq;
				while (q != NULL) {
					if (readmem(&queue, q, sizeof(queue),
						    rflag) != sizeof(queue)) {
						printf(catgets(scmc_catd,
							MS_crsh, QUEUE_MSG_3,
				  	 		"Cannot read the queue structure.\n"));
						break;
					}
					l_qtop = q;
					q = queue.q_next;
				}
				if (q == NULL)
					num_mplex++;
			}
			else
				num_mplex++;


			/*
			 * Got data for this stream head.  Print it out
			 * if the number of multiplexors is greater than
			 * zero.  The check seem funky, but if the read
			 * queue could not be read above, then it was not
			 * counted as a multiplexor.
			 */
			if (num_mplex == 1)
				printf("    QTOP     QBOT    INDEX\n");
			if (num_mplex != 0)
				printf("%8x %8x %8x\n", l_qtop, l_qbot,
					lsth.sth_muxid);

			s = sth.sth_next;
		} while (sth.sth_next != NULL);
	}

	if (num_mplex == 0)
		printf(catgets(scmc_catd, MS_crsh, LINKBLK_MSG_1,
			"No linkblk structure can be found on the system.\n"));
}


/*
 * dmodsw()
 * Print the STREAMS drivers switch table.
 */
void
dmodsw()
{
	get_modsw(p_dmodsw, DMODSW_MSG_1, "drivers");
}


/*
 * fmodsw()
 * Print the STREAMS modules switch table.
 */
void
fmodsw()
{
	get_modsw(p_fmodsw, FMODSW_MSG_1, "modules");
}


/*
 * get_modsw()
 * Generic print routine for the modsw structure linked list.
 */
void
get_modsw(struct_addr, msgnum, type)
ulong struct_addr; /* this is the memory address for the structure requested */
int   msgnum;
char *type;
{
	struct modsw  modsw;
	struct modsw  *m_ptr;
	struct modsw  *second_link = NULL;	/* first valid modsw */
	char    message[50];
	int     rflag = 0;          		/* procslot for readmem() */

	if (dumpflag == 0)
		rflag = 1;

	/*
	 * Build the message in case it is needed.
	 */
	sprintf(message, "No STREAMS %s are loaded on the system.\n",
		type);


	/*
	 * Get the modsw structure.
	 */
	if (readmem(&modsw, struct_addr, sizeof(struct modsw), rflag) !=
			sizeof(struct modsw)) {
		printf(catgets(scmc_catd, MS_crsh, msgnum, message));
		return;
	}

	/*
	 * modsw points to a circularly linked list.  The final
	 * link's d_next pointer points to the second link in the
	 * list.  Save off the address of the second link modsw.
	 * When the d_next pointer has the same value as the second
	 * link address, break out of the loop.
	 * Walk through the modsw and print out the contents.
	 * Do not print out the contents of the head - it's empty except
	 * for d_next.
	 */
	printf("NAME        ADDRESS     NEXT PREVIOUS FLAG   SYNCHQ STREAMTAB S-LVL COUNT MAJOR\n");

	m_ptr = &modsw;  /* Literal address is from stack - not kmem. */
	while (m_ptr->d_next != second_link) {
		m_ptr = modsw.d_next;
		if (readmem(&modsw, m_ptr, sizeof(modsw), rflag) !=
				sizeof(modsw)) {
			printf(catgets(scmc_catd, MS_crsh, DMODSW_MSG_2,
			    "0452-1150: Cannot locate the modsw structure.\n"));
		}

		if (second_link == NULL)
			second_link = m_ptr;

		printf("%-10.10s %8x %8x %8x  0x%x %8x  %8x %5d %5d %5d\n",
			modsw.d_name, m_ptr, modsw.d_next, modsw.d_prev,
			modsw.d_flags, modsw.d_sqh, modsw.d_str,
			modsw.d_sq_level, modsw.d_refcnt, modsw.d_major);

		m_ptr = &modsw;
	}
}

