static char sccsid[] = "@(#)91  1.20  src/bos/usr/sbin/install/installp/inu_summary_log.c, cmdinstl, bos41J, 9510A_all 2/27/95 12:14:31";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_add_to_appropriate_sop
 *		inu_final_summary_log
 *		inu_get_next_parsed_value
 *		inu_inter_summary_log
 *		inu_map_Status_to_file_value
 *		inu_rm_sum_log
 *		inu_set_sum_log_file_values
 *		inu_set_summary_log_path
 *		inu_write_installp_summary_record
 *		read_and_merge_sum_log
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/audit.h>
#include <installp.h>
#include <instl_define.h>
#include <instl_options.h>
#include <inuerr.h>


#define DONT_PRINT_STATE  0     /* an invalid VPD state */
#define INU_EMPTY_PART   ' '
#define ISA_PIFW_CODE(code) \
        (code > IS_INST_SUCCESS  &&  code < IS_INST_FAILURE) ? TRUE : FALSE


#ifdef INUDEBUG
 extern FILE    *dfp;
#endif  /* INUDEBUG */

extern int   string_len;
extern int   need_to_reexec;
extern char  *inu_cmdname;

static void inu_get_next_parsed_value      (char **, char **, char);
static void inu_set_sum_log_file_values    (Option_t *, int *, int *, char *, 
                                                        char *);
static void inu_add_to_appropriate_sop     (Option_t *, Option_t *, Option_t *);
static int  inu_set_summary_log_path       (char *);
static int  inu_write_installp_summary_record 
                                           (FILE *, int, Option_t *, int,  
                                            char, char *);
static int inu_map_Status_to_file_value    (int);
static int needs_an_installp_summary_file_entry (Option_t *op);



/*--------------------------------------------------------------------------*
**
** Function Name: inu_final_summary_log
**
** Purpose:       Writes out all items from the failure sop and all 
**                non-cancelled items from the sop to the summary log.
**
**                The summary log file gets overwritten each call to this
**                routine.
**
** Returns:       INUGOOD -  On success
**                FAILURE -  if couldn't write to the summary log file
**                         
**--------------------------------------------------------------------------*/
int inu_final_summary_log (Option_t *failsop, Option_t *sop)
{
   Option_t *op;              /* temp traverse ptr                         */
   FILE *fp;                  /* file ptr to the summary log file          */
   char file  [PATH_MAX];     /* full path to the summary log file         */
   char level [PATH_MAX];     /* level string containing level of a sop op */
   int  ccode;                /* completion code for the sop op            */
   int  state;                /* final state of the sop op                 */
   char part;                 /* part of the sop op                        */
   int  rc=INUGOOD;           /* function return code                      */

  /** --------------------------------------------------------- *
   **   Clean up the intermediate summary log file we've used.
   ** --------------------------------------------------------- */

   inu_rm_sum_log (INTERMED_SUM_LOG_FILE);

  /** ------------------------------------------------------- *
   **  Set the path to the final summary log file and open 
   **  it.  If problems give a message and return FAILURE.
   ** ------------------------------------------------------- */

   if ((inu_set_summary_log_path (file)) != INUGOOD   
                           || 
      ((fp= (fopen (file, "w+"))) == NIL (FILE)))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_FAIL_CR_SUM_LOG_E, 
                              INP_FAIL_CR_SUM_LOG_D), file);
      instl_msg (FAIL_MSG, "\n");
      return (FAILURE);
   }

  /** ------------------------------------------------------- *
   **  Dump the failure sop into the final summary log file.
   ** ------------------------------------------------------- */

   for (op=failsop->next;
        op != NIL (Option_t)   &&  rc == INUGOOD;
        op=op->next)
   {
      if (needs_an_installp_summary_file_entry (op))
      {
         inu_set_sum_log_file_values (op, &ccode, &state, &part, level);

         rc = inu_write_installp_summary_record (fp,ccode,op,state,part,level);
      }
   }

  /** ------------------------------------------------------- *
   **  Dump the sop into the final summary log file.
   ** ------------------------------------------------------- */

   for (op=sop->next;
        op != NIL (Option_t)  &&  rc==INUGOOD;
        op=op->next)
   {
      if (needs_an_installp_summary_file_entry (op))
      {
         inu_set_sum_log_file_values (op, &ccode, &state, &part, level);

         rc = inu_write_installp_summary_record (fp,ccode,op,state,part,level);
      }
   }

   fclose (fp);

   if (rc != INUGOOD)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_FAIL_CR_SUM_LOG_E, 
                              INP_FAIL_CR_SUM_LOG_D), file);
      rc = FAILURE;
   }

   return rc;
}

