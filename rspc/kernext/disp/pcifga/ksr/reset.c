static char sccsid[] = "@(#)20  1.3  src/rspc/kernext/disp/pcifga/ksr/reset.c, pcifga, rspc411, GOLD410 5/26/94 11:31:17";
/* reset.c */
/*
based on "@(#)46  1.7  src/bos/kernext/disp/wga/ksr/reset.c, bos, bos410 10/28/93 17:45:13";
 *
 * COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: fga_reset
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

#include "fga_INCLUDES.h"
#include <sys/m_param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>

void Prog_ICD();

BUGXDEF (dbg_reset);
BUGXDEF (dbg_load_vram);

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
fga_reset (vp, ld, ddf)
        struct vtmstruc *vp;
        struct fga_data *ld;
        struct fga_ddf *ddf;
{
        struct crt_control * p_crt_control;

        int             i;
        ulong           x;
        ulong           fga_cntl_val;  /* storage of current register values */
        ulong           wtk_cntl_val;  /* storage of current register values */
        volatile ulong  *wtk_vram_addr, *wtk_vram_end;
        struct fbdds    *dds;

        dds = (struct fbdds *) vp->display->odmdds;

        BUGLPR (dbg_reset, BUGNFO, ("Entering RESET\n"));

        /*
         * Set the local variable equal to global variable
         */
        p_crt_control = ld->P_CRT_CONTROL;

        ld->bus_mem_addr = PCI_MEM_ATT(&ddf->pci_mem_map);

        /************* Load the Fairway Control Regs - START **************/

        /* disable interrupts from the Weitek */
            BUGLPR (dbg_reset, BUGGID, ("Weitek Intr enable value 0x%x\n", \
                         ld->wtk_intr_enbl_reg ));
        *WTK_INTR_ENBL = ld->wtk_intr_enbl_reg;

        /* Enable PCLK 1 */
            BUGLPR (dbg_reset, BUGGID, ("Bt485 Reg 2 value 0x%x\n", \
                               ld->bt_command_reg_2 ));
        *BT485_COMMAND_REG_2 = ld->bt_command_reg_2;/* PCLK 1 enable */

        /* configure the chip */
            BUGLPR (dbg_reset, BUGGID, ("memory config value 0x%x\n", \
                      ld->wtk_memcfg_reg ));
        *WTK_MEMCFG = ld->wtk_memcfg_reg;

            BUGLPR (dbg_reset, BUGGID, ("srtctl value 0x%x\n", \
                      ld->wtk_srtctl_reg));
        *WTK_SRTCTL = ld->wtk_srtctl_reg;

            BUGLPR (dbg_reset, BUGGID, ("system config value 0x%x\n", \
                          ld->wtk_sys_config_reg));
        *WTK_SYS_CONFIG = ld->wtk_sys_config_reg;

        /* more initializing the Weitek's memory configuration */
            BUGLPR (dbg_reset, BUGGID, ("Memory refresh period value 0x%x\n", \
                        ld->wtk_rfperiod_reg));
        *WTK_RFPERIOD = ld->wtk_rfperiod_reg;

        /* finish initializing the Weitek's memory configuration */
            BUGLPR (dbg_reset, BUGGID, ("Memory RAS low max value 0x%x\n",
                     ld->wtk_rlmax_reg));
        *WTK_RLMAX = ld->wtk_rlmax_reg;

        /* Disable video */

            BUGLPR (dbg_reset, BUGNFO, ("Disable video\n"));
            BUGLPR (dbg_reset, BUGGID, ("srtctl value 0x%x\n", \
                      ld->wtk_srtctl_reg & WTK_DSB_VIDEO));
        *WTK_SRTCTL = ld->wtk_srtctl_reg & WTK_DSB_VIDEO;

