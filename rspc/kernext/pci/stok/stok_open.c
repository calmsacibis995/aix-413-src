static char sccsid[] = "@(#)51  1.2  src/rspc/kernext/pci/stok/stok_open.c, pcitok, rspc41J, 9515A_all 4/11/95 22:04:12";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: stok_act
 *		stok_open
 *		stok_open_adapter
 *		stok_rx_enable
 *		stok_rx_setup
 *		stok_rx_setup_undo
 *		stok_tx_setup
 *		stok_tx_setup_undo
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stddef.h>
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/mdio.h>

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include "stok_dslo.h"
#include "stok_mac.h"
#include "stok_dds.h"
#include "stok_dd.h"
#include "tr_stok_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

extern int stok_config();
extern struct cdt *cdt_func();
extern stok_dd_ctl_t stok_dd_ctl;
extern token_ring_all_mib_t tok_mib_status;
extern int stok_intr();
extern void stok_tx_timeout();
extern void stok_tx2_timeout();
extern void stok_cmd_timeout();
extern void stok_start_timeout();
extern void stok_start_recover();
extern void stok_lan_connected();

void stok_rx_setup_undo ();
void stok_tx_setup_undo();
/*****************************************************************************/
/*
 * NAME:     stok_open
  *
 * FUNCTION: Skyline driver open routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM:
 *      NS user by using the ndd_open field in the NDD on the NDD chain.
 *
 * CALLS TO:
 *      cdt_add
 *	dmp_add
 *	stok_open_adapter
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *
 * RETURNS: 
 *      0          - successful
 *      ENOCONNECT - unable to connect to network
 *      ENOMEM     - unable to allocate required memory
 */
/*****************************************************************************/
int stok_open(
ndd_t  	*p_ndd)		/* pointer to the ndd in the dev_ctl area */

