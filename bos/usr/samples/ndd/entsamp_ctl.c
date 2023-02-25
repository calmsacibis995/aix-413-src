static char sccsid[] = "@(#)41  1.1  src/bos/usr/samples/ndd/entsamp_ctl.c, entsamp, bos411, 9428A410j 1/14/94 13:39:51";
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_ctl
 *		entsamp_cmd
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
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/sleep.h>
#include <sys/err_rec.h>
#include <sys/dump.h>
#include <sys/mbuf.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>


#include "entsamp_mac.h"
#include "entsamp.h"



extern ethernet_all_mib_t entsamp_mib_status;



/*****************************************************************************/
/*
 * NAME:     entsamp_ctl
 *
 * FUNCTION: Ethernet driver ioctl routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ndd_ctl field in the NDD on the NDD chain.
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *	cmd		- control command
 *	arg		- pointer to the argument for the control command
 *	length		- length of the argument.
 *
 * RETURNS:  
 *	0 - OK
 *	ENETUNREACH - device is currently unreachable
 *	ENETDOWN - device is down
 *	EINVAL - invalid paramter
 *	ENOMEM - unable to allocate required memory
 *	EOPNOTSUPP - operation not supported
 */
/*****************************************************************************/
entsamp_ctl(
  ndd_t		*p_ndd,		/* pointer to the ndd in the dev_ctl area */
  int		cmd,		/* control command */
  caddr_t	arg,		/* argument of the control command */
  int		length)		/* length of the argument */
  

{
  entsamp_dev_ctl_t   *p_dev_ctl = (entsamp_dev_ctl_t *)(p_ndd->ndd_correlator);
  int rc = 0;		/* return code */
  int ipri;



  

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "IctB", (ulong)p_ndd, cmd, (ulong)arg);
  TRACE_SYS(HKWD_ENTSAMP_OTHER, "IctC", length, 0, 0);

/*
 * Lock the complex lock ctl_clock to lock out other ioctl to the same device.
 */

  lock_write(&CTL_LOCK);

  switch(cmd) {

/*
 * Get the generic statistics.
 * This is for ndd_genstats + ent_genstats.
 */
	case NDD_GET_STATS:
	{

		if (length != sizeof(ent_ndd_stats_t)) {
			rc = EINVAL;
			break;
		}
		/*
		 * Perform device-specific operations to gather various 
		 * statistics and put them in the device control table.
		 *
   		 * A section of device-specific code was omitted here.
		 */ 

         	/* copy statistics to user's buffer */
         	bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
         	bcopy(&(ENTSTATS), arg + sizeof(ndd_genstats_t), 
			sizeof(ent_genstats_t));

		break;
	}

/*
 * Get all of the device statistics. 
 * This is for ndd_genstats + ent_genstats + entsamp_stats
 */
	case NDD_GET_ALL_STATS:
	{

		if (length != sizeof(entsamp_all_stats_t)) {
			rc = EINVAL;
			break;
		}
		/*
		 * Perform device-specific operations to gather various 
		 * statistics and put them in the device control table.
		 *
   		 * A section of device-specific code was omitted here.
		 */ 

         	/* copy statistics to user's buffer */
         	bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
         	bcopy(&(ENTSTATS), arg + sizeof(ndd_genstats_t), 
			sizeof(ent_genstats_t) + sizeof(entsamp_stats_t));

		break;
	}

/*
 * Clear all of the statistics.
 */
	case NDD_CLEAR_STATS:
	{

		/*
		 * Perform device-specific operations to reset various 
		 * statistics counters either in the driver or on the
		 * device.
		 *
   		 * A section of device-specific code was omitted here.
		 */ 
		break;
	}

/*
 * Enable the adapter to receive all of the multicast packets on the network.
 */
	case NDD_ENABLE_MULTICAST:
	{

    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		WRK.enable_multi++;	/* inc the reference counter */

		if (WRK.enable_multi == 1) {
			p_ndd->ndd_flags |= NDD_MULTICAST;
			entsamp_cmd(p_dev_ctl, NDD_ENABLE_MULTICAST, FALSE);
		}
		break;
	}

/*
 * Disable the all multicast function. 
 */
	case NDD_DISABLE_MULTICAST:
	{
		if (!WRK.enable_multi) {
			rc = EINVAL;
			break;
		}

		WRK.enable_multi--;	/* dec the reference counter */
		if (!WRK.enable_multi) {
			p_ndd->ndd_flags &= ~NDD_MULTICAST;
			entsamp_cmd(p_dev_ctl, NDD_DISABLE_MULTICAST, FALSE);
		}
		break;
	}

/*
 * Enable the promiscuous mode. 
 */
	case NDD_PROMISCUOUS_ON:
	{
    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		WRK.promiscuous_count++;	/* inc the reference counter */
		if (WRK.promiscuous_count == 1) { 
			p_ndd->ndd_flags |= NDD_PROMISC;
			entsamp_cmd(p_dev_ctl, NDD_PROMISCUOUS_ON, FALSE);
		}
		break;
	}

/*
 * Disable the promiscuous mode. 
 */
	case NDD_PROMISCUOUS_OFF:
	{
		if (!WRK.promiscuous_count) {
			rc = EINVAL;
			break;
		}

		WRK.promiscuous_count--;	/* dev the reference counter */
		if (!WRK.promiscuous_count) {
			  p_ndd->ndd_flags &= ~NDD_PROMISC;
			  entsamp_cmd(p_dev_ctl, NDD_PROMISCUOUS_OFF, FALSE);
		}
		break;
	}

/*
 * Add a filter. Since the demuxer is managing all the type filtering,
 * this operation doesn't mean a whole lot to the driver. 
 * Increment a reference count and return.
 */
	case NDD_ADD_FILTER:
	{
		
    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}
		WRK.filter_count++;	/* inc the reference counter */

		break;
	}