BUGLPR (dbg_reset, BUGGID, ("CRT Control Register values:\n"));
BUGLPR (dbg_reset, BUGGID, ("ADCNTL        = 0x%x\n",p_crt_control->adcntl_reg));
BUGLPR (dbg_reset, BUGGID, ("HRZT          = 0x%x\n",p_crt_control->hrzt_reg));
BUGLPR (dbg_reset, BUGGID, ("HRZSR         = 0x%x\n",p_crt_control->hrzsr_reg));
BUGLPR (dbg_reset, BUGGID, ("HRZBR         = 0x%x\n",p_crt_control->hrzbr_reg));
BUGLPR (dbg_reset, BUGGID, ("HRZBF         = 0x%x\n",p_crt_control->hrzbf_reg));
BUGLPR (dbg_reset, BUGGID, ("VRTT          = 0x%x\n",p_crt_control->vrtt_reg));
BUGLPR (dbg_reset, BUGGID, ("VRTSR         = 0x%x\n",p_crt_control->vrtsr_reg));
BUGLPR (dbg_reset, BUGGID, ("VRTBR         = 0x%x\n",p_crt_control->vrtbr_reg));
BUGLPR (dbg_reset, BUGGID, ("VRTBF         = 0x%x\n",p_crt_control->vrtbf_reg));
BUGLPR (dbg_reset, BUGGID, ("X_RES         = 0x%x\n",p_crt_control->x_resolution));
BUGLPR (dbg_reset, BUGGID, ("Y_RES         = 0x%x\n",p_crt_control->y_resolution));
BUGLPR (dbg_reset, BUGGID, ("COLOR/MONO    = 0x%x\n",p_crt_control->color_mono));
BUGLPR (dbg_reset, BUGGID, ("SYNC_ON_GREEN = 0x%x\n",p_crt_control->sync_on_green));

        *WTK_HRZT    = p_crt_control->hrzt_reg;
        *WTK_HRZSR   = p_crt_control->hrzsr_reg;
        *WTK_HRZBR   = p_crt_control->hrzbr_reg;
        *WTK_HRZBF   = p_crt_control->hrzbf_reg;

        *WTK_VRTT    = p_crt_control->vrtt_reg;
        *WTK_VRTSR   = p_crt_control->vrtsr_reg;
        *WTK_VRTBR   = p_crt_control->vrtbr_reg;
        *WTK_VRTBF   = p_crt_control->vrtbf_reg;

        fga_cntl_val = *FGA_ADCNTL;        /* read FGA CNTL reg */
        fga_cntl_val &= FGA_SYNC_select_mask;/* clear sync bits */
        fga_cntl_val |= p_crt_control->adcntl_reg;/* get sync bits */
        *FGA_ADCNTL  = fga_cntl_val;
        Prog_ICD( ld, p_crt_control->dot_clock_reg );
        *FGA_ADCNTL  = fga_cntl_val;

        *WTK_PREVRTC = ld->wtk_prevrtc_reg;
        *WTK_PREHRZC = ld->wtk_prehrzc_reg;

        *WTK_WORIG = ( ( ld->center_y << 16 ) |  ld->center_x  );

        /* Now initialize the Brooktree part */

        *BT485_COMMAND_REG_0 = ld->bt_command_reg_0 ;
        *BT485_COMMAND_REG_0 |= p_crt_control->sync_on_green;
            BUGLPR (dbg_reset, BUGGID, ("Brooktree Reg0 data 0x%x\n", \
                               *BT485_COMMAND_REG_0 ));

            BUGLPR (dbg_reset, BUGGID, ("Brooktree Reg1 data 0x%x\n", \
                                ld->bt_command_reg_1 ));
        *BT485_COMMAND_REG_1  = ld->bt_command_reg_1 ;

        /* BT485_COMMAND_REG_2 was done earlier */

        *BT485_RAMDAC_WR_ADDR = 0x01;/* indirect for CR 3 */
            BUGLPR (dbg_reset, BUGGID, ("Brooktree Reg3 data 0x%x\n", \
                                ld->bt_command_reg_3 ));
        EIEIO;                          /* wait until pipeline drains */
        *BT485_COMMAND_REG_3  = ld->bt_command_reg_3 ;

            BUGLPR (dbg_reset, BUGGID, ("Brooktree pixel read mask data 0x%x\n", \
                                ld->bt_pixel_mask_reg));
        *BT485_PIXEL_MASK     = ld->bt_pixel_mask_reg;

	/* load the palette */
        *BT485_RAMDAC_WR_ADDR = 0x0;
        EIEIO;

        /* RAMDAC hardware auto increments the indirect address */

        for (i = 0; i < ld->ksr_color_table.num_entries; i++)
        {
        BUGLPR (dbg_reset, BUGGID, ("Color Index %d = %x\n", i,
                                        ld->ksr_color_table.rgbval[i]));

            *BT485_DAC_PALETTE = 
		(((ld->ksr_color_table.rgbval[i]) >> 16) & 0x00ffl);
            *BT485_DAC_PALETTE = 
		(((ld->ksr_color_table.rgbval[i]) >>  8) & 0x00ffl);
            *BT485_DAC_PALETTE = 
		(((ld->ksr_color_table.rgbval[i])      ) & 0x00ffl);
            io_delay(1);	/* hold off for a microsecond */
        }

        *WTK_PLANE_MASK       = ld->wtk_plane_mask;
        *WTK_DRAW_MODE        = ld->wtk_draw_mode ;

        /* Initialize the clipping area */
        *WTK_W_MIN_XY          = ld->wtk_w_min_xy;
        *WTK_W_MAX_XY          = ld->wtk_w_max_xy;

        wtk_vram_addr          = WTK_VRAM_BASE;
        wtk_vram_end           = ld->wtk_vram_end;

        /*
         * Set all of video memory to 0's.  This will force the default
         * color 0 in the palette ram to be displayed across the entire
         * screen everytime reset is invoked
         */

            BUGLPR (dbg_reset, BUGNFO, ("Clearing VRAM to 0.\n"));
            BUGLPR (dbg_reset, BUGGID, ("WTK_VRAM_BASE = 0x%x\n", wtk_vram_addr));

        for ( ; wtk_vram_addr < wtk_vram_end  ; )
        {
                *wtk_vram_addr++ = 0x00000000;
        }
            BUGLPR (dbg_reset, BUGGID, ("WTK_VRAM_END addr = 0x%x\n", wtk_vram_addr));

        /* Enable video */
            BUGLPR (dbg_reset, BUGGID, ("Enable video\n"));
            BUGLPR (dbg_reset, BUGGID, ("screen timing control address 0x%x\n", \
                            WTK_SRTCTL));
        wtk_cntl_val = *WTK_SRTCTL;        /* read CRT CNTL reg */
            BUGLPR (dbg_reset, BUGGID, ("screen timing control value 0x%x\n", \
                            wtk_cntl_val));
        wtk_cntl_val |= WTK_ENB_VIDEO;    /* Enable Video */
            BUGLPR (dbg_reset, BUGGID, ("screen timing control new value 0x%x\n", \
                      wtk_cntl_val));
        *WTK_SRTCTL = wtk_cntl_val;        /* write CNTL reg back out */

            BUGLPR (dbg_reset, BUGNFO, ("Leaving RESET\n"));
        PCI_MEM_DET (ld->bus_mem_addr);

        return (0);
}
