/*NOTUSED*/
/* @(#)82	1.6  src/tenplus/include/DT.h, tenplus, tenplus411, GOLD410 3/23/93 12:01:30 */

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
 
#include <time.h>


/* NOTE: the value of DTundefined must be a negative number */
#define DTundefined (-1)

#define JANUARY   (0)
#define FEBRUARY  (1)
#define MARCH     (2)
#define APRIL     (3)
#define MAY       (4)
#define JUNE      (5)
#define JULY      (6)
#define AUGUST    (7)
#define SEPTEMBER (8)
#define OCTOBER   (9)
#define NOVEMBER  (10)
#define DECEMBER  (11)

/*
Prototypes of all functions defined in hlib/DT
*/
 
 int DTmlength(int, int);
 int DTylength(int);
 int DTfind_entry (char *, char *[][2], int);
 void DTinit (void);
