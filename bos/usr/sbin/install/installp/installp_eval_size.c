static char sccsid[] = "@(#)83  1.24  src/bos/usr/sbin/install/installp/installp_eval_size.c, cmdinstl, bos411, 9438B411a 9/21/94 09:35:23";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: find_tlfs
 *		get_special_update_sizes
 *		installp_eval_size
 *		isupdate
 *		lppsave_count
 *		lppsave_free
 *		lppsave_insert
 *		new_bff
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include <sys/statfs.h>
#include <inu_eval_size.h>
#include <installp.h>
#include <instl_options.h>
#include <sys/vmount.h>
/*
#include <inudef.h>
*/

/* are we or aren't we an update */

#define isupdate(sop) IF_UPDATE(sop->op_type)

/* filesystem macros for these values (determined by experiment) */
#define BLOCKS_SYM_LINK 8       /* 8 512 byte blocks (4k) */
#define BLOCKS_PER_DIR  8       /* 8 512 byte blocks (4k) */
#define SYMLINK_SMALL   48      /* max target size of symlink fits in inode */
#define TFLAG_FACTOR    2       /* estimate directories needed */

static int is_default_savdir = TRUE;    /* usual save directory */

struct lppsave {                        /* linked list of lpp names */
    char           *lppname;            /* name of lpp */
    struct lppsave *next;               /* forward link */
    struct lppsave *prev;               /* backward link */
};

static struct lppsave *lppsave_head;    /* point to head of list of lpp's */
static int             lppsave_extra;   /* in case of malloc failure */

static get_special_update_sizes (Option_t * sop, 
                                 size_t fs_total[], 
                                 size_t updt_work[], 
                                 size_t work_max[]);

static find_tlfs (Option_t *sop, char *savdir);  /* blocks for alternate path */

static int new_bff (char *bffname);  		/* new bff detection */

static lppsave_count ();                 /* count # of LPP's */

static void get_installed_base_info (Option_t * op,
                                boolean  * isinstalledbase,
                                lpp_t    * lpp_rec);
static void get_rm_space (short lpp_id, size_t rm_total[]);
static void lppsave_insert (char *name);
static void lppsave_free ();
static void print_size_estimates ();

extern struct vmount *get_vmount (fsid_t * fsid);

char **fs_used = NULL;

extern void consolidate_sizes (char   * fs[],
                               char   * fs_used[],
                               int      nfs,
                               size_t fs_total[],
                               size_t work_max[],
                               size_t al_total[]);
 
/* installp's */

