static char sccsid[] = "@(#)20	1.9  src/rspc/kernext/isa/tty/sf_pm.c, isatty, rspc41J, 9524D_all 6/14/95 03:10:40";
/*
 *   COMPONENT_NAME: isatty
 *
 *   FUNCTIONS: sf_pm_handler, do_suspend, do_suspend_dev, do_resume,
 *              save_uart_registers restore_uart_registers,
 *              check_current_activity, pm_block, sf_power
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#ifdef PM_SUPPORT

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
/* #include <sys/lockl.h> */		/* for lockl and unlockl	*/
#include <sys/atomic_op.h>		/* for atomic operations	*/
#include <sys/trchkid.h>		/* trace hook id's		*/
#include <sys/sysinfo.h>		/* sysinfo stuff		*/
#include <sys/signal.h>			/* SIGSAK			*/
#include <sys/pri.h>
#include <sys/sleep.h>			/* EVENT_NULL			*/
#include <sys/except.h>			/* PIO stuff			*/
#include <sys/dump.h>			/* dump structures		*/
#include <sys/str_tty.h>		 /* Streams defines		*/
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
#ifdef PCMCIA
#include <sys/pcmciacs.h>
#endif
#include <sys/pmdev.h>			/* Power Management header	*/

void save_uart_registers( struct sfcom *sc );
int restore_uart_registers( struct sfcom *sc );

/*
 * NAME: sf_pm_handler
 *
 * FUNCTION: PM handler
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES: This routine is to be registered to PM system
 *        and called by the PM system.
 *
 * RETURNS:
 *   PM_SUCCESS: success
 *   PM_ERROR:   error
 */
int sf_pm_handler(caddr_t private, int ctrl)
{
    int rc = PM_SUCCESS;
    struct sfcom *sc;
    struct sty_tty *tp;
    extern int sf_slih(struct intr *);
    volatile uchar *port;
    
    sc = (struct sfcom *) private;
    tp = & sc->sf_tty;
    
    db_trace(TR_TTY, (tp, "sf_pm:>sf_pm_handler mode %X->%X"
		      , sc->pm_handle.mode
		      , ctrl));
    
    switch ( ctrl )
    {
    case PM_DEVICE_FULL_ON:
    case PM_DEVICE_ENABLE:
    case PM_DEVICE_IDLE:
	switch ( sc->pm_handle.mode )
	{
	case PM_DEVICE_FULL_ON:
	case PM_DEVICE_ENABLE:
	case PM_DEVICE_IDLE:
	    /* do nothing */
	    sc->pm_handle.mode = ctrl;
	    break;
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
	    rc = do_resume( sc );
	    if ( rc != 0 ){
		rc = PM_ERROR;
		break;
	    }
	    sc->pm_handle.mode = ctrl;
	    rc = PM_SUCCESS;
	    break;
	default:
	    break;
	}
	break;

    case PM_DEVICE_SUSPEND:	
    case PM_DEVICE_HIBERNATION:
	switch( sc->pm_handle.mode )
	{
	case PM_DEVICE_FULL_ON:
	case PM_DEVICE_ENABLE:
	case PM_DEVICE_IDLE:
	    rc = do_suspend( sc, ctrl );
	    if ( rc != 0 ){
		rc = PM_ERROR;
		break;
	    }
	    sc->pm_handle.mode = ctrl;
	    rc = PM_SUCCESS;
	    break;
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
	    /* do nothing */
	    sc->pm_handle.mode = ctrl;
	    break;
	default:
	    break;
	}
	break;

    case PM_RING_RESUME_ENABLE_NOTICE:

#ifdef PCMCIA
	if ( ! (sc->pm_handle.attribute & PM_RING_RESUME_SUPPORT)){
	    rc = PM_ERROR;
	    break;
	}
	sc->sf_status.ring_resume = 1;
#endif
	break;

    case PM_PAGE_FREEZE_NOTICE:

	if (sc->sf_status.freeze){
	    rc = PM_SUCCESS;
	    break;
	}

	rc = pincode(sf_slih);
	if ( rc != 0 ){
	    rc = PM_ERROR;
	    break;
	}
	rc = pin(sc, sizeof(*sc));
	if ( rc != 0 ){
	    (void) unpincode(sf_slih);
	    rc = PM_ERROR;
	    break;
	}
	sc->pm_event_word = EVENT_NULL;

	simple_lock( &sc->sf_status.f_lock );
	sc->sf_status.freeze = 1;
	simple_unlock( &sc->sf_status.f_lock );

	rc = PM_SUCCESS;
	break;

    case PM_PAGE_UNFREEZE_NOTICE:

	if ( ! (sc->sf_status.freeze)){
	    rc = PM_SUCCESS;
	    break;
	}

	rc = unpin(sc, sizeof(*sc));
	if ( rc != 0 ){
	    rc = PM_ERROR;
	    break;
	}
	rc = unpincode(sf_slih);
	if ( rc != 0 ){
	    rc = PM_ERROR;
	    break;
	}
	rc = PM_SUCCESS;

	simple_lock( &sc->sf_status.f_lock );
	sc->sf_status.freeze = 0;
	simple_unlock( &sc->sf_status.f_lock );

	/* if any open pending and CD is on, wakeup the pending open */
	port = io_att(sc);
	if ((!tp->t_isopen) && (*MSR(port) & DCD) && (tp->t_openRetrieve)) {
		sc->sf_msr = sc->sf_delta = *MSR(port);
		openDisc_input (tp->t_openRetrieve, &tp->t_event, 0, cd_on);
	}
	io_det(port);
	break;

    default:
	break;
    }


    db_trace(TR_TTY, (tp, "sf_pm:<sf_pm_handler rc=%d", rc));
    return rc;
}
/*
 * NAME: do_suspend
 *
 * FUNCTION: executes suspend operation
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS:
 *   0: success
 *   1: fail
 */

