static char sccsid[] = "@(#)47  1.1  src/rspc/kernext/pci/stok/stok_dump.c, pcitok, rspc41J, 9513A_all 3/21/95 17:27:47";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: dump_cmd_to_adapter
 *		dump_modify_receive_options
 *		dump_set_functional_address
 *		dump_unset_group_address
 *		ms2time
 *		multi_address_delete
 *		stok_dump
 *		stok_dump_output
 *		stok_dump_read
 *		stok_dump_recv
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

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include <sys/cblock.h>
#include <sys/poll.h>
#include <sys/systemcfg.h>
#include <sys/user.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_802_5.h>
#include <netinet/ip.h>
#include <netinet/in.h>

#include "stok_dslo.h"
#include "stok_mac.h"
#include "stok_cmd.h"
#include "stok_dds.h"
#include "stok_dd.h"
#include "tr_stok_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

#define TX_DUMP_TIMEOUT  1000          /* Dump write timeout value         */
#define ERR_RC           -1          

#define FCF_MASK        0xC0           /* mask for frame type field        */
#define MAC_TYPE        0x00           /* Medium Access Control Frame type */
/*-------------------------------------------------------------------------*/
/*                              MACROS                                     */
/*-------------------------------------------------------------------------*/

/*
 * Converts micro-second to rtcl_t time value.
 */
#define MSEC_PER_SEC    (NS_PER_SEC / NS_PER_MSEC)
ms2time(msec, tp)
int                     msec;    /*  Input timeout value         */
struct timestruc_t      *tp;     /*  Output converted time value */
{
  tp->tv_sec = msec / MSEC_PER_SEC;
  tp->tv_nsec = (msec % MSEC_PER_SEC) * NS_PER_MSEC;
} /* End of ms2time() */

 void dump_cmd_to_adapter();
 void dump_modify_receive_options();
 void dump_set_functional_address();
 void dump_unset_group_address();
/*---------------------  D U M P _ R E A D ----------------------*/
/*                                                                      */
/*  NAME: stok_read_dump                                                */
/*                                                                      */
/*  FUNCTION:                                                           */
/*     This routine will handle reading from the adapter in the         */
/*     case of a system dump.  It will copy this first valid packet     */
/*     sent to the tcpip address into the mbuf provided by the          */
/*     user.                                                            */
/*                                                                      */
/*  EXECUTION ENVIRONMENT:                                              */
/*      Called by the dump entry point.                                 */
/*                                                                      */
/*  DATA STRUCTURES:                                                    */
/*                                                                      */
/*  RETURNS: a -1 if there was a failure and 0 upon success.            */
/*                                                                      */
/*----------------------------------------------------------------------*/

int stok_dump_read(
stok_dev_ctl_t  *p_dev_ctl,
  struct dump_read    *dump_parms)
{
  struct mbuf         *dump_mbuf;     /* Dump mbuf pointer             */
  struct timestruc_t  current_time,   /* Keeps the current time        */
  		      timeout_time,    /* Time out value                */
  		      temp_time;
  int                 ioa, rc;
  ushort              sisr_status_reg = 0;/* interrupt status register */
  ushort              misr_reg = 0;  /* MISR interrupt status register */

  dump_mbuf = dump_parms->dump_bufread;
  dump_mbuf->m_next = NULL;
  dump_mbuf->m_len  = 0;
  dump_mbuf->m_flags |= M_PKTHDR;

  /*
   *  Set up time information
   */
  ms2time(dump_parms->wait_time, &temp_time);
  curtime(&current_time);
  ntimeradd(current_time, temp_time, timeout_time);

  ioa = (int)iomem_att(&WRK.iomap);

  /*
   *  Polls the adapter until timed out or interrupt is detected
   */
  while (TRUE) {

 	/*
         * Reads in the interrupt reason from adapter 
	 */
        PIO_GETSRX(ioa + SISR, &sisr_status_reg);

        /*
         *  Checks if the adapter have set the system interrupt bit
         */
        if (sisr_status_reg & MISR_INT) {
                PIO_GETSRX(ioa + MISR, &misr_reg);
                /*
                 *  Validate that it is a receive interrupt
                 */
                if ((misr_reg & Rx_EOF) && (WRK.eof)) {
                        /* Process the receive inter */
                        stok_dump_recv(p_dev_ctl, dump_mbuf);
                } else {
        		discard_packet(p_dev_ctl);
                }

		/*
                 * Resets the SISR & MISR interrupt 
		 */
                PIO_PUTSRX( ioa + MISR, ~misr_reg); 
                PIO_PUTSRX( ioa + SISR, ~sisr_status_reg);
                if (dump_mbuf->m_next) {
                        iomem_det((void *)ioa);
                        return(0);
                }
        }

        /*
         *  Checks if timed out has been reached.
         */
        if (ntimercmp(current_time, timeout_time, >)) {
                iomem_det((void *)ioa);
                return (ETIMEDOUT);
        }
        curtime(&current_time);
  }

} /* End of stok_dump_read() */

