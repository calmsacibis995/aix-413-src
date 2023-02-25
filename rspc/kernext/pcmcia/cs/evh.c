static char sccsid[] = "@(#)30	1.8  src/rspc/kernext/pcmcia/cs/evh.c, pcmciaker, rspc41J, 9515A_all 4/11/95 04:37:40";
/*
 *
 * COMPONENT_NAME: (PCMCIAKER)
 *
 * FUNCTIONS: PCMCIA Card Services - Event handler and client services
 *             evhCoreAdd, evhCoreDel, evhCoreSuspend, evhCoreResume,
 *             Enqueue, callbackall,
 *             cfehValidateClient, cfehClientSetSlot, cfehClientResetSlot
 *             cfehRegisterClient, cfehDeregisterClient,
 *             cfehSetEventMask, cfehGetEventMask,
 *             cfehRequestSlotMask, cfehReleaseSlotMask,
 *             cfehSetSlotMask, cfehGetSlotMask,
 *             cfehGetFirstClient, cfehGetNextClient, cfehGetClientInfo,
 *             cfehRequestExclusive, cfehReleaseExclusive
 *             cfehGetExclusiveClient
 *             svccfehRegisterHandler, svccfehGetEvent, svccfehDeregisterHandler
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

#include <sys/proc.h>
#include <sys/sleep.h>
#include <sys/m_param.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/pin.h>
#include <sys/intr.h>
#include <sys/trchkid.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include "EvhP.h"

static EvhCoreClass evhCoreClass = {
  0,   /* instance list */
  0,   /* kpid */
  1,   /* stopping flag */
  EVENT_NULL, /* configproc_word */
  EVENT_NULL, /* evhproc_word */
  0, /* readpoint */
  0, /* writepoint */
  0, /* ireadpoint */
  0, /* iwritepoint */
  { 0 }, /* iqueue */
  { 0 }, /* queuelock */
  { 0 }, /* iqueuelock */
  { 0 }, /* configlock */
  EVENT_NULL, /* wakeup_word */
};

static EventQueue eventQueues[ MAX_EVENTS ];

static ClientClass clientClass = {
  0,    /* clients */
};

extern EvhCoreClass* evhCoreClassp; /* evh_intr.c */

extern int evhCoreAdd( int flag, int (*getstatus)(), EventAction* evactlist,
		      dev_t devno, char* devname );

BUGVDEF( evhvar, BUGGID );

/****************************************************************************
*
* NAME:  evhCoreInit
*
*   FUNCTION:         initialize config event handler
*                       ( initialize evhCoreClass and start kproc )
*   RETURN VALUE:     0  ( O.K. )
*                     not 0  ( fail at initp(), pin() )
*   ENVIRONMENT:      config thread
*   LOCKS      :      lockl for code lock
*   Calls with Locks: (none)
*
****************************************************************************/
static int evhCoreInit()
{
  static void init_Usr_ClientClass( void );
  static void EventHandler( int flag );
  extern int IntrEnqueue( int src, int slot );
  static int inited = 0;
  static int init_lock = LOCK_AVAIL;
  EventQueue* p;
  int i;
  int rc = 0;

  lockl( &init_lock, LOCK_SHORT );

  if( !inited ){
    /* init SW event queue */
    if( evhCoreClass.read == 0 ){
      for( p = eventQueues, i = 0; i < MAX_EVENTS - 1; i++ ){
	p->next = p + 1;
	p++;
      }
      p->next = eventQueues; /* p->next should be &eventQueues[MAX_EVENTS-1] */
      evhCoreClass.read = evhCoreClass.write = eventQueues;
    }

    if( rc = pincode( IntrEnqueue ) ){
      goto end;
    }
    if( rc = pin( &evhCoreClass, sizeof( evhCoreClass ) ) ){
      unpincode( IntrEnqueue );
      goto end;
    }

    evhCoreClassp = &evhCoreClass;
    lock_alloc( &evhCoreClass.iqueuelock, LOCK_ALLOC_PIN, CS_IQUEUE_LOCK, -1 );
    simple_lock_init( &evhCoreClass.iqueuelock );
    lock_alloc( &evhCoreClass.queuelock, LOCK_ALLOC_PIN, CS_QUEUE_LOCK, -1 );
    simple_lock_init( &evhCoreClass.queuelock );
    lock_alloc( &evhCoreClass.configlock, LOCK_ALLOC_PIN, CS_EVH_LOCK, -1 );
    lock_init( &evhCoreClass.configlock, 0 ); /* complex lock */

    init_Usr_ClientClass();

    /* init kproc for event handler */
    if( evhCoreClass.kpid == 0 ){
      if( ( evhCoreClass.kpid = creatp() ) == -1 ){
	unpin( &evhCoreClass, sizeof( EvhCoreClass ) );
	unpincode( IntrEnqueue );
	rc = EPROCLIM;
	goto end;
      }
      evhCoreClass.stopping_flag = 0;
      if(  rc = initp( evhCoreClass.kpid, EventHandler, 0, 0, "cfgmon" ) ){
	unpin( &evhCoreClass, sizeof( EvhCoreClass ) );
	unpincode( IntrEnqueue );
	goto end;
      }
    }
    inited = 1;
  }

 end:
  unlockl( &init_lock );
  return( rc );
}

/****************************************************************************
*
* NAME:  evhCoreUninit
*
*   FUNCTION:         uninitialize config event handler
*                       ( unpin evhCoreClass and stop kproc )
*   RETURN VALUE:     0  ( O.K. )
*   ENVIRONMENT:      config thread with evhCoreClass.configlock=w
*   LOCKS      :      evhCoreClass.configlock=w is released in this
*                     function.
*   Calls with Locks: (none)
*
****************************************************************************/
static int evhCoreUninit()
{
  static void uninit_Usr_ClientClass( void );

  evhCoreClass.stopping_flag = 1;
  uninit_Usr_ClientClass();

  if( evhCoreClass.kpid ){
    e_wakeup( &evhCoreClass.evhproc_word );
    /* wait for the completion of kproc */
    e_sleep_thread( &evhCoreClass.configproc_word, &evhCoreClass.configlock,
		   LOCK_WRITE );
  }

  lock_done( &evhCoreClass.configlock );

  /* need to free clients */

  lock_free( &evhCoreClass.configlock );
  lock_free( &evhCoreClass.queuelock );
  lock_free( &evhCoreClass.iqueuelock );

  unpin( &evhCoreClass, sizeof( EvhCoreClass ) );
  unpincode( IntrEnqueue );

  return( 0 );
}

static EventActionListClass eventActionListClass = {
  0  /* all */
};

/****************************************************************************
*
* NAME:  eventActionListConst
*
*   FUNCTION:         register eventActions to EventActionList
*                     
*   ARGUMENTS:        eventActions - array of EventAction structure
*   RETURN VALUE:     pointer to eventActionList
*                     0 ( xmalloc() error or stopping event handler )
*   ENVIRONMENT:      config thread with evhCoreClass.configlock=w
*   LOCKS      :      (none)
*   Calls with Locks: (none)
*
****************************************************************************/
static EventActionList* eventActionListConst( EventAction* eventActions )
{
  EventActionList **p, *new;

  for( p = &eventActionListClass.all; *p; ){
    if( (*p)->eventActions == eventActions ){
      (*p)->ref_count++;
      return( *p );
    }
    p = &(*p)->next;
  }

  if( new = xmalloc( sizeof( EventActionList ), sizeof( int ), pinned_heap ) ){
    /* xmalloc success */
    new->eventActions = eventActions;
    new->ref_count = 1;
    new->next = 0;
    (*p) = new;
  }

  return( new );
}

/****************************************************************************
*
* NAME:  eventActionListDest
*
*   FUNCTION:         deregister eventActions from EventActionList
*                     
*   ARGUMENTS:        eventActionLists - pointer to eventActionList
*   RETURN VALUE:     1 ( O.K. )
*                     0 ( invalid argument pointer of eventActions )
*   ENVIRONMENT:      config thread with evhCoreClass.configlock=w
*   LOCKS      :      (none)
*   Calls with Locks: (none)
*
****************************************************************************/
static int eventActionListDest( EventActionList* eventActionLists )
{
  EventActionList **p, *new;
  EventAction* eventActions;
  int rc = 0;

  eventActions = eventActionLists->eventActions;

  for( p = &eventActionListClass.all; *p; ){
    if( (*p)->eventActions == eventActions ){
      (*p)->ref_count--;

      if( (*p)->ref_count == 0 ){
	xmfree( (*p), pinned_heap );
      }

      rc = 1;
      goto end;
    }
    p = &(*p)->next;
  }

 end:
  return( rc );
}

static void action_for_HWNotify( EventQueue* ev );
static void action_for_Registration( EventQueue* ev );
static void action_for_Suspend( EventQueue* ev );
static void action_for_Resume( EventQueue* ev );
static void action_for_Exclusive( EventQueue* ev );
static void action_for_Share( EventQueue* ev );

/****************************************************************************
*
* NAME:  evhCoreAdd
*
*   FUNCTION:         define new slot in event handler
*                        1. allocate evhCore structure
*                        2. define eventActionLists
*                        3. decide logical slot number
*   ARGUMENTS:        - flag       flag to specify the way to add evactlist
*                                  to default actions for events
*                     - getstauts  method to get slot's status
*                     - evactlist  slot unique actions for events
*                     - devno      device number of detecter DD
*                     - devname    device name in ODM
*   RETURN VALUE:     logical slot number
*                     -1 ( error on xmalloc(), or getstatus is null )
*   ENVIRONMENT:      config thread
*   LOCKS      :      evhCoreClass.configlock=w
*   Calls with Locks: eventActionListConst() with evhCoreClass.configlock=w
*
****************************************************************************/
int evhCoreAdd( int flag, int (*getstatus)(), EventAction* evactlist, dev_t devno, char* devname )
{
  int rc;
   EvhCore *p, *last;
   static EventAction evact[6] = {
     { EVH_HWNOTIFY,     EVM_CDChg|EVM_ReadyChg, action_for_HWNotify, evact + 1 },
     { EVH_REGISTRATION, 0, action_for_Registration, evact + 2 },
     { EVH_PM_SUSPEND,   0, action_for_Suspend, evact + 3 },
     { EVH_PM_RESUME,    0, action_for_Resume, evact + 4 },
     { EVH_EXCLUSIVE,    0, action_for_Exclusive, evact + 5 },
     { EVH_SHARE,        0, action_for_Share, 0 }
   };

   if( rc = evhCoreInit() ) return( -rc );

   if( evhCoreClass.stopping_flag || getstatus == 0 || devno == 0 ||
      ( p = xmalloc( sizeof( EvhCore ), sizeof( int ), pinned_heap ) ) == 0 ){
     rc = -1;
     lock_write( &evhCoreClass.configlock );
     goto end;
   }

   bzero( p, sizeof( EvhCore ) );
   lock_alloc( &p->lock, LOCK_ALLOC_PIN, CS_SLOT_LOCK, -1 );
   simple_lock_init( &p->lock );
   p->getstatus = getstatus;
   p->devno = devno;
   bcopy( devname, p->devname, 16 );
   p->flag = 0;

   lock_write( &evhCoreClass.configlock );

   switch( flag ){
   case ACTION_APPEND:
     p->eventActionLists = eventActionListConst( evactlist );
     p->eventActionLists2 = eventActionListConst( evact );
     break;
   case ACTION_OVERWRITE:
     p->eventActionLists = eventActionListConst( evactlist );
     break;
   case ACTION_DEFAULT:
   default:
     p->eventActionLists = eventActionListConst( evact );
     break;
   }

   if( p->eventActionLists == 0 ){
     lock_free( &p->lock );
     xmfree( p, pinned_heap );
     rc = -1;
     goto end;
   }

   if( evhCoreClass.evhCores ){
     for( last = evhCoreClass.evhCores; last->next; last = last->next );
     p->slot = last->slot + 1;
     last->next = p;
   }else{
     p->slot = 0;
     evhCoreClass.evhCores = p;
   }

  rc = p->slot;
 end:
  if( evhCoreClass.evhCores == 0 ){
    /* lock done is called in evhCoreUninit() */
    evhCoreUninit();
  }else{
    lock_done( &evhCoreClass.configlock );
  }
  return( rc );
}

