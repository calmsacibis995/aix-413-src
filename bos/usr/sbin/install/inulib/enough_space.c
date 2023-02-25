static char sccsid[] = "@(#)57  1.20  src/bos/usr/sbin/install/inulib/enough_space.c, cmdinstl, bos411, 9428A410j 6/23/94 19:03:07";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: enough_space
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

/*
#include <stdio.h>
#include <instlmsg.h>
#include <commonmsg.h>
#include <inudef.h>
#include <inudef.h>
*/

#include <inu_eval_size.h>
#include <inulib.h>
#include <sys/mntctl.h>
#include <sys/statfs.h>
#include <sys/vmount.h>

char **fs = NULL;
int *fs_flags = NULL;
ulong_t *fs_vfsnum = NULL;
size_t *fs_total = NULL;
size_t *al_total = NULL;
size_t *work_max = NULL;
int nfs=-1;

char **fs_used = NULL;

#define MAXEXTRA 500

/*
* Note on casting:  (long long) is needed for intermediate
* result of multiplication AND for final result of subtraction
* since result could be a large negative number.
* (long) is needed in 1st part of expression since subtraction could
* also result in negative number
*/
#define INU_BLOCKS_SHORT \
 ((long long)(((long)fs_total[j] + (long)work_max[j] - (long)al_total[j] + 1)/2)\
                               -\
  (long long)((((long long) statbuf.f_bavail * (long long) statbuf.f_bsize)\
                                       + (long long) 1023) / (long long) 1024))

extern struct vmount *get_vmount (fsid_t * fsid);

void consolidate_sizes (char   * fs[],
                        char   * fs_used[],
                        int      nfs,
                        size_t   fs_total[],
                        size_t   work_max[],
                        size_t   al_total[]);
/*
 * NAME: enough_space
 *
 * FUNCTION:
 * Test to see if enough disk space is available to install.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * NOTES:
 * If the user asked the we attempt to enlarge a logical volume if
 * there is not enough space (xpand == TRUE), then don't bother to
 * check.
 *
 * RETURNS:
 * iRUE if there is enough space or we'll expand anything
 *  that runs short.
 * FALSE if there is not enough space.
 */

