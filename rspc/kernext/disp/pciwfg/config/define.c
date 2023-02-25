static char sccsid[] = "@(#)68  1.5 src/rspc/kernext/disp/pciwfg/config/define.c, pciwfg, rspc41J, 9521A_all 5/21/95 20:58:41";
/* define.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: wfg_define, wfg_undefine
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
#include "wfg_funct.h"

#ifdef	PANEL_PROTECTION
extern struct wfg_ddf *wfgDDF;
extern struct fbdds *wfgDDS;
#endif	/* PANEL_PROTECTION */

int enable_wfg();

/****************************************************************************
 *                                                                          *
 * IDENTIFICATION   : bad_func()                                            *
 * DESCRIPTIVE NAME : Dummy routine                                         *
 * FUNCTIONS : No functions                                                 *
 * INPUTS    : None                                                         *
 * OUTPUTS   : None                                                         *
 * CALLED BY : wfg_define                                                   *
 * CALLS     : None                                                         *
 *                                                                          *
 ****************************************************************************/
int bad_func()
{
     return(0);
}

BUGXDEF (dbg_define);
/****************************************************************************
 *                                                                          *
 * IDENTIFICATION   : wfg_define()                                          *
 *                                                                          *
 * DESCRIPTIVE NAME : Define device routine                                 *
 *                                                                          *
 * FUNCTIONS : Procedure performs all house keeping functions necessary     *
 *             to get an adapter into the system.                           *
 *                                                                          *
 *               - Allocates physical display entry                         *
 *                   - sets up function pointers                            *
 *                   - initializes query information                        *
 *                                                                          *
 *               - Checks to see if any other adapters have been configured *
 *                 and adds this instance accordingly.                      *
 *                                                                          *
 * INPUTS    : Device number                                                *
 *             Pointer ans length of dds.                                   *
 *                                                                          *
 * OUTPUTS   : None                                                         *
 * CALLED BY : wfg_config                                                   *
 * CALLS     : None                                                         *
 *                                                                          *
 ****************************************************************************/

