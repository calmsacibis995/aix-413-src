#if !defined(lint)
static char sccsid[] = "@(#)70	1.4  src/tenplus/lib/util/attr_util.c, tenplus, tenplus411, GOLD410 3/3/94 16:34:42";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS: isattrspace skipattrspaces skipattr skipattr_idx
 *     backupattr backupattr_idx attrcpy attr_to_wc mb_to_attr
 *     attrlen
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
 
/*
   This file contains a number of functions to handle ATTR chars
   and ATTR strings.
*/


#include <stdlib.h>
#include <limits.h>
#include <chardefs.h>

#include <database.h>
#include <libutil.h>

/******************************************************

ISATTRSPACE

Returns 0 when the ATTR pointed to by mbp
is not a white space char. Returns something not equal
to 0 Otherwise. An underlined space is regarded to
be a white space.

*******************************************************/

int isattrspace(ATTR *mbp)
{

  if (MultibyteCodeSet) {
       wchar_t wc;
       char mb_temp[MB_LEN_MAX];
       int i;

       /* first translate the ATTR into a multibyte char
          (stripping the attributes).
       */
       for (i = 0; i < MB_CUR_MAX; i++)
          mb_temp[i] = (char)(de_attr(*mbp++));      

       (void) mbtowc(&wc, mb_temp, MB_CUR_MAX);
       return(iswspace(wc));
  }
  else return(isspace((char) (de_attr(*mbp))));
}


/****************************************************

SKIPATTRSPACES

*mbp points into an ATTR string.  *mbp is
advanced until it doesn't point at a white space 
char anymore. An underlined space is regarded to
be a white space.

****************************************************/

void skipattrspaces(ATTR **mbp)
{
  if (MultibyteCodeSet)  {
       wchar_t wc;
       char mb_temp[MB_LEN_MAX];
       int i;
       ATTR *mbp2;
       int charlen;

       while (TRUE) {
           /* first translate the ATTR into a multibyte char
              (stripping the attributes).
           */
           mbp2 = *mbp;
           for (i = 0; i < MB_CUR_MAX; i++)
              mb_temp[i] = (char)(de_attr(*mbp2++));      
   
           charlen = mbtowc(&wc, mb_temp, MB_CUR_MAX);
           if (!iswspace(wc))
                return;

	   /* move at least 1 char */
	   if (charlen < 1) charlen = 1;
           (*mbp) += charlen;
       }
  }
  else while ( isspace((char) de_attr(**mbp)))
                (*mbp)++;
}

/****************************************************

SKIPATTR

On invocation, *ap points at an ATTR in an ATTR string.
Skipattr advances *ap until it points at the next ATTR.

*****************************************************/

void skipattr(ATTR **ap)
{
  if (**ap == (ATTR)0)
     return;
  (*ap)++;
  while (!firstbyte(**ap))
     (*ap)++ ; 
} 

/****************************************************

SKIPATTR_IDX

On invocation, as[*ai] is the first ATTR of a potentially
multi ATTR character in an ATTR string.
Skipattr_idx increments *ai until as[*ai] is the first ATTR
of the next character.

*****************************************************/

void skipattr_idx(ATTR *as, int *ai)
{
  if (as[*ai] == (ATTR)0)
     return;
  (*ai)++ ;
  while (!firstbyte(as[*ai]))
     (*ai)++ ; 
} 

/****************************************************

BACKUPATTR

On invocation, *ap points at an ATTR in an ATTR string.
Backupattr decrements *ap until it points at the previous ATTR.

*****************************************************/

void backupattr(ATTR **ap)
{
  (*ap)-- ;
  while (!firstbyte(**ap))
     (*ap)-- ; 
} 

/****************************************************

BACKUPATTR_IDX

On invocation, as[*ai] is the first ATTR of a potentially
multi ATTR character in an ATTR string.
Backupattr_idx decrements *ai until as[*ai] is the first ATTR
of the previous character.

*****************************************************/

void backupattr_idx(ATTR *as, int *ai)
{
  (*ai)-- ;
  while (!firstbyte(as[*ai]))
     (*ai)-- ; 
} 

/*****************************************************

ATTRCPY

Copies the ATTR pointed to by *source to where *target
points to. Then advances *source and *target to make them
point after the last copied character.

******************************************************/

void attrcpy(ATTR **target, ATTR **source)
{
  do {
     **target = **source;
     (*target)++;
     (*source)++;
  } while (!firstbyte(**source));
}

/****************************************************

ATTR_TO_WC

Converts the character pointed to by ap to a 
wide char and returns this wide char.

****************************************************/

wchar_t attr_to_wc(ATTR *ap)
{
  wchar_t wc;
  char mb_temp[MB_LEN_MAX];
  int i;

  /* first translate the ATTR into a multibyte char
     (stripping the attributes).
  */
  for (i = 0; i < MB_CUR_MAX; i++)
     mb_temp[i] = (char)(de_attr(*ap++));      

  (void) mbtowc(&wc, mb_temp, MB_CUR_MAX);
  return(wc);
}

/*****************************************************

MB_TO_ATTR

Copies the multibyte char from *sp to an ATTR char from
*ap, making sure that the FIRST bit is set on the first ATTR
(except when the multibyte char is the null char). When the
function returns, *sp points after the copied multibyte char,
*ap points after the ATTR copy.

Returns TRUE (value other than zero) when the copied char is
not the null char, returns FALSE if the copied char is the 
null char.

******************************************************/

int mb_to_attr(ATTR **ap, char **sp)
{
  int is_null_char = (**sp == '\0');

  if (is_null_char) {
       **ap = **sp;
       (*ap)++;
       (*sp)++;
  }
  else if (MultibyteCodeSet) {
           int char_len = mblen(*sp, MB_CUR_MAX);
	   /* copy at least 1 char */
	   if (char_len < 1) char_len = 1;
           **ap = **sp | FIRST;
           (*ap)++;
           (*sp)++;
           while (--char_len > 0) {
               **ap = **sp;
               (*ap)++;
               (*sp)++;
           }
       }
       else {
               **ap = **sp | FIRST;
               (*ap)++;
               (*sp)++;
       }

  return(!is_null_char);
} 

/****************************************************

ATTRLEN

Returns the length in ATTRs of the character ap points
to.

*****************************************************/

int attrlen(ATTR *ap)
{
  ATTR *ap2 = ap;

  do 
    ap2++ ;
  while (!firstbyte(*ap2));

  return(ap2 - ap);
} 

