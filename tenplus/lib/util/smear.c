/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)84	1.5  src/tenplus/lib/util/smear.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:48";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	smear
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* File:  smear.c                                                           */
/****************************************************************************/

/****************************************************************************/
/* smear:  put a character into an array count times                        */
/*                                                                          */
/* arguments:              char  c       - character to be smeared          */
/*                         int   count   - number of times to smear it      */
/*                         char *pointer - where to start                   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     smear simply writes the smear character c into the character array   */
/*     pointer count times.  It does no error checking.                     */
/****************************************************************************/

void smear (register char c, register int count, register char *pointer)
    /*c;        character to smear  */
    /*count;    number to smear     */
    /*pointer;  place to smear char */
    {
    while (count--) *pointer++ = c;
    }

