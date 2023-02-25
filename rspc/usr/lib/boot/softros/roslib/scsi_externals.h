/* @(#)51	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_externals.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:56  */
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

// SCSI Debug Flag
extern int sdf;
#ifdef trace
// SCSI Bus trace information flag
extern int stf;
extern int shf;
#endif
// SCSI burst transfer length
extern unsigned char xfer_length;
// Cache flush control
//extern unsigned int flush_it;

#ifdef SCSI_CTL
#define ctl_extern
#else
#define ctl_extern extern
#endif

#pragma options align=packed
typedef struct { unsigned char error;
                 unsigned char segment;
                 unsigned char key;
                 unsigned int info;
                 unsigned char len;
                 unsigned int cmd_info;
                 unsigned char code;
                 unsigned char qualifier;
                 unsigned char FRU;
                 unsigned char key_specific[3];
               } SENSE;

#pragma options align=reset

ctl_extern SENSE sense;

#define NUM_SENSE_BYTES (sizeof sense)
