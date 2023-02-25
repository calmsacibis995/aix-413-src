static char sccsid[] = "@(#)82  1.6  src/rspc/kernext/disp/pcigga/config/open.c, pcigga, rspc41J, 9514A_all 3/30/95 14:05:10";
/* open.c */
/*
based on "@(#)71  1.5  src/bos/kernext/disp/wga/config/open.c, bos, bos410 10/28/93 15:39:08";
 *
 *   COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: gga_open
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
#include <sys/m_param.h>

GS_MODULE(gga_open);
BUGXDEF (dbg_open);
/*---------------------------------------------------------------------------
 * IDENTIFICATION:    gga_open()
 *
 * DESCRIPTIVE NAME:  Weitek PCI Graphic Adapter device open routine
 *
 * FUNCTIONS :        This routine is the standard device driver open routine
 *                    which resets the GGA and allocates system resourses to
 *                    to necessary for the device driver.
 *
 * INPUTS    :        device number
 *                    Flags for open type (ignored)
 *                    Channel number(ignored)
 *                    Extended parameter  (ignored)
 *
 * OUTPUTS   :        NONE
 *
 * CALLS TO  :        gga_reset()
 *
 * REFERENCES:        Glacier adapter spec Larry Knepper
 ---------------------------------------------------------------------------*/

long
gga_open (devno, flag, chan, ext)
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
        struct gbdds   *dds;           /* contains config method
                                        * data-prefilled */
        struct gga_ddf *ddf;

        /*
         * This routine is invoked by the config routine.  Open
         * completes installing the driver into the system and
         * allocates the interrupt and dma channels according to what  was
         * passed in the dds.  The routine also pins the interrupt  handler
         * and resets the adaptor. devno
         * specifies the GGA, device switch query returns the GGA's status in
         * the switch table and device dependent information pointed to by
         * pd. pd contains the dds,
         * pre filled with data  pd contains the a pointer
         * to vtmstruct (visable_vt).  vtmstruct contains  vttld.
         * some vttld elements are dependent on the GGA VRAM configuration
         * and must be filled in this routine
         */

        BUGLPR (dbg_open, BUGNFO, ("open>devswq(devno%X)\n", devno));
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
                BUGLPR (dbg_open, BUGNTA, ("No Display in Device Switch Table (ggaopen)\n"));
                return (-1);           /* no display in device switch table */
        }

	if (devno & VIDEO_MINOR) {
		rc = video_open(devno, flag, chan, ext, pd);
		if (rc)
			return (rc);
	}

        /*
         * If the driver is already open then we will exit
         */

	ddf = (struct gga_ddf *) pd->free_area;
        GGA_ENTER_TRC0(ddf,gga_open,2,GGA_OPEN); 

        if (pd->open_cnt != 0 || (devno & VIDEO_MINOR))
        {
        	GGA_EXIT_TRC0(ddf,gga_open,2,GGA_OPEN); 
                return (0);
        }

        /*
         * set the open_cnt field in the physical display entry
         */

        pd->open_cnt = 1;

        /* Pin driver code into memory */

        pincode ((void *) gga_intr);


        /* Initialize structures */
        dds = (struct gbdds *) pd->odmdds;
        ddf = (struct gga_ddf *) pd->free_area;

        BUGLPR (dbg_open, BUGNTA, ("POST INIT STRUCTS...ddf = %X\n", ddf));

       	GGA_EXIT_TRC0(ddf,gga_open,2,GGA_OPEN); 

        return (0);
}
