static char sccsid[] = "@(#)73	1.1  src/rspc/usr/sbin/acfgd/detector.c, pcmciaker, rspc41J, 9509A_all 2/20/95 11:17:37";
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: Functions for Detector
 *               - const_Detector, check_Detector, init_DetectorList
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
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include "cfgdebug.h"
#include "acfgd.h"

static Detector* detectors;

/*
 * NAME: dflt_getConfiguredList
 *
 * FUNCTION: default method to get configured devices in ODM.
 *           This default method assumes that a detector can detect insertion
 *           or removal of children devices, and can not detect insertion
 *           or removal of non-children devices.
 *           Actually, CuDv is queryed by "parent = d->devname and
 *           status = 1".
 *
 * ARGUMENTS:    d       - Detector to be checked.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	RDevice list configured now.
 *
 */
static RDevice* dflt_getConfiguredList( Detector* d )
{
  char criteria[80];
  struct CuDv cudv;
  struct Class* cusdev;
  RDevice* rd, *rdprev;
  int rc;

  DEBUG_0( "start of getconfiguredlist\n" );

  if( (int)( cusdev = odm_open_class( CuDv_CLASS ) ) == -1 ){
    DEBUG_0( "odm_open_class( cudv ) failed\n" );
    err_exit( E_ODMINIT );
  }

  sprintf( criteria, "parent = %s AND status = 1", d->cudv.name );
  rc = (int)odm_get_first( cusdev, criteria, &cudv );

  for( rd = rdprev = 0; rc != 0 && rc != -1; ){
    rd = const_RDevice( cudv.name, CONFIGURED );
    rd->next = rdprev;
    rdprev = rd;
    rc = (int)odm_get_next( cusdev, &cudv );
  }

  if( rc == -1 ){
    DEBUG_0( "ODM failure getting CuDv object\n" );
    err_exit( E_ODMGET );
  }

  /* close CuDv class */
  if( odm_close_class( CuDv_CLASS ) == -1 ){
    DEBUG_0( "error closing CuDv Class\n" );
    err_exit( E_ODMCLOSE );
  }

  DEBUG_0( "end of getconfiguredlist\n" );
  return( rd );
}

/*
 * NAME: dflt_getDetectList
 *
 * FUNCTION: default method to get detected devices by detector.
 *           This default method assumes that a detector can detect insertion
 *           or removal of children devices, and can not detect insertion
 *           or removal of non-children devices.
 *           Detector's configure method's standard output is used to
 *           determine what is detected.
 *
 * ARGUMENTS:    d       - Detector to be checked.
 *
 * EXECUTION ENVIRONMENT: dflt_getConfigredList should be called earlier
 *                        than this dflt_getDetectList, because this
 *                        function may updates CuDv entries.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	RDevice list detected now.
 *
 */
static RDevice* dflt_getDetectList( Detector* d )
{
  RDevice* rd, *rdprev;
  char devname[DEVIDSIZE];
  char cmdline[256];
  char *out_ptr = NULL, *err_ptr = NULL;
  int rc, i, k, pkg;

  sprintf( cmdline, "-l %s", d->cudv.name );
  rc = odm_run_method( d->Configure, cmdline, &out_ptr, &err_ptr );
  DEBUG_2( "run %s %s\n", d->Configure, cmdline );
  DEBUG_1( "out(%s)\n", out_ptr );
  DEBUG_1( "err(%s)\n", err_ptr );

  for( i = pkg = 0, rd = rdprev = 0; out_ptr[i]; i++ ){
    if( out_ptr[i] == ':' ){
      pkg = 1;
      continue;
    }
    for( k = 0; out_ptr[i] != ' ' && out_ptr[i] != '\t' &&
	out_ptr[i] != '\n' && out_ptr[i] != ','; k++, i++ ){
      if( k < DEVIDSIZE ) devname[k] = out_ptr[i];
    }
    if( k > 0 && !pkg ){
      if( k < DEVIDSIZE ) devname[k] = 0;
      devname[DEVIDSIZE-1] = 0;
      rd = const_RDevice( devname, INSERTED );
      rd->next = rdprev;
      rdprev = rd;
    }
    pkg = 0;
  }

  return( rd );
}

/*
 * NAME: const_Detector
 *
 * FUNCTION: Allocate one Detector structure.
 *
 * ARGUMENTS:    cudv -     Detector's CuDv structure pointer
 *               pddv -     Detector's PdDv structure pointer
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	Pointer to allocated Detector record.
 *
 */
Detector* const_Detector( struct CuDv* cudv, struct PdDv* pddv )
{
  Detector* d;

  if( ( d = (Detector*)malloc( sizeof( Detector ) ) ) == 0 ){
    DEBUG_0( "memory allocation error\n" );
    err_exit( E_MALLOC );
  }

  memcpy( &d->cudv, cudv, sizeof( struct CuDv ) );
  d->getConfiguredList = dflt_getConfiguredList;
  d->getDetectList = dflt_getDetectList;
  memcpy( &d->Configure, &pddv->Configure, CFGMTDLEN );
  d->next = 0;

  DEBUG_2( "detector %s (%s)\n", d->cudv.name, d->Configure );
  return( d );
}

