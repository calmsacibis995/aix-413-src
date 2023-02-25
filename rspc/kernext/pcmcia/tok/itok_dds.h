/* @(#)56       1.5  src/rspc/kernext/pcmcia/tok/itok_dds.h, pcmciatok, rspc41J, 9515A_all 4/10/95 02:49:32 */
/* @(#)55	1.3  src/rspc/kernext/isa/itok/itok_dds.h, isatok, rspc41J 9/8/94 14:24:45 */
/*
 * COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TOKDDS.H
#define _H_TOKDDS.H

/* Max. number of adapters. */
#define TOK_MAX_MINOR		2      /* There can never be more than 2      */

/* Adapter states (easily read when in the kernel debugger.) */
#define DEAD_STATE	0xdead
#define NULL_STATE	0xffff
#define OPEN_PENDING	0xcafe
#define OPEN_STATE	0xaaaa
#define CLOSED_STATE    0xbbbb
#define CLOSE_PENDING   0xcccc

typedef struct TOK_ERROR_LOG_SENSE_DATA
{
   struct        err_rec0    errhead;
   int           adap_state;     /* Adapter State */
   ushort        ring_status;    /* Last Ring Status */
   ushort        tx_cstat;       /* Last TX CSTAT */
   ushort        rcv_cstat;      /* Last RCV CSTAT */
   int           io_addr;        /* Adapter's I/O addr. (0xA20, 0xA24) */
   uchar         tok_addr[CTOK_NADR_LENGTH]; /* Network Address in use */
   uchar         hw_addr[CTOK_NADR_LENGTH];  /* Burned in hw-address */
   tok_error_log ring_errors;    /* Last DIR.READ.LOG result. */
} tok_error_log_data_t;

typedef struct xmt_elem_t {         /* a transmit queue element      */
  struct xmt_elem_t *next;          /* Next free or busy tx-element. */
  struct mbuf       *mbufp;         /* pointer to mbuf with data     */
  ushort            bytes;          /* number of bytes in packet     */
  uchar             in_use;         /* use flag                      */
} xmt_elem_t;

#define GET_FREE_ELEM(x) { WRK.xmits_queued++;                            \
                           x = WRK.tx_next_free;                          \
                           WRK.tx_next_free = WRK.tx_next_free->next;}        
                    
#define ADD_2_FREE_Q(x) { WRK.xmits_queued--;                        \
                       if (!WRK.tx_next_free) {                      \
                         WRK.tx_next_free = x;                       \
                         WRK.tx_last_free = x;                       \
                       } else {                                      \
                           x->next = WRK.tx_next_free;               \
                           WRK.tx_next_free = x;                     \
                       }}
                    
#define GET_ARB_ELEM(x) { WRK.tx_arb_waiting--;                       \
                          x = WRK.tx_next_arb;                        \
                          if (WRK.tx_arb_waiting <= 0) {              \
                            WRK.tx_next_arb = NULL;                   \
                            WRK.tx_last_arb = NULL;                   \
                          } else                                      \
                              WRK.tx_next_arb = WRK.tx_next_arb->next;}        

#define ADD_2_ARB_Q(x) { WRK.tx_arb_waiting++;                        \
                         if (!WRK.tx_next_arb) {                      \
                           WRK.tx_next_arb = x;                       \
                           WRK.tx_last_arb = x;                       \
                         } else {                                     \
                             WRK.tx_last_arb->next = x;               \
                             WRK.tx_last_arb = x;                     \
                         }}

#define WAITQ_FULL (WRK.xmits_queued >= DDI.xmt_que_size)

/* Active xmit-list macros. */
#define XMITQ_FULL (WRK.tx_buf_in_use >= OPEN_XMIT_BUF_COUNT)

typedef struct {
   ulong intr_lost;         /* Not used.                            */
   ulong wdt_lost;          /* number of tx. intr's lost.           */
   ulong timo_lost;         /* Not used.                            */
   ulong sta_que_overflow;  /* status lost due to full status que   */
   ulong rec_que_overflow;  /* rcv packet lost due to full recv que */
   ulong rec_no_mbuf;       /* no mbuf available for receive        */
   ulong rec_no_mbuf_ext;   /* Not used.                            */
   ulong recv_intr_cnt;     /* number of receive interrupts         */
   ulong xmit_intr_cnt;     /* number of transmit interrupts        */
   ulong ctr_pkt_rej_cnt;   /* Packets Rejected No NetID            */
   ulong pkt_acc_cnt;       /* Packets Accepted Valid NetID         */
   ulong rcv_byt_cnt;       /* Not used.                            */
   ulong trx_byt_cnt;       /* Not used.                            */
   ulong pkt_trx_cnt;       /* Not used.                            */
   ulong ovflo_pkt_cnt;     /* Overflow Packets Received            */
   ulong adap_tx_err_cnt;   /* Adapter Transmission errors.         */
} tok_stats_t;