/*--------------------------------------------------------------------------*
**
**  Function:   inu_write_installp_summary_record 
** 
**  Purpose:    Writes out a record (one line) to the installp.summary
**              file. 
**
**  Returns:    Return code from fprintf (0 if successful nonzero if not).
** 
**--------------------------------------------------------------------------*/

static int inu_write_installp_summary_record 
  (FILE     *fp, 
   int       ccode, 
   Option_t *op, 
   int       state, 
   char      part, 
   char     *level)

{
   int rc=0;

  /** ---------------------------------------------------------------------- *
   **  Format of each line is: [value][delimiter][value][delimiter]...
   **  Empty values result in [delimiter][delimiter].  
   **  Example:  "val1::val3:val4:", where ":" is the delimiter.
   ** ---------------------------------------------------------------------- */

   rc = fprintf (fp, "%d%s%s%s", ccode,  INSTALLP_SUMMARY_DELIMITER, 
                                 op->name, INSTALLP_SUMMARY_DELIMITER);

   if (state != DONT_PRINT_STATE)
      rc |= fprintf (fp, "%d", state);
   rc |= fprintf (fp, "%s",  INSTALLP_SUMMARY_DELIMITER);

  /** --------------------------------------------------------------- *
   **  We don't ever wanna print a part value for pre-installalation 
   **  failures or warnings.
   ** --------------------------------------------------------------- */

   if (part != INU_EMPTY_PART  &&  ! ISA_PIFW_CODE (ccode))
      rc |= fprintf (fp, "%c", part);
   rc |= fprintf (fp, "%s",  INSTALLP_SUMMARY_DELIMITER);

   if (level[0] != '\0')
      rc |= fprintf (fp, "%s", level);
   rc |= fprintf (fp, "%s",  INSTALLP_SUMMARY_DELIMITER);

   rc |= fprintf (fp, "\n");
 
  /** ------------------------------------------------- *
   ** if fprintf failed writing, then return an error.
   ** ------------------------------------------------- */

   if (rc < 0)
      rc = FAILURE;

   return INUGOOD;
}

/*--------------------------------------------------------------------------*
**
** Function Name: inu_set_summary_log_path
**
** Purpose:       Sets the appropriate path for the final summary log file. 
**                
** Parameters:    file  -  target string to copy path into.
**
** Returns:       INUGOOD -  if all was successful
**                FAILURE -  if couldn't make the directory in which the 
**                           log file is supposed to reside in.
**                         
**--------------------------------------------------------------------------*/

static int inu_set_summary_log_path (char *file)
{
   if ((inu_mk_dir (INSTALLP_SUMMARY_DIR))  !=  INUGOOD)
      return FAILURE;

   strcpy (file, INSTALLP_SUMMARY_DIR);
   strcat (file, "/"); 
   strcat (file, INSTALLP_SUMMARY_FILE); 
    
   return INUGOOD;
}

/*--------------------------------------------------------------------------*
**
** Function Name: inu_inter_summary_log
**
** Purpose:       Writes out all non-cancelled items from the sop to the 
**                intermediate summary log.
**
**                If the file doesn't exist, this routine creates it.  If it
**                does already exist, then this routine appends to it.
**              
** Returns:       INUGOOD -  if could successfully append to log file 
**                FAILURE -  if failed creating intermediate summary log file
**                         
**--------------------------------------------------------------------------*/


