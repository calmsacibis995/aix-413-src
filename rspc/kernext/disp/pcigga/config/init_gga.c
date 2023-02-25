static char sccsid[] = "@(#)80  1.6  src/rspc/kernext/disp/pcigga/config/init_gga.c, pcigga, rspc41B, 9504A 1/9/95 13:20:40";
/* init_gga.c */
/*
 *
 *   COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: gga_open
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
#include "gga_funct.h"

GS_MODULE(gga_init);

BUGXDEF (dbg_initf);

/*
 * Define the Monitor id values for each supported monitor
 *      MT_x are defined in gga_regval.h
 */

/* crt_control struct is defined in gga.h MT_xx is defined in gga_regval.h */
static struct crt_control s_crt_control[] =
{
    /* MT_0  0x201 */           /* 640x480, 72.81Hz, 31.57MHz    */
    {
        MT_0,                   /* Monitor ID               */
	0x05,			/* SRTCTL2		    */
	0x08563000,		/* SYSCNFG		    */
	0x3d,			/* F0			    */
	0x03,			/* PIXEL_FMT		    */
        0x27,			/* MISC_CLK		    */
        0x65, 	                /* HRZT                     */
        0x0a,                   /* HRZSR                    */
        0x0c, 	                /* HRZBR                    */
        0x5c,	                /* HRZBF                    */
        0x208,                  /* VRTT                     */
        0x03, 	                /* VRTSR                    */
        0x1e, 	                /* VRTBR                    */
        0x1fe,                  /* VRTBF                    */
          640, 	                /* Horizontal Resolution    */
          480,                  /* Vertical Resolution      */
        COLOR                   /* Color / Mono             */
    },

    /* MT_1  0x205 */           /* 1024x768  @ 75.6Hz 80Mhz */ 
    {
 	MT_1,                   /* Monitor ID               */
	0x00,			/* SRTCTL2		    */
	0x08603000,		/* SYSCNFG		    */
	0x8f,			/* F0			    */
	0x03,			/* PIXEL_FMT		    */
        0x27,			/* MISC_CLK		    */
        0xab, 	                /* HRZT                     */
        0x23,                   /* HRZSR                    */
        0x25, 	                /* HRZBR                    */
        0xa5,	                /* HRZBF                    */
        0x326,                  /* VRTT                     */
        0x08, 	                /* VRTSR                    */
        0x26, 	                /* VRTBR                    */
        0x325,                  /* VRTBF                    */
         1024, 	                /* Horizontal Resolution    */
          768,                  /* Vertical Resolution      */
        COLOR                   /* Color / Mono             */
    },

    /* MT_2  0x107 */           /* 1280x1024 @ 60Hz 111.5Mhz*/ 
    {				/* 4 meg		    */
 	MT_2,                   /* Monitor ID               */
	0x05,			/* SRTCTL2		    */
	0x08683000,		/* SYSCNFG		    */
	0xaf,			/* F0			    */
	0x03,			/* PIXEL_FMT		    */
        0x27,			/* MISC_CLK		    */
        0xdb, 	                /* HRZT                     */
        0x18,                   /* HRZSR                    */
        0x3a, 	                /* HRZBR                    */
        0xda,	                /* HRZBF                    */
        0x420,                  /* VRTT                     */
        0x03, 	                /* VRTSR                    */
        0x1d, 	                /* VRTBR                    */
        0x41d,                  /* VRTBF                    */
         1280, 	                /* Horizontal Resolution    */
         1024,                  /* Vertical Resolution      */
        COLOR                   /* Color / Mono             */
    },

    {				/* 2 meg		    */
 	MT_2,                   /* Monitor ID               */
	0x05,			/* SRTCTL2		    */
	0x08683000,		/* SYSCNFG		    */
	0xaf,			/* F0			    */
	0x03,			/* PIXEL_FMT		    */
        0x27,			/* MISC_CLK		    */
        0xdc, 	                /* HRZT                     */
        0x18,                   /* HRZSR                    */
        0x34, 	                /* HRZBR                    */
        0xd4,	                /* HRZBF                    */
        0x41e,                  /* VRTT                     */
        0x03, 	                /* VRTSR                    */
        0x1d, 	                /* VRTBR                    */
        0x41d,                  /* VRTBF                    */
         1280, 	                /* Horizontal Resolution    */
         1024,                  /* Vertical Resolution      */
        COLOR                   /* Color / Mono             */
    },

    /* MT_3  0x6 */		/* 640x480 @ 60Hz 25.17Mhz*/ 
    {
 	MT_3,                   /* Monitor ID               */
	0x05,			/* SRTCTL2		    */
	0x08563000,		/* SYSCNFG		    */
	0x24,			/* F0			    */
	0x03,			/* PIXEL_FMT		    */
        0x27,			/* MISC_CLK		    */
        0x63, 	                /* HRZT                     */
        0x0b,                   /* HRZSR                    */
        0x0c, 	                /* HRZBR                    */
        0x5c,	                /* HRZBF                    */
        0x214,                  /* VRTT                     */
        0x02, 	                /* VRTSR                    */
        0x22, 	                /* VRTBR                    */
        0x202,                  /* VRTBF                    */
         640, 	                /* Horizontal Resolution    */
         480,	                /* Vertical Resolution      */
        COLOR                   /* Color / Mono             */
    },

/*
 * This is the end of the structure, so if more monitor types are to be
 * added, do so ABOVE THIS LINE, as the scanning algorithm looks for the
 * 0 in the first field to stop.
 */

