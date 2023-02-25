static char sccsid[] = "@(#)46	1.6  src/rspc/kernext/disp/pciiga/config/define.c, pciiga, rspc41J, 9515B_all 3/22/95 16:57:48";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: iga_define, iga_undefine
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
#include <sys/pm.h>              /* for AIX Power Management */
#include "iga_INCLUDES.h"
#include "iga_funct.h"
BUGXDEF (dbg_define);
BUGXDEF (dbg_undefine);

GS_MODULE (iga_define);

int enable_iga();
extern long iga_memtrace_init();
extern long iga_memtrace_term();
extern void wakeup_device_pm(); 

/*-----------------------------------------------------------------------
 |
 | IDENTIFICATION: IGA_DEFINE
 |
 | DESCRIPTIVE NAME: IGA_DEFINE - Define device routine for Entry disp
 |
 | FUNCTION: Procedure performs all house keeping functions necessary
 | to get an adapter into the system.
 |
 | - Allocates physical display entry
 | -- sets up function pointers
 | -- initializes query information
 |
 | - Sets POS registers on the adapter.
 |
 | - Checks to see if any other adapters have been configured
 | and adds this instance accordingly.
 |
 | INPUTS:   Device number
 | Pointer ans length of dds.
 |
 | OUTPUTS:  None.
 |
 | CALLED BY: iga_config
 |
 | CALLS:    None.
 |
 |---------------------------------------------------------------------*/
int bad_func()
{
     return(0);
}

