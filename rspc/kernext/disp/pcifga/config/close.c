static char sccsid[] = "@(#)88  1.1  src/rspc/kernext/disp/pcifga/config/close.c, pcifga, rspc411, GOLD410 4/15/94 18:37:30";
/* close.c */
/*
based on "@(#)66  1.3  src/bos/kernext/disp/wga/config/close.c, bos, bos410 10/28/93 15:34:55";
 *
 * COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: fga_close
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

#include "fga_funct.h"
#include "fga_INCLUDES.h"
BUGXDEF (dbg_close);

/*-----------------------------------------------------------------------
 |
 | IDENTIFICATION: FGA_CLOSE
 |
 | DESCRIPTIVE NAME: FGA_CLOSE - Close routine for FGA
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
fga_close (devno, chan, ext)
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
        devswqry (devno, &status, &pd);

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
        unpincode(fga_intr);

        /* set open_cnt field to 0 to indicate device closed and return */

        pd->open_cnt = 0;

        BUGLPR (dbg_close, BUGNTA, ("CLOSE(0)\n"));

        return (0);
}
