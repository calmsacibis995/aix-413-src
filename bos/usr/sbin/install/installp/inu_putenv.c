static char sccsid[] = "@(#)99  1.9  src/bos/usr/sbin/install/installp/inu_putenv.c, cmdinstl, bos411, 9428A410j 3/6/94 19:32:16";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_putenv (installp)
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
#include <installp.h>

/*--------------------------------------------------------------------------*
**
** NAME: inu_putenv ()
**
** FUNCTION: put the environment variable into the environment.
**
** RETURNS:  SUCCESS - if no errors occurred.
**           FAILURE - Malloc or the putenv failed.
**
**--------------------------------------------------------------------------*/
int inu_putenv (
char *ptr)      /* the buffer for this environment variable */
{

   /*-----------------------------------------------------------------------*
    * do the putenv on the env passed in.
    *-----------------------------------------------------------------------*/
    if ((putenv (ptr)) != 0) {
        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_SET_ENV_VAR_E, 
		 CMN_SET_ENV_VAR_D), INU_INSTALLP_CMD, ptr);
        return (FAILURE);
    }
    return (SUCCESS);

}
