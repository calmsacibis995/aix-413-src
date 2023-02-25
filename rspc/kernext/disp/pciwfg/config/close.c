static char sccsid[] = "@(#)64  1.2  src/rspc/kernext/disp/pciwfg/config/close.c, pciwfg, rspc411, 9440A411d 10/5/94 00:45:41";
/* close.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: wfg_close
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

#include "wfg_funct.h"
#include "wfg_INCLUDES.h"
BUGXDEF (dbg_close);

/*-----------------------------------------------------------------------
 |
 | IDENTIFICATION: WFG_CLOSE
 |
 | DESCRIPTIVE NAME: WFG_CLOSE - Close routine for WFG
 |
 | FUNCTION: Standard device driver close routine - does opposite of
 | open in that it frees systems resources used by device
 | driver.
 |
 | INPUTS:   Device number
 | Channel number (ignored)
 | Extended paramter (ignored)
 |
 | OUTPUTS:  None.
 |
 | CALLS:  None.
 |
 |---------------------------------------------------------------------*/

long
wfg_close (devno, chan, ext)
        dev_t           devno;
        long            chan;          /* Channel number ignored by this
                                          routine */
        long            ext;           /* Extended system parameter - not
                                          used   */
{
        struct phys_displays *pd;
        long            i,
                        status;
        /*
         * This routine will check the usage field in the Phys_display
         * struct and see if it is non-zero. If so we can close the device
         * if not then a -1 is return to the caller
         */

        /*
         * Here will be the devsw traversal for devices which can have
         * multiple instances.
         */

        BUGLPR (dbg_close, BUGNTA, (">CLOSE\n"));
        devswqry (devno, (unsigned int *)&status, (unsigned char **)&pd);

        while (pd != NULL)
        {
                if (pd->devno == devno)
                        break;
                else
                        pd = pd->next;
        }

        if (pd == NULL)
        {
                return (-1);           /* No display present */
        }

        if (pd->open_cnt == 0)
                return (-1);            /* display not open */

        /* test to see if the device is in use */
        if (pd->usage > 0)
        {
                return (-1);           /* Error condition - there are still
                                          open vts */
        }

	/* Clean up system resources */
	unpincode(wfg_intr);

        /* set open_cnt field to 0 to indicate device closed and return */

        pd->open_cnt = 0;

        BUGLPR (dbg_close, BUGNTA, ("CLOSE(0)\n"));

        return (0);
}
