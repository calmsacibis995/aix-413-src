static char sccsid[] = "@(#)23	1.9  src/rspc/kernext/pcmcia/tty/sf_pcmcia.c, pcmciafax, rspc41J, 9523A_all 6/6/95 05:16:07";
/*
 * COMPONENT_NAME: pcmciafax
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/systemcfg.h>		/* runtime checks		*/
#include <sys/sysmacros.h>		/* minor and major macros	*/
#include <sys/intr.h>			/* interrupt handler stuff	*/
#include <sys/pin.h>			/* pinning stuff		*/
#include <sys/adspace.h>		/* address space junk		*/
#include <sys/ioacc.h>			/* bus access macros		*/
#include <sys/malloc.h>			/* malloc services		*/
#include <sys/errids.h>			/* get the error id's		*/
#include <sys/errno.h>			/* error codes			*/
#include <sys/uio.h>			/* struct uio			*/
#include "ttydrv.h"			/* instead of the old sys/tty.h	*/
#include <sys/termio.h>			/* struct termios		*/
#include <sys/termiox.h>		/* struct termiox		*/
#include <unistd.h>			/* need _POSIX_VDISABLE def	*/
#include <sys/ttydefaults.h>		/* TTY defaults			*/
#include <sys/ioctl.h>			/* Ioctl declerations		*/
#include <sys/device.h>			/* for config stuff		*/
#include <sys/dbg_codes.h>		/* temp for debugger thing	*/
#include <sys/mstsave.h>		/* temp for debugger thing	*/
#include <sys/low.h>			/* temp for debugger thing	*/
#include <sys/lockl.h>			/* for lockl and unlockl	*/
#include <sys/atomic_op.h>		/* for atomic operations	*/
#include <sys/trchkid.h>		/* trace hook id's		*/
#include <sys/sysinfo.h>		/* sysinfo stuff		*/
#include <sys/signal.h>			/* SIGSAK			*/
#include <sys/pri.h>
#include <sys/sleep.h>			/* EVENT_NULL			*/
#include <sys/except.h>			/* PIO stuff			*/
#include <sys/dump.h>			/* dump structures		*/
#include <sys/str_tty.h>		/* Streams defines		*/
#include <sys/stream.h>			/* Streams defines		*/
#include <sys/stropts.h>		/* Streams defines		*/
#include <sys/strlog.h>			/* Streams defines		*/
#include <sys/strconf.h>		/* Streams defines		*/
#include <string.h>			/* string handling function	*/
#include "common_open.h"		/* Common open discipline fns	*/
#include <sys/rs.h>			/* rs's ioctl's			*/
#include <sys/inline.h>
#include "sty.h"

#include "sf.h"
#include "pcmciaDD.h"
#include <sys/timer.h>
#include <sys/pcmciacs.h>
#include <sys/pcmciacsAix.h>
#include "sf_pcmcia.h"
/* #include <sys/koff/koff.h> */

/* definitions for PCMCIA CardServices */
#define CSIOADDRLINE 16

/* device driver uniq number */
#define FXM_NUMPORTS     8
#define FXM_IOADDRLINES 16
#define FXM_VCC         50

/* PC Card Functions */
#define SERIAL_PORT 2

/* Offsets of Configuration Registers XXX */
#define COR  0
#define CCSR 2

/* Configuration Option Register */
#define COR_SRESET      0x80
#define COR_LevelIRQ    0x40
#define COR_ConfigIndex 0x3F

/* Card Configuration and Status Register */
#define CCSR_Changed  0x80
#define CCSR_SigChg   0x40
#define CCSR_IOis8    0x20
#define CCSR_RingEn   0x10
#define CCSR_Audio    0x08
#define CCSR_PwrDown  0x04
#define CCSR_Intr     0x02

/*** PCMCIA Tuple Data ***/
#define DATA_OFFSET 2

/* CISTPL_DEVICE(0x01), CISTPL_DEVICE_A(0x17): The Device Information Tuples */
#define TPL_DEVICE_SPEED_EXT 0x80

/* CISTPL_CONFIG: Configuration Tuple (0x1A) */
#define TPCC_SZ   ( 2 - DATA_OFFSET )
#define TPCC_RADR ( 4 - DATA_OFFSET )
#define TPCC_RFSZ 0xC0
#define TPCC_RMSZ 0x3C
#define TPCC_RASZ 0x03
#define TPLFID_FUNCTION ( 2 - DATA_OFFSET )

/* CISTPL_CFTABLE_ENTRY: Card Configuration Table Entry Tuple */

/* TPCE_INDEX: Configuration Table Index Byte */
#define TPCE_INDEX_CFG_INDEX 0x3F
#define TPCE_INDEX_DEFAULT   0x40
#define TPCE_INDEX_INTFACE   0x80

/* TPCE_FS: Feature Selection Byte */
#define TPCE_FS_POWER     0x03
#define TPCE_FS_TIMING    0x04
#define TPCE_FS_IOSPACE   0x08
#define TPCE_FS_IRQ       0x10
#define TPCE_FS_MEMSPACE  0x60
#define TPCE_FS_MISC      0x80

/* TPCE_PD: Power Description Structure */
#define TPCE_PD_NOM       0x01
#define TPCE_PD_MIN       0x02
#define TPCE_PD_MAX       0x04
#define TPCE_PD_STATIC    0x08
#define TPCE_PD_AVG       0x10
#define TPCE_PD_PEAK      0x20
#define TPCE_PD_PDWN      0x40
#define TPCE_PD_RFU       0x80

/* TPCE_TD: Configuration Timing Information */
#define TPCE_TD_WAIT      0x03
#define TPCE_TD_READYBUSY 0x1C
#define TPCE_TD_RESERVED  0xE0

/* TPCE_IO: I/O Space Addresses Required For This Configuration */
#define TPCE_IO_IOADDR_LINES      0x1F
#define TPCE_IO_BUS8              0x20
#define TPCE_IO_BUS16             0x40
#define TPCE_IO_RANGE             0x80
#define TPCE_IO_NUM_IOADDR_RANGES 0x0F
#define TPCE_IO_ADDR_SIZE         0x30
#define TPCE_IO_LENGTH_SIZE       0xC0

/* limit for searching tuples */
#define TPL_DATA_LEN_MAX 0xFF
#define TPL_NUMBER_MAX   0xFF

