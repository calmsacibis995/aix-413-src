/* @(#)60        1.11  src/rspc/kernext/idecdrom/idecdromdd.h, cdide, rspc41J, 9521A_all 5/23/95 20:39:36 */
#ifndef _H_IDECDROMDD
#define _H_IDECDROMDD
/*
 * COMPONENT_NAME: (IDECD) IDE CD-ROM Device Driver Include File
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
/* idecdromdd.h header dependencies                                     */
/************************************************************************/

#include <sys/types.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/scsi.h>
#include <sys/watchdog.h>
#include <sys/iostat.h>
#include <sys/dump.h>
#include <sys/scdisk.h>
#include <sys/pm.h>
#include <sys/ide.h>
#include <sys/err_rec.h>

#ifdef DEBUG
#define DKprintf(args) if (idecdrom_debug) printf args
#endif


/************************************************************************/
/* Device driver internal trace table information.                      */
/************************************************************************/
/*
 * Debug Trace length
 */
#ifdef DEBUG
#define TRCLNGTH                1000    /* Length of debug trace table       */
#endif

/*
 * Debug Trace Table Structure
 */
#ifdef DEBUG
struct idecdrom_trace {
#define IDECDROM_TRC_STR_LENGTH       8
        char    desc[IDECDROM_TRC_STR_LENGTH];/* ASCII descrip of this entry */
        char    type[3];                    /* ASCII desrip of entry or exit */
        char    count;                      /* which of several entries      */
        uint    word1;                      /*  meaning depends on the desc  */
        uint    word2;                      /*  meaning depends on the desc  */
        uint    word3;                      /*  meaning depends on the desc  */
        uint    word4;                      /*  meaning depends on the desc  */
        uint    word5;                      /*  meaning depends on the desc  */
};
#endif


/************************************************************************/
/* IDE CD-ROM Sense Key Values 			                	*/
/************************************************************************/
#define DK_NO_SENSE		0x00
#define DK_RECOVERED_ERROR	0x01
#define DK_NOT_READY		0x02
#define DK_MEDIUM_ERROR		0x03
#define DK_HARDWARE_ERROR	0x04
#define DK_ILLEGAL_REQUEST	0x05
#define DK_UNIT_ATTENTION	0x06
#define DK_DATA_PROTECT		0x07
#define DK_ABORTED_COMMAND	0x0B
#define DK_MISCOMPARE		0x0E

/************************************************************************/
/* Block Sizes.								*/
/************************************************************************/
#define DK_BLOCKSIZE	512		/* Disk blocksize 		*/
#define DK_M2F1_BLKSIZE 2048            /* The block size for CD-ROM    */
/* #define DK_M2F1_BLKSIZE 2056  */          /* The block size for CD-ROM    */
                                        /* XA data mode 2 form 1.       */
#define DK_M2F2_BLKSIZE 2336            /* The block size for CD-ROM    */
                                        /* XA data mode 2 form 2.       */
#define DK_CDDA_BLKSIZE 2352            /* The block size for CD-DA     */


/************************************************************************/
/* Multi-Session CD-ROM Defines						*/
/************************************************************************/


#define DK_CD_PVD       	0x10    /* The LBA where the Physical   */
                                        /* Volume Descriptor (PVD) is   */
					/* located on CDs with using    */   
           				/* the ISO 9660 standard        */

/************************************************************************/
/* Misc. Defines 							*/
/************************************************************************/

#define HKWD_DD_IDECDROMDD 0x2A200000	/* IDE CD-ROM Device Driver */

#define DK_MAX_RETRY    5               /* maximum error retry count(+1)*/
#define DK_MAX_RESETS	3		/* maximum number of reset      */
					/* cycles on an error           */
#define DK_MAX_RESET_FAILURES 2		/* maximum number of reset cycle*/
					/* failures before we give up   */

#define DK_FMT_DEF_SIZE	4		/* size of format defect list.  */
					/* This size must be >= 4       */
#define DK_MAXCONFIG	0x08		/* Maximum number of disks that	*/
					/* may be configured at once	*/

#define DK_HASH		0x0F
#define DK_HASHSIZE	0x10
#define DK_REQSNS_LEN	0xFF
#define DK_ERROR	-1
#define DK_CMD_Q        0x1             /* Use the disks pending Queue  */
#define DK_QUEUE	0x1		/* Place cmd_ptr on end of Q    */
#define DK_STACK	0x2             /* Place cmd_ptr on front of Q  */
#define DK_PAGESIZE     PAGESIZE        /* Page size in bytes		*/

/* Queue types */
#define DK_PENDING	0x01
#define DK_INPROG	0x02

/* IDE CD-ROM Error types */
#define DK_MEDIA_ERR		0x01
#define DK_HARDWARE_ERR		0x02
#define DK_ADAPTER_ERR		0x03
#define DK_RECOVERED_ERR	0x04
#define DK_UNKNOWN_ERR		0x05

/* IDE CD-ROM Error severities */
#define DK_HARD_ERROR		0x01
#define DK_SOFT_ERROR		0x02

/* IDE CD-ROM Error severities */
#define DK_OVERLAY_ERROR	0x01
#define DK_NOOVERLAY_ERROR	0x02

/* IDE CD-ROM Mode Sense/Select defines */
#define DK_MAX_MODE_PAGES	0x3f	/* Maximum number of mode page  */
#define CD_MODE_HEADER_LEN	8	/* Size of Mode Sense/Select header */
#define DK_COMPLIES_PF		0x10	/* Indicates mode pages comply  */
					/* to Page Format               */

#define DK_MAX_SIZE_PG_E	0xe	/* The largest possible size of	*/
					/* Mode Page E from Parameter 	*/
					/* length.			*/
