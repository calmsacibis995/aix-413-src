static char sccsid[] = "@(#)91	1.8  src/rspc/kernext/disp/pcigga/pm/gga_pm.c, pcigga, rspc41J, 9520A_all 5/16/95 13:25:05";
 /*
 * COMPONENT_NAME: (PCIGGA) Weitek 9100 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: gga_pm_handler() 
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

#include "gga_INCLUDES.h" 
#include <sys/pm.h>

BUGXDEF (dbg_pm);
GS_MODULE(gga_pm);

long no_op()
{
}


/*----------------------------------------------------------------------
* IDENTIFICATION:  gga_pm_handler() 
*
* DESCRIPTIVE NAME: 
*
* FUNCTIONS:  support AIX Power Management
*            
* INPUTS: 
*        
*
* OUTPUTS 
*
* CALLED BY:  Kernel 
*
* REFERENCES:  Yamato Design for Feature 145708 
*              Design For Feature 145721
*
*---------------------------------------------------------------------*/


long gga_pm_handler (private_data,mode)
caddr_t private_data;
int mode;                    
{
	struct phys_displays *pd;
	struct pm_handle * p_pmdata;
	struct gga_ddf * ddf;
        int old_lvl;
        int rc = PM_SUCCESS;


        BUGLPR (dbg_pm, BUGNFO, ("Entering gga_pm_handler\n"));

	pd  = (struct phys_displays *) private_data;
	ddf = (struct gga_ddf *) pd->free_area;

	p_pmdata = (struct pm_handle *) ddf->p_aixpm_data;

        GGA_ENTER_TRC(ddf,gga_pm,2,GGA_PM,p_pmdata->mode,mode,0,0,0);

	switch(mode)   /* new mode to go to */
	{
		case PM_DEVICE_FULL_ON:

			vttdpm (pd,DPMS_ON);            /* turn on crt */

			old_lvl = i_disable(INTMAX);

			pd->vttpwrphase = vttdpm; /* enable DPMS */ 
		        ddf->dpms_enable = 1;

			i_enable(old_lvl);

			p_pmdata->mode = mode;
		break;

		case PM_DEVICE_ENABLE: 

			old_lvl = i_disable(INTMAX);

			if (ddf->dpms_enable)
			{
			    pd->vttpwrphase = no_op; /* disable DPMS, now PM takes over  */ 
			    ddf->dpms_enable = 0;
			}

			i_enable(old_lvl);

			switch(p_pmdata->mode)    /* from this old mode, try to go to new mode */ 
			{
				case PM_DEVICE_IDLE:      
				/* case PM_DEVICE_DPMS_OFF:  same as IDLE */
				case PM_DEVICE_DPMS_STANDBY:
				case PM_DEVICE_DPMS_SUSPEND:

					vttdpm (pd,DPMS_ON);    /* turn on crt */
				break;

				case PM_DEVICE_SUSPEND: 
				case PM_DEVICE_HIBERNATION:
					rc = reconfig_device(pd,p_pmdata->mode);
    	    	    	    		video_pm_handler(pd, mode);

				break;
			}

			if (rc == PM_SUCCESS)
				p_pmdata->mode = mode;
			else
			        rc = PM_ERROR;

		break;

		case PM_DEVICE_DPMS_STANDBY: 

			vttdpm(pd,DPMS_STANDBY);
			p_pmdata->mode = mode;

		break;

		case PM_DEVICE_DPMS_SUSPEND: 

			vttdpm(pd,DPMS_SUSPEND);
			p_pmdata->mode = mode;

		break;

		case PM_DEVICE_IDLE:             
		/* case PM_DEVICE_DPMS_OFF:  same as IDLE */

    	    		video_pm_handler(pd, mode);
			vttdpm (pd,DPMS_OFF);    /* turn off crt */
			p_pmdata->mode = mode;

		break;

		case PM_DEVICE_SUSPEND:
		case PM_DEVICE_HIBERNATION:

    	    		video_pm_handler(pd, mode);
			rc = shutdown_device(pd,mode);

			if (rc == PM_SUCCESS)
			   p_pmdata->mode = mode;
			else
			   rc = PM_ERROR;
		break;

                case PM_PAGE_FREEZE_NOTICE:   /* sent before PM_DEVICE_SUSPEND/HIBERNATION */

                        ;    /* nothing to do here */
                break;

                case PM_PAGE_UNFREEZE_NOTICE: /* sent after we resume suspend/hibernation */ 

                        ;   /* nothing to do here */
                break;

		default:
			rc = PM_ERROR ;
	}
        BUGLPR (dbg_pm, BUGNFO, ("exit gga_pm_handler\n"));

	GGA_EXIT_TRC1(ddf,gga_pm,2,GGA_PM,rc);

        return (rc);
}


/*-----------------------------------------------------------------
 |                                                                |
 |  This timer is used to wake up the PM thread which has been put|
 |  to sleep by the device PM handler.  This happens because the  |
 |  handler has to wait for IO in KSR mode to complete before it  |
 |  can proceed to shut down the device for suspend or hibernation|
 |  mode.                                                         |
 |                                                                |
 -----------------------------------------------------------------*/ 

void wakeup_device_pm(w)
   struct watchdog * w;
{
	struct gga_ddf * ddf;
	ddf = (struct gga_ddf *) w;

	e_wakeup(& ddf->pm_sleep_word);
}