/* typdef for CISTPL_CONFIG */
typedef struct
{    
    unsigned char tpcc_last;
    unsigned long tpcc_radr;
    unsigned int  tpcc_rmsk;
    unsigned int  tpcc_rfsz;
} config_t;

/* typedef for CISTPL_CFTABLE_ENTRY */
#define IO_ADDR_RANGES_MAX 16
typedef struct
{
    unsigned char tpce_index;
    int io_addr_lines;
    int num_io_addr_ranges;
    struct 
    {
	long start_io_addr;
	int length_io_addr;
    } io_addr_range[IO_ADDR_RANGES_MAX];
} cftable_entry_t;

#define itob(X) (~ ( ~ 0 << X ))

/*
 * NAME: CheckCS
 *
 * FUNCTION: calls GetCardServicesInfo CardService
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *            Check following CS version
 *                CS revision : 1.00
 *                CS level    : 2.10
 *
 * RETURNS: return codes define by CardServices
 */

int CheckCS(void)
{
    int            err = CSR_SUCCESS;
    CSGetCSInfoPkt pkt;

    bzero( &pkt, sizeof(pkt) );
    
    err = CardServices(CSGetCardServicesInfo, NULL, NULL,
		       sizeof(pkt), (void *)&pkt);
    if ( err == CSR_SUCCESS ){
        if ( pkt.Revision != CSREVISION || pkt.CSLevel != CSLEVEL ){
            err = CSR_UNSUPPORTED_FUNCTION;
	}
    }
    
    DEBUG_3("CSGetCSInfo rc=%X Rev=%x CSLevel=%x\n",
	    err, pkt.Revision, pkt.CSLevel );
    
    return( err );
}

/*
 * NAME: RegisterClient
 *
 * FUNCTION: calls RegisterClient CardService
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES: sc->pcmcia.clientID is set by CardServices.
 *
 * RETURNS: return codes defined by CardServices
 */

int RegisterClient(struct sfcom *sc )
{
    int         err;
    CSRegCliPkt pkt;

    pkt.Attributes = CSCliAttrIOCDD |  /* IO client device driver            */
                     CSCliAttrEXCL;    /* CARD_INSERTION for exclusively     */
    pkt.EventMask  = CSEvMaskLockChg|  /* Card lock change                   */
                     CSEvMaskEjReq  |  /* Ejection request                   */
                     CSEvMaskInReq  |  /* Insertion request                  */
                     CSEvMaskCDChg  |  /* get events of card detect change   */
                     CSEvMaskPMChg  |  /* Power management change            */
                     CSEvMaskReset;    /* Reset                              */
    pkt.ClientData = (char*)sc;   /* pointer to sfcom                 */
    pkt.Version    = CSREVISION;
    /* expects CS version is 2.1 or upper : BCD value */

    DEBUG_1(">CSRegClient sc=%X\n", sc);

    err = CardServices(CSRegisterClient,
		       &sc->pcmcia.clientID,
		       (void *)Callback,
		       sizeof( CSRegCliPkt ),
		       (void *) &pkt );

    DEBUG_2("<CSRegClient err=%x clientID=%X\n",
	    err, sc->pcmcia.clientID );

    return( err );
}

/*
 * NAME: Callback
 *
 * FUNCTION: callback routine
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *    arguments EventID    : Identifys the event on entry.
 *              Socket     : Identifys the socket affected by the event.
 *              Info       : Other information specific to the event.
 *              Resvd      : Reserved.
 *              Buffer     : Used to pass a buffer.
 *              Misc       : Used for misc information.
 *              client_data: Client Data buffer.
 *
 * RETURNS: return codes defined by CardServices.
 */

int Callback( int EventID, int Socket, int Info, char *Resvd,
	          char* Buffer, int Misc, char *client_data )
{
    int        err     = CSR_SUCCESS;
    struct sfcom  *sc = (struct sfcom *) client_data;
    int        ipri; /* previous interrupt priority */
    int zero = 0;

    DEBUG_4(">Callback EventID=0x%X cb_flag=%d sc->Lskt=%X cb_skt=%X\n",
	    EventID, sc->pcmcia.cb_flag, sc->pcmcia.logical_socket, Socket );

    compare_and_swap(&sc->pcmcia.cb_flag, &zero, 1);
    if (sc->pcmcia.cb_flag != 1){
	DEBUG_1("cb_flag=%d\n", sc->pcmcia.cb_flag);
	return( err );
    }

    /* At first, check whether the event is for me or not. */
    if( EventID != CSE_REGISTRATION_COMPLETE ) {
        if( sc->pcmcia.logical_socket != Socket ) {
            goto end;
        }
    }
    
    switch( EventID )
    {
    case CSE_REGISTRATION_COMPLETE: /* 0x82 */
	DEBUG_1("REG_COMP %X\n", sc->pcmcia.event_word);
        ipri = i_disable( INTMAX );
        sc->pcmcia.reg_comp = 1;
        e_wakeup( &sc->pcmcia.event_word );
        i_enable( ipri );
	DEBUG_1("REG_COMP wakeup %X\n", sc->pcmcia.event_word);
        break;

    case CSE_CARD_INSERTION: /* 0x40 */
        sc->pcmcia.no_card = 0;
        err = CardInsertion( sc );
	if (!sc->sf_open){
	    sf_power(sc, PM_PLANAR_OFF);
	}
	break;

    case CSE_CARD_REMOVAL: /* 0x05 */
        sc->pcmcia.no_card = 1;
        err = CardRemoval( sc );
        break;

    case CSE_RESET_COMPLETE: /* 0x80 */
	DEBUG_1("RESET_COMP %X\n", sc->pcmcia.event_word);
        ipri = i_disable( INTMAX );
	sc->pcmcia.reset_comp = 1;
        e_wakeup( &sc->pcmcia.event_word );
        i_enable( ipri );
	break;
	
    default:
        break;
    }

    /* Set return code for ddconfig */
    if( EventID != CSE_REGISTRATION_COMPLETE )
    {
        sc->pcmcia.return_callback = err;
    }

  end:
    sc->pcmcia.cb_flag = 0;
    DEBUG_1("<Callback rc=%X\n", err);
    return( err );
}

/*
 * NAME: CardInsertion()
 *
 * FUNCTION: Handling of Card Insertion Event
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */

