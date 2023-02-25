/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_exec_installp
 *		inu_regenerate_cmd_line
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
static char     sccsid[] = "@(#)16      1.7  src/bos/usr/sbin/install/installp/inu_exec_installp.c, cmdinstl, bos411, 9434B411a 8/12/94 08:07:10";

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/

#include <locale.h>             /* for call setlocale */
#include <signal.h>             /* for call setlocale */
#include <installp.h>
#include <instl_options.h>

extern char     *sys_errlist[];
extern Option_t  *SopPtr;        /* the anchor of the Selected Options list */
extern char      CWD[PATH_MAX * 2];              /* dir we were called from */
extern int       lpp_optind;            /* index into argv where lpps start */


static int  inu_regenerate_cmd_line (char * getargv[], char *** sendargv, 
                                     int    new_argv_size);

/* ------------------------------------------------------------------------ *
 *
 *  Function:   inu_exec_installp
 *
 *  Purpose:    to exec installp so it will process the cancelled items in
 *              the sop. The algorithm is:
 *              - regenerate the cmd line (based on cancelled items in sop)
 *              - if this succeeds, then exec installp w/ new cmd line
 *              - else use "system" to re-invoke installp w/ original cmd
 *                line.  (we'll get a lot of bogus msgs about stuff already
 *                being applied, but ...)
 *
 *  Parameters: argv[]    --  argv of this invocation of installp
 *
 *  Returns:    zero      -- if we can't exec ourselves, but can successfully
 *                           "system" call ourselves.
 *              nonzero   -- if either the exec call fails or if system call
 *                           fails.
 *              no return -- if we can successfully exec ourselves, 
 *                           we won't return.
 *
 * ------------------------------------------------------------------------ */



/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/* ------------------------------------------------------------------------ *
 *
 *  Function:   inu_exec_installp
 *
 *  Purpose:    to exec installp so it will process the cancelled items in
 *              the sop. The algorithm is:
 *              - regenerate the cmd line (based on cancelled items in sop)
 *              - if this succeeds, then exec installp w/ new cmd line
 *              - else use "system" to re-invoke installp w/ original cmd
 *                line.  (we'll get a lot of bogus msgs about stuff already
 *                being applied, but ...)
 *
 *  Parameters: argv[]    --  argv of this invocation of installp
 *
 *  Returns:    zero      -- if we can't exec ourselves, but can successfully
 *                           "system" call ourselves.
 *              nonzero   -- if either the exec call fails or if system call
 *                           fails.
 *              no return -- if we can successfully exec ourselves, 
 *                           we won't return.
 *
 * ------------------------------------------------------------------------ */

int inu_exec_installp (char * argv[])
{
   char          ** regen_argv;  /* regenerted cmd line in form of an argv  */
   Option_t       * tmpop;       /* used to traverse the sop                */
   int              regen_size;  /* initial size of regen_argv (# indices)  */
   int              rc = 1;
   int              any_cancelled = 0;

   /** ------------------------------------------------------------------ *
    ** See if anything is CANCELLED in the sop.  If no -- return success, 
    ** cuz we don't need to re-exec ourselves.   If yes -- we need to
    ** re-exec ourselves.
    ** ------------------------------------------------------------------ */

   for (tmpop = SopPtr->next;
        tmpop != NIL (Option_t);
        tmpop = tmpop->next)
   {
      if (tmpop->Status == STAT_CANCEL)
      {
         any_cancelled++;
         break;
      }
   }                            /* for */

   if ( ! any_cancelled)
      return INUGOOD;

   /** ------------------------------------------------------------------ *
    ** CWD is the directory that installp was originally called from.
    ** ------------------------------------------------------------------ */

   if ((chdir (CWD)) != SUCCESS)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                               CMN_CANT_CHDIR_D), INU_INSTALLP_CMD, CWD);
      inu_quit (INUCHDIR);
   }

   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_REINVOKE_I, INP_REINVOKE_D));

   /** ------------------------------------------------------------------ *
    ** Set an environment variable so that the next link in the exec call
    ** chain (of installp) knows that we are in an exec call chain.
    ** ------------------------------------------------------------------ */

   if ((inu_putenv ("INU_EXECING=1")) != INUGOOD)
      return (FAILURE);

   regen_size = 7 * PATH_MAX;
   rc = 1;

   /** ------------------------------------------------------------------ *
    ** Attempt to callocate an initial amount of space for the cmd line
    ** (which will be in the form of an argv -- array of ptrs to strings).
    ** If this succeeds, then attempt to regenerate the cmd line based on
    ** the cancelled items in the sop.
    ** ------------------------------------------------------------------ */

   if ((regen_argv = (char **) calloc (regen_size, sizeof (char *))) !=
                                                                 NIL (char *))
   {
      rc = inu_regenerate_cmd_line (argv, &regen_argv, regen_size);
      if (rc == INUGOOD)
      {
         /* get rid of the current INU_TMPDIR since its not needed anymore */
         (void) inu_rm (INU_TMPDIR);

	 active_tape_restore();
         rc = execv ("/usr/sbin/installp", regen_argv);
      }
   }
   else
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_REALLOC_FAILED_E, 
                               CMN_REALLOC_FAILED_D), inu_cmdname);
   }

   return rc;

} /* end inu_exec_installp */

