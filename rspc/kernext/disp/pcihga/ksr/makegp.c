static char sccsid[] = "@(#)39  1.7  src/rspc/kernext/disp/pcihga/ksr/makegp.c, pcihga, rspc411, 9432B411a 8/9/94 11:26:10";
/* makegp.c */
/*
 *
 *   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS:  hga_make_gp, hga_unmake_gp
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

#include "hga_INCLUDES.h"

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
 * CALLS:
 *
 --------------------------------------------------------------------*/

long
hga_make_gp (pdev, pproc, map, len, trace)
        gscDev         *pdev;
        rcmProcPtr      pproc;
        struct hga_map *map;
        int             len;
        int            *trace;
{
        ulong srval;
        struct vtmstruc *vp;
        struct fbdds *dds;
        struct phys_displays *pd;
        struct hga_data *ld;
        struct hga_ddf * ddf;

        BUGLPR (dbg_makegp, BUGGID, ("Entry into make graphics process\n"));

        /* This routine fills in the register mappings for this instance */
        /* of the hga driver and returns to the gsc call */

        pd  = (struct phys_displays *) pdev -> devHead.display;
        dds = (struct fbdds *)         pd   -> odmdds;
        vp  =                          pd   -> visible_vt;
        ld  = (struct hga_data *)      vp   -> vttld;
        ddf = (struct hga_ddf *)       pd   -> free_area ;

        ddf-> MEM_segment = NULL;
        ddf-> IO_segment = NULL;

        if (len < sizeof (struct hga_map))
                return -1;

        map->screen_height_mm  = dds -> screen_height_mm;
        map->screen_width_mm   = dds -> screen_width_mm;

        map->screen_height_pix = ld -> screen_height_pix;
        map->screen_width_pix  = ld -> screen_width_pix;

        map->color_mono_flag   = ld -> disp_type; /* Flag for mono or color */
        map->pixel_depth       = ld -> bpp;    /* number of bits per pixel */

        map->monitor_type      = ddf-> monitor_type; 

        /*
         *  Set up I/O segment register for user.
         */

        BUGLPR (dbg_makegp, BUGGID, ("\tattaching IO segment\n"));

        map->IO_segment =
                (unsigned long)u_iomem_att(&ddf->pci_io_map);

        if( map->IO_segment == NULL )
        {
            BUGLPR (dbg_makegp, BUGGID, ("\tIO segment attach failed!\n"));
            ddf-> IO_segment = NULL;
            return -1;                 /* couldn't map the I/O */
        }

        ddf-> IO_segment = map->IO_segment;

        BUGLPR (dbg_makegp, BUGGID, ("I/O segment register value = %x\n",
                map->IO_segment));

        /*
         * The following info is sufficient to attach the bus memory space.
         */

        pproc->procHead.srval = (pdev->devHead.display->interrupt_data.intr.bid);

	pd->io_map = (ulong) &ddf->pci_mem_map;

        /* reset PCI bus offset within the segment to zero */
        map->MEM_segment      = 0;
        
        BUGLPR (dbg_makegp, BUGGID, ("Memory segment register value = %x\n",
                pproc->procHead.srval));
#if 0	/* Documentation for u_iomem_att of memory segment */
        {
            /*
             * Here's how we would attach the memory segment.
             */
            BUGLPR (dbg_makegp, BUGGID, ("\tattaching MEM segment\n"));

            map->MEM_segment =
                    (unsigned long)u_iomem_att(&ddf->pci_mem_map);

            if( map->MEM_segment == NULL )
            {
                BUGLPR (dbg_makegp, BUGGID, ("\tMEM segment attach failed!\n"));
                ddf-> MEM_segment = NULL;
                return -1;             /* couldn't map the bus */
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
hga_unmake_gp (pdev, pproc)
        gscDev         *pdev;
        rcmProcPtr      pproc;
{
        struct phys_displays *pd;
        struct hga_ddf * ddf;

        BUGLPR (dbg_makegp, BUGGID, ("Entry into unmake graphics process\n"));

        pd  = (struct phys_displays *) pdev -> devHead.display;
        ddf = (struct hga_ddf *)       pd   -> free_area ;

        if( ddf->IO_segment != NULL)
        {
            BUGLPR (dbg_makegp, BUGGID,
                    ("\tdetaching IO segment 0x%8.8x\n", ddf->IO_segment));
	    u_iomem_det(ddf->IO_segment);
        }

#if 0	/* Documentation for u_iomem_att of memory segment */
        if( ddf->MEM_segment != NULL)
        {
            BUGLPR (dbg_makegp, BUGGID,
                    ("\tdetaching MEM segment 0x%8.8x\n", ddf->IO_segment));
	    u_iomem_det(ddf->MEM_segment);
        }
#endif

        BUGLPR (dbg_makegp, BUGGID, ("Exit from unmake graphics process\n"));

        return 0;
}

