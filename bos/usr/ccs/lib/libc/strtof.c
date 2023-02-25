static char sccsid[] = "@(#)51	1.4  src/bos/usr/ccs/lib/libc/strtof.c, libccnv, bos411, 9428A410j 6/16/90 01:05:51";
/*
 * COMPONENT_NAME: LIBCCNV strtof
 *
 * FUNCTIONS: strtof
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: strtof
 *                                                                    
 * FUNCTION: convert string to float
 *                                                                    
 * NOTES:
 *
 *  Strtof is generated by defining  STRTOF and then including
 *  atof.c. Depending on what is defined, atof.c can generate code
 *  for atof, atoff, strtod, or strtof.
 *
 *  See atof.c for details
 *
 * RETURNS: a float value
 */

#define STRTOF 1
#include "atof.c"
