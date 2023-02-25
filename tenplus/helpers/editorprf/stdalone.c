#if !defined(lint)
static char sccsid[] = "@(#)27	1.5  src/tenplus/helpers/editorprf/stdalone.c, tenplus, tenplus411, GOLD410 7/18/91 12:42:04";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Hinit, Hbefore, Hafter, Hmod, Hins, Hdel
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
/* File: standalone.c - stand alone interface for eprofile helper           */
/****************************************************************************/

#include <editorprf.h>

int Hinit (char *filename)
    {
    return (N_uinit(filename));
    }

int Hbefore (register int key, int flag, char *string, int numvalue)
    {
    return( N_ubefore(key, flag, string, numvalue) );
    }

void Hafter (int key)
    {
    N_uafter (key);
    }

/****************************************************************************/
/* Hmod:  calls D_Hmod                                                      */
/****************************************************************************/

/*ARGSUSED*/
int Hmod (char *fieldname, int line, char *oldtext, char *newtext)
/* name of field         */
/* zero based field line */
/* old text for line     */
/* new text for line     */
    {
    return (N_umod (fieldname, line, newtext));
    }

/****************************************************************************/
/* Hins:  calls N_uins                                                      */
/****************************************************************************/

void Hins (char *fieldname, int line, int count)
/* name of field            */
/* zero based field line    */
/* number of lines inserted */
    {
    N_uins (fieldname, line, count);
    }

/****************************************************************************/
/* Hdel:  calls N_udel                                                      */
/****************************************************************************/

/*ARGSUSED*/
void Hdel (char *fieldname, int line, int count)
/* name of field            */
/* zero based field line    */
/* number of lines inserted */
    {
    N_udel (fieldname, line);
    }
