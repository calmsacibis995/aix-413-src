static char sccsid[] = "@(#)29  1.3  src/rspc/kernext/pci/stok/stok_function.c, pcitok, rspc41J, 9520B_all 5/18/95 09:54:21";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: cmd_to_adapter
 *		config_hp_channel
 *		modify_receive_options
 *		open_adapter_cmd
 *		open_error
 *		read_adapter_log
 *		set_functional_address
 *		set_group_address
 *		set_ring_speed
 *		srb_response
 *		unset_group_address
 *              re_multi_add
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

#include "stok_dslo.h"
#include "stok_mac.h"
#include "stok_dds.h"
#include "stok_dd.h"
#include "stok_cmd.h"
#include "tr_stok_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

extern stok_dd_ctl_t stok_dd_ctl;
extern int stok_config();
extern struct cdt *cdt_func();

/******************************************************************************/
/*
* NAME: cmd_to_adapter
*
* FUNCTION: Sends a command to the adapter 
*
* EXECUTION ENVIRONMENT: process and interrupt
*
* CALLED FROM:
*	config_hp_channel
*	modify_receive_options
*	open_adapter_cmd
*	read_adapter_log
*	set_functional_address
*	set_group_address
*	unset_group_address
* 
* INPUT: 
*       p_dev_ctl  - pointer to device control structure.
*       parm_s     - pointer to the comman block
*       size       - size of the comman block   
*       at_int_lvl - flag for from interrupt level  
*
* RETURNS:
*      0 - OK
*      ENETDOWN - device is down
*
*/
/******************************************************************************/
void cmd_to_adapter (
stok_dev_ctl_t    	*p_dev_ctl,
ushort		    	*parm_s,
ushort  		size,
int 	  		at_int_lvl)
{

  register int  ioa, i;
  int 	        ipri;

  TRACE_SYS(STOK_OTHER, "IcmB", p_dev_ctl, parm_s, size);

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

  if (!at_int_lvl) {
  	w_start(&CMDWDT);

        /*
         * go to sleep with the cmd_slock freed up/regained.
         * Note that the ctl_lock remained locked during the whole time
         */
        WRK.ctl_event = EVENT_NULL;
        e_sleep_thread(&WRK.ctl_event, &CMD_LOCK, LOCK_HANDLER);

  } else {
       	io_delay(5000); 
  }
  iomem_det((void *)ioa);
  TRACE_SYS(STOK_OTHER, "ImrE", p_dev_ctl,  at_int_lvl, 0);

} /* end of cmd_to_adapter */

/******************************************************************************/
/*
* NAME: config_hp_channel
*
* FUNCTION:  This function tells the adapter to configure a high priority
*            transmit channel (transmit channel 1).
*
* EXECUTION ENVIRONMENT: interrupt only 
*
* INPUT:
*       p_dev_ctl 		- pointer to device control structure.
*       ioa          		- I/O bus address
*       access_priority       	- Token access priority
*
* CALLED FROM:
*  	srb_response
*
* CALLS TO: 
*       cmd_to_adapter
*
* RETURN:  
*
*/
/******************************************************************************/
void config_hp_channel (
  register stok_dev_ctl_t  *p_dev_ctl,
  register int         	ioa,
  uchar      	        access_priority) {

  cfg_hp_op   parm;
  ushort        *parm_s = (ushort *)&parm;
  ushort        rc;

  TRACE_SYS(STOK_OTHER, "IchB", p_dev_ctl, 0, 0);
  bzero(parm_s, sizeof(cfg_hp_op));

  parm.cmd     =  CONFIGURE_HP_CHANNEL;
  parm.retcode = 0xFF;
  parm.access_priority = access_priority;

  cmd_to_adapter(p_dev_ctl, parm_s, sizeof(cfg_hp_op), TRUE); 
  TRACE_SYS(STOK_OTHER, "IchE", p_dev_ctl, rc, WRK.pio_rc);

} /* end of config_hp_channel */

