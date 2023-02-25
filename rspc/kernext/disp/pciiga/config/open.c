static char sccsid[] = "@(#)50	1.2  src/rspc/kernext/disp/pciiga/config/open.c, pciiga, rspc41B, 9504A 12/15/94 16:24:13";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: iga_open
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
#include <sys/m_param.h>

BUGXDEF (dbg_open);
BUGXDEF (dbg_vramchk);
GS_MODULE(iga_open);

/*---------------------------------------------------------------------------
 * IDENTIFICATION:    iga_open()
 *
 * DESCRIPTIVE NAME:  S3 PCI Graphic Adapter device open routine
 *
 * FUNCTIONS :        This routine is the standard device driver open routine
 *                    which resets the IGA and allocates system resourses to
 *                    to necessary for the device driver.
 *
 * INPUTS    :        device number
 *                    Flags for open type (ignored)
 *                    Channel number(ignored)
 *                    Extended parameter  (ignored)
 *
 * OUTPUTS   :        NONE
 *
 * CALLS TO  :        iga_reset()
 *
 * REFERENCES:        Fairway adapter spec Phil Milling
 *                    WGA Adapter spec Ron Arroyo
 ---------------------------------------------------------------------------*/

long
iga_open (devno, flag, chan, ext)
        dev_t           devno;         /* Major and Minor number of the
                                        * device                 */
        long            flag;          /* flags for the type of
                                        * open.....ignored by this routin */
        long            chan;          /* channel number.....ignored by this
                                        * routine           */
        long            ext;           /* extended parameter.....ignored by
                                        * this routine       */
{
        long            i,
                        rc,
                        status;
        struct phys_displays *pd;
        struct fbdds   *dds;           /* contains config method
                                        * data-prefilled */
        struct iga_ddf *ddf;



        /*
         * This routine is invoked by the config routine.  Open
         * completes installing the driver into the system and
         * allocates the interrupt and dma channels according to what  was
         * passed in the dds.  The routine also pins the interrupt  handler
         * and resets the adaptor. devno
         * specifies the IGA, device switch query returns the IGA's status in
         * the switch table and device dependent information pointed to by
         * pd. pd contains the dds,
         * pre filled with data  pd contains the a pointer
         * to vtmstruct (visable_vt).  vtmstruct contains  vttld.
         * some vttld elements are dependent on the IGA VRAM configuration
         * and must be filled in this routine
         */

        BUGLPR (dbg_open, BUGNFO, ("open>devswq(devno%X)\n", devno));
        devswqry (devno, &status, &pd);

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
                BUGLPR (dbg_open, BUGNTA, ("No Display in Device Switch Table (igaopen)\n"));

                return (-1);           /* no display in device switch table */
        }

        ddf = (struct iga_ddf *) pd->free_area;
	IGA_ENTER_TRC0(ddf,iga_open,2,IGA_OPEN);

        /*
         * If the driver is already open then we will exit
         */

        if (pd->open_cnt != 0)
        {
		IGA_EXIT_TRC0(ddf,iga_open,2,IGA_OPEN);
                return (0);
        }

        /*
         * set the open_cnt field in the physical display entry
         */

        pd->open_cnt = 1;

        /* Pin driver code into memory */

        pincode ((void *) iga_intr);


        /* Initialize structures */
        dds = (struct fbdds *) pd->odmdds;
        ddf = (struct iga_ddf *) pd->free_area;

        BUGLPR (dbg_open, BUGNTA, ("POST INIT STRUCTS...ddf = %X\n", ddf));

	IGA_EXIT_TRC0(ddf,iga_open,2,IGA_OPEN);

        return (0);
}
