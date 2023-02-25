static char sccsid[] = "@(#)17	1.3  src/rspc/usr/sbin/acfgd/acfgd.c, pcmciaker, rspc41J, 9513A_all 3/28/95 16:13:55";
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: Auto-config daemon main routine, signal handlers
 *                - main, err_exit,
 *                - getintsignal, getcfgchgloop, getpmsignal
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
#include <stdio.h>
#include <sys/signal.h>
#include <setjmp.h>
#include <sys/errno.h>
#include <cf.h>		/* Error codes */
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <fcntl.h>

#if defined( PMLIB_ENABLE )
#include <pmlib.h>
#endif /* PMLIB_ENABLE */

#include "cfgdebug.h"
#include "Evh.h"
#include "acfgd.h"

static char *lockfile = "/etc/acfgd_lock";

/*
 * NAME: acfgd_lock 
 *
 * FUNCTION: lock acfgd_lock file for only one acfgd can run
 *
 * ARGUMENTS: ( void )
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *        1 - lock file successfully locked.
 *        0 - failed to lock acfgd lock file.
 */
static int acfgd_lock( void )
{
  int fd;

  fd = open(lockfile, O_RDWR|O_CREAT, 0600);
  if( lockf(fd, F_TLOCK, 0) != 0 ) {
    return( 0 );
  }
  return( 1 );
}

static int odm_inited = 0;
static int clientID = 0;
static int pmlib_registered = 0;

/*
 * NAME: err_exit
 *
 * FUNCTION: If ODM is initialized, terminate ODM. Then exit with exitcode.
 *
 * ARGUMENTS: exitcode     - Value of exit()
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	(void)
 *
 */
void err_exit( int exitcode )
{
#if ( PMLIB_ENABLE )
  if( pmlib_registered ) pmlib_register_application( PMLIB_UNREGISTER );
#endif /* PMLIB_ENABLE */
  if( clientID ) svccfehDeregisterHandler( clientID );
  if( odm_inited ) odm_terminate();
  unlink(lockfile);
  exit( exitcode );
}

/*
 * NAME: init_odm
 *
 * FUNCTION: Initialize ODM.
 *
 * ARGUMENTS: ( void )
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	(void)
 *
 */
static void init_odm()
{
  if( odm_initialize() == -1 ){
    DEBUG_0( "odm_initialize() failed\n" );
    err_exit( E_ODMINIT );
  }

  odm_inited = 1;
}

/*
 * NAME: getintsignal
 *
 * FUNCTION: Handler for SIGINT signal
 *             - program will be terminated
 *
 * ARGUMENTS:
 *
 * EXECUTION ENVIRONMENT: 
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	(void)
 *
 */
static void getintsignal( int sig )
{
  if( sig == SIGINT ){
    signal( SIGINT, SIG_IGN );
#if defined( PMLIB_ENABLE )
    signal( SIGPM, SIG_IGN );
#endif /* PMLIB_ENABLE */
    err_exit( 0 );
  }
}

static jmp_buf jmpenv;

/*
 * NAME: getcfgchgloop
 *
 * FUNCTION: Main loop to get configuration change
 *           SIGPM is not ignored.
 *
 * ARGUMENTS:
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	(void)
 *
 */
static void getcfgchgloop()
{
  char dev[DEVIDSIZE];
  int slot, eventSrc, rc;

  for(;;){
    /* Ihis system call is blocked until an event comes. If SIGPM raised,
       the system call returns immediately with return value, non 0. */
      rc = svccfehGetEvent( clientID, dev, DEVIDSIZE, &slot, &eventSrc );

      DEBUG_1( "svccfehGetEv returns %d\n", rc );
      if( rc ) continue;

      DEBUG_4( "acfgd: <<%x>> dev = %s, slot = %d, eventSrc = %d\n",
	      clientID, dev, slot, eventSrc );

      /* do not care about CFGDEV event which happens when a PCMCIA client
	 device driver requests or releases configuration */
      if( eventSrc == EVH_CFGDEV ) continue;

      /* increment slot value */
      check_Detector( dev, slot + 1 ,
		     eventSrc == EVH_INSERTION ? CALL_DETECT : 0  );
  }
}

