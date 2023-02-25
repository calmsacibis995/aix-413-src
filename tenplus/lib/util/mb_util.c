#if !defined(lint)
static char sccsid[] = "@(#)68	1.4  src/tenplus/lib/util/mb_util.c, tenplus, tenplus411, GOLD410 3/3/94 18:30:37";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:   ismbspace  skipmbspaces  skipmb  blank_trailing_underbars
 *              mbcpy wc_to_mbs ismbalnum
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
 *
 * This file contains a number of functions to handle multibyte chars
 * and multibyte strings.
 */

#include <stdlib.h>
#include <limits.h>

#include "chardefs.h"
#include "database.h"
#include "libutil.h"

/******************************************************

ISMBSPACE

Returns 0 when the multibyte char pointed to by mbp
is not a white space char or when it is an invalid multi
byte code. Returns something not equal
to 0 otherwise.

*******************************************************/

int ismbspace(char *mbp)
{
  wchar_t wc;

  if (MultibyteCodeSet) {
       if (mbtowc(&wc, mbp, MB_CUR_MAX) < 1)
            return(0);
       else return(iswspace(wc));
  }
  else return(isspace(*mbp));
}

/****************************************************

SKIPMBSPACES

*mbp points into a multibyte char string.  *mbp is
advanced until it doesn't point at a white space 
char anymore.

****************************************************/

void skipmbspaces(char **mbp)
{
  wchar_t wc;
  size_t charlen;

  if (MultibyteCodeSet) 
       while (TRUE) {
           charlen = mbtowc(&wc, *mbp, MB_CUR_MAX);
           if (!iswspace(wc))
                return;
	   /* copy at least 1 char */
	   if (charlen < 1) charlen = 1;
           (*mbp) += charlen;
       }
  else while (isspace(**mbp))
                (*mbp)++;
}

/****************************************************

SKIPMB

At invocation, *mbp points at a multibyte character.
Skipmb advances *mbp so that it points at the next mb char.

******************************************************/

void skipmb(char **mbp)
{
  if (MultibyteCodeSet) {
	int charlen;

        charlen = mblen(*mbp, MB_CUR_MAX);
	/* skip at least 1 char */
	if (charlen < 1) charlen = 1;
        (*mbp) += charlen;
  }
  else (*mbp)++;
}

/*****************************************************

BLANK_TRAILING_UNDERBARS

mbs points to the beginning of a multibyte string at invocation.
Blank_trailing_underbars replaces all the trailing underbars in
this string with spaces. So

"abcd_ef_gh___"

is converted to

"abcd_ef_gh   "

*******************************************************/

void blank_trailing_underbars (char *mbs)
{
  char *cp;
  char *first_undb = NULL;

  /* underbar char might be part of other multibyte char. */

  /* First find first underbar that is only followed by
     underbars in the rest of the string. */

     for (cp = mbs; *cp; skipmb(&cp)) {
	 if (*cp == '_') {
	     if (first_undb == NULL)
		  first_undb = cp;
	 }
	 else first_undb = NULL;
     }

     /* now replace all the underbars from first_undb with SPACE */
     if (first_undb != NULL)
	 while (*first_undb) {
	    *first_undb = SPACE;
	    first_undb++;
	 }
}

/******************************************************

ISMBALNUM

Returns 0 when the multibyte char pointed to by mbp
is not an alphanumeric char or when it is an invalid multi
byte code. Returns something not equal
to 0 otherwise.

*******************************************************/

int ismbalnum(char *mbp)
{
  wchar_t wc;

  if (MultibyteCodeSet) {
       if (mbtowc(&wc, mbp, MB_CUR_MAX) < 1)
            return(0);
       else return(iswalnum(wc));
  }
  else return(isalnum(*mbp));
}


/***************************************************

WC_TO_MBS

Converts the wide char passed in wc and copies it to
the multibyte string pointed to by *mbp. Then increments
*mbp so that it points to the byte directly after the
new multibyte char.

***************************************************/

void wc_to_mbs(char **mbp, wchar_t wc)
{
  int charlen = wctomb(*mbp, wc);
  *mbp += charlen;
}

/***************************************************

MBCPY

Copies the multibyte character pointed to by *source to
where *target is pointing. Then advances *source and 
*target to make them point at the byte after the
copied character.

***************************************************/

void mbcpy(char **target, char **source)
{
  if (MultibyteCodeSet) {
      int charlen = max(mblen(*source, MB_CUR_MAX), 1);
      while (charlen--) {
          **target = **source;
          (*target)++;
          (*source)++;
      }       
  }
  else {
      **target = **source;
      (*target)++;
      (*source)++;
  }
}


/**********************************************************

IS_WSPC

Returns TRUE when wc is a space character, FALSE otherwise.
(So it also returns FALSE on things like tabs and newlines.)

**********************************************************/

int is_wspc(wchar_t wc)
{
  return(iswspace(wc) && wc != L'\t' && wc != L'\n' &&
         wc != L'\r' && wc != L'\f' && wc != L'\v');
}


/**********************************************************

IS_WSPC_OR_WTAB

Returns TRUE if wc is a space character or a tab, FALSE otherwise.
(So it also returns FALSE on things like formfeeds and newlines.)

**********************************************************/

int is_wspc_or_wtab(wchar_t wc)
{
  return(iswspace(wc) && wc != L'\n' &&
         wc != L'\r' && wc != L'\f' && wc != L'\v');
}