long
iga_define (devno, dds, ddslen)
        dev_t           devno;
        struct fbdds   *dds;
        long            ddslen;
{
        struct phys_displays *pd,
                       *nxtpd;
	struct pm_handle * p_pmdata;
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

        struct iga_ddf *ddf;
        struct devsw    iga_devsw;
        int             rc,
                        status;
        extern          nodev ();
        extern long     iga_config ();
        BUGLPR (dbg_define, BUGNTX, ("In IGA_DEFINE....devno=%X\n", devno));

        /*
         * This routine will be evoked by the configuration menthod which
         * 1) malloc space for a physical display structure
         * 2) fill in the function pointer fields as well as the
         *    interrupt data, the display type and the pointer to the
         *    dds structure.
         * 3) It then will look into the devsw table using the iodn in the
         *    dds and determine if any other like adapters have been put in
         *    the system.
         *   a) If none,it will invoke the devswadd routine to get the
         *      driver in the system.
         *   b) If the adaptor is already in the devsw, the code will walk
         *      the chain of structures and attach the struct to the end of
         *      the list.
         */


        BUGLPR (dbg_define, BUGNTX, ("Mallocing Phys Display Struct\n"));

        /* malloc space for physical display structure */

        pd = (struct phys_displays *) xmalloc (sizeof (struct phys_displays),
                                               3, pinned_heap);
        if (pd == NULL)
        {
                iga_err (NULL, "PCIIGA", "DEFINE", "XMALLOC", pd, IGA_BAD_XMALLOC, RAS_UNIQUE_1);
                return (ENOMEM);
        }
        bzero (pd, sizeof (struct phys_displays));
        BUGLPR (dbg_define, BUGNTA, ("in igadefine\n"));

        count = minor (devno);
        count++;

        BUGLPR (dbg_define, BUGNTX, ("Initializing Phys Display Struct\n"));

        /* Initialze phyical display structure */

        pd->next                         = NULL;
        pd->interrupt_data.intr.next     = (struct intr *) NULL;
        pd->interrupt_data.intr.handler  = NULL;
        pd->interrupt_data.intr.bus_type = BUS_BID;
        pd->interrupt_data.intr.flags    = 0;
        pd->interrupt_data.intr.level    = NULL;
        pd->interrupt_data.intr.priority = NULL;
        pd->interrupt_data.intr.bid      = BID_ALTREG(dds->bid, PCI_BUSMEM);

        /*
         * Use the intr_args fields to get addressability to the pd table
         * and to set a unique value to determine if this was a iga
         * interrupt
         */


        BUGLPR (dbg_define, BUGNTX, ("Setting up function Pointers\n"));
        pd->same_level = NULL;
        pd->dds_length = ddslen;
        pd->odmdds = (char *) dds;

        /* set up function pointers */

        pd->vttact = vttact;           /* Move the data from this terminal to
                                        * the display */
        pd->vttcfl = vttcfl;           /* Move lines around        */
        pd->vttclr = vttclr;           /* Clear a box on screen    */
        pd->vttcpl = vttcpl;           /* Copy a part of the line  */
        pd->vttdact = vttdact;         /* Mark the terminal as being
                                        * deactivated */
        pd->vttddf = nodev;            /* Device Dependent stuff    */
        pd->vttdefc = vttdefc;         /* Change the cursor shape  */
        pd->vttdma = nodev;            /* DMA Access routine    */
        pd->vttterm = vttterm;         /* Free any resources used by this VT */
        pd->vttinit = vttinit;         /* setup new logical terminal */
        pd->vttmovc = vttmovc;         /* Move the cursor to the position
                                        * indicated */
        pd->vttrds = vttrds;           /* Read a line segment */
        pd->vtttext = vtttext;         /* Write a string of chars  */
        pd->vttscr = vttscr;           /* Scroll text on the VT */
        pd->vttsetm = vttsetm;         /* Set mode to KSR or MOM   */
        pd->vttstct = vttstct;         /* Change color mappings    */
        pd->vttpwrphase = vttdpm;      /* Display Power Management */

        pd->make_gp = iga_make_gp;
        pd->unmake_gp = iga_unmake_gp;

        pd->sync_mask = bad_func;       /* Sync event support   */
        pd->async_mask = bad_func;      /* Async event support  */
        pd->enable_event = bad_func;    /* Non report event support   */

        pd->display_info.colors_total = 16777215;
        pd->display_info.colors_active = 16;
        pd->display_info.colors_fg = 16;
        pd->display_info.colors_bg = 16;
        for (i = 0; i < 16; i++)
        {
                pd->display_info.color_table[i] = dds->ksr_color_table[i];
        }
        pd->display_info.font_width = 16;
        pd->display_info.font_height = 30;
        pd->display_info.bits_per_pel = 1;
        pd->display_info.adapter_status = 0xC0000000;
        pd->display_info.apa_blink = 0x80000000;
        pd->display_info.screen_width_pel = 1024;
        pd->display_info.screen_height_pel = 768;

        pd->display_info.screen_width_mm = dds->screen_width_mm;
        pd->display_info.screen_height_mm = dds->screen_height_mm;

        /* IGA physical device id */
        devdat.full = dds->devid;
        pd->disp_devid[0] = devdat.byte.b1;
        pd->disp_devid[1] = devdat.byte.b2;
        pd->disp_devid[2] = devdat.byte.b3;
        pd->disp_devid[3] = devdat.byte.b4;

        pd->usage = 0;                 /* Set to indicate no VTs open */
        pd->open_cnt = 0;              /* Set to indicate driver is closed */
        pd->devno = devno;             /* Set device number */

        BUGLPR (dbg_define, BUGNTX, ("Allocating  Device Dependent Data Area\n"));
        /* allocate area for device dependent function problems */

        pd->free_area = (ulong *) xmalloc (sizeof (struct iga_ddf), 3, pinned_heap);

        if (pd->free_area == NULL)
        {
                iga_err (NULL, "PCIIGA", "DEFINE", "XMALLOC", pd->free_area, IGA_BAD_XMALLOC,
                        RAS_UNIQUE_2);
                return (ENOMEM);
        }

        BUGLPR (dbg_define, BUGNTX, ("Initializing Device Dependent Data\n"));
        /* Initialize device dependent data */
        ddf = (struct iga_ddf *) pd->free_area;
        bzero (ddf, sizeof (struct iga_ddf));
#ifdef S3_INTERRUPTS
        ddf->sleep_addr = EVENT_NULL;
#endif
        pd->num_domains = 1;
        pd->dwa_device = 0;
	ddf->dpms_enable = 1;

	/* 
	 *      This function has to be HERE.  Otherwise some function could do the trace 
         *      before the buffer is set up.  If this happens machine will crash! 
         */
	if (iga_memtrace_init(pd) != 0)    /* set up in-memory trace buffer */
	{
		/* clean up before return  */

		xmfree (pd->free_area, pinned_heap);
		xmfree (pd, pinned_heap);

                return (ENOMEM);
	}

        /* Set up the "pos" regs for this adapter */
        rc = enable_iga( ddf, dds );
        if (rc != 0)
        {
                BUGLPR (dbg_define, BUGNTX, ("Unable to set up adapter.\n"));

		/* clean up before return  */

		iga_memtrace_term(pd->free_area);
		xmfree (pd->free_area, pinned_heap);
		xmfree (pd, pinned_heap);

                return rc;
        }

        /* Now decide where to put the structure */

        devswqry (devno, &status, &nxtpd);
        if (nxtpd != NULL)
        {
                /*
                 * When we reach here at least one other iga has been defined
                 * in the system.  Homestake can't share I/O space, so we
                 * can't configure this one.
                 */

                BUGLPR (dbg_define, 0, ("Other IGA's ARE DEFINED!...\n"));

		/* clean up before return  */

		iga_memtrace_term(pd->free_area);
		xmfree (pd->free_area, pinned_heap);
		xmfree (pd, pinned_heap);

                return -1;
        }
        BUGLPR (dbg_define, BUGNTX, ("Did DevSwQry..........\n"));


        /* AIX Power Management:
         * ---------------------
         *      - allocate and init. structure in order to register with PM core
         *      - register device with PM core
         *
	 *   Note for machine which does not support AIX Power Management, our
         *   handler won't be called.  Therefore the net effect is nothing!
         */

        p_pmdata = (struct pm_handle *) xmalloc (sizeof (struct pm_handle), 3, pinned_heap);

        if (p_pmdata == NULL)
        {
                iga_err (NULL, "PCIIGA", "DEFINE", "XMALLOC", pd->free_area, IGA_BAD_XMALLOC,
                        RAS_UNIQUE_5);

		/* clean up before return  */

		iga_memtrace_term(pd->free_area);
		xmfree (pd->free_area, pinned_heap);
		xmfree (pd, pinned_heap);

                return (ENOMEM);
        }
        else
        {
                bzero (p_pmdata, sizeof (struct pm_handle));

                ddf->p_aixpm_data = p_pmdata;

                p_pmdata->activity = -1;            /* always set to -1 (see Yamato Design) */
                p_pmdata->mode = PM_DEVICE_FULL_ON;

                p_pmdata->device_standby_time = 0 ;
                p_pmdata->device_idle_time    = dds->dpms_timeouts[2] * 60; 
                p_pmdata->device_idle_time1   = dds->dpms_timeouts[0] * 60; 
                p_pmdata->device_idle_time2   = dds->dpms_timeouts[1] * 60; 

                p_pmdata->idle_counter = 0; 

                p_pmdata->handler = iga_pm_handler;
                p_pmdata->private = (caddr_t) pd;
                p_pmdata->devno = devno;
                p_pmdata->device_logical_name = dds->component;

                p_pmdata->attribute = PM_GRAPHICAL_OUTPUT;  /* see Yamato Design */
                p_pmdata->pm_version = 0x100;               /* support full AIX Power Management (i.e phase 2) */

                pm_register_handle(p_pmdata, PM_REGISTER);  /* register device PM handler with PM core */

	        /* initialize watchdog for AIX Power Management of KSR mode */

        	ddf->wd.restart = 3;    /* seconds */
        	ddf->wd.next    = NULL;
        	ddf->wd.prev    = NULL;
        	ddf->wd.func    = wakeup_device_pm;
        	ddf->wd.count   = 0;
	
        	ddf->pm_sleep_word = EVENT_NULL;

        }

        /*
         * if this pointer is null then the card  is being configged for
         * the first time. In this case we must do a devswadd to get the
         * driver into the devswitch.
         */

        iga_devsw.d_open     = iga_open;
        iga_devsw.d_close    = iga_close;
        iga_devsw.d_read     = nodev;
        iga_devsw.d_write    = nodev;
        iga_devsw.d_ioctl    = iga_ioctl;
        iga_devsw.d_strategy = nodev;
        iga_devsw.d_select   = nodev;
        iga_devsw.d_config   = iga_config;
        iga_devsw.d_print    = nodev;
        iga_devsw.d_dump     = nodev;
        iga_devsw.d_mpx      = nodev;
        iga_devsw.d_revoke   = nodev;
        iga_devsw.d_dsdptr   = (char *) pd;
        iga_devsw.d_ttys     = NULL;
        iga_devsw.d_selptr   = NULL;
        iga_devsw.d_opts     = 0;

        /*
         * configure the IGA - IGA should be the only graphics device
         * on the bus.  Therefore we config by doing a devswadd to
         * get into the devswitch table
         */

        rc = devswadd (devno, &iga_devsw);

        BUGLPR (dbg_define, BUGNTX, ("Did DevSwAdd.........(%d).\n",rc));

        if (rc != 0)
        {                              /* Log an error */
                iga_err (NULL, "PCIIGA", "DEFINE", "DEVSWADD", rc, IGA_DEVSWADD,
                        RAS_UNIQUE_3);

		/* clean up before return  */

		iga_memtrace_term(pd->free_area);
		xmfree (pd->free_area, pinned_heap);
		xmfree (pd, pinned_heap);

                return ( rc );
        }

	IGA_EXIT_TRC0(pd->free_area,iga_define,2,IGA_DEFINE);

        return (0);
}