long
wfg_define (devno, dds, ddslen)
        dev_t           devno;
        struct fbdds   *dds;
        long            ddslen;
{
        struct phys_displays *pd,
                       *nxtpd;
        long            i;
        union
        {
                long            full;

                struct
                {
                        char            b1;
                        char            b2;
                        char            b3;
                        char            b4;
                }               byte;
        }               devdat;
        static char     count;
        char            slot, *str;

        struct wfg_ddf *ddf;
        struct devsw    wfg_devsw;
        int             rc,
                        status;
        extern          nodev ();
        extern long     wfg_config ();

        BUGLPR (dbg_define, BUGNTX, ("In WFG_DEFINE....devno=%X\n", devno));

        /* ----------------------------------------------------------------- *
         *  This routine will be evoked by the configuration menthod which   *  
         *  1) malloc space for a physical display structure                 *
         *  2) fill in the function pointer fields as well as the            *
         *     display type and the pointer to the dds structure.            *
         *  3) It then will look into the devsw table using the iodn in the  *
         *     dds and determine if any other like adapters have been put    *
         *     in the system.                                                *
         *    a) If none,it will invoke the devswadd routine to get the      *
         *       driver in the system.                                       *
         *    b) If the adaptor is already in the devsw, the code will walk  *
         *       the chain of structures and attach the struct to the end    *
         *       of the list.                                                *
         * ----------------------------------------------------------------- */

        /* ------------------------------------------------ *
         *    Malloc space for physical display structure   *
         * ------------------------------------------------ */

        BUGLPR (dbg_define, BUGNTX, ("Mallocing Phys Display Struct\n"));

        pd = (struct phys_displays *) xmalloc (sizeof (struct phys_displays),
                                               3, pinned_heap);
        if (pd == NULL)
        {
                wfg_err (NULL, "PCIWFG", "DEFINE", "XMALLOC", pd, 
				   WFG_BAD_XMALLOC, RAS_UNIQUE_1);
                return (ENOMEM);
        }
        bzero (pd, sizeof (struct phys_displays));


        /* ---------------------------------- *
         *    Get the device minor number     *
         * ---------------------------------- */
        count = minor (devno);
        count++;


        /* ------------------------------------------- *
         *    Initialze phyical display structure      *
         * ------------------------------------------- */

        BUGLPR (dbg_define, BUGNTX, ("Initializing Phys Display Struct\n"));

        pd->next                         = NULL;
        pd->interrupt_data.intr.next     = (struct intr *) NULL;
        pd->interrupt_data.intr.handler  = NULL;
        pd->interrupt_data.intr.bus_type = BUS_BID;
        pd->interrupt_data.intr.flags    = 0;
        pd->interrupt_data.intr.level    = NULL;
        pd->interrupt_data.intr.priority = NULL;
        pd->interrupt_data.intr.bid      = BID_ALTREG(dds->bid, PCI_BUSMEM);

        /* ------------------------------------------------------------ *
         *   Use the intr_args fields to get addressability to the pd   *
	 *   table and to set a unique value to determine if this was   *
	 *   a wfg interrupt                                            *
         * ------------------------------------------------------------ */

        BUGLPR (dbg_define, BUGNTX, ("Setting up function Pointers\n"));

        pd->same_level = NULL;
        pd->dds_length = ddslen;
        pd->odmdds = (char *) dds;

        /* ---------------------------- *
         *   Set up function pointers   *
         * ---------------------------- */
        pd->vttact      = vttact;    /* Activate                     */
        pd->vttcfl      = vttcfl;    /* Copy full lines              */
        pd->vttclr      = vttclr;    /* Clear rectangle              */
        pd->vttcpl      = vttcpl;    /* Copy a part of the line      */
        pd->vttdact     = vttdact;   /* Deactivate                   */
        pd->vttddf      = nodev;     /* Device Dependent stuff       */
        pd->vttdefc     = vttdefc;   /* Change the cursor shape      */
        pd->vttdma      = nodev;     /* DMA Access routine           */
        pd->vttterm     = vttterm;   /* Free any resources           */
        pd->vttinit     = vttinit;   /* setup new logical terminal   */
        pd->vttmovc     = vttmovc;   /* Move the cursor              */
        pd->vttrds      = vttrds;    /* Read a line segment          */
        pd->vtttext     = vtttext;   /* Write a string of chars      */
        pd->vttscr      = vttscr;    /* Scroll text on the VT        */
        pd->vttsetm     = vttsetm;   /* Set mode to KSR or Graphics  */
        pd->vttstct     = vttstct;   /* Change color mappings        */
	pd->vttpwrphase = vttdpm;    /* Display Power Management     */

        pd->make_gp     = wfg_make_gp;    /* Make a graphic process   */
        pd->unmake_gp   = wfg_unmake_gp;  /* Unmake a graphic process */

        pd->sync_mask    = bad_func;      /* Sync event support       */
        pd->async_mask   = bad_func;      /* Async event support      */
        pd->enable_event = bad_func;      /* Non report event support */

        pd->display_info.colors_total  = 16777215;
        pd->display_info.colors_active = 16;
        pd->display_info.colors_fg     = 16;
        pd->display_info.colors_bg     = 16;

        for (i = 0; i < 16; i++)
        {
                pd->display_info.color_table[i] = dds->ksr_color_table[i];
        }

        pd->display_info.font_width        = 8;
        pd->display_info.font_height       = 15;
        pd->display_info.bits_per_pel      = 1;
        pd->display_info.adapter_status    = 0xC0000000;
        pd->display_info.apa_blink         = 0x80000000;
        pd->display_info.screen_width_pel  = 640;
        pd->display_info.screen_height_pel = 480;
        pd->display_info.screen_width_mm   = dds->screen_width_mm;
        pd->display_info.screen_height_mm  = dds->screen_height_mm;

        /* -------------------------------- *
         *     WFG physical device id       *
         * -------------------------------- */
        devdat.full       = dds->devid;
        pd->disp_devid[0] = devdat.byte.b1;
        pd->disp_devid[1] = devdat.byte.b2;
        pd->disp_devid[2] = devdat.byte.b3;
        pd->disp_devid[3] = devdat.byte.b4;

        pd->usage         = 0;         /* Set to indicate no VTs open      */
        pd->open_cnt      = 0;         /* Set to indicate driver is closed */
        pd->devno         = devno;     /* Set device number                */

        BUGLPR (dbg_define, BUGNTX, 
		       ("Allocating  Device Dependent Data Area\n"));

        /* ---------------------------------------------------------- *
         *    Allocate area for device dependent function problems    *
         * ---------------------------------------------------------- */
        pd->free_area = (ulong *) xmalloc 
				 (sizeof (struct wfg_ddf), 3, pinned_heap);
        if (pd->free_area == NULL)
        {
                wfg_err (NULL, "PCIWFG", "DEFINE", "XMALLOC", pd->free_area, 
					 WFG_BAD_XMALLOC, RAS_UNIQUE_2);
                return (ENOMEM);
        }


        /* --------------------------------------------- *
         *     Initialize device dependent data          *
         * --------------------------------------------- */

        BUGLPR (dbg_define, BUGNTX, ("Initializing Device Dependent Data\n"));

#ifdef	PANEL_PROTECTION
        wfgDDF =
#endif	/* PANEL_PROTECTION */
        ddf = (struct wfg_ddf *) pd->free_area;
        bzero (ddf, sizeof (struct wfg_ddf));
        pd->num_domains = 1;
        pd->dwa_device = 0;

	ddf->dpms_enabled = TRUE;

        /* -------------------------------------------- *
         *    Set up the "pos" regs for this adapter    *
         * -------------------------------------------- */
        rc = enable_wfg( ddf, dds );
        if (rc != 0)
        {
                BUGLPR (dbg_define, BUGNTX, ("Unable to set up adapter.\n"));
                return rc;
        }

        /* -------------------------------------------- *
         *    Now decide where to put the structure     *
         * -------------------------------------------- */
        devswqry (devno, (unsigned int *)&status, (unsigned char **)&nxtpd);
        BUGLPR (dbg_define, BUGNTX, ("Did DevSwQry..........\n"));
        if (nxtpd != NULL)
        {
                /* ------------------------------------------------ *
                 *     When we reach here at least one other wfg    *
		 *     has been defined in the system.              *
		 *     WD90C24A2 can't share I/O space, so we       *
                 *     can't configure this one.                    *
                 * ------------------------------------------------ */

                BUGLPR (dbg_define, 0, ("Other WFG's ARE DEFINED!...\n"));
                return -1;
        }
        BUGLPR (dbg_define, BUGNTX, ("Did DevSwQry..........\n"));

        /* --------------------------------------------------------------- *
         *  if this pointer is null then the card  is being configged for  *
         *  the first time. In this case we must do a devswadd to get the  *
         *  driver into the devswitch.                                     *
         * --------------------------------------------------------------- */
        wfg_devsw.d_open     = wfg_open;
        wfg_devsw.d_close    = wfg_close;
        wfg_devsw.d_read     = nodev;
        wfg_devsw.d_write    = nodev;
        wfg_devsw.d_ioctl    = wfg_ioctl;
        wfg_devsw.d_strategy = nodev;
        wfg_devsw.d_select   = nodev;
        wfg_devsw.d_config   = wfg_config;
        wfg_devsw.d_print    = nodev;
        wfg_devsw.d_dump     = nodev;
        wfg_devsw.d_mpx      = nodev;
        wfg_devsw.d_revoke   = nodev;
        wfg_devsw.d_dsdptr   = (char *) pd;
        wfg_devsw.d_ttys     = NULL;
        wfg_devsw.d_selptr   = NULL;
        wfg_devsw.d_opts     = 0;

        /* --------------------------------------------------------------- *
         *   configure the WFG - WFG should be the only graphics device    *
         *   on the bus.  Therefore we config by doing a devswadd to       *
         *   get into the devswitch table                                  *
         * --------------------------------------------------------------- */
        rc = devswadd (devno, &wfg_devsw);

        BUGLPR (dbg_define, BUGNTX, ("Did DevSwAdd.........(%d).\n",rc));

        if (rc != 0)
        {
                wfg_err (NULL, "PCIWFG", "DEFINE", "DEVSWADD", rc, 
			 WFG_DEVSWADD, RAS_UNIQUE_3);
                return ( rc );
        }

#ifdef	PANEL_PROTECTION
	wfg_timeoutcf(1);
#endif	/* PANEL_PROTECTION */

        return (0);
}

