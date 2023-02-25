static char sccsid[] = "@(#)74  1.5  src/rspc/kernext/disp/pcigga/config/close.c, pcigga, rspc41B, 9504A 1/12/95 17:14:09";
/* close.c */
/*
based on "@(#)66  1.3  src/bos/kernext/disp/wga/config/close.c, bos, bos410 10/28/93 15:34:55";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: gga_close
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

#include "gga_funct.h"
#include "gga_INCLUDES.h"
BUGXDEF (dbg_close);
GS_MODULE(gga_close);

/*-----------------------------------------------------------------------
 |
 | IDENTIFICATION: GGA_CLOSE
 |
 | DESCRIPTIVE NAME: GGA_CLOSE - Close routine for GGA
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
gga_close (devno, chan, ext)
        dev_t           devno;
        long            chan;          /* Channel number ignored by this
                                          routine */
        long            ext;           /* Extended system parameter - not
                                          used   */
{
        struct phys_displays *pd;
	struct gga_ddf  * ddf;
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
        devswqry (devno & ~VIDEO_MINOR, &status, &pd);

        while (pd != NULL)
        {
                if (pd->devno == (devno & ~VIDEO_MINOR))
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
                return (-1);           /* No display present */
        }

	ddf = (struct gga_ddf *) pd->free_area;
        GGA_ENTER_TRC0(ddf,gga_close,2,GGA_CLOSE);

        if (pd->open_cnt == 0)
	{
		GGA_PARM_TRC0(ddf,gga_close,2,GGA_CLOSE,GGA_DISPLAY_NOT_OPEN);
                return (-1);            /* display not open */
	}

	if (devno & VIDEO_MINOR) {	/* close a video device */
		video_close(devno, chan, ext, pd);
		return (0);
	}

        /* test to see if the device is in use */
        if (pd->usage > 0)
        {
		GGA_PARM_TRC0(ddf,gga_close,2,GGA_CLOSE,GGA_DISPLAY_IN_USE);
                return (-1);           /* Error condition - there are still
                                          open vts */
        }

        /* Clean up system resources */
        unpincode(gga_intr);

        /* set open_cnt field to 0 to indicate device closed and return */

        pd->open_cnt = 0;

        BUGLPR (dbg_close, BUGNTA, ("CLOSE(0)\n"));

	GGA_EXIT_TRC0(ddf,gga_close,2,GGA_CLOSE);

        return (0);
}
