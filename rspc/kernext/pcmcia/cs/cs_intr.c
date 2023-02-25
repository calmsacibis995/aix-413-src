static char sccsid[] = "@(#)29	1.6  src/rspc/kernext/pcmcia/cs/cs_intr.c, pcmciaker, rspc41J, 9521A_all 5/24/95 00:09:58";
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: 
 *	cs_intr2, cs_intr, ConvertPower, devlvlpm, SocketServices
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

#include <sys/device.h>
#include <sys/intr.h>
#include <sys/pin.h>
#include <sys/timer.h>
#include <sys/sleep.h>
#include <sys/m_param.h>
#include <sys/ioacc.h>
#include <sys/pcmciass.h>
#include <sys/pcmciacs.h>
#include "cs.h"

int numAdapters = -1;
int numSockets  = 0;
int numWindows  = 0;

Adapter *adapters;
Socket  *sockets[MAX_SOCKET];
Window  *windows[MAX_WINDOW];

Complex_lock cs_cfg_lock;
Simple_lock  cs_twin_lock;
Simple_lock  cs_intr_lock;
Simple_lock  cs_io_lock;

int (*CSfunc[CSF_MAX])();

struct trb * OLintr;

/*****************************************************************************
 *
 * NAME:	cs_intr2
 *
 * FUNCTION:	Second level (off level) interrupt handler.
 *			
 * EXECUTION ENVIRONMENT:  interrupt
 * ARGUMENT: t : pointer of struct trb
 *               
 * RETURNS:	INTR_SUCC : Success
 *              INTR_FAIL : AcknowledgeInterrupt Fail
 *
 ****************************************************************************/
static int cs_intr2( struct trb* t )
{
    int a, s, old;
    unsigned int m;
    Adapter * adpt;

    old = disable_lock( adapters->intr.priority, &cs_intr_lock );

    for( a = 0; a < numAdapters; a++ ){
	adpt = &adapters[a];
	for( m = 1, s = 0; s < adpt->numSockets; s++, m <<= 1 ){
	    if( adpt->ackintr & m ){
		IntrEnqueue( EVH_HWNOTIFY, adpt->socket[s].LogSocket );
	    }
	}
	adpt->ackintr = 0;
    }

    unlock_enable( old, &cs_intr_lock );
    
    return( INTR_SUCC );
}

/*****************************************************************************
 *
 * NAME:	cs_intr
 *
 * FUNCTION:	First level interrupt handler.
 *
 * EXECUTION ENVIRONMENT: interrupt
 * ARGUMENT:	ihsptr : pointer of interrupt structure
 * RETURNS:	INTR_SUCC : Success
 *
 ****************************************************************************/

int cs_intr( struct intr* ihsptr )
{
    int rc, ret, old, m, s;
    AcknowledgeInterrupt_S ackintr;
    GetSocket_S getskt;
    Adapter * adpt = (Adapter *)ihsptr;
    
    ret = INTR_FAIL;
    old = disable_lock( ihsptr->priority, &cs_intr_lock );

    if( !(rc = adpt->ddioctl( adpt->PhyAdapter, PCM_ACK_INTERRUPT, &ackintr,
		 	    DKERNEL, 0, 0 )) && ackintr.Sockets != 0 ){
	adpt->ackintr |= ackintr.Sockets;
	
	for( m = 1, s = 0; s < adpt->numSockets; s++, m <<= 1 ){
	    if( ackintr.Sockets & m ){
		getskt.Socket = s;
		if( !(rc = adpt->ddioctl( adpt->PhyAdapter, PCM_GET_SOCKET,
					 &getskt, DKERNEL, 0, 0 ))
		   && ( getskt.State & CSStatDetected ) ){
		    (adpt->socket[s].critical_state)++;
		}
	    }
	}

    	while( tstop( OLintr ));

    	OLintr->ipri  = ihsptr->priority; 
    	OLintr->func  = (void (*)())cs_intr2;
    	OLintr->flags = 0;
    	OLintr->timeout.it_value.tv_sec = 0;
    	OLintr->timeout.it_value.tv_nsec = 100 * 1000 * 1000; /* 100ms */

    	tstart( OLintr );

	ret = INTR_SUCC;
    }

    unlock_enable( old, &cs_intr_lock );
    return( ret );
}

/*****************************************************************************
 *
 * NAME:	ConvertPower
 *
 * FUNCTION:	Convert Power to Power Index for SocketServices
 *			
 * EXECUTION ENVIRONMENT: config or client thread
 * ARGUMENT:
 *	adpt   : pointer of Adapter
 *	power  : 0 - 255 ( to represent volage 0 - 25.5 volt )
 *	signal : Vcc, Vpp1 or Vpp2
 * RETURNS:
 *	p ( p >= 0 ) : power index
 *	-1           : invalid power level
 *
 ****************************************************************************/