{
  stok_dev_ctl_t  *p_dev_ctl = (stok_dev_ctl_t *)(p_ndd->ndd_correlator);
  int rc, i;
  int ipri;


  /* if the driver is in a power management mode of suspend/hibernate then put
   * the caller to sleep until the driver resumes normal operation except for
   * open this needs to be done under a lock so that the state check is valid
   */
  if (p_dev_ctl->device_state == SUSPEND_STATE) {
        e_sleep (&WRK.pmh_event, EVENT_SHORT);
  }

  ASSERT (p_dev_ctl->device_state == CLOSED); 

  /* 
   * Pins the entire driver in memory 
   */
  if (pincode(stok_config)) {
	return(ENOCONNECT);
  }

  TRACE_BOTH(STOK_OTHER, "OpnB", (ulong)p_dev_ctl, 0, 0);

  if (p_dev_ctl->device_state != CLOSED) {
        unpincode(stok_config);
        return(EINVAL);
  }

  /*
   * Sets the device state 
   */
  p_dev_ctl->device_state = OPEN_PENDING;

 /*
  *  Setup the component dump table
  */
  lock_write(&DD_LOCK);
  if ((stok_dd_ctl.open_count) == 0) {
        /*
         *  Allocates memory for the cdt structure
         */
        CDT = (stok_cdt_t *)
                xmalloc(sizeof(stok_cdt_t), MEM_ALIGN, pinned_heap);

        if (!CDT) {
                TRACE_BOTH(STOK_ERR, "Opn1", p_dev_ctl, ENOMEM, 0);
        	unpincode(stok_config);
                return(ENOMEM);
        }
        bzero(CDT, sizeof(stok_cdt_t));
        CDT->cdt_max_entry = STOK_CDT_SIZE;

	CDT->count = 0;
        CDT->head._cdt_magic = DMP_MAGIC;
        strcpy(CDT->head._cdt_name, STOK_DD_NAME);
        CDT->head._cdt_len = sizeof(struct cdt_head);
        for (i = 0; i < STOK_CDT_SIZE; i++) {
                CDT->entry[i].d_len = 0;
                CDT->entry[i].d_ptr = NULL;
        }

  	cdt_add("dd_ctl",(char *)&stok_dd_ctl, sizeof(stok_dd_ctl_t));
  	dmp_add(cdt_func);

  }
  stok_dd_ctl.open_count++;
  cdt_add("dev_ctl", (char *)p_dev_ctl, sizeof(stok_dev_ctl_t));
  lock_done(&DD_LOCK);

  /*
   *  Initializes the locks in the dev_ctl area
   */
  lock_alloc(&TX_LOCK,   LOCK_ALLOC_PIN, STOK_TX_LOCK,  p_dev_ctl->seq_number);
  lock_alloc(&CMD_LOCK,  LOCK_ALLOC_PIN, STOK_CMD_LOCK, p_dev_ctl->seq_number);
  lock_alloc(&SLIH_LOCK, LOCK_ALLOC_PIN, STOK_SLIH_LOCK, p_dev_ctl->seq_number);

  simple_lock_init(&TX_LOCK);
  simple_lock_init(&CMD_LOCK);
  simple_lock_init(&SLIH_LOCK);

  /*
   * Set up the watchdog timer control structure section
   */
  HWEWDT.func        = stok_start_timeout;
  HWEWDT.restart     = 100;
  HWEWDT.count       = 0;

  LIMWDT.func        = stok_start_recover;
  LIMWDT.restart     = 30;
  LIMWDT.count       = 0;

  LANWDT.func        = stok_lan_connected;
  LANWDT.restart     = 30;
  LANWDT.count       = 0;

  CMDWDT.func        = stok_cmd_timeout;
  CMDWDT.restart     = 10;
  CMDWDT.count       = 0;

  TXWDT.func        = stok_tx_timeout;
  TXWDT.restart     = 10;
  TXWDT.count       = 0;

  while (w_init ((struct watchdog *)(&(TXWDT))));
  while (w_init ((struct watchdog *)(&(CMDWDT))));
  while (w_init ((struct watchdog *)(&(HWEWDT))));
  while (w_init ((struct watchdog *)(&(LIMWDT))));
  while (w_init ((struct watchdog *)(&(LANWDT))));

  /* 
   * Saves the ndd start time for statistics 
   * and clears the counters 
   */
  CLEAR_STATS();
  bzero (&MIB, sizeof(MIB));

  /*
   * Activates the adapter and allocate all the resources needed
   */
  WRK.srb_address = 0;
  if (rc = stok_open_adapter(p_dev_ctl)) {
        TRACE_BOTH(STOK_ERR, "Opn2", p_dev_ctl, rc, 0);
  	p_dev_ctl->device_state = CLOSED;

        lock_write(&DD_LOCK);
        cdt_del("dev_ctl", (char *)p_dev_ctl, sizeof(stok_dev_ctl_t));
        if ((stok_dd_ctl.open_count) == 1) {
                dmp_del(cdt_func);
  		cdt_del("dd_ctl", (char *)&stok_dd_ctl, sizeof(stok_dd_ctl_t));
        	xmfree(CDT, pinned_heap);        /* Frees the cdt area     */
        }
        stok_dd_ctl.open_count--;
        lock_done(&DD_LOCK);

        /*
         * Frees the locks in the dev_ctl area
         */
        lock_free(&TX_LOCK);
        lock_free(&CMD_LOCK);
        lock_free(&SLIH_LOCK);

        w_clear (&(TXWDT));
        w_clear (&(LANWDT));
        w_clear (&(LIMWDT));
        w_clear (&(HWEWDT));
        w_clear (&(CMDWDT));

        unpincode(stok_config);
	return(ENOCONNECT);
  }

  MIB.Token_ring_mib.Dot5Entry.ring_state = TR_MIB_OPENED;

  /* 
   * Updates the NDD flags
   */
  NDD.ndd_flags |= NDD_UP;

  TRACE_BOTH(STOK_OTHER, "OpnE", p_dev_ctl, 0, 0);
  return(0);
}

