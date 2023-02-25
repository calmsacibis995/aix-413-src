static char sccsid[] = "@(#)16  1.13 src/rspc/kernext/disp/pciwfg/ksr/reset.c, pciwfg, rspc41J, 9520A_all 5/16/95 02:46:40";
/* reset.c */
/*
 *
 * COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: wfg_reset
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
#include <sys/m_param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include "IO_defs.h"

void Set_vclk();
int (**i_v7310_reset_overlay)();

#define TEST_PATTERN     0x00000000     /* VRAM initial value */

BUGXDEF (dbg_reset);
/*************************************************************************
 *                                                                       *
 * IDENTIFICATION:    wfg_reset()                                        *
 *                                                                       *
 * DESCRIPTIVE NAME:  Reset Graphic Adapter                              *
 *                                                                       *
 * FUNCTIONS:       This routine issues the pixel interface commands     *
 *                  needed to reset the adapter into a base conditions.  *
 *                  The following operations are performed:              *
 *                       1.  Set up operating mode (# of Bits/Pel)       *
 *                       2.  Establish VRAM Interface                    *
 *                       3.  Set CRT Controller Registers                *
 *                       4.  Clear the Screen                            *
 *                       5.  Disable the Sprite                          *
 * INPUTS:            NONE                                               *
 *                                                                       *
 * OUTPUTS            Resets Adapter                                     *
 *                                                                       *
 * CALLED BY          INIT                                               *
 *                                                                       *
 * REFERENCES:  Graphics Adapter Specification                           *
 *                                                                       *
 *************************************************************************/

long
wfg_reset (vp, ld, ddf)
        struct vtmstruc *vp;
        struct wfg_data *ld;
        struct wfg_ddf  *ddf;
{

        int                   i, rc;
        uchar                 t, u, v;
        volatile uchar        x;
        volatile ulong       *wfg_vram_addr, *wfg_vram_end;
        volatile uchar       *pData, *pIndex;
        struct fbdds         *dds;
	struct pm_handle     *pmhdl;
	struct phys_displays *pd;
#ifdef 	PANEL_PROTECTION
        int               saved_level;
#endif 	/* PANEL_PROTECTION */

        pd     = (struct phys_displays *) vp->display;
        dds    = (struct fbdds *)         vp->display->odmdds;
        pmhdl  = (struct pm_handle *)     ld->wfg_pmh;

        BUGLPR (dbg_reset, BUGNFO, ("Entering RESET\n"));

#ifdef	PANEL_PROTECTION
        /* ========================================= *
	 *    Disable CRT & LCD for H/W protection   *
	 * ========================================= */
        device_power ( pd, CRT, PM_PLANAR_OFF );
        device_power ( pd, LCD, PM_PLANAR_OFF );

        /* ------------------------------------------ *
         *          Interrrupt disable                *
         * ------------------------------------------ */
        saved_level = i_disable(INTMAX);

        /* =============================== *
	 *    Attach the Video I/O space   *
	 * =============================== */
        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

	while (0 != PCI_GETC(INPUT_STATUS1) & 0x08); 
	while (0 == PCI_GETC(INPUT_STATUS1) & 0x08); 
#endif /* PANEL_PROTECTION */

        /* ==================================================== *
	 *    Disable LCD panel and CRT for panel protection    *
	 * ==================================================== */
#ifndef PANEL_PROTECTION
        PUT_GRF_REG ( PR5, PR5_UNLOCK );
        PUT_GRF_REG ( PR4, PR4_CRT );
        PUT_GRF_REG ( PR5, PR5_LOCK );
#endif /* PANEL_PROTECTION */

        /*
         * LCDEN bit of Carrera has been disabled at this moment.
         * We don't want to stop WD's dotclock by disabling
         * Flat Panel Display Enable bit (bit4) of PR19.
         */
/**
        PUT_CRTC_REG( PR1B, PR1B_UNLOCK );
        PUT_CRTC_REG( PR18, 0x80 );
        PUT_CRTC_REG( PR19, PR19_DISABLE );
        PUT_CRTC_REG( PR1B, PR1B_LOCK );
**/

