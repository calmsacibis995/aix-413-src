static char sccsid[] = "@(#)75  1.4  src/rspc/kernext/disp/pcigga/config/config.c, pcigga, rspc41B, 9504A 1/9/95 13:18:31";
/* config.c */
/*
based on "@(#)67  1.9  src/bos/kernext/disp/wga/config/config.c, bos, bos410 10/28/93 15:36:22";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: gga_config
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

#include "gga_INCLUDES.h"
BUGXDEF (dbg_cfg);

/*-----------------------------------------------------------------------
 |
 | IDENTIFICATION:  DD_CONFIG
 |
 | DESCRIPTIVE NAME: DD_CONFIG - Close routine for Pc Mono
 |
 | FUNCTION: Standard device driver config routine - Configures,
 | unconfigures and returns vpd to calling config method.
 |
 | Routine is passed in command and responds as follows:
 |
 | CFG_INIT - copies in dds information and invokes
 | gga_define routine to get adapter instance
 | into system.
 |
 | CFG_TERM - frees physical display structure thus
 | unconfiguring adapter instance.
 |
 | CFG_QVPD - Returns vital product data off of adapter
 |
 | INPUTS:   Device number
 | Command ( as listed above )
 | Uiop pointer to user space data (buffer to read vpd
 | into or dds information)
 |
 | OUTPUTS:  None.
 |
 | CALLED BY: Entry display config method
 |
 |
 |---------------------------------------------------------------------*/

gga_config (devno, cmd, uiop)
        dev_t           devno;
        long            cmd;
        struct uio     *uiop;
{
        struct gbdds   *dds;
        struct gga_ddf *ddf;
        int             rc,
                        i,
                        status;
        struct phys_displays *pd;
        char           *vpdaddr;

        /*
         * This routine switches off of the command passed in and either
         * inits terminates or does a query of the VPD on the adapter.
         */

        switch (cmd)
        {
        case CFG_INIT:

                /* In this case we need to get our driver in to the system
                 * by allocating a phys display struct and filling it in then
                 * adding our driver to the devsw table.  For this level of
                 * implementation we are compiled into the devsw so all we do
                 * is invoke dd_define  to build our phys display struct and
                 * hook it in
                 */

                dds = (struct gbdds *) xmalloc (sizeof (struct gbdds),
                                                3, pinned_heap);

                if (dds == NULL)
                {
                BUGLPR (dbg_cfg, BUGNTA, ("DDS xmalloc failure\n"));
                        gga_err (NULL, "PCIGGA", "CONFIG", "XMALLOC", dds, GGA_BAD_XMALLOC,
                                RAS_UNIQUE_1);
                        return (ENOMEM);
                }

                rc = uiomove (dds, sizeof( struct gbdds ) - sizeof( ulong ), 
							UIO_WRITE, uiop);
                if (rc != 0)
		{
			xmfree(dds,pinned_heap);
                        return (rc);
		}

                BUGLPR (dbg_cfg, BUGNTA, ("Calling gga_define\n"));

                rc = gga_define (devno, dds, sizeof (struct gbdds));

                if (rc != 0)
                        return (rc);

                rc = uiomove (&(dds->vram_config), sizeof( ulong ),
                              UIO_READ, uiop);

                BUGLPR (dbg_cfg, BUGACT, ("gga_define - rc = %d\n", rc));

                return (rc);

        case CFG_TERM:

                /* In this case we would remove all malloc'd data
                 * and remove ourselves from the device switch table
                 * if we are the last device of this type.
                 * Since there can only be one GGA then remove
                 */
                BUGLPR(dbg_cfg,BUGNTA,("Calling gga_undefine\n"));

                rc = gga_undefine(devno);

                BUGLPR(dbg_cfg,BUGACT,("gga_undefine rc = %d\n",rc));

                return(rc);

        case CFG_QVPD:

                /*
                 * This function returns the Vital Product Data from
                 * the adapter.
                 */

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
                        return (-1);   /* No display present */
                }
                else
                {
                        /*
                         * return the "not supported" string
                         */

                        vpdaddr = "No VPD is defined for this device.";
                        /*
                         * Write data out to user space
                         */

                        rc = uiomove (vpdaddr, strlen(vpdaddr), UIO_READ, uiop);
                        break;
                }
                break;

        default:
                return (-1);
        }

        return (0);
}
