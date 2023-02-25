static char sccsid[] ="@(#)84        1.1  src/rspc/kernext/pcmcia/cs/evh_intr.c, pcmciaker, rspc411, 9433A411a 8/6/94 15:08:29";
/*
 *
 * COMPONENT_NAME: (PCMCIAKER)
 *
 * FUNCTIONS: PCMCIA Card Services - Event handler pin code
 *             IntrEnqueue
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/sleep.h>
#include <sys/lock_def.h>
#include <sys/intr.h>
#include "EvhP.h"

EvhCoreClass* evhCoreClassp;

/****************************************************************************
*
* NAME:  IntrEnqueue
*
*   FUNCTION:         enqueue the logical slot number where status change
*                     happened. Duplicate calls to one slot is compressed
*                     in one queue data. the queued data will be processed
*                     by event handler process.
*                     
*   ARGUMENTS:        - src  event source type ( this argument's content
*                            is ignored. Assumed to be EVH_HWNOTIFY )
*                     - slot logical slot number where status change happened
*   RETURN VALUE:     1 ( O.K. )
*                     0 ( invalid slot number or event handler is stopping )
*   ENVIRONMENT:      interrupt
*   LOCKS      :      evhCoreClass.iqueuelock=disable_lock
*   Calls with Locks: (none)
*
****************************************************************************/
int IntrEnqueue( int src, int slot )
{
  int ret = 0;

  if( slot >= 0 && slot < MAX_SLOTS && !evhCoreClassp->stopping_flag ){
    int old;
    IEventQueue *p, *q;

    old = disable_lock( INTMAX, &evhCoreClassp->iqueuelock );

    q = evhCoreClassp->ieventQueues + slot;

    if( *q == 0 && q != evhCoreClassp->iwrite ){
      if( evhCoreClassp->iwrite != 0 ) 
	*(evhCoreClassp->iwrite) = (IEventQueue)q;
      evhCoreClassp->iwrite = q;
    }

    if( evhCoreClassp->iread == 0 ){
      evhCoreClassp->iread = evhCoreClassp->iwrite;
      e_wakeup( &evhCoreClassp->evhproc_word );
    }

    unlock_enable( old, &evhCoreClassp->iqueuelock );
    ret = 1;
  }

  return( ret );
}
