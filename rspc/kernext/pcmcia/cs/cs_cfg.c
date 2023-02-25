static char sccsid[] = "@(#)84	1.4  src/rspc/kernext/pcmcia/cs/cs_cfg.c, pcmciaker, rspc41J, 9515A_all 4/11/95 16:26:05";
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS:  pcmciacs_config
 *               InitCardServices, UninitCardServices
 *               InitAdapter, UninitAdapter, init_intr
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
#include <sys/types.h>
#include <sys/device.h>
#include <sys/uio.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/malloc.h>
#include <sys/sysconfig.h>
#include <sys/errno.h>
#include <sys/pin.h>
#include <sys/lockl.h>
#include <sys/lockname.h>
#include <sys/lock_alloc.h>
#include <sys/trcmacros.h>
#include <sys/trchkid.h>
#include <sys/syspest.h>
#include <sys/timer.h>
#include <sys/pmdev.h>
#include <sys/pcmciass.h>
#include <sys/pcmciacs.h>
#include "cs.h"

extern Adapter *adapters;
extern Socket *sockets[];
extern Window *windows[];
extern int numAdapters;
extern int numSockets;
extern int numWindows;

extern Complex_lock cs_cfg_lock;
extern Simple_lock  cs_io_lock;
extern Simple_lock  cs_intr_lock;
extern Simple_lock  cs_twin_lock;
extern struct trb * OLintr;           /* Off Level Interrupt Handler */
extern int cs_intr ( struct intr * ); /* Second Level Interrupt Handler */

static lock_t cfg_lock = LOCK_AVAIL;

static int  InitCardServices();
static void UninitCardServices();
static int  InitAdapter( int, struct cfg_cs_mdi * );
static int  UninitAdapter( int );

BUGVDEF( var, BUGACT );

/****************************************************************************
 *
 * NAME:	pcmciacs_config
 *
 * FUNCTION:	PCMCIA Card Services Configuration Routine
 *
 * EXECUTION ENVIRONMENT: config thread
 * ARGUMENTS:
 *     cmd : CFG_INIT or CFG_TERM
 *     uiop: pointer of struct uio
 * RETURNS:	
 *     0       The operation is successfully completed.
 *     ENOMEM  Memory space is unavailable for required allocation.
 *     EINVAL  Invalid config parameter is passed.
 *
 ****************************************************************************/
int pcmciacs_config( int cmd, struct uio *uiop )
{
    struct cfg_cs_mdi mdi;
    int a, rc;

    if ( lockl ( &cfg_lock, LOCK_SHORT ) != LOCK_SUCC ){
	return EIO;
    }
    if ( numAdapters == MAX_ADAPTER ){
	unlockl ( &cfg_lock );
	return ENOMEM;
    }
    if( rc = uiomove((caddr_t)&mdi, (int)sizeof(struct cfg_cs_mdi), 
		     UIO_WRITE, uiop) ){
	unlockl ( &cfg_lock );
	return rc;
    }
    
    switch( cmd )
    {
    case CFG_INIT:
	/*
	 * If this configuration is for the first PCMCIA adapter,
	 * initialize Card Services and Event Handler Kernel Process.
	 */
	if( numAdapters == -1 ){
	    if ( rc = InitCardServices() ){
		break;
	    }
	}
	/*
	 * Check if the adapter is already configured or not.
	 */
	for( a = 0; a < numAdapters; a++){
	    if( adapters[a].PhyAdapter == mdi.devno ){
		rc = EALREADY;
		break;
	    }
	}

	if ( a == numAdapters ){
	    lock_write( &cs_cfg_lock );
	    if ( rc = InitAdapter( a, &mdi )){
		adapters[a].PhyAdapter = 0;
	    }else{
		numAdapters++;
	    }
	    lock_done( &cs_cfg_lock );
	}
	break;
	
    case CFG_TERM:
	for( a = 0; a < numAdapters; a++){
	    if( adapters[a].PhyAdapter == mdi.devno ){
		break;
	    }
	}
	if ( a == numAdapters ){
	    rc = EINVAL;
	    break;
	}
	lock_write( &cs_cfg_lock );
	if ( !( rc = UninitAdapter( a ) ) ){
	    numAdapters--;
	}
	lock_done( &cs_cfg_lock );
	break;

    default:
	rc = EINVAL;
	break;
    }
    
    if( numAdapters == 0 ){
	/*
	 * Before unconfigure CardServices, EventHandler must be unconfigured
	 * to process all existing events. This is done when the last slot
	 * is unregisterd with calling evhCoreDel().
	 */
	UninitCardServices();
    }

    BUGLPR(var, BUGNTF, ("numAdapters = %d\n", numAdapters));

    unlockl( &cfg_lock );
    return ( rc );
}

