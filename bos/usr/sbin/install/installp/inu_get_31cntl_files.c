static char sccsid[] = "@(#)91  1.25  src/bos/usr/sbin/install/installp/inu_get_31cntl_files.c, cmdinstl, bos411, 9428A410j 5/31/94 08:55:35";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_get_31cntl_files
 *		inu_unarchive
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

#include <installp.h>
#include <instl_options.h>

/* NAME: inu_get_31cntl_files
 *
 * FUNCTION: extracts the control files from version 3.1 updates
 *
 * RETURN VALUE DESCRIPTION:
 *
 * SUCCESS    = if all the needed arp files are restored FAILURE    = if there
 * was an error at any point */

#define BUF_SIZE    256

int inu_get_31cntl_files 
(Option_t *sop,       /* pointer to the first option in the 3.1 update */
 Option_t *nxt_op)    /* pointer to the first option in the next operation */
{
   int  arp_size,             /* Size for arp files */
        New_format,           /* Update image is in new format */
        rc;                   /* return code */
   char       buf [BUF_SIZE],     /* i/o buffer */
           device [PATH_MAX + 1], /* The device to be used for this bff. */
          lpp_dir [PATH_MAX + 1], /* pathname of /usr/lpp/LPPNAME */
        arp_files [MAX_LPP_FULLNAME_LEN * MAX_UPDATES], /* used by restore */
      current_lpp [MAX_LPP_OPTIONNAME_LEN + 1], /* prior top lpp processed */
          lppname [MAX_LPP_OPTIONNAME_LEN + 1], /* The top lpp name */
     option_level [MAX_LEVEL_LEN + 1], /* lpp level in req_level file. */
      option_name [MAX_LPP_FULLNAME_LEN + 1], /* lpp name in req_level file */
         req_path [PATH_MAX + 1], /* path to the req_level file */
         req_line [MAX_LPP_FULLNAME_LEN + MAX_LEVEL_LEN + 2]; /* one line 
                                                    from the req_level file */
   char     *p;                    /* temp char pointer */
   FILE     *lpp_fp;               /* file pointer for lpp files */
   FILE     *size_fp;              /* file pointer for arp size files */
   FILE     *req_fp;               /* file pointer to req_level file */
   Option_t *ptr;                  /* pointer for the "for" loops */
   Option_t *last_op;              /* pointer to the last op to process */
   Option_t *op_ptr;               /* copy of the pointer passed in for use */
   Level_t   req_level;            /* level struct for the required level. */

   /* Begin inu_get_31cntl_files */

   INU_ABORT_SEV = 1;
   arp_size = 0;
   current_lpp[0] = '\0';
   strcpy (device, sop->bff->path);
   op_ptr = sop;
   New_format = 0;
   p = "";

   /* Create /usr/sys/inst_updt directory if it doesn't */

   if (access ("/usr/sys", F_OK) != 0)
   {
      rc = mkdir ("/usr/sys", (mode_t) 0777);

      if (rc != SUCCESS)
      { /* then we can not continue */

         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                                      CMN_NO_MK_DIR_D), "installp", "/usr/sys");
         return (FAILURE);
      } /* end if mkdir failed */

   } /* end if no /usr/sys */

   if (access ("/usr/sys/inst_updt", F_OK) != 0)
   {
      rc = mkdir ("/usr/sys/inst_updt", (mode_t) 0777);
      if (rc != SUCCESS)
      { /* then we can not continue */

         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_DIR_E, 
                           CMN_NO_MK_DIR_D), "installp", "/usr/sys/inst_updt");
         return (FAILURE);
      } /* end if mkdir failed */

   } /* end if no /usr/sys/inst_updt */

   /* cd to working directory */

   if (chdir ("/usr/sys/inst_updt") != 0)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                               CMN_CANT_CHDIR_D), 
               "installp", "/usr/sys/inst_updt");
      return (INUACCS);
   }

   /* restore control files */

   rc = inu_restore (device, TRUE, TocPtr->type, sop, 
                 "./copyright ./lpp_name ./service_num ./special ./req_level");
   if (rc != 0)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_RESTORE_E, 
                               INP_BAD_RESTORE_D));

      if (rc == EFBIG  ||  rc == ENOSPC)
         return (INUFULLD);
      else
         return (INURESTR);

   }    /* end if (restore fails) */

   /* check the req_level file against the list of SOP */

   strcpy (req_path, "/usr/sys/inst_updt");
   strcat (req_path, "/req_level");

   if ((req_fp = fopen (req_path, "r")) != NIL (FILE))
   {
      /* then process the file */

      while ((fgets (req_line, (MAX_LPP_FULLNAME_LEN + MAX_LEVEL_LEN + 2), 
                     req_fp)) != NIL (char))
      { /* the req_level file is empty */

         rc = sscanf (req_line, "% (MAX_LPP_FULLNAME_LEN)s % (MAX_LEVEL_LEN)s", 
                      option_name, option_level);
         if (rc != EOF)
         {  /* then we will continue */

            inu_31level_convert (option_level, &req_level);

            for (ptr = op_ptr;
                 ((ptr != NIL (Option_t))  &&  (ptr->bff == op_ptr->bff));
                 ptr = ptr->next)
            {
               if (strcmp (ptr->name, option_name) == 0)
               {
                  /* then compare the level of this lpp with req. */

                  if (req_level.mod > ptr->level.sys_mod)
                  {
                     instl_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, 
                                          CMN_LEVEL_NUM_E, CMN_LEVEL_NUM_D), 
                              INU_INSTALLP_CMD, option_name);

                     rc = inu_del_op (sop, nxt_op, ptr);
                     if (rc != SUCCESS)
                     {  /* then error and exit */

                        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
                                             CMN_SYSCOM_E, CMN_SYSCOM_D), 
                                             "installp", "inu_del_op ()");
                        return (FAILURE);
                     }  /* end if inu_del_failed */
                     else
                        continue;
                  } /* end if req_level.mod check */

                  if ((req_level.mod == ptr->level.sys_mod)
                                       && 
                      (req_level.fix > ptr->level.sys_fix))
                  {  /* error */

                     instl_msg (WARN_MSG, MSGSTR (MS_INUCOMMON, 
                                          CMN_LEVEL_NUM_E, CMN_LEVEL_NUM_D), 
                                          INU_INSTALLP_CMD, option_name);

                     rc = inu_del_op (sop, nxt_op, ptr);
                     if (rc != SUCCESS)
                     {  /* then error and exit */

                        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
                                             CMN_SYSCOM_E, CMN_SYSCOM_D), 
                                             "installp", "inu_del_op ()");
                        return (FAILURE);
                     }  /* end if inu_del_failed */
                     else
                        continue;

                  } /* end if (fix level also too low) */

               } /* end if the option_name matched */

            } /* end for (loop the the op list) */

            /* end else (the level was a good one) */

         } /* end if (req_line was not the EOF) */

      } /* end while (the req_level file is not empty) */

      fclose (req_fp);

   } /* end if there is a req_level file */

   /* check the special file */

   /* For version 3.2 of installp the checking of the special file was left
    * out.  The logic for doing this is that the code was added to 3.1 for
    * compatibility with 5A.  The problem is that the code did not do any
    * thing but cancel the install if there were 2 lpps in the list that have
    * RAS or CONFIG (both undefined) types.  The documentation for the file
    * and the actions on the file did not match and were not processed
    * correctly.  For theses reasons all the "special" file processing code is
    * being excluded for 3.2. */

   /* determine if this is a new format update image */

   if ((lpp_fp = fopen ("./lpp_name", "r")) == NULL)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_FL_FORMAT_E, 
                                   CMN_BAD_FL_FORMAT_D), INU_INSTALLP_CMD, 
                                   "/usr/sys/inst_updt/lpp_name");
      return (INUNOLPP);
   }

   while (*p == '\0')
   {
      if (fgets (buf, BUF_SIZE, lpp_fp) == NULL)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_FL_FORMAT_E, 
                                      CMN_BAD_FL_FORMAT_D), INU_INSTALLP_CMD, 
                                      "/usr/sys/inst_updt/lpp_name");
         fclose (lpp_fp);
         return (INUNOLPP);
      }

      for (p = buf;
           (*p == ' ')  &&  *p;
           p++)
      {
        ;
      }

      if (*p == '2')
         New_format = 1;
   }
   fclose (lpp_fp);

   /* Restore arpsize files:  Only if New_format */

   if (New_format)
   {
      if (chdir ("/") == -1)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                                      CMN_CANT_CHDIR_D), lpp_dir);
         return (INUACCS);
      } /* end if (can not chdir) */

      for (ptr = op_ptr;
           ((ptr != NIL (Option_t))  &&  (ptr->bff == op_ptr->bff));
           ptr = ptr->next)
      {
         inu_get_prodname (ptr, lppname);
         strcpy (arp_files, "./usr/lpp/");
         strcat (arp_files, lppname);
         strcat (arp_files, "/inst_updt/arpsize ");
      } /* end for (cycle throught the lpps) */

      rc = inu_restore (device, TRUE, TocPtr->type, sop, arp_files);

      if (rc != 0)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_RESTORE_E, 
                                                   INP_BAD_RESTORE_D));

         if (rc == EFBIG  ||  rc == ENOSPC)
            return (INUFULLD);
         else
            return (INURESTR);

      } /* end if (restore fails) */

      if (rc != 0)
      {
         instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_RESTORE_E, 
                                                   INP_BAD_RESTORE_D));

         if (rc == EFBIG  ||  rc == ENOSPC)
            return (INUFULLD);
         else
            return (INURESTR);

      } /* end if (restore fails) */

      /* Use the sizes given in the arpsize files:  Only if New_format */

      for (ptr = op_ptr;
           ((ptr != NIL (Option_t))  &&  (ptr->bff == op_ptr->bff));
           ptr = ptr->next)
      {
         inu_get_prodname (ptr, lppname);
         strcpy (arp_files, "./usr/lpp/");
         strcat (arp_files, lppname);
         strcat (arp_files, "/inst_updt/arpsize");

         if ((size_fp = fopen (arp_files, "r")) == NULL)
         {
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_FL_FORMAT_E, 
                                         CMN_BAD_FL_FORMAT_D), 
                                         INU_INSTALLP_CMD, arp_files);
            return (INUNOLPP);
         } /* end if (fopen fails) */

         p = "";
         while (*p == '\0')
         {
            if (fgets (buf, BUF_SIZE, size_fp) == NULL)
            {
               instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                    CMN_CANT_OPEN_D), "installp", arp_files);
               fclose (size_fp);
               return (INUNOLPP);
            } /* end if (fgets fails) */

            for (p = buf;
                 (*p == ' ')  &&  *p;
                 p++)
            {
               ;
            }

            if (*p != '\0')
               arp_size += atoi (p);

         } /* end while */

         fclose (size_fp);

      } /* end for (cycle through the SOP) */

      /* Check size requirements for arp files: Only if New_format */
      arp_size = arp_size / 2;
      if (inu_get_min_space ("/usr/lpp", arp_size) == FAILURE)
         return (INUNOSPC); /* error msg is displayed in inu_get_min_space()*/

   } /* end if New_format */

   /* Find the last option in the list to be processed for the -A and -i
    * options. */

   if (A_FLAG || i_FLAG)
   {
      for (ptr = op_ptr;
           ((ptr != NIL (Option_t))  &&  (ptr->bff == op_ptr->bff)) ;
           ptr = ptr->next)
      {
         ;
      }

      last_op = ptr;
   } /* end if (A_FLAG or i_FLAG) */

   /* Restore arp files */

   if (chdir ("/") == -1)
   {    /* all files are backed up relative to root "/" */
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                               CMN_CANT_CHDIR_D), 
               lpp_dir);
      return (INUACCS);
   } /* end if (can not chdir) */

   for (ptr = op_ptr;
        ((ptr != NIL (Option_t))  &&  (ptr->bff == op_ptr->bff));
        ptr = ptr->next)
   {
      inu_get_prodname (ptr, lppname);
      strcpy (arp_files, "./usr/lpp/");
      strcat (arp_files, lppname);
      strcat (arp_files, "/inst_updt/arp ");

   } /* end for (cycle through the SOP) */

   rc = inu_restore (device, TRUE, TocPtr->type, sop, arp_files);

   if (rc != 0)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BAD_RESTORE_E, 
                                                INP_BAD_RESTORE_D));
      if (rc == EFBIG  ||  rc == ENOSPC)
         return (INUFULLD);
      else
         return (INURESTR);
   } /* end if (inu_restore fails) */

   for (ptr = op_ptr;
        ((ptr != NIL (Option_t))  &&  (ptr->bff == op_ptr->bff));
        ptr = ptr->next)
   {
      inu_get_prodname (ptr, lppname);

      /* make sure we are not doing the same lpp twice */

      if (strcmp (current_lpp, lppname) != 0)
      { /* then different lpp */

         strcpy (current_lpp, lppname); /* set this lpp to "current_lpp" */

         rc = inu_unarchive (lppname);

         if (rc != SUCCESS)
         {      /* then the unarchive did not work */

            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_SYSCOM_E, 
                                 CMN_SYSCOM_D), "installp", "ar");
            return (FAILURE);
         } /* end if (ar failed) */

         strcpy (lpp_dir, "/usr/lpp/");
         strcat (lpp_dir, lppname);

         if (chdir (lpp_dir) == -1)
         {
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                                         CMN_CANT_CHDIR_D), lpp_dir);
            return (INUACCS);
         } /* end if (can not chdir) */

      } /* end if (this is a new lpp) */

   } /* end for (loop through the lpps) */

   return (SUCCESS);

} /* end inu_get_31cntl_files */

