/* "@(#)44  1.2  src/rspc/kernext/pci/stok/stok_dd.h, pcitok, rspc41J, 9515A_all 4/11/95 12:38:34" */
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_STOK_DD
#define _H_STOK_DD

#include <sys/pm.h>

#define   STOK_DD_LOCK           	XSTOK_DD_LOCK	
#define   STOK_TRACE_LOCK           	XSTOK_TRACE_LOCK	
#define   STOK_TX_LOCK           	XSTOK_TX_LOCK	
#define   STOK_CTL_LOCK           	XSTOK_CTL_LOCK	
#define   STOK_CMD_LOCK           	XSTOK_CMD_LOCK	
#define   STOK_SLIH_LOCK           	XSTOK_SLIH_LOCK	

/*****************************************************************************/
/*                      device states                                        */
/*****************************************************************************/

typedef enum {
        CLOSED = 0,                     /* initial device state             */
        OPEN_PENDING,                   /* open initiated                   */
        OPENED,                         /* opened successfully, functioning */
        CLOSE_PENDING,                  /* close initiated                  */
        LIMBO,                          /* error recovery period            */
        DEAD,                           /* fatal hardware error encountered */
	SUSPEND_STATE			/* power management mode            */
} DEVICE_STATE;

/*****************************************************************************/
/*           trace table and component dump table                            */
/*****************************************************************************/

typedef struct {
   Simple_lock  trace_slock;            	/* lock for the trace table */
   int          next_entry;                     /* index for the next entry */
   ulong        table[STOK_TRACE_SIZE];          /* trace table              */
} stok_trace_t;

typedef struct {
   int		    cdt_max_entry;             /* Maximun cdt table entry    */
   int              count;                     /* number of cdt entries used */
   struct cdt_head  head;                      /* cdt head                   */
   struct cdt_entry entry[STOK_CDT_SIZE];      /* cdt entries                */
} stok_cdt_t;

/*****************************************************************************/
/*           multicast address registration table                            */
/*****************************************************************************/

typedef struct multi_slot {
        char m_addr[CTOK_NADR_LENGTH];
        int  ref_count;
} multi_slot_t;

typedef struct stok_multi {
        struct stok_multi *next;         /* point to the table extension      */
        short  in_use;                  /* no. of slots in use in this table */
        multi_slot_t m_slot[MULTI_ENTRY];
} stok_multi_t;

/*****************************************************************************/
/*                  Transmit List type definition                            */
/*****************************************************************************/
typedef struct xmit_buf {
  	ulong       	data_pointer ; 	/* data buffer pointer               */
  	ushort        	buf_len;        /* buffer length                     */
  	ushort 		reserved;       /* reserved                          */
} xmit_buf_t;

 typedef struct tx_list {
  	ulong       	fw_pointer;     /* transmit forward pointer          */
  	ulong       	xmit_status;    /* transmit status                   */
  	ushort        	frame_len;      /* frame length                      */
  	ushort 		buf_count;      /* data buffer length                */
        xmit_buf_t      xbuf[30];
  	ulong 		reserved;       /* reserved                          */
} tx_list_t;

struct xmit_elem {                      /* a transmit queue element          */
        struct mbuf     *mbufp;         /* pointer to mbuf with data         */
        short           bytes;          /* number of bytes in packet         */
        tx_list_t       *tx_ds;         /* xmit descriptor                   */
};
typedef struct xmit_elem xmit_elem_t;

/*****************************************************************************/
/*                  Receive List type definition                             */
/*****************************************************************************/
typedef struct rx_list {
  	ulong       	fw_pointer;     /* receive forward pointer           */
  	ulong      	rx_status;    /* receive status                    */
  	ulong       	data_pointer;   /* data buffer pointer               */
  	ushort 		data_len;       /* data buffer length                */
  	ushort        	fr_len;         /* frame length                      */
        ulong		reserve[60];
} rx_list_t;

/*****************************************************************************/
/*			Tx control structure			             */ 
/*****************************************************************************/