/* Structures used for saving off data for use in the timeout function. */
typedef struct {
  void  *dds_ptr;
  int   cmd;
} timer_data_t;

typedef struct {
  uint  adapter_present;      /* True or false.                       */
  uint  bios_addr;            /* C0000, C2000, ... , DE000.           */
  uint	io_base;              /* 0x0A20 (primary adap.),0x0A24 (sec.) */
  uint	intr_priority;        /* AT-level 2,3,6 or 7.                 */
  uint	shared_ram_width;     /* 8Kb, 16 Kb, 32 Kb or 64 Kb.          */
  aip_t	aip;                  /* Adapter Identification Information.  */
} adap_cfg_t;

/*
  This is the adapter related structure.
*/

typedef struct {
  Simple_lock   tx_slock;       /* transmit/control lock  */
  Simple_lock   slih_slock;     /* SLIH lock */

  int		minor_no;	/* 0 or 1. index of the adap_cfg in ddctrl  */
  int           adap_state;     /* Current adapter state.                   */
  int           open_status;    /* Current open status.                     */
  int           open_retries;   /* Current nbr of open attempts.            */
  int           last_open;      /* Result of the last open (for MIBS).      */
  int           alloc_size;     /* Size of the dds.                         */
  int           ndd_stime;      /* Ndd statistics start time.               */
  int           dev_stime;      /* Device statistics start time.            */
  /* ______________________________________________________________________ */
  /*                      Adapter command areas                             */
  aca_t		aca;            /* Adapter Control Area.                    */
  srb_t		srb;		/* System Request Block.		    */
  ssb_t		ssb;		/* System Status Block.  		    */
  arb_t		arb;		/* Adapter Request Block.		    */
  asb_t		asb;		/* Adapter Status Block.		    */
  /* ______________________________________________________________________ */
  /*                      Adapter information offsets                       */
  ushort	adap_addr_offset; /* Offset to the adapters addresses.      */
  ushort	adap_param_offset;/* Offset to the adapters parameters.     */
  /* ______________________________________________________________________ */
  /*                          Misc. variables                               */
  int           max_frame_len;  /* Max. packet size.                        */
  int		mac_frame_active;/* Are we receiving MAC frames ?           */
  int		command_to_do;   /* Do we have some oust. work when opened. */
  /* ______________________________________________________________________ */
  /*                Group & functional address variables                    */
  int           func_addr_ref[31]; /* Reference counts for each bit.        */
  /* ______________________________________________________________________ */
  int           sap_allocated[255]; /* Ref. counts for each SAP.            */
  /* ______________________________________________________________________ */
  /*                        Wait event flags                                */
  int           grp_addr_event; /* For e_sleep and e_wakeup use only.       */
  int           func_addr_event;/* For e_sleep and e_wakeup use only.       */
  int           read_log_event; /* For e_sleep and e_wakeup use only.       */
  int           open_wait_event;/* For e_sleep and e_wakeup use only.       */
  int           sap_open_event; /* For e_sleep and e_wakeup use only.       */
  int           sap_close_event;/* For e_sleep and e_wakeup use only.       */
  int           close_event;    /* For e_sleep and e_wakeup use only.       */
  int           srb_wait_event; /* For e_sleep and e_wakeup use only.       */

  int		cmd_event;	/* Bit pattern of outst. commands, also     */
                                /* contains a wait indication.              */
#define TOK_GROUP_ADDR_EVENT_BIT	0x01
#define TOK_FUNC_ADDR_EVENT_BIT		0x02
#define TOK_READ_LOG_EVENT_BIT		0x04
#define TOK_OPEN_EVENT_BIT		0x08
#define TOK_SAP_CLOSE_EVENT_BIT		0x10
#define TOK_CLOSE_EVENT_BIT		0x20
#define TOK_SAP_OPEN_EVENT_BIT		0x40
#define TOK_WAKEUP_BIT			0x80
  /* ______________________________________________________________________ */
  /*                        Xmit structures                                 */

  xmt_elem_t    *tx_waitq;                     /* Wait q for xmit's.        */
  xmt_elem_t    *xmit_correlators[TOK_MAX_CORRELATOR+1]; /* XMit handles.   */
  uint  	xmits_queued;                  /* Nbr in wait list.         */
  int  		tx_srb_waiting;                /* Nbr waiting on the srb.   */
  int  		tx_arb_waiting;                /* Nbr waiting on the arb.   */
  xmt_elem_t  	*tx_next_free;                 /* First free tx-element.    */
  xmt_elem_t  	*tx_last_free;                 /* Last free tx-element.     */
  xmt_elem_t  	*tx_next_srb;                  /* First 'srb waiting' tx.   */
  xmt_elem_t  	*tx_last_srb;                  /* Last 'srb waiting' tx.    */
  xmt_elem_t  	*tx_next_arb;                  /* First 'arb waiting' tx.   */
  xmt_elem_t  	*tx_last_arb;                  /* Last 'arb waiting' tx.    */
  uint  	tx_buf_in_use;                 /* How many buffers in use.  */
  int           tx_stat;                       /* Last tx status.           */
  uint          xmit_wdt_active;      /* Is the timer currently active.     */
  ushort        station_ids[TOK_MAX_SAPS+1];   /* SAP table.                */
  void          (*tx_func)();         /* Points to a 8 or 16 bit wide func. */
  /* ______________________________________________________________________ */
  /*                        Receive structures                              */

  int           rcv_stat;             /* Last receive status.               */
  int           (*rx_func)();         /* Points to a 8 or 16 bit wide func. */

  /* ______________________________________________________________________ */
  uint            init_wdt_active;    /* Is the timer currently active.     */
  uint            cmd_wdt_active;     /* Is the timer currently active.     */
  uint            srb_wdt_active;     /* Is the timer currently active.     */
  uint		  xmit_wdt_opackets;  /* How many did we have when started. */
  open_param_t	  open_cfg;           /* Open cfg. sent to the adapter.     */
  /* ______________________________________________________________________ */
  /*                            Dump variables                              */
  int             dump_read_started;  /* Receive processing flag.           */
  int             dump_first_wrt;     /* Transmission processing flag.      */
  /* ______________________________________________________________________ */
  /*                        Misc. status & statistics                       */
  uint            srb_waiting;        /* No. cmds. waiting on the SRB area. */
  uint            intr_action;        /* Some intr thread cmd. waiting ?    */
  tok_ring_info_t ring_info;          /* Various ring information.          */
  tok_error_log   tok_network_errors; /* Errors reported by last READ_LOG   */
  ushort          network_status;     /* Used when reporting MIBS info.     */
} dds_wrk_t;