#define DK_VOL_START_PG_E	0x7	/* Starting offset of volume	*/
					/* from paramter length.	*/
					/* control info in Mode Page E	*/
#define DK_PAUSE		0x1	/* IDE Pause command		*/
#define DK_RESUME		0x2	/* IDE Resume command		*/


#define	DK_LG_IOCTL_LEN		65535   /* Size of large buffer of ioctl*/
#define DK_TOC_LEN		12	/* Size of buffer for read TOC  */
					/* data.			*/
#define DK_TOC_MSF		0x2     /* MSF bit setting of Read TOC  */
					/* IDE command.			*/

#define DK_MSENSE_E_LEN		24      /* Size of buffer for Mode Sense*/
					/* of page E (Volume data)	*/

#define DK_CD_VOL_MASK		(CD_VOLUME_ALL | \
				CD_VOLUME_CHNLS | \
				CD_ALL_AUDIO_MUTE) /* Mask to determine */
					     /* volume type.		*/
#define DK_SUBQ_FMT		0x01	/* Sub-channel data format code	*/
					/* for read sub-Q IDE command.	*/
#define DK_SUBQ			0x1	/* Sub-Q bit setting for read	*/
					/* sub-Q channel.		*/
#define DK_SUBQ_LEN		16	/* Length of Read Sub-Q channel	*/
					/* IDE command. 		*/
#define DK_SUBQ_MSF		0x2     /* MSF bit setting of Read Sub-Q*/
					/* channel data.		*/
#define DK_AUDIO_STATUS_INVALID	0x00	/* Audio status byte not 	*/
					/* supported or not valid.	*/
#define DK_AUDIO_PLAY_IN_PROGRESS 0x11  /* Audio play operation in 	*/
					/* progress.			*/
#define DK_AUDIO_PLAY_PAUSED	0x12	/* Audio play operation paused.	*/
#define DK_PLAY_COMPLETED	0x13	/* Audio play operation 	*/
					/* successfully completed.	*/
#define DK_PLAY_STOP_ERROR	0x14	/* Audio play operation stopped	*/
					/* due to error.		*/
#define DK_NO_AUDIO_STATUS 	0x15	/* No current audio status to	*/
					/* return.			*/
/************************************************************************/
/* Initialization information on individual disks                       */
/************************************************************************/
struct idecdrom_dds  {
	char	resource_name[8];	/* resource name logged with  	*/
					/* each error log entry.        */
	dev_t	adapter_devno;		/* adapter device driver        */
					/* major/minor num 		*/
	uchar	device_id;		/* IDE unique device ID         */
	uchar   prevent_eject;  	/* Prevent target's media from  */
					/* being ejected while open.    */
        uchar   play_audio;             /* TRUE if optional             */
                                        /* play audio command set is    */
                                        /* supported by the device.     */
        uchar   cd_mode2_form1_flag;    /* TRUE if data mode 2 form 1   */
                                        /* is supported by device.      */
                                        /* (Mandatory in ATAPI spec)    */
        uchar   cd_mode2_form2_flag;    /* TRUE if data mode 2 form 2   */
                                        /* is supported by device.      */
                                        /* (Mandatory in ATAPI spec)    */
        uchar   cd_da_flag;             /* TRUE if data mode CD-DA      */
                                        /* is supported by device.      */
                                        /* (Optional in ATAPI spec)     */
        uchar   multi_session;          /* TRUE if the CD-ROM drive     */
                                        /* supports multi session discs */

	ushort 	rw_timeout;		/* Time-out period for reads and*/
					/* writes.			*/
#define DK_TIMEOUT	30		/* Default time-out period(secs)*/
	ushort 	start_timeout;		/* Start Unit time-out period   */
					/* (secs).			*/
#define DK_START_TIMEOUT	60      /* 60 second time-out period    */

	uint	segment_size;		/* # bytes in sample rate       */
	uint	segment_cnt;		/* # of segments read from this */
					/* device 			*/
	uint	byte_count;		/* # bytes read beyond the      */
					/* number of segments		*/
	uint	max_coalesce;		/* Max number of bytes to be 	*/
					/* coalesced.			*/
	uint    max_request;		/* maximum request size for     */
					/* an IDE CD-ROM		*/
        uint    block_size;             /* The device's current block   */
                                        /* size.                        */
	int	mode_data_length;	/* mode select data length      */
	char	mode_data[256];		/* mode select data buffer      */
	int	mode_default_length;	/* mode select default data lng */
	char	mode_default_data[256];	/* mode select default data     */
        int     pm_dev_itime;           /* PM device idle time          */
        int     pm_dev_stime;           /* PM device standby time       */
        int     pm_dev_att;             /* PM device attribute          */
};

/************************************************************************/
/* Macro definitions							*/
/************************************************************************/

#define IDECDROM_IN_PROG_ENQUEUE(diskinfo, bp)				\
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

#define IDECDROM_IN_PROG_DEQUEUE(diskinfo, bp)				\
{									\
       if (diskinfo->dk_bp_queue.in_progress.head != NULL) {		\
	       diskinfo->dk_bp_queue.in_progress.head = bp->av_forw;	\
	       if (diskinfo->dk_bp_queue.in_progress.head == NULL) {	\
		       diskinfo->dk_bp_queue.in_progress.tail = NULL;   \
		       }						\
       }                                                                \
}

		
/*
 * If the block size does not equal DK_BLOCKSIZE then convert the
 * bp->blkno to the correct block number for the device's block size.
 * If diskinfo->block_size is not a multiple of DK_BLOCKSIZE then
 * set rc = -1;  If this is a multi-session CD then re-map all
 * DK_CD_PVD lba requests.
 */
