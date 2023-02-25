static char sccsid[] = "@(#)06  1.3  src/bos/kernext/lft/swkbd/diac88592.c, lftdd, bos411, 9428A410j 4/8/94 14:26:05";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

 /*
 *
 * Function:  These tables are used to identify valid diacritics and
 *      then match them up with a valid second character.
 *
 *      The shdiac structure contains a list of all valid diacritics.
 *      When a keystroke is received, it is matched against each symb
 *      in the structure to determine if it is a valid diacritic.  If
 *      it is, the index is retrieved.  The index is a ushort which
 *      contains a start index in the top half and a stop index in the
 *      lower half.  These indexes define a range in the shcomp table
 *      to search.
 *
 *      When the second keystroke is received, it is matched against each
 *      compin field of the the shcomp table from the start index to the
 *      stop index retrieved from the shdiac table in the first step.  If
 *      a match is found, the two characters are replaced by the compout
 *      character, which is the graphic for a valid diacritic.
 */


#define LFNUMDIACRITIC  0x56

#include <lftcode.h>
#include <kks.h>                        /* structure definitions */


/*-----------
   FUNCTION: Composition table for dead keys

   This table is set up so that the first entry of each diacritic "range"
   is the space character, which will result in an output of the
   diacritic itself.
------------*/