int CardInsertion( struct sfcom *sc )
{
    unsigned char chk_mask;
    int           err;
    unsigned char value, value2;

    DEBUG_1(">CardInsertion Lsocket=%d\n", sc->pcmcia.logical_socket );

    /* hold lock of socket for exclusive operation */
    err = hold_lock( sc );
    if( err != CSR_SUCCESS )
	goto end;

    /* Check whether another client has already configured the card or not. */

    err = CheckConfigured( sc );
    if( err != CSR_SUCCESS )
	goto end;
    
    /* Check whether the insertion event is for me or not. */
    
    /* Check tuple data to find what is in. */
    if( CheckCard( sc->pcmcia.logical_socket ) != 0 ){
	err = CSR_BAD_TYPE;
	goto end;
    }

    /* get the value to be written to configuration option register */
    /* from the io address specified */
    sc->pcmcia.config_index = 0;
    err = ioaddr_to_TPCE_INDEX(sc->pcmcia.logical_socket,
			       sc->sf_conf.port,
			       &sc->pcmcia.config_index );
    
    if ( err != CSR_SUCCESS )
    {
	goto end;
    }
    
    /* Request I/O */
    err = RequestIO( sc );
    if ( err != CSR_SUCCESS )
    {
	goto end;
    }
    
    /* Request IRQ */
    err = RequestIRQ( sc );
    if ( err != CSR_SUCCESS )
    {
	ReleaseIO( sc );
	goto end;
    }
    
    /* Request Configuration */
    err = RequestConfiguration( sc );
    if ( err != CSR_SUCCESS )
    {
	ReleaseIO( sc );
	ReleaseIRQ( sc );
	goto end;
    }

    delay( HZ * 2 );
  
    /* Complete Configuration Successfully */
    sc->pcmcia.configured = 1;

 end:
    /* release lock of socket for exclusive operation */
    release_lock( sc );

    DEBUG_1("<CardInsertion rc=%X\n", err);
    return( err );
}

/*
 * NAME: CardRemoval
 *
 * FUNCTION: Handling of Card Removal Event
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes define by CardServices
 */

int CardRemoval( struct sfcom *sc )
{
    int  err = CSR_SUCCESS;
    
    DEBUG_1(">CardRemoval configured=0x%x\n", sc->pcmcia.configured);

    /* Release all resources */
    if( sc->pcmcia.configured )
    {
      hold_lock( sc );
	ReleaseConfiguration( sc );
	ReleaseIO( sc );
	ReleaseIRQ( sc );
	sc->pcmcia.configured = 0;
      release_lock( sc );
    }
    DEBUG_1("<CardRemoval rc=%x\n", err);
    return( err );
}

/*
 * NAME: CheckCard
 *
 * FUNCTION: check tuple data (VERS_1, MANFID, and FUNCID)
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes define by CardServices
 */

int CheckCard(int Socket)
{
    int err;
    int rc = CSR_GENERAL_FAILURE;
    int i;
    char _ptr[ sizeof(CSGetTplDataPkt) + TPL_DATA_LEN_MAX ];
    CSGetTplPkt *gt_tpl = (CSGetTplPkt *)_ptr;
    CSGetTplDataPkt *tpl_dat = (CSGetTplDataPkt *)_ptr;
    
    DEBUG_1(">CheckCard %d\n", Socket);
    
    bzero( _ptr, sizeof _ptr);
    gt_tpl->Socket = Socket;
    gt_tpl->Attributes = CSGetLinkTpls;
    gt_tpl->DesiredTuple = CISTPL_FUNCID;
    gt_tpl->Reserved = 0;
    
    err = CardServices(CSGetFirstTuple, NULL, NULL, sizeof(CSGetTplPkt),
		       (void *)gt_tpl);
    
    DEBUG_1("CSGetFirstTuple rc=%X\n", err);
    
    if (err != CSR_SUCCESS) /* including CSR_NO_MORE_ITEMS */
    {
	rc = err;
	goto end;
    }

    err = CardServices(CSGetTupleData, NULL, NULL, sizeof _ptr,
		       (void *)tpl_dat);

    DEBUG_2("CSGetTupleData rc=%X TPLFID_FUNCTION=%X\n",
	    err, tpl_dat->TupleData[TPLFID_FUNCTION]);

    if (err != CSR_SUCCESS)
    {
	rc = err;
	goto end;
    }

    if (tpl_dat->TupleData[TPLFID_FUNCTION] == SERIAL_PORT)
    {
	rc = CSR_SUCCESS;
	goto end;
    }

 end:
    DEBUG_1("<CheckCard rc=%X\n", rc);
    
    return rc;
}

/*
 * NAME: CheckConfigured
 *
 * FUNCTION: calls GetConfigurationInfo CardService
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */

int CheckConfigured( struct sfcom *sc )
{
    int             err;
    int             ClientHandle = 0;
    CSGetCfgInfoPkt pkt;
    
    pkt.Socket = sc->pcmcia.logical_socket; /* Logical socket */
    
    err = CardServices(CSGetConfigurationInfo, &ClientHandle, NULL,
		       sizeof( CSGetCfgInfoPkt ), (void *) &pkt );
    DEBUG_4("CSGetConfigInfo rc=%X Lsocket=%d CHdl=%X Attr=%X\n", 
	    err, sc->pcmcia.logical_socket,
	    ClientHandle, pkt.Attributes );
    
    if( err != CSR_SUCCESS )
        return( err );
    
    if(( ClientHandle != NULL ) && ((pkt.Attributes & CSCfgAttrValidCli)>>8 ))
        return( CSR_BAD_ADAPTER );
    else
        return( CSR_SUCCESS );
}

/*
 * NAME: ReadTuple
 *
 * FUNCTION: reads tuple data
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *    arguments TupleID: 
 *              Socket : logical socket.
 *              tpl_dat: structure for Get Tuple Data.
 *                       ( must be set tpl_dat->TupleDataMax )
 *
 * RETURNS: return codes defined by CardServices
 */

