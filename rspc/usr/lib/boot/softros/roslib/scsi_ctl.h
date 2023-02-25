/* @(#)48	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_ctl.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:51  */
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

extern unsigned int scsi_cmd ( ADAPTER_INFO_PTR adapter_ptr,
                               DEVICE_INFO_PTR  device_ptr,
                               CMD_OPS          op,
                               unsigned char *  data_buffer,
                               unsigned int     byte_count,
                               unsigned int     lba,
                               unsigned int     lba_count,
                               int              retry_count,
                               int              timeout);

extern int xlat_sense(void);

#define  SCSI_RETRY_COUNT    3
#define  ONE_DEVICE          0
#define  FALSE               0
