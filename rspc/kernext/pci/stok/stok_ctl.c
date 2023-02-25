static char sccsid[] = "@(#)43  1.3  src/rspc/kernext/pci/stok/stok_ctl.c, pcitok, rspc41J, 9516A_all 4/14/95 15:34:49";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: get_mib
 *		get_stats
 *		multi_add
 *		multi_del
 *		stok_ctl
 *		turn_promis_off
 *		turn_promis_on
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
#include <sys/iocc.h>
#include <sys/sleep.h>
#include <sys/err_rec.h>
#include <sys/errids.h>
#include <sys/dump.h>
#include <sys/mbuf.h>
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

extern int stok_priority (ndd_t *p_ndd, struct mbuf *p_mbuf);
extern int stok_dump     (ndd_t *p_ndd, int cmd, caddr_t arg);
/*
 *  MIB status table - this table defines the MIB variable status returned
 *  on MIB query operation.
 */
token_ring_all_mib_t stok_mib_status = {

  /*
   * Generic Interface Extension Table
   */
  MIB_READ_ONLY,                 /* ifExtnsChipSet                  */
  MIB_READ_ONLY,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   /* ifExtnsRevWare  */
  MIB_READ_ONLY,                 /* ifExtnsMulticastsTransmittedOks */
  MIB_READ_ONLY,                 /* ifExtnsBroadcastsTransmittedOks */
  MIB_READ_ONLY,                 /* ifExtnsMulticastsReceivedOks    */
  MIB_READ_ONLY,                 /* ifExtnsBroadcastsReceivedOks    */
  MIB_READ_ONLY,                 /* ifExtnsPromiscuous              */

  /*
   * Generic Interface Test Table
   */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestCommunity            */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestRequestId            */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestType                 */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestResult               */
  MIB_NOT_SUPPORTED,              /* ifEXtnsTestCode                 */

  /*
   * Generic Receive Address Table
   */
  MIB_READ_ONLY,                  /* ifExtnsRcvAddress               */

  /*
   * The TOK Interface Group
   */
  MIB_READ_ONLY,                 /* dot5Commands;                   */
  MIB_READ_ONLY,                 /* dot5RingStatus;                 */
  MIB_READ_ONLY,                 /* dot5RingState;                  */
  MIB_READ_ONLY,                 /* dot5RingOpenStatus;             */
  MIB_READ_ONLY,                 /* dot5RingSpeed;                  */
  MIB_READ_ONLY,0,0,0,0,0,       /* dot5UpStream;                   */
  MIB_READ_ONLY,                 /* dot5ActMonParticipate;          */
  MIB_READ_ONLY,0,0,0,0,0,       /* dot5Functional;                 */

  /*
   * The TOK Statistics Group
   */
  MIB_READ_ONLY,                  /* dot5StatsLineErrors;            */
  MIB_READ_ONLY,                  /* dot5StatsBurstErrors;           */
  MIB_READ_ONLY,                  /* dot5StatsACErrors;              */
  MIB_READ_ONLY,                  /* dot5StatsAbortTransErrors;      */
  MIB_READ_ONLY,                  /* dot5StatsInternalErrors;        */
  MIB_READ_ONLY,                  /* dot5StatsLostFrameErrors;       */
  MIB_READ_ONLY,                  /* dot5StatsReceiveCongestions;    */
  MIB_READ_ONLY,                  /* dot5StatsFrameCopiedErrors;     */
  MIB_READ_ONLY,                  /* dot5StatsTokenErrors;           */
  MIB_READ_ONLY,                  /* dot5StatsSoftErrors;            */
  MIB_READ_ONLY,                  /* dot5StatsHardErrors;            */
  MIB_READ_ONLY,                  /* dot5StatsSignalLoss;            */
  MIB_READ_ONLY,                  /* dot5StatsTransmitBeacons;       */
  MIB_READ_ONLY,                  /* dot5StatsRecoverys;             */
  MIB_READ_ONLY,                  /* dot5StatsLobeWires;             */
  MIB_READ_ONLY,                  /* dot5StatsRemoves;               */
  MIB_READ_ONLY,                  /* dot5StatsSingles;               */
  MIB_READ_ONLY,                  /* dot5StatsFreqErrors;            */

  /*
   * The TOK Timer Group
   */
  MIB_NOT_SUPPORTED,              /* dot5TimerReturnRepeat;          */
  MIB_NOT_SUPPORTED,              /* dot5TimerHolding;               */
  MIB_NOT_SUPPORTED,              /* dot5TimerQueuePDU;              */
  MIB_NOT_SUPPORTED,              /* dot5TimerValidTransmit;         */
  MIB_NOT_SUPPORTED,              /* dot5TimerNoToken;               */
  MIB_NOT_SUPPORTED,              /* dot5TimerActiveMon;             */
  MIB_NOT_SUPPORTED,              /* dot5TimerStandbyMon;            */
  MIB_NOT_SUPPORTED,              /* dot5TimerErrorReport;           */
  MIB_NOT_SUPPORTED,              /* dot5TimerBeaconTransmit;        */
  MIB_NOT_SUPPORTED,              /* dot5TimerBeaconReceive;         */
};

