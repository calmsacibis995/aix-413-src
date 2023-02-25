static char sccsid[] = "@(#)61  1.9.2.33  src/bos/usr/sbin/install/installp/inu_vpd_lpp_prod.c, cmdinstl, bos41J, 9510A_all 2/27/95 12:14:47";

/*--------------------------------------------------------------------------
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: inu_vpd_lpp_prod, inu_ck_vpd_sys_rc, get_corr_svn, 
 *            get_prereq_file, get_fixinfo_file, get_productid_file, 
 *            inu_vpd_ch_sceded_by, 
 *            inu_set_supersedes_field, inu_add_supersedes, 
 *            inu_vpd_add_sceded_by, inu_blast_avail_rex_w_null_prereq, 
 *            inu_get_ptfs_to_rm_from_vpd, inu_set_root_cp_flag
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *-------------------------------------------------------------------------*/

#include <installp.h>
#include <instl_options.h>
#include <inu_vpd.h>
#include <stdarg.h>

#define CREATED          TRUE
#define INU_STR_INC_SIZE 1024   /* unit allocation size for returned string */

static void    inu_vpd_add_sceded_by             (Option_t *, char *);
static void    inu_blast_avail_rex_w_null_prereq (char *, char *);
static char  * inu_set_supersedes_field          (Option_t * sop);
static char  * inu_get_ptfs_to_rm_from_vpd       (char * , char *);
static short   inu_set_root_cp_flag              (Option_t *, char *);
static void    set_search_state                  (int, prod_t *, Option_t *);
static void    update_sceded_by_field            (prod_t, Option_t *, int);

extern char  * inu_cmdname;

void inu_rm_vpd_info	(Option_t *);



/*==========================================================================*
**
** Function:     inu_vpd_lpp_prod ()
**
** Description:  Set the state of the LPPs being processed.
**
** NOTES:        The state parameter refers to the state that's valid for 
**               the LPP and PRODUCT databases.
**
** ON ENTRY:     state   specifies the state to be set (comes from constants
**                       defined in /usr/include/swvpd.h)
**
** Returns:      None
**
**==========================================================================*/

