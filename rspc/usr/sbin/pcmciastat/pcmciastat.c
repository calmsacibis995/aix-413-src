static char sccsid[] = "@(#)75  1.4  src/rspc/usr/sbin/pcmciastat/pcmciastat.c, pcmciaker, rspc412, 9447C 11/14/94 11:24:12";
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: pcmciastat
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

#include <sys/types.h>
#include <sys/sysmacros.h>
#include <stdio.h>
#include <sys/pcmciacs.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <locale.h>
#include <nl_types.h>

#include "cfgdebug.h"
#include "pcmciastat_msg.h"
#include "hlcmds.h" /* from src/bos/usr/sbin/mkdev */

int odm_inited = 0;
struct Class* cusdev;
struct Class* predev;
struct CuDv   cudv;
struct PdDv   pddv;

int f_flag = 0; /* non-pretty, line output , should be 0 later */
int t_flag = 0; /* show all tuple contents */
nl_catd catd;
nl_catd lsdev_catd;

#define ERROUT(n,str) if(!f_flag) fprintf(stderr,catgets(catd,ERR_PCMCIASTAT,(n),(str)));
#define ERROUT1(n,str,a1) if(!f_flag) fprintf(stderr,catgets(catd,ERR_PCMCIASTAT,(n),(str)),(a1));
#define FLD(n,str) catgets(catd,FLD_PCMCIASTAT,(n),(str))

/*
 * NAME: main
 *
 * FUNCTION: processing options in command line, then call sub functions.
 *       valid options are
 *           -p  <busname>      : PCMCIA bus name
 *           -w  <socket number>: Physical socket number for the PCMCIA bus
 *           -f                 : output is non-prettey, line oriented output.
 *                                used with cfgbus_pcmcia
 *           -t                 : output all tuple contents
 *
 * EXECUTION ENVIRONMENT: during configuration phase, this command is called
 *                        by cfgbus_pcmcia. Also, user can type this command,
 *                        and execute from SMIT.
 *
 *
 * EXIT Code:  0  ( SUCCESS )
 *            13  ( E_LNAME, invalid busname or invalid socket number )
 *            ...
 */

void main( int argc, char** argv )
{
  int opt;
  char* busname = 0;
  dev_t devno;
  int rc;
  int slot = -1;     /* slot number */
  int errtuples = 0;

  dev_t get_devno();
  extern int optind;   /* for getopt() */
  extern char* optarg; /* for getopt() */

  setlocale( LC_ALL, "" );

  while( ( opt = getopt( argc, argv, "p:w:ft" ) ) != EOF ){
    switch( opt ){
    case 'p':
      busname = optarg;
      break;
    case 'w':
      slot = atoi( optarg );
      break;
    case 'f':
      f_flag = 1;
      break;
    case 't':
      t_flag = 1;
      break;
    }
  }

  if( f_flag ) t_flag = 0;

  if( !f_flag ){
    catd = catopen( MF_PCMCIASTAT, NL_CAT_LOCALE );
    /* CFG_MSG_CATALOG is defined in <cf.h> */
    lsdev_catd = catopen( CFG_MSG_CATALOG, NL_CAT_LOCALE ); 
  }

  if( busname == 0 ){
    ERROUT( USAGE, "Usage: pcmciastat -p PCMCIA_Bus_name [ -w Socket_number ]\n" );
    err_exit( E_LNAME );
  }

  init_odm();
  devno = get_devno( busname );
  DEBUG_1( "devno = %x\n", devno );
  rc = init_devlist();
  if( rc ){
    if( rc == E_MALLOC ){
      ERROUT( EMALLOC, "pcmciastat: Memory Allocation Error\n" );
    }else{
      ERROUT( EODM, "pcmciastat: ODM Error\n" );
    }
    err_exit( rc );
  }

  if( slot == -1 ){
    for( slot = 0; slot < 16; slot++ ){
      if( ( rc = scan_socket( busname, devno, slot ) ) == -1 ){
	if( slot == 0 ){
	  ERROUT1( ELNAME, "pcmciastat: PCMCIA Bus Name %s is invalid\n", busname );
	  err_exit( E_LNAME );
	}else{
	  break;
	}
      }
      if( rc == -2 ) errtuples = 1;
      reset_devlist();
    }
  }else{
    if( ( rc = scan_socket( busname, devno, slot-1 ) ) == -1 ){
      ERROUT1( ESOCKET, "pcmciastat: Socket number %d is invalid.\n", slot );
      err_exit( E_LNAME );
    }
    if( rc == -2 ) errtuples = 1;
  }

  if( errtuples ){
    ERROUT( ECIS, "Error on reading CIS.\n" );
    err_exit( E_DEVACCESS );
  }else{
    err_exit( 0 );
  }
}