int ReadTuple( int TupleID, int Socket, CSGetTplDataPkt *tpl_dat, int DataMax )
{
    int         err;
    CSGetTplPkt *gt_tpl = (CSGetTplPkt *)tpl_dat;;

    gt_tpl->Socket       = Socket;        /* Logical socket */
    gt_tpl->Attributes   = CSGetLinkTpls; /* bit-mapped */
    gt_tpl->DesiredTuple = TupleID;       /* Desired tuple code value*/
    gt_tpl->Reserved     = 0;             /* Reserved (reset to 0)   */

    err = CardServices( CSGetFirstTuple, NULL, NULL,
                        sizeof(CSGetTplPkt), (void *)gt_tpl );

    DEBUG_2("CSGetFirstTuple rc=%x skt=%d \n", err, Socket);

    if ( err != CSR_SUCCESS )
        return( err );
    
    tpl_dat->TupleDataMax = DataMax; /* Maximum size of tuple data area */
    
    err = CardServices( CSGetTupleData, NULL, NULL, 
		       sizeof(CSGetTplDataPkt) + tpl_dat->TupleDataMax,
		       (void *)tpl_dat );
    DEBUG_1("CSGetTplData rc=%X\n", err);
    return( err );
}

/*
 * NAME: RequestIO
 *
 * FUNCTION: calls RequestIO CardServices
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */

int RequestIO( struct sfcom *sc )
{
    CSIOPkt         pkt;
    int             err;

    pkt.Socket      = sc->pcmcia.logical_socket;   /* Logical socket */
    pkt.BasePort1   = (char *)sc->sf_conf.port; /* Base port address1 */
    pkt.NumPorts1   = FXM_NUMPORTS;        /* Number of contiguous ports    */
    pkt.Attributes1 = 0;                    /* bit-mapped                    */
    pkt.BasePort2   = 0;                    /* Base port address for range 2 */
    pkt.NumPorts2   = 0;                    /* Number of contiguous ports    */
    pkt.Attributes2 = 0;                    /* bit-mapped                    */
    pkt.IOAddrLines = FXM_IOADDRLINES;     /* Number of I/O address lines   */

    err = CardServices(CSRequestIO, &sc->pcmcia.clientID, NULL,
		       sizeof(pkt), (void *)&pkt);

    DEBUG_4("CSReqIO: rc=0x%X Socket=%d BasePort1=0x%x ArgLen=%d \n",
	    err, pkt.Socket, pkt.BasePort1, sizeof(pkt) );

    return( err );
}

/*
 * NAME: RequestIRQ
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * FUNCTION: calls RequestIRQ CardServices
 *
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */

int RequestIRQ( struct sfcom *sc )
{
    CSReqIRQPkt     pkt;
    int err;

    pkt.Socket     = sc->pcmcia.logical_socket; /* Logical socket */
    pkt.Attributes = CSIRQExcl;       	      /* bit-mapped     */
                                              /* First PCMCIA IRQ byte */
    pkt.IRQInfo1 = CSIRQInfo2Used;
    pkt.IRQInfo2   = 1 << sc->sf_conf.irq;

    /* Edge mode should be queried for the compatibility with 4.1.1.1 */
    err = CardServices( CSRequestIRQ, &sc->pcmcia.clientID, NULL,
		       sizeof( pkt ), (void *) &pkt );

    if( err == CSR_SUCCESS ){
	sc->sf_slih.flags = INTR_NOT_SHARED;
    }else{
	pkt.IRQInfo1   = CSIRQInfo2Used | CSIRQLevel;

        err = CardServices( CSRequestIRQ, &sc->pcmcia.clientID, NULL,
		       sizeof( pkt ), (void *) &pkt );

	if( err == CSR_SUCCESS ) sc->sf_slih.flags = INTR_LEVEL | INTR_POLARITY;
    }

    DEBUG_3("CSReqIRQ rc=%d socket=%d IRQ=%d \n",
	    err, pkt.Socket, sc->sf_conf.irq );

    return( err );
}

/*
 * NAME: RequestConfiguration
 *
 * FUNCTION: calls RequestConfiguration CardServices
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */

int RequestConfiguration( struct sfcom *sc )
{
    CSReqCfgPkt     pkt;
    int err;
    char _ptr[ sizeof(CSGetTplDataPkt) + TPL_DATA_LEN_MAX ];
    CSGetTplDataPkt *gt_tpl = (CSGetTplDataPkt *)_ptr;
    config_t config;
    
    pkt.Socket     = sc->pcmcia.logical_socket; /* Logical Socket */
    pkt.Attributes = 0;/*CSCfgIRQSteer*//* bit-mapped */
    pkt.Vcc        = FXM_VCC;           /* Vcc setting */
    pkt.Vpp1       = 0;                 /* Vpp1 setting */
    pkt.Vpp2       = 0;                 /* Vpp2 setting */
    pkt.IntType    = CSCfgIFIO;         /* Memory and I/O Interface */
    /* CSCfgIFIO = 0x02 */
    pkt.Status     = 0x00; /* 0x48; */  /* Card status register setting */
    pkt.Pin        = 0x00;              /* Card Pin register setting */
    
    /* get Configuration Base Register Address from CISTPL_CONFIG */
    bzero( _ptr, sizeof _ptr );
    err = ReadTuple(CISTPL_CONFIG, pkt.Socket, gt_tpl, TPL_DATA_LEN_MAX);
    if ( err != CSR_SUCCESS ){
	goto end;
    }
    
    /* parse CISTPL_CONFIG for getting */
    /*   configuration registers base address, and */
    /*   config register presence mask */
    err = parse_config( gt_tpl->TupleData, & config );
    
    DEBUG_1("ConfigBase = %X\n", config.tpcc_radr);
    DEBUG_1("Present    = %X\n", config.tpcc_rmsk);
    
    if ( err != 0 ){
	err = CSR_GENERAL_FAILURE;
	goto end;
    }
    
    pkt.ConfigBase = config.tpcc_radr;
    pkt.Present    = config.tpcc_rmsk
	& ( CSCfgOptValid | CSCfgPinValid | CSCfgStatValid );
    
    /* get the value to be written to configuration option register */
    /* from the io address specified */
    err = ioaddr_to_TPCE_INDEX(pkt.Socket, sc->sf_conf.port,
			       &sc->pcmcia.config_index );
    pkt.ConfigIndex = sc->pcmcia.config_index;
    if ( err != CSR_SUCCESS )
    {
	goto end;
    }
    
    /* set PCMCIA default level IRQ */
    pkt.ConfigIndex |= COR_LevelIRQ ;

    DEBUG_1("ConfigIndex=%X\n", pkt.ConfigIndex);
    
    /* enable audio */
    pkt.Status = CCSR_Audio;
    
    DEBUG_5("CSReqConfig rc=%X ioaddr=%X opt=%X stat=%X pin=%X\n",
	    err, sc->sf_conf.port, pkt.ConfigIndex, pkt.Status,
	    pkt.Pin);
    
    err = CardServices(CSRequestConfiguration, &sc->pcmcia.clientID,
		       NULL, sizeof( pkt ), (void *) &pkt );
 end:

    DEBUG_5("CSReqConfig rc=%X ioaddr=%X opt=%X stat=%X pin=%X\n",
	    err, sc->sf_conf.port, pkt.ConfigIndex, pkt.Status, pkt.Pin);

    return( err );
}

