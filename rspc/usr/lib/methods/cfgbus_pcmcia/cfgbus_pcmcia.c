static char sccsid[] = "@(#)69  1.8  src/rspc/usr/lib/methods/cfgbus_pcmcia/cfgbus_pcmcia.c, rspccfg, rspc41J, 9516A_all 4/17/95 15:35:46";
/*
 *   COMPONENT_NAME: RSPCCFG
 *
 *   FUNCTIONS: configure method for PCMCIA bus
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <cf.h>		/* Error codes */
#include <sys/types.h> 
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>

#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <sys/iplcb.h>          /* buc table entries */
#include <sys/ioacc.h>          /* BID_ALTREG define */
#include <stdlib.h>
#include "pcmciassdd.h"
#include "cs.h"
#include "bcm.h"

#include "cfgcommon.h"
#include "cfgdebug.h"

#define CS_KERN_EXT "/usr/lib/drivers/pcmciacs"
#define PCMCIASTAT  "/usr/sbin/pcmciastat"

static struct cfg_cs_mdi mdi;

mid_t kmid;
dev_t devno;
int devinst;
int loaded_dvdr=0;
int inited_dvdr=0;
char dvdr[16];
int Dflag;

/*
 * NAME: define_children
 * 
 * FUNCTION: Define children devices
 * 
 * EXECUTION ENVIRONMENT: pcmciastat command is executed to get CIS info
 *                        because symbol svcCardServices cannot be resolved
 *                        when this config method is loaded.
 *           
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int 
define_children(devname,phase,lname)
char *devname;
int phase;
char *lname;
{
  pid_t pid;
  int fd[2];

  if( pipe( fd ) == -1 ){
    return( E_OPEN );
  }

  if( pid = fork() ){
    /* parent */
    int slot;
    int rc;
    int l;
    /* maximum line size output from pcmciastat should be less than this size */
    char buf[520];

    close( fd[1] );   /* close pipe-fd for write */
    dup2( fd[0], 0 ); /* dup pipe-fd for read to stdin */

    /* get a line from pcmciastat command */
    /*   a line format can be             */
    /*     "UT::class/subclass/type\n" or */
    /*     "ID::( manifacturing id in %08x format )\n" or */
    /*     "NF::\n" ( inserted card can not be identifyed. ) or */
    /*     "\n" ( card is not inserted in the slot. ) */
    for( slot = 0; fgets( buf, 518, stdin ); slot++ ){
      l = strlen( buf );
      if( buf[l-1] == '\n' ) buf[l-1] = 0;
      if( buf[0] == 0 ){
	  /* if the line is just a new line char, it means no card exists */
	  DEBUG_1( "socket%d = (None)\n", slot+1 );
      }else{
	  if( strncmp( buf, "UT::", 4 ) == 0 ){
	    DEBUG_2( "socket%d = %s\n", slot+1, buf + 4 );
	    if( rc = define_child( lname, slot+1, buf + 4 ) ) return( rc );
	  }else if( strncmp( buf, "ID::", 4 ) == 0 ){
	    DEBUG_2( "socket%d = (notInstalled:MANF_ID=%s\n", slot+1, buf + 4 );
	    printf( ":%s.pcmcia.%s ", DEVPKG_PREFIX, buf + 4 );
	  }else{
	    DEBUG_1( "socket%d = (notFound)\n", slot+1 );
	    printf( ":%s.pcmcia.nomid ", DEVPKG_PREFIX );
	  }
      }
    }

    DEBUG_1( "number of slots is %d\n", slot );
  }else{
    /* child */
    close( fd[0] );   /* close pipe-fd for read */
    dup2( fd[1], 1 ); /* dup pipe-fd for write to stdout */
    if( execl( PCMCIASTAT, PCMCIASTAT, "-p", lname, "-f", 0 ) == -1 ){
      exit( errno );
    }
  }

  return(0);
}

/*
 * NAME: define_chlid
 * 
 * FUNCTION: Define a child device. If it is a bus extender, call configure
 *           method after the define method.
 * 
 * EXECUTION ENVIRONMENT:
 *  
 * RETURNS: 0 ( success )
 *          error code
 */
