static char sccsid[] = "@(#)42	1.2  src/rspc/kernext/disp/pciiga/config/close.c, pciiga, rspc41B, 9504A 12/15/94 16:00:46";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: iga_close
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

#include "iga_funct.h"
#include "iga_INCLUDES.h"
BUGXDEF (dbg_close);
GS_MODULE(iga_close);

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
iga_close (devno, chan, ext)
        dev_t           devno;
        long            chan;          /* Channel number ignored by this
                                          routine */
        long            ext;           /* Extended system parameter - not
                                          used   */
{
        struct phys_displays *pd;
        struct iga_ddf  * ddf;
        long            i, status;
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

	ddf = (struct iga_ddf *) pd->free_area;
	IGA_ENTER_TRC0(ddf,iga_close,2,IGA_CLOSE);

        if (pd->open_cnt == 0)
	{
		IGA_PARM_TRC0(ddf,iga_close,2,IGA_CLOSE,IGA_DISPLAY_NOT_OPEN);
                return (-1);            /* display not open */
	}

        /* test to see if the device is in use */
        if (pd->usage > 0)
        {
		IGA_PARM_TRC0(ddf,iga_close,2,IGA_CLOSE,IGA_DISPLAY_IN_USE);
                return (-1);           /* Error condition - there are still
                                          open vts */
        }

        /* Clean up system resources */
        unpincode(iga_intr);

        /* set open_cnt field to 0 to indicate device closed and return */

        pd->open_cnt = 0;

        BUGLPR (dbg_close, BUGNTA, ("CLOSE(0)\n"));

	IGA_EXIT_TRC0(ddf,iga_close,2,IGA_CLOSE);

        return (0);
}
