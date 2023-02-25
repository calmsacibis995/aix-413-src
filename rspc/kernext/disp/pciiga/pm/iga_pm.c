static char sccsid[] = "@(#)14	1.4  src/rspc/kernext/disp/pciiga/pm/iga_pm.c, pciiga, rspc41J, 9515B_all 3/28/95 12:33:46";
 /*
 * COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: iga_pm_handler() 
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
#include <sys/pm.h>

BUGXDEF (dbg_pm);
GS_MODULE(iga_pm);

long no_op()
{
}

/*----------------------------------------------------------------------
* IDENTIFICATION:  iga_pm_handler() 
*
* DESCRIPTIVE NAME: 
*
* FUNCTIONS:  support AIX Power Management
*            
* INPUTS: 
*
* OUTPUTS 
*
* CALLED BY:   kernel - PM core 
*
* REFERENCES:  Yamato Design For Feature 145708 
*              Design For Feature 145721
*
*---------------------------------------------------------------------*/


long iga_pm_handler (private_data,mode)
caddr_t private_data;
int mode;                    
{
	struct phys_displays *pd;
	struct pm_handle * p_pmdata;
	struct iga_ddf * ddf;
	int old_lvl;
	int rc = PM_SUCCESS;

        BUGLPR (dbg_pm, BUGNFO, ("Entering iga_pm_handler\n"));

	pd = (struct phys_displays *) private_data;
	ddf = (struct iga_ddf *) pd->free_area;

	p_pmdata = (struct pm_handle *) ddf->p_aixpm_data;

	IGA_ENTER_TRC(ddf,iga_pm,2,IGA_PM,p_pmdata->mode,mode,0,0,0);

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

			vttdpm (pd,DPMS_OFF);    /* turn off crt */
			p_pmdata->mode = mode;

		break;

		case PM_DEVICE_SUSPEND:
		case PM_DEVICE_HIBERNATION:

			rc = shutdown_device(pd,mode);

			if (rc == PM_SUCCESS)
			   p_pmdata->mode = mode;
			else
			   rc = PM_ERROR;
		break;

		case PM_PAGE_FREEZE_NOTICE:   /* sent to us before PM_DEVICE_SUSPEND/HIBERNATION */

			;    /* nothing to do here */
		break;

		case PM_PAGE_UNFREEZE_NOTICE:

			;   /* nothing to do here */
		break;

		default:
		     	rc = PM_ERROR;
	}

        BUGLPR (dbg_pm, BUGNFO, ("exit iga_pm_handler\n"));

	IGA_EXIT_TRC1(ddf,iga_pm,2,IGA_PM,rc);

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
	struct iga_ddf * ddf;
	ddf = (struct iga_ddf *) w;

	e_wakeup(& ddf->pm_sleep_word);
}

/*-----------------------------------------------------------------
 |                                                                |
 |  PM core tells the driver to resume from device suspend or     |
 |  hibernation by calling the device PM handler, iga_pm_handler. |
 |  This function is responsible turning on the device if needed, |
 |  reinitializes the PCI configuration registers for the device, |
 |  reinitializes the device itself, and redraws the presentation |
 |  space.  Note the RCM's PM handle is responsible for graphics  | 
 |  mode.                                                         |
 |                                                                |
 -----------------------------------------------------------------*/ 

long reconfig_device(pd,mode)

   struct phys_displays * pd;
   int mode;

{
        int rc = PM_SUCCESS;
	struct iga_ddf * ddf;
	struct fbdds   *dds;
	struct vtmstruc *vp;
	struct iga_data * ld;

	ddf = (struct iga_ddf *) pd->free_area;
	dds = (struct fbdds *) pd->odmdds;
	ld = (struct iga_data *) pd->visible_vt->vttld;
	vp = (struct vtmstruc *) pd->visible_vt;

	IGA_ENTER_TRC1(ddf,iga_pm,2,IGA_RECONFIG,mode);

        if (mode == PM_DEVICE_SUSPEND)   /* turn on device */
        {
           pm_planar_control(pd->devno,PMDEV_GCC,PM_PLANAR_ON);
           pm_planar_control(pd->devno,PMDEV_DAC,PM_PLANAR_ON);
           pm_planar_control(pd->devno,PMDEV_VRAM,PM_PLANAR_ON);
        }

	/* init PCI configuration registers of the devices */
	rc = enable_iga(ddf,dds);

	if (rc == PM_SUCCESS)
	{
	   rc = iga_reset(vp,ld,ddf);       /* re-initialize the device */

	   if (rc == PM_SUCCESS)
	   {
	      pd->current_dpm_phase = 1;    /* tell vttdpm that display is full on */

	      vttact(vp);                   /* redraw KSR data          */
	   }
	   else
	   {
	      /* failed to re-initialize the device */
	      IGA_PARM_TRC0(ddf,iga_pm,2,IGA_RECONFIG,IGA_RESET_FAILED);
	   }
	}
	else
	{
	   /* failed to init pci registers */
	   IGA_PARM_TRC0(ddf,iga_pm,2,IGA_RECONFIG,IGA_ENABLE_FAILED);
	}

	IGA_EXIT_TRC1(ddf,iga_pm,2,IGA_RECONFIG,rc);

	return(rc);
}

