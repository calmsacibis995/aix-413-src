/* @(#)49	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_drvr.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:53  */
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
#define ADAPTER_DATA_SIZ 64

// Miscellaneous definitions
#define DEFAULT_BUS_ID      7       // ID to be used when there are none stored in NVRAM
#define SCSI_DEBUG          1       // Enable (1) or Disable (0) display of debug information code

typedef enum cmd_ops { Adapter_Dsize,
                       TEST_UNIT_ready,
                       REQUEST_sense,
                       Inquiry,
                       DEVICE_Inquiry,
                       START_motor_immed,
                       START_motor,
                       READ_CAPacity,
                       READ_scsi_data,
                       WRITE_scsi_data,
                       OPEN,
                       CLOSE,
                       BUS_RESET,
                       LOCK,
                       UNLOCK,
                       DIAGnose,
                       SEND
                     } CMD_OPS;

typedef struct { struct { unsigned int read : 1;
                        } cntl;
                 uchar    len;
                 uchar   *data_buffer;
                 uint     byte_count;
                 uchar    cmd[12];
               } CMD_DESC, *CMD_DESC_PTR ;

#include "scsi_api.h"   // pull in definitions for configuration table

#ifndef rtc_value
#include <misc_sub.h>
#endif