/******************************************************************************/
/*
* NAME: modify_receive_options
*
* FUNCTION: Modify the receive channel options 
*
* EXECUTION ENVIRONMENT: process and interrupt
*
* INPUT: 
*       p_dev_ctl  - pointer to device control structure.
*       rcv_op     - receive modify option
*       at_int_lvl - flag for running at interrupt or process level
*
* CALLED FROM: 
*  	stok_ctl
*
* CALLS TO: 
*       cmd_to_adapter
*
* RETURN:  0     = Successful completion 
*          other = Error return
*/
/******************************************************************************/
int modify_receive_options (
register stok_dev_ctl_t       	*p_dev_ctl,
ushort  			rcv_op,
int  				at_int_lvl)
{

  mod_recv_op   parm;
  ushort        *parm_s = (ushort *)&parm;
  ushort        rc;

  TRACE_SYS(STOK_OTHER, "ImrB", p_dev_ctl, rcv_op, at_int_lvl);
  if (p_dev_ctl->device_state != OPENED) {
	return (ENETDOWN);
  }

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

  cmd_to_adapter(p_dev_ctl, parm_s, sizeof(mod_recv_op), at_int_lvl);
  rc = WRK.retcode[MODIFY_RECEIVE_OPTION];

  TRACE_SYS(STOK_OTHER, "ImrE", p_dev_ctl, rc, at_int_lvl);
  return (rc);


} /* end of modify_receive_options */

/******************************************************************************/
/*
* NAME: open_adapter_cmd : Ready the adapter for normal comunication.
*
* FUNCTION: 
*
* EXECUTION ENVIRONMENT: interrupt only
*
* INPUT:
*       p_dev_ctl - pointer to device control structure.
*       ioa       - I/O bus address
*
* OUTPUT: 
*  	Connect to the network.
*   	
* CALLED FROM: 
*  	srb_response
*
* CALLS TO: 
*       cmd_to_adapter
* 
*/
/******************************************************************************/
void  open_adapter_cmd (
register stok_dev_ctl_t  *p_dev_ctl,
register int         	ioa)
{

  open_op	o_parm;
  ushort        *parm_s = (ushort *)&o_parm;
  ushort        id;
  register int  i, j,  rc;

  TRACE_SYS(STOK_OTHER, "IopB", p_dev_ctl, 0, 0);

  bzero(parm_s, sizeof(open_op));
  o_parm.cmd = OPEN_ADAPTER;

  if (DDS.attn_mac) {
  	/* set TOK ATTENTION MAC */
  	o_parm.option1 |= ATTENTION;
     	NDD.ndd_flags |= TOK_ATTENTION_MAC;
  }

  if (DDS.beacon_mac) { 
  	/* set TOK BEACON MAC */
  	o_parm.option1 |= BEACON_MAC;
     	NDD.ndd_flags |= TOK_BEACON_MAC;
  }

  /* set network speed */
  if (DDS.ring_speed == FOUR_MBS) {
  	o_parm.option2 = SPEED4;
  } else if (DDS.ring_speed == SIXTEEN_MBS) {
  	o_parm.option2 = SPEED16;
  }
  
  /* set network address */	
  if (DDS.use_alt_addr != 0) {
  	for (i = 0; i < 6; i++) {
  		o_parm.node_address[i] = WRK.stok_addr[i];
  	}
  	o_parm.number_local_addr = DDS.use_alt_addr;
  }


  cmd_to_adapter(p_dev_ctl, parm_s, sizeof(open_op), TRUE);

  TRACE_SYS(STOK_OTHER, "IopE", p_dev_ctl, 0, 0);

} /* end of open_adapter_cmd */

