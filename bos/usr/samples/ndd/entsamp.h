/* @(#)33  1.1  src/bos/usr/samples/ndd/entsamp.h, entsamp, bos411, 9428A410j 1/14/94 13:37:11 */
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
 */

#ifndef _H_ENTSAMP
#define _H_ENTSAMP


/*****************************************************************************/
/*           trace table and component dump table                 	     */
/*****************************************************************************/

typedef struct {
   Simple_lock	trace_slock;			/* lock for the trace table */
   int		next_entry;			/* index to put data next */
   ulong	table[ENTSAMP_TRACE_SIZE];	/* trace table */
} entsamp_trace_t;

typedef struct {
   int		count;				/* number of cdt entries used */
   struct cdt_head  head;			/* cdt head */
   struct cdt_entry entry[ENTSAMP_CDT_SIZE];	/* cdt entries */
} entsamp_cdt_t;

/*****************************************************************************/
/*                      device states                                        */
/*****************************************************************************/

typedef enum {
	CLOSED = 0,                   	/* initial device state */
	DEAD,				/* fatal hardware error encountered */
	LIMBO,				/* error recovery period */
	OPEN_PENDING,			/* open initiated */
	OPENED,                       	/* opened successfully, functioning */
	CLOSE_PENDING                 	/* close initiated */
} DEVICE_STATE;

/*****************************************************************************/
/*           transmit/receive buffer descriptor control structure	     */
/*****************************************************************************/

typedef struct entsamp_bdc {
	struct entsamp_bdc *next;	/* point to the next descriptor */
	int	flags;			/* flag for marking used	*/
#define BDC_INITED	0x00000001	/* buffer initialized */
#define BDC_IN_USE	0x00000002	/* buffer in use */
#define BDC_BCAST	0x00000004	/* broadcast transmit */
#define BDC_MCAST	0x00000008	/* multicast transmit */
        char    *buf;			/* address of buffer		*/
	char    *dma_io;		/* dma io addresses   */
	ushort	bd_off;			/* offset of the BD on the adapter */
	int	tx_len;			/* length of data. used by tx   */
} entsamp_bdc_t;

/*****************************************************************************/
/*           device VPD save area                                      	     */
/*****************************************************************************/

#define MAX_SAMP_VPD_LEN	128	/* max vpd size for the adapter */

typedef struct entsamp_vpd {  
   ulong status;             		/* vpd status value       	*/
   ulong length;             		/* number of bytes actually returned */
   uchar vpd[MAX_SAMP_VPD_LEN];		/* vital product data characters     */
} entsamp_vpd_t;

/*****************************************************************************/
/* 	This is the dds structure for the entsamp device		     */
/*****************************************************************************/

typedef struct {	
      int   bus_type;       /* for use with i_init                           */
      int   bus_id;         /* for use with i_init and bus io                */
      int   intr_level;     /* for use with i_init                           */
      int   intr_priority;  /* interruct priority, not used                  */
      int   xmt_que_size;   /* one queue per device for transmit buffering   */

      uchar  lname[ERR_NAMESIZE];        /* logical name in ASCII characters */
      uchar  alias[ERR_NAMESIZE];        /* alias in ASCII characters */
      ulong  bus_mem_addr;               /* bus memory base address          */
      int    bus_mem_size;               /* size of bus memory region        */
      uchar *tcw_bus_mem_addr;           /* tcw bus memory base address      */
      long   tcw_bus_mem_size;           /* size of tcw bus memory region    */
      ulong  io_port;                    /* io port base for PS/2,etc        */
      int    slot;                       /* slot for this adapter            */
      int    dma_arbit_lvl;              /* DMA Arbitration Level            */
      int    use_alt_addr;               /* non-zero => use alt network addr */
      uchar  alt_addr[ENT_NADR_LENGTH];  /* alternate network address        */
      int    bnc_select;                 /* transceiver select               */
} entsamp_dds_t;

