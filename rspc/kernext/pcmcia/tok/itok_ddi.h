/* @(#)55       1.2  src/rspc/kernext/pcmcia/tok/itok_ddi.h, pcmciatok, rspc41J, 9511A_all 3/9/95 02:35:59 */
/* @(#)54	1.1  src/rspc/kernext/isa/itok/itok_ddi.h, isatok, rspc41J 5/2/94 16:00:08 */
/*
 * COMPONENT_NAME: PCMCIATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_ITOK_DDS
#define _H_ITOK_DDS

#include <sys/cdli_tokuser.h>

/* This structure is filled in by the config method. */
typedef struct {
  uchar lname[8];               /* Device logical name.             */
  uchar alias[8];               /* Alias name in ASCII characters   */

  uint  bus_type;               /* For i_init use                   */
  uint  bus_id;                 /* For d_init use                   */
  uint  intr_level;             /* Interrupt level (2,3,6 or 7).    */
  uint  intr_priority;          /* For i_init use.                  */
  ulong bios_addr;              /* Bios memory start address.       */
  ulong shared_mem_addr;        /* Shared memory start address.     */
  uint  io_addr;                /* io base 0x0A20 or 0x0A24.        */
  uint  slot;                   /* Not used by dd,used by cfg-method*/

  uint  xmt_que_size;           /* transmit que size                */
  uint  ring_speed;             /* 4 (== 0) or 16 Mbps (== 1).      */
  uint  attn_mac;               /* 1 == receive attn. MAC frames.   */
  uint  beacon_mac;             /* 1 == receive beacon MAC frames   */

  uint  use_alt_addr;           /* If <> 0 use alt_addr as tok-addr.*/
  uchar alt_addr[CTOK_NADR_LENGTH]; /* Possible substitute addr.    */
#ifdef PCMCIATOK
  dev_t devno;                  /* Device Number                    */
  int   logical_socket;         /* Logical Socket                   */
  int   bus_num;         	/* Bus number			    */
#endif
#ifdef PM_SUPPORT
  int   pm_dev_itime;           /* Device idle time for PM          */
  int   pm_dev_stime;           /* Device standby time for PM       */
#endif /* PM_SUPPORT */
} itok_ddi_t;

/* VPD status values. */
#define TOK_VPD_VALID      0x00              /* VPD obtained is valid */
#define TOK_VPD_INVALID    0x02              /* VPD obtained is invalid */
#define TOK_VPD_LENGTH     CTOK_NADR_LENGTH  /* VPD length of  bytes */

/* This structure is used when passing the VPD data via an ioctl. */
typedef struct {
        ulong   status;                 /* status of VPD */
        ulong   vpd_len;                /* length of VPD returned */
                                        /* (may be <= TOK_VPD_LENGTH) */
        uchar   vpd[TOK_VPD_LENGTH];    /* VPD */
} itok_vpd_t;

#endif /* if !_H_ITOK_DDS */