/******************************************************************************/
/*
* NAME: read_adapter_log
*
* FUNCTION: Read and reset the adapter error counters.
*
* EXECUTION ENVIRONMENT: process and interrupt
*
* INPUT:
*       p_dev_ctl   - pointer to device control structure.
*       at_int_lvl  - flag for running at interrupt or process level
*
* CALLED FROM: 
*  	stok_ctl
*
* CALLS TO: 
*       cmd_to_adapter
*
* RETURN:  0     = Successful completion 
*          other = Error return
*/
/******************************************************************************/
int read_adapter_log (
register stok_dev_ctl_t  *p_dev_ctl,
int  			at_int_lvl)
{

  read_log_op    parm;
  ushort        *parm_s = (ushort *)&parm;
  int		rc = 0;

  TRACE_SYS(STOK_OTHER, "IrlB", p_dev_ctl, at_int_lvl, 0);
  if (p_dev_ctl->device_state != OPENED) {
	return (ENETDOWN);
  }

  bzero(parm_s, sizeof(read_log_op));

  parm.cmd     = READ_LOG;
  parm.retcode = 0xFF;
 
  cmd_to_adapter(p_dev_ctl, parm_s, sizeof(read_log_op), at_int_lvl); 
  rc = WRK.retcode[READ_LOG];

  TRACE_SYS(STOK_OTHER, "IrlE", p_dev_ctl, at_int_lvl, 0);
  return(rc);

} /* end of read_adapter_log */

/******************************************************************************/
/*
* NAME: set_functional_address
*
* FUNCTION: Set the functional address for which the adapter will receive   
*           messages. 
*
* EXECUTION ENVIRONMENT: process and interrupt
*
* INPUT:
*       p_dev_ctl   - pointer to device control structure.
*       addr	    - address to be set
*       at_int_lvl  - flag for running at interrupt or process level
*
* CALLED FROM: 
*  	stok_ctl
*  	srb_response
*
* CALLS TO: 
*       cmd_to_adapter
*
* RETURN:  0     = Successful completion 
*          other = Error return
*/
/******************************************************************************/
int set_functional_address (
register stok_dev_ctl_t  *p_dev_ctl, 
uchar     		addr[4],
int  			at_int_lvl)
{
  f_address     parm;
  ushort        *parm_s = (ushort *)&parm;
  int 	        rc = 0, i;

  TRACE_SYS(STOK_OTHER, "IsfB", p_dev_ctl, 0, 0);
  bzero(parm_s, sizeof(f_address));

  parm.cmd     = SET_FUNCTIONAL_ADDRESS;
  parm.retcode = 0xFF;
  for (i = 0; i < 4; i++)
  	parm.f_addr[i] = addr[i];

  cmd_to_adapter(p_dev_ctl, parm_s, sizeof(f_address), at_int_lvl);
  rc = WRK.retcode[SET_FUNCTIONAL_ADDRESS];
 
  TRACE_SYS(STOK_OTHER, "IsfE", p_dev_ctl, rc, at_int_lvl);
  return (rc);

} /* end of set_functional_address */

/******************************************************************************/
/*
* NAME: set_group_address
*
* FUNCTION: Set a group address for which the adapter will receive messages. 
*
* EXECUTION ENVIRONMENT: process and interrupt
*
* INPUT: 
*       p_dev_ctl 	- pointer to device control structure.
*       addr		- address to be set
*       at_int_lvl      - flag for running at interrupt or process level
*
* CALLED FROM: 
*  	re_multi_add
*
* CALLS TO: 
*       cmd_to_adapter
*
* RETURN:  0     = Successful completion 
*          other = Error return
*/
/******************************************************************************/
int set_group_address (
register stok_dev_ctl_t  *p_dev_ctl, 
uchar  			*addr,
int  			at_int_lvl)
{

  g_address     parm;
  ushort        *parm_s = (ushort *)&parm;
  register int  ioa, i, x;
  ushort        id;
  int 	        ipri, rc = 0;

  TRACE_SYS(STOK_OTHER, "IrgB", p_dev_ctl, 0, 0);
  bzero(parm_s, sizeof(g_address));

  parm.cmd     = SET_GROUP_ADDRESS;
  parm.retcode = 0xFF;
  parm.num     = 0;
  parm.type    = 0;

  for (i = 0; i < 6; i++) {
  	parm.g_addr[i] = *(addr + i);
  }

  cmd_to_adapter(p_dev_ctl, parm_s, sizeof(g_address), at_int_lvl);
  rc = WRK.retcode[SET_GROUP_ADDRESS];

  TRACE_SYS(STOK_OTHER, "IrgE", p_dev_ctl, rc ,at_int_lvl);
  return(rc);
} /* end of set_group_address */