/****************************************************************************
 *
 * NAME:	InitCardServices
 *
 * FUNCTION:	Initialize the Card Services Internal data.
 *              This function is called by pcmciacs_config().
 *			
 * EXECUTION ENVIRONMENT: config thread
 * ARGUMENTS:   none.
 * RETURNS:	
 *     0 :      The operation is successfully completed.
 *     ENOMEM  Memory space is unavailable for required allocation.
 *
 ****************************************************************************/
static int InitCardServices()
{
    int rc; 

    /* malloc adapter struct space word-alligned from pinned_heap */
    if( adapters = ( Adapter * ) 
	xmalloc	( (sizeof( Adapter ) ) * MAX_ADAPTER, 4, pinned_heap ) ){
        if ( pin( adapters, sizeof( Adapter ) * MAX_ADAPTER )){
            xmfree( adapters, pinned_heap );
	    return( ENOMEM);
	}
    }else{
      return( ENOMEM);
    }

    bzero( adapters, sizeof( Adapter ) *  MAX_ADAPTER );
    
    /* initialize lock */
    lock_alloc( &cs_io_lock, LOCK_ALLOC_PIN, CS_IO_LOCK, -1 );
    simple_lock_init( &cs_io_lock );
    lock_alloc( &cs_intr_lock, LOCK_ALLOC_PIN, CS_INTR_LOCK, -1 );
    simple_lock_init( &cs_intr_lock );
    lock_alloc( &cs_twin_lock, LOCK_ALLOC_PIN, CS_CSWIN_LOCK, -1 );
    simple_lock_init( &cs_twin_lock );
    lock_alloc( &cs_cfg_lock, LOCK_ALLOC_PIN, CS_CFG_LOCK, -1 );
    lock_init( &cs_cfg_lock, 0 );
	
    /* Initialize CS function dispatch table  */
    initCSfuncs();

    /* allocate timer for off level interrupt */
    OLintr = talloc();
    OLintr->func_data = 0;

    numAdapters = 0;
    return(0);
}

/*****************************************************************************
 *
 * NAME:	UninitCardServices
 *
 * FUNCTION:	Uninitialize the Card Services Internal data.
 *              This function is called by pcmciacs_config().
 *			
 * EXECUTION ENVIRONMENT: config thread
 *
 * ARGUMENTS:   none
 *
 * RETURNS:	0 : Success
 *
 ****************************************************************************/
static void UninitCardServices()
{
    BUGLPR(var, BUGACT, ("Prepare to UnLoad CS Kernel Extention\n"));
    unpin ( adapters, sizeof( Adapter ) * MAX_ADAPTER );
    xmfree( adapters, pinned_heap );
    numAdapters = -1;

    tstop( OLintr );
    tfree( OLintr );
     
    lock_free( &cs_io_lock );
    lock_free( &cs_intr_lock );
    lock_free( &cs_cfg_lock );

    unpincode( cs_intr );
}


/****************************************************************************
 *
 * NAME:	InitAdapter
 *
 * FUNCTION:	Initialize an Adapter
 *                - Call SocketServices to inquire the capability
 *                    InquireAdapter, InquireWindow, InquireSocket
 *                - Initilize Adapter
 *                - If a Card has been inserted, Initialize the Socket 
 *			
 * EXECUTION ENVIRONMENT: config thread
 *
 * ARGUMENTS:   a 	: Adapter Index
 *		mdi     : pointer to struct cfg_cs_mdi
 *	
 * RETURNS:	0 : Success
 *              
 ****************************************************************************/
