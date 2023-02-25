#if !defined(lint)
static char sccsid[] = "@(#)62	1.9  src/tenplus/e2/common/intran.c, tenplus, tenplus411, 9437C411a 9/16/94 12:20:18";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:   nextin, command_seq
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* File: intran.c - terminal input translation                              */
/****************************************************************************/

/* application include files */
#include "def.h"
#include "keynames.h"


/* external definitions */
extern int           g_usechar;   /* TRUE if we should recycle g_keyvalue */
extern unsigned char *g_intran;   /* the single character table           */
extern POINTER       *g_multseq;  /* the sequence table                   */
extern char          *g_multcanon;/* parallel table of canonicals         */


/* values for input table slots

      SELF says this is a text character
      SEQ  says this is the first character of a command sequence
      BAD  says this is an invalid keystroke */

#define SELF 0xfd
#define SEQ  0xfe
#define BAD  0xff

static void command_seq(unsigned char *);

/*****************************************************************************/
/* nextin: get the next (g_keyvalue, g_textvalue) pair                       */
/*                                                                           */
/* arguments:          none                                                  */
/*                                                                           */
/* return value:       void                                                  */
/*                                                                           */
/* globals referenced: g_keyvalue, g_textvalue, g_usechar,                   */
/*                     g_intran, g_multseq, g_multcanon                      */
/*                                                                           */
/* globals changed:    g_keyvalue, g_textvalue, g_usechar                    */
/*                                                                           */
/* synopsis:                                                                 */
/*     nextin is responsible for getting the next input character and        */
/*     setting the editor state globals appropriately.                       */
/*                                                                           */
/*     If the g_usechar global is set, simply turn it off and return,        */
/*     thus re-using the existing g_keyvalue et al.                          */
/*                                                                           */
/*     Get the next character by invoking the nextchar() procedure.          */
/*     If the character starts a multi-character sequence, read and          */
/*     process the rest of the sequence, interpreting it by reference        */
/*     to the g_multseq and g_multcanon global tables.                       */
/*                                                                           */
/*     Finally, ensure that the various g_* editor state globals are         */
/*     set properly.  The globals updated are set as follows:                */
/*                                                                           */
/*         g_keyvalue is the keyvalue code for the input character           */
/*                     (as defined in keynames.h)                            */
/*         g_textvalue is the actual text character in the case where        */
/*                     g_keyvalue is TEXTKEY                                 */
/*****************************************************************************/

void nextin (void)
{

    unsigned char chr[MB_LEN_MAX+1];    /* input character as a string */
    int nbytes;                       /* number of bytes in 'chr' */

    wchar_t wc;                       /* 'chr' in wide character form */

    /* should the last character be reused? */
    if (g_usechar)
	{
	g_usechar = FALSE;
	return;
	}

    /* Get the next character. It may be a sequence of multibytes. An
       invalid multibyte sequence returns a -1 value. */

    nbytes = nextchar(chr);
    if (nbytes == -1)
	{
	g_keyvalue = BADKEY;
	return;
	}

    switch (g_intran [chr[0]])
	{
	case BAD:
	case SELF:
	    if (MultibyteCodeSet)
		{
		nbytes = mbtowc(&wc, (char *) chr, (size_t)MB_CUR_MAX);
		if (nbytes == -1 || iswcntrl(wc))
		    {
		    g_keyvalue = BADKEY;
		    break;
		    }
		}
	    else
		{
		if (iscntrl((int) chr[0]))
		    {
		    g_keyvalue = BADKEY;
		    break;
		    }
		}
	    /* valid TEXT key */
	    g_keyvalue = TEXT_KEY;
	    strcpy(g_textvalue, chr);
	    break;

	/* process multi keystroke commands */
	case SEQ:
	    command_seq(chr);
	    break;

	default:
	    g_keyvalue = g_intran[chr[0]];
	    break;
	}
    return;
}


    /* Handle multiple command sequences. Each part of a command
       is assumed to be a single byte */

void command_seq(unsigned char *chr)
{

    int top = 0;
    int bottom = obj_count (g_multseq) - 1;
    int length = 1;
    int nbytes, i, comp;

    unsigned char *icp;         /* pointer into current sequence */
    unsigned char inseq [20];   /* multi keystroke command buffer */

    icp = inseq;
    *icp++ = *chr;

    /* binary search the multseq command table */
    while (top <= bottom)
	{
	i = (top + bottom) / 2;
	comp = strncmp ((char *)inseq, (char *)g_multseq [i],
				       (size_t)length);

	while (comp == 0)
	    {
	    if (length == obj_count (g_multseq [i]))
		{
		g_keyvalue = g_multcanon [i];
		return;
		}

	    nbytes = nextchar(icp++);
	    if (nbytes == -1)
		{
		g_keyvalue = BADKEY;
		return;
		}

	    length++;
	    comp = strncmp ((char *)inseq, (char *)g_multseq [i],
					   (size_t)length);
	    }

	if (comp < 0)
	    bottom = i - 1;
	else if (comp > 0)
	    top = i + 1;
	}

    /* if we fall out of binary search, user hit something wrong        */
    g_keyvalue = BADKEY;
    return;
}