int do_suspend( struct sfcom *sc, int mode )
{
    int rc = 0;
    struct sty_tty *tp = & sc->sf_tty;
    
    db_trace(TR_TTY, (tp, "sf_pm:>do_suspend mode=%X open=%d"
		      , mode
		      , sc->sf_open));

    if (sc->sf_open == 1){
	rc = check_current_activity( sc );
	if ( rc != 0 ){
	    rc = 1;
	    goto end;
        }
    }

    simple_lock( &sc->sf_status.f_lock );
    sc->sf_status.freeze = 2;
    simple_unlock( &sc->sf_status.f_lock );

    simple_lock( &sc->pm_lock );
    rc = do_suspend_dev( sc );
    simple_unlock( &sc->pm_lock );

    if ( rc != 0 )
    {
	rc = 1;
	goto end;
    }
    
    sc->pm_handle.activity = -1;

 end:
    db_trace(TR_TTY, (tp, "sf_pm:<do_suspend rc=%d", rc));
    
    return rc;
}

/*
 * NAME: do_suspend_dev
 *
 * FUNCTION: executes suspend operation to the device
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS:
 *   0: success
 *   1: fail. remain power up
 *   2: fatal fail.
 */
int do_suspend_dev( struct sfcom *sc )
{
    int rc = 0;
    struct sty_tty *tp = & sc->sf_tty;
    
#ifdef PCMCIA
    /* if pc card does not exist */
    if (sc->pcmcia.no_card)
    {
	goto end;
    }

    /* if the device is closed */
    if (sc->sf_open == 0)
    {
	/* if close and ring_resume enable */
	if (sc->sf_status.ring_resume)
	{
	    rc = sf_power(sc, PM_PLANAR_ON);
	    if (rc != 0){
		rc = 1;
		goto end;
	    }
	    rc = enable_pccard(sc);
	    if (rc != 0){
		rc = 1;
		goto end;
	    }
	    rc = setup_ring_resume(sc);
	    if (rc != 0){
		rc = 1;
		goto end;
	    }
	} else { /* if close and ring_resume disable */

	    rc = sf_power(sc, PM_PLANAR_OFF);
	    if (rc != PM_SUCCESS){
		rc = 1;
		goto end;
	    }
	}
	
    } else { /* if the device is opened (sf_open==1) */
	
	save_uart_registers( sc );

	/* if open and ring_resume enable */
	if (sc->sf_status.ring_resume)
	{
	    rc = setup_ring_resume(sc);
	    if (rc != 0){
		rc = 1;
		goto end;
	    }

	} else { /* if open and ring_resume disable */
	    /* power off the socket */
	    rc = sf_power(sc, PM_PLANAR_OFF);
	    if (rc != 0){
		rc = 1;
		goto end;
	    }

	    /* log error because data may be lost */
	    sty_err(sc->sf_tty.t_name, ERRID_EPOW_SUS, 0);
	}
    }
#else
    if (sc->sf_open == 1){
	save_uart_registers( sc );
    }

    rc = sf_power(sc, PM_PLANAR_OFF);
    if ( rc != PM_SUCCESS ){
	rc = 1;
	goto end;
    }
#endif
    
 end:
    return rc;
}

/*
 * NAME: check_current_activity
 *
 * FUNCTION: check device's activity
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:

 * RETURNS: 0: idle state
 *          1: busy state
 */

