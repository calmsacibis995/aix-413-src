/* @(#)53	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_public.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:00  */

/* R2/scsi_public.h, rosipl_601, rosipl_13000 2/23/93 12:12:07                                                              */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 * FUNCTIONS: procname1, procname2, ...
 *
 *   ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
                                                                            */

#define  MIN_SCSI_ID         0
#define  MAX_SCSI_ID         15
#define  MIN_LUN_ID          0
#define  MAX_LUN_ID          32
#define  BLUEB_ADAPT         (1)
#define  INT_SCSI_ADAPT      (2)
#define  HARR_ADAPT          (3)
#define  LACE_ADAPT          (4)
#define  COR_ADAPT           (5)
#define  COR_POS0_ID         COR_REG0_VALUE
#define  COR_POS1_ID         COR_REG1_VALUE
#define  ONE_DEVICE          0
#define  ANY_DEVICE          1
#define  TAPE_ONLY           2
#define  CD_ROM_ONLY         3
#define  DASD_ONLY           4
#define  TRUE                1
#define  IRQ14_EISBID        1
#define  DSK_EISBID          IRQ14_EISBID

#define PCI_TO_SYS_MEM 0x80000000