BUGXDEF (dbg_undefine);
/****************************************************************************
 *                                                                          *
 * IDENTIFICATION   : wfg_undefine()                                        *
 *                                                                          *
 * DESCRIPTIVE NAME : Undefine device routine                               *
 *                                                                          *
 * FUNCTIONS : Free the allocated memory                                    *
 *                                                                          *
 * INPUTS    : Device number                                                *
 *                                                                          *
 * OUTPUTS   : None                                                         *
 * CALLED BY : wfg_config                                                   *
 * CALLS     : None                                                         *
 *                                                                          *
 ****************************************************************************/

long
wfg_undefine(
        dev_t   devno)
{
        struct phys_displays *  pd;
        struct phys_displays *  last_pd;
        struct phys_displays *  next_pd;
        long                    i;
        struct wfg_ddf *        ddf;
        struct fbdds *          dds;
        int                     rc;
        int                     status;
        int                     minors;

        BUGLPR (dbg_undefine, BUGNFO, ("\n\nEnter wfg_undefine \n"));
        rc = devswqry(devno, NULL, (unsigned char **)&pd);
        BUGLPR (dbg_undefine, BUGNTX, 
		      ("devswqry returns 0x%x, devno=0x%x\n",rc,devno));
        if (rc != 0)
                return rc;
        last_pd = NULL;
        minors = 0;

        while (pd) {
                if (pd->devno == devno)
                        break;
                last_pd = pd;
                minors ++;
                pd = pd->next;
        }

        if (pd == NULL) {
                BUGLPR (dbg_undefine, BUGNTX, 
			  ("No pd found for devno!! - return error\n"));
                wfg_err( NULL, "PCIWFG", "DEFINE", ENODEV, 
				WFG_NODEV, RAS_UNIQUE_4 );
                return ( ENODEV );
        }

        if (pd->next)
                minors ++;

        if (pd->open_cnt != 0) {
                BUGLPR (dbg_undefine, BUGNTX, 
		       ("Someone still has device open, return EBUSY\n"));
                return EBUSY;
        }

        /* ---------------------------------------------------- *
         *   We now have the pointer to the display structure   *
         *   free the ddf structure and the display structure   *
         *   set the pointer in the previous structure to null  *
         * ---------------------------------------------------- */
        ddf = (struct wfg_ddf *)pd->free_area;
        BUGLPR (dbg_undefine, BUGNTX, ("Free ddf at 0x%x\n",ddf));
        xmfree(ddf, pinned_heap);

        dds = (struct fbdds *)pd->odmdds;
        BUGLPR (dbg_undefine, BUGNTX, ("Free dds at 0x%x\n",dds));
        xmfree(dds, pinned_heap);

#ifdef	PANEL_PROTECTION
	wfgDDF = (struct wfg_ddf *)NULL;
	wfgDDS = (struct fbdds *)NULL;
#endif	/* PANEL_PROTECTION */

        /* ---------------------------------------------- *
         *   Dequeue this pd from the list and free it.   *
         * ---------------------------------------------- */
        if (minors == 0) {

                /* --------------------------- *
                 *    This is the last pd.     *
                 * --------------------------- */

                BUGLPR (dbg_undefine, BUGNTX, ("xmfree pd at 0x%x\n",pd));
                xmfree(pd, pinned_heap);
                BUGLPR (dbg_undefine, BUGNTX, ("devswdel\n"));
                devswdel(devno);

        } else {
                BUGLPR (dbg_undefine, BUGNTX, ("Searching for pd\n"));
                if (last_pd != NULL) {
                        last_pd->next = pd->next;
                        xmfree(pd, pinned_heap);
                } else {

                        /* ----------------------------------------------- *
                         *  This is the tricky case.                       *
                         *  If the one we are deleting is the first        *
                         *  in the list and there are more pds, we will    *
                         *  copy the second pd into the first one.         *
                         *  This is required because we can not get        *
                         *  to the pointer that points to the first one.   *
                         * ----------------------------------------------- */

                        next_pd = pd->next;
                        bcopy(  next_pd,        /* from ptr */
                                pd,             /* to ptr */
                                sizeof(struct phys_displays));
                        BUGLPR (dbg_undefine, BUGNTX, 
				 ("xmfree pd at 0x%x\n",next_pd));
                        xmfree(next_pd, pinned_heap);
                }
        }

#ifdef	PANEL_PROTECTION
	wfg_timeoutcf(-1);
#endif	/* PANEL_PROTECTION */

        BUGLPR (dbg_undefine, BUGNTX, ("Returning from wfg_undefine\n"));
        return 0;
}
