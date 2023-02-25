static char sccsid[] = "@(#)39  1.1  src/bos/usr/samples/ndd/entsamp_open.c, entsamp, bos411, 9428A410j 1/14/94 13:39:21";
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_open
 *		entsamp_setup
 *		entsamp_tx_setup
 *		entsamp_rv_setup
 *		entsamp_tx_free
 *		entsamp_rv_free
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
 */

#include <stddef.h>
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>

#include "entsamp_mac.h"
#include "entsamp_pio.h"
#include "entsamp_hw.h"
#include "entsamp.h"
#include "entsamp_errids.h"


extern struct cdt *entsamp_cdt_func();
extern entsamp_dd_ctl_t entsamp_dd_ctl;	
extern ethernet_all_mib_t entsamp_mib_status;

void entsamp_tx_free();
void entsamp_rv_free();


/*****************************************************************************/
/*
 * NAME:     entsamp_open
 *
 * FUNCTION: Ethernet driver open routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ns_alloc() service
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *
 * RETURNS:  
 *	0 - OK
 *	ENOCONNECT - unable to connect to network
 *	ENOMEM - unable to allocate required memory
 */
/*****************************************************************************/
entsamp_open(
  ndd_t		*p_ndd)		/* pointer to the ndd in the dev_ctl area */

{
  entsamp_dev_ctl_t   *p_dev_ctl = (entsamp_dev_ctl_t *)(p_ndd->ndd_correlator);
  int rc;




  /* pin the entire driver in memory */
  pincode(entsamp_open);

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "OpnB", (ulong)p_ndd, 0, 0);

  /*
   * Set the device state and NDD flags
   */
  p_dev_ctl->device_state = OPEN_PENDING;
  p_ndd->ndd_flags = NDD_UP | NDD_BROADCAST | NDD_SIMPLEX;
#ifdef DEBUG
  p_ndd->ndd_flags |= NDD_DEBUG;
#endif

  /* save the ndd start time for statistics */
  p_dev_ctl->ndd_stime = lbolt;


  lock_write(&DD_LOCK);
  if ((++entsamp_dd_ctl.open_count) == 1) {
	entsamp_cdt_add("entsamp_dd_ctl", (char *)&entsamp_dd_ctl, 
		sizeof(entsamp_dd_ctl_t));
  	dmp_add(entsamp_cdt_func);
  }
  entsamp_cdt_add("entsamp_dev_ctl", (char *)p_dev_ctl, 
	sizeof(entsamp_dev_ctl_t));
  lock_done(&DD_LOCK);

  /*
   * Activate the adapter and allocate all the resources needed
   */
  if (entsamp_start(p_dev_ctl, FALSE)) {
  	p_dev_ctl->device_state = CLOSED;
  	p_ndd->ndd_flags &= ~NDD_UP;

  	lock_write(&DD_LOCK);
	entsamp_cdt_del("entsamp_dev_ctl", (char *)p_dev_ctl, 
		sizeof(entsamp_dev_ctl_t));
  	if ((--entsamp_dd_ctl.open_count) == 0) {
		entsamp_cdt_del("entsamp_dd_ctl", (char *)&entsamp_dd_ctl, 
		  sizeof(entsamp_dd_ctl_t));
  		dmp_del(entsamp_cdt_func);
	}
  	lock_done(&DD_LOCK);

  	TRACE_SYS(HKWD_ENTSAMP_ERR, "Opn2", ENOCONNECT, 0, 0);
	unpincode(entsamp_open);
	return(ENOCONNECT);
  }
  else {
	if (rc = entsamp_setup(p_dev_ctl, FALSE)) {

		/* undo any resource setup */
		entsamp_cleanup(p_dev_ctl);

  		p_dev_ctl->device_state = CLOSED;
  		p_ndd->ndd_flags &= ~NDD_UP;

  		lock_write(&DD_LOCK);
		entsamp_cdt_del("entsamp_dev_ctl", (char *)p_dev_ctl, 
			sizeof(entsamp_dev_ctl_t));
  		if ((--entsamp_dd_ctl.open_count) == 0) {
			entsamp_cdt_del("entsamp_dd_ctl", 
			  (char *)&entsamp_dd_ctl, sizeof(entsamp_dd_ctl_t));
  			dmp_del(entsamp_cdt_func);
		}
  		lock_done(&DD_LOCK);

  		TRACE_SYS(HKWD_ENTSAMP_ERR, "Opn3", rc, 0, 0);
		unpincode(entsamp_open);
		return(rc);
	}

        /* update the device state and send a status block to demuxer */
	p_dev_ctl->device_state = OPENED;
	p_ndd->ndd_flags |= NDD_RUNNING;

  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "OpnE", 0, 0, 0);
	return(0);
  }


}
/*****************************************************************************/
/*
 * NAME:     entsamp_start
 *
 * FUNCTION: Initialize and activate the adapter.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_open
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the dev_ctl area.
 *
 * RETURNS:
 *      0 - OK
 *      EIO - PIO error occurred during the start
 *      ENOCONNECT - adapter error occurred during the start
 */
