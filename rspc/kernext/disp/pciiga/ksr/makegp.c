static char sccsid[] = "@(#)57	1.3  src/rspc/kernext/disp/pciiga/ksr/makegp.c, pciiga, rspc41B, 9504A 12/15/94 16:42:23";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS:  iga_make_gp, iga_unmake_gp
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

BUGXDEF (dbg_makegp);
GS_MODULE(iga_makegp);
GS_MODULE(iga_unmakegp);

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
iga_make_gp (pdev, pproc, map, len, trace)
        gscDev         *pdev;
        rcmProcPtr      pproc;
        struct iga_map *map;
        int             len;
        int            *trace;
{
        ulong srval;
        struct vtmstruc *vp;
        struct fbdds *dds;
        struct phys_displays *pd;
        struct iga_data *ld;
        struct iga_ddf * ddf;

        BUGLPR (dbg_makegp, BUGGID, ("Entry into make graphics process\n"));

        /* This routine fills in the register mappings for this instance */
        /* of the iga driver and returns to the gsc call */

        pd  = (struct phys_displays *) pdev -> devHead.display;
        dds = (struct fbdds *)         pd   -> odmdds;
        vp  =                          pd   -> visible_vt;
        ld  = (struct iga_data *)      vp   -> vttld;
        ddf = (struct iga_ddf *)       pd   -> free_area ;

	IGA_ENTER_TRC0(ddf,iga_makegp,2,IGA_MAKE_GP);

        ddf-> MEM_segment = NULL;
        ddf-> IO_segment = NULL;

        if (len < sizeof (struct iga_map))
	{
		IGA_EXIT_TRC1(ddf,iga_makegp,2,IGA_MAKE_GP,-1);
                return -1;
	}

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

	    IGA_PARM_TRC0(ddf,iga_makegp,2,IGA_MAKE_GP,IGA_IO_ATTACH_FAILED);
	    IGA_EXIT_TRC0(ddf,iga_makegp,2,IGA_MAKE_GP);

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

        /* reset PCI bus offset within the segment to 0 */
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

	IGA_EXIT_TRC0(ddf,iga_makegp,2,IGA_MAKE_GP);

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
iga_unmake_gp (pdev, pproc)
        gscDev         *pdev;
        rcmProcPtr      pproc;
{
        struct phys_displays *pd;
        struct iga_ddf * ddf;

        BUGLPR (dbg_makegp, BUGGID, ("Entry into unmake graphics process\n"));

        pd  = (struct phys_displays *) pdev -> devHead.display;
        ddf = (struct iga_ddf *)       pd   -> free_area ;

	IGA_ENTER_TRC0(ddf,iga_unmakegp,2,IGA_UNMAKE_GP);

        if( ddf->IO_segment != NULL)
        {
            BUGLPR (dbg_makegp, BUGGID,
                    ("\tdetaching IO segment 0x%8.8x\n", ddf->IO_segment));

	    IGA_PARM_TRC0(ddf,iga_unmakegp,2,IGA_UNMAKE_GP,IGA_IO_DETACH);

	    u_iomem_det(ddf->IO_segment);
        }

/* Documentation for u_iomem_att of memory segment */
/*
#if 0
*/
        if( ddf->MEM_segment != NULL)
        {
            BUGLPR (dbg_makegp, BUGGID,
                    ("\tdetaching MEM segment 0x%8.8x\n", ddf->IO_segment));
	    u_iomem_det(ddf->MEM_segment);

	    IGA_PARM_TRC0(ddf,iga_unmakegp,2,IGA_UNMAKE_GP,IGA_MEM_DETACH);
        }
/*
#endif
*/

        BUGLPR (dbg_makegp, BUGGID, ("Exit from unmake graphics process\n"));

	IGA_EXIT_TRC0(ddf,iga_unmakegp,2,IGA_UNMAKE_GP);

        return 0;
}

