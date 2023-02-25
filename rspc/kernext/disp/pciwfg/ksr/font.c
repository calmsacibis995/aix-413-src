static char sccsid[] = "@(#)12  1.1  src/rspc/kernext/disp/pciwfg/ksr/font.c, pciwfg, rspc411, 9434B411a 8/24/94 07:44:09";
/* font.c */
/*
 *
 * COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
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

#include "wfg_INCLUDES.h"
#include "wfg_funct.h"
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
        struct wfg_data *ld;
        ld = (struct wfg_data *) vp->vttld;

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
} /* end of load font */
