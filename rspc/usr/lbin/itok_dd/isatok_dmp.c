static char sccsid[] = "@(#)36	1.2  src/rspc/usr/lbin/itok_dd/isatok_dmp.c, isatok, rspc41J, 9516B_all 4/21/95 11:41:05";
/*
 * COMPONENT_NAME: isatok -- IBM ISA Tokenring Device Driver Dump formatter
 *
 * FUNCTIONS: 
 *		dmp_entry
 *		fmt_dd_ctl
 *		fmt_dds
 *		fmt_ndd
 *		fmt_ddi
 *		fmt_wrk
 *		hex_dmp
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

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


dd_ctrl_t  *dmp_dd_ctl;	 /* pointer to a dynamic allocated area */
dds_t      dmp_dds; /* global structure for the dds dump */

#define DMPNDD dmp_dds.ndd
#define NSTATS dmp_dds.ndd.ndd_genstats
#define DMPDDI dmp_dds.ddi
#define DMPWRK dmp_dds.wrk
#define DMPPMGMT dmp_dds.pmh

/*****************************************************************************/
/*
 * NAME:     dmp_entry
 *
 * FUNCTION: IBM ISA Tokenring driver dump formatter.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
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


  /* If the dump entry name is "ddctrl", this is a isaent_dd_ctl structure.*/
  /* We dynamically allocated a piece of memory for this area because the  */
  /* trace table in this structure may have different size depends on the  */
  /* driver has the DEBUG flag turned on or not when it was built.	   */
  if (!strcmp(name, "ddctrl")) {
        if ((dmp_dd_ctl = (dd_ctrl_t *)malloc(len)) == NULL) {
                return(-1);
        }
	if (read(kmem, dmp_dd_ctl, len) != len) {
		return(-1);
	}

	fmt_dd_ctl(len);
        free(dmp_dd_ctl);
	return(0);
  }

  /* If the dump entry name is "DDS", this is a isaent_dds structure.*/
  if (!strcmp(name, "DDS")) {
	if (read(kmem, &dmp_dds, sizeof(dds_t)) != 
		sizeof(dds_t)) {
		return(-1);
	}

	fmt_dds(d_ptr);
	return(0);
  }

  return(0);

}

/*****************************************************************************/
/*
 * NAME:     fmt_dd_ctl
 *
 * FUNCTION: The format routine for the isaent_dd_ctl structure.
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

  printf("\n<< Dump of the dd_ctrl >> :\n");
  printf("cfg_lock: %08x        dd_lock: %08x\n", dmp_dd_ctl->cfg_lock, 
	dmp_dd_ctl->dd_slock);
  printf("p_dev_list: %08x      num_adapters: %08x num_devs: %08x\n", 
	dmp_dd_ctl->p_dds_list, dmp_dd_ctl->num_adapters,
         dmp_dd_ctl->num_devs);
  printf("initialized: %08x     num_opens: %08x\n", 
	dmp_dd_ctl->initialized, dmp_dd_ctl->num_opens);
  
  /* Calculate the trace table length in case the driver was compiled 	*/
  /* with a DEBUG trace table. This calculation will dump the trace   	*/
  /* table according to the dump length, no matter what the driver    	*/
  /* trace table is DEBUG or non-DEBUG version.				*/
  printf("trace - (struct tracetable_t):");
  len -= sizeof(dd_ctrl_t);
  len += sizeof(tracetable_t);
  hex_dmp("", dmp_dd_ctl->tracetable, len);

  printf("cdt - (struct cdt_t):");
  p = (char *)dmp_dd_ctl;
  p += len;
  hex_dmp("", p, sizeof(cdt_t));

}

