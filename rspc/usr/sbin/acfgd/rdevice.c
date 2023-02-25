static char sccsid[] = "@(#)74	1.3  src/rspc/usr/sbin/acfgd/rdevice.c, pcmciaker, rspc41J, 9516A_all 4/18/95 03:52:47";
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: Functions for RDevice
 *               - const_RDevice, merge_RDevices, sync_RDevices
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
#include <sys/errno.h>
#include <unistd.h>
#include "cfgdebug.h"
#include "acfgd.h"

static RDevice rdevices[32];


/*
 * NAME: const_RDevice
 *
 * FUNCTION: Allocate one RDevice structure.
 *           Actually, a pointer to static structure array is
 *           returned.
 *
 * ARGUMENTS:    devname -    logical device name in CuDv
 *               flag    -    attribute of the device
 *                   INSERTED      Device is detected by a detector
 *                   CONFIGURED    Device is currently configured in system
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: This function can not allocate more than 32 structures.
 *        merge_RDevices and sync_RDevice are expected to be run.
 *        Then, free RDevice records will be available.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	Pointer to allocated RDevice record.
 *
 */
RDevice* const_RDevice( char* devname, int flag )
{
  RDevice* rd;
  int i;

  if( flag == 0 ) return( 0 );

  for( i = 0; i < 32; i++ ){
    if( rdevices[i].flag == FREED ){
      strncpy( rdevices[i].devname, devname, DEVIDSIZE );
      rdevices[i].flag = flag;
      rdevices[i].next = 0;
      return( rdevices + i );
    }
  }

  return( 0 );
}

/*
 * NAME: flush_RDevices
 *
 * FUNCTION: Frees all allocated RDevice records.
 *           Actually, clear the flags of rdevices[] array.
 *
 * ARGUMENTS:    ( void )
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	( void )
 *
 */
static void flush_RDevices( void )
{
  int i;

  for( i = 0; i < 32; i++ ){
    rdevices[i].flag = FREED;
  }
}

/*
 * NAME: merge_RDevices
 *
 * FUNCTION: Merge two RDevice lists into one. The flags for the devices
 *           which exist in both lists will be OR-ed.
 *
 * ARGUMENTS:    p1, p2  - original RDevice lists.
 *                         The contents will be changed on return.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:    It is assumed that each elements in each list ( rd1, rd2 )
 *           is different from the other elements in the list.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	Pointer to the newly generated list.
 *
 */
RDevice* merge_RDevices( RDevice* rd1, RDevice* rd2 )
{
  RDevice* p1, *p2, *p1next;

  if( rd2 == 0 ) return( rd1 );

  for( p1 = rd1; p1; ){
    for( p2 = rd2; p2; p2 = p2->next ){
      if( !strncmp( p1->devname, p2->devname, DEVIDSIZE ) ){
	p2->flag |= p1->flag;
	p1->flag = FREED;
	break;
      }
    }

    p1next = p1->next;

    if( !p2 ){
      p1->next = rd2;
      rd2 = p1;
    }

    p1 = p1next;
  }

  return( rd2 );
}

static struct Class* predev, *cusdev, *preatt, *cusatt;
static int odm_opened;

/*
 * NAME: open_odms
 *
 * FUNCTION: Opens PdDv, CuDv, PdAt and CuAt if they are not opened.
 *
 * ARGUMENTS:    ( void )
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	( void )
 *
 */
static void open_odms( void )
{
  if( !odm_opened ){
    if( (int)( predev = odm_open_class( PdDv_CLASS ) ) == -1 ||
       (int)( cusdev = odm_open_class( CuDv_CLASS ) ) == -1 ||
       (int)( preatt = odm_open_class( PdAt_CLASS ) ) == -1 ||
       (int)( cusatt = odm_open_class( CuAt_CLASS ) ) == -1 ){
      DEBUG_0( "odm_open_class( cudv,pdat,cuat ) failed\n" );
      err_exit( E_ODMINIT );
    }
    odm_opened = 1;
  }
}

/*
 * NAME: close_odms
 *
 * FUNCTION: Closes PdDv, CuDv, PdAt and CuAt if they are opened.
 *
 * ARGUMENTS:    ( void )
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	( void )
 *
 */
static void close_odms( void )
{
  if( odm_opened ){
    /* close CuDv class */
    if( odm_close_class( PdDv_CLASS ) == -1 ||
       odm_close_class( CuDv_CLASS ) == -1 ||
       odm_close_class( PdAt_CLASS ) == -1 ||
       odm_close_class( CuAt_CLASS ) == -1 ){
      DEBUG_0( "error closing CuDv, pdat, cuat Class\n" );
      err_exit( E_ODMCLOSE );
    }
    odm_opened = 0;
  }
}