/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: IGA_UNDEFINE                                        */
/*                                                                     */
/* DESCRIPTIVE NAME: IGA_UNDEFINE - Undefine device routine            */
/*                                                                     */
/* FUNCTION: Procedure performs all house keeping functions necessary  */
/*             to get an adapter into the system.                      */
/*                                                                     */
/* INPUTS:   Device number                                             */
/*                                                                     */
/* OUTPUTS:  None.                                                     */
/*                                                                     */
/* CALLED BY: iga_config                                               */
/*                                                                     */
/* CALLS:    None.                                                     */
/*                                                                     */
/***********************************************************************/

long
iga_undefine(
        dev_t   devno)
{
        struct phys_displays *  pd;
        struct phys_displays *  last_pd;
        struct phys_displays *  next_pd;
        long                    i;
        struct iga_ddf *        ddf;
        struct fbdds *          dds;
        int                     rc;
        int                     status;
        int                     minors;

        BUGLPR (dbg_undefine, BUGNFO, ("\n\nEnter iga_undefine \n"));
        rc = devswqry(devno, NULL, &pd);
        BUGLPR (dbg_undefine, BUGNTX, ("devswqry returns 0x%x, devno=0x%x\n",rc,devno));
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
                BUGLPR (dbg_undefine, BUGNTX, ("No pd found for devno!! - return error\n"));
                iga_err( NULL, "PCIIGA", "DEFINE", ENODEV, IGA_NODEV, RAS_UNIQUE_4 );
                return ( ENODEV );
                }
        if (pd->next)
                minors ++;
        if (pd->open_cnt != 0) {
                BUGLPR (dbg_undefine, BUGNTX, ("Someone still has device open, return EBUSY\n"));
                return EBUSY;
                }

	iga_memtrace_term(pd->free_area);

        /****************************************************/
        /* We now have the pointer to the display structure */
        /* free the ddf structure and the display structure */
        /* set the pointer in the previous structure to null*/
        /****************************************************/
        ddf = (struct iga_ddf *)pd->free_area;

        pm_register_handle(ddf->p_aixpm_data, PM_UNREGISTER);  /* unregister handler with PM core */
        xmfree(ddf->p_aixpm_data, pinned_heap);

        BUGLPR (dbg_undefine, BUGNTX, ("Free ddf at 0x%x\n",ddf));
        xmfree(ddf, pinned_heap);

        dds = (struct fbdds *)pd->odmdds;
        BUGLPR (dbg_undefine, BUGNTX, ("Free dds at 0x%x\n",dds));
        xmfree(dds, pinned_heap);

        /*
         * Dequeue this pd from the list and free it.
         */
        if (minors == 0) {
                /* This is the last pd. */
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
                        /*
                         * This is the tricky case.
                         * If the one we are deleting is the first
                         * in the list and there are more pds, we will
                         * copy the second pd into the first one.
                         * This is required because we can not get
                         * to the pointer that points to the first one.
                         */
                        next_pd = pd->next;
                        bcopy(  next_pd,        /* from ptr */
                                pd,             /* to ptr */
                                sizeof(struct phys_displays));
                        BUGLPR (dbg_undefine, BUGNTX, ("xmfree pd at 0x%x\n",next_pd));
                        xmfree(next_pd, pinned_heap);
                }
        }
        BUGLPR (dbg_undefine, BUGNTX, ("Returning from iga_undefine\n"));
        return 0;
}