/******************************************************************************/
/*
* NAME: unset_group_address
*
* FUNCTION: Reset a previously set group address. 
*
* EXECUTION ENVIRONMENT: process and interrupt
*
* INPUT: 
*       p_dev_ctl 	- pointer to device control structure.
*       addr		- address to be set
*       at_int_lvl      - flag for running at interrupt or process level
*
* CALLED FROM: 
*  	stok_clt
*
* CALLS TO: 
*       cmd_to_adapter
*
* RETURN:  0        = Successful completion 
*          other = Error return
*/
/******************************************************************************/
int unset_group_address (
register stok_dev_ctl_t  *p_dev_ctl, 
uchar  			*addr,
int  			at_int_lvl)
{

  g_address     parm;
  ushort        *parm_s = (ushort *)&parm;
  int 	        rc = 0, i;

  TRACE_SYS(STOK_OTHER, "IrgB", p_dev_ctl, 0, 0);
  bzero(parm_s, sizeof(g_address));

  parm.cmd     = RESET_GROUP_ADDRESS;
  parm.retcode = 0xFF;
  parm.num     = 0;
  parm.type    = 0;

  for (i = 0; i < 6; i++) {
  	parm.g_addr[i] = *(addr + i);
  }

  cmd_to_adapter(p_dev_ctl, parm_s, sizeof(g_address), at_int_lvl);
  rc = WRK.retcode[RESET_GROUP_ADDRESS];

  TRACE_SYS(STOK_OTHER, "IrgE", p_dev_ctl, WRK.retcode[RESET_GROUP_ADDRESS], 0);
  return (rc);
} /* end of unset_group_address */

/******************************************************************************/
/*
* NAME: set_ring_speed
*
* EXECUTION ENVIRONMENT: process and interrupt
*
* INPUT: 
*       p_dev_ctl        - pointer to device control structure.
*       speed            - ring speed to set 
*
* CALLED FROM: 
*  	srb_response
*
*/
/******************************************************************************/
void set_ring_speed(
register stok_dev_ctl_t  *p_dev_ctl,
int  		          speed)	
{
  ndd_t        *p_ndd = &(NDD);

  if (speed == SPEED_16) {
        NDD.ndd_mtu = CTOK_16M_MAX_PACKET;
        MIB.Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_SIXTEENMEGABIT;
        NDD.ndd_flags |= TOK_RING_SPEED_16;
	NDD.ndd_flags &=  ~TOK_RING_SPEED_4;

  } else {
        NDD.ndd_mtu = CTOK_4M_MAX_PACKET;
        MIB.Token_ring_mib.Dot5Entry.ring_speed = TR_MIB_FOURMEGABIT;
        p_ndd->ndd_flags |= TOK_RING_SPEED_4;
	NDD.ndd_flags &=  ~TOK_RING_SPEED_16;
  }
}