        /* ========================================== *
	 *    Load the Adapter Control Regs - START   *
	 * ========================================== */
        BUGLPR (dbg_reset, BUGNTA, ("Starting reset\n"));

        /* ------------------------------------------------------- *
	 *  Video Subsystem Enable Register   (Wake up the chip)   *
	 * ------------------------------------------------------- */
        PCI_PUTC(SETUP_VSE,0x16);   
        PCI_PUTC(POS_MODE_OPT_SEL,0x01);
        PCI_PUTC(SETUP_VSE,0x0e);

        /* -------------------------------------- *
	 *   Read misc output register,           *
	 *   and set CGA compatible I/O address   *
	 * -------------------------------------- */
        t = PCI_GETC(MISC_OUTPUT_R);    
        t |= 0x01;                     
	PCI_PUTC(MISC_OUTPUT_W,t);      

        /* ---------------------------------- *
	 *   Disable the Hardware Cursor      *
	 * ---------------------------------- */
        BUGLPR (dbg_reset, BUGNFO, ("Disable H/W Cursor\n"));
        PUT_EPR_REG(0x1002, 0x0000);

#ifdef	PANEL_PROTECTION
	/*
	 * Set_vclk() uses delay() to wait for sync signal stabilization
	 */
        i_enable(saved_level);
	Set_vclk(ddf,ld);
	saved_level = i_disable(INTMAX);
#else	/* PANEL_PROTECTION */
        /* ---------------------------------- *
	 *   Set the Video Clock              *
	 * ---------------------------------- */
	Set_vclk(ddf,ld);
#endif	/* PANEL_PROTECTION */

        /* ============================================================== *
         *  Set WD90C24A-specific Registers, i.e. Paradise Registers      * 
         * ============================================================== */
        BUGLPR (dbg_reset, BUGNTA, ("Doing reset\n"));

        /* ---------------------------------- *
	 *   Wait next vertical retrace       *
	 * ---------------------------------- */
	while (0 != PCI_GETC(INPUT_STATUS1) & 0x08); 
	while (0 == PCI_GETC(INPUT_STATUS1) & 0x08); 

        /*
         * LCDEN bit of Carrera has been disabled at this moment.
         * We don't want to stop WD's dotclock by disabling
         * Flat Panel Display Enable bit (bit4) of PR19.
         */
        /* ----------------------------------------------- *
	 *   Set the Paradise Registers  (crtc disable)    *
	 * ----------------------------------------------- */
/**
        BUGLPR (dbg_reset, BUGNTA, 
	       ("Initializing pr_crtc_disable regs - initial\n"));

        pData = ld->v_mode->PR_Mode_Table->pr_crtc_disable;
	for ( i=0;i<sizeof(ld->v_mode->PR_Mode_Table->pr_crtc_disable)/2; i++ )
           PUT_CRTC_REG ( *pData++, *pData++ );

        BUGLPR (dbg_reset, BUGNTA, 
	       ("Initializing pr_crtc_disable regs - end\n"));
**/

        /* ----------------------------------------------- *
	 *   Set the Paradise Registers  (sequencer)       *
	 * ----------------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, 
	       ("Initializing pr_seq regs - initial\n"));

        pData = ld->v_mode->PR_Mode_Table->pr_seq;
	for ( i=0;i< (ld->v_mode->PR_Mode_Table->pr_seq_length)/2; i++ )
            PUT_SEQ_REG ( *pData++, *pData++ );

        BUGLPR (dbg_reset, BUGNTA, 
	       ("Initializing pr_seq regs - end\n"));
 
        /* ---------------------------------------------------- *
	 *   Set the Paradise Registers  (graphics controller)  *
	 * ---------------------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, 
	       ("Initializing pr_gcr regs - initial\n"));

        pData = ld->v_mode->PR_Mode_Table->pr_gcr;
	for ( i=0;i<sizeof(ld->v_mode->PR_Mode_Table->pr_gcr)/2; i++ )
            PUT_GRF_REG ( *pData++, *pData++ );

        BUGLPR (dbg_reset, BUGNTA, 
	       ("Initializing pr_gcr regs - end\n"));
 
        /* ------------------------------------------------ *
	 *   Set the Paradise Registers  (crt controller)   *
	 * ------------------------------------------------ */
        BUGLPR (dbg_reset, BUGNTA, 
	       ("Initializing pr_crtc regs - initial\n"));