typedef struct tx {
        int          sw_txq_len;          /* current len of software tx queue */
        struct mbuf *sw_txq_first;        /* software transmit queue pointer  */
        struct mbuf *sw_txq_last;         /* software transmit queue pointer  */

        uchar      *tx_desc_blk;	  /* mem pointer to Tx descriptor blk*/
        tx_list_t  *tx_desc_p[MAX_TX_LIST];/* mem pointer to Tx descriptor blk*/
        uint        tx_dma_desc_p[MAX_TX_LIST];/* DMA pointer to Tx ddesc blk*/
        struct mbuf *hw_txq_in;           /* hardware transmit queue pointer */
        struct mbuf *hw_txq_out;          /* hardware transmit queue pointer */

        short   tx_elem_next_in;          /* Index to next in for adapter  */
        short   tx_elem_next_out;         /* next to be ACKed by adapter   */
        short   tx_frame_pending;         /* number of Tx frame pending    */

        short   tx_buf_next_in;           /* Index to next in for adapter  */
        short   tx_buf_use_count;         /* number of Tx buf used         */
        uchar   *tx_buf[MAX_BUF_LIST];    /* array of memory               */
        uint    tx_addr[MAX_BUF_LIST];    /* array of dma address          */

        short   tx_dma_next_in;           /* Index to next in for adapter  */
        short   tx_dma_next_out;          /* next to be ACKed by adapter   */
        short   tx_dma_use_count;         /* number of dma element used    */
        uint    tx_dma_addr[MAX_DMA_LIST];/* array of DMA memory           */

        uchar   max_buf_list;             /* #buffer driver will allocated */
        uint    lfda;                     /* TxLFDA register offset        */
        uint    disable;                  /* Tx channel disable bits       */
        uint    r_disable;                /* Tx channel disable request bits */

        uint    in;                /* Tx channel disable request bits */
} tx_t;

/*****************************************************************************/
/*                  Work section of device control structure                 */
/*****************************************************************************/

typedef struct stok_wrk {
      	int     ctl_event;                /* Event of ctl                    */
        int     pmh_event;             /* sleep call : while in suspend mode */
        int	promiscuous_count;        /* promiscuous count               */
      	int     retcode[RET_CODE_SIZE];   /* event return code               */

      	tok_adapter_log adap_log;         /* Adapter log                     */

      	/* Address of LAP registers                                          */
      	ushort  asb_address;        /* address of Aadapter Status Block      */
      	ushort  srb_address;        /* address of System Request Block       */
      	ushort  arb_address;        /* address of Adapter Request Block      */
      	ushort  trb_address;        /* address of Transmit Request Block     */
      	ushort  parms_addr;         /* address of adapter parameters         */

  	struct d_handle *dma_channel;     /* for use with DMA services       */

	/*
         *  Network address               
	 */
        uchar  stok_addr[CTOK_NADR_LENGTH];   /* actual net address in use    */

	/*
         * Information for transmit channel 1                          
	 */
	tx_t		tx1;	            	
	/*
         * Information for transmit channel 2                          
	 */
	tx_t		tx2;	            	
	/*
         * Information for receive list                                     
	 */
        uchar      *rx_desc_blk;        /* mem pointer to Rx descriptor block */
        rx_list_t  *rx_desc_p[MAX_INDEX]; /*mem array pointer to Rx desc block*/
        int         rx_dma_desc_p[MAX_INDEX]; /*DMA array ptr to Rx desc block*/

        int    read_index;            /* receive buffer array index         */
        struct mbuf *rx_mbuf[MAX_INDEX]; /* array of mbufs                  */
        uint   rx_addr[MAX_INDEX];    /* dma addrs of mbufs                 */
        int    eof;                   /* receive end of frame flag          */
        struct mbuf *mhead;           /* head of array of mbufs             */
        struct mbuf *mtail;           /* tail of array of mbufs             */

	/*
         * Dump variable 
	 */
        int          dump_started;
        int          dump_MISR;

  	int          multi_count;              /* total group addr. reg      */
  	stok_multi_t  multi_table;             /* first group addr. table    */
  	stok_multi_t  *multi_last;             /* last  group addr.          */
  	ulong        func_ref_cnt[LONG];       /* first func addr ref        */

        int          ndd_stime;        /* start time of the ndd statistics  */
        int          dev_stime;        /* start time of the dev statistics  */
        int          pio_rc;                  /* PIO error flag             */
        int	     re_limbo;		/* flag for re_enter_limbo          */
        int          hard_err;                /* hard error flag            */
        struct	     io_map	 iomap;                   
        struct 	     pm_handle   pmh; /* Holds the Power Management structure */
        int          limbo_err;       /* Error enter limbo number             */
} stok_wrk_t;

/****************************************************************************/
/*                                                                          */
/*    This is the whole device control area                                 */
/*                                                                          */
/****************************************************************************/


