/* @(#)60        1.10  src/rspc/kernext/idedisk/idediskdd.h, idedisk, rspc41J, 9519A_all 5/8/95 23:20:00 */
#ifndef _H_IDEDISKDD
#define _H_IDEDISKDD
/*
 * COMPONENT_NAME: (IDEDISK) IDE Disk Device Driver Include File
 *
 * FUNCTIONS:  NONE
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

/************************************************************************/
/* idediskdd.h header dependencies                                       */
/************************************************************************/

#include <sys/types.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/watchdog.h>
#include <sys/iostat.h>
#include <sys/dump.h>
#include <sys/ide.h>
#include <sys/err_rec.h>
#include <sys/pm.h>

/************************************************************************/
/* Device driver internal trace table information.                      */
/************************************************************************/
/*
 * Debug Trace length
 */
#ifdef DEBUG

#define TRCLNGTH		1000		/* debug trace table len */
#define IDEDISK_TRC_STR_LEN	8

/*
 * Debug Trace Table Structure
 */
struct idedisk_trace {
        char			desc[IDEDISK_TRC_STR_LEN];
						/* ASCII entry descrip      */
        char			type[3];	/* ASCII entry or exit descrip*/
        char			count;		/* which of several entries */
        uint			word1;		/* desc dependent meaning   */
        uint			word2;		/* desc dependent meaning   */
        uint			word3;		/* desc dependent meaning   */
        uint			word4;		/* desc dependent meaning   */
        uint			word5;		/* desc dependent meaning   */
};
#endif

/************************************************************************/
/* Misc. Defines 							*/
/************************************************************************/

#define DK_BLOCKSIZE		512		/* Disk blocksize	      */
#define DK_MAX_RETRY		5		/* max error retry count(+1)  */
#define DK_MAX_VERIFY_RETRY	3		/* max verify retry count(+1) */
#define DK_MAXCONFIG		8		/* Max # of disks that	      */
						/*  may be configured at once */
#define DK_CMD_Q		0x1             /* Use disk's pending Queue   */
#define DK_QUEUE		0x1		/* Place cmd_ptr on end of Q  */
#define DK_STACK		0x2             /* Place cmd_ptr on front of Q*/

#define HKWD_DD_IDEDISKDD	0x2A100000	/* IDE Disk Device Driver */

/*
 * IDE Disk IOCTL-specific constants
 */
#define	SEL_DMA_PIO		0x04		/* select DMA/PIO mode	 */
#define	IDE_DISK_DMA		0x00deafaa	/* select DMA mode	 */
#define	IDE_DISK_PIO		0x00deaf55	/* select PIO mode	 */

/* Queue types */
#define DK_PENDING		0x01
#define DK_INPROG		0x02

/* IDE Disk Error types */
#define DK_MEDIA_ERR		0x01
#define DK_HARDWARE_ERR		0x02
#define DK_ADAPTER_ERR		0x03
#define DK_RECOVERED_ERR	0x04
#define DK_UNKNOWN_ERR		0x05
#define DK_SOFTWARE_ERR		0x06

/* IDE Disk Error severities */
#define DK_HARD_ERROR		0x01
#define DK_SOFT_ERROR		0x02

/* IDE Disk Error severities */
#define DK_OVERLAY_ERROR	0x01
#define DK_NOOVERLAY_ERROR	0x02

/************************************************************************/
/* Initialization information on individual IDE disks			*/
/************************************************************************/
struct idedisk_dds {
	char			resource_name[8];/* resource name logged with */
						/*  each error log entry.     */
	dev_t			adapter_devno;	/* adapter device driver      */
						/*  major/minor num	      */
	uchar			device_id;	/* IDE unique device ID	      */
	uchar			chs_only;	/* TRUE => device requires CHS*/
	uchar			sectors_per_track; /* # sectors/track on disk */
	uchar			num_heads;	/* # of heads on disk	      */
	ushort			rw_timeout;	/* Time-out period for reads  */
						/*  and writes.		      */
#define DK_TIMEOUT		31		/* Default r/w time-out (secs)*/
	ushort			start_timeout;	/* spin-up time-out (secs)    */
#define DK_START_TIMEOUT	31		/* Default start-up time-out  */
	uint			segment_size;	/* # bytes in sample rate     */
	uint			segment_cnt;	/* # of segments read from    */
						/* this device 		      */
	uint			byte_count;	/* # bytes read beyond the    */
						/* number of segments	      */
	uint			max_coalesce;	/* Max number of bytes to be  */
						/* coalesced.		      */
	uint			max_request;	/* maximum request size for   */
						/* an IDE disk		      */
        uint			block_size;	/* The device's block size    */
	uint			total_sectors;	/* The # of available sectors */
        int			pm_dev_itime;	/* PM device idle time        */
        int			pm_dev_stime;	/* PM device standby time     */
        int			pm_dev_att;	/* PM device attribute        */
	uchar			read_cmd;	/* DMA/PIO ATA Read command   */
	uchar			write_cmd;	/* DMA/PIO ATA Write command  */
	uchar			dma_supported;	/* TRUE if device supports DMA*/
};