#define IDECDROM_GET_LBA(blkno,diskinfo,bp,rc)                          \
{                                                                       \
                                                                        \
       rc = 0;                                                          \
       if (diskinfo->block_size == DK_BLOCKSIZE) {                      \
	       /*							\
		* No conversion is requird here since			\
		* the buf structs b_blkno is in terms			\
		* of 512 byte blocks (DK_BLOCKSIZE).			\
		*/							\
               blkno = bp->b_blkno;                                     \
       }                                                                \
       else if (diskinfo->block_size % DK_BLOCKSIZE) {                  \
	       /*							\
		* If the device's block size is not a multiple		\
		* of 512 fail this request. In this case 		\
		* diskinfo->mult_of_blksize = 0.			\
		*/							\
									\
               rc = -1;                                     		\
       }                                                                \
       else {                                                           \
               blkno = bp->b_blkno/diskinfo->mult_of_blksize;		\
               if ((bp->b_blkno % diskinfo->mult_of_blksize) ||         \
                   ((bp->b_blkno + (bp->b_bcount / DK_BLOCKSIZE))       \
                    % diskinfo->mult_of_blksize)) {                     \
		      /*						\
		       * Request does not start on a device 		\
		       * block boundary or request does not end on a	\
		       * device block boundary. Special handling needed.\
		       */						\
                      rc = 1;                                           \
               }                                                        \
               else {                                                   \
		      /*						\
		       * Request starts on a device 			\
		       * block boundary. So convert it from		\
		       * 512 bytes/per block block number to 		\
		       * the correct block number for the device.	\
		       */						\
									\
									\
		      if ((blkno == DK_CD_PVD) &&			\
                         (diskinfo->last_ses_pvd_lba) &&		\
			  (diskinfo->block_size == DK_M2F1_BLKSIZE)) {  \
				/*					\
				 * If this is mult-session photo CD	\
				 * and the request is to lba 16		\
				 * (pvd) and we are running in M2F1	\
				 * (the only valid mode for photo CD),	\
				 * then remap it to the			\
				 * last session's pvd.			\
				 */					\
                                blkno = diskinfo->last_ses_pvd_lba;     \
                      }                                                 \
               }                                                        \
       }                                                                \
                                              				\
}


/************************************************************************/
/* Bufs used for raw I/O                                                */
/************************************************************************/
struct dk_raw_buf {
       struct buf              bp;      /* Raw buf to be processed.     */
       struct idecdrom_diskinfo *diskinfo;/* Raw Buf's diskinfo structure.*/
};                                                                     

/************************************************************************/
/* Bufs used for fragmented I/O (Block I/O not device block aligned)    */
/************************************************************************/
struct fragmented_io_struct {
       struct dk_raw_buf       temp_raw_bp;
       struct buf	       *saved_bp;
       uchar 		       buffer[4096];  /*   Buffer for partials  */
       int 		       first_partial_len;     /*   512 blks     */
       int 		       first_partial_blkno;   /*   512 blks     */
       int 		       first_partial_bcount;  /*   512 blks     */
       int 		       first_partial_baddr;   /*   512 blks     */
       int 		       first_partial_offset;  /*   512 blks     */
       int 		       middle_len;            /*  2048 blks     */
       int 		       middle_blkno;          /*  2048 blks     */
       int 		       middle_bcount;         /*  2048 blks     */
       int 		       middle_baddr;          /*  2048 blks     */
       int 		       last_partial_len;      /*   512 blks     */
       int 		       last_partial_blkno;    /*   512 blks     */
       int 		       last_partial_bcount;   /*   512 blks     */
       int 		       last_partial_baddr;    /*   512 blks     */
       int		       saved_resid;           /* beyond end disk*/
       uchar 		       issued;               /* active fragment */
#define DK_FIRST	0x01		    	/* first partial	*/
#define DK_MIDDLE	0x02			/* middle		*/
#define DK_LAST		0x03			/* last partial 	*/
};                                                                     


/************************************************************************/
/* Device capacity data block 						*/
/************************************************************************/
struct idecdrom_capacity {
	int	lba;	/* last logical block address */
	int	len;	/* block length in bytes */
};

/************************************************************************/
/* Device statistics and thresholds structure                           */
/************************************************************************/
struct idecdrom_stats {
	uint	segment_size;		/* # bytes in sample rate       */
	uint	segment_count;		/* # segments processed         */
	uint	byte_count;		/* # bytes current segment      */
};

/************************************************************************/
/* Disk driver operation descriptor block                               */
/************************************************************************/
struct dk_cmd {
	struct ataide_buf	ataidebuf;	/* buf for adapter use  */
	struct buf		*bp;		/* buf list for cmd     */
	uchar			type;		/* cmd type             */
#define DK_BUF		0x01			/* used by strategy     */
#define DK_IOCTL	0x02			/* used by ioctl        */
#define DK_REQSNS	0x04			/* used to request sense*/
#define DK_RESET	0x08			/* used during reset    */

	uchar			subtype;	/* cmd subtype  	*/

#define DK_TUR		0x01		    	/* test unit ready	*/
#define DK_START	0x02			/* start unit		*/
#define DK_STOP		0x03			/* stop unit		*/
#define DK_MSENSE	0x06			/* mode sense		*/
#define DK_SELECT	0x07			/* mode select		*/
#define DK_READCAP	0x08			/* read capacity	*/
#define DK_RDWR_SENSE	0x09			/* R/W with sense ioctl */
#define DK_IOCMD	0x0a			/* Diagnostic ioctl	*/
#define DK_PREVENT	0x0b			/* Prevent ejection     */
#define DK_ALLOW	0x0c			/* Allow  ejection      */
#define DK_ERP_IOCTL	0x0d			/* Ioctls in which the  */
						/* driver will perform  */
						/* error recovery       */
#define DK_READ_INFO    0xe                     /* Determine if CD is   */
                                                /* multi-session and,   */
                                                /* if so, it will       */
                                                /* return the starting  */
                                                /* address of the last  */
                                                /* session.             */

