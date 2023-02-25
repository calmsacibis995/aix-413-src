static char sccsid[] = "@(#)31	1.4  src/rspc/kernext/pcmcia/cs/evh_cs.c, pcmciaker, rspc41J, 9515A_all 4/11/95 04:37:43";
/*
 *
 * COMPONENT_NAME: (PCMCIAKER)
 *
 * FUNCTIONS: PCMCIA Card Services - Event Handler Glue code
 *             get_logsocket, cfehResetCard, cfehCfgCard
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/device.h>
#include <sys/uio.h>
#include "EvhP.h"
#include <sys/pcmciass.h>
#include <sys/pcmciacs.h>
#include "cs.h"

extern EvhCoreClass evhCoreClass;
extern ClientClass clientClass;
extern int callbackall();

#define CSES_RESET         100
static void action_for_Reset( EventQueue* ev );
static void action_for_Cfg( EventQueue* ev );
static int getstatus( int socket, int *status, int mask );

/****************************************************************************
*
* NAME:  get_logsocket
*
*   FUNCTION:         define new socket in event handler and get logical
*                     socket number.
*   ARGUMENTS:        - flag       flag to specify the way to add evactlist
*                                  to default actions for events
*                     - getstauts  method to get slot's status
*                     - evactlist  slot unique actions for events
*   RETURN VALUE:     logical slot number
*                     -1 ( error on xmalloc(), or getstatus is null )
*   ENVIRONMENT:      config thread
*   Calls with Locks: (none)
*
****************************************************************************/
int get_logsocket( Socket* skt, dev_t devno, char* devname )
{
  static EventAction evact[5] = {
    { CSES_RESET,  0, action_for_Reset, 0 },
    { EVH_CFGDEV,  0, action_for_Cfg, 0 }
  };
  extern int evhCoreAdd( int, int (*)(), EventAction*, dev_t, char* );

  evact[0].next = &evact[1];
  return( evhCoreAdd( ACTION_APPEND, getstatus, evact, devno, devname ) );
}

/****************************************************************************
*
* NAME:  getstatus
*
*   FUNCTION:         get the socket status of the specifyed socket
*                     
*   ARGUMENTS:        - socket   logical socket number
*                     - status   bitmapped status flags pointer to be
*                                stored.
*                                 SSTAT_CARD_EXIST
*                                 SSTAT_CARD_INSERTION
*                                 SSTAT_CARD_REMOVAL
*                                 SSTAT_CARD_READY
*                     - mask     mask to get status. If the bit is 1,
*                                it is queryed.
*                                If mask & (~SSTAT_CARD_EXIST) == 0,
*                                SetSocket is not called but just GetStatus
*                                is called.
*   RETURN VALUE:     1 ( O.K. )
*                     0 ( error : invalid socket number or
*                                 socket services return error )
*   ENVIRONMENT:      event handler
*                         with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=r
*   Calls with Locks: SocketServices( GetStatus/GetSocket/SetSocket ) with
*                         evhCoreClass.configlock=r
*
****************************************************************************/
static int getstatus( int socket, int *status, int mask )
{
  int ret, old;
  GetStatus_S getstat;
  SetSocket_S getskt;
  Socket * skt;
  Adapter * adpt;
  extern Adapter * adapters;
  extern int SocketServices();
  extern int ValidateSocket();
  extern Simple_lock cs_intr_lock;

  *status = 0;

  if ( ret = ValidateSocket( socket, &skt ) ){
    return( 0 );
  }
  adpt = &adapters[skt->AdapterIndex];

  lock_write( &skt->rw_lock );

  if( mask & ( ~SSTAT_CARD_EXIST ) ){
    old = disable_lock( adpt->intr.priority, &cs_intr_lock );
    if( skt->critical_state ) skt->critical_state = 1;
    getskt.Socket   = skt->PhySocket;
    if( ( ret = SocketServices( adpt, PCM_GET_SOCKET, (caddr_t)&getskt  ) ) || 
        ( ret = SocketServices( adpt, PCM_SET_SOCKET, (caddr_t)&getskt  ) ) ){
      unlock_enable( old, &cs_intr_lock );
      lock_done( &skt->rw_lock );
      return( 0 );
    }
    unlock_enable( old, &cs_intr_lock );
  }
  getstat.Socket  = skt->PhySocket;
  if( ret = SocketServices( adpt, PCM_GET_STATUS, (caddr_t)&getstat ) ){
    lock_done( &skt->rw_lock );
    return( 0 );
  }

#if defined( DEBUG )
  printf("getstat.{CardState,SocketState} = {0x%x,0x%x}, getskt.State = 0x%x\n",
	 getstat.CardState, getstat.SocketState, getskt.State);
#endif

  *status = getstat.CardState & PCM_SBM_CD ? SSTAT_CARD_EXIST : 0;

  if( mask & ( ~SSTAT_CARD_EXIST ) ){
    if( skt->card_present &&
       ( ( getskt.State & CSStatDetected ) ||
        !( getstat.CardState & PCM_SBM_CD ) ) ){
      *status |= SSTAT_CARD_REMOVAL;
      skt->card_present = 0;
    }
    if( getstat.CardState & PCM_SBM_CD &&
       ( ( getskt.State & CSStatDetected ) || !skt->card_present ) ){
      *status |= SSTAT_CARD_INSERTION;
      skt->card_present = 1;
    }
    if( ( getstat.SocketState & CSStatReady ) &&
       ( getstat.CardState & CSStatReady ) ){
      *status |= SSTAT_CARD_READY;
    }
  }

  lock_done( &skt->rw_lock );
  *status &= mask;
  return( 1 );
}

