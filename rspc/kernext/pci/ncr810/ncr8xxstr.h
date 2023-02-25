/* @(#)49	1.6  src/rspc/kernext/pci/ncr810/ncr8xxstr.h, pciscsi, rspc41J, 9516A_all 4/18/95 11:20:26 */

/*
 *   COMPONENT_NAME: PCISCSI
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

#ifndef _H_NCR8XXSTR
#define _H_NCR8XXSTR

/************************************************************************/
/* Miscellaneous Structures						*/
/************************************************************************/
typedef struct timer	{
    struct watchdog dog;		/* the watchdog struct		*/
    struct adapter_def *ap;		/* adapter structure address	*/
    uchar  save_time;
    uchar  timer_id;    		/* my internal timer id val	*/
					/*  1 = adapter cmd timer	*/
					/*  2 = scsi bus reset timer	*/
					/*  3 = dev_info cmd timer	*/
#define PSC_SIOP_TMR	1
#define PSC_RESET_TMR	2
#define PSC_COMMAND_TMR 3
#define PSC_RESTART_TMR 4
#ifdef	DEBUG_8XX
    int		timer_starts;		/* Count of timers currently
					   started */
#endif
} wdogt_t;

struct error_log_def {			/* driver error log structure	*/
    struct err_rec0    errhead;		/* error log header info	*/
    struct rc	 data;			/* driver dependent err data	*/
};

struct p8xx_cdt_table {			 /* component dump table struct	 */
    struct    cdt_head	 p8xx_cdt_head;	 /* header to the dump table	 */
    struct    cdt_entry	 p8xx_entry[MAX_ADAPTERS]; /* space for each
					              minor + trace */
};


/*
 * intr struct wrapper
 */
typedef struct pintr {
    struct intr	       its;	/* int handler struct. MUST be first */
    struct adapter_def *ap;	/* associated adapter structure */
} pintr_t;



/************************************************************************/
/* Structures related to device control					*/
/************************************************************************/

typedef struct scripts_struct	{	/* SCRIPTS used to run the SIOP */
    ulong   *vscript_ptr;		/* Virtaddr of SCRIPTS workarea	*/
    ulong    bscript_ptr;		/* Bus addr of SCRIPTS workarea	*/
} scripts_t;

typedef struct map_struct     {         /* maps pages used in managing  */
    ulong   *system_ptr;		/* the data pointers            */
    ulong   *dma_ptr;
};

typedef struct saved_pointers	{
    uint    tim_bus_addr;		/* bus address of next tim entry */
    uint    tim_cnt;		        /* remaining number of entries   */
    uint    tables_differ;		/* usage tbd			 */
    uint    tim_restore;		/* flag used for restore msg     */
} saved_t;
    

/*
 * This structure represents the 53C8XX's I/O command structure as used
 * with the Table Indirect Mode I/O instructions.
 * Note: The driver does not currently use table indirect mode I/O
 * instructions but since this information had to be saved it seemed
 * prudent to use the chip's format for possible future use.
 * Also, the flags byte should be zeros if we are really going to use
 * this for table indirect mode :-(
 */
typedef	struct io_struct {
	uchar	flags;
#define	EXTRA_CLOCK_SETUP	0x02	/* need extra clock of setup time */
#define	AVOID_WDTR       	0x04	/* target can't handle wdtr msg   */
	uchar	sxfer;			/* SXFER register value */
	uchar	id;			/* SCSI id */
	uchar	scntl3;			/* SCNTL3 register value */
} io_t;

/* Similar to above, but what we save across device start/stop's */
typedef	struct	io_save {
	uchar	flags;
#define	NEGOT_DONE		0x01	/* completed negotiations w/ tgt */
	uchar	sxfer;
	uchar	scntl3;
} io_save_t;


/*
 * This structure describes the 53C8XX Table Indirect move entry format.
 * Note that length and address must be stored Little Endian.
 */
typedef struct tim_table {
	ulong	tim_len;		/* length */
	ulong	tim_adr;		/* bus address */
} tim_t;


