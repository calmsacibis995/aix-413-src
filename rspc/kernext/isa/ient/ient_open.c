static char sccsid[] = "@(#)78  1.4.1.3  src/rspc/kernext/isa/ient/ient_open.c, isaent, rspc41J, 9517A_all 4/21/95 13:06:25";
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS: 
 *        ient_open
 *        ient_setup
 *        ient_tx_setup
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ient_proto.h"

extern dd_ctrl_t dd_ctrl;
extern ethernet_all_mib_t ient_mib_status;

/*****************************************************************************/
/*
 * NAME:     ient_open
 *
 * FUNCTION: Ethernet driver open routine.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:  
 */
/*****************************************************************************/

int
ient_open(ndd_t *p_ndd)    /* Ndd structure ptr. */
{
    dds_t   *p_dds;
    int rc;

    /* Get the adapter structure. */
    p_dds = (dds_t *) (((ulong) p_ndd) - offsetof(dds_t, ndd));
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "OPNb", (ulong)p_ndd, 0, 0);

    /*
     * if the driver is in a power management mode of suspend/hibernate
     * then put the caller to sleep until the driver resumes normal operation
     * the open check for the state does not need to be under a lock as
     * this routine cannot be called while the driver is running.
    */
    if (WRK.adap_state == SUSPEND_STATE)
         e_sleep(&WRK.pm_event, EVENT_SHORT);

    /* The driver/adapter must be in a closed state. */

    if (WRK.adap_state != CLOSED_STATE)
    {
        TRACE_BOTH(HKWD_IEN_ISA_ERR, "OPN1", (ulong)p_dds, (ulong)EBUSY,
                   (ulong)WRK.adap_state);
        return(EBUSY);
    }

    /* Set the device state and NDD flags */

    WRK.adap_state = OPEN_PENDING;
    p_ndd->ndd_flags |= NDD_UP | NDD_BROADCAST;

    /* Add our interrupt routine to kernel's interrupt chain */

    if (rc = i_init((struct intr *)(&(IHS))))
    {
        TRACE_BOTH(HKWD_IEN_ISA_ERR, "OPN2", (ulong)p_dds, (ulong)ENOCONNECT,
                   (ulong)rc);

        WRK.adap_state = CLOSED_STATE;
        p_ndd->ndd_flags &= ~(NDD_UP | NDD_BROADCAST);
        return(ENOCONNECT);
    }

    simple_lock(&DD_LOCK);

    if ((++dd_ctrl.num_opens) == 1)
    {
        /* Register for dump. */
        dmp_add(ient_cdt_func);
    }

    simple_unlock(&DD_LOCK);

    /* save the ndd start time for statistics */
    WRK.ndd_stime = lbolt;

    /* Activate the adapter and allocate all the resources needed */

    if (ient_start(p_dds))
    {
        WRK.adap_state = CLOSED_STATE;
        p_ndd->ndd_flags &= ~NDD_UP;

        simple_lock(&DD_LOCK);

        ient_del_cdt("DDS", (char *)p_dds, WRK.alloc_size);
        simple_unlock(&DD_LOCK);

        TRACE_SYS(HKWD_IEN_ISA_ERR, "Opn2", ENOCONNECT, 0, 0);
        return(ENOCONNECT);
    }
    else
    {
        if (rc = ient_setup(p_dds))
        {
            /* undo any resource setup */
            ient_cleanup(p_dds);

            WRK.adap_state = CLOSED_STATE;
            p_ndd->ndd_flags &= ~NDD_UP;

            simple_lock(&DD_LOCK);
            ient_del_cdt("DDS", (char *)p_dds, WRK.alloc_size);
            simple_unlock(&DD_LOCK);

            TRACE_SYS(HKWD_IEN_ISA_ERR, "Opn3", rc, 0, 0);
            return(rc);
        }

        /* update the device state and send a status block to demuxer */

        WRK.adap_state = OPEN_STATE;
        p_ndd->ndd_flags |= NDD_RUNNING;

        /* Clear the entstats counters. */
        ENTSTATS.mcast_rx_ok = 0;
        ENTSTATS.bcast_rx_ok = 0;
        ENTSTATS.mcast_tx_ok = 0;
        ENTSTATS.bcast_tx_ok = 0;

        TRACE_SYS(HKWD_IEN_ISA_OTHER, "OpnE", 0, 0, 0);
    }
    return(0);
}

/*****************************************************************************/
/*
 * NAME:     ient_setup
 *
 * FUNCTION: Setup the system resources for the adapter to operate.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * RETURNS:  
 *    0 - OK
 */
/*****************************************************************************/