/*****************************************************************************/
/* 	This is the entsamp device specific statistics table 		     */
/*****************************************************************************/

typedef struct {	
      int    adapter_status;		/* special flag for adapter status   */
} entsamp_stats_t;	

struct entsamp_all_stats {
        struct  ndd_genstats ent_ndd_stats;     /* network neneric stats */
        struct  ent_genstats ent_gen_stats;     /* ethernet generic stats */
        entsamp_stats_t entsamp_stats;      	/* device specific stats */
};
typedef struct entsamp_all_stats entsamp_all_stats_t;

/*****************************************************************************/
/*                  Work section of device control structure		     */
/*****************************************************************************/

typedef struct {
  uchar         net_addr[ENT_NADR_LENGTH];/* actual network address in use */
  uchar         pos_reg[NUM_POS_REG];   /* POS registers image */
  int	     	dma_channel;		/* for use with DMA services      */
  int		channel_alocd;		/* DMA channel state		*/
  int           tx_buf_alocd;           /* transmit buffer allocated or not */
  int           rv_buf_alocd;           /* receive buffer allocated or not */
  int		intr_inited;		/* flag for interrupt registration */
  int		tx_wdt_inited;		/* flag for tx watchdog registration */
  int		ctl_wdt_inited;		/* flag for ctl watchdog registration */
  int 		restart_count;		/* times the error recovery performed */
  int		promiscuous_count;	/* promiscuous mode reference count */
  int		badframe_count;		/* save bad packet reference count */
  int		otherstatus;		/* all other async status ref count */
  int		enable_multi;		/* enable multicast reference count */
  int		filter_count;		/* number of filters registered */
  int		multi_count;		/* total multicast addr. registered */
  ulong         tx_list_off;            /* Transmit List Start Offset      */
  ulong         rv_list_off;            /* Receive  List Start Offset      */
  uint          tx_tcw_base;            /* bus base addr for TX buffers   */
  uint          rv_tcw_base;            /* bus base addr for receive buffer */
  short         tx_tcw_cnt;             /* Transmit List Count via TCWs    */
  short         rv_tcw_cnt;             /* Receive List Count  via TCWs    */
  uchar         *tx_buf;                /* beginning of the tx buffer pool */
  uchar         *rv_buf;                /* beginning of the rv buffer pool */
  struct xmem   txbuf_xmem;             /* xmem descriptor for transmit buf */
  struct xmem   rvbuf_xmem;             /* xmem descriptor for receive buf */
  entsamp_bdc_t  *txd_first;            /* txd queue first pointer */
  entsamp_bdc_t  *txd_last;             /* txd queue last pointer  */
  entsamp_bdc_t  *txd_avail;            /* first available txd on the txd q */
  entsamp_bdc_t  *rvd_first;            /* rvd queue first pointer */
  entsamp_bdc_t  *rvd_last;             /* rvd queue last pointer */
  entsamp_bdc_t  txd[ENTSAMP_TX_LIST];        /* txd list */
  entsamp_bdc_t  rvd[ENTSAMP_RV_LIST];        /* rvd list */
} entsamp_wrk_t;

/***************************************************************************/
/*
 * This is the whole device control area				
 */
/***************************************************************************/
 