static int
InitAdapter( int a, struct cfg_cs_mdi *mdi )
{

    int s, w, l, i, busnum, rc;
    int reqcap = PCM_WC_COMMON | PCM_WC_ATTRIBUTE;
    uint status;
    caddr_t dsdptr;
    Adapter * adpt = &adapters[a];
    Socket  * skt;
    Window  * win;

    GetSSInfo_S      ssinfo;
    GetVendorInfo_S  vdinfo;
    InquireAdapter_S inqadpt;
    InquireSocket_S inqskt;
    InquireWindow_S inqwin;
    VISTRUCT vistruct;
    AISTRUCT aistruct;
    SISTRUCT sistruct;
    WISTRUCT wistruct;
    SetAdapter_S setadpt;
    GetStatus_S getstat;
    SetPage_S setpage;
    SetWindow_S setwin;

    extern int cspm_handler( caddr_t, int );           /* pm handle */
    extern int cspm_socket_control( dev_t, int, int ); /* pm planar control */
    extern int InitSocket( int );

    /*
     * Copy Adapter parameter from mdi
     */
    adpt->int_mode    = mdi->int_mode;
    adpt->PhyAdapter  = mdi->devno;
    adpt->bus_id      = mdi->bus_id;
    adpt->window_base = mdi->waddr;
    adpt->window_size = mdi->wsize;
    adpt->bus_type    = mdi->bus_type;
    bcopy( mdi->devname, adpt->devname, 0x10 );
    busnum = atoi( &adpt->devname[3]);

    /*
     * IOCTL entry point is set at the top of SS DDS
     */
    if( (rc = devswqry( adpt->PhyAdapter, &status, &dsdptr )) 
       || dsdptr == NULL ){
	BUGLPR(var, BUGACT, ("device swich table is incorrect!!\n"));
	return( rc );
    }
    BUGLPR(var, BUGACT, ("ioctl entry point = 0x%x\n", dsdptr));
    adpt->ddioctl = (int (*)()) *(int *)dsdptr;

    /* 
     * Open the device driver with fp_opendev().
     */
    rc = fp_opendev( adpt->PhyAdapter, DREAD|DWRITE|DNDELAY,
		    NULL, 0, &(adpt->fp) );
    if( rc ){
	return( rc );
    }

    /*
     * Check the Compliance of Socket Services 
     */
    bzero( &ssinfo, sizeof(GetSSInfo_S));
    rc = SocketServices( adpt, PCM_GET_SS_INFO, (caddr_t) &ssinfo );
    if( rc || ssinfo.Compliance != SS_LEVEL ){
	return( EINVAL );
    }

    /* Don't Care */
    bzero( &vdinfo,   sizeof(GetVendorInfo_S));
    bzero( &vistruct, sizeof(VISTRUCT));
    vdinfo.pBuffer = &vistruct;
    SocketServices( adpt, PCM_GET_VENDOR_INFO, (caddr_t) &vdinfo );

    /* 
     *  Check the capability of the Adapter
     */
    bzero( &inqadpt,  sizeof( InquireAdapter_S ) );
    bzero( &aistruct, sizeof(AISTRUCT));
    inqadpt.pBuffer = &aistruct; 
    inqadpt.pBuffer->wBufferLength = sizeof(AISTRUCT);
    if ( SocketServices( adpt, PCM_INQ_ADAPTER, (caddr_t) &inqadpt ) ){
	rc = EINVAL;
	goto end;
    }
    memcpy( &adpt->CharTable, &inqadpt.pBuffer->CharTable,
	       sizeof( ACHARTBL ));
    adpt->numPwrEntries = inqadpt.pBuffer->wNumPwrEntries;
    memcpy( &adpt->PwrEntry, &inqadpt.pBuffer->PwrEntry,
	   sizeof( PWRENTRY ) * adpt->numPwrEntries );
    adpt->numSockets = inqadpt.NumSockets;
    adpt->numWindows = inqadpt.NumWindows;
    adpt->numEDCs    = inqadpt.NumEDCs;

    /* 
     * Memory Allocate for Socket and Window structure 
     */
    if ( adpt->socket = ( Socket * )
	xmalloc( (sizeof( Socket ) ) * adpt->numSockets, 4, pinned_heap )){
        if ( pin  ( adpt->socket, (sizeof(Socket))*adpt->numSockets )){
	    xmfree( adpt->socket, pinned_heap );
	    rc = ENOMEM;
	    goto end;
	}
    }else{
        rc = ENOMEM;
        goto end;
    }
    if ( adpt->window = ( Window * ) 
	xmalloc( (sizeof( Window ) ) * adpt->numWindows, 4, pinned_heap )){
        if ( pin  ( adpt->window, (sizeof(Window))*adpt->numWindows )){
	    xmfree( adpt->window, pinned_heap );
	    rc = ENOMEM;
	    goto end;
	}
    }else{
        rc = ENOMEM;
        goto end;
    }

    bzero( adpt->socket, (sizeof(Socket))*adpt->numSockets );
    bzero( adpt->window, (sizeof(Window))*adpt->numWindows );

    lock_alloc( &adpt->win_lock, LOCK_ALLOC_PIN, CS_CSWIN_LOCK, -1 );
    simple_lock_init( &adpt->win_lock );

    /*
     * Map Physical Sockets to Logical Sockets
     */

    for(s = 0, l = numSockets, skt = adpt->socket;
	s < adpt->numSockets; s++, l++, skt++){
	skt->PhySocket      = s;
	skt->LogSocket      = l;
	skt->AdapterIndex   = a;
	skt->cfgInfo.Socket = l;
	sockets[l] = skt;

	skt->pwrofft = talloc();

#if defined( PM_ENABLE )
	skt->ppch.devid = PMDEV_PCMCIA00 + ( busnum << 4 ) + s;
	skt->ppch.control = cspm_socket_control;
	rc = pm_register_planar_control_handle( &skt->ppch, PM_REGISTER );
#endif
        lock_alloc( &skt->rw_lock, LOCK_ALLOC_PIN, CS_SOCKET_LOCK, -1 );
	lock_init( &skt->rw_lock, 0 );

	/*
	 * register skt to cfgevh 
	 * currenetly return value is ignored
	 */
	get_logsocket( skt, adpt->PhyAdapter,  adpt->devname );
    }

    /*
     * Map Physical Windows to Logical Windows
     */
    for(w = 0, l = numWindows, win = adpt->window;
	w < adpt->numWindows; w++, l++, win++){
	win->PhyWindow    = w;
	win->LogWindow    = l;
	win->AdapterIndex = a;
	win->LogSocket    = -1;
	windows[w] = win;
    }
    numSockets += adpt->numSockets;
    numWindows += adpt->numWindows;
    BUGLPR(var, BUGNTF, ("numAdapters = %d\n", numAdapters));
    BUGLPR(var, BUGNTF, ("numSockets  = %d\n", numSockets ));
    BUGLPR(var, BUGNTF, ("numWindows  = %d\n", numWindows ));

    /*
     * check the capability of each window
     */
    bzero( &inqwin,   sizeof( InquireWindow_S ) );
    bzero( &wistruct, sizeof( WISTRUCT ) );
    inqwin.pBuffer = &wistruct;
    inqwin.pBuffer->wBufferLength = sizeof( WISTRUCT );
    for(w = 0, win = adpt->window; w < adpt->numWindows; w++, win++ ){
	inqwin.Window  = w;
	if ( SocketServices( adpt, PCM_INQ_WINDOW, (caddr_t) &inqwin ) ){
	    rc = EINVAL;
	    goto end;
        }
	win->AdapterIndex = a;
	win->PhyWindow    = w;
	win->WinCaps      = inqwin.WndCaps;
	win->Sockets      = inqwin.Sockets;
	memcpy( win->WinTable, inqwin.pBuffer->WinTable, sizeof(WINTBL)*2 );
    }
    
    for(s = 0, skt = adpt->socket; s < adpt->numSockets; s++, skt++ ){
	/*
	 * check the capability of each socket
	 */
	bzero( &inqskt,   sizeof( InquireSocket_S ) );
	bzero( &sistruct, sizeof( SISTRUCT ) );
	inqskt.pBuffer = &sistruct;
	inqskt.pBuffer->wBufferLength = sizeof( SISTRUCT );
	inqskt.Socket  = s;

	if ( SocketServices( adpt, PCM_INQ_SOCKET, (caddr_t) &inqskt ) ){
	    rc = EINVAL;
	    goto end;
        }

	skt->SCIntCaps = inqskt.SCIntCaps;
	skt->SCRptCaps = inqskt.SCRptCaps;
	skt->CtlIndCaps= inqskt.CtlIndCaps;
	memcpy( &skt->SktTable, &inqskt.pBuffer->CharTable, sizeof(SCHARTBL));

	/* 
	 * assign a window for each socket to read tuple
	 */
	setwin.Window = -1;
	setwin.Socket = skt->PhySocket;
	setwin.Size   = adpt->window_size;
	setwin.State  = 0;
	setwin.Speed  = 0;	/* AssignWindow assign the slowest Speed */
	setwin.Base   = adpt->window_base;
	if( rc = AssignWindow( adpt, (PCM_WC_COMMON|PCM_WC_ATTRIBUTE),
			      (ulong) adpt->window_base, adpt->window_size,
			      &setwin ) ){
	    rc = ENOMEM;
	    goto end;
	}
	skt->window_base = setwin.Base;
	skt->window_size = setwin.Size;
 	setpage.Window = w = setwin.Window;
	setpage.Page   = 0;
	setpage.State  = PCM_PS_ATTRIBUTE;
	setpage.Offset = 0;
	rc = SocketServices( adpt, PCM_SET_PAGE,   (caddr_t) &setpage );
	if ( rc ) goto end;
	rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t) &setwin );
	if ( rc ) goto end;
	adpt->window[skt->tuple_window = setwin.Window].ownerID = CSID;
	BUGLPR(var, BUGACT, ("Assign a window(0x%x) to socket(0x%x) as tuple window\n",w, s));

	bzero( &getstat,   sizeof( GetStatus_S ) );
	getstat.Socket = skt->PhySocket;
	rc = SocketServices( adpt, PCM_GET_STATUS, (caddr_t)&getstat );
	if ( rc ) goto end;

	skt->critical_state = 1;
	if ( getstat.CardState & PCM_SBM_CD ){
	    skt->card_present = 1;
	    InitSocket( skt->LogSocket );
	}else{
	    skt->card_present = 0;
	    PowerOffSocket( skt->LogSocket );
	}

	/* IRQ 0xff means that no IRQ is assigned */
	skt->cfgInfo.AssignedIRQ = 0xff;
    }
    
    /*
     * Enable Interrupt
     */
    if( rc = init_intr( adapters, mdi->bus_type, mdi->irqlvl, mdi->irqpri,
		       adpt->bus_id ) ){
	BUGLPR(var, BUGACT, ("i_init() failed\n"));
	goto end;
    }
    setadpt.State = 0;
    setadpt.SCRouting = PCM_IRQ_ENABLE | adpt->intr.level;
    rc = SocketServices( adpt, PCM_SET_ADAPTER, (caddr_t)&setadpt );
    if ( rc ) goto end;