/*****************************************************************************/
/*
 * NAME:     fmt_dds
 *
 * FUNCTION: The format routine for the dds_t structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_dds(d_ptr)
char *d_ptr;
{
  printf("\n<< Dump of the dds [%08x] >> :\n", d_ptr);
  printf("ihs - (struct intr):");
  hex_dmp("", &dmp_dds.ihs, sizeof(struct intr));

  fmt_ndd();

  printf("next: %08x          seq_number: %08x\n", 
	dmp_dds.next, dmp_dds.seq_number);

  fmt_ddi();

  printf("tx_wdt - (struct watchdog):");
  hex_dmp("", &dmp_dds.xmit_wdt, sizeof(struct watchdog));

  printf("ctl_wdt - (struct watchdog):");
  hex_dmp("", &dmp_dds.cmd_wdt_dds, sizeof(struct watchdog));

  printf("vpd - (struct itok_vpd_t):");
  hex_dmp("", &dmp_dds.vpd, sizeof(itok_vpd_t));

  printf("tokstats - (struct tok_genstats_t):");
  hex_dmp("", &dmp_dds.tokstats, sizeof(tok_genstats_t));

  printf("devstats - (struct tr_ibm_isa_stats_t):");
  hex_dmp("", &dmp_dds.devstats, sizeof(tr_ibm_isa_stats_t));

  fmt_pmgmt();

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
 *      fmt_ddi 
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


  printf("ndd - (struct ndd):\n");
  printf("  ndd_next: %08x    ndd_refcnt: %08x\n", 
    DMPNDD.ndd_next, DMPNDD.ndd_refcnt);
  printf("  ndd_name: %08x    ndd_alias: %08x\n",
    DMPNDD.ndd_name, DMPNDD.ndd_alias);

  printf("  ndd_flags: %08x  (");
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
  printf(")\n");

  printf("  ndd_correlator: %08x\n", DMPNDD.ndd_correlator);
  printf("  ndd_open: %08x    ndd_close: %08x    ndd_output: %08x\n",
    DMPNDD.ndd_open, DMPNDD.ndd_close, DMPNDD.ndd_output);
  printf("  ndd_ctl: %08x     nd_receive: %08x   nd_status: %08x\n", 
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
  hex_dmp("  ", &DMPNDD.ndd_reserved, sizeof(DMPNDD.ndd_reserved));
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
  hex_dmp("    ", &NSTATS.ndd_stat_reserved, sizeof(NSTATS.ndd_stat_reserved));

}

/*****************************************************************************/
/*
 * NAME:     fmt_pmgmt
 *
 * FUNCTION: The format routine for the pm_handle structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      fmt_dds
 *
 * INPUT:
 *      none
 *
 * RETURNS:
 *      none
 */
/*****************************************************************************/
fmt_pmgmt()
{
  printf("pmh - (struct pm_handle):\n");
  printf("  pmh_activity: %08x    pmh_mode: %08x\n",
         DMPPMGMT.activity, DMPPMGMT.mode);
  printf("  pmh_device_idle_time: %08x    pmh_device_standby_time: %08x\n",
         DMPPMGMT.device_idle_time, DMPPMGMT.device_standby_time);
  printf("  pmh_idle_counter: %08x\n", DMPPMGMT.idle_counter);
  printf("  pmh_handler: %08x\n", DMPPMGMT.handler);
  printf("  pmh_private: %08x\n", DMPPMGMT.private);
  printf("  pmh_devno: %08x     pmh_attribute: %08x\n",
         DMPPMGMT.devno, DMPPMGMT.attribute);
  printf("  pmh_reserve: %08x     pmh_pm_version: %08x\n",
         DMPPMGMT.reserve, DMPPMGMT.pm_version);
}