int installp_eval_size (Option_t * sop, 
                        char     * Command)
{
   boolean isinstalledbase;
   boolean offset_totals_for_rm_space;
   int    i;
   int    pg_space_max=0;
   int    rc;
   int    save = 0;
   int    instwork_fs;        /* Filesystem index for installp working files */
   int    savespacendx;       /* Which savespace? usr, share, or usr and root*/
   int    is_padded = 0;      /* padding space for alternate directory */
   size_t updt_total[MAXFILESYSTEMS]; /* File system requirements for update */
   size_t updt_work[MAXFILESYSTEMS];  /* Working requirements for this pkg   */
   char   savdir[PATH_MAX + 1];  /* default save directory path */
   char   *prevbff = NULL;    /* Name of previous package processed */
   char    installed_name [MAX_LPP_FULLNAME_LEN];
   size_t * rm_total;
   lpp_t  lpp_rec;

   if (sop->next == NIL (Option_t))
      return (SUCCESS);

   /* do our setup for fs's */

   if ((rc = mk_fs_list ()) != INUGOOD)
      return (rc);

   /* are we saving stuff? */

   rm_total = (size_t *) calloc (nfs+1, sizeof(size_t));

   if ( ! N_FLAG)
      save = 1;

   /* get totals for all file systems */

   offset_totals_for_rm_space = FALSE;
   for (sop = sop->next;
        sop != NIL (Option_t);
        sop = sop->next)
   {
      /* do we have a size file to work with? */

      if (sop->size != NULL)
      {
         if (isupdate (sop))
         {
            /* Clear out the update related space slots. */
            for (i = 0; i < FIRST_REAL_FS; i++)
            {
                updt_total[i] = 0;
            }

            /* Need subtotaled Working space on a package-by-package basis. */
            for (i = FIRST_REAL_FS; i <= nfs; i++)
            {
                updt_work[i] = 0;
            }
            /* 
             * Get size requirements for PAGESPACE, INSTWORK, and SAVESPACE.
             * Also, go ahead and get any temporary space requirements -
             * as in /tmp, usually.  May have to reprocess work_max later on.
             */
            inu_get_fs_sizes (sop->size, updt_total, updt_work);

           /* Place update fs requirements into general purpose space array */
           for (i = 0; i < FIRST_REAL_FS; i++)
           {
              fs_total[i] += updt_total[i];
           }

           /* 
            * Process special size requirements for /tmp, library members, 
            * and ODM data.  /tmp is special because it should always be
            * treated as if it were temporary.  Library members are special
            * because they are not saved, but they are not atomically 
            * temporary (all the library members are temporarily on the system
            * at the same time - their tmp space must be summed).  ODM data
            * is special because the update size requirements are not in the
            * savespace but rather in the ODM - fortunately they are usually in
            * the same filesystem.
            */

            get_special_update_sizes (sop, fs_total, updt_work, work_max);
         }
         else
         {
            /*
             * Base level.  Get appropriate size info.  If base is already
             * installed (or if a superseded base level is installed), 
             * calculate the approximate size that will be recovered so that 
             * sizes may be offset appropriately.
             */
            inu_get_fs_sizes (sop->size, fs_total, work_max);
            get_installed_base_info (sop, &isinstalledbase, &lpp_rec);
            if (isinstalledbase)
            {  
               if (! offset_totals_for_rm_space) {
                  /*
                   * Initialize storage first time condition is detected
                   * for current op.
                   */
                  for (i = FIRST_REAL_FS; i <= nfs; i++)
                  {
                      rm_total[i] = 0;
                  }
               }
               offset_totals_for_rm_space = TRUE;  /* sizes are offset later */

               /*
                * Query appropriate vpds retrieving size that will be recovered 
                * by removing the installed option.
                */ 
               if (lpp_rec.cp_flag & LPP_USER)
               {
                  vpdremotepath (VPD_USR_PATH);
                  vpdremote();
                  get_rm_space (lpp_rec.lpp_id, rm_total);
                  if (lpp_rec.cp_flag & LPP_ROOT)
                  {
                     vpdlocalpath (VPD_ROOT_PATH);
                     vpdlocal();
                     if (vpdget (LPP_TABLE, LPP_NAME, &lpp_rec)==0)
                        get_rm_space (lpp_rec.lpp_id, rm_total);
                  }
               }
               else
               {
                  if (lpp_rec.cp_flag & LPP_SHARE)
                  {
                     vpdremotepath (VPD_SHARE_PATH);
                     vpdremote ();
                     get_rm_space (lpp_rec.lpp_id, rm_total);
                  }
               }
            }
         }
         /* do we have any save dir space to maintain? */

         /* Decide which savespace we need to be using */
         if ((sop->content == CONTENT_SHARE)  &&  
            ((fs_total[SHARESAVESPACE] > 0) || (work_max[SHARESAVESPACE] > 0)))
            savespacendx = SHARESAVESPACE;
         else
            savespacendx = USRSAVESPACE;

         /* Is there any save space to be dealt with? */
         if ( ! N_FLAG  &&  
            (((fs_total[savespacendx] > 0) || (work_max[savespacendx] > 0)) || 
              /* Usr/root package with root savespace but with no usr? */
             ((sop->content == CONTENT_BOTH) &&  
             ((fs_total[ROOTSAVESPACE] > 0) || (work_max[ROOTSAVESPACE] > 0)))))
         {
             inu_get_savdir (sop->vpd_tree, sop, savdir);
             if (t_FLAG) {   /* alternate directory path unless pre-exists */
               if ((i = find_tlfs (sop, savdir)) != -1  &&  ! is_padded) {
                 if (is_default_savdir == FALSE) {
                   /* Add padding to the alternate save directory filesystem.
                      The simplifying assumption is that USER, ROOT, and
                      SHARE save areas will be on the same filesystem beneath
                      the alternate save directory location */
                   is_padded++;                    /* pad only once */
                   fs_total[i] += (5 * BLOCKS_PER_DIR);  /* 20K */
                 }
               }
             } else
               i = find_lfs (savdir);   /* usual save path */
             if (i != -1) {
               fs_total[i] += fs_total[savespacendx];
               work_max[i] = max (work_max[i], work_max[savespacendx]);
             }
             fs_total[savespacendx] = 0;
             work_max[savespacendx] = 0;

             /* Add in root savespace, if any */
             if ((sop->content == CONTENT_BOTH)  &&  
               ((fs_total[ROOTSAVESPACE] > 0) || (work_max[ROOTSAVESPACE] > 0)))
             {
               inu_get_savdir (VPDTREE_ROOT, sop, savdir);
               if (t_FLAG)      /* alternate directory path */
                 i = find_tlfs (sop, savdir);
               else
                 i = find_lfs (savdir);
               if (i != -1) {
                 fs_total[i] += fs_total[ROOTSAVESPACE];
                 work_max[i] = max (work_max[i], work_max[ROOTSAVESPACE]);
               }
               fs_total[ROOTSAVESPACE] = 0;
               work_max[ROOTSAVESPACE] = 0;
             }
         }

	/* Only calculate INSTWORK for first option seen in a bff */
	if ((sop->bff->path != prevbff)  &&  (new_bff (sop->bff->path)))
	{
	 prevbff = sop->bff->path; 

         /* move INSTWORK to libdir */
         inu_set_libdir (sop->vpd_tree, sop);
         instwork_fs = find_lfs (INU_LIBDIR);
         if (instwork_fs >= 0)  /* Check for reasonable result from fs search */
         {
            /*  If the build tools have separated INSTWORK into
             *  contents of liblpp.a and liblpp.a itself, then
             *  we must ensure that the size of the largest
             *  liblpp.a for this filesystem will fit. 
             *
             *  If the build tools have NOT separated out INSTWORK
             *  into the archive and the contents, then we'll have
             *  to account for the space using empirically derived
             *  estimates.  If the archive contains lots of files
             *  which are smaller than the filesystem block size, 
             *  then it is conceivable that the extracted contents
             *  would be larger than the original archive.
             *  Conversely, if the files are few, or the contents
             *  relatively large, then the archive would likely be 
             *  larger than or approximately equal in size to the
             *  contents. So, estimate conservatively (high on both
             *  sides) and hope for the best. If we estimate 
             *  best case scenarios, then we run the risk of running
             *  out of space; if we estimate worst case, then we run
             *  the risk of wasting space on the installed system.
             */

            if (work_max[INSTWORK] == 0) /* Estimate breakdown if not avail. */
            {
               /* Estimate the liblpp.a size at .5*INSTWORK */
               work_max[INSTWORK] =  fs_total[INSTWORK] * .5;

               /* Estimate the extracted archive contents size at .7*INSTWORK */
               fs_total[INSTWORK] *= .7;
            } 

            /* Add in the liblpp.a contents size */
            fs_total[instwork_fs] += fs_total[INSTWORK];
            
            /* Keep track of largest temporary space required */
            work_max[instwork_fs]=max (work_max[instwork_fs], work_max[INSTWORK]);

         } /* end if */
	}
        fs_total[INSTWORK]=0;
        work_max[INSTWORK]=0;

         /* We need the max of all PAGESPACE requirements seen
            so far.  Lets also make sure that "temporary
            PAGESPACE" (work_max[0]) is also taken into account, 
            though this really doesn't make too much sense. */

         pg_space_max = max (fs_total[PAGESPACE], pg_space_max);
         pg_space_max = max (pg_space_max, work_max[PAGESPACE]);
         work_max[PAGESPACE] = 0;
         fs_total[PAGESPACE] = 0;
      }
   }

   fs_total[0] = pg_space_max;

   /* count #'lpps on the alternate save path and estimate blocks.
      There can be a USER, ROOT and SHARE path for each lpp.  So at
      most there are 3 save directories for each lpp.  Request less
      space because the worst case is unlikely, and there is padding
      anyway (search back to "is_padded" above).

      The other simplifying assumption is that USER, ROOT, and SHARE
      save areas are below the alternate save directory on 1 filesystem.
      This assumption was used for padding as well (see "is_padded" above) */
   if (t_FLAG  &&  (i = find_lfs (t_SAVE_DIR)) != -1) {
     fs_total[i] += (TFLAG_FACTOR * (lppsave_count ()+lppsave_extra));
     lppsave_free ();
   }

   if (offset_totals_for_rm_space)
   {
      /*
       * Decrement totals according to space that will be recovered on 
       * reinstall of a base level.
       */
      for (i = FIRST_REAL_FS; i <= nfs; i++)
      {
          if (fs_total[i] > rm_total[i])
             fs_total[i] -= rm_total[i];
          else
             fs_total[i] = 0;

      } /* end for */
   } /* end if */

   /** ------------------------------------------------------------------ *
    **  If we're previewing only, then don't try to expand the fs's.
    **  Print size estimates for apply ops.
    ** ------------------------------------------------------------------ */
   if (p_FLAG) {
      if (a_FLAG)
         print_size_estimates ();
      return (SUCCESS);
   }
   return (enough_space (Command));

} /* end installp_eval_size */