struct dev_ctl {
        struct intr     ihs;            /* interrupt handler ctl struct      */
        ndd_t           ndd;            /* ndd for NS ndd chain              */
        struct dev_ctl *next;       	/* point to the next device          */
        int             seq_number;     /* sequence number                   */
        Simple_lock     cmd_slock;      /* adatper command lock              */
        Simple_lock     tx_slock;       /* transmit lock                     */
        Simple_lock     slih_slock;     /* SLIH lock                         */
        DEVICE_STATE    device_state;   /* main state of the device          */
        DEVICE_STATE    pm_state;       /* This field holds the state of 
                                         * the device driver prior to a 
                                         * suspend/hibernate */
        uchar           vpd[MAX_STOK_VPD_LEN];  /* vital product data        */
        stok_dds_t       dds;           /* device dependent structure        */
        struct watchdog tx_wdt;         /* watchdog timer for transmit 1     */
        struct watchdog cmd_wdt;        /* watchdog timer for ioctl          */
        struct watchdog hwe_wdt;        /* watchdog timer for error revover  */
        struct watchdog lim_wdt;        /* watchdog timer for revover timer  */
        struct watchdog lan_wdt;        /* watchdog timer for lan status     */
        tok_genstats_t tokstats;        /* token ring generic statistics     */
        tr_sky_stats_t devstats;       /* STOK specific statistics          */
        token_ring_all_mib_t 	mibs;   /* STOK MIB's                        */
        stok_wrk_t       wrk;           /* skyline device work area          */
};

typedef struct dev_ctl stok_dev_ctl_t;

/*****************************************************************************/
/*                                                                           */
/* This is the global device driver control structure                        */
/*                                                                           */
/*****************************************************************************/

struct ndd_dd_ctl {
  lock_t                cfg_lock;               /* lockl lock for config     */
  stok_dev_ctl_t        *p_dev_list;  		/* device control list       */
  int                   num_devs;               /* count of devices configed */
  int                   open_count;             /* count of devices opened   */
  Complex_lock          dd_clock;               /* device driver lock        */
  stok_trace_t          trace;                  /* device driver trace table */
  stok_cdt_t            *cdt;                   /* device drvier dump table  */
};

typedef struct ndd_dd_ctl stok_dd_ctl_t;


/*
 * Macros for accessing device control area. The pointer to this area has 
 * to be named p_dev_ctl for using these macros.
 */

#define IHS     	p_dev_ctl->ihs
#define NDD     	p_dev_ctl->ndd
#define TXWDT   	p_dev_ctl->tx_wdt
#define HWEWDT  	p_dev_ctl->hwe_wdt
#define LIMWDT  	p_dev_ctl->lim_wdt
#define LANWDT  	p_dev_ctl->lan_wdt
#define CMDWDT  	p_dev_ctl->cmd_wdt
#define VPD     	p_dev_ctl->vpd
#define DDS     	p_dev_ctl->dds
#define TOKSTATS        p_dev_ctl->tokstats
#define DEVSTATS        p_dev_ctl->devstats
#define MIB   		p_dev_ctl->mibs
#define WRK     	p_dev_ctl->wrk
#define TX1     	p_dev_ctl->wrk.tx1
#define TX1_FRAME_PENDING     	p_dev_ctl->wrk.tx1.tx_frame_pending
#define TX2     	p_dev_ctl->wrk.tx2
#define TX2_FRAME_PENDING     	p_dev_ctl->wrk.tx2.tx_frame_pending

#define CFG_LOCK        stok_dd_ctl.cfg_lock
#define DD_LOCK         stok_dd_ctl.dd_clock
#define TRACE_LOCK      stok_dd_ctl.trace.trace_slock
#define CDT      	stok_dd_ctl.cdt
#define TX_LOCK         p_dev_ctl->tx_slock
#define CMD_LOCK        p_dev_ctl->cmd_slock
#define SLIH_LOCK       p_dev_ctl->slih_slock

#define FUNCTIONAL      MIB.Token_ring_mib.Dot5Entry 


/*****************************************************************************/
/*                  Error logging type definition                            */
/*****************************************************************************/


#define STOK_FNAME_LEN        32

struct error_log_def {
        struct err_rec0   errhead;        /* from com/inc/sys/err_rec.h      */
        uchar fname[STOK_FNAME_LEN];       /* filename and line number        */
        uchar stok_addr[CTOK_NADR_LENGTH]; /* actual net address in use       */
        ulong parm1;                      /* log data 1                      */
        ulong parm2;                      /* log data 2                      */
        ulong parm3;                      /* log data 3                      */
};

#endif /* _H_STOK_DD */