/*****************************************************************************/
/*
 * NAME:     stok_ctl
 *
 * FUNCTION: Skyline driver ioctl routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM:
 *      NS user by using the ndd_ctl field in the NDD on the NDD chain.
 *
 * CALLS TO:
 *	read_adapter_log
 *	get_stats
 *	turn_promis_off
 *	turn_promis_on
 *	multi_add
 *	multi_del
 *	set_function_address
 *
 * INPUT:
 *  p_ndd               - pointer to the ndd in the dev_ctl area
 *  cmd                 - control command
 *  arg                 - argument of the control command
 *  length              - length of the argument
 *
 * RETURNS:
 *      0           - successful
 *      ENETUNREACH - device is currently unreachable
 *      ENETDOWN    - device is down
 *      EINVAL      - invalid paramter
 *      ENOMEM      - unable to allocate required memory
 *      EOPNOTSUPP  - operation not supported
 */
/*****************************************************************************/
stok_ctl(
ndd_t   *p_ndd,         /* pointer to the ndd in the dev_ctl area */
int     cmd,            /* control command */
caddr_t arg,            /* argument of the control command */
int     length)         /* length of the argument */


{
  stok_dev_ctl_t   *p_dev_ctl = (stok_dev_ctl_t *)(p_ndd->ndd_correlator);
  int           rc = 0;         /* return code */
  int           ipri;
  int           i,j;
  ushort        rcv_op;
  uchar bcast1_addr[CTOK_NADR_LENGTH] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  uchar bcast2_addr[CTOK_NADR_LENGTH] = { 0xC0,0x00,0xFF,0xFF,0xFF,0xFF};

  ndd_mib_addr_t  *p_table = (ndd_mib_addr_t *)arg;
  ndd_mib_addr_elem_t  *p_elem;
  stok_multi_t *p_multi = &WRK.multi_table;
  int elem_len = sizeof(ndd_mib_addr_elem_t) + CTOK_NADR_LENGTH - 2;
  int count = 0;

  TRACE_SYS(STOK_OTHER, "IctB", (ulong)p_dev_ctl, cmd, (ulong)arg);
  TRACE_SYS(STOK_OTHER, "Ictc", (ulong)p_dev_ctl, length, 
					(ulong)p_dev_ctl->device_state);

  /*
   * Locks the complex lock ctl_clock to locks out other ioctl 
   * to the same device.
   */
  ipri = disable_lock(PL_IMP, &CMD_LOCK);

  /*
   * if the driver is in a power management mode of suspend/hibernate then put
   * the caller to sleep until the driver resumes normal operation except for
   * open this needs to be done under a lock so that the state check is valid
   */
  if (p_dev_ctl->device_state == SUSPEND_STATE) {
        e_sleep_thread (&WRK.pmh_event, &CMD_LOCK, LOCK_HANDLER);
  }

  switch(cmd) {

  /*
   * Returns the address of the device driver's dump routine.
   */
  case NDD_DUMP_ADDR:
        if (arg == 0) {
                rc = EINVAL;
                TRACE_BOTH(STOK_ERR, "Ict1", p_dev_ctl, 0, 0);
                break;
        }
        *(uint *)arg = (uint)stok_dump;
        break;

  /*
   * Returns the address of the device driver's priority transmit routine.
   */
  case NDD_PRIORITY_ADDR:
        if (arg == 0) {
                rc = EINVAL;
                TRACE_BOTH(STOK_ERR, "Ict2", p_dev_ctl, 0, 0);
                break;
        }
       	*(uint *)arg = (uint)stok_priority;
        break;

  /*
   * clears all the statistics.
   */
  case NDD_CLEAR_STATS:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(STOK_ERR, "Ict3", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(STOK_ERR, "Ict4", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
        	break;
  	}


        /*
         * Clears the statistics from the adapter
         */
        if (read_adapter_log(p_dev_ctl, FALSE)) {
                rc = ENETUNREACH;
                break;
	}

  	/*
  	 * Saves the ndd start time for statistics
  	 * and clears the counters
  	 */
	CLEAR_STATS();
        break;

  /*
   * Gets all of the device statistics.
   */
  case NDD_GET_ALL_STATS:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(STOK_ERR, "Ict5", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(STOK_ERR, "Ict6", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
        	break;
  	}

        if (length != (sizeof(sky_all_stats_t))) { 
                rc = EINVAL;
                TRACE_BOTH(STOK_ERR, "Ict7", p_dev_ctl, 0, 0);
                break;
        }

	if (get_stats(p_dev_ctl)) {
                rc = ENETUNREACH;
                break;
	}
 
        /*
         * Copys the statistics to user's buffer 
       	 */
        bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
        bcopy(&TOKSTATS, arg + sizeof(ndd_genstats_t),
                sizeof(tok_genstats_t) + sizeof(tr_sky_stats_t)); 
        break;

  /*
   * Gets the statistics.
   */
  case NDD_GET_STATS:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(STOK_ERR, "Ict8", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(STOK_ERR, "Ict9", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
        	break;
  	}


        if (length != sizeof(tok_ndd_stats_t)) {
                rc = EINVAL;
                TRACE_BOTH(STOK_ERR, "Icta", p_dev_ctl, 0, 0);
                break;
        }

	if (get_stats(p_dev_ctl)) {
                rc = ENETUNREACH;
                break;
	}

	/*
         * Copys the statistics to user's buffer 
     	 */
        bcopy(&NDD.ndd_genstats, arg, sizeof(ndd_genstats_t));
        bcopy(&TOKSTATS, arg + sizeof(ndd_genstats_t),
                sizeof(tok_genstats_t));
        break;

  /*
   * Enables the promiscuous mode.  If this is the first promiscuous on
   * operation and the adapter is not in promiscuous mode already, the
   * driver will configure the adapter to run in the promiscuous mode.
   */
  case NDD_PROMISCUOUS_ON:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(STOK_ERR, "Ictb", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(STOK_ERR, "Ictd", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
        	break;
  	}

	turn_promis_on(p_dev_ctl);
        break;

  /*
   * Disables the promiscuous mode.  If this is the last promiscuous off
   * operation and there is no other reason to stay in the promiscuous mode
   * the adapter will be re-configured to get out of the promiscuous mode.
   */
  case NDD_PROMISCUOUS_OFF:
	if (turn_promis_off(p_dev_ctl)) {
                rc = EINVAL;
                TRACE_BOTH(STOK_ERR, "Icte", p_dev_ctl, 0, 0);
        }
        break;

  /*
   * Gets receive address table (mainly for MIB variables).  The receive
   * address table is consists of all the addresses that the adapter is
   * armed to receive packets with.  It includes the host network address,
   * the broadcast address and the currently registered multicast addresses.
   * This operation doesn't report the device state.
   */
   case NDD_MIB_ADDR:
       if (arg == 0) {
                rc = EINVAL;
                break;
        }

  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(STOK_ERR, "Ictf", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(STOK_ERR, "Ictg", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
                break;
  	}

       if (length < sizeof(ndd_mib_addr_t)) {
                rc = EINVAL;
                TRACE_BOTH(STOK_ERR, "Icth", p_dev_ctl, 0, 0);
                break;
        }

        length -= sizeof(u_int);   /* reserve room for the count field */
        arg += 4;

	/*
         * Copys the specific network address in use first 
	 */
        if (length >= elem_len) {
                p_elem = (ndd_mib_addr_elem_t *)arg;
                p_elem->status = NDD_MIB_VOLATILE;
                p_elem->addresslen = CTOK_NADR_LENGTH;
                COPY_NADR(WRK.stok_addr, p_elem->address);
                length -= elem_len;
                arg += elem_len;
                count++;
        } else {
                rc = E2BIG;
        }

	/*
         * Copys the first broadcast address 
	 */
        if (length >= elem_len) {
                p_elem = (ndd_mib_addr_elem_t *)arg;
                p_elem->status = NDD_MIB_NONVOLATILE;
                p_elem->addresslen = CTOK_NADR_LENGTH;
                COPY_NADR(bcast1_addr, p_elem->address);
                length -= elem_len;
                arg += elem_len;
                count++;
        } else {
                rc = E2BIG;
        }

	/*
         * Copys the second broadcast address 
	 */
        if (length >= elem_len) {
                p_elem = (ndd_mib_addr_elem_t *)arg;
                p_elem->status = NDD_MIB_NONVOLATILE;
                p_elem->addresslen = CTOK_NADR_LENGTH;
                COPY_NADR(bcast2_addr, p_elem->address);
                length -= elem_len;
                arg += elem_len;
                count++;
        } else {
                rc = E2BIG;
        }

        if ( NDD.ndd_flags & TOK_RECEIVE_FUNC) {
		/*
        	 * Copies the functional address 
		 */
        	if (length >= elem_len) {
                	p_elem = (ndd_mib_addr_elem_t *)arg;
                	p_elem->status = NDD_MIB_VOLATILE;
                	p_elem->addresslen = CTOK_NADR_LENGTH;
                	COPY_NADR(FUNCTIONAL.functional, p_elem->address);
                	length -= elem_len;
                	arg += elem_len;
                	count++;
        	} else {
                	rc = E2BIG;
        	}
	}

	/*
         * Copys the multicast addresses 
	 */
        while (p_multi) {
                for (i=0; i < p_multi->in_use; i++) {
                        if (length >= elem_len) {
                                p_elem = (ndd_mib_addr_elem_t *)arg;
                                p_elem->status = NDD_MIB_VOLATILE;
                                p_elem->addresslen = CTOK_NADR_LENGTH;

                                COPY_NADR(p_multi->m_slot[i].m_addr,
                                                        p_elem->address);
                                length -= elem_len;
                                arg += elem_len;
                                count++;
                        } else {
                                rc = E2BIG;
                                break;
                        }
                }
                if (i < p_multi->in_use) {
                        break;
		}

                p_multi = p_multi->next;
        }

	/*
         * Puts the final count into the buffer 
 	 */
        p_table->count = count;

        break;

  /*
   * Queries MIB support status on the driver.
   */
  case NDD_MIB_QUERY:
       if (arg == 0) {
                rc = EINVAL;
                break;
        }

  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(STOK_ERR, "Icti", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(STOK_ERR, "Ictj", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
                break;
  	}


        if (length != sizeof(token_ring_all_mib_t)) {
                rc = EINVAL;
                TRACE_BOTH(STOK_ERR, "Ictk", p_dev_ctl, 0, 0);
                break;
        }
	/*
         * Copys the status to user's buffer 
	 */
        bcopy(&stok_mib_status, arg, sizeof(token_ring_all_mib_t));
        break;

  /*
   * Gets all MIB values.
   */
  case NDD_MIB_GET:
       if (arg == 0) {
                rc = EINVAL;
                break;
        }

  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(STOK_ERR, "Ictl", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
        	} else {
                	TRACE_BOTH(STOK_ERR, "Ictm", p_dev_ctl, ENETUNREACH, 0);
                	rc = ENETUNREACH;
        	}
                break;
  	}


        if (length != sizeof(token_ring_all_mib_t)) {
                rc = EINVAL;
                TRACE_BOTH(STOK_ERR, "Icto", p_dev_ctl, 0, 0);
                
        }

        /*
         * Gathers the current MIB statistics
         */
        if (get_mib(p_dev_ctl)) {
                TRACE_BOTH(STOK_ERR, "Ictp", p_dev_ctl, 0, 0);
                rc = ENETDOWN;
                break;
	}

	/*
         * Copys mibs to user's buffer 
	 */
        bcopy(&MIB, arg, sizeof(token_ring_all_mib_t));

        break;

  /*
   * Sets the alternate address
   */
  case NDD_ENABLE_ADDRESS:
  	if (p_dev_ctl->device_state != OPENED) {
        	if (p_dev_ctl->device_state == DEAD) {
                	TRACE_BOTH(STOK_ERR, "Ictq", p_dev_ctl, ENETDOWN, 0);
                	rc = ENETDOWN;
                	break;
        	}
  	}

        TRACE_DBG(STOK_OTHER, "Ictr", p_dev_ctl, NDD.ndd_flags, 0);

        if (*((char *)(arg + 2)) & MULTI_BIT_MASK) {
        	/*
        	 * Sets the group address
        	 */
                if (rc = multi_add(p_dev_ctl, p_ndd, arg))
                {
                        TRACE_BOTH(STOK_ERR, "Icts", p_dev_ctl,
                                        NDD.ndd_flags, rc);
                } else {
                        NDD.ndd_flags |= NDD_ALTADDRS | TOK_RECEIVE_GROUP;
                        TRACE_DBG(STOK_OTHER, "Ictt", p_dev_ctl,
                                        NDD.ndd_flags, 0);
                }
        } else {
		/*
		 * Check if it is a functional address
                 */
        	if (*((ushort *)arg) != 0xC000) { 
                	rc = EINVAL;
			break;
		}

                /*
                 * Keeps a reference count on each of the bits in the address
                 */
                for (i = 0, j = 1; i < 32; i++, j <<= 1) {
                          if ( *((uint *) (arg + 2)) & j) {
                                WRK.func_ref_cnt[i]++;
                          }
                  }

		if ((FUNCTIONAL.functional[2] == *(arg + 2)) &
			(FUNCTIONAL.functional[3] == *(arg + 3)) &
			(FUNCTIONAL.functional[4] == *(arg + 4)) &
			(FUNCTIONAL.functional[5] == *(arg + 5))) {
			break;
		}

		/*
                 * Updates the functional address 
		 */
                FUNCTIONAL.functional[2] |= *(arg + 2);
                FUNCTIONAL.functional[3] |= *(arg + 3);
                FUNCTIONAL.functional[4] |= *(arg + 4);
                FUNCTIONAL.functional[5] |= *(arg + 5);
                NDD.ndd_flags |= NDD_ALTADDRS | TOK_RECEIVE_FUNC;

  		if (p_dev_ctl->device_state == OPENED) {
                	if (rc = set_functional_address( p_dev_ctl,
                                &FUNCTIONAL.functional[2], FALSE)) {
                		TRACE_BOTH(STOK_ERR, "Ictu", p_dev_ctl, 0, 0);
			}
		}
        }

        break;    /* end of NDD_ENABLE_ADDRESS */

  /*
   * Resets the alternate address
   */
  case NDD_DISABLE_ADDRESS:
        if (*((char *)(arg + 2)) & MULTI_BIT_MASK) {
                /*
                 * Resets the group address
                 */
                if (multi_del(p_dev_ctl, p_ndd, arg)) {
                	rc = EINVAL;
			break;
		}
                if (!WRK.multi_count) {
                        NDD.ndd_flags &= ~TOK_RECEIVE_GROUP;
                	if (!(NDD.ndd_flags & TOK_RECEIVE_FUNC)) {
                        	NDD.ndd_flags &= ~NDD_ALTADDRS;
			}
		}

        } else {
		/*
		 * Check if it is a functional address
                 */
        	if (*((ushort *)arg) != 0xC000) { 
                	rc = EINVAL;
			break;
		}

                /*
                 * Resets the functional address
                 * 32nd bit defined group/functional addr - don't test this bit
                 */
                /* First, Ensure we have a valid address */
                for (i = 0, j = 1; i < 31; i++, j <<= 1) {
                         if ( *((uint *) (arg + 2)) & j) {
                                if (! WRK.func_ref_cnt[i]) {
                                        /* trying to disable a nonenabled bit */
                			TRACE_BOTH(STOK_ERR, "Ictw", p_dev_ctl,
							 (*(uint *)(arg+2)), 0);
                                        rc = EINVAL;
                                        break;
                                }
                          }
                }

                if (rc) { 
			break; 
		}

                /* 
		 * Then, Decrement bitwise reference count and (maybe) clear 
                 * Function address bits 
		 */

                for (i = 0, j = 1; i < 31; i++, j <<= 1) {
                         if ( *((uint *) (arg + 2)) & j) {
                                WRK.func_ref_cnt[i]--;
                                if (! WRK.func_ref_cnt[i]) {
                                     *(uint *)(&FUNCTIONAL.functional[2]) &= ~j;
                                }
                          }
                }

                /*
                 * If not receiving data for any functional addresses and
                 * any group addresses then NDD_ALTADDRS is off.
                 */
                if (! (*(uint *)(&FUNCTIONAL.functional[2])) ) {
                          NDD.ndd_flags &= ~TOK_RECEIVE_FUNC;
                	  if (!(NDD.ndd_flags & TOK_RECEIVE_GROUP)) {
                                  NDD.ndd_flags &= ~NDD_ALTADDRS;
			  }
                }

  		if (p_dev_ctl->device_state == OPENED) {
                	if (rc = set_functional_address( p_dev_ctl,
                                &FUNCTIONAL.functional[2], FALSE)) {
                		TRACE_BOTH(STOK_ERR, "Ictx", p_dev_ctl, rc, 0);
			}
		} else if (p_dev_ctl->device_state == LIMBO) {
			WRK.re_limbo = TRUE;
		}

  	}

        break;    /* end of NDD_DISABLE_ADDRESS */

  default:
        rc = EOPNOTSUPP;
        break;
  }

  unlock_enable(ipri, &CMD_LOCK);

  if (WRK.pio_rc) {
        TRACE_BOTH(STOK_ERR, "Icty", p_dev_ctl, WRK.pio_rc, 0);
        stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 0, 0, TRUE);
        return (WRK.pio_rc);
  }

  TRACE_SYS(STOK_OTHER, "IctE", p_dev_ctl, rc, 0);
  return(rc);

}

/*****************************************************************************/
/*
 * NAME:     get_stats
 *
 * FUNCTION: Gather the current statistics 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * CALLED FROM:
 *      stok_ctl
 *
 * CALLS TO:
 *      read_adapter_log
 *
 * RETURNS:
 *      OK      - successful
 *	ERROR 	- FALSE
 */
/*****************************************************************************/
int get_stats(
  stok_dev_ctl_t      *p_dev_ctl)     /* pointer to device control area */

{

  TRACE_SYS(STOK_OTHER, "IgsB", p_dev_ctl, 0, 0);

  COPY_NADR (WRK.stok_addr, TOKSTATS.tok_nadr);
  NDD.ndd_genstats.ndd_elapsed_time = NDD_ELAPSED_TIME(WRK.ndd_stime);
  TOKSTATS.dev_elapsed_time = NDD_ELAPSED_TIME(WRK.dev_stime);
  TOKSTATS.ndd_flags = NDD.ndd_flags;
  TOKSTATS.sw_txq_len = TX2.sw_txq_len;
  TOKSTATS.hw_txq_len = TX2.tx_frame_pending;
  TOKSTATS.device_type = TOK_SKY; 
  NDD.ndd_genstats.ndd_xmitque_cur = TX2.sw_txq_len + TX2.tx_frame_pending;

  /* temp use reserved for priority queue */
  TOKSTATS.reserve3 = TX1.sw_txq_len;
  TOKSTATS.reserve4 = TX1.tx_frame_pending;
  TOKSTATS.reserve1 = TX1.sw_txq_len + TX1.tx_frame_pending;

  /*
   * Gets the statistics from the adapter
   */
  if (read_adapter_log(p_dev_ctl, FALSE)) {
          TRACE_BOTH(STOK_ERR, "Igs1", p_dev_ctl, 0, 0);
	  return (ERROR);
  }

  TRACE_SYS(STOK_OTHER, "IgsE", p_dev_ctl, 0, 0);
  return (OK);
}
/*****************************************************************************/
/*
 * NAME:     get_mib
 *
 * FUNCTION: Gather the current statistics from the adapter to the MIB table
 *
 * EXECUTION ENVIRONMENT: process only
 * 
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * CALLED FROM:
 *      stok_ctl
 *
 * CALLS TO:
 *      read_adapter_log
 *
 * RETURNS:
 *      0       - successful
 *	1	- FALSE
 */
/*****************************************************************************/
int get_mib(
  stok_dev_ctl_t      *p_dev_ctl)     /* pointer to device control area */

{

  int     ioa, i,j,rc;
  ushort  addr;

  TRACE_SYS(STOK_OTHER, "ImbB", p_dev_ctl, 0, 0);

  /*
   * Gets the statistics from the adapter
   */
  if (rc = read_adapter_log(p_dev_ctl, FALSE)) {
        TRACE_BOTH(STOK_ERR, "Imb1", p_dev_ctl, rc, 0);
	return (ERROR);
  }

  bcopy(TR_MIB_IBM16,MIB.Generic_mib.ifExtnsEntry.chipset, CHIPSETLENGTH);

  MIB.Token_ring_mib.Dot5StatsEntry.line_errs = TOKSTATS.line_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.burst_errs = TOKSTATS.burst_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.int_errs = TOKSTATS.int_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.lostframes = TOKSTATS.lostframes;
  MIB.Token_ring_mib.Dot5StatsEntry.rx_congestion= TOKSTATS.rx_congestion;
  MIB.Token_ring_mib.Dot5StatsEntry.framecopies = TOKSTATS.framecopies;
  MIB.Token_ring_mib.Dot5StatsEntry.token_errs = TOKSTATS.token_errs;

  MIB.Token_ring_mib.Dot5StatsEntry.signal_loss = TOKSTATS.signal_loss;
  MIB.Token_ring_mib.Dot5StatsEntry.hard_errs = TOKSTATS.hard_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.soft_errs = TOKSTATS.soft_errs;
  MIB.Token_ring_mib.Dot5StatsEntry.tx_beacons = TOKSTATS.tx_beacons;
  MIB.Token_ring_mib.Dot5StatsEntry.lobewires = TOKSTATS.lobewires;
  MIB.Token_ring_mib.Dot5StatsEntry.removes = TOKSTATS.removes;
  MIB.Token_ring_mib.Dot5StatsEntry.singles = TOKSTATS.singles;
  MIB.Token_ring_mib.Dot5StatsEntry.recoverys = TOKSTATS.recoverys;

  MIB.Generic_mib.ifExtnsEntry.mcast_rx_ok = TOKSTATS.mcast_recv;
  MIB.Generic_mib.ifExtnsEntry.bcast_rx_ok = TOKSTATS.bcast_recv;
  MIB.Generic_mib.ifExtnsEntry.mcast_tx_ok = TOKSTATS.mcast_xmit;
  MIB.Generic_mib.ifExtnsEntry.bcast_tx_ok = TOKSTATS.bcast_xmit;

  MIB.Token_ring_mib.Dot5Entry.ring_ostatus = TR_MIB_LASTOPEN;
  MIB.Token_ring_mib.Dot5Entry.participate = TR_MIB_TRUE;

  /*
   * Gets the functional address 
   */
  for (i=0; i< 6; i++) {
        MIB.Token_ring_mib.Dot5Entry.functional[i] = FUNCTIONAL.functional[i];
  }

  /*
   * Gets the Up stream hardware address 
   */
  if (p_dev_ctl->device_state == OPENED) {

	ioa = (int)iomem_att(&WRK.iomap);

        PIO_PUTSRX(ioa + LAPE, 0x00);
       	PIO_PUTSRX(ioa + LAPA, WRK.parms_addr + 4);
       	for (i=0, j=0; i < 6; j++) {
            	PIO_GETSX(ioa + LAPD_I, &addr);
                MIB.Token_ring_mib.Dot5Entry.upstream[i++] = addr >> 8;
                MIB.Token_ring_mib.Dot5Entry.upstream[i++] = addr &  0xff;
       	}

        iomem_det((void *)ioa);         /* restore I/O Bus */
  }
  TRACE_SYS(STOK_OTHER, "ImbE", p_dev_ctl, 0, 0);
  return (0);
}

/****************************************************************************/
/*
 * NAME:     turn_promis_on
 *
 * FUNCTION:  Checks if we need to trun on the promiscuous mode
 *
 * EXECUTION ENVIRONMENT: process only
 * 
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * CALLED FROM:
 *      stok_ctl
 *      srb_response
 *
 * CALLS TO:
 *      modify_receive_option
 *
 */
/****************************************************************************/
int turn_promis_on(
  stok_dev_ctl_t      *p_dev_ctl) {   /* pointer to device control area */

  ndd_t                      *p_ndd = (ndd_t *)&(NDD);

  TRACE_SYS(STOK_OTHER, "ItpB", p_dev_ctl, WRK.promiscuous_count, 0);
  WRK.promiscuous_count++;   /* incr the ref counter */
  if (WRK.promiscuous_count == 1) {
        if (modify_receive_options(p_dev_ctl, PROMIS_ON, FALSE)) {
  		TRACE_BOTH(STOK_ERR, "ItpB",p_dev_ctl, WRK.promiscuous_count,0);
  		WRK.promiscuous_count--;   /* incr the ref counter */
		return (ERROR);
	}
      	NDD.ndd_flags |= NDD_PROMISC;
       	MIB.Generic_mib.ifExtnsEntry.promiscuous = PROMTRUE;
  }
  TRACE_SYS(STOK_OTHER, "ItpB", p_dev_ctl, WRK.promiscuous_count, 0);
  return(OK);

}

/****************************************************************************/
/*
 * NAME:     turn_promis_off
 *
 * FUNCTION:  Checks if we need to trun off the promiscuous mode
 *
 * EXECUTION ENVIRONMENT: process only
 * 
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area.
 *
 * CALLED FROM:
 *      stok_ctl
 *
 * CALLS TO:
 *      modify_receive_option
 *
 * RETURNS:
 *      OK       - successful
 *	ERROR	- FALSE
 */
/****************************************************************************/
int turn_promis_off(
  stok_dev_ctl_t      *p_dev_ctl) {   /* pointer to device control area */

  ndd_t                      *p_ndd = (ndd_t *)&(NDD);

  if (WRK.promiscuous_count < 1) {
	return (ERROR);
  }

  TRACE_SYS(STOK_OTHER, "ItfB", p_dev_ctl, WRK.promiscuous_count, 0);
  WRK.promiscuous_count--;/* dev the reference counter */
  if (WRK.promiscuous_count == 0) {
        if (p_dev_ctl->device_state == OPENED) {
                if (modify_receive_options (p_dev_ctl, PROMIS_OFF, FALSE)) {
  			TRACE_BOTH(STOK_ERR, "ItfB", p_dev_ctl, 
						WRK.promiscuous_count,0);
  			WRK.promiscuous_count++; /* dev the reference counter */
			return (ERROR);
		}
        }
        p_ndd->ndd_flags &= ~NDD_PROMISC;
        MIB.Generic_mib.ifExtnsEntry.promiscuous = PROMFALSE;
  }
  TRACE_SYS(STOK_OTHER, "ItfE", p_dev_ctl, WRK.promiscuous_count, 0);
  return (OK);
}

/*****************************************************************************/
/*
 * NAME:     multi_add
 *
 * FUNCTION: Add a multicast address to the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      stok_ctl
 *
 * INPUT:
 *      p_dev_ctl       - point to the dev_ctl area
 *      p_ndd           - pointer to the ndd in the dev_ctl area 
 *      addr            - point to the multicast address to be added
 *
 * RETURNS:
 *      0 - successful
 *      ENOMEM - unable to allocate required memory
 *      EINVAL - invalid parameter
 *
 * Note :
 *   	The new group address will always be added to the end of the table.  
 *      The Skyline adapter only support 256 group addresses.  When there 
 *	are less than 256 group addresses in the table, update the adapter's 
 *	group register as required. When there are more than 256 group 
 *	addresses in the table, the driver will enable the promiscuous mode 
 *      on the adapter.
 * 	The driver will keep the group address in its table and expand the
 * 	table when more space is needed.
 *
 */
/*****************************************************************************/
int multi_add(
  stok_dev_ctl_t   *p_dev_ctl,      /* point to the dev_ctl area              */
  ndd_t           *p_ndd,          /* pointer to the ndd in the dev_ctl area */
  char  	  *addr)           /* point to the multicast address         */

{

  stok_multi_t *p_multi = &WRK.multi_table;
  int i;
  int rc;

  TRACE_SYS(STOK_OTHER, "ImaB", (ulong)p_dev_ctl, (ulong)addr, (ulong)p_multi);

  /*
   * Determines if the multicast address is a duplicate or not.
   * For a duplicate address, simply increment the ref_count and return
   */
  while (p_multi) {
        for (i=0; i < p_multi->in_use; i++) {
                if (SAME_NADR(addr, p_multi->m_slot[i].m_addr)) {
                        p_multi->m_slot[i].ref_count++;
                        TRACE_SYS(STOK_OTHER, "Ima1", p_dev_ctl, 0, 0);
                        return(0);
                }
        }
        p_multi = p_multi->next;
  }

  /* 
   * Gets to the table that has room for new entry 
   */
  if (WRK.multi_last->in_use >= MULTI_ENTRY) {
  	/*
  	 * No room in existing tables, allocate
  	 * memory for additional multicast table.
  	 */
        p_multi = (stok_multi_t *)
                     	net_malloc(sizeof(stok_multi_t),M_DEVBUF,M_WAIT);
        if (!p_multi) {
               	TRACE_SYS(STOK_ERR, "Ima2", p_dev_ctl, ENOMEM, 0);
               	return(ENOMEM);
        }
	/*
         * Links the new table to the end of the chain 
	 */
  	WRK.multi_last->next = p_multi;
        p_multi->next = NULL;
        p_multi->in_use = 0;
  	WRK.multi_last = p_multi;

  } else {
  	p_multi = WRK.multi_last;
  }

  /* 
   * Adds the address into the table and increment counters 
   */
  COPY_NADR(addr, p_multi->m_slot[p_multi->in_use].m_addr);

 /* 
  * Checks if any room to add the group address to hardware register.
  * the hardware group register is limit to 256 
  */
 if (WRK.multi_count < MAX_MULTI) { 
 	if (rc = set_group_address (p_dev_ctl, addr, FALSE)) {
		TRACE_SYS (STOK_ERR, "Ima3", p_dev_ctl, addr, rc);
  		return (EINVAL);
	}
  } else {
 	if (WRK.multi_count == MAX_MULTI) { 
  		/* 
 	 	 * we need to trun on the promiscuous mode 
 		 */
		if (turn_promis_on(p_dev_ctl)) {
			return(ENETUNREACH);
		}

       	}
  }
  /* 
   * Updates counters
   */
  p_multi->m_slot[p_multi->in_use].ref_count = 1;
  p_multi->in_use++;
  WRK.multi_count++;

  TRACE_SYS(STOK_OTHER, "ImaE", p_dev_ctl, 0, 0);
  return(0);
}

/*****************************************************************************/
/*
 * NAME:     multi_del
 *
 * FUNCTION: Delete a multicast address from the multicast table.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      stok_ctl
 *
 * INPUT:
 *      p_dev_ctl       - point to the dev_ctl area
 *      p_ndd           - pointer to the ndd in the dev_ctl area 
 *      addr            - point to the multicast address to be added
 *
 * RETURNS:
 *      0 - successful
 *      EINVAL - invalid parameter
 *
 * Note :
 *	 The Multicast address table will not leave any hole in the table.  
 *	 After each delete, the table will repack by moving the last entry in 
 *	 the table to the entry that just have been removed.  The promiscuous 
 *	 mode will be turned off if number of entry in the table is not exceed 
 *	 the hardware group registers limit(256).
 *	 The driver will maintain its multicast table and shrink the table when 
 *	 spaces freed up.
 */
/*****************************************************************************/
int multi_del(
  stok_dev_ctl_t   *p_dev_ctl,      /* point to the dev_ctl area              */
  ndd_t           *p_ndd,          /* pointer to the ndd in the dev_ctl area */
  char  *addr)                     /* point to the multicast address         */

{

  stok_multi_t *p_multi = &WRK.multi_table;
  stok_multi_t *l_table;
  stok_multi_t *p_prev  = NULL;
  char *p;
  int i;
  int count = 0;
  int found = FALSE;


  TRACE_SYS(STOK_OTHER, "ImdB", (ulong)p_dev_ctl, (ulong)addr, 0);

  /* 
   * Searchs for the address 
   */
  while (p_multi) {
        for (i = 0; i < p_multi->in_use; i++) {
  		count++;
                if (SAME_NADR(addr, p_multi->m_slot[i].m_addr))  {
  			found = TRUE;
                        break;
                }
        }
  	if (found) { 
  		break;
        }
        p_multi = p_multi->next;
  }

  /* 
   * If can not find the address
   */
  if (!p_multi) {
        TRACE_SYS(STOK_ERR, "Imd1", p_dev_ctl, EINVAL, 0);
        return(EINVAL);
  }

  /*
   * If the ref_count is greater than 1, simply decrement it.
   * Otherwise, remove the address from the table 
   */
  p_multi->m_slot[i].ref_count--;
  if (!(p_multi->m_slot[i].ref_count)) {

  	/* 
   	 * we need to delete the group address from the hardware reg
   	 */
  	if ((count < MAX_MULTI) && (p_dev_ctl->device_state == OPENED)) { 
   		if (unset_group_address (p_dev_ctl, addr, FALSE)) {
  			TRACE_SYS(STOK_OTHER, "Imd2", p_dev_ctl, addr, count);
		}
	} else if (p_dev_ctl->device_state == LIMBO) {
		WRK.re_limbo = TRUE;
        }

  	l_table = WRK.multi_last;
  	/* Updates counters */
  	WRK.multi_count--;
  	l_table->in_use--;

  	/* Updates the multi address table */
  	if (WRK.multi_count) {
		COPY_NADR(l_table->m_slot[l_table->in_use].m_addr,
       				p_multi->m_slot[i].m_addr);
  		p_multi->m_slot[i].ref_count = 
				l_table->m_slot[l_table->in_use].ref_count;
	}

  	/* 
   	 * Puts the last added group address in the multi table but not 
   	 * in the hardware register to the hardware register.
   	 */
  	if ((count < MAX_MULTI) && (WRK.multi_count >= MAX_MULTI) &&
	     (p_dev_ctl->device_state == OPENED)) {
   		if (set_group_address (p_dev_ctl,  
         	     		  l_table->m_slot[l_table->in_use].m_addr, 
				  FALSE)) {
		}
  	}

 	/*
  	 * If the table is empty and it is an expansion, free the space
  	 */
  	if (!l_table->in_use) {
		/*
  	 	* Searchs for the previous table 
	 	*/
  		p_multi = &WRK.multi_table;
    		while (p_multi != l_table) {
         		p_prev = p_multi;
         		p_multi = p_multi->next;
    		}

        	if (p_prev) {
    			WRK.multi_last = p_prev;
                	p_prev->next = p_multi->next;
  			net_free(p_multi, M_DEVBUF);
        	}
  	}

  	/* 
   	 * Checks if we need to trun off the promiscuous mode 
   	 */
  	if (WRK.multi_count == MAX_MULTI) {
		turn_promis_off(p_dev_ctl);
         }

  }
  TRACE_SYS(STOK_OTHER, "ImdE", p_dev_ctl, 0, 0);
  return(0);

}

