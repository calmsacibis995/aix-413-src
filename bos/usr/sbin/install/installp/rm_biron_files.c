static char sccsid[] = "@(#)23  1.18  src/bos/usr/sbin/install/installp/rm_biron_files.c, cmdinstl, bos412 10/12/94 08:33:48";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: add_to_list_to_check
 *		any_exist
 *		any_partially_installed_usrs
 *		can_rm_libdir
 *		deinstl_files_exist
 *		free_name_list
 *		is_biron_set
 *		reject_files_exist
 *		rm_all_option_files
 *		rm_apply_files
 *		rm_biron_files
 *		rm_lpp_files
 *		rm_shipped_inst_root_files
 *		rm_undo_files
 *		root_parts_exist
 *		set_vpd_tree_back
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


#include <instl_define.h>
#include <sys/file.h>
#include <signal.h>
#include <installp.h>
#include <instl_options.h>
#include <inu_vpd.h>
#include <sys/stat.h>

extern int biron;

/* ------------------------------------------------------------- *
**    L O C A L     F U N C T I O N       P R O T O T Y P E S 
** ------------------------------------------------------------- */

int can_rm_libdir              (char * dir,    Option_t *op, 
                                               Option_t *next_op);
int deinstl_files_exist        (char * dir,    char * op_name);
int reject_files_exist         (char * dir,    char * name);
int no_usr_wo_roots            (Option_t * op, Option_t * next_op);
int rm_all_option_files        (char *dir,     Option_t * op);
int rm_apply_files             (char * dir,    Option_t * op);
int rm_lpp_files               (char * dir);
int rm_shipped_inst_root_files (char * dir,    char * op_name);
int rm_undo_files              (char * dir,    Option_t * op);
void add_to_list_to_check      (struct name_list * head, prod_t prod);
void set_vpd_tree_back         (Option_t *op);
void free_name_list            (struct name_list *head);


/** GLOBAL to this module **/

struct name_list 
{
   char              optname[MAX_LPP_FULLNAME_LEN];
   Level_t           level;
   struct name_list *next;
};

/* ---------------------------------------------------------------- *
**
**  Function    -   rm_biron_files
**
**  Purpose     -   Removes files that were needed to support
**                  an installp operation (apply, commit, or reject), 
**                  but no longer need to exist.
**
**  Returns     -   None
**
** ---------------------------------------------------------------- */

void rm_biron_files (Option_t *op, Option_t *next_op, int operation)
{
   Option_t *tp;
   struct stat sbuf;
   char   path [PATH_MAX+1];
   char   dir [PATH_MAX+1];

  /* --------------------------------------------------------- *
  **  If we're operating on a root part and the BIRON variable
  **  isn't set, then we don't wanna remove any root files.
  **  
  **  If we're dealing with a C update (cume) then there 
  **  should be no files to cleanup. 
  ** --------------------------------------------------------- */

   if ((op->vpd_tree == VPDTREE_ROOT  &&  biron == FALSE)
                           || 
       (IF_C_UPDT (op->op_type)))
      return;

   switch (operation)
   {
      case OP_APPLY:
          /** ---------------------------------------------------- *
           **  For usr/share parts, remove files needed for apply. 
           **  For root parts, additionally remove the delivered
           **  inst_root files.
           ** ---------------------------------------------------- */
           for (tp = op; tp != next_op; tp = tp->next)
           {
              if (op->vpd_tree == VPDTREE_ROOT)
              {
                  if (tp->Status != STAT_SUCCESS)
                     continue;
                  (void) rm_shipped_inst_root_files (INU_LIBDIR, tp->name);
              }

             /** ------------------------------------------------ *
              **  Remove the files used during the apply.
              ** ------------------------------------------------ */
              (void) rm_apply_files (INU_LIBDIR, tp);

              if (IF_INSTALL (tp->op_type))
              {
                 sprintf (path, "%s/%s.unconfig_d", INU_LIBDIR, 
                                 tp->name);
                 if (stat (path, &sbuf) == 0)
                    (void) rm_undo_files (INU_LIBDIR, tp); 
              }
           }
           break;

      case OP_COMMIT:
          /** ------------------------------------------------ *
           **  If we can blast the entire libdir, do it.
           **  Else remove what we can. 
           ** ------------------------------------------------ */
           if (can_rm_libdir (INU_LIBDIR, op, next_op))
           {
              chdir ("..");
              inu_rm (INU_LIBDIR);
              break;
           }

           for (tp=op; tp !=next_op; tp=tp->next)
           {
              if (IF_UPDATE (tp->op_type))
                 (void) rm_undo_files (INU_LIBDIR, tp);
              sprintf (path, "%s/%s.inventory", INU_LIBDIR, tp->name);
              unlink (path);
           }

           if (nothing_still_installed (op, ST_APPLIED))
              (void) rm_lpp_files (INU_LIBDIR);           
           break;


      case OP_CLEANUP_APPLY:  /* really for any cleanup */
      case OP_REJECT:
          /** ------------------------------------------------------ *
           **  Don't wanna blast anything if rejecting a root part, 
           **  cuz it's needed for root apply.
           ** 
           **  If none of op's sister options (if any) are in 
           **  the APPLIED or COMMITTED state, then blast the libdir.
           **  Else remove what we can.
           ** ------------------------------------------------ */

           if ( ! (op->vpd_tree == VPDTREE_ROOT))
           {
              if (nothing_still_installed (op, ST_APPLIED)  &&  
                  nothing_still_installed (op, ST_COMMITTED))
              {
                 chdir ("..");
                 inu_rm (INU_LIBDIR);
                 break;
              }
              for (tp=op; tp !=next_op; tp=tp->next)
                   (void) rm_all_option_files (INU_LIBDIR, tp);
           }
           break;

      default: 
           break;
   }
}

