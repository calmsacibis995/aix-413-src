/* @(#)48	1.1  src/bos/usr/samples/cdrom/cdromdd.h, cdrmsamp, bos411, 9428A410j 4/27/90 13:08:25 */
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
#ifndef  _H_CDROMDD
#define _H_CDROMDD
/*
 * COMPONENT_NAME: SCSI CD-ROM Device Driver Include File
 *
 * FUNCTIONS:	NONE
 * 
 * ORIGINS: 27
 * 
 */

/*********************************************************************/
/* cdromdd.h header dependencies				     */
/*********************************************************************/
#include <sys/buf.h>
#include <sys/scsi.h>
#include <sys/types.h>
#include <sys/watchdog.h>
#include <sys/iostat.h>

/************************************************************************/
/* Sense key values                                                     */
/************************************************************************/
#define CD_NO_SENSE             0x00
#define CD_RECOVERED_ERROR      0x01
#define CD_NOT_READY            0x02
#define CD_MEDIUM_ERROR         0x03
#define CD_ILLEGAL_REQUEST      0x05
#define CD_HARDWARE_ERROR       0x04
#define CD_UNIT_ATTENTION       0x06
#define CD_ABORTED_COMMAND      0x0B

/************************************************************************/
/* Other misc. defines                                                  */
/************************************************************************/
#define CD_MAX_RETRY            0x01    /* # of retries to attempt      */
#define CD_MAX_RESET_RETRY      0x03    /* # resets to attempt          */
#define CD_NO_RETRY             0x7F    /* don't retry (> CD_MAX_RETRY) */
#define CD_WAIT                 0xFD    /* cd_error rtn val = wait      */
#define CD_RETRY                0xFE    /* cd_error rtn val = retry cmd */
#define CD_RESET                0xFF    /* cd_error rtn val = reset dev */
#define CD_TIMEOUT              0x1E    /* command timeout value (secs) */
#define CD_WATCHDOG             0x05    /* watchdog timeout value (secs)*/
#define CD_PAGE_CODE            0xBF    /* page code for MODE_SENSE     */
#define CD_MODE_SELECT_SIZE      32     /* size of MODE_SELECT data      */
#define CD_MEDIUM_CHANGED       0x28    /* additional sense key         */
#define CD_START_UNIT           0x01    /* flag for start/stop unit cmd */
#define CD_STOP_UNIT            0x00    /* flag for start/stop unit cmd */
#define CD_PASS_THRU            0xEF    /* cmd state for pass-thru ioctl*/
#define CD_ORIGINAL_REQUEST     0x01    /* flag for cd_start routine    */
#define CD_CONTINUED_REQUEST    0x02    /* flag for cd_start routine    */
#define CD_HASHSIZE		0x0F

/************************************************************************/
/* Device capacity data block                                           */
/************************************************************************/
struct cd_capacity {
        int     lba;                    /* last logical block address-1 */
        int     len;                    /* block length in bytes        */
};

/************************************************************************/
/* Request Sense Data Block                                             */
/************************************************************************/
struct cd_req_sense_info  {
       uchar           err_code;        /* error class and code         */
       uchar           rsvd0;
       uchar           sense_key;
       uchar           addr_byte0;
       uchar           addr_byte1;
       uchar           addr_byte2;
       uchar           addr_byte3;
       uchar           add_sense_length;
       uchar           add_sense_byte0;
       uchar           add_sense_byte1;
       uchar           add_sense_byte2;
       uchar           add_sense_byte3;
       uchar           add_sense_key;
       uchar           rsvd1;
       uchar           fru;
       uchar           flag_byte;
       uchar           field_ptrM;
       uchar           field_ptrL;
};

/************************************************************************/
/* Buffer block definition (includes sc_buf)                            */
/************************************************************************/

struct cd_buf_block {
        struct sc_buf         scsi_buf;
        uint                  retry_count;
        struct cd_disk_df     *disk_ptr;
};

/************************************************************************/
/* Structure used for transfering request sense info on an ioctl.       */
/************************************************************************/
struct cd_ioctl_req_sense {
        struct xmem     xmemd;
        char            *buffer;
        int             count;
};

