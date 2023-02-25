static char sccsid[] = "@(#)02  1.7  src/bos/usr/sbin/install/installp/inu_run_sysck.c, cmdinstl, bos411, 9428A410j 6/18/93 16:52:21";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_run_sysck (installp)
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
#include <sys/file.h>
#include <installp.h>

/*--------------------------------------------------------------------------*
**
**
** NAME: run_sysck
**
** FUNCTION:
**  runs sysck for an op_name
**
** EXECUTION ENVIRONMENT: Part of user command.
**
** ON ENTRY:
**  op_name pointer to name of lpp-option to create entries for
**
** RETURNS:
**  SUCCESS     if entries are created correctly
**  FAILURE     if there is a failure
**--------------------------------------------------------------------------*/
int inu_run_sysck(
char *op_name ) /* lpp-option to run sysck on */
{
    int  rc;            /* return code              */
    char str[BUFSIZ];   /* generic string           */

   /*-----------------------------------------------------------------------*
    * Check to make sure there is a inventory file, it could be by the name
    * "inventory" or "lpp-option.inventory".  If an inventory file is found
    *  then call sysck. Return SUCCESS or FAILURE based on rc from sysck.
    *-----------------------------------------------------------------------*/
    if (access("inventory",F_OK) == SUCCESS) {  /* if name is inventory */
        strcpy(str,"/bin/sysck -i -f inventory ");
    }
    else {
        strcpy(str,op_name);
        strcat(str,".inventory");
        if (access(str,F_OK) == SUCCESS) {  /* if op_name.inventory */
            strcpy(str,"/bin/sysck -i -f ");
            strcat(str,op_name);
            strcat(str,".inventory ");
        }
        else
            return(SUCCESS);           /* no inventory file--OK */
    }
    strcat(str,op_name);               /* add op_name         */
    rc = system(str);                  /* run sysck           */
    if(MASK(rc))                       /* mask and shift return code */
        return(FAILURE);               /* so we pretend there was */
    else
        return(SUCCESS);
}
