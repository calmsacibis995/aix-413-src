static char sccsid[] = "@(#)38	1.14  src/rspc/kernext/pcmcia/cs/cs.c, pcmciaker, rspc41J, 9517A_all 4/24/95 13:39:04";
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: 
 *	svcCardServices, CardServices,
 *
 *	InitCSfuncs, InitSocket, ResetSocket, ReadTuple,
 *      PowerOnSocket, PowerOffSocket, SetIntrMask,
 *      cspm_handler, cspm_socket_control
 *
 *	RegisterClient,	DeregisterClient, GetCardServicesInfo
 *      GetStatus, ResetCard, SetEventMask, GetEventMask
 *
 *	RequestIO, ReleaseIO, RequestIRQ, ReleaseIRQ, 
 *	RequestWindow, ReleaseWindow, ModifyWindow, MapMemPage
 * 	RequestConfiguration, ModifyConfiguration, ReleaseConfiguration
 *	GetConfigurationInfo, RequestSocketMask, ReleaseSocketMask, 
 *	AccessConfigurationRegister
 *
 *	OpenMemory, ReadMemory, WriteMemory, CopyMemory, 
 *	RegisterEraseQueue, CheckEraseQueue, DeregisterEraseQueue
 *	CloseMemory
 *
 *	GetFirstTuple, GetNextTuple, GetTupleData, GetFirstRegion, 
 *	GetNextRegion, GetFirstPartition, GetNextPartition,
 *
 *	MapLogSocket, MapPhySocket, MapLogWindow, MapPhyWindow
 *	RegisterMTD, RegisterTimer, SetRegion, ValidateCIS, 
 *	RequestExclusive, ReleaseExclusive, VendorSpecific, 
 *	GetFirstClient, GetNextClient, GetClientInfo
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

#include <sys/types.h>
#include <sys/device.h>
#include <sys/uio.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/malloc.h>
#include <sys/sysconfig.h>
#include <sys/errno.h>
#include <sys/pin.h>
#include <sys/adspace.h>
#include <sys/sleep.h>
#include <sys/lockl.h>
#include <sys/lockname.h>
#include <sys/lock_alloc.h>
#include <sys/trcmacros.h>
#include <sys/trchkid.h>
#include <sys/syspest.h>
#include <sys/pcmciass.h>
#include <sys/pcmciacs.h>
#include <sys/pcmciacsAix.h>
#include "cs.h"

extern Adapter *adapters;
extern Socket *sockets[];
extern Window *windows[];
extern int numAdapters;
extern int numSockets;
extern int numWindows;

extern Complex_lock cs_cfg_lock;
extern Simple_lock  cs_twin_lock;
extern int (*CSfunc[])();

static int GetCardServicesInfo        ( int *, void *, int, CSGetCSInfoPkt * );
static int RegisterClient             ( int *, void *, int, CSRegCliPkt * );
static int DeregisterClient           ( int *, void *, int, void * );
static int GetStatus                  ( int *, void *, int, CSGetStatPkt * );
static int ResetCard                  ( int *, void *, int, CSRstCardPkt * );
static int SetEventMask               ( int *, void *, int, CSEvMaskPkt * );
static int GetEventMask               ( int *, void *, int, CSEvMaskPkt * );

static int RequestIO                  ( int *, void *, int, CSIOPkt * );
static int ReleaseIO                  ( int *, void *, int, CSIOPkt * );
static int RequestIRQ                 ( int *, void *, int, CSReqIRQPkt * );
static int ReleaseIRQ                 ( int *, void *, int, CSRelIRQPkt * );
static int RequestWindow              ( int *, void *, int, CSReqWinPkt * );
static int ReleaseWindow              ( int *, void *, int, void * );
static int ModifyWindow               ( int *, void *, int, CSModWinPkt * );
static int MapMemPage                 ( int *, void *, int, CSMapMemPagePkt * );
extern int RequestSocketMask          ( int *, void *, int, CSReqSockMPkt * );
extern int ReleaseSocketMask          ( int *, void *, int, CSRelSockMPkt * );
static int RequestConfiguration       ( int *, void *, int, CSReqCfgPkt * );
static int GetConfigurationInfo       ( int *, void *, int, CSGetCfgInfoPkt * );
static int ModifyConfiguration        ( int *, void *, int, CSModCfgPkt * );
static int ReleaseConfiguration       ( int *, void *, int, CSRelCfgPkt * );
static int AccessConfigurationRegister( int *, void *, int, CSAccCfgRegPkt * );
/*
static int OpenMemory                 ( int *, void *, int, void * );
static int ReadMemory                 ( int *, void *, int, void * );
static int WriteMemory                ( int *, void *, int, void * );
static int CopyMemory                 ( int *, void *, int, void * );
static int RegisterEraseQueue         ( int *, void *, int, void * );
static int CheckEraseQueue            ( int *, void *, int, void * );
static int DeregisterEraseQueue       ( int *, void *, int, void * );
static int CloseMemory                ( int *, void *, int, void * );
*/
static int GetFirstTuple              ( int *, void *, int, CSGetTplPkt * );
static int GetNextTuple               ( int *, void *, int, CSGetTplPkt * );
static int GetTupleData               ( int *, void *, int, CSGetTplDataPkt * );
/*
static int GetFirstRegion             ( int *, void *, int, void * );
static int GetNextRegion              ( int *, void *, int, void * );
static int GetFirstPartition          ( int *, void *, int, void * );
static int GetNextPartition           ( int *, void *, int, void * );
*/
static int MapLogSocket               ( int *, void *, int, CSMapSktPkt * );
static int MapPhySocket               ( int *, void *, int, CSMapSktPkt * );
static int MapLogWindow               ( int *, void *, int, CSMapWinPkt * );
static int MapPhyWindow               ( int *, void *, int, CSMapWinPkt * );
/*
static int RegisterMTD                ( int *, void *, int, void * );
static int RegisterTimer              ( int *, void *, int, void * );
static int SetRegion                  ( int *, void *, int, void * );
*/
static int ValidateCIS                ( int *, void *, int, CSValCISPkt * );
static int RequestExclusive           ( int *, void *, int, CSExclPkt * );
static int ReleaseExclusive           ( int *, void *, int, CSExclPkt * );
static int GetFirstClient             ( int *, void *, int, CSGetClientPkt * );
static int GetNextClient              ( int *, void *, int, CSGetClientPkt * );
static int GetClientInfo              ( int *, void *, int, CSGetCliInfoPkt * );
static int VendorSpecific             ( int *, void *, int, void * );

/* pm functions */
int cspm_handler( caddr_t, int );
int cspm_socket_control( dev_t, int, int );

/* internal functions */
int InitSocket( int );
int ResetSocket( int );
int PowerOffSocket( int );
static int PowerOnSocket( int );
static int ReadTuple( Socket * );
extern int ConvertPower( Adapter*, int, int );

/* For Debug */
#if DEBUG
extern int var;
#endif

/****************************************************************************
 *
 * NAME:	CardServices
 *
 * FUNCTION:    Common Interface to request PCMCIA Card Services
 * 
 * EXECUTION ENVIRONMENT: config, client or event-handler thread
 * ARGUMENTS:
 *	int    functionID : Function ID to specify the Request.
 *	int  * objectID   : Pointer of Handle for Client or Window
 *	void * pointer    : Entry point
 *	int    argLen     : Length of Packet
 *	void * argPtr     : Pointer of Packet
 *
 * RETURNS:	All PCMCIA defined Return Codes
 *
 ***************************************************************************/
int
CardServices(int functionID, int * objectID, void * pointer,
		int argLen, void * argBuf )
{
    int rc;

    TRCHKL5(HKWD_PCMCIA_CS, functionID, (objectID ? *objectID : 0),
		 pointer, argLen, argBuf);

    BUGLPR(var, BUGGID, ("CardServices( 0x%x )\n", functionID ));

    if ( functionID >= CSF_MAX || functionID < 0 || 
	CSfunc[functionID] == NULL ){
	rc = CSR_UNSUPPORTED_FUNCTION;
    }
    else{
	lock_read( &cs_cfg_lock );
	rc = ( * CSfunc[functionID] )( objectID, pointer, argLen, argBuf );
	lock_done( &cs_cfg_lock );
    }

#ifdef DEBUG
    if ( rc )
	BUGLPR(var, BUGACT, ("CardServices( 0x%x ) : rc = 0x%x\n", \
			     functionID, rc ));
#endif /* DEBUG */

    TRCHKL5(HKWD_PCMCIA_CS, functionID, (objectID ? *objectID : 0),
	    pointer, rc, argBuf);
    return( rc );
}

/****************************************************************************
 *
 * NAME:	svcCardServices
 *
 * FUNCTION:    This function allows to use CardServices Calls from
 *              User Mode as System Call with same interface of Card Services
 *		  ( call CardServices() )
 * NOTE:        Even if the second argument (some entry point)
 *              is specified, calling the User Mode entry is not 
 *              implemented. So, those functions do not work.
 * EXECUTION ENVIRONMENT: This routine can be called by a process.
 * ARGUMENTS:
 *	int    functionID : Function ID to specify the Request.
 *	int  * objectID   : Pointer of Handle for Client or Window
 *	void * pointer    : Entry point
 *	int    argLen     : Length of Packet
 *	void * argPtr     : Pointer of Packet
 * RETURNS:	All PCMCIA defined Return Codes
 *      CSR_GENERAL_FAILURE : The calling process does not privilege 
 *
 ***************************************************************************/
int svcCardServices( int functionID, int * uobjectID, void * upointer,
			 int argLen, void  * uargBuf )
{
    int    objectID  = NULL;
    int  * kobjectID = NULL;
    void * kpointer  = NULL;
    void * kargBuf   = NULL;
    char ep;
    int rc;

    /* Some functions are restricted to use */
    if ( suser( &ep ) == 0 &&
	(functionID != CSGetCardServicesInfo && functionID != CSGetStatus &&
	 functionID != CSGetFirstTuple && functionID != CSGetNextTuple &&
	 functionID != CSGetTupleData && functionID != CSValidateCIS &&
	 functionID != CSMapLogSocket && functionID != CSMapPhySocket &&
	 functionID != CSMapLogWindow && functionID != CSMapPhyWindow &&
	 functionID != CSGetEventMask &&
	 functionID != CSGetConfigurationInfo &&
	 functionID != CSGetFirstClient && functionID != CSGetNextClient &&
	 functionID != CSGetClientInfo )){
	BUGLPR(var, BUGACT, ("Not allowed to exec: CSFunc(%x)\n", functionID));
	return CSR_GENERAL_FAILURE;
    }

    if ( uobjectID ){
	kobjectID = & objectID;
	copyin( uobjectID, kobjectID, sizeof ( int ) );
    }
    if ( argLen ){
	kargBuf = xmalloc( argLen, 4, kernel_heap );
	copyin( uargBuf,   kargBuf,   argLen );
    }
    rc = CardServices( functionID, kobjectID, kpointer, argLen, kargBuf );
    if ( kobjectID )
	copyout( kobjectID, uobjectID, sizeof( int ) );
    if ( kargBuf ){
	copyout( kargBuf, uargBuf, argLen );
	xmfree( kargBuf, kernel_heap );
    }

    return rc;
}


/*****************************************************************************
 *
 * NAME:	InitCSfuncs
 *
 * FUNCTION:	Initialize CardServices sub-functions disaptch table
 *			
 * EXECUTION ENVIRONMENT: config thread ( InitCardServices )
 * ARGUMENTS:   none.
 * RETURNS:	none.
 *
 ****************************************************************************/
void initCSfuncs()
{
    /* 
     * Initialize CS function dispatch table
     */
    CSfunc[CSGetCardServicesInfo] = GetCardServicesInfo;
    CSfunc[CSRegisterClient] 	  = RegisterClient;
    CSfunc[CSDeregisterClient]    = DeregisterClient;
    CSfunc[CSGetStatus]           = GetStatus;
    CSfunc[CSResetCard]           = ResetCard;
    CSfunc[CSSetEventMask]        = SetEventMask;
    CSfunc[CSGetEventMask]        = GetEventMask;
    
    CSfunc[CSRequestIO]           = RequestIO;
    CSfunc[CSReleaseIO]           = ReleaseIO;
    CSfunc[CSRequestIRQ]          = RequestIRQ;
    CSfunc[CSReleaseIRQ]          = ReleaseIRQ;
    CSfunc[CSRequestWindow]       = RequestWindow;
    CSfunc[CSReleaseWindow]       = ReleaseWindow;
    CSfunc[CSModifyWindow]        = ModifyWindow;
    CSfunc[CSMapMemPage]          = MapMemPage;
    CSfunc[CSRequestSocketMask]   = RequestSocketMask;
    CSfunc[CSReleaseSocketMask]   = ReleaseSocketMask;
    CSfunc[CSRequestConfiguration]= RequestConfiguration;
    CSfunc[CSGetConfigurationInfo]= GetConfigurationInfo;
    CSfunc[CSModifyConfiguration] = ModifyConfiguration;
    CSfunc[CSReleaseConfiguration]= ReleaseConfiguration;
    CSfunc[CSAccessConfigurationRegister]=AccessConfigurationRegister;
/*    
    CSfunc[CSOpenMemory]          = OpenMemory;
    CSfunc[CSReadMemory]          = ReadMemory;
    CSfunc[CSWriteMemory]         = WriteMemory;
    CSfunc[CSCopyMemory]          = CopyMemory;
    CSfunc[CSRegisterEraseQueue]  = RegisterEraseQueue;
    CSfunc[CSCheckEraseQueue]     = CheckEraseQueue;
    CSfunc[CSDeregisterEraseQueue]= DeregisterEraseQueue;
    CSfunc[CSCloseMemory]         = CloseMemory;
*/    
    CSfunc[CSGetFirstTuple]       = GetFirstTuple;
    CSfunc[CSGetNextTuple]        = GetNextTuple;
    CSfunc[CSGetTupleData]        = GetTupleData;
/*
    CSfunc[CSGetFirstRegion]      = GetFirstRegion;
    CSfunc[CSGetNextRegion]       = GetNextRegion;
    CSfunc[CSGetFirstPartition]   = GetFirstPartition;
    CSfunc[CSGetNextPartition]    = GetNextPartition;
*/    
    CSfunc[CSMapLogSocket]        = MapLogSocket;
    CSfunc[CSMapPhySocket]        = MapPhySocket;
    CSfunc[CSMapLogWindow]        = MapLogWindow;
    CSfunc[CSMapPhyWindow]        = MapPhyWindow;
/*
    CSfunc[CSRegisterMTD]         = RegisterMTD;
    CSfunc[CSRegisterTimer]       = RegisterTimer;
    CSfunc[CSSetRegion]           = SetRegion;
*/
    CSfunc[CSValidateCIS]         = ValidateCIS;
    CSfunc[CSRequestExclusive]    = RequestExclusive;
    CSfunc[CSReleaseExclusive]    = ReleaseExclusive;
    CSfunc[CSGetFirstClient]      = GetFirstClient;
    CSfunc[CSGetNextClient]       = GetNextClient;
    CSfunc[CSGetClientInfo]       = GetClientInfo;
    CSfunc[CSVendorSpecific]      = VendorSpecific;
}

/****************************************************************************
 *
 * NAME:	InitSocket
 *
 * FUNCTION:	This function initialize a Socket. 
 *		  - ResetSocket();
 *		  - ReadTuple();
 * EXECUTION ENVIRONMENT: config thread PM thread event-handler thread
 * 
 * ARGUMENTS:	
 *	LogSocket : Logical Socket
 * RETURNS:	0      : Success
 *              others : Fail
 *	
 ****************************************************************************/
int InitSocket( int LogSocket )
{
    int old, rc;
    Socket  * skt;
    extern Simple_lock cs_intr_lock;

    BUGLPR(var, BUGACT, ("InitSocket( %x )\n", LogSocket));

    if ( rc = ValidateSocket( LogSocket, &skt ) ){
	return( rc );
    }

    lock_write( &skt->rw_lock );
    /*
     * Check if some additional card detection are happened.
     */
    old = disable_lock( adapters->intr.priority, &cs_intr_lock );
    if( rc = ( skt->critical_state != 1 ) ){
	unlock_enable( old, &cs_intr_lock );
	lock_done( &skt->rw_lock );
	return( rc );
    }else{
	skt->critical_state = 0;
    }
    unlock_enable( old, &cs_intr_lock );

    BUGLPR(var, BUGACT, ("A Card was detected at a:%x:s:%x\n",\
			 adapters[skt->AdapterIndex].PhyAdapter,
			 skt->PhySocket));
    /*
     * Reset the Socket and make the Card Available
     */
    if ( ! CheckInterest( skt ) || ! skt->cfgInfo.Vcc || 
#if defined( PM_ENABLE )
	(adapters[skt->AdapterIndex].pmh.mode != PM_DEVICE_SUSPEND) ){
#else
	1 ){
#endif
	    /*
	     * Reset and PowerOn Socket to read tuple
	     */
	    if ( !( rc = ResetSocket( LogSocket ) ) ){
		rc = PowerOnSocket( LogSocket );
	    }
    }
    /* Read Tuple to fill skt->cfgInfo struct */
    if ( ! rc ){
	rc = ReadTuple( skt );
    }

    if( rc ){
	/* 
	 * PC card must be removed buring this initialization
	 */
	PowerOffSocket( LogSocket );
    }
    else{
	set_PowerOff_timer( skt );
    }

    lock_done( &skt->rw_lock );
    BUGLPR(var, BUGACT, ("InitSocket( %x ) : rc = %x\n", LogSocket, rc));
    return( rc );
}