        pData = ld->v_mode->PR_Mode_Table->pr_crtc;
	for ( i=0;i< (ld->v_mode->PR_Mode_Table->pr_crtc_length)/2; i++ )
            PUT_CRTC_REG ( *pData++, *pData++ );

        BUGLPR (dbg_reset, BUGNTA, 
	       ("Initializing pr_crtc regs - end\n"));
 
        /* --------------------------------------------------- *
         *   Enable 256 Color Mode and 1Mbyte Linear Window    *
         * --------------------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA,
	       ("Enabling 256 color and 1M linear window - start\n"));

        GET_GRF_REG( PR5, u ); 
        PCI_PUTC(GRF_DATA, PR5_UNLOCK);
        PCI_PUTC(GRF_INDX, PR1);

        PCI_PUTC(GRF_DATA, (PCI_GETC(GRF_DATA) & 0x0f) | 0xf0); 
        PCI_PUTC(GRF_INDX, PR4); 
	PCI_PUTC(GRF_DATA, PCI_GETC(GRF_DATA) | 0x01);
        PCI_PUTC(GRF_INDX, PR5); 
	PCI_PUTC(GRF_DATA, u);
     
        GET_CRTC_REG( PR10, u ); 
        PCI_PUTC(CRTC_DATA, PR10_UNLOCK);
        PUT_CRTC_REG( PR16, 0 ); 
        PUT_CRTC_REG( PR10, u ); 
     
        GET_SEQ_REG( PR20, u ); 
        PCI_PUTC(SEQ_DATA, PR20_UNLOCK);
        GET_SEQ_REG( PR34A, v ); 
	v = (unsigned char)((ddf->base_address >> 20) & 0x0000000F);
        PUT_SEQ_REG( PR34A, v );
        GET_SEQ_REG( PR34A, v ); 
        PUT_SEQ_REG( PR20, u ); 

        BUGLPR (dbg_reset, BUGNTA,
	       ("Enabling 256 color and 1M linear window - end\n"));
 
        /* =============================================================== *
         *   Load VGA compatible registers except DAC palette registers    *
         * =============================================================== */

