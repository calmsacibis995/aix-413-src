/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: replace_quotes
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

#include <stdio.h>
char *
replace_quotes (char *src, char *rep, FILE *fp)
{
   char *target="",
        *str,
        *quote="\"";

   short beg=FALSE,
         end=FALSE;

   /* src begins with q quote */
   if (src[0] == '"')
       beg=TRUE;

   /* src begins with q quote */
   if (src[strlen(src)-1] == '"')
       end=TRUE;

   str=(char *) strtok(src, quote);

   if (str == (char *)0){ 
     fprintf (fp,"%s", src);
     return (target);
   }
  

   if (beg)
     fprintf (fp,"%s", rep);


   fprintf (fp,"%s", str);
   while ((str=(char *) strtok((char *)0, quote)) != (char *)0 ) 
          fprintf (fp,"%s%s", rep, str);

   if (end)
     fprintf (fp,"%s", rep);

   return (target);

}