/*****************************************************************************/
/*                                                                           
* NAME: stok_open_adapter
*                                                                           
* FUNCTION:  Setup PCI registers, allocated device resource  and start the 
*		sequence for open the adapter.   
*            
* EXECUTION ENVIRONMENT: process only
*                                                                           
* CALED FROM: 
*  	stok_open                                                  
*
* CALLS TO: 
*  	stok_rx_setup       - allocated the resource for rx channel.         
*  	stok_rx_setup_undo  - Undoes the effects of stok_rx_setup.
*  	stok_tx_setup       - allocated the resource for tx channel. 
*  	stok_tx_setup_undo  - Undoes the effects of stok_tx_setup.
*  	stok_rx_enable 	    - allocated the resource for tx channel.     
*       stok_act            - active the adapter            
*                                                                           
* INPUT: 
*      p_dev_ctl       - pointer to the dev_ctl area.
*
* RETURN:  
*      0 - OK
*      ENOCONNECT - unable to activate and setup the adapter
*      ENOMEM - unable to allocate required memory
*
*/
/*****************************************************************************/
int stok_open_adapter (
register stok_dev_ctl_t  *p_dev_ctl)

{
  int    ioa;
  int    cntl_reg;
  ushort data;
  int    i;                       /* loop counter                      */
  int    rc;                      /* Local return code                 */

  TRACE_BOTH(STOK_OTHER, "OoaB", p_dev_ctl, 0, 0);
 /*
  * the device driver uses PL_IMP to serialize with network services
  */

  ASSERT (PL_IMP == INTCLASS2); 
  if (PL_IMP != INTCLASS2) {
  	TRACE_BOTH(STOK_ERR, "Ooa2", p_dev_ctl, EINVAL, 0);
  	return(EINVAL);
  }

  /* 
   * Soft reset the adapter to force a known state                     
   */
  ioa = (int)iomem_att(&WRK.iomap);
  PIO_PUTSRX( ioa + BCTL, SOFT_RST_MSK);
  io_delay(5); 	/* Wait for adapter to reset */
  PIO_PUTSRX( ioa + BCTL, 0x0000);
  iomem_det((void *)ioa);              /* restore I/O Bus                */
  WRK.hard_err = FALSE;
  if (WRK.pio_rc) {
        stok_bug_out(p_dev_ctl,NDD_HARD_FAIL, NDD_PIO_FAIL,0,FALSE,FALSE,FALSE);
  	TRACE_BOTH(STOK_ERR, "Ooa4", p_dev_ctl, WRK.pio_rc, 0);
  	return(ENOCONNECT);
  }

  /* 
   * Set up dma channel 
   */
  WRK.dma_channel = D_MAP_INIT( BID_VAL(IO_PCI, PCI_IOMEM, DDS.busid),
				      DMA_MASTER, 0, 0);
  if ((int)WRK.dma_channel == DMA_FAIL) {
 	stok_logerr(p_dev_ctl, ERRID_STOK_MEM_ERR, __LINE__, 
  		   __FILE__, TOK_DMA_FAIL, 0, 0);
  	TRACE_BOTH(STOK_ERR, "Ooa2", p_dev_ctl, ENOCONNECT, 0);
  	return(ENOMEM);
  }

  /*
   * Set up the interrupt control structure section
   */
  IHS.next          = (struct intr *) NULL;
  IHS.handler       = stok_intr;
  IHS.bus_type      = BUS_BID;
  IHS.flags         = INTR_MPSAFE;      /* MP safe driver */
  IHS.level         = DDS.busintr;
  IHS.priority      = PL_IMP;           
  IHS.bid           = BID_VAL(IO_PCI,PCI_IOMEM, DDS.busid);

  /*
   * Adds our interrupt routine to kernel's interrupt chain 
   */
  if (rc = i_init ((struct intr *)(&(IHS)))) {
        D_MAP_CLEAR (WRK.dma_channel);   /* Free the DMA clannel         */
  	TRACE_BOTH(STOK_ERR, "Ooa3", p_dev_ctl, rc, 0);
       	return(ENOCONNECT);
  }

  /*********************************************************************/
  /* Set up for Control block ,transmit and receive memory area        */
  /*********************************************************************/
  if (rc = stok_rx_setup(p_dev_ctl)) {  
  	TRACE_BOTH(STOK_ERR, "Ooa5", p_dev_ctl, ENOMEM, 0);
        D_MAP_CLEAR (WRK.dma_channel);   /* Free the DMA clannel           */
  	i_clear(&(IHS));
  	return(ENOMEM);
  }

  if (rc = stok_tx_setup(p_dev_ctl, &TX2)) {  
  	TRACE_BOTH(STOK_ERR, "Ooa6", p_dev_ctl, ENOMEM, 0);
	stok_rx_setup_undo(p_dev_ctl);
        D_MAP_CLEAR (WRK.dma_channel);   /* Free the DMA clannel           */
  	i_clear(&(IHS));
  	return(ENOMEM);
  }

  if (DDS.priority_tx) {
  	if (rc = stok_tx_setup(p_dev_ctl, &TX1)) {  
  		TRACE_BOTH(STOK_ERR, "Ooa6", p_dev_ctl, ENOMEM, 0);
		stok_rx_setup_undo(p_dev_ctl);
		stok_tx_setup_undo(p_dev_ctl, &TX2);
        	D_MAP_CLEAR (WRK.dma_channel);   /* Free the DMA clannel     */
  		i_clear(&(IHS));
  		return(ENOMEM);
  	}
  }

  if (stok_rx_enable(p_dev_ctl)) {
	stok_rx_setup_undo(p_dev_ctl);
  	if (DDS.priority_tx) {
		stok_tx_setup_undo(p_dev_ctl, &TX1);
	}
	stok_tx_setup_undo(p_dev_ctl, &TX2);
        D_MAP_CLEAR (WRK.dma_channel);   /* Free the DMA clannel           */
  	i_clear(&(IHS));
  	return(ENOCONNECT);
  }

  if (stok_act(p_dev_ctl, FALSE)) {
	stok_rx_setup_undo(p_dev_ctl);
  	if (DDS.priority_tx) {
		stok_tx_setup_undo(p_dev_ctl, &TX1);
	}
	stok_tx_setup_undo(p_dev_ctl, &TX2);
        D_MAP_CLEAR (WRK.dma_channel);   /* Free the DMA clannel           */
  	i_clear(&(IHS));
  	return(ENOCONNECT);
  }

  WRK.ctl_event = EVENT_NULL;
  e_sleep(&WRK.ctl_event, EVENT_SHORT);

  if (p_dev_ctl->device_state == DEAD) {
  	TRACE_BOTH(STOK_ERR, "Ooa9", p_dev_ctl, p_dev_ctl->device_state, 0);
  	return(ENOCONNECT);
  }

  TRACE_BOTH(STOK_OTHER, "0oaE", p_dev_ctl, p_dev_ctl->device_state, 0);
  return (0);

} /* end stok_open_adapter                                                   */

