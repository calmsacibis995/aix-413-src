static char sccsid[] = "@(#)77	1.4  src/rspc/kernext/pcmcia/ssisa/pcm_pin.c, pcmciaker, rspc41J, 9523A_all 6/6/95 05:16:10";
/*
 * COMPONENT_NAME: PCMCIAKER
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "pcm_inc.h"
#include "pcm_ss_data.h"

/* static array containing pointer to adapter information */
struct adapter_def *adapter_ptrs = NULL;

/* an variable to disable interrupt */
int pcmcia_intr_prior = INTCLASS2 ;

#ifdef DEBUG
int pcmcia_debug = 0;
#endif 

/****************************************************************************
 *
 * NAME: pcmcia_ioctl_intr
 *
 * FUNCTION: PCMCIA Socket Service ioctl routine for interrupt
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by both a process and an interrupt.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EACCESS  Re-open with diag-mode is prohibited.
 *     EINVAL   An invalid parameter is specified.
 *     EIO      lockl fails. The adapter is not initialized.
 *
 ***************************************************************************/
int
pcmcia_ioctl_intr( dev_t devno, int op, int arg, ulong devflag,
				  int chan, int ext )
{
    struct adapter_def *ap;
    struct pcmcia_ioreg ioreg;
    int     rc;
	int     ret_code = 0; /* initial no errors */
    int     slot;
    uint    mem_addr_start;
    uint    mem_addr_stop;
    uint    mem_addr_width;
    uint    offset_addr;

#ifdef DEBUG
	if (pcmcia_debug)
		printf("Entering pcmcia_ioctl_intr( op = 0x%X )\n", op);
#endif

    DDHKWD5(HKWD_PCMCIA_SS, DD_ENTRY_IOCTL, ret_code, devno, op, devflag,
            chan, ext);

    /* search adapter list for this devno */
    ap = adapter_ptrs;

    while (ap != NULL) {
        if (ap->devno == devno){
            break;
        }
        ap = ap->next;
    }   /* endwhile */

    if ((ap == NULL) || (! ap->opened)){
	ret_code = EIO; /* error--adapter not inited */
	goto end;
    }

    switch ( op )
    {
    case PCM_ACK_INTERRUPT:
	ret_code = pcmcia_AcknowledgeInterrupt( ap, arg, devflag );
	break;
    case PCM_GET_SOCKET:
	ret_code = pcmcia_GetSocket( ap, arg, devflag );
	break;
    case PCM_SET_SOCKET:
	ret_code = pcmcia_SetSocket( ap, arg, devflag );
	break;
    default:
        ret_code = EINVAL;
    } /* end switch */

 end:

#ifdef DEBUG
    if (pcmcia_debug)
		printf("Exiting pcmcia_ioctl_intr() = %d\n", ret_code);
#endif

    DDHKWD1(HKWD_PCMCIA_SS, DD_EXIT_IOCTL, ret_code, devno);
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_AcknowledgeInterrupt
 *
 * FUNCTION: PCMCIA Socket Service AcknowledgeInterrupt
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by both a process and an interrupt.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *
 ***************************************************************************/
int
pcmcia_AcknowledgeInterrupt(
struct adapter_def *ap,
int arg,
int devflag )
{
    AcknowledgeInterrupt_S data;
	struct pcmcia_ioreg ioreg;
	int rc, ret_code = 0, socket;
	uchar ack_status;

	/* initialize the return values to zero */
	bzero( & data, sizeof( AcknowledgeInterrupt_S ));
	
	for ( socket = 0; socket < ap->num_sockets; socket++)
	{
		/* Card Status Change Register (0x04) */
		ioreg.index = ioreg_index( PCM_STAT_CHG, socket );
		rc = pcmcia_ioreg( ap, PCM_IOREAD, & ioreg );
		if ( rc ){
			data.retcode = PCM_SUCCESS;
			ret_code = EIO;
			goto end;
		}
		
		/* preserve the status change to the internal variable */

		ack_status = 
			( ioreg.data & BIT_3 ? PCM_SBM_CD       : 0 )
			| ( ioreg.data & BIT_2 ? PCM_SBM_RDYBSY : 0 )
			| ( ioreg.data & BIT_1 ? PCM_SBM_BVD2   : 0 )
			| ( ioreg.data & BIT_0 ? PCM_SBM_BVD1   : 0 ) ;

		ap->socket[socket].status_change |= ack_status;		

		data.Sockets |= ioreg.data & 0x0F ? pcmcia_itob(socket) : 0 ;
	}

 end:
	/* return data to user space */
	rc = pcmcia_copyout( & data, (char *) arg,
						 sizeof( AcknowledgeInterrupt_S ),
						 devflag );
	if ( rc ){
		ret_code = EFAULT;
	}
    DDHKWD2(HKWD_PCMCIA_SS, PCM_ACK_INTERRUPT, ret_code,
			ap->devno,
			pcmcia_word( data.Sockets , ack_status, 0, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_GetAdapter
 *
 * FUNCTION: PCMCIA Socket Service GetAdapter
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int pcmcia_GetAdapter( struct adapter_def *ap, int arg, int devflag )
{
    GetAdapter_S data;
    struct pcmcia_ioreg ioreg;
    int ret_code = 0, rc;
    
    /* initialize the return values to zero */
    bzero( &data, sizeof( GetAdapter_S ));
    
    /* Global Control Register (0x1E) */
    ioreg.index = ioreg_index( PCM_GLOBAL_CTRL, 0 );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    data.State = 
	( ioreg.data & BIT_0 ? PCM_AS_POWERDOWN : 0 )
	    | ( ap->as_maintain ? PCM_AS_MAINTAIN  : 0 );
	
    /* Card Status Change Inerrupt Config Register (0x05) */
    ioreg.index = PCM_STAT_CHG_INTR;
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    data.SCRouting = 
        (ioreg.data & 0xF0) >> 4 /* Bit 7-4 IRQ Level Selection */
			| ( ioreg.data & 0x0F ? PCM_IRQ_ENABLE : 0 ) ;
	/* If IRQ is set, then set IRQ_ENABLE bit */
	/* IRQ_HIGH bit is zero */
    data.retcode = PCM_SUCCESS;
	
 end:
    /* return data to user space */
    rc = pcmcia_copyout( &data, (char *) arg, sizeof( GetAdapter_S),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD3(HKWD_PCMCIA_SS, PCM_GET_ADAPTER, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.State, data.SCRouting, 0, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_SetAdapter
 *
 * FUNCTION: PCMCIA Socket Service SetAdapter
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int pcmcia_SetAdapter( struct adapter_def *ap, int arg, int devflag )
{
    SetAdapter_S data;
    struct pcmcia_ioreg ioreg;
    int socket;
    int rc, ret_code = 0;
	
    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof( SetAdapter_S ),
					   devflag );
    if ( rc ){
        return EFAULT;
    }
	
    /* check IRQ */
    if ( (data.SCRouting & PCM_IRQ_ENABLE)
	&& (pcmcia_itob(data.SCRouting & 0x0F) & IRQ_MAP) == 0x00 )
    {
	data.retcode = PCM_BAD_IRQ;
	ret_code = EINVAL;
	goto end;
    }

    /* handle PCM_AS_POWERDOWN and PCM_AS_MAINTAIN */
    if ( data.State & PCM_AS_POWERDOWN ){
	/* power down */
	ioreg.index = ioreg_index( PCM_GLOBAL_CTRL, 0 );
	ioreg.data = BIT_0;
	if( ap->ddi.int_mode == 'I' ) ioreg.data |= BIT_1;
	rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
	
	if ( data.State & PCM_AS_MAINTAIN ){
	    for ( socket = 0; socket < ap->num_sockets; socket++ ){
		pcmcia_store_registers( ap, socket );
	    }
	    ap->as_maintain = PCM_AS_MAINTAIN;
	}
	else if ( ap->as_maintain ){
	    for ( socket = 0; socket < ap->num_sockets; socket++ ){
		pcmcia_restore_registers( ap, socket );
	    }
	    ap->as_maintain = 0;
	}
    }
    else{
	/* restore context of registers */
	if ( ap->as_maintain ){
	    for ( socket = 0; socket < ap->num_sockets; socket++ ){
		pcmcia_restore_registers( ap, socket );
	    }
	    ap->as_maintain = 0;
	}
	/* power up */
	ioreg.index = ioreg_index( PCM_GLOBAL_CTRL, 0 );
	ioreg.data = 0;
	if( ap->ddi.int_mode == 'I' ) ioreg.data |= BIT_1;
	rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    }
	
    /* if IRQ is enabled */
	for ( socket = 0; socket < ap->num_sockets; socket++ ){
		/* Catd Status Change Interrupt Configuration Register (0x05) */
		ioreg.index = ioreg_index( PCM_STAT_CHG_INTR, socket );
		if ( data.SCRouting & PCM_IRQ_ENABLE )
		{
			ioreg.data = ( data.SCRouting & 0x0F ) << 4
				| (ap->socket[socket].SCIntMask & PCM_SBM_CD     ? BIT_3 : 0)
					| (ap->socket[socket].SCIntMask & PCM_SBM_RDYBSY ? BIT_2 : 0)
						| (ap->socket[socket].SCIntMask & PCM_SBM_BVD2   ? BIT_1 : 0)
							| (ap->socket[socket].SCIntMask & PCM_SBM_BVD1   ? BIT_0 : 0) ;
		} else {
			ioreg.data = 0x00;
		}
		rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
		if ( rc ){
			data.retcode = PCM_SUCCESS;
			ret_code =  EIO;
			goto end;
		}
	}
	
    data.retcode = PCM_SUCCESS;
	
 end:
    /* return the data to the user space */
    rc = pcmcia_copyout( &data, (char *) arg, sizeof( SetAdapter_S ),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD3(HKWD_PCMCIA_SS, PCM_SET_ADAPTER, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.State, data.SCRouting, 0, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_GetSocket
 *
 * FUNCTION: PCMCIA Socket Service GetSocket
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EINVAL   An invalid parameter is specified.
 *     EIO      An I/O error is detected.
 *     EFAULT   Data copy failed between user and kernel space.
 *
 ***************************************************************************/
int pcmcia_GetSocket( struct adapter_def *ap, int arg, int devflag )
{
    GetSocket_S data;
    struct pcmcia_ioreg ioreg;
    uchar new_state;
    int old_intr_prior;
    int rc, ret_code = 0;
	
    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof( GetSocket_S ),
					   devflag );
    if ( rc ){
        return EFAULT;
    }
	
    if ( data.Socket < 0 || ap->num_sockets <= data.Socket ){
        data.retcode = PCM_BAD_SOCKET;
        ret_code = EINVAL;
        goto end;
    }
	
    /* SCIntMask */
    /* Card Status Change Interrupt Configuration Register (0x05) */
    data.SCIntMask = ap->socket[data.Socket].SCIntMask;
	
    /* Power */
    /* Power and Resetdrv Control Register (0x02) */
    ioreg.index = ioreg_index( PCM_PWR_RESETDRV, data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    /* VccLevel */
    data.VccLevel = ioreg.data & BIT_4 ? 1 : 0 ;
    /* VppLevels */
    data.VppLevels  = ioreg.data & ( BIT_0 | BIT_1 );
    /* if RICHO pcic, then Vpp2 is same as Vpp1 */
    if ( (ioreg.data & ( BIT_2 | BIT_3 )) == ( BIT_2 | BIT_3 )){
	data.VppLevels |= ( ioreg.data & ( BIT_0 | BIT_1 )) << 4;
    } else {
        data.VppLevels |= ( ioreg.data & ( BIT_2 | BIT_3 )) << 2;
    }
	
    /* State */
    /* Interface Status Register (0x01) */
    ioreg.index = ioreg_index( PCM_IF_STAT, data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    new_state =
        ( ioreg.data & BIT_5 ? PCM_SBM_RDYBSY : 0 )
			| ( ioreg.data & BIT_4 ? PCM_SBM_WP     : 0 )
				| ( ( ioreg.data & BIT_3 &&
				      ioreg.data & BIT_2 ) ? PCM_SBM_CD     : 0 )
					| ( ioreg.data & BIT_1 ? PCM_SBM_BVD2   : 0 )
						| ( ioreg.data & BIT_0 ? PCM_SBM_BVD1   : 0 ) ;
	
    /* Card Status Change Register (0x04) */
    ioreg.index = ioreg_index( PCM_STAT_CHG, data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    data.State =
        ap->socket[data.Socket].status_change
			| pcmcia_xor( new_state , ap->socket[data.Socket].old_state )
				| ( ioreg.data & BIT_3 ? PCM_SBM_CD     : 0 )
					| ( ioreg.data & BIT_2 ? PCM_SBM_RDYBSY : 0 )
						| ( ioreg.data & BIT_1 ? PCM_SBM_BVD2   : 0 )
							| ( ioreg.data & BIT_0 ? PCM_SBM_BVD1   : 0 ) ;
	
	/* save the status */
	ap->socket[data.Socket].old_state = new_state;
	
    old_intr_prior = i_disable( pcmcia_intr_prior );
	ap->socket[data.Socket].status_change |= data.State;
    i_enable( old_intr_prior );
	
    /* CtlInd */
    data.CtlInd = ap->socket[data.Socket].CtlInd;
	
    /* IREQRouting */
    /* Interrupt and General Control Register (0x03) */
    ioreg.index = ioreg_index( PCM_INTR_CTRL, data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    data.IREQRouting =
        ioreg.data & 0x0F
			| ( ioreg.data & 0x0F ? PCM_IRQ_ENABLE : 0 ) ;
    /* IRQ_ENABLE is set if an IRQ level is set */
    /* IRQ_HIGH is zero */
	
    /* IFType */
    data.IFType = ioreg.data & BIT_5 ? PCM_IF_IO : PCM_IF_MEMORY ;
	
    data.retcode = PCM_SUCCESS;
	
 end:
    /* return data to user space */
    rc = pcmcia_copyout( &data, (char *) arg, sizeof( GetSocket_S ),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
	
    DDHKWD4(HKWD_PCMCIA_SS, PCM_GET_SOCKET, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.Socket, data.SCIntMask,
						data.VccLevel, data.VppLevels ),
			pcmcia_word( data.State, data.CtlInd,
						data.IREQRouting, data.IFType )); 
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_GetStatus
 *
 * FUNCTION: PCMCIA Socket Service GetStatus for interrupt
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by both a process and an interrupt.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int pcmcia_GetStatus( struct adapter_def *ap, int arg, int devflag )
{
    GetStatus_S data;
    struct pcmcia_ioreg ioreg;
    uchar new_state;
    int old_intr_prior;
    int rc, ret_code = 0;
    
    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof( GetStatus_S ),
					   devflag );
    if ( rc ){
        return EFAULT;
    }
	
    if ( data.Socket < 0 || ap->num_sockets <= data.Socket ){
        data.retcode = PCM_BAD_SOCKET;
        ret_code = EINVAL;
        goto end;
    }
	
    /* CardState */
    /* Interface Status Register (0x01) */
    ioreg.index = ioreg_index( PCM_IF_STAT, data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    data.CardState =
        ( ioreg.data & BIT_5 ? PCM_SBM_RDYBSY : 0 )
			| ( ioreg.data & BIT_4 ? PCM_SBM_WP     : 0 )
				| ( ( ioreg.data & BIT_3 &&
				      ioreg.data & BIT_2 ) ? PCM_SBM_CD     : 0 )
					| ( ioreg.data & BIT_1 ? PCM_SBM_BVD2   : 0 )
						| ( ioreg.data & BIT_0 ? PCM_SBM_BVD1   : 0 ) ;

    /* SocketState */
    /* Card Status Change Register (0x04) */
    ioreg.index = ioreg_index( PCM_STAT_CHG, data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        ret_code = EIO;
        goto end;
    }
    /* If polling the status change,
     * then 'Card Status Change Register (0x04)' is not set.
     * Difference of 'Interface Status Register(0x01)' is reported.
     */
    data.SocketState =
        ap->socket[data.Socket].status_change
			| pcmcia_xor( data.CardState, ap->socket[data.Socket].old_state )
				| ( ioreg.data & BIT_3 ? PCM_SBM_CD     : 0 )
					| ( ioreg.data & BIT_2 ? PCM_SBM_RDYBSY : 0 )
						| ( ioreg.data & BIT_1 ? PCM_SBM_BVD2   : 0 )
							| ( ioreg.data & BIT_0 ? PCM_SBM_BVD1   : 0 ) ;
	
    /* save the status */
    ap->socket[data.Socket].old_state     = data.CardState;
	
    old_intr_prior = i_disable( pcmcia_intr_prior );
	ap->socket[data.Socket].status_change |= data.SocketState;
    i_enable( old_intr_prior );
	
    /* CtlInd */
    data.CtlInd = ap->socket[data.Socket].CtlInd;
	
    /* IRQRouting */
    /* IFType */
    /* Interrupt and General Control Register (0x03) */
    ioreg.index = ioreg_index ( PCM_INTR_CTRL , data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    data.IREQRouting =
        ioreg.data & 0x0F
			| ( ioreg.data & 0x0F ? PCM_IRQ_ENABLE : 0 ) ;
    data.IFType = ioreg.data & BIT_5 ? PCM_IF_IO : PCM_IF_MEMORY ;
	
    data.retcode = PCM_SUCCESS;
	
 end:
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof( GetStatus_S ),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD4(HKWD_PCMCIA_SS, PCM_GET_STATUS, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.Socket, data.CardState,
						data.SocketState, data.CtlInd ),
			pcmcia_word( data.IREQRouting, data.IFType, 0, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_SetSocket
 *
 * FUNCTION: PCMCIA Socket Service SetSocket
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *     EINVAL   An invalid parameter is specified.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int pcmcia_SetSocket( struct adapter_def *ap, int arg, int devflag )
{
    SetSocket_S data;
    struct pcmcia_ioreg ioreg;
    int ret_code = 0, rc;
    int old_intr_prior;
	
	
    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof( SetSocket_S),
					   devflag );
    if ( rc ){
        return EFAULT;
    }
	
#ifdef DEBUG
    printf("SetSocket: Socket=%d, SCIntMask=0x%X, VccLevel=%d, VppLevels=%d,"
           "State=0x%X, CtlInd=0x%X, IREQRouting=0x%X, IFType=0x%X \n",
           data.Socket, data.SCIntMask, data.VccLevel, data.VppLevels,
           data.State, data.CtlInd, data.IREQRouting, data.IFType);
#endif 
	
    /* check socket number */
    if ( data.Socket < 0 && ap->num_sockets <= data.Socket ){
        data.retcode = PCM_BAD_SOCKET;
        ret_code =  EINVAL;
        goto end;
    }
	
    /* SCIntMask: is set to adapter_def for use by SetAdapter */
    /* In case the IRQ has been enabled, IntMask should be set here */ 
    ap->socket[data.Socket].SCIntMask = data.SCIntMask;
    ioreg.index = ioreg_index( PCM_STAT_CHG_INTR, data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    if ( ioreg.data & 0xF0 ){
	ioreg.data = ioreg.data & 0xF0 
	    | (data.SCIntMask & PCM_SBM_CD     ? BIT_3 : 0)
		| (data.SCIntMask & PCM_SBM_RDYBSY ? BIT_2 : 0)
		    | (data.SCIntMask & PCM_SBM_BVD2   ? BIT_1 : 0)
			| (data.SCIntMask & PCM_SBM_BVD1   ? BIT_0 : 0);
	rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
	if ( rc ){
	    data.retcode = PCM_SUCCESS;
	    ret_code =  EIO;
	    goto end;
	}
    }
	
    /* VccLevel, Vpp1Level, Vpp2Level */
    if (   data.VccLevel < 0
        || NUM_PWR_ENTRY <= data.VccLevel
        || ! ( PwrEntry[data.VccLevel].ValidSignals & PCM_VCC ) )
	{
        data.retcode = PCM_BAD_VCC;
        ret_code = EINVAL;
        goto end;
    }
    if (   NUM_PWR_ENTRY <= ( data.VppLevels & 0x0F )
        || NUM_PWR_ENTRY <= ( (data.VppLevels & 0xF0) >> 4 )
        || ! (  PwrEntry[ data.VppLevels  & 0x0F      ].ValidSignals
			  & PCM_VPP1 )
        || ! (  PwrEntry[ (data.VppLevels & 0xF0) >> 4].ValidSignals
			  & PCM_VPP2 ) )
	{
        data.retcode = PCM_BAD_VPP;
        ret_code = EINVAL;
        goto end;
    }
    ioreg.index = ioreg_index( PCM_PWR_RESETDRV, data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    /* clear bits 0-4 for Vpp1, Vpp2, and Vcc */
    ioreg.data &= 0xE0;
	
    ioreg.data = ioreg.data
		| ( data.VppLevels      & ( BIT_0 | BIT_1 ) )
			| ( data.VppLevels >> 2 & ( BIT_2 | BIT_3 ) )
				| ( data.VccLevel ?         BIT_4 : 0 )
					| BIT_6   /* Disable Resume Reset */
						| BIT_7;  /* Output Enable bit    */
	
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* State */
    /* reset status change info variable */
    old_intr_prior = i_disable( pcmcia_intr_prior );
    ap->socket[data.Socket].status_change &= ~ data.State ;
    i_enable( old_intr_prior );
	
	/* CtlInd */
	ioreg.index = ioreg_index( PCM_ACCESS_INDLAMP, data.Socket );
    if ( data.CtlInd & PCM_SBM_BUSY )
	{
		/* Access Lamp ON */
		ioreg.data = BIT_7;
		ap->socket[data.Socket].CtlInd = PCM_SBM_BUSY;
	} else {
		/* Access Lamp OFF */
		ioreg.data = 0x00;
		ap->socket[data.Socket].CtlInd = 0x00;
	}
	rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
	if ( rc ){
		data.retcode = PCM_SUCCESS;
		ret_code = EIO;
		goto end;
	}
	
    /* IREQRouting */
    /* IFType      */
    /* Interrupt and General control Register (0x03) */
    ioreg.index = ioreg_index( PCM_INTR_CTRL, data.Socket );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* if IRQ is disable */
    if ( (data.IREQRouting & PCM_IRQ_ENABLE) == 0x00 )
    {
        ioreg.data &= 0xE0;
    }
    else /* if IRQ is to be set to the adapter */
    {
        /* clear bits 0-3 for IRQ level */
        ioreg.data &= 0xE0;
        /* bits 0-4 of IREQRouting are used for IRQ level */
        if ( (pcmcia_itob( data.IREQRouting & 0x1F ) & IRQ_MAP) == 0x00 ){
            data.retcode = PCM_BAD_IRQ;
            ret_code = EINVAL;
            goto end;
        }
	/* bits 0-3 of IREQRouting are valid for the adapter */
	ioreg.data |= data.IREQRouting & 0x0F;
    }
    /* bit 5 means for IC Card Type (Memory or I/O) */
    switch ( data.IFType ){
    case PCM_IF_MEMORY:
        ioreg.data &= ~ BIT_5;
        break;
    case PCM_IF_IO:
        ioreg.data |= BIT_5 | BIT_7;
        break;
    default:
        data.retcode = PCM_BAD_TYPE;
        ret_code =  EINVAL;
        goto end;
    }
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    data.retcode = PCM_SUCCESS;
	
 end:
    /* return data to user space */
    rc = pcmcia_copyout( &data, (char *) arg, sizeof( SetSocket_S ),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
    } /* if copyout is failed, then overwrite the ret_code to EFAULT */
	
    DDHKWD4(HKWD_PCMCIA_SS, PCM_SET_SOCKET, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.Socket, data.SCIntMask,
						data.VccLevel, data.VppLevels ),
			pcmcia_word( data.State, data.CtlInd,
						data.IREQRouting, data.IFType ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_ResetSocket
 *
 * FUNCTION: PCMCIA Socket Service ResetSocket
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int pcmcia_ResetSocket( struct adapter_def *ap, int arg, int devflag )
{
    ResetSocket_S data;
    struct pcmcia_ioreg ioreg;
    int rc, ret_code = 0;
	
    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof( ResetSocket_S ),
					   devflag );
    if ( rc ){
        return EFAULT;
    }
	
#ifdef DEBUG
	printf("ResetSocket.Socket = %d \n", data.Socket);
#endif
	
    /* check socket number */
    if ( data.Socket < 0 && ap->num_sockets <= data.Socket ){
        data.retcode = PCM_BAD_SOCKET;
        ret_code = EINVAL;
        goto end;
    }
	
    /* Interrupt and General Control Register (0x03) */
    ioreg.index = ioreg_index( PCM_INTR_CTRL, data.Socket );
    /* bit 7 Ring Indicator Enable : 0 */
    /* bit 6 IC Card Reset         : 0 (RESET active) */
    /* bit 5 IC Card Type          : 0 (Memory) */
    /* bit 4 INTR Enable           : 0 */
    /* bit 3-0 IRQ Level Selection : 0000 */
    ioreg.data = 0x00;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, & ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* Address Window Enabel Register (0x06) */
    ioreg.index = ioreg_index( PCM_WINDOW_ENABLE, data.Socket );
    /* all windows are disabled */
    ioreg.data = 0x20;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* Power Off */
    /* Power and Resetdrv Control Register (0x02) */
    ioreg.index = ioreg_index( PCM_PWR_RESETDRV , data.Socket );
    /* bit 7 Output Enable */
    /* bit 6 Disable Resume Reset */
    /* bit 5 Auto Power Switch Enable */
    /* bit 4 IC Card Power Enable */
    /* bit 3-2 IC Card Vpp2 Power Control Field */
    /* bit 1-0 IC Card Vpp1 Power Control Field */
    ioreg.data = BIT_0 | BIT_2 | BIT_4 | BIT_6;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }

    pcmcia_wait_ms( 300 ); /* wait for 300ms because Power voltage may */
                           /* be changed. Then state moves to low-Z. */
	
    /* wait 300ms */
    /* pcmcia_wait_ms(300); */
	
    /* Power On */
    /* Power and Resetdrv Control Register (0x02) */
    ioreg.index = ioreg_index( PCM_PWR_RESETDRV , data.Socket );
    /* bit 7 Output Enable */
    /* bit 6 Disable Resume Reset */
    /* bit 5 Auto Power Switch Enable */
    /* bit 4 IC Card Power Enable */
    /* bit 3-2 IC Card Vpp2 Power Control Field */
    /* bit 1-0 IC Card Vpp1 Power Control Field */
    ioreg.data = BIT_0 | BIT_2 | BIT_4 | BIT_6 | BIT_7 ;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* SS does not have a responsibility to wait */
    /* This should be done by the client: CS */
    /* wait 300ms */
    /* pcmcia_wait_ms(300); */

    pcmcia_wait_ms( 20 ); /* create 20msec reset pulse after low-Z state */
	
    /* return from reset state to normal state */
    /* Interrupt and General Control Register (0x03) */
    ioreg.index = ioreg_index( PCM_INTR_CTRL, data.Socket );
    /* bit 6 IC Card Reset : 1 (RESET inactive) */
    ioreg.data = BIT_6;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, & ioreg );
    if ( rc ){
        data.retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    data.retcode = PCM_SUCCESS;
    
 end:
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof( ResetSocket_S ),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD3(HKWD_PCMCIA_SS, PCM_RESET_SOCKET, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.Socket, 0, 0, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_GetWindow
 *
 * FUNCTION: PCMCIA Socket Service GetWindow
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *     EIO      An I/O error is detected.
 *
 * CALLS:
 *     pcmcia_GetWindow_mem
 *     pcmcia_GetWindow_io
 *
 ***************************************************************************/
int pcmcia_GetWindow( struct adapter_def *ap, int arg, int devflag )
{
    GetWindow_S data;
    int ret_code = 0, rc;
	
    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof(GetWindow_S),
					   devflag );
    if ( rc ){
        return EFAULT;
    }

	/* check Window */
	if ( NUM_WINDOWS * ap->num_sockets <= data.Window ){
		data.retcode = PCM_BAD_WINDOW;
		ret_code = EINVAL;
	}
	
    if ( Window[data.Window % NUM_WINDOWS].WndCaps & PCM_WC_IO ){
        /* I/O Window */
        ret_code =  pcmcia_GetWindow_io( ap, & data, devflag );
    } else {
        /* System Memory Window */
        ret_code = pcmcia_GetWindow_mem( ap, & data, devflag );
    }
	
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof(GetWindow_S),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD4(HKWD_PCMCIA_SS, PCM_GET_WINDOW, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.Window, data.Socket, 0, data.Size ),
			pcmcia_word( data.State, data.Speed, 0, data.Base ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_GetWindow_mem
 *
 * FUNCTION: PCMCIA Socket Service GetWindow for system memory window
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int
pcmcia_GetWindow_mem(
struct adapter_def *ap, GetWindow_S * data_ptr, int devflag )
{
    struct pcmcia_ioreg ioreg;
    uint stop_addr;
    int ret_code = 0, rc;
	
    data_ptr->Base = 0;
    data_ptr->State = 0;
    data_ptr->Socket = pcmcia_win2socket(data_ptr->Window);
	
    /* System Memory Address Start Low Byte Register */
    ioreg.index = ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index,
							  pcmcia_win2socket(data_ptr->Window));
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    data_ptr->Base |= ioreg.data & 0xFF;
	
    /* System Memory Address Start High Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 1,
					pcmcia_win2socket(data_ptr->Window));
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    data_ptr->Base |= ioreg.data << 8 & 0x0F00;
    data_ptr->State |= ioreg.data & BIT_7 ? PCM_WS_16BIT : 0 ;
	
    /* System Memory Address Stop Low Register */
    ioreg.index = 
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 2,
					pcmcia_win2socket(data_ptr->Window));
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    stop_addr = 0;
    stop_addr |= ioreg.data & 0xFF;
    
    /* System Memory Address Stop High Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 3,
					pcmcia_win2socket(data_ptr->Window));
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
		ret_code = EIO;
		goto end;
    }
    stop_addr |= ioreg.data << 8 & 0x0F00;
	
    data_ptr->Size = stop_addr - data_ptr->Base + 1;
	
	/* get wait bits and convert them to Device Speed Codes */
	rc = pcmcia_get_speed( ioreg.data, & data_ptr->Speed );
	if ( rc ){
		data_ptr->retcode = PCM_SUCCESS;
		ret_code  = EIO;
		goto end;
	}
	
    /* Address Window Enable Register (0x06) */
    ioreg.index =
        ioreg_index( PCM_WINDOW_ENABLE, pcmcia_win2socket(data_ptr->Window) );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    data_ptr->State |=
		ioreg.data & Window[data_ptr->Window % NUM_WINDOWS].EnableBit 
			? PCM_WS_ENABLED : 0 ;
	
    /* in case of TOMCAT, Wait State Bits are set to 00 */
	
    data_ptr->retcode = PCM_SUCCESS;
	
 end:
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_GetWindow_io
 *
 * FUNCTION: PCMCIA Socket Service GetWindow for I/O window
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int
	pcmcia_GetWindow_io(
						struct adapter_def *ap, GetWindow_S *data_ptr, int devflag )
{
    struct pcmcia_ioreg ioreg;
    uint stop_addr;
    int rc, ret_code = 0;
	
    /* Socket */
    data_ptr->Socket = pcmcia_win2socket(data_ptr->Window);
	
    /* I/O Address Start Low Byte Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index,
					pcmcia_win2socket(data_ptr->Window));
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code =  EIO;
        goto end;
    }
    data_ptr->Base = ioreg.data & 0xFF;
	
    /* I/O Address Start High Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 1,
					pcmcia_win2socket(data_ptr->Window));
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    data_ptr->Base |= ( ioreg.data & 0xFF ) << 8;
	
    /* I/O Address Stop Low Register */
    ioreg.index = 
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 2,
					pcmcia_win2socket(data_ptr->Window) );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    stop_addr = ioreg.data & 0xFF;
    
    /* I/O Address Stop High Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 3,
					pcmcia_win2socket(data_ptr->Window) );
    /* in case of TOMCAT, Wait State Bits are set to 00 */
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    stop_addr |= ( ioreg.data << 8 ) & 0xFF00;
	
    data_ptr->Size = stop_addr - data_ptr->Base + 1 ;
	
    /* State */
    data_ptr->State = PCM_WS_IO ;
	
    /* Address Window Enable Register (0x06) */
    ioreg.index =
        ioreg_index( PCM_WINDOW_ENABLE, pcmcia_win2socket(data_ptr->Window) );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    data_ptr->State |=
		ioreg.data & Window[data_ptr->Window % NUM_WINDOWS].EnableBit
			? PCM_WS_ENABLED : 0 ;
	
    /* I/O Control Register (0x07) */
    ioreg.index =
		ioreg_index( PCM_IO_CTRL, pcmcia_win2socket(data_ptr->Window) );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    if ( Window[data_ptr->Window % NUM_WINDOWS].Index == 0x08 
        || Window[data_ptr->Window % NUM_WINDOWS].Index == 0x48 )
    {
        /* I/O Window 0 */
        data_ptr->State |= ioreg.data & BIT_0 ? PCM_WS_16BIT : 0 ;
    } else {
        /* I/O Window 1 */
        data_ptr->State |= ioreg.data & BIT_4 ? PCM_WS_16BIT : 0 ;
    }
	
    data_ptr->retcode = PCM_SUCCESS;
	
 end:
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_SetWindow
 *
 * FUNCTION: PCMCIA Socket Service SetWindow
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *     EINVAL   An invalid argument is specified.
 *     EIO      An I/O error is detected.
 *
 * CALLS:
 *     pcmcia_SetWindow_mem
 *     pcmcia_SetWindow_io
 *
 ***************************************************************************/
int pcmcia_SetWindow( struct adapter_def *ap, int arg, int devflag )
{
    SetWindow_S data;
    int ret_code = 0, rc;
	
    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof( SetWindow_S ),
					   devflag );
    if ( rc ){
        return EFAULT;
    }

	/* check Window */
	if ( NUM_WINDOWS * ap->num_sockets <= data.Window ){
		data.retcode = PCM_BAD_WINDOW;
		ret_code = EINVAL;
	}

	ap->window[data.Window].setwindow_s = data;
	
#ifdef DEBUG
    printf("SetWindow: Window=%d, Socket=%d, Size=0x%X, State=0x%X, "
           "Speed=0x%X, Base=0x%X \n",
           data.Window, data.Socket, data.Size, data.State,
           data.Speed, data.Base );
#endif
	
    if ( data.State & PCM_WS_IO ){
        if ( Window[data.Window % NUM_WINDOWS].WndCaps & PCM_WC_IO ){
            data.retcode = PCM_BAD_ATTRIBUTE;
            ret_code = pcmcia_SetWindow_io( ap, & data, devflag );
            goto end;
        } else {
            /* This window can not be mapped as I/O window */
            data.retcode = PCM_BAD_ATTRIBUTE;
            ret_code = EINVAL;
            goto end;
        }
    } else {
        if ( Window[data.Window % NUM_WINDOWS].WndCaps 
            & ( PCM_WC_COMMON | PCM_WC_ATTRIBUTE ))
        {
            ret_code = pcmcia_SetWindow_mem( ap, & data, devflag );
            goto end;
        } else {
            data.retcode = PCM_BAD_ATTRIBUTE;
            ret_code = EINVAL;
            goto end;
        }
    }
	
 end:
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof( SetWindow_S ),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD4(HKWD_PCMCIA_SS, PCM_SET_WINDOW, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.Window, data.Socket, 0, data.Size ),
			pcmcia_word( data.State,  data.Speed,  0, data.Base ));
	
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_SetWindow_mem
 *
 * FUNCTION: PCMCIA Socket Service SetWindow for system memory window
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EIO      An I/O error is detected.
 ***************************************************************************/
int
pcmcia_SetWindow_mem(
struct adapter_def *ap, SetWindow_S * data_ptr, int devflag )
{
    struct pcmcia_ioreg ioreg;
    uint offset_addr;
    int ret_code = 0, rc;
	uchar speed_bits;
	
	/* check Device Speed Codes and convert them to wait bits */
	rc = pcmcia_put_speed( data_ptr->Speed, & speed_bits );
	if ( rc ){
		data_ptr->retcode = PCM_BAD_SPEED;
		ret_code = EINVAL;
		goto end;
	}
	
    /* Address Window Enable Register (0x06) */
    ioreg.index =
        ioreg_index( PCM_WINDOW_ENABLE, pcmcia_win2socket(data_ptr->Window) );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    /* Disable Window */
    ioreg.data &= ~Window[data_ptr->Window % NUM_WINDOWS].EnableBit;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* System Memory Address Start Low Byte Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index,
					 pcmcia_win2socket(data_ptr->Window) );
    ioreg.data  = data_ptr->Base & 0xFF;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
		data_ptr->retcode = PCM_SUCCESS;
		ret_code = EIO;
		goto end;
	}
	
    /* System Memory Address Start High Register */
    ioreg.index = 
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 1,
					 pcmcia_win2socket(data_ptr->Window) );
    ioreg.data = data_ptr->Base >> 8 & 0x0F
		| ( data_ptr->State & PCM_WS_16BIT ? BIT_7 : 0 ) ;
	/* Data Size bit */
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* System Memory Address Stop Low Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 2,
					 pcmcia_win2socket(data_ptr->Window) );
    ioreg.data = ( data_ptr->Base + data_ptr->Size - 1 ) & 0xFF;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
		data_ptr->retcode = PCM_SUCCESS;
		ret_code = EIO;
		goto end;
	}
	
    /* System Memory Address Stop High Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 3,
					 pcmcia_win2socket(data_ptr->Window) );
    ioreg.data = ( data_ptr->Base + data_ptr->Size - 1 ) >> 8 & 0x0F 
	         | speed_bits;
	
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    if ( data_ptr->Base > ap->window[data_ptr->Window].setpage_s.Offset ){
        offset_addr = 0x4000
            - ( data_ptr->Base
               - ap->window[data_ptr->Window].setpage_s.Offset );
    } else {
        offset_addr =
            ap->window[data_ptr->Window].setpage_s.Offset - data_ptr->Base;
    }
	
#ifdef DEBUG
	printf("offset_addr = 0x%X \n", offset_addr );
#endif
	
    /* Card Memory Offset Address Low Byte Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 4,
					 pcmcia_win2socket(data_ptr->Window) );
    ioreg.data =  offset_addr & 0xFF;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* Card Memory Offset Address High Byte Register */
    ioreg.index = 
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 5,
					 pcmcia_win2socket(data_ptr->Window) );
    ioreg.data = offset_addr >> 8 & 0x3F
        | ( ap->window[data_ptr->Window].setpage_s.State & PCM_PS_ATTRIBUTE
		   ? BIT_6 : 0 )
			| ( ap->window[data_ptr->Window].setpage_s.State & PCM_PS_WP
			   ? BIT_7 : 0 ) ;
	
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* Address Window Enable Register (0x06) */
    ioreg.index =
        ioreg_index( PCM_WINDOW_ENABLE, pcmcia_win2socket(data_ptr->Window) );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    if ( data_ptr->State & PCM_WS_ENABLED ){
        /* Enable Window */
        ioreg.data |= Window[data_ptr->Window % NUM_WINDOWS].EnableBit;
    } else {
        /* Disable Window */
        ioreg.data &= ~Window[data_ptr->Window % NUM_WINDOWS].EnableBit;
    }
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    data_ptr->retcode = PCM_SUCCESS;
	
 end:
	
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_SetWindow_io
 *
 * FUNCTION: PCMCIA Socket Service SetWindow for I/O window
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int
	pcmcia_SetWindow_io(
						struct adapter_def *ap, SetWindow_S * data_ptr, int devflag )
{
    struct pcmcia_ioreg ioreg;
    int ret_code = 0, rc;
    
    /* Address Window Enable Register (0x06) */
    ioreg.index = ioreg_index( PCM_WINDOW_ENABLE,
							  pcmcia_win2socket(data_ptr->Window) );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    /* Disable I/O Window */
    ioreg.data &= ~Window[data_ptr->Window % NUM_WINDOWS].EnableBit;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
	
    /* I/O Address Start Low Byte Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index,
					 pcmcia_win2socket(data_ptr->Window) );
    ioreg.data  = data_ptr->Base & 0xFF;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    
    /* I/O Address Start High Register */
    ioreg.index = 
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 1,
					 pcmcia_win2socket(data_ptr->Window) );
    ioreg.data = data_ptr->Base >> 8 & 0xFF ;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    
    /* I/O Address Stop Low Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 2,
					pcmcia_win2socket(data_ptr->Window) );
    ioreg.data = ( data_ptr->Base + data_ptr->Size - 1 ) & 0xFF;
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    
    /* I/O Address Stop High Register */
    ioreg.index =
		ioreg_index(Window[data_ptr->Window % NUM_WINDOWS].Index + 3,
					 pcmcia_win2socket(data_ptr->Window) );
    ioreg.data = ( data_ptr->Base + data_ptr->Size - 1 ) >> 8 & 0xFF;
    /* in case of TOMCAT, Wait State Bits are set to 00 */
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    

	/* State WS_16BIT: 16-bit I/O bus or 8-bit I/O bus */
	/* I/O Control Register (0x07) */
	ioreg.index = ioreg_index( PCM_IO_CTRL,
							  pcmcia_win2socket(data_ptr->Window) );
	rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
	if ( rc ){
		data_ptr->retcode = PCM_SUCCESS;
		ret_code = EIO;
		goto end;
	}
	/* I/O Control Registers' indexes are 0x08, 0x48, 0x88, 0xC8 */
	if ( Window[data_ptr->Window % NUM_WINDOWS].Index & 0x3F == 0x08 )
	{
		/* I/O Window 0 */
		if ( data_ptr->State & PCM_WS_16BIT ){ /* Data Size bit */
			ioreg.data &= ~(BIT_0 | BIT_1 | BIT_2 | BIT_3);
			if ( data_ptr->State & PCM_WS_ENABLED ){
			  ioreg.data |= BIT_0 | BIT_1 | BIT_3;
			}
		} else {
			ioreg.data &= ~(BIT_0 | BIT_1 | BIT_2 | BIT_3);
			if ( data_ptr->State & PCM_WS_ENABLED ){
			  ioreg.data |= BIT_1;
			}
		}
	} else {
		/* I/O Window 1 */
		if ( data_ptr->State & PCM_WS_16BIT ){ /* Data Size bit */
			ioreg.data &= ~(BIT_4 | BIT_5 | BIT_6 | BIT_7);
			if ( data_ptr->State & PCM_WS_ENABLED ){
			  ioreg.data |= BIT_4 | BIT_5 | BIT_7;
			}
		} else {
			ioreg.data &= ~(BIT_4 | BIT_5 | BIT_6 | BIT_7);
			if ( data_ptr->State & PCM_WS_ENABLED ){
			  ioreg.data |= BIT_5;
			}
		}
	}
	rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
	if ( rc ){
		data_ptr->retcode = PCM_SUCCESS;
		ret_code = EIO;
		goto end;
	}

    /* Address Window Enable Register (0x06) */
    ioreg.index = ioreg_index( PCM_WINDOW_ENABLE,
							   pcmcia_win2socket(data_ptr->Window) );
    rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }
    if ( data_ptr->State & PCM_WS_ENABLED ){
        /* Enable I/O Window */
        ioreg.data |= Window[data_ptr->Window % NUM_WINDOWS].EnableBit;
    } else {
        /* Disable I/O Window */
        ioreg.data &= ~Window[data_ptr->Window % NUM_WINDOWS].EnableBit;
    }
    rc = pcmcia_ioreg( ap, PCM_IOWRITE, &ioreg );
    if ( rc ){
        data_ptr->retcode = PCM_SUCCESS;
        ret_code = EIO;
        goto end;
    }

    data_ptr->retcode = PCM_SUCCESS;

 end:
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_GetPage
 *
 * FUNCTION: PCMCIA Socket Service GetPage
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int pcmcia_GetPage( struct adapter_def *ap, int arg, int devflag )
{
    GetPage_S data;
    struct pcmcia_ioreg ioreg;
    int ret_code = 0, rc;

    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof( GetPage_S ),
                        devflag );
    if ( rc ){
        return EFAULT;
    }

    data.Window  = ap->window[data.Window].setpage_s.Window;
    data.Page    = ap->window[data.Window].setpage_s.Page;
    data.State   = ap->window[data.Window].setpage_s.State;
    data.Offset  = ap->window[data.Window].setpage_s.Offset;
    data.retcode = PCM_SUCCESS;

 end:
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof( GetPage_S ),
                         devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD4(HKWD_PCMCIA_SS, PCM_GET_PAGE, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.Window, data.Page, data.State, 0 ),
			pcmcia_word( 0, data.Offset, 0, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_SetPage
 *
 * FUNCTION: PCMCIA Socket Service SetPage
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *     EIO      An I/O error is detected.
 *
 ***************************************************************************/
int pcmcia_SetPage( struct adapter_def *ap, int arg, int devflag )
{
    SetPage_S data;
    struct pcmcia_ioreg ioreg;
    int ret_code = 0, rc;

    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof( SetPage_S ),
                        devflag );
    if ( rc ){
        return EFAULT;
    }

#ifdef DEBUG
printf("SetPage: Window=%d, Page=%d, State=0x%X, Offset=0x%X \n",
       data.Window, data.Page, data.State, data.Offset);
#endif

    ap->window[data.Window].setpage_s = data;

	/* if WS_ENABLED of SetWindow is set, update the window mapping */
	if ( ap->window[data.Window].setwindow_s.State & PCM_WS_ENABLED ){
		rc = pcmcia_SetWindow_mem(
			  ap, & ap->window[data.Window].setwindow_s, devflag);
		if ( rc ){
			ret_code = rc;
			data.retcode = ap->window[data.Window].setwindow_s.retcode;
			goto end;
		}
	}

    data.retcode = PCM_SUCCESS;

 end:
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof( SetPage_S ),
                         devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD4(HKWD_PCMCIA_SS, PCM_SET_PAGE, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( data.Window, data.Page, data.State, 0 ),
			pcmcia_word( 0, data.Offset, 0, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_copyout
 *
 * FUNCTION: PCMCIA Socket Service misc function
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by both a process and an interrupt.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Copyout operation is faliled.
 *
 ***************************************************************************/
int pcmcia_copyout( char *src, char *dst, size_t size, int devflag )
{
	int rc;
	if ( devflag & DKERNEL ){
		bcopy ( src, dst, size );
	} else {
		rc = copyout( src, dst, size );
		if ( rc ){
			return EFAULT;
		}
	}
	return 0;
}

/****************************************************************************
 *
 * NAME: pcmcia_copyout
 *
 * FUNCTION: PCMCIA Socket Service misc function
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by both a process and an interrupt.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Copyout operation is faliled.
 *
 ***************************************************************************/
int pcmcia_copyin( char *src, char *dst, size_t size, int devflag )
{
	int rc;
	if ( devflag & DKERNEL ){
		bcopy ( src, dst, size );
	} else {
		rc = copyin( src, dst, size );
		if ( rc ){
			return EFAULT;
		}
	}
	return 0;
}

/****************************************************************************
 *
 * NAME: pcmcia_itob
 *
 * FUNCTION: PCMCIA Socket Service misc function
 *
 *     This function converts the int to the bitmap. The uniquely-set bit
 *     show the input. I.e.,
 *       If the input is 2, then returns 0x02.
 *       If the input is 5, then returns 0x10.
 *       If the input is 10, then returns 0x200.
 *     More than 32 and minus numbers may not accepted.
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process and an interrupt.
 *
 * RETURNS:
 *     0   out of range
 *     The bitmap corresponding to the input.
 *
 ***************************************************************************/
int pcmcia_itob( int i )
{
	if ( i < 0 || 31 < i ){
		return 0;
	}
	return( 1 << i );
}
