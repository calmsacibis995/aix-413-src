static char sccsid[] = "@(#)98	1.2  src/rspc/usr/sbin/pcmciastat/get_ut.c, pcmciaker, rspc411, 9435A411a 8/26/94 16:23:13";
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: init_devlist, reset_devlist, select_devs, decide_dev
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>
#include <cf.h>		/* Error codes */
#include <sys/types.h> 
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include <sys/errno.h>
#include <stdlib.h>

#include "cfgdebug.h"

typedef struct _DevCondition{
  int ok;
  int tplcode;
  int offset;
  int numvalues;
  int lens[4];
  char* hexvalues[4];
  struct _DevCondition* next;
} DevCondition;

/*
 * NAME: alloc_hexvalues
 * 
 * FUNCTION: allocate and set the hexvalues member in DevCondition.
 *           The format of values given is hexastring eg. " 0x3adf334 "
 *           the bytes of hexastring can be odd. Or the format can be
 *           ascii string eg. "abdcdd". As much as 4 values can be specifyed
 *           by separating each value by ','.
 * 
 * EXECUTION ENVIRONMENT:
 *  
 * RETURNS: 0 ( error )
 *          number of values
 */
int alloc_hexvalues( DevCondition* p, char* values )
{
  char* tok;
  char* tokb;
  int i, j, l;

  for( i = 0; i < 4 && values; i++ ){
      tok = values;
      values = strchr( tok, ',' );
      if( values ) *values++ = 0;

      DEBUG_1( "value is '%s'\n", tok );
      
      while( isspace( *tok ) && !*tok ) tok++;
      tokb = tok + strlen( tok ) - 1; 
      while( isspace( *tokb ) && tokb > tok ) tokb--;
      p->lens[i] = 0;

      if( (unsigned)tok <= (unsigned)tokb ){
	tokb[1] = 0;
	l = strlen( tok );
	DEBUG_1( "real token is '%s'\n", tok );

	if( tok[0] == '0' && ( tok[1] == 'x' || tok[1] == 'X' ) && l > 2 ){
	  if( l & 1 ){
	    *(++tok) = '0';
	  }else{
	    tok += 2;
	  }
	  p->lens[i] = strlen( tok );
	  if( ( p->hexvalues[i] = malloc( strlen( tok ) + 1 ) ) == 0 ){
	    return( 0 );
	  }
	  strcpy( p->hexvalues[i], tok );
	}else if( l ){
	  p->lens[i] = l * 2;
	  if( ( p->hexvalues[i] = malloc( l * 2 + 1 ) ) == 0 ){
	    return( 0 );
	  }
	  for( j = 0; j < l; j++ ){
	    sprintf( p->hexvalues[i] + j * 2, "%02x", tok[j] );
	  }
	  p->hexvalues[i][l*2] = 0;
	}
      }

      if( p->lens[i] == 0 ){
	p->lens[i] = 2;
	p->hexvalues[i] = "00";
      }
      DEBUG_3( "hexvalue%d is %s(%d)\n", i, p->hexvalues[i], p->lens[i] );
  }

  return( i );
}

/*
 * NAME: alloc_devconditions
 * 
 * FUNCTION: Allocate and set DevCondition structures. Argument asccnd is
 *           the list of conditions. Each condition is seperated by ':'.
 * 
 * EXECUTION ENVIRONMENT:
 *  
 * RETURNS: 0 ( error )
 *          pointer to DevCondition
 */
DevCondition* alloc_devconditions( char* asccnd )
{
  char* tok;
  int key;
  char* values;
  DevCondition *p = 0, *prev;

  for( ; asccnd; ){
    prev = p;
    tok = asccnd;
    asccnd = strchr( tok, ':' );
    if( asccnd ) *asccnd++ = 0;

    DEBUG_1( "condition = %s\n", tok );

    if( ( values = strchr( tok, ',' ) ) == 0 ){
      /* no value specified */
      continue;
    }

    if( ( p = malloc( sizeof( DevCondition ) ) ) == 0 ){
      DEBUG_0( "malloc error\n" );
      return( 0 );
    }

    *values++ = 0;
    sscanf( tok, "%x", &key );
    DEBUG_1( "key = %x\n", key );
    p->ok = 0;
    p->tplcode = ( key >> 8 ) & 0xff;
    p->offset = ( key & 0xff ) * 2 - 4; /* for hex string offset */
    p->next = prev;

    p->numvalues = alloc_hexvalues( p, values );
    DEBUG_1( "numvalues = %d\n", p->numvalues );
  }

  return( p );
}

static struct Class* preatt;
static int devlist_inited = 0;