/*-----------------------------------------------------------------
 |                                                                |
 | Iga driver is instructed to enter device suspend or hibernation|
 |                                                                |
 | The RCM's PM handle has quiesce graphics mode IO so proceed to | 
 | shut down device if everything is ok.                          |
 |                                                                |
 -----------------------------------------------------------------*/

long shutdown_device(pd,mode)

   struct phys_displays *pd;
   int mode;                   /* PM_DEVICE_SUSPEND or PM_DEVICE_HIBERNATION */

{
	int rc = PM_SUCCESS;
	int old_lvl;
	struct iga_ddf * ddf;
	struct iga_data * ld;

	ddf = (struct iga_ddf *) pd->free_area;
	ld = (struct iga_data *) pd->visible_vt->vttld;

	IGA_ENTER_TRC1(ddf,iga_pm,2,IGA_SHUTDOWN,mode);

	old_lvl = i_disable(INTMAX);

	/* By the time our PM handle is called with PM_DEVICE_SUSPEND or
           PM_DEVICE_HIBERNATION, all user processes have been stopped.
           Therefore, all IO should have been stopped.  We might not need
           to set this flag but just for safety reason, it won't hurt to
           do it.  Setting this flag, call the vtt functions to draw/update
           the presentation space only.  The frame buffer is not updated 
           until the virtual terminal is activated. 
         */

 	ld->Vttenv.vtt_active = VTT_INACTIVE;       /* deactivate vt */	

	i_enable(old_lvl);
	
	if (rc == PM_SUCCESS)   /* ok to proceed */
	{
	   /* Is there any IO in KSR mode ?  If yes, the driver needs to wait 
            * for IO to complete before it can turn off the device.  Note the
            * driver can only wait for (by puting the device PM thread to sleep)  
            * for 3 seconds.
            */ 

	   if (ddf->IO_in_progress)      
	   {
	      w_init(&ddf->wd);
	      w_start(&ddf->wd);

	      e_sleep(& ddf->pm_sleep_word, EVENT_SHORT);  /* wakeup_device_pm wakes us up */

	      IGA_PARM_TRC0(ddf,iga_pm,2,IGA_SHUTDOWN,IGA_WAKEUP_FROM_E_SLEEP);

	      w_clear(&ddf->wd);

	      if (ddf->IO_in_progress)   /* the driver failed to quiesce IO so return error */
	      {
	         IGA_PARM_TRC0(ddf,iga_pm,2,IGA_SHUTDOWN,IGA_IO_ACTIVE);
	         rc = PM_ERROR;
	      }
	   } 

           if (rc == PM_SUCCESS)  /* quiesced IO successfully */
           {
	      IGA_PARM_TRC0(ddf,iga_pm,2,IGA_SHUTDOWN,IGA_TURN_OFF_DEV);

              vttdpm(pd,4);       /* turn off crt    */

              if (mode == PM_DEVICE_SUSPEND)   /* turn off device */
              {
                 pm_planar_control(pd->devno,PMDEV_GCC,PM_PLANAR_OFF);
                 pm_planar_control(pd->devno,PMDEV_DAC,PM_PLANAR_OFF);
                 pm_planar_control(pd->devno,PMDEV_VRAM,PM_PLANAR_OFF);
              }

           }
	}

	IGA_EXIT_TRC1(ddf,iga_pm,2,IGA_SHUTDOWN,rc);

	return (rc);
}