void inu_vpd_lpp_prod (Option_t * op, int state)
{
   lpp_t           lpp;
   prod_t          prod;
   int             vpdrc;  
   char            prodname [MAX_LPP_FULLNAME_LEN];
   boolean         created_root_lpp_rec = FALSE;

   memset (&lpp,  NULL, sizeof (lpp_t));
   memset (&prod, NULL, sizeof (prod_t));


  /*-----------------*
   * LPP DATABASE
   *-----------------------------------------------------------------------*
   * The lpp database isn't set with the "ing" states.  However, this is
   * where the lpp install record gets created (updates do not have records 
   * in the lpp table). 
   *-----------------------------------------------------------------------*/
   if (state != ST_COMMITTING  &&  state != ST_REJECTING  &&
       state != ST_DEINSTALLING  &&  state != ST_AVAILABLE)
   {
      if (IF_INSTALL (op->op_type))
      {
         if (state == ST_APPLYING)
            update_lpp_table (op, ST_AVAILABLE);
         else
            update_lpp_table (op, state);
      }

     /*-----------------------------------------------------------------*
      * When processing a ROOT update we must make sure there's already
      * an LPP record (representing the corresponding base level) which
      * has been committed.
      *-----------------------------------------------------------------*/
      else if ((state == ST_APPLYING) && (op->vpd_tree == VPDTREE_ROOT))
      {
        /*--------------------------*
         * Establish search criteria
         *--------------------------*/
         strcpy (lpp.name, op->name);

        /*---------------------------------------------------------*
         * If there's no LPP record for op, add an available record
         *---------------------------------------------------------*/
         if ((vpdrc = vpdget (LPP_TABLE, LPP_NAME, &lpp)) == VPD_NOMATCH)
         {
            update_lpp_table (op, ST_AVAILABLE);
            created_root_lpp_rec = TRUE;
         }
         else
            inu_ck_vpd_sys_rc (vpdrc);

      }
     /*-----------------------------------------------------------------*
      * If applying an oem-specific fileset, mark the LPP record as
      * being OEM modified.
      *-----------------------------------------------------------------*/
      if ((state == ST_APPLYING) &&
		(op->lang[0] == 'o') && (!strncmp(op->lang, "oem_", 4)))
      {
        /*--------------------------*
         * Search for LPP record
         *--------------------------*/
         strcpy (lpp.name, op->name);
         vpdrc = vpdget (LPP_TABLE, LPP_NAME, &lpp);
         inu_ck_vpd_sys_rc (vpdrc);

        /*-------------------------------*
         * Update LPP record if necessary
         *-------------------------------*/
	 if ((vpdrc != VPD_NOMATCH) && (!(lpp.cp_flag & LPP_OEM_SPECIFIC)))
	 {
	     lpp.cp_flag |= LPP_OEM_SPECIFIC;
	     vpdrc = vpdchgget (LPP_TABLE, &lpp);
	     inu_ck_vpd_ok (vpdrc,
		     "\tvpdchgget LPP_TABLE.name=%s vpderrno=%d odmerrno=%d\n",
		     lpp.name, vpdrc, odmerrno);
	 }
      }

   } /* if state not ST_COMMITTING, ST_REJECTING, or ST_DEINSTALLING */


  /*-----------------*
   * PRODUCT DATABASE
   *-----------------------------------------------------------------------*
   * Change the state value in the PRODUCT database to what we were passed
   * (the parameter called state). 
   *-----------------------------------------------------------------------*/
   if (state == ST_APPLYING)
   {
      if (IF_UPDATE (op->op_type)  &&  op->vpd_tree == VPDTREE_ROOT)
      {
        /*-----------------------------------------------------------------*
         **  Ensure that we have supersedes info for the root part of 3.2
         **  updates, so that we can update the sceded_by field.  Supersedes
         **  info was not in the VPD in 3.2.  Don't need to do this for 4.1
         **  and later installs cuz the supersedes info (barrier info) is
         **  stored in the VPD.
         ** -----------------------------------------------------------------*/
         if (IF_3_2 (op->op_type))
            op->supersedes = inu_set_supersedes_field (op);

         if (created_root_lpp_rec)
            create_root_base_level_prod_rec (op);
      }

      create_product_record (op, state);
   }
 
   else   /* state != ST_APPLYING */
   {
      strcpy (prod.lpp_name, op->name);
      prod.ver = op->level.ver;
      prod.rel = op->level.rel;
      prod.mod = op->level.mod;
      prod.fix = op->level.fix;

      if (state == ST_DEINSTALLING) /* vpdget independent of PROD_STATE */
      {
         prod.ptf[0] = '\0';
         vpdrc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL | 
                         PROD_MOD | PROD_FIX | PROD_PTF), &prod);
      }
      else
      {
        /*-----------------------------------------------------------------*
         * Based on the state we're asked to set it to, determine what the
         * current state should be in the database.
         *-----------------------------------------------------------------*/
         strcpy (prod.ptf, op->level.ptf);
         set_search_state (state, &prod, op);

         vpdrc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_STATE | PROD_VER |
                         PROD_REL | PROD_MOD | PROD_FIX | PROD_PTF), &prod);
      }

      inu_ck_vpd_ok (vpdrc,
		"\tvpdget PRODUCT_TABLE.lpp_name=%s vpderrno=%d odmerrno=%d\n",
		prod.lpp_name, vpdrc, odmerrno);

     /*--------------------------------------------------------------------*
      * Blast AVAILABLE vpd records.  There is no use for them.
      *--------------------------------------------------------------------*/

      if (state == ST_AVAILABLE)
      {

        /*------------------------------------------------------------------*
         * If op is a base level then also delete the corresponding record
         * from the LPP, PRODUCT, and HISTORY tables.
         *------------------------------------------------------------------*/
         if (IF_INSTALL (op->op_type))
	 {
	    inu_rm_vpd_info(op);
	 }
	 else
	 {
	    hist_t          hist;

	    /* wipe out that useless history */
	    if (vpdreslpp_name (op->name, &hist.lpp_id) != VPD_NOID)
	    {
	       hist.ver = op->level.ver;
	       hist.rel = op->level.rel;
	       hist.mod = op->level.mod;
	       hist.fix = op->level.fix;
               strcpy (hist.ptf, op->level.ptf);
	       vpdrc = vpddelall (HISTORY_TABLE,
		    (HIST_LPP_ID | HIST_VER | HIST_REL | HIST_MOD | HIST_FIX |
			HIST_PTF),
		     &hist);
	    }

            if (IF_3_2(op->op_type))
               vpdrc = vpddel (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_PTF), &prod);
            else
               vpdrc = vpddel (PRODUCT_TABLE, (PROD_LPP_NAME |
                            PROD_VER | PROD_REL | PROD_MOD | PROD_FIX), &prod);

            inu_ck_vpd_ok (vpdrc,
		"\tvpddel PRODUCT_TABLE.lpp_name=%s vpderrno=%d odmerrno=%d\n",
		prod.lpp_name,vpdrc,odmerrno);
	 }
      }
      else
      {
        /*------------------------------------------------------------------*
         *  Change the product record's state value to that requested. 
         *------------------------------------------------------------------*/
         prod.state = state;

         vpdrc = vpdchgget (PRODUCT_TABLE, &prod);
         inu_ck_vpd_ok (vpdrc,
		"\tvpdchgget PRODUCT_TABLE.lpp_name=%s vpderrno=%d odmerrno=%d\n",
		prod.lpp_name,vpdrc,odmerrno);
      }

   } /* state != ST_APPLYING */

  /*------------------------------------------------------------------*
   *  For 3.2 updates, update the sceded_by field
   *------------------------------------------------------------------*/
   if (IF_3_2 (op->op_type)  &&  IF_UPDATE (op->op_type))
      update_sceded_by_field (prod, op, state);

   vpd_free_vchars (LPP_TABLE, &lpp);
   vpd_free_vchars (PRODUCT_TABLE, &prod);

} /* inu_vpd_lpp_prod */