/* 
 * For most files in an update, it is reasonable to assume that
 * there will be no space required on the update except for that
 * space necessary to hold the saved versions of the updated files.
 * That reasoning comes from the premise that the updated files
 * will be at least as large as the files being replaced, usually
 * true since function is rarely deleted on an update.  However, 
 * there are places where this is not true - in /tmp, in the ODM, 
 * and in inst_updt directories where library members are restored.
 * The /tmp space required is indeed temporary - not all
 * size files are written this way, but no permanent files should
 * live in /tmp.  The ODM files have permanent space requirements
       
 * but do not involve save space; the files merely grow. 
 * However, if the ODM directory is the same as the savespace directory, 
 * then we don't need to do anything extraordinary.
 * The files in inst_updt simply weren't being accounted for in 
 * the update size processing at all.  They are temporary during the
 * apply, but they all exist at the same time, so the largest one cannot
 * simply be used as the max; they must be summed.
 *
 * Therefore, there must be special parsing of the size information
 * to extract the size information for /tmp, inst_updt, and ODM in
 * case of an update, because the code ignores that space otherwise.
 */

static get_special_update_sizes (Option_t * sop, 
                                 size_t fs_total[], 
                                 size_t updt_work[], 
                                 size_t work_max[])
{
   char   updtnosv[PATH_MAX + 80];   /* Non-saved update requirements    */
   char   *updtszptr;         /* Update size information parsing ptr     */
   int    permspace = 0;      /* Permanent file system requirement       */
   int    tmpspace = 0;       /* Temporary file system requirement       */
   char   *bolptr;            /* Beginning of line in size string        */
   int    fs;                 /* File system index for directory         */
   int    ln_items;           /* Number of items matched on input line   */
   int    special_updt = 0;   /* Update of /tmp or a library member      */
   size_t updt_tmp_sz[MAXFILESYSTEMS+FIRST_REAL_FS];  /* transient reqs */

   updtnosv[0] = '\0';

    /* Check for a /tmp entry - convert it to tmp space if necessary */
    updtszptr = sop->size;
    while ((updtszptr = strstr (updtszptr, "/tmp")) != NULL) {
       /* Make sure that this is the real /tmp entry and not .../tmp */
       if ((ln_items=sscanf (updtszptr, "/tmp %d %d\n", 
						  &permspace, &tmpspace)) < 1)
       {
           updtszptr++;
       } else {
	   special_updt++;
           /* Just need to check if /tmp wasn't counted as working space. */
           if (ln_items == 1) /* Work space unfound, guarantee 0, scanf won't */
                tmpspace = 0;
           /* Working space was already counted in installp_eval_size.    */
           if (permspace != 0)
           {
             if (tmpspace != 0)        /* permanent &tmp space specified */
             {
                 sprintf (updtnosv, " /tmp %d %d\n", permspace, tmpspace);
             }
             else
             {
                 sprintf (updtnosv, " /tmp 0 %d\n", permspace);
             }
             /* Add in permanent and working space for /tmp. */
             inu_get_fs_sizes (updtnosv, fs_total, updt_work);
           }
           break; /* Found the /tmp entry */
       }
    } /* /tmp processing */

    /*
     * Process requirements for library member restoration directories.
     * Library members get restored into ../inst_updt/.. directories, 
     * where they remain until they are archived into a copy of the 
     * targeted library archive.  Thus, there are temporary requirements
     * which must be added together before checking for the largest size
     * required.
     */

    updtszptr = sop->size;
    while ((updtszptr = strstr (updtszptr, "/inst_updt/")) != NULL) {
            special_updt++;
            /* Find beginning of line */
            for (bolptr = updtszptr; (bolptr > sop->size)  &&  (*bolptr != '\n');
                 bolptr--);

            /* Check for valid entry */
            if ((ln_items = sscanf (bolptr, " %s %d %d\n", 
                                    updtnosv, &permspace, &tmpspace)) >= 2)
            {
                if (ln_items == 2) /* Guarantee value if tmpspace not scanned */
                    tmpspace = 0;
                /* Old ptfs have the inst_updt space as permanent space;
                 * new ptfs might have the inst_upst space as working space.
                 */
                 if ((fs = find_lfs (updtnosv)) != -1) {
                      updt_work[fs] += (permspace+tmpspace);
                 }
            }
            updtszptr++;
    }

    /*
     *  Special ODM size requirement handling is not included here.
     *  The package tools have been placing the size requirements for the
     *  ODM into the SAVESPACE.  The only time that this will be a real
     *  problem is if for some reason the filesystem for the SAVESPACE is
     *  different than the filesystem for the ODM, which is usually not 
     *  the case.  This should be remedied in the future.
     */

   if (special_updt) {
       for (fs = FIRST_REAL_FS; fs <= nfs; fs++) {
             work_max[fs] = max (work_max[fs], updt_work[fs]);
       }
   }
}

