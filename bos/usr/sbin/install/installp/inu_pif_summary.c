static char sccsid[] = "@(#)90  1.40  src/bos/usr/sbin/install/installp/inu_pif_summary.c, cmdinstl, bos41J, 9510A_all 2/27/95 12:14:28";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_pif_summary
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

#include  <sys/audit.h>
#include  <installp.h>
#include  <inu_string.h>

#ifdef INUDEBUG
extern FILE    *dfp;

#endif  /* INUDEBUG */

extern int      string_len;

int failsop_is_nonempty (Option_t *);

/*--------------------------------------------------------------------------*
**
** Function Name: inu_pif_summary
**
** Purpose:       Generates a summary report of what failed pre-installation
**                processing.  This report is sent to FAIL_MSG.
**
** Returns:       INUGOOD -  if no hardcore failures encountered on the 
**                           failsop.  Warnings are not considered failures.
**                FAILURE -  if one or more options outright failed 
**                           pre-installation processing
**
**--------------------------------------------------------------------------*/
int inu_pif_summary (Option_t * failsop)
{
   int       rc=0;
   Option_t *t;
   int       part = UNKNOWN_STR;   /* index into the tbl for the part */
   char     *msg;
   int      i=0;
   char     level_str[MAX_LEVEL_LEN+1];

   if (failsop_is_nonempty (failsop))
   {
      instl_msg (WARN_MSG, "\n");
      instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, INP_PIFWS_HDR1_I, 
                                                INP_PIFWS_HDR1_D));
      instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, INP_PIFWS_HDR2_I, 
                                                INP_PIFWS_HDR2_D));

      for (t = failsop->next;
           t != NIL (Option_t);
           t = t->next)
      {
         if (t->Status !=  STAT_DUPE_VERSION)
         {
           /** -------------------------------------------------------------*
            **  Print the 1st 2 field of the pifw line (name, level).
            **  level will be:
            **         ""  - if ptf id is empty, and if version = -1
            **     ptf id  - for 3.2 pkgs
            **     v.r.m.f - for everything else
            ** -------------------------------------------------------------*/

            if (t->level.ptf[0] != '\0')
               strcpy (level_str, t->level.ptf);
            else if (t->level.ver >= 0)
               sprintf (level_str, "%d.%d.%d.%d", t->level.ver, t->level.rel,
                                                  t->level.mod, t->level.fix);
            else
               strcpy (level_str, "");

            instl_msg (WARN_MSG, "%-25.25s %-15.15s ", t->name, level_str);
         }

         switch (t->Status)
         {
            case STAT_NOT_FOUND_ON_MEDIA:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                      INP_PIF_NOT_FOUND_ON_MEDIA_I, 
                                      INP_PIF_NOT_FOUND_ON_MEDIA_D));
                 rc++;
                 break;
            case STAT_REQUISITE_FAILURE:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_REQUISITE_FAILURE_I, 
                                              INP_PIF_REQUISITE_FAILURE_D));
                 rc++;
                 break;
            case STAT_ALREADY_SUPERSEDED:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_ALREADY_SUPERSEDED_I, 
                                              INP_PIF_ALREADY_SUPERSEDED_D), 
                                              t->supersedes); 
                 break;
            case STAT_ALREADY_INSTALLED:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_ALREADY_INSTALLED_I, 
                                              INP_PIF_ALREADY_INSTALLED_D)); 
                 break;
            case STAT_TO_BE_SUPERSEDED:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_TO_BE_SUPERSEDED_I, 
                                              INP_PIF_TO_BE_SUPERSEDED_D), 
                                              t->supersedes); 
                 break;
            case STAT_CAN_BE_SUPERSEDED:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_NOM_TRY_SEDES_I, 
                                              INP_PIF_NOM_TRY_SEDES_D),
                                              t->supersedes);
                 rc++;
                 break;
            case STAT_ROOT_CAN_BE_SUPERSEDED:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP,
                                              INP_PIF_USR_NOT_INSTALLED_I,
                                              INP_PIF_USR_NOT_INSTALLED_D),
                                              t->supersedes);
                 rc++;
                 break;
            case STAT_BROKEN:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_BROKEN_NEEDS_COMMIT_I, 
                                              INP_PIF_BROKEN_NEEDS_COMMIT_D));
                 rc++;
                 break;
            case STAT_BASE_MUST_BE_COMMITTED:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                             INP_PIF_BASE_MUST_BE_COMMITTED_I, 
                                             INP_PIF_BASE_MUST_BE_COMMITTED_D));
                 rc++;
                 break;
            case STAT_BASE_ALREADY_INSTALLED:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_ALREADY_INST_I, 
                                              INP_PIF_ALREADY_INST_D));
                 break;
            case STAT_NO_USR_MEANS_NO_ROOT:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_NO_USR_MEANS_NO_ROOT_I, 
                                              INP_PIF_NO_USR_MEANS_NO_ROOT_D));
                 break;
            case STAT_NUTTIN_TO_APPLY:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_NUTTIN_TO_APPLY_I, 
                                              INP_PIF_NUTTIN_TO_APPLY_D));
                 break;
            case STAT_NUTTIN_TO_COMMIT:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_NUTTIN_TO_COMMIT_I, 
                                              INP_PIF_NUTTIN_TO_COMMIT_D));
                 break;
            case STAT_NUTTIN_TO_REJECT:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_NUTTIN_TO_REJECT_I, 
                                              INP_PIF_NUTTIN_TO_REJECT_D));
                 break;
            case STAT_NUTTIN_TO_DEINSTL:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_NUTTIN_TO_DEINSTL_I, 
                                              INP_PIF_NUTTIN_TO_DEINSTL_D));
                 break;
            case STAT_MUST_APPLY_ROOT_TOO:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_MUST_APPLY_ROOT_TOO_I, 
                                              INP_PIF_MUST_APPLY_ROOT_TOO_D));
                 rc++;
                 break;
            case STAT_ALREADY_COMMITTED:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_ALREADY_COMMIT_I, 
                                              INP_PIF_ALREADY_COMMIT_D));
                 break;
            case STAT_COMMITTED_CANT_REJECT:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                             INP_CANT_REJ_COMMITEDS_I, 
                                             INP_CANT_REJ_COMMITEDS_D));
                 rc++;
                 break;
            case STAT_OTHER_BROKENS:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                               INP_PIF_OTHER_BROKENS_NEED_COMMIT_I, 
                               INP_PIF_OTHER_BROKENS_NEED_COMMIT_D));
                 rc++;
                 break;
            case STAT_SUP_OF_BROKEN:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                            INP_PIF_SUPS_A_BROKEN_FILESET_I, 
                            INP_PIF_SUPS_A_BROKEN_FILESET_D));
                 rc++;
                 break;
            case STAT_WARN_DEINST_3_1:
            case STAT_WARN_DEINST_3_2:
            case STAT_WARN_DEINST_MIG:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIW_SEE_WARNING_SECTION_I, 
                                              INP_PIW_SEE_WARNING_SECTION_D));
                 break;
            case STAT_FAILED_PRE_D:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_FAILED_PRE_D_I, 
                                              INP_PIF_FAILED_PRE_D_D));
                 rc++;
                 break;
            case STAT_NO_DEINST_BOS:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_NO_DEINST_BOS_I, 
                                              INP_PIF_NO_DEINST_BOS_D));
                 rc++;
                 break;
            case STAT_PART_INCONSIST:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_INCOST_UR_PARTS_I, 
                                              INP_INCOST_UR_PARTS_D));
                 rc++;
                 break;
            case STAT_NO_FORCE_APPLY_PTF:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_FORCE_APPLY_PTF_I, 
                                              INP_PIF_FORCE_APPLY_PTF_D));
                 rc++;
                 break;
            case STAT_OEM_MISMATCH:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_OEM_MISMATCH_I, 
                                              INP_OEM_MISMATCH_D));
                 rc++;
                 break;
            case STAT_OEM_REPLACED:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_OEM_REPLACED_I, 
                                              INP_OEM_REPLACED_D));
                 rc++;
                 break;
            case STAT_OEM_BASELEVEL:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_OEM_BASELEVEL_I, 
                                              INP_OEM_BASELEVEL_D));
                 rc++;
                 break;
            case STAT_NOTHING_FOUND_ON_MEDIA:
                 break;
            case STAT_DUPE_VERSION:   /* do not print a msg here */
                 break;
            default:
                 instl_msg (WARN_MSG, MSGSTR (MS_INSTALLP, 
                                              INP_PIF_FAILURE_I, 
                                              INP_PIF_FAILURE_D));
                 break;
         }
      } /* for */

      instl_msg (WARN_MSG, "\n");
   }

   return (rc);
}

/*--------------------------------------------------------------------------*
**
** Function Name:    failsop_is_empty
**
** Purpose:          Determines a boolean answer to the question "is the
**                   failsop empty?"
**             
** Returns:          TRUE - if the failsop either has no options on it or
**                          no options that we wanna report (some options
**                          are on the failsop soley for the installp.summary
**                          file.
**                  FALSE - if the failsop has one or more options on it
**                          that we wanna report.
**               
**--------------------------------------------------------------------------*/

int failsop_is_nonempty (Option_t *failsop)
{
   Option_t *t;

   if (failsop->next == NIL (Option_t))
      return FALSE;

   for (t=failsop->next; 
        t != NIL (Option_t); 
        t=t->next)
   {
      if (t->Status == STAT_DUPE_VERSION  ||  t->Status == STAT_ALL_KW_FAILURE)
         continue;
      else
         return TRUE;
   }

   return FALSE;
}