/*
 * NAME: ReleaseIO
 *
 * FUNCTION: calls ReleaseIO CardServices
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int ReleaseIO( struct sfcom *sc )
{
    CSIOPkt         pkt;
    int             err;

    pkt.Socket      = sc->pcmcia.logical_socket; /* Logical Socket    */
    pkt.BasePort1   = (uchar *)sc->sf_conf.port; /* Base port address */
    pkt.NumPorts1   = FXM_NUMPORTS; /* Number of contiguous ports  */
    pkt.Attributes1 = 0x00;          /* Bit-mapped                           */
    pkt.BasePort2   = 0x00;          /* Base port address for range          */
    pkt.NumPorts2   = 0x00;          /* Number of contiguous ports           */
    pkt.Attributes2 = 0x00;            /* Bit-mapped                         */
    pkt.IOAddrLines = FXM_IOADDRLINES; /* Number of IO addr lines decoded */

    err = CardServices( CSReleaseIO, &sc->pcmcia.clientID, NULL,
		       sizeof( pkt ), &pkt );

    DEBUG_1("CSRelIO rc=%X\n", err );

    return( err );
}

/*
 * NAME: ReleaseIRQ
 *
 * FUNCTION: calls ReleaseIRQ CardServices
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */

int ReleaseIRQ( struct sfcom *sc )
{
    CSRelIRQPkt pkt;
    int         err;

    pkt.Socket      = sc->pcmcia.logical_socket; /* Logical Socket */
    pkt.Attributes  = CSIRQExcl;                        /* Bit-mapped */
    pkt.AssignedIRQ = sc->sf_conf.irq; /* IRQ Number Assigned */

    DEBUG_2(">CSRelIRQ skt=%d IRQ=%d\n",
	    sc->pcmcia.logical_socket, sc->sf_conf.irq );
    
    err = CardServices(CSReleaseIRQ, &sc->pcmcia.clientID, NULL,
		       sizeof( pkt ), &pkt );
    
    DEBUG_1("<CSRelIRQ rc=%X\n", err);
    
    return( err );
}

/*
 * NAME: ReleaseConfiguration
 *
 * FUNCTION: calls ReleaseConfiguration CardServices
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */

int ReleaseConfiguration( struct sfcom *sc )
{
    CSRelCfgPkt pkt;
    int         err;

    pkt.Socket = sc->pcmcia.logical_socket;

    err = CardServices(CSReleaseConfiguration, &sc->pcmcia.clientID,
		       NULL, sizeof( pkt ), &pkt );

    DEBUG_3("CSRelConfig rc=%X socket=%d id=%X\n",
	    err, pkt.Socket, sc->pcmcia.clientID);
    return( err );
}

/*
 * NAME: DeregisterClient
 *
 * FUNCTION: calls DeregisterClient CardServices
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */

int DeregisterClient( struct sfcom *sc )
{
    int err;
    
    DEBUG_2(">CSDeregClient sc=%X clientID=%X\n", sc, sc->pcmcia.clientID);
    err = CardServices(CSDeregisterClient, &sc->pcmcia.clientID,
		       NULL, 0, NULL );
    
    DEBUG_1("<CSDeregClient rc=%X\n", err);
    return( err );
}

#ifdef PM_SUPPORT
/*
 * NAME: AccessConfigurationRegister
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int
AccessConfigurationRegister(
struct sfcom *sc,
unsigned char action,
unsigned char offset,
unsigned char *value_ptr)
{
    int err;
    CSAccCfgRegPkt pkt;

    DEBUG_3(">AccCfgReg act=%d offset=%d value=%X\n",
	    action, offset, *value_ptr);

    bzero(&pkt, sizeof pkt);
    pkt.Socket = sc->pcmcia.logical_socket;
    pkt.Action = action;  /* CSCfgRegREAD or CSCfgRegWRITE */
    pkt.Offset = offset;
    pkt.Value = *value_ptr;
    err = CardServices(CSAccessConfigurationRegister, NULL, NULL,
		       sizeof pkt, & pkt );

    if( action == CSCfgRegWRITE ){
        delay( 2 * HZ / 10 );
    }

    DEBUG_1("<AccCfgReg rc=%X\n", err);
    return err;
}

/*
 * NAME: ResetCard
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int ResetCard( struct sfcom *sc )
{
    int i, ipri;
    int err = CSR_SUCCESS;
    CSRstCardPkt pkt;

    DEBUG_1(">ResetCard Lsocket=%d\n", sc->pcmcia.logical_socket);

    bzero(&pkt, sizeof pkt);
    pkt.Socket = sc->pcmcia.logical_socket;
    pkt.Attributes = 0;

    /* RESET_COMPLETE event make reset_comp==1 */
    sc->pcmcia.reset_comp = 0;

    DEBUG_1(">CSResetCard clientID=%X\n", sc->pcmcia.clientID);

    err = CardServices(CSResetCard,
		       &sc->pcmcia.clientID, NULL, sizeof pkt, &pkt);

    DEBUG_1("<CSResetCard rc=%X\n", err);

    if (err != CSR_SUCCESS){
	goto end;
    }

 end:
    DEBUG_1("<ResetCard rc=%X\n", err);
    
    return err;
}

