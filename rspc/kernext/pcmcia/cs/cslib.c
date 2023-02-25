static char sccsid[] = "@(#)28	1.8  src/rspc/kernext/pcmcia/cs/cslib.c, pcmciaker, rspc41J, 9521A_all 5/24/95 00:10:00";
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS:
 *	AssignWindow, ConvertSpeed,
 *	GetWindwo, SetWindow, EnableWindow, DisableWindow,
 *	EnableTupleWindow, DisableTupleWindow, SetPageOffset,
 *	cs_iomem_att, cs_iomem_det, cs_busio_putc, cs_busio_getc,
 *      set_PowerOff_timer, set_activity
 *      CheckCardPresence,  CheckInterest, tuple_search,
 *      ValidateSocket, ValidateWindow
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
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/syspest.h>
#include <sys/timer.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/inline.h>
#include <sys/pcmciacs.h>
#include <sys/pcmciass.h>
#include <sys/pcmciacsAix.h>
#include "cs.h"

extern Simple_lock cs_io_lock;
extern Simple_lock cs_twin_lock;
extern Simple_lock cs_intr_lock;
extern Adapter * adapters;
extern Socket  * sockets[];
extern Window  * windows[];
extern numAdapters;
extern numSockets;

#ifdef DEBUG
extern int var;
#endif

/*****************************************************************************
 *
 * NAME:	AssignWindow
 *
 * FUNCTION:	Assign a window which match to the required capability
 *			
 * EXECUTION ENVIRONMENT: config or client thread
 * ARGUMENT:
 *	adpt   : pointer of Adapter
 *	reqCap : Capability of requested window
 *	reqBase: requested Base address of window
 * 	reqSize: requested Size of window
 *	setWin : pointer of SetWindow
 * RETURNS:
 *	CSR_SUCCESS : Success
 *	CSR_BAD_BASE: Base address is invalid
 *	CSR_BAD_SIZE: Size is invalid
 *
 ****************************************************************************/