/************************************************************************/
/* Macro definitions							*/
/************************************************************************/

#define IDEDISK_IN_PROG_ENQUEUE(diskinfo, bp)				\
{									\
	if (diskinfo->dk_bp_queue.in_progress.head == NULL) {		\
		diskinfo->dk_bp_queue.in_progress.head = bp;		\
		diskinfo->dk_bp_queue.in_progress.tail = bp;		\
		bp->av_forw = NULL;					\
	} else {							\
		diskinfo->dk_bp_queue.in_progress.tail->av_forw = bp;	\
		bp->av_forw = NULL;					\
		diskinfo->dk_bp_queue.in_progress.tail = bp;		\
	}								\
}

#define IDEDISK_IN_PROG_DEQUEUE(diskinfo, bp)				\
{									\
       if (diskinfo->dk_bp_queue.in_progress.head != NULL) {		\
	       diskinfo->dk_bp_queue.in_progress.head = bp->av_forw;	\
	       if (diskinfo->dk_bp_queue.in_progress.head == NULL) {	\
		       diskinfo->dk_bp_queue.in_progress.tail = NULL;   \
		       }						\
       }                                                                \
}

/*
 * If diskinfo->block_size is not DK_BLOCKSIZE then set rc = -1;
 */
#define IDEDISK_GET_LBA(blkno, diskinfo, bp, rc)			\
{                                                                       \
       if (diskinfo->block_size == DK_BLOCKSIZE) {                      \
		/*							\
		 * No conversion is required here since			\
		 * the buf structs b_blkno is in terms			\
		 * of 512 byte blocks (DK_BLOCKSIZE).			\
		 */							\
		blkno = bp->b_blkno;					\
		rc = 0;							\
       } else {								\
	       /*							\
		* If the device's block size is not			\
		* 512, fail this request.		 		\
		*/							\
               rc = -1;                                     		\
       }								\
}

#define	COPY_TO_ARG(src_ptr, struct_size)				\
	if (devflag & DKERNEL)						\
		bcopy((caddr_t) src_ptr, (caddr_t) arg, struct_size);	\
	else								\
		copyout((caddr_t) src_ptr, (caddr_t) arg, struct_size);


#define CNVRT_IF_CHS(atacmd,blkno)					\
	atacmd->flags = ATA_LBA_MODE;					\
	atacmd->startblk.lba = blkno;					\
	if (diskinfo->chs_only) {					\
		atacmd->flags = ATA_CHS_MODE;				\
		atacmd->startblk.chs.sector =				\
			(blkno % diskinfo->sectors_per_track)+1;	\
		abs_track = (blkno / diskinfo->sectors_per_track);	\
		atacmd->startblk.chs.head = (uchar)			\
			(abs_track % diskinfo->num_heads);		\
		cylinder = (ushort) (abs_track / diskinfo->num_heads);	\
		atacmd->startblk.chs.cyl_hi = (uchar) (cylinder >> 8);	\
		atacmd->startblk.chs.cyl_lo =				\
					(uchar) (cylinder & 0x00ff);

#define END_CNVRT_NO_DEBUG						\
	}

#define END_CNVRT_DEBUG(atacmd)						\
		printf("\tC/H/S = %02x%02x / %02x / %02x\n",		\
			atacmd->startblk.chs.cyl_hi,			\
			atacmd->startblk.chs.cyl_lo,			\
			atacmd->startblk.chs.head,			\
			atacmd->startblk.chs.sector);			\
	}
	

#ifdef DEBUG
#define CONVERT_IF_CHS(atacmd,lba)	CNVRT_IF_CHS(atacmd,lba)	\
					END_CNVRT_DEBUG(atacmd)
#else
#define CONVERT_IF_CHS(atacmd,lba)	CNVRT_IF_CHS(atacmd,lba)	\
					END_CNVRT_NO_DEBUG
#endif


/************************************************************************/
/* Watchdog Timer Structure						*/
/************************************************************************/
struct idedisk_timer {
	struct watchdog		watch;		/* system watchdog timer*/
	void			*pointer;	/* diskinfo pointer     */
};

