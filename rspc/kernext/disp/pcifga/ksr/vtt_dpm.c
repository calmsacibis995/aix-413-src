static char sccsid[] = "@(#)25  1.1  src/rspc/kernext/disp/pcifga/ksr/vtt_dpm.c, pcifga, rspc411, 9432B411a 8/8/94 15:34:10";
/*
 *   COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttpwrphase
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


#include "fga_INCLUDES.h"
#include <sys/m_param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>


BUGXDEF (dbg_vttdpm);

/*----------------------------------------------------------------------
* IDENTIFICATION:   vttdpm 
*
* DESCRIPTIVE NAME:  support display power management 
*
* FUNCTIONS:         Enable blanking via setting bit 5 of the 
*                    srtctl (screen repaint timing control) register 
*                    to zero. 
*                    Turn on/off various combination of vertical sync
*                    and horizontal sync pulses to the display (actually
*                    make the pulses either steady high or low)
*
* INPUTS:            NONE
*
* OUTPUTS           
*
* CALLED BY       watchdog timer (started in vttinit) and
*                 LFT Display Power Management timer  
*
*
* REFERENCES:     Weitek Power 9000 Graphics Adaptor Specification
*
*                 Fairway PCI Weitek Video Graphics Card Functional Spec 
*                 by Phil Milling
*
---------------------------------------------------------------------*/

long vttdpm(pd, phase)
struct phys_displays *pd;
int phase;
{
	struct fga_data * ld;
	struct fga_ddf  * ddf;
	ulong wtk_srtctl_val;
	ushort i;
	ulong old_bus_mem_addr;

	ld = pd->visible_vt->vttld;
	ddf = ld->ddf;

	old_bus_mem_addr = ld->bus_mem_addr ;         /* save old segment */

	ld->bus_mem_addr = PCI_MEM_ATT(&ddf->pci_mem_map);


	/*  for adapter which don't support Display Power Managment 
         *  users could notice display malfunctions when we turn off
         *  V or H syncs.  To get around this, turn off video before
         *  we we turn off the sync pulse(s)
	 */
	if (phase != 1)
	{
		wtk_srtctl_val = *WTK_SRTCTL;                      /* read CRT CNTL reg */
		*WTK_SRTCTL = wtk_srtctl_val & WTK_DSB_VIDEO;      /* disable video */
	}

	switch (phase)
	{
		case 1:
			/* on mode: H sync and V sync pulses on */ 
			i = *FGA_ADCNTL;

			*FGA_ADCNTL = i & ~(FGA_HSYNC_off | FGA_VSYNC_off);

			wtk_srtctl_val = *WTK_SRTCTL;                  /* read CRT CNTL reg */
			*WTK_SRTCTL = wtk_srtctl_val | WTK_ENB_VIDEO;  /* enable video      */
			break;

		case 2:
			/* standby mode: H sync pulse off and V sync pulse on */ 
			i = *FGA_ADCNTL ;
			*FGA_ADCNTL = i | FGA_HSYNC_off;   /* set bit 6 to 1 */

			break;

		case 3:
			/* suspend mode: H sync pulse on and V sync pulse off */ 
			i = *FGA_ADCNTL ;
			*FGA_ADCNTL = (i & ~FGA_HSYNC_off) | FGA_VSYNC_off ;  /* set bit 7 to 1 */
			break;


		case 4:
			/* off mode: H sync and V sync pulses off */ 
			i = *FGA_ADCNTL ;   
			*FGA_ADCNTL = i | FGA_HSYNC_off | FGA_HSYNC_off ; /* set bits 6 and 7 to 1 */
			break;

		default:
			;
	}
		
	PCI_MEM_DET (ld->bus_mem_addr);

	ld->bus_mem_addr = old_bus_mem_addr ;   /* restore  */ 

	return(0);
}