/*****************************************************************************/
/*
 * NAME: stok_dump_recv
 *
 *  FUNCTION:                                                           
 *      Copy data to dump user mbuf - if it's a good frame.
 *                                                                      
 * RETURNS:
 *      TRUE  if successful
 *      FALSE on failure
*/
/*****************************************************************************/
int stok_dump_recv (
stok_dev_ctl_t  *p_dev_ctl,
struct mbuf   *dump_mbuf)                 /* Dump mbuf pointer            */
{
  long			  rx_status, fr_len;
  struct mbuf             *m;
  int                     i;
  uchar                   dsap;           /* DSAP                         */
  uchar                   *p_dsap;        /* DSAP address                 */
  ushort                  bad_frame=FALSE,/* Invalid dump frame indicator */
                          rc,             /* General return code          */
                          macsize;        /* size of mac header           */
  struct ie5_mac_hdr      *p_mac;         /* Pointer to MAC header        */

  i = WRK.read_index;             /* start at next receive list   */

  rx_status  = TOENDIANL(WRK.rx_desc_p[i]->rx_status);
  fr_len     = TOENDIANW(WRK.rx_desc_p[i]->fr_len);

  /*
   * Checks if this is a good frame to pass on
   */
  m = WRK.rx_mbuf[i];
  m->m_len = TOENDIANW(WRK.rx_desc_p[i]->data_len); /* buffer len */
  bad_frame = FALSE;
  if ((fr_len > HAFT_PAGE) ||
      (rx_status & RX_ERR & PROTOCOL_ERR1) ||
      (rx_status & PROTOCOL_ERR1) ||
      (rx_status & PROTOCOL_ERR2)) {

	/*
         * Bad data 
	 */
        bad_frame = TRUE;

  } else {

       	/*
       	 *  If frame is NOT to an individual address, it is a bad frame
       	 */
       	if (*(uchar *)((m->m_data)+2) & MULTI_BIT_MASK) {
               	bad_frame = TRUE;
	}

       	/*
      	 *  If frame is a mac frame then mark it as a bad frame
       	 */
      	 p_mac = MTOD(m, struct ie5_mac_hdr *);
      	 if ((p_mac->mac_fcf & FCF_MASK) == MAC_TYPE) {
       	 	bad_frame = TRUE;
       	 }
       	macsize = mac_size(p_mac);

       	/*
       	 *  Filter off all frames that are not to AA SAP
       	 */
       	p_dsap = MTOD(m, uchar *);
       	dsap = *(uchar *)(p_dsap + macsize);
       	if (dsap != 0xaa) {
       		bad_frame = TRUE;
       	}
  }

 /*
  *  If the frame was bad then drop frame and read in
  *     next frame, otherwise process frame.
  */
  if (!bad_frame) {
       	bcopy(mtod(m, caddr_t), mtod(dump_mbuf, caddr_t), m->m_len);
  	dump_mbuf->m_len = m->m_len;

       	/*
       	 * Skips MAC hdr, LLC hdr, and SNAP hdr
       	 */
       	dump_mbuf->m_len = dump_mbuf->m_len - (macsize +
                      		sizeof(struct ie2_llc_snaphdr) );
       	dump_mbuf->m_pkthdr.len = dump_mbuf->m_len;
       	dump_mbuf->m_data = (caddr_t)p_mac + macsize +
					 sizeof(struct ie2_llc_snaphdr);

    	arm_rx_list(p_dev_ctl, i, FALSE, m);
       	WRK.read_index=(WRK.read_index + 1) % DDS.rx_que_sz;
  } else {
    	discard_packet(p_dev_ctl);
  }

} /* end of stok_dump_recv */

