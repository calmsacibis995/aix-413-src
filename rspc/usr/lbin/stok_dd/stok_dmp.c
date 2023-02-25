static char sccsid[] = "@(#)33  1.2  src/rspc/usr/lbin/stok_dd/stok_dmp.c, pcitok, rspc41J, 9516A_all 4/14/95 05:13:23";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: dmp_entry
 *		fmt_dd_ctl
 *		fmt_dds
 *		fmt_dev_ctl
 *		fmt_ndd
 *		fmt_wrk
 *		hex_dmp
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/intr.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/adspace.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>
#include <sys/cdli_tokuser.h>

#include "stok_dslo.h"
#include "stok_mac.h"
#include "stok_dds.h"
#include "stok_dd.h"

stok_dd_ctl_t *dmp_dd_ctl;	/* pointer to a dynamic allocated area */
stok_dev_ctl_t dmp_dev_ctl;	/* global structure for the dev_ctl dump */

#define DMPNDD dmp_dev_ctl.ndd
#define NSTATS dmp_dev_ctl.ndd.ndd_genstats
#define DMPDDS dmp_dev_ctl.dds
#define DMPTOK dmp_dev_ctl.tokstats
#define DMPDEV dmp_dev_ctl.devstats
#define DMPWRK dmp_dev_ctl.wrk

/*****************************************************************************/
/*
 * NAME:     dmp_entry
 *
 * FUNCTION: Wildwood Token Ring driver dump formatter.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      main 
 *
 * INPUT:
 *      kmem		- the fd of the dump file. 
 *	p_cdt_entry	- point to the cdt entry saved in the global table.
 *
 * RETURNS:  
 *	0 - OK
 *	-1 - Can't read the dump file
 */
/*****************************************************************************/
dmp_entry(kmem, p_cdt_entry)
int kmem;
struct cdt_entry *p_cdt_entry;

{

  char          *d_ptr;
  char          *name;
  int           len;

  /*
   * get values from the cdt_entry
   */
  name  = p_cdt_entry->d_name;
  len   = p_cdt_entry->d_len;
  d_ptr = p_cdt_entry->d_ptr;


  printf("Enter of dmp_entry : name : %s\n",name);
  /* If the dump entry name is "dd_ctl", this is a stok_dd_ctl structure.  */
  /* We dynamically allocated a piece of memory for this area because the  */
  /* trace table in this structure may have different size depends on the  */
  /* driver has the DEBUG flag turned on or not when it was built.	   */
  if (!strcmp(name, "dd_ctl")) {
        if ((dmp_dd_ctl = (stok_dd_ctl_t *)malloc(len)) == NULL) {
                return(-1);
        }
	if (read(kmem, dmp_dd_ctl, len) != len) {
		return(-1);
	}

	fmt_dd_ctl(len);
        free(dmp_dd_ctl);
	return(0);
  }

  /* If the dump entry name is "dev_ctl", this is a stok_dev_ctl structure. */
  if (!strcmp(name, "dev_ctl")) {
	if (read(kmem, &dmp_dev_ctl, sizeof(dmp_dev_ctl)) != 
		sizeof(dmp_dev_ctl)) {
		return(-1);
	}

	fmt_dev_ctl(d_ptr);
	return(0);
  }

