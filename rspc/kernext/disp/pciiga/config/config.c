static char sccsid[] = "@(#)43	1.2  src/rspc/kernext/disp/pciiga/config/config.c, pciiga, rspc41B, 9504A 12/15/94 16:03:55";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: iga_config
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

#include "iga_INCLUDES.h"
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
 | iga_define routine to get adapter instance
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

iga_config (devno, cmd, uiop)
        dev_t           devno;
        long            cmd;
        struct uio     *uiop;
{
        struct fbdds   *dds;
        struct iga_ddf *ddf;
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

                dds = (struct fbdds *) xmalloc (sizeof (struct fbdds),
                                                3, pinned_heap);

                if (dds == NULL)
                {
                BUGLPR (dbg_cfg, BUGNTA, ("DDS xmalloc failure\n"));
                        iga_err (NULL, "PCIIGA", "CONFIG", "XMALLOC", dds, IGA_BAD_XMALLOC,
                                RAS_UNIQUE_1);
                        return (ENOMEM);
                }

                rc = uiomove (dds, sizeof (struct fbdds),
                              UIO_WRITE, uiop);
                if (rc != 0)
                        return (rc);
                BUGLPR (dbg_cfg, BUGNTA, ("Calling iga_define\n"));

                for (i = 0; i < 16; i++)
                {
                        BUGLPR(dbg_cfg,BUGGID,("RGB[%d] = %X\n",i,dds->ksr_color_table[i]));

                }

                rc = iga_define (devno, dds, sizeof (struct fbdds));

		if (rc)
		   xmfree(dds,pinned_heap);
	
                BUGLPR (dbg_cfg, BUGACT, ("iga_define - rc = %d\n", rc));

                return (rc);

        case CFG_TERM:

                /* In this case we would remove all malloc'd data
                 * and remove ourselves from the device switch table
                 * if we are the last device of this type.
                 * Since there can only be one IGA then remove
                 */
                BUGLPR(dbg_cfg,BUGNTA,("Calling iga_undefine\n"));

                rc = iga_undefine(devno);

                BUGLPR(dbg_cfg,BUGACT,("iga_undefine rc = %d\n",rc));

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
