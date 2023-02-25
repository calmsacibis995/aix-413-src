static char sccsid[] = "@(#)94  1.2  src/rspc/kernext/disp/pcifga/config/init_fga.c, pcifga, rspc411, GOLD410 4/21/94 18:13:36";
/* init_fga.c */
/*
 *
 *   COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: fga_open
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
#include "fga_funct.h"

void Get_Monitor();                     /* determine monitor, etc. */
void load_palette ();                   /* load Bt485 */

BUGXDEF (dbg_initf);

/*
 * Define the Monitor id values for each supported monitor
 *      MT_x are defined in fga_regval.h
 */

/* crt_control struct is defined in fga.h MT_xx is defined in fga_regval.h */
static struct crt_control s_crt_control[] =
{                               /* first one is the default */
    /* MT_5 0x000f */           /* 1024x768, 60Hz, 64MHz    */
                                /* Galaxy X, vendor         */
    {
        MT_5,                   /* Monitor ID               */
        0x147,                  /* HRZT                     */
        0x017,                  /* HRZSR                    */
        0x042,                  /* HRZBR                    */
        0x142,                  /* HRZBF                    */
        0x32d,                  /* VRTT                     */
        0x003,                  /* VRTSR                    */
        0x02a,                  /* VRTBR                    */
        0x32a,                  /* VRTBF                    */
         1024,                  /* Horizontal Resolution    */
          768,                  /* Vertical Resolution      */
        COLOR,                  /* Color / Mono             */
        NO_SYNC_ON_GREEN,       /* Sync on green bits       */
        0x009bc31,              /* ICD2061A serial data     */
        FALSE,                  /* clock doubling           */
        0x00                    /* sync bits for control    */
    },

    /* MT_0 0x000a */           /* 1024x768, 76Hz, 86MHz    */
                                /* IBM Value Point,  Galaxy */
    {
        MT_0,                   /* Monitor ID               */
        0x15f,                  /* HRZT                     */
        0x04f,                  /* HRZSR                    */
        0x058,                  /* HRZBR                    */
        0x158,                  /* HRZBF                    */
        0x326,                  /* VRTT                     */
        0x008,                  /* VRTSR                    */
        0x025,                  /* VRTBR                    */
        0x325,                  /* VRTBF                    */
         1024,                  /* Horizontal Resolution    */
          768,                  /* Vertical Resolution      */
        COLOR,                  /* Color / Mono             */
        NO_SYNC_ON_GREEN,       /* Sync on green bits       */
        0x015f829,              /* ICD2061A serial data     */
        FALSE,                  /* clock doubling           */
        0x30                    /* sync bits for control    */
    },

    /* MT_1 0x010a */           /* 1024x 768, 70Hz, 78 MHz. */
                                /* IBM - 8517               */
    {
        MT_1,                   /* Monitor ID               */
        0x153,                  /* HRZT                     */
        0x045,                  /* HRZSR                    */
        0x049,                  /* HRZBR                    */
        0x149,                  /* HRZBF                    */
        0x32f,                  /* VRTT                     */
        0x008,                  /* VRTSR                    */
        0x02e,                  /* VRTBR                    */
        0x32e,                  /* VRTBF                    */
         1024,                  /* Horizontal Resolution    */
          768,                  /* Vertical Resolution      */
        COLOR,                  /* Color / Mono             */
        NO_SYNC_ON_GREEN,       /* Sync on green bits       */
        0x011301b,              /* ICD2061A serial data     */
        FALSE,                  /* clock doubling           */
        0x30                    /* sync bits for control    */
    },

    /* MT_2 0x0004 */           /* 1280x1024, 60Hz, 112MHz  */
                                /* IBM 6091, 6317           */
    {
        MT_2,                   /* Monitor ID               */
        0x1b7,                  /* HRZT                     */
        0x031,                  /* HRZSR                    */
        0x070,                  /* HRZBR                    */
        0x1b0,                  /* HRZBF                    */
        0x420,                  /* VRTT                     */
        0x003,                  /* VRTSR                    */
        0x01d,                  /* VRTBR                    */
        0x41d,                  /* VRTBF                    */
         1280,                  /* Horizontal Resolution    */
         1024,                  /* Vertical Resolution      */
        COLOR,                  /* Color / Mono             */
        SYNC_ON_GREEN,          /* Sync on green bits       */
        0x0195c15,              /* ICD2061A serial data     */
        FALSE,                  /* clock doubling           */
        0x00                    /* sync bits for control    */
    },