/*****************************************************************************
*                                                                           
* NAME: stok_rx_setup                                                        
*                
* FUNCTION: Setup the system resources for the adapter to operate.
*                                                                           
* EXECUTION ENVIRONMENT: process only
*                                                                           
* INPUT: 
*      p_dev_ctl       - pointer to the dev_ctl area.
*
* CALLED FROM: 
*  	stok_open_adapter      
*                                             
* RETURN:   
*      0 - OK
*      ENOMEM - unable to allocate required memory
*                                                                           
****************************************************************************/
int stok_rx_setup (stok_dev_ctl_t *p_dev_ctl)
{
  int      rc, i, j, num_rx_buf;

  TRACE_BOTH(STOK_OTHER, "OrsB", p_dev_ctl, 0, 0);
  /*********************************************************************/
  /* Set up for receive                                                */
  /*********************************************************************/
  /* 
   * Each Rx descriptor needs to be in 256 bytes boundary. 
   */
  num_rx_buf = (DDS.rx_que_sz - 1) / 16 + 1;
  WRK.rx_desc_blk = (uchar *)
		xmalloc(num_rx_buf * PAGESIZE, DMA_L2PSIZE, pinned_heap);

  /* 
   * Checks if memory allocation was successful 
   */
  if (WRK.rx_desc_blk == NULL) {
  	TRACE_BOTH(STOK_ERR, "Ors1", p_dev_ctl, 0, 0);
        stok_logerr(p_dev_ctl, ERRID_STOK_MEM_ERR, __LINE__, __FILE__, 0, 0, 0);
  	return(ENOMEM);     
  }  /* end if no memory available */

  for (i = 0; i < DDS.rx_que_sz; i++) {
	WRK.rx_desc_p[i] = (rx_list_t *)WRK.rx_desc_blk + i;
  	if (rc = D_MAP_PAGE(WRK.dma_channel, DMA_READ, WRK.rx_desc_p[i], 
				&WRK.rx_dma_desc_p[i], xmem_global)) {

  		TRACE_BOTH(STOK_ERR, "Ors2", p_dev_ctl, rc, 0);
		for (j = 0; j < i; j++) {
			D_UNMAP_PAGE(WRK.dma_channel, &WRK.rx_dma_desc_p[j]);
		}
        	xmfree(WRK.rx_desc_blk, pinned_heap);
        	stok_logerr(p_dev_ctl,ERRID_STOK_MEM_ERR, __LINE__, 
							__FILE__, rc, 0, 0);
  		return(ENOMEM);     /* No! return errno */
  	}
  }

  /*
   * Initializes the dma & mbuf pointers arrays
   */
  for (i = 0; i < DDS.rx_que_sz; i++) {
        /*
         * Gets page size chunk of memory
         */
	for (j = 0; j < 10; j++) {
        	WRK.rx_mbuf[i] = m_getclust(M_DONTWAIT, MT_HEADER);
                if (WRK.rx_mbuf[i]) {
        		if (D_MAP_PAGE(WRK.dma_channel, DMA_READ,
                        		mtod(WRK.rx_mbuf[i], uchar *),
                        		&WRK.rx_addr[i], xmem_global)) {
                		m_freem(WRK.rx_mbuf[i]);
                		WRK.rx_mbuf[i] = NULL;
			}
			break;
		}
        }

        if (WRK.rx_mbuf[i] == NULL) {
		for (j = 0; j < DDS.rx_que_sz; j++) {
			D_UNMAP_PAGE(WRK.dma_channel, &WRK.rx_dma_desc_p[j]);
		}
        	xmfree(WRK.rx_desc_blk, pinned_heap);
  		for (j = 0; j < i; j++) {
			D_UNMAP_PAGE(WRK.dma_channel, &WRK.rx_addr[j]);
                	m_freem(WRK.rx_mbuf[j]);
                	WRK.rx_mbuf[j] = NULL;
		}
                return (ENOMEM);
        }
  } /* end of for */

  TRACE_BOTH(STOK_OTHER, "OrsE", p_dev_ctl, 0, 0);
  return(0);
}