/*
 * find_tlfs is like find_lfs but does extra processing for
 * the alternate save directory that was specified by the -t flag.
 * find_tlfs will also call find_lfs.
 *
 * If the path for the alternate save directory is more than 48 characters, 
 * then the symbolic link in the default save location will require
 * 4K from the filesystem that it is on.  Also, if many PTFs have
 * symlinks in the lpp directory, that directory can grow larger than 4K, 
 * BUT we don't check for that because it is rare and there is plenty of
 * padding.
 *
 * To calculate the space needed for the alternate save directory, 
 * figure 2 * LPPs * 8 (8 blocks of 512 bytes for each directory).
 * 2 is a good estimate, the max possible is 3 locations for the lpp directory.
 * See the call to lppsave_count () above.
 *
 * No attempt is made to calculate inode usage.  AIX filesystems have
 * plenty of inodes.  Even if there are many PTFs and each symbolic link to
 * the alternate save location require an inode, there are plenty of inodes.
 *
 * The worst case is a small filesystem.  Estimate an inode for each block.
 * Now consider a large # of PTFs.  Each symbolic link to a save directory
 * consumes an inode.  On a small filesystem, it could run out of inodes before
 * blocks because symbolic links to paths < 49 characters fit in the inode ! 
 * That's a highly unlikely worst case.  If it is needed, add code to track the
 * inode count.
 *
 * Code or packaging should be sizing for the save directory itself, 
 * so this code doesn't add anything for the save directory itself.
 * But blocks are consumed for directories from t_SAVE_DIR to tsavdir.
 * So, a pad value for the t_SAVE_DIR filesystem is added in installp_eval_size.
 */
