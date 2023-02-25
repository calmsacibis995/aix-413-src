static char sccsid[] = "@(#)66	1.4  src/rspc/kernext/disp/pciiga/ksr/vtt_init.c, pciiga, rspc41J, 9515B_all 3/22/95 17:11:54";
/*
 *
 *   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttinit
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
#include "iga_funct.h"
#define SCRN_WIDTH 80
#define SCRN_HEIGHT 25
BUGXDEF (dbg_vttinit);
GS_MODULE(iga_vttinit);

/*********************************************************************/
/*                                                                   */
/* IDENTIFICATION: IGA_INIT                                          */
/*                                                                   */
/* DESCRIPTIVE name: Initialize Graphics Adapter                     */
/*                     Virtual Display Driver (VDD)                  */
/*                                                                   */
/* FUNCTION: Initialize the internal state of the device driver:     */
/*                                                                   */
/*                                                                   */
/*                     Following steps are done once:                */
/*                     ------------------------------                */
/*                                                                   */
/*   - Allocate local data structure for the virtual terminal        */
/*     and initializer various fields of the structure               */
/*                                                                   */
/*   - From the display mode, calculate screen resolution            */
/*                                                                   */
/*   - From the screen resolution, pick best font to fit it          */
/*                                                                   */
/*   - Initializer environment table by setting values for           */
/*     cursor shape and position, character box values based on      */
/*     font size.                                                    */
/*                                                                   */
/*   - Allocate Presentation space for 80x25 character grid          */
/*                                                                   */
/*   - Initialize adapter and get monitor id                         */
/*                                                                   */
/*   - load local data structure with KSR color map                  */
/*                                                                   */
/*        Following steps are done every time vttinit is called:     */
/*        -------------------------------------------------------    */
/*                                                                   */
/*   _ Initialize each word in the Pspace to a blank with a          */
/*     normal attribute.                                             */
/*                                                                   */
/*   _ reset the adapter and load color map for KSR mode             */
/*                                                                   */
/*  INPUTS:    Virtual terminal structure                            */
/*        Pointer to list of font ids    (not used)                  */
/*        Pointer to Presentation space size structure               */
/*                                                                   */
/*  OUTPUTS:   INVALID_VIRTUAL_TERMINAL_ID                           */
/*        INVALID_FONT_ID                                            */
/*        INVALID_FONT_SIZE                                          */
/*        NO_USABLE_FONT                                             */
/*                                                                   */
/*  CALLED BY: lftinit() and rcm                                     */
/*                                                                   */
/*  CALLS:                                                           */
/*        Select_new_font to set default font                        */
/*        init_hga to set-up initialization values                   */
/*        iga_reset to reset adapter                                 */
/*                                                                   */
/*********************************************************************/


long
vttinit (vp, font_ids, ps_s)
        struct vtmstruc *vp;
        struct fontpal *font_ids;      /* real font ids  */
        struct t_ps_s  *ps_s;          /* presentation space size     */

{

        long  		rc,
                        i,
                        j,
                        font_w,
                        font_h;

        struct iga_data *ld;
        struct fbdds   *dds;
        struct iga_ddf *ddf;
        ushort          xres,
                        yres;

        int             x_offset;
        int             y_offset;
        int             screen_width, screen_height;

        dds = (struct fbdds *) vp->display->odmdds;
        ddf = (struct iga_ddf *) vp->display->free_area;

#undef INIT

        IGA_ENTER_TRC0(ddf,iga_vttinit,2,IGA_VTTINIT);

        VDD_TRACE(INIT, ENTRY, vp);

        /*
         * The following is a debug block for input parameters
         */


        BUGLPR (dbg_vttinit, BUGGID, ("\n"));
        BUGLPR (dbg_vttinit, BUGGID, ("Input parameters follow:\n"));

        BUGLPR (dbg_vttinit, BUGGID, ("vp                = 0x%x\n", vp));
        BUGLPR (dbg_vttinit, BUGGID, ("vp->display       = 0x%x\n", vp->display));
        BUGLPR (dbg_vttinit, BUGGID, ("vp->display->odmdds = 0x%x\n", vp->display->odmdds));
        BUGLPR (dbg_vttinit, BUGGID, ("vp->display_free_area = 0x%x\n", vp->display->free_area));
        BUGLPR (dbg_vttinit, BUGGID, ("vp->display->disp_devid[1] = 0x%x\n", vp->display->disp_devid[1]));
        BUGLPR (dbg_vttinit, BUGGID, ("vp->display->usage = 0x%x\n", vp->display->usage));

