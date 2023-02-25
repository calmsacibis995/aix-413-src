static char sccsid[] = "@(#)14  1.7  src/rspc/kernext/disp/pcigga/ksr/vtt_dpm.c, pcigga, rspc41J, 9513A_all 3/22/95 11:16:33";
/*
 *
 * COMPONENT_NAME: (PCIGGA) Weitek 9100 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: vttdpm 
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
#include  <sys/pm.h>             /* for AIX Power Management */

BUGXDEF (dbg_vttdpm);
GS_MODULE(gga_vttdpm);

/*----------------------------------------------------------------------
* IDENTIFICATION:    vttdpm 
*
* DESCRIPTIVE NAME:  
*
* FUNCTIONS:     support Display Power Management by turning on and 
*                off various combinations of the vertical and horizontal 
*                sync pulses to the attached display.
*
* INPUTS:        physical display structure and 
*                display power saving phase -- display full-on = 1 
*                                              display standby = 2
*                                              display suspend = 3
*                                              display off     = 4
*
* OUTPUTS        
*
*
* CALLED BY: Xwindow 's screen saver which calls loaddx which calls us (dd)
*                        OR
*            LFT Display Power Management timer
*                        OR                                                   
*            PM core (kernel) if the adapter is Pony 9100 
*
*                                                                           
* REFERENCES:  Graphics Adaptor Specification and S3 864 Data Book
*              Feature 118979 -- DPMS                                      
*              Feature 145708 -- Yamato's AIX Power Management            
*
---------------------------------------------------------------------*/
long
vttdpm (pd,phase)
struct phys_displays * pd;
int phase;
{
	struct gga_ddf *ddf;
        struct gga_data *ld;
        ulong data, old_bus_mem_addr, old_lvl, trash; 

 	struct crt_control * p_crt_control;
	struct pm_handle * p_aixpm_data;


        BUGLPR (dbg_vttdpm, BUGNFO, ("Entering vttdpm\n"));

	ddf = (struct gga_ddf *) pd->free_area;

	GGA_ENTER_TRC(ddf,gga_vttdpm,2,GGA_VTTDPM,pd->current_dpm_phase,phase,0,0,0);

        if (pd->current_dpm_phase == phase)       /* already in requested phase so do nothing */
        { 
           BUGLPR (dbg_vttdpm, BUGNFO, ("Leaving vttdpm()\n"));  
	   GGA_EXIT_TRC0(ddf,gga_vttdpm,2,GGA_VTTDPM);
           return(0);                                           
        }                                                      

	ld = (struct gga_data *) pd->visible_vt->vttld;
	p_aixpm_data = (struct pm_handle *) ddf->p_aixpm_data;

        /*
         * This function is called by LFT timer, loaddx, (and PM core if we run 
	 * on Polo).  To avoid any possibility of race condition, it's good idea 
	 * to serialize this code by disabling all interrupts
         */

        old_lvl = i_disable (INTMAX);

	ddf->IO_in_progress += 1;     /* set flag to indicate IO happening */

        /*
         * Set the local variable equal to global variable
         */
        p_crt_control = ld->P_CRT_CONTROL;

	old_bus_mem_addr = ld->bus_mem_addr;   /* save to restore later */

        ld->bus_mem_addr = PCI_MEM_ATT(&ddf->pci_mem_map);     /* gain acess to device */

	/* Just in case the display does not support Display Power Management
         * we turn on blanking.  This is because when the Vertical and/or Horizontal
         * sync pulses are turned off, users could notice weird stuff on screen.
         *
	 * Note: Weitek document says the bit which turns off video might also
         *       turn off horizontal and vertical sync pulses too.  They need
         *       to confirm if this is true or not.  It is is true, then we could
         *       get rid off the swich statement because we can't turn on/off
         *       various combination of horizontal and vertical sync pulses.
         *       This means only on and off states are supported when this bit
         *       is used.  Well, I don't have to use this bit if we'll know for
         *       sure displays connected to the adapter will support DPMS. 
         *       So what to do here?
         */
	if (phase != 1)
	{
           BUGLPR (dbg_reset, BUGNFO, ("Disable video\n"));
  	   data = *WTK_SRTCTL;
	   data &= WTK_DSB_VIDEO;      /* Disable video */

	   /* due to a problem in Weitek chip, we have to read some VRAM
            * before doing a write to srctl and srctl2.  Added after code
            * review
            */
	   trash = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);

           *WTK_SRTCTL = data;
	}