    /* MT_3 0x0007 */           /* 1280x1024, 67Hz, 128MHz  */
                                /* IBM 8508 - Mono          */
    {
        MT_3,                   /* Monitor ID               */
        0x1c3,                  /* HRZT                     */
        0x03f,                  /* HRZSR                    */
        0x06b,                  /* HRZBR                    */
        0x1ab,                  /* HRZBF                    */
        0x420,                  /* VRTT                     */
        0x008,                  /* VRTSR                    */
        0x01f,                  /* VRTBR                    */
        0x41f,                  /* VRTBF                    */
         1280,                  /* Horizontal Resolution    */
         1024,                  /* Vertical Resolution      */
         MONO,                  /* Color / Mono             */
        NO_SYNC_ON_GREEN ,      /* Sync on green bits       */
        0x009bc31,              /* ICD2061A serial data     */
        FALSE,                  /* clock doubling           */
        0x00                    /* sync bits for control    */
    },

    /* MT_4 0x020f */           /* 1280x1024, 72Hz, 128MHz  */
                                /* 1091-51                  */
    {
        MT_4,                   /* Monitor ID               */
        0x1a5,                  /* HRZT                     */
        0x022,                  /* HRZSR                    */
        0x05e,                  /* HRZBR                    */
        0x19e,                  /* HRZBF                    */
        0x424,                  /* VRTT                     */
        0x003,                  /* VRTSR                    */
        0x01f,                  /* VRTBR                    */
        0x41f,                  /* VRTBF                    */
         1280,                  /* Horizontal Resolution    */
         1024,                  /* Vertical Resolution      */
        COLOR,                  /* Color / Mono             */
        SYNC_ON_GREEN   ,       /* Sync on green bits       */
        0x009bc31,              /* ICD2061A serial data     */
        FALSE,                  /* clock doubling           */
        0x00                    /* sync bits for control    */
    },

/*
 * This is the end of the structure, so if more monitor types are to be
 * added, do so ABOVE THIS LINE, as the scanning algorithm looks for the
 * 0 in the first field to stop.
 */

    /* MT_END 0 */
    {
        MT_END,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    }
};

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                     FGA INITIALIZATION ROUTINES                            */
/*                                                                            */
/*      Needs:  pointer to local data structure                               */
/*                                                                            */
/*      Returns: -1 if can't find FGA, otherwise 0                            */
/*                                                                            */
/*      Called by: vtt_init                                                   */
/*                                                                            */
/*      Calls:  Get_Monitor                                                   */
/*              load_palette                                                  */
/*              fga_reset                                                     */
/*                                                                            */
/*----------------------------------------------------------------------------*/