        BUGLPR (dbg_vttinit, BUGGID, ("ps_s->ps_w        = %d\n", ps_s->ps_w));
        BUGLPR (dbg_vttinit, BUGGID, ("ps_s->ps_h        = %d\n", ps_s->ps_h));

	ddf->IO_in_progress += 1;   /* set flag to indicate IO is going to take place */

        if (vp->vttld == NULL)         /* If non null then we are changing
                                        * fontpalette */
        {
                /* Allocate memory for local data area */

                ld = (struct iga_data *) xmalloc (sizeof (struct iga_data), 3, pinned_heap);
                if (ld == NULL)
                {                      /* Log an error */
                        iga_err (vp, "PCIIGA", "VTT_INIT", "XMALLOC", ld, IGA_BAD_XMALLOC, RAS_UNIQUE_1);

	                ddf->IO_in_progress -= 1;   

                        return (ENOMEM);
                }

                vp->vttld = (char *) ld;        /* CIS,36 */
                bzero (ld, sizeof (struct iga_data));   /* clear local data area */
                ld->comp_name = dds->component;
                vp->display->usage++;  /* see CIS,32 */
                ld->ddf = ddf;


        	BUGLPR (dbg_vttinit, BUGNTA, ("-PRE INIT_IGA\n"));

	        /********************************/
        	/*   Initialize the adapter     */
        	/********************************/
        	rc = (long) init_iga( vp, ld, ddf, dds );

        	BUGLPR (dbg_vttinit, BUGNTA, ("POST INIT_IGA\n"));

        	/*-----------------------------------------------
             	Use display mode to determine resolution
         	-----------------------------------------------*/

        	switch ( dds->display_mode )
        	{
          	case 0x201:                     /*  640 x  480 x  8 */
               		xres =  640; yres = 480;
               		break;

          	case 0x205:                     /* 1024 x  768 x  8 */
               		xres = 1024; yres = 768;
               	break;

          	case 0x107:                     /* 1280 x 1024 x  8 */
          	case 0x3ff:                     /* 1280 x 1024 x  8 */
               	xres = 1280; yres = 1024;
               		break;

          	case 0x111:                     /*  640 x  480 x 16 */
          	case 0x211:                     /*  640 x  480 x 16 */
          	case 0x204:                     /* 1024 x  768 x  4 */
          	case 0x177:                     /* 1024 x  768 x 16 */
          		default:
               		BUGLPR( dbg_vttinit,BUGNFO,("Unsupported display mode: 0x%x\n",dds->display_mode ));
               		BUGLPR( dbg_vttinit,BUGNFO,("\tassuming 1024 x 768 (0x205)\n"));
               		dds->display_mode = 0x205;
               		xres=1024; yres= 768;
               		break;
        	}

		ld->vert_stride = xres;	/* distance between adjacent vertical pixels */

        	/* Call load_font_table to load the best font into the font table */
        	if ( (rc=load_font_table(vp, xres, yres)) != 0)
        	{
            		/* log an error */
            		iga_err(vp,"PCIIGA","VTT_INIT","LOAD_FONT_TABLE",rc,IGA_FONT_LOAD,RAS_UNIQUE_2);

	                ddf->IO_in_progress -= 1;   

            		return( rc );
        	}

	
		/***************************************************
		 * set the cursor double underscore                * 
 		***************************************************/

        	ld->Vttenv.cursor_select = UNDERSCORE2;

        	font_w = ld->Vttenv.font_table[0].width;
        	font_h = ld->Vttenv.font_table[0].height;

        	ld->Vttenv.cursor_shape.cursor_top = font_h - 2;
        	ld->Vttenv.cursor_shape.cursor_bot = font_h - 1;
	
		/* set presentation space size and centering offset */

        	ld->Vttenv.ps_size.wd = ps_s -> ps_w = SCRN_WIDTH;
        	ld->Vttenv.ps_size.ht = ps_s -> ps_h = SCRN_HEIGHT;

        	if ( (screen_width = SCRN_WIDTH * font_w) > xres )
              		ld->center_x = 1;
        	else
              		ld->center_x = (xres - screen_width) >> 1;

        	if ( (screen_height = SCRN_HEIGHT * font_h) > yres )
              		ld->center_y = font_h + 1;
        	else
              		ld->center_y = (yres - screen_height) >> 1;

        	BUGLPR (dbg_vttinit, BUGGID, ("Font and display parameters follow:\n"));
        	BUGLPR (dbg_vttinit, BUGGID, ("Font default = 0x%x\n", font_default));
        	BUGLPR (dbg_vttinit, BUGGID, ("Font height = 0x%x\n", font_h));
        	BUGLPR (dbg_vttinit, BUGGID, ("Font width  = 0x%x\n", font_w));
        	BUGLPR (dbg_vttinit, BUGGID, ("Display height 0x%x \n", yres));
        	BUGLPR (dbg_vttinit, BUGGID, ("Display width 0x%x \n", xres));
        	BUGLPR (dbg_vttinit, BUGGID, ("Display x offset 0x%x \n", ld->center_x));
        	BUGLPR (dbg_vttinit, BUGGID, ("Display y offset 0x%x \n", ld->center_y));

	        /******************************************************
        	* set the character box values for cursor definition *
        	*****************************************************/
        	ld->fonthptr = (aixFontInfoPtr) ld->Vttenv.font_table[0].addr;
        	ld->Vttenv.character_box.height = ld->fonthptr->minbounds.ascent +
                	ld->fonthptr->minbounds.descent;
        	ld->Vttenv.character_box.width = ld->fonthptr->minbounds.characterWidth;
        	BUGLPR (dbg_vttinit, BUGNTA, ("character_boxh = %d      character_boxw = %d\n",
                                      		ld->Vttenv.character_box.height,
                                      		ld->Vttenv.character_box.width));

      		if (ld->Vttenv.character_box.width & 0x7)
        	{
                ld->Vttenv.font_box_width =
                        (((ld->fonthptr->minbounds.characterWidth >> 3) + 1) << 3);
        	}
        	else
        	{
                	ld->Vttenv.font_box_width =
                        	((ld->fonthptr->minbounds.characterWidth >> 3) << 3);
        	}
        	ld->charwd2 = ld->Vttenv.character_box.width / 2;
        	if (ld->Vttenv.character_box.width % 2)
        	{
                	ld->charwd2++;
        	}

                /************************************************************
                * Allocate the Presentation Space - 80 by 25 character grid *
                * Note each character is represented by 1 word (4 bytes)    *
                *************************************************************/

                ld->Vttenv.ps_bytes = SCRN_WIDTH * SCRN_HEIGHT * sizeof(long) ;
                ld->Vttenv.ps_words = (ld->Vttenv.ps_bytes) / sizeof(long);

                ld->Vttenv.pse = (ulong *) xmalloc (ld->Vttenv.ps_bytes, 3, pinned_heap);
                if (ld->Vttenv.pse == NULL)
                {
                        iga_err (vp, "PCIIGA", "VTT_INIT", "XMALLOC", ld->Vttenv.pse, IGA_BAD_XMALLOC,
                                RAS_UNIQUE_3);

	                ddf->IO_in_progress -= 1;   

                        return (ENOMEM);
                }

                /****************************************/
                /* Initialize color table with defaults */
                /****************************************/

                ld->ksr_color_table.num_entries = 16;
                /* BUGLPR(dbg_vttinit,1,("Setting NUMENTRIES TO %d\n",16)); */
                for (i = 0; i < ld->ksr_color_table.num_entries; i++)
                {
                        ld->ksr_color_table.rgbval[i] = dds->ksr_color_table[i];
                        /*
                         * BUGLPR(dbg_vttinit,1,("RGB[%d] =
                         * %X\n",i,dds->ksr_color_table[i]));
                         */
                }

		/* set the vt mode for KSR */
                ld->Vttenv.vtt_mode = KSR_MODE;
        }