#define V_SYNC_OFF 	0x2   /* bit 0 = 0 and bit 1 = 1 */
#define H_SYNC_OFF 	0x8   /* bit 2 = 0 and bit 3 = 1 */

	switch(phase)
	{
	   case 1:    /* Horizontal Sync and Vertical Sync are enabled */ 

		data = *WTK_SRTCTL2 ;

	   	/* due to a problem in Weitek chip, we have to read some VRAM
                 * before doing a write to srtctl and srtctl2.  Added after code
		 * code review.  I didn't have this line before but it didn't cause
	         * any problem.  Is this necessary?
                 */
	   	trash = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);

		*WTK_SRTCTL2 = data & ~(H_SYNC_OFF | V_SYNC_OFF);

           	BUGLPR (dbg_reset, BUGNFO, ("Enable video\n"));
           	data = *WTK_SRTCTL;
           	data |= WTK_ENB_VIDEO;          /* Enable Video */

	   	/* due to a problem in Weitek chip, we have to read some VRAM
                 * before doing a write to srctl and srctl2.  Added after code
		 * code review
                 */
	   	trash = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);

           	*WTK_SRTCTL = data;            

		p_aixpm_data->mode = PM_DEVICE_ENABLE;         /* for AIX Power Management - Pony 9100 */ 

		break;

	   case 2:     /* Horizontal Sync is disabled and Vertical Sync is enabled */ 

		   data = *WTK_SRTCTL2;

	   	   /* due to a problem in Weitek chip, we have to read some VRAM
                    * before doing a write to srtctl and srtctl2.  Added after code
		    * code review
                    */
	   	   trash = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);

		   *WTK_SRTCTL2 = (data | H_SYNC_OFF) & ~ V_SYNC_OFF;

		   p_aixpm_data->mode = PM_DEVICE_IDLE;         /* for AIX Power Management - Pony 9100 */ 

		break;

	   case 3:     /* Horizontal Sync is enabled and Vertical Sync is disabled */ 


		   data = *WTK_SRTCTL2 ;

	   	   /* due to a problem in Weitek chip, we have to read some VRAM
                    * before doing a write to srtctl and srtctl2.  Added after code
		    * code review
                    */
	   	   trash = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);

		   *WTK_SRTCTL2 = (data | V_SYNC_OFF) & ~ H_SYNC_OFF;

		   p_aixpm_data->mode = PM_DEVICE_IDLE;         /* for AIX Power Management - Pony 9100 */ 

		break;

	   case 4:     /* Horizontal Sync and Vertical Sync are disabled */ 


		   data = *WTK_SRTCTL2;

	   	   /* due to a problem in Weitek chip, we have to read some VRAM
                    * before doing a write to srtctl and srtctl2.  Added after code
		    * code review
                    */
	   	   trash = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);

		   *WTK_SRTCTL2 = data | H_SYNC_OFF | V_SYNC_OFF;

		   p_aixpm_data->mode = PM_DEVICE_IDLE;         /* for AIX Power Management - Pony 9100 */ 

		break;

	   default:
		;

	}

        BUGLPR (dbg_vttdpm, BUGNFO, ("Leaving vttdpm\n"));

        PCI_MEM_DET (ld->bus_mem_addr);

	ld->bus_mem_addr = old_bus_mem_addr ;    /* restore when done */

        pd->current_dpm_phase = phase;         /* keep track of current power  */
                                               /* saving mode which the display*/
                                               /* is in                        */

	ddf->IO_in_progress -= 1;              /* reset flag */

        i_enable(old_lvl);                     /* restore previous intr level */

	GGA_EXIT_TRC0(ddf,gga_vttdpm,2,GGA_VTTDPM);

        return (0);
}