/* ---------------------------------------------------------------- *
**
**  Function   -    reject_files_exist
**
**  Purpose    -    Determines if 1 or more files needed for reject
**                  exist in the directory specified by the dir
**                  parameter.  reject files:
**                    OPTION.unconfig_u
**                    OPTION.unpost_u
**                    OPTION.unpre_u
**                    OPTION.*rodmadd
**                    OPTION.*unodmadd
**                    OPTION.undo.err
**                    OPTION.undo.trc
**                       lpp.reject
**                         
**  Note       -    This routine is not currently used but may be.
**
**  Returns    -    TRUE - if 1 or more files needed for reject exist
**                 FALSE - if no files needed for reject exist
**
** ---------------------------------------------------------------- */

int reject_files_exist (char * dir, char * op_name)
{
   char list [17*MAX_LPP_NAME + 100];
            /* 17 ==> 8 dir's + 7 op_names's + 2 padding */ 

   sprintf (list, "%s/%s.unconfig_u %s/%s.unpost_u %s/%s.unpre_u \
                   %s/%s.*rodmadd %s/%s.*unodmadd  %s/%s.undo.err \
                   %s/%s.undo.trc %s/lpp.reject", 

                   dir, op_name, dir, op_name, dir, op_name, 
                   dir, op_name, dir, op_name, dir, op_name, 
                   dir, op_name, dir);

   return (any_exist (list));
}

/* ---------------------------------------------------------------- *
**
**  Function   -  rm_all_option_files
**
**  Purpose    -  Removes all OPTION.* files (where option is the name
**                field of the op parameter) in the directory 
**                specified by the dir parameter.
**
**  Returns    -  Return code from system call to the rm cmd. 
**
** ---------------------------------------------------------------- */

int rm_all_option_files (char *dir, Option_t * op)
{
   char cmd [PATH_MAX+1];

   sprintf (cmd, "/usr/bin/rm -f %s/%s.*", dir, op->name);
   return (inu_do_exec (cmd));
}

/* ---------------------------------------------------------------- *
**
**  Function   -  can_rm_libdir 
**
**  Purpose    -  Determines if the libdir specified by the dir
**                parameter can be removed. 
**
**  Returns    -  TRUE - if the libdir can be removed 
**               FALSE - if the libdir cannot be removed
**
** ---------------------------------------------------------------- */
int can_rm_libdir (char * dir, Option_t *op, Option_t *next_op)
{
   if (IF_INSTALL (op->op_type)  &&
       (op->vpd_tree != VPDTREE_ROOT  ||  deinstl_files_exist (dir, op->name)))
      return (FALSE);

   switch (op->vpd_tree)
   {
      case VPDTREE_USR:
           if (root_parts_exist (op, next_op)  ||  
               any_partially_installed_usrs (op))
              return (FALSE);

          /** --------------------------------------------------------- *
           **  If we're committing an option that only has a usr part, 
           **  but other options to the same pkg exist that have usr and
           **  root parts (which happen to be applied), then we can't 
           **  blast the usr libdir.  So, check the root VPD for such
           **  critters.
           ** --------------------------------------------------------- */
           inu_set_vpdtree (VPDTREE_ROOT);
           if (nothing_still_installed (op, ST_APPLIED) == FALSE)
           {
              inu_set_vpdtree (VPDTREE_USR);
              return (FALSE);
           }
           inu_set_vpdtree (VPDTREE_USR);
           
           break;

      case VPDTREE_SHARE:
           if (nothing_still_installed (op, ST_APPLIED) == FALSE)
              return (FALSE);
           break;

      case VPDTREE_ROOT:
           if (any_partially_installed_usrs (op)  || 
              (nothing_still_installed (op, ST_APPLIED) == FALSE))
              return (FALSE);
           break;

      default:
           break;
   }

   return (TRUE);
}