/****************************************************************************
*
* NAME:  evhCoreDel
*
*   FUNCTION:         undefine a slot in event handler
*                        1. remove the target data from evhCore link
*                        2. undefine eventActionLists
*                        3. free the data area
*   ARGUMENTS:        - slot logical slot number to undefine
*   RETURN VALUE:     1 ( O.K. )
*                     0 ( invalid slot number )
*   ENVIRONMENT:      config thread
*   LOCKS      :      evhCoreClass.configlock=w, evhCore.lock=s
*   Calls with Locks: eventActionListDest() with evhCoreClass.configlock=w
*
****************************************************************************/
int evhCoreDel( int slot )
{
   EvhCore **p, *q;
   SlotMask *sm, *sm0;
   int rc = 0;

   lock_write( &evhCoreClass.configlock );

   for( p = &evhCoreClass.evhCores; (*p); p = &(*p)->next ){
     if( (*p)->slot == slot ){
       q = (*p);     /* q is target data */
       *p = q->next; /* remove q from the link */
       /* may also need to tell ExclusiveUser */
       for ( sm = q->slotMasks; sm; ){
	 sm0 = sm;
	 sm = sm->next;
	 xmfree( sm0, pinned_heap );
       }

       eventActionListDest( q->eventActionLists );
       if( q->eventActionLists2 ) eventActionListDest( q->eventActionLists2 );
       lock_free( &q->lock );
       xmfree( q, pinned_heap );
       break;
     }
   }

  if( evhCoreClass.evhCores == 0 ){
    /* lock done is called in evhCoreUninit() */
    evhCoreUninit();
  }else{
    lock_done( &evhCoreClass.configlock );
  }
   return( 1 );
}

extern int Enqueue( int src, int slot, Client* me, Client* tgt );
static int classlock_read( complex_lock_t l );
static int classlock_rdone( complex_lock_t l );
static EvhCore* selectslot( int slot );

/****************************************************************************
*
* NAME:  action_for_Suspend
*
*   FUNCTION:         Set SLOT_SUSPENDED flag in evhCore, and wake up
*                     blocked process who requested suspension.
*                     
*   ARGUMENTS:        - ev   queued event data
*   RETURN VALUE:     (none)
*   ENVIRONMENT:      event handler with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=w
*   Calls with Locks: 
*
****************************************************************************/
static void action_for_Suspend( EventQueue* ev )
{
  EvhCore* p;

  if( ( p = selectslot( ev->slot ) ) == 0 ) return;

  lock_read_to_write( &evhCoreClass.configlock );
  p->flag |= SLOT_SUSPENDED;
  e_wakeup( &evhCoreClass.wakeup_word );
  lock_write_to_read( &evhCoreClass.configlock );
}

/****************************************************************************
*
* NAME:  action_for_Resume
*
*   FUNCTION:         Reset SLOT_SUSPENDED flag in evhCore, and wake up
*                     blocked process who requested resume.
*                     
*   ARGUMENTS:        - ev   queued event data
*   RETURN VALUE:     (none)
*   ENVIRONMENT:      event handler with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=w
*   Calls with Locks: 
*
****************************************************************************/
static void action_for_Resume( EventQueue* ev )
{
  EvhCore* p;

  if( ( p = selectslot( ev->slot ) ) == 0 ) return;

  lock_read_to_write( &evhCoreClass.configlock );
  p->flag &= (~SLOT_SUSPENDED);
  e_wakeup( &evhCoreClass.wakeup_word );
  lock_write_to_read( &evhCoreClass.configlock );
}

/****************************************************************************
*
* NAME:  evhCoreSuspend
*
*   FUNCTION:         Prepare for the suspension of the slot.
*                     If IntrEnqueue() is continued to call, this function
*                     will not return until action_for_HWNotify() calls
*                     end.
*
*   ARGUMENTS:        - slot logical slot number to suspend
*   RETURN VALUE:     1 ( O.K. )
*                     0 ( invalid slot number )
*   ENVIRONMENT:      client(PM) thread
*   LOCKS      :      evhCoreClass.configlock=r
*   Calls with Locks: (none)
*
****************************************************************************/
static int evhCoreSuspend( int slot )
{
  EvhCore* p;

  if( slot == NO_SLOT ) return( 0 );

  classlock_read( &evhCoreClass.configlock );
  Enqueue( EVH_PM_SUSPEND, slot, 0, 0 );

  while( 1 ){
    if( ( p = selectslot( slot ) ) == 0 ) break;
    if( p->flag & SLOT_SUSPENDED ) break;

    e_sleep_thread( &evhCoreClass.wakeup_word, &evhCoreClass.configlock,
		    LOCK_READ );
  }

  classlock_rdone( &evhCoreClass.configlock );

  return( p == 0 ? 0 : 1 );
}

/****************************************************************************
*
* NAME:  evhCoreResume
*
*   FUNCTION:         Resume from the suspension of the slot.
*                     If IntrEnqueue() is continued to call, this function
*                     will not return until action_for_HWNotify() calls
*                     end.
*
*   ARGUMENTS:        - slot logical slot number to resume
*   RETURN VALUE:     1 ( O.K. )
*                     0 ( invalid slot number )
*   ENVIRONMENT:      client(PM) thread
*   LOCKS      :      evhCoreClass.configlock=r
*   Calls with Locks: (none)
*
****************************************************************************/
static int evhCoreResume( int slot )
{
  EvhCore* p;

  if( slot == NO_SLOT ) return( 0 );

  classlock_read( &evhCoreClass.configlock );
  Enqueue( EVH_PM_RESUME, slot, 0, 0 );

  while( 1 ){
    if( ( p = selectslot( slot ) ) == 0 ) break;
    if( ( p->flag & SLOT_SUSPENDED ) == 0 ) break;

    e_sleep_thread( &evhCoreClass.wakeup_word, &evhCoreClass.configlock,
		    LOCK_READ );
  }

  classlock_rdone( &evhCoreClass.configlock );

  return( p == 0 ? 0 : 1 );
}

/****************************************************************************
*
* NAME:  Enqueue
*
*   FUNCTION:         enqueue event and ask event handler to process it.
*                     
*   ARGUMENTS:        - src  event source type
*                     - slot logical slot number where the event source
*                            is related to. NO_SLOT means the event is
*                            not related to one slot.
*                     - me   client ID who request enqueue
*                     - target client ID used by GetClientInfo
*   RETURN VALUE:     1 ( O.K. )
*                     0 ( invalid slot number or event handler is stopping )
*   ENVIRONMENT:      client thread with evhCoreClass.configlock=r
*   LOCKS      :      evhCoreClass.queuelock=s
*   Calls with Locks: (none)
*
****************************************************************************/
int Enqueue( int src, int slot, Client* me, Client* tgt )
{
  int ret = 0;

  if( !evhCoreClass.stopping_flag ){
    simple_lock( &evhCoreClass.queuelock );

    if( evhCoreClass.write->next != evhCoreClass.read ){
      BUGLPR(evhvar, BUGACT, ("enqueue ( %d, %d, %x, %x )\n", src, slot, me, tgt ));
      evhCoreClass.write->eventSrc = src;
      evhCoreClass.write->slot = slot;
      evhCoreClass.write->requester = me;
      evhCoreClass.write->target = tgt;
      evhCoreClass.write = evhCoreClass.write->next;
      e_wakeup( &evhCoreClass.evhproc_word );
      ret = 1;
    }
    simple_unlock( &evhCoreClass.queuelock );
  }

  return( ret );
}

/****************************************************************************
*
* NAME:  evhCoreDequeue
*
*   FUNCTION:         extract one queued data and copy it to the area
*                     specifyed by the argument buf.
*                     
*   ARGUMENTS:        - buf buffer area where data is to be copyed to.
*   RETURN VALUE:     same value as buf ( There is queued data )
*                     0 ( no queued data )
*                     -1 ( error ( buf is null ) )
*   ENVIRONMENT:      Event Handler with evhCoreClass.configlock=r
*   LOCKS      :      evhCoreClass.iqueuelock=s
*                     evhCoreClass.queuelock=s
*   Calls with Locks: (none)
*
****************************************************************************/
static EventQueue* evhCoreDequeue( EventQueue* buf )
{
  EventQueue* ret = 0;
  int old;

  if( buf == 0 ) return( (EventQueue*)(-1) );
  old = disable_lock( INTMAX, &evhCoreClass.iqueuelock );

  if( evhCoreClass.iread ){
    IEventQueue* p;

    buf->eventSrc = EVH_HWNOTIFY;
    buf->slot = ( (int)evhCoreClass.iread - (int)evhCoreClass.ieventQueues )
      /sizeof( IEventQueue );
    buf->requester = 0;
    buf->target = 0;
    p = evhCoreClass.iread;
    evhCoreClass.iread = (IEventQueue*)*p;
    *p = 0;
    if( evhCoreClass.iread == 0 ) evhCoreClass.iwrite = 0;
    unlock_enable( old, &evhCoreClass.iqueuelock );
    return( buf );
  }

  unlock_enable( old, &evhCoreClass.iqueuelock );
  simple_lock( &evhCoreClass.queuelock );
  if( evhCoreClass.read != evhCoreClass.write ){
    bcopy( evhCoreClass.read, buf, sizeof( EventQueue ) );
    evhCoreClass.read = evhCoreClass.read->next;
    ret = buf;
  }

  simple_unlock( &evhCoreClass.queuelock );
  return( ret );
}

/****************************************************************************
*
* NAME:  selectslot
*
*   FUNCTION:         get the pointer of evhCore strucure for the specifyed
*                     logical slot.
*                     
*   ARGUMENTS:        - slot logical slot number which is interested in.
*                            If NO_SLOT is specifyed, the pointer for the
*                            first evhCore entry is assumed. ( In usual
*                            case, it will be logical slot 0. )
*   RETURN VALUE:     the pointer to evhCore structure
*                     0 ( invalid slot number )
*   ENVIRONMENT:      Event Handler or client thread with
*                           evhCoreClass.configlock=r
*   LOCKS:            (none)
*   Calls with Locks: (none)
*
****************************************************************************/
static EvhCore* selectslot( int slot )
{
  EvhCore* p;

  if( slot == NO_SLOT ) return( evhCoreClass.evhCores );

  for( p = evhCoreClass.evhCores; p; p = p->next ){
    if( p->slot == slot ) break;
  }

  return( p );
}

