static char sccsid[] = "@(#)74	1.1  src/rspc/kernext/disp/pciiga/intr/interrupt.c, pciiga, rspc411, 9436D411b 9/5/94 16:31:40";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
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

/*--------------------------------------------------------------------
 |
 | IDENTIFICATION:  IGA_INTR
 |
 | DESCRIPTIVE NAME:  IGA_INTR - Interrupt handler for Frame Buffer
 |
 | FUNCTIONS:   iga_intr
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
#include "iga_INCLUDES.h"
BUGXDEF (dbg_intr);

int
iga_intr (intdat)
        struct intr    *intdat;
{

#if S3_INTERRUPTS /* when IGA does interrupts */
        struct phys_displays *pd;
        struct vtmstruc *vp;
        struct iga_data *ld;
        struct iga_ddf *ddf;
        struct fbdds   *dds;
        ushort int_set;

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
        ddf = (struct iga_ddf *) pd->free_area;
        vp = pd->visible_vt;
        ld = (struct iga_data *) vp->vttld;
	dds = (struct fbdds *) vp->display->odmdds;

	ld->PCI_BUS_ATT(&ddf->pci_io_map);

        int_set = PCI_GETS(SUBSYS_STAT);/* see if interrupting */

        BUGLPR (dbg_intr, BUGGID, ("status = 0x%x\n", int_set));

        int_set &= S3_VALID_INTERRUPTS;
        clear_op = TRUE;

        if (int_set)
        {

                BUGLPR (dbg_intr, BUGGID, ("INTSET\n"));
                /* We have received a iga interrupt */
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
                                ddf->report.data[1] = int_set;/* IGA Interrupt Status */
                                ddf->report.data[2] = 0;
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
                                PCI_PUTS(SUBSYS_CNTL, S3_INTR_ENABLE_DATA); /* all interrupts off */

                                ddf->cmd &= SYNC_OFF | EVENT_OFF;
                                ddf->s_event_mask = 0;

                        }              /* end if SYNC_REQ or ASYNC_REQ */

                }                      /* end if EVENT_MODE */

                if (ddf->cmd & DIAG_MODE)
                {
                        BUGLPR (dbg_intr, BUGGID, ("DIAG MODE\n"));
                        if (int_set & S3_VSY_INT_SET)
                        {
                                ddf->diaginfo[7]++;
                        }
                        else if (int_set & S3_GE_BSY_SET)
                        {
                                ddf->diaginfo[8]++;
                        }
                        else if (int_set & S3_FIFO_OVF_SET)
                        {
                                ddf->diaginfo[9]++;
                        }
                        else if (int_set & S3_FIFO_EMP_SET)
                        {
                                ddf->diaginfo[10]++;
                        }
                }

                if (ddf->cmd & DMA_WAIT_REQ)
                {
                        /* IGA Don't Know DMA */
                        BUGLPR (dbg_intr, BUGGID, ("IGAdd Sent DMA Wait Interrupt CMD!!!!!!\n"));
                }
                else
                if (ddf->cmd & DMA_IN_PROGRESS)
                {
                        /* IGA Don't Know DMA */
                        BUGLPR (dbg_intr, BUGGID, ("IGAdd Sent DMA Interrupt CMD!!!!!!\n"));
                }

                /* Clear interrupt field in the adapter */
                /****************** iop->int_status &= int_type; ***********/
                BUGLPR (dbg_intr, BUGGID, ("\tclearing interrupt\n"));

                PCI_PUTS(SUBSYS_CNTL, int_set); /* clear interrupt bits by */
                                                /* storing them            */

                PCI_BUS_DET(ld->IO_base_address);
                i_reset (intdat);
                return (INTR_SUCC);

        }                              /* Int_type != 0 */

        /* If we get here then it is not our interrupt */
        PCI_BUS_DET(ld->IO_base_address);
        return (INTR_FAIL);
#endif
}
