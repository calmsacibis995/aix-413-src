static char sccsid[] = "@(#)77  1.5  src/rspc/kernext/pcmcia/ent/ient_close.c, pcmciaent, rspc41J, 9516A_all 4/18/95 00:54:07";
/*
static char sccsid[] = "@(#)72  1.5.1.1  src/rspc/kernext/isa/ient/ient_close.c, isaent, rspc41J 1/6/95 15:46:55";
*/
/*
 * COMPONENT_NAME: PCMCIAENT - IBM PCMCIA Ethernet Device Driver
 *
 * FUNCTIONS: 	   ient_close
 *		   ient_stop
 *		   ient_cleanup
 *
 * ORIGINS:        27
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
#ifdef PCMCIAENT
  int i;
#endif

    /* Get the adapter structure */
    p_dds = (dds_t *) (((ulong) p_ndd) - offsetof(dds_t, ndd));

#ifdef PCMCIAENT
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "ClsB",(ulong)p_ndd,(ulong)dd_ctrl.num_opens,
        (ulong)WRK.adap_state); 
    if( WRK.adap_state == CLOSED_STATE ) {
        TRACE_SYS(HKWD_IEN_ISA_OTHER, "CLO1",(ulong)p_ndd,(ulong)dd_ctrl.num_opens,
            (ulong)WRK.adap_state); 
        return( 0 );
    }
#else
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "ClsB", (ulong)p_ndd, 0, 0);
#endif

    ipri = disable_lock(PL_IMP, &SLIH_LOCK);

    if (WRK.adap_state == OPEN_STATE)
    {
        WRK.adap_state = CLOSE_PENDING;
    }

    unlock_enable(ipri, &SLIH_LOCK);

#ifdef PCMCIAENT
    /* wait for 10 seconds at most */
    for ( i = 0; WRK.xmits_queued && i < 10; i++ ) {
#else
    while (WRK.xmits_queued)              /* Empty the xmitQ. */
    {
#endif
        DELAYMS(1000);
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

#ifdef PCMCIAENT
    pcmcia_activity_lamp(p_dds, _CSaixTurnOffLED);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "ClsE",(ulong)WRK.adap_state, 0, 0);
#else
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "ClsE", 0, 0, 0);
#endif
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
