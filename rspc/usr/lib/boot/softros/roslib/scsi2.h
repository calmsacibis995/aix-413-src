/* @(#)43	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi2.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:36  */
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

/* R2/scsi2.h, rosipl_601, rosipl_13000 2/23/93 12:12:11 */

#define ASC  12                      /* additional sense code */
#define ASCQ 13                      /* additional sense code qualifier */
#define SENSE_KEY            (0x02)
#define NR                   (0x02)
#define LU_NOT_SUPPORTED     (0x2500)
#define LU_NOT_RESP          (0x0500)
#define NO_MEDIA_PRESENT     (0x3A00)
#define CAUSE_NOT_REPORTABLE (0x0400)
#define INIT_REQUIRED        (0x0402)
#define CART_NOT_PRESENT     (0x0012)
#define CART_NOT_LOADED      (0x0013)
#define FORMAT_IN_PROG       (0x0404)
#define DEFAULT_DENSITY      (0x0)
#define NO_DENSITY_CHANGE    (0x7f)
#define DEFAULT_BYTE_COUNT   (0xff)
#define LUN_NOT_SUPPORTED    (0x7f)
#define MODE_SEL_BYTE_COUNT  (12)
#define SELF_TEST_BIT        (0x00040000)
#define PF_BIT               (0x00100000)
#define DEV_OFFLINE_BIT      (0x00020000)
#define EXABYTE_HIGH         (0x45584142)
#define EXABYTE_LOW          (0x59544520)
#define TANDBERG_HIGH        (0x54414e44)
#define TANDBERG_LOW         (0x42455247)
#define HP_HIGH              (0x48502020)
#define HP_LOW               (0x20202020)

/* The command definiton 0xffffffff is reserved */
/* for use by the NCR contoller self test, it   */
/* is not declared public                       */
//#define TEST_UNIT_ready        (0x00000000)
#define TAPE_rewind            (0x01000000)
//#define REQUEST_sense          (0x03000000)
#define FORMAT_unit            (0x04000000)
#define TAPE_read              (0x08010000)
#define TAPE_space             (0x11000000)
#define DEVICE_inquiry         (0x12000000)
//#define DIAGnose               (0x1d000000)
#define MODE_select            (0x15000000)
#define MODE_sense             (0x1a000000)
//#define START_motor_immed      (0x1b000001)
//#define START_motor            (0x1b000000)
//#define READ_CAPacity          (0x25000000)
#define DISK_read              (0x28000000)
#define READ_defects           (0x37000000)
#define OP_CODE_MASK           (0xff000000)
#define BYTE_COUNT_MASK        (0xff000000)
#define TARGET_ID_MASK         (0x00ff0000)
//#define READ_scsi_data         (0x00000001)
//#define WRITE_scsi_data        (0x2A000000)
#define SCSI_adapter_self_test (0x00000002)
#define SOFT_reset             (0x00000003)
#define FEATURE_control        (0x00000004)
#define IMMEDIATE_assign       (0x00000005)
#define ADAPTER_diagnose       (0x00000006)
#define LOCK_media             (0x1E000001)
#define UNLOCK_media           (0x1E000000)
#define EJECT_media            (0x1b000002)
#define MEM_TO_MEM             (0xFFFFFFF0)

#define SCSI2_TEST_UNIT_READY  (0x00)
#define SCSI2_REZERO_UNIT      (0x01)
#define SCSI2_REWIND           (0x01)
#define SCSI2_REQUEST_SENSE    (0x03)
#define SCSI2_READ_6           (0x08)
#define SCSI2_SEEK_6           (0x11)
#define SCSI2_SPACE            (0x11)
#define SCSI2_INQUIRY          (0x12)
#define SCSI2_SEND_DIAGNOSTIC  (0x1d)
#define SCSI2_MODE_SELECT_6    (0x15)
#define SCSI2_START_STOP_UNIT  (0x1b)
#define SCSI2_READ_10          (0x28)

/* SCSI Bus phase definitions */
#define DATA_OUT_PHASE                  0
#define DATA_IN_PHASE                   1
#define CMD_PHASE                               2
#define STATUS_PHASE                    3
#define MSG_OUT_PHASE                   6
#define MSG_IN_PHASE                    7


#define SCSI2_MAX_LUN_ID     (7)
/*---------------------------------------------------------------------------*/
/* Peripheral Device Types returned from Inquiry data                        */
/*---------------------------------------------------------------------------*/
#define DASD1                  0
#define TAPE1                  1
#define CD_ROM1                4
#define CD_ROM2                5
#define CD_ROM3                7
/*---------------------------------------------------------------------------*/
/* Status Byte Codes                                                         */
/*---------------------------------------------------------------------------*/
#define SCSI_STATUS_GOOD                 (0x00)
#define SCSI_STATUS_CHECK_CONDITION      (0x02)
#define SCSI_STATUS_CONDITION_MET        (0x04)
#define SCSI_STATUS_DEVICE_BUSY          (0x08)
#define SCSI_STATUS_INTERMEDIATE         (0x10)
#define SCSI_STATUS_INTERMEDIATE_CM      (0x14)
#define SCSI_STATUS_RESERVATION_CONFLICT (0x18)
#define SCSI_STATUS_COMMAND_TERMINATED   (0x22)
#define SCSI_STATUS_QUEUE_FULL           (0x28)



/*---------------------------------------------------------------------------*/
