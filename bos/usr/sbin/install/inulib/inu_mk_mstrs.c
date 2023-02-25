static char sccsid[] = "@(#)17  1.15  src/bos/usr/sbin/install/inulib/inu_mk_mstrs.c, cmdinstl, bos411, 9428A410j 3/6/94 19:28:02";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_mk_mstr_al, inu_mk_mstr_acf
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */ 
 
/* -----------------------------------------------------------------------
 *
 * NAME - inu_mk_mstr_al      and      inu_mk_mstr_acf 
 *
 * FUNCTION -			inu_mk_mstr_al  -  gets passed an apply list
 *			to sort and eliminate duplicates from.  It also gets
 *			passed a fullpath name of where to store the result-
 *			ing sorted apply_list. This routine sorts on the 1ST
 *			field of the apply list.
 *
 *				inu_mk_mstr_al  -  gets passed an archive
 *			control file to sort and eliminate duplicates from.
 *			It also gets passed a fullpath name of where to store
 *			the resulting sorted archive control file.  This 
 *			routine sorts on the 2ND field of the acf.
 *
 * RETURNS -			Both functions return INUGOOD -- which is
 *			0 -- on success or the return code of the system 
 *			call to the sort command on failure. 
 *
 * NOTES -			These functions are part of "common code" 
 *			used by inurest.c, inucp.c, and inusave.c.
 *
 * ----------------------------------------------------------------------- */




/* ---------------------------------------------------------------------
                       I  N  C  L  U  D  E  S 
   --------------------------------------------------------------------- */

#include <inulib.h>

/*
#include <stdio.h>
#include <inuerr.h>
#include <limits.h>
#include "../messages/instlmsg.h"
#include "../messages/commonmsg.h"
#include "../inudef.h"
*/


extern char *inu_cmdname;  	/* Name of command calling these routines */


/* ----------------------------------------------------------------------
     F  U  N  C  T  I  O  N        D  E  F  I  N  I  T  I  O  N  S
   ---------------------------------------------------------------------- */

int inu_mk_mstr_al (apply_list, sorted_al)

char *apply_list;        /* full path name of an apply list to be sorted    */
char *sorted_al;         /* full path name of the sorted apply list, which  */
                         /* does not exist until after this function call.  */
{
 char cmd    [STRING_LEN];               /* string used for system command  */
 int rc;                                 /* return code for the system call */

 sprintf (cmd, "/bin/sort -u -o %s %s", sorted_al, apply_list);

 rc = inu_do_exec (cmd);               /* execute the command */

 if (rc != 0)                          /* then sort was unsuccessful */
 {
    inu_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_NOSORT_E, CMN_NOSORT_D), 
                         inu_cmdname, apply_list);
    strcpy (sorted_al, apply_list);
    return rc;                         /* return code from the system call  */
 }

 return INUGOOD;                       /* All went well, if get here */
} /* of inu_mk_mstr_al */

/* ----------------------------------------------------------------------- */
int inu_mk_mstr_acf (acf, sorted_acf)

char * acf;               /* full path name of archive control file          */
char * sorted_acf;        /* full path name of sorted acf in the temp dir,   */
			  /* which doesn't exist until after this func call. */
{
   char cmd [STRING_LEN]; /* string used to construct the system call in */
   int rc;                /* return code from the system call */

   /****************************************************
    ** sort command syntax:
    ** +1  ==> skip first field (sort on second field)
    ** +0  ==> use the first field in the sort of the 
    **         second field (as a secondary sort field)
    ** -u  ==> remove duplicates
    ** -o  ==> specify output file to write results to
    ****************************************************/ 

    sprintf (cmd, "/bin/sort +1 +0 -u -o %s %s", sorted_acf, acf);  
    /* sort on second field (the lib field) */

    rc = inu_do_exec (cmd);   /* execute the sort command */
 
    if (rc != 0)              /* then the system call failed */
    {
       inu_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, CMN_NOSORT_E, CMN_NOSORT_D), 
                                    inu_cmdname, acf);
       strcpy (sorted_acf, acf);
       return rc;             /* return code from failed sort command */
 }

 return INUGOOD;              /* ALL went well if get here */
}