/*
 * NAME: init_odm
 *
 * FUNCTION: initialize ODM and open CuDv and PdDv class.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */
init_odm()
{
  if( odm_initialize() == -1 ){
    DEBUG_0( "odm_initialize() failed\n" );
    ERROUT( EODM, "ODM error.\n" );
    err_exit( E_ODMINIT );
  }

  odm_inited = 1;

  if( (int)( cusdev = odm_open_class( CuDv_CLASS ) ) == -1 ){
    DEBUG_0( "odm_open_class( cudv ) failed\n" );
    ERROUT( EODM, "ODM error.\n" );
    err_exit( E_ODMINIT );
  }

  if( (int)( predev = odm_open_class( PdDv_CLASS ) ) == -1 ){
    DEBUG_0( "odm_open_class( pddv ) failed\n" );
    ERROUT( EODM, "ODM error.\n" );
    err_exit( E_ODMINIT );
  }
}

/*
 * NAME: err_exit
 *
 * FUNCTION: If ODM is initialized, terminate ODM. Then exit with exitcode.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */
err_exit( int exitcode )
{
  if( odm_inited ) odm_terminate();

  if( !f_flag ){
    catclose( catd );
    catclose( lsdev_catd );
  }

  exit( exitcode );
}

/*
 * NAME: get_devno
 *
 * FUNCTION: get devno from busname. In addition, get CuDv and PdDv objects
 *           for the busname.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: devno
 */
dev_t get_devno( char* busname )
{
  char sstring[32];
  int rc;
  int majorno, minorno;
  long *minor_list;
  int how_many;


  DEBUG_1( "getting cudv object for %s\n", busname );
  sprintf( sstring, "name = %s", busname );
  rc = (int)odm_get_first( cusdev, sstring, &cudv );
  if( rc == 0 ){
    DEBUG_1( "no CuDv for %s\n", busname );
    ERROUT1( ELNAME, "pcmciastat: PCMCIA Bus Name %s is invalid.\n", busname );
    err_exit( E_NOCuDv );
  }else if( rc == -1 ){
    DEBUG_0( "ODM failure getting CuDv object\n" );
    ERROUT( EODM, "ODM error.\n" );
    err_exit( E_ODMGET );
  }

  DEBUG_1( "getting pddv object for uniquetype = %s\n", cudv.PdDvLn_Lvalue );
  sprintf( sstring, "uniquetype = %s", cudv.PdDvLn_Lvalue );
  rc = (int)odm_get_first( predev, sstring, &pddv );
  if( rc == 0 ){
    DEBUG_1( "no PdDv for %s\n", busname );
    ERROUT1( ELNAME, "pcmciastat: PCMCIA Bus Name %s is invalid.\n", busname );
    err_exit( E_NOPdDv );
  }else if( rc == -1 ){
    DEBUG_0( "ODM failure getting PdDv object\n" );
    ERROUT( EODM, "ODM error.\n" );
    err_exit( E_ODMGET );
  }

  /* get major number      */
  DEBUG_1("Calling genmajor( %s )\n", pddv.DvDr );
  if ((majorno = genmajor(pddv.DvDr)) == -1) {
    DEBUG_0("error generating major number");
    ERROUT1( ELNAME, "pcmciastat: PCMCIA Bus Name %s is invalid.\n", busname );
    err_exit(E_MAJORNO);
  }
  DEBUG_1("Major number: %d\n",majorno);

  /* get minor number      */
  DEBUG_0("Calling getminor()\n");
  minor_list = getminor(majorno,&how_many,cudv.name);
  if (minor_list == NULL || how_many == 0) {
    DEBUG_0("Could not get minor number\n" );
    ERROUT1( ELNAME, "pcmciastat: PCMCIA Bus Name %s is invalid.\n", busname );
    err_exit( E_MINORNO );
  }else{
    minorno = *minor_list;
  }
  DEBUG_1("minor number: %d\n",minorno);

  return( makedev( majorno, minorno ) );
}

/*
 * NAME: scan_socket
 *
 * FUNCTION: Output lines for one socket specifyed by devno and physical socket
 *           number.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 1 ( SUCCESS and Card is inserted in the socket )
 *          0 ( SUCCESS and Card is not inserted in the socket )
 *         -1 ( invalid devno or socket number )
 *         -2 ( invalid CIS format )
 */