/*****************************************************************************
*
* NAME: stok_rx_setup_undo
*
* FUNCTION: Undoes the effects of stok_rx_setup.
*           Frees the receive mbufs allocated & frees descriptor area.
*
* EXECUTION ENVIRONMENT: 
*
*      This routine runs under both the process thread and the off level
*      interrupt handler.
*
* NOTES:
*
* INPUT:
*  p_dev_ctl - pointer to device control structure.
*
* CALLED FROM:
*	stok_open_adapter
*	stok_close
*
*****************************************************************************/
void stok_rx_setup_undo (
stok_dev_ctl_t       *p_dev_ctl)  /* pointer to the device control area */
{
  register int          i;

  TRACE_BOTH(STOK_OTHER, "OruB", p_dev_ctl, 0, 0);

  for (i = 0; i < DDS.rx_que_sz; i++) {
        D_UNMAP_PAGE(WRK.dma_channel, &WRK.rx_dma_desc_p[i]);
        D_UNMAP_PAGE(WRK.dma_channel, &WRK.rx_addr[i]);
        m_freem(WRK.rx_mbuf[i]);
        WRK.rx_mbuf[i] = NULL;
  }

  /*
   * Undo adapter control area
   */
  xmfree(WRK.rx_desc_blk, pinned_heap);
  TRACE_BOTH(STOK_OTHER, "OruE", p_dev_ctl, WRK.rx_mbuf[0], 0);

} /* end stok_rx_setup_undo                                                  */



