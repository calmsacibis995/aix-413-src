static char sccsid[] = "@(#)07  1.5  src/rspc/kernext/disp/pcigga/ksr/reset.c, pcigga, rspc41B, 9504A 1/9/95 13:31:58";
/* reset.c */
/*
based on "@(#)46  1.7  src/bos/kernext/disp/wga/ksr/reset.c, bos, bos410 10/28/93 17:45:13";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: gga_reset
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


/* #define SHOW_HW_CURSOR 1 *//* show the hardware cursor */
/* #define SHOW_COLOR_BACK 1 *//* show a striped outer border */

#include "gga_INCLUDES.h"
#include <sys/m_param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>

void Prog_ICD();
void rgb525_write();

BUGXDEF (dbg_reset);
BUGXDEF (dbg_load_vram);
GS_MODULE(gga_reset);

/*----------------------------------------------------------------------
* IDENTIFICATION:    RESET
*
* DESCRIPTIVE NAME:  Reset Graphic Adaptor
*
* FUNCTIONS:         This routine issues the pixel interface commands needed
*                    to reset the adaptor into a base conditions.  The
*                    following operations are performed:
*                       1.  Set up operating mode (# of Bits/Pel)
*                       2.  Establish VRAM Interface
*                       3.  Set CRT Controller Registers
*                       4.  Clear the Screen
*                       5.  Disable the Sprite
* INPUTS:            NONE
*
* OUTPUTS            Resets Adaptor
*
* CALLED BY          INIT
*
*
* REFERENCES:  Graphics Adaptor Specification
*
---------------------------------------------------------------------*/
long
gga_reset (vp, ld, ddf)
        struct vtmstruc *vp;
        struct gga_data *ld;
        struct gga_ddf *ddf;
{


        struct crt_control * p_crt_control;

        int             i;
        ulong           x;
        ulong           gga_cntl_val;  /* storage of current register values */
        volatile ulong  wtk_cntl_val;  /* storage of current register values */
        volatile ulong  *wtk_vram_addr, *wtk_vram_end;
        struct gbdds    *dds;
	ulong		RepdData;

	GGA_ENTER_TRC0(ddf,gga_reset,2,GGA_RESET);

        dds = (struct gbdds *) vp->display->odmdds;

        BUGLPR (dbg_reset, BUGNFO, ("Entering RESET\n"));

        /*
         * Set the local variable equal to global variable
         */
        p_crt_control = ld->P_CRT_CONTROL;

        ld->bus_mem_addr = PCI_MEM_ATT(&ddf->pci_mem_map);

        /************* Load the Glacier Control Regs - START **************/

        /* disable interrupts from the Weitek */
        BUGLPR (dbg_reset, BUGNFO, ("Weitek Intr enable value 0x%x\n", \
                         ld->wtk_intr_enbl_reg ));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000000);	
        *WTK_INTR_ENBL = ld->wtk_intr_enbl_reg;

#if 0
        /* Disable video */
	wtk_cntl_val = *WTK_SRTCTL;
	wtk_cntl_val &= WTK_DSB_VIDEO;
        BUGLPR (dbg_reset, BUGNFO, ("srtctl value 0x%x\n", \
                      wtk_cntl_val));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_SRTCTL = wtk_cntl_val;
