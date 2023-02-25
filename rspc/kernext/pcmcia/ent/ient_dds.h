/* @(#)79       1.4  src/rspc/kernext/pcmcia/ent/ient_dds.h, pcmciaent, rspc41J, 9515A_all 4/10/95 03:00:33 */
/* @(#)61       1.3.1.1  src/rspc/kernext/isa/ient/ient_dds.h, isaent, rspc41J 1/6/95 15:46:34 */
/*
 * COMPONENT_NAME: PCMCIAENT - IBM PCMCIA Ethernet Device Driver
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_IENT_DDS
#define _H_IENT_DDS

/*****************************************************************************/
/*                  Work section of device control structure                 */
/*****************************************************************************/

typedef struct
{
    Simple_lock   tx_slock;           /* transmit lock                      */
    Simple_lock   slih_slock;         /* SLIH lock                          */
    Simple_lock   cmd_slock;          /* Adapter command lock               */
    int           adap_state;         /* Current adapter state.             */
    int           alloc_size;         /* Size of the dds.                   */
    int           ndd_stime;          /* Start time of the ndd statistics   */
    int           dev_stime;          /* Start time of the dev statistics   */
    uchar         net_addr[ENT_NADR_LENGTH];   /* actual network address    */
    uchar         burn_addr[ENT_NADR_LENGTH];  /* Burned-in network address */

    uchar        adapter_type;        /* Type of adapter.                   */

    /* __________________________________________________________________   */
    /*                        Xmit structures                               */

    xmt_elem_t    *tx_waitq;          /* Wait q for xmit's.                 */
    xmt_elem_t    *cur_tx_elem;       /* Ongoing xmission.                  */
    uint          xmits_queued;       /* Nbr in queue system.               */
    uint          nbr_in_wait_q;      /* Nbr in wait list.                  */
    xmt_elem_t    *tx_next_free;      /* First free tx-element.             */
    xmt_elem_t    *tx_last_free;      /* Last free tx-element.              */
    xmt_elem_t    *tx_next_wait;      /* First waiting tx-element.          */
    xmt_elem_t    *tx_last_wait;      /* Last waiting tx-element.           */
    uint          tx_buf_in_use;      /* How many buffers in use.           */
    int           tx_stat;            /* Last tx status.                    */
    uint          xmit_wdt_active;    /* Is the timer currently active.     */
    uint          xmit_wdt_opackets;  /* Tx. count when tx-timer started.   */
    uint          do_send;            /* Do a xmission when rx done ?       */

    uint          new_packet;         /* Nbr of last packet buffer.         */
    uint          receiving;          /* Are we on the receive thread.      */
    int           ctl_status;         /* ioctl command status.              */
    int           ctl_pending;        /* ioctl command outstanding flag.    */
    int           ctl_event;          /* sleep event for ent_ioctl.         */
    int           tx_wdt_inited;      /* flag for tx watchdog. registration */
    int           ctl_wdt_inited;     /* flag for ctl watchdog.             */
    uint          cmd_wdt_active;     /* Is the timer currently active.     */

    /* CDLI feature variables (promiscuous mode, badframe, multicast).      */
    int           promiscuous_count;  /* promiscuous mode reference count.  */
    int           badframe_count;     /* save bad packet reference count.   */
    int           otherstatus;        /* all other async status ref count.  */
    int           enable_multi;       /* enable multicast reference count.  */
    int           multi_count;        /* total multicast addr. registered.  */
    ient_multi_t  *multi_table;       /* first multicast addr. table.       */

    /* Specific adapter information. */
    uchar         slot_width;         /* Are we in a 16 bit slot. */
    err_stats     errors;             /* Tally counters.          */
} ient_wrk_t;

