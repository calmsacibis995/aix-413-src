static char sccsid[] = "@(#)78  1.16  src/rspc/kernext/disp/pcigga/config/define.c, pcigga, rspc41J, 9520B_all 5/19/95 15:28:10";
/* define.c */
/*
based on "@(#)68  1.2.1.2  src/bos/kernext/disp/wga/config/define.c, bos, bos410 10/28/93 15:37:50";
 *
 *   COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: gga_define, gga_undefine
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

#include <sys/pm.h>              /* for AIX Power Management */

#include "gga_INCLUDES.h"
#include "gga_funct.h"
BUGXDEF (dbg_define);
BUGXDEF (dbg_undefine);

GS_MODULE (gga_define);
extern long gga_memtrace_init();
extern long gga_memtrace_term();
extern void wakeup_device_pm();

int enable_gga();
void rgb525_write2();
extern void Prog_ICD ();

/*-----------------------------------------------------------------------
 |
 | IDENTIFICATION: GGA_DEFINE
 |
 | DESCRIPTIVE NAME: GGA_DEFINE - Define device routine for Entry disp
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
 | CALLED BY: gga_config
 |
 | CALLS:    None.
 |
 |---------------------------------------------------------------------*/
int bad_func()
{
     return(0);
}

long
gga_define (devno, dds, ddslen)
        dev_t           devno;
        struct gbdds   *dds;
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

        struct gga_ddf *ddf;
        struct devsw    gga_devsw;
        int             rc,
                        status;
	ulong		bus_mem_addr, wtk_cntl_val;
	volatile ulong  *gga_vram_addr;

        extern          nodev ();
        extern long     gga_config ();
        BUGLPR (dbg_define, BUGNTX, ("In GGA_DEFINE....devno=%X\n", devno));

        /*
         * This routine will be evoked by the configuration menthod which
         * 1) malloc space for a physical display structure
         * 2) fill in the function pointer fields as well as the
         *    pointer to the dds structure.
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
                gga_err (NULL, "PCIGGA", "DEFINE", "XMALLOC", pd, GGA_BAD_XMALLOC, RAS_UNIQUE_1);
                return (ENOMEM);
        }
        bzero (pd, sizeof (struct phys_displays));
        BUGLPR (dbg_define, BUGNTA, ("in ggadefine\n"));

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
         * and to set a unique value to determine if this was a gga
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
        pd->vttsetm = vttsetm;         /* Set mode to KSR or Graphics */
        pd->vttstct = vttstct;         /* Change color mappings    */
        pd->vttpwrphase = vttdpm;      /* Display Power Management */
        pd->make_gp = gga_make_gp;
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
        pd->display_info.screen_width_pel = 640;
        pd->display_info.screen_height_pel = 480;

        pd->display_info.screen_width_mm = dds->screen_width_mm;
        pd->display_info.screen_height_mm = dds->screen_height_mm;

        /* GGA physical device id */
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

        pd->free_area = (ulong *) xmalloc (sizeof (struct gga_ddf), 3, pinned_heap);
        if (pd->free_area == NULL)
        {
                gga_err (NULL, "PCIGGA", "DEFINE", "XMALLOC", pd->free_area, GGA_BAD_XMALLOC,
                        RAS_UNIQUE_2);
                return (ENOMEM);
        }

        BUGLPR (dbg_define, BUGNTX, ("Initializing Device Dependent Data\n"));
        /* Initialize device dependent data */
        ddf = (struct gga_ddf *) pd->free_area;
        bzero (ddf, sizeof (struct gga_ddf));
#ifdef FW_INTERRUPTS
        ddf->sleep_addr = EVENT_NULL;