/*==========================================================================*
**
** Function:   inu_set_supersedes_field
**
** Purpose:    Finds all the ptf ids that have been superseded by the ptf
**             specified by the sop op passed in.   It does this by querying
**             the usr product table of the vpd.
**
** Parameters: sop - is just some option node to set the supersedes info for.
**
** Note:       The work this function needs to do is typically done on root-
**             only applys (when no media is used and hence no toc is avail-
**             able).  It follows, then, that if we have no toc we don't
**             have the supersedes info.  So we need to go to the VPD to get
**             it.
**
** Returns:    A string containing all ptfs superseded by the option passed
**             in.  The string is of the form "ptf ptf ptf ..."
**
**==========================================================================*/

static char * inu_set_supersedes_field (Option_t * sop)
{
   char           *sedes_ptr;   /* string returned by this function */
   prod_t          prod;        /* for vpd queries                  */
   int             str_size,    /* current size of sedes_ptr string */
                   vpdrc;       /* return code from a vpd operation */

   str_size = INU_STR_INC_SIZE;

   /*-----------------------------------------------------------------*
    * if we already have something in the supersedes field or if this
    * is not an update, or if it's a 3.1 update, then just return
    * what's already there.
    *-----------------------------------------------------------------*/

   if (sop->supersedes != NIL (char)  || sop->level.ptf[0] == '\0'  
                 || 
       IF_3_1_UPDATE (sop->op_type))
      return (sop->supersedes);

   /*-------------------------------------------------------------------*
    * Set the currently-used vpd to be the usr vpd, if it's not already.
    *-------------------------------------------------------------------*/

   if (sop->vpd_tree != VPDTREE_USR)
      inu_set_vpdtree (VPDTREE_USR);

   if ((sedes_ptr = (char *) malloc (str_size)) == NIL (char))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                   CMN_MALLOC_FAILED_D), inu_cmdname);
      inu_quit (INUMALLOC);
   }

   strcpy (prod.sceded_by, sop->level.ptf);
   *sedes_ptr = '\0';

   /*-----------------------------------------------------------------*
    * Find everything in the usr product table of the vpd that is
    * superseded by this update.
    * Create a big string that contains all superseded ptf ids in it
    * of the form:  "ptf ptf ptf ...".
    *-----------------------------------------------------------------*/

   vpdrc = vpdget (PRODUCT_TABLE, PROD_SCEDED_BY, &prod);
   inu_ck_vpd_sys_rc (vpdrc);

   while (vpdrc == VPD_OK)
   {
      /*-----------------------------------------------------------*
       * See if we need to realloc more space for the string.
       *-----------------------------------------------------------*/

      if ((strlen (sedes_ptr) + strlen (prod.ptf) + 1)  >=  str_size)
      {
         str_size += INU_STR_INC_SIZE;
         if ((sedes_ptr = (char *) realloc (sedes_ptr, str_size)) == NIL (char))
         {
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                         CMN_MALLOC_FAILED_D), inu_cmdname);
            inu_quit (INUMALLOC);
         }
      }
      strcat (sedes_ptr, prod.ptf);
      strcat (sedes_ptr, " ");

      vpd_free_vchars (PRODUCT_TABLE, &prod);

      vpdrc = vpdgetnxt (PRODUCT_TABLE, &prod);
      inu_ck_vpd_sys_rc (vpdrc);

   } /* while */

   /*-----------------------------------------------------------------*
    * We need to set the currently used vpd back to what it was.
    *-----------------------------------------------------------------*/

   if (sop->vpd_tree != VPDTREE_USR)
      inu_set_vpdtree (sop->vpd_tree);

   return (sedes_ptr);

} /* inu_set_supersedes_field */


/*==========================================================================*
**
**  Function  -   update_sceded_by_field 
**
**  Purpose   -   Updates the sceded_by fields in the product database
**                for all options superseded by the op parameter as 
**                appropriate.
** 
**  Notes     -   This routine is only needed for 3.2 updates.
**
**  Returns   -   None
**
**==========================================================================*/