/******************************************************************************/
/*
* NAME: open_error
*
* FUNCTION: Handle adapter open error code 
*
* EXECUTION ENVIRONMENT: interrupt only
*
* INPUT: 
*       p_dev_ctl        - pointer to device control structure.
*       open_error_code  - adapter open error code      
*       ioa              - I/O bus address
*
* CALLED FROM: 
*  	srb_response
*
* CALLS TO: 
*  	stok_enter_limbo
*       stok_bug_out
*       set_ring_speed	
*
*/
/******************************************************************************/
void open_error (
register stok_dev_ctl_t  *p_dev_ctl,
int  			open_error_code,
int  			ioa)
{

  ndd_t        *p_ndd = &(NDD);

  switch (open_error_code & 0xF) {
	case FUNCTION_FAILURE :
  	case WIRE_FAULT :
                if (WRK.limbo_err != 1) {
       			stok_logerr(p_dev_ctl, ERRID_STOK_WIRE_FAULT, __LINE__, 
				__FILE__, TOK_WIRE_FAULT, open_error_code, 0);
		}
		WRK.limbo_err = 1;
		stok_enter_limbo (p_dev_ctl, TRUE, TRUE, 30, TOK_WIRE_FAULT,
				 open_error_code, 0);
  		break;

  	case SIGNAL_LOS :
  	case TIME_OUT :
  	case RING_FAILURE :
  	case RING_BEACONING :
  	case REQUEST_PARAMETER :
                if (WRK.limbo_err != 2) {
  			stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_OPEN, __LINE__, 
				__FILE__, TOK_ADAP_OPEN, open_error_code, 0);
		}
		WRK.limbo_err = 2;
		stok_enter_limbo (p_dev_ctl, TRUE, TRUE, 30, TOK_WIRE_FAULT,
				 open_error_code, 0);
  		break;

  	case REMOVE_RECEIVED :
                if (WRK.limbo_err != 3) {
  			stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_OPEN, __LINE__, 
				__FILE__, TOK_RMV_ADAP, open_error_code, 0);
		}
		WRK.limbo_err = 3;
		stok_enter_limbo (p_dev_ctl, TRUE, TRUE, 30, TOK_WIRE_FAULT,
					 open_error_code, 0);
  		break;

  	case DUPLICATE_NODE_ADDRESS :
                if (WRK.limbo_err != 4) {
          		stok_logerr(p_dev_ctl, ERRID_STOK_DUP_ADDR, __LINE__, 
				   __FILE__, TOK_DUP_ADDR, open_error_code, 0);
		}
		WRK.limbo_err = 4;
		stok_bug_out(p_dev_ctl, TOK_DUP_ADDR, open_error_code,
				0, TRUE, FALSE, FALSE);

  		break;

  	case RING_SPEED_MISMATCH :
  		if (DDS.ring_speed == AUTOSENSE) {
  			if (p_ndd->ndd_flags & TOK_RING_SPEED_16) {
				DDS.ring_speed = FOUR_MBS;
				set_ring_speed(p_dev_ctl, SPEED_4);
  			} else {
				DDS.ring_speed = SIXTEEN_MBS;
				set_ring_speed(p_dev_ctl, SPEED_16);
  			}
  			open_adapter_cmd(p_dev_ctl, ioa);
		} else {
			stok_bug_out(p_dev_ctl, TOK_RING_SPEED, open_error_code,
					 0, TRUE, FALSE, FALSE);
          		stok_logerr(p_dev_ctl, ERRID_STOK_RING_SPEED, 
  					__LINE__, __FILE__, TOK_RING_SPEED, 
  					open_error_code, 0);
		}

  		break;
  	case NO_MONITOR_RPL :
  	case MONITOR_FAILED_RPL :
  		break;
  	default :
  		stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_OPEN, __LINE__, __FILE__,
				 TOK_ADAP_OPEN, open_error_code, 0);
  		break;
  	}
  }