#if defined( PMLIB_ENABLE )
/*
 * NAME: getpmsignal
 *
 * FUNCTION: Handler for power management events
 *            all errors of PM library are unrecoverable!!!
 *
 * ARGUMENTS:
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	(void)
 *
 */
static void getpmsignal( int sig )
{
  static int power_downed = 0;
  int ev;
  pmlib_state_t ok = { PMLIB_SYSTEM_CHANGE_OK };

  if( sig == SIGPM ){
    signal( SIGPM, SIG_IGN );
    signal( SIGINT, SIG_IGN );
DEBUG_0( "Enter to SIGPM handler" );
    if( pmlib_get_event_notice( &ev ) == PMLIB_ERROR ) err_exit( -1 );
DEBUG_1( "- Ev = 0x%x\n", ev );

    switch( ev & PMLIB_EVENTMASK_STATE_CHANGE ){
    case PMLIB_EVENT_NOTICE_TO_SUSPEND:
    case PMLIB_EVENT_NOTICE_TO_HIBERNATION:
      power_downed = 1;
      /* fall down */
    case PMLIB_EVENT_NOTICE_TO_STANDBY:
    case PMLIB_EVENT_NOTICE_TO_FULL_ON:
    case PMLIB_EVENT_NOTICE_TO_ENABLE:
    case PMLIB_EVENT_NOTICE_TO_SHUTDOWN:
      if( pmlib_request_state( PMLIB_CONFIRM, &ok ) == PMLIB_ERROR ){
        err_exit( -1 );
      }
      break;
    case PMLIB_EVENT_FAIL_TO_CHANGE_STATE:
    case PMLIB_EVENT_NOTICE_OF_REJECTION:
      power_downed = 0;
      break;
    case PMLIB_EVENT_RESUME_FROM_SUSPEND:
    case PMLIB_EVENT_RESUME_FROM_HIBERNATION:
      if( power_downed ){
        power_downed = 0;
        signal( SIGPM, getpmsignal );
        seteuid( 0 );
        DEBUG_1( "seteuid-root errno = %d\n", errno );
        initgroups( "root", 0 );
        DEBUG_1( "initgrp-system errno = %d\n", errno );
        check_Detector( 0, ALL_SLOT, CALL_DETECT );
        signal( SIGINT, getintsignal );
        longjmp( jmpenv, sig );
      }
      break;
    case PMLIB_EVENT_START_TO_CHANGE_STATE:
    case PMLIB_EVENT_FORCE_TO_CHANGE_STATE:
    case PMLIB_EVENT_RESUME_FROM_STANDBY:
    default:
      break;
    }

    if( !power_downed ) signal( SIGINT, getintsignal );
    signal( SIGPM, getpmsignal );
  }
}
#endif /* PMLIB_ENABLE */

/*
 * NAME: siginit
 *
 * FUNCTION: Register PM signal handler
 *
 * ARGUMENTS:
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	(void)
 *
 */
static void siginit( void )
{
#if defined( PMLIB_ENABLE )
  if( pmlib_registered ) signal( SIGPM, getpmsignal );
#endif /* PMLIB_ENABLE */
}

/*
 * NAME: main
 *
 * FUNCTION: Main routine. Main routine waits for signals.
 *
 * EXECUTION ENVIRONMENT: To be invoked in /etc/inittab.
 *                        If PM is enabled in the system, PM daemon
 *                        should be started faster than this daemon.
 *
 * ARGUMENTS:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	(void)
 *
 */
void main( int argc, char** argv )
{
  signal( SIGINT, getintsignal );
  signal( SIGHUP, SIG_IGN);
  signal( SIGCLD, SIG_IGN);
  setpgrp();

  if( !acfgd_lock() ) exit( 0 );
  
  init_odm();
  init_DetectorList( "bus/isa_sio/pcmcia" );

  if( fork() == 0 ){
    /* child */
    clientID = svccfehRegisterHandler();
    DEBUG_1( "acfgd: clientID = %x\n", clientID );
#if defined( PMLIB_ENABLE )
    if( pmlib_register_application( PMLIB_REGISTER ) == PMLIB_SUCCESS ){
      pmlib_registered = 1;
    }
#endif /* PMLIB_ENABLED */

    setjmp( jmpenv );
    siginit();

    getcfgchgloop();
  }else{
    /* parent */
    exit( 0 );
  }
}