int check_current_activity( struct sfcom * sc )
{
    int rc = 0;
    int old_intr;
    struct sty_tty *tp = & sc->sf_tty;
    volatile uchar *port;
    db_trace(TR_TTY, (tp, "sf_pm:>chk_curr_act"));

    port = io_att( sc );
    
    old_intr = disable_lock( (int)tp->t_hpri, (simple_lock_t) &tp->t_lock);

    if ( *LSR(port) & DR || tp->t_busy || tp->t_wptr != tp->t_rptr )
    {
	rc = 1;
    } else {
	rc = 0;
    }

 end:
    unlock_enable( old_intr, (simple_lock_t) &tp->t_lock);
    io_det( port );

    db_trace(TR_TTY, (tp, "sf_pm:<chk_curr_act rc=%d", rc));
    
    return rc;
}

/*
 * NAME: do_resume
 *
 * FUNCTION: executes resume operation
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS: 0: success
 *          1: fail
 */
int do_resume( struct sfcom *sc )
{
    int rc = 0;
    unsigned char sf_mcr, sf_lcr;
    unsigned int card_state;
    unsigned int socket_state;
    struct sty_tty *tp = & sc->sf_tty;
    
    db_trace(TR_TTY, (tp, "sf_pm:>do_resume sf_open=%d", sc->sf_open));

    simple_lock( &sc->pm_lock );
    
    if (sc->sf_open == 1){
	/* log error because data might be lost */
	sty_err(sc->sf_tty.t_name, ERRID_EPOW_RES, 0);
    }

    sc->pm_handle.activity = 1;

#ifdef PCMCIA
    /* check PC Card existance */
    rc = GetStatus(sc, &card_state, &socket_state);
    if (rc != CSR_SUCCESS){
	rc = 1;
	goto end;
    }
    /* if no PC Card or unexpected PC Card, then release resources */
    if (!(card_state & CSStatDetected)
	|| CheckCard(sc->pcmcia.logical_socket ) != 0 )
    {
        sc->pcmcia.no_card = 1;
	(void) CardRemoval(sc);
	if ( sc->sf_open == 1 )
	{
	    rc = i_init(&sc->sf_slih);
	    if ( rc != INTR_SUCC ){
		rc = 1;
		goto end;
	    }
	}
	simple_lock( &sc->sf_status.f_lock );
	sc->sf_status.freeze = 1;
	simple_unlock( &sc->sf_status.f_lock );

	rc = 0;
	goto end;
    }
    else if (sc->pcmcia.no_card) 
    {
        sc->pcmcia.no_card = 0;
	(void) CardInsertion(sc);
    }

    /* check UART Register to determine if the card is still available */
    if (sc->sf_open == 1)
    {
	volatile uchar *port;

	port = io_att( sc );
	sf_mcr = *MCR(port);
	sf_lcr = *LCR(port);
	io_det(port);
    }

    /* if the PC Card is disable, then enable the PC Card */
    if ((sc->sf_open == 1 && 
	 (sc->sf_mcr != sf_mcr || sc->sf_lcr != sf_lcr)))
    {
	rc = sf_power(sc, PM_PLANAR_ON);
	if (rc != 0){
	    rc = 1;
	    goto end;
	}

	rc = enable_pccard(sc);
	if (rc != 0){
	    rc = 1;
	    goto end;
	}

    } else {

	if (sc->sf_status.ring_resume){

	    rc = clear_ring_resume(sc);
	    if (rc != CSR_SUCCESS){
		rc = 1;
		goto end;
	    }
	}
	if ( sc->sf_open == 0 ){

	    rc = sf_power(sc, PM_PLANAR_OFF);
	    if (rc != 0){
		rc = 1;
		goto end;
	    }
	}
    }

#else
    rc = sf_power(sc, PM_PLANAR_ON);
    if (rc != PM_SUCCESS){
	rc = 1;
	goto end;
    }
#endif

    simple_lock( &sc->sf_status.f_lock );
    sc->sf_status.freeze = 1;
    simple_unlock( &sc->sf_status.f_lock );

    if ( sc->sf_open == 1 )
    {
	/* restore the registers */
	rc = restore_uart_registers( sc );
    }

 end:
    simple_unlock( &sc->pm_lock );
    e_wakeup( & sc->pm_event_word );

    sc->sf_status.ring_resume = 0;
    db_trace(TR_TTY, (tp, "sf_pm:<do_resume rc=%d", rc));
    return rc;
}

/*
 * NAME: save_uart_registers
 *
 * FUNCTION: save UART registers
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 * SCR: not saved, because not used.
 * MSR: not saved. It only shows the current status.
 * LSR: not saved. It only shows the current status.
 * MCR: saved in a member of sfcom.
 * LCR: saved in a member of sfcom.
 * IIR: can not be saved. Read Only.
 * FCR: not saved. It always initialized as
 *   zero to sf_rtrig | FIFO_ENABLE.
 * IER: not saved because it is already saved.
 *   Enable Transmitter Holding Register Empty Register (ETBEI) is saved to
 *   sf_ethrei.
 *   Other registers (EDSSI, ELSI, and ERBFI) are set to 1 when open and
 *   0 when close.
 * 
 * RETURNS: void
 */