int scan_socket( char* busname, dev_t devno, int w )
{
  int s;
  int rc;

  /* we need to get logical socket number in card service, */
  /* for other card service call require logical socket number */
  s = MapPhySocket( devno, w );

  if( s == -1 ){
    return( -1 );
  }

  /* check the socket status first. */
  if( ( rc = GetStatus( s ) ) == -1 ){
    return( -1 );
  }else if( rc ){
    if( !f_flag ){
      printf( FLD( FSOCKET, "Socket" ) );
      printf( "%d : ", w+1 );
    }
    rc = ScanTuple( s );
    lsdev( busname, w+1 );
    return( rc == -1 ? -2 : 1 );
  }else{
    if( !f_flag ){
      printf( FLD( FSOCKET, "Socket" ) );
      printf( "%d : ", w+1 );
      printf( FLD( FNOCARD, "No Card" ) );
    }
    printf( "\n" );
    lsdev( busname, w+1 );
    return( 0 );
  }
}


/*
 * NAME: print_devname
 *
 * FUNCTION: print out device description in devices.cat
 *           If message is not found in $NLSPATH or /usr/lib/methods,
 *           unique type name will be printed.
 *
 * EXECUTION ENVIRONMENT: f_flag should not be true
 *
 * RETURNS: None
 */
void print_devname( char* ut )
{
  nl_catd catd;
  struct PdDv pddv;
  int rc;
  char sstr[60];
  char* p;

  sprintf( sstr, "uniquetype = %s", ut );

  if( ( rc = (int)odm_get_first( PdDv_CLASS, sstr, &pddv ) ) == -1 ){
    ERROUT( EODM, "ODM error\n" );
    err_exit( E_ODMGET );
  }else if( rc == 0 ){
    printf( ut );
    return;
  }

  DEBUG_3( "catalog = %s, setno = %d, msgno = %d\n",
	&pddv.catalog, (int)pddv.setno, (int)pddv.msgno );

  if( (int)( catd = catopen( pddv.catalog, NL_CAT_LOCALE ) ) == -1 ){
    sprintf( sstr, "/usr/lib/methods/%s", pddv.catalog );
    catd = catopen( sstr, NL_CAT_LOCALE );
  }

  DEBUG_1( "catd = %d\n", catd );

  p = catgets( catd, (int)pddv.setno, (int)pddv.msgno, "" );

  if( *p == 0 ){
    printf( ut );
  }else{
    printf( p );
  }

  catclose( catd );
}


/*
 * NAME: lsdev
 *
 * FUNCTION: Output devices defined in ODM at the specified bus and slot
 *           like lsdev. Name, status and description are output.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None
 */
lsdev( char* busname, int slot )
{
  struct CuDv cudv;
  char sstr[128];
  char* cp;
  int rc;

  if( f_flag || t_flag ) return;

  sprintf( sstr, "parent = %s AND connwhere = %d", busname, slot );

  rc = (int)odm_get_first( CuDv_CLASS, sstr, &cudv ); 

  for( ; rc != 0 && rc != -1; ){
    printf( "  %-10s ", cudv.name );

    switch( cudv.status ){
    case DEFINED:
      cp = catgets( lsdev_catd, CFG_MSG_SET, CFG_MSG_DEFINED, "Defined" );
      break;
    case STOPPED:
      cp = catgets( lsdev_catd, CFG_MSG_SET, CFG_MSG_STOPPED, "Stopped" );
      break;
    case AVAILABLE:
      cp = catgets( lsdev_catd, CFG_MSG_SET, CFG_MSG_AVAILABLE, "Available" );
      break;
    case DIAGNOSE:
      cp = catgets( lsdev_catd, CFG_MSG_SET, CFG_MSG_DIAGNOSE, "Diagnose" );
      break;
    default:
      cp = catgets( lsdev_catd, CFG_MSG_SET, CFG_MSG_UNKNOWN, "Unknown" );
      break;
    }

    printf( "%-10s ", cp );

    print_devname( cudv.PdDvLn_Lvalue );
    printf( "\n" );

    rc = (int)odm_get_next( CuDv_CLASS, &cudv );
  }

  printf( "\n" );
  if( rc == -1 ){
    ERROUT( EODM, "ODM error\n" );
    err_exit( E_ODMGET );
  }
}


/*
 * NAME: MapPhySocket
 *
 * FUNCTION: call card services system call to get logical socket from
 *           devno and physical socket.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: logical socket number ( SUCCESS )
 *          -1 ( invalid devno or physocket )
 */
