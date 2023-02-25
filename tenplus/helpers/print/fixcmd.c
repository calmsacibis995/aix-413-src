#if !defined(lint)
static char sccsid[] = "@(#)00	1.6  src/tenplus/helpers/print/fixcmd.c, tenplus, tenplus411, GOLD410 3/23/93 11:58:43";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	PTfixcommand
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
 
/****************************************************************************/
/* File:  fixcmd.c                                                          */
/****************************************************************************/

#include <stdio.h>
#include <limits.h>

#include "chardefs.h"
#include "database.h"
#include "edexterns.h"
#include "libobj.h"
#include "libutil.h"
#include "INprint_msg.h"

/****************************************************************************/
/* PTfixcommand:  prompt for underlined args in string                      */
/* Returns an allocated copy with the prompts replaced by the user          */
/* responses or ERROR if user hit CANCEL                                    */
/*                                                                          */
/* arguments:              char *command - to be fixed                      */
/*                                                                          */
/* return value:           char *  - the fixed versions                     */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Copy the input command into an output buffer until encountering      */
/*     an underlined character.  Copy the underlined section into another   */
/*     buffer, concatenate a field of spaces to it, and use Eask to get     */
/*     the user to provide a string to be substituted for the underlined    */
/*     section in the output buffer.  If the user hit CANCEL, return        */
/*     ERROR.  Otherwise continue this loop until end of command string,    */
/*     null-terminate the output buffer, and return an allocated copy.      */
/*                                                                          */
/****************************************************************************/

char *PTfixcommand (char *command)
    {
    ATTR *string;   /* converted command    */
    ATTR *strp;     /* pointer to string    */
    ATTR *bufp;     /* pointer to buffer    */
    ATTR *cp;       /* pointer to prompt    */
    ATTR *answer;   /* answer to prompt     */

    ATTR buffer [200 * MB_LEN_MAX];       /* new command string   */
    ATTR prompt [50 * MB_LEN_MAX];

    char *tmp_prompt;        /* prompt ext format    */
    char *tmp_answer;        /* answer ext format    */

#ifdef DEBUG
    debug ("PTfixcommand:  command = '%s'", command);
#endif

    string = unpackup (command, 0);
    strp = string;
    bufp = buffer;

    while (*strp)
	{
	if (!(is_underlined (*strp)))    /* if char not underlined */
	    {
	    *bufp++ = *strp++;
	    continue;
	    }

	/* pick out the prompt from the command string */
	cp = prompt;

	while (is_underlined (*strp))

	    /* remove underlining during xfer */
	    *cp++ = de_underline (*strp++);

	*cp = 0;

	/* ask question and put answer into the command string */
	tmp_prompt = (char *)packup (prompt); 
	tmp_answer = Eask (P_EPROMPT, "%s                       ", tmp_prompt);

	if (tmp_answer == (char *) ERROR)
	    {
	    (void)s_free ((char *)string);
	    (void)s_free (tmp_prompt);
	    return ((char *) ERROR);
	    }

#ifdef DEBUG
	debug ("PTfixcommand:  question = '%s', answer = '%s'", tmp_prompt,
		tmp_answer);
#endif
	answer = (ATTR *)unpackup (tmp_answer, 0);

	for (cp = answer; *cp; )
	    *bufp++ = *cp++;

	(void)s_free ((char *)answer);
	(void)s_free (tmp_answer);
	}
    (void)s_free ((char *)string);
    *bufp = 0;
    return ((char *)packup (buffer)); 
    }
