/* @(#)90       1.3  src/rspc/kernext/pcmcia/ent/ient_proto.h, pcmciaent, rspc41J, 9516A_all 4/18/95 00:54:12 */
/* @(#)65       1.2.1.1  src/rspc/kernext/isa/ient/ient_proto.h, isaent, rspc41J 1/6/95 15:46:42 */
/*
 * COMPONENT_NAME: PCMCIAENT - IBM PCMCIA Ethernet Device Driver
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
#ifdef PCMCIAENT
#include <sys/atomic_op.h>
#include <sys/pcmciacs.h>
#include <sys/pcmciacsAix.h>
#include "pcmciaDD.h"
#endif
#ifdef PM_SUPPORT
#include <sys/pm.h>
#include <sys/pmdev.h>
#endif /* PM_SUPPORT */
#include "ient_errids.h"
#include "ient_hw.h"
#include "ient_ddi.h"
#include "ient_mac.h"
#include "ient.h"
#include "ient_dds.h"

#ifdef PCMCIAENT
/* For PCMCIA */
extern int CheckCS(void);
extern int RegisterClient(dds_t *);
extern int Callback(int, int, int, char *, char *, int, char *);
extern int CardInsertion(dds_t *);
extern int CardRemoval(dds_t *);
extern unsigned char CheckCard(int);
extern int CheckTuple(int, int, chk_tpl_pkt_t *);
extern int ReadTuple(int, int, CSGetTplDataPkt *, int);
extern int CheckConfigured(dds_t *);
extern int GetTupleInfo(int, dds_t *);
extern int RequestIO(dds_t *);
extern int RequestIRQ(dds_t *);
extern int RequestConfiguration(dds_t *);
extern int RequestWindow(dds_t *);
extern int AccessConfigurationRegister(dds_t *);
extern int GetMACaddr(dds_t *);
extern int MapMemPage(dds_t *);
extern int ReleaseIO(dds_t *);
extern int ReleaseIRQ(dds_t *);
extern int ReleaseWindow(dds_t *);
extern int ReleaseConfiguration(dds_t *);
extern int DeregisterClient(dds_t *);
extern void pcmcia_activity_lamp(dds_t *, int);
#endif
#ifdef PM_SUPPORT
/* Entry points. */
extern int ent_pm_handler(caddr_t private, int mode);
#endif /* PM_SUPPORT */

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

#endif /* ! _H_IENT_PROTO */