  return(0);

}
/*****************************************************************************/
/*
 * NAME:     fmt_dd_ctl
 *
 * FUNCTION: The format routine for the stok_dd_ctl structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dmp_entry
 *
 * INPUT:
 *	len	- length of the dumped area.
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_dd_ctl(len)
int len;

{
	
  char *p;

  printf("\n<< Dump of stok_dd_ctl >> :\n");
  printf("cfg_lock: %08x        dd_lock: %08x\n", 
	  dmp_dd_ctl->cfg_lock, dmp_dd_ctl->dd_clock);
  printf("p_dev_list: %08x      num_dev: %08x     open_count: %08x\n", 
	  dmp_dd_ctl->p_dev_list, dmp_dd_ctl->num_devs, dmp_dd_ctl->open_count);
  
  /* Calculate the trace table length in case the driver was compiled 	*/
  /* with a DEBUG trace table. This calculation will dump the trace   	*/
  /* table according to the dump length, no matter what the driver    	*/
  /* trace table is DEBUG or non-DEBUG version.				*/
  /* (also subtract 8 for the other two fields in stok_trace_t)          */
  printf("\ntrace - (struct stok_trace):");
  printf("\n  trace_slock: %08x   next_entry: %08x\n",
        dmp_dd_ctl->trace.trace_slock, dmp_dd_ctl->trace.next_entry);
  len = (len - (sizeof(stok_dd_ctl_t) - sizeof(stok_trace_t)) - 8);
  hex_dmp("", &dmp_dd_ctl->trace.table, len);

  printf("\ncdt - (struct stok_cdt):");
  hex_dmp("", dmp_dd_ctl->cdt, sizeof(stok_cdt_t));

}