int enough_space (char *Command)
{
 size_t new_size;   /* total size of the filesystem */
 int i, j;    /* misc. counter  */
 int rc;
 long long blocks_short;   /* n. of blocks needed if not enough */
 int extra;
 int expanded;
 struct statfs statbuf;  /* buffer for statfs () call */
 int xpand=0;
 char *ptr;
 char *savedir;
 char *instworkdir;
 struct vmount *vm;

 if (nfs < FIRST_REAL_FS)
    return (SUCCESS); /* can't build list of fs so hope for best */

 rc = SUCCESS;
 if ((ptr=getenv ("INUEXPAND")) !=NULL)
    xpand=atoi (ptr);

 /* include inst work dir */
 if (fs_total[INSTWORK] > 0  ||  work_max[INSTWORK] > 0)
 {
    if ((instworkdir=getenv ("INULIBDIR")) !=NULL)
    {
       i=find_lfs (instworkdir);
       if (i>=FIRST_REAL_FS)
       {
          fs_total[i]+=fs_total[INSTWORK];
          work_max[i]=max (work_max[i], work_max[INSTWORK]);
       }
    }
 }

 /* include save dir? */
 /* Only need to add this in in case of INUSAVE because it's already
  * been added in to the appropriate directory for installp.
  * inusave just stuffs space requirement into the default SAVESPACE
  * slot, regardless of whether it's root, share, or usr.
  */
 if ((fs_total[DEFSAVESPACE] > 0  ||  work_max[DEFSAVESPACE] > 0)
      &&  (ptr=getenv ("INUSAVE")) != NULL  &&  strcmp (ptr, "1") == 0)
 {
    if ((savedir=getenv ("INUSAVEDIR")) != NULL)
    {
       i=find_lfs (savedir);
       if (i>2)
       {
          fs_total[i]+=fs_total[DEFSAVESPACE];
          work_max[i]=max (work_max[i], work_max[DEFSAVESPACE]);
       }
    }
 }

 /* 
  * Consolidate resource requirements into the actual filesystem
  * for which the requirement exists.
  */
 consolidate_sizes (fs, fs_used, nfs, fs_total, work_max, al_total);

 /*
  * For all file systems check the free space, writeablity and extendablity.
  * This first set of checks causes space checking to fail if ANY 
  * filesystem causes problems.  (A subsequent iteration through the
  * filesystem arrays performs additional in-progress checking, while expanding
  * in case space is consumed elesewhere while expanding.)
  */
 for (j = FIRST_REAL_FS ; j <= nfs ; j++) 
 {
    /* ingnore those with out allocated space */
    if (fs_total[j] == 0  &&  work_max[j] == 0)
       continue;

    if (fs_flags[j] &MNT_READONLY)
    {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_READONLY_FS_E, 
                                    CMN_READONLY_FS_D), Command, fs[j]);
       rc = FAILURE;
    }

   /*
    * statfs () returns a structure containing blocks left as well
    * as the size of the block in the specified filesystem (might
    * be 512, might be 2048...  We will use 1024 byte blocks
    * for the comparison but report failures in 512 byte blocks
    * since that's what df reports.
    */

    if (statfs (fs_used[j], &statbuf) != 0) 
    {
       perror ("stat");
       rc=FAILURE;  /* continue with other fs */
       continue;
    }

   /*
    * compute number of 1024 byte blocks we are short
    */
    blocks_short = INU_BLOCKS_SHORT;

   /**********
   ** If additional space is needed (beyond the amount available), then any of
   ** the following will produce errors:
   **  -- filesystem is nfs mounted (cannot auto-expand)
   **  -- filesystem is NOT a mounted physical device ex. NOT /dev/hd1
   **     (cannot auto-expand)
   **  -- -X is not provided.
   **********/
    if ((blocks_short + (long long) MAXEXTRA) > 0)
    {
       if(fs_flags[j] & MNT_REMOTE) {

          vm = get_vmount(&statbuf.f_fsid);
          inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_REMOTE_FS_E,
                                     CMN_REMOTE_FS_D),
                  Command,
                  vmt2dataptr(vm, VMT_HOST),
                  vmt2dataptr(vm, VMT_OBJECT),
                  vmt2dataptr(vm, VMT_STUB),
                  ((int) blocks_short + MAXEXTRA)*2);
          rc = FAILURE;

       } else {

       if(!(fs_flags[j] & MNT_DEVICE))
       {
          vm = get_vmount(&statbuf.f_fsid);
          inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_EXPAND_ALLOWED_E,
                                     CMN_NO_EXPAND_ALLOWED_D),
                  Command,
                  vmt2dataptr(vm, VMT_STUB),
                  ((int) blocks_short + MAXEXTRA)*2);
          rc = FAILURE;

       } else {

       if (!xpand) {

          vm = get_vmount(&statbuf.f_fsid);
          inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NOSPACE_E,
                                     CMN_NOSPACE_D),
                  Command,
                  vmt2dataptr(vm, VMT_STUB),
                  ((int) blocks_short + MAXEXTRA)*2);
          rc = FAILURE;

       }     /* end if (!(xpand.... */
       }   /* end if (!(fs_flags[j]... */
       } /* end if ((fs_flags[j]... */
    } /* end if (blocks_short... */
 } /* for */

 /* 
  * Return if any failures were detected above.
  */
 if (rc==FAILURE)
    return (rc);

 /* expand the page space? */
 if (fs_total[0] > 0  ||  work_max[0] > 0)
 {
    if ((rc=inu_expand_ps (fs_total[0] + work_max[0])) != SUCCESS)
    {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_PGSPACE_MEG_E, 
                            CMN_PGSPACE_MEG_D), Command, rc);
       return (FAILURE);
    }
 }

 /* 
  * All above checks must have been successful.  Rescan the filesystem
  * arrays expanding where necessary.  Perform addtional checks for
  * space availability while expanding.
  */
 for (j = FIRST_REAL_FS ; j <= nfs ; j++)
 {
    /* ignore if there is no space required */
    if (fs_total[j] == 0  &&  work_max[j] == 0)
       continue;

   /*
    * statfs () returns a structure containing blocks left as well
    * as the size of the block in the specified filesystem (might
    * be 512, might be 2048...  We will use 1024 byte blocks
    * for the comparison but report failures in 512 byte blocks
    * since that's what df reports.
    */

    if (statfs (fs_used[j], &statbuf) != 0) 
    {
       perror ("stat");
       rc=FAILURE;  /* continue with other fs */
       continue;
    }

   /*
    * compute number of 1024 byte blocks we are short
    */
    blocks_short = INU_BLOCKS_SHORT;

   /**********
   ** Check if the current filesystem needs expansion.
   ** Perform the same checks as above for writability/extendability since 
   ** circumstances may have changed while in this loop expanding filesystems.
   **********/
   if ((blocks_short + (long long) MAXEXTRA) > 0)
   {
      /*
       * Note on storage size of blocks_short: If we get here, blocks_short is
       * likely no more than 4 bytes of storage.  The only time we are concerned
       * about it needing the storage of a long long is when the result of
       * INU_BLOCKS_SHORT is a large negative number.  At this point, we know
       * this is not the case.  The remaining computations involving
       * blocks_short will cast to the predominant type in the computation.
       */

      /* must not be remote to expand */
      if (fs_flags[j] &MNT_REMOTE) {

          vm = get_vmount(&statbuf.f_fsid);
          inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_REMOTE_FS_E,
                                     CMN_REMOTE_FS_D),
                  Command,
                  vmt2dataptr(vm, VMT_HOST),
                  vmt2dataptr(vm, VMT_OBJECT),
                  vmt2dataptr(vm, VMT_STUB),
                  ((int) blocks_short + MAXEXTRA)*2);
         rc = FAILURE;

      } else {

      /* must be a device for us to expand it */
      if ( ! (fs_flags[j] &MNT_DEVICE)) {

          vm = get_vmount(&statbuf.f_fsid);
          inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_EXPAND_ALLOWED_E,
                                     CMN_NO_EXPAND_ALLOWED_D),
                  Command,
                  vmt2dataptr(vm, VMT_STUB),
                  ((int) blocks_short + MAXEXTRA)*2);
         rc = FAILURE;

      } else {
        
      if (xpand) {

         /**********
         ** Add MAXEXTRA to number of blocks, 
         ** if that fails then try 0
         **********/
         expanded = FALSE;

         for (extra=MAXEXTRA; extra >= 0; extra = extra - MAXEXTRA) 
         {
            new_size = (size_t) blocks_short + (size_t) extra + 
            (size_t) ((((long long) statbuf.f_blocks * 
                                                  (long long) statbuf.f_bsize) 
                              + (long long) 1023) / (long long)1024);
            /**********
            ** if the extra to be exanded plus the blocks short
            ** is greater than zero, then try to expand this file
            ** system
            **********/
            if (((size_t) blocks_short + extra) > 0) 
            {
               if (expand_lv (fs[j], new_size) == SUCCESS) 
               {
                  expanded = TRUE;
                  break;
               }
            }
            /**********
            ** Not really short, just trying to be safe.
            **********/
            else 
            {
               expanded = TRUE;
               break;
            }
         }

         /**********
         ** the expansion failed
         **********/
         if ( ! expanded) 
         {
            vm = get_vmount(&statbuf.f_fsid);
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_EXPAND_E, 
                                 CMN_CANT_EXPAND_D), 
                                 Command, 
                                 vmt2dataptr(vm, VMT_STUB),
                                 ((int) blocks_short + MAXEXTRA)*2);
            rc = FAILURE;
         }

      } else { 

      vm = get_vmount(&statbuf.f_fsid);
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NOSPACE_E, 
                                 CMN_NOSPACE_D), 
                                 Command,
                                 vmt2dataptr(vm, VMT_STUB),
                                 ((int) blocks_short + MAXEXTRA)*2);
      rc = FAILURE;

      }      /* end if (xpand...  */
      }   /* end if (!(fs_flags[j]... */
      } /* end if (fs_flags[j]... */
   }  /* if ((blocks_short + MAXEXTRA) > 0) */

 }  /* for (j = FIRST_REAL_FS ; j <= nfs ; j++) */

 return (rc);
}