        BUGLPR (dbg_vttinit, BUGNTA, ("In vtt_init -  clearing presentation SPACE....."));

        /****************************************************************
        * for each word in the presentation space, set character code   * 
        * to "spaces" and the attribute code to fg color 1              *
        *****************************************************************/

        ld = (struct iga_data *) vp->vttld;

        for (i = 0; i < ld->Vttenv.ps_words; i++)

        {
                ld->Vttenv.pse[i] = BLANK_CODE | ATTRIB_DEFAULT;
        }

	ld->Scroll_offset = 0;  /* set virtual presentation space top */

        ld->Vttenv.current_attr = 0xFFFF;
	ld->cur_font = -1;

        ld->Vttenv.cursor_pos.cursor_col = 1;
        ld->Vttenv.cursor_pos.cursor_row = 1;

	iga_reset(vp,ld,ddf);

        VDD_TRACE(INIT, EXIT, vp);

        /* BUGLPR(dbg_vttinit,BUGNTA,("INIT Exiting\n")); */

        ddf->IO_in_progress -= 1;    /* reset flag */ 

        IGA_EXIT_TRC0(ddf,iga_vttinit,2,IGA_VTTINIT);

        return (0);
}                                      /* end  of  vttinit   */


 /*-------------------------------------------------------------------------

        Function Name: load_font_table

        Descriptive Name: Load local font table

        Function:

        This function will load the local font table with font data stored
        in the lft font table. The lft font table may have from 1 to 8
        fonts stored in the table. Only one font is selected from the lft
        table and will be loaded into all eight indices of the local font
        table. If a custom font is specified (font_index >= 0), then it will
        be used to determine the font otherwise the font from the lft table
        that best fits an 80x25 character grid within the screen size of
        the given monitor type will be used.

  ------------------------------------------------------------------------*/

