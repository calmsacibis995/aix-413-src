static char sccsid[] = "@(#)04  1.9  src/bos/usr/sbin/install/installp/inu_set_env_vars.c, cmdinstl, bos411, 9428A410j 5/25/94 12:56:54";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_set_env_vars (installp)
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
#include <instl_options.h>

/*-----------------------------------------------------------------------*
 * GLOBALS for the environment variables used by installp
 *-----------------------------------------------------------------------*/
char tmpdir    [100];  /* buffer for INUTEMPDIR env var */
char expandflg [15];   /* buffer for INUEXPAND  env var */
char verifyflg [15];   /* buffer for INUVERIFY  env var */
char saveflg   [15];   /* buffer for INUSAVE    env var */
char *path_ptr;        /* pointer for PATH      env var */

/*--------------------------------------------------------------------------*
**
** NAME: inu_set_env_vars()
**
** FUNCTION: Set the environment variables that can be set upfront.
**           A few env vars have to be set later, INUSAVEDIR and INUTREE
**           because they vary with what options are being worked on.
**
** RETURNS:  void - if an error occurs the error message will be displayed
**                  and we'll return to the calling program.
**
**--------------------------------------------------------------------------*/
int inu_set_env_vars(void)
{
    int     rc;                 /* return code              */
    int     len;                /* length of malloc string  */
    char    *ptr;               /* generic character pointer */
    char    buf[20];            /* temp buffer for PATH var */

   /*-----------------------------------------------------------------------*
    * Set Environment Variable INUTEMPDIR to define tmp directory...
    * (the temp directory has already been create in installp()
    *-----------------------------------------------------------------------*/
    strcpy(tmpdir,"INUTEMPDIR=");
    strcat(tmpdir, INU_TMPDIR);
    if ((rc = inu_putenv(tmpdir)) != SUCCESS)
        return(INUUNKWN);

   /*-----------------------------------------------------------------------*
    * Setup the PATH environment variable to include :.:
    *-----------------------------------------------------------------------*/
    if ((ptr = getenv("PATH")) != NIL(char))
	len = strlen(ptr) + 10;
    else
        len = 10; /* size of "PATH=" and ":.:" + null character */
	
    if ((path_ptr = (char *) malloc(len)) == NIL(char)) {
         (void) instl_msg (SCREEN_LOG, MSGSTR(MS_INUCOMMON, CMN_MALLOC_FAILED_E,
			         CMN_MALLOC_FAILED_D), INU_INSTALLP_CMD);
         return(INUUNKWN);
    }
    strcpy(path_ptr, "PATH=");
    strcat(path_ptr, ptr);
    strcat(path_ptr, ":.:");

    if ((rc = inu_putenv(path_ptr)) != SUCCESS)
	    return(INUUNKWN);

   /*-----------------------------------------------------------------------*
    * Do a putenv to set the environment variable INUEXPAND="<1> or <0>"
    * 1 = Yes, expand is permitted, 0 = No, expand is not permitted.
    *-----------------------------------------------------------------------*/
    strcpy(expandflg, "INUEXPAND=");
    if (X_FLAG)
        strcat(expandflg, YES_STR);
    else
        strcat(expandflg, NO_STR);
    if ((rc = inu_putenv(expandflg)) != SUCCESS)
	return(INUUNKWN);

   /*-----------------------------------------------------------------------*
    * Do a putenv to set the environment variable INUVERIFY="<1> or <0>"
    * 1 = Yes - sysck should do chksums, 0 = No - sysck should'nt do chksums.
    *-----------------------------------------------------------------------*/
    strcpy(verifyflg, "INUVERIFY=");
    if (v_FLAG)
        strcat(verifyflg, YES_STR);
    else
        strcat(verifyflg, NO_STR);
    if ((rc = inu_putenv(verifyflg)) != SUCCESS)
	return(INUUNKWN);

   /*-----------------------------------------------------------------------*
    * Do a putenv to set the environment variable INUSAVE="<1> or <0>"
    * 1 = Yes - save current files, 0 = No - don't save current files.
    *-----------------------------------------------------------------------*/
    strcpy(saveflg, "INUSAVE=");
    if (N_FLAG)
        strcat(saveflg, NO_STR);
    else
        strcat(saveflg, YES_STR);
    if ((rc = inu_putenv(saveflg)) != SUCCESS)
	return(INUUNKWN);
    
    return(SUCCESS);
}
