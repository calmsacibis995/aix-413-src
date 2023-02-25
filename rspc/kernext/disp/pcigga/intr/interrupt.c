static char sccsid[] = "@(#)97  1.1  src/rspc/kernext/disp/pcigga/intr/interrupt.c, pcigga, rspc411, 9435D411a 9/2/94 16:05:30";
/* interrupt.c */
/*
based on "@(#)75  1.4  src/bos/kernext/disp/wga/intr/interrupt.c, bos, bos410 10/28/93 18:06:41";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

    /*----------------------------------------------------------*
     |								|
     |   THIS CODE IS AN EXAMPLE, GGA DOESN'T USE INTERRUPTS.	|
     |								|
     *----------------------------------------------------------*/

/*--------------------------------------------------------------------
 |
 | IDENTIFICATION:  GGA_INTR
 |
 | DESCRIPTIVE NAME:  GGA_INTR - Interrupt handler for Frame Buffer
 |
 | FUNCTIONS:   gga_intr
 |
 | INPUTS:  Interrupt structure pointer - same as phys display pointer
 |
 | OUTPUTS:  None.
 |
 | CALLED BY:  First level interrupt handler
 |
 | passed in call back functions for dma/event handling
 |
 |----------------------------------------------------------------------*/
#include "gga_INCLUDES.h"
BUGXDEF (dbg_intr);

int
gga_intr (intdat)
        struct intr    *intdat;
{

#ifdef FW_INTERRUPTS	/* Glacier not currently supporting interrupts. */

        struct phys_displays *pd;
        struct vtmstruc *vp;
        struct gga_data *ld;
        struct gga_ddf *ddf;
        struct gbdds   *dds;
        ulong           int_set,
                        int_enabled;

        extern time_t   time;
        char            clear_op;
        volatile ulong  i;

        /* This routine will be invoked when an interrupt occurs on the level */
        /* passed to i_init routine by the device init routine. The code must */
        /* go and look at the adapter and verify it was the cause of the     */
        /* interrupt.                                                    */
        /* If it did not cause the interrupt then it will return to the      */
        /* level interrupt handler with a code of INTR_FAIL.             */

        pd = (struct phys_displays *) intdat;
        ddf = (struct gga_ddf *) pd->free_area;
        vp = pd->visible_vt;
        ld = (struct gga_data *) vp->vttld;
	dds = (struct gbdds *) vp->display->odmdds

	ld->bus_mem_addr = PCI_MEM_ATT(&ddf->pci_mem_map);

        int_set =  *WTK_INTR_STAT;      /* see if interrupting */
        int_enabled = *WTK_INTR_ENBL;

        printf("status = 0x%x, interrupt enabled =0x%x\n", int_set, int_enabled);
        BUGLPR (dbg_intr, BUGGID,
           ("status = 0x%x, interrupt enabled =0x%x\n", int_set, int_enabled));

        clear_op = TRUE;

