static char sccsid[] = "@(#)72  1.1  src/rspc/kernext/disp/pciwfg/config/ioctl.c, pciwfg, rspc411, 9434B411a 8/24/94 07:24:14";
/* ioctl.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: wfg_ioctl
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

#include "wfg_INCLUDES.h"
BUGXDEF (dbg_ioctl);

/*-----------------------------------------------------------------------
 *
 * IDENTIFICATION: WFG_IOCTL
 *
 * DESCRIPTIVE NAME: WFG_IOCTL
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
wfg_ioctl (devno, cmd, arg, devflag, chan, ext)
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
        struct wfg_ddf *ddf;
        int             status;
        struct fbdds   *dds;
        BUGLPR (dbg_ioctl, BUGGID, ("Entering IOCTL\n"));

        /* Get phys_display pointer THIS CODE ASSUMES the WFG is minor 0 */

        i = minor (devno);
        devswqry (devno, (unsigned int *)&status, (unsigned char **)&pd);

        while (pd != NULL)
        {
                if (pd->devno == devno)
                        break;
                else
                        pd = pd->next;
        }

        if (pd == NULL)
                return (-1);           /* No display present */

        ddf = (struct wfg_ddf *) pd->free_area;
        dds = (struct fbdds *) pd->odmdds;

        switch (cmd)
        {
        default:
                {
                        wfg_err (NULL, "WFGDD", "IOCTL", NULL, EINVAL, WFG_BAD_CMD, RAS_UNIQUE_1);
                        return (EINVAL);
                }
        }                              /* End of switch */
        BUGLPR (dbg_ioctl, BUGGID, ("Leaving IOCTL\n"));
        return (0);
} /* wfg_ioctl */
