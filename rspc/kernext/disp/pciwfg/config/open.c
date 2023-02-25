static char sccsid[] = "@(#)73  1.3 src/rspc/kernext/disp/pciwfg/config/open.c, pciwfg, rspc41J, 9507C 1/27/95 03:04:09";
/* open.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: wfg_open
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "wfg_funct.h"
#include "wfg_INCLUDES.h"
#include <sys/m_param.h>

BUGXDEF (dbg_open);
/*****************************************************************************
 * IDENTIFICATION:   wfg_open()                                              *
 *                                                                           *
 * DESCRIPTIVE NAME: Western Digital PCI Graphic Adapter device open routine *
 *                                                                           *
 * FUNCTIONS :       This routine is the standard device driver open routine *
 *                   which resets the WFG and allocates system resourses to  *
 *                   to necessary for the device driver.                     *
 *                                                                           *
 * INPUTS    :       device number                                           *
 *                   Flags for open type (ignored)                           *
 *                   Channel number(ignored)                                 *
 *                   Extended parameter  (ignored)                           *
 *                                                                           *
 * OUTPUTS   :       NONE                                                    *
 *                                                                           *
 * CALLS TO  :       wfg_reset()                                             *
 *****************************************************************************/

long
wfg_open (devno, flag, chan, ext)
        dev_t  devno; /* Major and Minor number of the device               */
        long   flag;  /* flags for the type of open...ignored by this routin*/
        long   chan;  /* channel number...ignored by this routine           */
        long   ext;   /* extended parameter..ignored by this routine        */
{
        long            i,
                        rc,
                        status;
        struct phys_displays *pd;
        struct fbdds   *dds;     /* contains config method data-prefilled */
        struct wfg_ddf *ddf;

        /* ---------------------------------------------------------------- *
         *  This routine is invoked by the config routine.  Open completes  *
	 *  installing the driver into the system and allocates             *
	 *  the interrupt and dma channels according to what was passed in  *
	 *  the dds. The routine also pins the interrupt  handler and       *
	 *  resets the adaptor. devno specifies the WFG, device switch      *
	 *  query returns the WFG's status in the switch table and          *
	 *  device dependent information pointed to by pd. pd contains      *
	 *  the dds, pre filled with data  pd contains the a pointer        *
         *  to vtmstruct (visable_vt).  vtmstruct contains  vttld.          *
         *  some vttld elements are dependent on the WFG VRAM               *
	 *  configuration and must be filled in this routine                *
         * ---------------------------------------------------------------- */

        BUGLPR (dbg_open, BUGNFO, ("open>devswq(devno%X)\n", devno));
        devswqry (devno, (unsigned int *)&status, (unsigned char **)&pd);

        while (pd != NULL)
        {
                if (pd->devno == devno)
                {
                        break;
                }
                else
                {
                        pd = pd->next;
                }
        }
        if (pd == NULL)
        {
                BUGLPR (dbg_open, BUGNTA, 
			   ("No Display in Device Switch Table (wfgopen)\n"));
                return (-1);           /* no display in device switch table */
        }

        /* ----------------------------------------------------- *
         *    If the driver is already open then we will exit    *
         * ----------------------------------------------------- */
        if (pd->open_cnt != 0)
        {
                return (0);
        }

        /* ------------------------------------------------------- *
         *   Set the open_cnt field in the physical display entry  *
         * ------------------------------------------------------- */
        pd->open_cnt = 1;

        /* -------------------------------- *
	 *   Pin driver code into memory    *
         * -------------------------------- */
	pincode (wfg_intr);

        /* -------------------------------- *
         *      Initialize structures       *
         * -------------------------------- */
        dds = (struct fbdds *)   pd->odmdds;
        ddf = (struct wfg_ddf *) pd->free_area;

        BUGLPR (dbg_open, BUGGID, ("POST INIT STRUCTS...ddf = %X\n", ddf));

        return (0);
}