/*
  This is the Device Driver Structure (DDS) which contains all the
  necessary information for the correct operation of a tokenring adapter.
*/

typedef struct dds {
  struct intr		ihs;        /* Interrupt handler control struct.     */
  struct dds            *next;      /* Pointer to the next adapter structure.*/
  int                   seq_number; /* Ndd sequence number for this device.  */
  struct ndd            ndd;        /* Ndd structure for this device.        */
  itok_ddi_t		ddi;        /* Configuration passed from the method. */
  itok_vpd_t            vpd;        /* Vital product data (tok-addr.).       */

  /* Don't add variables between the following two structures. */
  struct watchdog	init_wdt;    /* Command watchdog timer control struct.*/
  struct dds            *init_wdt_dds; /* Pointer to appropriate dds.         */

  /* Don't add variables between the following two structures. */
  struct watchdog	cmd_wdt;    /* Command watchdog timer control struct.*/
  struct dds            *cmd_wdt_dds; /* Pointer to appropriate dds.         */

  /* Don't add variables between the following two structures. */
  struct watchdog	xmit_wdt;    /* Xmit watchdog timer control struct.  */
  struct dds            *xmit_wdt_dds; /* Pointer to appropriate dds.        */

  /* Don't add variables between the following two structures. */
  struct watchdog	srb_wdt;     /* SRB watchdog timer control struct.   */
  struct dds            *srb_wdt_dds; /* Pointer to appropriate dds.         */

  tok_genstats_t        tokstats;   /* Media specific statistics.            */
  tr_ibm_isa_stats_t    devstats;   /* Device specific statistics.           */
  dds_wrk_t		wrk;        /* Device specific work area.            */
#ifdef PCMCIATOK
  int           clientID;           /* Client ID                             */
  int           event_word;         /* For a e_sleep/e_wokeup event          */
  int           configured;         /* Configured ? or not                   */
  int           windowID0;          /* For Request Window                    */
  int           windowID1;          /* For Request Window                    */
  int           return_callback;    /* Return value from Callback            */
  int           config_base;        /* Configuration base address            */
  int           config_index;       /* Default Index                         */
  int           vcc;                /* Normal power                          */
  int           no_card;            /* Any card into slot, or not            */
  int           reg_comp;           /* flag for registration completion      */
  int           cb_flag;	    /* -1: unconfig, 1: callback, 0: none    */
  int           net_status;  	    /* 1: NDD_UP    0: net down              */
#endif
#ifdef PM_SUPPORT
  struct        pm_handle pm;       /* pm handle structure                   */
#endif /* PM_SUPPORT */
} dds_t;