int define_child( char* busname, int slot, char* uniquetype )
{
  static struct PdDv cpddv;
  static struct CuDv ccudv;
  char sstring[64];
  char cmdline[64];
  char *out_ptr = NULL, *err_ptr = NULL;
  int rc;

  sprintf( sstring, "uniquetype = %s", uniquetype );
  rc = (int)odm_get_first( PdDv_CLASS, sstring, &cpddv );

  if( rc == -1 ){
    return( E_ODMGET );
  }else if( rc == 0 ){
    /* pddv not found */
    return( E_FINDCHILD );
  }

  sprintf( sstring, "parent=%s AND connwhere=%d AND PdDvLn=%s",
	  busname, slot, uniquetype );
  rc = (int)odm_get_first( CuDv_CLASS, sstring, &ccudv );

  if( rc == -1 ){
    return( E_ODMGET );
  }else if( rc == 0 ){
    sprintf( cmdline, "-c %s -s %s -t %s -p %s -w %d",
	    cpddv.class, cpddv.subclass, cpddv.type, busname, slot );
    rc = odm_run_method( cpddv.Define, cmdline, &out_ptr, &err_ptr );
    DEBUG_2( "run %s %s\n", cpddv.Define, cmdline );
    DEBUG_1( "out(%s)\n", out_ptr );
    DEBUG_1( "err(%s)\n", err_ptr );

    rc = (int)odm_get_first( CuDv_CLASS, sstring, &ccudv );
    if( rc == -1 ){
      return( E_ODMGET );
    }else if( rc == 0 ){
      return( E_NOCuDv );
    }
  }

  if( ccudv.chgstatus == MISSING ){
    ccudv.chgstatus = SAME;
    if( odm_change_obj( CuDv_CLASS, &ccudv ) == -1 )  return( E_ODMUPDATE );
  }

  if( cpddv.bus_ext ){
    rc = invoke_adapt_cfg( ipl_phase, &cpddv, &ccudv );
    if( ipl_phase ) setleds( pddv.led );
    /* Ignore errors : if( rc ) return( rc ); */
  }

  printf( "%s\n", ccudv.name );
  DEBUG_1( "cudv %s defined\n", ccudv.name );
  return( 0 );
}

/*
 * NAME: build_dds
 * 
 * FUNCTION: This function builds the DDSes which are
 *           passed down to card and socket services module.
 *           DDS pointer for socket services is set in *ddsptr.
 *           DDS for card services, mdi, is the static structure in this
 *           source file and it is used in bus_depend().
 * 
 * EXECUTION ENVIRONMENT:
 *
 *        configure method. 
 *
 *  
 * RETURNS: Returns 0 for SUCCESS.
 */