/* ---------------------------------------------------------------- *
**
**  Function   -  root_parts_exist
**
**  Purpose    -  Determines if any of the options from (inclusive) 
**                op to next_op have both usr and root parts.
**
**  Note       -  This routine just checks the sop chunk passed
**                to it.  It doesn't check the VPD. 
**
**  Returns    -  TRUE - if root parts exist in sop
**               FALSE - if NO root parts exist in sop
**
** ---------------------------------------------------------------- */
int root_parts_exist (Option_t *op, Option_t *next_op)
{
   Option_t *tp;

   for (tp=op; tp != next_op; tp=tp->next)
      if (tp->content == CONTENT_BOTH)
         return (TRUE);

   return (FALSE);
}

/* ---------------------------------------------------------------- *
**
**  Function  -  deinstl_files_exist
**
**  Purpose   -  Determines if any files needed for deinstall
**               currently exist in the directory specified by the
**               dir parameter.  Deinstall files:
**                   OPTION.pre_d
**                   OPTION.unconfig_d
**                   OPTION.unpre_i
**                   OPTION.unpost_i
**                   OPTION.unconfig
**                   OPTION.undo.err
**                   OPTION.undo.trc
**                   OPTION.*rodmadd
**                   OPTION.*unodmadd
**                   lpp.deinstal 
**                   
**  Notes     -  It's currently assumed that this routine is only
**               called for base install pkgs.
**
**  Returns   -  TRUE - if no deinstall files exist in dir.
**              FALSE - if 1 or more deinstall files do exist in dir.
**
** ---------------------------------------------------------------- */
int deinstl_files_exist (char * dir, char *op_name)
{
   char   list [21*MAX_LPP_NAME];
             /* 21 ==> 10 dir's + 9 op_names + 2 padding */
   int    rc=TRUE;

   sprintf (list, "%s/%s.pre_d %s/%s.unconfig_d %s/%s.unpre_i \
                   %s/%s.unpost_i %s/%s.unconfig %s/%s.undo.err \
                   %s/%s.undo.trc %s/%s.*rodmadd %s/%s.*unodmadd \
                   %s/lpp.deinstal", 

                   dir, op_name, dir, op_name, dir, op_name, 
                   dir, op_name, dir, op_name, dir, op_name, 
                   dir, op_name, dir, op_name, dir, op_name, dir);

   return (any_exist (list));
}

/* ---------------------------------------------------------------- *
**
**  Function   -  any_exist 
**
**  Purpose    -  Determines if any files listed in the list 
**                parameter exist on the filesystem.
**
**  Returns    -  TRUE - if 1 or more of the files exist
**               FALSE - if none of the files exist
**
** ---------------------------------------------------------------- */

int any_exist (char *list)
{
   struct stat sbuf;  /* for stat call       */
   char   *df, *p;    /* list traversal ptrs */

   df = &(list[0]); 

   do
   {
     /** -------------------------------------------------- * 
      **  Find the next filename.  Eat any existing spaces. 
      **  Wocka, wocka, wocka ...
      ** -----------------i-------------------------------- */
      if ((p = strchr (df, ' ')) != NIL (char))
         while (*p == ' ')
         {
            *p = '\0';
            ++p; 
         }

     /** ------------------------------------------------- * 
      **  See if this file exists.  If yes, return TRUE.
      ** ------------------------------------------------- */
      if (stat (df, &sbuf) == 0)
         return TRUE;

   } while ((df=p) != NIL (char));

   return FALSE;
}

