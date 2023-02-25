/* @(#)01        1.11  src/rspc/kernext/isa/ide/ataidedd.h, isaide, rspc41J, 9518A_all 5/2/95 17:31:53 */
#ifndef _H_ATAIDEDD
#define _H_ATAIDEDD
/*
 * COMPONENT_NAME: (ISAIDE) IBM ATA/IDE Adapter Driver Header File
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************/
/* IDE Adapter DDI structure */
/*****************************/
#include "ataidecfg.h"

/************************************************************************/
/* General Device Driver Defines                                        */
/************************************************************************/

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif

#define HKWD_DD_IDEDD	0x2A000000	 /* IDE Adapter Driver */

#define MAXREQUEST      0x20000          /* largest data xfer size:      */
                                         /* =  131072 dec. (128KB)       */
                                         /* =  512 * 256                 */
#define MAXSECTOR	4096		 /* Maximum sector size (for PIO)*/
#define MAX_ADAPTERS    2                /* Allow a max of 2 IDE adapters*/
                                         /* There could actually be 4,   */
                                         /* but there's only 2 here.     */
#define MAX_DEVICES     2                /* This allows a max of 2 IDE   */
                                         /* devices on each adapter.     */
#define LONGWAIT        5                /* timeout value for abrt/bdrs  */
#define RESETWAIT       32               /* timeout value for bus resets */

                    /* the ioctl ATA/IDE commands      */
#define LIDE_NO_ERR      0x00            /* routine call succeeded       */
#define LIDE_FAILED      0x01            /* routine call failed          */
#define LIDE_COPY_ERROR  0x02            /* error returned during xmem   */
#define LIDE_DMA_ERROR   0x03            /* error returned during cleanup*/
#define LIDE_RESET_CHIP  0x01            /* flag used to cleanup register*/
#define LIDE_RESET_DMA   0x02            /* flag used to cleanup DMA     */

#define SLEEP_EVENT     (1 << 31)        /* for lide_sleep wakeup event  */

/************************************************************************/
/* Adapter I/O Defines                                                  */
/************************************************************************/
/* ATA Task File/Port register addressing definitions */
#define LIATA0_BASE     0x01F0  /* base address of Primary ATA/IDE port */
#define LIATA1_BASE     0x0170  /* base address of Secondary ATA/IDE    */
#define LIATA2_BASE     0x01E8  /* base address of Tertiary ATA/IDE port*/
#define LIATA3_BASE     0x0168  /* base address of Quaternary ATA/IDE   */
#define LIDATA_SIZE     0x02    /* Width (in bytes) of ATA/IDE data reg.*/
#define LIDATA          0x0000  /* offset of Data reg                   */
#define LIERR_SIZE      0x01    /* width of ATA/IDE Error Register      */
#define LIERR           0x0001  /* offset of ATA/IDE Error Reg (Read)   */
#define LIFEAT_SIZE     0x01    /* offset of Feature reg from port base */
#define LIFEAT          0x0001  /* offset of Feature reg (Write only)   */
#define LISECC_SIZE     0x01    /* width of Sector Count Register       */
#define LISECC          0x0002  /* offset of Sector Count Reg           */
#define LISECN_SIZE     0x01    /* width of Sector Number Register      */
#define LISECN          0x0003  /* offset of Sector Number Register     */
#define LICYLL_SIZE     0x01    /* width of Cylinder Low Register       */
#define LICYLL          0x0004  /* offset of Cylinder Low Register      */
#define LICYLH_SIZE     0x01    /* width of Cylinder High Register      */
#define LICYLH          0x0005  /* offset of Cylinder High Register     */
#define LIDRVH_SIZE     0x01    /* width of Drive/Head Register         */
#define LIDRVH          0x0006  /* offset of Drive/Head Register        */
#define LISTA_SIZE      0x01    /* width of Status Register             */
#define LISTA           0x0007  /* offset of Status Register (Read only)*/
#define LICMD_SIZE      0x01    /* width of Command Register            */
#define LICMD           0x0007  /* offset of Command Register(Write)    */
#define LIASTA_SIZE     0x01    /* width of Alternate Status Reg        */
#define LIASTA          0x0206  /* offset of Alt. Status Reg (Read only)*/
#define LIDCTL_SIZE     0x01    /* width of Device Control Reg          */
#define LIDCTL          0x0206  /* offset of Dev. Ctrl. Reg (Write only)*/
#define LIDADR_SIZE     0x01    /* width of Device Addr. Reg.           */
#define LIDADR          0x0207  /* offset of Dev. Addr. Register        */