	uchar			group_type;
#define DK_SINGLE	0x01
#define DK_COALESCED	0x02
#define DK_FRAGMENTED	0x03

	uchar			flags;
#define DK_READY	0x01

	uchar			status;        /* State of command block */
#define DK_FREE		0x00
#define DK_IN_USE	0x01
#define DK_ACTIVE       0x02
#define DK_QUEUED	0x04
#define DK_RETRY	0x08	

	uchar			queued;       /* TRUE if queued         */
	ushort			retry_count;
	uchar			intrpt;        /* used to wait for      */
					       /* resources             */
	uchar			resvd1;
	uchar			resvd2;
	uchar			aborted;       /* TRUE if command is    */
					       /* being aborted		*/
	uchar		        error_type;    /* Type of error         */
					       /* (for logging)		*/
	uint			segment_count;
	uint			soft_resid;
	struct idecdrom_diskinfo	*diskinfo;
	struct dk_cmd		*next;        
	struct dk_cmd		*prev;
};



/************************************************************************/
/* CD-ROM Error Log Record format                                       */
/************************************************************************/

struct idecdrom_error_rec {
	struct err_rec0	entry;		/* defined in sys/err_rec.h	*/
	uchar		status_validity;/* valid values in sys/ata_ide.h*/
	uchar		ata_status;
	uchar		ata_error;
	uchar		reserved1;
	struct atpkt_cmd cmd;		/* ATAPI cmd that caused error	*/
	uchar		req_sense_data[128]; /* sense data              */
	uint		reserved2;
	uint		reserved3;
};


/************************************************************************/
/* Disk bp queue control structs                                        */
/************************************************************************/

struct dk_queue_control {
	struct buf	*head;
	struct buf	*tail;
};

struct dk_bp_queue {
	struct dk_queue_control	pending;
	struct dk_queue_control	in_progress;
};

/************************************************************************/
/* Information for entire driver                                        */
/************************************************************************/
struct idecdrom_info {
	int		config_cnt;	/* Configuration count          */
	int		open_cnt;	/* Number devices open		*/
	lock_t		lock;		/* Lock Word                    */
	struct cdt	*cdt;		/* Component dump table ptr	*/
#ifdef _POWER_MP
	Simple_lock	disk_lock;	/* interrupt-thread lock	*/
#endif


};

/************************************************************************/
/* Structure used for DKIORDSE and DKIOWRSE ioctl's.                    */
/************************************************************************/
struct idecdrom_ioctl_req_sense {
	struct xmem	xmemd;		/* xmemd for request sense data */
	char		*buffer;	/* user address of r.s. buf     */
	int		count;		/* length of user r.s. buf      */
};

/************************************************************************/
/* Mode Data Format Control Structure					*/
/************************************************************************/
struct idecdrom_mode_format {
        signed char page_index[DK_MAX_MODE_PAGES]; /* offset to page    */
        ushort  sense_length;                      /* length of sense   */
        uint    block_length;                      /* device block lgth */
};
struct idecdrom_timer {
	struct watchdog watch;			  /* system watchdog timer*/
	void   *pointer;			  /* diskinfo pointer     */
};
/************************************************************************/
/* Power Management Data Structures                                     */
/************************************************************************/
struct idecdrom_pm_handle {
        struct pm_handle   	      handle;
        struct idecdrom_diskinfo      *diskinfo;
};

/************************************************************************/
/* Information on individual disks                                      */
/************************************************************************/
struct idecdrom_diskinfo {
	struct idecdrom_diskinfo  *next;	/* Ptr next disk's info */
	struct idecdrom_diskinfo  *next_open;   /* Ptr to next open disk*/
	dev_t			devno;          /* Device major+minor   */
						/* numbers		*/

        /*
         * NOTE: the preceding fields must remain in
         * this order to stay consistent with hardware's
	 * thresholding disablement scheme.  In other words they
	 * currently traverse idedisk_open_list of diskinfo structures
	 * via the next pointer looking for a devno match.
         */

	dev_t			adapter_devno;  /* Adapter maj+min      */
	struct idecdrom_timer   watchdog_timer;	/* For reset recovery   */
	uchar			device_id;      /* id for device        */
	ushort			reset_count;    /* Number of reset tries*/
	struct dk_cmd           *dk_cmd_q_head; /* head of dk_cmd queue */
	struct dk_cmd           *dk_cmd_q_tail; /* tail of dk_cmd queue */
	struct dk_cmd           ioctl_cmd;      /* reserved ioctl cmd */

	struct dk_cmd	        *cmd_pool;      /* Pool of buf & ioctl  */
						/* cmds                 */
	int                     pool_index;     /* index into above pool*/

	uint			open_event;	/* open/close event word*/
	struct dk_cmd		*checked_cmd;   /* cmd with check cond. */
	struct dk_cmd	        reset_cmd;      /* reserved reset cmd   */
	struct dk_cmd	        reqsns_cmd;     /* reserved reqsns cmd  */
	struct dk_bp_queue	dk_bp_queue;    /* bp queue control     */