/* ---------------------------------------------------------------- *
**
**  Function   -  rm_shipped_inst_root_files
**
**  Purpose    -  Removes the shipped files belonging to the option
**                specified by the op_name parameter that reside in
**                the directory specified by the dir parameter. 
**
**  Returns    -  Return code from system call to the rm cmd. 
**
** ---------------------------------------------------------------- */
int rm_shipped_inst_root_files (char *dir, char * op_name)
{
   char cmd     [2*PATH_MAX+2];
   char cmd2    [2*PATH_MAX+2];
   char al_path [PATH_MAX+1];
   struct stat sbuf;  /* for stat call       */
   int rc=0;
   int rc2=0;

   sprintf (al_path, "%s/%s.al", dir, op_name);

  /** ------------------------------------------------- *
   **  It's not really a failure for the apply list 
   **  to not exist.  If it doesn't exist, we have 
   **  nothing to do. 
   ** ------------------------------------------------- */
   
   if (stat (al_path, &sbuf) == -1)
      return SUCCESS;

   sprintf (cmd , "/usr/bin/cat %s/%s.al | /usr/bin/xargs /usr/bin/rm -rf", 
                   dir, op_name);

  /** ------------------------------------------------- *
   **  Remove all empty directories below inst_root.
   ** ------------------------------------------------- */

   sprintf (cmd2 , "/usr/bin/find %s -print | /usr/bin/sort -u -r | while read FILE; do /usr/bin/rmdir $FILE 2> /dev/null; done", INU_LIBDIR);

   rc  = inu_do_exec (cmd);
   rc2 = inu_do_exec (cmd2);

   return (rc || rc2);
}

/* --------------------------------------------------------------- *
**
**  Function  -  rm_apply_files
**
**  Purpose   -  Removes the files from the libdir needed for apply.
**               apply files  - OPTION.pre_i/u, OPTION.post_i/u, 
**                              OPTION.config/_u, OPTION.*odmdel.
**               Where OPTION = the option name.  Ex: "bos.net.tcpip"
**
**  Returns   -  Return code from system call to rm cmd.
**
** --------------------------------------------------------------- */

/* NOTE that this list of files comes from the LPP Owners Guide.  If a new */
/*   type of file gets added/removed from that document, then this list */
/*   should be updated */
char *lpp_files[] = {
   "insize",
   "upsize",
   "prereq",
   "first",
   "inv_u",
   "pre_rm",
   "pre_i",
   "pre_u",
   "pre_d",
   "post_i",
   "post_u",
   "config",
   "config_u",
   "unconfig",
   "unconfig_u",
   "unpost_i",
   "unpost_u",
   "unpre_i",
   "unpre_u",
   "unconfig_d",
   "namelist",
   "cfgfiles",
   "rm_inv",
   "cfginfo",
   "acf",
   "supersede",
   "odmdel",
   "odmadd",
   "unodmadd",
   "err",
   "codepoint",
   "trc",
   "copyright",
   "al",
   "inventory",
   "tcb",
   "size",
   NULL
};

#define XARGS "/usr/bin/xargs /usr/bin/rm -f"

int rm_apply_files (char *dir, Option_t * op)
{
   int  rc;
   FILE *xargs;
   int i=0;

   /* we need to remove all the lpp_files which might exist, so open a pipe */
   /*   to xargs to have these files removed */
   if ( (xargs = popen( XARGS, "w" )) == NULL )
      return( 0 );

   /* write each filename to xargs */
   while ( lpp_files[i] != NULL )
      fprintf( xargs, "%s/%s.%s\n", dir, op->name, lpp_files[ i++ ] );

   pclose( xargs );

   return( 1 );
}


/* ---------------------------------------------------------------- *
**
**  Function  -  rm_undo_files
**
**  Purpose   -  Removes the "undo" hook files from the libdir.
**               "Undo" hook files - OPTION.unpre_i/u, 
**                      OPTION.unpost_i/u, OPTION.unconfig/_u
**                Where OPTION = the option name.  Ex: "bos.net.tcpip"
**
**  Returns   -  Return code from system call to rm cmd.
**
** ---------------------------------------------------------------- */
int rm_undo_files (char * dir, Option_t *op)
{
   char cmd [12*MAX_LPP_NAME];
            /* 12 ==> 5 dir's + 5 op_names + 2 padding */

   if (IF_UPDATE (op->op_type))
      sprintf (cmd, "/usr/bin/rm -f %s/%s.unpre_u   %s/%s.unpost_u \
                     %s/%s.unconfig_u %s/%s.*rodmadd %s/%s.*unodmadd", 
                     dir, op->name, dir, op->name, dir, op->name, 
                     dir, op->name, dir, op->name);

  /** ---------------------------------------------------------------- *
   **  Only wanna blast the 3 major undo hook files for a base level.
   ** ---------------------------------------------------------------- */
   else
      sprintf (cmd, "/usr/bin/rm -f %s/%s.unpre_i   %s/%s.unpost_i \
                     %s/%s.unconfig", 
                     dir, op->name, dir, op->name, dir, op->name);

   return (inu_do_exec (cmd));
}