int AssignWindow( Adapter * adpt, int reqCap, ulong reqBase, ulong reqSize,
		 SetWindow_S * setWin )
{
    MEMWINTBL * tbl;
    Window    * win;
    ulong       Base;
    ulong       Size, size;
    int Speed;
    int rc = CSR_OUT_OF_RESOURCE;
    int loop, w;
    int found  = 0;
    int mask   = PCM_WC_COMMON | PCM_WC_ATTRIBUTE | PCM_WC_IO;

    reqCap &= mask;

    BUGLPR( var, BUGGID, ("reqCap  = %x\n", reqCap));
    BUGLPR( var, BUGGID, ("setWin->Socket  = %x\n", setWin->Socket));
    BUGLPR( var, BUGGID, ("setWin->State   = %x\n", setWin->State));
    BUGLPR( var, BUGGID, ("reqBase         = %x\n", reqBase ));
    BUGLPR( var, BUGGID, ("reqSize         = %x\n", reqSize ));
    BUGLPR( var, BUGGID, ("setWin->Speed   = %x\n", setWin->Speed));

    /*
     * In the first loop, try to assign a window which have an exact 
     * capability the client requests. In the second loop, try to 
     * assign a window which includes the requested capability.
     */
    for(loop = 0; loop < 2; loop++){
	for(w = 0, win = adpt->window; w < adpt->numWindows; w++, win++){
	    /*
	     * check if the window capability is the required one
	     */
	    if(win->ownerID == 0 && (win->Sockets & (0x1 << setWin->Socket))){
		if ( loop == 0){
		    if ((win->WinCaps & mask) != reqCap )
			continue;
		}
		else{
		    if ((win->WinCaps & reqCap) != reqCap  ||
			(win->WinCaps & mask) == reqCap )
			continue;
		}
		/*
		 * validate the BASE, SIZE & SPEED etc.
		 */
		tbl = &win->WinTable[0].MemWinTbl;
		if ( ( reqCap & PCM_WC_IO ) &&
		    ( win->WinCaps & ( PCM_WC_COMMON | PCM_WC_ATTRIBUTE ))){
		    tbl++;
		}

		Base = ( ulong ) reqBase / tbl->ReqGran;
		Size = reqSize / tbl->ReqGran;
		/*
		 * check base
		 */
		if ( ( ulong ) reqBase % tbl->ReqGran ){
		    rc = CSR_BAD_BASE;
		    continue;
		}
		if (tbl->MemWndCaps & PCM_WC_BASE ){
		    if (tbl->FirstByte > Base ||
			(/* tbl->LastByte != NULL &&*/ tbl->LastByte < Base)){
			rc = CSR_BAD_BASE;
			continue;
		    }
		}
		else{
		    if (tbl->FirstByte != reqBase){
			rc = CSR_BAD_BASE;
			continue;
		    }
		}
		/*
		 * check size
		 */
		if (tbl->MemWndCaps & PCM_WC_SIZE ){
		    if (tbl->MinSize > reqSize ||
			(tbl->MaxSize && tbl->MaxSize < reqSize)){
			rc = CSR_BAD_SIZE;
			continue;
		    }
		    /* check pow2 */
		    if (tbl->MemWndCaps & PCM_WC_POW2 ){
			for(size = tbl->ReqGran; 
			    size < tbl->MinSize; size <<= 1);
			for(; size < reqSize && 
			    (tbl->MaxSize ? size < tbl->MaxSize : size != 0);
			    size <<=1);
			if (size != reqSize){
			    rc = CSR_BAD_SIZE;
			    continue;
			}
		    }
		    else{
			if (reqSize % tbl->ReqGran){
			    rc = CSR_BAD_SIZE;
			    continue;
			}
		    }
		}
		else{
		    if (tbl->MinSize != Size){
			rc = CSR_BAD_SIZE;
			continue;
		    }
		}
		
		/*
		 * no need to check PCM_WC_WENABLE now
		 */
		/* ckeck State */
		if ((setWin->State & PCM_WS_16BIT ) ?
		    !(tbl->MemWndCaps & PCM_WC_16BIT) :
		    !(tbl->MemWndCaps & PCM_WC_8BIT)){
		    rc = CSR_BAD_ATTRIBUTE;
		    continue;
		}
		if ((setWin->State & PCM_WS_PAGED ) ?
		    !(tbl->MemWndCaps & PCM_WC_PAVAIL) : 0 ){
		    rc = CSR_BAD_ATTRIBUTE;
		    continue;
		}
		
		/* ckeck base alignment */
		if (tbl->MemWndCaps & PCM_WC_BALIGN ){
		    if ( Base % Size ){
			rc = CSR_BAD_BASE;
			continue;
		    }
		}

		/*
		 * If the bit PCM_WC_CALIGN is set,
		 * CSWinOffset bit is set and return with Attributes.
		 */
		/* if (tbl->MemWndCaps & PCM_WC_WP ) */

		setWin->Base = Base;
		setWin->Size = Size;
  	        if ((PCM_WS_IO & setWin->State)){
		    BUGLPR( var, BUGACT, ("Assign PhyWindow (0x%x) to Socket (0x%x) as IO window\n", w, setWin->Socket));
		}
		else{
		    if ( setWin->Speed == 0 )
			setWin->Speed = tbl->Slowest;
		    if (( Speed = ConvertSpeed(setWin->Speed)) == 0 ||
			( Speed > ConvertSpeed( tbl->Slowest ) ) ||
			( Speed < ConvertSpeed( tbl->Fastest ) ) ){
			rc = CSR_BAD_SPEED;
			continue;
		    }

		    BUGLPR( var, BUGACT, ("Assign PhyWindow (0x%x) to Socket (0x%x) as MEM window\n", w, setWin->Socket));
		}
		setWin->Window = w;
		return( CSR_SUCCESS );
	    }
	}
    }
    return( rc );
}

/*****************************************************************************
 *
 * NAME:	ConvertSpeed
 *
 * FUNCTION:	Converts the access speed parameter passed to RequestWindow
 *              to a time in units of 0.1ns. 
 * EXECUTION ENVIRONMENT: config or client thread
 * ARGUMENTS:
 *	speed : Access speed in format of the extended speed byte of
 *		the Device ID tuple.
 * RETURNS:
 *	Access Speed in units of 0.1ns (In case of Success)
 *	0 : if speed is invalid
 *
 ****************************************************************************/
