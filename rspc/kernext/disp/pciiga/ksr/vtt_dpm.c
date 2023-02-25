static char sccsid[] = "@(#)38	1.5  src/rspc/kernext/disp/pciiga/ksr/vtt_dpm.c, pciiga, rspc41J, 9515B_all 3/22/95 17:10:41";
 /*
 * COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: vttdpm() 
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
#include  <sys/pm.h>             /* for AIX Power Management */

BUGXDEF (dbg_vttdpm);
GS_MODULE(iga_vttdpm);

/*----------------------------------------------------------------------
* IDENTIFICATION:   
*
* DESCRIPTIVE NAME:  vttdpm() 
*
* FUNCTIONS:  support display power management by turning on and off 
*             various combination of the vertical and horizontal syns to
*             attached display. 
*            
* INPUTS: physical display structure and 
*         display power saving phase -- display full-on = 1 
*                                       display standby = 2 
*                                       display suspend = 3 
*                                       display off     = 4 
*
* OUTPUTS          
*
* CALLED BY: Xwindow 's screen saver which calls loaddx which calls us (dd) 
*                        OR
*            LFT Display Power Management timer 
*                        OR 
*            PM core (kernel) if the adapter is Pony DXP  
*
* REFERENCES:  Graphics Adaptor Specification and S3 864 Data Book
*              Feature 118979 -- DPMS 
*              Feature 145708 -- Yamato's AIX Power Management
*
*---------------------------------------------------------------------*/


long vttdpm (pd,phase)
struct phys_displays *pd;
int phase;                    /* 1 = on, 2 = standby, 3 = suspend, 4 = off */
{
        struct iga_data *ld;
        struct iga_ddf *ddf;
        uchar  data, save_old_index, save_unlk_ext_seg;
	ulong save_io_base, old_lvl;
	struct pm_handle * p_aixpm_data;

        BUGLPR (dbg_vttdpm, BUGNFO, ("Entering vttdpm()\n"));

	ld = pd->visible_vt->vttld;
	ddf = ld->ddf;

	IGA_ENTER_TRC(ddf,iga_vttdpm,2,IGA_VTTDPM,pd->current_dpm_phase,phase,0,0,0);

	if (pd->current_dpm_phase == phase) /* already in requested phase so do nothing */
	{
           BUGLPR (dbg_vttdpm, BUGNFO, ("Leaving vttdpm()\n"));
	   IGA_EXIT_TRC0(ddf,iga_vttdpm,2,IGA_VTTDPM);
	   return(0);
	}


	p_aixpm_data = (struct pm_handle *) ddf->p_aixpm_data;
	
	/*
         * This function is called by LFT timer, loaddx, (and PM core if we run on Polo)
         * To avoid race condition, it's good idea to serialize this code by 
         * disabling all interrupts
         */

	old_lvl = i_disable (INTMAX);

	ddf->IO_in_progress += 1;     /* set flag to indicate IO happening */

        save_io_base = ld->IO_base_address;                     /* save data to restore it later */

        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);    /* gain access to device */

#define  VIDEO_OFF	0x20
#define  UNLOCK_VAL	0x06