/****************************************************************************
 *
 * NAME:	ResetSocket
 *
 * FUNCTION:	Reset the Socket to reset the Card
 *
 * EXECUTION ENVIRONMENT: config or event-handler thread
 *
 * ARGUMENTS:	LogSocket : Logical Socket
 * RETURNS:	CSR_SUCCESS         : Success
 *              CSR_GENERAL_FAILURE : Lock or ConvPower Fail
 *
 ****************************************************************************/
/*
 *             Reset                      Reset           
 * PowerON    _-~| |~~~~~~~~|~~~~    ~~~~~~| |~~~~~~~~~~~ 
 *         _-~   | |        |<--  Card  -->| |        |<-- Card -->  
 *      _-~      | |        |    Enable    | |        |   Enable 
 * ----+---------+-+--------|---- ... -----+-+--------|----
 *     |<------->| |<------>|                |<------>|   
 *      Max 300ms   Min 20ms                  Min 20ms    
 */

int ResetSocket( int LogSocket )
{
    SetSocket_S setskt;
    ResetSocket_S rstskt;
    GetStatus_S getstat;
    SetWindow_S setWin;

    Window * win;
    Socket * skt;
    Adapter * adpt;
	
    int Vcc, Vpp1, Vpp2;
    int i, w, rc;

    BUGLPR(var, BUGACT, ("ResetSocket( %x )\n", LogSocket ));

    if ( rc = ValidateSocket( LogSocket, &skt ) ){
	return( rc );
    }

    if ( rc = PowerOnSocket( skt->LogSocket ) ){
	return( rc );
    }
    adpt = &adapters[skt->AdapterIndex];

    /* 
     * Reset the Socket
     */
    bzero( &rstskt,   sizeof( ResetSocket_S ) );
    rstskt.Socket = skt->PhySocket;
    rc = SocketServices( adpt, PCM_RESET_SOCKET, (caddr_t)&rstskt );
    if ( rc ) goto end;
    /*
     * Wait until Card to be Ready
     */
    bzero( &getstat, sizeof( GetStatus_S ) );
    getstat.Socket = skt->PhySocket;
    for( i = 0; i < 500; i++ ){ /* 10 ms */
        delay ( 20 * HZ / 1000 );
	rc = SocketServices( adpt, PCM_GET_STATUS, (caddr_t)&getstat );
	if ( rc ) goto end;
	if( getstat.CardState & PCM_SBM_RDYBSY ){
	    BUGLPR(var, BUGNTF, ("PC Card on Socket (%x) is ready\n",\
				 skt->LogSocket));
	    break;
	}
    }
    
    /*
     * Set Interrupt Mask
     */
    setskt.Socket      = skt->PhySocket;
    rc = SocketServices( adpt, PCM_GET_SOCKET, (caddr_t)&setskt );
    if ( rc ) goto end;
    setskt.SCIntMask   = skt->SCIntCaps & PCM_SBM_CD;
    setskt.State       = 0;
    setskt.CtlInd      = skt->CtlIndCaps & PCM_SBM_BUSY;
    setskt.IREQRouting =
	( skt->cfgInfo.Attributes & CSCfgIRQSteer ) ?
	    (PCM_IRQ_ENABLE | skt->cfgInfo.AssignedIRQ) : 0;
    setskt.IFType      =
	skt->cfgInfo.IntType ? skt->cfgInfo.IntType : PCM_IF_MEMORY;
    rc = SocketServices( adpt, PCM_SET_SOCKET, (caddr_t)&setskt );

    /*
     * Enable MEM & IO windows
     */
    for( win = adpt->window, w = 0; w < adpt->numWindows; win++, w++ ){
        if ( win->LogSocket == skt->LogSocket &&
	     ( win->setWin.State & PCM_WS_ENABLED ) != 0 ){
	    setWin.Window = win->PhyWindow;
	    setWin.Socket = skt->PhySocket;
	    rc = SocketServices( adpt, PCM_GET_WINDOW, (caddr_t)&setWin );
	    if ( rc ) goto end;
	    setWin.State |= PCM_WS_ENABLED;
	    rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)&setWin );
	    if ( rc ) goto end;
	}
    }

 end:
    set_activity( skt );
    if ( !CheckInterest( skt ) ){
	set_PowerOff_timer( skt );
    }
    BUGLPR(var, BUGACT, ("ResetSocket( %x ) : rc = %x\n", LogSocket, rc ));
    return( rc ); 
}

/****************************************************************************
 *
 * NAME:	ReadTuple
 *
 * FUNCTION:	Read CIS to Set the Tuple data required by 
 *              GetConfigurationInfo()
 *              ( DEVI, FUNC, MANU Tuples )
 * EXECUTION ENVIRONMENT: config, or  event-handler thread
 * ARGUMENTS:
 *      adpt   : pointer of Adapter
 *      skt    : pointer of Socket
 * RETURNS:
 *      CSR_SUCCESS         : Success
 *      CSR_GENERAL_FAILURE : Memory Allocation Error
 *      CSR_NO_CARD         : No Card 
 *
 ****************************************************************************/
int ReadTuple( Socket * skt )
{
    static char ptr[ sizeof(CSGetTplDataPkt) + 0xff ];
    CSGetTplDataPkt *tuple = (CSGetTplDataPkt *) ptr;
    int rc;
    
    /* initilize the value : 0xff means there is no matched tuple */
    skt->cfgInfo.FirstDevType = 0xff;
    skt->cfgInfo.FuncCode     = 0xff;
    skt->cfgInfo.SysInitMask  = 0xff;
    skt->cfgInfo.ManufCode    = 0xff;
    skt->cfgInfo.ManufInfo    = 0xff;

    bzero( tuple, sizeof( CSGetTplDataPkt ) + 0xff );
    tuple->Socket = skt->LogSocket;
    tuple->Attributes = 0;
    tuple->TupleOffset  = 0;
    tuple->TupleDataMax = 0xff;

    tuple->DesiredTuple = CISTPL_DEVICE;
    if( ( rc = tuple_search( (CSGetTplPkt * )tuple,
			    TPGetFirstData ) ) == CSR_SUCCESS ){
	skt->cfgInfo.FirstDevType = *(tuple->TupleData);
    }
    else if( rc == CSR_NO_CARD || rc == CSR_READ_FAILURE ){
	goto end;
    }
    
    tuple->DesiredTuple = CISTPL_FUNCID;
    if( ( rc = tuple_search( (CSGetTplPkt * )tuple,
			    TPGetFirstData ) ) == CSR_SUCCESS ){
	/*
	 * Function Code should be between 0 and 7. 
	 */
	if( *tuple->TupleData < 0 || 7 < *tuple->TupleData ){
	    rc = CSR_READ_FAILURE;
	    goto end;
	}
	skt->cfgInfo.FuncCode    = *(tuple->TupleData);
	skt->cfgInfo.SysInitMask = *(tuple->TupleData+1);
    }
    else if( rc == CSR_NO_CARD || rc == CSR_READ_FAILURE ){
	goto end;
    }
    
    tuple->DesiredTuple = CISTPL_MANFID; 
    if( ( rc = tuple_search( (CSGetTplPkt * )tuple,
			    TPGetFirstData ) ) == CSR_SUCCESS ){
	skt->cfgInfo.ManufCode   = *(tuple->TupleData + 1);
	skt->cfgInfo.ManufCode <<= 8;
	skt->cfgInfo.ManufCode  += *(tuple->TupleData);
	skt->cfgInfo.ManufInfo   = *(tuple->TupleData + 3);
	skt->cfgInfo.ManufInfo <<= 8;
	skt->cfgInfo.ManufInfo  += *(tuple->TupleData + 2);
    }
    else if( rc == CSR_NO_CARD || rc == CSR_READ_FAILURE ){
	goto end;
    }

 end:
    if( rc == CSR_NO_CARD || rc == CSR_READ_FAILURE ){
        return ( rc );
    }
    return( CSR_SUCCESS );
}

/****************************************************************************
 *
 * NAME:	PowerOnSocket
 *
 * FUNCTION:	Power on the specified socket 
 *              5V is set to Vcc, Vpp1 and Vpp2, respectively.
 *		
 * EXECUTION ENVIRONMENT: client, config, event-handler and PM thread
 *      ( ResetSocket(), GetFrist/NextTuple(), GetTupleData(), ValidateCIS() )
 * ARGUMENTS:   
 *	LogSocket : Logical Socket
 * RETURNS:
 *      CSR_SUCCESS         : Success
 *	CSR_GENERAL_FAILURE : ConvPower Fail 
 *                    or Card is configured but the client do not request Vcc
 *
 ****************************************************************************/
int PowerOnSocket (int LogSocket )
{
    Adapter * adpt;
    Socket * skt;
    
    int rc;
    int Vcc, Vpp1, Vpp2;
    SetSocket_S setskt;

    if ( rc = ValidateSocket( LogSocket, &skt ) ){
	return( rc );
    }
    adpt = &adapters[skt->AdapterIndex];
    if ( !CheckCardPresence( skt )){
	return( CSR_NO_CARD );
    }

#if defined( PM_ENABLE )
    adpt->pmh.activity |= PM_ACTIVITY_OCCURRED;
#endif

    while( tstop( skt->pwrofft ) );

    if( skt->cfgInfo.Vcc ){
	return( CSR_SUCCESS );
    }

    BUGLPR(var, BUGGID, ("PowerOnSocket( %d )\n", LogSocket));

    if(( Vcc  = ConvertPower( adpt, DEFPOWER, PCM_VCC  ) ) == -1 ||
       ( Vpp1 = ConvertPower( adpt, DEFPOWER, PCM_VPP1 ) ) == -1 ||
       ( Vpp2 = ConvertPower( adpt, DEFPOWER, PCM_VPP2 ) ) == -1 ){
	BUGLPR(var, BUGACT, ("Bad PowerIndex\n"));
	rc = CSR_GENERAL_FAILURE;
    }

    setskt.Socket      = skt->PhySocket;
    if ( rc = SocketServices( adpt, PCM_GET_SOCKET, (caddr_t)&setskt ) ){
	return( rc );
    }
    setskt.VccLevel    = 0xff & Vcc;      /* supply 5.0 volte      */
    setskt.VppLevels   = (0xf0 & (Vpp2 << 4)) | (0x0f & Vpp1);
    setskt.State       = 0;
    setskt.CtlInd      = skt->CtlIndCaps & PCM_SBM_BUSY;
    setskt.IREQRouting =                  /* set after reset       */
	( skt->cfgInfo.Attributes & CSCfgIRQSteer ) ?
	    (PCM_IRQ_ENABLE | skt->cfgInfo.AssignedIRQ) : 0;
    setskt.IFType      =
	skt->cfgInfo.IntType ? skt->cfgInfo.IntType : PCM_IF_MEMORY;
	/* PCM_IF_MEMORY; */   /* default is IF_MEMORY  */
    if ( rc = SocketServices( adpt, PCM_SET_SOCKET, (caddr_t)&setskt ) ){
	return( rc );
    }
    skt->cfgInfo.Vcc     = DEFPOWER;
    skt->cfgInfo.Vpp1    = DEFPOWER;
    skt->cfgInfo.Vpp2    = DEFPOWER;

    delay ( 300 * HZ / 1000 );

    return( rc );
}


/****************************************************************************
 *
 * NAME:	CSPowerOffSocket
 *
 * FUNCTION:	Call CS internal PowerOffSocket 
 *			
 * EXECUTION ENVIRONMENT:  event-handler thread
 * ARGUMENTS:
 *	LogSocket : Logical Socket
 * RETURNS:	
 *      CSR_SUCCESS         : Success
 *      CSR_BAD_SOCKET      : Socket is invalid
 *      CSR_GENERAL_FAILURE : ConvPower Fail
 *
 ****************************************************************************/
int CSPowerOffSocket( int LogSocket )
{
    int rc;
    Socket * skt;

    if ( rc = ValidateSocket( LogSocket, &skt ) ){
	return( rc );
    }
    lock_write( &skt->rw_lock );
    rc = PowerOffSocket( LogSocket );
    lock_done( &skt->rw_lock );

    return( rc );
}


/****************************************************************************
 *
 * NAME:	PowerOffSocket
 *
 * FUNCTION:	Power off the specified socket.
 *			
 * EXECUTION ENVIRONMENT: config or event-handler thread
 * ARGUMENTS:
 *	LogSocket : Logical Socket
 * RETURNS:	
 *      CSR_SUCCESS         : Success
 *      CSR_BAD_SOCKET      : Socket is invalid
 *      CSR_GENERAL_FAILURE : ConvPower Fail
 *
 ****************************************************************************/
int PowerOffSocket( int LogSocket )
{
    Adapter * adpt;
    Socket * skt;
    int rc;
    int Vcc, Vpp1, Vpp2;
    SetSocket_S setskt;

    if ( rc = ValidateSocket( LogSocket, &skt ) ){
	return( rc );
    }
    adpt = &adapters[skt->AdapterIndex];

    while( tstop( skt->pwrofft ) );

    /*
     * Power Off, But Set Interrupt Mask
     */
    BUGLPR(var, BUGACT, ("PowerOffSocket( %x )\n", LogSocket) );
    if(( Vcc  = ConvertPower( adpt, 0, PCM_VCC  ) ) == -1 ||
       ( Vpp1 = ConvertPower( adpt, 0, PCM_VPP1 ) ) == -1 ||
       ( Vpp2 = ConvertPower( adpt, 0, PCM_VPP2 ) ) == -1 ){
	rc = CSR_GENERAL_FAILURE;
    }else{
	setskt.Socket      = skt->PhySocket;
	setskt.SCIntMask   = skt->SCIntCaps & PCM_SBM_CD;
	setskt.VccLevel    = 0xff & Vcc;
	setskt.VppLevels   = (0xf0 & (Vpp2 << 4)) | (0x0f & Vpp1);
	setskt.State       = 0;
	setskt.CtlInd      = 0;
	setskt.IREQRouting = 0;
	setskt.IFType      = PCM_IF_MEMORY;
	if (!(rc = SocketServices( adpt, PCM_SET_SOCKET, (caddr_t)&setskt ))){
	    skt->cfgInfo.Vcc     = 0;
	    skt->cfgInfo.Vpp1    = 0;
	    skt->cfgInfo.Vpp2    = 0;
	}
    }
    delay ( 300 * HZ / 1000 );

    return( rc );
}

/*****************************************************************************
 *
 * NAME:        cspm_handler
 *
 * FUNCTION:	planer control routine for PM
 *
 * EXECUTION ENVIRONMENT: PM kernel thread
 * ARGUMENTS:
 *	private  : I    pointer to Adapter struct
 *	cmd      : I    PM mode 
 * RETURNS:
 *      PM_SUCCESS  : Success
 *      PM_ERROR    : Error
 ****************************************************************************/
