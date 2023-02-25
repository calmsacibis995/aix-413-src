static char sccsid[] = "@(#)53	1.2  src/rspc/kernext/disp/pciiga/ksr/font.c, pciiga, rspc41B, 9504A 12/15/94 16:38:40";
/*
 *
 * COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
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

#include "iga_INCLUDES.h"
#include "iga_funct.h"
GS_MODULE(iga_load_font);

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
load_font (vp, id)
        struct vtmstruc *vp;
        short           id;            /* font id, derived from the character
                                        * attribute */
{
        long            i;
        struct iga_data *ld;
        struct iga_ddf *ddf;

        ld = (struct iga_data *) vp->vttld;
        ddf = (struct iga_ddf *) vp->display->free_area;

	IGA_ENTER_TRC1(ddf,iga_load_font,2,IGA_LOAD_FONT,id);
	
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

	IGA_EXIT_TRC0(ddf,iga_load_font,2,IGA_LOAD_FONT);

} /* end of load font */
