static char sccsid[] = "@(#)72  1.5.1.3  src/rspc/kernext/isa/ient/ient_close.c, isaent, rspc41J, 9521A_all 5/23/95 13:37:13";
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *
 * FUNCTIONS:  ient_close
 *             ient_stop
 *             ient_cleanup
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ient_proto.h"

extern dd_ctrl_t dd_ctrl;

/*****************************************************************************/
/*
 * NAME:     ient_close
 *
 * FUNCTION: Ethernet driver close routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:  NS user by using the ns_free() service
 *
 * INPUT:
 *            p_ndd   - pointer to the ndd.
 *
 * RETURNS:  
 *            0 - successful
 */
/*****************************************************************************/

int
ient_close(ndd_t *p_ndd)    /* ndd in the dev_ctl area ptr */
{
    dds_t   *p_dds;
    int     ipri;
    int     i=0;

    /* Get the adapter structure */
    p_dds = (dds_t *) (((ulong) p_ndd) - offsetof(dds_t, ndd));

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "ClsB", (ulong)p_ndd, 0, 0);

    ipri = disable_lock(PL_IMP, &SLIH_LOCK);

    /*
     * if the driver is in a power management mode of suspend/hibernate
     * then put the caller to sleep until the driver resumes normal operation
     * except for open this needs to be done under a lock so that the state
     * check is valid
     */
     if (WRK.adap_state == SUSPEND_STATE)
          e_sleep_thread (&WRK.pm_event,&SLIH_LOCK, LOCK_HANDLER);

    if (WRK.adap_state == OPEN_STATE)
    {
        WRK.adap_state = CLOSE_PENDING;
    }

    unlock_enable(ipri, &SLIH_LOCK);


    /* Give some time to empty transmit queue, but if there
     * is still something left on the xmit queue after this, it 
     * will just get thrown away
     */
    while (i<10 && WRK.xmits_queued) {
        DELAYMS(1000);
        i++;
    }

    ipri = disable_lock(PL_IMP, &SLIH_LOCK);

    ient_stop(p_dds);

    WRK.adap_state = CLOSED_STATE;
    p_ndd->ndd_flags &= ~NDD_RUNNING;

    unlock_enable(ipri, &SLIH_LOCK);

    /* cleanup all the resources allocated for open */
    ient_cleanup(p_dds);

    simple_lock(&DD_LOCK);

    ient_del_cdt("dev_ctl", (char *)p_dds, sizeof(dds_t));

    dd_ctrl.num_opens--;

    if(!dd_ctrl.num_opens)
    {
        ient_del_cdt("dd_ctrl", (char *)&dd_ctrl, sizeof(dd_ctrl_t));
        dmp_del(ient_cdt_func);
    }

    simple_unlock(&DD_LOCK);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "ClsE", 0, 0, 0);
    return(0);
}

/*****************************************************************************/
/*
 * NAME:                  ient_stop
 *
 * FUNCTION:              De-activate the adapter.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * RETURNS:               none
 */
/*****************************************************************************/

int
ient_stop(dds_t *p_dds) /* dev_ctl area ptr */
{
    volatile uchar *ioaddr;      /* IO-bus handle. */

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CstB", (ulong)p_dds, 0, 0);

    /* Attach to the IO-space. */
    ioaddr = iomem_att(dd_ctrl.io_map_ptr);

    /* Stop the NIC. */
    ient_stop_NIC(p_dds, ioaddr);

    /* Detach from IO-space. */
    iomem_det((void *) ioaddr);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CstE", 0, 0, 0);
}

/*****************************************************************************/
/*
 * NAME:     ient_cleanup
 *
 * FUNCTION: Cleanup all the resources used by opening the adapter.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *              ient_close
 *
 * INPUT:
 *               p_dds       - pointer to the dev_ctl area.
 *
 * RETURNS:  
 *               0 - Success always
 */
/*****************************************************************************/

int
ient_cleanup(dds_t *p_dds)        /* dev_ctl area ptr */
{
    ient_multi_t *p_multi, *p_temp;
    xmt_elem_t             *tx_elem;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CLEb", (ulong)p_dds, 0, 0);

    /* Stop the transmit timeout timer. */

    if (WRK.xmit_wdt_active)
    {
        w_stop(&TXWDT);
    }

    /* free the multicast table extensions */

    p_multi = WRK.multi_table;

    while (p_multi)
    {
        p_temp = p_multi->next;
        net_free(p_multi, M_DEVBUF);
        p_multi = p_temp;
    }

    WRK.multi_table = NULL;

    /* Remove the timers. */
    ient_remove_timers(p_dds);

    /* De-register the interrupt handler */
    i_clear(&(IHS));

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "CLEe", 0, 0, 0);
    return(0);
}