        if (int_set)
        {

                BUGLPR (dbg_intr, BUGGID, ("INTSET\n"));
                /* We have received a gga interrupt */
                /* Handle it based on mode flag  */
                if (ddf->cmd & EVENT_MODE)
                {
                        BUGLPR (dbg_intr, BUGGID, ("Event MODE\n"));
                        if (ddf->cmd & NORPT_EVENT)
                        {
                                BUGLPR (dbg_intr, BUGGID, ("Event MODE-No Report\n"));
                                /* No report event requested                      */
                                /* No processing needed for this type event        */
                                /*
                                 * Pick or other device specific data would
                                 * be read
                                 */
                                /* from the adapter in this mode.                    */
                        }
                        if (ddf->cmd & SYNC_REQ || ddf->cmd & ASYNC_REQ)
                        {
                                BUGLPR (dbg_intr, BUGGID, ("Event MODE-SYNC/ASYNC\n"));
                                /*
                                 * In event mode we have been asked to
                                 * develop
                                 */
                                /* an interrupt report and return it to the    */
                                /* callback function                       */
                                ddf->report.event = int_set;
                                ddf->report.time = time;
                                ddf->report.rcx = 0;
                                ddf->report.wa = 0;
                                ddf->report.data[0] = 0;
                                ddf->report.data[1] = int_set;    /* GGA Interrupt Status */
                                ddf->report.data[2] = int_enabled;/* GGA Interrupts
                                                                   * Currently Enabled */
                                ddf->report.data[3] = 0;

                                /* Clear timer */
                                tstop (ddf->cdatime);
                                ddf->timerset = FALSE;
                                /*
                                 * If it was a sync wait request then wake up
                                 * process
                                 */
                                if (ddf->cmd & SYNC_WAIT_REQ)
                                        e_wakeup (&ddf->sleep_addr);
                                else
                                        /* Invoke event callback routine */
                                        (ddf->callback) (pd->cur_rcm, &(ddf->report));
                                /*
                                 * clear interrupt mask - This turns all INTS
                                 * OFF !
                                 */
                                /************* iop->int_enable = 0x00; *****************************/
                                *WTK_INTR_ENBL = WTK_INTR_ENABLE_DATA; /* all interrupts off */
                                ddf->cmd &= SYNC_OFF | EVENT_OFF;
                                ddf->s_event_mask = 0;

                        }              /* end if SYNC_REQ or ASYNC_REQ */

                }                      /* end if EVENT_MODE */

                if (ddf->cmd & DIAG_MODE)
                {
                        BUGLPR (dbg_intr, BUGGID, ("DIAG MODE\n"));
                        if (int_set & WTK_VBLANK_SET) /*only one used by GGA */
                                ddf->diaginfo[7]++;
                }

                if (ddf->cmd & DMA_WAIT_REQ)
                {
                        /* GGA Don't Know DMA */
                        BUGLPR (dbg_intr, BUGGID, ("GGAdd Sent DMA Wait Interrupt CMD!!!!!!\n"));
                }
                else
                if (ddf->cmd & DMA_IN_PROGRESS)
                {
                        /* GGA Don't Know DMA */
                        BUGLPR (dbg_intr, BUGGID, ("GGAdd Sent DMA Interrupt CMD!!!!!!\n"));
                }

                /* Clear interrupt field in the adapter */
                /****************** iop->int_status &= int_type; ***********/
                BUGLPR (dbg_intr, BUGGID, ("\tclearing interrupt\n"));

                *WTK_INTR_STAT = int_set << 1;      /* clear interrupt bits by    */
                                                    /* making them control bits   */

                PCI_MEM_DET (ld->bus_mem_addr);
                i_reset (intdat);
                return (INTR_SUCC);

        }                              /* Int_type != 0 */

        /* If we get here then it is not our interrupt */
        PCI_MEM_DET (ld->bus_mem_addr);
        return (INTR_FAIL);
}



void
cda_timeout (wd)
        struct trb     *wd;
{
        struct phys_displays *pd;
        struct gga_ddf *ddf;

        /* This routine will be invoked when the time has expired */

        pd = (struct phys_displays *) wd->func_data;
        ddf = (struct gga_ddf *) pd->free_area;

        if (ddf->cmd & DMA_WAIT_REQ || ddf->cmd & EVENT_MODE)
        {
                /* If we get here timer has popped and dma has not completed */
                /* in one second wake up process                             */
                e_wakeup (&(ddf->sleep_addr));
                ddf->cmd &= DMA_COMPLETE;
                ddf->cmd &= EVENT_OFF;
                return;
        }




        if (ddf->jumpmode)
        {
                if (ddf->lastcount == ddf->jumpcount)
                {
                        ddf->timerset = FALSE;
                        ddf->scrolltime = FALSE;
                }
                else
                {
                        ddf->scrolltime = TRUE;
                        ddf->lastcount = ddf->jumpcount;
                        ddf->cdatime->timeout.it_value.tv_sec = 0;
                        ddf->cdatime->timeout.it_value.tv_nsec = 150000000;
                        ddf->timerset = TRUE;
                        tstart (ddf->cdatime);
                }
        }
        else
        {


                if (ddf->jumpcount > ddf->jthreshold)
                {
                        ddf->jumpmode = TRUE;
                        ddf->jumpcount = 0;
                        ddf->lastcount = -1;
                        ddf->cdatime->timeout.it_value.tv_sec = 0;
                        ddf->cdatime->timeout.it_value.tv_nsec = 200000000;
                        ddf->timerset = TRUE;
                        tstart (ddf->cdatime);
                }
                else
                {
                        ddf->jumpcount = 0;
                        ddf->timerset = FALSE;
                }

        }
#endif /* FW_INTERRUPTS */
}
