/* @(#)47	1.1  src/rspc/usr/lib/boot/softros/roslib/rtcodes.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:44  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define OK                     0
#define TRUE                   1        /* TRUE = 1        */
#define NO_INT                 2
#define GET_RESULTS            3
#define RDY_TO_WRITE           4
#define NO_DRIVE_PRESENT       5
#define WRONG_INTERRUPT        6
#define NO_NIO_CARD            7
#define INVALID_DEVICE         8
#define READ_TEST              9
#define DRIVE_TEST             10
#define NO_SCSI_CARD           11
#define INQUIRY_CMD_FAILED     12
#define CMD_FAILED             13
#define INVALID_IPLRECID       14
#define READ_ID_FAILED         15
#define MEDIA_TEST             16
#define BAD_READMEDIA_PARMS    17
#define DONT_KNOW_MEDIA        18
#define SPECIFY_FAILED         19
#define BAD_SENSE_INT_RESULTS  20
#define BAD_SENSE_INT_CMD      21
#define READ_CMD_FAILED        22
#define RID_CMD_RESULTS_ERR    23
#define READ_CMD_RESULTS_ERR   24
#define NO_DISKETTE            25
#define SEEK_FAILED            26
#define RECAL_FAILED           27
#define RESERVATION_CONFLICT   28
#define DEVICE_NOT_AVAILABLE   29
#define TEST_UNIT_READY_FAILED 30
#define BLUEB_WRONG_CMD_INT    31
#define MB31_ERROR             32
#define BLUEB_INT_TIMEOUT      33
#define STILL_DONT_KNOW        34
#define MAY_NEED_TO_INIT       35
#define SHORT_LENGTH           36
#define STILL_WAITING          37
#define MODE_SELECT_FAILED     38
#define TAPE_REWIND_FAILED     39
#define TAPE_SPACE_FAILED      40
#define INITIAL_MEMORY_BAD     41
#define NO_FRAGMENTATION       42
#define NOT_ENOUGH_MEMORY      43
#define FATAL_ERROR            44
#define SET_MODE_FAILED        45
#define SCSI_DEVICE_NOT_AVAIL  46
#define RETURN_TO_SCRIPT       47
#define SENSE_NEEDED           49
#define HARR_FAILED            50
#define RESET_NEEDED           51
#define TIMEOUT_BOUNDARY       52
#define WAIT_ON_MSR_TIMEOUT    53
#define WAIT_ON_BUSY_TIMEOUT   54
#define NOT_READY              55
#define INVALID_COMMAND        56
#define WRITE_PROTECT          57
#define SHORT_RECORD           0x80000000
#define NO_SENSE               58
#define MEDIA_ERROR            59
#define HARDWARE_ERROR         60
#define UNIT_ATTENTION         61
#define BLANK_CHECK            62
#define CMD_ABORTED            63
#define BUSY                   64
#define NO_MEDIA               65
#define SCRIPT_ERROR           66
#define INVALID_ADDRESS        67
#define MEDIA_CHANGED          68
#define DEVICE_RESET           69
#define FORMAT_IN_PROGRESS     70
#define FORMAT_CORRUPT         71
#define START_REQUIRED         72    // Start Unit command required */ /* see note in scsi_ctl.c
#define DEVICE_ID_CONFLICT     73
#define MANUAL_INTERVENTION    74
#define OP_WANTS_MEDIA         75    // Operator request for media with media removal prevented
#define DISCONNECT             76