#endif
        pd->num_domains = 1;
        pd->dwa_device = 0;
	ddf->dpms_enable = 1;

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
                gga_err (NULL, "PCIGGA", "DEFINE", "XMALLOC", pd, GGA_BAD_XMALLOC, RAS_UNIQUE_5);

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

                p_pmdata->device_idle_time    =  dds->dpms_timeouts[2] * 60;
                p_pmdata->device_idle_time1   =  dds->dpms_timeouts[0] * 60;
                p_pmdata->device_idle_time2   =  dds->dpms_timeouts[1] * 60;
                p_pmdata->device_standby_time =  0;
                p_pmdata->idle_counter        =  0;

                p_pmdata->handler = gga_pm_handler;
                p_pmdata->private = (caddr_t) pd;
                p_pmdata->devno = devno;

                p_pmdata->attribute = PM_GRAPHICAL_OUTPUT;  /* see Yamato Design */

                p_pmdata->device_logical_name = dds->component; 

		/* support full AIX Power Management (i.e., phase 2) */
                p_pmdata->pm_version = 0x100;              

                rc = pm_register_handle(p_pmdata, PM_REGISTER);  /* register device PM handler with PM core */

                /* initialize watchdog for AIX Power Management of KSR mode */

                ddf->wd.restart = 3;    /* seconds */
                ddf->wd.next    = NULL;
                ddf->wd.prev    = NULL;
                ddf->wd.func    = wakeup_device_pm;
                ddf->wd.count   = 0;

                ddf->pm_sleep_word = EVENT_NULL;
        }

        /*
         *      This function has to be HERE.  Otherwise some function could do the trace
         *      before the buffer is set up.  If this happens machine will crash!
         */
        if (gga_memtrace_init(pd) != 0)    /* set up in-memory trace buffer */
        {
                /* clean up before return  */

                pm_register_handle(ddf->p_aixpm_data, PM_UNREGISTER);  
                xmfree(ddf->p_aixpm_data, pinned_heap);

                xmfree (pd->free_area, pinned_heap);
                xmfree (pd, pinned_heap);

                return (ENOMEM);
        }

        /* Set up the "pos" regs for this adapter */
        rc = enable_gga( ddf, dds );
        if (rc != 0)
        {
                BUGLPR (dbg_define, BUGNTX, ("Unable to set up adapter.\n"));

                pm_register_handle(ddf->p_aixpm_data, PM_UNREGISTER);
		gga_memtrace_term(pd->free_area);

                xmfree(ddf->p_aixpm_data, pinned_heap);
                xmfree (pd->free_area, pinned_heap);
                xmfree (pd, pinned_heap);

                return rc;
        }

	ddf->monitor_type = dds->display_mode;

	ddf->IO_in_progress += 1;       /* set flag to indicate IO happenning */

	/* Get the VRAM size */
        bus_mem_addr = PCI_MEM_ATT( &ddf->pci_mem_map );


	wtk_cntl_val = * (volatile unsigned long *)( bus_mem_addr + 0x00060184L );
	if ( ( wtk_cntl_val & 0x7 ) == GGA_4MB_VRAM ) 
		dds->vram_config = 1;
	else if ( ( wtk_cntl_val & 0x7 ) == GGA_2MB_VRAM )
	{
#if 0
		wtk_cntl_val = * (volatile unsigned long *)( bus_mem_addr + 0x00060198L );

		if ( ( wtk_cntl_val & 0xF00 ) == 0x0 ) 
		{
			/* Pony */
			wtk_cntl_val = * (volatile unsigned long *)( bus_mem_addr - 0x01000000L + 0x000C0000L );
			if ( ( wtk_cntl_val & 0xF ) == MT_3 ) 
			{
				/* TFT */
				ddf->monitor_type = dds->display_mode = MT_3;
			}
			dds->vram_config = 2;
		}
		else dds->vram_config = 0; 
#endif
		dds->vram_config = 0; 
	}
	else {
		/* Now initialize the RGB525 part */
       		BUGLPR (dbg_reset, BUGNFO, ("SET RGB525\n"));
       		i = 0;
       		while (rgb525_init_tab[i].addr != 0)
       		{
               		rgb525_write2( bus_mem_addr, rgb525_init_tab[i].addr, rgb525_init_tab[i].data );
               		i++;
      		}
		EIEIO;

  		wtk_cntl_val = *(volatile unsigned long *)(bus_mem_addr + 0x00800100L);
		gga_vram_addr = ( bus_mem_addr | 0x00060000L | 0x184L );
        	*gga_vram_addr = WTK_MEM_CONFIG_4;

		gga_vram_addr = ( bus_mem_addr | 0x00060000L | 0x188L );
        	*gga_vram_addr = WTK_RF_PERIOD_DATA;

		gga_vram_addr = ( bus_mem_addr | 0x00060000L | 0x190L );
        	*gga_vram_addr = WTK_RL_MAX_DATA;
		EIEIO;

        	gga_vram_addr = ( bus_mem_addr | 0x00800000L );
		for (i=0; i<0x100000; i++)
		{
        		*gga_vram_addr = gga_vram_addr;
        		gga_vram_addr++;
		}

        	gga_vram_addr = ( bus_mem_addr | 0x00800000L );
		dds->vram_config = 1; 
		for ( i=0; i<0x100000; i++ )
		{
        		wtk_cntl_val = *gga_vram_addr;
			if ( wtk_cntl_val != gga_vram_addr )
			{
				Prog_ICD( ddf, dds, 0x00775898 ); 
				dds->vram_config = 0; 
				break;
			 } 
        		gga_vram_addr++;
		}
	}
        PCI_MEM_DET ( bus_mem_addr );

	ddf->IO_in_progress -= 1;           /* reset flag */

        /* Now decide where to put the structure */

        devswqry (devno, &status, &nxtpd);
        BUGLPR (dbg_define, BUGNTX, ("Did DevSwQry..........\n"));
        if ((nxtpd != NULL))
        {
        /* When we reach here at leat one other Glacier has been defined in */
        /* the system so we need to calculate where to hook the display    */
        /* structure into the chain of displays                            */

            while (( nxtpd->next != NULL ))
            {
                nxtpd = nxtpd->next;
            }
            /* When we fall out of the loop nxtpd will point to the structure */
            /* that the new display should be attached to.                    */
            nxtpd->next = pd;
        }
        else
        {  /* if this pointer is null then the card  is being configged for
            * the first time. In this case we must do a devswadd to get the
            * driver into the devswitch.
            */

            gga_devsw.d_open     = gga_open;
            gga_devsw.d_close    = gga_close;
            gga_devsw.d_read     = nodev;
            gga_devsw.d_write    = nodev;
            gga_devsw.d_ioctl    = gga_ioctl;
            gga_devsw.d_strategy = nodev;
            gga_devsw.d_select   = nodev;
            gga_devsw.d_config   = gga_config;
            gga_devsw.d_print    = nodev;
            gga_devsw.d_dump     = nodev;
            gga_devsw.d_mpx      = gga_mpx;
            gga_devsw.d_revoke   = nodev;
            gga_devsw.d_dsdptr   = (char *) pd;
            gga_devsw.d_ttys     = NULL;
            gga_devsw.d_selptr   = NULL;
            gga_devsw.d_opts     = 0;

            /* call devswadd to get us into the switch table */
            rc = devswadd (devno, &gga_devsw);

            BUGLPR (dbg_define, BUGNTX, ("Did DevSwAdd.........(%d).\n",rc));

            if (rc != 0)
            {                              /* Log an error */
                    gga_err (NULL, "PCIGGA", "DEFINE", "DEVSWADD", rc, GGA_DEVSWADD,
                            RAS_UNIQUE_3);

		    gga_memtrace_term(pd->free_area);
               	    pm_register_handle(ddf->p_aixpm_data, PM_UNREGISTER);

                    xmfree(ddf->p_aixpm_data, pinned_heap);
                    xmfree (pd->free_area, pinned_heap);
                    xmfree (pd, pinned_heap);

                    return ( rc );
            }
        }

	GGA_EXIT_TRC0(pd->free_area,gga_define,2,GGA_DEFINE);

        return (0);
}