/*****************************************************************************/
entsamp_start(
  entsamp_dev_ctl_t      *p_dev_ctl)     /* pointer to the dev_ctl area */

{

  /*
   * Activate the adapter and allocate all the resources needed
   * Perform device-specific operations to actually put the device 
   * into a working state on the network. 
   * The device will be enabled, fully initialized and ready to 
   * accept transmit/receive commands after the entsamp_start() is 
   * completed successfully.
   */

}


/*****************************************************************************/
/*
 * NAME:     entsamp_setup
 *
 * FUNCTION: Setup the system resources for the adapter to operate.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_open
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the dev_ctl area.
 *
 * RETURNS:  
 *	0 - OK
 *	ENOCONNECT - unable to activate and setup the adapter
 *	ENOMEM - unable to allocate required memory
 */
/*****************************************************************************/
entsamp_setup(
  entsamp_dev_ctl_t	*p_dev_ctl)	/* pointer to the dev_ctl area */

{


  int rc;


  TRACE_SYS(HKWD_ENTSAMP_OTHER, "AupB", (ulong)p_dev_ctl, 0, 0);

  if (!WRK.intr_inited) {
	/* add our interrupt routine to kernel's interrupt chain */
	i_init ((struct intr *)(&(IHS)));
	WRK.intr_inited = TRUE;
  }

  if (!WRK.tx_wdt_inited) {
        /* add our watchdog timer routine to kernel's list */
	while (w_init ((struct watchdog *)(&(TXWDT))));
        WRK.tx_wdt_inited = TRUE;
  }
  if (!WRK.ctl_wdt_inited) {
        /* add our watchdog timer routine to kernel's list */
	while (w_init ((struct watchdog *)(&(CTLWDT))));
        WRK.ctl_wdt_inited = TRUE;
  }



  /* clear all ndd_genstats and device stats */
  bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));
  bzero (&ENTSTATS, sizeof(ENTSTATS));
  bzero (&DEVSTATS, sizeof(DEVSTATS));

  WRK.restart_count = 0;
  WRK.promiscuous_count = 0;
  WRK.badframe_count = 0;
  WRK.otherstatus = 0;
  WRK.enable_multi = 0;
  WRK.filter_count = 0;
  WRK.multi_count = 0;


  p_dev_ctl->tx_pending = 0;
  p_dev_ctl->ctl_pending = FALSE;
  

  /*
   * set up DMA channel 
   */
  if (!WRK.channel_alocd) {

	/* get dma channel id by calling d_init                             */
	WRK.dma_channel = d_init(DDS.dma_arbit_lvl,MICRO_CHANNEL_DMA,
                                  DDS.bus_id);

	if (WRK.dma_channel != DMA_FAIL) {
		d_unmask(WRK.dma_channel);

		/* Update the state of the dma_channel  */
         	WRK.channel_alocd = TRUE;
	}
	else {
		entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_DMAFAIL, __LINE__, 
			__FILE__, WRK.dma_channel, 0, 0);
  		TRACE_SYS(HKWD_ENTSAMP_ERR, "Aupb", ENOCONNECT, 
			WRK.dma_channel, 0);
		return(ENOCONNECT);
	}
  } 


  /*
   * Set up for transmit buffer descriptors 
   */
  if (rc = entsamp_tx_setup(p_dev_ctl)) {

      TRACE_SYS(HKWD_ENTSAMP_ERR, "Aupc", rc, 0, 0);
      return(rc);
  }


  /*
   * Set up for receive buffer descriptor
   */
  if (rc = entsamp_rv_setup(p_dev_ctl, FALSE)) {

      TRACE_SYS(HKWD_ENTSAMP_ERR, "Aupd", rc, 0, 0);
      return(rc);
  }

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "AupE", 0, 0, 0);

  return(0);

} 