/****************************************************************************
*
* NAME:  EventHandler
*
*   FUNCTION:         process queued events according to event action
*                     lists defined in each evhCore entry. If there is
*                     no queued data, this thread will sleep or end if
*                     evhCoreClass.stopping_flag is on.
*                     
*   ARGUMENTS:        - flag  not used now
*   RETURN VALUE:     (void)
*   ENVIRONMENT:      Event Handler 
*   LOCKS:            evhCoreClass.configlock=r
*   Calls with Locks: selectslot() with evhCoreClass.configlock=r
*                     evhCoreDequeue() with evhCoreClass.configlock=r
*                     (*action)() with evhCoreClass.configlock=r
*
****************************************************************************/
static void EventHandler( int flag )
{
  EventQueue ev;
  EventAction* p;
  EvhCore* evh;

  lock_read( &evhCoreClass.configlock );

  while( 1 ){
 BUGLPR(evhvar, BUGACT, ( "loop\n" ));
    if( evhCoreDequeue( &ev ) ){
      BUGLPR(evhvar, BUGACT, ("src = %d, slot = %d, me = %x, tgt = %x\n",\
	     ev.eventSrc, ev.slot, ev.requester, ev.target )); 
      evh = selectslot( ev.slot );
      if( evh ){
#if defined( PM_ENABLE )
	if( ( evh->flag & SLOT_SUSPENDED ) && ev.eventSrc != EVH_PM_RESUME ){
	  goto actionend;
	}
#endif /* PM_ENABLE */
	for( p = evh->eventActionLists->eventActions; p; p = p->next ){
	  if( ev.eventSrc == p->eventSrc ){
	    p->action( &ev );
	    goto actionend;
	  }
	}
	if( evh->eventActionLists2 ){
	  for( p = evh->eventActionLists2->eventActions; p; p = p->next ){
	    if( ev.eventSrc == p->eventSrc ){
	      p->action( &ev );
	      goto actionend;
	    }
	  }
	}
      actionend:
	;
      }
    }else{
      if( evhCoreClass.stopping_flag ) break;
      BUGLPR(evhvar, BUGACT, ( "sleep\n" ));
      e_sleep_thread( &evhCoreClass.evhproc_word, &evhCoreClass.configlock,
		     LOCK_READ );
      BUGLPR(evhvar, BUGACT, ( "wakeup\n" ));
    }
  }

  BUGLPR(evhvar, BUGACT, ( "exiting event handler\n" )); 
  e_wakeup( &evhCoreClass.configproc_word );
  lock_done( &evhCoreClass.configlock );
  BUGLPR(evhvar, BUGACT, ( "kproc exit\n" ));
}

/****************************************************************************
*
* NAME:  classlock_read
*
*   FUNCTION:         perform lock_read( &evhCoreClass.configlock ) and
*                     check the stopping_flag
*                     
*   ARGUMENTS:        l - lock word ( should be &evhCoreClass.configlock )
*   RETURN VALUE:     ENV_EVH  ( O.K. return with read_lock )
*                     ENV_CLI  ( O.K. return with read_lock )
*                     ENV_STOP ( return without lock )
*   ENVIRONMENT:      client thread
*
****************************************************************************/
static int classlock_read( complex_lock_t l )
{
  if( evhCoreClass.kpid == getpid() ){
    return( ENV_EVH );
  }else if( evhCoreClass.stopping_flag ){
    return( ENV_STOP );
  }else{
    lock_read( l );
    return( ENV_CLI );
  }
}

/****************************************************************************
*
* NAME: classlock_rdone 
*
*   FUNCTION:         release lock for a class. If the callling thread
*                     is event handler change write lock to read lock
*                     
*   ARGUMENTS:        - l lock word ( should be &evhCoreClass.configlock )
*   RETURN VALUE:     ENV_EVH ( calling thread is event handler )
*                     ENV_CLI ( calling thread is not event handler )
*                     ENV_STOP ( evhCoreClass.stopping_flag is on )
*   ENVIRONMENT:      client thread
*
****************************************************************************/
static int classlock_rdone( complex_lock_t l )
{
 if( getpid() == evhCoreClass.kpid ){
    return( ENV_EVH );
  }else if( evhCoreClass.stopping_flag ){
    return( ENV_STOP );
  }else{
    lock_done( l );
    return( ENV_CLI );
  }
}

/****************************************************************************
*
* NAME:  ValidateClient_lock
*
*   FUNCTION:         check the value of client ID
*                     and if it is valid, return with holding evhCoreClass.
*                     configlock=r
*                     
*   ARGUMENTS:        - cl  client ID to validate
*   RETURN VALUE:     EVR_SUCCESS ( good client ID )
*                     EVR_BAD_CLIENT ( bad client ID )
*   ENVIRONMENT:      client thread
*   LOCKS:            evhCoreClass.configlock=r
*   Calls with Locks: (none)
*
****************************************************************************/
static int ValidateClient_lock( Client* cl )
{
  Client* p;

  if( classlock_read( &evhCoreClass.configlock ) == ENV_STOP ){
    return( EVR_BAD_CLIENT );
  }

  for( p = clientClass.clients; p != cl && p != 0; p = p->next );

  if( p != 0 && p->valid ){
    return( EVR_SUCCESS );
  }else{
    classlock_rdone( &evhCoreClass.configlock );
    return( EVR_BAD_CLIENT );
  }
}

/****************************************************************************
*
* NAME:  refreshSlot
*
*   FUNCTION:         Refresh hardware interrupt mask according to all of
*                     eventMasks and slotMasks
*                     
*   ARGUMENTS:        - slot specify the slot number to refresh. If NO_SLOT
*                            is specifyed, all of slot will be refreshed.
*   RETURN VALUE:     (none)
*   ENVIRONMENT:      client thread with evhCoreClass.configlock=r
*                     event handler with evhCoreClass.configlock=r
*                     usr thread with evhCoreClass.configlock=r
*   LOCKS      :      evhCore.lock=s
*   Calls with Locks: cs.c#SetIntrMask() with evhCoreClass.configlock=r
*
****************************************************************************/
static void refreshSlot( int slot )
{
  EvhCore* evh;
  Client* cl;
  SlotMask* sm;
  unsigned int eventMask = 0;

  if( slot == NO_SLOT ){
    unsigned int eventMask0 = 0;

    for( cl = clientClass.clients; cl; cl = cl->next ){
      if( cl->valid ) eventMask0 |= cl->eventMask;
    }

    for( evh = evhCoreClass.evhCores; evh; evh = evh->next ){
      eventMask = eventMask0;
      simple_lock( &evh->lock );
      for( sm = evh->slotMasks; sm; sm = sm->next ){
	if( sm->valid ) eventMask |= sm->eventMask;
      }
      simple_unlock( &evh->lock );
      SetIntrMask( evh->slot, eventMask );
    }
  }else{
    if( ( evh = selectslot( slot ) ) == 0 ){
      return;
    }

    for( cl = clientClass.clients; cl; cl = cl->next ){
      if( cl->valid ) eventMask |= cl->eventMask;
    }
    simple_lock( &evh->lock );
    for( sm = evh->slotMasks; sm; sm = sm->next ){
      if( sm->valid ) eventMask |= sm->eventMask;
    }
    simple_unlock( &evh->lock );
    SetIntrMask( slot, eventMask );
  }
}

static Usr_ClientClass usr_ClientClass = {
  0, /* usr_clients */
};

#if defined( UDEBUG )
static void disp_queues()
{
  EventQueue* q;
  Usr_Client* cl;

  uprintf( "Used queues : " );
  for( q = usr_ClientClass.used_queues; q; q = q->next ){
    uprintf( "<%x> ",
	    ( q - usr_ClientClass.usr_eventQueues ) );
  }
  uprintf( "\n" );

  uprintf( "Free queues : " );
  for( q = usr_ClientClass.free_queues; q; q = q->next ){
    uprintf( "<%x> ",
	    ( q - usr_ClientClass.usr_eventQueues ) );
  }
  uprintf( "\n" );

  uprintf( "Usr clients : " );
  for( cl = usr_ClientClass.usr_clients; cl; cl = cl->next ){
    if( cl->client.valid ){
      uprintf( "<%x> ", cl );
    }
  }
  uprintf( "\n" );
}
#endif /* UDEBUG */

/****************************************************************************
*
* NAME:  init_Usr_ClientClass
*
*   FUNCTION:         Initialize User_ClientClass data
*                     
*   RETURN VALUE:     void
*   ENVIRONMENT:      config thread with init_lock
*   LOCKS:            (none)
*   Calls with Locks: (none)
*
****************************************************************************/
static void init_Usr_ClientClass()
{
  int i;
  EventQueue* q;

#if defined( UDEBUG )
uprintf( "init_Usr_ClientClass-->\n" );
#endif /* UDEBUG */

  usr_ClientClass.newevent = EVENT_NULL;
  lock_alloc( &usr_ClientClass.queuelock, LOCK_ALLOC_PIN, CS_QUEUE_LOCK, -1 );
  simple_lock_init( &usr_ClientClass.queuelock );

  for( i = 0, q = 0; i < MAX_USR_EVENTS; i++ ){
    usr_ClientClass.usr_eventQueues[i].next = q;
    q = usr_ClientClass.usr_eventQueues + i;
  }
  usr_ClientClass.free_queues = q;
#if defined( UDEBUG )
disp_queues();
#endif /* UDEBUG */
}

/****************************************************************************
*
* NAME:  uninit_Usr_ClientClass
*
*   FUNCTION:         Uninitialize User_ClientClass data
*                     
*   RETURN VALUE:     void
*   ENVIRONMENT:      config thread with evhCoreClass.configlock=w
*                     and evhCoreClass.stopping_flag=1
*   LOCKS:            (none)
*   Calls with Locks: (none)
*
****************************************************************************/
static void uninit_Usr_ClientClass()
{
  /* This would not be required */
  /*e_wakeup( &usr_ClientClass.newevent );*/
}