/*
 * NAME: ModifyConfiguration
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int ModifyConfiguration(
struct sfcom *sc,
unsigned int attributes,
unsigned char vcc,
unsigned char vpp1,
unsigned char vpp2)
{
    int err = CSR_SUCCESS;
    CSModCfgPkt pkt;

    DEBUG_1(">CSModifyConfig vcc=%d\n", vcc);

    bzero(&pkt, sizeof pkt);
    pkt.Socket = sc->pcmcia.logical_socket;
    pkt.Attributes = attributes;
    pkt.Vcc = vcc;
    pkt.Vpp1 = vpp1;
    pkt.Vpp2 = vpp2;

    err = CardServices(CSModifyConfiguration,
		       &sc->pcmcia.clientID, NULL, sizeof pkt, &pkt);

    DEBUG_1("<CSModifyConfig rc=%d\n", err);

    return err;
}

/*
 * NAME: GetStatus
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int
GetStatus(
struct sfcom *sc,
unsigned int *cardstate,
unsigned int *socketstate)
{
    int err;
    CSGetStatPkt pkt;

    DEBUG_1(">CSGetStatus Lsocket=%d\n", sc->pcmcia.logical_socket);

    pkt.Socket = sc->pcmcia.logical_socket;
    err = CardServices(CSGetStatus, NULL, NULL, sizeof pkt, &pkt);

    *cardstate = pkt.CardState;
    *socketstate = pkt.SocketState;

    DEBUG_3("<CSGetStatus rc=%X card=%X socket=%X\n",
	    err, *cardstate, *socketstate);

    return err;
}
/*
 * NAME: setup_ring_resume
 *
 * FUNCTION: sets up ringing resume
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int setup_ring_resume( struct sfcom *sc )
{
    int rc = 0;
    unsigned char value;

    DEBUG_0(">setup_ring_resume\n");
    
    rc = AccessConfigurationRegister(sc, CSCfgRegREAD, CCSR, &value);
    if (rc != CSR_SUCCESS){
	goto end;
    }
    value &= ~CCSR_SigChg;
    value |= CCSR_RingEn;

    rc = AccessConfigurationRegister(sc, CSCfgRegWRITE, CCSR, &value);
    if (rc != CSR_SUCCESS){
	goto end;
    }

 end:
    DEBUG_1("<setup_ring_resume rc=%X\n", rc);
    return rc;
}

/*
 * NAME: clear_ring_resume
 *
 * FUNCTION: clears ringing resume setup
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int clear_ring_resume( struct sfcom *sc )
{
    int rc = 0;
    unsigned char value;

    DEBUG_0(">clear_ring_resume\n");
    
    rc = AccessConfigurationRegister(sc, CSCfgRegREAD, CCSR, &value);
    if (rc != CSR_SUCCESS){
	goto end;
    }

    value &= ~CCSR_RingEn;
    value |= CCSR_SigChg;

    rc = AccessConfigurationRegister(sc, CSCfgRegWRITE, CCSR, &value);
    if (rc != CSR_SUCCESS){
	goto end;
    }
 end:
    DEBUG_1(">clear_ring_resume rc=%X\n", rc);
    return rc;
}
#endif /* PM_SUPPORT */
/*
 * NAME: get_TPCC_RADR
 *
 * FUNCTION: get Configuration Registers Base Address
 *           from Configuration Tuple (CISTPL_CONFIG)
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: Configuration Registers Base Address in Reg Space
 */
unsigned long get_TPCC_RADR ( CSGetTplDataPkt *gt_tpl )
{
    int size;
    int i;
    unsigned long addr;
    
    size = (gt_tpl->TupleData[ TPCC_SZ ] & TPCC_RASZ) + 1;
    
#ifdef DEBUG
    DEBUG_1("TD(TPCC_SZ)=%X\n", gt_tpl->TupleData[ TPCC_SZ ]);
    DEBUG_1("size=%d\n", size);
    for (i = 0; i < 8; i++)
    {
	DEBUG_1("%02X ", gt_tpl->TupleData[ i ]);
    }
    DEBUG_0("\n");
#endif
    
    addr = 0;
    for (i = 0; i < size; i++)
    {
	addr += (unsigned long) gt_tpl->TupleData[ TPCC_RADR + i ]
	    << ( 8 * i );
    }
    
    return addr;
}

/*
 * NAME: ioaddr_to_TPCE_INDEX
 *
 * FUNCTION: searches Configuration Entry Number for the IO address
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int ioaddr_to_TPCE_INDEX(int socket, long port, unsigned char *index_ptr)
{
    int err;
    int rc;
    int i, j;
    char _ptr[ sizeof(CSGetTplDataPkt) + TPL_DATA_LEN_MAX ];
    CSGetTplPkt *gt_tpl = (CSGetTplPkt *)_ptr;
    CSGetTplDataPkt *tpl_dat = (CSGetTplDataPkt *)_ptr;
    cftable_entry_t cftable_entry;
    
    DEBUG_1(">ioadd_to_TPCE_INDEX %X\n", port);

    if( *index_ptr != 0x0 ){
      DEBUG_1( "<ioadd_to_TPCE_INDEX return without CS(0x%x)\n", (int)*index_ptr );
      return( CSR_SUCCESS );
    }
 
    bzero(_ptr, sizeof _ptr);
    gt_tpl->Socket = socket;
    gt_tpl->Attributes = CSGetLinkTpls;
    gt_tpl->DesiredTuple = CISTPL_CFTABLE_ENTRY;
    gt_tpl->Reserved = 0;
    
    err = CardServices(CSGetFirstTuple, NULL, NULL, sizeof _ptr,
		       (void *)gt_tpl);
    
    DEBUG_1("CSGetFirstTuple rc=%X\n", err);
    
    if (err != CSR_SUCCESS)
    {
	rc = err;
	goto end;
    }
    for(i = 0; i < TPL_NUMBER_MAX; i++)
    {
	err = CardServices(CSGetTupleData, NULL, NULL, sizeof _ptr,
			   (void *)tpl_dat);
#ifdef DEBUG
	DEBUG_1("CSGetTupleData rc=%X: ", err);
	for (j = 0; j < 16; j++)
	{
	    DEBUG_1("%02X ", tpl_dat->TupleData[ j ]);
	}
	DEBUG_0("\n");
#endif
	
	err = parse_cftable_entry(tpl_dat->TupleData, &cftable_entry);
	if ( err != 0 )
	{
	    rc = CSR_GENERAL_FAILURE;
	    goto end;
	}
	
	for (j = 0; j < cftable_entry.num_io_addr_ranges; j++)
	{

	    DEBUG_5("IOAddr(%d,%d)=%X IOLines=%d ? port=%X\n",
		    i, j, cftable_entry.io_addr_range[j].start_io_addr,
		    cftable_entry.io_addr_lines, port);

	    
	    if ((cftable_entry.io_addr_range[j].start_io_addr
		 /* & itob(cftable_entry.io_addr_lines) */)
		== (port /* & itob(cftable_entry.io_addr_lines) */))
	    {
		*index_ptr = cftable_entry.tpce_index & TPCE_INDEX_CFG_INDEX;
		rc = CSR_SUCCESS;
		goto end;
	    }
	}
	err = CardServices(CSGetNextTuple, NULL, NULL, sizeof _ptr,
			   (void *)tpl_dat);
	
	DEBUG_1("CSGetNextTuple rc=%X\n", err);
	
	if (err != CSR_SUCCESS)
	{
	    rc = err;
	    goto end;
	}
    }
 end:
    return rc;
}