int inu_inter_summary_log 
 (
  Option_t *failsop,     /* the fail sop linked list   */
  Option_t *sop          /* the normal sop linked list */
)
{
   Option_t *top;        /* tmp option ptr for traversal      */
   FILE     *fp;         /* file ptr to intermediate log file */
   char     part;        /* op's usr, root, or share part      */ 
   char     level [100]; /* op's level                        */
   int      rc=INUGOOD;  /* return code */


  /** -------------------------------------------------------------------- *
   **  If this is the final time we need to write the summary log out, 
   **  then we have to recreate the whole thing.  We've just read the
   **  log file in and merged it into the Sop.  Part of the Sop is already
   **  in the log file.  So, in order to not duplicate these options, we
   **  will recreate the whole log file. 
   ** -------------------------------------------------------------------- */

   if ((fp=fopen (INTERMED_SUM_LOG_FILE, "a+")) == NIL (FILE))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                        CMN_CANT_CREATE_D), inu_cmdname, INTERMED_SUM_LOG_FILE);
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INCMPLT_SUMMARY_E, 
                                                INP_INCMPLT_SUMMARY_D));
      return FAILURE;
   }

  /** ----------------------------------------------------------------------- *
   **  Write everything on the failsop to the intermediate summary log file. 
   ** ----------------------------------------------------------------------- */

   for (top=failsop->next;
        top != NIL (Option_t)  &&  rc==INUGOOD;
        top=top->next)
   {
      if (top->Status != STAT_CANCEL)
      {
         inu_set_sum_log_file_values (top, NIL (int), NIL (int), &part, level);

         if (fprintf (fp, "%s:%s:%c:%d:%d:%c:%d:\n", 
                      top->name, level, part, top->operation, top->op_type,  
                      top->content, top->Status)  < 0)
         {
            rc = FAILURE;
         }
      }
   }

  /** -------------------------------------------------------------------- *
   **  Write everything on the sop to the intermediate summary log file. 
   ** -------------------------------------------------------------------- */

   for (top=sop->next;
        top != NIL (Option_t)  &&  rc==INUGOOD;
        top=top->next)
   {
      if (top->Status != STAT_CANCEL)
      {
         inu_set_sum_log_file_values (top, NIL (int), NIL (int), &part, level);

         if (fprintf (fp, "%s:%s:%c:%d:%d:%c:%d:\n", 
                      top->name, level, part, top->operation, top->op_type,  
                      top->content, top->Status)  < 0)
         {
            rc = FAILURE;
         }
      }
   }

   fclose (fp);

   if (rc != INUGOOD)
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CREATE_E, 
                        CMN_CANT_CREATE_D), inu_cmdname, INTERMED_SUM_LOG_FILE);
   return rc;
}

/*--------------------------------------------------------------------------*
**
** Function Name: inu_rm_sum_log
**
** Purpose:       Simply removes the summary log file, so we
**                can create and write to a new one.
**              
** Returns:       None 
**                         
**--------------------------------------------------------------------------*/
void inu_rm_sum_log (char *file)
{
   unlink (file);
}

/*--------------------------------------------------------------------------*
**
** Function Name:  read_and_merge_sum_log
**
** Purpose:        Reads the intermediate summary log file -- which is created
**                 on re-exec of installp -- and merges this info into the
**                 sops passed in.  This includes both the fail sop and the
**                 non-fail sop.
**              
** Returns:        INUGOOD -  
**                 FAILURE - 
**                         
**--------------------------------------------------------------------------*/
int read_and_merge_sum_log 
 (
  Option_t *failsop,   /* list to add pif entries from log file to */
  Option_t *sop,       /* list to add non-pif failures to          */
  char *file           /* the intermediate summary log file        */
)

