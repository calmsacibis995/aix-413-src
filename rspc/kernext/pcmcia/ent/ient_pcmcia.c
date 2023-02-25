static char sccsid[] = "@(#)89  1.7  src/rspc/kernext/pcmcia/ent/ient_pcmcia.c, pcmciaent, rspc41J, 9516A_all 4/18/95 00:54:11";
/*
 * COMPONENT_NAME: PCMCIAENT - IBM PCMCIA Ethernet Device Driver
 *
 * FUNCTIONS: For PCMCIA.  Callback(), CardInsertion(), CardRemoval(),
 *            CheckCS(), RegisterClient(), CheckCard(), CheckTuple(),
 *            CheckConfigured(), ReadTuple(), RequestIO(), RequestIRQ(),
 *            RequestConfiguration(), RequestWindow(), MapMemPage(),
 *            ReleaseIO(), ReleaseIRQ(), ReleaseConfiguration(),
 *            ReleaseWindow(), DeregisterClient(), GetTupleInfo()
 *            AccessConfigurationRegister(), GetMACaddr()
 *	      hold_lock(), release_lock()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ient_proto.h"

extern dd_ctrl_t dd_ctrl;

/*****************************************************************************/
/*
 * NAME:                  Callback
 *
 * FUNCTION:              Called by CardService for handling CS events.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *    arguments EventID    : Identifying the event on entry.
 *              Socket     : Identifies the socket affected by the event.
 *              Info       : Other information specific to the event.
 *              Resvd      : Reserved.
 *              Buffer     : Used to pass a buffer.
 *              Misc       : Used for miscellaneous information.
 *              client_data: Client Data buffer.
 *
 * RETURNS:               0 or errno
 *
 */
/*****************************************************************************/

int Callback( int EventID, int Socket, int Info, char *Resvd,
                  char* Buffer, int Misc, char *client_data )
{
    int        err    = CSR_SUCCESS;
    dds_t      *p_dds = (dds_t *)client_data;
    int        ipri;


    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCBb", (ulong)p_dds, EventID, Socket);
    {  
        int zero = 0;

	compare_and_swap( &p_dds->cb_flag, &zero, 1 );
	if( p_dds->cb_flag != 1 )  {
	    TRACE_PCMCIA( HKWD_IEN_ISA_OTHER,"cCB0",EventID,Socket,p_dds->cb_flag );
	    return( err );
	}
    }

    /******** At first, check whether the event is for me or not. ************/
    if( EventID != CSE_REGISTRATION_COMPLETE ) {
	TRACE_PCMCIA( HKWD_IEN_ISA_OTHER,"cCB1",
	    EventID,Socket,p_dds->ddi.logical_socket);
        if( p_dds->ddi.logical_socket != Socket )
            goto end;
    }
    switch( EventID ) {
    case CSE_REGISTRATION_COMPLETE:
        ipri = i_disable( INTMAX );
        p_dds->reg_comp = TRUE;
        e_wakeup( &p_dds->event_word );
        i_enable( ipri );
        break;
    case CSE_CARD_INSERTION:
        p_dds->no_card = FALSE;
        err = CardInsertion( p_dds );
        break;
    case CSE_CARD_REMOVAL:
        p_dds->no_card = TRUE;
        err = CardRemoval( p_dds );
        break;
    default:
        break;
    }

    /******** Set return code for ddconfig ***********************************/
    if( EventID != CSE_REGISTRATION_COMPLETE ) {
        p_dds->return_callback = err;
    }


end:
    p_dds->cb_flag = 0;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCBe", err, p_dds->return_callback, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  CardInsertion
 *
 * FUNCTION:              Handling of Card Insertion event.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               0 or errno
 *
 */
/*****************************************************************************/

int CardInsertion( dds_t *p_dds )
{
    unsigned char chk_mask;
    int           err;
    int           rc;                 /* Function return code.               */
    int           dds_size;           /* Used when allocating dds memory.    */
    ndd_config_t  ndd_config;         /* Config information.                 */
    ddi_t         *p_ddi, tempddi;    /* Database information pointers.      */

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCIb", (ulong)p_dds, 0, 0);

    /******** Check whether another client has already configured ************/
    if( err = CheckConfigured( p_dds )) {
        return( err );
    }

    /******** Check whether the insertion event is for me or not. ************/
    if( CheckCard( p_dds->ddi.logical_socket ) != MSK_MATCH_ALL )
        return( CSR_BAD_TYPE );

    /******** Get the Tuple information, set the dds *************************/
    if( err = GetTupleInfo( p_dds->ddi.logical_socket, p_dds )) {
        return( err );
    }

    /******** Request I/O ****************************************************/
    if ( err = RequestIO( p_dds )) {
        return( err );
    }

    /******** Request IRQ ****************************************************/
    if ( err = RequestIRQ( p_dds )) {
        ReleaseIO( p_dds );
        return( err );
    }

    /******** Hold Lock ******************************************************/
    if( err = hold_lock( p_dds ) ){
        ReleaseIO( p_dds );
        ReleaseIRQ( p_dds );
        return( err );
    }

    /******** Request Configuration ******************************************/
    if ( err = RequestConfiguration( p_dds )) {
	release_lock( p_dds );
        ReleaseIO( p_dds );
        ReleaseIRQ( p_dds );
        return( err );
    }

    /******** Access configuration( Option ) register ************************/
    if ( err = AccessConfigurationRegister( p_dds )) {
        ReleaseConfiguration( p_dds );
	release_lock( p_dds );
        ReleaseIO( p_dds );
        ReleaseIRQ( p_dds );
        return( err );
    }

    /******** Release Lock ***************************************************/
    err = release_lock( p_dds );

    /******** Get the MAC address, set to burn address. **********************/
    if( err = GetMACaddr( p_dds)) {
        ReleaseConfiguration( p_dds );
        ReleaseIO( p_dds );
        ReleaseIRQ( p_dds );
        return( err );
    }

    /******** Request Window *************************************************/
    if ( err = RequestWindow( p_dds )) {
        ReleaseConfiguration( p_dds );
        ReleaseIO( p_dds );
        ReleaseIRQ( p_dds );
        return( err );
    }

    /******** Map Memory Page ************************************************/
     if ( err = MapMemPage( p_dds )) {
         ReleaseConfiguration( p_dds );
         ReleaseIO( p_dds );
         ReleaseIRQ( p_dds );
         ReleaseWindow( p_dds );
         return( err );
     }

    /******** Turn Off PCMCIA Activity Lamp **********************************/
    pcmcia_activity_lamp(p_dds, _CSaixTurnOffLED);

    /******** Complete Configuration Successfully ****************************/
    p_dds->configured = 1;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCIe", err, 0, 0);

    return( err );
}



/*****************************************************************************/
/*
 * NAME:                  CardRemoval
 *
 * FUNCTION:              Handling of Card Removal event.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               0 or errno
 *
 */
/*****************************************************************************/

int CardRemoval( dds_t *p_dds )
{
    int err;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCRb", (ulong)p_dds, p_dds->configured,
              dd_ctrl.initialized);
    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCR+", WRK.adap_state, 0, 0);

    if( p_dds->configured ) {
	err = hold_lock( p_dds );
	pcmcia_activity_lamp(p_dds, _CSaixTurnOnLED);
        /******** Release all resources. *************************************/
        ReleaseConfiguration( p_dds );
        ReleaseIO( p_dds );
        ReleaseIRQ( p_dds );
        ReleaseWindow( p_dds );
	if( err == CSR_SUCCESS )
	    release_lock( p_dds );
        p_dds->configured = 0;
    }

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCRe", p_dds->configured,
                dd_ctrl.initialized, WRK.adap_state);

    return( CSR_SUCCESS );
}