static
find_tlfs (Option_t *sop, char *savdir)
{
    int         i, j;                  /* filesystem array index */
    struct stat statbuf;
    char        *saveptr;              /* ptr into savedir path */
    char        tsavdir[PATH_MAX + 1]; /* full alternate dir path */
    char        lppname[MAX_LPP_NAME]; /* top level lpp name */

    /* get path into tsavdir, 
      if the default savdir does not exist, 
        if alternate location is a long path.
          consume blocks in filesystem of default location for the symbolic link
        return filesystem for alternate location
      if the default savedir symbolic link exists, 
          return filesystem for filesystem referenced by symbolic link.
      if the default directory exists
         return filesystem for default directory */
    tsavdir[0] = '\0';
    if (lstat (savdir, &statbuf) == -1) { /* default savdir does not exist */
        is_default_savdir = FALSE;      /* nothing is there */
        if (strlen (t_SAVE_DIR) + strlen (savdir) >= sizeof (tsavdir))
          return -1;                    /* apply fails on too long path */

        strcpy (tsavdir, t_SAVE_DIR);     /* form alternate save location */
        if (*savdir != '/')
          strcat (tsavdir, "/");
        strcat (tsavdir, savdir);         /* append default path to alternate */

        if (strlen (tsavdir) > SYMLINK_SMALL)  /* symlink doesn't fit in inode */
         if (saveptr = strrchr (savdir, '/')) {
           *saveptr = '\0';             /* find_lfs would stat symbolic link */
           if ((j = find_lfs (savdir)) != -1)    /* find default filesystem */
            fs_total[j] += BLOCKS_SYM_LINK;     /* add blocks for symlink */
           *saveptr = '/';
         }
        return find_lfs (tsavdir);               /* alternate path filesystem */
    }
    is_default_savdir = TRUE;           /* default save directory path exists */
    if ((readlink (savdir, tsavdir, sizeof (tsavdir))) > 0) { /* a symlink */
        lppname[0] = '\0';
        inu_get_prodname (sop, lppname);
        if (lppname[0])
          lppsave_insert (lppname);              /* for the lpp directory */
        return find_lfs (tsavdir);               /* alternate path filesystem */
    } else
        return find_lfs (savdir);                /* default directory exists */
}