int ConvertSpeed( uint speed )
{
    int mantissa = (speed >> 3) & 0xf;
    int exponent =  speed       & 0x7;
    
    /* if mantissa is zero, exponent field is speed code */
    if (mantissa == 0){
	switch (exponent){
	case CSWin250ns: return(250*10);
	case CSWin200ns: return(200*10);
	case CSWin150ns: return(150*10);
	case CSWin100ns: return(100*10);
	default:  	 return(0);
	}
    }
    else{
	static int m[] = {0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
	static int e[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};
	int t = m[mantissa] * e[exponent];
	return( t );
    }
}

/****************************************************************************
 *
 * NAME:	SetPageOffset
 *
 * FUNCTION:	Set Page Offset for tuple window which has assign for
 *		each socket.
 * EXECUTION ENVIRONMENT: config or client thread
 * ARGUMENTS:	
 *	fp    :	file descriptor for SocketServices
 *	w     :	physical window	number
 *	p     :	page number
 *	offset:	offset
 *	attr  : 1 ( attribute memory )
 *		0 ( common memory )
 * RETURNS: 
 *	0      : Success
 *	others : return value from SocketServices
 *  
 ****************************************************************************/
/*
 * It is assumed that all windows are not paged 
 */
static int paged = 0;

int SetPageOffset( Adapter * adpt, int w, int p, ulong offset, int attr )
{
    SetPage_S setpage;
    int rc;

    setpage.Window = w;
    setpage.Page   = p;
    setpage.State  = (attr ? PCM_PS_ATTRIBUTE : 0 ) | PCM_PS_ENABLED;
    setpage.Offset = offset;
    rc = SocketServices( adpt, PCM_SET_PAGE, (caddr_t)&setpage );
    
    return( rc );
}

/****************************************************************************
 *
 * NAME:	GetWindow
 *
 * FUNCTION:	Get Window and Page
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	fp     : file descriptor for SocketServices
 *	getWin : pointer of GetWindow_S
 *	getPage: pointer of GetPage_S
 * RETURNS:	
 *	0      : Success
 *	others : return value from SocketServices
 *
 ****************************************************************************/
int GetWindow( Adapter *adpt, GetWindow_S *getWin, GetPage_S * getPage )
{
    int rc;

    if ( getWin ){
	rc = SocketServices( adpt, PCM_GET_WINDOW, (caddr_t)getWin );
	if ( rc ) return( rc );
    }
    if ( getPage )
	rc = SocketServices( adpt, PCM_GET_PAGE, (caddr_t)getPage );

    return( rc );
}    

/****************************************************************************
 *
 * NAME:	SetWindow
 *
 * FUNCTION:	Set the Window and Page
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	fp     : file descriptor for SocketServices
 *	getWin : pointer of GetWindow_S
 *	getPage: pointer of GetPage_S
 * RETURNS:	
 *	0      : Success
 *	others : return value from SocketServices
 *
 ****************************************************************************/
int SetWindow( Adapter *adpt, SetWindow_S *setWin, SetPage_S * setPage )
{
    int rc;

    if ( setPage ){
	rc = SocketServices( adpt, PCM_SET_PAGE, (caddr_t)setPage );
	if ( rc ) return( rc );
    }
    if ( setWin )
	rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)setWin );

    return( rc );
}


/****************************************************************************
 *
 * NAME:	DisableWindow
 *
 * FUNCTION:	Disable the specified window
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	fp     : file descriptor for SocketServices
 *	getWin : pointer of GetWindow_S
 *	getPage: pointer of GetPage_S
 * RETURNS:	
 *	0      : Success
 *	others : return value from SocketServices
 *
 ****************************************************************************/
int DisableWindow( Adapter *adpt, SetWindow_S *setWin, SetPage_S * setPage )
{
    int rc = 0;

    if( ! paged || setPage == NULL){
	if ( setWin->State & PCM_WS_ENABLED ){
	    setWin->State &= ~PCM_WS_ENABLED;
	    rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)setWin );
	    if ( rc ) return( rc );
	}
    }
    else{
	if ( setPage->State & PCM_PS_ENABLED ){
	    setPage->State &= ~PCM_PS_ENABLED;
	    rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)setPage );
	}
    }

    return( rc );
}

/****************************************************************************
 *
 * NAME:	FreeWindow
 *
 * FUNCTION:	Disable the specified window and clear values
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	fp     : file descriptor for SocketServices
 *	getWin : pointer of GetWindow_S
 *	getPage: pointer of GetPage_S
 * RETURNS:	
 *	0      : Success
 *	others : return value from SocketServices
 *
 ****************************************************************************/
int FreeWindow( Adapter *adpt, SetWindow_S *setWin, SetPage_S * setPage )
{
    int rc = 0;

    if( ! paged || setPage == NULL){
	if ( setWin->State & PCM_WS_ENABLED ){
	    setWin->State &= ~PCM_WS_ENABLED;
	    setWin->Base = 0;
	    setWin->Size = 1;
	    rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)setWin );
	    if ( rc ) return( rc );
	}
    }
    else{
	if ( setPage->State & PCM_PS_ENABLED ){
	    setPage->State &= ~PCM_PS_ENABLED;
	    rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)setPage );
	}
    }

    return( rc );
}

/****************************************************************************
 *
 * NAME:        EnableWindow
 *
 * FUNCTION:	Enable the specified window.
 *		The way to enable the window depends on the adapters
 *		pagable	capability.
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	fp     : file descriptor for SocketServices
 *	getWin : pointer of GetWindow_S
 *	getPage: pointer of GetPage_S
 * RETURNS:	
 *	0      : Success
 *	others : return value from SocketServices
 *
 ****************************************************************************/