int build_dds(lname, ddsptr, ddslen)
char *lname;
uchar **ddsptr;
int *ddslen;
{
  int rc, num;
  static struct pcmcia_ddi* dds;
  static struct pcmcia_ddi ddsrec;
  struct attr_list* alist = 0;
  char tmpbuf[ 64 ];
  struct PdAt pdat;
  extern struct attr_list* get_attr_list();


  dds = &ddsrec;

  dds->slot = atoi( cudv.connwhere ) - 1;
  DEBUG_2( "build_dds: connwhere = %s, slot = %d\n", cudv.connwhere, dds->slot );
  mdi.devno = devno;
  DEBUG_1( "build_dds: devno = %x\n", devno );

  strncpy( mdi.devname, lname, 16 );
  DEBUG_1( "build_dds: devname = %s\n", lname );

  /* get attributes from parent bus */

  if( ( alist = get_attr_list( pcudv.name, pcudv.PdDvLn_Lvalue, &num, 10 ) ) == 0 ){
    return( E_ODMGET );
  }

  if( ( rc = getatt( alist, "bus_id", &dds->bus_id, 'l', &num ) ) > 0 ){
    return( rc );
  }
  mdi.bus_id = dds->bus_id ;
  DEBUG_1( "build_dds: bus_id = %x\n", dds->bus_id );

  dds->bus_type = 5;			/* this value indicates ISA bus */
  mdi.bus_type = dds->bus_type;
  DEBUG_1( "build_dds: bus_type = %d\n", dds->bus_type );
  
  if( ( alist = get_attr_list( cudv.name, cudv.PdDvLn_Lvalue, &num, 10 ) ) == 0 ){
    return( E_ODMGET );
  }

  if( ( rc = getatt( alist, "bus_intr_lvl", &dds->int_lvl, 'i', &num ) ) > 0 ){
    return( rc );
  }
  mdi.irqlvl = dds->int_lvl;
  DEBUG_1( "build_dds: int_lvl = %d\n", dds->int_lvl );

  if( ( rc = (int)getatt( alist, "intr_priority", &dds->int_prior, 'i', &num ) ) > 0 ){
    return( rc );
  }
  mdi.irqpri = dds->int_prior;
  DEBUG_1( "build_dds: int_pri = %d\n", dds->int_prior );

  /* PCMCIA interrupts are level triggered */
  /* This value should be passed to both card and socket services */
  mdi.int_mode = dds->int_mode = 'I';

  if( ( rc = getatt( alist, "bus_io_addr", &dds->io_addr, 'i', &num ) ) > 0 ){
    return( rc );
  }
  DEBUG_1( "build_dds: io_addr = %x\n", dds->io_addr );

  if( ( rc = getatt( alist, "bus_mem_addr", &mdi.waddr, 'i', &num ) ) > 0 ){
    return( rc );
  }
  DEBUG_1( "build_dds: mem_addr = %x\n", mdi.waddr );

  if( ( rc = getatt( alist, "pm_dev_itime", &mdi.dev_itime, 'i', &num ) ) > 0 ){
    return( rc );
  }
  DEBUG_1( "build_dds: pm_dev_itime = %x\n", mdi.dev_itime );

  if( ( rc = getatt( alist, "pm_dev_stime", &mdi.dev_stime, 'i', &num ) ) > 0 ){
    return( rc );
  }
  DEBUG_1( "build_dds: pm_dev_stime = %x\n", mdi.dev_stime );

  sprintf( tmpbuf, "uniquetype = %s AND attribute = bus_mem_addr", pddv.uniquetype );
  if( ( rc = (int)odm_get_first( PdAt_CLASS, tmpbuf, &pdat ) ) == 0 ){
    /* PdAt not found */
    DEBUG_0( "no PdAt found\n" );
    return( E_NOATTR );
  }else if( rc == -1 ){
    /* ODM failure */
    DEBUG_0( "ODM failure getting PdAt object\n" );
    return( E_ODMGET );
  }

  if( ( rc = convert_att( &mdi.wsize, 'i', &pdat.width, 'n' ) ) != 0 ){
	return( rc );
  }
  /* mdi.wsize = 0x1000; */

  *ddsptr = (char*)dds;
  *ddslen = sizeof( struct pcmcia_ddi );
  return(0);
}


/*
 * NAME: generate_minor 
 * 
 * FUNCTION: This function generates device minor number for the specific 
 *           device. 
 * 
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method 
 *      to generate device minor number for the specific device by calling
 *      lib function genminor.
 *
 * RETURNS: Returns 0 on SUCCESS.
 */

long generate_minor(lname, majorno, minorno)
char *lname;
long majorno;
long *minorno;
{
  long *tmpminor;

  DEBUG_0( "generate_minor: \n" );
  tmpminor = genminor( lname, majorno, -1, 1, 1, 1 );
  if( tmpminor == 0 ) return( E_MINORNO );
  *minorno = *tmpminor;
  return(0);
}


/*
 * NAME: make_special_file 
 * 
 * FUNCTION: This function creates special file(s) for the specific device. 
 * 
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method 
 *      to create special file(s) for the specific device.
 *
 * RETURNS: Returns 0 on SUCCESS.
 */

#define	CRF	(S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

int 
make_special_files(lname, devno)
char *lname;
dev_t devno;
{
  DEBUG_0 ("make_special_files: nop.\n");

  /* we will not have /dev entry */
#if defined( MK_SP_FILE )
  return( mk_sp_file( devno, lname, CRF ) );
#else
  return( 0 );
#endif
}


/*
 * NAME: download_microcode
 * 
 * FUNCTION: This function downloads microcode of specific device 
 *           actually, do nothing
 * 
 * EXECUTION ENVIRONMENT:
 *      This device specific routine is called by the generic config method 
 *      to download microcode for the specific device.
 *
 * RETURNS: Returns 0 on SUCCESS.
 */

int 
download_microcode(lname)
char *lname;
{
	DEBUG_0 ("download microcode:\n")
	return(0);
}

