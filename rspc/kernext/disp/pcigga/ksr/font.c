static char sccsid[] = "@(#)01  1.2  src/rspc/kernext/disp/pcigga/ksr/font.c, pcigga, rspc41B, 9504A 1/9/95 13:29:34";
/* font.c */
/*
based on "@(#)44  1.3  src/bos/kernext/disp/wga/ksr/font.c, bos, bos410 10/28/93 17:41:51";
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 * FUNCTIONS: load_font
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
GS_MODULE(gga_load_font);
/*-----------------------------------------------------------------------
 *
 * DESCRIPTION OF MODULE:
 *
 * IDENTIFICATION :   LOAD_FONT()
 *
 * DESCRIPTIVE NAME:
 *
 *
 * FUNCTION: This function is used as
 *
 *
 * INPUTS:   Virtual Terminal pointer
 *             The ID of the font to load on the adapter
 *
 * OUTPUTS:  NONE
 *
 * CALLED BY: SELECT_NEW_FONT
 *
 * CALLS:    NONE
 *
 ------------------------------------------------------------------------*/

void
load_font (vp, id)
        struct vtmstruc *vp;
        short           id;            /* font id, derived from the character
                                        * attribute */
{
        long            i;
        struct gga_data *ld;
        struct gga_ddf *ddf;

        ld = (struct gga_data *) vp->vttld;
        ddf = (struct gga_ddf *) vp->display->free_area;

	GGA_ENTER_TRC1(ddf,gga_load_font,2,GGA_LOAD_FONT,id);

        /* Set up pointers to font parts */
        ld->fonthptr = (aixFontInfoPtr) ld->Vttenv.font_table[id].addr;

        ld->fontcindex = (aixCharInfoPtr)
                ((ulong) (ld->fonthptr) + BYTESOFFONTINFO (ld->fonthptr));

        ld->glyphptr = (aixGlyphPtr)
                ((ulong) (ld->fontcindex) + BYTESOFCHARINFO (ld->fonthptr));

        ld->Vttenv.character_box.height = ld->fonthptr->minbounds.ascent +
                ld->fonthptr->minbounds.descent;

        ld->Vttenv.character_box.width = ld->fonthptr->minbounds.characterWidth;

        i = (ld->fonthptr->minbounds.characterWidth & 0x7) ? 1 : 0;

        ld->Vttenv.font_box_width =
                ((ld->fonthptr->minbounds.characterWidth >> 3) + i) << 3;

	GGA_EXIT_TRC0(ddf,gga_load_font,2,GGA_LOAD_FONT);

} /* end of load font */