/*-----------------------------------------------------------------
 |                                                                |
 |  PM core tells the driver to resume from device suspend or     |
 |  hibernation by calling the device PM handler, gga_pm_handler. |
 |  This function is responsible turning on the device if needed, |
 |  reinitializes the PCI configuration registers for the device, |
 |  reinitializes the device itself, redraws the presentation     |
 |  space.  Note the RCM is responsible for graphics mode         | 
 |                                                                |
 -----------------------------------------------------------------*/ 

long reconfig_device(pd,mode)

   struct phys_displays * pd;
   int mode;

{
        int rc = PM_SUCCESS;
	struct gga_ddf * ddf;
	struct fbdds   *dds;
	struct vtmstruc *vp;
	struct gga_data * ld;

	ddf = (struct gga_ddf *) pd->free_area;
	dds = (struct fbdds *) pd->odmdds;
	ld = (struct gga_data *) pd->visible_vt->vttld;
	vp = (struct vtmstruc *) pd->visible_vt;

	GGA_ENTER_TRC1(ddf,gga_pm,2,GGA_RECONFIG,mode);

        if (mode == PM_DEVICE_SUSPEND)   /* turn on device */
        {
           pm_planar_control(pd->devno,PMDEV_GCC,PM_PLANAR_ON);
           pm_planar_control(pd->devno,PMDEV_DAC,PM_PLANAR_ON);
           pm_planar_control(pd->devno,PMDEV_VRAM,PM_PLANAR_ON);
        }

	/* init PCI configuration registers of the devices */
	rc = enable_gga(ddf,dds);

	if (rc == PM_SUCCESS)
	{
	   rc = gga_reset(vp,ld,ddf);       /* re-initialize the device */

	   if (rc == PM_SUCCESS)
	   {
	      pd->current_dpm_phase = 1;    /* tell vttdpm display is full on */

	      vttact(vp);                   /* redraw KSR data          */
	   }
	   else
	   {
	      /* failed to re-initialize the device */
	      GGA_PARM_TRC0(ddf,gga_pm,2,GGA_RECONFIG,GGA_RESET_FAILED);
	   }
	}
	else
	{
	   /* failed to init pci registers */
	   GGA_PARM_TRC0(ddf,gga_pm,2,GGA_RECONFIG,GGA_ENABLE_FAILED);
	}

	GGA_EXIT_TRC1(ddf,gga_pm,2,GGA_RECONFIG,rc);

	return(rc);
}

/*-----------------------------------------------------------------
 |                                                                |
 | gga driver is instructed to enter device suspend or hibernation|
 |                                                                |
 | The RCM has stopped all graphics processes from doing IO so    | 
 | proceed to shut down device if everything is ok.               |
 |                                                                |
 -----------------------------------------------------------------*/

long shutdown_device(pd,mode)

   struct phys_displays *pd;
   int mode;                   /* PM_DEVICE_SUSPEND or PM_DEVICE_HIBERNATION */

{
	int rc = PM_SUCCESS;
	struct gga_ddf * ddf;
	struct gga_data * ld;

	ddf = (struct gga_ddf *) pd->free_area;
	ld = (struct gga_data *) pd->visible_vt->vttld;

	GGA_ENTER_TRC1(ddf,gga_pm,2,GGA_SHUTDOWN,mode);

        /* 
           by the time we receive PM_DEVICE_SUSPEND/PM_DEVICE_HIBERNATION,
           all user processes have been stopped.  The chances of any IO happens
           in LFT mode is next to zero.  To be safe we still want to deactivate
           the virtual terminal. 
        */

	ld->Vttenv.vtt_active = VTT_INACTIVE;   

        /* Is there any IO in KSR mode ?  If yes, the driver needs to wait 
           for IO to complete before it can turn off the device.  Note the
           driver can only wait for (by puting the device PM thread to sleep)  
           for 3 seconds.
         */ 

	if (ddf->IO_in_progress)      
	{
	   w_init(&ddf->wd);
	   w_start(&ddf->wd);

	   e_sleep(& ddf->pm_sleep_word, EVENT_SHORT);  /* wakeup_device_pm wakes us up */

	   GGA_PARM_TRC0(ddf,gga_pm,2,GGA_SHUTDOWN,GGA_WAKEUP_FROM_E_SLEEP);

	   w_clear(&ddf->wd);

	   if (ddf->IO_in_progress)   /* the driver failed to quiesce IO so return error */
	   {
	      GGA_PARM_TRC0(ddf,gga_pm,2,GGA_SHUTDOWN,GGA_IO_ACTIVE);
	      rc = PM_ERROR;
	   }
	} 

        if (rc == PM_SUCCESS)  /* quiesced IO successfully */
        {
	   GGA_PARM_TRC0(ddf,gga_pm,2,GGA_SHUTDOWN,GGA_TURN_OFF_DEV);

           vttdpm(pd,4);       /* turn off crt    */

           if (mode == PM_DEVICE_SUSPEND)   /* turn off device */
           {
              pm_planar_control(pd->devno,PMDEV_GCC,PM_PLANAR_OFF);
              pm_planar_control(pd->devno,PMDEV_DAC,PM_PLANAR_OFF);
              pm_planar_control(pd->devno,PMDEV_VRAM,PM_PLANAR_OFF);
           }
        }

	GGA_EXIT_TRC1(ddf,gga_pm,2,GGA_SHUTDOWN,rc);

	return (rc);
}
