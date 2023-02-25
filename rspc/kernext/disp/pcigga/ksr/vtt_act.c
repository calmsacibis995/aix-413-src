static char sccsid[] = "@(#)08  1.3  src/rspc/kernext/disp/pcigga/ksr/vtt_act.c, pcigga, rspc41J, 9513A_all 3/22/95 11:13:37";
/* vtt_act.c */
/*
based on "@(#)47  1.3  src/bos/kernext/disp/wga/ksr/vtt_act.c, bos, bos410 10/28/93 17:45:50";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: vttact
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
#define SHOW 1
BUGXDEF (dbg_vttact);
GS_MODULE(gga_vttact);

/*-------------------------------------------------------------------
 *
 * IDENTIFICATION: VTTACT
 *
 * DESCRIPTIVE name:  Activate an inactive virtual terminal,
 *           
 *
 * FUNCTION:
 *             Set the VDD state to active.
 *
 *             If we are activating a Character (KSR) terminal:
 *                - Copy the contents of the presentation space into
 *                  the adapter frame buffer.
 *                - Turn on cursor
 *
 *             If user has requested something other than KSR 
 *             they are in error:
 *                - Log an error and return a Zero
 *
 *
 * INPUTS:    Virtual terminal pointer
 *
 * OUTPUTS:
 *
 * CALLED BY:  lftinit() and rcm 
 *             vttsetm - dead code 
 *
 * CALLS:
 *              copy_ps
 *              vttdef
 *
 -------------------------------------------------------------------*/

long
vttact (vp)
        struct vtmstruc *vp;
{

        struct gga_data *ld;
        struct gga_ddf *ddf;
	struct gbdds *dds;
        int rc = 0;
/*
 * The following is a debug print block intended to display all input parameters
 */

BUGLPR(dbg_vttact,BUGGID,("\n"));
BUGLPR(dbg_vttact,BUGGID,("Input parameters follow:\n"));

BUGLPR(dbg_vttact,BUGGID,("vp                = 0x%x\n",vp));
BUGLPR(dbg_vttact,BUGGID,("vp->vttld         = 0x%x\n",vp->vttld));
BUGLPR(dbg_vttact,BUGGID,("vp->display       = 0x%x\n",vp->display));
BUGLPR(dbg_vttact,BUGGID,("vp->display->free_area = 0x%x\n",vp->display->free_area));


        ld = (struct gga_data *) vp->vttld;
        ddf = (struct gga_ddf *) vp->display->free_area;
	dds = (struct gbdds *) vp->display->odmdds;

	GGA_ENTER_TRC0(ddf,gga_vttact,2,GGA_VTTACT);

        VDD_TRACE (ACT, ENTRY, vp);


        /*----------------------------------
        | set the VDD state to activated
        |---------------------------------*/

        ld->Vttenv.vtt_active = VTT_ACTIVE;


        /*----------------------------------------
        | switch on the current state of the VDD
        |-----------------------------------------*/
        switch (ld->Vttenv.vtt_mode)
        {

        case KSR_MODE:


                /*------------------------------------------------------------
                | copy the contents of the presentation space into the
                | frame buffer establish the correct
                | position and shape of the hardware cursor.
                |
                | NOTE: A newly selected VDD has the following attributes:
                | - the presentation space is initialized with all spaces
                | - the shape of the hardware cursor is a double underscore
                | - the hw cursor is in the upper-left corner of the screen
                |-----------------------------------------------------------*/

		GGA_PARM_TRC0(ddf,gga_vttact,2,GGA_VTTACT,GGA_VTTACT_KSR_MODE);

		ddf->IO_in_progress += 1;        /* set flag to indicate IO happening */

                BUGLPR (dbg_vttact, BUGNTA, ("Calling copy_ps\n"));
                copy_ps (vp, 1L, (long) ld->Vttenv.ps_size.ht, TRUE, FALSE);
                BUGLPR (dbg_vttact, BUGNTA, ("Returned from copy_ps\n"));

                /*  show cursor */
                BUGLPR (dbg_vttact, BUGNTA, ("Calling define cursor\n"));
                vttdefc (vp, ld->Vttenv.cursor_select, SHOW);
                BUGLPR (dbg_vttact, BUGNTA, ("Returned from define cursor \n"));

		ddf->IO_in_progress -= 1;        /* reset flag */

                break; 

        case GRAPHICS_MODE:
        default:

                gga_err (vp, "PCIGGA", "VTT_ACT", NULL, EINVAL, GGA_INVALID_MODE, RAS_UNIQUE_1);
                rc = EINVAL;
        }       

        VDD_TRACE (ACT, EXIT, vp);
        BUGLPR (dbg_vttact, BUGNTA, ("Returning from vtt_activate ( error code = %d )\n",rc));

	GGA_EXIT_TRC0(ddf,gga_vttact,2,GGA_VTTACT);

        return (rc);
}                                      /* end  of  vttact */