static void update_sceded_by_field (prod_t prod, Option_t *op, int state)
{
   if (! (IF_3_2(op->op_type)))
      return;

   if (IF_UPDATE (op->op_type))
   {
      switch (state)
      {
         case ST_APPLYING:
              /* Ensure that the sceded_by fields are set */
              if (op->supersedes != NULL  &&  *op->supersedes != '\0')
                 inu_add_supersedes (op);
              break;

         case ST_APPLIED:
         case ST_COMMITTING:
         case ST_COMMITTED:
              break;

         case ST_AVAILABLE:
         case ST_BROKEN:
         case ST_REJECTING:
         case ST_DEINSTALLING:
         default:                       /* remove superedes otherwise */
              inu_vpd_ch_sceded_by (prod.ptf, "", op->name);
              break;
      } 
   } 
} 


/*==========================================================================*
**
**  Function  -   set_search_state
**
**  Purpose   -   Extrapolates what state we should search the VPD for
**                based on the state we're asked to set an option to.
** 
**  Returns   -   None
**
**==========================================================================*/

static void set_search_state 

  (int state,      /* VPD state that we're gonna set an option to */
   prod_t *prod,   /* VPD search criteria */
   Option_t *op)   /* option to extrapolate state for */ 

{
   switch (state)
   {
      case ST_APPLYING:
         prod->state = ST_AVAILABLE;
         break;
      case ST_REJECTING:
         prod->state = ST_APPLIED;
         break;
      case ST_DEINSTALLING:
         /* state could be APPLIED, COMMITTED, or BROKEN so don't specify */
         /* PROD_STATE in the search criteria in vpd_get            */
         break;
      case ST_COMMITTING:
         switch (op->operation)
         {
            case OP_CLEANUP_COMMIT:
               prod->state = ST_COMMITTING;
               break;
            default:
               prod->state = ST_APPLIED;
               break;
         }
         break;
      case ST_COMMITTED:
         if (IF_INSTALL (op->op_type)  &&  
             (op->operation == OP_APPLY  ||  op->operation == OP_APPLYCOMMIT))
            prod->state = ST_APPLYING;
         else
            prod->state = ST_COMMITTING;
         break;
      case ST_APPLIED:
      case ST_AVAILABLE:
      case ST_BROKEN:
         switch (op->operation)
         {
            case OP_APPLY:
            case OP_APPLYCOMMIT:
            case OP_CLEANUP_APPLY:
               prod->state = ST_APPLYING;
               break;
            case OP_REJECT:
               prod->state = ST_REJECTING;
               break;
            case OP_DEINSTALL:
               prod->state = ST_DEINSTALLING;
               break;
            case OP_COMMIT:
            case OP_CLEANUP_COMMIT:
               prod->state = ST_COMMITTING;
               break;
         }      /* end switch (op->operation) */
         break;
   }    /* switch (state) */

} /* set_search_state */


/*==========================================================================*
**
** FUNCTION:     inu_ck_vpd_sys_rc ()
**
** DESCRIPTION:  Does an inuquit if the vpd return code passed in is a 
**               VPD_SYS error or a VPD_SQLMAX error (a serious error).
**
** RETURNS:      None.
**
**==========================================================================*/

void inu_ck_vpd_sys_rc (int vpdrc) /* a return code from a vpdget operation */
{
   switch (vpdrc)
   {
      case VPD_SYS:      /* system error */
      case VPD_SQLMAX:
         instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, 
                                      CMN_VPDERR_D), INU_INSTALLP_CMD);
         inu_quit (INUVPDER);
         break;

      default:
         break;
   }

} /* inu_ck_vpd_sys_rc */


/*==========================================================================*
**
** FUNCTION:     inu_ck_vpd_ok ()
**
** DESCRIPTION:  Does an inuquit if the vpd return code is anything other
**               than VPD_OK.  
**
** RETURNS:      None.
**
**==========================================================================*/

void inu_ck_vpd_ok (int vpdrc, char * format, ...) /* a return code from a vpdget operation */
{
   va_list arg;
   va_start (arg, format);

   if (vpdrc != VPD_OK)
   {
      extern int inu_doing_logging;
      switch (vpdrc)
      {
         case VPD_NOMATCH:
            instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_INTRN_MISSVPDI_E, 
                                          INP_INTRN_MISSVPDI_D));
            break;

         default:
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, 
                                         CMN_VPDERR_D), INU_INSTALLP_CMD);
            break;
      }

      vfprintf(stderr, format, arg);
      if(inu_doing_logging)
      	  vfprintf(stdout, format, arg);

      va_end (arg);
      inu_quit (vpdrc);
   }
   va_end (arg);

} /* inu_ck_vpd_ok */


/*==========================================================================*
**
** NAME:      get_corr_svn ()
**
** FUNCTION:  Attempt to read in the service number from the "service_num"
**            file in INU_TMPDIR, and put it in the history record, 
**            hist->corr_svn.  If the service number file can't be opened
**            then just null terminate the hist->corr_svn field.
**
** RETURNS:   None.
**
**==========================================================================*/