/*****************************************************************************/
/*
 * NAME: stok_dump_output
 *
 * FUNCTION: write function for remote dump
 *
 * RETURNS:
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
int stok_dump_output ( ndd_t          *p_ndd,
                       struct mbuf    *p_mbuf)
{
  stok_dev_ctl_t         *p_dev_ctl;
  int                   i,rc,ioa;
  int                   count = 0;
  int                   room = 0;
  struct     mbuf       *buf_to_free;
  struct     mbuf       *mbufp;
  struct     mbuf       *p_last;
  int                   ipri;
  ushort                sisr_status_reg =0; /* interrupt status register    */
  ushort                Stat;
  ulong                 DBA;
  ulong                 BDA;
  ulong                 FDA;
  ushort                misr_reg = 0;       /*MISR interrupt status register*/
  struct timestruc_t    current_time,       /* Keeps the current time       */
                        timeout_time,       /* Timed out value              */
                        temp_time;


  /*
   *  Get adapter structure
   */
  p_dev_ctl = (stok_dev_ctl_t *) 
		(((unsigned long) p_ndd) - offsetof(stok_dev_ctl_t, ndd));


  i = TX2.tx_elem_next_in;

  /*
   * Copys data into tranmit buffer 
   */
  bcopy(mtod(p_mbuf, caddr_t), TX2.tx_buf[i], p_mbuf->m_len);

  /*
   * Set up buffer descriptor entry                                  
   */
  TX2.tx_desc_p[i]->fw_pointer   = 
		TOENDIANL((ulong)TX2.tx_dma_desc_p[(i + 1)%MAX_TX_LIST]);
  TX2.tx_desc_p[i]->xmit_status  = 0;
  TX2.tx_desc_p[i]->buf_count    = TOENDIANW(1);
  TX2.tx_desc_p[i]->frame_len    = 0;
  TX2.tx_desc_p[i]->xbuf[0].data_pointer = TOENDIANL((ulong)TX2.tx_addr[i]);
  TX2.tx_desc_p[i]->xbuf[0].buf_len      = TOENDIANW((ushort)p_mbuf->m_len);

  ioa = (int)iomem_att(&WRK.iomap);
  PIO_PUTLRX(ioa + TX2LFDA_L, (int)TX2.tx_dma_desc_p[i]);
  iomem_det((void *)ioa);

  INC_DESC(TX2.tx_elem_next_in, MAX_TX_LIST);

}