        /* ----------------------------------------------------- *
         *    Load VGA compatible registers       (sequencer)    *
	 *                            (Reset sequencer first)    *
         * ----------------------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, ("Initializing sequencer (VGA)\n"));
        PCI_PUTC(SEQ_INDX, 0x00);
        PCI_PUTC(SEQ_DATA, 0x01);
    
        pData = ld->v_mode->Base_Mode_Table->seq;
	for (i=0;i< sizeof(ld->v_mode->Base_Mode_Table->seq); i++ )
            PUT_SEQ_REG((unsigned char) i+1, *pData++);

        BUGLPR (dbg_reset, BUGNTA, ("Initializing for monitor %d\n", 
						     ld->monitor));

        /* --------------------------------------------------------- *
         * Load VGA compatible registers       (misc_out)            *
         *                                                           *
         * Program the Miscellaneous Output Register                 *
         *    Note: The miscellaneous output register                *
         *          must be programmed with the sequencer stopped.   *
         * --------------------------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, ("Initializing misc_out (VGA)\n"));
        pData = &ld->v_mode->Base_Mode_Table->misc_output[ld->monitor];
        PCI_PUTC(MISC_OUTPUT_W, *pData);
    
        /* ------------------------------------------------------- * 
         *  Load VGA compatible registers       (start sequencer)  * 
         * ------------------------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, ("Starting sequencer\n"));
        PUT_SEQ_REG(RST_SYNC, 0x03);

        /* -------------------------------------------------------- * 
         *   Load VGA compatible registers       (CRT Controller)   *
         *                       (Unlock CRTC 00h - 07h  bit 7=0)   *
         * -------------------------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, ("Programming CRT controller\n"));
        PUT_CRTC_REG( 0x11, 0x0e );

	if ( ld->monitor == ISO_MON )
            pData = ld->v_mode->crtc_iso;
	else
            pData = ld->v_mode->crtc_non_iso;

        for (i=0;i<=0x18 ;i++) {
            PUT_CRTC_REG( (unsigned char) i, *pData++);
        }

        /* ---------------------------------------- * 
	 *    Feature Control - Vertical Sync Ctl   * 
         * ---------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, ("Programming Extended CRT controller\n"));
        PCI_PUTC(FEATURE_CONTROL_W,0x04);

        /* ------------------------------------------------------- *
         *  Load VGA compatible registers       (attribute)        *
         *  Attribute Controller Regisers                          *
         *  Reading Input Status Register 1                        *
         *  Clears the Flip-Flop and Selects the address register  *
         * ------------------------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, ("Initializing attributes\n"));

        x = PCI_GETC(INPUT_STATUS1);    

        pData = ld->v_mode->Base_Mode_Table->attributes;
	for (i=0;i< sizeof(ld->v_mode->Base_Mode_Table->attributes); i++ )
            PUT_ATTR_REG( (unsigned char) i, *pData++);

        x = PCI_GETC(INPUT_STATUS1);     /* Re-enable video */
        PCI_PUTC(ATTR_INDX, ENB_PLT);

        /* ----------------------------------------------------------- *
         *  Load VGA compatible registers       (Graphics controller)  *
         * ----------------------------------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, ("Programming Graphics controller\n"));

	for (i=0;i< sizeof(ld->v_mode->Base_Mode_Table->gcr); i++ )
            PUT_GRF_REG( (unsigned char) i, *pData++);

        PUT_SEQ_REG(MEM_MODE, ld->v_mode->Memory_Mode);

        /* =============================== *
	 *    Detach the Video I/O space   *
	 * =============================== */
        PCI_BUS_DET(ld->IO_base_address);

        /* ----------------------------------- *
         *     Now initialize the RAMDAC       * 
         * ----------------------------------- */
        BUGLPR (dbg_reset, BUGNTA, ("Programming RAMDAC\n"));
	load_palette (ld);

        /* =============================== *
	 *    Attach the Video Memory      *
	 * =============================== */
        ld->bus_base_addr = PCI_BUS_ATT(&ddf->pci_mem_map);

        BUGLPR (dbg_reset, BUGNTA,
            ("\tadapter memory real address = 0x%8x\n", ddf->base_address ));
        BUGLPR (dbg_reset, BUGNTA,
            ("\taccessing adapter memory via  0x%8x\n", ld->bus_base_addr ));
        /*
         * Set all of video memory to 0's.  This will force the default
         * color 0 in the palette ram to be displayed across the entire
         * screen
         */

        wfg_vram_addr = (volatile ulong *)ld->bus_base_addr;
        wfg_vram_end  = (volatile ulong *)(ld->bus_base_addr+ddf->fb_size-1);

        BUGLPR (dbg_reset, BUGNTA, ("Clearing VRAM to 0x%8x.\n",TEST_PATTERN));
        BUGLPR (dbg_reset, BUGNTA, ("VRAM_BASE = 0x%8x\n", wfg_vram_addr));
        BUGLPR (dbg_reset, BUGNTA, ("VRAM_END  = 0x%8x\n", wfg_vram_end));
        BUGLPR (dbg_reset, BUGNTA, ("fb_size  = 0x%8x\n", ddf->fb_size));

