static char sccsid[] = "@(#)78	1.6  src/rspc/kernext/pcmcia/ssisa/pcm_ss.c, pcmciaker, rspc41J, 9509A_all 2/20/95 17:36:43";
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

/*
 * This module contains
 *     PCMCIA Socket Service routines
 */

#include "pcm_inc.h"
#include "pcm_ss_data.h"

extern int pcmcia_intr_prior;
void pcmcia_errlog( struct adapter_def *ap, int error_id, int op );
#ifdef DEBUG
extern int pcmcia_debug;
#endif
/****************************************************************************
 *
 * NAME: pcmcia_GetSSInfo
 *
 * FUNCTION: PCMCIA Socket Service GetSSInfo
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *
 ***************************************************************************/

int pcmcia_GetSSInfo( struct adapter_def *ap, int arg, int devflag )
{
    GetSSInfo_S data;
    int rc, ret_code = 0;
    extern inited_adapters;
    
    /* initialize the return values to zero */
    bzero( & data, sizeof( GetSSInfo_S ));
	
    data.Compliance = PCM_COMPLIANCE;
    data.NumAdapters = 1;
    data.FirstAdapter = 0;
    data.retcode = PCM_SUCCESS;
	
    /* return data to user space */
    rc = pcmcia_copyout( &data, (char *) arg,  sizeof( GetSSInfo_S ),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD3(HKWD_PCMCIA_SS, PCM_GET_SS_INFO, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( 0, data.Compliance, data.NumAdapters,
						data.FirstAdapter ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_InquireAdapter
 *
 * FUNCTION: PCMCIA Socket Service InquireAdapter
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *
 ***************************************************************************/
int pcmcia_InquireAdapter( struct adapter_def* ap, int arg, int devflag )
{
    InquireAdapter_S data;
    AISTRUCT aistruct;
    int short_buf = 0; /* if client-supplied buffer is short, then set */
    int i, rc, ret_code = 0;
	
    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data,
					   sizeof( InquireAdapter_S ), devflag );
    if ( rc ){
        return EFAULT;
    }
	
    /* get AISTRUCT data from user space */
    rc = pcmcia_copyin( (char *) data.pBuffer, & aistruct, sizeof( AISTRUCT ),
					   devflag );
    if ( rc ){
        return EFAULT;
    }
	
    aistruct.wDataLength = 
        sizeof(AISTRUCT) - 2 * sizeof(WORD) < data.pBuffer->wBufferLength
			? sizeof(AISTRUCT) - 2 * sizeof(WORD) : data.pBuffer->wBufferLength ;
	
    aistruct.wNumPwrEntries = NUM_PWR_ENTRY;
    for ( i = 0; i < NUM_PWR_ENTRY; i++ ){
        aistruct.PwrEntry[i].PowerLevel   = PwrEntry[i].PowerLevel;
        aistruct.PwrEntry[i].ValidSignals = PwrEntry[i].ValidSignals;
    }
	
    aistruct.CharTable.AdpCaps    = ADP_CAPS;
    aistruct.CharTable.ActiveHigh = ACTIVE_HIGH;
    aistruct.CharTable.ActiveLow  = ACTIVE_LOW;
	
    data.NumSockets = ap->num_sockets;
    data.NumWindows = ap->num_sockets * NUM_WINDOWS;
    data.NumEDCs = NUM_EDCS;
    data.retcode = PCM_SUCCESS;
	
    /* if AISTRUCT is stored then return the data */
    rc = pcmcia_copyout( & aistruct, (char *) data.pBuffer,
						aistruct.wDataLength + 2 * sizeof(WORD) , devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
	
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg,
						sizeof(InquireAdapter_S), devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD4(HKWD_PCMCIA_SS, PCM_INQ_ADAPTER, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			data.pBuffer,
			pcmcia_word( data.NumSockets, data.NumWindows, data.NumEDCs, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_InquireSocket
 *
 * FUNCTION: PCMCIA Socket Service InquireSocket
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
int pcmcia_InquireSocket( struct adapter_def *ap, int arg, int devflag )
{
    InquireSocket_S data;
    SISTRUCT sistruct;
    int rc, ret_code = 0;
	
    /* get data from user space */
    rc = pcmcia_copyin( ( char *) arg, & data, sizeof(InquireSocket_S),
					   devflag );
    if ( rc ){
        return EFAULT;
    }
	
    /* get SISTRUCT(wBufferLength,wDataLength) from user space */
    rc = pcmcia_copyin( (char *) data.pBuffer, & sistruct,
					   sizeof(WORD) + sizeof(WORD), devflag );
    if ( rc ){
        return EFAULT;
    }
	
    /* check the client-supplied buffer length */
    sistruct.wDataLength =
		data.pBuffer->wBufferLength < sizeof(SISTRUCT) - 2 * sizeof(WORD)
			? data.pBuffer->wBufferLength : sizeof(SISTRUCT) - 2 * sizeof(WORD) ;
	
    /* SkkCaps */
    sistruct.CharTable.SktCaps = SKT_CAPS;
	
    /* ActiveHigh */
    sistruct.CharTable.ActiveHigh = ACTIVE_HIGH;
	
    /* ActiveLow */
    sistruct.CharTable.ActiveLow = ACTIVE_LOW;
	
    /* SCIntCaps */
    data.SCIntCaps = SC_INT_CAPS;
	
    /* SCRptCaps */
    data.SCRptCaps = SC_RPT_CAPS;
	
    /* SCIndCaps */
    data.CtlIndCaps = SC_IND_CAPS;
	
    data.retcode = PCM_SUCCESS;
	
    /* return SISTRUCT to the user space */
    rc = pcmcia_copyout( & sistruct, (char *) data.pBuffer,
						sistruct.wDataLength + 2 * sizeof(WORD),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
        goto end;
    }
	
 end:
    /* return the data to the user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof(InquireSocket_S),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
        goto end;
    }
    DDHKWD4(HKWD_PCMCIA_SS, PCM_INQ_SOCKET, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			data.pBuffer,
			pcmcia_word( data.Socket, data.SCIntCaps,
						data.SCRptCaps, data.CtlIndCaps ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_InquireWindow
 *
 * FUNCTION: PCMCIA Socket Service InquireWindow
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *
 ***************************************************************************/
int pcmcia_InquireWindow( struct adapter_def *ap, int arg, int devflag )
{
    InquireWindow_S data;
    WISTRUCT wistruct;
    int rc, ret_code = 0;
	
    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof(InquireWindow_S),
					   devflag );
    if ( rc ){
        return EFAULT;
    }
	
    /* get WinTable from user space */
    rc = pcmcia_copyin( data.pBuffer, & wistruct, 2 * sizeof(WORD),
					   devflag );
    if ( rc ){
        return EFAULT;
    }
	
    if ( Window[data.Window % NUM_WINDOWS].WndCaps & PCM_WC_COMMON ){
        wistruct.WinTable[0].MemWinTbl = MemWinTbl;
    } else {
        wistruct.WinTable[0].IOWinTbl = IOWinTbl;
    }
    wistruct.wDataLength =
        sizeof(WISTRUCT) - 2 * sizeof(WORD) < data.pBuffer->wBufferLength
			? sizeof(WISTRUCT) - 2 * sizeof(WORD) : data.pBuffer->wBufferLength ;
	
    /* WndCaps */
    data.WndCaps = Window[data.Window % NUM_WINDOWS].WndCaps;
	
    /* Socket */
    data.Sockets = pcmcia_itob( pcmcia_win2socket(data.Window) );
	
    data.retcode = PCM_SUCCESS;
	
    /* return WinTable to user space */
    rc = pcmcia_copyout(
						& wistruct, data.pBuffer,
						wistruct.wDataLength + 2 * sizeof(WORD),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
        goto end;
    }
	
 end:
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof(InquireWindow_S),
						devflag );
    if ( rc ){
        ret_code = EFAULT;
        goto end;
    }
    DDHKWD4(HKWD_PCMCIA_SS, PCM_INQ_WINDOW, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			data.pBuffer,
			pcmcia_word( data.Window, data.WndCaps, data.Sockets, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_VendorInfo
 *
 * FUNCTION: PCMCIA Socket Service VendorInfo
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EFAULT   Data copy failed between user and kernel space.
 *
 ***************************************************************************/
int pcmcia_GetVendorInfo( struct adapter_def *ap, int arg, int devflag )
{
    GetVendorInfo_S data;
    VISTRUCT vistruct;
    int i, ret_code = 0, rc;

    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data, sizeof(GetVendorInfo_S ),
                        devflag );
    if ( rc ){
        return EFAULT;
    }

    /* get VISTRUCT from user space */
    rc = pcmcia_copyin( data.pBuffer, & vistruct, sizeof( VISTRUCT ),
                        devflag );
    if ( rc ){
        return EFAULT;
    }

    /* Only supported type */
    if ( data.Type != 0 ){
        data.retcode = PCM_BAD_FUNCTION;
        ret_code = EINVAL;
        goto end;
    }

    vistruct.wDataLength = 
        sizeof(VISTRUCT) - 2 * sizeof(WORD) < data.pBuffer->wBufferLength
        ? sizeof(VISTRUCT) - 2 * sizeof(WORD) : data.pBuffer->wBufferLength ;

    for ( i = 0; vistruct.szImplementor[i] = PCM_IMPLEMENTOR[i]; i++ )
        ;

    data.Release = PCM_RELEASE;
    data.retcode = PCM_SUCCESS;

    /* return VISTRUCT to user space */
    rc = pcmcia_copyout( & vistruct, data.pBuffer,
                         vistruct.wDataLength + 2 * sizeof(WORD), devflag );
    if ( rc ){
        ret_code = EFAULT;
        goto end;
    }

 end:
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof(GetVendorInfo_S),
                         devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD4(HKWD_PCMCIA_SS, PCM_GET_VENDOR_INFO, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			data.pBuffer,
			pcmcia_word( data.Type, 0, data.Release, 0 ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_VendorSpecific
 *
 * FUNCTION: PCMCIA Socket Service VendorSpecific
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EINVAL   An invalid parameter is specified.
 *
 * CALLS:
 *     pcmcia_ioreg
 *
 * CALLED BY:
 *     pcmcia_ioctl
 *
 ***************************************************************************/
int pcmcia_VendorSpecific( struct adapter_def *ap, int arg, int devflag )
{
    VendorSpecific_S data;
    int ret_code = 0, rc;

    /* get data from user space */
    rc = pcmcia_copyin( (char *) arg, & data,
                        sizeof( VendorSpecific_S ), devflag );
    if ( rc ){
        return EFAULT;
    }

    switch ( data.function )
    {
    case PCM_IOREAD:
        rc = pcmcia_ioreg( ap, PCM_IOREAD, & data.varg.ioreg );
        break;
    case PCM_IOWRITE:
        rc = pcmcia_ioreg( ap, PCM_IOWRITE, & data.varg.ioreg );
        break;
    default:
        data.retcode = PCM_SUCCESS;
        return EINVAL;
    }

    data.retcode = PCM_SUCCESS;

 end:
    /* return data to user space */
    rc = pcmcia_copyout( & data, (char *) arg, sizeof(VendorSpecific_S),
                         devflag );
    if ( rc ){
        ret_code = EFAULT;
    }
    DDHKWD3(HKWD_PCMCIA_SS, PCM_VEND_SPECIFIC, ret_code,
			ap->devno,
			pcmcia_word( 0, PCM_COMPLIANCE, data.retcode, 0 ),
			pcmcia_word( 0, data.function, (uchar) data.varg.ioreg.index,
                                           (uchar) data.varg.ioreg.data ));
    return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_bad_fucntion
 *
 * FUNCTION: PCMCIA Socket Service bad fuction
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     EINVAL   An invalid parameter is specified.
 *     EFAULT   Data copy failed between user and kernel space.
 *
 * CALLS:
 *
 * CALLED BY:
 *     pcmcia_ioctl
 *
 ***************************************************************************/
int
pcmcia_bad_function( struct adapter_def *ap, int arg, int devflag, int op )
{
	RETCODE data = PCM_BAD_FUNCTION;
	int ret_code = EINVAL;
	int rc;
	
	/* temp id for complie */
	pcmcia_errlog( ap, ERRID_PCMCIA_SS_BAD_FUNC, op );

	rc = pcmcia_copyout( & data, (char *) arg, sizeof( RETCODE ) );
	if ( rc ){
		ret_code = EFAULT;
	}
	return ret_code;
}

void pcmcia_errlog( struct adapter_def *ap, int error_id, int op )
{
	struct pcmcia_err pcmcia_err;

	pcmcia_err.err.error_id = error_id;
	strcpy( pcmcia_err.err.resource_name, ap->ddi.resource_name );
	pcmcia_err.errdata[0] = op;
	errsave( & pcmcia_err, sizeof( pcmcia_err ) );
	return ;
}
