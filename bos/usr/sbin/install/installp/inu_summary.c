static char sccsid[] = "@(#)04  1.9.1.19  src/bos/usr/sbin/install/installp/inu_summary.c, cmdinstl, bos411, 9428A410j 4/14/94 11:17:42";
/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS: inu_summary, 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/
#include  <sys/audit.h>
#include  <installp.h>
#include  <inu_string.h>

extern int string_len;
extern int need_to_reexec;

/*--------------------------------------------------------------------------*
**
** NAME: inu_summary
**
** FUNCTION:
**  Generates a summary report of what was installed, it also will generate
**  a status file in /tmp so instclient will know what failed.
**
** EXECUTION ENVIRONMENT:
**
**
** RETURNS: void
**
**--------------------------------------------------------------------------*/

int inu_summary (
Option_t *SopPtr)
{
    char *ptr;
    char Option[255];
    char Level[255];
    char Outcome[255];
    int Status, CleanupStatus;
    Option_t *sop;
    int i;
    int len;
    FILE *fp;
    char *env;
    char ptf[10];
    int update  = UNKNOWN_STR; /* index into the tbl for the update column */
    int part    = UNKNOWN_STR; /* index into the tbl for the part   column */
    int event   = UNKNOWN_STR; /* index into the tbl for the event  column */
    int result  = UNKNOWN_STR; /* index into the tbl for the result column */
    int state   = UNKNOWN_STR; /* index into the tbl for the state  column */
    int cleanup = 0;           /* indicator to whether a cleanup line needs */
                               /* to be written out also */
    int rc      = INUGOOD;     /* return code from this function */
    char level_str [MAX_LEVEL_LEN+1];


    if (SopPtr->next == NIL (Option_t))
      return INUGOOD;

   /*-----------------------------------------------------------------------*
    * Try to get the environment variable INUCLTSTAT which should contain the
    * path to the status file that instlclient needs to determine what was
    * the status of the installation. If the env variable isn't set then
    * nothing will be written to the status file. If the env variable does
    * exist, the information will be appended to the bottom of the file.
    *-----------------------------------------------------------------------*/
    if ((env = getenv ("INUCLTSTAT")) == NIL (char))
         fp = NIL (FILE);
    else {
         if ((fp = fopen (env, "a")) == NIL (FILE))
              instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), INU_INSTALLP_CMD, env);
    }

    inu_init_strings ();

  /*-----------------------------------------------------------------------*
   * Print out the header for the summary report.
   *-----------------------------------------------------------------------*/

   instl_msg (INFO_MSG, "\n");
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_SUMM_LINE_I, INP_SUMM_LINE_D));
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_SUM_HEAD_I, INP_SUM_HEAD_D));


   /*-----------------------------------------------------------------------*
    * Loop though the entire Selected Options List and generate the summary
    * report and the the status file.
    *-----------------------------------------------------------------------*/
    for (sop = SopPtr->next;
         sop != NIL (Option_t);
         sop = sop->next)
    {
        if (   (need_to_reexec)       /* We are about to re-exec installp */
                        && 
             (sop->Status == STAT_CANCEL))
        {
          continue;
        }

        state = SUCCESS_STR; /* default to success */

       /** ----------------------------------------------------------------*
        **  Define level_str:
        **   3.2 update:       Use ptf id
        **   Everything else:  Use v.r.m.f
        ** ----------------------------------------------------------------*/

        if (! (IF_3_2_UPDATE(sop->op_type)))
           sprintf(level_str, "%d.%d.%d.%d", sop->level.ver, sop->level.rel,
                                             sop->level.mod, sop->level.fix);
        else
           strcpy (level_str, sop->level.ptf);


       /*-------------------------------------------------------------------*
        * switch on the vpd tree to define what part we're dealing with.
        *-------------------------------------------------------------------*/
        switch (sop->vpd_tree) {
            case VPDTREE_USR:
                part = USR_STR;
                break;

            case VPDTREE_ROOT:
                part = ROOT_STR;
                break;

            case VPDTREE_SHARE:
                part = SHARE_STR;
                break;

            default:
                part = UNKNOWN_STR;
                break;

        } /* end switch (sop->vpd_tree) */

       /*-------------------------------------------------------------------*
        * switch on the operation field to define the operation we attempted.
        *-------------------------------------------------------------------*/
        CleanupStatus = Status = AUDIT_FAIL;
        switch (sop->operation) {
            case OP_APPLY:
            case OP_APPLYCOMMIT:
                event = APPLY_STR;
                switch (sop->Status) {
                    case STAT_SUCCESS:
                        result = SUCCESS_STR;
                        if (IF_INSTALL (sop->op_type))
                           state = COMMITTED_STR;
                        else
                           state = APPLIED_STR;
                        Status = AUDIT_OK;
                        break;
                    case STAT_CANCEL:
                        result = CANCELLED_STR;
                        state  = AVAILABLE_STR;
                        break;
                    case STAT_CLEANUP_SUCCESS:
                        result  = FAILURE_STR;
                        state   = BLANK_STR;
                        cleanup = SUCCESS_STR;
                        CleanupStatus = AUDIT_OK;
                        break;
                    case STAT_CLEANUP_FAILED:
                        result  = FAILURE_STR;
                        state   = BLANK_STR;
                        cleanup = FAILURE_STR;
                        break;
                    case STAT_CLEANUP:
                        /* means the cleanup wasn't even attempted */
                        result  = FAILURE_STR;
                        state   = BLANK_STR;
                        cleanup = CANCELLED_STR;
                        break;
                    case STAT_FAILURE:
                        result  = FAILURE_STR;
                        state   = BROKEN_STR;
                        break;
                    default:
                        result  = UNKNOWN_STR;
                        state   = UNKNOWN_STR;
                        break;
                } /* end  switch (sop->Status) */
                break;

            case OP_COMMIT:
                event = COMMIT_STR;
                switch (sop->Status) {
                    case STAT_SUCCESS:
                        result  = SUCCESS_STR;
                        state   = COMMITTED_STR;
                        Status  = AUDIT_OK;
                        break;
                    default:
                        result  = UNKNOWN_STR;
                        state   = UNKNOWN_STR;
                        break;
                } /* end  switch (sop->Status) */
                break;

            case OP_CLEANUP_APPLY:
                event = CLEANUP_STR;
                switch (sop->Status) {
                    case STAT_CLEANUP_SUCCESS:
                        result = SUCCESS_STR;
                        state  = AVAILABLE_STR;
                        Status = AUDIT_OK;
                        break;
                    case STAT_CLEANUP_FAILED:
                        result = FAILURE_STR;
                        state  = BROKEN_STR;
                        break;
                    case STAT_CLEANUP:
                        /* means the cleanup wasn't even attempted */
                        result = CANCELLED_STR;
                        state  = APPLYING_STR;
                        break;
                    default:
                        result = UNKNOWN_STR;
                        state  = UNKNOWN_STR;
                        break;
                } /* end  switch (sop->Status) */
                break;

            case OP_CLEANUP_COMMIT:
                event  = CLEANUP_STR;
                result = SUCCESS_STR;
                state  = COMMITTED_STR;
                Status = AUDIT_OK;
                break;

            case OP_REJECT:
                event = REJECT_STR;
                switch (sop->Status) {
                    case STAT_SUCCESS:
                        result = SUCCESS_STR;
                        state  = AVAILABLE_STR;
                        Status = AUDIT_OK;
                        break;
                    case STAT_FAILURE:
                        result = FAILURE_STR;
                        state  = BROKEN_STR;
                        break;
                    default:
                        result = UNKNOWN_STR;
                        state  = UNKNOWN_STR;
                        break;
                } /* end  switch (sop->Status) */
                break;

            case OP_DEINSTALL:
               event = DEINSTALL_STR;
               switch (sop->Status)
               {
                  case STAT_SUCCESS:
                     result = SUCCESS_STR;
                     state  = AVAILABLE_STR;
                     Status = AUDIT_OK;
                     break;
                  case STAT_FAILURE:
                     result = FAILURE_STR;
                     state  = BROKEN_STR;
                     break;
                  case STAT_CANCEL:
                     result = CANCELLED_STR;
                     state  = BLANK_STR;
                     break;
                  default:
                     result = UNKNOWN_STR;
                     state  = UNKNOWN_STR;
                     break;
               }   /* end  switch (sop->Status) */
               break;

            default:
                event  = UNKNOWN_STR;
                result = UNKNOWN_STR;
                state  = UNKNOWN_STR;
                break;

        } /* end switch (sop->operation) */

        if (result != SUCCESS_STR) {

        /* installp for at least one option was not successful */

                rc = INUSUMMARY;
        }

       /*-------------------------------------------------------------------*
        * Now print out the summary line.
        *-------------------------------------------------------------------*/
        instl_msg (INFO_MSG, "%-27.27s %-15.15s %-11s %-11s %-11s\n",
                             sop->name,
                             level_str,
                             string[part],
                             string[event],
                             string[result]);

       /*-------------------------------------------------------------------*
        * Now write out an audit record reflecting the install status.
        *-------------------------------------------------------------------*/
        sprintf (Option, "%s (%s)", sop->name, string[part]);
        if (sop->level.ptf[0] == '\0')
                sprintf (Level, "%.2d.%.2d.%.4d.%.4d", sop->level.ver, 
                        sop->level.rel, sop->level.mod, sop->level.fix);
        else
                sprintf (Level, "%.2d.%.2d.%.4d.%.4d.%s", sop->level.ver, 
                        sop->level.rel, sop->level.mod, sop->level.fix, 
                        sop->level.ptf);
        sprintf (Outcome, "%s Result: %s", string[event], string[result]);
        auditwrite (INST_EVENT, Status, Option, strlen (Option) + 1, 
                   Level, strlen (Level) + 1, Outcome, strlen (Outcome) + 1, 
                   NULL);

       /*-------------------------------------------------------------------*
        * If the FILE ptr is valid then we want to write out to status file.
        * Check to see if ptf field has something in it, if not put NONE in.
        * Then print out the status line to the file for instlclient.
        *-------------------------------------------------------------------*
        *  The instlclient command when calling installp will create a tmp file
        *  and pass installp the name of this file by using the environment
        *  variable INUCLTSTAT. The installp command will append the following
        *  information in the file in the following format:
        *
        *          ------level--------
        *  lppname ver rel mod fix ptf content operation op_type Status
        *  %s      %d  %d  %d  %d  %s  %c      %d        %d      %d
        *  Only leaving one blank between fields, if ptf is not valid installp
        *  will print "NONE".
        *-------------------------------------------------------------------*/
        if (fp != NIL (FILE)) {
            if (sop->level.ptf[0] == '\0')
                 strcpy (ptf, "NONE");
            else
                 strcpy (ptf, sop->level.ptf);

            fprintf (fp, "%s %d %d %d %d %s %c %d %d %d\n", 
                         sop->name, 
                         sop->level.ver, 
                         sop->level.rel, 
                         sop->level.mod, 
                         sop->level.fix, 
                         ptf, 
                         sop->content, 
                         sop->operation, 
                         sop->op_type, 
                         sop->Status);
        }

       /*-------------------------------------------------------------------*
        * If we performed a cleanup also then print out a line and write an
        * audit record to reflect it.
        *-------------------------------------------------------------------*/
        if (cleanup) {
            if (cleanup == SUCCESS_STR)
                state = AVAILABLE_STR;
            else
                if (cleanup == CANCELLED_STR)
                    state = APPLYING_STR;
                else
                    state = BROKEN_STR;
            event = CLEANUP_STR;
            instl_msg (INFO_MSG, "%-27.27s %-15.15s %-11s %-11s %-11s\n",
                                  sop->name,
                                  level_str,
                                  string[part],
                                  string[event],
                                  string[cleanup]);
            sprintf (Outcome, "%s Result: %s", string[event], string[cleanup]);
            auditwrite (INST_EVENT, CleanupStatus, Option, 
                strlen (Option) + 1, Level, strlen (Level) + 1, Outcome, 
                strlen (Outcome) + 1, NULL);
            cleanup = 0;
        } /* end if (cleanup) */

    } /* end for () loop */

    return (rc);
}