	uchar		mode;   /* Current op mode of device            */
#define DK_NORMAL	0x01    /* Normal IO mode for device            */
#define DK_PASSTHRU	0x02    /* Diagnostic mode for device           */
#define DK_SINGLE_MD	0x04    /* Mode allows audio and changing modes */
	uchar		state;  /* Current state of device              */
#define DK_OFFLINE	0x00    /* Device is uninitialized and offline  */
#define DK_ONLINE	0x01    /* Device is initialized and online     */
#define DK_RST_FAILED	0x02    /* Device is uninitialized and offline  */
#define DK_REQSNS_PENDING   DK_REQSNS  /* Device has request sense      */
				       /* pending			*/
#define DK_RESET_PENDING    DK_RESET   /* Device has reset pending      */

#define DK_RECOV_MASK   (DK_REQSNS_PENDING | DK_RESET_PENDING)
				/* Mask of DK_REQSNS_PENDING	        */
				/*      DK_RESET_PENDING  		*/
	uchar		disk_intrpt;     /* used to wait for resources  */
	uchar		raw_io_intrpt;	/* used for raw I/O event word  */
	uchar		ioctl_chg_mode_flg; /* Used to indicate that the*/
					/* CDMODE ioctl has been	*/
					/* performed.			*/
        uchar           m_sense_status; /* flag if chgable or curr sns  */
#define DK_SENSE_CURRENT        0x00    /* the sense reflects current   */
#define DK_SENSE_CHANGEABLE     0x40    /* the sense reflects changeable*/
	uchar		opened;         /* device has been opened       */

	uchar		cmd_pending;    /* cmd pending in strategy      */
	uchar		errno;          /* saved error on open path     */
	uchar		timer_status;	/* If a watchdog timer is set   */
					/* as well as why it is set	*/
#define DK_TIMER_PENDING   0x1		/* Timer is set			*/
#define DK_TIMER_BUSY      0x2		/* Timer was set due to IDE     */
					/* Busy status			*/
	
	uchar		restart_unit;   /* Issue SC_RESUME to adapter   */
	uchar		retry_flag;     /* Set if a retry is to be done */

	uchar		starting_close; /* TRUE if a close is being     */
					/* attempted for this disk.	*/
	uchar		reset_failures; /* Number of times the reset    */
					/* cycle has failed without a   */
					/* a good buf or ioctl 		*/
					/* completion.			*/
	uchar           prevent_eject;  /* Prevent target's media from  */
					/* being ejected while open.    */ 
  	uchar		cfg_prevent_ej; /* prevent eject configurable   */
					/* attribute's value.		*/
  	uchar		cfg_reserve_lck;/* reserve lock configurable    */
					/* attribute's value.		*/
	uchar		ioctl_pending;  /* Ioctl operation pending.	*/
        uchar           play_audio;     /* TRUE if optional	        */
                                        /* play audio command set is    */
                                        /* supported by the device.     */
	uchar		overide_pg_e;	/* TRUE if a play audio ioctl	*/
					/* has been requested to change	*/
					/* the volume or audio channel	*/
					/* mapping, since opened.	*/
        uchar   cd_mode2_form1_flag;    /* TRUE if data mode 2 form 1   */
                                        /* is supported by device.      */
                                        /* (Mandatory in ATAPI spec)    */
        uchar   cd_mode2_form2_flag;    /* TRUE if data mode 2 form 2   */
                                        /* is supported by device.      */
                                        /* (Mandatory in ATAPI spec)    */
        uchar   	cd_da_flag;     /* TRUE if data mode CD-DA      */
                                        /* is supported by device.      */
                                        /* (Optional in ATAPI spec)     */
        uchar           current_cd_mode;/* The Current CD-ROM Data mode */
        uchar           multi_session;  /* TRUE if the CD-ROM drive     */
                                        /* supports multi session discs */
        uchar           mult_of_blksize;/* The multiple needed to go    */
                                        /* from DK_BLOCKSIZE to the     */
                                        /* diskinfo->block_size         */
	uchar		play_audio_started; /* A play audio command was */
					/* issued without a subsequent  */
					/* stop unit.  Thus the play	*/
					/* operation may still be in 	*/
					/* progress.			*/
	ushort 		rw_timeout;	/* Time-out period for reads and*/
					/* writes.			*/
	ushort 		start_timeout;	/* Start Unit time-out period   */
					/* (secs).			*/
	ushort          queue_depth;	/* maximum number of commands   */
					/* that can be queued up to a   */
					/* device			*/
	ushort	        cmds_out;       /* Number of command issued for */
					/* this device			*/
        struct dk_cmd   *raw_io_cmd;	/* Pointer to a raw I/O command	*/
					/* that either are not on block	*/
					/* boundaries or do not have	*/
					/* transfer lengths that are 	*/
					/* multiples of the block size.	*/
	struct buf 	*currbuf;	/* current buf of elevator      */
	struct buf 	*low;		/* low buf of elevator          */
					/* that can be queued up to a   */
					/* device			*/
        uint            block_size;     /* The device's current block   */
                                        /* size.                        */
        uint            cfg_block_size; /* The block size specified by  */
                                        /* the configuration method.    */

        uint            last_ses_pvd_lba; /* The location of the PVD    */
                                        /* (Physical Volume Descriptor) */
                                        /* on the last session of a     */
                                        /* multi-session CD.            */

	uint		max_request;    /* Maximum request to scsi dd   */
	uint		max_coalesce;	/* Maximum size to coalesce	*/
   
	lock_t		lock;           /* Lock Word                    */
	struct file	*fp;            /* filepointer for dev--- calls */

	struct idecdrom_error_rec error_rec; /* Error record for device */