#if defined( PM_ENABLE )
    /*
     * initialize pm_handle
     */
    adpt->pmh.activity   = PM_ACTIVITY_NOT_SET;
    adpt->pmh.mode       = PM_DEVICE_FULL_ON;
    adpt->pmh.device_idle_time    = mdi->dev_itime;
    adpt->pmh.device_standby_time = mdi->dev_stime;
    adpt->pmh.handler    = cspm_handler;
    adpt->pmh.private    = (caddr_t)adpt;
    adpt->pmh.devno      = adpt->PhyAdapter;
    adpt->pmh.device_logical_name = adpt->devname;
    adpt->pmh.pm_version = PM_PHASE2_SUPPORT;
    /* all other members are initialized with '0' */

    /*
     * register the handle to PM core
     */
    pm_register_handle( &adpt->pmh, PM_REGISTER );
#endif /* PM_ENABLE */
    
 end:
    if ( rc ){
	fp_close( adpt->fp );
    }
    return( rc );
}


/****************************************************************************
 *
 * NAME:	UninitAdapter
 *
 * FUNCTION:	Uninitialize an Adapter
 *
 * EXECUTION ENVIRONMENT: config thread
 *                        with cs_cfg_lock=w
 * 
 * ARGUMENTS:   a : Adapter Index
 * RETURNS:	0 : Success
 *
 ***************************************************************************/