        for ( ; wfg_vram_addr < wfg_vram_end ; )
        {
                *wfg_vram_addr++ = TEST_PATTERN;
        }
        BUGLPR (dbg_reset, BUGNTA, ("VRAM_END addr = 0x%x\n", wfg_vram_end));

        /* =============================== *
	 *    Detach the Video Memory      *
	 * =============================== */
        PCI_BUS_DET(ld->bus_base_addr);

        /* Enable video */
        BUGLPR (dbg_reset, BUGNTA, ("Enable video\n"));

        /* =============================== *
	 *    Attach the I/O space         *
	 * =============================== */
        ld->IO_base_address = PCI_BUS_ATT(&ddf->pci_io_map);

        /* ========================= *
	 *    Re-enable the Video    *
	 * ========================= */
        x = PCI_GETC(INPUT_STATUS1);
        PCI_PUTC(ATTR_INDX, ENB_PLT);

        /* =============================== *
	 *    Enable LCD panel and CRT     *
	 * =============================== */
        PUT_CRTC_REG( PR1B, PR1B_UNLOCK );
        PUT_CRTC_REG( PR18, PR18_S32 );
        PUT_CRTC_REG( PR19, PR19_S32 );
        PUT_CRTC_REG( PR1B, PR1B_LOCK );

        PUT_GRF_REG ( PR5, PR5_UNLOCK );
        PUT_GRF_REG ( PR4, PR4_ALL | 0x01 );
        PUT_GRF_REG ( PR5, PR5_LOCK );

        /* =============================== *
	 *    Detach the Video I/O space   *
	 * =============================== */
        PCI_BUS_DET(ld->IO_base_address);

        /* --------------------------------------------------------- *
         *   If this routine is invoked from un_make_gp(),           *
	 *   initialize V7310 Video Capture/Overlay controller and   *
	 *   BT812 digitizer                                         *
         * --------------------------------------------------------- */
        if ( *i_v7310_reset_overlay != NULL )
	        ddf->model_type = ld->model_type = ADVANCED_MODEL;
        else 
	        ddf->model_type = ld->model_type = ENTRY_MODEL;

        if (ld->gp_flag == WFG_GP_TERM &&
                ld->model_type == ADVANCED_MODEL ){
            if ( *i_v7310_reset_overlay != NULL ){
                  (**i_v7310_reset_overlay)();
            }
        }

	if (ld->gp_flag == WFG_GP_TERM ){
	    ld->gp_flag  = WFG_NGP;
	}

        /* ------------------------------------------ *
         *          Interrrupt enable                 *
         * ------------------------------------------ */
#ifdef	PANEL_PROTECTION
        i_enable(saved_level);
#endif	/* PANEL_PROTECTION */

        if ( ld->gp_flag != WFG_GP ) {
                rc |= device_power ( pd, CRT, PM_PLANAR_ON );
                rc |= device_power ( pd, LCD, PM_PLANAR_ON );
        }

        BUGLPR (dbg_reset, BUGNFO, ("Leaving RESET\n"));

        return (rc);
}

/* ------------------------------------------------------------------------- * 
 *      Set_vclk()                                                           * 
 *                                                                           * 
 *      Set Video clock and Memory clock frequncy for WD                     * 
 * ------------------------------------------------------------------------- */

#define MCLOCK_BASE		0x05 	/* Memory Clock  44.3    MHz */
#define VCLOCK_ISO		0x0c 	/* Video  Clock  31.5    MHz */
#define VCLOCK_NON_ISO		0x04 	/* Video  Clock  25.175  MHz */

void Set_vclk(ddf,ld)
        struct wfg_ddf *ddf;
        struct wfg_data *ld;
{
   unsigned char  pr10_val, pr11_val, msel, vsel;
   unsigned char  pr68_val, pr72_val, misc_val, freq_code=0;