	struct idecdrom_stats stats;      /* I/O & Soft error statistics*/
	int		mode_data_length; /* Length of useable mode data*/
        uchar           disc_info;       /* Disc information flag       */
	uchar		mode_buf[256];  /* Mode data buffer             */
	uchar		sense_buf[256]; /* Request Sense data buffer    */
	uchar		ch_data[256];	/* buffer of chngeable mode data*/
	uchar		df_data[256];	/* buffer of default mode data  */
	uchar ioctl_buf[DK_REQSNS_LEN]; /* Ioctl data buffer		*/

	uchar mode_page_e[DK_MAX_SIZE_PG_E-DK_VOL_START_PG_E+1]; 
					/* Volume control settings from	*/
					/* Mode Page E in ODM.		*/ 
	struct idecdrom_mode_format dd;   /* Mode control for desired data*/
        struct idecdrom_mode_format df;   /* Mode control for default data*/
        struct idecdrom_mode_format ch;   /* Mode control for chngeable   */
        struct idecdrom_mode_format cd;   /* Mode control for current     */
	int		number_pages_to_select; /* remaining mode selects */
	uchar		select_buf[512];	/* buffer for mode select */
	uchar		select_page_offset[DK_MAX_MODE_PAGES]; /*select offset*/
	struct idecdrom_ioctl_req_sense ioctl_req_sense;/* Req Sense info */
	struct fragmented_io_struct fragment; /* Block I/O 512 aligned    */
	struct idecdrom_capacity capacity; /* Disk capacity information   */
	struct dkstat	dkstat;		/* Disk iostat information      */
        struct idecdrom_pm_handle pmh;           /* power management pm_handle */
        uchar                   pm_pending;     /* PM pending operation */
#define PM_IDECDROM_PENDING_OP           0x01
#define PM_IDECDROM_PENDING_SUSPEND      0x02
        uint                    pm_event;       /* PM event word        */
        struct idecdrom_timer    pm_timer;

};

/************************************************************************/
/* Request Sense Data Block                                             */
/************************************************************************/
struct idecdrom_req_sense_info  {
	uchar	err_code;	/* error class and code   */
	uchar	rsvd0;
	uchar	sense_key;
	uchar	sense_byte0;
	uchar	sense_byte1;
	uchar	sense_byte2;
	uchar	sense_byte3;
	uchar	add_sense_length;
	uchar	add_sense_byte0;
	uchar	add_sense_byte1;
	uchar	add_sense_byte2;
	uchar	add_sense_byte3;
	uchar	add_sense_key;
	uchar	add_sense_qualifier;
	uchar	fru;
	/*klh: check following fields*/
	uchar	flag_byte;
	uchar	field_ptrM;
	uchar	field_ptrL;
	uchar	rsvd2;
	uchar	rsvd3;
	uchar	unit_ref_code1;
	uchar	unit_ref_code2;
	uchar	rsvd4;
	uchar	rsvd5;
	uchar	error_rec0;
	uchar	error_rec1;
	uchar	error_rec2;
	uchar	error_rec3;
	uchar	error_rec4;
	uchar	error_rec5;
	uchar	rsvd6;
	uchar	rsvd7;
};

#ifndef _NO_PROTO 
/************************************************************************/
/* idecdrom functions                                                   */
/************************************************************************/

int	idecdrom_config(dev_t devno, int op, struct uio *uiop);
int	idecdrom_open(dev_t devno, int rwflag, int chan, int ext);
int	idecdrom_close(dev_t devno, int chan, int ext);
int	idecdrom_read(dev_t devno, struct uio *uiop, int chan, int ext);
int	idecdrom_rdwr(dev_t devno,struct uio *uiop,int chan, 
		    int ext,int rw_flag);

int	idecdrom_mincnt(struct buf *bp, void *minparms);
int	idecdrom_raw(struct uio *uiop,int ext,int rw_flag,
		   struct idecdrom_diskinfo  *diskinfo);

#ifdef _LONG_LONG
int	idecdrom_raw_buffer(struct iovec *iovp,struct xmem *xmemp,
			  struct uio *uiop,offset_t offset,
			  int rw_flag,
			  struct idecdrom_diskinfo  *diskinfo,
			  int ext);
int     idecdrom_io_buffer(struct iovec *iovp,struct xmem  *xmemp,  
			 struct uio *uiop, offset_t offset,  
                         int  rw_flag, 
			 struct idecdrom_diskinfo  *diskinfo,
			 int size,             
			 int  start_flag);
int	idecdrom_raw_io(struct iovec *iovp,struct uio *uiop,
		      offset_t  offset,  int rw_flag,
		      struct idecdrom_diskinfo *diskinfo, 
		      int size);
#else
int	idecdrom_raw_buffer(struct iovec *iovp,struct xmem *xmemp,
			  struct uio *uiop,off_t offset,
			  int rw_flag,
			  struct idecdrom_diskinfo  *diskinfo,
			  int ext);

int     idecdrom_io_buffer(struct iovec *iovp,struct xmem  *xmemp,  
			 struct uio *uiop, off_t offset,  
                         int  rw_flag, 
			 struct idecdrom_diskinfo  *diskinfo,
			 int size,             
			 int  start_flag);
int	idecdrom_raw_io(struct iovec *iovp,struct uio *uiop,
		      off_t  offset,  int rw_flag,
		      struct idecdrom_diskinfo *diskinfo, 
		      int size);
#endif
struct dk_cmd *idecdrom_build_raw_cmd(struct idecdrom_diskinfo *diskinfo,
				    int rw_flag, int  block_number,
				    int size, char *buffer,
				    struct dk_raw_buf *raw_bp, int aspace);
int	idecdrom_ioctl(dev_t devno, int op, int arg, int flag, int chan,int ext);
int     idecdrom_rdse(int op,int arg, int devflag, 
		      struct idecdrom_diskinfo	*diskinfo);
