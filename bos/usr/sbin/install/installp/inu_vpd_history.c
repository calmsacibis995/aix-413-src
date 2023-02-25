static char     sccsid[] = "@(#)10  1.3.1.8  src/bos/usr/sbin/install/installp/inu_vpd_history.c, cmdinstl, bos411, 9428A410j 6/22/94 15:29:06";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_vpd_history
 *
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

#include <pwd.h>
#include <installp.h>
#include <instl_options.h>

/*--------------------------------------------------------------------------*
**
** NAME: inu_vpd_history ()
**
** FUNCTION:
**  Add or updates a history record based on the event (apply cleanup commit)
**
** EXECUTION ENVIRONMENT: Part of user command.
**
** NOTES:
**  The state parameter refers to the state that's valid for the LPP and
**  PRODUCT databases. The HISTORY database will be updated (or added)
**  with a HISTORY state based on the state given for the LPP and PRODUCT dbs.
**
** ON ENTRY:
**  state   specifies the state to be set (comes from constants
**          defined in /usr/include/swvpd.h
**
**--------------------------------------------------------------------------*/

void inu_vpd_history (Option_t * op,    /* Ptr to the option of this
                                           operation. */
                      int        state, /* Which state to to set it to. */
                      int        event) /* The event: HIST_APPLY, HIST_CLEANUP, 
                                         * HIST_COMMIT */
{
   struct passwd  *pw;  /* passwd structure returned from getpwuid  */
   hist_t          hist;
   int             rc;  /* return code                  */

   /*-----------------*
    * HISTORY DATABASE
    *-----------------------------------------------------------------------*
    * Change/add a vpd entry for the HISTORY database.
    *-----------------------------------------------------------------------*/

   if ((rc = vpdreslpp_name (op->name, &hist.lpp_id)) == VPD_NOID)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INTRN_MISSVPDI_E, 
                               INP_INTRN_MISSVPDI_D));
      instl_msg (FAIL_MSG, "\tLPP_HISTORY.lpp_id -> LPP_TABLE.name = %s" , op->name);
      return;
   }
   hist.event = event;
   hist.ver = op->level.ver;
   hist.rel = op->level.rel;
   hist.mod = op->level.mod;
   hist.fix = op->level.fix;
   strcpy (hist.ptf, op->level.ptf);
   hist.state = HIST_PENDING;

   /*----------------------------------------------------------------------*
    * If the state = HIST_PENDING, then we need to add a new audit record to
    * the vpd for the event which is about to take place...
    *----------------------------------------------------------------------*/

   if (state == HIST_PENDING)
   {
      hist.cp_mod[0] = '\0';
      hist.cp_fix[0] = '\0';
      (void) get_corr_svn (&hist);      /* get info out of service_num  */
      if ((pw = getpwuid (getuid ())) != NIL (struct passwd))
         strcpy (hist.login_name, pw->pw_name);
      else
         strcpy (hist.login_name, "UNKNOWN");

      hist.time = time ((long *) 0);
      hist.comment = NullString;
      rc = vpdadd (HISTORY_TABLE, &hist);
      inu_ck_vpd_sys_rc (rc);
   }
   else
   {
      /*------------------------------------------------------------------*
       * Try to get the record that was created (above this else stmt) from
       * before when we attempted to apply/cleanup/commit this option and
       * update its state based on the outcome of the operation attempt.
       *------------------------------------------------------------------*/

      rc = vpdget (HISTORY_TABLE, 
                   HIST_LPP_ID | HIST_EVENT | HIST_VER | HIST_REL | HIST_FIX |
                   HIST_PTF | HIST_STATE, &hist);
      inu_ck_vpd_sys_rc (rc);

      /*------------------------------------------------------------------*
       * If we find the history record (which we should) then if the state
       * passed in is ST_APPLIED then set the history record to APPLIED.
       * If the state passed in is any thing else then this apply attempt
       * failed so mark the history state to BROKEN...
       *------------------------------------------------------------------*/

      if (rc == VPD_OK)
      {
         hist.state = state;
         rc = vpdchgget (HISTORY_TABLE, &hist);
         inu_ck_vpd_sys_rc (rc);
         vpd_free_vchars (HISTORY_TABLE, &hist);
      }
   }
} /* end inu_vpd_history */