{
   Option_t *op;
   FILE     *fp;
   Level_t  level;
   char     *name, *operationstr, *part, *levelstr;
   char     line[PATH_MAX];
   char     *p;
   char     *op_typestr;
   char     *contentstr;
   char     *Statusstr;
   char     content;
   int      operation;
   int      op_type;
   int      Status;
   

   if ((fp=fopen (file, "r+")) == NIL (FILE))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                           CMN_CANT_OPEN_D), inu_cmdname, file);
      return FAILURE;
   }

   while ((fgets (line, PATH_MAX, fp)) != NIL (char))
   {
      name = levelstr  = part = operationstr = op_typestr = contentstr 
           = Statusstr = NIL (char);

      p = line;

      inu_get_next_parsed_value (&p, &name,          ':');
      inu_get_next_parsed_value (&p, &levelstr,      ':');
      inu_get_next_parsed_value (&p, &part,          ':');
      inu_get_next_parsed_value (&p, &operationstr,  ':');
      inu_get_next_parsed_value (&p, &op_typestr,    ':');
      inu_get_next_parsed_value (&p, &contentstr,    ':');
      inu_get_next_parsed_value (&p, &Statusstr,     ':');

      op_type    = atoi (op_typestr);
      Status     = atoi (Statusstr);
      operation  = atoi (operationstr);

      if (inu_level_convert (levelstr, &level) != INUGOOD)
         level.ver = -1;

      if ((op = create_option (name, NO, NO, CONTENT_UNKNOWN, "", &level, "", 
                               NIL (BffRef_t))) != NIL (Option_t))
      {
        /** --------------------------------------------------------------*
         **  Set the vpd_tree field of this option. If it's a usr or 
         **  share part, then it's already set to the correct value. 
         **
         **  If it's a root part, set part to VPDTREE_ROOT ('M'), cuz we 
         **  wrote 'R' to the intermediate log file.
         ** ---------------------------------------------------------------*/

         if (*part == 'R')
            op->vpd_tree = VPDTREE_ROOT;
         else
            op->vpd_tree = *part;
 
        /** ----------------------------------------*
         **  Set the op_type, content, Status, and
         **  Operation fields for the summary.
         ** ----------------------------------------*/

         op->op_type   = op_type;
         op->content   = *contentstr;
         op->Status    = Status; 
         op->operation = operation; 

        /** ------------------------------------------------------------*
         **  Hurl this option onto the appropriate sop so the pif or the
         **  installation summary can report it. 
         ** ------------------------------------------------------------*/

         inu_add_to_appropriate_sop (failsop, sop, op);
      }

      else
          inu_quit (INUMALLOC); 
   } /* while */

   return INUGOOD;
}

/*--------------------------------------------------------------------------*
**
** Function Name:  inu_add_to_appropriate_sop
**
** Purpose:        Adds the option specified by op to the appropriate sop list. 
**              
** Returns:        None
**                         
**--------------------------------------------------------------------------*/

static void 
inu_add_to_appropriate_sop (Option_t *failsop, Option_t *sop, Option_t *op)
{
   static Option_t *fsop_end=NIL (Option_t);        /* end of failsop marker */
   static Option_t *sop_end=NIL (Option_t);         /* end of sop marker     */
   Option_t *saveop;                               /* tmp op ptr            */

   switch (op->Status)
   {
     /** -------------------------------------------- *
      **  If this op experienced a pif failure, then
      **  put it on the failsop.
      ** -------------------------------------------- */

      case STAT_CANCEL:
      case STAT_NOT_FOUND_ON_MEDIA:
      case STAT_REQUISITE_FAILURE:
      case STAT_ALREADY_SUPERSEDED:
      case STAT_ALREADY_INSTALLED:
      case STAT_TO_BE_SUPERSEDED:
      case STAT_BROKEN:
      case STAT_BASE_MUST_BE_COMMITTED:
      case STAT_BASE_ALREADY_INSTALLED:

               if (fsop_end == NIL (Option_t))
                  fsop_end = failsop;
               saveop         = fsop_end->next;
               fsop_end->next = op;
               op->next       = saveop; 
               fsop_end       = fsop_end->next;
               break; 

     /** -------------------------------------------- *
      **  If this op did not experience a pif failure, 
      **  then put it on the sop.
      ** -------------------------------------------- */
      default:
               if (sop_end == NIL (Option_t))
                  sop_end  = sop;
               saveop        = sop_end->next;
               sop_end->next = op;
               op->next      = saveop; 
               sop_end       = sop_end->next;
               break; 
   }
}

/*--------------------------------------------------------------------------*
**
** Function Name:  inu_set_sum_log_file_values
**
** Purpose:        Sets the values needed for the intermediate summary log
**                 AND for the final summary log.
**              
** Parameters      See below
**
** Returns:        None  
**                         
**--------------------------------------------------------------------------*/

static void inu_set_sum_log_file_values 
 (
   Option_t *op,         /* option to get values from                         */
   int      *ccode,      /* completion code (could be NULL)                   */
   int      *finalstate, /* final state of op (could be NULL)                 */
   char     *part,       /* usr, root, or share part of op we're dealing with */
   char     *level       /* ptr to level string vv.rr.mmmm.ffff.pppppppp      */
)