/****************************************************************************/
/*
* NAME: stok_tx_setup
*
* FUNCTION: Set up the transmit list and Initialize the transmit list
*           variables.
*
* EXECUTION ENVIRONMENT: process only
*
* INPUT:
* 	p_dev_ctl - pointer to device control structure
*       wrk       - pointer to the Tx channel structure
*
* CALLED FROM:
*  	stok_open_adapter
*
* RETURN:  
*	0       - Successful completion
*       ENOMEM - unable to allocate required memory
*/
/****************************************************************************/

int stok_tx_setup(
stok_dev_ctl_t  *p_dev_ctl,
tx_t		*wrk)
{
  int                   rc,ioa, pio_rc;
  uchar                 *addr_p;
  int                   i,j;          /* Loop Counter                        */

  TRACE_BOTH(STOK_OTHER, "OtsB", p_dev_ctl, 0, 0);

  /********************************************************************/
  /* Set up for transmit channel -  TX variables, etc               */
  /********************************************************************/
  /* Gets page size chunk of memory */
  wrk->tx_desc_blk = (uchar *) 
		xmalloc(MAX_TX_LIST * sizeof(tx_list_t), PGSHIFT, pinned_heap);
  bzero(wrk->tx_desc_blk, MAX_TX_LIST * sizeof(tx_list_t));

  /* 
   * Checks if memory allocation was successful 
   */
  if (wrk->tx_desc_blk == NULL) {
       	TRACE_BOTH(STOK_ERR, "Ost1", p_dev_ctl, 0, 0);
       	stok_logerr(p_dev_ctl,ERRID_STOK_MEM_ERR,__LINE__, __FILE__,0,0,0);
  	return(ENOMEM);     
  }  /* end if no memory available */

  /*
   *  Set up Tx buffer descriptor for Tx descriptor area   
   *  Each Tx descriptor is 256 bytes. 
   */
  for (i = 0; i < MAX_TX_LIST; i ++) {
	wrk->tx_desc_p[i] = (tx_list_t *)wrk->tx_desc_blk + i;
  	if (rc = D_MAP_PAGE(WRK.dma_channel, DMA_READ, wrk->tx_desc_p[i],
			    &wrk->tx_dma_desc_p[i], xmem_global) != DMA_SUCC) {
       		TRACE_BOTH(STOK_ERR, "Ots2", p_dev_ctl, rc, 0);

		for (j = 0; j < i; j++) {
			D_UNMAP_PAGE(WRK.dma_channel,&wrk->tx_dma_desc_p[j]);
		}
               	xmfree(wrk->tx_desc_blk, pinned_heap);
       		stok_logerr(p_dev_ctl,ERRID_STOK_MEM_ERR,__LINE__,
						__FILE__,0,0,0);
  		return(ENOMEM);     
	}
  }


  for (i = 0; i < MAX_TX_LIST; i ++) {
        wrk->tx_desc_p[i]->fw_pointer  =
                   TOENDIANL((ulong)wrk->tx_dma_desc_p[(i + 1) % MAX_TX_LIST]);

  } /* end of for */
  /*********************************************************************/
  /*
   *  Creates the transmit chain by initializing the pointer to TX
   *  channel list.  Initializes the dma & mbuf pointers arrays
   */
  /*********************************************************************/
  for (i = 0; i < wrk->max_buf_list; i++) {

        wrk->tx_buf[i] = (uchar *) xmalloc(HAFT_PAGE, PGSHIFT, pinned_heap);
        
        if (wrk->tx_buf[i] != NULL) {
        	if (D_MAP_PAGE(WRK.dma_channel, DMA_READ, wrk->tx_buf[i],
                   		&wrk->tx_addr[i], xmem_global)) {
                        xmfree(wrk->tx_buf[i], pinned_heap);
                        wrk->tx_addr[i] = NULL;

		}
	}

        if (wrk->tx_buf[i] == NULL) {
                /*
                 * Fails to allocated a tx buffer memory
                 */
                int j;

                TRACE_BOTH(STOK_ERR, "Ost3", p_dev_ctl, i, 0);
                for (j = 0; j < i; j++) {
                        D_UNMAP_PAGE(WRK.dma_channel, &wrk->tx_addr[j]);
                        xmfree(wrk->tx_buf[j], pinned_heap);
                        wrk->tx_buf[j] = NULL;
                        wrk->tx_addr[j] = NULL;
                }
		for (j = 0; j < MAX_TX_LIST; j++) {
			D_UNMAP_PAGE(WRK.dma_channel, &wrk->tx_dma_desc_p[j]);
		}
               	xmfree(wrk->tx_desc_blk, pinned_heap);
                return(ENOBUFS);
        }

  } /* end of for */


  /*
   * Set up variable for Transmit list
   */
  wrk->tx_elem_next_out = 0;
  wrk->tx_elem_next_in  = 0;
  wrk->tx_frame_pending = 0;
  wrk->tx_buf_next_in   = 0;
  wrk->tx_buf_use_count = 0;
  wrk->tx_dma_next_out  = 0;
  wrk->tx_dma_next_in   = 0;
  wrk->tx_dma_use_count = 0;
  wrk->sw_txq_len       = 0;
  wrk->sw_txq_first     = NULL;
  wrk->sw_txq_last      = NULL;


  TRACE_BOTH(STOK_OTHER, "OstE", p_dev_ctl, 0, 0);
  return(0);
}  /* end function stok_tx_setup                                              */