struct entsamp_dev_ctl {
      	struct intr     ihs; 		/* interrupt handler ctl struct */
	int 		ctl_correlator; /* point to the dd_ctl table    */
	ndd_t		ndd;		/* ndd for NS ndd chain		*/
	struct entsamp_dev_ctl *next;	/* point to the next device 	*/
	int		seq_number;	/*  sequence number */
  	Complex_lock	ctl_clock;	/* control operation lock */
	Simple_lock	cmd_slock;	/* adatper command lock */
	Simple_lock	tx_slock;	/* transmit lock  */
	Simple_lock	slih_slock;	/* SLIH lock */
  	DEVICE_STATE	device_state;   /* main state of the device */
	int		ndd_stime;	/* start time of the ndd statistics */
	int		dev_stime;	/* start time of the dev statistics */
	int		txq_len;	/* current length of transmit queue */
	struct mbuf	*txq_first;	/* transmit queue */
	struct mbuf	*txq_last;	/* transmit queue */
  	int		tx_pending;	/* number of transmit outstanding */
  	int		ctl_status;	/* ioctl command status */
	int 		ctl_pending;	/* ioctl command outstanding flag */
	int		ctl_event;	/* sleep event for ent_ioctl */
	entsamp_dds_t	dds;		/* device dependent structure */
	struct watchdog	tx_wdt;		/* watchdog timer for transmit  */
	struct watchdog	ctl_wdt;	/* watchdog timer for ioctl  	*/
	struct trb      *systimer;	/* system timer for error recovery */
      	entsamp_vpd_t   vpd; 		/* vital product data */
	ent_genstats_t	entstats;	/* ethernet generic statistics */
	entsamp_stats_t	devstats;	/* entsamp spcific statistics */
	ethernet_all_mib_t	mibs;	/* ethernet MIB's 		*/
      	entsamp_wrk_t    wrk; 		/* device work area             */
};

typedef struct entsamp_dev_ctl entsamp_dev_ctl_t;


/***************************************************************************/
/*
 * This is the global device driver control structure
 */
/***************************************************************************/

struct entsamp_dd_ctl {
  lock_t		cfg_lock;		/* lockl lock for config */
  Complex_lock		dd_clock;		/* device driver lock */
  entsamp_dev_ctl_t  	*p_dev_list;		/* device control list */
  int 			num_devs;		/* count of devices configed */
  int 			open_count;		/* count of devices opened */
  entsamp_trace_t	trace;			/* device driver trace table */
  entsamp_cdt_t		cdt;			/* device drvier dump table */
};

typedef struct entsamp_dd_ctl entsamp_dd_ctl_t;


/*
 * Macros for accessing device control area. The pointer to this area has to 
 * be named p_dev_ctl for using these macros.
 */

#define IHS 	p_dev_ctl->ihs
#define NDD 	p_dev_ctl->ndd
#define TXWDT 	p_dev_ctl->tx_wdt
#define CTLWDT 	p_dev_ctl->ctl_wdt
#define VPD 	p_dev_ctl->vpd
#define DDS 	p_dev_ctl->dds
#define ENTSTATS	p_dev_ctl->entstats
#define DEVSTATS	p_dev_ctl->devstats
#define MIB	p_dev_ctl->mibs
#define WRK 	p_dev_ctl->wrk
#define CFG_LOCK	entsamp_dd_ctl.cfg_lock
#define DD_LOCK		entsamp_dd_ctl.dd_clock
#define TRACE_LOCK	entsamp_dd_ctl.trace.trace_slock
#define CTL_LOCK 	p_dev_ctl->ctl_clock
#define CMD_LOCK 	p_dev_ctl->cmd_slock
#define TX_LOCK 	p_dev_ctl->tx_slock
#define SLIH_LOCK 	p_dev_ctl->slih_slock


/*****************************************************************************/
/*                  Error logging type definition                            */
/*****************************************************************************/

#define ENTSAMP_FNAME_LEN	32

struct error_log_def {
	struct err_rec0   errhead;        /* from com/inc/sys/err_rec.h      */
	uchar fname[ENTSAMP_FNAME_LEN];   /* filename and line number        */
	uchar pos_reg[8];       	  /* Adapter POS Registers           */
	uchar ent_addr[ENT_NADR_LENGTH];  /* actual net address in use       */
	ulong parm1;                      /* log data 1 		     */
	ulong parm2;                      /* log data 2 		     */
	ulong parm3;                      /* log data 3 		     */
};

#endif /* _H_ENTSAMP */