/************************************************************************/
/* Information on individual disks                                      */
/************************************************************************/
struct cd_disk_df {
        struct watchdog  watchdog_timer;     /* Stucture used for timer      */
        struct cd_buf_block norm_buf;        /* Ptr to scsi buf, etc.        */
        struct cd_buf_block reset_buf;       /* buf block to use for a reset */
        struct cd_buf_block rs_buf;          /* buf block to use for req.sens*/
        struct cd_buf_block ioctl_buf;       /* buf block to use for ioctl's */
        struct sc_error_log_df log_struct;   /* error log structure          */
        struct cd_disk_df *next_defined;     /* Ptr to next defined disk_df  */
        struct cd_disk_df *next_opened;      /* Ptr to next opened disk_df   */
        dev_t            devno;              /* What device number           */
        struct buf       *head;              /* Next buf struct in queue     */
        struct buf       *tail;              /* Last item in queue           */
        struct file      *fp;                /* File pointer for dev-- calls */
        struct buf       rdse_buf_struct;    /* used for CDIORDSE ioctl      */
        struct cd_capacity disk_nblks;       /* Number of blocks on disk     */
        struct cdrom_dds_df disk_ddi;        /* ddi info assoc. with device  */
        struct cd_req_sense_info req_sense_data;/* storage for r.s. data     */
        struct cd_ioctl_req_sense ioctl_req_sense;/* req sense info for ioctl*/
	struct dkstat	 cd_dkstat;	     /* for io statistics measure    */
        int              lock_word;          /* lock structure for this dev  */
        int              reset_count;        /* number of resets tried       */
        uchar            cmd_state;          /* current command              */
        uchar            old_cmd_state;      /* previous command             */
        uchar            reset_state;        /* previous command during reset*/
        uchar            busy;               /* a command is in progress     */
        uchar            open_pending;       /* cd_open started the reset op.*/
        uchar            ioctl_pending;      /* the cmd came from an ioctl   */
        uchar            ioctl_rqst;         /* a CDIORDSE ioctl is waiting  */
        uchar            reset_pending;      /* a device reset is in progress*/
        uchar            error_pending;      /* set if recovering from error */
        uchar            cmd_pending;        /* cmd pending in strategy      */
        uchar            diag_mode;          /* device is open in diag mode  */
        uchar            reset_failed;       /* the device is dead           */
        uchar            failed_errno;       /* errno for the dead device    */
        uchar            opened;             /* the device has been opened   */
        uchar            retain_reservation; /* no release on close          */
	uchar		 card_scsi_id;	     /* SCSI ID of the adapter	     */
	int		 max_transfer;	     /* max transfer size allowed    */
	int		 reserve_lock;	     /* Lock target from other inits */
};

/************************************************************************/
/* Disk Structure Hash Table Definition                                 */
/************************************************************************/

struct cd_hash_table_df {
        struct cd_disk_df       *defined_head;
        struct cd_disk_df       *opened_head;
};

#ifndef _NO_PROTO
int    cd_config( dev_t devno, int op, struct uio *uiop );
int    cd_open( dev_t devno, int devflag, int chan, int ext );
int    cd_close( dev_t devno, int chan, int ext);
int    cd_read( dev_t devno, struct uio *uiop, int chan, int ext );
int    cd_ioctl( dev_t devno, int op, int arg, ulong dev_flag );
char   cd_open_support( struct cd_disk_df *disk_ptr, int ext,
	char disk_offset );
int    cd_mincnt( struct buf bp, void *minparms );
void   cd_release( struct cd_disk_df *disk_ptr );
void   cd_pass_thru( struct cd_disk_df *disk_ptr, struct sc_iocmd *sc_iocmd_ptr,
 struct xmem *xmem_desc_ptr );
void   cd_device_reset( struct cd_disk_df *disk_ptr );
void   cd_mode_select( struct cd_disk_df *disk_ptr );
void   cd_read_capacity( struct cd_disk_df *disk_ptr );
void   cd_reserve( struct cd_disk_df *disk_ptr );
void   cd_start_stop( struct cd_disk_df *disk_ptr, struct sc_buf *scsi_buf_ptr,
 uchar start_stop_flag );
void   cd_start( struct cd_disk_df *disk_ptr, uchar flag );
uchar  cd_busy( struct cd_disk_df *disk_ptr );
void   cd_watchdog( struct watchdog *watchdog_timer );
int    cd_strategy( struct buf *buf_ptr );
uchar  cd_scsi_error( struct cd_disk_df *disk_ptr );
uchar  cd_adapter_error( struct cd_buf_block *buf_ptr,
 struct cd_disk_df *disk_ptr );
void   cd_iodone( struct buf *bp );

#else

int    cd_config();
int    cd_open();
int    cd_close();
int    cd_read();
char   cd_open_support();
int    cd_ioctl();
int    cd_mincnt();
void   cd_release();
void   cd_pass_thru();
void   cd_device_reset();
void   cd_mode_select();
void   cd_read_capacity();
void   cd_reserve();
void   cd_start_stop();
void   cd_start();
uchar  cd_busy();
void   cd_watchdog();
int    cd_strategy();
uchar  cd_scsi_error();
uchar  cd_adapter_error();
void   cd_iodone();

#endif /* _NO_PROTO */
#endif /* ! _H_CDROMDD */