#define	 HSYNC_OFF	0x20       /* bits 4 and 5 = 10 */
#define	 VSYNC_OFF	0x80       /* bits 6 and 7 = 10 */

	save_old_index = PCI_GETC(SEQ_INDX);   /* save index in order to restore it later */	

	/* Just in case the display does not support Display Power Management
         * we turn on blanking.  This is because when the Vertical and/or Horizontal
         * sync pulses are turned off, users won't notice weird stuff on screen.
         */
	if (phase != 1)
	{
	   GET_SEQ_REG(CLK_MODE,data);
	   PUT_SEQ_REG(CLK_MODE,data| VIDEO_OFF);   /* screen off */
	}

	switch(phase)
	{
	   case 1:    /* Horizontal Sync and Vertical Sync are enabled */ 

		GET_SEQ_REG(UNLK_EXT_SEQ,save_unlk_ext_seg);	
	   	PUT_SEQ_REG(UNLK_EXT_SEQ, UNLOCK_VAL);         /* unlock ext sequencer reg.  */
	
		GET_SEQ_REG(EXT_SEQ,data);	
		data &= (~ (HSYNC_OFF | VSYNC_OFF)) ;
	   	PUT_SEQ_REG(EXT_SEQ,data);                     /* turn both V and H syncs on */

		PUT_SEQ_REG(UNLK_EXT_SEQ,save_unlk_ext_seg);   /* lock ext sequencer reg.    */	

	   	GET_SEQ_REG(CLK_MODE,data);
	   	PUT_SEQ_REG(CLK_MODE,data & ~VIDEO_OFF);       /* turn screen on */

		PCI_PUTC(SEQ_INDX,save_old_index);             /* restore previous index value */	

		p_aixpm_data->mode = PM_DEVICE_ENABLE;         /* for AIX Power Management - Pony DXP */

		break;

	   case 2:     /* Horizontal Sync is disabled and Vertical Sync is enabled */ 

                GET_SEQ_REG(UNLK_EXT_SEQ,save_unlk_ext_seg);
                PUT_SEQ_REG(UNLK_EXT_SEQ, UNLOCK_VAL);         /* unlock ext sequencer reg.  */
                GET_SEQ_REG(EXT_SEQ,data);

                data = (data | HSYNC_OFF) & (~VSYNC_OFF);
                PUT_SEQ_REG(EXT_SEQ,data);                    /* turn both V and H syncs on */

                PUT_SEQ_REG(UNLK_EXT_SEQ,save_unlk_ext_seg);   /* lock ext sequencer reg.    */

                PCI_PUTC(SEQ_INDX,save_old_index);             /* restore previous index value */

		p_aixpm_data->mode = PM_DEVICE_IDLE;           /* for AIX Power Management -- Pony DXP */

		break;

	   case 3:     /* Horizontal Sync is enabled and Vertical Sync is disabled */ 

                GET_SEQ_REG(UNLK_EXT_SEQ,save_unlk_ext_seg);
                PUT_SEQ_REG(UNLK_EXT_SEQ, UNLOCK_VAL);         /* unlock ext sequencer reg.  */

                GET_SEQ_REG(EXT_SEQ,data);
                data = (data | VSYNC_OFF) & (~HSYNC_OFF);
                PUT_SEQ_REG(EXT_SEQ,data);                    /* turn both V and H syncs on */

                PUT_SEQ_REG(UNLK_EXT_SEQ,save_unlk_ext_seg);   /* lock ext sequencer reg.    */

                PCI_PUTC(SEQ_INDX,save_old_index);             /* restore previous index value */

		p_aixpm_data->mode = PM_DEVICE_IDLE;           /* for AIX Power Management -- Pony DXP */

		break;

	   case 4:     /* Horizontal Sync and Vertical Sync are disabled */ 

                GET_SEQ_REG(UNLK_EXT_SEQ,save_unlk_ext_seg);
                PUT_SEQ_REG(UNLK_EXT_SEQ, UNLOCK_VAL);         /* unlock ext sequencer reg.  */

                GET_SEQ_REG(EXT_SEQ,data);
                data |= (VSYNC_OFF | HSYNC_OFF);
                PUT_SEQ_REG(EXT_SEQ,data);                   

                PUT_SEQ_REG(UNLK_EXT_SEQ,save_unlk_ext_seg);   /* lock ext sequencer reg.    */

                PCI_PUTC(SEQ_INDX,save_old_index);             /* restore previous index value */

		p_aixpm_data->mode = PM_DEVICE_IDLE;           /* for AIX Power Management -- Pony DXP */

		break;

	   default:
		;

	}

        PCI_BUS_DET(ld->IO_base_address);      /* release access to device  */

	ld->IO_base_address = save_io_base ;   /* restore previous data       */ 

	pd->current_dpm_phase = phase;         /* keep track of current power  */
                                               /* saving mode which the display*/ 
                                               /* is in                        */

	ddf->IO_in_progress -= 1;              /* reset flag */

	i_enable(old_lvl);                     /* restore previous intr level */


	IGA_EXIT_TRC0(ddf,iga_vttdpm,2,IGA_VTTDPM);

        BUGLPR (dbg_vttdpm, BUGNFO, ("Leaving vttdpm()\n"));

        return (0);
}