int	idecdrom_cdiocmd(int arg, int devflag, 
		      struct idecdrom_diskinfo	*diskinfo);
int	idecdrom_cd_amr_pmr(int op,struct idecdrom_diskinfo	*diskinfo);
int	idecdrom_cdeject(struct idecdrom_diskinfo	*diskinfo);
int	idecdrom_cdformat(int arg, int devflag, 
			struct idecdrom_diskinfo *diskinfo);
int	idecdrom_cdaudio(int arg, int devflag,
		       struct idecdrom_diskinfo  *diskinfo);
int	idecdrom_cdmode(int arg,int devflg,
			   struct idecdrom_diskinfo  *diskinfo);
int     idecdrom_alloc(struct idecdrom_diskinfo *diskinfo);
void    idecdrom_init_cmds(struct idecdrom_diskinfo *diskinfo);
void    idecdrom_allow(struct idecdrom_diskinfo *diskinfo);
int	idecdrom_ioctl_mode_sense(struct idecdrom_diskinfo  *diskinfo,
				uchar  pc,uchar page_code,
				int allocation_length);
int 	idecdrom_audio_msf(struct idecdrom_diskinfo  *diskinfo,
			 int start_msf,int end_msf);
int	idecdrom_audio_trk_indx(struct idecdrom_diskinfo  *diskinfo,
			      uchar  start_trk,uchar  start_indx,
			      uchar  end_trk,uchar end_indx);
int	idecdrom_pause_resume(struct idecdrom_diskinfo  *diskinfo,
			    uchar pause_resume_flag);
int	idecdrom_read_subchnl(struct idecdrom_diskinfo  *diskinfo,
			    uchar sub_chnl_fmt,uchar  trk_num,
			    uchar subq,uchar msf,
			    int allocation_length);
int     idecdrom_read_toc(struct idecdrom_diskinfo  *diskinfo,uchar start_trk,
			uchar msf,int allocation_length, uchar format);
int	idecdrom_pin_buffer(caddr_t  buffer_addr,int *length,short segflag,
			  register int    blocksize);

int	idecdrom_strategy(struct buf *bp);
void	idecdrom_pending_enqueue(struct idecdrom_diskinfo *diskinfo, 
	struct buf *bp);