void get_corr_svn (hist_t * hist)
{
   char            tmp_path[PATH_MAX];
   FILE           *fp;

   /*-----------------------------------------------------------------------*
    * If we find a service_num file, read it and put its contents in the
    * history vpd field corr_svn...
    *-----------------------------------------------------------------------*/

   strcpy (tmp_path, INU_TMPDIR);
   strcat (tmp_path, "service_num");
   if ((fp = fopen (tmp_path, "r")) != NIL (FILE))
   {
      if ((fscanf (fp, "%s", hist->corr_svn)) == EOF)
         hist->corr_svn[0] = '\0';
      (void) fclose (fp);
   }
   else
      hist->corr_svn[0] = '\0';

} /* end get_corr_svn */


/*==========================================================================*
**
** NAME: get_prereq_file ()
**
** FUNCTION:
**
**==========================================================================*/

void get_prereq_file (Option_t * op, prod_t   * prod)
{
   char            tmp_path[PATH_MAX];
   FILE           *fp;

   /*-----------------------------------------------------------------------*
    * Try to find a file called "prereq" if that doesn't exist, then try to
    * find a file called "<lpp-option>.prereq" else terminate.
    *-----------------------------------------------------------------------*/

   strcpy (tmp_path, "prereq");
   if (access (tmp_path, R_OK) != 0)
   {
      strcpy (tmp_path, op->name);
      strcat (tmp_path, ".prereq");
      if (access (tmp_path, R_OK) != 0)
      {
         prod->prereq = strdup (NullString);
         return;
      }
   }

   /*-----------------------------------------------------------------------*
    * We found a file so open it...
    *-----------------------------------------------------------------------*/

   if ((fp = fopen (tmp_path, "r")) == NIL (FILE))
   {
      prod->prereq = strdup (NullString);
      return;
   }

   /*-----------------------------------------------------------------------*
    * Call inu_read_file () to put the prereq file into a memory buffer.
    *-----------------------------------------------------------------------*/

   if (inu_read_file (fp, &prod->prereq) != SUCCESS)
      prod->prereq = strdup (NullString);
   fclose (fp);

}  /* get_prereq_file */


/*==========================================================================*
**
** Function  -   get_fixinfo_file ()
**
** Purpose   -   Creates the fixinfo field for the given entry in the product
**               database.  
**
** Returns   -   None.
**
**==========================================================================*/

void get_fixinfo_file (Option_t * op, prod_t   * prod)
{
   int         bytes_needed;
   char      * fix_info;
   FILE      * fp;

  /*-----------------------------------------------------------------------*
   * Try to open the fixinfo file, if we can't open it then just continue...
   *-----------------------------------------------------------------------*/

   if ((fp = fopen ("fixinfo", "r")) == NIL (FILE))
      fix_info = strdup (NullString);
   else
   {
     /*-------------------------------------------------------------------*
      * Call inu_read_file () to put the fixinfo file into a memory buffer.
      *-------------------------------------------------------------------*/

      if (inu_read_file (fp, &fix_info) != SUCCESS)
         fix_info = strdup (NullString);
      fclose (fp);
   }

   prod->fixinfo = fix_info;

}  /* get_fixinfo_file */


/*==========================================================================*
**
** NAME: get_productid_file ()
**
** FUNCTION:
**
**==========================================================================*/

void get_productid_file (Option_t * op, prod_t   * prod)
{
   FILE           *fp;
   char            prodname[MAX_LPP_FULLNAME_LEN];

   prod->comp_id[0] = '\0';
   prod->fesn[0]    = '\0';

   fp = fopen ("productid", "r");       /* look for comp id */
   if (fp != NIL (FILE))
   {
      (void) fscanf (fp, "%s %s %s", prodname, prod->comp_id, prod->fesn);
      (void) fclose (fp);
   }

} /* get_productid_file */


/*==========================================================================*
 *
 * Function:  inu_get_ptfs_to_rm_from_vpd
 *
 * Purpose:   Finds all ptf ids in the vpd that have been superseded by
 *            the ptf id specified by the sceded_by paramter.
 *
 * Returns:   A space-delimited string of ptd ids that match the above
 *            conditions, if any are found.
 *
**==========================================================================*/