struct diacritic diactbl88592 = {
        {'8','8','5','9','-','2','\0','\0'},    /*charset encoding*/
        0x56,                                   /* number of entries in table*/
        0,
        {
/*0x00*/ {0xB4, IC_SP , 0xB4},          /* acute */
/*0x01*/ {0xB4, IC_LCE, 0xE9},
/*0x02*/ {0xB4, IC_LCA, 0xE1},
/*0x03*/ {0xB4, IC_LCI, 0xED},
/*0x04*/ {0xB4, IC_LCO, 0xF3},
/*0x05*/ {0xB4, IC_LCU, 0xFA},
/*0x06*/ {0xB4, IC_UCE, 0xC9},
/*0x07*/ {0xB4, IC_UCA, 0xC1},
/*0x08*/ {0xB4, IC_UCI, 0xCD},
/*0x09*/ {0xB4, IC_UCO, 0xD3},
/*0x0A*/ {0xB4, IC_UCU, 0xDA},
/*0x0B*/ {0xB4, IC_LCN, 0xF1},
/*0x0C*/ {0xB4, IC_LCR, 0xE0},
/*0x0D*/ {0xB4, IC_UCN, 0xD1},
/*0x0E*/ {0xB4, IC_UCR, 0xC0},
/*0x0F*/ {0xB4, IC_LCS, 0xB6},
/*0x10*/ {0xB4, IC_LCZ, 0xBC},
/*0x11*/ {0xB4, IC_LCL, 0xE5},
/*0x12*/ {0xB4, IC_LCY, 0xFD},
/*0x13*/ {0xB4, IC_UCS, 0xA6},
/*0x14*/ {0xB4, IC_UCZ, 0xAC},
/*0x15*/ {0xB4, IC_UCL, 0xC5},
/*0x16*/ {0xB4, IC_UCY, 0xDD},

/*0x17*/ {0x5E, IC_SP , 0x5E},          /* circumflex */
/*0x18*/ {0x5E, IC_LCA, 0xE2},
/*0x19*/ {0x5E, IC_LCI, 0xEE},
/*0x1a*/ {0x5E, IC_LCO, 0xF4},
/*0x1b*/ {0x5E, IC_UCA, 0xC2},
/*0x1c*/ {0x5E, IC_UCI, 0xCE},
/*0x1d*/ {0x5E, IC_UCO, 0xD4},

/*0x1e*/ {0xA8, IC_SP , 0xA8},          /* diacresis (umlaut) */
/*0x1f*/ {0xA8, IC_LCU, 0xFC},
/*0x20*/ {0xA8, IC_LCA, 0xE4},
/*0x21*/ {0xA8, IC_LCO, 0xF6},
/*0x22*/ {0xA8, IC_LCE, 0xEB},
/*0x23*/ {0xA8, IC_UCU, 0xDC},
/*0x24*/ {0xA8, IC_UCA, 0xC4},
/*0x25*/ {0xA8, IC_UCO, 0xD6},
/*0x26*/ {0xA8, IC_UCE, 0xCB},

/*0x27*/ {0xB8, IC_SP , 0xB8},          /* cedilla */
/*0x28*/ {0xB8, IC_LCC, 0xC7},
/*0x29*/ {0xB8, IC_LCS, 0xBA},
/*0x2a*/ {0xB8, IC_LCT, 0xFE},
/*0x2b*/ {0xB8, IC_UCT, 0xDE},
/*0x2c*/ {0xB8, IC_UCC, 0xE7},
/*0x2d*/ {0xB8, IC_UCS, 0xAA},

/*0x2e*/ {0xFF, IC_SP , 0xFF},          /* overdot */
/*0x2f*/ {0xFF, IC_LCZ, 0xBF},
/*0x30*/ {0xFF, IC_LCU, 0xF9},
/*0x31*/ {0xFF, IC_UCZ, 0xAF},
/*0x32*/ {0xFF, IC_UCU, 0xD9},

/*0x33*/ {0xA2, IC_SP , 0xA2},          /* breve */
/*0x34*/ {0xA2, IC_LCA, 0xE3},
/*0x35*/ {0xA2, IC_UCA, 0xC3},

/*0x36*/ {0xB0, IC_SP , 0xB0},          /* overcircle */
/*0x37*/ {0xB0, IC_LCU, 0xF9},
/*0x38*/ {0xB0, IC_UCU, 0xD9},

/*0x39*/ {0xB2, IC_SP , 0xB2},          /* ogonek */
/*0x3a*/ {0xB2, IC_LCA, 0xB1},
/*0x3b*/ {0xB2, IC_UCA, 0xA1},
/*0x3c*/ {0xB2, IC_LCE, 0xEA},
/*0x3d*/ {0xB2, IC_UCE, 0xCA},

/*0x3e*/ {0xB7, IC_SP , 0xB7},          /* caron */
/*0x3f*/ {0xB7, IC_LCL, 0xB5},
/*0x40*/ {0xB7, IC_UCL, 0xA5},
/*0x41*/ {0xB7, IC_LCS, 0xB9},
/*0x42*/ {0xB7, IC_UCS, 0xA9},
/*0x43*/ {0xB7, IC_LCT, 0xBB},
/*0x44*/ {0xB7, IC_UCT, 0xAB},
/*0x45*/ {0xB7, IC_LCZ, 0xBE},
/*0x46*/ {0xB7, IC_UCZ, 0xAE},
/*0x47*/ {0xB7, IC_LCC, 0xE8},
/*0x48*/ {0xB7, IC_UCC, 0xC8},
/*0x49*/ {0xB7, IC_LCE, 0xEC},
/*0x4a*/ {0xB7, IC_UCE, 0xCC},
/*0x4b*/ {0xB7, IC_LCD, 0xEF},
/*0x4c*/ {0xB7, IC_UCD, 0xCF},
/*0x4d*/ {0xB7, IC_LCN, 0xF2},
/*0x4e*/ {0xB7, IC_UCN, 0xD2},
/*0x4f*/ {0xB7, IC_LCR, 0xF8},
/*0x50*/ {0xB7, IC_UCR, 0xD8},

/*0x51*/ {0xBD, IC_SP , 0xBD},          /* double acute */
/*0x52*/ {0xBD, IC_LCO, 0xF5},
/*0x53*/ {0xBD, IC_UCO, 0xD5},
/*0x54*/ {0xBD, IC_LCU, 0xFB},
/*0x55*/ {0xBD, IC_UCU, 0xDB},
        }
};