void save_uart_registers( struct sfcom *sc )
{
    volatile uchar *port;
    int old_intr;
    struct sty_tty *tp = & sc->sf_tty;

    i_clear(&sc->sf_slih);

    port = io_att( sc );
    old_intr = disable_lock( (int)tp->t_hpri, (simple_lock_t) &tp->t_lock);

    *IER(port) = 0;			/* disable port	*/
    
    sc->sf_mcr = *MCR(port);
    sc->sf_lcr = *LCR(port);
    set_dlab(port);
    sc->sf_dll = *DLL(port);
    sc->sf_dlm = *DLM(port);
    clr_dlab(port);
    
    unlock_enable( old_intr, (simple_lock_t) &tp->t_lock );
    io_det( port );
    
    db_trace(TR_TTY, (tp, "sf_pm: save_uart_reg: MCR=%X LCR=%X DLL=%X DLM=%X"
		      , sc->sf_mcr
		      , sc->sf_lcr
		      , sc->sf_dll
		      , sc->sf_dlm));
    
    return;
}
/*
 * NAME: restore_uart_registers
 *
 * FUNCTION: restore the UART registers
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES: See the NOTES of save_usar_registers.
 *
 * RETURNS: void
 */
int restore_uart_registers( struct sfcom *sc )
{
    volatile uchar *port;
    int old_intr, rc;
    struct sty_tty *tp = & sc->sf_tty;

    rc = i_init(&sc->sf_slih);
    if ( rc != INTR_SUCC )
    {
	return rc;
    }

    port = io_att( sc );
    old_intr = disable_lock( (int)tp->t_hpri, (simple_lock_t) &tp->t_lock);

    *FCR(port) = 0;
    eieio();
    *FCR(port) = sc->sf_rtrig | FIFO_ENABLE ;
    *IER(port) = ERBDAI | ELSI | EMSSI | ETHREI;
    *MCR(port) = sc->sf_mcr;
    *LCR(port) = sc->sf_lcr;

    /* TMP */
    { 
	uchar	s[1];
	sprintf(s, "%x", *FCR(port));
    }
    
    db_trace(TR_TTY, (tp, "sf_pm: restore_uart_reg FCR=%X IER=%X MCR=%X LCR=%X"
		      , *FCR(port)
		      , *IER(port)
		      , *MCR(port)
		      , *LCR(port)));
    
    set_dlab(port);
    *DLL(port) = sc->sf_dll;
    eieio();
    *DLM(port) = sc->sf_dlm;
    
    db_trace(TR_TTY, (tp, "restore_uart_reg: DLL=%X DLM=%X"
		      , *DLL(port)
		      , *DLM(port)));
    
    clr_dlab(port);
    
    unlock_enable( old_intr, (simple_lock_t) &tp->t_lock );
    io_det( port );

    return rc;
}

/*
 * NAME: pm_block
 *
 * FUNCTION: block comming requests
 *
 * EXECUTION ENVIRONMENT: process level and interrupt level
 * 
 * NOTES:
 *
 * RETURNS: void
 */
void pm_block( struct sfcom *sc )
{
    int old_intr;
    struct sty_tty *tp = & sc->sf_tty;

    /* if called from interrupt environment then return */
    if (sc->sf_status.intr || getpid() == (pid_t) -1){
	return;
    }

    /* if suspend or hibernation, block the request */
    simple_lock( &sc->sf_status.f_lock );
    if (sc->sf_status.freeze > 1)
    {
	(void) e_sleep_thread( &sc->pm_event_word, &sc->sf_status.f_lock,
			      LOCK_SIMPLE );
    }
    simple_unlock( &sc->sf_status.f_lock );

    /* set the activity flag */
    sc->pm_handle.activity = 1;

    return;
}

/*
 * NAME: sf_power
 *
 * FUNCTION: call pm_planar_control to turn on/off the power of controller
 *
 * EXECUTION ENVIRONMENT: process level
 * 
 * NOTES:
 *
 * RETURNS:
 *   PM_SUCCESS: success
 *   PM_ERROR:   error
 */
int sf_power(struct sfcom *sc, int cmd)
{
    int rc;

    rc = pm_planar_control(sc->sf_tty.t_dev, sc->pm_device_id, PM_PLANAR_QUERY);
    if( rc != PM_ERROR && ( rc & cmd ) )
    {
	rc = pm_planar_control(sc->sf_tty.t_dev, sc->pm_device_id, cmd);

    } else {
	/* PM_ERROR because DEVID is not registerd or */
	/* PM_PLANAR_ON/OFF is not required           */
	rc = PM_SUCCESS;
    }

    return rc;
}

#endif /* PM_SUPPORT */