/****************************************************************************
*
* NAME:  action_for_Reset
*
*   FUNCTION:         communicate with clients to ask the card reset, and
*                     and if all clients who interested in the socket agree,
*                     reset the socket.
*                     
*   ARGUMENTS:        - ev  queued data
*   RETURN VALUE:     (none)
*   ENVIRONMENT:      event handler with evhCoreClass.configlock
*   LOCKS      :      evhCoreClass.configlock
*   Calls with Locks: callbackall() with evhCoreClass.configlock
*                     cs.c#ResetSocket() with evhCoreClass.configlock
****************************************************************************/
static void action_for_Reset( EventQueue* ev )
{
  extern int ResetSocket( int );

  if( callbackall( ev, CSE_RESET_REQUEST, ev->slot, SLOT, CSEvMaskReset, 0, 0 ) ){
    callbackall( ev, CSE_RESET_COMPLETE, ev->slot, REQUESTER, 0, CSR_IN_USE, 0 );
    return;
  }

  callbackall( ev, CSE_RESET_PHYSICAL, ev->slot, SLOT, CSEvMaskReset, 0, 0 );

  ResetSocket( ev->slot );

  callbackall( ev, CSE_CARD_RESET, ev->slot, SLOT, CSEvMaskReset, CSR_SUCCESS, 0 );
  callbackall( ev, CSE_RESET_COMPLETE, ev->slot, REQUESTER, 0, CSR_SUCCESS, 0 );
}

/****************************************************************************
*
* NAME:  cfehResetCard
*
*   FUNCTION:         enqueue client's reset request on the card
*                     
*   ARGUMENTS:        - me   client ID who requested
*                     - slot logical socket number to reset
*   RETURN VALUE:     CSR_SUCCESS ( O.K. )
*   ENVIRONMENT:      client thread
*   LOCKS      :      (none)
*   Calls with Locks: (none)
*
****************************************************************************/
cfehResetCard( Client* me, int slot )
{
  int rc;

  if( rc = cfehValidateClient( me ) ) return( rc );

  Enqueue( CSES_RESET, slot, me, 0 );
  return( CSR_SUCCESS );
}


/****************************************************************************
*
* NAME:  action_for_Cfg
*
*   FUNCTION:         enqueue to usr_queue to notify a card configuration
*                     event to registerd processes in user space
*                     
*   ARGUMENTS:        - ev  queued data
*   RETURN VALUE:     (none)
*   ENVIRONMENT:      event handler with evhCoreClass.configlock
*   LOCKS      :      evhCoreClass.configlock
*   Calls with Locks: usr_enqueue() with evhCoreClass.configlock
*                     signal_Usr_Client() with evhCoreClass.configlock
****************************************************************************/
static void action_for_Cfg( EventQueue* ev )
{
  usr_enqueue( ev, KEEP_PREVIOUS_EVENTS );
  signal_Usr_Client();
}

/****************************************************************************
*
* NAME:  cfehCfgCard
*
*   FUNCTION:         enqueue a card configuration event
*                     User process will be notified the event.
*                     
*   ARGUMENTS:        - me   client ID who requested configuration
*                     - slot the slot number where configuration is
*                            requested
*   RETURN VALUE:     CSR_SUCCESS ( O.K. )
*                     CSR_BAD_CLIENT
*   ENVIRONMENT:      client thread
*   LOCKS      :      (none)
*   Calls with Locks: (none)
*
****************************************************************************/
cfehCfgCard( Client* me, int slot )
{
  int rc;

  if( rc = cfehValidateClient( me ) ) return( rc );

  Enqueue( EVH_CFGDEV, slot, me, 0 );
  return( CSR_SUCCESS );
}
