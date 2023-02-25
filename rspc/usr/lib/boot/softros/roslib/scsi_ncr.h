/* @(#)50	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_ncr.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:55  */
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

#ifdef SCSI_NCR
#define ncr_extern
#else
#define ncr_extern extern
#endif

#define NCR_CHIP 810
//AIX// #define USE_POLLING  0  /* using hardware interrupts */
#define USE_POLLING  1  /* using software interrupts */

extern int ncr_cmd(ADAPTER_INFO_PTR adapter_ptr,     // pointer to adapter information record
                   DEVICE_INFO_PTR  device_ptr,      // pointer to device information record
                   CMD_OPS          op,              // command op code
                   unsigned char   *data_ptr,        // command descriptor or data buffer
                   unsigned int     byte_cnt,        // byte count for cmds other than SEND
                   unsigned int     retry_count,     //
                   unsigned int     int_timeout_s);  //


#define NCR_MAX_LUN_ID         7
#define NCR_MAX_PUN_ID         7
#define NCR_MAX_BUS_ID         0
//#define LATENCY_TIMER          0x080
#define LATENCY_TIMER          0x00     // Changed to 0 for pass 4 planar
#define NCR_BASE_ADDR          0x20000000