static int UninitAdapter( int a )
{
    int s, w, tw, rc;
    Adapter * adpt = &adapters[a];
    SetAdapter_S setadpt;
#if defined( PM_ENABLE )
    pm_register_handle( &adpt->pmh, PM_UNREGISTER );
#endif /* PM_ENABLE */    
    rc = SocketServices( adpt, PCM_GET_ADAPTER, (caddr_t)&setadpt );
    if ( rc ) goto end;
    setadpt.SCRouting &= ~PCM_IRQ_ENABLE;
    rc = SocketServices( adpt, PCM_SET_ADAPTER, (caddr_t)&setadpt );
    if ( rc ) goto end;

    i_clear( &adpt->intr );

    /*
     * If there is a client which is intrested the socket on the adapter,
     * this adapter should not be unconfigured.
     */
    /* unmap logical sockets and windows */
    for( s = 0; s < MAX_SOCKET; s++){
	if( sockets[s] && sockets[s]->AdapterIndex == a ){
	    lock_free( &sockets[s]->rw_lock );
	    evhCoreDel( sockets[s]->LogSocket );
            tstop( sockets[s]->pwrofft );
            tfree( sockets[s]->pwrofft );
	    sockets[s] = NULL;
	}
    }
    for( w = 0; w < numWindows; w++){
	if( windows[w] && windows[w]->AdapterIndex == a ){
	    windows[w] = NULL;
	}
    }
    /* remap windows */ 	
    for( w = 0; w < numWindows - adapters[a].numWindows; w++)
	if ( windows[w] == NULL ) 
            for( tw = w; tw < numWindows; tw++)
	        if( windows[tw] )
		    windows[w] = windows[tw];

    adpt->PhyAdapter = 0;
    if( adpt->fp ){
	rc = fp_close( adpt->fp );
	if( rc ){
	    BUGLPR(var, BUGACT, ("fp_close: fail\n"));
	}
    }
    lock_free( &adpt->win_lock ); 

    numSockets -= adpt->numSockets;
    numWindows -= adpt->numWindows;

    BUGLPR(var, BUGNTF, ("unpin socket and window structuer\n"));
    unpin( adpt->socket, sizeof( Socket ) * adpt->numSockets );
    unpin( adpt->window, sizeof( Window ) * adpt->numWindows );

    BUGLPR(var, BUGNTF, ("xfree socket and window structuer\n"));
    xmfree( adpt->socket, pinned_heap );
    xmfree( adpt->window, pinned_heap );

 end:
    BUGLPR(var, BUGNTF, ("numSockets  = %d\n", numSockets ));
    BUGLPR(var, BUGNTF, ("numWindows  = %d\n", numWindows ));

    return( rc );
}

/*****************************************************************************
 *
 * NAME:	init_intr
 *
 * FUNCTION:	initialize interrupt handler
 *
 * EXECUTION ENVIRONMENT: config thread
 * ARGUMENTS:	(none)
 * RETURNS:	0 : Success
 *		
 ****************************************************************************/
static int init_intr( Adapter * adpt, unsigned short bus_type, 
		int level, int priority, unsigned long bid )
{
    int i;

    pincode( cs_intr ); /* pcmciacspin */
	
    adpt->intr.next     = NULL;
    adpt->intr.handler  = cs_intr;
    adpt->intr.bus_type = BUS_BID; /* bus_type */
    adpt->intr.flags    = adpt->int_mode == 'I' ?
			  ( INTR_LEVEL | INTR_POLARITY ) :
			  INTR_NOT_SHARED;
    adpt->intr.level    = level;
    adpt->intr.priority = priority;
    adpt->intr.bid      = bid;

    if( i_init( &adpt->intr ) != INTR_SUCC ){
	/* if error, unpin bottom half and return */
	unpincode((int (*) ())cs_intr);
	return( 1 );
    }
    
    return( 0 );
}