/* ------------------------------------------------------------------------ *
 *
 *  Function:   inu_regenerate_cmd_line
 *
 *  Purpose:    Regenerates a command line based on the initial arguments
 *              to installp (these are in getargv -- the original argv to
 *              installp) and based on the cancelled items in the sop. This
 *              regenerated cmd line is used to exec installp.
 *
 *  Parameters: char   * getargv []  -- is the original argv to installp
 *
 *              char *** sendargv  -- is the final regenerated cmd line
 *                                    in the form of a ptr to a a ptr to an
 *                                    "argv".  Think of it as a ptr to a ptr
 *                                    to an array of ptrs to char strings:
 *
 *                                  /-->"parameter 1"
 *                               +--|------------------------------+
 *    sendargv -> *sendargv ->   | | | | | | | | | | | | | | | | | |
 *                               +|---|----------------------------+
 *                                |   |
 *                                |   \->"parameter 2"   ... and so on ...
 *                                |
 *                                \-->"command name"
 *
 *              int new_argvsize  -- is the current number of allocated
 *                                   indices in the sendargv array.
 *
 *  Returns:    nonzero  --  on failure to regenerate the cmd line
 *                           The ONLY way we should fail is if a malloc error
 *                           occurs.
 *              zero     --  on successful regeneration of cmd
 *                           line.
 *
 *
 *
 * ------------------------------------------------------------------------ */

static int
inu_regenerate_cmd_line (char   * getargv[],   /* get this argv from caller
                                                  (was orig * installp argv) */
                         char *** sendargv,    /* send this argv back to
                                                  caller (regenerated * argv) */
                         int      new_argv_size) /* the current # of indices
                                                    in the sendargv * array */
{
   char            name_lev[PATH_MAX];  /* holds a string of form: "lppname
                                         * level" */
   char           *tmptr;       /* intermediate str ptr to hold a string.  */
   Option_t       *tmpop;       /* used to traverse the sop                */
   int             i;
   int             name_len;    /* length of "lppname level" string generated */

   /** ------------------------------------------------------------------ *
    ** The format of installp's argv is  "flags lpps".  The lpp_optind
    ** variable (which is set in inu_instl_args) is set to the argv index
    ** of the 1st lpp.
    **
    ** This for loop will copy the flags part of the original argv -- which
    ** corresponds to the flags originally specified by the user -- into the
    ** new cmd line.
    ** ------------------------------------------------------------------ */

   for (i = 0; i < lpp_optind; i++)
   {
      if ((tmptr = (char *) malloc (strlen (getargv[i]) + 1)) == NULL)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                  CMN_MALLOC_FAILED_D), inu_cmdname);
         return (FAILURE);
      }

      strcpy (tmptr, getargv[i]);
      (*sendargv)[i] = tmptr;

      /** ------------------------------------------------------------------ *
       **  See if we need to allocate more space for the regenerated cmdline
       ** ------------------------------------------------------------------ */

      if (i >= new_argv_size - 1)
      {
         new_argv_size += 50;
         if (((*sendargv) = (char **) realloc ((*sendargv), 
                             sizeof (char *) * new_argv_size)) == NIL (char *))
         {
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_REALLOC_FAILED_E, 
                                     CMN_REALLOC_FAILED_D), inu_cmdname);
            return (FAILURE);
         }
      }
   } /* end for */


   /** ----------------------------------------------------------- *
    **  We always want to pass the -q flag to the next exec.
    ** ----------------------------------------------------------- */

   if ( ! (q_FLAG))
   {
      if ((tmptr = (char *) malloc (strlen ("-q") + 1)) == NULL)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                  CMN_MALLOC_FAILED_D), inu_cmdname);
         return (FAILURE);
      }

      strcpy (tmptr, "-q");
      (*sendargv)[i++] = tmptr;
   }

   /** ------------------------------------------------------------------ *
    **  Only add lpps that were CANCELLED to the regenerated cmdline.
    **  We do this by looping through the sop and searching for options
    **  marked as cancelled (their Status field will be STAT_CANCEL).
    **  When we find a cancelled option, put it in the new cmd line
    **  we're generating -- sendargv.
    ** ------------------------------------------------------------------ */

   for (tmpop = SopPtr->next; tmpop != NIL (Option_t); tmpop = tmpop->next)
   {
      if (tmpop->Status == STAT_CANCEL)
      {
         /** -------------------------------------------------------------- *
          **  Since we allocated a static amount of space to our array of
          **  char ptrs (sendargv), we need to check if we need to reallocate
          **  more space.
          ** -------------------------------------------------------------- */

         if (i >= new_argv_size - 1)
         {
            new_argv_size += 50;
            if (((*sendargv) = (char **) realloc ((*sendargv), 
                                         sizeof (char *) * new_argv_size)) ==
                                                                  NIL (char *))
            {
               instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_REALLOC_FAILED_E, 
                                        CMN_REALLOC_FAILED_D), inu_cmdname);
               return (FAILURE);
            }
         }

         inu_get_optlevname_from_Option_t (tmpop, name_lev);

         if ((name_len = strlen (name_lev)) == 0)
            continue;

         if ((tmptr = (char *) malloc (name_len + 1)) == NULL)
         {
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                     CMN_MALLOC_FAILED_D), inu_cmdname);
            return (FAILURE);
         }

         strcpy (tmptr, name_lev);
         (*sendargv)[i++] = tmptr;

      }                         /* if */
   }                            /* for */

   /** ------------------------------------------------------------------ *
    **  By convention, the last ptr in an argv array should be NULL.
    ** ------------------------------------------------------------------ */

   if (i == new_argv_size)
   {
      new_argv_size += 1;
      if (((*sendargv) = (char **) realloc ((*sendargv), 
                                          sizeof (char *) * new_argv_size)) ==
                                                                  NIL (char *))
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_REALLOC_FAILED_E, 
                                  CMN_REALLOC_FAILED_D), inu_cmdname);
         return (FAILURE);
      }
   }

   (*sendargv)[i] = NIL (char);

   return (INUGOOD);

} /* end inu_regenerate_cmd_line */