/****************************************************************************/
/*
* NAME: stok_tx_setup_undo
*
* FUNCTION: Set up the transmit list and Initialize the transmit list
*           variables.
*
* EXECUTION ENVIRONMENT: process only
*
* INPUT:
*  	p_dev_ctl - pointer to device contril structure
*       wrk       - pointer to the Tx channel structure
*
* CALLED FROM:
*  	stok_open_adapter
*  	stok_close
*
*/
/****************************************************************************/

void stok_tx_setup_undo(
stok_dev_ctl_t  *p_dev_ctl,
tx_t		*wrk)
{
  int                   i;            /* Loop Counter                        */

  TRACE_BOTH(STOK_OTHER, "OtuB", p_dev_ctl, 0, 0);

  for (i = 0; i < MAX_TX_LIST; i++) {
	D_UNMAP_PAGE(WRK.dma_channel, &wrk->tx_dma_desc_p[i]);
  } /* end of for */
  xmfree(wrk->tx_desc_blk, pinned_heap);

  for (i = 0; i < wrk->max_buf_list; i++) {
	D_UNMAP_PAGE(WRK.dma_channel, &wrk->tx_addr[i]);
	xmfree(wrk->tx_buf[i], pinned_heap);
	wrk->tx_buf[i] = NULL;
	wrk->tx_addr[i] = NULL;
  } /* end of for */

  TRACE_BOTH(STOK_OTHER, "OtuE", p_dev_ctl, 0, 0);
}  /* end function stok_tx_setup_undo                                      */


/****************************************************************************/
/*
* NAME: stok_act
*
* FUNCTION: Start the sequence for open the adapter
*
* EXECUTION ENVIRONMENT: process only
*
* INPUT:
*  	p_dev_ctl - pointer to device contril structure
*       at_int_lvl - flag for from interrupt level
*
* CALLED FROM:
*  	stok_open_adapter
*  	stok_re_open_adapter
*
*/
/****************************************************************************/