/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: GGA_UNDEFINE                                        */
/*                                                                     */
/* DESCRIPTIVE NAME: GGA_UNDEFINE - Undefine device routine            */
/*                                                                     */
/* FUNCTION: Procedure performs all house keeping functions necessary  */
/*             to get an adapter into the system.                      */
/*                                                                     */
/* INPUTS:   Device number                                             */
/*                                                                     */
/* OUTPUTS:  None.                                                     */
/*                                                                     */
/* CALLED BY: gga_config                                               */
/*                                                                     */
/* CALLS:    None.                                                     */
/*                                                                     */
/***********************************************************************/

long
gga_undefine(
        dev_t   devno)
{
        struct phys_displays *  pd;
        struct phys_displays *  last_pd;
        struct phys_displays *  next_pd;
        long                    i;
        struct gga_ddf *        ddf;
        struct gbdds *          dds;
        int                     rc;
        int                     status;
        int                     minors;

        BUGLPR (dbg_undefine, BUGNFO, ("\nEnter gga_undefine \n"));
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
                gga_err( NULL, "PCIGGA", "DEFINE", ENODEV, GGA_NODEV, RAS_UNIQUE_4 );
                return ( ENODEV );
                }
        if (pd->next)
                minors ++;
        if (pd->open_cnt != 0) {
                BUGLPR (dbg_undefine, BUGNTX, ("Someone still has device open, return EBUSY\n"));
                return EBUSY;
                }

        /****************************************************/
        /* We now have the pointer to the display structure */
        /* free the ddf structure and the display structure */
        /* set the pointer in the previous structure to null*/
        /****************************************************/
        ddf = (struct gga_ddf *)pd->free_area;

	gga_memtrace_term(ddf);

        pm_register_handle(ddf->p_aixpm_data, PM_UNREGISTER);      /* AIX Power Management */ 
        xmfree(ddf->p_aixpm_data, pinned_heap);              

        BUGLPR (dbg_undefine, BUGNTX, ("Free ddf at 0x%x\n",ddf));
        xmfree(ddf, pinned_heap);

        dds = (struct gbdds *)pd->odmdds;
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
        BUGLPR (dbg_undefine, BUGNTX, ("Returning from gga_undefine\n"));
        return 0;
}
void rgb525_write2(ulong bus_mem_addr, ushort index, uchar data) {

  ulong gga_cntl_val;
  uint  RepdData;

  RepdData  = index & 0x00ff;
  RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  gga_cntl_val = *(volatile unsigned long *)(bus_mem_addr + 0x00800200L);
  *(volatile unsigned long *)(bus_mem_addr + 0x00060210L) = RepdData;
  gga_cntl_val = *(volatile unsigned long *)(bus_mem_addr + 0x00060198L);

  RepdData  = ( index & 0xff00 ) >> 8;
  RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  gga_cntl_val = *(volatile unsigned long *)(bus_mem_addr + 0x00800200L);
  *(volatile unsigned long *)(bus_mem_addr + 0x00060214L) = RepdData;
  gga_cntl_val = *(volatile unsigned long *)(bus_mem_addr + 0x00060198L);

  RepdData  = data;
  RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  gga_cntl_val = *(volatile unsigned long *)(bus_mem_addr + 0x00800200L);
  *(volatile unsigned long *)(bus_mem_addr + 0x00060218L) = RepdData;
  gga_cntl_val = *(volatile unsigned long *)(bus_mem_addr + 0x00060198L);

  return;

}

gga_mpx(dev_t dev, chan_t *chan, caddr_t chan_str)
{
	return(video_mpx(dev, chan, chan_str));
}