static char * inu_get_ptfs_to_rm_from_vpd (char * sceded_by, char * lpp_name)
{
   int             vpdrc;
   prod_t          prod;
   int             str_size = 0;
   char           *del_ptfs;

  /*-------------------------------------------------------------------*
   * Allocate an initial chunk of memory for our return string.
   *-------------------------------------------------------------------*/

   str_size = INU_STR_INC_SIZE;
   if ((del_ptfs = (char *) malloc (str_size)) == NIL (char))
   {
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                               CMN_MALLOC_FAILED_D), inu_cmdname);
      inu_quit (INUMALLOC);
   }
   *del_ptfs = '\0';


  /*---------------------------------------------------------------*
   * Do the 1st vpd query to find any ptfs that meet our criteria.
   *---------------------------------------------------------------*/

   prod.state = ST_AVAILABLE;

   strcpy (prod.sceded_by, sceded_by);
   strcpy (prod.lpp_name, lpp_name);
   vpdrc = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_SCEDED_BY |
                PROD_STATE, &prod);
   inu_ck_vpd_sys_rc (vpdrc);

  /*---------------------------------------------------------------*
   * While we find more matches in the vpd, store each match in the
   * del_ptf string.
   *---------------------------------------------------------------*/

   while (vpdrc == VPD_OK)
   {
      if (prod.prereq == NIL (char)  ||  *prod.prereq == '\0')
      {
        /*---------------------------------------------------------------*
         * See if our string needs more space.
         *---------------------------------------------------------------*/

         if (strlen (del_ptfs) + strlen (prod.ptf) + 1 >= str_size)
         {
            str_size += INU_STR_INC_SIZE;
            if ((del_ptfs = (char *) realloc (del_ptfs, str_size)) ==
                                                                   NIL (char))
            {
               instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                        CMN_MALLOC_FAILED_D), inu_cmdname);
               inu_quit (INUMALLOC);
            }
         }
         strcat (del_ptfs, prod.ptf);
         strcat (del_ptfs, " ");
      }
      vpd_free_vchars (PRODUCT_TABLE, &prod);
      vpdrc = vpdgetnxt (PRODUCT_TABLE, &prod);
      inu_ck_vpd_sys_rc (vpdrc);
   } /* end while */

  /*---------------------------------------------------------------*
   * If we didn't find anything in the vpd, the free our string.
   *---------------------------------------------------------------*/

   if (*del_ptfs == '\0')
   {
      free (del_ptfs);
      return NIL (char);
   }

   return del_ptfs;

} /* inu_get_ptfs_to_rm_from_vpd */


/*==========================================================================*
 *
 * Function:  inu_blast_avail_rex_w_null_prereq
 *
 * Purpose:   Gets rid of VPD records (in the current tree) that meet
 *            the following criteria:
 *               1.) the 'sceded_by' field matches the sceded_by parameter.
 *               2.) state = AVAILABLE
 *               3.) prereq field is set to NULL.
 *
 *             These records were created as dummy place-holder records when
 *             the superseding update was applied.  So, they should be
 *             removed when the superseding fix is rejected.
 *
 * Returns:    none
 *
 * Notes:
 *
**==========================================================================*/

static void inu_blast_avail_rex_w_null_prereq 
 (char * sceded_by, /* superseding ptf id to search for and remove from vpd */
 char * lpp_name)  /* Option name used to restrict search. */
{
   char        * del_ptfs,    /* space-separated list of ptf ids to be
                               * removed from vpd */
               * next_ptf,    /* current ptf id being removed from the vpd */
               * delimiter;   /* ptr to next space delimiter in del_ptfs
                               * string */
   prod_t        prod;
   int           vpdrc;

   if ((del_ptfs = inu_get_ptfs_to_rm_from_vpd (sceded_by, lpp_name)) ==
                                                                    NIL (char))
      return;

   delimiter = del_ptfs;

  /*----------------------------------------------------------*
   * While we have more vpd records to delete: delete them.
   * We're assuming the the del_ptfs string will be of the form:
   * "ptf ptf ptf " and will end with a space.
   *----------------------------------------------------------*/

   do
   {
      next_ptf = delimiter;
      if ((delimiter = strchr (next_ptf, ' ')) == NIL (char))
         break;
      *delimiter = '\0';
      ++delimiter;

     /*----------------------------------------------------------*
      * Delete the vpd record that corresponds to the ptf id.
      *----------------------------------------------------------*/

      strcpy (prod.ptf, next_ptf);
      strcpy (prod.lpp_name, lpp_name);
      vpdrc = vpddel (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_PTF), &prod);
      inu_ck_vpd_sys_rc (vpdrc);

   } while (vpdrc == VPD_OK);

   if (del_ptfs)
      free (del_ptfs);

} /* end inu_blast_avail_rex_w_null_prereq */


/*==========================================================================*
 *
 * Function:    inu_set_root_cp_flag
 *
 * Purpose:     Determines what the cp_flag should be for a root option
 *              of a superseded update.  This is not as straight-forward as
 *              it first may seem, because we may have NO information about
 *              a superseded update at all (in the VPD nor on the media).
 *              In this case we have to just assume that only a usr part
 *              exists.  This is really a design hole, so we have to minimize
 *              the damage.
 *
 * Returns:     The value that the cp_flag for the root part of the
 *              superseded update.
 *
 * Notes:       We know the following from context:
 *               -- op has a root part that is not APPLIED (and maybe
 *                  non-existant).
 *               -- op supersedes the ptf id specified by seded_ptf.
 *
 *
**==========================================================================*/