void	idecdrom_pending_dequeue(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_start_disable(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_start(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_iodone(struct dk_cmd *cmd_ptr);
void	idecdrom_process_good(struct dk_cmd *cmd_ptr);
void	idecdrom_process_sense(struct dk_cmd *cmd_ptr);
void idecdrom_recover_adap_q(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_process_error(struct dk_cmd *cmd_ptr);
void	idecdrom_process_passthru_error(struct dk_cmd *cmd_ptr);
void	idecdrom_process_diagnostic_error(struct dk_cmd *cmd_ptr);
void	idecdrom_process_adapter_error(struct dk_cmd *cmd_ptr);
void	idecdrom_process_ide_error(struct dk_cmd *cmd_ptr);
void	idecdrom_process_buf_error(struct dk_cmd *cmd_ptr);
void	idecdrom_process_ioctl_error(struct dk_cmd *cmd_ptr);
void	idecdrom_process_reset_error(struct dk_cmd *cmd_ptr);
void    idecdrom_fail_disk(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_process_reqsns_error(struct dk_cmd *cmd_ptr);
void	idecdrom_process_special_error(struct dk_cmd *cmd_ptr);
void	idecdrom_process_buf(struct dk_cmd *cmd_ptr);
void	idecdrom_process_reset(struct dk_cmd *cmd_ptr);
uchar	idecdrom_coalesce(struct idecdrom_diskinfo *diskinfo);
struct dk_cmd	*idecdrom_fragmented_build_cmd(struct idecdrom_diskinfo 
				  *diskinfo, char good_path);
struct dk_cmd	*idecdrom_build_cmd(struct idecdrom_diskinfo *diskinfo, 
				  struct buf **head, char good_path);
void	idecdrom_write_verify(struct idecdrom_diskinfo *diskinfo, 
	struct dk_cmd *prev_cmd_ptr, ulong blkno);
void	idecdrom_request_sense(struct idecdrom_diskinfo *diskinfo,
        struct dk_cmd *cmd_ptr);
void	idecdrom_start_unit(struct idecdrom_diskinfo *diskinfo, uchar
			  start_stop_flag);
void	idecdrom_start_unit_disable(struct idecdrom_diskinfo *diskinfo, uchar
				  start_stop_flag);
void	idecdrom_test_unit_ready_disable(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_test_unit_ready(struct idecdrom_diskinfo *diskinfo);

void	idecdrom_prevent_allow_disable(struct idecdrom_diskinfo *diskinfo, uchar
				     prevent_allow_flag);
void	idecdrom_prevent_allow(struct idecdrom_diskinfo *diskinfo, uchar
			     prevent_allow_flag);
void	idecdrom_reserve(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_mode_sense(struct idecdrom_diskinfo *diskinfo);
void 	idecdrom_format_mode_data(char *mode_data,struct idecdrom_mode_format *mf,
        int sense_length,char over_ride,struct idecdrom_diskinfo *diskinfo);
int	idecdrom_mode_data_compare(struct idecdrom_diskinfo *diskinfo);
void    idecdrom_q_mode(struct idecdrom_diskinfo *diskinfo,char  i,char ch_index,
		     char dd_index);
void	idecdrom_mode_select(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_read_cap(struct idecdrom_diskinfo *diskinfo);
void    idecdrom_read_disc_info(struct idecdrom_diskinfo    *diskinfo);
void	idecdrom_release_disable(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_release(struct idecdrom_diskinfo *diskinfo);
void	idecdrom_build_error(struct dk_cmd *cmd_ptr, int type, int priority, 
	int valid_sense);
void	idecdrom_log_error(struct dk_cmd *cmd_ptr, int sev);
void	idecdrom_watchdog(struct watchdog *watchdog_timer);
void    idecdrom_pm_watchdog(struct watchdog *watchdog_timer);
struct cdt 	*idecdrom_cdt_func(int arg);
void idecdrom_q_cmd(struct  dk_cmd *cmd_ptr,char queue,uchar  which_q);
void idecdrom_d_q_cmd_disable(struct dk_cmd *cmd_ptr,uchar which_q);
void idecdrom_d_q_cmd(struct dk_cmd *cmd_ptr,uchar which_q);
struct  dk_cmd *idecdrom_cmd_alloc_disable(struct idecdrom_diskinfo *diskinfo,
					 uchar cmd_type);
struct  dk_cmd *idecdrom_cmd_alloc(struct idecdrom_diskinfo *diskinfo,
				 uchar cmd_type);
void idecdrom_free_cmd_disable(struct dk_cmd *cmd_ptr);
void idecdrom_free_cmd(struct dk_cmd *cmd_ptr);
void idecdrom_sleep(uchar *intrpt,uint  *event);
void idecdrom_start_watchdog(struct idecdrom_diskinfo *diskinfo,ulong timeout);
void idecdrom_raw_iodone(struct dk_raw_buf *raw_bp);
void idecdrom_trc_disable(char *desc,char *type,char count,uint word1,uint word2,
		uint word3,uint word4,uint word5);
void idecdrom_trc(char *desc,char *type,char count,uint word1,uint word2,
		uint word3,uint word4,uint word5);
int	idecdrom_spindown_closeddisk(struct idecdrom_diskinfo *diskinfo);
int	idecdrom_pm_handler(caddr_t private, int mode);


#else

/************************************************************************/
/* idecdrom functions                                                   */
/************************************************************************/

int	idecdrom_config();
int	idecdrom_open();
int	idecdrom_close();
int	idecdrom_read();
int	idecdrom_write();
int	idedcrom_rdwr();
int	idedcrom_mincnt();
int	idedcrom_raw();
int	idedcrom_raw_buffer();
int     idedcrom_io_buffer();
int	idedcrom_raw_io();
struct dk_cmd *idedcrom_build_raw_cmd();
int	idedcromidecdrom_ioctl();
int     idecdrom_rdse();
int	idecdrom_cdiocmd();
int	idecdrom_cd_amr_pmr();
int	idecdrom_cdeject();
int	idecdrom_cdformat();
int	idecdrom_cdaudio();
int	idecdrom_cdmode();
int     idecdrom_alloc();
void    idecdrom_init_cmds();
void    idecdrom_allow();
int	idedcrom_ioctl_mode_sense();
int 	idedcrom_audio_msf();
int	idedcrom_audio_trk_indx();
int	idedcrom_pause_resume();
int	idedcrom_read_subchnl();
int     idedcrom_read_toc();
int	idedcrom_pin_buffer();


int	idecdrom_strategy();
void	idecdrom_pending_enqueue();
void	idecdrom_pending_dequeue();
void	idecdrom_start_disable();
void	idecdrom_start();
void	idecdrom_iodone();
void	idecdrom_process_good();
void	idecdrom_process_sense();
void 	idecdrom_recover_adap_q();
void	idecdrom_process_error();
void	idecdrom_process_passthru_error();
void	idecdrom_process_diagnostic_error();
void	idecdrom_process_adapter_error();
void	idecdrom_process_ide_error();
void	idecdrom_process_buf_error();
void	idecdrom_process_ioctl_error();
void	idecdrom_process_reset_error();
void    idecdrom_fail_disk();
void	idecdrom_process_reqsns_error();
void	idecdrom_process_special_error();
void	idecdrom_process_buf();
void	idecdrom_process_reset();
uchar	idecdrom_coalesce();
struct dk_cmd	*idecdrom_fragmented_build_cmd();
struct dk_cmd	*idecdrom_build_cmd();
void	idecdrom_write_verify();
void	idecdrom_request_sense();
void	idecdrom_start_unit_disable();
void	idecdrom_start_unit();
void	idecdrom_test_unit_ready_disable();
void	idecdrom_test_unit_ready();
void	idecdrom_prevent_allow_disable();
void	idecdrom_prevent_allow();
void	idecdrom_reserve();
void	idecdrom_mode_sense();
void 	idecdrom_format_mode_data();
int 	idecdrom_mode_data_compare();
void    idecdrom_q_mode();
void	idecdrom_mode_select();
void	idecdrom_read_cap();
void    idecdrom_read_disc_info();
void	idecdrom_release_disable();
void	idecdrom_release();
void	idecdrom_build_error();
void	idecdrom_log_error();
void	idecdrom_watchdog();
void    idecdrom_pm_watchdog();
struct cdt	*idecdrom_cdt_func();
void idecdrom_q_cmd();
void idecdrom_d_q_cmd();
void idecdrom_d_q_cmd_disable();
struct  dk_cmd *idecdrom_cmd_alloc_disable();
struct  dk_cmd *idecdrom_cmd_alloc();
void idecdrom_free_cmd_disable();
void idecdrom_free_cmd();

void idecdrom_sleep();
void idecdrom_start_watchdog();
void idecdrom_raw_iodone(struct buf *bp);
void idecdrom_trc_disable();
void idecdrom_trc();

int	idecdrom_spindown_closeddisk();
int	idecdrom_pm_handler();
#endif /* _NO_PROTO */
#endif /* _H_IDECDROMDD */