/* ---------------------------------------------------------------- *
**
**  Function   -   rm_lpp_files 
**
**  Purpose    -   Removes all lpp.* files in the directory specified
**                 by the dir parameter (excluding README and lpp.doc
**                 files). 
**
**  Returns    -   Return code from system call to rm cmd.
**
** ---------------------------------------------------------------- */
int rm_lpp_files (char * dir)
{
   char cmd [3*MAX_LPP_NAME];
            /* 3 ==> 1 dir + 2 padding */

   sprintf (cmd, "/usr/bin/ls %s/lpp.* 2>/dev/null | \
                  /usr/bin/grep -Ev 'README|lpp.doc' | \
                  /usr/bin/xargs /usr/bin/rm -f", dir);
   return (inu_do_exec (cmd));
}

/* ---------------------------------------------------------------- *
**
**  Function   -   any_partially_installed_usrs
**
**  Purpose    -   Determines if any partially installed usrs 
**                 exist.  A partially installed usr exists when
**                 a usr part is installed (applied or committed)
**                 and its corresponding root part is not installed
**                 (applied or committed).
**
**  Returns    -   TRUE - if 1 or more partially installed usrs exist
**                FALSE - if no partially installed usrs exist
**
** ---------------------------------------------------------------- */

int any_partially_installed_usrs (Option_t * op)
{
   prod_t prod;
   int rc;
   char  prodname[MAX_LPP_NAME+1];
   
   struct name_list head={"", {0, 0, 0, 0, 0, 0, 0, 0, ""}, 
                          NIL (struct name_list)};
   struct name_list *p=NIL (struct name_list);

  /** -------------------------------------------------------------------- *
   **  Form a list of all usr part SWVPD product table entries that
   **   1. Match op's product name
   **   2. Are in an installed state (applied or committed)
   **   3. Have a corresponding root part 
   ** -------------------------------------------------------------------- */

   memset (&prod, NULL, sizeof (prod_t));

   inu_get_prodname (op, prodname); 
   strcpy (prod.lpp_name, prodname);
   strcat (prod.lpp_name, ".*");

   if (op->vpd_tree != VPDTREE_USR)
      inu_set_vpdtree (VPDTREE_USR);
   rc = vpdget (PRODUCT_TABLE, PROD_LPP_NAME, &prod);

   inu_ck_vpd_sys_rc (rc);

   while (rc == VPD_OK)
   {
      /* Need to free vchars here -- memory leak and possible coredumps in
         lower level routines if we don't. */

      vpd_free_vchars (PRODUCT_TABLE, &prod);  
      if (strcmp (prod.name, prodname) == 0
                      && 
          (prod.state == ST_APPLIED  ||  prod.state == ST_COMMITTED) 
                      && 
          (prod.cp_flag &LPP_ROOT))
      {
         add_to_list_to_check (&head, prod);
      }
      rc = vpdgetnxt (PRODUCT_TABLE, &prod);
      inu_ck_vpd_sys_rc (rc);
   }

  /** -------------------------------------------------------------------- *
   **  Ensure that all root parts corresponding to each member of the list 
   **  are in an installed state.  If any one isn't, then return TRUE. 
   ** -------------------------------------------------------------------- */

   inu_set_vpdtree (VPDTREE_ROOT);

   for (p=head.next; 
        p != NIL (struct name_list);  
        p=p->next) 
   {
      strcpy (prod.lpp_name, p->optname);
      prod.ver = p->level.ver;
      prod.rel = p->level.rel;
      prod.mod = p->level.mod;
      prod.fix = p->level.fix;
      strcpy (prod.ptf, p->level.ptf);

      rc = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_VER | PROD_REL | 
                                  PROD_MOD | PROD_FIX | PROD_PTF, &prod);
      inu_ck_vpd_sys_rc (rc);
      if (rc == VPD_NOMATCH)
      {
         free_name_list (&head);
         set_vpd_tree_back (op);
         return (TRUE);
      }

      /* ------------------------------ *
      **  Query match ... 
      ** ------------------------------ */

      if (rc == VPD_OK)
      {
         vpd_free_vchars (PRODUCT_TABLE, &prod);  
         if (prod.state != ST_APPLIED  &&  prod.state != ST_COMMITTED)
         {
            free_name_list (&head);
            set_vpd_tree_back (op);
            return (TRUE);
         }
      }
   }

   free_name_list (&head);
   return (FALSE);
}