/*****************************************************************************/
/*
 * NAME:     fmt_dev_ctl
 *
 * FUNCTION: The format routine for the en3com_dev_ctl structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dmp_entry
 *
 * INPUT:
 *	d_ptr	- real memory address of the dumped entry.
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_dev_ctl(d_ptr)
char *d_ptr;

{
	
  printf("\n<< Dump of stok_dev_ctl [%08x] >> :\n", d_ptr);
  printf("ihs - (struct intr):");
  hex_dmp("", &dmp_dev_ctl.ihs, sizeof(struct intr));

  fmt_ndd();

  printf("    next: %08x          seq_number: %08x\n", 
	dmp_dev_ctl.next, dmp_dev_ctl.seq_number);
  printf("    cmd_slock: %08x    tx_slock: %08x      slih_slock: %08x\n", 
	dmp_dev_ctl.cmd_slock, dmp_dev_ctl.tx_slock, dmp_dev_ctl.slih_slock);

  printf("    device_state:");
  if (dmp_dev_ctl.device_state == CLOSED)
	printf(" CLOSED");
  if (dmp_dev_ctl.device_state == SUSPEND_STATE)
	printf(" SUSPEND_STATE");
  if (dmp_dev_ctl.device_state == DEAD)
	printf(" DEAD");
  if (dmp_dev_ctl.device_state == LIMBO)
	printf(" LIMBO");
  if (dmp_dev_ctl.device_state == OPEN_PENDING)
	printf(" OPEN_PENDING");
  if (dmp_dev_ctl.device_state == OPENED)
	printf(" OPENED");
  if (dmp_dev_ctl.device_state == CLOSE_PENDING)
	printf(" CLOSE_PENDING");

  printf("\n    pm_state:");
  if (dmp_dev_ctl.pm_state == CLOSED)
	printf(" CLOSED");
  if (dmp_dev_ctl.pm_state == SUSPEND_STATE)
	printf(" SUSPEND_STATE");
  if (dmp_dev_ctl.pm_state == DEAD)
	printf(" DEAD");
  if (dmp_dev_ctl.pm_state == LIMBO)
	printf(" LIMBO");
  if (dmp_dev_ctl.pm_state == OPEN_PENDING)
	printf(" OPEN_PENDING");
  if (dmp_dev_ctl.pm_state == OPENED)
	printf(" OPENED");
  if (dmp_dev_ctl.pm_state == CLOSE_PENDING)
	printf(" CLOSE_PENDING");

  printf("/n    vpd :");
  hex_dmp("      ", dmp_dev_ctl.vpd, MAX_STOK_VPD_LEN);

  fmt_dds();

  printf("  tx_wdt - (struct watchdog):");
  hex_dmp("    ", &dmp_dev_ctl.tx_wdt, sizeof(struct watchdog));

  printf("  hwe_wdt - (struct watchdog):");
  hex_dmp("    ", &dmp_dev_ctl.hwe_wdt, sizeof(struct watchdog));

  printf("  lim_wdt - (struct watchdog):");
  hex_dmp("    ", &dmp_dev_ctl.lim_wdt, sizeof(struct watchdog));

  printf("  lan_wdt - (struct watchdog):");
  hex_dmp("    ", &dmp_dev_ctl.lan_wdt, sizeof(struct watchdog));

  printf("\n");
  printf("tokstats - (struct tok_genstats):\n");
  printf("  device_type: %08x      dev_elapsed_time: %08x\n",
    DMPTOK.device_type, DMPTOK.dev_elapsed_time);
  printf("  ndd_flags: %08x\n", DMPTOK.ndd_flags);
  printf("  tok_nadr:");
  hex_dmp("    ", DMPTOK.tok_nadr, CTOK_NADR_LENGTH);
  printf("  mcast_xmit: %08x       bcast_xmit: %08x\n",
    DMPTOK.mcast_xmit, DMPTOK.bcast_xmit);
  printf("  mcast_recv: %08x       bcast_recv: %08x\n",
    DMPTOK.mcast_recv, DMPTOK.bcast_recv);
  printf("  line_errs: %08x        burst_errs: %08x\n",
    DMPTOK.line_errs, DMPTOK.burst_errs);
  printf("  ac_errs: %08x          abort_errs: %08x\n",
    DMPTOK.ac_errs, DMPTOK.abort_errs);
  printf("  int_errs: %08x         lostframes: %08x\n",
    DMPTOK.int_errs, DMPTOK.lostframes);
  printf("  rx_congestion: %08x    framecopies: %08x\n",
    DMPTOK.rx_congestion, DMPTOK.framecopies);
  printf("  token_errs: %08x       soft_errs: %08x\n",
    DMPTOK.token_errs, DMPTOK.soft_errs);
  printf("  hard_errs: %08x        signal_loss: %08x\n",
    DMPTOK.hard_errs, DMPTOK.signal_loss);
  printf("  tx_beacons: %08x       recoverys: %08x\n",
    DMPTOK.tx_beacons, DMPTOK.recoverys);
  printf("  lobewires: %08x        removes: %08x\n",
    DMPTOK.lobewires, DMPTOK.removes);
  printf("  singles: %08x          freq_errs: %08x\n",
    DMPTOK.singles, DMPTOK.freq_errs);
  printf("  tx_timeouts: %08x      sw_txq_len: %08x\n",
    DMPTOK.tx_timeouts, DMPTOK.sw_txq_len);
  printf("  hw_txq_len: %08x       reserve1: %08x\n",
    DMPTOK.hw_txq_len, DMPTOK.reserve1);
  printf("  reserve2: %08x         reserve3: %08x\n",
    DMPTOK.reserve2, DMPTOK.reserve3);
  printf("  reserve4: %08x\n", DMPTOK.reserve4);

  printf("\n");
  printf("devstats - (struct tr_stok_stats):\n");
  printf("  Receive_Overrun: %08x   Transmit_Underrun: %08x\n",
    DMPDEV.recv_overrun, DMPDEV.xmit_underrun);

  printf("\n");
  printf("mibs - (struct token_ring_all_mib):");
  hex_dmp("  ", &dmp_dev_ctl.mibs, sizeof(token_ring_all_mib_t));

  fmt_wrk();

}

/*****************************************************************************/
/*
 * NAME:     fmt_ndd
 *
 * FUNCTION: The format routine for the ndd structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dev_ctl 
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_ndd()
{

  printf("\n");
  printf("ndd - (struct ndd):\n");
  printf("  ndd_next: %08x    ndd_refcnt: %08x\n",
    DMPNDD.ndd_next, DMPNDD.ndd_refcnt);
  printf("  ndd_name: %08x    ndd_alias: %08x\n",
    DMPNDD.ndd_name, DMPNDD.ndd_alias);

  printf("  ndd_flags: %08x  (", DMPNDD.ndd_flags);
  if (DMPNDD.ndd_flags & NDD_UP)
        printf(" UP");
  if (DMPNDD.ndd_flags & NDD_BROADCAST)
        printf(" BROADCAST");
  if (DMPNDD.ndd_flags & NDD_DEBUG)
        printf(" DEBUG");
  if (DMPNDD.ndd_flags & NDD_RUNNING)
        printf(" RUNNING");
  if (DMPNDD.ndd_flags & NDD_SIMPLEX)
        printf(" SIMPLEX");
  if (DMPNDD.ndd_flags & NDD_DEAD)
        printf(" DEAD");
  if (DMPNDD.ndd_flags & NDD_LIMBO)
        printf(" LIMBO");
  if (DMPNDD.ndd_flags & NDD_PROMISC)
        printf(" PROMISC");
  if (DMPNDD.ndd_flags & NDD_ALTADDRS)
        printf(" ALTADDRS");
  if (DMPNDD.ndd_flags & NDD_MULTICAST)
        printf(" MULTICAST");
  if (DMPNDD.ndd_flags & NDD_DETACHED)
        printf(" DETACHED");
  if (DMPNDD.ndd_flags & TOK_ATTENTION_MAC)
        printf(" TOK_ATTENTION_MAC");
  if (DMPNDD.ndd_flags & TOK_BEACON_MAC)
        printf(" TOK_BEACON_MAC");
  if (DMPNDD.ndd_flags & TOK_RECEIVE_FUNC)
        printf(" TOK_RECEIVE_FUNC");
  if (DMPNDD.ndd_flags & TOK_RECEIVE_GROUP)
        printf(" TOK_RECEIVE_GROUP");
  if (DMPNDD.ndd_flags & TOK_RING_SPEED_4)
        printf(" TOK_RING_SPEED_4");
  if (DMPNDD.ndd_flags & TOK_RING_SPEED_16)
        printf(" TOK_RING_SPEED_16");
  printf(")\n");

  printf("  ndd_correlator: %08x\n", DMPNDD.ndd_correlator);
  printf("  ndd_open: %08x       ndd_close: %08x    ndd_output: %08x\n",
    DMPNDD.ndd_open, DMPNDD.ndd_close, DMPNDD.ndd_output);
  printf("  ndd_ctl: %08x        nd_receive: %08x   nd_status: %08x\n",
    DMPNDD.ndd_ctl, DMPNDD.nd_receive, DMPNDD.nd_status);
  printf("  ndd_mtu: %08x        ndd_mintu: %08x\n", DMPNDD.ndd_mtu,
    DMPNDD.ndd_mintu);
  printf("  ndd_type: %08x       ndd_addrlen: %08x\n",
    DMPNDD.ndd_type, DMPNDD.ndd_addrlen);
  printf("  ndd_hdrlen: %08x     ndd_physaddr: %08x\n",
    DMPNDD.ndd_hdrlen, DMPNDD.ndd_physaddr);
  printf("  ndd_demuxer: %08x    ndd_nsdemux: %08x\n",
    DMPNDD.ndd_demuxer, DMPNDD.ndd_nsdemux);
  printf("  ndd_specdemux: %08x  ndd_demuxsource: %08x  ndd_demux_lock: %08x\n",
    DMPNDD.ndd_specdemux, DMPNDD.ndd_demuxsource, DMPNDD.ndd_demux_lock);
  printf("  ndd_trace: %08x      ndd_trace_arg: %08x    ndd_lock: %08x\n",
    DMPNDD.ndd_trace, DMPNDD.ndd_trace_arg, DMPNDD.ndd_lock);
  printf("  ndd_reserved:");
  hex_dmp("    ", &DMPNDD.ndd_reserved, sizeof(DMPNDD.ndd_reserved));
  printf("  ndd_specstats: %08x    ndd_speclen: %08x\n",
    DMPNDD.ndd_specstats, DMPNDD.ndd_speclen);

  printf("  ndd_genstats - (struct ndd_genstats):\n");
  printf("    elapsed_time: %08x\n", NSTATS.ndd_elapsed_time);
  printf("    ipackets_msw: %08x    ipackets_lsw: %08x\n",
    NSTATS.ndd_ipackets_msw, NSTATS.ndd_ipackets_lsw);
  printf("    ibytes_msw: %08x      ibytes_lsw: %08x\n",
    NSTATS.ndd_ibytes_msw, NSTATS.ndd_ibytes_lsw);
  printf("    recvintr_msw: %08x    recvintr_lsw: %08x    ierrors: %08x\n",
    NSTATS.ndd_recvintr_msw, NSTATS.ndd_recvintr_lsw, NSTATS.ndd_ierrors);
  printf("    opackets_msw: %08x    opackets_lsw: %08x\n",
    NSTATS.ndd_opackets_msw, NSTATS.ndd_opackets_lsw);
  printf("    obytes_msw: %08x      obytes_lsw: %08x\n",
    NSTATS.ndd_obytes_msw, NSTATS.ndd_obytes_lsw);
  printf("    xmitintr_msw: %08x    xmitintr_lsw: %08x    oerrors: %08x\n",
    NSTATS.ndd_xmitintr_msw, NSTATS.ndd_xmitintr_lsw, NSTATS.ndd_oerrors);
  printf("    nobufs: %08x          xmitque_max: %08x     xmitque_ovf: %08x\n",
    NSTATS.ndd_nobufs, NSTATS.ndd_xmitque_max, NSTATS.ndd_xmitque_ovf);
  printf("    ipackets_drop: %08x   ibadpackets: %08x\n",
    NSTATS.ndd_ipackets_drop, NSTATS.ndd_ibadpackets);
  printf("    opackets_drop: %08x   xmitque_cur: %08x\n",
    NSTATS.ndd_opackets_drop, NSTATS.ndd_xmitque_cur);
  printf("    stat_reserved:");
  hex_dmp("      ", &NSTATS.ndd_stat_reserved,
        sizeof(NSTATS.ndd_stat_reserved));
}

/*****************************************************************************/
/*
 * NAME:     fmt_dds
 *
 * FUNCTION: The format routine for the stok_dds structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dev_ctl 
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_dds()
{
  printf("\ndds - (struct stok_dds):\n");
  printf("  busid: %08x          busintr: %08x\n", DMPDDS.busid,DMPDDS.busintr);
  printf("  busio: %08x		md_sla: %08x\n", DMPDDS.busio, DMPDDS.md_sla);
  printf("  lname: %-16s     alias: %-16s\n", DMPDDS.lname, DMPDDS.alias);
  printf("  xmt_que_size: %08x\n", DMPDDS.xmt_que_sz);
  printf("  rx_que_size: %08x\n", DMPDDS.rx_que_sz);
  printf("  pri_xmt_que_size: %08x\n", DMPDDS.pri_xmt_que_sz);
  printf("  ring_speed: %08x\n", DMPDDS.ring_speed);

  printf("  use_alt_addr: %08x\n", DMPDDS.use_alt_addr);
  printf("  alt_addr:");
  hex_dmp("  ", &DMPDDS.alt_addr, sizeof(DMPDDS.alt_addr));
  printf("  attn_mac : %08x        beacon_mac : %08x	priority_tx : %08x\n\n",
     		DMPDDS.attn_mac, DMPDDS.beacon_mac, DMPDDS.priority_tx);

}

/*****************************************************************************/
/*
 * NAME:     fmt_wrk
 *
 * FUNCTION: The format routine for the en3com_wrk structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dev_ctl 
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_wrk()
{

  int i;

  printf("\n\nwrk - (struct stok_wrk):\n");
  printf("  ctl_event: %08x pmh_event: %08x",DMPWRK.ctl_event,DMPWRK.pmh_event);
  printf("  promiscuous_count: %08x\n", DMPWRK.promiscuous_count);
  printf("  retcode:");
  hex_dmp("   ", &DMPWRK.retcode, sizeof(DMPWRK.retcode));

  printf("\n  adapter_log:");
  hex_dmp("  ", &DMPWRK.adap_log, sizeof(tok_adapter_log));

  /* Address of LAP register */
  printf("\n  asb_address: %08x\n", DMPWRK.asb_address);
  printf("  srb_address: %08x\n", DMPWRK.srb_address);
  printf("  arb_address: %08x\n", DMPWRK.arb_address);
  printf("  trb_address: %08x\n", DMPWRK.trb_address);
  printf("  parms_address: %08x\n", DMPWRK.parms_addr);

  printf("  dma_channel: %08x\n", DMPWRK.dma_channel);

  printf("\n  tx1 channel ********************************************\n");
  fmt_tx(DMPWRK.tx1);

  printf("\n  tx2 channel ********************************************\n");
  fmt_tx(DMPWRK.tx2);

  printf("  stok_addr:");
  hex_dmp("    ", DMPWRK.stok_addr, sizeof(DMPWRK.stok_addr));

  /*=========================*/
  /* information for receive */
  /*=========================*/
  printf("  rx_desc_blk: %08x\n", DMPWRK.rx_desc_blk);
  for (i = 0; i < DMPDDS.rx_que_sz; i++) {
  	printf("\n  rx_dma_desc_p[%d] : %08x  rx_desc_p[%d] : ", 
			i, DMPWRK.rx_dma_desc_p[i], i);
  	hex_dmp("    ", DMPWRK.rx_desc_p[i], 16);
  }

  printf("  read_index: %08x\n", DMPWRK.read_index);
  for (i = 0; i < DMPDDS.rx_que_sz; i++) {
  	printf("\n  rx_addr[%d] : %08x  rx_mbuf[%d] : %08x", 
			i, DMPWRK.rx_addr[i], i, DMPWRK.rx_mbuf[i]);
  }
  printf("\n  eof  : %08x  mhead: %08x  mtail: %08x\n", 
  		DMPWRK.eof, DMPWRK.mhead, DMPWRK.mtail);

  printf("  dump_started: %08x\n", DMPWRK.dump_started);
  printf("  dump_MISR: %08x\n", DMPWRK.dump_MISR);

  printf("  multi_count: %08x\n", DMPWRK.multi_count);
  printf("  multi_table: %08x\n", DMPWRK.multi_table);
  printf("  multi_last: %08x\n", DMPWRK.multi_last);
  printf("  func_ref_cnt:");
  hex_dmp("  ", DMPWRK.func_ref_cnt, sizeof(DMPWRK.func_ref_cnt));
  
  printf("\n  ndd_stime: %08x\n", DMPWRK.ndd_stime);
  printf("  dev_stime: %08x\n", DMPWRK.dev_stime);
  printf("  pio_rc: %08x\n", DMPWRK.pio_rc);
  printf("  re_limbo: %08x\n", DMPWRK.re_limbo);
  printf("  limbo_err: %08x\n", DMPWRK.limbo_err);
  printf("  hard_err: %08x\n", DMPWRK.hard_err);
  printf("%4 io_map: ");
  hex_dmp("         ", &DMPWRK.iomap, sizeof(struct io_map));
  printf("\n");
  printf("%4 pm_handle: ");
  hex_dmp("         ", &DMPWRK.pmh, sizeof(struct pm_handle));
  printf("\n");


}