/*
 * NAME: consolidate_sizes
 *
 * FUNCTION:
 * Combines size information from the various arrays used to
 * gather that data into a single filesystem where possible.
 * Particularly necessary in a NIM environment where  
 * mounted directories may resolve to a single filesystem
 * for expansion.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS:
 * void function
 */
void consolidate_sizes (char   * fs[],
                        char   * fs_used[],
                        int      nfs,
                        size_t fs_total[],
                        size_t work_max[],
                        size_t al_total[])
{
   short i,j;

   bzero (fs_used, sizeof (char *) * nfs);

   for (j = FIRST_REAL_FS ; j <= nfs ; j++) 
   {
      /* has another fs been resolved to this one? */
      if (fs_used[j])
         continue;

      /* ignore if there is no space required */
      if (fs_total[j] == 0  &&  work_max[j] == 0)
         continue;

      /* make sure the original fs is remembered */
      fs_used[j]=fs[j];

      /* what is the real one? */
      i=find_pfs (fs[j]);
      if (i==j  ||  i<0)
         continue;

      /* make new old fs known to new fs */
      fs_used[i]=fs[j];

      work_max[i]=max (work_max[j], work_max[i]);
      fs_total[i]+=fs_total[j];
      al_total[i]+=al_total[j];
      work_max[j]=0;
      fs_total[j]=0;
      al_total[j]=0;
   }

} /* consolidate_sizes */