static short inu_set_root_cp_flag 
 (Option_t * op,          /* Option specifying the superseding update. */
 char     * seded_ptf)   /* ptf id of the superseded update */
{
   prod_t          prod;
   short           cp_flag = 0;
   int             vpdrc;

   /*--------------------------------------------------------*
    * If the superseded update has a usr vpd record, then
    *    use the cp_flag from it.
    * Else, set the cp_flag to indicate a usr-only part.
    *--------------------------------------------------------*/

   strcpy (prod.lpp_name, op->name);
   prod.ver = op->level.ver;
   prod.rel = op->level.rel;
   prod.mod = op->level.mod;
   prod.fix = op->level.fix;
   strcpy (prod.ptf, seded_ptf);

  /*-------------------------------------------------------------------*
   * Set the vpd environment to read from the USR VPD.
   *-------------------------------------------------------------------*/

   inu_set_vpdtree (VPDTREE_USR);

   vpdrc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                                   PROD_MOD | PROD_FIX | PROD_PTF), &prod);
   inu_ck_vpd_sys_rc (vpdrc);

   if (vpdrc == VPD_OK)
   {
     /*-------------------------------------------------------------------*
      * If the USR part has already been superseded by some other ptf, then
      * let's not try to supersede any dummy root part by some other ptf. 
      *-------------------------------------------------------------------*/

      vpd_free_vchars (PRODUCT_TABLE, &prod);

      if ((strlen (prod.sceded_by) != 0)
                         && 
           (strcmp (prod.sceded_by, op->level.ptf) != 0))
      {
        cp_flag = 0;
      }
      else
      {
        cp_flag = (short) prod.cp_flag;
      }
   }
   else
   {
     /*------------------------------------------------------------------*
      * Assume usr part only.  This fixes the following design problem.
      * If update A's prereq is met thru a superseding update (update C), 
      * but only the root parts of A and C are applied, then how do we
      * know to reject C if the user says reject A with -g?
      *------------------------------------------------------------------*/

      cp_flag |= (short) LPP_USER;
      cp_flag |= (short) LPP_UPDATE;
      if (IF_3_1 (op->op_type))
         cp_flag |= (short) LPP_31_FMT;
   }

  /*-----------------------------------------*
   * Set the vpd environment back to the Root
   *-----------------------------------------*/
   inu_set_vpdtree (VPDTREE_ROOT);

   return (cp_flag);

} /* end inu_set_root_cp_flag */


/*==========================================================================*
 *
 * Function:   inu_vpd_ch_sceded_by
 *
 * Purpose:    changes all vpd product records (in the current tree) that
 * currently have their 'sceded_by' field set to the value in the
 * 'sceded_by' parameter to the new value specified by the
 * 'new_sceded_by' parameter.
 *
 * Returns:    none
 *
 * Notes:      This function is indirectly called by inu_reject_opts more
 * than once on reject: once for state=ST_REJECTING and once for
 * state=ST_AVAILABLE.
 *
**==========================================================================*/

void inu_vpd_ch_sceded_by (char * sceded_by, 
                           char * new_sceded_by, 
                           char * lpp_name)
{
   int             vpdrc;
   prod_t          prod;

   if (* sceded_by == '\0')
      return;

  /*-------------------------------------------------------------*
   * Remove all VPD records in the current VPD tree that have been
   *    1.) superseded by the 'sceded_by' parameter  AND
   *    2.) which are in the AVAILABLE state         AND
   *    3.) have their prereq field set to NULL.
   * These records were created as dummy place-holder records
   * when the superseding update was applied.  So, they should
   * be removed when the superseding fix is rejected.
   *-------------------------------------------------------------*/

   inu_blast_avail_rex_w_null_prereq (sceded_by, lpp_name);

   strcpy (prod.sceded_by, sceded_by);
   strcpy (prod.lpp_name, lpp_name);

   vpdrc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_SCEDED_BY), &prod);

   inu_ck_vpd_sys_rc (vpdrc);

   while (vpdrc == VPD_OK)
   {
     /*--------------------------------------------------------------------*
      * If the record is in the ST_AVAILABLE state and it's not an 'MC' 
      * ptf, then blast this record.
      *--------------------------------------------------------------------*/

      if (prod.state == ST_AVAILABLE  &&  ! IF_LPP_PKG_PTF_TYPE_C (prod.cp_flag))
      {
         vpdrc = vpddel (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_PTF), &prod);
         inu_ck_vpd_sys_rc (vpdrc);
      }
      else
      {
         strcpy (prod.sceded_by, new_sceded_by);

         vpdrc = vpdchgget (PRODUCT_TABLE, &prod);
         inu_ck_vpd_sys_rc (vpdrc);

         vpd_free_vchars (PRODUCT_TABLE, &prod);
      }

      strcpy (prod.lpp_name, lpp_name);
      strcpy (prod.sceded_by, sceded_by);

      vpdrc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_SCEDED_BY), &prod);
      inu_ck_vpd_sys_rc (vpdrc);
   } 

} /* inu_vpd_ch_sceded_by */