/* Device Driver structure. */
typedef struct dds
{
    struct intr        ihs;           /* Interrupt handler ctl struct.    */
    struct dds         *next;         /* Point to the next device.        */
    int                seq_number;    /* Sequence number                  */
    ndd_t              ndd;           /* Ndd for NS ndd chain.            */
    ddi_t              ddi;           /* Cfg. passed from the cfg-method. */
    ibm_isa_vpd_t      vpd;           /* Revision level's and such        */
    struct watchdog    xmit_wdt;      /* watchdog timer for transmit      */
    struct dds         *xmit_wdt_dds; /* Pointer to appropriate dds.      */
    struct watchdog    cmd_wdt;       /* watchdog timer for ioctl         */
    struct dds         *cmd_wdt_dds;  /* Pointer to appropriate dds.      */
    struct trb         *systimer;     /* system timer for error recovery  */
    ent_genstats_t     entstats;      /* ethernet generic statistics      */
    ien_isa_stats_t    devstats;      /* adapter specific statistics      */
    ethernet_all_mib_t mibs;          /* ethernet MIB's                   */
    ient_wrk_t         wrk;           /* device work area                 */
#ifdef PCMCIAENT
    int  clientID;                    /* Client ID                        */
    int  event_word;                  /* For a e_sleep/e_wokeup event     */
    int  configured;                  /* Configured ? or not              */
    int  windowID;                    /* For Request Window               */
    int  return_callback;             /* Return value from Callback       */
    int  no_card;                     /* Any card into slot, or not       */
    int  config_base;                 /* Configuration base address       */
    int  config_index;                /* Default Index                    */
    int  reg_comp;                    /* flag for registration completion */
    int  cb_flag;                     /* -1:unconfig, 1:callback, 0:none  */
    int  net_status;                  /* 1: NDD_UP    0: net down         */
#endif
#ifdef PM_SUPPORT
    struct        pm_handle pm;       /* pm handle structure                   */
#endif /* PM_SUPPORT */
} dds_t;

/***************************************************************************/
/*
 * This is the global device driver control structure
 */
/***************************************************************************/

typedef struct
{
    lock_t         cfg_lock;       /* For ient_config lockl use only.  */
    Simple_lock    dd_slock;       /* Initialization lock.             */
    dds_t          *p_dds_list;    /* One for each adapter.            */
    int            num_devs;       /* Number devices configured.       */
    int            initialized;    /* Has the driver been initialized. */
    int            num_opens;      /* Nbr. of devices opened.          */
    tracetable_t   tracetable;     /* Trace table for debugging.       */
    ient_cdt_t     cdt;            /* Dump table.                      */
    struct io_map  *io_map_ptr;    /* For doing in/out-bytes.          */
    struct io_map  *mem_map_ptr;   /* For doing shared mem. accesses.  */
#ifdef PCMCIAENT
    struct io_map  *attr_map_ptr;  /* For doing attribute mem. access. */
#endif
} dd_ctrl_t;

/*
 * Macros for accessing device control area. The pointer to this area has to 
 * be named p_dds for using these macros.
 */

#define IHS        p_dds->ihs
#define NDD        p_dds->ndd
#define DDI        p_dds->ddi
#define VPD        p_dds->vpd
#define TXWDT      p_dds->xmit_wdt
#define CMDWDT     p_dds->cmd_wdt
#define ENTSTATS   p_dds->entstats
#define DEVSTATS   p_dds->devstats
#define CFG_LOCK   dd_ctrl.cfg_lock
#define DD_LOCK    dd_ctrl.dd_slock
#define TRACE_LOCK dd_ctrl.tracetable.trace_slock
#define MIB        p_dds->mibs
#define WRK        p_dds->wrk
#define TX_LOCK    p_dds->wrk.tx_slock
#define SLIH_LOCK  p_dds->wrk.slih_slock
#define CMD_LOCK   p_dds->wrk.cmd_slock

/*****************************************************************************/
/*                  Error logging type definition                            */
/*****************************************************************************/

#define IEN_ISA_FNAME_LEN    32

struct error_log_def
{
    struct err_rec0   errhead;        /* from com/inc/sys/err_rec.h      */
    uchar fname[IEN_ISA_FNAME_LEN];   /* filename and line number        */
    uchar ent_addr[ENT_NADR_LENGTH];  /* actual net address in use       */
    ulong parm1;                      /* log data 1                */
    ulong parm2;                      /* log data 2                */
    ulong parm3;                      /* log data 3                */
};

#endif /* _H_IENT_DDS */