int EnableWindow( Adapter *adpt, SetWindow_S *setWin, SetPage_S * setPage )
{
    int rc = 0;

    if( !paged ){
	if ( !(setWin->State & PCM_WS_ENABLED) ){
	    setWin->State |= PCM_WS_ENABLED;
	    rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)setWin );
	    if ( rc ) return( rc );
	}
    }
    else{
	if ( !(setPage->State & PCM_PS_ENABLED) ){
	    setPage->State |= PCM_PS_ENABLED;
	    SocketServices( adpt, PCM_SET_PAGE, (caddr_t)setPage );
	}
    }

    return( rc );
}

/****************************************************************************
 *
 * NAME:	EnableTupleWindow
 *
 * FUNCTION:	Enable the window of spcified socket which has been assigned to
 *		a certain socket to read tuple or CIS.
 * EXECUTION ENVIRONMENT: config or client thread
 * ARGUMENTS:
 *	adpt   : pointer of Adapter which the socket belongs
 *	skt    : pointer of socket
 * RETURNS:	
 *	0      : Success
 *	others : return value from SocketServices
 *
 ****************************************************************************/
int EnableTupleWindow( Adapter *adpt, Socket *skt )
{
    SetSocket_S setskt;
    SetWindow_S setwin;
    int rc;

    ControlLED( skt, _CSaixTurnOnLED );

    setwin.Window = skt->tuple_window;
    rc = SocketServices( adpt, PCM_GET_WINDOW, (caddr_t)&setwin );
    if ( rc ) return( rc );
    setwin.Base   = skt->window_base;
    setwin.Size   = skt->window_size;
    if ( !(setwin.State & PCM_WS_ENABLED) ){
	setwin.State |= PCM_WS_ENABLED;
	rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)&setwin );
    }
    adpt->mapped_socket = skt->PhySocket;
    
    return( rc );
}

/****************************************************************************
 *
 * NAME:	DisableTupleWindow
 *
 * FUNCTION:	Disable the Window of specified Socket which has assigned to 
 *		read tuple or CIS.
 * EXECUTION ENVIRONMENT: config or client thread
 * ARGUMENTS:
 *	adpt   : pointer of Adapter which the socket belongs
 *	skt    : pointer of socket
 * RETURNS:	
 *	0      : Success
 *	others : return value from SocketServices
 *
 ****************************************************************************/
int DisableTupleWindow( Adapter *adpt, Socket *skt )
{
    SetWindow_S setwin;
    SetPage_S setpage;
    int rc;

    setwin.Window = skt->tuple_window;
    rc = SocketServices( adpt, PCM_GET_WINDOW, (caddr_t)&setwin );
    if ( rc ) return( rc );
    
    if ( setwin.State & PCM_WS_ENABLED ){
	setwin.State &= ~PCM_WS_ENABLED;
	rc = SocketServices( adpt, PCM_SET_WINDOW, (caddr_t)&setwin );
	if ( rc ) return( rc );
    }

    setpage.Window = skt->tuple_window;
    setpage.Page   = 0;
    setpage.State  = PCM_PS_ATTRIBUTE;
    setpage.Offset = 0;
    rc = SocketServices( adpt, PCM_SET_PAGE, (caddr_t)&setpage );
    
    adpt->mapped_socket = -1;

    return( rc );
}

/****************************************************************************
 *
 * NAME:	CheckCardPresence
 *
 * FUNCTION:	Check if a card is present at the specified socket.
 * EXECUTION ENVIRONMENT: config, client or event-handler thread
 * ARGUMENTS:
 *	skt    : pointer of socket
 * RETURNS:	
 *	0      : No card in the soocket,
 *               or the card in the socket is not ready to access.
 *	1      : A Card exists in the socket and ready to access.
 *
 ****************************************************************************/
int CheckCardPresence( Socket *skt )
{
    return ( !skt->critical_state );
}

/*****************************************************************************
 *
 * NAME:	cs_iomem_att
 *
 * FUNCTION:	Assist bus memory attach on different type of bus.
 *              ex, ISA and MICRO_CHANEL.
 * EXECUTION ENVIRONMENT: config or client thread
 * ARGUMENTS:
 *	bus_id   : bus ID
 *	bus_addr : address
 *	bus_size : size 
 * RETURNS:	( char * ) Address
 *
 *****************************************************************************/

caddr_t
cs_iomem_att( Adapter * adpt )
{
    struct io_map io_map;

    if( adpt->bus_id ){
	io_map.key     = IO_MEM_MAP;
	io_map.flags   = 0;
	io_map.size    = adpt->window_size;
	io_map.bid     = BID_VAL( BID_TYPE( adpt->bus_id ),
			ISA_BUSMEM, BID_NUM( adpt->bus_id ) );
	io_map.busaddr = adpt->window_base;
	return( iomem_att( &io_map ) );
    }
    else{ /* bus_type == BUS_MICRO_CHANNEL */
	return( BUSMEM_ATT( adpt->bus_id, adpt->window_base ) );
    }
}

