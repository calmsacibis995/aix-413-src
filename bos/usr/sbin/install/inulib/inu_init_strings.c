static char sccsid[] = "@(#)51  1.17  src/bos/usr/sbin/install/inulib/inu_init_strings.c, cmdinstl, bos411, 9428A410j 1/4/94 15:26:02";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_init_strings
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/

#include <installp.h>

/*--------------------------------------------------------------------------*
 * The following table is the default messages for the MS_STRING message set
 * in cmdinstl_e.msg file. The defines in the msg file must be = to the index
 * into this table. At the beginning of the function, each of the strings in
 * this table will be passed to the catgets() function which will return
 * either the translated string or the default string (seen in this table).
 * This table will be then filled in with the string received from catgets().
 * And access into this table will be via the #define specified in the msg
 * file. The only exceptions are: (1)the first entry which is defined above
 * "BLANK_STR" because the msg file starts at "1" and the table starts at "0".
 * (2) The last entry, which uses the string_len variable for its index is set
 * to the last entry in the table.  This is where the PTF id string will be
 * placed when the option is an 3.2 update.
 *--------------------------------------------------------------------------*/

char  *string[] = {
                    "       ",     /* index = "#define BLANK_STR" above */ 
                 /* The following indices are defined in cmdinstl_e.msg. */
                    "UNKNOWN",     /* index = UNKNOWN_STR        */
                    "3.1updt",     /* index = UPDT_3_1_STR       */
                    "USR    ",     /* index = USR_STR            */
                    "ROOT   ",     /* index = ROOT_STR           */
                    "SHARE  ",     /* index = SHARE_STR          */
                    "usr     ",    /* index = USR2_STR           */
                    "root    ",    /* index = ROOT2_STR          */
                    "share   ",    /* index = SHARE2_STR         */
                    "usr,root",    /* index = BOTH_STR           */
                    "APPLY      ", /* index = APPLY_STR          */
                    "APPLYING   ", /* index = APPLYING_STR       */
                    "APPLIED    ", /* index = APPLIED_STR        */
                    "COMMIT     ", /* index = COMMIT_STR         */
                    "COMMITTING ", /* index = COMMITTING_STR     */
                    "COMMITTED  ", /* index = COMMITTED_STR      */
                    "REJECT     ", /* index = REJECT_STR         */
                    "REJECTING  ", /* index = REJECTING_STR      */
                    "CLEANUP    ", /* index = CLEANUP_STR        */
                    "DEINSTALL  ", /* index = DEINSTALL_STR      */
                    "DEINSTALLING",/* index = DEINSTALLING_STR   */
                    "CANCELLED  ", /* index = CANCELLED_STR      */
                    "AVAILABLE  ", /* index = AVAILABLE_STR      */
                    "BROKEN     ", /* index = COMMITTED_STR      */
                    "SUCCESS    ", /* index = SUCCESS_STR        */
                    "FAILED     ", /* index = FAILURE_STR        */
                    "       "      /* index = string_len which is
                                    * a variable calculated based
                                    * on the size of this array
                                    * "string[]"                 */
};

/* Make sure that NUMBER_OF_ENTRIES reflects the number of entries in the
   array string. */

#define NUMBER_OF_ENTRIES (FAILURE_STR + 2)

/**
 **  GLOBAL
 **/

int             string_len;     /* used as an index into the string[] for the
                                 * last entry */

/*--------------------------------------------------------------------------*
**
** NAME: inu_init_strings
**
** FUNCTION:   Generates a summary report of what was installed, it also will
**             generate a status file in /tmp so instclient will know what
**             failed.
**
**
** RETURNS:    void
**
**--------------------------------------------------------------------------*/

void inu_init_strings (void)

{
   char  * ptr;
   int     i;
   int     len;
   int     k;

   /*-----------------------------------------------------------------------*
    * Initialize the string array. We call catgets to get the translated
    * string and put it into our static array. If it doesn't find it, then
    * the default string will be used.
    *-----------------------------------------------------------------------*/

   k = NUMBER_OF_ENTRIES - 1;
   string_len = k;      /** A global var **/

   for (i = 0; i <= k; i++)
   {
      ptr = catgets (catd, MS_STRINGS, i, string[i]);
      string[i] = strdup (ptr);
   }

} /* end inu_init_strings */