int ConvertPower( Adapter * adpt, int power, int signal )
{
    int p;
    
    for(p = 0; p < adpt->numPwrEntries; p++ ){
	if ( adpt->PwrEntry[p].PowerLevel == power &&
	    adpt->PwrEntry[p].ValidSignals & signal ){
	    return( p );
	}
    }
    return( -1 );
}

/*****************************************************************************
 *
 * NAME:        devlvlpm
 *
 * FUNCTION:	entry pointer of off level interrupt handler to power off
 *              the specified socket as device level power management
 *
 * EXECUTION ENVIRONMENT: off level interrupt (OFFLEVEL0)
 * ARGUMENTS:
 *	t       : I    pointer to timer struct
 *                     pointer to skt struct is saved in t->t_func_data
 * RETURNS:     none
 *
 ****************************************************************************/
void devlvlpm(struct trb * t )
{
    Adapter * adpt;
    Socket * skt = (Socket *) t->t_func_addr;
    int rc, old;
    int Vcc, Vpp1, Vpp2;
    SetSocket_S setskt;
    
    if( lock_islocked( &skt->rw_lock ) ){
	skt->pwrofft->timeout.it_value.tv_sec  = CS_PWROFF_TIME;
	skt->pwrofft->timeout.it_value.tv_nsec = 0;
        tstart( t );
        return;
    }

    adpt = &adapters[skt->AdapterIndex];

    if(( Vcc  = ConvertPower( adpt, 0, PCM_VCC  ) ) == -1 ||
       ( Vpp1 = ConvertPower( adpt, 0, PCM_VPP1 ) ) == -1 ||
       ( Vpp2 = ConvertPower( adpt, 0, PCM_VPP2 ) ) == -1 ){

        return;
    }
    setskt.Socket      = skt->PhySocket;
    setskt.SCIntMask   = skt->SCIntCaps & PCM_SBM_CD;
    setskt.VccLevel    = 0xff & Vcc;
    setskt.VppLevels   = (0xf0 & (Vpp2 << 4)) | (0x0f & Vpp1);
    setskt.State       = 0;
    setskt.CtlInd      = 0;
    setskt.IREQRouting = 0;
    setskt.IFType      = PCM_IF_MEMORY;

    old = disable_lock( adapters->intr.priority, &cs_intr_lock );    
    if (!(rc = adpt->ddioctl( adpt->PhyAdapter, PCM_SET_SOCKET,
			     (caddr_t)&setskt, DKERNEL, 0, 0 ))){

        skt->cfgInfo.Vcc     = 0;
	skt->cfgInfo.Vpp1    = 0;
	skt->cfgInfo.Vpp2    = 0;
    }
    unlock_enable( old, &cs_intr_lock );

    return;
}


/*****************************************************************************
 *
 * NAME:        SocketServices
 *
 * FUNCTION:	Call Socket Services entry point directry which located in 
 *              pcmciasspin module ( not the entry point for fpioctl )
 * EXECUTION ENVIRONMENT: process and interrupt environment
 * ARGUMENTS:
 *      adpt     : I    pointer to Adapter struct;
 *	cmd      : I    sub function of Socket Services
 *	buf      : I/O  pointer to data struct corresponding to the specified
 *                      command.
 * RETURNS:
 *      CSR_SUCCESS             : Success
 *	CSR_BAD_SOCKET		: Socket is invalid
 *
 ****************************************************************************/
int SocketServices( Adapter * adpt, int cmd, caddr_t buf )
{
    int old, rc;

    BUGLPR(var, BUGGID, ("SS_CMD ( 0x%x )\n", cmd));

    /* 
     * PCM_SET_SOCKET is also called from offlevel interrupt routine
     */
    if ( cmd == PCM_SET_SOCKET || cmd == PCM_GET_SOCKET ){
	old = disable_lock( adapters->intr.priority, &cs_intr_lock );
	if ( rc = adpt->ddioctl( adpt->PhyAdapter, cmd, buf, DKERNEL,0,0 )){
	    rc = ( int )*buf; /* rc should be first member */
	    BUGLPR(var, BUGACT, ("SS_CMD ( 0x%x ) : Fail with 0x%x\n",
				 cmd, rc));
	}
	unlock_enable( old, &cs_intr_lock );
    }
    else {
	if ( rc = fp_ioctl( adpt->fp, cmd, buf ) ){
	    rc = ( int )*buf; /* rc should be first member */
	    BUGLPR(var, BUGACT, ("SS_CMD ( 0x%x ) : Fail with 0x%x\n",
				 cmd, rc));
	}
    }

    return ( rc );
}