#endif
	GGA_PARM_TRC0(ddf,gga_reset,2,GGA_RESET,GGA_RGB525_INIT);

        /* Now initialize the RGB525 part */
	BUGLPR (dbg_reset, BUGNFO, ("SET RGB525\n"));
	i = 0;
 	while (rgb525_init_tab[i].addr != 0) 
	{
		rgb525_write( ld, rgb525_init_tab[i].addr, rgb525_init_tab[i].data );
		i++;
	}

	BUGLPR (dbg_reset, BUGNFO, ("SET RGB525\n"));
	rgb525_write( ld, RGB525_F0, p_crt_control->f0_reg );
	rgb525_write( ld, RGB525_PIXEL_FMT, p_crt_control->pixelFmt_reg );
	rgb525_write( ld, RGB525_MISC_CLK, p_crt_control->miscClk_reg );

	GGA_PARM_TRC0(ddf,gga_reset,2,GGA_RESET,GGA_P9100_INIT);

	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );

        BUGLPR (dbg_reset, BUGNFO, ("srtctl value 0x%x\n", \
                      ld->wtk_srtctl_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);

        *WTK_SRTCTL = ld->wtk_srtctl_reg;

	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);


	/* Disable HSYNC & VSYNC while setting registers */	
	if ( ! p_crt_control->srtctl2_reg )
        	*WTK_SRTCTL2 = 0x0000000F;
        else *WTK_SRTCTL2 = 0x0000000A;

        BUGLPR (dbg_reset, BUGNFO, ("MEMCFG value 0x%x\n", \
                      ld->wtk_memcfg_reg));

	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_MEMCFG = ld->wtk_memcfg_reg;

	BUGLPR (dbg_reset, BUGNFO, ("PREVRTC    = 0x%x\n",ld->wtk_prevrtc_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_PREVRTC = ld->wtk_prevrtc_reg;

	BUGLPR (dbg_reset, BUGNFO, ("PREHRZC    = 0x%x\n",ld->wtk_prehrzc_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_PREHRZC = ld->wtk_prehrzc_reg;

        BUGLPR (dbg_reset, BUGNFO, ("Refresh rate value 0x%x\n",
                     ld->wtk_rfperiod_reg));
        *WTK_RFPERIOD = ld->wtk_rfperiod_reg;

        /* finish initializing the Weitek's memory configuration */
        BUGLPR (dbg_reset, BUGNFO, ("Memory RAS low max value 0x%x\n",
                     ld->wtk_rlmax_reg));
        *WTK_RLMAX = ld->wtk_rlmax_reg;

	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );

	BUGLPR (dbg_reset, BUGNFO, ("SET RASTER\n"));
        *WTK_RASTER       = WTK_PATRN_DISABLE;

	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );

	BUGLPR (dbg_reset, BUGNFO, ("PLANE_MASK = 0x%x\n",ld->wtk_plane_mask));
        *WTK_PLANE_MASK       = ld->wtk_plane_mask;

	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );

	BUGLPR (dbg_reset, BUGNFO, ("DRAW_MODE    = 0x%x\n",ld->wtk_draw_mode));
        *WTK_DRAW_MODE        = ld->wtk_draw_mode ;

	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );

	BUGLPR (dbg_reset, BUGNFO, ("SET WTK_WORIG\n"));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000200);
        *WTK_WORIG = ( ( ld->center_y << 16 ) |  ld->center_x  );

	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );

        BUGLPR (dbg_reset, BUGNFO, ("system config value 0x%x\n", \
                          p_crt_control->syscfg_reg));

	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000000);
        *WTK_SYS_CONFIG = p_crt_control->syscfg_reg;

        /* more initializing the Weitek's memory configuration */
        BUGLPR (dbg_reset, BUGNFO, ("Memory refresh period value 0x%x\n", \
                        ld->wtk_rfperiod_reg));

	BUGLPR (dbg_reset, BUGNFO, ("CRT Control Register values:\n"));
	BUGLPR (dbg_reset, BUGNFO, ("ADCNTL        = 0x%x\n",p_crt_control->adcntl_reg));
	BUGLPR (dbg_reset, BUGNFO, ("X_RES         = 0x%x\n",p_crt_control->x_resolution));
	BUGLPR (dbg_reset, BUGNFO, ("Y_RES         = 0x%x\n",p_crt_control->y_resolution));
	BUGLPR (dbg_reset, BUGNFO, ("COLOR/MONO    = 0x%x\n",p_crt_control->color_mono));
	BUGLPR (dbg_reset, BUGNFO, ("SYNC_ON_GREEN = 0x%x\n",p_crt_control->sync_on_green));

	BUGLPR (dbg_reset, BUGNFO, ("HRZT          = 0x%x\n",p_crt_control->hrzt_reg));

	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_HRZT    = p_crt_control->hrzt_reg;

	BUGLPR (dbg_reset, BUGNFO, ("HRZSR         = 0x%x\n",p_crt_control->hrzsr_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_HRZSR   = p_crt_control->hrzsr_reg;

	BUGLPR (dbg_reset, BUGNFO, ("HRZBR         = 0x%x\n",p_crt_control->hrzbr_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_HRZBR   = p_crt_control->hrzbr_reg;

	BUGLPR (dbg_reset, BUGNFO, ("HRZBF         = 0x%x\n",p_crt_control->hrzbf_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_HRZBF   = p_crt_control->hrzbf_reg;

	BUGLPR (dbg_reset, BUGNFO, ("VRTT          = 0x%x\n",p_crt_control->vrtt_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_VRTT    = p_crt_control->vrtt_reg;

	BUGLPR (dbg_reset, BUGNFO, ("VRTSR         = 0x%x\n",p_crt_control->vrtsr_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_VRTSR   = p_crt_control->vrtsr_reg;

	BUGLPR (dbg_reset, BUGNFO, ("VRTBR         = 0x%x\n",p_crt_control->vrtbr_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_VRTBR   = p_crt_control->vrtbr_reg;

	BUGLPR (dbg_reset, BUGNFO, ("VRTBF         = 0x%x\n",p_crt_control->vrtbf_reg));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_VRTBF   = p_crt_control->vrtbf_reg;

	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000200);
        *WTK_PIXEL_MASK   = 0xFFFFFFFF;

        /* RAMDAC hardware auto increments the indirect address */

	GGA_PARM_TRC0(ddf,gga_reset,2,GGA_RESET,GGA_RAMDAC_INIT);

	BUGLPR (dbg_reset, BUGNFO, ("SET PALCUR_W\n"));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000200);
	*WTK_PALCUR_W = 0x00000000;
  	gga_cntl_val = *WTK_PU_CONFIG;

        for (i = 0; i < ld->ksr_color_table.num_entries; i++)
        {
	    BUGLPR (dbg_reset, BUGNFO, ("Color Index %d = %x\n", i,
                                        ld->ksr_color_table.rgbval[i]));
	    RepdData = ((ld->ksr_color_table.rgbval[i]) & 0x00FF);
            *WTK_DAC_PALETTE = RepdData | (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  	    gga_cntl_val = *WTK_PU_CONFIG;
  	    gga_cntl_val = *WTK_PU_CONFIG;

	    RepdData = (((ld->ksr_color_table.rgbval[i]) >> 8) & 0x00FF);
            *WTK_DAC_PALETTE = RepdData | (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  	    gga_cntl_val = *WTK_PU_CONFIG;
  	    gga_cntl_val = *WTK_PU_CONFIG;

	    RepdData = (((ld->ksr_color_table.rgbval[i]) >> 16) & 0x00FF);
            *WTK_DAC_PALETTE = RepdData | (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  	    gga_cntl_val = *WTK_PU_CONFIG;
  	    gga_cntl_val = *WTK_PU_CONFIG;

        }

        /* Initialize the clipping area */

	GGA_PARM_TRC0(ddf,gga_reset,2,GGA_RESET,GGA_CLIPPING_INIT);

	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );
	BUGLPR (dbg_reset, BUGNFO, ("P_W_MIN    = 0x%x\n",ld->wtk_w_min_xy));
        *WTK_P_W_MIN             = ld->wtk_w_min_xy;

	BUGLPR (dbg_reset, BUGNFO, ("P_W_MAX    = 0x%x\n",ld->wtk_w_max_xy));
        *WTK_P_W_MAX  	       = ld->wtk_w_max_xy;

	BUGLPR (dbg_reset, BUGNFO, ("B_W_MIN    = 0x%x\n",ld->wtk_w_min_xy));
        *WTK_B_W_MIN  	       = ld->wtk_w_min_xy;

	BUGLPR (dbg_reset, BUGNFO, ("B_W_MAX    = 0x%x\n",ld->wtk_w_max_xy));
        *WTK_B_W_MAX  	       = ld->wtk_w_max_xy;

#if 0
	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );
	BUGLPR (dbg_reset, BUGNFO, ("BGROUND    = 0x%x\n",ld->ksr_color_table.rgbval[0]));
        *WTK_BGROUND  	       = 0x00000000;

	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );
	BUGLPR (dbg_reset, BUGNFO, ("FGROUND    = 0x%x\n",ld->ksr_color_table.rgbval[1]));
        *WTK_FGROUND  	       = 0x03030303;
#endif

        wtk_vram_addr          = WTK_VRAM_BASE;
        wtk_vram_end           = WTK_VRAM_BASE + ld->wtk_vram_end;

        /*
         * Set all of video memory to 0's.  This will force the default
         * color 0 in the palette ram to be displayed across the entire
         * screen everytime reset is invoked
         */

	GGA_PARM_TRC0(ddf,gga_reset,2,GGA_RESET,GGA_CLEAR_VRAM);

            BUGLPR (dbg_reset, BUGNFO, ("Clearing VRAM to 0.\n"));
            BUGLPR (dbg_reset, BUGNFO, ("WTK_VRAM_BASE = 0x%x\n", wtk_vram_addr));
        for ( ; wtk_vram_addr < wtk_vram_end  ; )
        {
                *wtk_vram_addr++ = 0x00000000;
        }
            BUGLPR (dbg_reset, BUGNFO, ("WTK_VRAM_END addr = 0x%x\n", wtk_vram_addr));

	while( (gga_cntl_val = *WTK_STATUS) & WTK_BUSY );

        BUGLPR (dbg_reset, BUGNFO, ("srtctl2 value 0x%x\n", \
                          p_crt_control->srtctl2_reg));

	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
        *WTK_SRTCTL2 = p_crt_control->srtctl2_reg;

        /* Enable video */

	BUGLPR (dbg_reset, BUGNFO, ("Enable video\n"));
	
	BUGLPR (dbg_reset, BUGNFO, ("screen timing control address 0x%x\n", WTK_SRTCTL));
        wtk_cntl_val = *WTK_SRTCTL;		/* read CRT CNTL reg */

	BUGLPR (dbg_reset, BUGNFO, ("screen timing control value 0x%x\n", wtk_cntl_val));
        wtk_cntl_val |= WTK_ENB_VIDEO;		/* Enable Video */
           
	BUGLPR (dbg_reset, BUGNFO, ("screen timing control new value 0x%x\n", wtk_cntl_val));
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);

        *WTK_SRTCTL = wtk_cntl_val;		/* write CNTL reg back out */

        wtk_cntl_val = *WTK_MEMCFG;
        wtk_cntl_val |= 0x00100000;
	gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000100);
	*WTK_MEMCFG = wtk_cntl_val;

        BUGLPR (dbg_reset, BUGNFO, ("Leaving RESET\n"));

        PCI_MEM_DET (ld->bus_mem_addr);

	GGA_EXIT_TRC0(ddf,gga_reset,2,GGA_RESET);

        return (0);
}

void rgb525_write(struct gga_data *ld, ushort index, uchar data) {

  ulong gga_cntl_val;
  uint  RepdData;

  RepdData  = index & 0x00ff;
  RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000200);
  *WTK_INDEXLOW  = RepdData;
  gga_cntl_val = *WTK_PU_CONFIG;
 
  RepdData  = ( index & 0xff00 ) >> 8;
  RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000200);
  *WTK_INDEXHIGH = RepdData;
  gga_cntl_val = *WTK_PU_CONFIG;

  RepdData  = data;
  RepdData |= (RepdData << 8) | (RepdData << 16) | (RepdData << 24);
  gga_cntl_val = *(volatile unsigned long *)(WTK_VRAM_BASE + 0x00000200);
  *WTK_INDEXDATA = RepdData;

  return;

}