/****************************************************************************
*
* NAME:  usr_enqueue
*
*   FUNCTION:         Enqueue a data for processes in user space.
*                     A user process will call user_dequeue() through
*                     svccfehGetEvent().
*                     
*   ARGUMENT:         - ev             event queue item.
*                                      ev->slot, ev->eventSrc should be set.
*                                      If ev->slot is invalid, unexpected
*                                      result may happen.
*                     - flag           If FLUSH_EVENTS_ON_THE_SLOT bit is 1,
*                                      all previous events on the slot will
*                                      be flushed. Otherwise, all previous
*                                      events will be kept.
*   RETURN VALUE:     EVR_SUCCESS      success
*                     EVR_IN_USE       all queue items are in use
*   ENVIRONMENT:      event handler with evhCore.configlock=r
*   LOCKS:            usr_ClientClass.queuelock=s
*   Calls with Locks: (none)
*
****************************************************************************/
int usr_enqueue( EventQueue* ev, int flag )
{
  EventQueue* q;
  EventQueue** r_q;
  Usr_Client* cl;
  int dup;

#if defined( DEBUG )
printf( "usr_enqueue( %x, %x )\n", ev->eventSrc, ev->slot );
#endif /* DEBUG */

  simple_lock( &usr_ClientClass.queuelock );

  /* First, move read(unrequired) queue items to free queue */
  for( q = usr_ClientClass.used_queues; q; ){
    for( cl = usr_ClientClass.usr_clients; cl; cl = cl->next ){
      if( cl->client.valid && cl->read == q ) break;
    }

    if( cl ){
      break;
    }else{
      EventQueue* q2 = q->next;
      q->next = usr_ClientClass.free_queues;
      usr_ClientClass.free_queues = q;
      q = q2;
    }
  }

  usr_ClientClass.used_queues = q;
  /* ev->slot value check is not performed. It is trusted. */

  /* flush old queued data which is not required */
  if( flag & FLUSH_EVENTS_ON_THE_SLOT ){
    for( r_q = &usr_ClientClass.used_queues; *r_q; ){
      if( (*r_q)->slot == ev->slot ){
	EventQueue* q2 = *r_q;
	*r_q = q2->next;
	q2->next = usr_ClientClass.free_queues;
	usr_ClientClass.free_queues = q2;

	for( cl = usr_ClientClass.usr_clients; cl; cl = cl->next ){
	  if( cl->client.valid && cl->read == q2 ) cl->read = *r_q;
	}
      }else{
	r_q = &(*r_q)->next;
      }
    }
  }

  /* compress events if the previous event is not read by any client and
     it has the same event type and slot number */
  dup = 0;
  for( cl = usr_ClientClass.usr_clients; cl; cl = cl->next ){
    if( cl->client.valid && cl->read == 0 ) break;
  }
  if( !cl ){
   for( q = usr_ClientClass.used_queues; q; q = q->next ){
    for( cl = usr_ClientClass.usr_clients; cl; cl = cl->next ){
      if( cl->client.valid && cl->read == q ){
        dup = 0;
        break;
      }
    }
    if( q->slot == ev->slot ) dup = ( q->eventSrc == ev->eventSrc );
   }
  }

  /* queues new data */
  if( !dup && ( q = usr_ClientClass.free_queues ) ){
    usr_ClientClass.free_queues = q->next;

    q->eventSrc = ev->eventSrc;
    q->slot = ev->slot;
    q->next = 0;
    /* requester and target are not refered */
    for( r_q = &usr_ClientClass.used_queues; *r_q; r_q = &(*r_q)->next );
    *r_q = q;

    /* set clients' read pointers if null */
    for( cl = usr_ClientClass.usr_clients; cl; cl = cl->next ){
      if( cl->client.valid && cl->read == 0 ) cl->read = q;
    }
  }

  simple_unlock( &usr_ClientClass.queuelock );
#if defined( DEBUG )
printf( "usr_enqueue end-->\n" );
#endif /* DEBUG */
#if defined( UDEBUG )
disp_queues();
#endif /* UDEBUG */
}

/****************************************************************************
*
* NAME:  usr_dequeue
*
*   FUNCTION:         Read a queued data for a user space process.
*                     
*                     
*   ARGUMENT:         - cl   clientID for a user space process.
*                            no client ID validation is done.
*                     - ev   Reference to the buffer to store a queued data.
*                            ev->eventSrc and ev->slot are valid values.
*                            ev->target, ev->me and ev->next are invalid.
*   RETURN VALUE:     non 0 on success.
*                     0 if no more items.
*   ENVIRONMENT:      usr thread with evhCore.configlock=r
*   LOCKS:            usr_ClientClass.queuelock=s
*   Calls with Locks: (none)
*
****************************************************************************/
static EventQueue* usr_dequeue( Usr_Client* cl, EventQueue* ev )
{
  EventQueue* q;

#if defined( UDEBUG )
  static int calls;
uprintf( "usr_dequeue<%d> -->\n", ++calls );

disp_queues();

#endif /* UDEBUG */

  simple_lock( &usr_ClientClass.queuelock );

  for( q = usr_ClientClass.used_queues; q; q = q->next ){
    if( q == cl->read ){
      cl->read = q->next;
      ev->eventSrc = q->eventSrc;
      ev->slot = q->slot;
      break;
    }
  }

  simple_unlock( &usr_ClientClass.queuelock );
  return( q );
}

/****************************************************************************
*
* NAME:  const_Client
*
*   FUNCTION:         Construct a client
*                     
*   ARGUMENTS:        - cl          client pointer if the data area is already
*                                   allocated. Otherwise, 0 should be specifyed.
*                     - callback    callback entry point
*                     - attributes  client attribute flags
*                                ( fixed as CL_TYPE1|CL_EXCL )
*                     - eventMask   event mask for the client
*                                EVM_CDChg          Card Detect Change
*                                EVM_ReadyChg       Ready signal Change
*                     - clientData  pointer that will be passwd as an
*                                   argument on callback
*   RETURN VALUE:     clientID ( O.K. )
*                     0 ( xmalloc() error, or invalid arguments )
*   ENVIRONMENT:      client or user thread
*   LOCKS:            (none)
*   Calls with Locks: (none)
*
****************************************************************************/
static Client* const_Client( Client* cl, int (*callback)(), int attributes,
         unsigned int eventMask, char* clientData
)
{
  int i;
  static int dest_Client( Client* cl );

#if defined( UDEBUG )
uprintf( "const_client -->\n" );
#endif /* UDEBUG */

  if( cl == 0 ){
    Client* new = xmalloc( sizeof( Client ), sizeof( int ), pinned_heap );
    if( new == 0 ){
      return( 0 );
    }
    cl = new;
  }
  bzero( cl, sizeof( Client ) );
  lock_alloc( &cl->lock, LOCK_ALLOC_PIN, CS_CLIENT_LOCK, -1 );
  simple_lock_init( &cl->lock );
  for( i = 0; i < 4; i++ ){
    cl->slot[i] = NO_SLOT;
    cl->slot_refcount[i] = 0;
  }
  cl->destruct = dest_Client;
  cl->callback = callback;
  cl->attributes = attributes;
  cl->eventMask = eventMask;
  cl->clientData = clientData;
  cl->valid = 1;

  return( cl );
}


/****************************************************************************
*
* NAME:  dest_Client
*
*   FUNCTION:         Destruct a client
*                     
*   ARGUMENTS:        - cl clientID
*   RETURN VALUE:     status ( EVR_SUCCESS on success )
*   ENVIRONMENT:      user thread or event handler
*                      with evhCoreClass.configlock=w
*   LOCKS:            (none)
*   Calls with Locks: (none)
*
****************************************************************************/
static int dest_Client( Client* cl )
{
  Client** p;
  
  for( p = &clientClass.clients; *p; p = &(*p)->next ){
    if( cl == *p ){
      *p = cl->next;
      lock_free( &cl->lock );
      xmfree( cl, pinned_heap );
      return( EVR_SUCCESS );
    }
  }

  return( EVR_BAD_CLIENT );
}


/****************************************************************************
*
* NAME:  register_Client
*
*   FUNCTION:         Register a client. After the registration, a client
*                     is treated as a valid client and can get a callback.
*                     
*   ARGUMENTS:        - cl   clientID to be registered
*   RETURN VALUE:     status code ( EVR_SUCCESS on success )
*   ENVIRONMENT:      user thread or event handler thread
*                      with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=w
*   Calls with Locks: refreshSlot() with evhCoreClass.configlock=r
*                     (*cl->destruct)() with evhCoreClass.configlock=w
*
****************************************************************************/
static int register_Client( Client* cl )
{
  Client** p;
  Client *q, *r;

#if defined( UDEBUG )
uprintf( "register_client -->\n" );
#endif /* UDEBUG */

  lock_read_to_write( &evhCoreClass.configlock );

  /* garbage collection of client list */
  for( r = clientClass.clients; r; ){
    q = r;
    r = q->next;
    if( !q->valid ){
      (*q->destruct)( q );
    }
  }

  if( cl->attributes & CL_TYPE1 ){ /* IO CDD LIFO order */
 BUGLPR(evhvar, BUGGID, ("iodd\n"));
    cl->next = clientClass.clients;
    clientClass.clients = cl;
  }else if( cl->attributes & CL_TYPE2 ){ /* MTD FIFO order */
 BUGLPR(evhvar, BUGGID, ("mtd\n"));
    for( p = &clientClass.clients; *p; p = &((*p)->next) ){
      if( ( (*p)->attributes & ( CL_TYPE1 | CL_TYPE2 ) ) == 0 ) break;
    }
    cl->next = *p;
    *p = cl;
  }else if ( cl->attributes & CL_TYPE3 ){ /* CL_TYPE3 : mem client FIFO order */
 BUGLPR(evhvar, BUGGID, ("memdd\n"));
    for( p = &clientClass.clients; *p; p = &((*p)->next) ){
      if( ( (*p)->attributes & ( CL_TYPE1 | CL_TYPE2 | CL_TYPE3 ) ) == 0 ) break;
    }
    cl->next = *p;
    *p = cl;
  }else{ /* CL_USR */
    for( p = &clientClass.clients; *p; p = &((*p)->next) );
    *p = cl;
  }

  lock_write_to_read( &evhCoreClass.configlock );

  refreshSlot( NO_SLOT );
  return( EVR_SUCCESS ); 
}

/****************************************************************************
*
* NAME:  unregister_Client
*
*   FUNCTION:         Unregister a client
*                     
*   ARGUMENTS:        - client   clientID to be unregistered
*   RETURN VALUE:     status code ( EVR_SUCCESS on success )
*   ENVIRONMENT:      user thread with evhCoreClass.configlock=r
*                     client thread with evhCoreClass.configlock=r
*   LOCKS:            (none)
*   Calls with Locks: refreshSlot() with evhCoreClass.configlock=r
*
****************************************************************************/
static int unregister_Client( Client* client )
{
  int ret, i;

#if defined( UDEBUG )
  uprintf( "start to unregister...\n" );
#endif /* UDEBUG */
  simple_lock( &client->lock );
  if( !client->valid ){
    ret = EVR_BAD_CLIENT;
  }else if( client->slot[0] == NO_SLOT && client->slot[1] == NO_SLOT &&
	client->slot[2] == NO_SLOT && client->slot[3] == NO_SLOT ){
    client->valid = 0;
    client->attributes &= ~(unsigned)(CL_USR|CL_SHR|CL_EXCL);
    client->eventMask = 0;
    ret = EVR_SUCCESS;
  }else{
    ret = EVR_IN_USE;
  }
  simple_unlock( &client->lock );

  refreshSlot( NO_SLOT );

  return( ret );
}

/****************************************************************************
*
* NAME:  const_Usr_Client
*
*   FUNCTION:         Construct a usr_Client
*                     
*   ARGUMENTS:        - cl      pointer to the memory if the data area is
*                               already allocated.
*                     - tid     thread ID for the newly creating usr-client.
*   RETURN VALUE:     clientID
*                     or 0 ( error )
*   ENVIRONMENT:      user thread
*   LOCKS:            (none)
*   Calls with Locks: (none)
*
****************************************************************************/
static Usr_Client* const_Usr_Client( Usr_Client* cl, tid_t tid )
{
  static int dest_Usr_Client( Usr_Client* cl );

#if defined( UDEBUG )
  uprintf( "const_usr_client -->\n" );
#endif /* UDEBUG */

  /* validation of tid is preferable */

  if( cl == 0 ){
    Usr_Client* new = xmalloc( sizeof( Usr_Client ), sizeof(int), pinned_heap );
    if( new == 0 ){
      return( 0 );
    }
  }

  cl = (Usr_Client*)const_Client( (Client*)cl, 0, CL_USR, 0, (char*)tid );
  cl->client.destruct = dest_Usr_Client;
  cl->tid = tid;
  cl->next = 0;

  return( cl );
}