/******************************************************************************/
/*
* NAME: srb_response
*
* FUNCTION: process the System Request Block(SRB) command response. 
*
* EXECUTION ENVIRONMENT: interrupt only
*
* INPUT: 
*       p_dev_ctl - pointer to device control structure.
*       ioa       - I/O bus address
*
* CALLED FROM: 
*  	stok_intr
*
* CALLS TO: 
*	set_ring_speed
*	config_hp_channel
* 	stok_enter_limbo
*	set_functional_address
* 	re_multi_add
*       cmd_to_adapter
*	open_error
*
*/
/******************************************************************************/
void srb_response(
register stok_dev_ctl_t  *p_dev_ctl,
int  			ioa)
{

  ndd_t        *p_ndd = &(NDD);
  ushort       open_error_code;
  ushort       command;
  ushort       data[10],i,j;
  ushort       func_addr_offset;
  ushort	Mcode_level_offset;
  ndd_statblk_t  stat_blk;   /* status block */
  int rc;
  int ipri;
  uchar init_status_h;

  TRACE_SYS(STOK_OTHER, "IsrB", p_dev_ctl, 0, 0);

  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i=0; i < 10; i++) {
  	PIO_GETSX(ioa + LAPD_I, &data[i]);
  }

  if (WRK.pio_rc) {
  	TRACE_BOTH(STOK_ERR, "Isr1", p_dev_ctl, WRK.pio_rc, 0);
        stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
			TRUE, FALSE, FALSE);
	return;
  }

  command = data[0] >> 8;
  WRK.retcode[command] = data[1] >> 8;

  w_stop(&CMDWDT);

  switch (command) {
  case INITIALIZATION_COMPLETE :

        TRACE_SYS(STOK_OTHER,"Isr2",p_dev_ctl,p_dev_ctl->device_state,
                                                        WRK.retcode[command]);
  	WRK.retcode[command] = data[3];
  	if (!WRK.retcode[command]) {

                func_addr_offset = data[6];
		Mcode_level_offset = data[5];
		/*
  		 * Sets the ring speed in MIB for autosense 
		 */
		init_status_h = data[0] & 0x00FF;
  		if (DDS.ring_speed == AUTOSENSE) {
  			if (init_status_h & SPEED_16) {
				set_ring_speed(p_dev_ctl, SPEED_16);
  			} else {
				set_ring_speed(p_dev_ctl, SPEED_4);
  			}
		}

		/*
  		 * Address of adapter parameters 
		 */
  		WRK.parms_addr = data[7];

		/*
  		 * Gets the Micro code level 
		 */
  		PIO_PUTSRX(ioa + LAPE, 0x00);
  		PIO_PUTSRX(ioa + LAPA, Mcode_level_offset);
  		for (i=0; i < 5; i++) {
  			PIO_GETSX(ioa + LAPD_I, &data[i]);
  		}
  		memcpy(VPD + 4, data, 10);

                if (WRK.pio_rc) {
                        TRACE_BOTH(STOK_ERR, "Isr3", p_dev_ctl, WRK.pio_rc, 0);
                        stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0,
                                                TRUE, FALSE, FALSE);
                        break;
                }

		/*
  		 * Sets Micro code level in MIB
		 */
  		for (i=0, j=0 ; j < 5; j++) {
  			MIB.Generic_mib.ifExtnsEntry.revware[i++]=data[j] >> 8;
  			MIB.Generic_mib.ifExtnsEntry.revware[i++]=data[j]& 0xff;
  		}

  		MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_NO_STATUS;
		
		if (DDS.priority_tx) {
			/* config high priority channel */
			config_hp_channel(p_dev_ctl,ioa,TOKEN_ACCESS_PRIORITY);
		} else {
  			open_adapter_cmd(p_dev_ctl, ioa);
		}

  	} else {
  		TRACE_BOTH(STOK_ERR,"Isr4",p_dev_ctl,p_dev_ctl->device_state,
							WRK.retcode[command]);
		stok_bug_out(p_dev_ctl, NDD_CMD_FAIL, command, 
				WRK.retcode[command], TRUE, FALSE, FALSE);
  		stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, __LINE__, 
  			   __FILE__, command, WRK.retcode[command], 0);

  		WRK.retcode[command] = ENETDOWN;
  	}
  	break;

  case CONFIGURE_HP_CHANNEL :
  	if (WRK.retcode[command] == OPERATION_SUCCESSFULLY) {
  		open_adapter_cmd(p_dev_ctl, ioa);

  	} else {
  		TRACE_BOTH(STOK_ERR,"Isr5",p_dev_ctl,p_dev_ctl->device_state,
							WRK.retcode[command]);
                if (WRK.limbo_err != 5) {
  			stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, __LINE__, 
  			   __FILE__, command, WRK.retcode[command], 0);
		};
		WRK.limbo_err = 5;
  		WRK.retcode[command] = ENETDOWN;
          	stok_enter_limbo(p_dev_ctl,TRUE,TRUE,0,NDD_CMD_FAIL,command,0);
  	}
  	break;

  case OPEN_ADAPTER :

	/*
  	 * Stops the watchdog timers 
	 */
  	w_stop(&(HWEWDT));

  	if (WRK.retcode[command] == OPERATION_SUCCESSFULLY) {

  		TRACE_SYS(STOK_OTHER,"Isr6",p_dev_ctl,p_dev_ctl->device_state,0);
		/*
  		 * Gets the adapter block address 
		 */
  		WRK.asb_address = data[4];
  		WRK.srb_address = data[5];
  		WRK.arb_address = data[6];
  		WRK.trb_address = data[8];

		/*
  		 * Checks if promiscouus mode need to be set
		 */
                if (WRK.promiscuous_count) {
        		NDD.ndd_flags |= NDD_PROMISC;
        		MIB.Generic_mib.ifExtnsEntry.promiscuous = PROMTRUE;
        		if (modify_receive_options(p_dev_ctl, PROMIS_ON, TRUE)){
  				TRACE_BOTH(STOK_ERR, "Isr8", p_dev_ctl, 
				      p_dev_ctl->device_state, open_error_code);
				return;
			}
                }

		/*
  		 * Sets the functional address 
		 */
                if (set_functional_address(p_dev_ctl, &FUNCTIONAL.functional[2],
			TRUE)) {
  			TRACE_BOTH(STOK_ERR, "Isr9", p_dev_ctl, 
				      p_dev_ctl->device_state, open_error_code);
			return;
		}
		/*
  		 * Sets the group address 
		 */
  		if (WRK.multi_count) {
                        if (re_multi_add( p_dev_ctl, p_ndd)) {
  				TRACE_BOTH(STOK_ERR, "Isra", p_dev_ctl, 
				      p_dev_ctl->device_state, open_error_code);
				return;
			}
		}

  		MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_NOPROBLEM;
                if (p_dev_ctl->device_state == LIMBO) {
                       	w_start(&(LANWDT)); 
                } else {
                        /*
                         * Updates the adapter device state & NDD flag
                         * and then wakeup the open process (in stok_act)
                         */
			WRK.limbo_err = 0;
                        p_dev_ctl->device_state = OPENED;
                        NDD.ndd_flags |= NDD_RUNNING;

			/*
  			 * Enables the Rx & TX channel 
			 */
  			PIO_PUTSRX(ioa + BMCTL_RUM, CHNL_ENABLE);

			if (stok_rx_enable(p_dev_ctl)) {
  				TRACE_BOTH(STOK_ERR, "Isr7", p_dev_ctl, 
				      p_dev_ctl->device_state, open_error_code);
				return;
			}

                        e_wakeup((int *)&WRK.ctl_event);
                }

  	} else if (WRK.retcode[command] == COMMAND_CANCELLED) {
  		open_error_code = data[3];
  		TRACE_BOTH(STOK_ERR,"Isrb",p_dev_ctl,p_dev_ctl->device_state,
							open_error_code);
		open_error(p_dev_ctl, open_error_code, ioa);

  	} else if (WRK.retcode[command] == INVALID_CONFIGURATION) {
                if (WRK.limbo_err != 6) {
          		stok_logerr(p_dev_ctl, ERRID_STOK_WIRE_FAULT, __LINE__, 
				   __FILE__, TOK_WIRE_FAULT, 0, 
				   WRK.retcode[command]); 
		}
		WRK.limbo_err = 6;
          	stok_enter_limbo(p_dev_ctl,TRUE,TRUE,30,NDD_CMD_FAIL,command,0);
  	} else {
          	stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_OPEN, __LINE__, 
			__FILE__, TOK_ADAP_OPEN, WRK.retcode[command], 0); 
		stok_bug_out(p_dev_ctl, TOK_DUP_ADDR, TOK_ADAP_OPEN, 
				WRK.retcode[command], TRUE, FALSE, FALSE);
          }	
  	break;
  case READ_LOG :
  	TRACE_SYS(STOK_OTHER,"Isrc",p_dev_ctl,p_dev_ctl->device_state,
							WRK.retcode[command]);
  	if (WRK.retcode[command] == OPERATION_SUCCESSFULLY) {
		/*
  		 * Adapter error counter 
		 */
  		bcopy((uchar *)&data[3], (uchar *)&WRK.adap_log, 
  		    sizeof(WRK.adap_log));

		/*
  		 * Updates the counter 
		 */
  		TOKSTATS.line_errs    +=WRK.adap_log.line_error;
  		TOKSTATS.burst_errs   +=WRK.adap_log.burst_error;
  		TOKSTATS.int_errs     +=WRK.adap_log.internal_error;
  		TOKSTATS.lostframes   +=WRK.adap_log.lost_frame;
  		TOKSTATS.rx_congestion+=WRK.adap_log.receive_congestion;
  		TOKSTATS.framecopies  +=WRK.adap_log.frame_copied_error;
  		TOKSTATS.token_errs   +=WRK.adap_log.token_error;
  	} else {
  		bzero((uchar *)&WRK.adap_log, sizeof(WRK.adap_log));
                if (WRK.limbo_err != 7) {
  			stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, __LINE__, 
  		    	   __FILE__, command, WRK.retcode[command], 0);
		}
		WRK.limbo_err = 7;
  		WRK.retcode[command] = ENETDOWN;
          	stok_enter_limbo(p_dev_ctl,TRUE,TRUE,0,NDD_CMD_FAIL,command,0);
  	}
  	break;

  case MODIFY_RECEIVE_OPTION :
  case RESET_GROUP_ADDRESS :
        TRACE_SYS(STOK_OTHER,"Isrd",p_dev_ctl,p_dev_ctl->device_state,
                                                        WRK.retcode[command]);
        if (WRK.retcode[command] != OPERATION_SUCCESSFULLY) {
                if (WRK.limbo_err != 8) {
                	stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, __LINE__,
                           __FILE__, command, WRK.retcode[command], 0);
		}
		WRK.limbo_err = 8;

                WRK.retcode[command] = ENETDOWN;
                stok_enter_limbo(p_dev_ctl,TRUE,TRUE, 0,NDD_CMD_FAIL,command,0);
        }
        break;

  case SET_FUNCTIONAL_ADDRESS :
  case SET_GROUP_ADDRESS :
        TRACE_SYS(STOK_OTHER,"Isrf",p_dev_ctl,p_dev_ctl->device_state,
                                                        WRK.retcode[command]);
        if (WRK.retcode[command] != OPERATION_SUCCESSFULLY) {
                if (WRK.limbo_err != 9) {
                	stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, __LINE__,
                           __FILE__, command, WRK.retcode[command], 0);
		}
		WRK.limbo_err = 9;

                if (WRK.retcode[command] != ALREADY_SET_GROUP) {
                        WRK.retcode[command] = ENETDOWN;
                        stok_enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_CMD_FAIL,
                                        command, 0);
                }
        }
        break;

  default :
  	TRACE_BOTH(STOK_ERR,"Isrg",p_dev_ctl,p_dev_ctl->device_state, command);
	if (command) {
        	stok_enter_limbo(p_dev_ctl,TRUE,TRUE,0,NDD_CMD_FAIL,command,0);
	}
  	break;
  } /* end of switch of command to adapter                             */

  if (p_dev_ctl->device_state != OPEN_PENDING) { 
      	ipri = disable_lock(PL_IMP, &CMD_LOCK);
   	e_wakeup((int *)&WRK.ctl_event);
        unlock_enable(ipri, &CMD_LOCK);
  }
  TRACE_SYS(STOK_OTHER, "IsrE", p_dev_ctl, command, WRK.retcode[command]);
} /* end of srb_response */