/*==========================================================================*
**
** Function: inu_vpd_add_sceded_by
**
** Purpose:  Sets a value for the 'sceded_by' field for all superseded
**           updates in the current VPD tree.  If no VPD record exists
**           for a superseded update, one is created with the appropriate
**           info.
**
** Returns:  None
**
** Notes:
**
**==========================================================================*/

static void inu_vpd_add_sceded_by (Option_t * sop, 
                                   char     * sceded)
{
   int             vpdrc;
   prod_t          prod;

   bzero (&prod, sizeof (prod));
   strcpy (prod.lpp_name, sop->name);

   prod.ver = sop->level.ver;
   prod.rel = sop->level.rel;
   prod.mod = sop->level.mod;
   prod.fix = sop->level.fix;
   strcpy (prod.ptf, sceded);

  /*-------------------------------------------------------------------*
   * Ignore the fixid when looking for the superseded ptf.  This allows
   * ptfs with different fix ids to supersede one another.
   *-------------------------------------------------------------------*/
   vpdrc = vpdget (PRODUCT_TABLE, (PROD_LPP_NAME | PROD_VER | PROD_REL |
                                   PROD_MOD | PROD_PTF), &prod);
   inu_ck_vpd_sys_rc (vpdrc);

  /*---------------------------------------------------------------*
   * If no VPD record exists for this option then create a dummy
   * place-holder VPD record for this option.
   *---------------------------------------------------------------*/

   if (vpdrc != VPD_OK)
   {
      prod.state = ST_AVAILABLE;
      strcpy (prod.ptf, sceded);
      strcpy (prod.sceded_by, sop->level.ptf);
      prod.description = strdup (sop->desc);

     /*------------------------------------------------------*
      * Set the cp_flag appropriately.  If we're processing
      * the root part, we have to determine if the superseded
      * update has a root part.
      *
      * We may not be able to determine this, if it's not
      * applied and not on the media (in the toc).
      *------------------------------------------------------*/

      if (sop->vpd_tree == VPDTREE_ROOT)
         prod.cp_flag = inu_set_root_cp_flag (sop, sceded);
      else
      {
         prod.cp_flag = (short) 0;

         if (sop->vpd_tree == VPDTREE_USR)
            prod.cp_flag |= (short) LPP_USER;

         else if (sop->vpd_tree == VPDTREE_SHARE)
            prod.cp_flag |= (short) LPP_SHARE;

         prod.cp_flag |= (short) LPP_UPDATE;

         if (IF_3_1 (sop->op_type))
            prod.cp_flag  |=  (short) LPP_31_FMT;

         else if (IF_3_2 (sop->op_type))
            prod.cp_flag  |=  (short) LPP_32_FMT;

         else if (IF_4_1 (sop->op_type))
            prod.cp_flag  |=  (short) LPP_41_FMT;
      }

     /** -------------------------------------------------------------- *
      **  We should always be dealing with an update in this routine.
      ** -------------------------------------------------------------- */

      prod.update = INU_TRUE;

     /*-----------------------------------------------------------------*
      * If the cp_flag returned by inu_set_root_cp_flag is zero, then
      * we should not create a dummy ROOT part because the USR part has
      * been superseded by somebody else. 
      *-----------------------------------------------------------------*/

      if (prod.cp_flag != 0)
         vpdrc = vpdadd (PRODUCT_TABLE, &prod);
   }
   else
   {
     /*---------------------------------------------------------------*
      * Else a record already exists for this option.  So, we wanna
      * just set the sceded_by field.
      *---------------------------------------------------------------*/

      if (*prod.sceded_by == '\0')
      {
         strcpy (prod.sceded_by, sop->level.ptf);
         vpdrc = vpdchgget (PRODUCT_TABLE, &prod);
      } /* end if */

   } /* end if */

   inu_ck_vpd_sys_rc (vpdrc);
   vpd_free_vchars (PRODUCT_TABLE, &prod);

} /* inu_vpd_add_sceded_by */


/*==========================================================================*
**
** Function: inu_add_supersedes
**
** Purpose:  Main caller for routines which update the sceded_by field of
**           existing vpd records that have just been superseded, and creates
**           dummy sceded_by records for those that don't yet exist.
**
** Returns:  None
**
**==========================================================================*/

void inu_add_supersedes (Option_t * sop)
{
   FILE            fp;
   char            sceded[100];

   /* make size pointer look like a file */
   fp._flag = (_IOREAD | _IONOFD);
   fp._ptr  = fp._base = (unsigned char *) sop->supersedes;
   fp._cnt  = strlen (sop->supersedes);
   fp._file = _NFILE;

   while (fscanf (&fp, "%s ", sceded) != EOF)
      inu_vpd_add_sceded_by (sop, sceded);

} /* inu_add_supersedes */