/*****************************************************************************
 *
 * NAME:	cs_iomem_det
 *
 * FUNCTION:	Assist bus memory detach on different type of bus.
 *              i.e. ISA and MICRO_CHANNEL.
 * EXECUTION ENVIRONMENT: config or client thread
 * ARGUMENTS: 
 *	addr     : address assigned by cs_iomem_det()
 * RETURNS:	( void )
 *
 *****************************************************************************/
void cs_iomem_det( ulong bus_id, uchar * addr )
{
    if( bus_id ){
	iomem_det( addr );
    }
    else{ /* bus_type == BUS_MICRO_CHANNEL */
	BUSMEM_DET( addr );
    }
}

/*****************************************************************************
 *
 * NAME:	cs_busio_putc
 *
 * FUNCTION:	Write the specified data to different type of bus.
 *              (i.e. ISA and MCA)
 * EXECUTION ENVIRONMENT: client thread
 * ARGUMENTS:
 *	adpt	 : Pointer to adapter structure
 *      offset   : offset to put
 *	value    : value to put
 * RETURNS:	( void )
 *
 *****************************************************************************/
void cs_busio_putc( Adapter * adpt, ulong offset, uchar value )
{
    int old_pri;
    caddr_t base;

    if( adpt->socket[adpt->mapped_socket].critical_state  ){
        return;
    }

    old_pri = disable_lock( adpt->intr.priority, &cs_io_lock );
    base = cs_iomem_att( adpt );

    if( adpt->bus_id ){
	BUSIO_PUTC( base + offset, value );
        eieio();
    }
    else{ /* bus_type == BUS_MICRO_CHANNEL */
	BUS_PUTC( base + offset, value );
    }
    cs_iomem_det( adpt->bus_id, base );
    unlock_enable( old_pri, &cs_io_lock );
}

/*****************************************************************************
 *
 * NAME:	cs_busio_getc
 *
 * FUNCTION:	Read a value from different type of bus.
 *              (i.e. ISA and MCA)
 * EXECUTION ENVIRONMENT: client or config thread
 * ARGUMENT:	
 *	addr	 : address
 * RETURNS:     the value read from the address
 *
 *****************************************************************************/
uchar cs_busio_getc( Adapter * adpt, ulong offset )
{
    uchar value;
    int old_pri;
    caddr_t base;

    if( offset > adpt->window_size
       || adpt->socket[adpt->mapped_socket].critical_state  ){
        return( 0xFF );
    }
    old_pri = disable_lock( adpt->intr.priority, &cs_io_lock );
    base = cs_iomem_att( adpt );
    if( adpt->bus_id ){
	value = BUSIO_GETC( base + offset );
    }
    else{ /* bus_type == BUS_MICRO_CHANNEL */
	value = BUS_GETC( base + offset );
    }
    cs_iomem_det( adpt->bus_id, base );
    unlock_enable( old_pri, &cs_io_lock );

    return ( value );
}

/*****************************************************************************
 *
 * NAME:        set_PowerOff_timer
 *
 * FUNCTION:	set timer to power off the specified socket
 *
 * EXECUTION ENVIRONMENT: client thread
 *                        with skt->rw_lock = r or w.
 * ARGUMENTS:
 *	skt      : I    pointer of corresponding Socket struct
 * RETURNS:     none
 *
 ****************************************************************************/