int cspm_handler ( caddr_t private, int cmd )
{
#if defined( PM_ENABLE )
    int i, s, interest, rc = 0;
    Adapter * adpt = (Adapter *) private;
    Socket  * skt;
    SetSocket_S setskt;
    SetAdapter_S setadpt;

    BUGLPR(var, BUGACT, ("pm_cmd ( %x -> %x )\n", adpt->pmh.mode, cmd) );

    switch( cmd ) {
    case PM_DEVICE_FULL_ON:
	if ( adpt->pmh.mode != PM_DEVICE_ENABLE ){
	    return( PM_ERROR );
	}
	BUGLPR(var, BUGACT, ("cmd = PM_DEVICE_FULL_ON\n") );

	for(s = 0, skt = adpt->socket; s < adpt->numSockets; s++, skt++ ){
	    while( tstop( skt->pwrofft ) );
	}
	adpt->pmh.mode = cmd;
	break;

    case PM_DEVICE_ENABLE:
	BUGLPR(var, BUGACT, ("cmd = PM_DEVICE_ENABLE\n") );
	switch ( adpt->pmh.mode ){
	case PM_DEVICE_FULL_ON:
	    lock_write( &cs_cfg_lock );
	    for(s = 0, interest = 0, skt = adpt->socket;
		s < adpt->numSockets; s++, skt++ ){
	        if ( ! CheckInterest( skt ) ){
                    set_PowerOff_timer( skt );
		}else{
		    interest++;
		}
	    }
	    /* set activity flag */
	    if ( interest ){
	        adpt->pmh.activity = PM_ACTIVITY_NOT_SET;
	    }else{
	        adpt->pmh.activity = PM_NO_ACTIVITY;
	    }
	    lock_done( &cs_cfg_lock );
	    break;

	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:

	    if(rc = SocketServices( adpt, PCM_GET_ADAPTER, (caddr_t)&setadpt )){
	        lock_done( &cs_cfg_lock );
		break;
	    }
	    /* restore register context */
	    setadpt.State &= ~(PCM_AS_POWERDOWN | PCM_AS_MAINTAIN);
	    /* enable status change interrupt */
	    setadpt.SCRouting = PCM_IRQ_ENABLE | adpt->intr.level;
	    if(rc = SocketServices( adpt, PCM_SET_ADAPTER, (caddr_t)&setadpt )){
	        lock_done( &cs_cfg_lock );
		break;
	    }
	    /* re-initialize sockets */
	    for(s = 0, interest = 0, skt = adpt->socket;
		s < adpt->numSockets; s++, skt++ ){
		GetStatus_S getstat;

		bzero( &getstat,   sizeof( GetStatus_S ) );
		getstat.Socket = s;
		rc = SocketServices( adpt, PCM_GET_STATUS, (caddr_t)&getstat );
		if ( rc ) break;

		skt->critical_state = 1;
		if ( getstat.CardState & PCM_SBM_CD ){
		    skt->card_present = 1;
		    InitSocket( skt->LogSocket );
		}else{
		    skt->card_present = 0;
		}

		if ( CheckInterest( skt ) ){
		    interest++;
		    setskt.Socket = s;
		    if ( rc = SocketServices( adpt, PCM_GET_SOCKET,
					     (caddr_t)&setskt ))
		        break;
		    /* enable interrupt */
		    if ( skt->cfgInfo.Attributes & CSCfgIRQSteer )
			setskt.IREQRouting =
			    PCM_IRQ_ENABLE | skt->cfgInfo.AssignedIRQ;
		    /* turn on LED */
		    if ( skt->cfgInfo.Vcc && 
			skt->client_LED == _CSaixTurnOnLED)
			setskt.CtlInd |= skt->CtlIndCaps & PCM_SBM_BUSY;
		    if ( rc = SocketServices( adpt, PCM_SET_SOCKET,
					     (caddr_t)&setskt ))
		        break;
		}
	    }
	    if ( rc ){
	        lock_done( &cs_cfg_lock );
		break;
	    }
	    /* set activity flag */
	    if ( interest ){
	        adpt->pmh.activity = PM_ACTIVITY_NOT_SET;
	    }else{
	        adpt->pmh.activity = PM_NO_ACTIVITY;
	    }
	    lock_done( &cs_cfg_lock );
	    break;

	case PM_DEVICE_ENABLE:
	    break;
	case PM_DEVICE_IDLE:
	    /* currently, there is no I/F to control IDLE mode */
	    rc = PM_ERROR;
	    break;
	default:
	    rc = PM_ERROR;
	    break;
	}

	if ( rc ){
	    break;
	}
	adpt->pmh.mode = cmd;
	BUGLPR(var, BUGACT, ("cmd = PM_DEVICE_ENABLE : Done\n") );
	break;

    case PM_DEVICE_IDLE:
	/* this mode is not supported */
	BUGLPR(var, BUGACT, ("cmd = PM_DEVICE_IDLE is not suppored\n") );
	rc = PM_ERROR;
	break;

    case PM_DEVICE_SUSPEND:
    case PM_DEVICE_HIBERNATION:
	BUGLPR(var, BUGACT, ("cmd = PM_DEVICE_SUSPEND / HIBERNATION\n") );
	if ( adpt->pmh.mode != PM_DEVICE_ENABLE ){
	    return( PM_ERROR );
	}
	lock_write( &cs_cfg_lock );
	for(s = 0, skt = adpt->socket; s < adpt->numSockets; s++, skt++ ){
	    while( tstop( skt->pwrofft ) );
	    setskt.Socket      = s;
	    if ( rc = SocketServices( adpt, PCM_GET_SOCKET, (caddr_t)&setskt ) )
		break;
	    /* turn off indicators and disable I/O interrupt */
	    setskt.CtlInd &= ~(skt->CtlIndCaps & PCM_SBM_BUSY);
	    setskt.IREQRouting   = 0; 
	    /* if card is not configured power off */ 
	    if( !(skt->cfgInfo.Attributes & CSCfgAttrValidCli) ){
		setskt.VccLevel  =  0xff & ConvertPower( adpt, 0, PCM_VCC  );
		setskt.VppLevels 
		    = (0xf0 & ( ConvertPower( adpt, 0, PCM_VPP2 ) << 4 ))
			| (0x0f & ConvertPower( adpt, 0, PCM_VPP1 ) );
	    }
	    if ( rc = SocketServices( adpt, PCM_SET_SOCKET, (caddr_t)&setskt ) )
		break;
	}
	if ( rc ){
	    lock_done( &cs_cfg_lock );
	    break;
	}
	BUGLPR(var, BUGACT, ("cmd = PM_DEVICE_SUSPEND / HIBERNATION...\n") );
	    
	/* If some slot is configured, SS stores the context of PCIC(s) */
	if ( rc = SocketServices( adpt, PCM_GET_ADAPTER,
				 (caddr_t)&setadpt ) ){
	    lock_done( &cs_cfg_lock );
	    break;
	}
	if ( adpt->pmh.activity == PM_ACTIVITY_NOT_SET){
	    setadpt.State |= (PCM_AS_POWERDOWN | PCM_AS_MAINTAIN);
	}else{
	    setadpt.State |= PCM_AS_POWERDOWN;
	}
	setadpt.SCRouting &= ~PCM_IRQ_ENABLE;
	if ( rc = SocketServices( adpt, PCM_SET_ADAPTER, (caddr_t)&setadpt ) ){
	    lock_done( &cs_cfg_lock );
	    break;
	}
	/* 
	 * Routing RI signal is done by SS 
	 * whenever I/O PCcards are configured.
	 */
	adpt->pmh.activity = PM_ACTIVITY_NOT_SET;
	adpt->pmh.mode = cmd;
	BUGLPR(var, BUGACT, ("cmd = PM_DEVICE_SUSPEND/HIBERNATION: Done\n") );
	break;

    case PM_PAGE_FREEZE_NOTICE:
	rc = pincode( CardServices );
	break;

    case PM_PAGE_UNFREEZE_NOTICE:
	unpincode( CardServices );
	break;

    default:
	rc = PM_ERROR;
	break;
    }

    if ( rc ){
	return( PM_ERROR );
    }else{
	return( PM_SUCCESS );
    }
#endif /* PM_ENABLE */
}

/*****************************************************************************
 * 
 * NAME:        cspm_socket_control
 *
 * FUNCTION:	planer (socket)  control routine for PM
 *
 * EXECUTION ENVIRONMENT: PM kernel thread
 * ARGUMENTS:
 *      devno    : I    device number
 *	devid    : I    PM device ID ( PMDEV_PCMCIA00 + bus# + PhySocket# )
 *	cmd      : I    PM planar control command
 * RETURNS:
 *      PM_SUCCESS  : Success
 *      PM_ERROR    : devid or cmd is invalid
 *                    fail to control planar on/off
 *
 *****************************************************************************/
int cspm_socket_control ( dev_t devno, int devid, int cmd )
{
#if defined( PM_ENABLE )
    int a, rc;
    int PhySocket = devid & 0xf;
    int busnum = (0xffff & devid) >> 4;

    Socket  * skt;

    if ( (uint)( devid >> 0x10 ) != (uint)( PMDEV_PCMCIA00 >> 0x10 ) )
	return( PM_ERROR );
    for( a = 0; a < numAdapters; a++ ){
	if ( atoi(&adapters[a].devname[3]) == busnum ){
	    skt = &adapters[a].socket[PhySocket];
	    break;
	}
    }
    if ( a == numAdapters ){
	return( PM_ERROR );
    }

    switch( cmd ){
    case PM_PLANAR_QUERY:
	rc = PM_PLANAR_QUERY | PM_PLANAR_ON | PM_PLANAR_OFF
		| PM_PLANAR_CURRENT_LEVEL;
	break;
    case PM_PLANAR_ON:
	lock_write( &skt->rw_lock );
	if ( ! ResetSocket( skt->LogSocket ) ){
	    rc = PM_SUCCESS;
	}else{
	    rc = PM_ERROR;
	}
	skt->client_Vcc = 1;
	lock_done( &skt->rw_lock );
	break;
    case PM_PLANAR_OFF:
	lock_write( &skt->rw_lock );
	if ( ! PowerOffSocket ( skt->LogSocket ) ){
	    rc = PM_SUCCESS;
	}else{
	    rc = PM_ERROR;
	}
	skt->client_Vcc = 0;
	lock_done( &skt->rw_lock );
	break;
    case PM_PLANAR_CURRENT_LEVEL:
	lock_read( &skt->rw_lock );
	if( skt->cfgInfo.Vcc || skt->cfgInfo.Vpp1 ||skt->cfgInfo.Vpp2 ){
	    rc = PM_PLANAR_ON;
	}else{
	    rc = PM_PLANAR_OFF;
	}
	lock_done( &skt->rw_lock );
	break;
    default:  /* PM_PLANAR_LOWPOWER1/2 */
	rc = PM_ERROR;
    }

    return ( rc );
#endif /* PM_ENABLE */
}

/****************************************************************************
 *
 * NAME:	SetIntrMask
 *
 * FUNCTION:	Set the Interrupte mask 
 * EXECUTION ENVIRONMENT: event-handler thread
 * ARGUMENTS:
 *	LogSocket : Logical Socket
 *	mask      : Interrupt Mask
 * RETURNS:
 *	CSR_SUCCESS   : Success
 *	CSR_BAD_SOCKET:	Socket is invalid
 *
 ****************************************************************************/
int SetIntrMask( int LogSocket, uint mask )
{
    SetSocket_S setskt;
    Socket * skt;
    int rc;

    if ( rc = ValidateSocket( LogSocket, &skt ) ){
	return( rc );
    }
    setskt.Socket = skt->PhySocket;
    rc = SocketServices( &adapters[skt->AdapterIndex], 
			PCM_GET_SOCKET, (caddr_t)&setskt );
    if ( rc ) return( rc );
    setskt.State = 0;
    setskt.SCIntMask = skt->SCIntCaps & ( PCM_SBM_CD | mask );
    rc = SocketServices( &adapters[skt->AdapterIndex], 
		   PCM_SET_SOCKET, (caddr_t)&setskt );

    return( rc );
}

/*
 * CLIENT SERVICES FUNCTIONS 
 */
/****************************************************************************
 *
 * NAME:	RegisterClient
 *
 * FUNCTION:	Register a client with Card Services
 *		  ( call cfehRegisterClient() )
 * EXECUTION ENVIRONMENT: client thread
 *                        with cs_cfg_lock=r 
 * ARGUMENTS:
 *	clientID : O	pointer of client handle
 *	cbEntry  : I	entry point of Client's Callback Handler
 *	argLen   : I	length of packet CSRegCliPkt
 *	regCli   : I	pointer of CSRegCliPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSRegCliPkt )
 *	CSR_BAD_ATTRIBUTE  : No client type or more than one type spcified
 * 	CSR_OUT_OF_RESOURCE: No space in CardServices to register client
 * 
 ****************************************************************************/
int
RegisterClient( int * clientID, void * cbEntry, 
			int argLen, CSRegCliPkt * regCli )
{ 
    uint     attr = regCli -> Attributes;
    int      type = attr & ( CSCliAttrMCDD | CSCliAttrMTD | CSCliAttrIOCDD );

    /* 
     *  Validate the CSRegCliPkt
     */
    if ( argLen < sizeof( CSRegCliPkt ) )
	 return( CSR_BAD_ARG_LENGTH );
    
    /* Check client type flags */
    if( type == 0 || ( attr & CSCliAttrRSV ) ||
       type != CSCliAttrMCDD && type != CSCliAttrMTD && type != CSCliAttrIOCDD ){
	return( CSR_BAD_ATTRIBUTE );
    }
    
    /*
     * Register a new card services client to Config EventHandler
     */
    if( !( *clientID = cfehRegisterClient(
      cbEntry, attr, regCli->EventMask, regCli->ClientData ) ) ){
	return( CSR_OUT_OF_RESOURCE );
    }
    
    return( CSR_SUCCESS );
}

/****************************************************************************
 *
 * NAME:	DeregisterClient
 *
 * FUNCTION:	Remove a client from list of registerd clients
 *		  ( call cfehDeregisterClient() )
 * EXECUTION ENVIRONMENT: client thread
 *                        with cs_cfg_lock=r 
 * ARGUMENTS:   
 *	clientID : I	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	must be zero (0)
 *	argBuf   : -	(not used)
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is not equal to zero (0)
 *      CSR_BUSY           : ( currently, not used )
 * 	CSR_OUT_OF_RESOURCE: Resources not released by this client
 *
 ***************************************************************************/
int
DeregisterClient( int * clientID, void * pointer, int argLen, void * argBuf )
{
    int w;
    
    if ( argLen != 0 )
	 return( CSR_BAD_ARG_LENGTH );

    /* 
     * check if the client have not released all requested resource
     *  Here only check the Memory Window Resource
     */
    if ( *clientID == NULL ){
	return( CSR_BAD_HANDLE );
    }
    for( w = 0; w < numWindows; w++ ){
	if ( windows[w]->ownerID == *clientID ){
	    return( CSR_IN_USE );
	}
    }
    /*
     * Call Cfg Event Handler for Deregister
     *   cfehDeregisterClient does the following validation
     *     - CSR_BAD_HANDLE
     *     - CSR_IN_USE
     */
    return( cfehDeregisterClient( *clientID ) );
}

/****************************************************************************
 *
 * NAME:	GetStatus
 *
 * FUNCTION:	Get the current status of a PC Card and its socket
 *
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	objectID : -	(not used) 
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSGetStatPkt
 *	getStat  : I/O	pointer of CSGetStatPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSGetStatPkt )
 *	CSR_BAD_SOCKET	   : Socket is invalid
 *
 ***************************************************************************/