ient_setup(dds_t *p_dds)       /* Device structure ptr. */
{
    int    i;                  /* Temp variable for loop counting        */
    int    rc;                 /* Local return code                      */

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "SETb", (ulong)p_dds, 0, 0);

    /* Setup the timers. */
    ient_setup_timers(p_dds);

    /* clear all ndd_genstats and device stats */
    bzero(&NDD.ndd_genstats, sizeof(ndd_genstats_t));
    bzero (&ENTSTATS, sizeof(ENTSTATS));
    bzero (&DEVSTATS, sizeof(DEVSTATS));

    bcopy(ETH_MIB_National8390, ient_mib_status.Generic_mib.ifExtnsEntry.chipset,
          CHIPSETLENGTH);

    /* TMP TMP */
    ient_mib_status.Ethernet_mib.Dot3StatsEntry.s_coll_frames =
        MIB_NOT_SUPPORTED;
    ient_mib_status.Ethernet_mib.Dot3StatsEntry.m_coll_frames =
        MIB_NOT_SUPPORTED;

    WRK.promiscuous_count = 0;
    WRK.badframe_count    = 0;
    WRK.enable_multi      = 0;
    WRK.multi_count       = 0;
    WRK.receiving         = FALSE;
    WRK.new_packet        = IEN_ISA_RCV_START + 1;

    /* Setup the wait queue and initialize tx. variables in the dds. */

    WRK.tx_waitq          = (xmt_elem_t *)((uint)p_dds + sizeof(dds_t));

    WRK.multi_table = (ient_multi_t *) NULL;

    /* Set up the transmit elements in the waiting list. */

    if (rc = ient_tx_setup(p_dds))
    {

        TRACE_SYS(HKWD_IEN_ISA_ERR, "SET1", rc, 0, 0);
        return(rc);
    }

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "SETe", 0, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 * NAME:     ient_tx_setup
 *
 * FUNCTION: Setup the TCWs, buffers and descriptors for transmit.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * RETURNS:  
 *           0 - OK
 */
/*****************************************************************************/

int
ient_tx_setup(dds_t *p_dds)  /* device control area ptr */
{
    int i;
    xmt_elem_t *cur_elem;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "TXSb", (ulong)p_dds, 0, 0);

    WRK.buf0.state = EMPTY;
    WRK.buf0.offset = XMIT_BUF1_OFFSET;
    WRK.buf0.elem = NULL;

    WRK.buf1.state = EMPTY;
    WRK.buf1.offset = XMIT_BUF2_OFFSET;
    WRK.buf1.elem = NULL;

    WRK.next_buf_xmit = BUF0;
    WRK.next_buf_load = BUF0;

    WRK.xmits_queued   = 0;
    WRK.nbr_in_wait_q  = 0;

    /* Reset the transmit wait list. */

    cur_elem = (xmt_elem_t *)&(WRK.tx_waitq[0]);

    for (i = 0; i < DDI.xmt_que_size; i++, cur_elem++)
    {
        cur_elem->next        = cur_elem + 1;
        cur_elem->bytes       = 0;
        cur_elem->in_use      = 0;
        cur_elem->mbufp       = 0;
    }

    /* Adjust the last one to NULL. */
    WRK.tx_waitq[DDI.xmt_que_size - 1].next = NULL;

    /* Setup the free queue. */
    WRK.tx_next_free      = &WRK.tx_waitq[0];
    WRK.tx_last_free      = &WRK.tx_waitq[DDI.xmt_que_size-1];
    WRK.tx_next_wait      = NULL;
    WRK.tx_last_wait      = NULL;

    /* Set up for transmit buffer descriptors */

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "TXSe", 0, 0, 0);
    return(0);
}


/*****************************************************************************/
/*
 * NAME:                  ient_setup_timers    
 *
 * FUNCTION:              Allocate the watchdog timers.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/

void
ient_setup_timers(dds_t   *p_dds)  /* Device structure ptr. */
{
    /* Setup the cmd timer structure */
    while(w_init((struct watchdog *)(&(CMDWDT))));
    WRK.cmd_wdt_active  = FALSE;
    p_dds->cmd_wdt_dds  = p_dds;
    CMDWDT.func         = ient_cmd_timeout;

    /* Setup the transmit timeout timer structures */
    while(w_init((struct watchdog *)(&(TXWDT))));
    WRK.xmit_wdt_active   = FALSE;
    p_dds->xmit_wdt_dds   = p_dds;
    TXWDT.func            = ient_tx_timeout;

}


/*****************************************************************************/
/*
 * NAME:                  ient_remove_timers    
 *
 * FUNCTION:              De-allocate the watchdog timers.
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * RETURNS:               nothing
 *
 */
/*****************************************************************************/

void
ient_remove_timers(dds_t   *p_dds)  /* Device structure ptr. */
{
    /* Reset the cmd timer structure */
    w_stop((struct watchdog *)(&(CMDWDT)));
    WRK.cmd_wdt_active  = FALSE;
    while (w_clear(&CMDWDT));

    /* Reset the transmit timeout timer structures */
    w_stop((struct watchdog *)(&(TXWDT)));
    WRK.xmit_wdt_active   = FALSE;
    while (w_clear(&TXWDT));
}