{
   int code  = 0;
   int state = 0;

  /** ----------------------------------------- *
   **   Zero out these values.
   ** ----------------------------------------- */

   *part    =  INU_EMPTY_PART;
   level[0] =  '\0';

   /*-------------------------------------------------------------------*
    *  Set the level variable to be in vv.rr.mmmm.ffff.pppppppp format.
    *-------------------------------------------------------------------*/

    if (op->level.ver != -1)
    {
       if (op->level.ptf[0] == '\0')
          sprintf (level, "%d.%d.%d.%d",    op->level.ver, op->level.rel, 
                                            op->level.mod, op->level.fix);
       else
          sprintf (level, "%d.%d.%d.%d.%s", op->level.ver, op->level.rel, 
                                            op->level.mod, op->level.fix, 
                                            op->level.ptf);
    }

   /*-------------------------------------------------------------------*
    * Set the part variable to 'U', 'R', or 'S' for usr, root, or share. 
    *-------------------------------------------------------------------*/

    if (op->vpd_tree == VPDTREE_ROOT)
       *part = 'R';
    else
       *part = op->vpd_tree;

   /*-------------------------------------------------------------------*
    *  If ccode and finalstate are both NULL, then we don't have to
    *  set either value -- which is all that's left to do.   So, just 
    *  return.
    *-------------------------------------------------------------------*/

    if (ccode == NIL (int)  &&  finalstate == NIL (int))
       return;

   /*-------------------------------------------------------------------*
    * Set the state and the code values. 
    * Based on what we were trying to do and if we were successful or
    * not, determine what the state and corresponding code should be.
    *-------------------------------------------------------------------*/

    switch (op->operation)
    {
       case OP_APPLY:
       case OP_APPLYCOMMIT:
          switch (op->Status)
          {
             case STAT_SUCCESS:
                if (IF_UPDATE (op->op_type))
                   state = ST_APPLIED;
                else
                   state = ST_COMMITTED;
                code  = IS_INST_SUCCESS;
                break;
             case STAT_IFREQ_FAIL:
             case STAT_CLEANUP_SUCCESS:
                state = ST_AVAILABLE;
                code  = IS_INST_FAILURE;
                break;
             case STAT_CLEANUP:        /* cleanup needed, but not attempted */
             case STAT_CLEANUP_FAILED:
                state = ST_BROKEN;
                code  = IS_INST_FAILURE;
                break;
             case STAT_FAILURE:
                state = ST_BROKEN;
                code  = IS_INST_FAILURE;
                break;
             case STAT_PART_INCONSIST:
             case STAT_CANCEL:
             case STAT_NOT_FOUND_ON_MEDIA:
             case STAT_REQUISITE_FAILURE:
             case STAT_ALREADY_SUPERSEDED:
             case STAT_ALREADY_INSTALLED:
             case STAT_TO_BE_SUPERSEDED:
             case STAT_BROKEN:
             case STAT_BASE_MUST_BE_COMMITTED:
             case STAT_BASE_ALREADY_INSTALLED:
             case STAT_NUTTIN_TO_APPLY:
             case STAT_NO_USR_MEANS_NO_ROOT:
             case STAT_MUST_APPLY_ROOT_TOO:
             case STAT_OTHER_BROKENS:
             case STAT_SUP_OF_BROKEN:
             case STAT_OEM_BASELEVEL:
             case STAT_OEM_MISMATCH:
             case STAT_OEM_REPLACED:
                state = ST_AVAILABLE;
                code  = inu_map_Status_to_file_value (op->Status);
                break;
             default:
                state = DONT_PRINT_STATE; 
                code  = IS_INST_FAILURE;
                break;
          }   
          break;

       case OP_COMMIT:
          switch (op->Status)
          {
             case STAT_SUCCESS:
                state = ST_COMMITTED;
                code = IS_INST_SUCCESS;
                break;
             case STAT_REQUISITE_FAILURE:
                state = ST_APPLIED;
                code  = IS_PIFW_REQUISITE_FAIL;
                break;
             case STAT_BROKEN:
                state = DONT_PRINT_STATE;
                code  = IS_PIFW_BROKEN; 
                break;
             case STAT_OTHER_BROKENS:
                state = DONT_PRINT_STATE;
                code  = IS_PIFW_OTHER_BROKENS; 
                break;
             case STAT_ALREADY_COMMITTED:
             case STAT_PART_INCONSIST:
             case STAT_NUTTIN_TO_COMMIT:
                state = DONT_PRINT_STATE;
                code  = inu_map_Status_to_file_value (op->Status);
                break;
             default:
                state = 0;
                code  = IS_INST_FAILURE;
          }
          break;

       case OP_CLEANUP_APPLY:
          code = IS_INST_FAILURE;
          switch (op->Status)
          {
             case STAT_CLEANUP_SUCCESS:
                state = ST_AVAILABLE;
                break;
             case STAT_CLEANUP_FAILED:
                state = ST_BROKEN;
                break;
             case STAT_CLEANUP:
                /* means the cleanup wasn't even attempted */
                   state = ST_APPLYING;
                break;
             default:
                state = 0;
                break;
          }   
          break;

       case OP_CLEANUP_COMMIT:
          state = ST_COMMITTED;
          code  = IS_INST_SUCCESS;
          break;

       case OP_REJECT:
       case OP_DEINSTALL:
          switch (op->Status)
          {
             case STAT_SUCCESS:
                state = ST_AVAILABLE;
                code  = IS_INST_SUCCESS;
                break;
             case STAT_FAILURE:
                state = ST_BROKEN;
                code  = IS_INST_FAILURE;
                break;
             case STAT_PART_INCONSIST:
             case STAT_NUTTIN_TO_REJECT:
             case STAT_NUTTIN_TO_DEINSTL:
             case STAT_COMMITTED_CANT_REJECT:
             case STAT_WARN_DEINST_MIG:
             case STAT_WARN_DEINST_3_1:
             case STAT_WARN_DEINST_3_2:
             case STAT_FAILED_PRE_D:
             case STAT_NO_DEINST_BOS:
                state = DONT_PRINT_STATE;
                code  = inu_map_Status_to_file_value (op->Status);
                break;
             default:
                state = DONT_PRINT_STATE;
                code  = IS_PIFW_REQUISITE_FAIL;
                break;
          }   
          break;

       default:
          state = 0;
          code  = IS_PIFW_REQUISITE_FAIL;
          break;
    } /* switch */

    if (ccode != NIL (int))
        *ccode  = code;

    if (finalstate != NIL (int))
        *finalstate = state;
}