/* Miscellaneous defines */
#define IPL_MAX_SECS        31
/* Defines used in ataideb.c */
#define NO_DMA_REQD    0
#define READ_DMA_REQD  1
#define WRITE_DMA_REQD 2

/************************************************************************/
/* Miscellaneous Structures                                             */
/************************************************************************/
struct timer    {
    struct watchdog dog;                /* the watchdog struct          */
    struct adapter_def *ap;             /* adapter structure address    */
#define LIDE_RESET_TMR   1
#define LIDE_COMMAND_TMR 2
#define LIDE_RESTART_TMR 3
    uint        timer_id;               /* my internal timer id val     */
                                        /*  1 = adapter reset timer     */
                                        /*  2 = adapter cmd timer       */
                                        /*  3 = dev_info cmd timer      */
};

struct error_log_def {                  /* driver error log structure   */
    struct err_rec0    errhead;         /* error log header info        */
    struct ataide_rc   data;            /* driver dependent err data    */
};

struct lide_cdt_table {                 /* component dump table struct  */
    struct    cdt_head   lide_cdt_head; /* header to the dump table     */
    struct    cdt_entry  lide_entry[MAX_ADAPTERS]; /* one per adapter   */
};

typedef struct lintr {
    struct intr         its;   /* int handler structure. MUST be first */
    struct adapter_def *ap;    /* associated adapter structure */
} lintr_t;

/************************************************************************/
/* Structures related to device control                                 */
/************************************************************************/
struct dev_info {
    struct timer    dev_watchdog;   /* watchdog timer for dev struct*/
    struct adapter_def *ap;         /* pointer to adapter_def */
    uchar           opened;
    uchar           dev_id;         /* IDE Device ID of this device */
    uchar           restart_in_prog;/* a restart of this device     */
    uchar           resvd0;
    struct ataide_buf *cmd_pend;

#define DEVICE_NOT_ACTIVE         0x00
#define EXPECT_ATAPI_CMD          0x01
#define EXPECT_DATAIN             0x02
#define EXPECT_DATAOUT            0x04
#define EXPECT_STATUS             0x08
#define COMMAND_TIMED_OUT         0x10
    uchar            cmd_activity_state;
                                 /* what the device structure is doing*/
#define ACTIVE        0
#define STOPPING      1
#define HALTED        2
    uchar            queue_state;  /* device general queue state   */
                                   /* ACTIVE, STOPPING, or HALTED    */
                                   /* this only represents the     */
                                   /* state of the cmd_active queue*/
#define RETRY_ERROR         0x0001 /* flag used for retry of abort/bdr */
#define CAUSED_TIMEOUT      0x0002 /* this device caused a timeout */
    ushort          flags;         /* field used to hold the setting of*/
                                   /* flags for the drive.             */
#define LIDE_RETRY_COUNT              10
    int             retry_count;    /* times allowed to retry issue */
                                    /* of an abort/bdr command.     */

