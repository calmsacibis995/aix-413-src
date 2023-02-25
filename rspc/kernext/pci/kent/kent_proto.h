/* @(#)64       1.2  src/rspc/kernext/pci/kent/kent_proto.h, pcient, rspc41J, 9515A_all 4/11/95 21:21:29 */
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_KENTPROTO
#define _H_KENTPROTO

/* -------------------------------------------------------------------- */
/*   Include files for whole driver   */
/* -------------------------------------------------------------------- */
#include "kent_bits.h"
#include "kent_errids.h"
#include "kent_types.h"
#include "kent_macro.h"

extern time_t lbolt;              /* number ticks since last boot */

#ifndef _NO_PROTO
/* -------------------------------------------------------------------- */
/*   Function Prototypes    */
/* -------------------------------------------------------------------- */

/* config entry point */
extern int kent_config( int cmd, struct uio *p_uio);

/* open entry point */
extern int kent_open( ndd_t *p_ndd);

/* close entry point */
extern int kent_close( ndd_t *p_ndd);

/* ctl entry point */
extern int kent_ctl( ndd_t *p_ndd,
  int cmd, /* ctl command */
  caddr_t arg, /* arg for this cmd, (usually struc ptr) */
  int length); /* length of the arg */

/* output entry point */
extern int kent_output( ndd_t *p_ndd,
   struct mbuf *p_mbuf);

/* retry PIO operation */
extern int kent_pio_retry( kent_acs_t *p_acs,
    enum pio_func iofunc,
    void  *p_ioaddr,
    long  ioparam,
    int  cnt);

extern void kent_tx_to ( struct watchdog *p_wdt);
extern struct cdt * kent_cdt_func( int     pass );
extern void kent_cdt_add ( char *p_name, char *ptr, int len);
extern void kent_cdt_del ( char *ptr);

extern void undo_tx ( kent_acs_t *p_acs);
extern int kent_slih (kent_acs_t *p_acs);

extern void kent_logerr ( kent_acs_t *p_acs, ulong  errid, int  line, 
		char  *p_file, int  status1, int  status2, int  status3);

extern void init_acs(kent_acs_t *p_acs);
extern int init_adapter(kent_acs_t *p_acs);
extern void send_packet( kent_acs_t *p_acs, struct mbuf *p_mbuf);
extern void bugout( kent_acs_t *p_acs);
extern void enter_limbo( kent_acs_t *p_acs);
extern void free_services( kent_acs_t *p_acs);
extern void reset_adapter( kent_acs_t *p_acs);
extern void tx_complete(kent_acs_t *p_acs);
extern void free_cdt();
extern void kent_cdt_add(char *p_name, char *ptr, int len);
extern void kent_cdt_del(char *ptr);
extern struct cdt * kent_cdt_func(int pass);
extern void calculate_ladrf( kent_acs_t 	*p_acs);
extern void calculate_flag( kent_acs_t *p_acs, caddr_t addr, uchar *flagno, 
	uchar *flag);
extern int kent_dump( ndd_t *p_ndd, int cmd, caddr_t arg);
extern int kent_dump_output( ndd_t *p_ndd, struct mbuf	*p_mbuf);
extern int kent_pmh( kent_acs_t *p_acs, int ctrl);
#else
extern int kent_config();
extern int kent_open();
extern int kent_close();
extern int kent_ctl();
extern int kent_output();
extern int kent_slih();
extern int kent_pio_retry();
extern void kent_tx_to ();
extern struct cdt * kent_cdt_func();
extern void kent_cdt_add ();
extern void kent_cdt_del ();
extern void kent_logerr ();
extern void init_acs();
extern int init_adapter();
extern void send_packet();
extern void bugout();
extern void enter_limbo();
extern void free_services();
extern void reset_adapter();
extern void tx_complete();
extern void free_cdt();
extern void kent_cdt_add();
extern void kent_cdt_del();
extern struct cdt * kent_cdt_func();
extern void calculate_ladrf();
extern void calculate_flag();
extern int kent_dump();
extern int kent_dump_output();
extern int kent_pmh();
#endif /* end if ! _NO_PROTO */

#endif /* end if ! _H_FDDIPROTO */