/*
 * NAME: get_Detector
 *
 * FUNCTION: Query Detector by logical device name in CuDv.
 *
 * ARGUMENTS:    devname -    logical device name in CuDv
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	Pointer to selected Detector record.
 *
 */
static Detector* get_Detector( char* devname )
{
  Detector* d;

  for( d = detectors; d; d = d->next ){
    if( !strncmp( devname, d->cudv.name, DEVIDSIZE ) ){
      return( d );
    }
  }
  
  return( 0 );
}

/*
 * NAME: check_Detector
 *
 * FUNCTION: Adjust device configuration for the specified detector and
 *           slot.
 *
 * ARGUMENTS:   devname - detector device name to be checked.
 *              slot    - slot number to be checked.
 *              flag    - if ( flag & CALL_DETECT ) != 0, getDetectList
 *                        is called.
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
static void check_ADetector( Detector* d, int slot, int flag )
{
  RDevice* rd1, *rd2;

    if( d ){	
      /* get configured devices first */
      rd1 = (*d->getConfiguredList)( d );
      /* then get currently detected devices */
      if( flag & CALL_DETECT ){
        rd2 = (*d->getDetectList)( d );
        /* merge the two list in one list */
        rd1 = merge_RDevices( rd1, rd2 );
      }
DEBUG_1( "rd1 = %x\n", rd1 );
      sync_RDevices( rd1, slot );
    }
}

/*
 * NAME: check_Detector
 *
 * FUNCTION: Adjust device configuration of all detectors and slots or
 *           the specified detector and slot.
 *
 * ARGUMENTS:   devname - detector device name to be checked.
 *                        If devname is null, all detector devices and
 *                        all slots are checked.
 *              slot    - slot number to be checked.
 *              flag    - if ( flag & CALL_DETECT ) != 0, getDetectList
 *                        is called.
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
void check_Detector( char* devname, int slot, int flag )
{
  Detector* d;

  if( devname != 0 ){
    /* select Detector by ODM device name */
    d = get_Detector( devname );
DEBUG_1( "detector = %x\n", d );
    check_ADetector( d, slot, flag );
  }else{
    /* check all detectors */
    for( d = detectors; d; d = d->next ){
      check_ADetector( d, slot, flag );
    }
  }
}

/*
 * NAME: init_DetectorList
 *
 * FUNCTION: Query detectors in system and initialize Detector records.
 *
 * ARGUMENTS:    d_uniquetype  - Detector unique type names
 *                               ( currently only one unique type )
 *
 * EXECUTION ENVIRONMENT: This function is called on the initialization
 *                        of auto-config daemon. Detector list is not
 *                        updated anytime.
 *
 * NOTES: Currently only PCMCIA adapter can be a detector.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	( void )
 *
 */
void init_DetectorList( char* d_uniquetype )
{
  Detector* d, *dprev;
  struct CuDv cudv;
  struct PdDv pddv;
  struct Class* cusdev, *predev;
  int rc;
  char criteria[80];
    
  DEBUG_0( "init_DetectorList\n" );

  if( (int)( cusdev = odm_open_class( CuDv_CLASS ) ) == -1 ){
    DEBUG_0( "odm_open_class( cudv ) failed\n" );
    err_exit( E_ODMINIT );
  }

  if( (int)( predev = odm_open_class( PdDv_CLASS ) ) == -1 ){
    DEBUG_0( "odm_open_class( pddv ) failed\n" );
    err_exit( E_ODMINIT );
  }

  sprintf( criteria, "PdDvLn = %s AND status = 1", d_uniquetype );
  rc = (int)odm_get_first( cusdev, criteria, &cudv );

  for( d = dprev = 0; rc != 0 && rc != -1; ){
    sprintf( criteria, "uniquetype = %s", cudv.PdDvLn_Lvalue );
    rc = (int)odm_get_first( predev, criteria, &pddv );

    if( rc == -1 ){
      err_exit( E_ODMGET );
    }else if( rc == 0 ){
      /* pddv not found */
      rc = (int)odm_get_next( cusdev, &cudv );
      continue;
    }

    d = const_Detector( &cudv, &pddv );
    d->next = dprev;
    dprev = d;
    rc = (int)odm_get_next( cusdev, &cudv );
  }

  if( rc == -1 ){
    DEBUG_0( "ODM failure getting CuDv object\n" );
    err_exit( E_ODMGET );
  }

  /* close CuDv class */
  if( odm_close_class( CuDv_CLASS ) == -1 ){
    DEBUG_0( "error closing CuDv Class\n" );
    err_exit( E_ODMCLOSE );
  }

  /* close PdDv class */
  if( odm_close_class( PdDv_CLASS ) == -1 ){
    DEBUG_0( "error closing PdDv Class\n" );
    err_exit( E_ODMCLOSE );
  }

  detectors = d;
}