/*--------------------------------------------------------------------------*
**
** Function Name:  inu_get_next_parsed_value 
**
** Purpose:        Parses the string pointed to by *p and gets the next
**                 value up until the delimiter is reached.  Assumes format
**                 is [value][delimiter][value][delimiter]... 
**              
** Returns:        INUGOOD -  
**                 FAILURE - 
**                         
**--------------------------------------------------------------------------*/


static void inu_get_next_parsed_value 
 (
  char **p,      /* string to parse                        */
  char **val,    /* parsed value returned to caller        */ 
  char delimiter /* delimiter that the parsing is based on */ 
)

{
  /** ----------------------------------------------------------- *
   **  Make val point to the first character of the parsed value.
   ** ----------------------------------------------------------- */
 
   *val = *p;

  /** ----------------------------------------------------------- *
   **  If the 1st char is the delimiter, then the parsed value is
   **  empty.   So, null terminate the string and return. 
   ** ----------------------------------------------------------- */

   if (**p == delimiter)
   {
      **p = '\0';
      (*p)++;
      return;
   }

  /** ----------------------------------------------------------- *
   **  While we don't encounter the delimiter and we don't run out 
   **  of string, keep moving on up.
   ** ----------------------------------------------------------- */

   while (**p != delimiter  &&  **p != '\0')
      (*p)++;

  /** ----------------------------------------------------------- *
   **  If we haven't hit the end of the string,  null terminate 
   **  the piece we've parsed, so that val (above) will be a null 
   **  terminated string. 
   ** ----------------------------------------------------------- */

   if (**p != '\0')
   {
     **p = '\0';
     (*p)++;
   }
}

/*--------------------------------------------------------------------------*
**
** Function Name:    inu_map_Status_to_file_value
**
** Purpose:          Converts from the Status field value (from the op ptr)
**                   to the value that will actually be written to the final
**                   installp.summary file.
**              
** Returns:          The completion code value as it will actually appear
**                   in the installp.summary file.
**                         
**--------------------------------------------------------------------------*/

static int inu_map_Status_to_file_value (int Status)