/* ------------------------------------------------------------------------ */

/* NAME: inu_unarchive
 *
 * FUNCTION: extract archived files from /usr/lpp/LPPNAME/inst_updt/arp
 *
 * RETURN VALUE DESCRIPTION: NONE */

/* DEFINE all the archive commands */

#define AR_INSTR "/usr/ccs/bin/ar x arp"
#define AR_APAR  "/usr/ccs/bin/ar x arp fixinfo"
#define AR_ALL   "/usr/ccs/bin/ar x arp"

int inu_unarchive (char *lpp_name)  /* Name of parent LPP   */
{
   char            inst_updt_path[MAX_LPP_NAME + 21];   /* pathname of work
                                                         * directory */
   int             rc;  /* return code from system call */

   /* Begin inu_unarchive  */

   (void) strcpy (inst_updt_path, "/usr/lpp/");
   (void) strcat (inst_updt_path, lpp_name);
   (void) strcat (inst_updt_path, "/inst_updt");

   rc = chdir (inst_updt_path);
   if (rc != SUCCESS)
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E, 
                                   CMN_CANT_CHDIR_D), inst_updt_path);
      return (INUACCS);
   } /* end if (can not chdir) */

   /* extract files    */

   if (i_FLAG)  /* then only unarchive the lpp.instr member */
      rc = inu_do_exec (AR_INSTR);

   if (rc != SUCCESS)
      return (rc);

   if (A_FLAG)  /* then only unarchive the lpp.instr member */
      rc = inu_do_exec (AR_APAR);

   if (rc != SUCCESS)
      return (rc);

   if ( ! (i_FLAG && A_FLAG))  /* then unarchive all the members */
      rc = inu_do_exec (AR_ALL);

   if (rc != SUCCESS)
      return (rc);

   /* remove arp file  */

   (void) inu_rm ("arp");

   return (SUCCESS);

} /* end inu_unarchive */
