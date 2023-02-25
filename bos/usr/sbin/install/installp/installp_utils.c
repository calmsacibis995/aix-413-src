static char sccsid[] = "@(#)68  1.12  src/bos/usr/sbin/install/installp/installp_utils.c, cmdinstl, bos412, 9446C 11/11/94 13:51:44";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_different_level
 *		inu_set_graph_status
 *		inurid_is_running
 *		write_command_line
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

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/

#include <installp.h>
#include <inudef.h>
#include <ckp_common_defs.h>
#include <ckprereqmsg.h>

#define  MAX_NIM_FILE_LINE_LEN  1024

extern fix_info_type * fix_info_lookup (char * name, Level_t * level, 
                                       boolean ignore_fix_id);

/* ------------------------------------------------------------------------ *
**
** Function    inu_different_level
**
** Purpose     Determines if the level of what we're about to apply (op)
**             is different from any other option belonging to the same
**             product that MAY or MAY NOT already be installed.
**
**
** Returns     TRUE  -  if op's level is different from what's installed
**            FALSE  -  if op's level is the same as what's installed or
**                      if nothing with the same product name is already
**                      installed.
**
** ------------------------------------------------------------------------ */

int inu_different_level (Option_t *op)    /* option to check level against */

{
  prod_t  prod;
  int     rc;

  memset (&prod, '\0', sizeof (prod_t));

 /** --------------------------------------------------------------------- *
  **  Get and set the product name we're gonna search on. 
  ** --------------------------------------------------------------------- */

  if (op->prodname[0] != '\0')
  {
     if ((prod.name = strdup (op->prodname)) == NULL)
        perror ("strdup");
  }
  else
     inu_get_prodname (op, prod.name);

 /** --------------------------------------------------------------------- *
  **  Set the state and level of what we're querying for. 
  ** --------------------------------------------------------------------- */

  prod.ver    =  op->level.ver;
  prod.rel    =  op->level.rel;
  prod.mod    =  op->level.mod;
  prod.fix    =  op->level.fix;
  prod.ptf[0] =  '\0';
  prod.update = INU_FALSE;

 /** --------------------------- *
  **  Set update field to FALSE
  ** --------------------------- */

  rc = vpdget (PRODUCT_TABLE, PROD_NAME | PROD_UPDATE | PROD_VER | 
                              PROD_REL | PROD_MOD | PROD_FIX | PROD_PTF, &prod);

  inu_ck_vpd_sys_rc (rc); 

 /** ----------------------------------------------------------------- *
  **  If this VPD record is for an INSTALL image, and it's already
  **  installed, and it's for a different level, then we need to 
  **  return TRUE.
  ** ----------------------------------------------------------------- */

  vpd_free_vchars (PRODUCT_TABLE, &prod);

  if (rc == VPD_OK  && (prod.state == ST_APPLIED || prod.state == ST_COMMITTED))
     return FALSE;

  return TRUE;
}

/*
 * NAME: inu_set_graph_status
 *
 * FUNCTION:  Marks the "apply_state" field of the requisite
 *            graph entries that correspond to the sop entries between 
 *            start_op and next_op (including start_op but NOT next_op).  
 *            The graph node's state is based on the Status field of 
 *            the sop entry.
 *
 *
 * RETURNS: NONE
 */
void inu_set_graph_status (Option_t * start_op, Option_t * next_op)
{
   fix_info_type * fix;
   Option_t      * op;

   for (op = start_op;
        op != next_op;
        op = op->next)
   {
      /* Call the ckprereq table look-up primitive. */

      fix = fix_info_lookup (op->name, &(op->level), TRUE);

      if (fix == NIL (fix_info_type))
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E, 
                    CKP_INT_CODED_MSG_D), inu_cmdname, SOP_ENTRY_NOT_IN_GRAPH);
         inu_quit (INUUNKWN);
      }

      if (op->Status == STAT_SUCCESS)
         fix->apply_state = ALL_PARTS_APPLIED;
      else
         fix->apply_state = FAILED;

   } /* end for */

} /* end inu_set_graph_status */

/* ------------------------------------------------------------------- *
**
**  Function   -   inurid_is_running
**
**  Purpose    -  Determines if the inurid command is running
**
**  Returns    -  TRUE - if the inurid command is running
**               FALSE - if the inurid command is NOT running
**
** ------------------------------------------------------------------- */

int inurid_is_running ()
{
   struct stat sb;   /* stat buffer */

   if ((stat ("/usr/lpp/inurid_LOCK", &sb)) == 0)
      return (TRUE);
   return (FALSE);
}

/*----------------------------------------------------------------------
 * NAME: write_command_line 
 *
 * FUNCTION:  
 *          Writes the command line in the log file.
 *          Splits command line if > 80 chars, appends "\" at the end 
 *          of each line.
 *
 *
 * RETURNS: NONE
 *--------------------------------------------------------------------*/


void
write_command_line (int argc, 
                    char **argv)
{
   char   cmd_line [CMD_LINE_LT+1];
   int i, 
       arg_lt=0,     /* strlen  of this arg */ 
       cmd_lt=0;     /* accumulated lt. of cmd_line */

   *cmd_line='\0';

   /* print full path of command instead of just the basename */
   instl_msg (LOG_MSG, "Command line is:\n");
   for (i=0; i<= argc; i++) {
        if ((cmd_lt + (arg_lt = strlen (argv[i]))) >= CMD_LINE_LT) {
             instl_msg (LOG_MSG, "%s\\\n", cmd_line);
             strcpy (cmd_line, argv[i]);
             cmd_lt = arg_lt;
        }
        else {
              cmd_lt += arg_lt;
              strcat (cmd_line, argv[i]);
              strcat (cmd_line, " ");
              cmd_lt++;
        }
   }

   instl_msg (LOG_MSG, "%s ", cmd_line);
   instl_msg (LOG_MSG, "\n");
}

/*----------------------------------------------------------------------
 *
 *  Function      NIM_usr_spot
 *
 *  Purpose       Determines if /usr filesystem is a NIM SPOT or not
 * 
 *  Returns       >NULL - ptr to string which contains SPOT's name
 *               NULL   - /usr filesystem is not a SPOT
 *
 *--------------------------------------------------------------------*/

#define AWK_SCRIPT      "/usr/bin/awk\
                            '/NIM_USR_SPOT/{ gsub(/.*=/,\"\");\
                                             gsub(/\"/,\"\");\
                                             print }' /etc/niminfo 2>/dev/null"
#define MAX_SPOT_NAME   80

char *NIM_usr_spot ()
{
   FILE *fp;
   char tmp[MAX_SPOT_NAME];
   char *name = NULL;

   /* open a pipe to awk */
   if ( (fp = popen( AWK_SCRIPT, "r" )) != NULL )
   {
      /* read the SPOT's name */
      tmp[0] = '\0';
      fgets( tmp, MAX_SPOT_NAME, fp );

      /* if the SPOT name was returned from awk... */
      if ( tmp[0] != '\0' )
      {
         /* malloc enough space for the name */
         name = malloc( strlen( tmp ) + 1 );

         /* copy the SPOT's name into the buffer */
         strcpy( name, tmp );
      }

      fclose( fp );
   }

   /* return name */
   return( name );

}