    /* MT_END 0 */
    {
        MT_END,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    }
};

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                     GGA INITIALIZATION ROUTINES                            */
/*                                                                            */
/*      Needs:  pointer to local data structure                               */
/*                                                                            */
/*      Returns: -1 if can't find GGA, otherwise 0                            */
/*                                                                            */
/*      Called by: vtt_init                                                   */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

int init_gga(vp, ld, ddf, dds)          /* Mainline routine */
struct vtmstruc *vp;
struct gga_data *ld;
struct gga_ddf *ddf;
struct gbdds   *dds;                    /* contains config method
                                         * data-prefilled */
{
    struct crt_control * p_crt_control;
    int  i, rc;
    ulong  wtk_reg;
    struct phys_displays *pd;

    BUGLPR (dbg_initf, BUGNTA, ("Starting initialization\n"));

    GGA_ENTER_TRC(ddf,gga_init,2,GGA_INIT,vp,ld,ddf,0,0);

    switch( ddf->monitor_type )
    {
	case MT_0: i = 0; break;
	case MT_1: i = 1; break;
	case MT_2: 
    		   if ( dds->vram_config == 1 )
			i = 2; 
		   else i = 3;
		   break;
	case MT_3: i = 4; break;
	default: i = 0;
    }

    GGA_PARM_TRC(ddf,gga_init,2,GGA_INIT,GGA_DSP_TYPE,ddf->monitor_type,i,0,0,0);

    ld->P_CRT_CONTROL = &s_crt_control[i];

    BUGLPR (dbg_initf, BUGGID, ("ld->P_CRT_CONTROL = 0x%x\n",ld->P_CRT_CONTROL));

    /*
     * Set the local variable equal to global variable
     */

    p_crt_control = ld->P_CRT_CONTROL;

    /* set addressability */

        BUGLPR (dbg_initf, BUGNTA, ("WTK_BASE_ADDR   = 0x%x\n",WTK_BASE_ADDR  ));
        BUGLPR (dbg_initf, BUGNTA, ("WTK_PARM_BASE   = 0x%x\n",WTK_PARM_BASE  ));
        BUGLPR (dbg_initf, BUGNTA, ("WTK_VRAM_BASE   = 0x%x\n",WTK_VRAM_BASE  ));
        BUGLPR (dbg_initf, BUGNTA, ("RGB525_BASE_ADDR = 0x%x\n",RGB525_BASE_ADDR));

    ld->wtk_intr_enbl_reg      = WTK_INTR_ENABLE_DATA;  /* disable interrupts */

    if ( dds->vram_config == 1 )
    {
    	BUGLPR (dbg_initf, BUGGID, ("Adapter has 4 Meg VRAM\n"));
    	ld->wtk_memcfg_reg 	= WTK_MEM_CONFIG_4;
    	ld->wtk_vram_end = WTK_VRAM_SIZE_4 - 1;
    }
    else
    {
    	BUGLPR (dbg_initf, BUGGID, ("Adapter has 2 Meg VRAM\n"));
    	ld->wtk_memcfg_reg 	= WTK_MEM_CONFIG_2;
    	ld->wtk_vram_end = WTK_VRAM_SIZE_2 - 1;
    }
    ld->wtk_srtctl_reg		= WTK_SRTCTL_DATA;
    ld->mem_shift		= ld->P_CRT_CONTROL->x_resolution;

    ld->wtk_rfperiod_reg       = WTK_RF_PERIOD_DATA;
    ld->wtk_rlmax_reg          = WTK_RL_MAX_DATA;
    ld->wtk_prevrtc_reg        = WTK_PREVRTC_DATA;
    ld->wtk_prehrzc_reg        = WTK_PREHRZC_DATA;
    ld->wtk_plane_mask         = WTK_PLANE_MASK_DATA;
    ld->wtk_draw_mode          = WTK_DRAW_MODE_DATA;
    ld->wtk_coor_offs          = WTK_COOR_OFFS_DATA;


    /* see what dds says about max resolution */

    ld->screen_width_pix  	= ld->P_CRT_CONTROL->x_resolution;
    ld->screen_height_pix 	= ld->P_CRT_CONTROL->y_resolution;
    ld->wtk_w_min_xy           	= 0;       /* same as Pack_XY( 0, 0 ); */
    ld->wtk_w_max_xy           	= Pack_XY(ld->screen_width_pix-1, \
                                         ld->screen_height_pix-1);
    BUGLPR (dbg_initf, BUGNTA, ("ld max_xy = 0x%x\n", ld->wtk_w_max_xy));

    GGA_EXIT_TRC0(ddf,gga_init,2,GGA_INIT);

    return 0; /* end of init_gga() */
}