/*
 * Delete a filter. The driver decrement the reference count and return.
 */
	case NDD_DEL_FILTER:
	{

		if (!WRK.filter_count) {
			rc = EINVAL;
			break;
		}

		WRK.filter_count--;	/* dec the reference counter */

		break;
	}

/*
 * Query the MIB support status on the driver.
 */
	case NDD_MIB_QUERY:
	{

    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		if (length != sizeof(ethernet_all_mib_t)) {
			rc = EINVAL;
			break;
		}

         	/* copy status to user's buffer */
         	bcopy(&entsamp_mib_status, arg, sizeof(ethernet_all_mib_t));
		break;

	}

/*
 * Get all MIB values.
 */
	case NDD_MIB_GET:
	{

		if (length != sizeof(ethernet_all_mib_t)) {
			rc = EINVAL;
			break;
		}

		/*
		 * Performs device-specific operations to gather various 
		 * statistics for the MIB variables.
   		 *
   		 * A section of device-specific code was omitted here.
		 */ 

         	/* copy mibs to user's buffer */
         	bcopy(&MIB, arg, sizeof(ethernet_all_mib_t));

		break;
	}

/*
 * Get receive address table (mainly for MIB variables). 
 * The receive address table is consists of all the addresses that the
 * adapter is armed to receive packets with. It includes the host
 * network address, the broadcast address and the currently registered
 * multicast addresses.
 */
	case NDD_MIB_ADDR:
	{

    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		if (length < sizeof(ndd_mib_addr_t)) {
			rc = EINVAL;
			break;
		}

		/*
		 * Performs device-specific operations to gather all 
		 * the current valid receive addresses for the device 
		 * and copy them into the user provided table.
   		 *
   		 * A section of device-specific code was omitted here.
		 */ 

		break;
	}

/*
 * Add an asynchronous status filter. This driver only track the
 * NDD_BAD_PKTS status filter. If this is the first time the bad packets
 * status is added, the driver will config the adapter to start receiving
 * bad packets on the network.
 */
	case NDD_ADD_STATUS:
	{
		
		ns_com_status_t *p_stat = (ns_com_status_t *)arg;


    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		if (p_stat->filtertype == NS_STATUS_MASK) {
			if (p_stat->mask == NDD_BAD_PKTS) {
				WRK.badframe_count++; /* inc reference count */
				if (WRK.badframe_count == 1) {
					p_ndd->ndd_flags |= ENT_RCV_BAD_FRAME;
					entsamp_cmd(p_dev_ctl, NDD_ADD_STATUS, 
						FALSE);
				}
			}
			else {
				WRK.otherstatus++; /* inc reference count */
			}
		}
		else {
			rc = EINVAL;
		}
		break;
	}

/*
 * Delete an asynchronous status filter. This driver only track the
 * NDD_BAD_PKTS status filter. If this is the last time the bad packets
 * status is deleted, the driver will config the adapter to stop receiving
 * bad packets on the network.
 */
	case NDD_DEL_STATUS:
	{
		
		ns_com_status_t *p_stat = (ns_com_status_t *)arg;


		if (p_stat->filtertype == NS_STATUS_MASK) {
			if (p_stat->mask == NDD_BAD_PKTS) {
				if (!WRK.badframe_count) {
					rc = EINVAL;
					break;
				}

				WRK.badframe_count--; /* dec reference count */
				if (!WRK.badframe_count) {
					p_ndd->ndd_flags &= ~ENT_RCV_BAD_FRAME;
					entsamp_cmd(p_dev_ctl, NDD_DEL_STATUS,
						FALSE);
				}
			}
			else {
				if (!WRK.otherstatus) {
					rc = EINVAL;
					break;
				}
				WRK.otherstatus--; /* dec reference count */
			}
				
		}
		else {
			rc = EINVAL;
		}
		break;
	}