int MapPhySocket( dev_t devno, int physocket )
{
  static CSMapSktPkt buf;
  int rc;

  buf.PhyAdapter = devno;
  buf.PhySocket = physocket;

  rc = svcCardServices( CSMapPhySocket, 0, 0,  sizeof( CSMapSktPkt ), &buf );

  if( rc != CSR_SUCCESS ){
    return( -1 );
  }

  return( buf.LogSocket );
}

/*
 * NAME: GetStatus
 *
 * FUNCTION: Call card services system call to get the socket status for the
 *           specifyed logical socket.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  1 ( card is inserted )
 *           0 ( card is not inserted )
 *          -1 ( invalid logical socket number )
 */
int GetStatus( int s )
{
  static CSGetStatPkt buf;
  int rc;

  buf.Socket = s;

  rc = svcCardServices( CSGetStatus, 0, 0, sizeof( CSGetStatPkt ), &buf );

  if( rc != CSR_SUCCESS ){
    return( -1 );
  }

  return( buf.CardState & CSStatDetected ? 1 : 0 );
}


static   char manid[20];

/*
 * NAME: print_deviceid
 *
 * FUNCTION: Print out a line for the socket, according to f_flag.
 *           If f_flag is true, id or uniquetype is output. If false,
 *           devices message for the uniquetype is output.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 ( SUCCESS )
 *         -1 ( ERROR )
 */
void print_deviceid()
{
    char* p;

    p = (char*)decide_dev();
    if( f_flag ){
      if( p != 0 ){
	  printf( "UT::%s", p );
      }else if( manid[0] ){
	  printf( "ID::%s", manid );
      }else{
	  printf( "NF::" );
      }
    }else{
      if( p != 0 ){
	  printf( FLD( FEXIST, "Card Exist" ) );
	  printf( " - " );
	  print_devname( p );
      }else{
	printf( FLD( FUNKNOWN, "Unknown Card" ) );
	printf( " - " );
	if( manid[0] ){
	  printf( FLD( FCARDIDIS, "Card ID is %s" ), manid );
	}else{
	  printf( FLD( FNOCARDID, "No Card ID" ) );
	}
      }
    }
}


/*
 * NAME: ScanTuple
 *
 * FUNCTION: Scan all tuples for the card inserted in the specifyed socket.
 *           And output lines according to f_flag and t_flag.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 ( SUCCESS )
 *         -1 ( ERROR )
 */
ScanTuple( int s )
{
  int rc;
  CSGetTplPkt gettpl;
  static union{
    char b[ sizeof( CSGetTplPkt ) + 0x100 ];
    CSGetTplDataPkt pkt;
  } gettpldata;
  static char * tplname(int);


  bzero( &gettpl, sizeof( CSGetTplPkt ) );
  bzero( &gettpldata, sizeof( CSGetTplDataPkt ) + 0x100 );
  manid[0] = 0;
    
  gettpl.Socket = s;
  gettpl.Attributes = 0; 
  gettpl.DesiredTuple = 0xff; 
  gettpl.Reserved = 0;
    
  rc = svcCardServices( CSGetFirstTuple, NULL, NULL, sizeof(CSGetTplPkt), &gettpl );

  /* if getfirst/nexttuple does not return with CSR_SUCCESS, exit the loop */
  for( ; rc == CSR_SUCCESS; ){
    if( t_flag ){
      printf("CISOffset    = 0x%02x\n", gettpl.CISOffset   );
      printf("TupleCode    = 0x%02x : %s\n", gettpl.TupleCode, tplname( gettpl.TupleCode) );
      printf("TupleLink    = 0x%02x\n", gettpl.TupleLink   );
    }

    gettpldata.pkt.Socket = s;
    gettpldata.pkt.Attributes = gettpl.Attributes;
    gettpldata.pkt.DesiredTuple = gettpl.TupleCode;
    gettpldata.pkt.TupleOffset = 0;
    gettpldata.pkt.Flags = gettpl.Flags;
    gettpldata.pkt.LinkOffset = gettpl.LinkOffset;
    gettpldata.pkt.CISOffset = gettpl.CISOffset;
    gettpldata.pkt.TupleDataMax = 0xff;
	
    rc = svcCardServices( CSGetTupleData, NULL, NULL, sizeof(CSGetTplDataPkt)+0x100, &gettpldata );    
    if( rc == CSR_SUCCESS ){
      scan_tplcontent( gettpl.TupleCode, &gettpldata );
    }
    rc = svcCardServices( CSGetNextTuple, NULL, NULL, sizeof(CSGetTplPkt), &gettpl );
  }

  if( !t_flag ) print_deviceid();

  printf( "\n" );
  DEBUG_1( "svcCardServices returns 0x%x\n", rc );
  return( rc == CSR_NO_MORE_ITEMS ? 0 : -1 );
}