/*****************************************************************************/
/*
 * NAME:     re_multi_add
 *
 * FUNCTION:
 *      Re_add multicast addressess to adapter.
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
 * RETURNS:
 *      0      - successful
 *      other  - error
 */
/*****************************************************************************/
int re_multi_add(
  stok_dev_ctl_t  *p_dev_ctl,      /* point to the dev_ctl area              */
  ndd_t           *p_ndd)          /* pointer to the ndd in the dev_ctl area */


{

  stok_multi_t *p_multi = &WRK.multi_table;
  int   i;
  int   count = 0;

  TRACE_SYS(STOK_OTHER, "IraB", (ulong)p_dev_ctl, (ulong)p_ndd, 0);

  /*
   * Check if any group address to set
   */
  while (p_multi) {
        for (i=0; i < p_multi->in_use; i++) {
                if (set_group_address (p_dev_ctl, p_multi->m_slot[i].m_addr, 
			TRUE)) {
  			TRACE_BOTH(STOK_ERR, "Ira1", p_dev_ctl, p_ndd, count);
			return (count);
		}
                count++;
                if (count >= MAX_MULTI) {
                        break;
                }
        }

        p_multi = p_multi->next;
  }

  TRACE_SYS(STOK_OTHER, "IraE", p_dev_ctl, 0, 0);
  return(0);

}