/*****************************************************************************/
/*
 * NAME:     entsamp_tx_setup
 *
 * FUNCTION: Setup the TCWs, buffers and descriptors for transmit.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_setup
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *
 * RETURNS:  
 *	0 - OK
 *	ENOMEM - unable to allocate required memory
 *	ENOCONNECT - unable to initialize the buffers
 */
/*****************************************************************************/
entsamp_tx_setup(
  entsamp_dev_ctl_t	*p_dev_ctl)  /* pointer to the device control area */

{

	int buf_size;
	int i;
	uchar *tbuf;
	uchar *tdma;
	int bus;
	ushort boffset;
	uchar tchar;
	int pio_rc = 0;




	TRACE_SYS(HKWD_ENTSAMP_OTHER, "AtuB", (ulong)p_dev_ctl, 0, 0);

	/*
	 * allocate space for transmit buffers
	 */

	if (!WRK.tx_buf_alocd) {
		buf_size = WRK.tx_tcw_cnt * DMA_PSIZE / 2;
		WRK.tx_buf = xmalloc(buf_size, DMA_L2PSIZE, pinned_heap);
		if (WRK.tx_buf == NULL)
		{
			entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_NOBUFS, 
				__LINE__, __FILE__, 0, 0, 0);
			TRACE_SYS(HKWD_ENTSAMP_ERR, "Atu1", ENOMEM, 0, 0);
			return(ENOMEM);
		}

		/*
 	 	* attach cross-memory for the tranmit buffers
         	*/
        	WRK.txbuf_xmem.aspace_id = XMEM_INVAL;
        	if (xmattach(WRK.tx_buf, buf_size, &WRK.txbuf_xmem,
                        SYS_ADSPACE) != XMEM_SUCC)
        	{
                	xmfree(WRK.tx_buf, pinned_heap);
			entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_NOBUFS, 
				__LINE__, __FILE__, 0, 0, 0);
			TRACE_SYS(HKWD_ENTSAMP_ERR, "Atu2", ENOMEM, 0, 0);
                	return(ENOMEM);
        	}

	}

	/*
         * Set up the for Transmit list
         * get the offsets for all the transmit buffer descriptors.
	 * initialize the DMA on every transmit buffer.
         */

        bus = (int)BUSMEM_ATT(DDS.bus_id, DDS.bus_mem_addr);

	tbuf = WRK.tx_buf;
	tdma = WRK.tx_tcw_base;
        boffset = WRK.tx_list_off;
	for (i=0; i < WRK.tx_tcw_cnt - 1; i++) {
		WRK.txd[i].next = &WRK.txd[i+1];
		WRK.txd[i].buf = tbuf;
		WRK.txd[i].dma_io = tdma;
		tbuf += DMA_PSIZE / 2;
		tdma += DMA_PSIZE / 2;
                WRK.txd[i].bd_off = boffset;
                ENT_PUTLRX((long *)(bus + boffset + offsetof(BUFDESC, buflo)),
                                WRK.txd[i].dma_io);

		if (!WRK.tx_buf_alocd) {
                	d_master(WRK.dma_channel, DMA_WRITE_ONLY,
                        	WRK.txd[i].buf, DMA_PSIZE/2,
                        	&WRK.txbuf_xmem, (char *)WRK.txd[i].dma_io);
		}
		WRK.txd[i].flags = BDC_INITED;
                ENT_GETSRX(bus + boffset + offsetof(BUFDESC, next), &boffset);
	}
	/* link the first one to the last one to create a circular list */
	WRK.txd[i].next = &WRK.txd[0];
	WRK.txd[i].buf = tbuf;
	WRK.txd[i].dma_io = tdma;
        WRK.txd[i].bd_off = boffset;
        ENT_PUTLRX((long *)(bus + boffset + offsetof(BUFDESC, buflo)),
                                WRK.txd[i].dma_io);
	if (!WRK.tx_buf_alocd) {
        	d_master(WRK.dma_channel, DMA_WRITE_ONLY,
                        WRK.txd[i].buf, DMA_PSIZE/2,
                        &WRK.txbuf_xmem, (char *)WRK.txd[i].dma_io);
	}
	WRK.txd[i].flags = BDC_INITED;

	WRK.txd_last = &WRK.txd[0];
	WRK.txd_first = WRK.txd_avail = &WRK.txd[1];
		
        /* read the last and first txd setup just for sanity check only */
        ENT_GETSRX(bus + boffset + offsetof(BUFDESC, next), &boffset);
        ASSERT(boffset == WRK.tx_list_off);

        ENT_GETCX(bus + WRK.txd[0].bd_off + offsetof(BUFDESC, control),
                &tchar);
        ASSERT(tchar & EL_BIT_MASK);

	WRK.tx_buf_alocd = TRUE;

        BUSMEM_DET(bus);

	if (pio_rc) {
		TRACE_SYS(HKWD_ENTSAMP_ERR, "Atu3", ENOCONNECT, pio_rc, 0);
                return(ENOCONNECT);
        }
		

	TRACE_SYS(HKWD_ENTSAMP_OTHER, "AtuE", 0, 0, 0);
	return(0);

}


