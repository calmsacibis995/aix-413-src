/* @(#)65	1.6  src/tenplus/tools/fc/box.h, tenplus, tenplus411, GOLD410 3/23/93 12:17:37 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/*****************************************************************************
* File: box.h - defines the box structure used by the new forms compiler     *
*****************************************************************************/

/* this is my representation of field boxes within the invariant text.
   Independent of the output representation, so most of the algorithm
   can be re-used if we change the output format                        */

struct box
    {
    int top, bot;       /* zero origined coords of this box             */
    int left, right;
    struct box *gangp;  /* pointer to next element of gang              */
    int bits;           /* INDEXED, HGANGED, SHOWTABS, NAMEFIELD        */
			/* definitions for the bits structure member    */
			/* come from DEF.H                              */
    char *prefix;       /* data path                                    */
    char *suffix;       /*   "   "                                      */
    char *tablin;       /* tab line                                     */
    char *pickname;     /* pick-name for this kind of data              */
    int lmarg, rmarg;   /* margins, -1 means none set                   */
    char *zoom;         /* zoom form name, or NULL if no zoom           */
    int msgno;	        /* mnemonic for message catalog                 */
    char *deftxt;       /* default text for that message                */
    int isboxed;        /* says whether field is inside of graphics box */
    };

typedef struct box BOX;

extern int T_BOX;       /* DTYPES type for boxes                        */