/*
 * NAME: execute_a_method
 *
 * FUNCTION: Execute a command line for a device.
 *
 * ARGUMENTS:    devname - Device name if the method is a config method
 *                         Otherwise, null.
 *               method  - Command to be executed
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	0 :  Success
 *                             -1 :  Error
 *
 */
static int execute_a_method( char* devname, char* method )
{
  char cmdline[300];
  char buf[40];
  /*char *out_ptr = NULL, *err_ptr = NULL;*/
  int rc, i, k, pkg;

  strncpy( cmdline, method, 300 );
  cmdline[299] = 0;

  if( devname ){
    sprintf( buf, " -l %s", devname );
    strcat( cmdline, buf );
  } /* else cmdline[0] = 0; */
  /*rc = odm_run_method( method, cmdline, &out_ptr, &err_ptr );*/
  rc = system( cmdline );
  DEBUG_2( "run %s %s\n", method, cmdline );
  /*DEBUG_1( "out(%s)\n", out_ptr );*/
  /*DEBUG_1( "err(%s)\n", err_ptr );*/
  DEBUG_1( "rc = %d\n", rc );
  return( rc );
}

/*
 * NAME: execute_RDevice_action
 *
 * FUNCTION: Execute commands specifyed by ODM attribute name
 *           for a device. If there is no specifyed attribute,
 *           execute nothig.
 *           The following strings in the value of ODM attribute are
 *           converted.
 *            %d    -->    ODM logical device name
 *            %n    -->    Number following to prefix in ODM
 *                         logical device name
 *            %%    -->    %
 *          Commands are executed with 'bin' effective user ID.
 *
 * ARGUMENTS:    devname -    logical device name in CuDv
 *               utype   -    uniquetype of the device
 *               prefix  -    prefix name of the device
 *               attr    -    ODM attribute name to be executed.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	0 : Success
 *                             -1 : Error
 *
 */
static int execute_RDevice_action( char* devname, char* utype,
				   char* prefix, char* attr )
{
  char buff[512];
  struct PdAt pdat;
  struct CuAt cuat;
  char* action;
  int rc, i, j, k;

  DEBUG_2( "%s->%s\n", devname, attr );

  sprintf( buff, "attribute = %s AND name = %s", attr, devname );
  rc = (int)odm_get_first( cusatt, buff, &cuat );

  if( rc == -1 ){
    DEBUG_0( "ODM failure getting CuAt object\n" );
    err_exit( E_ODMGET );
  }else if( rc == 0 ){
    sprintf( buff, "attribute = %s AND uniquetype = %s", attr, utype );
    rc = (int)odm_get_first( preatt, buff, &pdat );

    if( rc == -1 ){
      DEBUG_0( "ODM failure getting PdAt object\n" );
      err_exit( E_ODMGET );
    }else if( rc == 0 ){
      return( 0 );
    }

    action = pdat.deflt;
  }else{
    action = cuat.value;
  }

  rc = 0;

  if( *action ){
    for( i = 0, j = 0; action[i] && i < 256 && j < 500; i++ ){
      if( action[i] == '%' ){
	switch( action[i+1] ){
	case 'd': /* %d is converted to device name */
	  i++;
	  for( k = 0; devname[k] && k < 16; k++ ){
	    buff[j++] = devname[k];
	  }
	  break;
	case 'n': /* %n is converted to post fix number */
	  i++;
	  for( k = 0; prefix[k] && k < 16; k++ );
	  for( ; devname[k] && k < 16; k++ ){
	    buff[j++] = devname[k];
	  }
	  break;
	case '%': /* %% is converted to % */
	  i++;
	  /* fall down */
	default:
	  buff[j++] = action[i];
	  break;
	}
      }else{
	buff[j++] = action[i];
      }
    }

    buff[j] = 0;
    initgroups( "bin", 2 );
    DEBUG_1( "initgrp-bin errno = %d\n", errno );
    seteuid( 2 );
    DEBUG_1( "seteuid-bin errno = %d\n", errno );
    rc = execute_a_method( 0, buff );
    seteuid( 0 );
    DEBUG_1( "seteuid-root errno = %d\n", errno );
    initgroups( "root", 0 );
    DEBUG_1( "initgrp-system errno = %d\n", errno );
  }

  return( rc );
}

/*
 * NAME: get_Devinfo
 *
 * FUNCTION:   get PdDv and CuDv for a logical device name if it is at the
 *             desired slot.
 *
 * ARGUMENTS:    devname -    logical device name in CuDv
 *               slot    -    interested slot number.
 *                            ALL_SLOT means all slot will be interested.
 *               pcudv   -    pointer to store PdDv information
 *               ppddv   -    pointer to store CuDv information
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	1 data got correctly
 *                              0 slot is not desired one or error on getting data
 *
 */
static int get_Devinfo( char* devname, int slot,
		       struct CuDv* pcudv, struct PdDv* ppddv )
{
  char criteria[256];
  int rc;