/*
 * NAME: parse_config
 *
 * FUNCTION: parses the CISTPL_CONFIG tuple
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: 0
 */
int parse_config(char *data, config_t *config_ptr)
{
    int i;
    int offset;
    int tpcc_rfsz;
    int tpcc_rmsz;
    int tpcc_rasz;

    DEBUG_0(">parse_config\n");

#ifdef DEBUG
    for (i = 0; i < 16; i++){
	DEBUG_1("%02X ", data[i]);
    }
    DEBUG_0("\n");
#endif

    offset = 0;

    /* TPCC_SZ: Size of Fields Byte */
    tpcc_rasz = extract_bits(data[offset], TPCC_RASZ) + 1;
    tpcc_rmsz = extract_bits(data[offset], TPCC_RMSZ) + 1 ;
    tpcc_rfsz = extract_bits(data[offset], TPCC_RFSZ);
    offset++;

    /* TPCC_LAST: Card Configuration Table Last Entry Index */
    config_ptr->tpcc_last = data[offset];
    offset++;

    /* TPCC_RADA: Configuration Registers Base Address in REG Space */
    config_ptr->tpcc_radr = 0;
    for (i = 0; i < tpcc_rasz; i++){
	config_ptr->tpcc_radr += (unsigned long) data[offset] << ( 8 * i );
	offset++;
    }

    /* TPCC_RMSK: Configuration Register Presence Mask Field for Interface */
    /* Tuple */
    config_ptr->tpcc_rmsk = 0;
    for (i = 0; i < tpcc_rmsz; i++){
	config_ptr->tpcc_rmsk += (unsigned int) data[offset] << ( 8 * i );
	offset++;
    }

    /* TPCC_RSVD: Reserved area */
    config_ptr->tpcc_rfsz = 0;

    DEBUG_0("<parse_config\n");
    return 0;
}

/*
 * NAME: parse_cftable_entry
 *
 * FUNCTION: parses the CISTPL_CFTABLE_ENTRY tuple
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: 0
 */
int parse_cftable_entry(char *data, cftable_entry_t *cftable_ptr)
{
    int rc = 0;
    int i, j;
    int offset;
    unsigned char feature_select;

    DEBUG_0(">parse_cftable_entry\n");

    cftable_ptr->num_io_addr_ranges = 0;
 
    /* TPCE_INDEX */
    offset = 0;
    cftable_ptr->tpce_index = data[offset];
   
    if (data[offset] & TPCE_INDEX_INTFACE)
    {
	/* TPCE_IF */
	offset++;
    }
    
    /* TPCE_FS */
    offset++;
    
    /* Feature Selection Byte */
    
    feature_select = data[offset];
    
    offset++;	
    
    /* TPCE_PD */
    if (feature_select & TPCE_FS_POWER)
    {
	for (i = 0; i < (feature_select & TPCE_FS_POWER); i++)
	{
	    unsigned char power_select = data[offset];
	    
	    while ( power_select != 0 )
	    {
		if ( power_select & 0x01 )
		{
		    offset++;

		    while (data[offset] & TPCE_PD_RFU)
		    {
			offset++;
		    }
		}
		power_select >>= 1;
	    }
	}
	offset++;
    }

    /* TPCE_TD */
    if (feature_select & TPCE_FS_TIMING)
    {
	unsigned char tpce_td;
	tpce_td = data[offset];
	
	offset++;
	
	if ((tpce_td & TPCE_TD_WAIT) != TPCE_TD_WAIT)
	{

	    while (data[offset] & TPL_DEVICE_SPEED_EXT)
	    {
		offset++;
	    }
	    offset++;
	}
	if ((tpce_td & TPCE_TD_READYBUSY) != TPCE_TD_READYBUSY)
	{

	    while (data[offset] & TPL_DEVICE_SPEED_EXT)
	    {
		offset++;
	    }
	    offset++;
	}
	if ((tpce_td & TPCE_TD_RESERVED) != TPCE_TD_RESERVED)
	{
	    
	    while (data[offset] & TPL_DEVICE_SPEED_EXT)
	    {
		offset++;
	    }
	    offset++;
	}
    }
    
    if (feature_select & TPCE_FS_IOSPACE)
    {
	int length_size;
	int address_size;
	int num_io_address_ranges;
	
	cftable_ptr->io_addr_lines = data[offset] & TPCE_IO_IOADDR_LINES;

        offset++;

	cftable_ptr->num_io_addr_ranges
	    = extract_bits(data[offset], TPCE_IO_NUM_IOADDR_RANGES) + 1;
	
	switch (extract_bits(data[offset], TPCE_IO_LENGTH_SIZE))
	{
	case 0:
	    length_size = 0;
	    break;
	case 1:
	    length_size = 1;
	    break;
	case 2:
	    length_size = 2;
	    break;
	case 3:
	    length_size = 4;
	    break;
	}

	switch(extract_bits(data[offset], TPCE_IO_ADDR_SIZE))
	{
	case 0:
	    address_size = 0;
	    break;
	case 1:
	    address_size = 1;
	    break;
	case 2:
	    address_size = 2;
	    break;
	case 3:
	    address_size = 4;
	    break;
	}
	
	offset++;
	for (i = 0; i < cftable_ptr->num_io_addr_ranges; i++)
	{
	    cftable_ptr->io_addr_range[i].start_io_addr = 0;
	    for (j = 0; j < address_size; j++)
	    {
		cftable_ptr->io_addr_range[i].start_io_addr
		    += data[offset] << 8 * j;
		offset++;
	    }

	    cftable_ptr->io_addr_range[i].length_io_addr = 0;
	    for (j = 0; j < length_size; j++)
	    {
		cftable_ptr->io_addr_range[i].length_io_addr
		    += data[offset] << 8 * j;
	    }
	}
    }
 end:
    DEBUG_1("<parse_cftable_entry rc=%X\n", rc);
    return rc;
}


