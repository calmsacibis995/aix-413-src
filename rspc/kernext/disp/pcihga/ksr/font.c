static char sccsid[] = "@(#)35  1.1  src/rspc/kernext/disp/pcihga/ksr/font.c, pcihga, rspc411, GOLD410 4/15/94 18:22:45";
/* font.c */
/*
based on "@(#)44  1.3  src/bos/kernext/disp/wga/ksr/font.c, bos, bos410 10/28/93 17:41:51";
 *
 * COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
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

#include "hga_INCLUDES.h"
#include "hga_funct.h"
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
        struct hga_data *ld;
        ld = (struct hga_data *) vp->vttld;

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