void set_PowerOff_timer( Socket *skt )
{
    int old;
    Adapter * adpt = &adapters[skt->AdapterIndex];
    extern void devlvlpm(struct trb * t );

    set_activity( skt );
    /*
     * This power off routine is device level power management 
     * This should not be done in PM_DEVICE_FULL_ON mode 
     */

#if defined( PM_ENABLE )
    if ( adpt->pmh.mode == PM_DEVICE_ENABLE ){
#else
    if ( 1 ){
#endif
	old = disable_lock( adpt->intr.priority, &cs_intr_lock );

	while( tstop( skt->pwrofft ) );

	skt->pwrofft->ipri = adpt->intr.priority;
	skt->pwrofft->func = devlvlpm;
	skt->pwrofft->flags = 0;
	skt->pwrofft->timeout.it_value.tv_sec  = CS_PWROFF_TIME;
	skt->pwrofft->timeout.it_value.tv_nsec = 0;
	skt->pwrofft->t_func_addr = (caddr_t)skt;

	tstart( skt->pwrofft );

	unlock_enable( old, &cs_intr_lock );
    }
}

/*****************************************************************************
 *
 * NAME:        set_activity
 *
 * FUNCTION:	Set activity flag for PM and turn on/off LED
 *
 * EXECUTION ENVIRONMENT: client thread
 *              skt->rw_lock should be locked
 * ARGUMENTS:
 *	skt      : I    pointer of corresponding Socket struct
 * RETURNS:     none
 *
 ****************************************************************************/
void set_activity( Socket * skt )
{
    Adapter * adpt = &adapters[ skt->AdapterIndex ];
    SetSocket_S setskt;
    int s;

    setskt.Socket = skt->PhySocket;
    SocketServices( adpt, PCM_GET_SOCKET, (caddr_t)&setskt );
    setskt.State  = 0;
#if defined( PM_ENABLE )
    if ( CheckInterest( skt ) ){
	if ( adpt->pmh.mode == PM_DEVICE_ENABLE ){
	    /* unset activity */
	    adpt->pmh.activity = PM_ACTIVITY_NOT_SET;
	}
    }else{
	if ( adpt->pmh.mode == PM_DEVICE_ENABLE ){
	    for( s = 0; s < adpt->numSockets && !CheckInterest( sockets[s] );
		s++ );
	    if ( s == numSockets ){
		/* set no_activity */
		adpt->pmh.activity = PM_NO_ACTIVITY;
	    }
	}
    }
#endif
    if ( skt->client_Vcc && skt->client_LED == _CSaixTurnOnLED ){
	/* turn on LED */
	setskt.CtlInd = skt->CtlIndCaps & PCM_SBM_BUSY;
    }else{
	/* trun off LED */
	setskt.CtlInd &= ~PCM_SBM_BUSY;
    }
    SocketServices( adpt, PCM_SET_SOCKET, (caddr_t)&setskt );
}

/*****************************************************************************
 *
 * NAME:	CheckInterest
 *
 * FUNCTION:	Check if some clients are interested in the Card in the 
 *              Specified Socket.
 * EXECUTION ENVIRONMENT: client thread
 *                        skt->rw_lock must be locked.
 * ARGUMENTS:
 *	skt        : pointer to Socket struct
 * RETURNS:
 *	0          : no clients is interested in the Card in the socket
 *      CSR_IN_USE : some clients is interested in the Card in the socket
 *
 ****************************************************************************/
int CheckInterest( Socket * skt )
{
    int      w;
    Adapter *adpt = &adapters[ skt->AdapterIndex ];
    Window  *win;
    
    if ( skt->cfg_owner || skt->irq_owner || skt->io_owner ){
	return CSR_IN_USE;
    }
    for( w = 0, win = adpt->window; w < adpt->numWindows; w++, win++ ){
	if ( win->LogSocket == skt->LogSocket ){
	    if ( win->ownerID == CSID || win->ownerID == 0 ){
		continue;
	    }
	    else{
		return CSR_IN_USE;
	    }
	}
    }
    return( 0 );
}

/****************************************************************************
 *
 * NAME:	tuple_search
 *
 * FUNCTION:	Search / Get the specified tuple
 *
 * EXECUTION ENVIRONMENT: config, client, event-handler and PM threads
 * ARGUMENTS:   
 *	tuple  : I/O	pointer of CSGetTplPkt / CSGetTplDataPkt
 *	cmd    : I	TPGetFirst : The first search or search from the
 *                                   head of Attribute Memory Space.
 *			TPGetNext  : Search from the specified Offset.
 *                      TPGetData  : Get tuple data from the specified Offset.
 * RETURNS:
 *      CSR_SUCCESS       : Success
 *	CSR_NO_MORE_ITEMS : No more tuple data on PC Card
 *      CSR_READ_FAILURE  : Other failures 
 *
 ****************************************************************************/
int tuple_search( CSGetTplPkt * tuple, int cmd )
{ 
    int     attr;
    int     inc;
    ulong   cis;
    int     rc = CSR_SUCCESS;
    Socket  * skt  = sockets[tuple->Socket];
    Adapter * adpt = &adapters[skt->AdapterIndex];
    Window  * win  = &adpt->window[skt->tuple_window];

    if ( cmd & TPGetFirst ){
	tuple -> Flags	    = 0;
	tuple -> LinkOffset = 0;
	tuple -> CISOffset  = 0;
    }
    
    if ( ( tuple->Flags & TP_FINISHED) != 0 )
	return( rc );
    
    attr = ( tuple->Flags & TP_COMMON ) == 0;
    cis	 = tuple->CISOffset;
    inc  = attr ? 2 : 1;
    
    simple_lock( &cs_twin_lock );
    SetPageOffset( adpt, win->PhyWindow, 0, 0, attr);
    EnableTupleWindow( adpt, skt );
    
    if ( ( cmd & TPGetFirst ) || ( cmd & TPGetNext ) )
    {
	BUGLPR( var, BUGGID, ("Scanning CIS chain in %s memory at addr %x\n",\
			      attr ? "attribute" : "common",
			      tuple -> CISOffset));

	rc = CSR_NO_MORE_ITEMS;

	/* Loop through a succession of tuples  */
	for (;;)
	{ 
	    int    code = cs_busio_getc( adpt, cis ); 
	    int    link = 0;
	    ulong  next;

	    /*
	     * Tuple code in The following rage is reserved for future use 
	     */
	    if(( 0x01 < code && code < 0x10 ) ||
	       ( 0x23 < code && code < 0x40 ) ||
	       ( 0x46 < code && code < 0x80 ) ){
		BUGLPR( var, BUGACT, ("Invalide tuple code: %x\n", code ));
		rc = CSR_READ_FAILURE;
	    }
	    
	    if (code == CISTPL_NULL || code == CISTPL_END)
		next = cis + inc;
	    else
		/*
		 *  No link field: never return to caller.
		 */
	    { 
		int     linktype = 0;
		ulong data = cis + 2 * inc;
		
		link = cs_busio_getc( adpt, cis + inc );
		next = data + link * inc;
		
		/* Process link tuples  */
		switch (code)
		{ 
		case CISTPL_LONGLINK_A:
		case CISTPL_LONGLINK_C:
		    /*
		     * Long link
		     */
		    if ( link < 4 ){
			/* Longlink block too short! */
			break;
		    }
		    else {
			tuple -> Flags |= TP_LINKSET;
			
			if (code == CISTPL_LONGLINK_C)
			    tuple -> Flags |= TP_COMMONLINK;
			tuple->LinkOffset = 
			    cs_busio_getc( adpt,   data          )       |
			    cs_busio_getc( adpt, ( data +   inc )) << 8  |
			    cs_busio_getc( adpt, ( data + 2*inc )) << 16 |
			    cs_busio_getc( adpt, ( data + 3*inc )) << 24 ;
		    }
		    linktype = 1;
		    break;
		    
		case CISTPL_NO_LINK:
		    /*
		     * No default link to common memory.
		     */
		    linktype	  = 1;
		    tuple -> Flags |= TP_NOLINK;
		    break;
		    
		case CISTPL_LINKTARGET:
		    linktype = 1;
		    break;
		    
		case CISTPL_CHECKSUM:
		    /*
		     * Parameters are offset, length and checksum.
		     * Offset and length are 16-bits, LSB first.
		     * Offset is signed.
		     */
		    if (link < 5)
		    {
			/* Checksum block is too short */
			break;
		    }
		    else
		    {
			short  offset;
			ushort i, len;
			uchar expsum, sum;

			offset = ( cs_busio_getc( adpt, data) + 
				  (cs_busio_getc( adpt, data+inc) << 8)) * inc;
			len =    cs_busio_getc( adpt, data+2*inc) +
			    ( cs_busio_getc( adpt, data+3*inc) << 8 );
			expsum = cs_busio_getc( adpt, data+4*inc);
			for (i = 0, sum = 0; i < len; i++, offset += inc)
			    sum += cs_busio_getc( adpt, cis+offset );
			
			BUGLPR( var, BUGGID, ("Checksum offset %04x, len %04x, sum = 0x%04x, expsum = 0x%04x\n", offset, len, sum, expsum ));
			
			if ( sum == cs_busio_getc( adpt, data+4*inc ) ){
			    BUGLPR( var, BUGGID, ("Checksum OK\n"));
			}
			else{
			    /* some warning should be displayed */
			    BUGLPR( var, BUGGID, ("Checksum Error\n"));
			}
		    }
		    break;
		}
		
		/* Check for match: we are not returning NULLs or ENDs. */
		if ( cmd & TPGetFirst && (tuple -> DesiredTuple == code || 
					  tuple -> DesiredTuple == 0xff) &&
		    ( ! linktype || (tuple -> Attributes & TUPLE_ATTR_RETURN_LINK) != 0))
		    /*
		      All happy: save state and return to user.
		      */
		{
		    tuple -> CISOffset = cis;
		    tuple -> TupleCode = code;
		    tuple -> TupleLink = link;
		    rc = CSR_SUCCESS;
		    break;
		}
	    }
	    
	    cmd |= TPGetFirst;
	    /* Not matched: round again */
	    if (code == CISTPL_END || link == 0xff)
	    {
		uint flags = tuple->Flags;
		if ((flags & TP_LINKSET) == 0 && (flags & TP_NOLINK) != 0 )
		    /*
		     *  No link to follow.
		     */
		    break;
		/* Switch to new area and check link target */
		attr = (tuple->Flags & TP_COMMONLINK) == 0;
		inc  = attr ? 2 : 1;
		
		DisableTupleWindow( adpt, skt );
		SetPageOffset( adpt, win->PhyWindow, 0, 0, attr );
		EnableTupleWindow( adpt, skt );
		cis	 = tuple->LinkOffset;
		if ( cs_busio_getc( adpt, cis ) != CISTPL_LINKTARGET ||
		    cs_busio_getc( adpt, cis + inc ) < 3 ||
		    cs_busio_getc( adpt, cis + 2*inc ) != 'C' ||
		    cs_busio_getc( adpt, cis + 3*inc ) != 'I' ||
		    cs_busio_getc( adpt, cis + 4*inc ) != 'S' )
		{
		    /* Invalid LINKTARGET : terminating scan */
		    break;
		}
		/* Reset flags */
		tuple -> Flags = TP_NOLINK | (attr ? 0 : TP_COMMON);
	    }
	    /* Move to next sequential tuple */
	    else
		cis = next;
	}
    } /* if( cmd & TPGetFirst ) || ( cmd & TPGetNext ) )*/

    if ( ( rc == CSR_SUCCESS ) && ( cmd & TPGetData ) )
    {
	int len  = cs_busio_getc( adpt, cis + inc );
	int off  = 2 * inc;
	int i;
	CSGetTplDataPkt *tpldata = ( CSGetTplDataPkt * )tuple;
	
	tpldata->TupleDataLen = len;
	len -= tpldata->TupleOffset;
	off += tpldata->TupleOffset * inc;
	
	if (len < 0)
	    len = 0;
	
	if (len > tpldata->TupleDataMax)
	    len = tpldata->TupleDataMax;
	
	for (i = 0 ; i < len; i++, off += inc)
	    tpldata->TupleData[i] = cs_busio_getc( adpt, cis + off );
	
	rc = CSR_SUCCESS;
    }

    DisableTupleWindow( adpt, skt );
    simple_unlock( &cs_twin_lock );
    
    return( rc );
}
    
    
/*****************************************************************************
 *
 * NAME:        ValidateSocket
 *
 * FUNCTION:	Validate Logical soket number and return
 *              pointer to corresponding Socket struct
 * EXECUTION ENVIRONMENT: process environment
 * ARGUMENTS:
 *	LogSocket: I    Logical Socket Number
 *	skt      : O    pointer of corresponding Socket struct
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_SOCKET		: Socket is invalid
 *
 ****************************************************************************/
int ValidateSocket( int LogSocket, Socket ** skt )
{
    if ( LogSocket >= MAX_SOCKET || LogSocket < 0
	|| ( *skt = sockets[ LogSocket ] ) == NULL ){
	return( CSR_BAD_SOCKET );
    }
    return ( CSR_SUCCESS );
}

/*****************************************************************************
 *
 * NAME:        ValidateWindow
 *
 * FUNCTION:	Validate Logical windowt number and set
 *              pointer to corresponding Socket struct
 * EXECUTION ENVIRONMENT: process environment
 * ARGUMENTS:
 *	win      : I    Window ID
 * RETURNS:
 *      CSR_SUCCESS             : Window ID is valid
 *	CSR_BAD_SOCKET		: Window ID is invalid
 *
 ****************************************************************************/
int ValidateWindow( Window ** win )
{
    int w;

    for ( w = 0; w < MAX_WINDOW 
	 && (windows[w] == NULL || windows[w] != *win ); w++);
    if ( w == MAX_WINDOW ){
        return( CSR_BAD_HANDLE );
    }
    return ( CSR_SUCCESS );
}


/*****************************************************************************
 *
 * NAME:        ControlLED
 *
 * FUNCTION:	The interface to control LED ( PCMCIA activity lamp ) for 
 *              CS clients. ( Subfunction of CSVendorSpecific() )
 * EXECUTION ENVIRONMENT:  client thead
 * ARGUMENTS:
 *      sktt     : I    pointer to Socket struct;
 *	cmd      : I    _CSaixTurnOnLED  : to turn on  LED
 *                      _CSaixTurnOffLED : to turn off LED
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_SOCKET		: Fail to turn on/off LED
 *
 ****************************************************************************/
int ControlLED( Socket* skt, int cmd )
{
    int rc;
    SetSocket_S setskt;
    Adapter* adpt;
  
    adpt = adapters + skt->AdapterIndex;
    setskt.Socket = skt->PhySocket;

    if ( rc = SocketServices( adpt, PCM_GET_SOCKET, (caddr_t)&setskt )){
	return ( rc );
    }
    if( cmd  == _CSaixTurnOnLED ){
	setskt.CtlInd |= skt->CtlIndCaps & PCM_SBM_BUSY;
    }else{
	setskt.CtlInd &= ~PCM_SBM_BUSY;
    }
    setskt.State = 0;
    rc = SocketServices( adpt, PCM_SET_SOCKET, (caddr_t)&setskt );
    
    return( rc );
}