/*
 * NAME: enable_pccard
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */

int enable_pccard(struct sfcom *sc)
{
    int err = CSR_SUCCESS;
    unsigned char value;

    DEBUG_1(">enable_pccard socket=%d", sc->pcmcia.logical_socket);

    /* hold lock of socket for exclusive operation */
    err = hold_lock( sc );
    if( err != CSR_SUCCESS )
    {
	goto end;
    }

    err = PowerVcc( sc, PM_PLANAR_ON );
    if( err != CSR_SUCCESS )
    {
	goto end;
    }

    delay( 3 * HZ / 10 );

    /* reset (0x80) Configuration Option Register */
    value = COR_SRESET;
    err = AccessConfigurationRegister(sc, CSCfgRegWRITE, COR, &value);
    if ( err != CSR_SUCCESS )
    {
	goto end;
    }

    /* clear (0x00) Configuration Option Register */
    value = 0;
    err = AccessConfigurationRegister(sc, CSCfgRegWRITE, COR, &value);
    if ( err != CSR_SUCCESS )
    {
	goto end;
    }
    delay( 10 * HZ / 10 );

    /* get the value to be written to configuration option register */
    /* from the io address specified */
    err = ioaddr_to_TPCE_INDEX(sc->pcmcia.logical_socket,
			       sc->sf_conf.port,
			       &sc->pcmcia.config_index );
    value = sc->pcmcia.config_index;
    if ( err != CSR_SUCCESS )
    {
	goto end;
    }

    /* set level interrupt between the PC Card and the PCIC */
    value |= COR_LevelIRQ;

    /* write index to Card Configuration Register */
    err = AccessConfigurationRegister(sc, CSCfgRegWRITE, COR, &value);
    if ( err != CSR_SUCCESS )
    {
	goto end;
    }

    /* read from Card Configuration and Status Register */
    err = AccessConfigurationRegister(sc, CSCfgRegREAD, CCSR, &value);
    if ( err != CSR_SUCCESS )
    {
	goto end;
    }

    /* enable audio */
    value |= CCSR_Audio;

    /* write to Card Configuration Register */
    err = AccessConfigurationRegister(sc, CSCfgRegWRITE, CCSR, &value);
    if ( err != CSR_SUCCESS )
    {
	goto end;
    }

    delay( HZ * 2 );

 end:
    /* release lock of socket for exclusive operation */
    release_lock( sc );

    DEBUG_1("<enable_pccard rc=%X\n", err);
    return err;
}

/*
 * NAME: disable_pccard
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS:
 */
int disable_pccard(struct sfcom *sc)
{
    int err = CSR_SUCCESS;
    unsigned char value;

    DEBUG_1(">disable_pccard socket=%d", sc->pcmcia.logical_socket);

    /* hold lock of socket for exclusive operation */
    err = hold_lock( sc );
    if( err != CSR_SUCCESS )
    {
	goto end;
    }

    /* reset (0x80) Configuration Option Register */
    value = COR_SRESET;
    err = AccessConfigurationRegister(sc, CSCfgRegWRITE, COR, &value);
    if (err != CSR_SUCCESS){
	goto end;
    }

    /* clear (0x00) Configuration Option Register */
    value = 0;
    err = AccessConfigurationRegister(sc, CSCfgRegWRITE, COR, &value);
    if (err != CSR_SUCCESS){
	goto end;
    }

    delay( HZ * 2 );

 end:
    /* release lock of socket for exclusive operation */
    release_lock( sc );

    DEBUG_1("<disable_pccard rc=%X\n", err);
    return err;
}

/*
 * NAME: PowerVcc
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int PowerVcc(struct sfcom *sc, int on)
{
    return ModifyConfiguration(sc,
			       (on ? CSCfgIRQSteer : 0) | CSCfgIRQChg |
			       CSCfgVccChg | CSCfgVpp1Chg | CSCfgVpp2Chg,
			       (on ? FXM_VCC : 0),
			       0, 0)
	   == CSR_SUCCESS ? 0 : 1 ;
}

/*
 * NAME: hold_lock
 *
 * FUNCTION: hold lock of specified socket for exclusive operation
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int hold_lock(struct sfcom *sc)
{
        int err;
        CSaixVdrSpcPkt  pkt;

        pkt.Socket   = sc->pcmcia.logical_socket;
        pkt.funccode = CSaixLockSocket;
        pkt.subcode  = _CSaixHoldLock;
        err = CardServices( CSVendorSpecific, &sc->pcmcia.clientID, NULL,
                        sizeof( CSaixVdrSpcPkt ), &pkt );

        return( err );
}

/*
 * NAME: release_lock
 *
 * FUNCTION: release lock of specified socket which has been held with
 *           hold_lock() request.
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: return codes defined by CardServices
 */
int release_lock(struct sfcom *sc)
{
        int err;
        CSaixVdrSpcPkt  pkt;

        pkt.Socket   = sc->pcmcia.logical_socket;
        pkt.funccode = CSaixLockSocket;
        pkt.subcode  = _CSaixReleaseLock;
        err = CardServices( CSVendorSpecific, &sc->pcmcia.clientID, NULL,
                        sizeof( CSaixVdrSpcPkt ), &pkt );

        return( err );
}

/*
 * NAME: extract_bits
 *
 * FUNCTION: converts the specified bits to int
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *   if field==0001010(binary) and mask==0001110(binary),
 *   then return==5(decimal).
 *
 * RETURNS: int for the bits
 */
int extract_bits( unsigned field, unsigned mask )
{
    field &= mask;
    if (field == 0) /* includes (mask == 0) */
    {
	return 0;
    }
    while ((mask & 1) == 0)
    {
	field >>= 1;
	mask >>= 1;
    }
    return (int)field;
}