/*
 * lppsave_count count the list of lpp's 
 */
static
lppsave_count ()
{
  int            rc = 0;
  struct lppsave *lptr;

  for (lptr = lppsave_head; lptr; lptr = lptr->next)
    rc++;
  return rc;
}

/*
 * lppsave_insert inserts the lpp name into a linked list in
 *                reverse alphabetical order (lexicographically descending).
 *
 * Note: ignore malloc errors.
 */
static void
lppsave_insert (char *name)
{
    int               rc;
    struct lppsave    *lptr;
    struct lppsave    *next, *prev;

    lptr = prev = NULL;
    if (name  &&  *name) {
      for (next = lppsave_head; next; next = next->next) {
        prev = next->prev;
        if ((rc = strcmp (next->lppname, name)) <= 0)
          break;
      }
      if (next  &&  rc == 0)
        lptr = next;                         /* found it */
      else if ((lptr= (struct lppsave *) malloc (sizeof (struct lppsave))) == NULL)
        if ((lptr->lppname = (char *) malloc (strlen (name) + 1)) == NULL) {
          free (lptr);
          lptr = NULL;
        } else {
            if (lppsave_head == next)
              lppsave_head = lptr;              /* at front of list */
            if (lptr->next = next)              /* next in list */
              next->prev = lptr;                /* previous of next */
            if (lptr->prev = prev)              /* previous in list */
              prev->next = lptr;                /* next of previous */ 
            (void) strcpy (lptr->lppname, name);  /* save the name */
        }
    }
    if (lptr == NULL)
      lppsave_extra++;  /* malloc failed, assume all lpp's are unique */
}

/*
 * lppsave_free frees malloc'd memory for a linked list of lpp names.
 */
static void
lppsave_free ()
{
        struct lppsave *lptr, *next;

        for (lptr = lppsave_head; lptr; lptr = next) {
          next = lptr->next;            /* save next before free'ing */
          if (lptr->lppname)            /* double check name pointer */
              free (lptr->lppname);      /* free the name string */
          free (lptr);                   /* free the linked list element */
    }
}


#define BFFHASHSZ 53	/* Semi-arbitrary medium-small prime for bffname tbl */
static struct bffhash {
   char *bffname;	/* pathname for current package */
   struct bffhash *next;       /* next package with this hash value */
} *bfflist[BFFHASHSZ];

static int new_bff (char *curbff) 
{
   int hashval;
   struct bffhash *hashp;

   hashval = ((int) curbff) % BFFHASHSZ;	/* Checking ptrs is enough */

   /* Either find the bff or fall off of the end of the bucket */
   for (hashp = bfflist[hashval];
	 (hashp != NULL)  &&  (hashp->bffname != curbff);
	hashp = hashp->next);

   /* Found a match? */
   if (hashp != NULL) {
       return 0;
   } else {	/* Need to add this bff to bucket and return affirmative. */
       hashp = (struct bffhash *) malloc (sizeof (struct bffhash));       
       if (hashp != NULL) {
	   /* Insert this bff at head of hash list */
           hashp->next = bfflist[hashval];
	   hashp->bffname = curbff;	   /* pathname for current package */
	   bfflist[hashval] = hashp;
	   return 1;
       } else {
	   return 1;
       }
   }

}

/* 
 * NAME:     print_size_estimates
 *
 * FUNCTION: Prints our best attempt at estimating the up-front size
 *           requirements.
 * 
 * RETURNS:  Void function.
 */