/****************************************************************************
*
* NAME:  dest_Usr_Client
*
*   FUNCTION:         Destruct a usr-client
*                     
*   ARGUMENTS:        - cl clientID
*   RETURN VALUE:     status ( EVR_SUCCESS on success )
*   ENVIRONMENT:      user thread or event handler
*                      with evhCoreClass.configlock=w
*   LOCKS:            (none)
*   Calls with Locks: (none)
*
****************************************************************************/
static int dest_Usr_Client( Usr_Client* cl )
{
  Usr_Client** p, *q;
  
  for( p = &usr_ClientClass.usr_clients; *p; p = &(*p)->next ){
    if( cl == *p ){
      q = cl->next;
      if( dest_Client( &cl->client ) == EVR_SUCCESS ){
	*p = q;
	return( EVR_SUCCESS );
      }
      break;
    }
  }

  return( EVR_BAD_CLIENT );
}


/****************************************************************************
*
* NAME:  register_Usr_Client
*
*   FUNCTION:         Register a usr_Client
*                     
*   ARGUMENTS:        - q   clientID to be registered
*   RETURN VALUE:     status code ( EVR_SUCCESS on success )
*   ENVIRONMENT:      user thread
*   LOCKS:            evhCoreClass.configlock=r,w
*   Calls with Locks: register_Client() with evhCoreClass.configlock=r
*                     (*q->client.destruct)() with evhCoreClass.configlock=w
*
****************************************************************************/
static int register_Usr_Client( Usr_Client* cl )
{
  Usr_Client** p;
  Usr_Client *q, *r;

#if defined( UDEBUG )
uprintf( "register_usr_client -->\n" );
#endif /* UDEBUG */
  lock_write( &evhCoreClass.configlock );
  /* garbage collection of usr-client list */
  for( r = usr_ClientClass.usr_clients; r; ){
    q = r;
    r = q->next;
    /* validation of tid is preferable */
    if( !q->client.valid ){
      (*q->client.destruct)( q );
    }
  }
  lock_write_to_read( &evhCoreClass.configlock );
  /* evhCore.configlock=r must not be released */
  register_Client( (Client*)cl );

  lock_read_to_write( &evhCoreClass.configlock );
  for( p = &usr_ClientClass.usr_clients; *p; p = &((*p)->next) );
  *p = cl;
  cl->read = 0;
  lock_done( &evhCoreClass.configlock );
#if defined( UDEBUG )
disp_queues();
#endif /* UDEBUG */
  return( EVR_SUCCESS );
}

/****************************************************************************
*
* NAME:  signal_Usr_Client
*
*   FUNCTION:         wakeup all blocked usr_clients
*                     
*   RETURN VALUE:     status code ( EVR_SUCCESS on success )
*   ENVIRONMENT:      event handler with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=w
*   Calls with Locks: (none)
*
****************************************************************************/
int signal_Usr_Client()
{
  Usr_Client* cl;

#if defined( UDEBUG )
  static int calls;
  calls++;
#endif /* UDEBUG */

#if defined( DEBUG )
printf( "signal_usr_client" );
#endif /* DEBUG */
  lock_read_to_write( &evhCoreClass.configlock );
  e_wakeup( &usr_ClientClass.newevent );
  lock_write_to_read( &evhCoreClass.configlock );

#if defined( DEBUG )
printf( "signal_usr_client end\n" );
#endif /* DEBUG */
  return( EVR_SUCCESS );
}

/****************************************************************************
*
* NAME:  svccfehRegisterHanlder
*
*   FUNCTION:         System call to register handler to get configuration
*                     change events
*                     
*   RETURN VALUE:     client ID on success
*                     0 on error
*   ENVIRONMENT:      usr thread
*   LOCKS:            (none)
*   Calls with Locks: (none)
*
****************************************************************************/
int svccfehRegisterHandler()
{
  Usr_Client* cl;

  cl = const_Usr_Client( 0, thread_self() );
  if( cl ){
    register_Usr_Client( cl );
  }

#if defined( UDEBUG )
uprintf( "svc:registerhandler:cl = %x\n", cl );
uprintf( "svc:registerhandler:tid = %d\n", cl->tid );
#endif

  return( (int)cl );
}

/****************************************************************************
*
* NAME:  svccfehGetEvent
*
*   FUNCTION:         System call to get a configuration event for a user
*                     process.
*                     
*   ARGUMENT:         - cl             client ID of the calling process.
*                     - devn           buffer pointer to store detecting device
*                                      name.
*                     - devsize        maximum buffer size
*                     - slot           reference to slot number where the
*                                      event happened.
*                     - eventSrc       reference to event code
*   RETURN VALUE:     0 on success.
*                     non 0 on failure.
*   ENVIRONMENT:      usr thread
*   LOCKS:            evhCoreClass.configlock=r
*   Calls with Locks: user_dequeue() evhCoreClass.configlock=r
*
****************************************************************************/
int svccfehGetEvent( int cl, char* devn, int devsize, int* slot, int* eventSrc )
{
  static EventQueue ev;
  EventQueue* q;
  int i, ret;
  EvhCore* evh;

#if defined( DEBUG )
printf( "svcgetevent:cl = %x\n", cl );
#endif

  /* devsize must be more than 2 */
  if( devsize < 2 ) return( 1 );

  ev.slot = ev.eventSrc = 0;

  if( ( ret = ValidateClient_lock( (Client*)cl ) ) != EVR_SUCCESS ){
    return( ret );
  }

  if( thread_self() != ((Usr_Client*)cl)->tid ){
    lock_done( &evhCoreClass.configlock );
    return( EVR_BAD_CLIENT );
  }

  do{
#if defined( DEBUG )
printf( "before usr_dequeue\n" );
#endif /* DEBUG */
    q = usr_dequeue( (Usr_Client*)cl, &ev );

    if( q != 0 ) break;

#if defined( DEBUG )
printf( "enter e_sleep_thread\n" );
#endif /* DEBUG */
    ret = e_sleep_thread( &usr_ClientClass.newevent, &evhCoreClass.configlock,
			 INTERRUPTIBLE | LOCK_READ );

#if defined( DEBUG )
printf( "wakeup from e_sleep_thread with %d\n", ret );
#endif /* DEBUG */
    if( ret == THREAD_INTERRUPTED || evhCoreClass.stopping_flag ){
      lock_done( &evhCoreClass.configlock );
      return( 1 );
    }
  }while( 1 );

#if defined( DEBUG )
printf( "svcgetevent-selectslot:cl = %x\n", cl );
#endif
  evh = selectslot( ev.slot );
  for( i = 0; i < devsize - 1 && evh->devname[i]; i++ );
  copyout( evh->devname, devn, i + 1 );

  lock_done( &evhCoreClass.configlock );

  copyout( &ev.eventSrc, eventSrc, sizeof( int ) );
  copyout( &ev.slot, slot, sizeof( int ) );

#if defined( DEBUG )
printf( "svcgetevent-end:cl = %x\n", cl );
#endif
  return( !q );
}

/****************************************************************************
*
* NAME:  svccfehDeregisterHanlder
*
*   FUNCTION:         System call to unregister handler.
*                     
*   ARGUMENT:         - cl    client ID to be unregistered.
*   RETURN VALUE:     EVR_SUCCESS ( good client ID )
*                     EVR_BAD_CLIENT ( bad client ID )
*   ENVIRONMENT:      usr thread
*   LOCKS:            (none)
*   Calls with Locks: unregister_Client with evhCoreClass.configlock=r
*
****************************************************************************/
int svccfehDeregisterHandler( int cl )
{
  int rc;
  Usr_Client* ucp = (Usr_Client*)cl;

  if( ( rc = ValidateClient_lock( (Client*)cl ) ) != EVR_SUCCESS ){
      return( rc );
  }

  if( thread_self() != ucp->tid ){
    lock_done( &evhCoreClass.configlock );
    return( EVR_BAD_CLIENT );
  }

  rc = unregister_Client( (Client*)cl );
  lock_done( &evhCoreClass.configlock );

#if defined( UDEBUG )
disp_queues();
#endif /* UDEBUG */
#if defined( UDEBUG )
uprintf( "svcderegisterhandler:cl = %x\n", cl );
#endif

  return( rc );
}

/****************************************************************************
*
* NAME:  cfehValidateClient
*
*   FUNCTION:         check the value of client ID
*                     
*   ARGUMENTS:        - cl  client ID to validate
*   RETURN VALUE:     EVR_SUCCESS ( good client ID )
*                     EVR_BAD_CLIENT ( bad client ID )
*   ENVIRONMENT:      client thread
*   LOCKS:            evhCoreClass.configlock=r
*   Calls with Locks: (none)
*
****************************************************************************/
int cfehValidateClient( Client* cl )
{
  int rc;

  if( ( rc = ValidateClient_lock( cl ) ) != EVR_SUCCESS ){
      return( rc );
  }

  classlock_rdone( &evhCoreClass.configlock );
  return( EVR_SUCCESS );
}

/****************************************************************************
*
* NAME:  cfehClientSetSlot
*
*   FUNCTION:         set logical slot number which the client is interested
*                     in. ( requestResource will call this function )
*                     
*   ARGUMENTS:        - cl  client ID
*                     - slot logical slot number
*   RETURN VALUE:     EVR_SUCCESS
*                     EVR_BAD_CLIENT
*                     EVR_BAD_SLOT
*   ENVIRONMENT:      client thread
*   LOCKS:            evhCoreClass.configlock=r
*                     client.lock=w
*   Calls with Locks: selectslot() with evhCoreClass.configlock=r
*
****************************************************************************/
int cfehClientSetSlot( Client* cl, int slot )
{
  int ret, i;
  Client* p;

  if( ( ret = ValidateClient_lock( cl ) ) != EVR_SUCCESS ){
    return( ret );
  }

  if( selectslot( slot ) == 0 ){
    classlock_rdone( &evhCoreClass.configlock );
    return( EVR_BAD_SLOT );
  }

  simple_lock( &cl->lock );
  if( cl->valid == 0 ){
    ret = EVR_BAD_CLIENT;
  }else{
    ret = EVR_BAD_SLOT;
    for( i = 0; i < 4 && ret == EVR_BAD_SLOT; i++ ){
      if( cl->slot[i] == slot ){
        cl->slot_refcount[i]++;
        ret = EVR_SUCCESS;
      }
    }
    for( i = 0; i < 4 && ret == EVR_BAD_SLOT; i++ ){
      if( cl->slot[i] == NO_SLOT ){
        cl->slot[i] = slot;
        cl->slot_refcount[i] = 1;
        ret = EVR_SUCCESS;
      }
    }
  }
  simple_unlock( &cl->lock );

  classlock_rdone( &evhCoreClass.configlock );
  return( ret );
}

/****************************************************************************
*
* NAME:  cfehClientResetSlot
*
*   FUNCTION:         clear logical slot number which the client is interested
*                     in. ( releaseResource will call this function )
*                     
*   ARGUMENTS:        - cl   client ID
*                     - slot slot number
*   RETURN VALUE:     EVR_SUCCESS
*                     EVR_BAD_CLIENT
*                     EVR_BAD_SLOT
*   ENVIRONMENT:      client thread
*   LOCKS:            evhCoreClass.configlock=r, client.lock=w
*   Calls with Locks: (none)
*
****************************************************************************/
int cfehClientResetSlot( Client* cl, int slot )
{
  int ret, i;
  Client* p;

  if( ( ret = ValidateClient_lock( cl ) ) != EVR_SUCCESS ){
    return( ret );
  }
  simple_lock( &cl->lock );
  if( cl->valid == 0 ){
    ret = EVR_BAD_CLIENT;
  }else if( slot == NO_SLOT ){
    ret = EVR_BAD_SLOT;
  }else{
    ret = EVR_BAD_SLOT;
    for( i = 0; i < 4 && ret == EVR_BAD_SLOT; i++ ){
      if( cl->slot[i] == slot && cl->slot_refcount[i] > 0 ){
        cl->slot_refcount[i]--;
        if( cl->slot_refcount[i] == 0 ){
          cl->slot[i] = NO_SLOT;
        }
        ret = EVR_SUCCESS;
      }
    }
  }
  simple_unlock( &cl->lock );
  classlock_rdone( &evhCoreClass.configlock );
  return( ret );
}