/************************************************************************/
/* Power Management Data Structures					*/
/************************************************************************/
struct idedisk_pm_handle {
	struct pm_handle	handle;
	struct idediskinfo	*diskinfo;
};

/************************************************************************/
/* Device capacity data block 						*/
/************************************************************************/
struct idedisk_capacity {
	uint			lba;		/* last logical block address*/
	uint			len;		/* block length in bytes */
};

/************************************************************************/
/* Device statistics and thresholds structure                           */
/************************************************************************/
struct idedisk_stats {
	uint			segment_size;	/* # bytes in sample rate  */
	uint			segment_count;	/* # segments processed    */
	uint			byte_count;	/* # bytes current segment */
};

/************************************************************************/
/* Disk driver operation descriptor block                               */
/************************************************************************/
struct dk_cmd {
	struct ataide_buf	idebuf;		/* buf for adapter use  */
						/*  this field must be	*/
						/*  first in struct	*/
	struct buf		*bp;		/* buf list for cmd     */
	struct idediskinfo	*diskinfo;
	struct dk_cmd		*next;        
	struct dk_cmd		*prev;
	uint			soft_resid;	/* non-0 on failures	*/
	uint			segment_count;
	ushort			retry_count;

	uchar			type;		/* cmd type             */
#define DK_BUF		0x01			/* used by strategy     */
#define DK_PM		0x08			/* used by power mgmnt  */
#define DK_VERIFY	0x10			/* verifies prev write	*/
#define DK_WRITEV	0x20			/* WRITE preceding VERIFY*/

	uchar			subtype;	/* cmd subtype  	*/
#define DK_IDLE		0x02			/* IDLE IMMEDIATE request */
#define DK_STANDBY	0x03			/* STANDBY IMMEDIATE req*/
#define DK_SLEEP	0x04			/* SLEEP request	*/

	uchar			group_type;
#define DK_SINGLE	0x01			/* one in this request	*/
#define DK_COALESCED	0x02			/* Several in one request */

	uchar			flags;
#define DK_READY	0x01

	uchar			status;        /* State of command block */
#define DK_FREE		0x00
#define DK_IN_USE	0x01
#define DK_ACTIVE       0x02
#define DK_QUEUED	0x04
#define DK_RETRY	0x08	

	uchar			queued;       /* TRUE if queued         */
	uchar			aborted;       /* TRUE if command is    */
					       /* being aborted		*/
	uchar		        error_type;    /* IDE Disk Error type	*/
};

/************************************************************************/
/* Disk bp queue control structs                                        */
/************************************************************************/

struct dk_queue_control {
	struct buf		*head;
	struct buf		*tail;
};

struct dk_bp_queue {
	struct dk_queue_control	pending;
	struct dk_queue_control	in_progress;
};

/************************************************************************/
/* Information for entire driver                                        */
/************************************************************************/
struct idedd_info {
	int			config_cnt;	/* Configuration count     */
	int			open_cnt;	/* Number devices open	   */
	lock_t			lock;		/* Lock Word               */
struct	cdt			*cdt;		/* Component dump table ptr*/
};

/************************************************************************/
/* Information on individual disks                                      */
/************************************************************************/
struct idediskinfo {
	struct idediskinfo	*next;	        /* Ptr next disk's info */
	struct idediskinfo	*next_open;     /* Ptr to next open disk*/
	dev_t			devno;          /* Device major+minor   */
						/* numbers		*/
        /*
         * NOTE: the preceding fields must remain in
         * this order to stay consistent with hardware's
	 * thresholding disablement scheme.  In other words they
	 * currently traverse idedisk_open_list of diskinfo structures
	 * via the next pointer looking for a devno match.
         */

	dev_t			adapter_devno;  /* Adapter maj+min	*/
	uchar			device_id;      /* id for device	*/
	uchar			chs_only;	/* TRUE => disk requires CHS */
	uchar			sectors_per_track; /* # sectors/track on disk*/
	uchar			num_heads;	/* # of heads on disk	*/
	struct dk_cmd           *dk_cmd_q_head; /* head of dk_cmd queue */
	struct dk_cmd           *dk_cmd_q_tail; /* tail of dk_cmd queue */
	struct dk_cmd           pm_cmd;		/* reserved PM cmd	*/
	struct dk_cmd		verify_cmd;	/* rsrvd READ/VERIFY cmd*/
	struct dk_cmd		*writev_cmd;	/* ptr to write command	*/
						/*  to be verified	*/
	struct dk_cmd	        *cmd_pool;      /* Pool of buf cmds	*/
	int                     pool_index;     /* index into above pool*/
	struct dk_cmd		dmp_cmd;	/* reserved command for */
						/* dump error recovery  */
	struct dk_bp_queue	dk_bp_queue;    /* bp queue control	*/