typedef struct scsi_id_info {
    uchar            scsi_id;          	/* the target's scsi id         */
    uchar            negotiate_flag;   	/* if SCSI negotiation needed   */
    uchar            async_device;     	/* if this is defined as async  */
    uchar            restart_in_prog;  	/* after BDRs and SCSI resets   */
    uchar            disconnect_flag;  	/* indicates disconnect priv.   */
                                       	/* in patched identify message  */
    uchar            tag_flag;         	/* currently patched tag        */
    uchar            tag_msg_flag;     	/* currently patched tag msg    */
    uchar            lun_flag;         	/* currently patched lun        */
    ulong            bscript_ptr;      	/* Bus addr of the script       */
    ulong            *vscript_ptr;     	/* Virtual addr of the script   */
    io_t	     io_table;	       	/* Negotiation information      */
    struct dev_info *bdring_lun;       	/* ptr to lun associated w/ bdr */
} sid_t;

typedef struct cmd_info {
    uchar        tag;                   /* Index of this struct in the array */
    uchar        preempt_counter;       /* to prevent command starvation */

    uchar        resource_state;        /* Type of resources used */
#define LARGE_TCE_RESOURCES_USED      1
#define SMALL_TCE_RESOURCES_USED      2
#define STA_RESOURCES_USED            3
#define NO_RESOURCES_USED             4

    uchar        flags;
#define PREP_MAIN_COMPLETE  0x01        /* flag used for prep main check */
#define NEGOT_CMD           0x08        /* negotiated before sending cmd */

    uchar        in_use;                /* used to help validate tag */

    struct cmd_info    *active_fwd;     /* next command in queue */
                                        /* (either waitq or activeq) */
    struct cmd_info    *active_bkwd;    /* previous command in queue */
                                        /* (activeq only) */
    struct cmd_info    *next_dev_cmd;   /* next command for a device */
                                        /* (used in the waitq)       */
    struct dev_info   *device;          /* target device of the command */
    struct sc_buf     *bp;              /* struct sc_buf involved */

    uint         tim_bus_addr;          /* base (bus) address of TIM tbl */
    uint         resource_index;        /* save area for TIM index */
    uint         tim_tbl_used;          /* total number of TIM entries   */
				        /* used by this command          */
    uint         TCE_count;             /* number of dio iovecs reserved */
} cmd_t;

/*
 * Per SCSI lun structure
 */

typedef struct dev_info {
    struct timer    dev_watchdog;	/* watchdog timer for dev struct*/
    struct adapter_def *ap;		/* adapter structure ptr */
    uchar	    opened;
    uchar	    scsi_id;		/* SCSI ID of this device */
    uchar	    lun_id;		/* LUN ID of this device */
    uchar	    ioctl_wakeup;	/* wakeup sleeping ioctl call */
    int		    ioctl_event;
    int		    ioctl_errno;
    int		    stop_event;

    struct dev_info *DEVICE_ACTIVE_fwd;
    struct dev_info *DEVICE_ACTIVE_bkwd;
    struct dev_info *DEVICE_WAITING_fwd;
    struct dev_info *ABORT_BDR_fwd;
    struct dev_info *ABORT_BDR_bkwd;

    struct cmd_info *active_head;
    struct cmd_info *active_tail;
    struct cmd_info *wait_head;
    struct cmd_info *wait_tail;
    struct cmd_info *cmd_save_head;    /* ptr to frozen cmd queue */
    struct cmd_info *cmd_save_tail;    /* ptr to frozen cmd queue */

    struct sc_buf *bp_save_head;       /* ptr to frozen sc_buf queue  */
    struct sc_buf *bp_save_tail;       /* ptr to frozen sc_buf queue  */


#define NOTHING_IN_PROGRESS       0x01
#define CMD_IN_PROGRESS           0x02
#define ABORT_IN_PROGRESS         0x04
#define BDR_IN_PROGRESS           0x08
#define SDTR_IN_PROGRESS          0x10
#define WDTR_IN_PROGRESS          0x20
#define NEGOTIATE_IN_PROGRESS     0x30
    uchar		cmd_activity_state;

#define ACTIVE          0x01
#define WAIT_FLUSH      0x02
#define HALTED          0x04
#define HALTED_CAC      0x08
#define WAIT_INFO       0x10
#define STOPPING        0x20
#define STOPPING_MASK   0x38            /* stopping during a CAC */
#define WAIT_INFO_or_HALTED_CAC        0x18
#define STOPPING_or_HALTED_or_FLUSHING 0x26
#define ACTIVE_or_WAIT_FLUSH           0x03
#define STOPPING_or_HALTED_CAC         0x28
    uchar	     queue_state;	/* device general queue state */
					/* ACTIVE, STOPPING, or HALTED */
					/* this only represents the */
					/* state of the cmd_active queue*/
#define RETRY_ERROR         0x0001 /* flag used for retry of abort/bdr */
#define CAUSED_TIMEOUT      0x0002 /* this device caused a timeout */
#define SCSI_ABORT          0x0004 /* a scsi abort is active */
#define SCSI_BDR            0x0008 /* a scsi bdr is active for device */
#define SELECT_TIMEOUT      0x0010 /* a selection timeout occurred */
#define BLOCKED             0x0040 /* indicates failed allocation */
#define STARVATION          0x0080 /* device is undergoing cmd starvation */
#define SCSI_BDR_or_ABORT   0x000C /* a scsi bdr or abort is active */
#define FLUSH_or_STARVE_or_BLOCKED 0x00CC
    ushort	flags;			/* field used to hold the setting of*/
					/* flags for the drive. */
#define PSC_RETRY_COUNT		30      
    int		retry_count;		/* times allowed to retry issue */
					/* of an abort/bdr command. */

    sid_t       *sid_ptr;		/* Ptr to target id information */
} dvi_t;