int init_fga(vp, ld, ddf, dds)          /* Mainline routine */
struct vtmstruc *vp;
struct fga_data *ld;
struct fga_ddf *ddf;
struct fbdds   *dds;                    /* contains config method
                                         * data-prefilled */
{
    struct crt_control * p_crt_control;
    int  i, rc;
    uchar adapter_stat_reg;
    struct phys_displays *pd;

    BUGLPR (dbg_initf, BUGNTA, ("Starting initialization\n"));

    ld->P_CRT_CONTROL = &s_crt_control[0];

    BUGLPR (dbg_initf, BUGGID, ("ld->P_CRT_CONTROL = 0x%x\n",ld->P_CRT_CONTROL));

    /*
     * Set the local variable equal to global variable
     */

    p_crt_control = ld->P_CRT_CONTROL;

        BUGLPR (dbg_initf, BUGNTA, ("Attaching PCI adapter space [0x%8.8x]\n",
                                     ddf->base_address));

    /* set addressability */
    ld->bus_mem_addr = PCI_MEM_ATT(&ddf->pci_mem_map);

        BUGLPR (dbg_initf, BUGGID, ("ld->bus_mem_addr   = 0x%8.8x\n",ld->bus_mem_addr  ));
        BUGLPR (dbg_initf, BUGGID, ("WTK_BASE_ADDR   = 0x%8.8x\n",WTK_BASE_ADDR  ));
        BUGLPR (dbg_initf, BUGGID, ("WTK_PARM_BASE   = 0x%8.8x\n",WTK_PARM_BASE  ));
        BUGLPR (dbg_initf, BUGGID, ("WTK_VRAM_BASE   = 0x%8.8x\n",WTK_VRAM_BASE  ));
        BUGLPR (dbg_initf, BUGGID, ("BT485_BASE_ADDR = 0x%8.8x\n",BT485_BASE_ADDR));

    ld->wtk_intr_enbl_reg      = WTK_INTR_ENABLE_DATA;  /* disable interrupts */

    adapter_stat_reg           = *FGA_ADSTAT;

        BUGLPR (dbg_initf, BUGNTA, ("Adapter sense reg: 0x%8.8x\n",adapter_stat_reg));

    if (adapter_stat_reg & FGA_1MB_VRAM )
    {
        ld->wtk_memcfg_reg     = WTK_MEM_CONFIG_2;/*Set Memory config 2 (1MB)*/
        ld->wtk_srtctl_reg     = WTK_SRTCTL_2;
        ld->wtk_sys_config_reg = WTK_SYS_CONFIG_2;
        ld->mem_shift          = WTK_MEM_SHIFT_2;
        ld->wtk_vram_end       = WTK_VRAM_BASE + WTK_VRAM_SIZE_2 -1;
            BUGLPR (dbg_initf, BUGGID, ("Adapter has 1 Meg VRAM\n"));
    }
    else
    {
        ld->wtk_memcfg_reg     = WTK_MEM_CONFIG_3;/*Set Memory config 3 (2MB)*/
        ld->wtk_srtctl_reg     = WTK_SRTCTL_3;
        ld->wtk_sys_config_reg = WTK_SYS_CONFIG_3;
        ld->mem_shift          = WTK_MEM_SHIFT_3;
        ld->wtk_vram_end       = WTK_VRAM_BASE + WTK_VRAM_SIZE_3 -1;
            BUGLPR (dbg_initf, BUGGID, ("Adapter has 2 Meg VRAM\n"));
    }
    ld->wtk_rfperiod_reg       = WTK_RF_PERIOD_DATA;
    ld->wtk_rlmax_reg          = WTK_RL_MAX_DATA;
    ld->wtk_prevrtc_reg        = WTK_PREVRTC_DATA;
    ld->wtk_prehrzc_reg        = WTK_PREHRZC_DATA;
    ld->wtk_plane_mask         = WTK_PLANE_MASK_DATA;
    ld->wtk_draw_mode          = WTK_DRAW_MODE_DATA;
    ld->wtk_coor_offs          = WTK_COOR_OFFS_DATA;

    ld->bt_command_reg_0       = BT485_REG_0_DATA;
    ld->bt_command_reg_1       = BT485_REG_1_DATA;
    ld->bt_command_reg_2       = BT485_REG_2_DATA;/* Enable PCLK 1 + */
    ld->bt_command_reg_3       = BT485_REG_3_DATA;
    ld->bt_pixel_mask_reg      = BT485_PXL_READ_MSK_DATA;

        BUGLPR (dbg_initf, BUGGID, ("-PRE Get_Monitor\n"));

    /*-----------------------------------------------------*/
    /*        Set up for attached monitor                  */
    /*-----------------------------------------------------*/
    Get_Monitor (ld, ddf, dds );  /* determine monitor, etc. */

        BUGLPR (dbg_initf, BUGGID, ("POST Get_Monitor\n"));

    if( ld->P_CRT_CONTROL->sync_on_green != NO_SYNC_ON_GREEN)
        ld->bt_command_reg_0 |= ld->P_CRT_CONTROL->sync_on_green;

    /* see what dds says about max resolution */

    ld->wtk_w_min_xy           = 0;       /* same as Pack_XY( 0, 0 ); */
    ld->wtk_w_max_xy           = Pack_XY(ld->screen_width_pix-1, \
                                         ld->screen_height_pix-1);
        BUGLPR (dbg_initf, BUGNTA, ("ld max_xy = 0x%8.8x\n", ld->wtk_w_max_xy));

        BUGLPR (dbg_initf, BUGNTA, ("Detaching PCI adapter space\n"));

    PCI_MEM_DET (ld->bus_mem_addr); /* detach the bus, since they'll get it */

    return 0; /* end of init_fga() */
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*               SET VIDEO PARMS ACCORDING TO MONITOR USED                    */
/*                                                                            */
/*----------------------------------------------------------------------------*/
void
Get_Monitor (ld, ddf, dds)
struct fga_data *ld;
struct fga_ddf *ddf;
struct fbdds   *dds;                    /* contains config method
                                         * data-prefilled */
{
        struct crt_control * p_crt_control;
        unsigned char control_orig;     /* place to save adcntl value */
        unsigned long   cable_id,
                        status_vert,
                        status_horz;

        /*
         * Declare a variable to indicate which monitor type has been
         * detected.
         * This is coded the same as FGA_ADSTAT
         */

        int             monitor_type;

        /*
         * Set the local variable equal to global variable
         */

        p_crt_control = ld->P_CRT_CONTROL;

        /*
         * Determine the monitor for the FGA. The monitor type
         * determines all of the configuration values.
         */

        control_orig = *FGA_ADCNTL;     /* save adcntl value */

        /*
         * To determine the monitor type the following procedure is used:
         * 1. Set the vertical and horiz drive bits in the control reg to 0
         * 2. Read and save the status register value.
         * 3. Set the vertical bit in the control register to 1
         * 4. Read and save the status register value.
         * 5. Clear the vert bit and set the horiz bit in the control reg.
         * 6. Read and save the status register register value.
         * 7. Compare the original value with the two subsequent reads.
         */
#define HORZ_MODE 0x00000100
#define VERT_MODE 0x00000200

        /* Turn off H sync & V sync (bit = 1 means off) */
        *FGA_ADCNTL = control_orig | FGA_HSYNC_off | FGA_VSYNC_off;
        EIEIO;
        cable_id = *FGA_ADSTAT & 0x0f; /* read cable */

            BUGLPR (dbg_initf, BUGGID, ("cable_id = 0x%2.2x\n", cable_id));

        /* turn on H sync only */
        *FGA_ADCNTL = ( control_orig | FGA_VSYNC_off ) & ~FGA_HSYNC_off ;
        EIEIO;

        status_horz = (*FGA_ADSTAT & 0x0f) ^ cable_id;

            BUGLPR (dbg_initf, BUGGID, ("Horiz status = 0x%2.2x\n", \
                                         status_horz));

        /* turn on V sync only */
        *FGA_ADCNTL = ( control_orig | FGA_HSYNC_off ) & ~FGA_VSYNC_off ;
        EIEIO;

        status_vert = (*FGA_ADSTAT & 0x0f) ^ cable_id;

            BUGLPR (dbg_initf, BUGGID, ("Vertical status = 0x%2.2x\n", \
                                         status_vert));

        if ((status_horz == 0) && (status_vert == 0))
        {
                monitor_type = cable_id;
        }
        else if ((status_horz != 0) && (status_vert == 0))
        {
                monitor_type =  cable_id | status_horz | HORZ_MODE;
        }
        else if ((status_horz == 0) && (status_vert != 0))
        {
                monitor_type =  cable_id | status_vert | VERT_MODE;
        }
        else if ((status_horz != 0) && (status_vert != 0))
        {
                monitor_type =  cable_id | status_horz |
                                status_vert | HORZ_MODE | VERT_MODE;
        }

        /* We now have the monitor type */

            BUGLPR (dbg_initf, BUGNTA, ("Monitor type = 0x%8.8x\n", monitor_type));

        ddf->monitor_type = monitor_type;

        /*
         * Restore control register
         */

        *FGA_ADCNTL = control_orig;     /* restore control reg */
        EIEIO;

        /*
         * Scan through the structure for monitor type
         * The default crt register data is in index 0, so if
         * the ddf->monitor_type is not in the array,
         * the default crt register values are picked.
         */

            BUGLPR (dbg_initf, BUGGID, ("Monitor id desired = 0x%8.8x\n", \
                                          ddf->monitor_type));

        do
        {
                if ( p_crt_control->monitor_id == ddf->monitor_type )
                        break;
                p_crt_control++;

        } while ( p_crt_control->monitor_id != 0 );

            BUGLPR (dbg_initf, BUGGID, ("Monitor id found = 0x%8.8x\n", \
                                         p_crt_control->monitor_id));

        if ( p_crt_control->monitor_id == 0 )
        {
                p_crt_control = ld->P_CRT_CONTROL;
        }
        else
        {
                ld->P_CRT_CONTROL = p_crt_control;
        }

            BUGLPR (dbg_initf, BUGNTA, ("ld->P_CRT_CONTROL screen_res = %d x %d.\n", \
                                        ld->P_CRT_CONTROL->x_resolution,
                                        ld->P_CRT_CONTROL->y_resolution));

        /* pick up default resolution */
        ld->screen_width_pix  = ld->P_CRT_CONTROL->x_resolution;
        ld->screen_height_pix = ld->P_CRT_CONTROL->y_resolution;

        return;  /* end of Get_Monitor () */
}