	uchar			state;		/* Current state of device */
#define DK_OFFLINE		0x00		/* Device offline	*/
#define DK_ONLINE		0x01		/* Device init'ed and online */
#define DK_FAILED		0x02		/* Device uninit'ed & offline */
#define DK_DUMP_PENDING     	0x10		/* Device has dump pending */

	uchar			errno;		/* Saves b_error for PM cmds */
	uchar			opened;         /* device has been opened */
	uchar			cmd_pending;    /* cmd pending in strategy */
	uchar			dump_inited;    /* TRUE if dump started */
	uchar			starting_close; /* TRUE if a close is being */
						/* attempted for this disk. */
	ushort 			rw_timeout;	/* Time-out period for reads */
						/* reads and writes.	*/
	ushort 			start_timeout;	/* Start Unit time-out	*/
						/* (secs).		*/
	ushort		        cmds_out;       /* Number of command issued */
						/* for this device	*/
	struct buf	 	*currbuf;	/* current buf of elevator */
	struct buf 		*low;		/* low buf of elevator that */
						/* can be queued up to a*/
						/* device		*/
        uint			block_size;     /* The device's block size */
        uint			cfg_block_size; /* The block size specified */
                                        	/* by the config method.*/
	uint			max_request;    /* Maximum request to IDE dd */
	uint			max_coalesce;	/* Maximum size to coalesce */
	lock_t			lock;           /* Lock Word		*/
	struct file		*fp;            /* filepointer for dev calls */
	struct idedisk_error_rec error_rec;	/* Error record for device    */
	struct idedisk_stats	stats;		/* I/O & Soft error statistics*/
	struct idedisk_capacity capacity;	/* Disk capacity information */
	struct dkstat		dkstat;		/* Disk iostat information */
	struct idedisk_pm_handle pmh;		/* power management pm_handle */
	uchar			pm_pending;     /* PM pending operation */
#define PM_IDEDISK_PENDING_OP		0x01
#define PM_IDEDISK_PENDING_SUSPEND	0x02
	uint			pm_event;	/* PM event word	*/
	struct idedisk_timer	pm_timer;
	uchar			read_cmd;	/* DMA/PIO ATA Read command */
	uchar			write_cmd;	/* DMA/PIO ATA Write command */
	uchar			dma_supported;	/* TRUE if device supports DMA*/
	uchar			verify_flags;
#define VERIFY_SUPPORTED		0x80
						/* MSb set if the device is  */
						/* ATA-2 compliant (supports */
						/* Read-Verify cmd).  The    */
						/* lower nibble tracks the # */
						/* of consecutive cmd aborts */
						/* received on issuing Read- */
						/* Verify.  MAX_VERIFY_RETRY */
						/* failures will cause the   */
						/* MSB to be cleared to      */
						/* indicate the device does  */
						/* not support Read-Verify   */
};

#ifndef _NO_PROTO 
/************************************************************************/
/* idedisk functions                                                    */
/************************************************************************/

int	idedisk_config(dev_t devno, int op, struct uio *uiop);
int	idedisk_open(dev_t devno, int rwflag, int chan, int ext);
int	idedisk_close(dev_t devno, int chan, int ext);
int	idedisk_read(dev_t devno, struct uio *uiop, int chan, int ext);
int	idedisk_write(dev_t devno, struct uio *uiop, int chan, int ext);
int	idedisk_rdwr(dev_t devno,struct uio *uiop,int chan,int ext,int rw_flag);
int	idedisk_mincnt(struct buf *bp, void *minparms);
int	idedisk_ioctl(dev_t devno, int op, int arg, int flag, int chan,int ext);
int	idedisk_dkiocmd(int arg, int devflag, struct idediskinfo *diskinfo);
int     idedisk_alloc(struct idediskinfo *diskinfo);
void    idedisk_init_cmds(struct idediskinfo *diskinfo);
void    idedisk_release_allow(struct idediskinfo *diskinfo);

int	idedisk_strategy(struct buf *bp);
int	idedisk_dump(dev_t devno, struct uio *uiop, int cmd, int arg, int chan,
		     int ext);
int	idedisk_dump_write(struct idediskinfo *diskinfo,
				struct dk_cmd *write_cmd_ptr,
				int chan, int ext);