{
   int rc=IS_INST_FAILURE;    /* DEFAULT to an installation failure */

   switch (Status)
   {
      case STAT_NOT_FOUND_ON_MEDIA:
           rc = IS_PIFW_NOT_ON_MEDIA;
           break;
      case STAT_REQUISITE_FAILURE:
           rc = IS_PIFW_REQUISITE_FAIL;
           break;
      case STAT_ALREADY_SUPERSEDED:
           rc = IS_PIFW_ALREADY_SEDED;
           break;
      case STAT_ALREADY_INSTALLED:
           rc = IS_PIFW_ALREADY_INSTALLED;
           break;
      case STAT_TO_BE_SUPERSEDED:
           rc = IS_PIFW_TO_BE_SEDED;
           break;
      case STAT_CAN_BE_SUPERSEDED:
           rc = IS_PIFW_CAN_BE_SEDED;
           break;
      case STAT_ROOT_CAN_BE_SUPERSEDED:
           rc = IS_PIFW_ROOT_CAN_BE_SEDED;
           break;
      case STAT_BROKEN:
           rc = IS_PIFW_BROKEN;
           break;
      case STAT_BASE_MUST_BE_COMMITTED:
           rc = IS_PIFW_MUST_COMMIT;
           break;
      case STAT_BASE_ALREADY_INSTALLED:
           rc = IS_PIFW_ALREADY_INSTALLED;
           break;
      case STAT_NO_USR_MEANS_NO_ROOT:
           rc = IS_PIFW_NO_USR_MEANS_NO_ROOT;
           break;
      case STAT_NUTTIN_TO_APPLY:
           rc = IS_PIFW_NOTH_TO_APPLY;
           break;
      case STAT_NUTTIN_TO_COMMIT:
           rc = IS_PIFW_NOTH_TO_COMMIT;
           break;
      case STAT_NUTTIN_TO_REJECT:
           rc = IS_PIFW_NOTH_TO_REJECT;
           break;
      case STAT_NUTTIN_TO_DEINSTL:
           rc = IS_PIFW_NOTH_TO_DEINSTL;
           break;
      case STAT_MUST_APPLY_ROOT_TOO:
           rc = IS_PIFW_MUST_DO_ROOT_TOO;
           break;
      case STAT_ALREADY_COMMITTED:
           rc = IS_PIFW_ALREADY_COMMIT;
           break;
      case STAT_COMMITTED_CANT_REJECT:
           rc = IS_PIFW_COMMITTED_NO_REJECT;
           break;
      case STAT_OTHER_BROKENS:
           rc = IS_PIFW_OTHER_BROKENS;
           break;
      case STAT_WARN_DEINST_MIG:
           rc = IS_PIFW_DEINST_MIG;
           break;
      case STAT_WARN_DEINST_3_1:
           rc = IS_PIFW_DEINST_3_1;
           break;
      case STAT_WARN_DEINST_3_2:
           rc = IS_PIFW_DEINST_3_2;
           break;
      case STAT_FAILED_PRE_D:
           rc = IS_PIFW_FAILED_PRE_D;
           break;
      case STAT_NO_DEINST_BOS:
           rc = IS_PIFW_NO_DEINST_BOS;
           break;
      case STAT_PART_INCONSIST:
           rc = IS_PIFW_PART_INCONSIST;
           break;
      case STAT_SUP_OF_BROKEN:
           rc = IS_PIFW_SUP_OF_BROKEN;
           break;
      case STAT_OEM_BASELEVEL:
           rc = IS_PIFW_OEM_BASELEVEL;
           break;
      case STAT_OEM_MISMATCH:
           rc = IS_PIFW_OEM_MISMATCH;
           break;
      case STAT_OEM_REPLACED:
           rc = IS_PIFW_OEM_REPLACED;
           break;
      case STAT_DUPE_VERSION:   /* Don't want an entry for dupes */
           break;
   }

   return rc;
}

/*--------------------------------------------------------------------------*
**
** Function Name:    needs_an_installp_summary_file_entry 
**
** Purpose:          Determines a boolean answer to the question "Should
**                   this option have an entry in the installp.summary file?" 
**              
** Returns:          TRUE - if the answer is yes
**                  FALSE - if the answer is no
**                         
**--------------------------------------------------------------------------*/

static int needs_an_installp_summary_file_entry (Option_t *op)
{
   int rc=TRUE;

   if (op->Status == STAT_DUPE_VERSION)
      rc = FALSE;

   return rc;
}