/* macros for accessing DDS contents require that the dds pointer be p_dds */
/* example: to access the netid table address use "CIO.netid_table"      */

#define IHS p_dds->ihs
#define NDD p_dds->ndd
#define DDI p_dds->ddi
#define VPD p_dds->vpd
#define WDT p_dds->wdt
#define INITWDT p_dds->init_wdt
#define XMITWDT p_dds->xmit_wdt
#define CMDWDT p_dds->cmd_wdt
#define SRBWDT p_dds->srb_wdt
#define TOKSTATS p_dds->tokstats
#define DEVSTATS p_dds->devstats
#define WRK p_dds->wrk
#define TX_LOCK WRK.tx_slock
#define SLIH_LOCK WRK.slih_slock


#ifdef DEBUG
#define TRACE_TABLE_SIZE (500)       /* SAVETRACE table (ulongs)      */
#else
#define TRACE_TABLE_SIZE (60)        /* SAVETRACE table (ulongs)      */
#endif /* DEBUG */

typedef volatile struct {
   Simple_lock trace_slock;              /* lock for the trace table       */
   ulong       nexthole;                 /* index into table for next add  */
   ulong       table[TRACE_TABLE_SIZE];  /* ring buffer of trace data      */
} tracetable_t;

/* enough for trace table and, for each adap, the dds, open structs, and ds  */
#define MAX_CDT		1
#define MAX_CDT_ELEMS (1 + (TOK_MAX_MINOR * 2))

typedef struct {
   struct cdt_head  header;
   struct cdt_entry entry[MAX_CDT_ELEMS];
} cdt_t;

/*
  This is the major Device Driver Control Structure which contains all the
  information about the tokenring adapters in the system (max. of 2).
  All structures that are needed for the DRIVER to function correctly needs
  to go into this structure. All structures that are specific to one adapter
  (one minor) needs to go into the structures that are indexed by minor
  number (..[TOK_MAX_MINOR]) i.e. dds_t.
*/

typedef struct {           
  lock_t        cfg_lock;               /* For tokconfig lockl use only.      */
  Simple_lock   dd_slock;               /* Initialization lock.               */
  dds_t	        *p_dds_list;            /* One for each adapter.              */
  int	        num_adapters;           /* Number found on the isa bus.       */
  int	        num_devs;               /* Number devices configured.         */
  int	        initialized;            /* Has the driver been initialized.   */
  int	        num_opens;              /* Nbr of devices opened.             */
  tracetable_t  tracetable;             /* Trace table for debugging.         */
  cdt_t         cdt;                    /* Dump table.                        */
  struct io_map *io_map_ptr;            /* For doing in/out-bytes.            */
  struct io_map *mem_map_ptr;           /* For doing shared mem. accesses.    */
  adap_cfg_t    adap_cfg[TOK_MAX_MINOR];/* Adapter hw. configuration.         */
} dd_ctrl_t;

#define CFG_LOCK   dd_ctrl.cfg_lock
#define DD_LOCK    dd_ctrl.dd_slock
#define TRACE_LOCK dd_ctrl.tracetable.trace_slock

#endif /* if !_H_TOKDDS.H */