/************************************************************************/
/*                                                                      */
/* NAME:        stok_dump                                               */
/*                                                                      */
/* FUNCTION:    Adapter Driver Dump Routine                             */
/*                                                                      */
/*      This routine handles requests for dumping data to a previously  */
/*      opened device.                                                  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine is called when there is certain to be limited      */
/*      functionality available in the system.  However, system         */
/*      dma kernel services are available.  The passed data is already  */
/*      pinned by the caller.  There are no interrupt or timer kernel   */
/*      services available.  This routine should run at INTMAX level.   */
/*                                                                      */
/* NOTES:                                                               */
/*      The driver should ignore extra dumpinit and dustoktart calls.   */
/*      The driver must check for the availability of internal          */
/*      resources, as these are not guaranteed to be available          */
/*      when the dump routine is called, but the system will have       */
/*      tried to quiesce itself before making the first dump call.      */
/*      Any lack of resources, or error in attempting to run any        */
/*      command, is considered fatal in the context of the dump.        */
/*      It is assumed that normal operation will not continue once      */
/*      this routine has been executed.  Once the DUMPWRITE logic has   */
/*      been executed, in fact, it will be impossible to use the        */
/*      driver's normal path successfully.                              */
/*                                                                      */
/*                                                                      */
/* INPUTS:                                                              */
/*      p_ndd   - pointer to the ndd in the dev_ctl area                */
/*      cmd     - parameter specifying the dump operation               */
/*      arg     - pointer to command specific structure                 */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      A zero will be returned on successful completion, otherwise,    */
/*      one of the errno values listed below will be given.             */
/*                                                                      */
/* ERROR DESCRIPTION:  The following errno values may be returned:      */
/*      0       - successful completion                                 */
/*      ENXIO   - kernel service failure, not opened, or general        */
/*                failure running DUMPWRITE option.                     */
/*      EACCES  - user is not a trusted user                            */
/*      EINVAL  - invalid request                                       */
/*      ETIMEDOUT - the DUMPREAD option timed-out                       */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      i_disable       i_enable                                        */
/*                                                                      */
/************************************************************************/
int
stok_dump(
ndd_t         *p_ndd,         /* pointer to the ndd in the dev_ctl area */
int           cmd,            /* control command */
int           arg)            /* address of parameter block */
{
  stok_dev_ctl_t        *p_dev_ctl = (stok_dev_ctl_t *)(p_ndd->ndd_correlator);
  int                   rc = 0;  	/* Exit code              */
  int                   pio_attachment; /* PIO attachment         */
  struct dmp_query      dump_ptr;       /* Dump info. structure   */
  struct dump_read      dump_readp;     /* Dump info. structure   */
  int                   stok_dump_output();
  int                   dump_pri, ioa, i;
  volatile rx_list_t    recvlist;

  /*
   *  Processes dump command
   */
  switch (cmd) {

  	case DUMPINIT:
        	break;

  	case DUMPQUERY:
        	dump_ptr.min_tsize = CTOK_MIN_PACKET;
        	dump_ptr.max_tsize = HAFT_PAGE;
        	bcopy(&dump_ptr,arg,sizeof(struct dmp_query));
        	break;

  	case DUMPSTART:

        	if (p_dev_ctl->device_state != OPENED) {
			return(ENETDOWN);
		}
 
        	ioa = (int)iomem_att(&WRK.iomap);
        	PIO_PUTSRX( ioa + MISR, 0);
        	PIO_PUTSRX( ioa + SISR, 0);
        	iomem_det((void *)ioa);

		multi_address_delete (p_dev_ctl, p_ndd);

  		/*
  		 * Set up variables for Transmit list 
  		 */
  		TX2.tx_elem_next_out = 0;
  		TX2.tx_elem_next_in  = 0;
  		TX2.tx_frame_pending = 0;

        	NDD.ndd_output = stok_dump_output;

		WRK.mhead = 0;
  		while (TRUE) {
        		i = WRK.read_index;        /* start at next recv list */

        		bcopy(WRK.rx_desc_p[i], &recvlist, (uint)RX_LIST_SIZE);
        		recvlist.rx_status  = TOENDIANL(recvlist.rx_status);

  			if (!((recvlist.rx_status & END_OF_FRAME) ||
	  		      (recvlist.rx_status & BUF_PROCESS ))) {
				break;
			}
        		discard_packet(p_dev_ctl, 0);
		}

        	WRK.dump_started   = TRUE;
        	break;

  	case DUMPREAD:
        	/*
        	 *  Calls internal routine to execute the command
        	 */
        	if (!WRK.dump_started) {
                	return(EINVAL);
        	}

        	rc = (int)stok_dump_read(p_dev_ctl, (struct dump_read *)arg);
        	break;

  	case DUMPEND:
        	if (!WRK.dump_started) {
                	return(EINVAL);
		}
		reset_adapter(p_dev_ctl); 
        	WRK.dump_started = FALSE;
        	p_dev_ctl->device_state = DEAD;
        	break;

  	case DUMPTERM:
        	break;

  	default:
        	rc = EINVAL;
        	break;

  }     /* end switch (cmd) */

  return (rc);

}  /* End of stok_dump */

/*****************************************************************************/
/*
 * NAME:     multi_address_delete
 *
 * FUNCTION: 
 * 	Unset group address, functional address and trun off promiscuous mode
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      stok_ctl
 *
 * INPUT:
 *      p_dev_ctl       - point to the dev_ctl area
 *      p_ndd           - pointer to the ndd in the dev_ctl area
 *
 */
/*****************************************************************************/
multi_address_delete (
  stok_dev_ctl_t   *p_dev_ctl,      /* point to the dev_ctl area              */
  ndd_t            *p_ndd)          /* pointer to the ndd in the dev_ctl area */

{

  stok_multi_t *p_multi = &WRK.multi_table;
  int   i;
  int   count = 0;


  /*
   * Unsets the functional address
   */
  FUNCTIONAL.functional[2] = 0x00;
  FUNCTIONAL.functional[3] = 0x00;
  FUNCTIONAL.functional[4] = 0x00;
  FUNCTIONAL.functional[5] = 0x00;
  dump_set_functional_address(p_dev_ctl, &FUNCTIONAL.functional[2]);

  /*
   * Check if any group address net to unset
   */
   while (p_multi) {
        for (i=0; i < p_multi->in_use; i++) {
                dump_unset_group_address (p_dev_ctl,p_multi->m_slot[i].m_addr);
                count++;
                if (count >= MAX_MULTI) {
                        break;
                }
        }

        if (count >= MAX_MULTI) {
                break;
        }
        p_multi = p_multi->next;
  }


  /*
   * Checks if we need to turn off the promiscuous mode
   */
  if (WRK.promiscuous_count > 0) {
       dump_modify_receive_options (p_dev_ctl, PROMIS_OFF);
  }

}

