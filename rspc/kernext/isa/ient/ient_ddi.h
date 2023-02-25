/* @(#)60       1.1.1.1  src/rspc/kernext/isa/ient/ient_ddi.h, isaent, rspc41B, 412_41B_sync 1/6/95 15:46:32 */
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS: none.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_IENT_DDI
#define _H_IENT_DDI

typedef struct
{
    uchar lname[8];                  /* Device logical name.               */ 
    uchar alias[8];                  /* Alias name in ASCII characters.    */
    uint  bus_type;                  /* For i_init use.                    */
    uint  bus_id;                    /* For i_init and bus io use.         */
    uint  intr_level;                /* Interrupt level (9,3,4,5,7,10,11). */
    uint  intr_priority;             /* For i_init use (INTCLASSx).        */
    ulong shared_mem_addr;           /* bus memory base address.           */
    int   shared_mem_size;           /* size of bus memory region.         */
    int   xmt_que_size;              /* Transmit queue size.               */
    uint  io_addr;                   /* IO base (240,280,2C0,....,360).    */
    int   media_type;                /* Transceiver select.                */
    int   use_alt_addr;              /* non-zero => use alt network addr.  */
    uchar alt_addr[ENT_NADR_LENGTH]; /* alternate network address.         */
} ddi_t;

#define MAX_IEN_ISA_VPD_LEN 128      /* max vpd size for the IBM ISA card. */

/* VPD structure (different from standard micro-channel stuff). */

typedef struct ibm_isa_vpd_t
{
    ulong status;                        /* vpd status value                  */
    ulong length;                        /* number of bytes actually returned */
    uchar vpd[MAX_IEN_ISA_VPD_LEN];      /* vital product data characters     */
} ibm_isa_vpd_t;

/*
 * vpd status codes
 */
#define VPD_NOT_READ   (0) /* the vpd data has not been obtained from adap   */
#define VPD_NOT_AVAIL  (1) /* the vpd data is not available for this adapter */
#define VPD_INVALID    (2) /* the vpd data was obtained but is invalid       */
#define VPD_VALID      (3) /* the vpd data was obtained and is valid         */

#endif /* _H_IENT_DDI */
