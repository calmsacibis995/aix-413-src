static char sccsid[] = "@(#)95  1.1  src/rspc/kernext/disp/pcifga/config/ioctl.c, pcifga, rspc411, GOLD410 4/15/94 18:37:46";
/* ioctl.c */
/*
based on "@(#)70  1.3  src/bos/kernext/disp/wga/config/ioctl.c, bos, bos410 10/28/93 15:38:39";
 *
 *   COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: fga_ioctl
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

#include "fga_INCLUDES.h"
BUGXDEF (dbg_ioctl);

/*-----------------------------------------------------------------------
 *
 * IDENTIFICATION: FGA_IOCTL
 *
 * DESCRIPTIVE NAME: FGA_IOCTL- Interrupt handler for Entry Display
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
fga_ioctl (devno, cmd, arg, devflag, chan, ext)
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
        struct fga_ddf *ddf;
        int             status;
        struct fbdds   *dds;
        BUGLPR (dbg_ioctl, BUGGID, ("Entering IOCTL\n"));

        /* Get phys_display pointer THIS CODE ASSUMES the FGA is minor 0 */

        i = minor (devno);
        devswqry (devno, &status, &pd);

        while (pd != NULL)
        {
                if (pd->devno == devno)
                        break;
                else
                        pd = pd->next;
        }

        if (pd == NULL)
                return (-1);           /* No display present */

        ddf = (struct fga_ddf *) pd->free_area;
        dds = (struct fbdds *) pd->odmdds;

        switch (cmd)
        {
        default:
                {
                        fga_err (NULL, "PCIFGA", "IOCTL", NULL, EINVAL, FGA_BAD_CMD, RAS_UNIQUE_1);
                        return (EINVAL);
                }
        }                              /* End of switch */
        BUGLPR (dbg_ioctl, BUGGID, ("Leaving IOCTL\n"));
        return (0);
} /* fga_ioctl */
