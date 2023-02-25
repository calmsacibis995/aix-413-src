/* @(#)65       1.2.1.2  src/rspc/kernext/isa/ient/ient_proto.h, isaent, rspc41J, 9516A_all 4/13/95 16:21:26 */
/*
 * COMPONENT_NAME: ISAENT 
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_IENT_PROTO
#define _H_IENT_PROTO

#include <sys/types.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/dump.h>
#include <sys/errno.h>
#include <sys/err_rec.h>
#include <sys/file.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/atomic_op.h>
#include <sys/lock_alloc.h>
#include <sys/lockl.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/sleep.h>
#include <sys/timer.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/watchdog.h>
#include <sys/xmem.h>
#include <sys/trchkid.h>
#include <sys/adspace.h>
#include <sys/listmgr.h>
#include <sys/inline.h>
#include <sys/except.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <sys/ethernet_mibs.h>
#include <stddef.h>
#include <net/spl.h>
#include <sys/cdli_entuser.h>
#include <sys/pm.h>
#include "ient_errids.h"
#include "ient_hw.h"
#include "ient_ddi.h"
#include "ient_mac.h"
#include "ient.h"
#include "ient_dds.h"

extern void config_term (dds_t  *p_dds);

/* Second level interrupt handler. */
int ient_intr(struct intr *p_ihs);

/* add an entry to the component dump table */
extern void ient_add_cdt (char *name, char *ptr, int len);

/* delete an entry from the component dump table */
extern void ient_del_cdt (char *name, char *ptr, int len);

/* component dump table function */
extern struct ient_cdt_t *ient_cdt_func();

/* Entry points. */

extern int ientconfig(int cmd, struct uio *p_uio);

extern int ient_open(ndd_t *p_ndd);

extern int ient_close(ndd_t *p_ndd);

extern int ient_output(ndd_t *p_ndd, struct mbuf *p_mbuf);

extern int ient_ioctl(ndd_t *p_ndd, int cmd, caddr_t arg, int length);

extern int ientdump(ndd_t *p_ndd, int cmd, caddr_t arg);

extern void logerr(dds_t *p_dds, ulong errid);

extern ient_tx_intr(dds_t *p_dds, volatile uchar *ioaddr);

extern ient_rcv(dds_t *p_dds, volatile uchar *ioaddr);

extern void ient_setup_timers(dds_t *p_dds);

extern void ient_remove_timers(dds_t *p_dds);

extern void ient_cmd_timeout(struct watchdog *p_wdt);

extern void ient_tx_timeout(struct watchdog *p_wdt);

extern void ient_get_mibs(dds_t *p_dds, ethernet_all_mib_t *arg);

extern void ient_get_stats(dds_t *p_dds);

extern int ient_grp_cmd_response(dds_t *p_dds);

extern int ient_func_cmd_response(dds_t *p_dds);

extern int ient_start_NIC(dds_t *p_dds, volatile uchar *ioaddr);

extern int ient_stop_NIC(dds_t *p_dds, volatile uchar *ioaddr);
                      
extern void ient_incr_bnry(dds_t *p_dds, rcv_hdr_t *rcv_hdr,
                           volatile uchar *ioaddr);

extern void ient_trace(ulong num, char str1[], ulong arg1, ulong arg2,
                       ulong arg3);

extern int ient_config(int cmd, struct uio *p_uio);

extern int ient_pmgmt(dds_t *p_dds, int ctrl);

extern int ient_init(dds_t  *p_dds);

#endif /* ! _H_IENT_PROTO */