/****************************************************************************
*
* NAME:  GetValidClient
*
*   FUNCTION:         get a valid client pointer from the given pointer.
*                     
*   ARGUMENTS:        - slot get client entry who is interested in the slot.
*                            NO_SLOT means dont care
*                     - p    pointer to the client list.
*   RETURN VALUE:     client ID ( O.K. )
*                     0     ( no client exist )
*   ENVIRONMENT:      client thread with evhCoreClass.configlock = r or
*                     Event Handler with evhCoreClass.configlock = r
*   LOCKS      :      client.lock = s
*   Calls with Locks: 
*
****************************************************************************/
static Client* GetValidClient( int slot, Client* p )
{
  int cnd;

  for( ; p; p = p->next ){
    simple_lock( &p->lock );
    cnd = ( p->valid && ( slot == NO_SLOT || slot == p->slot[0] ||
		slot == p->slot[1] || slot == p->slot[2] ||
		slot == p->slot[3] ) );
    simple_unlock( &p->lock );
    if( cnd ) break;
  }

  return( p );
}

/****************************************************************************
*
* NAME:  callbackone
*
*   FUNCTION:         call one callback entry. During the callback, callback
*                     counter ( cl->calling ) is incremented, and after the
*                     completion of callback, the counter is decremented.
*                     
*   ARGUMENTS:        - cl        client ID to be called.
*                     - event     event code that to be sent
*                     - slot      slot number which event is associated with
*                     - info      Info argument that will be passed to clients
*                     - buf       Buffer argument that iwll be passed to clients
*   RETURN VALUE:     0=EVR_SUCCESS
*                     EVR_BAD_CLIENT
*                     (return value from client's callback )
*   ENVIRONMENT:      Event Handler with evhCoreClass.configlock=r or
*                     client thread with evhCoreClass.configlock=r
*   LOCKS:            client.lock=s
*   Calls with Locks: (*client->callback)() with evhCoreClass.configlock=r
*
****************************************************************************/
int callbackone( Client* cl, int event, int slot, int info, char* buf )
{
  int rc;
  int f;
  simple_lock( &cl->lock );
  f = cl->valid;
  if( f ) cl->calling++;
  simple_unlock( &cl->lock );

  if( cl->valid ){
    if( cl->callback ){
      rc = (*cl->callback)( event, slot, info, 0, buf, 0, cl->clientData );
    }else{
      rc = EVR_SUCCESS;
    }
  }else{
    rc = EVR_BAD_CLIENT;
  }

  simple_lock( &cl->lock );
  /* should use f not cl->valid */
  if( f ) cl->calling--;
  simple_unlock( &cl->lock );

  return( rc );
}

/****************************************************************************
*
* NAME:  callbacksm
*
*   FUNCTION:         call callback for the specifyed slot mask list.
*                     
*   ARGUMENTS:        - sm        pointer to the slot mask list.
*                     - em        event mask of the event
*                     - event     event code that to be sent
*                     - slot      slot number which event is associated with
*                     - info      Info argument that will be passed to clients
*                     - buf       Buffer argument that iwll be passed to clients
*                     - l         simple lock for the slotmask pointers
*   RETURN VALUE:     0=EVR_SUCCESS
*                     (return value from client's callback )
*   ENVIRONMENT:      Event Handler with evhCoreClass.configlock=r
*   LOCKS:            evhCore.lock = s
*   Calls with Locks: callbackone() with evhCoreClass.configlock=r
*
****************************************************************************/
int callbacksm( SlotMask* sm, unsigned int em, int event, int slot, int info, char* buf, simple_lock_t l )
{
  int rc, ret;

  ret = 0;

  if( em == 0 ) return( 0 );;

  for( ; sm; ){
    if( sm->valid && ( em & sm->eventMask ) ){
      rc = callbackone( sm->client, event, slot==NO_SLOT?0:slot, info, buf );
      if( rc != EVR_BAD_CLIENT ) ret |= rc;
    }
    simple_lock( l );
    sm = sm->next;
    simple_unlock( l );
  }

  return( ret );
}

/****************************************************************************
*
* NAME:  callbackall
*
*   FUNCTION:         call callback entries in clients with event code
*                     
*   ARGUMENTS:        - ev        event queue data that caused callback
*                     - event     event code that to be sent
*                     - slot      slot number which event is associated with
*                                 NO_SLOT means event is not associated with
*                                 a specific slot and converted to 0 before
*                                 calling each client's callback entry.
*                     - bctype  = ALL       all clients get the event
*                               = SLOT      all clients who interested in the
*                                           slot will get the event
*                               = REQUESTER only requesting client will get
*                                           the event
*                               = TARGET    only target client will get the
*                                           event ( for CLIENT_INFO )
*                     - eventMask eventmask value for the event code
*                     - Info      Info argument that will be passed to clients
*                     - Buffer    Buffer argument that iwll be passed to clients
*   RETURN VALUE:     0=EVR_SUCCESS ( all of callbacks return EVR_SUCCESS )
*                     EVR_BAD_SLOT
*                     not 0 ( some of callbacks return non EVR_SUCCESS )
*   ENVIRONMENT:      event handler with evhCoreClass.configlock=r
*   LOCKS:            
*   Calls with Locks: selectslot()  with evhCoreClass.configlock=r
*                     callbackone() with evhCoreClass.configlock=r
*                     callbacksm() with evhCoreClass.configlock=r
*
****************************************************************************/
int callbackall( EventQueue* ev, int event, int slot, int bctype, unsigned int eventMask, int Info, char* Buffer )
{
  int ret, rc;
  Client* p;
  EvhCore* evh;
  SlotMask* sm;
  Client* exclusiveUser;

  BUGLPR( evhvar, BUGACT, ( "callbackall: event = %d, slot = %d, bctype = %d, em = %x, Info = %d, buf = %x\n",\
event, slot, bctype, eventMask, Info, Buffer )); 
  if( slot == NO_SLOT ){
    sm = 0;
    exclusiveUser = 0;
  }else{
    if( ( evh = selectslot( slot ) ) != 0 ){
      sm = evh->slotMasks;
      exclusiveUser = evh->ExclusiveUser;
    }else{
      /* should not come here for this function is called with configlock */
      return( EVR_BAD_SLOT );
    }
  }

  if( bctype != SLOT && slot == NO_SLOT ) slot = 0;
  ret = 0;

  switch( bctype ){
  case ALL:
    for( p = clientClass.clients; p; p = p->next ){
      if( eventMask & p->eventMask || eventMask == 0 ){
	rc = callbackone( p, event, slot, Info, Buffer );
	if( rc != EVR_BAD_CLIENT ) ret |= rc;
      }
    }

    ret |= callbacksm( sm, eventMask, event, slot, Info, Buffer, &evh->lock );
    break;
  case SLOT:
    if( exclusiveUser == 0 ){
      for( p = clientClass.clients; p; p = p->next ){
	if( ( ( eventMask & p->eventMask ) || eventMask == 0 ) 
	   && (slot == p->slot[0] || slot == p->slot[1] ||
		slot == p->slot[2] || slot == p->slot[3] ) ){
	  rc = callbackone( p, event, slot, Info, Buffer );
	  if( rc != EVR_BAD_CLIENT ) ret |= rc;
	}
      }
      ret |= callbacksm( sm, eventMask, event, slot, Info, Buffer, &evh->lock );
    }else{
      if( ( eventMask & exclusiveUser->eventMask ) || eventMask == 0 ){
	rc = callbackone( exclusiveUser, event, slot==NO_SLOT?0:slot, Info, Buffer );
	if( rc != EVR_BAD_CLIENT ) ret |= rc;
      }
    }
    break;
  case REQUESTER:
    ret = callbackone( ev->requester, event, slot, Info, Buffer );
    break;
  case TARGET:
    ret = callbackone( ev->target, event, slot, Info, Buffer );
    break;
  default:
    /* return as no error */
    break;
  }

  return( ret );
}

/****************************************************************************
*
* NAME:  slotstatus
*
*   FUNCTION:         get the socket status of the specifyed slot
*                     
*   ARGUMENTS:        - slot   logical slot number
*   RETURN VALUE:     bitmapped status flags
*                         SSTAT_CARD_EXIST
*                         SSTAT_CARD_INSERTION
*                         SSTAT_CARD_REMOVAL
*                         SSTAT_CARD_READY
*                     -1 ( error : invalid slot number )
*   ENVIRONMENT:      event handler
*                         with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=r
*   Calls with Locks: (none)
*
****************************************************************************/
int slotstatus( int slot )
{
  EvhCore *p;
  int status;

  for( p = evhCoreClass.evhCores; p; p = p->next ){
    if( slot == p->slot ){
      p->getstatus( slot, &status, ~0 );
      return( status );
    }
  }
  return( -1 );
}

/****************************************************************************
*
* NAME:  action_for_HWNotify
*
*   FUNCTION:         check the slot status, request initializing slot, and
*                     send appropriate events to clients.
*                     To initialize slot on CARD_INSERTION, call InitSocket()
*                     of cs.c. And to power down on CARD_REMOVAL, call
*                     CSPowerOffSocket() of cs.c.
*                     
*   ARGUMENTS:        - ev   queued event data
*   RETURN VALUE:     (none)
*   ENVIRONMENT:      event handler
*                         with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=r
*   Calls with Locks: slotstatus() with evhCoreClass.configlock=r
*                     callbackall() with evhCoreClass.configlock=r
*                     cs.c#InitSocket() with evhCoreClass.configlock=r
*                     cs.c#CSPowerOffSocket() with evhCoreClass.configlock=r
*
****************************************************************************/
static void action_for_HWNotify( EventQueue* ev )
{
  int stat = slotstatus( ev->slot );
  int evsave = 0;
  EvhCore* evh;
  extern int CSPowerOffSocket( int );

BUGLPR(evhvar, BUGACT, ("action_for_HWNotify!!\n"));
  if( ( evh = selectslot( ev->slot ) ) == 0 ) return;

  if( stat != -1 ){
    /* card removal should be processed first. */
    /* It depends on slotstatus() function. */
    /* If removal then insertion happens, both SSTAT_CARD_REMOVAL */
    /* and SSTAT_CARD_INSERTION are set. But, if insertion then */
    /* removal happens, no bits are set. */
    if( stat & SSTAT_CARD_REMOVAL ){
      callbackall( ev, EV_CARD_REMOVAL, ev->slot, SLOT, EVM_CDChg, 0, 0 );
      CSPowerOffSocket( ev->slot );
      ev->eventSrc = EVH_REMOVAL; /* for usr_enqueue */
/*      evh->ExclusiveUser = 0; */
    }
    if( stat & SSTAT_CARD_INSERTION ){
      int rc;
      if( ( rc = InitSocket( ev->slot ) ) == EVR_SUCCESS ){
	callbackall( ev, EV_CARD_INSERTION, ev->slot, ALL, 0, 0, 0 );
	evsave = EVH_INSERTION; /* for usr_enqueue */
      }
    }

    /* usr_enqueue should be called after INSERTION and REMOVAL callbacks */
    if( ev->eventSrc == EVH_REMOVAL ){
      usr_enqueue( ev, FLUSH_EVENTS_ON_THE_SLOT );
    }
    if( evsave == EVH_INSERTION ){
	ev->eventSrc = evsave;
        usr_enqueue( ev, KEEP_PREVIOUS_EVENTS );
    }

    if( stat & SSTAT_CARD_READY ){
      callbackall( ev, EV_CARD_READY, ev->slot, SLOT, EVM_ReadyChg, 0, 0 );
    }

#if defined( UDEBUG )
    {
      static int calls;
      uprintf( "signal_usr_client:calls = %d\n", ++calls );
    }
#endif /* UDEBUG */
    signal_Usr_Client();
  }


}

