static char sccsid[] = "@(#)98  1.10  src/bos/usr/sbin/install/installp/inu_mk_opt_fil.c, cmdinstl, bos411, 9428A410j 5/24/94 13:19:11";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_mk_opt_fil (installp)
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

/*--------------------------------------------------------------------------*
**
** NAME: inu_mk_opt_fil ()
**
** FUNCTION: This function uses the sop and next_op pointer to determine
**           which options  in the selected options list to try to process.
**           The status field is what is used to determine if this option
**           will be put into the "lpp.option" file.
**           based on the information in the option structure.
**
** NOTES: INU_TMPDIR is a global variable to represent the working directory
**        that the "lpp.option" will reside in.
**
** RETURNS: SUCCESS - everything went smoothly
**          INUOPEN - The attempt to open (create) the lpp.options file failed.
**
**--------------------------------------------------------------------------*/
int inu_mk_opt_fil (
Option_t *sop,      /* First option for this operation */
Option_t *next_op,  /* First option for the next operation */
int      status,    /* Status flag to indicate what options go in the file */
char     *content)  /* To further indicate what options go in the file, if */
                    /* it's = NIL (char), then any type of content is valid */
{
    int rc;         /* return code                  */
    char lpp_opt_fil[L_tmpnam + 12]; /* path to lpp.options file */
    FILE *fp;       /* file pointer to the lpp.options file */
    Option_t *op;   /* generic Option_t structure pointer */

   /*----------------------------------------------------------------------*
    * Build the path of the lpp.options file. Put it into the temp directory.
    *----------------------------------------------------------------------*/
    strcpy (lpp_opt_fil, INU_TMPDIR);
    strcat (lpp_opt_fil, "/lpp.options");
    if ((fp = fopen (lpp_opt_fil, "w")) == NIL (FILE)) {
	instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
		CMN_CANT_CREATE_D), INU_INSTALLP_CMD, lpp_opt_fil);
        return (INUOPEN);
    }

   /*----------------------------------------------------------------------*
    * Scan the sop, 
    * If the parameter content == NIL (char) then only test on the status
    *    parameter to determine whether to put this option into the file.
    * If the parameter content != NIL (char) then test the status field and
    *    make sure the content field in the op structure (op->content) = to
    *    the parameter content value before allowing this option into the file.
    *----------------------------------------------------------------------*/
    for (op = sop; op != next_op; op = op->next) {
        if ((content == NIL (char)  &&  op->Status == status)  || 
            (content != NIL (char)  && 
            (op->Status == status  &&  op->content == *content))) {
	     if (IF_INSTALL (op->op_type))
             {
                if (op->operation == OP_DEINSTALL  &&
                    IF_MIGRATING (op->op_type))
                    rc = fprintf (fp, "%s MIGRATING\n", op->name);
                else
                    rc = fprintf (fp, "%s\n", op->name);
             }
             else
                 rc = fprintf (fp, "%s %04d %04d %04d %04d\n", 
                         op->name, op->level.sys_ver, op->level.sys_rel, 
	 		 op->level.ver, op->level.rel);

	     if (rc < 0) {
	         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
	 	            CMN_CANT_CREATE_D), INU_INSTALLP_CMD, lpp_opt_fil);
                 return (INUOPEN);
	     }
	}
    }
    fclose (fp);
    return (SUCCESS);

}