int stok_act (stok_dev_ctl_t *p_dev_ctl,
	      int	  at_int_lvl) {

  int    ioa;

  /*
   * Gets access to the I/O bus to access I/O registers
   */
  TRACE_BOTH(STOK_OTHER, "OatB", p_dev_ctl, 0, 0);
  ioa = (int)iomem_att(&WRK.iomap);
  PIO_PUTSRX( ioa + MISRMask, MISR_MSK); /* setup the MISR mask register*/

  /*********************************************************************/
  /*
   * Starts the adapter init sequence - SISR_Mask is edge triggered
   * The response of this will be in INITIALIZATION_COMPLETE in
   * srb_response function.
   */
  /*********************************************************************/
  PIO_PUTSRX( ioa + SISRMask, SISR_MSK);
  iomem_det((void *)ioa);                    /* restore I/O Bus              */
  if (WRK.pio_rc)
  {
        stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0,
                        at_int_lvl, FALSE, FALSE);
        TRACE_BOTH(STOK_ERR, "Oat1", p_dev_ctl, WRK.pio_rc, 0);
        return (ENOCONNECT);
  }

  w_start (&(HWEWDT));                 /* starts watchdog timer        */
  TRACE_BOTH(STOK_OTHER, "OatE", p_dev_ctl, 0, 0);
  return(0);
}

/*****************************************************************************/
/*
* NAME: stok_rx_enable
*
* FUNCTION: Enable receive channel
*
* EXECUTION ENVIRONMENT:
*
*      This routine runs only under the process thread.
*
* INPUT:
*       p_dev_ctl       - point to the device control area
*
* CALLED FROM:
*  	stok_open_adapter
*  	stok_re_open_adapter
*
* RETURN:  
*	0 - Successful completion
*       ENETDOWN - device is down
*/
/*****************************************************************************/
int stok_rx_enable (
stok_dev_ctl_t   *p_dev_ctl)  /* pointer to the device control area */

{
  volatile rx_list_t    recvlist;
  uchar                 *data_p;
  uchar                 *addr_p;
  caddr_t               busmem;
  int                   rc, ioa;
  int                   i, j;        /* Loop Counter                        */

  TRACE_BOTH(STOK_OTHER, "OreB", p_dev_ctl, 0, 0);

  WRK.read_index = 0;
  WRK.eof = TRUE;
  WRK.mhead = NULL;
  WRK.mtail = NULL;

  /*
   * Initializes the dma & mbuf pointers arrays
   */

  for (i = 0; i < DDS.rx_que_sz; i++) {
        /*
         * Set up buffer table entry
         */
        WRK.rx_desc_p[i]->fw_pointer   = 
		TOENDIANL((ulong)WRK.rx_dma_desc_p[(i + 1) % DDS.rx_que_sz]);
        WRK.rx_desc_p[i]->rx_status    = 0;
        WRK.rx_desc_p[i]->data_pointer = TOENDIANL((ulong) WRK.rx_addr[i]);
        WRK.rx_desc_p[i]->data_len     = TOENDIANW((ushort) PAGESIZE);
        WRK.rx_desc_p[i]->fr_len       = 0;
  } /* end of for */

  /*
   * Gives the buffer descriptor address to adapter
   */
  ioa = (int)iomem_att(&WRK.iomap);
  PIO_PUTLRX(ioa + RxBDA_L, WRK.rx_dma_desc_p[0]);
  io_delay(10);
  PIO_PUTLRX(ioa + RxLBDA_L, WRK.rx_dma_desc_p[DDS.rx_que_sz -1]);
  iomem_det((void *)ioa);                    /* restore I/O Bus              */
  if (WRK.pio_rc) {
        stok_bug_out(p_dev_ctl, NDD_HARD_FAIL,NDD_PIO_FAIL,0,TRUE,FALSE,FALSE);
        TRACE_BOTH(STOK_ERR, "Ore1", p_dev_ctl, WRK.pio_rc, 0);
        return (ENETDOWN);
  }

  TRACE_BOTH(STOK_OTHER, "OreE", p_dev_ctl, WRK.rx_mbuf[0], 0);
  return (0);
}