void	idedisk_process_dmp_error(struct idediskinfo *diskinfo,
				struct dk_cmd **dmp_cmd_ptr,
				struct dk_cmd *write_cmd_ptr);
void	idedisk_pending_enqueue(struct idediskinfo *diskinfo, 
				struct buf *bp);
void	idedisk_pending_dequeue(struct idediskinfo *diskinfo);
void	idedisk_start_disable(struct idediskinfo *diskinfo);
void	idedisk_start(struct idediskinfo *diskinfo);
void	idedisk_iodone(struct dk_cmd *cmd_ptr);
void	idedisk_process_good(struct dk_cmd *cmd_ptr);
void	idedisk_process_error(struct dk_cmd *cmd_ptr);
void	idedisk_process_buf_error(struct dk_cmd *cmd_ptr);
void	idedisk_process_verify_error(struct dk_cmd *cmd_ptr);
void	idedisk_process_partial(struct dk_cmd *cmd_ptr);
void	idedisk_process_dev_error(struct dk_cmd *cmd_ptr);
void	idedisk_process_buf(struct dk_cmd *cmd_ptr);
void	idedisk_coalesce(struct idediskinfo *diskinfo);
struct dk_cmd	*idedisk_build_cmd(struct idediskinfo *diskinfo, 
				   struct buf **head, char good_path);
void	idedisk_build_error(struct dk_cmd *cmd_ptr, int priority);
void	idedisk_log_error(struct dk_cmd *cmd_ptr, int sev);
void	idedisk_pm_watchdog(struct watchdog *watchdog_timer);
struct cdt 	*idedisk_cdt_func(int arg);
void	idedisk_q_cmd(struct  dk_cmd *cmd_ptr,char queue,uchar  which_q);
void	idedisk_d_q_cmd(struct dk_cmd *cmd_ptr,uchar which_q);
struct dk_cmd	*idedisk_cmd_alloc_disable(struct idediskinfo *diskinfo,
					   uchar cmd_type);
struct dk_cmd	*idedisk_cmd_alloc(struct idediskinfo *diskinfo,
				   uchar cmd_type);
void	idedisk_free_cmd_disable(struct dk_cmd *cmd_ptr);
void	idedisk_free_cmd(struct dk_cmd *cmd_ptr);
void	idedisk_sleep(uchar *intrpt, uint *event);
void	idedisk_trc_disable(char *desc,char *type,char count,uint word1,
			    uint word2,uint word3,uint word4,uint word5);
void	idedisk_trc(char *desc,char *type,char count,uint word1,uint word2,
		    uint word3,uint word4,uint word5);
void	idedisk_spinupdn(struct idediskinfo *diskinfo, uchar req_cmd);
int	idedisk_spindown_closeddisk(struct idediskinfo *diskinfo);
int	idedisk_pm_handler(caddr_t private, int mode);

#else

/************************************************************************/
/* idedisk functions                                                    */
/************************************************************************/

int	idedisk_config();
int	idedisk_open();
int	idedisk_close();
int	idedisk_read();
int	idedisk_write();
int	idedisk_rdwr();
int	idedisk_mincnt();
int	idedisk_ioctl();
int     idedisk_rdwrse();
int	idedisk_dkiocmd();
int     idedisk_alloc();
void    idedisk_init_cmds();

int	idedisk_strategy();
int	idedisk_dump();
int     idedisk_dump_write();
void    idedisk_process_dmp_error();
void	idedisk_pending_enqueue();
void	idedisk_pending_dequeue();
void	idedisk_start_disable();
void	idedisk_start();
void	idedisk_iodone();
void	idedisk_process_good();
void	idedisk_process_error();
void	idedisk_process_buf_error();
void	idedisk_process_verify_error();
void	idedisk_process_partial();
void	idedisk_process_dev_error();
void	idedisk_process_buf();
void	idedisk_coalesce();
struct dk_cmd	*idedisk_build_cmd();
void	idedisk_build_error();
void	idedisk_log_error();
void	idedisk_pm_watchdog();
struct cdt	*idedisk_cdt_func();
void	idedisk_q_cmd();
void	idedisk_d_q_cmd();
struct dk_cmd	*idedisk_cmd_alloc_disable();
struct dk_cmd	*idedisk_cmd_alloc();
void	idedisk_free_cmd_disable();
void	idedisk_free_cmd();
void	idedisk_sleep();
void	idedisk_trc_disable();
void	idedisk_trc();
void	idedisk_spinupdn();
int	idedisk_spindown_closeddisk();
int	idedisk_pm_handler();
#endif /* _NO_PROTO */
#endif /* _H_IDEDISKDD */
