static char sccsid[] = "@(#)29  1.3  src/rspc/kernext/disp/pcihga/ksr/vtt_dpm.c, pcihga, rspc411, 9436D411a 9/6/94 18:19:18";
 /*
 * COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
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


#include "hga_INCLUDES.h"
#include "hga_funct.h"
#include <sys/m_param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include "IO_defs.h"

BUGXDEF (dbg_vttdpm);


/*----------------------------------------------------------------------
* IDENTIFICATION:   
*
* DESCRIPTIVE NAME:  vttdpm() 
*
* FUNCTIONS:  support display power management (on and off mode)
*             by turning on and off both the vertical and horizontal syns to
*             attached display. 
*            
* INPUTS: physical display structure and 
*         display power saving phase 
*
* OUTPUTS          
*
* CALLED BY: Xwindow 's screen saver which calls loaddx which calls us (dd) 
*                        OR
*            LFT Display Power Management timer 
*
*
* REFERENCES:  Graphics Adaptor Specification
*
*---------------------------------------------------------------------*/


long vttdpm (pd,phase)
struct phys_displays *pd;
int phase;                    /* 1 = on, 2 = standby, 3 = suspend, 4 = off */
{
        struct hga_data *ld;
        struct hga_ddf *ddf;
        uchar  data, emul_mode, save_old_index;
	ulong save_io_base;

        BUGLPR (dbg_vttpwrphase, BUGNFO, ("Entering vttpwrphase\n"));

	ld = pd->visible_vt->vttld;
	ddf = ld->ddf;

        save_io_base = ld->IO_base_address;


        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

#if 0
	data = PCI_GETC(MISC_OUTPUT_R);    /* Read misc output register */

        /* check bit 0 - I/O address select 
	 * 0 = monochrome emulation.  Address based at 3Bx 
	 * 1 = color emulation.  Address based at 3Dx 
	 */

#define IOA_SEL     0x01
#define COLOR       0x1 
#define MONOCHROME  0x2 

	if (data & IOA_SEL)    
	{                                
	   emul_mode= COLOR;                
	}
	else
	{
	   emul_mode = MONOCHROME;
	}

	if (emul_mode != COLOR)
	{
        	/* TROUBLE */ 
	}
#endif

#define  V_AND_H_ON	0x80

	save_old_index = PCI_GETC(CRTC_INDX);   /* save index in order to restore it later */	


	if ((phase == 1) && (phase != pd->current_dpm_phase))      /* case 1 */ 
	{
		GET_CRTC_REG(CRT_MD,data);                /* read CRT_MD (CR17)  and            */
		PUT_CRTC_REG(CRT_MD,data | V_AND_H_ON );  /* set bit 7 to turn on V and H syncs */

        	/* Enable video */
        	GET_SEQ_REG(CLK_MODE, data);   /* read CLK_MODE (SR1) register and     */
        	data &= ~0x20;
        	PUT_SEQ_REG(CLK_MODE, data);   /* clear bit 5 to turn the screen on    */

		PCI_PUTC(CRTC_INDX,save_old_index);   /* restore previous index value */	
	}
	else if ( (phase != 1) && (pd->current_dpm_phase < 2) )    /* case 2 */ 
	{

        	/* Disable video */

        	GET_SEQ_REG(CLK_MODE, data);   /* read CLK_MODE (SR1) register and     */
		data |= 0x20;
        	PUT_SEQ_REG(CLK_MODE, data);   /* set bit 5 to turn the screen off     */

		GET_CRTC_REG(CRT_MD,data);                    /* read CRT_MD (CR17) nd                 */
		PUT_CRTC_REG(CRT_MD,data & (~ V_AND_H_ON) );  /* clear bit 7 to turn off V and H syncs */

		PCI_PUTC(CRTC_INDX,save_old_index);   /* restore previous index value */	
	}


        PCI_BUS_DET(ld->IO_base_address);

	ld->IO_base_address = save_io_base ; 

	pd->current_dpm_phase = phase;

        return (0);
}
