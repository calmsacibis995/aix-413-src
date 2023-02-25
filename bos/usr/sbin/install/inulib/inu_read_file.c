static char sccsid[] = "@(#)71  1.9  src/bos/usr/sbin/install/inulib/inu_read_file.c, cmdinstl, bos411, 9428A410j 3/6/94 19:28:14";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_read_file (inulib)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*-------------------------------------------------------------------------*
 * Includes
 *-------------------------------------------------------------------------*/
/* 
#include "../inudef.h"
#include "../messages/instlmsg.h"
#include "../messages/commonmsg.h"
#include <inuerr.h>
#include <stdio.h>
*/

#include <inulib.h>

#define MEM_BUF_SZ 1024

/*--------------------------------------------------------------------------*
**
** NAME: inu_read_file ()
**
** FUNCTION: Mallocs some space and then reads a file from disk (or tape) and
**           puts it into memory. If we didn't malloc enough the first time, 
**           then we'll realloc some more.
**
** RETURNS:
**
**
**
**--------------------------------------------------------------------------*/
int inu_read_file (
FILE    *fp,       /* The file pointer of the file to read */
char    **mem_buf) /* The address of the char array to put the file into */
{
    int     buf_sz;
    int     i;
    int     c;
   /*-----------------------------------------------------------------------*
    * Malloc the initial memory for the "in memory" file buffer
    *-----------------------------------------------------------------------*/
    *mem_buf = (char *) malloc (MEM_BUF_SZ);
    if (*mem_buf == NIL (char))
    {
        (void) inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
	                            CMN_MALLOC_FAILED_D), INU_INSTALLP_CMD);
	return (FAILURE);
    }
    else
        buf_sz = MEM_BUF_SZ;

   /*-----------------------------------------------------------------------*
    * Read in the entire file into the malloc'ed buffer
    * If we run out of memory realloc some more memory...
    *-----------------------------------------------------------------------*/
    for (i = 0; (c = getc (fp)) != EOF;) 
    {
        *((*mem_buf) + i) = c;
	i++;
        if (i >= buf_sz)
        {
           buf_sz += MEM_BUF_SZ;
           if ((*mem_buf = (char *) realloc (*mem_buf, buf_sz)) == NIL (char))
           {
              inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
	                           CMN_MALLOC_FAILED_D), INU_INSTALLP_CMD);
	      return (FAILURE);
           }
        }
    } /* for (i = 0; (c = getc (fp)) != EOF;) */

   /*-----------------------------------------------------------------------*
    * Null terminate the memory buffer and return success.
    *-----------------------------------------------------------------------*/
    *((*mem_buf) + i) = '\0';
    return (SUCCESS);
}
