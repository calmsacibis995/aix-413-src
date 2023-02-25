/* @(#)13       1.4.1.2  src/rspc/kernext/isa/ient/ient.h, isaent, rspc41J, 9516A_all 4/13/95 16:10:43 */
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS:      none.
 *
 * ORIGINS:        27
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

#ifndef _H_IENT
#define _H_IENT

#define DD_NAME_STR             "ientdd"  /* Driver name for dump func.       */
#define IEN_ISA_MAX_ADAPTERS           3  /* Currently limited by bus slots.  */
#define MAX_MULTI                     10  /* max no. multicast addr's.        */
#define MULTI_TABLE_SLOT              16  /* # of addr. per multicast table.  */
#define RAM_SIZE                  0x4000  /* Adapter RAM size - 16K.          */
#define MEM_ALIGN                      2
#define ENT_HDR_LEN                   14  /* Ethernet header length           */
#define MULTI_BIT_MASK              0x01  /* multicast bit in first addr.byte */
#define MAX_TX_TCW                    64  /* Maximum Transmit tcw's           */
#define MAX_RX_TCW                    64  /* Maximum Receive  tcw's           */
#define IEN_ISA_TRACE_END     0x2A2A2A2A  /* **** is end of trace.            */



/******************************************************************************/
/*                   Trace Table and Component Dump Table                     */
/******************************************************************************/

typedef struct
{
    Simple_lock trace_slock;                  /* lock for the trace table     */
    int         next_entry;                   /* index to put data next       */
    ulong       table[IEN_ISA_TRACE_SIZE];    /* trace table                  */
} tracetable_t;

typedef struct
{
    int    count;                             /* number of cdt entries used   */
    struct cdt_head  head;                    /* cdt head and entries         */
    struct cdt_entry entry[IEN_ISA_CDT_SIZE];
} ient_cdt_t;



/******************************************************************************/
/*                              Adapter States                                */
/******************************************************************************/

#define DEAD_STATE      0xDEAD
#define NULL_STATE      0xFFFF
#define OPEN_PENDING    0xCAFE
#define OPEN_STATE      0xAAAA
#define CLOSED_STATE    0xBBBB
#define CLOSE_PENDING   0xCCCC
#define SUSPEND_STATE   0xDDDD


/******************************************************************************/
/*           Transmit/Receive Buffer Descriptor Control Structure             */
/******************************************************************************/

typedef struct xmt_elem_t
{
    struct xmt_elem_t *next;                  /* point to the next descriptor */
    int               flags;                  /* flag for marking used        */
    struct mbuf       *mbufp;                 /* pointer to mbuf with data    */
    ushort            bytes;                  /* number of bytes in packet    */
    uchar             in_use;                 /* use flag                     */
} xmt_elem_t;


/* xmt_elem_t flags */
#define BDC_BCAST    0x00000001               /* broadcast transmit           */
#define BDC_MCAST    0x00000002               /* multicast transmit           */



/******************************************************************************/
/*                   Multicast Address Registration Table                     */
/******************************************************************************/

typedef struct ient_multi
{
    struct ient_multi *next;                  /* point to the table extension */
    int    ref_count;
    char   m_addr[ENT_NADR_LENGTH];
    char   mar_num;                           /* MiCast Address Reg. (0-7)    */
    char   mar_val;                           /* Value for this MCast address */
} ient_multi_t;

/******************************************************************************/
/*                    Adapter Error Statistics Table.                         */
/******************************************************************************/

typedef struct err_stats
{
    uint nbr_colls;                           /* Nbr of collisions.           */
    uint frame_align;                         /* Nbr of frame alignment errs. */
    uint crc_errs;                            /* CRC errors.                  */
    uint missed_packets;                      /* Missed packets.              */
} err_stats;

#endif /* _H_IENT */