/* ---------------------------------------------------------------- *
**
**  Function   -   add_to_list_to_check
**
**  Purpose    -   Adds a member to the list to checked.   The
**                 list is anchored by the head parameter.  No
**                 data in the head.
**
**  Returns    -   None
**
** ---------------------------------------------------------------- */

void add_to_list_to_check (struct name_list * head, prod_t prod)
{
   static struct name_list *lastnode=NIL (struct name_list);  
                                  /* ptr to last node in list */
   struct name_list        *p;    /* traversal ptr            */

  /** --------------------------------------------------- *
   **  If this is the 1st call, set the lastnode marker. 
   ** --------------------------------------------------- */
   if (lastnode == NIL (struct name_list))
      lastnode = head;

  /** ----------------------------------------------- *
   **  Allocate space for new list member.  Then
   **  stuff it w/ data.
   ** ----------------------------------------------- */

   if ((p = (struct name_list *) malloc (sizeof (struct name_list))) == 
                                 NIL (struct name_list))
   { 
      instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                       CMN_MALLOC_FAILED_D), inu_cmdname);
      inu_quit (INUMALLOC);
   }

   strcpy (p->optname, prod.lpp_name);
   p->level.ver = prod.ver;
   p->level.rel = prod.rel;
   p->level.mod = prod.mod;
   p->level.fix = prod.fix;
   strcpy (p->level.ptf, prod.ptf);

  /** ----------------------------------------------- *
   **  Add new list member to list. 
   ** ----------------------------------------------- */

   lastnode->next = p;
   p->next        = NIL (struct name_list);   
   lastnode       = p;
}

/* ---------------------------------------------------------------- *
**
**  Function   -    free_name_list
**
**  Purpose    -    Frees each node in the list anchored by the
**                  head parameter.
**
**  Returns    -    None
**
** ---------------------------------------------------------------- */

void free_name_list (struct name_list *head)
{
   struct name_list *f, *n;    /* simple traversal ptrs */

   for (f=head;
        f != NIL (struct name_list);
        f=n)
   {
      n = f->next;
      free (f);
   }
}

/* ---------------------------------------------------------------- *
**
**  Function   -   set_vpd_tree_back
**
**  Purpose    -   Sets the active VPD tree back to whatever it was.
**                 ASSUMES the last call was for ROOT part.  So, 
**                 if op is a root part we save a call.
**
**  Returns    -   None 
**
** ---------------------------------------------------------------- */

void set_vpd_tree_back (Option_t *op)
{
   if (op->vpd_tree != VPDTREE_ROOT)
      inu_set_vpdtree (op->vpd_tree);
}


/* ---------------------------------------------------------------- *
**
**  Function   -   is_biron_set 
**
**  Purpose    -   Queries the VPD and does some bit twiddling
**                 to determine if the BIRON bit is set.
**
**  Returns    -   TRUE - if the BIRON bit is set (on).
**                FALSE - if the BIRON bit is not set (off).
**
** ---------------------------------------------------------------- */

int is_biron_set ()
{
   lpp_t lpp;
   int rc;
    
  /** ---------------------- *
   **  Do the VPD query ...
   ** ---------------------- */

   memset (&lpp, NULL, sizeof (lpp_t));

   inu_set_vpdtree (VPDTREE_USR);

   strcpy (lpp.name, LPP_CTL_NAME);
   rc = vpdget (LPP_TABLE, LPP_NAME, &lpp);

   inu_ck_vpd_sys_rc (rc);

  /** ---------------------------------------------------------- *
   **  If we can't access the VPD control record, we're in big
   **  trouble ...
   ** ---------------------------------------------------------- */

   if (rc != VPD_OK)
   {
     instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, 
                                  CMN_VPDERR_D), INU_INSTALLP_CMD);
     inu_quit (INUVPDER);
   }

  /** ------------------------- *
   **  Do the bit twiddling ... 
   ** ------------------------- */

   if (lpp.cp_flag & LCR_BIRON_BIT)
      rc = TRUE;
   else
      rc = FALSE;

   vpd_free_vchars (LPP_TABLE, &lpp);
   return rc;
}
