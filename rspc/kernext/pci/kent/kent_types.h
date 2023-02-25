/* @(#)67       1.4  src/rspc/kernext/pci/kent/kent_types.h, pcient, rspc41J, 9515A_all 4/11/95 21:21:34 */
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_KENT_TYPES
#define _H_KENT_TYPES

#include <sys/ndd.h>
#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/dma.h>
#include <sys/err_rec.h>
#include <sys/cdli_entuser.h>
#include <sys/ethernet_mibs.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/pm.h>
#include "kent_dds.h"

/*
 * initialization block structure : this is the structure used by the adapter
 * to get the initialization data.
 */
struct kent_init_blk
{
	int	mode;
	int	padr1;
	int	padr2;
	char	ladrf[KENT_LADRF_LEN];
	int 	rdra;
	int 	tdra;
};
typedef struct kent_init_blk kent_init_blk_t;

/*
 * 	Adapter tx and rx descriptor:
 */
struct kent_desc
{
	ulong	addr;
	ulong	stat1;
	ulong	stat2;
	ulong	stat3;
};
typedef struct kent_desc kent_desc_t;

typedef char kent_addr_t[ENT_NADR_LENGTH];

/* -------------------------------------------------------------------- */
/*	ACS Transmit Control Variables					*/
/* -------------------------------------------------------------------- */

struct kent_tx
{

	Simple_lock	lock; 		/* The lock for the tx path */
	kent_desc_t	*desc;		/* Pointer to the tx descriptors */
	uint		in_use;		/* number of tx desc set for tx */
	uint		nxt_tx;		/* index where next tx goes */
	uint		nxt_cmplt;	/* index where next complete will be */
	uint		p_d_desc;	/* dma base address for tx desc */
	char		*p_buffer;	/* address for tx buffers */
	struct watchdog	wdt;		/* tx watchdog timer structure */
};
typedef struct kent_tx kent_tx_t;

/* -------------------------------------------------------------------- */
/*	ACS Receive Control Variables					*/
/* -------------------------------------------------------------------- */

/*
 * Host RECEIVE descriptor
 */
struct	kent_rx
{
	kent_desc_t	*desc;		/* pointer to the rx descriptors */
	uint		nxt_rx;		/* index where to the next rx desc */
	uint		p_d_desc;	/* dma base address for rx desc */
	char		*p_buffer;	/* address for rx buffers */
};
typedef struct kent_rx kent_rx_t;



/* -------------------------------------------------------------------- */
/*	ACS Device Driver Variables					*/
/* -------------------------------------------------------------------- */

struct kent_dev
{
	int		state;   	/* 
					 * This field holds the state of 
					 * the device driver
					 */
	int		pmh_state;   	/* 
					 * This field holds the state of 
					 * the device driver prior to a 
					 * suspend/hibernate
					 */
	int		seq_number;	/*
					 * This field holds the sequence
					 * number the acs was configured
					 * with 
					 */
	/* -------------------------------------------- */
	/*  The lock sub section 		 	*/
	/* -------------------------------------------- */

	Simple_lock	ctl_lock; 	/* The lock for the ctl section */
	Simple_lock	slih_lock; 	/* The lock for the slih routine */
	Complex_lock	dd_clock;	/* The lock for the open/close sync */
	

	/* -------------------------------------------- */
	/* The counter section.  For the ctl options    */
	/* -------------------------------------------- */
	uint		multi_cnt;
	uint		prom_cnt;
	uint		bf_cnt;

	/* -------------------------------------------- */
	/*	Misc					*/
	/* -------------------------------------------- */
	char		chip_rev;	/* used to hold the version id of the */
					/* amd chip */

	int		pio_rc;		/* Set if there has been a pio error */
	int		iox;		/* Last PIO exception code */
	time_t		stime;		/* The starting time (lbolt) */
	uint		p_d_init;	/* dma base address for init blk */

	int		pmh_event;	/* sleep call : while in suspend mode */

	struct 	io_map	iomap;
	struct 	d_handle *dh;
};
typedef struct kent_dev kent_dev_t;

