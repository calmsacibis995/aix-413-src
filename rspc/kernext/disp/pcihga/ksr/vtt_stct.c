static char sccsid[] = "@(#)52  1.1  src/rspc/kernext/disp/pcihga/ksr/vtt_stct.c, pcihga, rspc411, GOLD410 4/15/94 18:23:30";
/* vtt_stct.c */
/*
based on "@(#)60        1.3  src/bos/kernext/disp/wga/ksr/vtt_stct.c, bos, bos410 3/16/93 20:45:32";
 *
 *   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: vttstct
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
BUGXDEF(dbg_vttstct);
/***********************************************************************/
/*                                                                     */
/* IDENTIFICATION: VTTSTCT                                             */
/*                                                                     */
/* DESCRIPTIVE NAME: Set color table routine                           */
/*                                                                     */
/* FUNCTION:  This procedure sets the default color table to the color */
/*              table passed into the routine.                         */
/*                                                                     */
/*              If not in character mode:                              */
/*              - Return EIO                                           */
/*                                                                     */
/*              If the number of colors sent down is bigger than max   */
/*              defined, only load what we can handle.                 */
/*                                                                     */
/*              Load colors onto adapter one at at time while loading  */
/*              values into the default table.                         */
/*                                                                     */
/* INPUTS:    Color table in the structure of vttcolt.                 */
/*              Number of entries in the input table.                  */
/*                                                                     */
/* OUTPUTS:   EIO                                                      */
/*                                                                     */
/* CALLED BY: Mode processor                                           */
/*                                                                     */
/* CALLS:     load_palette                                             */
/*                                                                     */
/***********************************************************************/

long vttstct (vp, color_table)
struct vtmstruc    *vp;
struct colorpal    *color_table;

{
 register long           i;
 struct hga_data *ld;
 long table_size;

BUGLPR(dbg_vttstct,BUGGID,("\n"));
BUGLPR(dbg_vttstct,BUGGID,("Input parameters follow:\n"));
BUGLPR(dbg_vttstct,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttstct,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));
BUGLPR(dbg_vttstct,BUGGID,("Number colors ordered       = 0x%x\n",color_table->numcolors));

    ld = (struct hga_data *) vp->vttld;

    VDD_TRACE(STCT , ENTRY, vp);

    if /*-----------------------*/
       /* not in character mode */
       /*-----------------------*/
              (ld->Vttenv.vtt_mode != KSR_MODE)
    {
       /*------------------------------------*/
       /* Only valid in character (KSR) mode */
       /*------------------------------------*/
       return(EIO);
    }

    table_size = color_table->numcolors;

    if (table_size > COLORTAB)      /* Only load as many colors as the table */
       table_size = COLORTAB;       /* can hold                              */

    ld->ksr_color_table.num_entries = table_size;

    BUGLPR(dbg_vttstct,BUGNTX,("Setting NUMENTRIES TO %d\n",table_size));
    for ( i = 0; i < table_size; i++ )  /* for all of the colors in the */
    {                                   /* passed table.                */
                                        /* reload default color table   */
       ld->ksr_color_table.rgbval[i] = color_table->rgbval[i] ;
       BUGLPR(dbg_vttstct,BUGNTX,("Input RGB[%d] = %X\n",i,color_table->rgbval[i]));
       BUGLPR(dbg_vttstct,BUGNTX,("ld->rgbval[%d] = %X\n",i,ld->ksr_color_table.rgbval[i]));
    }


    if (ld->Vttenv.vtt_active)          /* the virtual terminal is active */
    {
      /*BUGLPR(dbg_vttstct,BUGNTX,("STCT is calling LOADPALETTE\n"));*/
      load_palette(ld);
      /*BUGLPR(dbg_vttstct,BUGNTX,("STCT has returned from LOADPALETTE\n"));*/
    }

    VDD_TRACE(STCT , EXIT, vp);
    /*BUGLPR(dbg_vttstct,BUGNTX,("Leaving STCT\n"));*/
    return(0);
} /*  end  of  vttstct */