/*
 * NAME: scan_tplcontent
 *
 * FUNCTION: Display tuple data in hex code if t_flag is true.
 *           If t_flag is false, call select_devs to scan pcmcia
 *           device table.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: none
 */
scan_tplcontent( int tplcode, CSGetTplDataPkt* gettpldata )
{
  int i, j;
  char dstr[0x11];

  if( !t_flag ){
    char buf[520];

    for( i = 0; i < gettpldata->TupleDataLen; i++ ){
      sprintf( buf + 2*i, "%02x", gettpldata->TupleData[i] );
    }
    select_devs( tplcode, buf );

    if( tplcode == CISTPL_MANFID ){
      strncpy( manid, buf, 8 );
      manid[8]=0;
    }
  }else{
    /* if f_flag is not specifyed, hex output and ascii string is printed */
    /* for each 16 bytes */
    for( i = 0, j = 0; i < gettpldata->TupleDataLen; i++, j++ ){
      /* dstr[] is used to print out ascii string later */
      dstr[j] = *(gettpldata->TupleData + i);
      printf("%02x ", dstr[j]);
      if ( !isprint( dstr[j] ) ) dstr[j] = '.';
      if ( (i % 0x10) == 0xf ){
	dstr[0x10] = 0;
	printf("  |%s|\n", dstr );
	j = -1;
      }
    }
    dstr[j] = 0;
    for(; j < 0x10; j++)
      printf("   ");
    printf("  |%s|\n", dstr );
  }
}

/*
 * NAME: tplname
 *
 * FUNCTION: convert binary tuple code to ascii string that indicate the
 *           tuple code.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 */
static char * tplname(int code)
{
  switch (code){
  case CISTPL_NULL: 	   return("CISTPL_NULL");
  case CISTPL_DEVICE: 	   return("CISTPL_DEVICE");
  case CISTPL_CHECKSUM: 	   return("CISTPL_CHECKSUM");
  case CISTPL_LONGLINK_A:    return("CISTPL_LONGLINK_A");
  case CISTPL_LONGLINK_C:    return("CISTPL_LONGLINK_C");
  case CISTPL_LINKTARGET:    return("CISTPL_LINKTARGET");
  case CISTPL_NO_LINK: 	   return("CISTPL_NO_LINK");
  case CISTPL_VERS_1: 	   return("CISTPL_VERS_1");
  case CISTPL_ALTSTR: 	   return("CISTPL_ALTSTR");
  case CISTPL_DEVICE_A: 	   return("CISTPL_DEVICE_A");
  case CISTPL_JEDEC_C: 	   return("CISTPL_JEDEC_C");
  case CISTPL_JEDEC_A: 	   return("CISTPL_JEDEC_A");
  case CISTPL_CONFIG: 	   return("CISTPL_CONFIG");
  case CISTPL_CFTABLE_ENTRY: return("CISTPL_CFTABLE_ENTRY");
  case CISTPL_DEVICE_OC: 	   return("CISTPL_DEVICE_OC");
  case CISTPL_DEVICE_OA: 	   return("CISTPL_DEVICE_OA");
  case CISTPL_VERS_2: 	   return("CISTPL_VERS_2");
  case CISTPL_FORMAT: 	   return("CISTPL_FORMAT");
  case CISTPL_GEOMETRY: 	   return("CISTPL_GEOMETRY");
  case CISTPL_BYTEORDER: 	   return("CISTPL_BYTEORDER");
  case CISTPL_DATE: 	   return("CISTPL_DATE");
  case CISTPL_BATTERY: 	   return("CISTPL_BATTERY");
  case CISTPL_ORG: 	   return("CISTPL_ORG");
  case CISTPL_END: 	   return("CISTPL_END");
    
  case CISTPL_MANFID: 	   return("CISTPL_MANFID");
  case CISTPL_FUNCID: 	   return("CISTPL_FUNCID");
  case CISTPL_FUNCE: 	   return("CISTPL_FUNCE");
  case CISTPL_SWIL: 	   return("CISTPL_SWIL");
  default:
    return(NULL);
  }
}