/*****************************************************************************/
/*
 * NAME:     entsamp_rv_setup
 *
 * FUNCTION: Setup the TCWs, buffers and descriptors for receive.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_setup
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	at_int_lvl	- flag for from interrupt level
 *
 * RETURNS:  
 *	0 - OK
 *	ENOMEM - unable to allocate required memory
 *	ENOCONNECT - unable to initialize the buffers
 */
/*****************************************************************************/
entsamp_rv_setup(
  entsamp_dev_ctl_t	*p_dev_ctl,  /* pointer to the device control area */
  int	at_int_lvl)		     /* flag for from interrupt level */

{
	int bus, ioa;
	int mbox, i;
	int buf_size;
	ushort boffset;
	uchar *rbuf, *rdma;
	int pio_rc = 0;
	int rc = 0;


	TRACE_SYS(HKWD_ENTSAMP_OTHER, "AruB", (ulong)p_dev_ctl, 0, 0);

	if (!WRK.rv_buf_alocd) {
	  	/*
	   	* allocate space for receive buffers
	   	*/
	  	buf_size = WRK.rv_tcw_cnt * DMA_PSIZE / 2;
	  	WRK.rv_buf = xmalloc(buf_size, DMA_L2PSIZE, pinned_heap);
	  	if (WRK.rv_buf == NULL)
	  	{
		  entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_NOBUFS, __LINE__, 
			__FILE__, 0, 0, 0);
		  TRACE_SYS(HKWD_ENTSAMP_ERR, "Aru1", ENOMEM, 0, 0);
		  return(ENOMEM);
	  	}

		/*
 	 	* attach cross-memory for the receive buffers
         	*/
        	WRK.rvbuf_xmem.aspace_id = XMEM_INVAL;
        	if (xmattach(WRK.rv_buf, buf_size, &WRK.rvbuf_xmem,
                        SYS_ADSPACE) != XMEM_SUCC)
        	{
                  xmfree(WRK.rv_buf, pinned_heap);
		  entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_NOBUFS, __LINE__, 
			__FILE__, ENT_NOBUFS, 0, 0);
		  TRACE_SYS(HKWD_ENTSAMP_ERR, "Aru2", ENOMEM, 0, 0);
                  return(ENOMEM);
        	}

	}
	/*
	 * initialize dma and buffer information
         */
        bus = (int)BUSMEM_ATT((ulong)DDS.bus_id, DDS.bus_mem_addr );
        ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port );

	/*
         * Set up recv buffer descriptor pointers for all the buffer
         * descriptors
         */
	rbuf = WRK.rv_buf;
	rdma = WRK.rv_tcw_base;
        boffset = WRK.rv_list_off;
	for (i = 0; i < WRK.rv_tcw_cnt - 1; i++)
	{
		WRK.rvd[i].next = &WRK.rvd[i+1];
		WRK.rvd[i].buf = rbuf;
		WRK.rvd[i].dma_io = rdma;
		rbuf += DMA_PSIZE / 2;
		rdma += DMA_PSIZE / 2;
                WRK.rvd[i].bd_off = boffset;
                ENT_PUTLRX((long *)(bus + boffset + offsetof(BUFDESC,buflo)),
                                        (long)WRK.rvd[i].dma_io);

		/*
		 * Setup the corresponding receive buffer descriptor 
		 * on the adapter.
		 *
		 * A section of device-specific code was omitted here.
		 */
		if (!WRK.rv_buf_alocd) {
                	d_master(WRK.dma_channel, DMA_READ|DMA_NOHIDE, 
				WRK.rvd[i].buf, DMA_PSIZE/2, &WRK.rvbuf_xmem, 
				WRK.rvd[i].dma_io);
		}
                ENT_PUTCX(bus + boffset + offsetof(BUFDESC,control), 0);
                ENT_PUTCX(bus + boffset + offsetof(BUFDESC, status),  0);
                WRK.rvd[i].flags = BDC_INITED;

                ENT_GETSRX(bus + boffset + offsetof(BUFDESC, next), &boffset);
        }


	/* link the first one to the last one to create a circular list */
	WRK.rvd[i].next = &WRK.rvd[0];
	WRK.rvd[i].buf = rbuf;
	WRK.rvd[i].dma_io = rdma;
        WRK.rvd[i].bd_off = boffset;
        ENT_PUTLRX((long *)(bus + boffset + offsetof(BUFDESC,buflo)),
                                        (long)WRK.rvd[i].dma_io);

	if (!WRK.rv_buf_alocd) {
        	d_master(WRK.dma_channel, DMA_READ|DMA_NOHIDE, WRK.rvd[i].buf,
                         DMA_PSIZE/2, &WRK.rvbuf_xmem, WRK.rvd[i].dma_io);
	}
        ENT_PUTSX(bus + boffset, EL_BIT_MASK);
	WRK.rvd[i].flags = BDC_INITED;

	WRK.rvd_first = &WRK.rvd[0];
	WRK.rvd_last = &WRK.rvd[i];

        /* read the last rvd setup for sanity check only */
        ENT_GETSRX(bus + boffset + offsetof(BUFDESC, next), &boffset);
        ASSERT(boffset == WRK.rv_list_off);

	WRK.rv_buf_alocd = TRUE;

	/*
	 * start the receiver
	 *
	 * A section of device-specific code was omitted here.
	 */


        BUSIO_DET(ioa);
        BUSMEM_DET(bus);

	if (pio_rc) {
		TRACE_SYS(HKWD_ENTSAMP_ERR, "Aru3", ENOCONNECT, pio_rc, 0);
                return(ENOCONNECT);
        }

	TRACE_SYS(HKWD_ENTSAMP_OTHER, "AruE", rc, 0, 0);
	return(rc);
}