/****************************************************************************
*
* NAME:  action_for_Registration
*
*   FUNCTION:         check all slots status and if a card exists in a slot,
*                     send CARD_INSERTION event. And then, send REGISTRATION_
*                     COMPLETE event.
*                     
*   ARGUMENTS:        - ev   queued event data
*   RETURN VALUE:     (none)
*   ENVIRONMENT:      event handler
*                         with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=w
*   Calls with Locks: register_Client() with evhCoreClass.configlock=r
*                     callbackall() with evhCoreClass.configlock=r
*                     evhCore.getstatus() with evhCoreClass.configlock=r
*
****************************************************************************/
static void action_for_Registration( EventQueue* ev )
{
  EvhCore *evh;
  int stat;

  register_Client( ev->requester );

  if( ev->requester->attributes & ( CL_SHR | CL_EXCL ) ){
    for( evh = evhCoreClass.evhCores; evh; evh = evh->next ){
      evh->getstatus( evh->slot, &stat, SSTAT_CARD_EXIST );

      if( stat & SSTAT_CARD_EXIST ){
	callbackall( ev, EV_CARD_INSERTION, evh->slot, REQUESTER, 0, 0, 0 );
#if defined( DEBUG )
printf( "returned from pseudo - insertion\n" );
#endif 

      }
    }

#if defined( DEBUG )
printf( "start regist_comp\n" );
#endif 
    callbackall( ev, EV_REGISTRATION_COMPLETE, NO_SLOT, REQUESTER, 0, 0, 0 );
#if defined( DEBUG )
printf( "returned from regist_comp\n" );
#endif 
  }
}

/****************************************************************************
*
* NAME:  cfehRegisterCient
*
*   FUNCTION:         register a new client
*                     
*   ARGUMENTS:        - callback    callback entry point
*                     - attributes  client attribute flags
*                                ( fixed as CL_TYPE1|CL_EXCL )
*                     - eventMask   event mask for the client
*                                EVM_CDChg
*                                EVM_ReadyChg
*                     - clientData  pointer that will be passwd as an
*                                   argument on callback
*   RETURN VALUE:     clientID ( O.K. )
*                     0 ( xmalloc() error, or invalid arguments )
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock=r
*   Calls with Locks: (none)
*
****************************************************************************/
Client* cfehRegisterClient(
	 int (*callback)(), int attributes,
         unsigned int eventMask, char* clientData
){
  int i;
  Client **p, *q;

  if( classlock_read( &evhCoreClass.configlock ) == ENV_STOP ){
    return( 0 );
  }

  classlock_rdone( &evhCoreClass.configlock );
  q = const_Client( 0, callback, attributes, eventMask, clientData );
  if( q == 0 ){
    return( 0 );
  }

  Enqueue( EVH_REGISTRATION, NO_SLOT, q, 0 );
  return( q );
}

/****************************************************************************
*
* NAME:  cfehDeregisterClient
*
*   FUNCTION:         deregister a client
*                     
*   ARGUMENTS:        - client  clientID to be deregistered
*   RETURN VALUE:     EVR_SUCCESS ( O.K. )
*                     EVR_BAD_CLIENT ( clientID is invalid )
*                     EVR_IN_USE  ( maybe resources are not yet released )
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock=r, client.lock=s
*   Calls with Locks: unregister_Client() with evhCoreClass.configlock=r
*
****************************************************************************/
int cfehDeregisterClient( Client* client )
{
  int ret, i;

  if( ( ret = ValidateClient_lock( client ) ) != EVR_SUCCESS ){
      return( ret );
  }

  if( ( ret = unregister_Client( client ) ) != EVR_SUCCESS ){
    classlock_rdone( &evhCoreClass.configlock );
    return( ret );
  }

  /* If event handler is calling the deregistering client, this thread */
  /* will wait for the completion of the callback, to avoid unconfiguration */
  /* before the completion of the callback. */
  /* As the thread can be callback for getclientinfo, the thread will */
  /* restart after the timeout. ( this is very rare and unusual case. ) */
  if( getpid() != evhCoreClass.kpid ){
    for( i = 0; client->calling && i < 100; i++ ){
      delay( HZ / 10 + 1 ); /* sleeping 0.1 second */
    }
  }

  classlock_rdone( &evhCoreClass.configlock );
  return( ret );
}

/****************************************************************************
*
* NAME:  cfehSetEventMask
*
*   FUNCTION:         set eventMask for the specifyed client
*                     
*   ARGUMENTS:        - p  ClientID
*                     - eventmask  new eventMask
*   RETURN VALUE:     EVR_SUCCESS
*                     EVR_BAD_CLIENT
*   ENVIRONMENT:      client thread
*   LOCKS      :      client.lock=s
*   Calls with Locks: (none)
*
****************************************************************************/
int cfehSetEventMask( Client* p, unsigned int eventmask )
{
  int rc;

  if( ( rc = ValidateClient_lock( p ) ) != EVR_SUCCESS ) return( rc );
  simple_lock( &p->lock );
  if( !p->valid ){
    rc = EVR_BAD_CLIENT;
  }else{
    p->eventMask = eventmask;
  }
  simple_unlock( &p->lock );
  refreshSlot( NO_SLOT );

  classlock_rdone( &evhCoreClass.configlock );
  return( rc );
}

/****************************************************************************
*
* NAME:  cfehGetEventMask
*
*   FUNCTION:         get eventMask for the specifyed client
*                     
*   ARGUMENTS:        - p  ClientID
*                     - eventmask  pointer to store eventMask
*   RETURN VALUE:     EVR_SUCCESS
*                     EVR_BAD_CLIENT
*   ENVIRONMENT:      client thread
*   LOCKS      :      client.lock=s
*   Calls with Locks: (none)
*
****************************************************************************/
int cfehGetEventMask( Client* p, unsigned int *eventmask )
{
  int rc;

  if( ( rc = cfehValidateClient( p ) ) != EVR_SUCCESS ) return( rc );
  simple_lock( &p->lock );
  *eventmask = p->eventMask;
  simple_unlock( &p->lock );

  return( rc );
}

/****************************************************************************
*
* NAME:  ValidateSlotMask_lock
*
*   FUNCTION:         validate slot mask for the specifyed client, slot
*                     and do no release evhCore->lock
*                     
*   ARGUMENTS:        - cl   Client ID for the slot mask
*                     - evh  pointer to evhCore structure
*   RETURN VALUE:     reference to the pointer of SlotMask
*                     ( *p == 0 ) means not found and p indicate refence to
*                     the last link.
*   ENVIRONMENT:      client thread with evhCoreClass.configlock=r
*   LOCKS      :      evhCore->lock=s
*   Calls with Locks: 
*
****************************************************************************/
static SlotMask** ValidateSlotMask_lock( Client* cl, EvhCore* evh )
{
  SlotMask** p;

  simple_lock( &evh->lock );

  for( p = &evh->slotMasks; *p; p = &(*p)->next ){
    if( (*p)->client == cl && (*p)->valid ) break;
  }

  return( p );
}

/****************************************************************************
*
* NAME:  cfehRequestSlotMask
*
*   FUNCTION:         allocate slot mask for the specifyed client, slot
*                     
*   ARGUMENTS:        - cl  Client ID for the slot mask
*                     - slot slot number for the slot mask
*                     - eventmask initial value of slot mask
*   RETURN VALUE:     EVR_SUCCESS
*                     EVR_BAD_SLOT
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock=w
*   Calls with Locks: refreshSlot() with evhCoreClass.configlock=w
*
****************************************************************************/
int cfehRequestSlotMask( Client* cl, int slot, unsigned int eventmask )
{
  int rc;
  EvhCore* evh;
  SlotMask** p, *q;

  if( ( rc = ValidateClient_lock( cl ) ) != EVR_SUCCESS ){
    return( rc );
  }

  if( slot != NO_SLOT && ( evh = selectslot( slot ) ) ){
    p = ValidateSlotMask_lock( cl, evh );

    if( *p == 0 ){
      q = xmalloc( sizeof( SlotMask ), sizeof( int ), pinned_heap );
      q->client = cl;
      q->slot = slot;
      q->eventMask = eventmask;
      q->next = 0;
      q->valid = 1;
      *p = q;
      rc = EVR_SUCCESS;
    }else{
      rc = EVR_BAD_SLOT;
    }
    simple_unlock( &evh->lock );
  }else{
    rc = EVR_BAD_SLOT;
  }

  if( rc == EVR_SUCCESS ) refreshSlot( slot );
  classlock_rdone( &evhCoreClass.configlock );
  return( rc );
}

/****************************************************************************
*
* NAME:  cfehReleaseSlotMask
*
*   FUNCTION:         free memory for the slot mask specifyed.
*                     
*   ARGUMENTS:        - cl  clientID
*                     - slot slot number
*   RETURN VALUE:     EVR_SUCCESS
*                     EVR_BAD_SLOT
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCore.lock=s
*   Calls with Locks: refreshSlot() with evhCoreClass.configlock=r
*                     selectslotReady() with evhCoreClass.configlock=r
*                     ValidateSlotMask_lock() with evhCoreClass.configlock=r
****************************************************************************/
int cfehReleaseSlotMask( Client* cl, int slot )
{
  EvhCore* evh;
  SlotMask** p;
  int rc;

  if( ( rc = ValidateClient_lock( cl ) ) != EVR_SUCCESS ){
    return( rc );
  }

  if( slot != NO_SLOT && ( evh = selectslot( slot ) ) ){
    p = ValidateSlotMask_lock( cl, evh );

    if( *p ){
      (*p)->valid = 0;
      rc = EVR_SUCCESS;
    }else{
      rc = EVR_BAD_SLOT;
    }

    simple_unlock( &evh->lock );
  }else{
    rc = EVR_BAD_SLOT;
  }

  if( rc == EVR_SUCCESS ) refreshSlot( slot );
  classlock_rdone( &evhCoreClass.configlock );
  return( rc );
}