/*
 * Add a multicast address to the multicast filter. 
 */
	case NDD_ENABLE_ADDRESS:
	{
		
    		if (p_dev_ctl->device_state != OPENED) {
			if (p_dev_ctl->device_state == DEAD) {
				rc = ENETDOWN;
			}
			else {
				rc = ENETUNREACH;
			}
			break;
    		}

		/* verify that it is a valid multicast address */
		if (*((char *)arg) & MULTI_BIT_MASK) {
		  /*
		   * Perform device-specific operations to add a multicast 
		   * address to the multicast filter on the device.
		   *
   		   * A section of device-specific code was omitted here.
   		   */

		}
		else {
		  rc = EINVAL;
		}
			
		break;
	}

/*
 * Delete a multicast address from the multicast filter. 
 */
	case NDD_DISABLE_ADDRESS:
	{

		/* verify that it is a valid multicast address */
		if (*((char *)arg) & MULTI_BIT_MASK) {

		  /* 
		   * Perform device-specific operations to remove a 
		   * multicast address from the multicast filter on the device.
                   *
                   * A section of device-specific code was omitted here.
		   */ 
		}
		else 
		  rc = EINVAL;
				
		break;
	}

	default:
		rc = EOPNOTSUPP;
  		TRACE_SYS(HKWD_ENTSAMP_ERR, "Ict3", rc, cmd, 0);
		break;

		
  }

  lock_done(&CTL_LOCK);
  TRACE_SYS(HKWD_ENTSAMP_OTHER, "IctE", rc, 0, 0);
  return(rc);


}

/*****************************************************************************/
/*
 * NAME:     entsamp_cmd
 *
 * FUNCTION: Issue device-specific control commands to the adapter.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_ctl
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *	cmd		- adapter control command
 *	at_int_lvl	- flag for running at interrupt or process level
 *
 * RETURNS:  
 *	0 - OK
 *	ENETUNREACH - device is currently unreachable
 *	ENETDOWN - device is down
 *	EOPNOTSUPP - operation not supported
 */
/*****************************************************************************/
entsamp_cmd(
  entsamp_dev_ctl_t	*p_dev_ctl,	/* pointer to device control area */
  int			cmd, 		/* adapter control command */
  int 			at_int_lvl)	/* flag for intr/proc level */
  
{


  int ipri;



  TRACE_SYS(HKWD_ENTSAMP_OTHER, "IcmB", (ulong)p_dev_ctl, cmd, at_int_lvl);

  /*
   * Get the cmd_lock and check the device state, make sure that the 
   * error recovery has not occurred.
   */
  if (!at_int_lvl) {
    ipri = disable_lock(PL_IMP, &CMD_LOCK);
    if (p_dev_ctl->device_state != OPENED && 
	p_dev_ctl->device_state != OPEN_PENDING) {
	if (p_dev_ctl->device_state == DEAD) {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_ENTSAMP_ERR, "Icm1", ENETDOWN, 
			p_dev_ctl->device_state, 0);
		return(ENETDOWN);
	}
	else {
  		unlock_enable(ipri, &CMD_LOCK);
  		TRACE_SYS(HKWD_ENTSAMP_ERR, "Icm2", ENETUNREACH, 
			p_dev_ctl->device_state, 0);
		return(ENETUNREACH);
	}
    }
  }

  /*
   * According to the cmd parameter, perform various device-specific
   * control operations to the adapter.
   *
   * A section of device-specific code was omitted here.
   */



  if (!at_int_lvl) {
  	p_dev_ctl->ctl_pending = TRUE;

  	/* set null event to sleep on */
  	p_dev_ctl->ctl_event = EVENT_NULL;

  	/* start the ioctl timer */
  	w_start(&(CTLWDT));

  	/*
   	 * go to sleep with the cmd_slock freed up/regained. 
   	 * Note that the ctl_slock remained locked during the whole time
   	 */

  	e_sleep_thread(&p_dev_ctl->ctl_event, &CMD_LOCK, LOCK_HANDLER);

  	p_dev_ctl->ctl_pending = FALSE;
  }

		
  if (!at_int_lvl)
  	unlock_enable(ipri, &CMD_LOCK);

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "IcmE", (ulong)p_dev_ctl->ctl_status, 0, 0);
  return(p_dev_ctl->ctl_status);


}