  /* Get CuDv entry for the device */
  if( slot != ALL_SLOT ){
    sprintf( criteria, "name = %s AND connwhere = %d", devname, slot );
  }else{
    sprintf( criteria, "name = %s", devname );
  }
  rc = (int)odm_get_first( cusdev, criteria, pcudv );

  if( rc == -1 ){
    DEBUG_0( "ODM failure getting CuDv object\n" );
    err_exit( E_ODMGET );
  }else if( rc == 0 ) return( 0 );

  /* Get PdDv entry for the device */
  sprintf( criteria, "uniquetype = %s", pcudv->PdDvLn_Lvalue );
  rc = (int)odm_get_first( predev, criteria, ppddv );

  if( rc == -1 ){
    DEBUG_0( "ODM failure getting PdDv object\n" );
    err_exit( E_ODMGET );
  }else if( rc == 0 ) return( 0 );

  return( 1 );
}

/*
 * NAME: ucfg_RDevice_with_scripts
 *
 * FUNCTION: execute pre_rmdev, unconfigure method and post_rmdev for
 *           the specifyed device, if the device is connected at the
 *           specifyed slot or slot value is ALL_SLOT.
 *
 * ARGUMENTS:    devname -    logical device name in CuDv
 *               slot    -    interested slot number.
 *                            ALL_SLOT means all slot will be interested.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	( void )
 *
 */
static void ucfg_RDevice_with_scripts( char* devname, int slot )
{
  struct PdDv pddv;
  struct CuDv cudv;
  int rc;

  open_odms();
  if( !get_Devinfo( devname, slot, &cudv, &pddv ) ) return;
  rc = execute_RDevice_action( devname, cudv.PdDvLn_Lvalue, pddv.prefix, "pre_rmdev" );
  if( !rc ) rc = execute_a_method( devname, pddv.Unconfigure );

  /* need to set cudv.chgstatus to MISSING */
  if( !rc ){
    cudv.chgstatus = MISSING;
    cudv.status = DEFINED;
    rc = ( odm_change_obj( cusdev, &cudv ) == -1 );
  }

  if( !rc ) rc = execute_RDevice_action( devname, cudv.PdDvLn_Lvalue, pddv.prefix, "post_rmdev" );
}

/*
 * NAME: cfg_RDevice_with_scripts
 *
 * FUNCTION: execute pre_mkdev, configure method and post_mkdev for
 *           the specifyed device, if the device is connected at the
 *           specifyed slot or slot value is ALL_SLOT.
 *
 * ARGUMENTS:    devname -    logical device name in CuDv
 *               slot    -    interested slot number.
 *                            ALL_SLOT means all slot will be interested.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	( void )
 *
 */
static void cfg_RDevice_with_scripts( char* devname, int slot )
{
  struct PdDv pddv;
  struct CuDv cudv;
  int rc;

  open_odms();
  if( !get_Devinfo( devname, slot, &cudv, &pddv ) ) return;
  rc = execute_RDevice_action( devname, cudv.PdDvLn_Lvalue, pddv.prefix, "pre_mkdev" );
  if( !rc ) rc = execute_a_method( devname, pddv.Configure );
  if( !rc ) rc = execute_RDevice_action( devname, cudv.PdDvLn_Lvalue, pddv.prefix, "post_mkdev" );
}

/*
 * NAME: sync_RDevices
 *
 * FUNCTION: Configure or unconfigure devices in the specifyed RDevice list.
 *           For each RDevice record at the specifyed slot number,n
 *             If the flag value is ( CONFIGURED | 0 ), device is deleted.
 *             If the flag value is ( 0 | INSERTED ), device is added.
 *             If the flag value is ( CONFIGURED | INSERTED ), nothing happens.
 *
 * ARGUMENTS:    rd   -   RDevice list to be checked.
 *               slot -   interested slot number.
 *                        ALL_SLOT means all slot will be interested.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	( void )
 *
 */
void sync_RDevices( RDevice* rd, int slot )
{
  RDevice* p;

#if defined( CFGDEBUG ) 
  DEBUG_1( "Slot%d - ", slot );
  for( p = rd; p; p = p->next ){
    int i;
    for( i = 0; i < strlen( p->devname ); i++ ){
      DEBUG_1( "%02x", p->devname[i] );
    }
    DEBUG_2( "( %s: %d ) ", p->devname, p->flag );
  }
  DEBUG_0( "\n" );
#endif

  /* process removed devices first */
  for( p = rd; p; p = p->next ){
    if( ( p->flag & ( CONFIGURED | INSERTED ) ) == CONFIGURED ){
      ucfg_RDevice_with_scripts( p->devname, slot );
    }
  }

  /* then, process inserted devices */
  for( p = rd; p; p = p->next ){
    if( ( p->flag & ( CONFIGURED | INSERTED ) ) == INSERTED ){
      cfg_RDevice_with_scripts( p->devname, slot );
    }
  }

  close_odms();
  flush_RDevices();
}