/****************************************************************************
*
* NAME:  cfehSetSlotMask
*
*   FUNCTION:         change the slotmask value in the existing slotmask
*                     
*   ARGUMENTS:        - cl  clinetID for the slotmask
*                     - slot slot number for the slotmask
*                     - eventmask new value to be changed
*   RETURN VALUE:     EVR_SUCCESS
*                     EVR_BAD_SLOT
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock=r, evhCore.lock=s
*   Calls with Locks: refreshSlot() with evhCoreClass.configlock=r
*                     selectslot() with evhCoreClass.configlock=r
*                     ValidateSlotMask_lock() with evhCoreClass.configlock=r
****************************************************************************/
int cfehSetSlotMask( Client* cl, int slot, unsigned int eventmask )
{
  EvhCore* evh;
  SlotMask** p;
  int rc;

  if( ( rc = ValidateClient_lock( cl ) ) != EVR_SUCCESS ){
    return( rc );
  }

  if( slot != NO_SLOT && ( evh = selectslot( slot ) ) ){
    p = ValidateSlotMask_lock( cl, evh );

    if( *p ){
      (*p)->eventMask = eventmask;
      rc = EVR_SUCCESS;
    }else{
      rc = EVR_BAD_SLOT;
    }

    simple_unlock( &evh->lock );
  }else if( slot == NO_SLOT ){
    simple_lock( &cl->lock );
    cl->eventMask = eventmask;
    simple_unlock( &cl->lock );
    rc = EVR_SUCCESS;
  }else{
    rc = EVR_BAD_SLOT;
  }

  if( rc == EVR_SUCCESS ) refreshSlot( slot );
  classlock_rdone( &evhCoreClass.configlock );
  return( rc );
}

/****************************************************************************
*
* NAME:  cfehGetSlotMask
*
*   FUNCTION:         get the slotmask value
*                     
*   ARGUMENTS:        - cl  clinetID for the slotmask
*                     - slot slot number for the slotmask
*                     - eventmask the value of the slotmask
*   RETURN VALUE:     EVR_SUCCESS
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock=r
*   Calls with Locks: ValidateSlotMask_lock() with evhCoreClass.configlock=r
*                     selectslot() with evhCoreClass.configlock=r
*
****************************************************************************/
int cfehGetSlotMask( Client* cl, int slot, unsigned int *eventmask )
{
  EvhCore* evh;
  SlotMask** p;
  int rc;

  if( ( rc = ValidateClient_lock( cl ) ) != EVR_SUCCESS ){
    return( rc );
  }

  if( slot != NO_SLOT && ( evh = selectslot( slot ) ) ){
    p = ValidateSlotMask_lock( cl, evh );

    if( *p ){
      *eventmask = (*p)->eventMask;
      rc = EVR_SUCCESS;
    }else{
      rc = EVR_BAD_SLOT;
    }

    simple_unlock( &evh->lock );
  }else if( slot == NO_SLOT ){
    *eventmask = cl->eventMask;
    rc = EVR_SUCCESS;
  }else{
    rc = EVR_BAD_SLOT;
  }

  classlock_rdone( &evhCoreClass.configlock );
  return( rc );
}

/****************************************************************************
*
* NAME:  cfehGetFirstClient
*
*   FUNCTION:         get the first client entry in event handler
*                     
*   ARGUMENTS:        - slot get client entry who is interested in the slot.
*                            NO_SLOT means dont care
*   RETURN VALUE:     client ID ( O.K. )
*                     0     ( no client exist )
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock = r
*   Calls with Locks: GetValidClient() with evhCoreClass.configlock = r
*
****************************************************************************/
Client* cfehGetFirstClient( int slot )
{
  Client* p;
  int cnd;

  if( classlock_read( &evhCoreClass.configlock ) == ENV_STOP ){
    return( 0 );
  }

  p = GetValidClient( slot, clientClass.clients );
  classlock_rdone( &evhCoreClass.configlock );
  return( p );
}

/****************************************************************************
*
* NAME:  cfehGetNextClient
*
*   FUNCTION:         get the next entry of clients
*                     
*   ARGUMENTS:        - p    previous client ID
*                     - slot get client entry who is interested in the slot.
*                            NO_SLOT means dont care
*   RETURN VALUE:     client ID ( O.K. )
*                     0     ( no client exist )
*                    -1     ( previous client is invalid )
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock = r
*   Calls with Locks: GetValidClient() with evhCoreClass.configlock =r
*
****************************************************************************/
Client* cfehGetNextClient( Client* cl, int slot )
{
  Client* p;
  int cnd;

  if( classlock_read( &evhCoreClass.configlock ) == ENV_STOP ){
    return( (Client*)-1 );
  }

  /* no check of valid flag */
  for( p = clientClass.clients; p != cl && p != 0; p = p->next );

  if( p != 0 && p == cl ){
    p = GetValidClient( slot, cl->next );
  }else{
    p = (Client*)-1;
  }

  classlock_rdone( &evhCoreClass.configlock );
  return( p );
}

/****************************************************************************
*
* NAME:  cfehGetClientInfo
*
*   FUNCTION:         get information of the specifyed client by CLIENT_INFO
*                     event callback.
*                     
*   ARGUMENTS:        - p   client ID to get the information
*                     - buf buffer area to store the information
*   RETURN VALUE:     attribute value for the client ( O.K. )
*                     0 ( invalid client )
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock=r
*   Calls with Locks: 
*
****************************************************************************/
unsigned int cfehGetClientInfo( Client* p, char* buf )
{
  int rc;
  /* EventQueue ev; */

  if( ( rc = ValidateClient_lock( p ) ) != EVR_SUCCESS ){
    return( 0 );
  }

  /* callback is called by client thread */
  callbackone( p, EV_CLIENT_INFO, /*slot*/ 0, /*info*/ 0, buf );
  simple_lock( &p->lock );
  rc = p->valid ? p->attributes : 0;
  simple_unlock( &p->lock );

  classlock_rdone( &evhCoreClass.configlock );
  return( rc );
}

/****************************************************************************
*
* NAME:  action_for_Exclusive
*
*   FUNCTION:         sends callbacks on exclusive request
*                     
*   ARGUMENTS:        - ev   queued event data
*   RETURN VALUE:     (none)
*   ENVIRONMENT:      event handler
*                         with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=r
*   Calls with Locks: 
*
****************************************************************************/
void action_for_Exclusive( EventQueue* ev )
{
  int rc;
  EvhCore* evh;

  evh = selectslot( ev->slot );
  if(evh){
    if( callbackall( ev, EV_EXCLUSIVE_REQUEST, ev->slot, SLOT, 0, 0, 0 ) ){
      callbackall(ev, EV_EXCLUSIVE_COMPLETE, ev->slot, REQUESTER, 0, EVR_IN_USE, 0 );
    simple_lock( &evh->lock );
    evh->RequestingExclusiveUser = 0;
    simple_unlock( &evh->lock );
  }else{
    callbackall( ev, EV_CARD_REMOVAL, ev->slot, SLOT, EVM_CDChg, 0, 0 );
    callbackall( ev, EV_CARD_INSERTION, ev->slot, REQUESTER, 0, 0, 0 );
    simple_lock( &evh->lock );
    evh->ExclusiveUser = evh->RequestingExclusiveUser;
    evh->RequestingExclusiveUser = 0;
    simple_unlock( &evh->lock );
    callbackall(ev, EV_EXCLUSIVE_COMPLETE, ev->slot, REQUESTER, 0, EVR_SUCCESS, 0);
  }
  }
}

/****************************************************************************
*
* NAME:  cfehRequestExclusive
*
*   FUNCTION:         request exclusive usage of a slot
*                     
*   ARGUMENTS:        - p    requesting client ID
*                     - slot slot number to request exclusive use.
*   RETURN VALUE:     EVR_SUCCESS
*                     EVR_BAD_SLOT
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock=r, evhCore.lock=s
*   Calls with Locks: 
*
****************************************************************************/
int cfehRequestExclusive( Client* p, int slot )
{
  EvhCore* evh;
  int rc, stat;
  

  if( slot == NO_SLOT ) return( EVR_BAD_SLOT );

  if( ( rc = ValidateClient_lock( p ) ) != EVR_SUCCESS ){
    return( rc );
  }

  evh = selectslot( slot );

  if( evh == 0 ){
    classlock_rdone( &evhCoreClass.configlock );
    return( EVR_BAD_SLOT );
  }
  evh->getstatus( slot, &stat, SSTAT_CARD_EXIST );
  simple_lock( &evh->lock );
  if( evh->ExclusiveUser || evh->RequestingExclusiveUser ){
    rc = EVR_IN_USE;
  }else{
    if( stat & SSTAT_CARD_EXIST ){
      evh->RequestingExclusiveUser = p;
      rc = EVR_SUCCESS;
    }else{
      rc = EVR_NO_CARD;
    }
  }
  simple_unlock( &evh->lock );
  if( rc == EVR_SUCCESS ) Enqueue( EVH_EXCLUSIVE, slot, p, 0 );
  classlock_rdone( &evhCoreClass.configlock );
  return( rc );
}

/****************************************************************************
*
* NAME:  action_for_Share
*
*   FUNCTION:         sends callbacks on exclusive release
*                     
*   ARGUMENTS:        - ev   queued event data
*   RETURN VALUE:     (none)
*   ENVIRONMENT:      event handler
*                         with evhCoreClass.configlock=r
*   LOCKS:            evhCoreClass.configlock=r
*   Calls with Locks: 
*
****************************************************************************/
static void action_for_Share( EventQueue* ev )
{
  EvhCore* evh;

  if( ev->slot == NO_SLOT ) return;
  evh = selectslot( ev->slot );

  callbackall( ev, EV_CARD_REMOVAL, ev->slot, REQUESTER, 0, 0, 0 );
  simple_lock( &evh->lock );
  evh->ExclusiveUser = 0;
  simple_unlock( &evh->lock );
  callbackall( ev, EV_CARD_INSERTION, ev->slot, ALL, 0, 0, 0 );
}

/****************************************************************************
*
* NAME:  cfehReleaseExclusive
*
*   FUNCTION:         release exclusive usage on a slot
*                     
*   ARGUMENTS:        - p    client ID who holding exclusive usage
*                     - slot slot number
*   RETURN VALUE:     EVR_SUCCESS
*                     EVR_BAD_SLOT
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock=r, evhCore.lock=s
*   Calls with Locks: selectslot() with evhCoreClass.configlock=r
*
****************************************************************************/
int cfehReleaseExclusive( Client* p, int slot )
{
  EvhCore* evh;
  int rc, stat;
  
  if( slot == NO_SLOT ) return( EVR_BAD_SLOT );

  if( ( rc = ValidateClient_lock( p ) ) != EVR_SUCCESS ){
    return( rc );
  }

  evh = selectslot( slot );

  if( evh == 0 ){
    rc = EVR_BAD_SLOT;
  }else if( evh->ExclusiveUser == p ){
    Enqueue( EVH_SHARE, slot, p, 0 );
    rc = EVR_SUCCESS;
  }else{
    rc = EVR_BAD_CLIENT;
  }

  classlock_rdone( &evhCoreClass.configlock );
  return( rc );
}

/****************************************************************************
*
* NAME:  cfehGetExclusiveClient
*
*   FUNCTION:         get the exclusive client ID for the specified slot
*                     
*   ARGUMENTS:        - slot slot number 
*   RETURN VALUE:     client ID ( O.K. )
*                     0 ( invalid slot or no exclusive user )
*   ENVIRONMENT:      client thread
*   LOCKS      :      evhCoreClass.configlock=r
*   Calls with Locks: 
*
****************************************************************************/
Client* cfehGetExclusiveClient( int slot )
{
  EvhCore* evh;
  Client* p;

  if( slot == NO_SLOT ) return( 0 );

  if( classlock_read( &evhCoreClass.configlock ) == ENV_STOP ){
    return( 0 );
  }

  if( evh = selectslot( slot ) ){
    p = evh->ExclusiveUser;
  }else{
    p = 0;
  }

  classlock_rdone( &evhCoreClass.configlock );
  return( p );
}
