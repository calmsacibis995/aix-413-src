static char sccsid[] = "@(#)15  1.6 src/rspc/kernext/disp/pciwfg/ksr/makegp.c, pciwfg, rspc41J, 9516A_all 4/17/95 00:51:54";
/* makegp.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS:  wfg_make_gp, wfg_unmake_gp
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "wfg_INCLUDES.h"

int (**i_v7310_reset_overlay)();

BUGXDEF (dbg_makegp);
/**********************************************************************
 *                                                                    *
 * IDENTIFICATION:  MAKE_GP                                           *
 *                                                                    *
 * DESCRIPTIVE NAME: MAKE_GP - Graphics process support               *
 *                                                                    *
 * FUNCTION: Return adapter addressing information to graphics        *
 *             process                                                *
 *                                                                    *
 * INPUTS:   RCM device structure                                     *
 *             RCM process pointer                                    *
 *             Device dependent mapping structure pointer and length  *
 *             Trace pointer                                          *
 *                                                                    *
 * OUTPUTS:  None.                                                    *
 *                                                                    *
 * CALLED BY: Device independent rcm routine make_gp.                 *
 *                                                                    *
 * CALLS:                                                             *
 *                                                                    *
 **********************************************************************/

long
wfg_make_gp (pdev, pproc, map, len, trace)
        gscDev         *pdev;
        rcmProcPtr      pproc;
        struct wfg_map *map;
        int             len;
        int            *trace;
{
        ulong srval;
        struct vtmstruc *vp;
        struct fbdds *dds;
        struct phys_displays *pd;
        struct wfg_data *ld;
        struct wfg_ddf * ddf;

        BUGLPR (dbg_makegp, BUGGID, ("Entry into make graphics process\n"));

        /* ---------------------------------------------------------- *
         *   This routine fills in the register mappings for this     *
	 *   instance of the wfg driver and returns to the gsc call   *
         * ---------------------------------------------------------- */
        pd  = (struct phys_displays *) pdev -> devHead.display;
        dds = (struct fbdds *)         pd   -> odmdds;
        vp  =                          pd   -> visible_vt;
        ld  = (struct wfg_data *)      vp   -> vttld;
        ddf = (struct wfg_ddf *)       pd   -> free_area ;

        ddf-> MEM_segment = NULL;
        ddf-> IO_segment = NULL;

	/*
	 * makegp() does not need to disable LCD and CRT.
	 * This disabling is a role of loadddx.
	 */

#ifndef	PANEL_PROTECTION
        /* ---------------------------------------------------------- *
         *       Disable LCD Panel and CRT                            *
         * ---------------------------------------------------------- */
        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

        PUT_GRF_REG ( PR5, PR5_UNLOCK );
       	PUT_GRF_REG ( PR4, PR4_CRT );
        PUT_GRF_REG ( PR5, PR5_LOCK );

        PUT_CRTC_REG( PR1B, PR1B_UNLOCK );
       	PUT_CRTC_REG( PR19, PR19_DISABLE );
        PUT_CRTC_REG( PR1B, PR1B_LOCK );

        PCI_BUS_DET(ld->IO_base_address);
#endif	/* PANEL_PROTECTION */

        if (len < sizeof (struct wfg_map))
                return -1;

        map->screen_height_mm  = dds -> screen_height_mm;
        map->screen_width_mm   = dds -> screen_width_mm;

        map->screen_height_pix = ld -> screen_height_pix;
        map->screen_width_pix  = ld -> screen_width_pix;

        /* --------------------------------------------------------------- *
         *   LCD Panel screen pixel pitch                                  *
         *       LCD Panel  Panel Resolution     Panel Size    Dot pitch   *
         *       ---------  ----------------  ---------------  ---------   *
         *       F8515        640x480          210.24x158.40   0.33x0.33   *
         *       F8532        800x600          211.20x158.40   0.26x0.26   *
         * --------------------------------------------------------------- */
        map->panel_scrn_height_mm  = ld -> panel_scrn_height_mm; 
        map->panel_scrn_width_mm   = ld -> panel_scrn_width_mm;

        map->panel_scrn_height_pix = ld -> panel_scrn_height_pix;
        map->panel_scrn_width_pix  = ld -> panel_scrn_width_pix;

        if ( *i_v7310_reset_overlay == NULL )
        	map->model_type = ENTRY_MODEL;
	else
        	map->model_type = ADVANCED_MODEL;

        map->lcd_type   = ld->panel;
        map->vram_size  = ld->vram_size;

        map->color_mono_flag   = ld -> disp_type; /* Flag for mono or color */
        map->pixel_depth       = ld -> bpp;       /* # of bits per pixel    */

        map->monitor_type      = ddf-> monitor_type; 

        ld->gp_flag            = WFG_GP;

        /* -------------------------------------------------- *
         *       Set up I/O segment register for user.        *
         * -------------------------------------------------- */
        BUGLPR (dbg_makegp, BUGGID, ("attaching IO segment\n"));

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

        /* --------------------------------------------------------------- *
         *   Following info is sufficient to attach the bus memory space   *
         * --------------------------------------------------------------- */
        pproc->procHead.srval = 
			(pdev->devHead.display->interrupt_data.intr.bid);

        /* ------------------------------------------ *
         *    Get PCI bus offset within a segment     *
         * ------------------------------------------ */
	pd->io_map = (struct io_map *) &ddf->pci_mem_map;
        map->MEM_segment      = 0;

        BUGLPR (dbg_makegp, BUGGID, ("Memory segment register value = %x\n",
                pproc->procHead.srval));

#if 0	/* Documentation for u_iomem_att of memory segment */
        {
            /* ------------------------------------------------- *
             *   Here's how we would attach the memory segment   *
             * ------------------------------------------------- */
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

/**********************************************************************
 *                                                                    *
 * IDENTIFICATION:  UNMAKE_GP                                         *
 *                                                                    *
 * DESCRIPTIVE NAME: UNMAKE_GP - Graphics process support             *
 *                                                                    *
 * CALLED BY: Device independent rcm routine unmake_gp.               *
 *                                                                    *
 * CALLS:                                                             *
 *                                                                    *
 **********************************************************************/

long
wfg_unmake_gp (pdev, pproc)
        gscDev         *pdev;
        rcmProcPtr      pproc;
{
        struct phys_displays *pd;
        struct wfg_data *ld;
        struct wfg_ddf * ddf;

        BUGLPR (dbg_makegp, BUGGID, ("Entry into unmake graphics process\n"));

        pd  = (struct phys_displays *) pdev -> devHead.display;
        ddf = (struct wfg_ddf *)       pd   -> free_area ;
        ld  = (struct wfg_data *)      pd   -> visible_vt   -> vttld;

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

        ld->gp_flag            = WFG_GP_TERM;

        BUGLPR (dbg_makegp, BUGGID, ("Exit from unmake graphics process\n"));

        return 0;
}