/*****************************************************************************/
/*
 * NAME:     entsamp_tx_free
 *
 * FUNCTION: Free up all the resources set up for transmit.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *	entsamp_cleanup
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
entsamp_tx_free(
  entsamp_dev_ctl_t  *p_dev_ctl)   /* pointer to the device control area */

{
        int rc;
	int i;
        


	TRACE_SYS(HKWD_ENTSAMP_OTHER, "AtfB", (ulong)p_dev_ctl, 0, 0);

	/*
         * Complete all the DMA operation.
         */
        for (i=0; i<WRK.tx_tcw_cnt; i++) {

		if (WRK.txd[i].flags & BDC_INITED) {
                  rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
                        WRK.txd[i].buf, DMA_PSIZE/2, 
			&WRK.txbuf_xmem, WRK.txd[i].dma_io);

                  if (rc != DMA_SUCC) {
                        entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_DMAFAIL, 
				__LINE__, __FILE__, WRK.dma_channel, 
				WRK.txd[i].buf, DMA_PSIZE/2);
		  }
		  WRK.txd[i].flags = 0;
		}
        }


	/*
         * free transmit data buffer
         */
        if (WRK.tx_buf) {
                rc = xmdetach((struct xmem *)&WRK.txbuf_xmem);
                assert(rc == 0);
                rc = xmfree(WRK.tx_buf, pinned_heap);
                WRK.tx_buf = NULL;
        }

	WRK.txd_first = WRK.txd_last = WRK.txd_avail = NULL;
	WRK.tx_buf_alocd = FALSE;

	TRACE_SYS(HKWD_ENTSAMP_OTHER, "AtfE", 0, 0, 0);
}