    uint            dma_addr;       /* save area for dma address    */
    uint            bytes_moved;/* num of bytes moved in last transfer */
    uint            sector_len; /* num of bytes in each sector */
    uchar	    chs_only;	/* TRUE => disk requires CHS */
    uchar	    sectors_per_track; /* # sectors/track on disk*/
    uchar	    num_heads;	/* # of heads on disk */
    uchar           resvd1;
/* io characteristics for PIO and DMA */
#define IDE_PIO_MODE_0 0
#define IDE_PIO_MODE_0_NS 600
#define IDE_PIO_MODE_1 1
#define IDE_PIO_MODE_1_NS 383
#define IDE_PIO_MODE_2 2
#define IDE_PIO_MODE_2_NS 240
#define IDE_PIO_MODE_3 3
#define IDE_PIO_MODE_3_NS 180
    int pio_cycle_time;
    int pio_max_mode;
    int pio_cur_mode;
    int iordy_supported;
    int iordy_enabled;
    int dma_supported;
    int dma_cycle_time;
#define IDE_DMA_SINGLW_MODE 0x0010
#define IDE_DMA_MULTIW_MODE 0x0020
    int dma_max_mode;
    int dma_cur_mode;
    uchar protocol_type;          /* see ide.h; struct ide_identify;        */
                                  /* ata_atapi element for valid values     */
    uchar atapi_cmd_drq; /*two bits-00b = transfer ATAPI Packet on DRQ only */
                         /*                  max 3 ms delay before DRQ      */
                         /*        01b = transfer ATAPI Packet on DRQ or INT*/
                         /*                max 10ms before DRQ or INTERRUPT */
                         /*        10b = transfer ATAPI Packet on DRQ only  */
                         /*                max 50 microsec delay before DRQ */
#define IDE_CMD_DRQ_3MILS  0x0000
#define IDE_CMD_DRQ_INTR   0x0001
#define IDE_CMD_DRQ_50MICS 0x0002
    uchar atapi_cmd_size; /* 00b = 12 bytes, 01b = 16 bytes, 1Xb = reserved */
#define IDE_ATAPI_CMD12 0x0000
#define IDE_ATAPI_CMD16 0x0001
    uchar device_type;     /* five bits indicating device type */
                           /* 0x00 = hard disk; 0x01=sequential device */
                           /* 0x05 = CD ROM; 0x07 = optical disk */
};


#define LIDE_TRACE_SIZE  0x1000
struct lide_trace_entry {
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

struct lide_trace_struct {
    struct lide_trace_entry  trace_buffer[LIDE_TRACE_SIZE];
};

struct adapter_def {
    int             inited;           /* False is not initialized      */
    lock_t          lock;             /* adapter lock */
    struct ide_adap_ddi ddi;
    struct timer    reset_watchdog;   /* watchdog timer for bus_resets */
    struct timer    restart_watchdog; /* watchdog timer for cmd delays */
    ulong           base_addr0;       /* addr of ATA Task File         */
    uchar           opened;
    uchar           power_state;      /* adapter power mode flag       */
#define LIDE_NORMAL_PWR 0             /*  0 = normal state             */
#define LIDE_PM_SYS_PWR 2             /*  2 = system suspend/hibernate */
#define EPOW_PENDING    4             /*  4 = EPOW pending             */
    uchar           errlog_enable;    /* set if errors are to be logged*/
    char           *xfer_area;        /* pointer to transfer area      */
    int             active_device;    /* Currently active Device       */
    int		    device_cnt_done;  /* Flag indicating that the      */
				      /* device count routine has been */
				      /* completed for this device     */
    int		    device_cnt;       /* Count of attached devices     */
#define NO_DEV_ACTIVE   MAX_DEVICES


    struct dev_info    *device_queue_hash[MAX_DEVICES];
                                /* pointers to the device               */
                                /* queue structures                     */
                                /* access the array via direct index:   */
                                /*    Master = device 0                 */
                                /*    Slave  = device 1                 */