static void 
print_size_estimates ()
{
   boolean sizes_found;
   struct statfs statbuf;
   long kb_free = 0;
   long kb_needed = 0;
   long tot_free = 0;
   long tot_needed = 0;
   short  i;
   short  j;
   struct vmount *vm;

   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_RESOURCES_TITLE_I, 
                                             INP_RESOURCES_TITLE_D));

   sizes_found = FALSE;   
   i = FIRST_REAL_FS;
   while ((i <= nfs)  &&  ( ! sizes_found))  {
       if (fs_total[i] != 0  ||  work_max[i] != 0) {
          sizes_found = TRUE;
       }
       ++i;
   }

   if ( ! sizes_found)
   {
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NO_RESOURCE_EST_I, 
                                                INP_NO_RESOURCE_EST_D));
      return;
   }
      
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_RESOURCES_HDR_I, 
                                             INP_RESOURCES_HDR_D));
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_RESOURCES_COL_HDR_I, 
                                             INP_RESOURCES_COL_HDR_D));

   /*
    * Depending upon the way filesystems are mounted, particularly in a 
    * NIM environment, resource requirements may need to be resolved to a 
    * single filesystem.  Call the routine used by enough_space() to 
    * consolidate the information accumulated thus far.
    */
   consolidate_sizes (fs, fs_used, nfs, fs_total, work_max, al_total);

   for (i = FIRST_REAL_FS; i <= nfs; i++) {
        if ((fs_total[i] != 0)  ||  (work_max[i] != 0)) {
            (void) statfs (fs_used [i], &statbuf);
            kb_needed = fs_total[i] + work_max[i];
            /*
             * Note on casting:  long long is only needed for intermediate 
             * result of multiplication.  (long (or int) - 4 bytes - is good 
             * enough to save # of 512 blks for up to 1,0737,741,824 blks).
             */
            kb_free = (long) (((long long) statbuf.f_bavail * 
                        (long long) statbuf.f_bsize)/ (long long) 512);
            tot_needed += kb_needed;
            tot_free += kb_free;
            vm = get_vmount(&statbuf.f_fsid);  /* get access to real fs name 
                                                  for printing */ 
            if (k_FLAG && p_FLAG) 
                instl_msg (INFO_MSG,"_SIZE_ %-30s %10d %22ld\n",
                                   vmt2dataptr(vm, VMT_STUB),
                                   kb_needed, 
                                   kb_free);
            else
                 instl_msg (INFO_MSG,"      %-30s %10d %22ld\n",
                                   vmt2dataptr(vm, VMT_STUB),
                                   kb_needed, 
                                   kb_free);
        }
   }

   if (k_FLAG && p_FLAG)
       instl_msg (INFO_MSG,"_SIZE_ %-30s %10d %22d\n", "TOTAL", tot_needed, tot_free);
   else {
       instl_msg (INFO_MSG, "      %s %35s %22s\n", "-----", "--------", "------");
       /*
        * Print TOTAL line.
        */
       instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_TOTAL_RESOURCE_I, 
                                INP_TOTAL_RESOURCE_D), tot_needed, tot_free);
   }

   if (fs_total[PAGESPACE] != 0) {
      /*
       * print page space required. 
       */
      if (k_FLAG && p_FLAG)
          instl_msg (INFO_MSG,"_PGSP_ %-30s %10d\n","PAGESPACE",fs_total[PAGESPACE]);
      else
          instl_msg (SCREEN_LOG, MSGSTR (MS_INSTALLP, INP_PAGE_SPACE_HDR_I, 
                                         INP_PAGE_SPACE_HDR_D), fs_total[PAGESPACE]);
   }

   if (k_FLAG && p_FLAG) 
       instl_msg (INFO_MSG,"\n");

   /*
    * print message about accuracy of estimates.
    */
   instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_NEEDED_SPACE_I, 
                                             INP_NEEDED_SPACE_D));

   if (X_FLAG)
      instl_msg (INFO_MSG, "\n");
   else 
      /*
       * Add note about using -X for automatic extension when necessary.
       */
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USE_X_I, INP_USE_X_D));
 
} /* print_size_estimates */

/* 
 * NAME:     get_rm_space
 *
 * FUNCTION: Searches the inventory vpd for each entry with an lpp_id 
 *           matching the lpp_id argument to the function.  Adds the size
 *           field from the inventory entry to the rm_total array.
 * 
 * RETURNS:  Void function.
 */