int load_font_table(vp, xres, yres)
struct vtmstruc *vp;
ushort xres, yres;
{
  struct iga_data *ld;
  int best_index, default_index, best_value, default_value;
  int width, height;
  int i, tmp_value;

  /* first check if a custom font was specified, if so use it */
  if( vp->font_index >= 0 )
  {
    best_index = vp->font_index;
  }
  else /* need to search all fonts in the table for the best fit */
  {
    /* initialize values to worst case to ensure a font is found */
    best_value = default_value = xres + yres;
    best_index = default_index = -1;

    /* loop thru all the fonts looking for the best fit for a 80x25 screen */
    for( i = 0; i < vp->number_of_fonts; i++ )
    {
      /* calculate total width and height in pixels for 80x25 characters */
      width = vp->fonts[i].font_width * SCRN_WIDTH;
      height = vp->fonts[i].font_height * SCRN_HEIGHT;
#define IDEAL_MARGIN 50*2
#define abs( x ) ( (x) < 0 ? -(x) : (x) )
      /* calculate how close font is to ideal screen size */
      tmp_value = abs(xres - IDEAL_MARGIN - width) +
                  abs(yres - IDEAL_MARGIN - height);
#define MIN_MARGIN 0
      if( (width <= xres - MIN_MARGIN) && (height <= yres - MIN_MARGIN) )
      {
        /* best catagory, check if current font is better fit */
        if( tmp_value < best_value )
        {
          best_index = i;
          best_value = tmp_value;
        }
      }
      else
      {
        /* default catagory, check if current font is better fit */
        if( tmp_value < default_value || default_index == -1 )
        {
          default_index = i;
          default_value = tmp_value;
        }
      }
    } /* end of for loop of all fonts */

    /* if best case font not found then use default font */
    if ( best_index < 0 )
      best_index = default_index;

  } /* end of  if( vp->font_index >= 0 ) */

  /* sanity check */
  if( best_index < 0 || best_index >= vp->number_of_fonts )
    best_index = 0;

  /* get local data structure pointer */
  ld = (struct iga_data *) vp->vttld;

  /* force the initial font to be loaded by setting current font to -1 */
  ld->cur_font = -1;

  /* load the local font table with the selected font */
#define MAX_NUM_FONTS 8
  for (i = 0; i < MAX_NUM_FONTS; i++)
  {
    ld->Vttenv.font_table[i].index  = best_index;
    ld->Vttenv.font_table[i].addr   = (char *) vp->fonts[best_index].font_ptr;
    ld->Vttenv.font_table[i].id     = vp->fonts[best_index].font_id;
    ld->Vttenv.font_table[i].height = vp->fonts[best_index].font_height;
    ld->Vttenv.font_table[i].width  = vp->fonts[best_index].font_width;
    ld->Vttenv.font_table[i].size   = vp->fonts[best_index].font_size >> 2;
  }
  return(0);
}