#define P8XX_TRACE_SIZE  0x800
struct p8_trace_entry {
    union {
        struct {
            char            header1[12];
            int             data;
        } one_val;
        struct {
            char            header2[8];
            int             data1;
            int             data2;
        } two_vals;
        struct {
            char            header3[4];
            int             val1;
            int             val2;
            int             val3;
        } three_vals;
        char                header[16];
    }un;
};

struct p8_trace_struct {
    struct p8_trace_entry  trace_buffer[P8XX_TRACE_SIZE];
};


/*
 * Structure for the adapter (NCR 53C8XX chip, actually)
 */

typedef	struct adapter_def {
    struct pintr    intr_struct;	/* Normal interrupt structure */
    struct pintr    epow_intr_struct;	/* EPOW interrupt structure */
    struct pm_handle pmh;		/* Power mgmt. handler structure */
    p8xx_ddi_t	    ddi;
    struct timer    reset_watchdog;	/* watchdog timer for bus_resets */
    struct timer    restart_watchdog;	/* watchdog timer for cmd delays */
    ulong	    base_addr0;		/* addr of chip's registers */

    uchar 	    light_supported;    /* operator panel activity light */
    uchar	    str_inited;
    uchar	    opened;
    uchar	    reset_pending;	/* A SCSI bus reset is pending */
    uchar	    scsi_clk;		/* period of our SCLK (in ns) */
    uchar	    slow_sync;		/* Max 5mb/sec sync transfer speed */
    uchar	    xfer_pd;		/* Default sync transfer period
					   (in 4ns units) */
    uchar	    req_ack;		/* Default sync offset */
    uchar	    power_state;	/* adapter power mode flag */
#define P8XX_NORMAL_PWR 0		   /*  0 = normal state    */
#define P8XX_PM_SYS_PWR 2		   /*  system suspend/hibernate */
#define EPOW_PENDING	4		   /*  EPOW pending        */

    uchar	    errlog_enable;	    /* set if errors are to */
    uchar           open_mode;              /* mode opened in:      */
#define NORMAL_MODE     0                   /*  normal operation    */
#define DIAG_MODE       1                   /*  diagnostic mode     */
    uchar	    iowait_inited;	    /* allow 1 iowait init  */

    io_save_t	device_state[MAX_SIDS]; /* state saved across 
					   device start/stop */

    struct dev_info    *device_queue_hash[MAX_DEVICES];
				/* pointers to the device		*/
				/* queue structures			*/
				/* access the array via scsi id/lun:	*/
				/*	 msb  7 6 5 4 3 2 1 0 lsb	*/
				/*	 ----+-+-+-+-+-+-+-+-+---	*/
				/*	      x x i i i l l l		*/
				/*		  d d d u u u		*/
				/*		  2 1 0 n n n		*/
				/*			2 1 0		*/
				/*					*/
    sid_t   sid_info[MAX_SIDS];

#define NUM_TAG		256
    struct cmd_info	   command[NUM_TAG];  /* command elements */

#define TAG_TABLE_SIZE   (((NUM_TAG-1) / PSC_WORDSIZE) + 1)
#define TAG_UNUSED	0xFFFFFFFF
    uint TAG_alloc[TAG_TABLE_SIZE];

    struct sc_buf	*REQUEST_WFR_head;
    struct sc_buf	*REQUEST_WFR_tail;
    struct sc_buf	*blocked_bp;

    struct dev_info	   *DEVICE_ACTIVE_head;
    struct dev_info	   *DEVICE_ACTIVE_tail;

    struct cmd_info	   *DEVICE_WAITING_head;
    struct cmd_info	   *DEVICE_WAITING_tail;

    struct dev_info	   *ABORT_BDR_head;
    struct dev_info	   *ABORT_BDR_tail;

    struct map_struct     IND_TABLE;    /* Pointers to the saved_t page */
    struct scripts_struct Scripts;	/* This is the ptrs to the IO_WAIT
					   scripts.  It serves as the
					   scheduler and all the device
					   scripts refer to/depend upon it. */
    struct map_struct   loc_tim_tbl;    /* Pointers to the current tim vectors */
    struct map_struct   sav_tim_tbl;    /* Pointers to the saved tim vectors */

    dev_t		devno;			/* adapter major/minor */
    uchar		dump_started;		/* dump start status */
#define	P8XX_DMP_STARTING	0x01
#define	P8XX_SYS_DMP		0x02            /* system dump start complete */
#define	P8XX_PM_DMP		0x04            /* pm dump start complete */
    dvi_t		*dump_dev_ptr;		/* hibernation dev ptr */
    int			max_request;		/* max xfer allowed */
    int			bus_bid;		/* BID for our bus */
    struct d_handle    *handle;
    struct io_map	iom;			/* io map structure */
    struct dio blist;
    ulong * blist_base_vector;			/* base dio iovec address */
    struct dio vlist;

#define IOVEC_UNUSED	0xFFFFFFFF
#define NUM_IOVEC       (PAGESIZE/8)
#define IOVEC_TABLE_SIZE (((NUM_IOVEC-1)/ PSC_WORDSIZE) + 1)
    uint		iovec_alloc[IOVEC_TABLE_SIZE];
#ifdef P8_TRACE
    struct p8_trace_struct *trace_ptr;
    int    current_trace_line;
#endif

#ifdef DEBUG_8XX
    uint		strategy_cnt;		/* count of strategy call */
    uint		intr_cnt;		/* count of interrupts taken */
    uint		intr_proc_cnt;		/* count of interrupts proc */
    uint		pio_w_cnt;		/* count of write pios */
    uint		pio_r_cnt;		/* count of read pios */
    uint		chip_strt_cnt;		/* count of chip starts */
    uint		chip_strt_wd_cnt;	/*  with TIM addr */
    uint		no_int_cnt;		/*  without interrupt */
    uint		wait_req_cnt;		/*  with interrupt */
    uint		sigp_set_cnt;		/* times we set SIGP */
    uint		sigp_intr_cnt;		/* count of sigp interrupts */
    uint		phase_mismatch_cnt;	/* count of phase mismatchs */
#endif
} ads_t;


/*
 * Devno hash entry for locating adapter structure
 */

#define ADS_HASH_MASK	0x07
typedef	struct dev_hash {
	struct dev_hash	*next;		/* must be first field! */
	dev_t		devno;		/* major/minor #s */
	ads_t		*ap;		/* corresponding adapter struct */
} hash_t; 


#endif	/* _NCR8XXSTR */