/*****************************************************************************/
/*
 * NAME:     fmt_ddi
 *
 * FUNCTION: The format routine for the isaent_dds structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_ddi()
{
  printf("ddi - (struct ddi_t):\n");
  printf("  lname: %-16s       alias: %-16s\n", DMPDDI.lname, DMPDDI.alias);
  printf("  bus_type: %08x          bus_id: %08x\n", DMPDDI.bus_type,
	DMPDDI.bus_id);
  printf("  intr_level: %08x          intr_priority: %08x\n", DMPDDI.intr_level,
	DMPDDI.intr_priority);
  printf("  bios_addr: %08x           ring_speed: %08x\n", DMPDDI.bios_addr,
	DMPDDI.ring_speed);
  printf("  shr_mem_addr: %08x        attn_mac: %08x\n",
    DMPDDI.shared_mem_addr, DMPDDI.attn_mac);
  printf("  xmt_que_size: %08x      io_addr: %08x\n", DMPDDI.xmt_que_size,
     DMPDDI.io_addr);
  printf("  beacon_mac: %08x       use_alt_addr: %08x\n",
     DMPDDI.beacon_mac, DMPDDI.use_alt_addr);
  printf("  alt_addr:");
  hex_dmp("  ", &DMPDDI.alt_addr, sizeof(DMPDDI.alt_addr));
}

/*****************************************************************************/
/*
 * NAME:     fmt_wrk
 *
 * FUNCTION: The format routine for the isaent_wrk structure.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
fmt_wrk()
{
  printf("wrk - (struct itok_wrk_t):\n");
  printf("device_state:");
  if (dmp_dds.wrk.adap_state == CLOSED_STATE)
        printf(" CLOSED");
  if (dmp_dds.wrk.adap_state == DEAD_STATE)
        printf(" DEAD");
  if (dmp_dds.wrk.adap_state == OPEN_STATE)
        printf(" OPEN");
  if (dmp_dds.wrk.adap_state == NULL_STATE)
        printf(" NULL");
  if (dmp_dds.wrk.adap_state == SUSPEND_STATE)
        printf(" SUSPEND");

  printf("power management:");
  if (DMPWRK.pm_state == CLOSED_STATE)
        printf(" CLOSED");
  if (DMPWRK.pm_state == DEAD_STATE)
        printf(" DEAD");
  if (DMPWRK.pm_state == OPEN_STATE)
        printf(" OPEN");
  if (DMPWRK.pm_state == NULL_STATE)
        printf(" NULL");
  if (DMPWRK.pm_state == SUSPEND_STATE)
        printf(" SUSPEND");

  printf("pm_event : %08x\n", DMPWRK.pm_event);

  printf("\nminor_no: %08x     open_status: %08x\n", dmp_dds.wrk.minor_no,
        dmp_dds.wrk.open_status);
  printf("\nopen_retries: %08x     last_open: %08x\n",
     dmp_dds.wrk.open_retries, dmp_dds.wrk.last_open);
  printf("\nndd_stime: %08x     dev_stime: %08x\n", dmp_dds.wrk.ndd_stime,
        dmp_dds.wrk.dev_stime);
  printf("max_frame_len: %08x     command_to_do: %08x      mac_frame: %08x\n",
        dmp_dds.wrk.max_frame_len, dmp_dds.wrk.command_to_do, 
        dmp_dds.wrk.mac_frame_active);
  printf("cmd_event: %08x\n", dmp_dds.wrk.cmd_event);
  printf("tx_srb_waiting: %08x    tx_arb_waiting: %08x    xmits_queued: %08x\n",
        dmp_dds.wrk.tx_srb_waiting, dmp_dds.wrk.tx_arb_waiting,
         dmp_dds.wrk.xmits_queued);
  printf("srb_waiting: %08x\n", dmp_dds.wrk.srb_waiting);
  printf("intr_action: %08x\n", dmp_dds.wrk.intr_action);
  printf("network_status: %08x\n", dmp_dds.wrk.network_status);

}

/*****************************************************************************/
/*
 * NAME:     hex_dmp
 *
 * FUNCTION: Format a block of data in a hex with ascii format.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 */
/*****************************************************************************/
hex_dmp(pad, adr, len)
char *pad;  /*  a string for output indentation. */
char *adr;  /*  starting address of the data area. */
int len;    /*  length of the data area to be formatted. */

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