/*
 * NAME: query_vpd
 * 
 * FUNCTION: This function querys the device via the device driver to 
 *           obtain the Vital Product Data(VPD) for the NIO Parallel Port. 
 *           actually, do nothing.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * This function operates as a device dependent subroutine called by the 
 * generic configure method for all devices. It is used to obtain the vpd
 * data from the device and to update the database with the information.
 *
 * RETURNS: Returns 0 on SUCCESS.
 */

int 
query_vpd(newobj, kmid, devno, vpd)
struct CuDv *newobj;
mid_t kmid;
dev_t  devno;
char *vpd;
{
	DEBUG_0 ("query vpd:\n")
	return(0);
}

/*
 * NAME     : device_specific
 *
 * FUNCTION : 
 *
 * RETURNS : 0  ( success )
 */
int device_specific()
{
	DEBUG_0( "device_specific\n");
	return(0);
}


/*
 * NAME     : bus_depend
 *
 * FUNCTION :
 *
 * RETURNS : 0  ( success )
 *
 */

int bus_depend( char* lname, char* devname )
{

  return(0);
}

/*
 *
 *   NAME:      bus_dependent_cfg
 *
 *   FUNCTION:  Gets the bus id from BUC table
 *              Call configure_device().
 *              Then, load and configure PCMCIA card services module.
 *
 *   INPUT:     buc table entry
 *   OUTPUT:    buid value set
 *
 *   RTN CODE:  0 - successful return
 *              E_x - error return
 */

#define MKNOD_MODE S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

int
bus_dependent_cfg(buc,index,bus_devname)
struct buc_info *buc;           /* IPL cntrl block buc info section */
int             index;          /* Index of BUC entry in BUC table */
char            *bus_devname;   /* /dev name for bus */
{
    int             rc;
    ulong           buid;
    struct CuAt     *cuat;
    ulong           cuat_busnumber;
    int     cnt;            /* Return value from getattr() */
    struct cfg_load load;
    struct cfg_kmod kmod;

    DEBUG_0( "bus_dependent_cfg:\n" );

    /*
     * Generate the buid, given the buid and region.
     */
    buid = (ulong)BID_ALTREG(buc->buid_data[0].buid_value, 0);

    /* Set the bus's 'bus_id' attribute */
    rc = set_busid(buid);
    if (rc)
	    return(rc);


    rc = configure_device();
    if( rc ) err_exit( rc );

    load.path = CS_KERN_EXT;
    load.libpath = "";
    load.kmid = 0;

    if( sysconfig( SYS_SINGLELOAD, &load, sizeof( load ) ) == CONF_FAIL ){
      DEBUG_1( "bus_depend: sys_singleload failed( 0x%x ).\n", errno );
      return( E_LOADEXT );
    }else{
      DEBUG_1( "bus_depend: %s has been loaded\n", load.path );
    }

    DEBUG_1( "bus_depend: mdi.devno = %x\n", mdi.devno );
    DEBUG_1( "bus_depend: mdi.bus_id = %x\n", mdi.bus_id );
    DEBUG_1( "bus_depend: mdi.bus_type = %x\n", mdi.bus_type );
    DEBUG_1( "bus_depend: mdi.intr_lvl = %x\n", mdi.irqlvl );
    DEBUG_1( "bus_depend: mdi.intr_pri = %x\n", mdi.irqpri );
    DEBUG_1( "bus_depend: mdi.waddr = %x\n", mdi.waddr );
    DEBUG_1( "bus_depend: mdi.wsize = %x\n", mdi.wsize );
    DEBUG_1( "bus_depend: mdi.dev_itime = %x\n", mdi.dev_itime );
    DEBUG_1( "bus_depend: mdi.dev_stime = %x\n", mdi.dev_stime );
    DEBUG_1( "bus_depend: mdi.devname = %s\n", mdi.devname );
    DEBUG_1( "bus_depend: mdi.int_mode = %x\n", mdi.int_mode );


    kmod.kmid   = load.kmid;
    kmod.cmd    = CFG_INIT;
    kmod.mdiptr = (caddr_t)&mdi;
    kmod.mdilen = sizeof( mdi );

    if( sysconfig( SYS_CFGKMOD, &kmod, sizeof( kmod ) ) == CONF_FAIL ){
      DEBUG_1( "bus_depend: sys_cfgkmod failed( 0x%x ).\n", errno );
      return( E_CFGINIT );
    }else{
      DEBUG_0( "bus_depend: card services has been configureed\n" );
    }

    return(0);
}
