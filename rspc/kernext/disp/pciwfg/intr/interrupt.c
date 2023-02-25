static char sccsid[] = "@(#)40  1.1  src/rspc/kernext/disp/pciwfg/intr/interrupt.c, pciwfg, rspc411, 9440A411d 10/5/94 00:46:36";
/* interrupt.c */
/*
 *
 *   COMPONENT_NAME: (PCIWFG) WD90C24A2 PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: none
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

/*--------------------------------------------------------------------
 |
 | IDENTIFICATION:  WFG_INTR
 |
 | DESCRIPTIVE NAME:  WFG_INTR - Interrupt handler for Frame Buffer
 |
 | FUNCTIONS:   wfg_intr
 |
 | INPUTS:  Interrupt structure pointer - same as phys display pointer
 |
 | OUTPUTS:  None.
 |
 | CALLED BY:  First level interrupt handler
 |
 | passed in call back functions for dma/event handling
 |
 |----------------------------------------------------------------------*/
#include "wfg_INCLUDES.h"

int
wfg_intr (intdat)
        struct intr    *intdat;
{
}