/******************************************************************************/
/*
* NAME: dump_cmd_to_adapter
*
* FUNCTION: Sends a command to the adapter
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input:
*       p_dev_ctl  - pointer to device control structure.
*
*/
/******************************************************************************/
void dump_cmd_to_adapter (
stok_dev_ctl_t          *p_dev_ctl,
ushort                  *parm_s,
ushort                  size)
{

  register int  ioa, i;
  int           ipri;

  /*
   * Gets access to the I/O bus to access I/O registers
   */
  ioa = (int)iomem_att(&WRK.iomap);

  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i = 0; i < size/2; i++) {
        PIO_PUTSX(ioa + LAPD_I, *(parm_s + i));
  }
  PIO_PUTCX(ioa + LISR_SUM, SRB_CMD); /* SUM.LISR.bit5 0010 0000*/
  iomem_det((void *)ioa);

  io_delay(5000);

} /* end of dump_cmd_to_adapter */

/******************************************************************************/
/*
* NAME: dump_modify_receive_options
*
* FUNCTION: Modify the receive channel options
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input:
*       p_dev_ctl  - pointer to device control structure.
*       rcv_op     - receive modify option
*
*    Called From:
*       stok_ctl
*
* RETURN:  0        = Successful completion
*          ENETDOWN = Error return
*/
/******************************************************************************/
void dump_modify_receive_options (
register stok_dev_ctl_t         *p_dev_ctl,
ushort                          rcv_op)
{

  mod_recv_op   parm;
  ushort        *parm_s = (ushort *)&parm;

  bzero(parm_s, sizeof(mod_recv_op));

  parm.cmd     = MODIFY_RECEIVE_OPTION;
  parm.retcode = 0xFF;
  parm.recv_op = rcv_op;

  parm.passwd[0] = PASSWD_0;
  parm.passwd[1] = PASSWD_1;
  parm.passwd[2] = PASSWD_2;
  parm.passwd[3] = PASSWD_3;
  parm.passwd[4] = PASSWD_4;
  parm.passwd[5] = PASSWD_5;
  parm.passwd[6] = PASSWD_6;
  parm.passwd[7] = PASSWD_7;

  dump_cmd_to_adapter(p_dev_ctl, parm_s, sizeof(mod_recv_op));

} /* end of dump_modify_receive_options */

/******************************************************************************/
/*
* NAME: dump_set_functional_address
*
* FUNCTION: Set the functional address for which the adapter will receive
*           messages.
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input:
*       p_dev_ctl   - pointer to device control structure.
*       addr        - address to be set
*
* RETURN:  0        = Successful completion
*          ENETDOWN = Error return
*/
/******************************************************************************/
void dump_set_functional_address (
register stok_dev_ctl_t  *p_dev_ctl,
uchar                   addr[4])
{
  f_address     parm;
  ushort        *parm_s = (ushort *)&parm;
  int           i;

  bzero(parm_s, sizeof(f_address));

  parm.cmd     = SET_FUNCTIONAL_ADDRESS;
  parm.retcode = 0xFF;
  for (i = 0; i < 4; i++)
        parm.f_addr[i] = addr[i];

  dump_cmd_to_adapter(p_dev_ctl, parm_s, sizeof(f_address));

} /* end of dump_set_functional_address */

/******************************************************************************/
/*
* NAME: dump_unset_group_address
*
* FUNCTION: Reset a previously set group address.
*
* EXECUTION ENVIRONMENT: process only
*
* NOTES:
*    Input:
*       p_dev_ctl       - pointer to device control structure.
*       addr            - address to be set
*
* RETURN:  0        = Successful completion
*          ENETDOWN = Error return
*/
/******************************************************************************/
void dump_unset_group_address (
register stok_dev_ctl_t  *p_dev_ctl,
uchar                   *addr)
{

  g_address     parm;
  ushort        *parm_s = (ushort *)&parm;
  int           rc = 0, i;

  bzero(parm_s, sizeof(g_address));

  parm.cmd     = RESET_GROUP_ADDRESS;
  parm.retcode = 0xFF;
  parm.num     = 0;
  parm.type    = 0;

  for (i = 0; i < 6; i++) {
        parm.g_addr[i] = *(addr + i);
  }

  dump_cmd_to_adapter(p_dev_ctl, parm_s, sizeof(g_address));

} /* end of dump_unset_group_address */