typedef struct _DevList{
  int ok;
  char uniquetype[48];
  struct _DevCondition* conditions;
  struct _DevList* next;
} DevList;

static DevList* Root_DevList = 0;

/*
 * NAME: init_devlist
 * 
 * FUNCTION: Initialize DevList structures by reading pcmcia_devid
 *           attributes in PdAt
 * 
 * EXECUTION ENVIRONMENT:
 *  
 * RETURNS: 0 ( success )
 *          error code
 */
init_devlist()
{
  if( !devlist_inited ){
    struct PdAt  pdat;
    DevList *p, *prev;
    int rc;
    
    devlist_inited = 1;

    if( (int)( preatt = odm_open_class( PdAt_CLASS ) ) == -1 ){
      DEBUG_0( "odm_open_class( pdat ) failed\n" );
      return( E_ODMINIT );
    }

    DEBUG_0( "constructing device list\n" );

    rc = (int)odm_get_first( preatt, "attribute = pcmcia_devid", &pdat );

    for( p = prev = 0; rc != 0 && rc != -1; ){
      /* need some condition for each pdat */
      if( pdat.deflt[0] == 0 ) continue;

      if( ( p = malloc( sizeof( DevList ) ) ) == 0 ){
	return( E_MALLOC );
      }

      DEBUG_1( "uniquetype = %s\n", pdat.uniquetype );
      strcpy( p->uniquetype, pdat.uniquetype );
      p->ok = 1;
      p->conditions = alloc_devconditions( pdat.deflt );
      if( p->conditions == 0 ) return( E_MALLOC );

      if( prev == 0 ){
	Root_DevList = p;
      }else{
	prev->next = p;
      }
      prev = p;

      rc = (int)odm_get_next( preatt, &pdat );
    }

    if( p ) p->next = 0;

    if( rc == -1 ){
      DEBUG_0( "ODM getting failure of PdAt object\n" );
      return( E_ODMGET );
    }

    /* close PdAt class */
    if( odm_close_class( PdAt_CLASS ) == -1 ){
      DEBUG_0( "error closing PdAt Class\n" );
      return( E_ODMCLOSE );
    }
  }else{
    reset_devlist();
  }

  return( 0 );
}

/*
 * NAME: reset_devlist
 * 
 * FUNCTION: Reinitialize ok values in DevList and DevConditions structure
 *           all ok values in DevList are set to 1.
 *           all ok values in DevCondition are set to 0
 * 
 * EXECUTION ENVIRONMENT:
 *  
 * RETURNS: none
 */
reset_devlist()
{
  DevList* p;
  DevCondition* q;

  for( p = Root_DevList; p; p = p->next ){
    p->ok = 1;
    for( q = p->conditions; q; q = q->next ){
      q->ok = 0;
    }
  }
}

/*
 * NAME: select_devs
 * 
 * FUNCTION: Scan DevList structure to mark that the conditions match with
 *           arguments' tplcode and hex string.
 * 
 * EXECUTION ENVIRONMENT:
 *  
 * RETURNS: none
 */
select_devs( int tplcode, char* hex )
{
  DevList* p;
  DevCondition* q;
  int i;

  DEBUG_1( "select: tplcode = %02x\n", tplcode );

  for( p = Root_DevList; p; p = p->next ){
    DEBUG_1( "select: ut = %s\n", p->uniquetype );
    if( !p->ok ) continue;
    for( q = p->conditions; q; q = q->next ){
      DEBUG_2( "select: cnd = %x, %x\n", q->tplcode, q->offset );
      if( q->ok ) continue;

      if( tplcode == q->tplcode ){
	for( i = 0; i < q->numvalues; i++ ){
	  DEBUG_2( "%s <-> %s\n", q->hexvalues[i], hex + q->offset );
	  if( !strncmp( hex + q->offset, q->hexvalues[i], q->lens[i] ) ){
	    q->ok = 1;
	    break;
	  }
	}
	if( i == q->numvalues ){
	  p->ok = 0;
	  break;
	}
      }
    }
  }

  DEBUG_0( "end of select_devs\n" );
}

/*
 * NAME: decide_dev
 * 
 * FUNCTION: Decide uniquetype that is marked ok in device list.
 * 
 * EXECUTION ENVIRONMENT:
 *  
 * RETURNS: 
 */
char* decide_dev()
{
  DevList* p;
  DevCondition* q;

  for( p = Root_DevList; p; p = p->next ){
    if( !p->ok ) continue;
    for( q = p->conditions; q; q = q->next ){
      if( !q->ok ) break;
    }

    if( q == 0 ){
      /* matched!! */
      return( p->uniquetype );
    }
  }

  return( 0 );
}