static void get_rm_space (short lpp_id, size_t rm_total[])
{
   char  *file_name;
   inv_t  inv_rec;
   short  indx;
   int    rc;
   struct stat stat_buf;

   /*
    * Strategy:  Loop on inventory table doing vpdgets on lpp_id.
    *            if entry is directory, add 4K (8 blocks to total)
    *            if entry is volatile file, get actual size using stat.
    *            otherwise use size from the inventory record (rounded up
    *                                                         to 4K units)
    */
   inv_rec.lpp_id = lpp_id;

   rc = vpdget (INVENTORY_TABLE, INV_LPP_ID, &inv_rec);
   while (rc == VPD_OK)
   {
      if (inv_rec.loc0[0] == '.')
         file_name = &inv_rec.loc0[1];
      else
         file_name = inv_rec.loc0;

      indx = find_lfs (file_name);   /* get logical filesystem for file */

      if (indx >= 0)   /* valid filesystem for this file? */
      {
         if (inv_rec.file_type == 1)
            /*
             * Directory.
             */
            rm_total[indx] += (size_t) 8;    
         else
            /*
             * Volatile File? (size = 0 and type = 0)
             */
            if ((inv_rec.size == 0)      && 
                (inv_rec.file_type == 0) && 
                (stat (file_name, &stat_buf) == 0))
               rm_total[indx] += 
                (size_t)((((((stat_buf.st_size + stat_buf.st_blksize - 1) / 
                        stat_buf.st_blksize)) *stat_buf.st_blksize) + 511)/512);
            else                                
                /* 
                 * Regular file. 
                 */
               rm_total[indx] += (size_t)(((inv_rec.size + 4095)/4096) * 8);
      } /* end if */

      vpd_free_vchars (INVENTORY_TABLE, &inv_rec);
      rc = vpdgetnxt (INVENTORY_TABLE, &inv_rec);

   } /* while */

} /* get_rm_space */

/* 
 * NAME:     get_installed_base_info
 *
 * FUNCTION: Determines if the option pointed to by op is already installed,
 *           perhaps at a different level OR mayabe under a different name 
 *           (if op supersedes the installed base level).  
 * 
 * RETURNS:  Void function.  Sets isinstalledbase to TRUE if op is already 
 *           installed, FALSE otherwise.  lpp_rec is initialized with info
 *           from lpp database if isinstalledbase is TRUE.
 */
static void get_installed_base_info (Option_t * op,
                                     boolean  * isinstalledbase,
                                     lpp_t    * lpp_rec)
{
   FILE     fp;
   boolean  base_installed = FALSE;
   char     level [MAX_LEVEL_LEN];
   char     name_buf [MAX_LPP_FULLNAME_LEN];

   *isinstalledbase = FALSE;

   if (! IF_INSTALL (op->op_type))
      return;

   if (op->level.sys_ver) {
      /*
       * Option by same name is already installed.
       */
      strcpy (lpp_rec->name, op->name); /* used below in vpd get */
      base_installed = TRUE;

   } else {

   if (op->supersedes == NIL (char) || op->supersedes[0] == '\0') {
      /*
       * No supersedes data, can't be installed at an older level
       * with a different name.
       */
      return;

   } else { 

      /*
       * Parse the supersedes info.  Looking for a different name to that
       * in op (the same name indicates barrier rather than supersedes info).
       * Make the supersedes string look like a file for easy parsing.
       */
      fp._flag = (_IOREAD | _IONOFD);
      fp._ptr = fp._base = (unsigned char *) op->supersedes;
      fp._cnt = strlen (op->supersedes);
      fp._file = _NFILE;

      while (fscanf (&fp, "%199s %99s", name_buf, level) != EOF)
      {
         if (strcmp (op->name, name_buf) != 0) 
         {
            strcpy (lpp_rec->name, name_buf);
            base_installed = TRUE;
            break;        
         } 
      } /* end while */

   }    /* end if (op->supersedes... */
   }    /* end if (op->level.....    */

   if (! base_installed)
      return;

   /* 
    * Search each lpp database until the name found in supersedes info
    * is located.
    */
   vpdremotepath (VPD_USR_PATH);
   vpdremote();
   if (vpdget (LPP_TABLE, LPP_NAME, lpp_rec) == VPD_OK)
   {
      *isinstalledbase = TRUE;
      return;
   }

   vpdremotepath (VPD_SHARE_PATH);
   vpdremote();
   if (vpdget (LPP_TABLE, LPP_NAME, lpp_rec) == VPD_OK)
   {
      *isinstalledbase = TRUE;
      return;
   }

   vpdlocalpath (VPD_ROOT_PATH);
   vpdlocal();
   if (vpdget (LPP_TABLE, LPP_NAME, lpp_rec) == VPD_OK)
   {
      *isinstalledbase = TRUE;
      return;
   }

} /* get_installed_base_info */