/*****************************************************************************/
/*
 * NAME:                  CheckCS
 *
 * FUNCTION:              Checks CardService's Version Number.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  Chek the following CS Version.
 *                             CS Revision : 1.00
 *                             CS Level    : 2.10
 * RETURNS:               
 *        CSR_SUCCESS                 0x00 - The request succeeded
 *        CSR_BAD_ARG_LENGTH          0x1B - ArgLength arg are invalid
 *        CSR_UNSUPPORTED_FUNCTION    0x15 - This function is not supported
 */
/*****************************************************************************/

int CheckCS()
{
    int            err;
    uint           revision = 0x0100;
    uint           cslevel  = 0x0210;
    CSGetCSInfoPkt pkt;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCSb", cslevel, revision, 0);

    bzero( &pkt, sizeof(pkt) );

    if( !( err = CardServices( CSGetCardServicesInfo, NULL, NULL,
                        sizeof( pkt ), (void *) &pkt ))) {
        if( pkt.Revision != revision || pkt.CSLevel != cslevel )
            err = CSR_UNSUPPORTED_FUNCTION;
    }

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCSe", err, 0, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  RegisterClient
 *
 * FUNCTION:              This function registers a client with Card Services.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *			  The clientID setted by CardServices.
 *                        A REGISTRATION_COMPLETE event is generated for the
 *                        client callback handler when registration processing
 *                        for the client has been completed by Card Services.
 *                        The ClientHandle returned in the Handle argument 
 *                        must be passed to DeregisterClient when the client
 *                        terminates.
 * RETURNS:               
 *        CSR_SUCCESS            0x00 - The request succeeded
 *        CSR_BAD_ATTRIBUTE      0x02 - Attributes field is invalid
 *        CSR_BAD_ARG_LENGTH     0x1B - ArgLength arg are invalid
 *        CSR_OUT_OF_RESOURCE    0x20 - Card Services has exhausted resource
 */
/*****************************************************************************/

int RegisterClient( dds_t *p_dds )
{
    int         err;
    CSRegCliPkt pkt;

    pkt.Attributes = CSCliAttrIOCDD |  /* IO client device driver            */
                     CSCliAttrEXCL;    /* CARD_INSERTION for exclusively     */
    pkt.EventMask  = CSEvMaskLockChg|  /* Events to notify Client   	     */
                     CSEvMaskEjReq  |
                     CSEvMaskInReq  |
                     CSEvMaskCDChg  |
                     CSEvMaskPMChg  |
                     CSEvMaskReset  ;
    pkt.ClientData = (char*)p_dds;     /* pointer to dds_structure(_dds)     */
    pkt.Version    = 0x0210; 	       /* expects CS version is 2.1 or upper */

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRGb", (ulong)p_dds, (ulong)pkt.ClientData, 0);

    err =  CardServices( CSRegisterClient, &p_dds->clientID, (void *)Callback,
                         sizeof( CSRegCliPkt ), (void *) &pkt );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRGe", err, p_dds->clientID, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  CheckCard
 *
 * FUNCTION:              Check Card.(check VERS_1, MANFID, FUNCID)
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  arguments Socket: logical socket
 *
 * RETURNS:               
 *    Bit mask:
 *             0 0 0 0 0 0 0 0
 *             === === === ===
 *              |   |   |   |
 *              |   |   |   +-- status of VERS_1
 *              |   |   +------ status of MANFID
 *              |   +---------- status of FUNCID
 *              +-------------- not use ( zero )
 *
 *           [ 0 0 ] -> matched Tuple data with check_tuple_packet.contens
 *           [ 1 0 ] -> didn't match
 *           [ 0 1 ] -> there is no tuple data.
 *           [ 1 1 ] -> faild to get tuple data.
 *
 *    Example :
 *             0 0 0 1 1 0 0 0  ===> 0x18 ( return code )
 *             === === === ===
 *              |   |   |   |
 *              |   |   |   +-- VERS_1's  matched
 *              |   |   +------ MANFID's  didn't match
 *              |   +---------- FUNCID's  wasn't there
 *              +-------------- not use ( zero )
 *
 */
/*****************************************************************************/

unsigned char CheckCard(int Socket)
{
    unsigned char rc_mask = 0x0;
    int           rc_chk1, rc_chk2;
    char          product_info1[]    = "0933495";
    char          product_info2[]    = "0934214";
    char          manufacturer_id[] = {0xa4, 0x00, 0x02, 0x00};
    char          function_id[]     = {0x06, 0x03};
    chk_tpl_pkt_t chk_pkt;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCCb", Socket, 0, 0);

    /******** Check the Level 1 Version/Product Infomation Tuple. ************/
    chk_pkt.offset   = 21;
    chk_pkt.length   = 7;
    chk_pkt.contents = product_info1;
    rc_chk1 = CheckTuple( CISTPL_VERS_1, Socket, &chk_pkt );

    chk_pkt.offset   = 21;
    chk_pkt.length   = 7;
    chk_pkt.contents = product_info2;
    rc_chk2 = CheckTuple( CISTPL_VERS_1, Socket, &chk_pkt );

    if( rc_chk1 != RC_MATCHED && rc_chk2 != RC_MATCHED ) {
        if( rc_chk1 == RC_FAILD_READ || rc_chk2 == RC_FAILD_READ )
            rc_mask |= MSK_VERS_1_FAILD;
        else if( rc_chk1 == RC_NO_TUPLE || rc_chk2 == RC_NO_TUPLE )
            rc_mask |= MSK_VERS_1_NO_TUPLE;
        else
            rc_mask |= MSK_VERS_1_NOT_MATCH;
    }

    /******** Check the Manufacturer Identification tuple. *******************/
    chk_pkt.offset   = 0;
    chk_pkt.length   = 4;
    chk_pkt.contents = manufacturer_id;

    switch( CheckTuple( CISTPL_MANFID, Socket, &chk_pkt )) {
        case RC_NOT_MATCH : rc_mask |= MSK_MANFID_NOT_MATCH; break;
        case RC_NO_TUPLE  : rc_mask |= MSK_MANFID_NO_TUPLE;  break;
        case RC_FAILD_READ: rc_mask |= MSK_MANFID_FAILD;     break;
    }

    /******** Check the Function identificaton tuple. ************************/
    chk_pkt.offset   = 0;
    chk_pkt.length   = 2;
    chk_pkt.contents = function_id;

    switch( CheckTuple( CISTPL_FUNCID, Socket, &chk_pkt )) {
        case RC_NOT_MATCH : rc_mask |= MSK_FUNCID_NOT_MATCH; break;
        case RC_NO_TUPLE  : rc_mask |= MSK_FUNCID_NO_TUPLE;  break;
        case RC_FAILD_READ: rc_mask |= MSK_FUNCID_FAILD;     break;
    }

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCCe", (ulong)rc_mask, 0, 0);

    return( rc_mask );
}


/*****************************************************************************/
/*
 * NAME:                  CheckTuple
 *
 * FUNCTION:              Check the TUPLE data.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               
 *        CSR_SUCCESS            0x00 - The request succeeded
 *        CSR_BAD_ADAPTER        0x01 - Specified adapter is invalid.
 *        CSR_BAD_SOCKET         0x08 - Specified socket is invalid
 *        CSR_NO_CARD            0x14 - No PC Card in socket
 *        CSR_BAD_ARG_LENGTH     0x1B - ArgLength arg are invalid
 *        CSR_NO_MORE_ITEMS      0x1F - There are no more of requested item
 */
/*****************************************************************************/

int CheckTuple( int TupleID, int Socket, chk_tpl_pkt_t *chk_ptr )
{
    int             err;
    char            *dp;
    CSGetTplDataPkt *tpl_dat_ptr;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCTb", TupleID, Socket, (ulong)chk_ptr);

    /******** Read the Tuple Data. *******************************************/
    tpl_dat_ptr = (CSGetTplDataPkt *) xmalloc(
                  (sizeof( CSGetTplDataPkt ) + 0xff), 0, pinned_heap );
    bzero( tpl_dat_ptr, sizeof( CSGetTplDataPkt ) + 0xff );
    err = ReadTuple( TupleID, Socket, tpl_dat_ptr, 0xff );
    if ( err != CSR_SUCCESS ) {
        xmfree( tpl_dat_ptr, pinned_heap );
        if( err == CSR_NO_MORE_ITEMS )
            return( RC_NO_TUPLE );
        else
            return( RC_FAILD_READ );
    }
    dp = tpl_dat_ptr->TupleData;

    /******** Compare TupleData with chk_pkt.contents ************************/
    if( tpl_dat_ptr->TupleDataLen < chk_ptr->length + chk_ptr->offset ){
        xmfree( tpl_dat_ptr, pinned_heap );
        return( RC_NOT_MATCH );
    }
    err = bcmp(&dp[chk_ptr->offset], chk_ptr->contents, chk_ptr->length);
    xmfree( tpl_dat_ptr, pinned_heap );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCTe", err, 0, 0);

    if( err )
        return( RC_NOT_MATCH );
    else
        return( RC_MATCHED );
}


/*****************************************************************************/
/*
 * NAME:                  CheckConfigured
 *
 * FUNCTION:              Check the ClientHandle which of returns information 
 *                        about the specified socket and PC Card configuration.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 *        CSR_SUCCESS            0x00 - The request succeeded
 *        CSR_BAD_SOCKET         0x08 - Specified socket is invalid
 *        CSR_NO_CARD            0x14 - No PC Card in socket
 *        CSR_BAD_ARG_LENGTH     0x1B - ArgLength arg are invalid
 */
/*****************************************************************************/

int CheckConfigured( dds_t *p_dds )
{
    int             err;
    int             ClientHandle = 0;
    CSGetCfgInfoPkt pkt;

    pkt.Socket = p_dds->ddi.logical_socket;       /* Logical socket */

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCFb", (ulong)p_dds, 0, 0);

    if( err = CardServices( CSGetConfigurationInfo, &ClientHandle, NULL,
                            sizeof( CSGetCfgInfoPkt ), (void *) &pkt )){
	return( err );
    }

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cCFe", (ulong)ClientHandle,
                 (ulong)pkt.Attributes, 0);

    if(( ClientHandle != NULL ) && ((pkt.Attributes & CSCfgAttrValidCli)>>8 ))
        return( CSR_BAD_ADAPTER );
    else
        return( CSR_SUCCESS );
}


/*****************************************************************************/
/*
 * NAME:                  ReadTuple
 *
 * FUNCTION:              Read the TUPLE Data.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               
 *        CSR_SUCCESS            0x00 - The request succeeded
 *        CSR_BAD_SOCKET         0x08 - Specified socket is invalid
 *        CSR_NO_CARD            0x14 - No PC Card in socket
 *        CSR_BAD_ARG_LENGTH     0x1B - ArgLength arg are invalid
 *        CSR_BAD_ARG            0x1C - Values in Argument Packet are invalid
 *        CSR_NO_MORE_ITEMS      0x1F - There are no more of requested item
 */
/*****************************************************************************/

int ReadTuple( int TupleID, int Socket, CSGetTplDataPkt *tpl_dat, int DataMax )
{
    int          err;
    CSGetTplPkt *gt_tpl = (CSGetTplPkt *) tpl_dat;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRTb", TupleID, Socket, (ulong)tpl_dat);
    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRT+", DataMax, 0, 0);

    gt_tpl->Socket       = Socket;                 /* Logical socket          */
    gt_tpl->Attributes   = CSGetLinkTpls;          /* bit-mapped              */
    gt_tpl->DesiredTuple = TupleID;                /* Desired tuple code value*/
    gt_tpl->Reserved     = 0;                      /* Reserved (reset to 0)   */

    if( err = CardServices( CSGetFirstTuple, NULL, NULL,
                        sizeof( CSGetTplPkt ), (void *) gt_tpl )){
        return( err );
    }

    tpl_dat->TupleDataMax = DataMax;       /* Maximum size of tuple data area */

    err = CardServices( CSGetTupleData, NULL, NULL, (sizeof( CSGetTplDataPkt )
                      + tpl_dat->TupleDataMax), (void *) tpl_dat );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRTe", err, 0, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  GetTupleInfo
 *
 * FUNCTION:              Get the Tuple Information and Set the DDS
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:         arguments Socket: logical socket
 *
 * RETURNS:               0 or errno
 */
/*****************************************************************************/

int GetTupleInfo(int Socket, dds_t *p_dds )
{
    int             err, offset, length;
    char            *dp;
    unsigned char   ch1,ch2;
    CSGetTplDataPkt *tpl_dat_ptr;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cGIb", (ulong)p_dds, 0, 0);

    /******** Read the card configration tuple Data. **************************/
    tpl_dat_ptr = (CSGetTplDataPkt *) xmalloc(
                  (sizeof( CSGetTplDataPkt ) + 0xff), 4, pinned_heap );
    bzero( tpl_dat_ptr, sizeof( CSGetTplDataPkt ) + 0xff );
    err = ReadTuple( CISTPL_CONFIG, Socket, tpl_dat_ptr, 0xff );
    if ( err != CSR_SUCCESS ) {
        xmfree( tpl_dat_ptr, pinned_heap );
        if( err == CSR_NO_MORE_ITEMS )
            return( RC_NO_TUPLE );
        else
            return( RC_FAILD_READ );
    }
    dp = tpl_dat_ptr->TupleData;

    /******** Set the TupleData into the DDS *********************************/
    offset = 0x02;
    length = 0x03;
    if( tpl_dat_ptr->TupleDataLen < length + offset )
        return( RC_NOT_MATCH );
    p_dds->config_base = (int)(( dp[4] << 16 ) | dp[3] << 8 | dp[2] );
    if( err = xmfree( tpl_dat_ptr, pinned_heap ))
        return( err );

    /******** Read the card entry tuple Data. *********************************/
    tpl_dat_ptr = (CSGetTplDataPkt *) xmalloc(
                  (sizeof( CSGetTplDataPkt ) + 0xff), 4, pinned_heap );
    bzero( tpl_dat_ptr, sizeof( CSGetTplDataPkt ) + 0xff );
    err = ReadTuple( CISTPL_CFTABLE_ENTRY, Socket, tpl_dat_ptr, 0xff );
    if ( err != CSR_SUCCESS ) {
        xmfree( tpl_dat_ptr, pinned_heap );
        if( err == CSR_NO_MORE_ITEMS )
            return( RC_NO_TUPLE );
        else
            return( RC_FAILD_READ );
    }
    dp = tpl_dat_ptr->TupleData;

    /******** Set the TupleData into the DDS *********************************/
    offset = 0x00;
    length = 0x01;
    if( tpl_dat_ptr->TupleDataLen < length + offset )
        return( RC_NOT_MATCH );
    p_dds->config_index = dp[0] & 0x3F;
    err = xmfree( tpl_dat_ptr, pinned_heap );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cGIe", err, 0, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  RequestIO
 *
 * FUNCTION:              Request I/O addresses for a socket.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 */
/*****************************************************************************/

int RequestIO( dds_t *p_dds )
{
    int             err;
    CSIOPkt         pkt;

    pkt.Socket      = p_dds->ddi.logical_socket;  /* Logical socket          */
    pkt.BasePort1   = (char *)p_dds->ddi.io_addr; /* BasePort addr for range1*/
    pkt.NumPorts1   = 32;                   /* Number of contiguous ports    */
    pkt.Attributes1 = 0;                    /* bit-mapped                    */
    pkt.BasePort2   = 0;                    /* Base port address for range 2 */
    pkt.NumPorts2   = 0;                    /* Number of contiguous ports    */
    pkt.Attributes2 = 0;                    /* bit-mapped                    */
    pkt.IOAddrLines = 0x10;                 /* Number of I/O address lines   */

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRIb", (ulong)p_dds, pkt.Socket,
                (ulong)pkt.BasePort1);

    err = CardServices( CSRequestIO, &p_dds->clientID, NULL,
                        sizeof( pkt ), (void *) &pkt );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRIe", err, p_dds->clientID, 0);

    return( err );
}



/*****************************************************************************/
/*
 * NAME:                  RequestIRQ
 *
 * FUNCTION:              Requests an interrupt request line. 
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 *        CSR_SUCCESS                 0x00 - The request succeeded
 *        CSR_BAD_ATTRIBUTE           0x02 - Sharing or Pulse request invalid
 *        CSR_BAD_IRQ                 0x06 - IRQ is invalid
 *        CSR_BAD_SOCKET              0x08 - Specified socket is invalid
 *        CSR_NO_CARD                 0x14 - No PC Card in socket
 *        CSR_BAD_ARG_LENGTH          0x1B - ArgLength arg are invalid
 *        CSR_BAD_ARGS                0x1C - IRQ info fields are invalid
 *        CSR_CONFIGURATION_LOCKED    0x1D - Configuration already set
 *        CSR_IN_USE                  0x1E - IRQ requested is already in-use
 *        CSR_BAD_HANDLE              0x21 - Client Handle is invalid
 */
/*****************************************************************************/

int RequestIRQ( dds_t *p_dds )
{
    int             err;
    CSReqIRQPkt     pkt;

    pkt.Socket     = p_dds->ddi.logical_socket;   /* Logical socket          */
    pkt.Attributes = CSIRQExcl;                   /* IRQ type=Exdusive       */
    pkt.IRQInfo1   = CSIRQInfo2Used;              /* IRQ = edge Mode         */
    pkt.IRQInfo2   = 1<<p_dds->ddi.intr_level;    /* Optional PCMCIA IRQ byte*/

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRRb", (ulong)p_dds, pkt.Socket, 
                (ulong)pkt.IRQInfo2);

    /* Request the Edge mode */
    if(( err = CardServices( CSRequestIRQ, &p_dds->clientID, NULL,
               sizeof( pkt ), (void *) &pkt )) == CSR_SUCCESS ) {
	IHS.flags = INTR_NOT_SHARED;
    }
    else {	/* Request the Level mode */
	pkt.IRQInfo1   = CSIRQInfo2Used | CSIRQLevel; /* IRQ = level Mode    */
        if(( err = CardServices( CSRequestIRQ, &p_dds->clientID, NULL,
                   sizeof( pkt ), (void *) &pkt )) == CSR_SUCCESS ) {
	    IHS.flags = INTR_LEVEL | INTR_POLARITY;
	}
    }

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRRe", err, p_dds->clientID, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  RequestConfiguration
 *
 * FUNCTION:              Configures the PC Card and socket.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *                        Card Services will apply power to the socket if the
 *                        socket was not powered.
 * RETURNS:               
 *        CSR_SUCCESS                 0x00 - The request succeeded
 *        CSR_BAD_ATTRIBUTE           0x02 - Sharing or Pulse request invalid
 *        CSR_BAD_IRQ                 0x06 - IRQ is invalid
 *        CSR_BAD_SOCKET              0x08 - Specified socket is invalid
 *        CSR_BAD_TYPE                0x0D - I/O and memory interface not support
 *        CSR_BAD_VCC                 0x0E - Requested Vcc is not available
 *        CSR_BAD_VPP                 0x0F - Requested Vpp is not available
 *        CSR_NO_CARD                 0x14 - No PC Card in socket
 *        CSR_BAD_ARG_LENGTH          0x1B - ArgLength arg are invalid
 *        CSR_BAD_ARGS                0x1C - IRQ info fields are invalid
 *        CSR_CONFIGURATION_LOCKED    0x1D - Configuration already set
 *        CSR_IN_USE                  0x1E - IRQ requested is already in-use
 *        CSR_BAD_HANDLE              0x21 - Client Handle is invalid
 */
/*****************************************************************************/

int RequestConfiguration( dds_t *p_dds )
{
    int             err;
    CSReqCfgPkt     pkt;

    pkt.Socket     = p_dds->ddi.logical_socket; /* Logical Socket             */
    pkt.Attributes = CSCfgIRQSteer;     /* bit-mapped                         */
    pkt.Vcc        = 50;                /* Vcc setting                        */
    pkt.Vpp1       = 120;               /* Vpp1 setting                       */
    pkt.Vpp2       = 120;               /* Vpp2 setting                       */
    pkt.IntType    = CSCfgIFIO;         /* Memory and I/O Interface           */
    pkt.ConfigBase = p_dds->config_base;/* Card Configuration Reg. base addr. */
                                        /* Get tuple(CISTPL_CONFIG)           */
    pkt.Present    = CSCfgOptValid |    /* Card Configuration Register present*/
                     CSCfgStatValid;
    pkt.Status     = 0x20;              /* Card status register setting       */
    pkt.Pin        = 0x00;              /* Card Pin register setting          */
    pkt.Copy       = 0x00;              /* Card Socket/Copy register setting  */
    pkt.ConfigIndex= 0x40 | p_dds->config_index;     /* Card Option register  */

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRCb", (ulong)p_dds,
                 (ulong)pkt.ConfigBase, 0);

    err = CardServices( CSRequestConfiguration, &p_dds->clientID, NULL,
                        sizeof( pkt ), (void *) &pkt );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRCe", err, 0, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  AccessConfigurationRegister
 *
 * FUNCTION:              
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *                        Card Services will apply power to the socket if the
 *                        socket was not powered.
 * RETURNS:               
 *        CSR_SUCCESS                 0x00 - The request succeeded
 */
/*****************************************************************************/

int AccessConfigurationRegister( dds_t *p_dds )
{
    int err;
    CSAccCfgRegPkt pkt;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cACb", (ulong)p_dds, 0, 0);

    /******** Writes the SREST field and puts the card in reset state ********/
    pkt.Socket = p_dds->ddi.logical_socket;
    pkt.Action = CSCfgRegWRITE;
    pkt.Offset = 0x0;
    pkt.Value  = 0xc1;
    if( err = CardServices( CSAccessConfigurationRegister, NULL, NULL,
                        sizeof( CSAccCfgRegPkt ), &pkt )) {
	return( err );
    }

    eieio(); DELAYMS(250);

    /******** After the reset bit has been cleared ***************************/
    pkt.Socket = p_dds->ddi.logical_socket;
    pkt.Action = CSCfgRegWRITE;
    pkt.Offset = 0x0;
    pkt.Value  = 0x40 | p_dds->config_index;
    if( err = CardServices( CSAccessConfigurationRegister, NULL, NULL,
                        sizeof( CSAccCfgRegPkt ), &pkt )) {
	return( err );
    }

    eieio(); DELAYMS(250);

    /******** Set I/O is 8 to 1 in the Card Configuration adn status Reg *****/
    pkt.Socket = p_dds->ddi.logical_socket;
    pkt.Action = CSCfgRegWRITE;
    pkt.Offset = 0x02;
    pkt.Value  = 0x20;
    if( err = CardServices( CSAccessConfigurationRegister, NULL, NULL,
                        sizeof( CSAccCfgRegPkt ), &pkt )) {
	return( err );
    }
    eieio();

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cACe", 0, 0, 0);

    return( CSR_SUCCESS );
}


/*****************************************************************************/
/* NAME:               GetMACaddr
 *
 * FUNCTION:           Get MAC address and set the DDS.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *    arguments p_dds: pointer to dds_struct.
 *
 *
 * RETURNS:
 *        CSR_SUCCESS           0x00 - The request succeeded
 *        CSR_BAD_ARG_LENGTH    0x1B - ArgLength arg are invalid
 *        CSR_BAD_BASE          0x03 - System memory address invalid
 *        CSR_BAD_HANDLE        0x21 - Client Handle is invalid
 *        CSR_BAD_SIZE          0x0A - Window size is invalid
 *        CSR_BAD_SOCKET        0x08 - Specified socket is invalid
 *        CSR_NO_CARD           0x14 - No PC Card in socket
 *        CSR_OUT_OF_RESOURCE   0x20 - Internal data spece is exhausted
 *        CSR_IN_USE            0x1E - IRQ requested is already in-use
 */
/*****************************************************************************/

int GetMACaddr( dds_t *p_dds )
{
    int         i, err;
    int         winID;
    CSReqWinPkt pkt;
    volatile uchar *atmaddr;       /* attribute memory add.*/

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cGAb", (ulong)p_dds, 0, 0);

    /******** Get the MAC address at attribute memory *************************/
    pkt.Socket      = p_dds->ddi.logical_socket;          /* Logical socket   */
    pkt.Base        = (char *)p_dds->ddi.shared_mem_addr; /* System base addr */
    pkt.Size        = 0x1000;                             /* Memory windowSize*/
    pkt.Attributes  = CSWinEnable|CSWin16Bits| CSWinAttr; /* bit-mapped       */
    pkt.AccessSpeed = (unsigned) 0x01; /* Window speed field <Set to 250ns>   */
                                       /* Get tuple dump(CISTPL_DEVICE):150 ns*/
    winID = p_dds->clientID;

    if( err = CardServices( CSRequestWindow, &winID, NULL,
                          sizeof( pkt ), (void *) &pkt ) ) {
        return( err );
    }

    /******** Set MAC address to the DDS **************************************/
    atmaddr = iomem_att(dd_ctrl.attr_map_ptr);

    /* Read the burned-in Ethernet address. */
    for (i = 0; i < ENT_NADR_LENGTH; i++) {
      WRK.burn_addr[i] = *(atmaddr + p_dds->ddi.shared_mem_addr + 0xff0 + i*2);
      eieio();
    }

    iomem_det((void *) atmaddr);

    err = CardServices( CSReleaseWindow, &winID, NULL, 0, NULL );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cGAe", err, (ulong)WRK.burn_addr, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  RequestWindow
 *
 * FUNCTION:              Requests a block of system memory space be assigned 
 *                        to a memory region of a PC Card in a socket.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 *        CSR_SUCCESS             0x00 - The request succeeded
 *        CSR_BAD_ATTRIBUTE       0x02 - Sharing or Pulse request invalid
 *        CSR_BAD_BASE            0x03 - System memory address invalid
 *        CSR_BAD_SOCKET          0x08 - Specified socket is invalid
 *        CSR_BAD_SIZE            0x0A - Window size is invalid
 *        CSR_NO_CARD             0x14 - No PC Card in socket
 *        CSR_BAD_SPEED           0x17 - Speed not supported
 *        CSR_BAD_ARG_LENGTH      0x1B - ArgLength arg are invalid
 *        CSR_IN_USE              0x1E - IRQ requested is already in-use
 *        CSR_BAD_HANDLE          0x21 - Client Handle is invalid
 *        CSR_OUT_OF_RESOURCE     0x20 - Internal data spece is exhausted
 */
/*****************************************************************************/

int RequestWindow(  dds_t *p_dds )
{
    int         err;
    CSReqWinPkt pkt;

    /***************** for Shared Memory Area *********************************/
    pkt.Socket       = p_dds->ddi.logical_socket;   /* Logical socket         */
    pkt.Base         = (char *)p_dds->ddi.shared_mem_addr;  /* SystemBase addr*/
    pkt.Size         = 0x4000;                      /* Memory window size 16K */
    pkt.Attributes   = CSWin16Bits | CSWinEnable;   /* Bit-Mapped             */
    pkt.AccessSpeed  = (unsigned) 0x01;/* Window speed field <Set to 250ns>   */
                                       /* Get tuple dump(CISTPL_DEVICE):100 ns*/
    p_dds->windowID  = p_dds->clientID;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRWb", (ulong)p_dds, (ulong)pkt.Base,
                 (ulong)p_dds->windowID);

    err = CardServices( CSRequestWindow, &p_dds->windowID, NULL,
                        sizeof( pkt ), (void *) &pkt );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cRWe", err, (ulong)p_dds->windowID, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  MapMemPage   
 *
 * FUNCTION:              Map Memory Page.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 *        CSR_SUCCESS             0x00 - The request succeeded
 *        CSR_BAD_ARG_LENGTH      0x18 - ArgLength arg are invalid
 *        CSR_BAD_HANDLE          0x21 - Client Handle is invalid
 *        CSR_BAD_OFFSET          0x07 - Offset is invalid
 *        CSR_BAD_PAGE            0x09 - Page is invalid
 *        CSR_NO_CARD             0x14 - No PC Card in socket
 */
/*****************************************************************************/

int MapMemPage( dds_t *p_dds )
{
    int             err;
    CSMapMemPagePkt pkt;

    pkt.CardOffset = PACKET_BUFFER_ADDRESS;      /* Card Offset Address      */
    pkt.Page       = 0x00;                       /* Page Number              */

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cMPb", (ulong)p_dds, pkt.CardOffset,
                 pkt.Page);

    err = CardServices( CSMapMemPage, &p_dds->windowID, NULL,
                        sizeof( pkt ), (void *) &pkt );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cMPe", err, (ulong)p_dds->windowID, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  ReleaseIO
 *
 * FUNCTION:              Releases previously requested I/O addresses.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 *        CSR_SUCCESS                 0x00 - The request succeeded
 *        CSR_BAD_SOCKET              0x08 - Specified socket is invalid
 *        CSR_BAD_ARG                 0x1C - Argument Packet are invalid
 *        CSR_CONFIGURATION_LOCKED    0x1D - Configuration already set
 *        CSR_BAD_ARG_LENGTH          0x18 - ArgLength arg are invalid
 *        CSR_BAD_HANDLE              0x21 - Client Handle is invalid
 */
/*****************************************************************************/

int ReleaseIO( dds_t *p_dds )
{
    int             err;
    CSIOPkt         pkt;

    pkt.Socket      = p_dds->ddi.logical_socket;  /* Logical Socket           */
    pkt.BasePort1   = (char *)p_dds->ddi.io_addr; /* Base port addr for range */
    pkt.NumPorts1   = 32;               /* Number of contiguous ports         */
    pkt.Attributes1 = 0;                /* Bit-mapped                         */
    pkt.BasePort2   = 0;                /* Base port address for range        */
    pkt.NumPorts2   = 0;                /* Number of contiguous ports         */
    pkt.Attributes2 = 0;                /* Bit-mapped                         */
    pkt.IOAddrLines = 0x10;             /* Number of IO address lines decoded */

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cLIb", (ulong)p_dds, (ulong)pkt.BasePort1, 0);

    err = CardServices( CSReleaseIO, &p_dds->clientID, NULL,
                        sizeof( pkt ), &pkt );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cLIe", err, 0 ,0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  ReleaseIRQ
 *
 * FUNCTION:              Releases a previously requested interrupt request line.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 *        CSR_SUCCESS                 0x00 - The request succeeded
 *        CSR_BAD_ATTRIBUTE           0x02 - Attributes field is invalid
 *        CSR_BAD_IRQ                 0x06 - IRQ is invalid
 *        CSR_BAD_ARG_LENGTH          0x1B - ArgLength arg are invalid
 *        CSR_CONFIGURATION_LOCKED    0x1D - Configuration already set
 *        CSR_BAD_ARG_LENGTH          0x18 - ArgLength arg are invalid
 *        CSR_BAD_HANDLE              0x21 - Client Handle is invalid
 */
/*****************************************************************************/

int ReleaseIRQ( dds_t *p_dds )
{
    int         err;
    CSRelIRQPkt pkt;

    pkt.Socket      = p_dds->ddi.logical_socket; /* Logical Socket         */
    pkt.Attributes  = CSIRQExcl;             /* Bit-mapped                 */
    pkt.AssignedIRQ = p_dds->ddi.intr_level; /* IRQ Number Assigned by CS  */

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cLRb", (ulong)p_dds, pkt.AssignedIRQ, 0);

    err = CardServices( CSReleaseIRQ, &p_dds->clientID, NULL,
                        sizeof( pkt ), &pkt );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cLRe", err, 0, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  ReleaseConfiguration
 *
 * FUNCTION:              Returns a PC Card and socket to simple memory only
 *                        interface and configuration zero. Card Services may
 *                        remove power from the socket if no clients have 
 *                        indicated their usage of the socket by an 
 *                        RequestWindow.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 *        CSR_SUCCESS           0x00 - The request succeeded
 *        CSR_BAD_ARG_LENGTH    0x1B - ArgLength arg are invalid
 *        CSR_BAD_HANDLE        0x21 - Client Handle is invalid
 *        CSR_BAD_SOCKET        0x08 - Specified socket is invalid
 */
/*****************************************************************************/

int ReleaseConfiguration( dds_t *p_dds )
{
    int         err;
    CSRelCfgPkt pkt;

    pkt.Socket = p_dds->ddi.logical_socket;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cLCb", (ulong)p_dds, 0, 0);

    return( CardServices( CSReleaseConfiguration, &p_dds->clientID, NULL,
                          sizeof( pkt ), &pkt ) );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cLCe", err, 0, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  ReleaseWindow
 *
 * FUNCTION:              Release a block of system memory space which was 
 *                        obrained previously by a corresponding RequestWindow.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 *        CSR_SUCCESS           0x00 - The request succeeded
 *        CSR_BAD_ARG_LENGTH    0x1B - ArgLength arg are invalid
 *        CSR_BAD_HANDLE        0x21 - Client Handle is invalid
 */
/*****************************************************************************/

int ReleaseWindow( dds_t *p_dds )
{
    int    err;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cLWb", (ulong)p_dds, p_dds->windowID, 0);

    err = CardServices( CSReleaseWindow, &p_dds->windowID, NULL, 0, NULL );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cLWe", err, 0, 0);

    return( err );
}


/*****************************************************************************/
/*
 * NAME:                  DeregisterClient
 *
 * FUNCTION:              Removes a client from the list of registered clients
 *                        maintained by Card Services. The ClientHandle returned
 *                        by RegisterClient is passwd in the Handle argument.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:		  *p_dds : Device structure pointer.
 *
 * RETURNS:               
 *        CSR_SUCCESS           0x00 - The request succeeded
 *        CSR_BAD_ARG_LENGTH    0x1B - ArgLength arg are invalid
 *        CSR_BAD_ATTRIBUTE     0x02 - Attributes field is invalid
 *        CSR_OUT_OU_RESOURCE   0x20 - Card Services has exhausted resource
 */
/*****************************************************************************/
int DeregisterClient( dds_t *p_dds  )
{
    int    err;

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cDRb", (ulong)p_dds, p_dds->clientID, 0);

    err = CardServices( CSDeregisterClient, &p_dds->clientID, NULL,
                        0, NULL );

    TRACE_PCMCIA(HKWD_IEN_ISA_OTHER, "cDRe", err, 0, 0);

    return( err );
}

int hold_lock(dds_t *p_dds)
{
	int err;
        CSaixVdrSpcPkt  pkt;

        pkt.Socket   = p_dds->ddi.logical_socket;
        pkt.funccode = CSaixLockSocket;
        pkt.subcode  = _CSaixHoldLock;
        err = CardServices( CSVendorSpecific, &p_dds->clientID, NULL,
                        sizeof( CSaixVdrSpcPkt ), &pkt );
        return( err );
}

int release_lock(dds_t *p_dds)
{
	int err;
        CSaixVdrSpcPkt  pkt;

        pkt.Socket   = p_dds->ddi.logical_socket;
        pkt.funccode = CSaixLockSocket;
        pkt.subcode  = _CSaixReleaseLock;
        err = CardServices( CSVendorSpecific, &p_dds->clientID, NULL,
                        sizeof( CSaixVdrSpcPkt ), &pkt );
        return( err );
}

void pcmcia_activity_lamp(dds_t *p_dds, int cmd)
{
        CSaixVdrSpcPkt  pkt;

        pkt.Socket   = p_dds->ddi.logical_socket;
        pkt.funccode = CSaixControlLED;
        pkt.subcode  = cmd;
        CardServices( CSVendorSpecific, &p_dds->clientID, NULL,
		     sizeof( CSaixVdrSpcPkt ), &pkt );
}
