static char sccsid[] = "@(#)89  1.1  src/bos/usr/sbin/install/inulib/file_query.c, cmdinstl, bos411, 9428A410j 12/13/93 16:26:48";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: path_exists
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <inulib.h>

/*
* NAME: path_exists
*
* FUNCTION:
*      Checks if the path exists
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* Error Condition:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*     parameters:
*                path -- path of the file 
*     global:
*        None
*
* RETURNS: (boolean)
*         TRUE
*         FALSE
*
* OUTPUT:
*
*--------------------------------------------------------------------------*/


boolean
path_exists (char *path)

{
   struct stat st_buf;          /* stat buffer */

   if (*path == '\0')   /* Null string */
       return (FALSE);

   /* check if the path exists */
   if (stat(path, &st_buf) == 0) 
       return (TRUE);  
   else 
       return (FALSE);
}

