static char sccsid[] = "@(#)19  1.6  src/rspc/kernext/disp/pcifga/ksr/makegp.c, pcifga, rspc411, 9432B411a 8/9/94 11:57:18";
/* makegp.c */
/*
based on "@(#)43      1.6.1.1  src/bos/kernext/disp/wga/ksr/makegp.c, bos, bos410 9/14/93 14:42:28";
 *
 *   COMPONENT_NAME: (PCIFGA) Weitek PCI graphics adapter device driver
 *
 *   FUNCTIONS:  fga_make_gp, fga_unmake_gp
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

BUGXDEF (dbg_makegp);

/*---------------------------------------------------------------------
 *
 * IDENTIFICATION:  MAKE_GP
 *
 * DESCRIPTIVE NAME: MAKE_GP - Graphics process support
 *
 * FUNCTION: Return adapter addressing information to graphics
 *             process
 *
 * INPUTS:   RCM device structure
 *             RCM process pointer
 *             Device dependent mapping structure pointer and length
 *             Trace pointer
 *
 * OUTPUTS:  None.
 *
 * CALLED BY: Device independent rcm routine make_gp.
 *
 * CALLS: None.
 *
 --------------------------------------------------------------------*/

long
fga_make_gp (pdev, pproc, map, len, trace)
        gscDev         *pdev;
        rcmProcPtr      pproc;
        struct fga_map *map;
        int             len;
        int            *trace;
{
        struct fbdds *dds;
        struct phys_displays *pd;
        struct fga_ddf *ddf;
        struct vtmstruc *vp;
        struct fga_data *ld;

        BUGLPR (dbg_makegp, BUGGID, ("Entry into make graphics process\n"));

        /* This routine fills in the register mappings for this instance */
        /* of the fga driver and returns to the gsc call */

        pd = (struct phys_displays *) pdev->devHead.display;
        dds = (struct fbdds *)   pd->odmdds;
        ddf = (struct fga_ddf *) pd->free_area;
        vp = pd->visible_vt;
        ld = (struct fga_data *) vp->vttld;

        ddf-> MEM_segment = NULL;

        if (len < sizeof (struct fga_map))
                return -1;

        map->screen_height_mm  = dds -> screen_height_mm;
        map->screen_width_mm   = dds -> screen_width_mm;

        map->screen_height_pix = ld->P_CRT_CONTROL->y_resolution;
        map->screen_width_pix  = ld->P_CRT_CONTROL->x_resolution;

        map->color_mono_flag   = ld->P_CRT_CONTROL->color_mono;
        map->monitor_type      = ddf->monitor_type;

        /*
         *  Set up segment register for user.  This picks up the bus id from
         *  the phys_displays structure.
         */

        pproc->procHead.srval = (pdev->devHead.display->interrupt_data.intr.bid);

        pd->io_map = (ulong) &ddf->pci_mem_map;
         
        /* reset PCI bus offset to zero */
        map->base_address      = 0;

        BUGLPR (dbg_makegp, BUGGID, ("Segment register value = %x\n", \
                pproc->procHead.srval));
#if 0 	/* Documentation only for u_iomem_att of memory segment */
        {
            /*
             * Here's how we would attach the memory segment.
             */
            BUGLPR (dbg_makegp, BUGGID, ("\tattaching MEM segment\n"));

            map->base_address =
                    (unsigned long)u_iomem_att(ddf->pci_mem_map);
            EIEIO;

            if( map->base_address == NULL )
            {
                BUGLPR (dbg_makegp, BUGGID, ("\tMEM segment attach failed!\n"));
                ddf-> MEM_segment = NULL;
                return -1;                 /* couldn't map the bus */
            }
            ddf->MEM_segment = map->MEM_segment;
        }
#endif

        return (0);
}

/*---------------------------------------------------------------------
 *
 * IDENTIFICATION:  UNMAKE_GP
 *
 * DESCRIPTIVE NAME: UNMAKE_GP - Graphics process support
 *
 * CALLED BY: Device independent rcm routine unmake_gp.
 *
 * CALLS:
 *
 --------------------------------------------------------------------*/

long
fga_unmake_gp (pdev, pproc)
        gscDev         *pdev;
        rcmProcPtr      pproc;
{
        struct phys_displays *pd;
        struct fga_ddf * ddf;

        BUGLPR (dbg_makegp, BUGGID, ("Entry into unmake graphics process\n"));

#if 0 	/* Documentation only */
        pd  = (struct phys_displays *) pdev -> devHead.display;
        ddf = (struct fga_ddf *)       pd   -> free_area ;

        if( ddf->MEM_segment != NULL)
        {
                BUGLPR (dbg_makegp, BUGGID,
                    ("\tdetaching MEM segment 0x%8.8x\n", ddf->MEM_segment));
                u_iomem_det(ddf->MEM_segment);
        }
#endif

        BUGLPR (dbg_makegp, BUGGID, ("Exit from unmake graphics process\n"));

        return 0;
}