   switch ( ddf->monitor_type ){
   case ISO_MON:
	msel = MCLOCK_BASE;
	vsel = VCLOCK_ISO;
	break;
   case NON_ISO_MON:
   case VESA_MON:
   case NO_CRT:
	msel = MCLOCK_BASE;
	vsel = VCLOCK_NON_ISO;
	break;
   }

   /* -------------------------- *
    *    Unlock Clock Select     *
    * -------------------------- */
   GET_CRTC_REG(PR10, pr10_val);
   PCI_PUTC(CRTC_DATA, PR10_UNLOCK);

   GET_CRTC_REG(PR11, pr11_val);       /* Unlock internal VCLK multiplexer */
   PCI_PUTC(CRTC_DATA, PR11_UNLOCK);

   /* --------------------------------------- *
    *    Set Miscellaneous Output Register    *
    *                  (mask off bits 3, 2)   *
    * --------------------------------------- */
   misc_val = PCI_GETC(MISC_OUTPUT_R);
   misc_val &= 0xf3;
   misc_val |= (vsel & 0x03) << 2;
   PCI_PUTC(MISC_OUTPUT_W, misc_val);

   /* -------------------------------- *
    *   Unlock PR68/PR69 Registers     *
    * -------------------------------- */
   PCI_PUTC(SEQ_INDX, PR20);
   PCI_PUTC(SEQ_DATA, PR20_UNLOCK);
   PCI_PUTC(SEQ_INDX, PR72);
   pr72_val = PCI_GETC(SEQ_DATA);
   PCI_PUTC(SEQ_DATA, PR72_UNLOCK);

   /* --------------------------- *
    *   Set PR68/PR69 Registers   *
    * --------------------------- */
   PCI_PUTC(SEQ_INDX, PR68);
   pr68_val = PCI_GETC(SEQ_DATA);
   if (ld->panel_scrn_width_pix == 640){
       pr68_val &= 0xe0;                  /* mask off bits 4-0 */
       pr68_val |= ((vsel & 0x0c) << 1) | msel;
       if (vsel == 0x02) {
          PCI_PUTC(SEQ_INDX, PR69);       /* Select Programmable Frequency */
	  PCI_PUTC(SEQ_DATA, freq_code);  /* VCLK = freq_code*14.318/32(MHz) */
       }
   }else{
       pr68_val &= 0xe0;                  /* mask off bits 4-0 */
       pr68_val |= (0x10 | (msel & 0x07));
       if (vsel == 0x02) {
          PCI_PUTC(SEQ_INDX, PR69);       /* Select Programmable Frequency */
	  PCI_PUTC(SEQ_DATA, freq_code);  /* VCLK = freq_code*14.318/32(MHz) */
       }
   }
   PCI_PUTC(SEQ_INDX, PR68);
   PCI_PUTC(SEQ_DATA, pr68_val);

#ifdef	PANEL_PROTECTION
   /* ------------------------------------------------------------ *
    *   Wait for 20msec (2tic) to stabilize the dot clock	   *
    * ------------------------------------------------------------ */
   delay(2);
#else	/* PANEL_PROTECTION */
   /* ------------------------------------------------------------ *
    *   Sleep for 500 micro seconds to make the dot clock stable   *
    *                          (unit time is micro second)         *
    * ------------------------------------------------------------ */
   io_delay(500);
#endif	/* PANEL_PROTECTION */

   /* ------------------------------------ *
    *    Restore Unlock Registers (PR72)   *
    * ------------------------------------ */
   PCI_PUTC(SEQ_INDX, PR72);
   PCI_PUTC(SEQ_DATA, pr72_val);
   PCI_PUTC(SEQ_INDX, PR20);
   PCI_PUTC(SEQ_DATA, PR20_LOCK);

   /* ------------------------------------ *
    *         Restore PR10/PR11            *
    * ------------------------------------ */
   PCI_PUTC(CRTC_INDX, PR11);
   PCI_PUTC(CRTC_DATA, pr11_val);
   PCI_PUTC(CRTC_INDX, PR10);
   PCI_PUTC(CRTC_DATA, pr10_val);

   return;
}
