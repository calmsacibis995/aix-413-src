static char sccsid[] = "@(#)80  1.2  src/rspc/kernext/pcmcia/ent/ient_err.c, pcmciaent, rspc41J, 9511A_all 3/9/95 03:03:07";
/*
static char sccsid[] = "@(#)73  1.4.1.1  src/rspc/kernext/isa/ient/ient_err.c, isaent, rspc41J 1/6/95 15:46:57";
*/
/*
 * COMPONENT_NAME: PCMCIAENT - IBM PCMCIAENT Ethernet Device Driver
 *
 * FUNCTIONS: 
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
 * NAME:     ient_tx_timeout
 *
 * FUNCTION: Ethernet driver transmit watchdog timer timeout routine.
 *          Try to recover the adapter once.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * RETURNS:      none
 */
/*****************************************************************************/

void
ient_tx_timeout(struct watchdog *p_wdt) /* watchdog timer ptr. */
{
    int        txpri;
    xmt_elem_t *tx_elem;
    dds_t      *p_dds = (dds_t *)((ulong)p_wdt - offsetof(dds_t, xmit_wdt));

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "XTOb", (ulong)p_wdt, 0, 0);

    txpri = disable_lock(PL_IMP, &TX_LOCK);

    /* If the wait transmit queue is empty, nothing to do */

    if (!WRK.xmits_queued)
    {
        WRK.xmit_wdt_active = FALSE;
        unlock_enable(txpri, &TX_LOCK);
        return;
    }

    /*
    ** The transmit wait queue is not empty, if any packets have been
    ** transmitted, just restart the timer and check again later
    */

    if (WRK.xmit_wdt_opackets != NDD.ndd_genstats.ndd_opackets_lsw)
    {
        TRACE_SYS(HKWD_IEN_ISA_XMIT, "XTOs",(ulong)p_dds,
                 (ulong)WRK.xmit_wdt_opackets,
                 (ulong)NDD.ndd_genstats.ndd_opackets_lsw);

        WRK.xmit_wdt_active = TRUE;  /* Just making sure. */

        TXWDT.count         = 0;
        TXWDT.restart       = IENT_XMIT_TIMEOUT;
        WRK.xmit_wdt_opackets = NDD.ndd_genstats.ndd_opackets_lsw;
        w_start(&TXWDT);

        unlock_enable(txpri, &TX_LOCK);

        return;
    }

    /* The transmission failed, log an error, and cleanup the element. */

    WRK.xmit_wdt_active = FALSE;
    WRK.tx_buf_in_use--;

    ENTSTATS.tx_timeouts++;

    m_freem(WRK.cur_tx_elem->mbufp);

    ADD_2_FREE_Q(WRK.cur_tx_elem);

    /* Check for waiting transmission elements. */

    if (WRK.xmits_queued)
    {
        GET_WAIT_ELEM(tx_elem);
        ient_xmit(p_dds, tx_elem);
    }

    ient_logerr(p_dds, ERRID_IEN_ISA_TMOUT, __LINE__, __FILE__, 0, 0, 0);

    unlock_enable(txpri, &TX_LOCK);
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "XTOe", 0, 0, 0);
    return;
}


/*****************************************************************************/
/*
 * NAME:     ient_cmd_timeout
 *
 * FUNCTION:
 *       
 *
 * EXECUTION ENVIRONMENT: interrupt level
 *
 * NOTES:
 *
 * RETURNS:  nothing
 */
/*****************************************************************************/

void
ient_cmd_timeout(struct watchdog *p_wdt)     /* watchdog timer ptr. */
{
    dds_t   *p_dds = (dds_t *) ((ulong)p_wdt - offsetof(dds_t, cmd_wdt));

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "TIMb", (ulong)p_wdt, 0, 0);
    ient_logerr(p_dds, ERRID_IEN_ISA_TMOUT, __LINE__, __FILE__, 0, 0, 0);
    TRACE_SYS(HKWD_IEN_ISA_OTHER, "TIMe", 0, 0, 0);
}


/*****************************************************************************/
/*
 * NAME:     ient_restart
 *
 * FUNCTION: Ethernet driver network recovery handler.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * NOTES:
 *
 * RETURNS:  
 *           0 - OK
 *           EIO - PIO error occurred during the start
 *           ENOCONNECT - adapter error occurred during the start
 */
/*****************************************************************************/

int
ient_restart(dds_t *p_dds,      /* pointer to device control area */
             int   err_code)    /* error reason code */
{

    ndd_statblk_t  stat_blk;   /* status block */
    ndd_t          *p_ndd = &(NDD);
    int ioa, rc;
    int pio_rc = 0;

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "NRSb", (ulong)p_dds, err_code, 0);

    /* pass a status block to demuxer */
    bzero(&stat_blk, sizeof(ndd_statblk_t));
    stat_blk.code = NDD_LIMBO_ENTER;
    stat_blk.option[0] = err_code;
    (*(p_ndd->nd_status))(p_ndd, &stat_blk);

    /* save the device start time for statistics */
    WRK.dev_stime = lbolt;

    /* Hard Reset the adapter to force a known state */

    /* set system timer to delay 10 ms. */
    STIMER_MS(10);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "NRSE", 0, 0, 0);
    return(0);
}


/*****************************************************************************/
/*
 * NAME:     ient_hard_fail
 *
 * FUNCTION: Set the adapter to DEAD state, notify user and cleanup any
 *         outstanding transmit/ioctl requests.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * NOTES:
 *
 * RETURNS:  
 *    none.
 */
/*****************************************************************************/

int
ient_hard_fail(dds_t *p_dds,     /* device control area ptr. */
               int err_code)     /* error reason code */
{
    ndd_statblk_t  stat_blk;   /* status block */
    ndd_t        *p_ndd = &(NDD);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "NhfB", (ulong)p_dds, err_code, 0);

    WRK.adap_state = DEAD_STATE;
    p_ndd->ndd_flags &= ~NDD_RUNNING;
    p_ndd->ndd_flags |= NDD_DEAD;

    ient_logerr(p_dds, ERRID_IEN_ISA_FAIL, __LINE__, __FILE__, 0, 0, 0);

    /* de-activate the adapter */
    ient_stop(p_dds);

    /* pass a status block to demuxer */
    bzero(&stat_blk, sizeof(ndd_statblk_t));
    stat_blk.code = NDD_HARD_FAIL;
    stat_blk.option[0] = err_code;
    (*(p_ndd->nd_status))(p_ndd, &stat_blk);

    TRACE_SYS(HKWD_IEN_ISA_OTHER, "NhfE", 0, 0, 0);
    return(0);
}