/*****************************************************************************/
/*
 * NAME:     fmt_tx
 *
 * FUNCTION: The format routine for the Tx structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *	fmt_wrk
 *
 * INPUT:
 *	wrk	- pointer to the tx structure 
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_tx(tx_t wrk) {
  int i;

  printf("  sw_txq_len: %08x    sw_txq_first: %08x    sw_txq_last: %08x\n", 
	wrk.sw_txq_len, wrk.sw_txq_first, wrk.sw_txq_last);

  printf("\n  tx_desc_blk: %08x\n", wrk.tx_desc_blk);
  for (i = 0; i < MAX_TX_LIST; i++) {
  	printf("\n  tx_dma_desc_p[%d]: %08x  tx_desc_p[%d]: ", 
		i, wrk.tx_dma_desc_p[i], i);
  	hex_dmp("    ", wrk.tx_desc_p[i], 20); 
  }
  printf("  hw_txq_first: %08x    hw_txq_last: %08x\n", 
		wrk.hw_txq_in, wrk.hw_txq_out);

  printf("\n  tx_elem_next_in: %08x  tx_elem_next_out: %08x   tx_frame_pending: %08x\n",
     wrk.tx_elem_next_in, wrk.tx_elem_next_out, wrk.tx_frame_pending);

  printf("\n  tx_buf_next_in: %08x  tx_buf_use_count: %08x\n", 
		wrk.tx_buf_next_in, wrk.tx_buf_use_count);

  for (i = 0; i < MAX_BUF_LIST; i++) {
  	printf("  tx_buf[%d]: %08x   tx_addr[%d]: %08x\n", 
		i, wrk.tx_buf[i], i, wrk.tx_addr[i]);
  }

  printf("\n  tx_dma_next_in: %08x  tx_dma_next_out: %08x   tx_dma_use_count: %08x\n",
     wrk.tx_dma_next_in, wrk.tx_dma_next_out, wrk.tx_dma_use_count);

  for (i = 0; i < MAX_DMA_LIST; i++) {
  	printf("  tx_dma_addr[%d]: %08x\n",  i, wrk.tx_dma_addr[i]);
  }
  printf("\n  max_buf_list: %08x\n",  i, wrk.max_buf_list);
  printf("  lfda: %08x  disable: %08x   r_disable: %08x\n\n",
     		wrk.lfda, wrk.disable, wrk.r_disable);
 }

/*****************************************************************************/
/*
 * NAME:     hex_dmp
 *
 * FUNCTION: Format a block of data in a hex with ascii format.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dd_ctl 
 *	fmt_dev_ctl
 *	fmt_ndd
 *	fmt_dds
 *	fmt_wrk
 *
 * INPUT:
 *	pad	- a string for output indentation 
 *	adr	- starting address of the data area
 *	len	- length of the data area to be formatted
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
hex_dmp(pad, adr, len)
char *pad;
char *adr;
int len;

{
  
  int i, j, offset;
  char *dp;
  ulong *lp;
  char buf[20];



  printf("\n");

  offset = 0;
  dp = adr;
  buf[0] = '|';

  /*
   * Format 16 bytes of data per line with its hex offset in the front.
   * The buf is used as the ascii string buffer for the ascii display 
   * on the right.
   */
  while (len >= 16) {
	j = 1;		/* index for buf */
  	for (i = 1; i <= 16; i++) {
		if (*dp >= 0x20 && *dp <= 0x7e) 
			buf[j++] = *dp;
		else
			buf[j++] = '.';
		dp++;
	}
	buf[j++] = '|';
	buf[j] = '\0';

	lp = (ulong *)adr;
	printf("%s%08x: %08x %08x %08x %08x    %s\n", pad, offset, *lp, 
		*(lp + 1), *(lp + 2), *(lp + 3), buf);
		
	len -= 16;
	offset += 16;
	adr += 16;
  }
  
  /*
   * Format the last line with less than 16 bytes of data.
   */
  if (len) {
  	j = 1;		/* index for buf */
  	lp = (ulong *)adr;
  	printf("%s%08x: ", pad, offset);
  	for (i = 1; i <= len; i++) {
		if (*dp >= 0x20 && *dp <= 0x7e)  
			buf[j++] = *dp;
		else
			buf[j++] = '.';
		printf("%02x", *dp);
		dp++;
		if (i % 4 == 0)
			printf(" ");
  	}

  	for (; i < 16; i++) {
  		printf("  ");
		buf[j++] = ' ';
		if (i % 4 == 0)
			printf(" ");
	}
  	buf[j++] = ' ';
  	buf[j++] = '|';
  	buf[j] = '\0';

  	printf("      ");
  	printf("%s\n", buf);

  }
}