    struct lintr        intr_struct;             /* int handler struct   */
    struct lintr        epow_struct;             /* EPOW interrupt struct*/
    struct pm_handle	pmh;			 /* Power Mgr Handlr str.*/
    dev_t               devno;                   /* adapter major/minor  */
    uchar               dump_inited;             /* dump init completed  */
    uchar               dump_started;            /* dump start completed */
#define LIDE_DMP_STARTING	0x01             /* PM or Sys dump start */
#define LIDE_SYS_DMP            0x02             /* System dump request  */
#define LIDE_PM_DMP             0x04             /* Power mgmt. dump     */
    uchar               dma_pending;             /* dma running flag     */
#define LIDE_DMA_RUNNING  0x01
#define LIDE_DMA_NORES    0x02
    int                 max_request;             /* max xfer allowed     */
    int                 channel_id;              /* dma channel id       */
    int                 dump_pri;                /* saved dump int prior.*/
    int                 bus_num;                 /* our bus number       */
    struct d_handle    *handle;                  /* DMA access handle(dma.h) */
    struct dio          dio;                     /* DMA I/O vector struct*/
    struct io_map       iom;                     /* io map structure(ioacc.h)*/
    struct xmem         xmem;                    /* cross mem attach(xmem.h) */
    struct buf         *ubuf_ptr;                /* pointer to current userbuf*/
    uint                ubuf_ix;
    int                 light_supported;         /* disk active light support */
#ifdef LIDE_TRACE
    struct lide_trace_struct  *trace_ptr;
    int                 current_trace_line;
#endif
};

struct regs {
    uint isa_reg;
    int  data;
    char size;
};

/* DEBUGGING AIDS: */
#ifdef DEBUG
#include <stdio.h>
#define DEBUG_0(A)                      {printf(A);}
#define DEBUG_1(A,B)                    {printf(A,B);}
#define DEBUG_2(A,B,C)                  {printf(A,B,C);}
#define DEBUG_3(A,B,C,D)                {printf(A,B,C,D);}
#define DEBUG_4(A,B,C,D,E)              {printf(A,B,C,D,E);}
#define DEBUG_5(A,B,C,D,E,F)            {printf(A,B,C,D,E,F);}
#define DEBUG_6(A,B,C,D,E,F,G)          {printf(A,B,C,D,E,F,G);}
#define DEBUGELSE                       else
#else
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#define DEBUGELSE
#endif
#ifdef LIDE_TRACE
#define TRACE_1(A,B)                      {lide_trace_1(A,B);}
#define TRACE_2(A,B,C)                    {lide_trace_2(A,B,C);}
#define TRACE_3(A,B,C,D)                  {lide_trace_3(A,B,C,D);}
#else
#define TRACE_1(A,B)
#define TRACE_2(A,B,C)
#define TRACE_3(A,B,C,D)
#endif

/*****************************************************************************/
/*     function macros                                                       */
/*****************************************************************************/
#define LIDE_READ_ATA(A,B,C)  lide_read_reg(&A->iom, (A->base_addr0+ B ), C)
#define LIDE_WRITE_ATA(A,B,C,D) lide_write_reg(&A->iom,(A->base_addr0+ B ),C,D)
#define LIDE_READ_PCI(A,B)    lide_read_reg(A, B, 1)
#define LIDE_WRITE_PCI(A,B,C) lide_write_reg(A, B, 1, C)

#ifndef    _NO_PROTO
/*****************************************************************************/
/*     functions in ataidet.c                                                */
/*****************************************************************************/

int     lide_config(dev_t devno, int op, struct uio *uiop);
int     lide_open(dev_t devno, ulong devflag, int chan, int ext);
void	lide_config_devices(struct adapter_def *ap, dev_t devno);
int     lide_close(dev_t devno, int offchan);
void    lide_clear_open(struct adapter_def *ap, int undo_level);
void    lide_fail_open(struct adapter_def *ap, int undo_level, int ret_code,
                       dev_t devno);
int     lide_inquiry(struct adapter_def *ap, dev_t devno, int arg,
                        ulong devflag);
int     lide_start_unit(struct adapter_def *ap, dev_t devno, int arg,
                        ulong devflag);
int     lide_test_unit_rdy(struct adapter_def *ap, dev_t devno, int arg,
                           ulong devflag);
void    lide_adp_str_init();
int     lide_identify_device(struct adapter_def *ap, dev_t devno, int arg,
                             ulong devflag);
int     lide_read_ata(struct adapter_def *ap, dev_t devno, int arg,
                      ulong devflag);
int     lide_ioctl(dev_t devno, int cmd, int arg, ulong devflag, int chan,
                  int ext);
struct ataide_buf * lide_build_command();
int     lide_diagnostic(struct adapter_def *ap, int arg, ulong devflag);
static int lide_xlate_mode(int mode);
int     lide_start_dev(struct adapter_def *ap, dev_t devno, int dev_index);
int     lide_stop_dev(struct adapter_def *ap, int dev_index);
int     lide_issue_abort(struct adapter_def *ap, int dev_index);
int     lide_issue_reset(struct adapter_def *ap, int dev_index);

/*****************************************************************************/
/*     functions in ataideb.c                                                */
/*****************************************************************************/

void    lide_chip_register_reset(struct adapter_def *ap);
int     lide_count_devices(struct adapter_def *ap);
int	lide_raw_cmd(struct adapter_def *ap, int cmd_reg, int drvh_reg,
	   int feat_reg, int secc_reg, int secn_reg, int cyll_reg,
	   int cylh_reg, int timeout);
int	lide_init_device(struct adapter_def *ap, int dev_index);
int     lide_reconfig_adapter(struct adapter_def *ap);
void    lide_logerr(struct adapter_def *ap, int errid, int add_halfword_status,
           int errnum, int data1, struct dev_info *dev_ptr, uchar read_regs);
int     lide_read_reg(struct io_map *iom, uint offset, char reg_size);
int     lide_write_reg(struct io_map *iom, uint offset, char reg_size,
                       int data);
int     lide_strategy(struct ataide_buf *bp);
void    lide_start(struct adapter_def *ap, struct dev_info *dev_ptr);
int     lide_issue_cmd(struct adapter_def *ap, struct dev_info *dev_ptr,
                       struct ataide_buf *bp, int dma_reqd);
int     lide_iodone(struct ataide_buf *bp);
int     lide_dump(dev_t devno, struct uio *uiop, int cmd, int arg,
                 int chan, int ext);
int     lide_dump_intr(struct adapter_def *ap, struct dev_info *dev_ptr);
int     lide_dumpstrt(struct adapter_def *ap, int arg, int ext);
int     lide_dumpwrt(struct adapter_def *ap, struct ataide_buf *bp, int ext);
int     lide_dump_dev(struct adapter_def *ap, struct dev_info *dev_ptr);
struct  cdt *lide_cdt_func(int arg);
void    lide_delay(struct adapter_def *ap, int delay_sec, int delay_nsec);
void    lide_sleep(struct adapter_def *ap, int delay_sec, int delay_nsec);
void    lide_sleep_handler(struct trb *trb_ptr);
void    lide_xfer_thread(int flags, void *init_parms, int parms_length);
void    lide_xfer_func(struct adapter_def *ap);
int     lide_intr(struct lintr *handler);
int     lide_epow(struct lintr *handler);
int	lide_pm_handler(caddr_t private, int requested_mode);
void	lide_turn_chip_on(struct adapter_def *ap, int change_pmh_mode);
void    lide_command_watchdog(struct watchdog *w);
void    lide_command_reset_ide_bus();
void    lide_trace_1(char *string, int data);
void    lide_trace_2(char *string, int val1, int val2);
void    lide_trace_3(char *string, int data1, int data2, int data3);
#else
/*****************************************************************************/
/*     functions in ataidet.c                                                */
/*****************************************************************************/

int     lide_config();
int     lide_open();
void	lide_config_devices();
int     lide_close();
void    lide_clear_open();
void    lide_fail_open();
int     lide_inquiry();
int     lide_start_unit();
int     lide_test_unit_rdy();
void    lide_adp_str_init();
int     lide_identify_device();
int     lide_read_ata();
int     lide_ioctl();
struct  ataide_buf * lide_build_command();
int     lide_diagnostic();
static int lide_xlate_mode();
int     lide_start_dev();
int     lide_stop_dev();
int     lide_issue_abort();
int     lide_issue_reset();
/*****************************************************************************/
/*     functions in ataideb.c                                                */
/*****************************************************************************/

void    lide_chip_register_reset();
int     lide_count_devices();
int	lide_raw_cmd();
int	lide_init_device();
int     lide_reconfig_adapter();
void    lide_logerr();
int     lide_read_reg();
int     lide_write_reg();
int     lide_strategy();
void    lide_start();
int     lide_issue_cmd();
int     lide_iodone();
int     lide_dump();
int     lide_dump_intr();
int     lide_dumpstrt();
int     lide_dumpwrt ();
int     lide_dump_dev();
struct  cdt *lide_cdt_func();
void    lide_delay();
void    lide_sleep();
void    lide_sleep_handler();
void	lide_xfer_thread();
void    lide_xfer_func();
int     lide_intr();
int     lide_epow();
int	lide_pm_handler();
void	lide_turn_chip_on();
void    lide_command_watchdog();
void    lide_command_reset_ide_bus();
void    lide_trace_1();
void    lide_trace_2();
void    lide_trace_3();
#endif /* not _NO_PROTO */

#endif /* _H_ATAIDEDD */

