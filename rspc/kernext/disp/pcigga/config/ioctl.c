static char sccsid[] = "@(#)81  1.4  src/rspc/kernext/disp/pcigga/config/ioctl.c, pcigga, rspc41B, 9504A 1/12/95 17:14:13";
/* ioctl.c */
/*
based on "@(#)70  1.3  src/bos/kernext/disp/wga/config/ioctl.c, bos, bos410 10/28/93 15:38:39";
 *
 *   COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: gga_ioctl
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
BUGXDEF (dbg_ioctl);

/*-----------------------------------------------------------------------
 *
 * IDENTIFICATION: GGA_IOCTL
 *
 * DESCRIPTIVE NAME: GGA_IOCTL- Interrupt handler for Entry Display
 *
 * FUNCTION: Standard IOCTL entry point for AIX device driver.
 *
 * INPUTS: Device number
 *           Command
 *           Argument for command
 *           Device flag (not used by this driver)
 *           Channel (ignored by this driver)
 *           Extension parameter (ignored by this driver)
 *
 * OUTPUTS: None.
 *
 * CALLS: None.
 *
 ----------------------------------------------------------------------*/


long
gga_ioctl (devno, cmd, arg, devflag, chan, ext)
        dev_t           devno;         /* The device number of the adapter */
        long            cmd;           /* The command for the ioctl */
        long            arg;           /* The address of paramter block */
        ulong           devflag;       /* Flag indicating type of operation */
        long            chan;          /* Channel number ignored by this
                                        * routine */
        long            ext;           /* Extended system parameter - not
                                        * used   */
{

        short           i;
        struct phys_displays *pd;
        struct gga_ddf *ddf;
        int             status;
        struct gbdds   *dds;
        BUGLPR (dbg_ioctl, BUGGID, ("Entering IOCTL\n"));

        /* Get phys_display pointer THIS CODE ASSUMES the GGA is minor 0 */

        i = minor (devno);
        devswqry (devno & ~VIDEO_MINOR, &status, &pd);

        while (pd != NULL)
        {
                if (pd->devno == (devno & ~VIDEO_MINOR))
                        break;
                else
                        pd = pd->next;
        }

        if (pd == NULL)
                return (-1);           /* No display present */

        ddf = (struct gga_ddf *) pd->free_area;
        dds = (struct gbdds *) pd->odmdds;

        switch (cmd)
        {
        default:
	    {
		if((minor(devno) & VIDEO_MINOR) == VIDEO_MINOR) {
			return(video_ioctl(devno, cmd, arg, 
					devflag, chan, ext, pd));
		} else
			gga_err (NULL, "PCIGGA", "IOCTL", NULL, EINVAL, 
						GGA_BAD_CMD, RAS_UNIQUE_1);
			return (EINVAL);
            }
        }                              /* End of switch */
        BUGLPR (dbg_ioctl, BUGGID, ("Leaving IOCTL\n"));
        return (0);
} /* gga_ioctl */