/*****************************************************************************/
/*
 * NAME:     entsamp_rv_free
 *
 * FUNCTION: Free up all the resources for receive descriptors.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *	entsamp_cleanup
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
entsamp_rv_free(
	entsamp_dev_ctl_t  *p_dev_ctl)	/* pointer to device control area */
{
	int i;
	int rc;

	

	TRACE_SYS(HKWD_ENTSAMP_OTHER, "ArfB", (ulong)p_dev_ctl, 0, 0);

	/*
	 * Free up the DMA buffer resources
	 */
	for (i = 0; i < WRK.rv_tcw_cnt; i++) {
		if (WRK.rvd[i].flags & BDC_INITED) {
			/*
			 * Finish the DMA processing of the buffer
			 */
			rc = d_complete (WRK.dma_channel, DMA_READ|DMA_NOHIDE,
				WRK.rvd[i].buf, DMA_PSIZE/2,
				&WRK.rvbuf_xmem, WRK.rvd[i].dma_io);
			if (rc != DMA_SUCC) {
                        	entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_DMAFAIL, 
					__LINE__, __FILE__, WRK.dma_channel, 
					WRK.rvd[i].buf, DMA_PSIZE/2);
			}
			WRK.rvd[i].flags = 0;
		}
	}

	/*
	 * free the receive buffer area if it was allocated
	 */
	if (WRK.rv_buf)
	{
                rc = xmdetach((struct xmem *)&WRK.rvbuf_xmem);
 		assert(rc == 0);
  		rc = xmfree(WRK.rv_buf, pinned_heap);
		assert(rc == 0);
 		WRK.rv_buf = NULL;
	}

	WRK.rvd_first = WRK.rvd_last = NULL;
	WRK.rv_buf_alocd = FALSE;

	TRACE_SYS(HKWD_ENTSAMP_OTHER, "ArfE", 0, 0, 0);

} 

