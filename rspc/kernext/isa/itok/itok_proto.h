/* @(#)58	1.5  src/rspc/kernext/isa/itok/itok_proto.h, isatok, rspc41J, 9516B_all 4/21/95 10:30:47 */
/*
 * COMPONENT_NAME: ISATOK - IBM 16/4 PowerPC Token-ring driver.
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

#ifndef _H_ITOK_PROTO
#define _H_ITOK_PROTO

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
#include <sys/except.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <sys/inline.h>
#include <sys/tokenring_mibs.h>
#include <stddef.h>
#include <net/spl.h>
#include <sys/cdli_tokuser.h>
#include <sys/pm.h>
#include "itok_errids.h"
#include "itok_hw.h"
#include "itok_misc.h"
#include "itok_ddi.h"
#include "itok_dds.h"

extern void config_term (dds_t  *p_dds);

/* add an entry to the component dump table */
extern void tok_add_cdt (char *name,
                         char *ptr,
                         int   len);

/* delete an entry from the component dump table */
extern void tok_del_cdt (char *name,
                         char *ptr,
                         int   len);

/* component dump table function */
extern cdt_t *tok_cdt_func ( );

/* Entry points. */

extern int tokconfig(int        cmd,
                     struct uio *p_uio);

extern int tokopen(ndd_t *p_ndd);

extern int tokclose(ndd_t *p_ndd);

extern int tok_output(ndd_t       *p_ndd,
                      struct mbuf *p_mbuf);

extern int tokioctl(ndd_t   *p_ndd,
                    int     cmd,
                    caddr_t arg,
                    int     length);
extern int tokdump(ndd_t *p_ndd, int cmd, caddr_t arg);

/*
 * Bringup Timer
 */

extern void logerr(dds_t *p_dds,
                   ulong errid);

extern int tok_setup_tx(dds_t  *p_dds);

extern tok_16bit_tx(dds_t          *p_dds,
                    uchar          correlator,
                    ushort         offset,
                    volatile uchar *memaddr);

extern tok_tx(dds_t  *p_dds,
              uchar  correlator,
              ushort offset,
              volatile uchar *memaddr);

extern tok_rcv(dds_t       *p_dds,
            volatile uchar *arb_area,
            volatile uchar *memaddr);

extern int tok_16bit_rcv(dds_t           *p_dds,
                         volatile ushort *arb_area,
                         volatile uchar  *memaddr);

extern void tok_setup_op_params(dds_t *p_dds);

extern void tokopen_cleanup(dds_t *p_dds);

extern void tok_setup_timers(dds_t *p_dds);

extern void tok_remove_timers(dds_t *p_dds);

extern void tok_cmd_timeout(struct watchdog *p_wdt);

extern void tok_srb_timeout(struct watchdog *p_wdt);

extern void tok_ring_startup(struct watchdog *p_wdt);

extern void tok_xmit_timeout(struct watchdog *p_wdt);

extern void tok_get_mibs(dds_t                *p_dds,
                         token_ring_all_mib_t *arg);

extern int tok_get_stats(dds_t *p_dds);

extern int tok_grp_cmd_response(dds_t *p_dds);

extern int tok_func_cmd_response(dds_t *p_dds);

extern void tok_save_trace(register ulong num,
                           register char  str1[],
                           ulong          arg2,          
                           ulong          arg3,         
                           ulong          arg4);

extern int tok_pmgmt(dds_t *p_dds, int ctrl);

#endif /* ! _H_ITOK_PROTO */