int
GetStatus(int * ObjectID, void * Pointer, int argLen, CSGetStatPkt * getStat)
{
    GetStatus_S getstat;
    Adapter * adpt;
    Socket  * skt;
    int rc;

    if ( argLen < sizeof( CSGetStatPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( getStat->Socket, &skt ) ){
	return( rc );
    }
    adpt = &adapters[ skt->AdapterIndex ];

    lock_read( &skt->rw_lock );

    bzero( &getstat, sizeof( GetStatus_S ) );
    getstat.Socket = skt->PhySocket;
    if ( rc = SocketServices( adpt, PCM_GET_STATUS, (caddr_t)&getstat ) ){
	lock_done( &skt->rw_lock );
	return( rc );
    }
    getStat->CardState   = getstat.CardState;
    getStat->SocketState = getstat.SocketState;

    BUGLPR(var, BUGNTF, ("GetStatus adapter[%x].socket[%x]\n", \
			 skt->AdapterIndex, skt->PhySocket));

    lock_done( &skt->rw_lock );
    
    return( rc );
}

/****************************************************************************
 *
 * NAME:	GetCardServicesInfo
 *
 * FUNCTION:	Return the number of logical sockets installed and
 *		information about Card Services
 * EXECUTION ENVIRONMENT: client and user thread
 *                        with cs_cfg_lock=r 
 * ARGUMENTS:   
 *	olientID : -	(not used)
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSGetCSInfoPkt
 *	csInfo   : I	pointer of CSGetCSInfoPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than 8
 *      CSR_BAD_ARGS       : signature field is not null
 *
 ****************************************************************************/
#define CSVSTRLEN       38 /* sizeof( CSVENDORSTRING ) + 1;*/
int
GetCardServicesInfo( int * ObjectID, void * pointer,
			 int argLen,  CSGetCSInfoPkt *csinfo )
{
    CSGetCSInfoPkt defcsinfo = { sizeof( CSGetCSInfoPkt ) + CSVSTRLEN,
				     CSSIGNATURE, numSockets, 
				     CSREVISION, CSLEVEL, sizeof(int)*2,
				     CSVSTRLEN, NULL};
    char tmpstr[] = CSVENDORSTRING;

    if ( argLen < (sizeof(defcsinfo.InfoLen)+sizeof(defcsinfo.signature)))
	 return( CSR_BAD_ARG_LENGTH );
    if( argLen > defcsinfo.InfoLen ){
	argLen = defcsinfo.InfoLen;
    }
    if( csinfo->signature[0] || csinfo->signature[1] ){
	return( CSR_BAD_ARGS );

    }
    defcsinfo.InfoLen = argLen;
    memcpy( csinfo, &defcsinfo, argLen);
    
    if( (argLen -= sizeof( CSGetCSInfoPkt ) ) > 0){
	memcpy( &csinfo->VendorString[0], tmpstr, argLen );
	csinfo->VendorString[argLen] = NULL;
    }

    return( CSR_SUCCESS );
}

/****************************************************************************
 *
 * NAME:	ResetCard
 *
 * FUNCTION:	Reset the PC Card in the specifed socket
 *		  ( call cfehResetCard() )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENT:
 *	objectID : -	(not used) 
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSRstCardPkt
 *	rstCard  : I/O	pointer of CSRstCardPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSRstCardPkt )
 *	CSR_BAD_HANDLE	   : Client Handle does not match owning client
 *	CSR_BAD_SOCKET	   : Socket is invalid
 *	CSR_NO_CARD	   : No PC Card in socket
 *
 ****************************************************************************/
int
ResetCard( int * clientID, void * pointer, int argLen, CSRstCardPkt * rstCard )
{
    Socket * skt;
    int rc;

    /*
     * Validate the arguments
     */
    if ( argLen < sizeof( CSRstCardPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( rstCard->Socket, &skt ) )
	return( rc );
    if ( rc = cfehValidateClient( *clientID ) )
	return( rc );

    if ( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    lock_read( &skt->rw_lock );

    if ( skt->cfg_owner && *clientID != skt->cfg_owner ){
	lock_done( &skt->rw_lock );
	return( CSR_BAD_HANDLE );
    }
    /* 
     * 	Standard says the actual reset is done in the background
     * 	asynchronously.
     */
    rc = cfehResetCard( *clientID, rstCard->Socket );

    lock_done( &skt->rw_lock );

    return( rc );
}

/****************************************************************************
 *
 * NAME:	SetEventMask
 *
 * FUNCTION:	change the event mask for the client.
 *		  ( call cfehSetSlotMask )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	clientID : I	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSEvMaskPkt
 *	evMask   : I	pointer of CSEvMaskPkt
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_ARG_LENGTH      : argLen is less than sizeof( CSEvMsskPkt )
 *	CSR_BAD_HANDLE		: ClientHandle is incalid
 *	CSR_BAD_SOCKET	        : Socket is invalid (Socket request only) or
 *		this socket has not been requested via RequestSocketMask
 *	CSR_NO_CARD		: No PC Card in socket
 *
 ****************************************************************************/
int
SetEventMask( int * clientID, void * pointer, int argLen, CSEvMaskPkt * evMask)
{
    Socket * skt;
    int rc;

    /*
     * Validate the arguments
     */
    if ( argLen < sizeof( CSEvMaskPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( evMask->Socket, &skt ) ){
	return( rc );
    }
    if ( evMask->Attributes & ~CSEvMaskThisSocket ){
	return( CSR_BAD_ATTRIBUTE );
    }

    if ( evMask->Attributes ){
	if ( !CheckCardPresence( skt )){
	    return( CSR_NO_CARD );
	}
    }
    rc = cfehSetSlotMask( *clientID, 
			 evMask->Attributes ? evMask->Socket : NO_SLOT,
			  evMask->EventMask );

    return( rc );
}    

/****************************************************************************
 *
 * NAME:	GetEventMask
 *
 * FUNCTION:	get event mask for the client.
 *		  ( call cfehGetSlotMask() )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	clientID : I	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSEvMaskPkt
 *	evMask   : I/O	pointer of CSEvMaskPkt
 * RETURNS:
 * 	CSR_SUCCESS        : Success	
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSEvMaskPkt )
 *	CSR_BAD_HANDLE	   : ClientHandle is invalid
 *	CSR_BAD_SOCKET	   : Socket is invalid
 *	CSR_NO_CARD	   : No PC Card in socket
 *
 ****************************************************************************/
int
GetEventMask( int * clientID, void * pointer,
		 int argLen, CSEvMaskPkt * evMask )
{
    Socket *skt;
    int rc;

    /*
     * Validate the arguments
     */
    if ( argLen < sizeof( CSEvMaskPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( evMask->Socket, &skt ) ){
	return( rc );
    }
    if ( evMask->Attributes & ~CSEvMaskThisSocket ){
	return( CSR_BAD_ATTRIBUTE );
    }
    if ( evMask->Attributes ){
	if( !CheckCardPresence( skt )){
	    return( CSR_NO_CARD );
	}
    }
    rc = cfehGetSlotMask( *clientID,
			 evMask->Attributes ? evMask->Socket : NO_SLOT,
			 &evMask->EventMask );

    return( rc );
}

/*
 * Resource Management Functions
 */

/****************************************************************************
 *
 * NAME:	RequestIO
 *
 * FUNCTION:	Request I/O Window for the specified socket.
 *                Client can request two I/O windows ( Port1 and Port2 )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	clientID : pointer of client handle
 *	pointer  : (not used)
 *	argLen   : length of packet CSIOPkt
 *	reqIO    : pointer of CSIOPkt
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_ARG_LENGTH      : argLen is less than sizeof( CSIOPkt )
 *	CSR_BAD_ATTRIBUTE       : Sharing or alias request invalid
 *	CSR_BAD_BASE	        : Base port address is invalid
 *	CSR_BAD_HANDLE	        : Client Handle is invalid
 *	CSR_BAD_SOCKET	        : Socket is invalid
 *	CSR_CONFIGURATION_LOCKED: Configuration already set
 *	CSR_IN_USE		: I/O ports requested are already in-use or
 *			 function has already been successfully invoked	
 *      CSR_BAD_ARGS            : IO Address line is not 0x10
 *	CSR_NO_CARD		: No PC Card in socket
 *	CSR_OUT_OF_RESOURCE     : Internal data space exhausted
 *
 ****************************************************************************/
static int
RequestIO( int * clientID, void * pointer, int argLen, CSIOPkt * reqIO )
{
    SetWindow_S setwin;
    CSGetCfgInfoPkt * cfgInfo;
    Window  * win;
    Socket  * skt;
    Adapter * adpt;
    int attr1, attr2;
    int w;
    int attr = PCM_WC_IO;
    int rc;
    
    if ( argLen < sizeof( CSIOPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if (reqIO->Attributes1 & (CSIOATTRRSV|CSIOSHARED|CSIOFIRSTSHARED) ||
	reqIO->Attributes2 & (CSIOATTRRSV|CSIOSHARED|CSIOFIRSTSHARED)){
	return( CSR_BAD_ATTRIBUTE );
	
    }
    if ( rc = ValidateSocket( reqIO->Socket, &skt ) ){
	return( rc );
    }

    if( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    lock_write( &skt->rw_lock );

    while( tstop( skt->pwrofft ) );

    adpt = &adapters[skt->AdapterIndex];
    
    if ( skt->cfg_owner){
	rc = CSR_CONFIGURATION_LOCKED;
	goto end;
    }
    /* AddressLine must be 0x10 */
    if ( reqIO->IOAddrLines != 0x10 ){ 
	rc = CSR_BAD_ATTRIBUTE;
	goto end;
    }

    simple_lock( &adpt->win_lock );
    if (skt->io_owner ){
	rc = CSR_IN_USE;
	simple_unlock( &adpt->win_lock );
	goto end;
    }
    if( ( rc = cfehClientSetSlot( *clientID, reqIO->Socket ) )
       != CSR_SUCCESS ){
	simple_unlock( &adpt->win_lock );
	goto end;
    }

    if ( reqIO->BasePort1 && reqIO->NumPorts1 ){
        /* 
         * Assign a IO window for the specified socket
         */
	setwin.Socket = skt->PhySocket;
	setwin.Size   = reqIO->NumPorts1;
	setwin.State  = PCM_WS_IO | PCM_WS_ENABLED |
	    (( reqIO->Attributes1 & CSIO16BITS ) ? PCM_WS_16BIT : 0 );
	setwin.Speed  = 0; /* not valid for IO window */
	setwin.Base   = ( ushort ) reqIO->BasePort1;
	    
	if ( rc = AssignWindow( adpt, PCM_WC_IO, (ulong) reqIO->BasePort1,
			       reqIO->NumPorts1, &setwin ) ){
	    simple_unlock( &adpt->win_lock );
	    goto end;
	}
	rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)&setwin );
	if ( rc ) goto end;
	skt->io_window1 = windows[setwin.Window];
	windows[setwin.Window]->LogSocket = reqIO->Socket;
	windows[setwin.Window]->ownerID = * clientID;
	windows[setwin.Window]->setWin.State = setwin.State;
    }else{
	rc = CSR_BAD_BASE;
	simple_unlock( &adpt->win_lock );
	goto end;
    }
	
    if ( reqIO->BasePort2 && reqIO->NumPorts2 ){
	setwin.Window = w;
	setwin.Socket = skt->PhySocket;
	setwin.Size   = reqIO->NumPorts2;
	setwin.State  = PCM_WS_IO | PCM_WS_ENABLED |
	    (( reqIO->Attributes2 & CSIO16BITS ) ? PCM_WS_16BIT : 0 );
	setwin.Speed  = 0; /* not valid for IO window */
	setwin.Base   = ( ushort ) reqIO->BasePort2;
	    
	if ( rc = AssignWindow( adpt, PCM_WC_IO, (ulong) reqIO->BasePort2,
			       reqIO->NumPorts2, &setwin ) ){
	    skt->io_window1 = 0;
	    windows[setwin.Window]->ownerID = 0;
	    simple_unlock( &adpt->win_lock );
	    goto end;
	}
	rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)&setwin );
	if ( rc ) goto end;
	skt->io_window2 = windows[setwin.Window];
	windows[setwin.Window]->LogSocket = reqIO->Socket;
	windows[setwin.Window]->ownerID = * clientID;
	windows[setwin.Window]->setWin.State = setwin.State;
    }
    
    cfgInfo = &skt->cfgInfo;
    cfgInfo->BasePort1   = reqIO->BasePort1;
    cfgInfo->NumPorts1   = reqIO->NumPorts1;
    cfgInfo->Attributes1 = reqIO->Attributes1;
    cfgInfo->BasePort2   = reqIO->BasePort2;
    cfgInfo->NumPorts2   = reqIO->NumPorts2;
    cfgInfo->Attributes2 = reqIO->Attributes2;
    cfgInfo->IOAddrLines = reqIO->IOAddrLines;

    skt->io_owner = *clientID;
    simple_unlock( &adpt->win_lock );
 end:
    set_activity( skt );
    lock_done( &skt->rw_lock );

    return( rc );
} 

/****************************************************************************
 *
 * NAME:	ReleaseIO
 *
 * FUNCTION:	Release previously requested I/O address and windows.
 *
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	clientID : I	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSIOPkt
 *	relIO    : I	pointer of CSIOPkt
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_ARG_LENGTH      : argLen is less than sizeof( CSIOPkt )
 *	CSR_BAD_ARGS	        : I/O description doesn't match allocation
 *	CSR_BAD_HANDLE	        : ClientHandle does not match owning client
 *				  or no I/O ports to release.
 *	CSR_BAD_SOCKET	        : Socket is invalid
 *	CSR_CONFIGURATION_LOCKED: Configuration has not been released
 *
 ****************************************************************************/
static int
ReleaseIO( int * clientID, void	* Pointer, int argLen, CSIOPkt * relIO )
{
    SetWindow_S setWin;
    Window  * win;
    Socket  * skt;
    Adapter * adpt;
    int w;
    int rc;

    /*
     * Validate the argument
     */
    if ( argLen < sizeof( CSIOPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( relIO->Socket, &skt ) ){
	return( rc );
    }
    
    lock_write( &skt->rw_lock );

    /* check whether the clientID is valid or not */
    /* also check if the client is an owner of the IO window */

    if ( !*clientID || * clientID != skt->io_owner ){
	rc = CSR_BAD_HANDLE;
	goto end;
    }
    if ( skt->cfg_owner != 0){
	rc = CSR_CONFIGURATION_LOCKED;
	goto end; 
    }

    if ( skt->cfgInfo.BasePort1   != relIO->BasePort1   ||
	 skt->cfgInfo.NumPorts1   != relIO->NumPorts1   ||
	/* skt->cfgInfo.Attributes1 != relIO->Attributes1 || */
         skt->cfgInfo.BasePort2   != relIO->BasePort2   ||
	 skt->cfgInfo.NumPorts2   != relIO->NumPorts2   ||
	/* skt->cfgInfo.Attributes2 != relIO->Attributes2 || */
	 skt->cfgInfo.IOAddrLines != relIO->IOAddrLines ){
	rc = CSR_BAD_ARGS;
	goto end;
    }

    adpt = &adapters[skt->AdapterIndex];
    if( ( rc = cfehClientResetSlot( *clientID, relIO->Socket ) ) 
       != CSR_SUCCESS ){
	goto end;
    }

    simple_lock( &adpt->win_lock );
    if ( win = skt->io_window1 ){
	setWin.Window = win->PhyWindow;
	setWin.Socket = skt->PhySocket;
	if((rc = GetWindow( adpt, &setWin, NULL )) ||
	   (rc = FreeWindow( adpt, &setWin, NULL ))){
	    simple_unlock( &adpt->win_lock );
	    goto end;
	}
	win->ownerID = 0;
	skt->io_window1 = 0;
	skt->io_owner = 0;
	win->setWin.State = 0;
    }
    
    if ( win = skt->io_window2 ){
	setWin.Window = win->PhyWindow;
	setWin.Socket = skt->PhySocket;
	if((rc = GetWindow( adpt, &setWin, NULL ) ) ||
	   (rc = FreeWindow( adpt, &setWin, NULL ))){
	    simple_unlock( &adpt->win_lock );
	    goto end;
	}
	win->ownerID = 0;
	skt->io_window2 = 0;
	win->setWin.State = 0;
    }

    if ( !skt->io_window1 ){
	skt->cfgInfo.BasePort1   = 0;
	skt->cfgInfo.NumPorts1   = 0;
	skt->cfgInfo.Attributes1 = 0;
	skt->cfgInfo.BasePort2   = 0;
	skt->cfgInfo.NumPorts2   = 0;
	skt->cfgInfo.Attributes2 = 0;
	skt->cfgInfo.IOAddrLines = 0;
    }
    simple_unlock( &adpt->win_lock );

 end:
    set_activity( skt );
    if ( !CheckInterest( skt ) ){
	PowerOffSocket( skt->LogSocket );
    }
    lock_done( &skt->rw_lock );

    return( rc );
} 

/*****************************************************************************
 *
 * NAME:	RequestIRQ
 *
 * FUNCTION:	Request an interrupt request line.
 * 
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	clientID : I	pointer of client handle
 *	pointer  :	(not used)
 *	argLen   : I	length of packet CSReqIRQPkt
 *	reqIRQ   : I	pointer of CSReqIRQPkt
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_ARG_LENGTH	: ArgLen is less than sizeof( CSReqIRQPkt )
 *	CSR_BAD_ARGS	        : IRQ info fields are invalid
 *	CSR_BAD_ATTRIBUTE	: Sharing or pulse request invalid 
 *	CSR_BAD_HANDLE	        : ClientHandle is invalid 
 *					(returned by Event Handler)
 *	CSR_BAD_IRQ		: IRQ is invalid (returned by SocketServices)
 *	CSR_BAD_SOCKET	        : Socket is invalid
 *	CSR_CONFIGURATION_LOCKED: Configuration already set
 *	CSR_IN_USE		: IRQ requested is already in-use of function
 *				  has already been successfully invoked
 *	CSR_NO_CARD		: No PC Card in socket
 *
 *****************************************************************************/
static int 
RequestIRQ( int *clientID, void * pointer, int argLen, CSReqIRQPkt * reqIRQ )
{
    SetSocket_S setskt;
    Socket * skt;
    int i = 0;
    uint attr, irq;
    int rc;

    if ( argLen < sizeof( CSReqIRQPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( reqIRQ->Socket, &skt ) ){ 
	return( rc );
    }
    if ( rc = cfehValidateClient( *clientID )){
	return( rc );
    }
    if ( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    lock_write( &skt->rw_lock );

    while( tstop( skt->pwrofft ) );

    /* 
     * The following bits are not controlled by CardServices,
     * CSIRQExcl CSIRQTmShr CSIRQDyShr CSIRQFstShr
     * busresolve must have resolved it and Clients should controll them. 
     */
    if (reqIRQ->Attributes & (CSIRQFrcPulse|CSIRQPulseAlc) ||
	(reqIRQ->Attributes & (CSIRQTmShr|CSIRQDyShr)) 
	== (CSIRQTmShr|CSIRQDyShr)){
	rc = CSR_BAD_ATTRIBUTE;
	goto end;
    }
    if ( ( reqIRQ->IRQInfo1 & (CSIRQNMI|CSIRQIOCK|CSIRQBERR|CSIRQVEND) ) ||
        !( reqIRQ->IRQInfo1 & CSIRQInfo2Used ) ){
	rc = CSR_BAD_IRQ;
	goto end;
    }
    if ( (reqIRQ->IRQInfo1 & (CSIRQLevel)) !=
	 ( adapters[skt->AdapterIndex].int_mode == 'I' ? CSIRQLevel : 0 ) ){
	BUGLPR(var, BUGACT, ("reqIRQ is rejected  type = %c and L=%x\n",\
	adapters[skt->AdapterIndex].int_mode, reqIRQ->IRQInfo1 & CSIRQLevel ));
	rc = CSR_BAD_IRQ;
	goto end;
    }
    /*
     * Only one bit can be set, otherwise, return with BAD_ARGS 
     */
    if ((irq = (reqIRQ->IRQInfo2 &= 0xffff)) != 0)
	for(; !(irq & 1) && i < 16; i++, irq >>= 1);
    /* Client should set only one of the bit */
    if ( irq != 1 || i == 16 ){
	rc = CSR_BAD_IRQ;
	goto end;
    }
    if ( skt->cfg_owner ){
	rc = CSR_CONFIGURATION_LOCKED;
	goto end; 
    }
    if ( skt->irq_owner ){
	rc = CSR_IN_USE;
	goto end;
    }
    if( ( rc = cfehClientSetSlot( *clientID, reqIRQ->Socket ) ) != CSR_SUCCESS ){
	BUGLPR(var, BUGACT,("cfehClientSetSlot Fail\n"));
	goto end;
    }

    /*
     * the assigned IRQ is requested to SocketServices 
     * when the RequestConfiguration is called.
     */
    reqIRQ->AssignedIRQ = i;
    skt->cfgInfo.AssignedIRQ = i;
    skt->cfgInfo.IRQAttributes = reqIRQ->Attributes;
    skt->irq_owner = *clientID;

 end:
    set_activity( skt );
    lock_done( &skt->rw_lock );

    return( rc );
} 

/****************************************************************************
 *
 * NAME:	ReleaseIRQ
 *
 * FUNCTION:	release a previously requested interrupt request line.
 *
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	clientID : pointer of client handle
 *	pointer  : (not used)
 *	argLen   : length of packet CSRelIRQPkt
 *	relIRQ   : pointer of CSRelIRQPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSRelIRQPkt )
 *	CSR_BAD_ATTRIBUTE  : Attributes don't match allocation
 *	CSR_BAD_IRQ        : AssignedIRQ doesn't match allocation
 *	CSR_BAD_HANDLE	   : ClientHandle does not match owning client
 *				or no IRQ to release
 *	CSR_BAD_SOCKET	   : Socket is invalid
 *	CSR_CONFIGURATION_LOCKED: Configuration has not been released
 *
 ****************************************************************************/
static int
ReleaseIRQ( int * clientID, void * pointer, int argLen, CSRelIRQPkt * relIRQ )
{
    Socket *skt;
    int rc;

    /*
     * Validate the Arguments
     */
    if ( argLen < sizeof( CSRelIRQPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( relIRQ->Socket, &skt ) ){
	return( rc );
    }
    lock_write( &skt->rw_lock );

    if( !*clientID || skt->irq_owner != *clientID ){
	rc = CSR_BAD_HANDLE;
	goto end;
    }
    if( skt->cfg_owner ){
	rc = CSR_CONFIGURATION_LOCKED;
	goto end;
    }
    if( skt->cfgInfo.IRQAttributes != relIRQ->Attributes ){
	rc = CSR_BAD_ATTRIBUTE;
	goto end;
    }
    if( skt->cfgInfo.AssignedIRQ != relIRQ->AssignedIRQ ){
	rc = CSR_BAD_IRQ;
	goto end; 
    }
    if( ( rc = cfehClientResetSlot( *clientID, relIRQ->Socket ) ) 
       != CSR_SUCCESS ){
	goto end;
    }

    skt->cfgInfo.AssignedIRQ = 0xff;
    skt->cfgInfo.IRQAttributes = 0x0;

    skt->irq_owner = 0;

 end:
    set_activity( skt );
    if ( !CheckInterest( skt ) ){
	PowerOffSocket( skt->LogSocket );
    }
    lock_done( &skt->rw_lock );


    return( rc );
} 

/****************************************************************************
 *
 * NAME:	RequestWindow
 *
 * FUNCTION:	request a block of system memory space be assigned to
 *		a memory region of a P Card in a socket. 
 *
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	objectID : I/O  pointer of client/window handle
 *	pointer  :      (not used)
 *	argLen   : I    length of packet CSReqWinPkt
 *	reqWin   : I    pointer of CSReqWinPkt
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_ARG_LENGTH      : argLen is less than sizeof( CSReqWinPkt )
 *	CSR_BAD_ATTRIBUTE	: Data width or paging invalid or
 *					sharing conflicts
 *	CSR_BAD_BASE		: System memory address invalid
 *	CSR_BAD_HANDLE		: ClientHandle is incalid
 *	CSR_BAD_SIZE		: Window size is invalid
 *	CSR_BAD_SOCKET		: Socket is invalid
 *	CSR_BAD_SPEED		: Speed not supported
 *	CSR_NO_CARD		: No PC Card in socket
 *	CSR_OUT_OF_RESOURCE     : internal data space is exhausted
 *	CSR_IN_USE		: Window requested in in-use
 *
 ****************************************************************************/
/* the Bit 4 (: Paged) is reset to zero */
static int
RequestWindow(int * objectID, void * pointer, int argLen, CSReqWinPkt * reqWin)
{
    SetPage_S   setPage;
    SetWindow_S setWin;
    Window * win;
    Socket * skt;
    Adapter * adpt;
    int w, rc;
    int reqGran;
    int attr = ( reqWin->Attributes & CSWinAttr ) 
	? PCM_WC_ATTRIBUTE : PCM_WC_COMMON;
    MEMWINTBL * tbl;

    /*
     * Validate the arguments
     */
    if ( argLen < sizeof( CSReqWinPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if( rc = ValidateSocket( reqWin->Socket, &skt ) ){
	return( rc );
    }
    lock_write( &skt->rw_lock );

    while( tstop( skt->pwrofft ) );

    if ( rc = cfehValidateClient( *objectID ) ){
	goto end;
    }
    if ( !CheckCardPresence( skt ) ){
	rc = CSR_NO_CARD;
	goto end;
    }
    adpt = &adapters[skt->AdapterIndex];

    if ( ! reqWin->Base ){
	rc = CSR_BAD_BASE;
	goto end;
    }
    if ( ! reqWin->Size ){
	rc = CSR_BAD_SIZE;
	goto end;
    }
    if ( reqWin->Attributes & (CSWinRSV|CSWinFShared|CSWinShared) ){
	rc = CSR_BAD_ATTRIBUTE;
	goto end;
    }
    if ( !reqWin->AccessSpeed ){
	rc = CSR_BAD_SPEED;
	goto end;
    }

    simple_lock( &adpt->win_lock );
    for( w = 0; w < numWindows; w++ ){
	if(windows[w]->Base == reqWin->Base ){
	    rc = CSR_IN_USE;
	    break;
	}
    }
    if ( rc ){
	simple_unlock( &adpt->win_lock );
	goto end;
    }

    /* 
     * assign a mem window for the specified socket 
     */
    setPage.Page   = 0; 
    setPage.State  = 
	(( reqWin->Attributes & CSWinAttr   ) ? PCM_PS_ATTRIBUTE : 0) |
	(( reqWin->Attributes & CSWinEnable ) ? PCM_PS_ENABLED   : 0) ;
    setPage.Offset = 0;
    setWin.Socket = skt->PhySocket;
    setWin.Size   = reqWin->Size;
    setWin.State  = 
	(( reqWin->Attributes & CSWinEnable ) ? PCM_WS_ENABLED : 0) |
	(( reqWin->Attributes & CSWinPaged  ) ? PCM_WS_PAGED   : 0) |
	(( reqWin->Attributes & CSWin16Bits ) ? PCM_WS_16BIT   : 0) ;
    setWin.Speed  = reqWin->AccessSpeed & 0x7f;
    setWin.Base   = ( ulong )reqWin->Base;

    if ((rc = AssignWindow( adpt, ((reqWin->Attributes & CSWinAttr)
				   ? PCM_WC_ATTRIBUTE : PCM_WC_COMMON),
			   (ulong) reqWin->Base, reqWin->Size,
			   &setWin )) != CSR_SUCCESS){
	simple_unlock( &adpt->win_lock );
	goto end;
    }
    windows[setWin.Window]->LogSocket = reqWin->Socket;
    setPage.Window = w = setWin.Window;

    /* Call Socket Services to Set Window */
    rc = SetWindow( adpt, &setWin, &setPage );
    if ( reqWin->Attributes & CSWinEnable ){
	rc |= EnableWindow( adpt, &setWin, &setPage );
    }
    if ( rc ){
	simple_unlock( &adpt->win_lock );
	goto end;
    }
    /* If it is true, Page offset must be on Page Size boundaries */ 
    if ( windows[w]->WinCaps & PCM_WC_CALIGN ){
	reqWin->Attributes |= CSWinOffset;
    }
    windows[w]->ownerID = * objectID;
    windows[w]->LogSocket = reqWin->Socket;
    windows[w]->Base = reqWin->Base;
    windows[w]->Size = reqWin->Size;
    windows[w]->Attributes = reqWin->Attributes;
    windows[w]->setWin.State = setWin.State;
    *objectID = (int) windows[w];
    simple_unlock( &adpt->win_lock );

 end:
    set_activity( skt );
    lock_done( &skt->rw_lock );

    return( rc );
} 

/****************************************************************************
 *
 * NAME:	ReleaseWindow
 *
 * FUNCTION:	Release a block of system memory space which was
 *		obtained previously by a corresponding RequestWindow.
 * NOTE:	CardServices assumes only the owning client will make
 *		this request. ( No clientID validation! )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	windowID : I	pointer of window handle
 *	pointer  : -	(not used)
 *	argLen   : I	must be zero (0)
 *	argBuf   : I	(not used)
 * RETURNS:
 *	CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is not equal to zero
 *	CSR_BAD_HANDLE     : WindowHandle is invalid
 *
 ****************************************************************************/
static int
ReleaseWindow( int * windowID, void * pointer, int argLen, void * argBuf )
{
    SetPage_S   setPage;
    SetWindow_S setWin;
    Window * win = (Window *) *windowID;
    Socket * skt;
    Adapter * adpt;
    int s, rc;

    /* Validate the argument */

    if ( argLen != 0 )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateWindow( &win ) )
	return( rc );
    adpt = &adapters[win->AdapterIndex];

    simple_lock( &adpt->win_lock );
    if ( !win->ownerID ){
	rc = CSR_BAD_HANDLE;
	simple_unlock( &adpt->win_lock );
	goto end;
    }
    setWin.Window  = win->PhyWindow;
    setWin.Socket  = sockets[win->LogSocket]->PhySocket;
    setPage.Window = win->PhyWindow;
    setPage.Page   = 0;
    if ((rc = GetWindow( adpt, &setWin, &setPage )) ||
	(rc = FreeWindow( adpt, &setWin, &setPage))){
	simple_unlock( &adpt->win_lock );
	goto end;
    }
    win->ownerID = 0;
    win->Base = 0;
    win->Size = 0;
    win->Attributes = 0;
    win->setWin.State = 0;
    simple_unlock( &adpt->win_lock );

    skt = sockets[win->LogSocket];

    lock_write( &skt->rw_lock );

    set_activity( skt );
    if ( !CheckInterest( skt ) ){
	PowerOffSocket( skt->LogSocket );
    }

    lock_done( &skt->rw_lock );  /* write_lock */

 end:
    return( rc );
} 

/****************************************************************************
 *
 * NAME:	ModifyWindow
 *
 * FUNCTION:	Modify the attributes or access speed of a window
 *		previously allocaated with the RequestWindow function.
 * NOTE: 	( Currently, there is no client validation)
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	windowID : I	pointer of window handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSModWinPkt
 *	modWin   : I	pointer of CSModWinPkt
 * RETURNS:
 *      CSR_SUCCESS       : Success
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSReqCfgPkt )
 *	CSR_NO_CARD       : No PC Card in socket
 *	CSR_BAD_ATTRIBUTE : Attributes are invalid 
 *				or window cannot enabled/disabled
 *	CSR_BAD_HANDLE    : Window Handle is invalid
 *	CSR_BAD_SPEED     : Speed is invalid
 *
 ****************************************************************************/
static int
ModifyWindow( int * windowID, void * pointer,
		int argLen, CSModWinPkt * modWin )
{
    SetPage_S   setPage;
    SetWindow_S setWin;
    Window * win = (Window *) * windowID;
    Adapter * adpt;
    int Speed;
    int s, rc;

    if ( argLen < sizeof( CSModWinPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateWindow( &win ) ){
	return( rc );
    }
    adpt = &adapters[win->AdapterIndex];

    if ( !win->ownerID ){
	rc = CSR_BAD_HANDLE;
	goto end;
    }
    if(!CheckCardPresence( sockets[win->LogSocket] ) ){
	rc = CSR_NO_CARD;
	goto end;
    }
    if ( modWin->Attributes & ~(CSModWinAttMem|CSModWinEnable|CSModWinAccSpd)){
	rc = CSR_BAD_ATTRIBUTE;
	goto end;
    }
    if ((modWin->Attributes & CSModWinAccSpd) && 
	!(modWin->AccessSpeed & CSWinWait )){
	if((Speed = ConvertSpeed( modWin->AccessSpeed ) ) == 0 ||
	    Speed > ConvertSpeed( win->WinTable[0].MemWinTbl.Slowest ) ||
	    Speed < ConvertSpeed( win->WinTable[0].MemWinTbl.Fastest ) ){
	    rc = CSR_BAD_SPEED;
	    goto end;
	}
    }
    setWin.Window = win->PhyWindow;
    setWin.Socket = sockets[win->LogSocket]->PhySocket;
    setPage.Window = win->PhyWindow;
    setPage.Page = 0;

    simple_lock( &adpt->win_lock );
    if ((rc = GetWindow( &adapters[win->AdapterIndex], &setWin, &setPage ))||
        (rc = DisableWindow( &adapters[win->AdapterIndex], &setWin, &setPage ))){
	simple_unlock( &adpt->win_lock );
	goto end;
    }
    if ( modWin->Attributes & CSModWinAttMem ){
	setPage.State |= PCM_PS_ATTRIBUTE;
    }
    else{
	setPage.State &= ~PCM_PS_ATTRIBUTE;
    }
    
    if ( modWin->Attributes & CSModWinAccSpd ){
	if ( modWin->AccessSpeed & CSWinWait ){
	    setWin.Speed = win->WinTable[0].MemWinTbl.Slowest;
	}
	else{
	    setWin.Speed = modWin->AccessSpeed & 0x7f;
	}
    }
    if ( rc = SetWindow( &adapters[win->AdapterIndex], &setWin, &setPage )){
	simple_unlock( &adpt->win_lock );
	goto end;
    }
    if ( modWin->Attributes & CSModWinEnable ){
        if ( rc = EnableWindow( &adapters[win->AdapterIndex],
			       &setWin, &setPage )){
	    simple_unlock( &adpt->win_lock );
	    goto end;
	}
    }

    win->setWin.State = setWin.State;
    simple_unlock( &adpt->win_lock );

 end:
    return( rc );
} 

/****************************************************************************
 *
 * NAME:	MapMemPage
 *
 * FUNCTION:	Select the memory area on a PC Card into a page of a
 *		window allocated with the RequestWindow function. 
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	windowID : I	pointer of Window Handle 
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSMapMemPagePkt
 *	mapPage  : I/O	pointer of CSMapMemPagePkt
 * RETURNS:
 *      CSR_SUCCESS       : Success
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSMapMemPagePkt )
 *	CSR_BAD_HANDLE	  : Window Handle is invalid
 *	CSR_BAD_OFFSET	  : Offset is invalid
 *	CSR_BAD_PAGE	  : Page is invalid
 *	CSR_NO_CARD	  : No card in socket
 *
 ****************************************************************************/
/*
 * System Memory Space	 PC Card Memory Space
 *    --+--	 		      --+--		      
 * 	    |	         	Offset	|
 *	    + - - - - - - =+= - - - - ->+
 *	    |   Window	   |		|
 *	    |   Space 	   |		|
 *	    + - - - - - - =+= - - - - ->+
 *	    |		Window		|
 *	    V				V
 */
static int
MapMemPage( int * windowID, void * pointer,
		 int argLen, CSMapMemPagePkt * mapPage )
{
    SetPage_S   setPage;
    SetWindow_S setWin;
    Adapter * adpt;
    Window * win = (Window *) * windowID;
    Socket * skt;
    int s, rc;
    ulong offset;

    if ( argLen < sizeof( CSMapMemPagePkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if( rc = ValidateWindow( &win ) ){
	return( rc );
    }
    if ( !win->ownerID ){
	return( CSR_BAD_HANDLE );
    }
    if ( rc = ValidateSocket( win->LogSocket, &skt ) ){
	return( rc );
    }

    if ( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    if( mapPage->CardOffset % win->WinTable[0].MemWinTbl.ReqGran ){
	return( CSR_BAD_OFFSET );
    }

    /* 
     * if the offset bit is set as the returned attribute of RequestWindow
     *       (mapPage->CardOffset % win->size )
     */
    if( mapPage->Page ){
	return( CSR_BAD_PAGE );
    }

    adpt = &adapters[ win->AdapterIndex ];
    
    setWin.Window = win->PhyWindow;
    setPage.Window = win->PhyWindow;
    setPage.Page = mapPage->Page;

    simple_lock( &adpt->win_lock );
    GetWindow( adpt, &setWin, &setPage );
    DisableWindow( adpt, &setWin, &setPage );

    setPage.Offset = mapPage->CardOffset / win->WinTable[0].MemWinTbl.ReqGran;
    
    SetWindow( adpt, &setWin, &setPage );
    EnableWindow( adpt, &setWin, &setPage );
    simple_unlock( &adpt->win_lock );

    return( rc );
} 

/****************************************************************************
 *
 * NAME:	RequestSocketMask
 *
 * FUNCTION:	request that the client be notified of status changes
 *		for this socket.
 *		  ( call cfehRequestSlotMask() )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	clientID : I	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSReqSockMPkt
 *	reqSktM  : I	pointer of CSReqSockMPkt
 * RETURNS:
 *      CSR_SUCCESS       : Success
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSReqSockMPkt )
 *	CSR_BAD_HANDLE	  : Client Handle is invalid
 *	CSR_BAD_SOCKET	  : Socket is invalid
 *	CSR_NO_CARD	  : No PC Card in socket
 *
 ****************************************************************************/
int
RequestSocketMask( int * clientID, void * Pointer,
			int argLen, CSReqSockMPkt * reqSktM )
{
    Socket * skt;
    int rc;

    if ( argLen < sizeof( CSReqSockMPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( reqSktM->Socket, &skt ) ){
	return( rc );
    }
    if ( !CheckCardPresence( skt ) ){
	return ( CSR_NO_CARD );
    }

    rc = cfehRequestSlotMask( *clientID, reqSktM->Socket, reqSktM->EventMask );
    if ( !rc ){
	if ( rc = cfehClientSetSlot( *clientID, reqSktM->Socket ) ){
	    cfehReleaseSlotMask( *clientID, reqSktM->Socket );
	}
    }

    return( rc );
} 

/****************************************************************************
 *
 * NAME:	ReleaseSocketMask
 *
 * FUNCTION:	Request that the client no longer be notified of
 *		status changes for this socket
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	clientID : I	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSRelSockMPkt
 *	relSktM  : I	pointer of CSRelSockMPkt
 * RETURNS:
 *      CSR_SUCCESS       : Success
 *	CSR_BAD_ARG_LENGTH: ArgLen is less than sizeof( CSRelSockMPkt )
 *	CSR_BAD_HANDLE    : ClientHandle does not match owning client
 *				or no socket mask to release
 *	CSR_BAD_SOCKET    : Socket is invalid
 *
 ***************************************************************************/
int
ReleaseSocketMask( int * clientID, void * pointer,
			int argLen, CSRelSockMPkt * relSktM )
{
    Socket * skt;
    int rc;

    if ( argLen < sizeof( CSRelSockMPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( relSktM->Socket, &skt ) ){
	return( rc );
    }

    rc = cfehReleaseSlotMask( *clientID, relSktM->Socket );
    if ( rc == CSR_SUCCESS ){
	rc = cfehClientResetSlot( *clientID, relSktM->Socket );
    }

    return( rc );
} 

/****************************************************************************
 *
 * NAME:	RequestConfiguration
 *
 * FUNCTION:	Configure the PC Card and socket.
 *
 * NOTE:	After finding an appropriate configuration with
 *		RequestIO() and RequestIRQ(), RequestConfiguration()
 *		establishes the configuration in the socket adapter
 *		and PC Card.  	
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	clientID : I	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSReqCfgPkt
 *	reqCfg   : I	pointer of  CSReqCfgPkt
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_ARG_LENGTH      : argLen is less than sizeof( CSReqCfgPkt )
 *	CSR_ATTRIBUTE           : IRQ steering enable conflict
 *	CSR_BAD_HANDLE          : ClientHandle is invalid
 *	CSR_BAD_TYPE            : I/O and memory interface not supported
 *	CSR_BAD_SOCKET          : Socket is invalid
 *	CSR_BAD_VCC             : Requested Vcc is not avaliable on socket 
 *	CSR_BAD_VPP             : Requested Vpp is not avaliable on socket 
 *	CSR_CONFIGURATION_LOCKED: Configuration already set 
 *	CSR_IN_USE              : No PC Card in socket 
 *	CSR_NO_CARD             : PC Card already being used
 *
 ****************************************************************************/
static int
RequestConfiguration( int * clientID, void * pointer,
			 int argLen, CSReqCfgPkt * reqCfg )
{
    SetSocket_S setskt;
    caddr_t addr;
    ulong base_offset, page_offset;
    int size, gran;
    Socket * skt;
    Window * win;
    Adapter * adpt;
    int p;
    int Vcc, Vpp1, Vpp2;
    int rc;

    if ( argLen < sizeof( CSReqCfgPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( reqCfg->Socket, &skt ) ){
	return( rc );
    }
    adpt = &adapters[skt->AdapterIndex];
    if ( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    lock_write( &skt->rw_lock );

    while( tstop( skt->pwrofft ) );

    if ( ( reqCfg->Attributes & CSCfgIRQSteer ) && 
	 ( skt->cfgInfo.AssignedIRQ == 0xff )){
	rc = CSR_BAD_ATTRIBUTE;
	goto end;
    }
    if ( skt->cfg_owner ){
	rc = CSR_CONFIGURATION_LOCKED;
	goto end;
    }
    if( ( Vcc  = ConvertPower( adpt, reqCfg->Vcc,  PCM_VCC  ) ) == -1){
	rc = CSR_BAD_VCC;
	goto end;
    }
    if( ( Vpp1 = ConvertPower( adpt, reqCfg->Vpp1, PCM_VPP1 ) ) == -1 ||
        ( Vpp2 = ConvertPower( adpt, reqCfg->Vpp2, PCM_VPP2 ) ) == -1 ){
	rc = CSR_BAD_VPP;
	goto end;
    }

    if( ( rc = cfehClientSetSlot( *clientID, reqCfg->Socket ) ) 
       != CSR_SUCCESS ){
	goto end;
    }

    setskt.Socket      = skt->PhySocket;
    rc = SocketServices( adpt, PCM_GET_SOCKET, (caddr_t)&setskt );
    if ( rc ) goto end;
    setskt.VccLevel     = 0xff & Vcc;
    setskt.VppLevels    = (0xf0 & (Vpp2 << 4)) | (0x0f & Vpp1);
    setskt.State        = 0;
    setskt.IREQRouting  = (reqCfg->Attributes & CSCfgIRQSteer) ?
	( PCM_IRQ_ENABLE | skt->cfgInfo.AssignedIRQ ) : setskt.IREQRouting;
    setskt.IFType       = reqCfg->IntType;
    if ( reqCfg->Vcc && skt->client_LED == _CSaixTurnOnLED ){
	setskt.CtlInd = skt->CtlIndCaps & PCM_SBM_BUSY;
    }else{
	setskt.CtlInd &= ~PCM_SBM_BUSY;
    }
    rc = SocketServices( adpt, PCM_SET_SOCKET, (caddr_t)&setskt );
    if ( rc ) goto end;

    /*
     * CS have to set the various configuration register, if it is specified. 
     */
    win = &adpt->window[skt->tuple_window];
    gran = win->WinTable[0].MemWinTbl.ReqGran;
    size = adpt->window_size;
    page_offset = reqCfg->ConfigBase / size;
    base_offset = reqCfg->ConfigBase % size;

    simple_lock( &cs_twin_lock );
    SetPageOffset( adpt, skt->tuple_window, 0, page_offset, 1);
    EnableTupleWindow( adpt, skt );

    if ( reqCfg->Present & CSCfgOptValid  ){
	cs_busio_putc( adpt, base_offset, reqCfg->ConfigIndex );
	delay ( 50 * HZ / 1000 );
	skt->cfgInfo.CardValues |= CSCfgCardOptValid;
	skt->cfgInfo.Option      = reqCfg->ConfigIndex;
	BUGLPR(var, BUGGID, ("CCR Option : was set\n"));
    }
    if ( reqCfg->Present & CSCfgStatValid ){
	cs_busio_putc( adpt, base_offset+2, reqCfg->Status );
	delay ( 50 * HZ / 1000 );
	skt->cfgInfo.CardValues |= CSCfgCardStatValid;
	skt->cfgInfo.Status      = reqCfg->Status;
	BUGLPR(var, BUGGID, ("CCR Status : was set\n"));
    }
    if ( reqCfg->Present & CSCfgPinValid  ){
	cs_busio_putc( adpt, base_offset+4, reqCfg->Pin );
	delay ( 50 * HZ / 1000 );
	skt->cfgInfo.CardValues |= CSCfgCardPinValid;
	skt->cfgInfo.Pin         = reqCfg->Pin;
	BUGLPR(var, BUGGID, ("CCR Pin    : was set\n"));
    }
    if ( reqCfg->Present & CSCfgCopyValid ){
	cs_busio_putc( adpt, base_offset+6, reqCfg->Copy );
	delay ( 50 * HZ / 1000 );
	skt->cfgInfo.CardValues |= CSCfgCardCopyValid;
	skt->cfgInfo.Copy        = reqCfg->Copy;
	BUGLPR(var, BUGGID, ("CCR Copy   : was set\n"));
    }

    DisableTupleWindow( adpt, skt );
    simple_unlock( &cs_twin_lock );

    skt->cfgInfo.Attributes = reqCfg->Attributes;
    skt->cfgInfo.Vcc        = reqCfg->Vcc;
    skt->cfgInfo.Vpp1       = reqCfg->Vpp1;
    skt->cfgInfo.Vpp2       = reqCfg->Vpp2;
    skt->cfgInfo.IntType    = reqCfg->IntType;
    skt->cfgInfo.ConfigBase = reqCfg->ConfigBase;
    skt->cfgInfo.Present    = reqCfg->Present;

    skt->cfgInfo.Attributes |= CSCfgAttrValidCli;
    skt->cfg_owner = * clientID;
    skt->client_Vcc = reqCfg->Vcc ? 1 : 0;

 end:
    set_activity( skt );
    if ( !CheckInterest( skt ) ){
	PowerOffSocket( skt->LogSocket );
    }
    lock_done( &skt->rw_lock );

    if( !rc ) cfehCfgCard( *clientID, reqCfg->Socket );

    return( rc );
} 

/****************************************************************************
 *
 * NAME:	GetConfigurationInfo
 *
 * FUNCTION:	Return information about the specified socket and
 *		PC Card configuration. 
 *		  getInfo <-- socket[getInfo->socket]->config_info
 * NOTE:	Tuples is read when a card is detected in the socket.

 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	objectID : O	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet 
 *	getInfo  : I/O	pointer of CSGetCfgInfoPkt
 *
 * RETURNS:
 *      CSR_SUCCESS       : Success
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSGetCfgInfoPkt )
 *	CSR_BAD_SOCKET	  : Socket is invalid
 * 	CSR_NO_CARD       : No PC Card in socket
 *
 ****************************************************************************/
static int
GetConfigurationInfo( int * clientID, void * pointer,
			 int argLen, CSGetCfgInfoPkt * cfgInfo )
{
    Socket *skt;
    int rc;

    if ( argLen < sizeof( CSGetCfgInfoPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( cfgInfo->Socket, &skt ) ){
	return( rc );
    }
    if( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    lock_read( &skt->rw_lock );

    memcpy( cfgInfo, &skt->cfgInfo, sizeof( CSGetCfgInfoPkt ));

    if( skt->cfgInfo.Attributes & CSCfgAttrValidCli )
	* clientID = skt->cfg_owner;
    else
	* clientID = 0;
    
    lock_done( &skt->rw_lock );

    return( rc );
}

/****************************************************************************
 *
 * NAME:	ModifyConfiguration
 *
 * FUNCTION:	Allows a socket and PC Card configuration to be
 *		modified without a pair of Release/RequestConfiguration.
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	clientID : I	pointer of client handle
 *	pointer  :	(not used)
 *	argLen   : I	length of packet CSModCfgPkt
 *	modCfg   : I	pointer of CSModCfgPkt
 * RETURNS:
 *      CSR_SUCCESS       : Success
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSGetTplDataPkt )
 *	CSR_BAD_ATTRIBUTE : IRQ steering cannot be diabled or enabled
 *	CSR_BAD_HANDLE    : Client Handle does not match owning client
 *	CSR_BAD_SOCKET    : Socket is invalid
 *	CSR_BAD_VCC	  : Requested Vcc is not available on socket
 *	CSR_BAD_VPP	  : Requested Vpp is not available on socket
 *	CSR_NO_CARD	  : No PC Card in socket
 *
 ****************************************************************************/
static int
ModifyConfiguration( int * clientID, void * pointer,
			 int argLen,  CSModCfgPkt * modCfg )
{
    SetSocket_S setskt;
    Socket  * skt;
    Adapter * adpt;
    int Vcc, Vpp1, Vpp2;
    int attr;
    int rc;

    if ( argLen < sizeof( CSModCfgPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    attr = modCfg->Attributes & (CSCfgIRQChg|CSCfgVccChg|CSCfgVpp1Chg|CSCfgVpp2Chg);
    if ( attr == NULL ){
	return( CSR_BAD_ATTRIBUTE );
	
    }
    if ( rc = ValidateSocket( modCfg->Socket, &skt ) ){
	return( rc );
    }
    adpt = &adapters[skt->AdapterIndex];
    if( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    lock_write( &skt->rw_lock );

    /* all clients can modify the configuration */
    if ( rc = cfehValidateClient( *clientID )){
	goto end;
    }
    /*  
    if( !(*clientID) || skt->cfg_owner != *clientID ){
	rc = CSR_BAD_HANDLE;
	goto end;
    }*/
    /* 
     * Set Power to the Socket etc...
     */
    setskt.Socket = skt->PhySocket;
    rc = SocketServices( adpt, PCM_GET_SOCKET, (caddr_t)&setskt );
    if ( rc ) goto end;

    if ( attr & CSCfgIRQChg ){
	setskt.IREQRouting = (modCfg->Attributes & CSCfgIRQSteer) ?
	    ( PCM_IRQ_ENABLE | skt->cfgInfo.AssignedIRQ ) :
		( setskt.IREQRouting & ~PCM_IRQ_ENABLE );
	if (modCfg->Attributes & CSCfgIRQSteer){
	    skt->cfgInfo.Attributes |= CSCfgIRQSteer;
	}else{
	    skt->cfgInfo.Attributes &= ~CSCfgIRQSteer;
	}
    }
    if ( attr & CSCfgVccChg ){
	if( ( Vcc  = ConvertPower( adpt, modCfg->Vcc,  PCM_VCC  ) ) == -1){
	    rc = CSR_BAD_VCC;
	    goto end;
	}
	if ( modCfg->Vcc && skt->client_LED == _CSaixTurnOnLED ){
	    setskt.CtlInd = skt->CtlIndCaps & PCM_SBM_BUSY;
	}else{
	    setskt.CtlInd &= ~PCM_SBM_BUSY;
	}
	setskt.VccLevel  = 0xff & Vcc;
	skt->client_Vcc = ( skt->cfgInfo.Vcc = modCfg->Vcc ) ? 1 : 0;
    }
    if ( attr & CSCfgVpp1Chg ){
	if ( ( Vpp1 = ConvertPower( adpt, modCfg->Vpp1, PCM_VPP1 ) ) == -1 ){
	    rc = CSR_BAD_VPP;
	    goto end;
	}
	setskt.VppLevels = (0x0f & Vpp1) | (0xf0 & setskt.VppLevels );
	skt->cfgInfo.Vpp1 = modCfg->Vpp1;
    }

    if ( attr & CSCfgVpp2Chg ){
        if ( ( Vpp2 = ConvertPower( adpt, modCfg->Vpp2, PCM_VPP2 ) ) == -1 ){
	    rc = CSR_BAD_VPP;
	    goto end;
	}
	setskt.VppLevels = (0xf0 & (Vpp2 << 4)) | (0x0f & setskt.VppLevels );
	skt->cfgInfo.Vpp2 = modCfg->Vpp2;
    }

    setskt.State  = 0;
    setskt.IFType = skt->cfgInfo.IntType;
    BUGLPR(var, BUGACT, ("cfginfo.IntType = %x\n", skt->cfgInfo.IntType));
    rc = SocketServices( adpt, PCM_SET_SOCKET, (caddr_t)&setskt );
    if ( rc ) goto end;

 end:
    lock_done( &skt->rw_lock );

    return( rc );
} 

/****************************************************************************
 *
 * NAME:	ReleaseConfiguration
 *
 * FUNCTION:	return a PC Card and socket to a simple memory only
 *		interface and configuration zero
 *
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	clientID : I	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSReqCfgPkt
 *	reqCfg   : I	pointer of  CSReqCfgPkt
 * RETURNS:
 *	CSR_BAD_ARG_LENGTH:
 *	CSR_BAD_HANDLE	  :
 *	CSR_BAD_SOCKET	  :
 *
 ****************************************************************************/
static int
ReleaseConfiguration( int * clientID, void * pointer,
			 int argLen, CSRelCfgPkt * relCfg )
{
    Socket * skt;
    SetSocket_S setskt;
    int rc;

    if ( argLen < sizeof( CSRelCfgPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( relCfg->Socket, &skt ) ){
	return( rc );
    }

    lock_write( &skt->rw_lock );

    if ( skt->cfg_owner != *clientID ){
	rc = CSR_BAD_HANDLE;
	goto end;
    }

    if( ( rc = cfehClientResetSlot( *clientID, relCfg->Socket ) ) 
       != CSR_SUCCESS ){
	goto end;
    }

    setskt.Socket       = skt->PhySocket;
    rc = SocketServices( &adapters[skt->AdapterIndex], 
			PCM_GET_SOCKET, (caddr_t)&setskt );
    if ( rc ) goto end;
    setskt.State        = 0;
    setskt.IREQRouting &= ~PCM_IRQ_ENABLE;
    setskt.IFType       = PCM_IF_MEMORY;
    rc = SocketServices( &adapters[skt->AdapterIndex], 
			PCM_SET_SOCKET, (caddr_t)&setskt );
    if ( rc ) goto end;

    skt->cfg_owner = 0;
    skt->cfgInfo.Attributes &= ~( CSCfgAttrValidCli | CSCfgIRQSteer ); 
    skt->cfgInfo.IntType    = CSCfgIFMem;
    skt->cfgInfo.CardValues = 0;
    skt->cfgInfo.ConfigBase = 0;
    skt->cfgInfo.Status     = 0;
    skt->cfgInfo.Pin        = 0;
    skt->cfgInfo.Copy       = 0;
    skt->cfgInfo.Option     = 0;
    skt->cfgInfo.Present    = 0;

    skt->client_Vcc         = 0;
    skt->client_LED         = 0;

    /*
     * CS is prohibited from resetting the PC Card  
     * and is not required to cycle power through zero volts
     * CS may remove power from the socket 
     * if no clients have indicated their usage of the socket 
     * by an Open Memory of REquestWindow.
     */

 end:
    set_activity( skt );
    if ( !CheckInterest( skt ) ){
	PowerOffSocket( skt->LogSocket );
    }
    lock_done( &skt->rw_lock );

    if( !rc ) cfehCfgCard( *clientID, skt->LogSocket );

    return( rc );
} 

/***************************************************************************
 *
 * NAME:	AccessConfigurationRegister
 *
 * FUNCTION:	Allows a client to read or write a PC Card
 *		Configuration Register
 * NOTE:     	Currently, there is no client validation.
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	objectID : -	(not used)
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSAccCfgRegPkt
 *	accCfgReg: I	pointer of CSAccCfgRegPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : ArgLen is less than sizeof( CSAccCfgRegPkt )
 *	CSR_BAD_ARGS	   : Specified arguments are invalid
 *	CSR_BAD_SOCKET     : Socket is invalid
 *
 ***************************************************************************/
static int
AccessConfigurationRegister( int * objectID, void * pointer,
				int argLen, CSAccCfgRegPkt * accCfgReg )
{
    uchar action = accCfgReg->Action;
    ulong size, base_offset, page_offset;
    int gran;
    Socket  * skt;
    Window  * win;
    Adapter * adpt;
    int rc;

    /* 
     * Read or Write the specified value
     *  from/to the specified configuration register
     */
    if ( argLen < sizeof( CSAccCfgRegPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( accCfgReg->Socket, &skt ) ){
	return( rc );
    }
    if ( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    lock_write( &skt->rw_lock );

    if (skt->cfg_owner == NULL){
	rc = CSR_BAD_SOCKET;
	goto end;
    }
    if ( action != CSCfgRegREAD && action != CSCfgRegWRITE ){
	rc = CSR_BAD_ARGS;
	goto end;
    }
    adpt = &adapters[skt->AdapterIndex];
    win  = &adpt->window[skt->tuple_window];
    
    /* 
     * Register Address : Adapter.config_info.ConfigBase + offset 
     */
    gran = win->WinTable[0].MemWinTbl.ReqGran;
    size = adpt->window_size;
    page_offset = skt->cfgInfo.ConfigBase / size;
    base_offset = skt->cfgInfo.ConfigBase % size;

    BUGLPR( var, BUGGID, ("Info.ConfigBase = %x\n", skt->cfgInfo.ConfigBase));
    BUGLPR( var, BUGGID, ("page_offset = %x\n", page_offset));
    BUGLPR( var, BUGGID, ("base_offset = %x\n", base_offset));
    BUGLPR( var, BUGGID, ("accCfgReg->Value = %x\n", accCfgReg->Value));

    simple_lock( &cs_twin_lock );
    SetPageOffset( adpt, skt->tuple_window, 0, page_offset, 1 );
    EnableTupleWindow( adpt, skt );

    if ( action == CSCfgRegREAD ){
	accCfgReg->Value = 
	    cs_busio_getc( adpt, base_offset + accCfgReg->Offset );
    }
    else{    /* == CSCfgRegWRITE */
	cs_busio_putc( adpt, base_offset + accCfgReg->Offset,
		      accCfgReg->Value );
        delay ( 50 * HZ / 1000 );
    }
    
    DisableTupleWindow( adpt, skt );
    simple_unlock( &cs_twin_lock );
    
 end:
    lock_done( &skt->rw_lock );
    return( rc );
}	

/**********************************
 * BULK MEMORY SERVICES FUNCTIONS *
 **********************************/
/*
 * Currently, all Bulk Memory Service functions are not supported
 * This part will be done in near future
 */
#ifdef NOT_SUPPORTED
/***************************************************************************
 *
 * NAME:	OpenMemory
 *
 * FUNCTION:	open an area of a memory card to allow use of 
 *		the Read/Write/CopyMemory and the erase functions.
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
OpenMemory(int * objectID, void * pointer, int argLen, void * argBuf )
{
    return( CSR_UNSUPPORTED_FUNCTION );
} 

/***************************************************************************
 *
 * NAME:	ReadMemory
 *
 * FUNCTION:	Read data from a PC Card via the specified Memory Handle
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
ReadMemory( int * objectID, void * pointer, int argLen, void * argBuf )
{
    return( CSR_UNSUPPORTED_FUNCTION );
} 

/****************************************************************************
 *
 * NAME:	WriteMemory
 *
 * FUNCTION:	write data to a PC Card via the specified Memory-Handle
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
WriteMemory( int * objectID, void * pointer, int argLen, void * argBuf )
{
    return( CSR_UNSUPPORTED_FUNCTION );
} 

/***************************************************************************
 *
 * NAME:	CopyMemory
 *
 * FUNCTION:	Read data from a PC Card in the specified logical
 *		socket and writes it to another location in the same region.
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
CopyMemory( int * objectID, void * pointer, int argLen, void * argBuf )
{
    return( CSR_UNSUPPORTED_FUNCTION );
} 

/***************************************************************************
 *
 * NAME:	RegisterEraseQueue
 *
 * FUNCTION:	Register a client supplied erase queue with CardServices
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
RegisterEraseQueue(int * objectID, void * pointer, int argLen, void * argBuf)
{
    return( CSR_UNSUPPORTED_FUNCTION );
} 

/***************************************************************************
 *
 * NAME:	CheckEraseQueue
 *
 * FUNCTION:	Notify CardServices that the client has placed new
 *		entries into the queue to be serviced.
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
CheckEraseQueue(int * objectID, void * pointer, int argLen, void * argBuf)
{
    return( CSR_UNSUPPORTED_FUNCTION );
} 

/***************************************************************************
 *
 * NAME:	DeregisterEraseQueue
 *
 * FUNCTION:	Deregister the erase queue that the client previously
 *		registerd with Card Services
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
DeregisterEraseQueue(int * objectID, void * pointer, 
			int argLen, void * argBuf)
{
    return( CSR_UNSUPPORTED_FUNCTION );
} 

/***************************************************************************
 *
 * NAME:	CloseMemory
 *
 * FUNCTION:	Close an area of a memory card that was opened by a
 *		corresponding OpenMemory.
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
CloseMemory(int * objectID, void * pointer, int argLen, void * argBuf)
{
    return( CSR_UNSUPPORTED_FUNCTION );
} 
#endif /* NOT_SUPPORTED */

/******************************
 * CLIENT UTILITIES FUNCTIONS *
 ******************************/
/*
 * The following functions are not supported, but do in future.
 *	- GetFirstRegion, GetNextRegion
 *	- GetFirstPartition, GetNextPartition
 */

/****************************************************************************
 *
 * NAME:	GetFirstTuple
 *
 * FUNCTION:	Get the first tuple of the specified type in the
 *		CIS for the specified socket.
 *		  ( call tuple_search() )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	objectID : -	(not used)
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSGetCliInfoPkt
 *	getTpl   : I/O	pointer of CSGetCliInfoPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSGetTplPkt )
 *	CSR_BAD_SOCKET	   : Socket is invalid
 *	CSR_NO_CARD	   : No PC Card in socket
 *	CSR_NO_MORE_ITEMS  : No Card Information Structure (CIS) or
 *				desird tuple not found
 *
 ***************************************************************************/
static int
GetFirstTuple( int * objectID, void * pointer, 
		int argLen, CSGetTplPkt * getTpl )
{
    Socket * skt;
    int rc;

    if ( argLen < sizeof( CSGetTplPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( getTpl->Socket, &skt ) ){
	return( rc );
    }
    if ( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    lock_write( &skt->rw_lock );

    if ( !skt->cfgInfo.Vcc ){
	if ( rc = ResetSocket( skt->LogSocket ) ){
	    lock_done( &skt->rw_lock );
	    return( rc );
	}
    }
    if ( !( rc = PowerOnSocket( skt->LogSocket ) ) ){
        rc = tuple_search( getTpl, TPGetFirst );
        if ( !CheckInterest( skt ) || !skt->client_Vcc ){
            set_PowerOff_timer( skt );
        }else{
	    set_activity( skt );
	}
    }

    lock_done( &skt->rw_lock );

    return( rc );
}


/****************************************************************************
 *
 * NAME:	GetNextTuple
 *
 * FUNCTION:	return the next tuple of the specified type in the
 *		CIS for the specified socket.
 *		  ( call tuple_search() )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	objectID : -	(not used)
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSGetCliInfoPkt
 *	getTpl   : I/O	pointer of CSGetCliInfoPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSGetTplPkt )
 *	CSR_BAD_SOCKET	   : Socket is invalid
 *	CSR_NO_CARD	   : No PC Card in socket
 *	CSR_NO_MORE_ITEMS  : No Card Information Structure (CIS) or
 *				desird tuple not found
 *
 ****************************************************************************/
static int
GetNextTuple(int * objectID, void * pointer, int argLen, CSGetTplPkt *getTpl)
{
    Socket * skt;
    int rc;

    if ( argLen < sizeof( CSGetTplPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( getTpl->Socket, &skt ) ){
	return( rc );
    }
    if ( !CheckCardPresence( skt ) ){
	return( CSR_NO_CARD );
    }

    lock_write( &skt->rw_lock );

    if ( !skt->cfgInfo.Vcc ){
	if ( rc = ResetSocket( skt->LogSocket ) ){
	    lock_done( &skt->rw_lock );
	    return( rc );
	}
    }
    if ( !( rc = PowerOnSocket( skt->LogSocket ) ) ){
        rc = tuple_search( getTpl, TPGetNext );
        if ( !CheckInterest( skt ) || !skt->client_Vcc ){
            set_PowerOff_timer( skt );
        }else{
	    set_activity( skt );
        }
    }

    lock_done( &skt->rw_lock );

    return( rc );
}

	
/****************************************************************************
 *
 * NAME:	GetTupleData	
 *
 * FUNCTION:	Get the content of the last tuple returned by 
 *		GetFirst/NextTuple.
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	objectID : -	(not used)
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSGetTplDataPkt
 *	tpldata  : I/O	pointer of CSGetTplDataPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSGetTplDataPkt )
 *	CSR_BAD_ARGS       : Data from prior GetFirst/NextNext is corrupt
 *	CSR_BAD_SOCKET	   : Socket is invalid
 *	CSR_NO_CARD	   : No PC Card in socket
 *	CSR_NO_MORE_ITEMS  : No more tuple data on PC Card
 *
 ****************************************************************************/
static int
GetTupleData( int * objectID, void * pointer,
		 int argLen, CSGetTplDataPkt * tpldata )
{
    Socket  * skt;
    int     rc;

    if ( argLen < sizeof( CSGetTplDataPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( tpldata->Socket, &skt ) ){
	return( rc );
    }
    if ( !CheckCardPresence( skt )){
	return( CSR_NO_CARD );
    }

    lock_write( &skt->rw_lock );

    if ( !skt->cfgInfo.Vcc ){
	if ( rc = ResetSocket( skt->LogSocket ) ){
	    goto end;
	}
    }
    if ( rc = PowerOnSocket( skt->LogSocket ) ){
        goto end;
    }
    rc = tuple_search( (CSGetTplPkt *) tpldata, TPGetData );

 end:
    if ( !CheckInterest( skt ) || !skt->client_Vcc ){
	set_PowerOff_timer( skt );
    }else{
	set_activity( skt );
    }
    lock_done( &skt->rw_lock );

    return ( rc );
}

#ifdef NOT_SUPPORTED
/****************************************************************************
 *
 * NAME:	GetFirstRegion
 *
 * FUNCTION:	Get device information for the first region of devices
 *		on the card in the specified socket
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ****************************************************************************/
static int 
GetFirstRegion( int * MTDHandle, void * pointer, int argLen, void * argBuf )
{
    return CSR_UNSUPPORTED_FUNCTION;
}

/****************************************************************************
 *
 * NAME:	GetNextRegion
 *
 * FUNCTION:	Get device information for the next region of devices
 *		on the card in the specified socket
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
GetNextRegion(int * MTDHandle, void * pointer, int argLen, void * argBuf )
{
    return CSR_UNSUPPORTED_FUNCTION;
}

/****************************************************************************
 *
 * NAME:	GetFirstPartition
 *
 * FUNCTION:	Get device information for the first partion on the
 *		card in the specified socket based on the PC Card's CIS
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *
 ***************************************************************************/
static int
GetFirstPartition( int * objectID, void * pointer, int argLen, void * argBuf)
{
    return CSR_UNSUPPORTED_FUNCTION;
}
	
/****************************************************************************
 *
 * NAME:	GetNextPartition
 *
 * FUNCTION:	Get device information for the next partion on the
 *		card in the specified socket based on the PC Card's CIS
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *		
 ****************************************************************************/
static int
GetNextPartition( int * objectID, void * pointer, int argLen, void * argBuf )
{
    return CSR_UNSUPPORTED_FUNCTION;
}
#endif /* NOT_SUPPORTED */

/****************************************************************************
 *
 * 	ADVANCED CLIENT SERVICES FUNCTIONS 
 *
 * Following Functions are not planed to support in AIX
 *      - AdjustResourceInfo
 *      - AddSocketServices
 *      - ReplaceSocketServices
 *      - ReturnSSEntry
 */

/****************************************************************************
 *
 * NAME:	MapLogSocket
 *
 * FUNCTION:	Map a Card Services logical socket to its
 *		Socket Services physical adapter and socket values
 *		( Logical Socket --> Physical Adapter and Socket )
 * EXECUTION ENVIRONMENT: client and usr thread
 *                        with cs_cfg_lock=r 
 * ARGUMENTS:
 *	objectID : -	(not used)
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSMapSktPke
 *	mapSkt   : I/O	pointer of CSMapSktPke
 * RETURNS:	
 *      CSR_SUCCESS       : Success	
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSMapSktPkt )
 *	CSR_BAD_SOCKET    : Socket is invalid
 *
 ****************************************************************************/
int
MapLogSocket(int * objectID, void * pointer, int argLen, CSMapSktPkt *mapSkt)
{
    Socket * skt;
    int rc;

    if ( argLen < sizeof( CSMapSktPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( mapSkt->LogSocket, &skt ) ){
	return( rc );
    }
    mapSkt->PhyAdapter = adapters[skt->AdapterIndex].PhyAdapter;
    mapSkt->PhySocket  = skt->PhySocket;

    return( CSR_SUCCESS );
}

/****************************************************************************
 *
 * NAME:	MapPhySocket
 *
 * FUNCTION:	Map Socket Services physical adapter and socket values
 *		to a Card Services logcal socket
 *		  ( Physical Adapter and Socket --> Logical Socket )
 * EXECUTION ENVIRONMENT: client or usr thread
 *                        with cs_cfg_lock=r 
 * ARGUMENTS:
 *	objectID : -	(not used)
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSMapSktPke
 *	mapSkt   : I/O	pointer of CSMapSktPke
 * RETURNS:	
 *      CSR_SUCCESS       : Success	
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSMapSktPkt )
 *	CSR_BAD_ADAPTER   : Adapter is invalid
 *	CSR_BAD_SOCKET    : Socket is invalid
 *
 ****************************************************************************/
int
MapPhySocket( int * objectID, void * pointer,
		 int argLen, CSMapSktPkt * mapSkt )
{
    int a;

    if ( argLen < sizeof( CSMapSktPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    for( a = 0; a < numAdapters && mapSkt->PhyAdapter != adapters[a].PhyAdapter; a++);
    if ( a == numAdapters ){
	return( CSR_BAD_ADAPTER );
    }
    if ( mapSkt->PhySocket >= adapters[a].numSockets ){
	return( CSR_BAD_SOCKET );
    }
    mapSkt->LogSocket = 
	adapters[a].socket[mapSkt->PhySocket].LogSocket;

    return( CSR_SUCCESS);
}

/****************************************************************************
 *
 * NAME:	MapLogWindow
 *
 * FUNCTION:	Maps a Card Sercies WindowID passed in the Handle
 * 		argument to its Socket Services physical adapter and
 * 		window.
 *		  ( LogWindow (windowID) --> Physical Adapter and Window )
 * EXECUTION ENVIRONMENT: client and usr thread
 *                        with cs_cfg_lock=r 
 * ARGUMENTS:
 *	windowID : I	pointer of window handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSMapWinPkt
 *	mapWin   : I/O	pointer of CSMapWinPkt
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_ARG_LENGTH      : argLen is less than sizeof( CSMapWinPkt )
 *	CSR_BAD_HANDLE          : WindowHandle is invalid
 *
 ****************************************************************************/
int
MapLogWindow( int * windowID, void * pointer, 
		int argLen, CSMapWinPkt * mapWin )
{
    int w, rc;
    Window * win = (Window *) *windowID;
    extern int ValidateWindow( Window ** );

    if ( argLen < sizeof( CSMapWinPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateWindow( &win ))
	return( rc );
    w = win->PhyWindow;
    if( ! ((Window *) *windowID)->ownerID ){
	return( CSR_BAD_HANDLE );
    }
    mapWin->PhyAdapter = adapters[windows[w]->AdapterIndex].PhyAdapter;
    mapWin->PhyWindow  = windows[w]->PhyWindow;
    
    return( CSR_SUCCESS );
}

/****************************************************************************
 *
 * NAME:	MapPhyWindow
 *
 * FUNCTION:	Map Socket Services physical adapter and window value
 *		to a Card Services logical WindowID.
 *		  ( Physical Adapter and Window --> Logical Window )
 * EXECUTION ENVIRONMENT: client and usr thread
 *                        with cs_cfg_lock=r 
 * ARGUMENTS:
 *	windowID : I	pointer of window handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSMapWinPkt
 *	mapWin   : I/O	pointer of CSMapWinPkt
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_ARG_LENGTH      : argLen is less than sizeof( CSMapWinPkt )
 *	CSR_BAD_HANDLE          : WindowHandle is invalid
 *
 ****************************************************************************/
int
MapPhyWindow( int * windowID, void * pointer,
		int argLen, CSMapWinPkt * mapWin )
{
    int a;

    if ( argLen < sizeof( CSMapWinPkt ))
	 return( CSR_BAD_ARG_LENGTH );
    for( a = 0; a < numAdapters 
	&& mapWin->PhyAdapter != adapters[a].PhyAdapter; a++);
    if ( a == numAdapters ){
	return( CSR_BAD_ADAPTER );
    }
    if ( mapWin->PhyWindow >= adapters[a].numWindows ){ 
	return( CSR_BAD_WINDOW );
    }
    windowID = (int *) &(adapters[a].window[mapWin->PhyWindow]);

    return( CSR_SUCCESS);
}

#ifdef NOT_SUPPORTED
/****************************************************************************
 *
 * NAME:	RegisterMTD
 *
 * FUNCTION:	Allow a Memory Technology Driver to register to ahndle
 *		accesses for a region by a memory function.
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *		
 ****************************************************************************/
static int
RegisterMTD ( int * objectID, void * pointer, int argLen, void * argBuf )
{
    return CSR_UNSUPPORTED_FUNCTION;
}

/***************************************************************************
 *
 * NAME:	RegisterTimer
 *
 * FUNCTION:	register a callback structure with Card Services
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *		
 ***************************************************************************/
static int
RegisterTimer( int *objcetID, void *pointer, int argLen, void *argbuf ) 
{
    return CSR_UNSUPPORTED_FUNCTION;
}

/***************************************************************************
 *
 * NAME:	SetRegion
 *
 * FUNCTION:	Allow a client to set a PC Card region's characteristics.
 * NOTE:        This function will be supported in future.
 * RETURNS:	CSR_CSR_UNSUPPORTED_FUNCTION:
 *		
 ***************************************************************************/
static int
SetRegion( int *objcetID, void *pointer, int argLen, void *argbuf )
{
    return CSR_UNSUPPORTED_FUNCTION;
}
#endif /* NOT_SUPPORTED */

/****************************************************************************
 *
 * NAME:	ValidateCIS
 *
 * FUNCTION:	validates the Card Information Structure on the PC
 *		Card in the spedified socket.
 *                ( call tuple_search() )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	objectID : -	(not used)
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSValCISPkt
 *	valCIS   : I/O	pointer of CSValCISPkt
 * RETURNS:
 *	CSR_SUCCESS       : Success
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSValCISPkt )
 *	CSR_BAD_SOCKET	  : Socket is invalid
 *	CSR_NO_CARD	  : No PC Card in socket
 *
 ****************************************************************************/
static int
ValidateCIS( int * objectID, void * pointer, int argLen, CSValCISPkt *valCIS)
{
    CSGetTplPkt gettpl;
    Socket * skt;
    int count = 0;
    int rc;

    if ( argLen < sizeof( CSValCISPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( valCIS->Socket, &skt ) ){
	return( rc );
    }

    lock_write( &skt->rw_lock );

    if ( !skt->cfgInfo.Vcc ){
	if ( rc = ResetSocket( skt->LogSocket ) ){
	    lock_done( &skt->rw_lock );
	    return( rc );
	}
    }
    if ( rc = PowerOnSocket( skt->LogSocket ) ){
        goto end;
    }

    bzero( &gettpl, sizeof( CSGetTplPkt ) );  
    gettpl.Socket = skt->PhySocket;
    gettpl.Attributes = CSGetLinkTpls;
    gettpl.DesiredTuple = 0xff;

    if ( tuple_search( &gettpl, TPGetFirst ) == CSR_SUCCESS )
	for( count = 1;
	    tuple_search( &gettpl, TPGetNext ) == CSR_SUCCESS; count++ );

    valCIS->Chains = count;

 end:
    if ( !CheckInterest( skt ) || !skt->client_Vcc ){
	set_PowerOff_timer( skt );
    }else{
	set_activity( skt );
    }

    lock_done( &skt->rw_lock );

    return( rc );
}


/****************************************************************************
 *
 * NAME:	RequestExclusive
 *
 * FUNCTION:	request the exclusive use of a PC Card in a socket for
 *		a client
 *		  ( call cfehRequestExclusive() )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	clientID : I	pointer of client handle
 *	pointer  :	(not used)
 *	argLen   : I	length of packet CSExclPkt
 *	reqExcl  : I	pointer of CSExclPkt
 * RETURNS:
 *      CSR_SUCCESS       : Success
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSExclPkt )
 *	CSR_BAD_HANDLE	  : ClientHandle is invalid
 *	CSR_BAD_SOCKET	  : Socket is invalid
 *	CSR_IN_USE	  : PC Card already in-use exclusively
 *	CSR_NO_CARD	  : No PC Card in socket
 *
 ***************************************************************************/
int
RequestExclusive( int * clientID, void * pointer,
		 int argLen, CSExclPkt * reqExcl )
{
    Socket * skt;
    int rc;
    extern int cfehRequestExclusive( int, int );

    if ( argLen < sizeof( CSExclPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( reqExcl->Socket, &skt ) ){
	return( rc );
    }
    if ( !CheckCardPresence( skt )){
	return( CSR_NO_CARD );
    }
    rc = cfehRequestExclusive( *clientID, reqExcl->Socket );

    return( rc );
}

/****************************************************************************
 *
 * NAME:	ReleaseExclusive
 *
 * FUNCTION:	release the exclusive use of a card in a socket for a client
 *		  ( call cfehReleaseExclusive() )
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:   
 *	clientID : I	pointer of client handle
 *	pointer  :	(not used)
 *	argLen   : I	length of packet CSExclPkt
 *	relExcl  : I	pointer of CSExclPkt
 * RETURNS:
 *      CSR_SUCCESS       : Success
 *	CSR_BAD_ARG_LENGTH: argLen is less than sizeof( CSExclPkt )
 *	CSR_BAD_HANDLE	  : ClientHandle is invalid
 *	CSR_BAD_SOCKET	  : Socket is invalid
 *
 ****************************************************************************/
int
ReleaseExclusive(int *clientID, void *pointer, int argLen, CSExclPkt *relExcl)
{
    Socket * skt;
    int rc;
    extern int cfehReleaseExclusive( int, int );

    if ( argLen < sizeof( CSExclPkt ) )
	return( CSR_BAD_ARG_LENGTH );
    if ( rc = ValidateSocket( relExcl->Socket, &skt ) ){
	return( rc );
    }

    rc = cfehReleaseExclusive( *clientID, relExcl->Socket );

 end:
    return( rc );
}

/****************************************************************************
 *
 * NAME:	GetFirstClient
 *
 * FUNCTION:	get the first ClientHandle of the clients that have
 *		registerd with Card Services
 *		  ( call cfehGetFirstClient() )
 * EXECUTION ENVIRONMENT: client and user thread
 *                        with cs_cfg_lock=r
 * ARGUMENTS:   
 *	clientID : O	pointer of client handle
 *	pointer  :	(not used)
 *	argLen   : I	length of packet CSGetCliInfoPkt
 *	getCli   : I/O	pointer of CSGetCliInfoPkt
 *                      if bit0 of getCli->Attributes is reset to zero (0)
 *                          all clients registered to CS	
 *                      if the bit0 is set to one (1) 
 *                          clients for the specified socket only
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSGetCliInfoPkt )
 *	CSR_BAD_SOCKET	   : Socket is invalid
 *	CSR_NO_MORE_ITEMS  : No clients are registerd
 *	CSR_NO_CARD	   : No PC Card in socket
 *
 ****************************************************************************/
int
GetFirstClient( int *clientID, void * pointer, int argLen, 
	       CSGetClientPkt * getCli )
{
    Socket * skt;
    int rc;

    if ( argLen < sizeof( CSGetClientPkt ) )
	 return( CSR_BAD_ARG_LENGTH );

    if( getCli->Attributes & CSGetCliThisSocket ){
	if ( rc = ValidateSocket( getCli->Socket, &skt ) ){
	    return( rc );
	}
	if ( !CheckCardPresence( skt ) ){
	    return( CSR_NO_CARD );
	}

	if ( !( *clientID = cfehGetFirstClient( getCli->Socket ) ) ){
	    return( CSR_NO_MORE_ITEMS );
	}
    }
    else{
	if ( !( *clientID = cfehGetFirstClient( NO_SLOT ) ) ){
	    return( CSR_NO_MORE_ITEMS );
	}
    }
    
    return( CSR_SUCCESS );
}

/****************************************************************************
 *
 * NAME:	GetNextClient
 *
 * FUNCTION:	get the ClientHandle for the next registerd client.
 *		  ( call cfehGetNextClient() )
 * EXECUTION ENVIRONMENT: client and usr thread
 *                        with cs_cfg_lock=r
 * ARGUMENTS:   
 *	clientID : I/O	pointer of client handle
 *      			( which is previously returned by
 *				 GetFirstClient or GetNextClient
 *	pointer  :	(not used)
 *	argLen   : I	length of packet CSGetCliInfoPkt
 *	getCli   : I	pointer of CSGetCliInfoPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : argLen is less than sizeof( CSGetCliInfoPkt )
 *	CSR_BAD_HANDLE	   : ClientHandle is invalid
 *	CSR_BAD_SOCKET	   : Socket is invalid (Socket request only)
 *	CSR_NO_MORE_ITEMS  : No more clients are registerd
 *	CSR_NO_CARD	   : No PC Card in socket
 *
 ****************************************************************************/
int
GetNextClient( int * clientID, void * pointer,
		 int argLen,   CSGetClientPkt * getCli )
{
    Socket * skt;
    int rc;

    if ( argLen < sizeof( CSGetClientPkt ) )
	 return( CSR_BAD_ARG_LENGTH );

    if( getCli->Attributes & CSGetCliThisSocket ){
	if ( rc = ValidateSocket( getCli->Socket, &skt ) ){
	    return( rc );
	}
	if ( !CheckCardPresence( skt ) ){
	    return( CSR_NO_CARD );
	}
	*clientID = cfehGetNextClient( *clientID, getCli->Socket );
    }
    else{
	*clientID = cfehGetNextClient( *clientID, NO_SLOT );
    }
    if ( *clientID == -1 ){
	rc = CSR_BAD_HANDLE;
	*clientID = 0;
    }else if( *clientID == 0 ){
	rc = CSR_NO_MORE_ITEMS;
	*clientID = 0;
    }else{
	rc = CSR_SUCCESS;
    }
    return( rc );
}

/****************************************************************************
 *
 * NAME:	GetClientInfo
 *
 * FUNCTION:	return information describing a client.
 *		  (call cfehGetClientInfo() )
 * EXECUTION ENVIRONMENT: client thread
 *                        with cs_cfg_lock=r
 * ARGUMENTS:   
 *	clientID : I	pointer of client handle
 *	pointer  : -	(not used)
 *	argLen   : I	length of packet CSGetCliInfoPkt
 *	getCli   : I/O	pointer of CSGetCliInfoPkt
 * RETURNS:
 *      CSR_SUCCESS        : Success
 *	CSR_BAD_ARG_LENGTH : ArgLen is less than sizeof CSGetCliInfoPkt
 *	CSR_BAD_HANDLE     : ClientHandle is invalid
 *
 ****************************************************************************/
int
GetClientInfo( int *clientID, void * pointer, int argLen,
	      CSGetCliInfoPkt *	getCli )
{
    int rc = CSR_SUCCESS;

    if ( argLen < sizeof( CSGetCliInfoPkt ) )
	 return( CSR_BAD_ARG_LENGTH );
    if( !cfehGetClientInfo( *clientID, *getCli ) ){
	return( CSR_BAD_HANDLE );
    }

    return( rc );
}

/***************************************************************************
 *
 * NAME:	VendorSpecific
 *
 * FUNCTION:	Currently, just implemented for debug function.
 *		
 * RETURNS:	CSR_UNSUPPORTED_FUNCTION;
 *
 ****************************************************************************/
static int
VendorSpecific( int * objectID, void * pointer, int argLen, void * argBuf )
{
    CSaixVdrSpcPkt* p;
    Socket  * skt;
    Adapter * adpt;
    int rc;

    p = (CSaixVdrSpcPkt*) argBuf;

    if ( argLen < sizeof( CSaixVdrSpcPkt ) ){
      rc = CSR_UNSUPPORTED_FUNCTION;
      goto end;
    }
    if( p->funccode == CSaixSocketServices ){
      if ( rc = ValidateSocket( p->Socket, &skt ) ){
	return( rc );
      }
      adpt = &adapters[skt->AdapterIndex];
      if( rc = SocketServices( adpt, p->subcode, (caddr_t)&p->subpkt ) ){
	BUGLPR( var, BUGACT, ("SS_CMD ( 0x%x ) : fail with 0x%x\n"\
			      , p->subcode, rc ) );
	rc = (int)p->subpkt[0]; /* rc should be first member */
	goto end;
      }

    }else if( p->funccode == CSaixLockSocket ){
      if( ( rc = cfehValidateClient( *objectID ) ) ||
	 ( rc = ValidateSocket( p->Socket, &skt ) ) ){
	goto end;
      }
      /* rc should be set as CSR_SUCCESS at here */
      if( p->subcode == _CSaixHoldLock ){
	lock_write( &skt->rw_lock );
	lock_set_recursive( &skt->rw_lock );
      }else if( p->subcode == _CSaixReleaseLock ){
	lock_clear_recursive( &skt->rw_lock );
	lock_done( &skt->rw_lock );
      }else{
	rc = CSR_UNSUPPORTED_FUNCTION;
      }

    }else if( p->funccode == CSaixControlLED ){
      if( ( rc = cfehValidateClient( *objectID ) ) ||
	 ( rc = ValidateSocket( p->Socket, &skt ) ) ){
	goto end;
      }
      if ( p->subcode == _CSaixTurnOnLED || p->subcode == _CSaixTurnOffLED ){
	lock_write( &skt->rw_lock );
	skt->client_LED = p->subcode;
	rc = ControlLED( skt, p->subcode );
	lock_done( &skt->rw_lock );
      }else{
	rc = CSR_UNSUPPORTED_FUNCTION;
      }

    }else{
	rc = CSR_UNSUPPORTED_FUNCTION;
    }
 end:
    return rc;
}