/* -------------------------------------------------------------------- */
/*	ACS Address section						*/
/* -------------------------------------------------------------------- */
struct kent_addr_elem
{
	kent_addr_t	addr;
	int		addr_cnt;
};
typedef struct kent_addr_elem kent_addr_elem_t;

#define KENT_MAX_ADDR_BLK (20)

struct kent_addr_blk
{
	struct kent_addr_blk 	*next;
	struct kent_addr_blk 	*prev;
	int			blk_cnt;
	kent_addr_elem_t 	addrs[KENT_MAX_ADDR_BLK];
};
typedef struct kent_addr_blk kent_addr_blk_t;

struct kent_addrs
{
	kent_addr_t	src_addr;	/* 
					 * Holds the card's source 
					 * address 
					 */

	kent_addr_blk_t	*addrs;		/* 
					 * Holds the group addresses set
					 */

	int		addr_cnt;	/* 
					 * Holds the count of group addresses
					 * set
					 */

	char		ladrf[KENT_LADRF_LEN];	/* 
					 * Holds the group address flag 
					 * currently set on the adapter
					 */
};
typedef struct kent_addrs kent_addrs_t;
	

/* -------------------------------------------------------------------- */
/* Adapter control structure 						*/
/* -------------------------------------------------------------------- */
struct kent_acs 
{
	struct intr		ihs;	/* interrupt handler structure */
	struct kent_acs		*next_acs;	/* 
						 * pointer to next acs in 
						 * chain
						 */
	ndd_t			ndd; 	/* 
					 * The copy of the ndd structure for
					 * this adapter 
					 */
	kent_dds_t		dds;	/* Define Device Structure */
	kent_tx_t		tx;	/* tx control area */
	kent_rx_t		rx;	/* rx control area */
	kent_dev_t		dev;	/* device control area */
	kent_addrs_t		addrs;  /* address area */
	kent_init_blk_t		init;

	kent_all_stats_t	ls;  	/* Holds the link statistics */
	ethernet_all_mib_t	mibs;	/* Holds the mib counters */
	struct pm_handle	pmh;	/* Holds the Power Management */
					/*  structure */
};
typedef struct kent_acs kent_acs_t;

/* -------------------------------------------------------------------- */
/* 			kent Fixed storage area				*/
/* -------------------------------------------------------------------- */

#ifdef KENT_DEBUG
#define KENT_TRACE_SIZE		(500*4) /* max number of trace table entries */
#else
#define KENT_TRACE_SIZE		(32*4) /* max number of trace table entries */
#endif

struct kent_trace
{
	int 	res[3];
	int	next;	/* next index hole to put trace data in the table */
	int	res1;
	int	res2;
	int	res3;
	ulong	table[KENT_TRACE_SIZE];
};
typedef struct kent_trace kent_trace_t;

struct kent_tbl
{
	lock_t		table_lock;		/* the dd's global lock */
	int		acs_cnt;		/* # of acs's we have */
	int		open_cnt; 		/* Count of active opens */
	kent_acs_t 	*p_acs;			/* acs ptr for each adap */
	Simple_lock	trace_lock;		/* Locks internal trace table*/
	kent_trace_t	trace;			/* Internal trace table */
};
typedef struct kent_tbl kent_tbl_t;

/* -------------------------------------------------------------------- */
/*      Structures for Error Logging					*/
/* -------------------------------------------------------------------- */

struct kent_errlog
{
	struct err_rec0	errhead;
	char		file[32];	/* file name of error */
	ent_genstats_t  ent_genstats;
	uint		state;		
	uint		iox;		/* Set to the last PIO error */
	uint		pio_rc;		/* Set if there has been a fatal pio */
					/* exception */
	uint		status1; 	/* The status fields hold three */
	uint		status2;	/* words of local driver information */
	uint		status3;
	uchar		src_addr[ENT_NADR_LENGTH];
};
typedef struct kent_errlog kent_errlog_t;

#endif /* endif ! _H_KENT_TYPES */
