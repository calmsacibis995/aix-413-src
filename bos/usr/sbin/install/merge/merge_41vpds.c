static char sccsid[] = "@(#)74  1.6  src/bos/usr/sbin/install/merge/merge_41vpds.c, cmdinstl, bos411, 9428A410j 6/3/94 10:45:12";
/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: enough_space_for_vpd
 *		expand_my_lv
 *		get_N_put_table
 *		get_free_blocks_in_filesystem
 *		get_size_of_file
 *		get_size_of_vpd
 *		main
 *		validate_command_line
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <merge_41vpds.h>
#include <stdarg.h>

/* static nl_catd catd;  */

char *vpdsrc; 
char *vpdtarget; 
int  target_lpp_count = 0;
char *inu_cmdname;


/*--------------------------------------------------------------------------*
*
* NAME: main (merge_41vpds)
*
* FUNCTION:
*  Merge the following vpds:
*      a. lpp
*      b. product
*      c. history
*      d. inventory
*
* EXECUTION ENVIRONMENT: 
*   Can be executed by root user only
*
* Usage:
*        merge_41vpds <32_vpd_path> <41_vpd_path>
*  
* Arguments: 
*       source_vpd_path  -- path where source VPD is located
*       target_vpd_path  --  path where target VPD will be located
*
* RETURNS: 0 on success, non-zero on failure
*
*-------------------------------------------------------------------------*/

int
main (int    argc,
      char   **argv)
{
   /* the three VPD paths */

   int vpd []={   LPP_TABLE,
                  PRODUCT_TABLE, 
                  INVENTORY_TABLE,
                  HISTORY_TABLE,
                  LAST_ELEMENT 
                 },
          i, j,  /* generic counters */
          rc;


   inu_cmdname=getbasename(argv[0]);      /* For error messages */

   validate_command_line (argc, argv);

   setlocale (LC_ALL, "");

   CATOPEN ();

   umask ((mode_t) (S_IWGRP | S_IWOTH));    /* set umask to 022     */

   /* --------------------------------------------------------------
   * Is there enough space in fs containing vpdtarget for all vpds 
   * in vpdsrc?
   * -------------------------------------------------------------- */
   if (!enough_space_for_vpd(vpdsrc, vpdtarget, TRUE) ) 
       exit (1);

   /* set vpd path */
   if ((rc = vpdlocalpath(vpdsrc)) != VPD_OK) {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D),
                          inu_cmdname);
        exit (rc);
   }

   if ((rc = vpdremotepath(vpdtarget)) != VPD_OK) {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D),
                          inu_cmdname);
        exit (rc);
   }


   /* -----------------------------------------------------------------------
   * for vpd in "lpp product history inventory"
   * do
   *    merge_source_with_target  
   * done
   *-----------------------------------------------------------------------*/

   for (j=0; vpd[j] != LAST_ELEMENT; j++) 
        if ( get_N_put_table( vpd[j], vpdsrc, vpdtarget) != TRUE ) {
              inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, 
                                 CMN_VPDERR_D), inu_cmdname);
              exit (1);
        }

  /* close local path */
  vpdlocal();
  vpdterm();

  /* close remote path */
  vpdremote();
  vpdterm();

  CATCLOSE ();

  return (0);  /* Success */

} /* end main */

/* ------------------------------------------------------------------------
*
* NAME: get_N_put_table
* 
* FUNCTION:    1. Gets 41 product records from source vpd 
*              2. Puts 1. in to the 41 prod table of target 
*
*
* EXECUTION ENVIRONMENT:
*           
* NOTES:
*    exits on vpd errors.
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*     parameters:
*     global:
*
* RETURNS: (boolean)
*     TRUE   on success  
*     FALSE  failure 
*
* OUTPUT:
*
* ------------------------------------------------------------------------*/

boolean
get_N_put_table       (int tbl_id,        /* which table? */
                       char *vpd_src,     /* source vpd path */
                       char * vpd_target) /* target vpd path */
{
  int rc=FALSE;        /* rteurn code */ 
  prod_t prod;         /* product struct */
  lpp_t lpp;          /* lpp     struct */
  hist_t hist;         /* history struct */
  inv_t inv;          /* inventory struct */
  void *ptr;           /* generic pointer */
  
    switch (tbl_id)
   {
      case LPP_TABLE :
           ptr = (lpp_t *) &lpp;
           break;

      case PRODUCT_TABLE :
           ptr = (prod_t *) &prod;
           break;

      case HISTORY_TABLE :
           ptr = (hist_t *) &hist;
           break;

      case INVENTORY_TABLE :
           ptr = (inv_t *) &inv;
           break;

      default:
           inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, 
                                      CMN_VPDERR_D), inu_cmdname);
           exit (1);
   }

  init_vchars (tbl_id, ptr); /* initialize prod struct */
  
  vpdlocal();   /* set it to vpdsrc */
  rc = vpdget (tbl_id,VPD_ALL, ptr);  /* get the first record */
 
  if (rc != VPD_OK) {
      if (strstr (vpd_src,"share") != NIL(char) )
           return (TRUE);       /* share vpd was empty */

      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                 inu_cmdname);
      exit(FALSE);
  }

  while ( rc == VPD_OK)
   {
       switch (tbl_id)
      {
         case LPP_TABLE :
	      /* This program does not detect duplicate records *
	       * across the two databases.  It will happily     *
	       * leave the duplicates in, which leaves the VPD  *
	       * in an unseemly state.  It is expected that a   *
	       * program has weeded out the duplicates prior    *
	       * to the calling of this program.                *
	       * This program also expects that another program *
	       * will bump the __SWVPD_CTL__ value in the       *
	       * LPP database to eliminate conflicts between    *
	       * the LPP ids in the two databases.              */
	      lpp.lpp_id += target_lpp_count;
              break;
   
         case HISTORY_TABLE :
	      hist.lpp_id += target_lpp_count;
              break;
   
         case INVENTORY_TABLE :
	      inv.lpp_id += target_lpp_count;
              break;
   
         case PRODUCT_TABLE :
	 default:
              break;
      }

     /* set vpd path to target vpd */
     vpdremote();            /* always returns VPD_OK */
     if (vpdadd ((tbl_id == LPP_TABLE) ? MERGING_LPP_TABLE : tbl_id, ptr) != VPD_OK) {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D),
                                    inu_cmdname);
         exit(FALSE);
     }

     /* set vpd path to src vpd */
     vpdlocal();
     vpd_free_vchars (tbl_id, ptr);
     rc = vpdgetnxt (tbl_id, ptr);
     if (rc == VPD_SYS || rc == VPD_BADCNV) {
        inu_msg (FAIL_MSG,MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                  inu_cmdname);
        exit(FALSE);
     }
   }

  return (TRUE); 
}

/* ------------------------------------------------------------------------
*
* NAME: get_free_blocks_in_filesystem
* 
* FUNCTION: Obtain number of free blocks in the filesystem
*           in 512-byte blocks.
*
*
* DATA STRUCTURES:
*      parameters:  path -- name of the file whose
*                           filesystem we wanna find the size of.
*      global:      none
*
* RETURNS: (long)
*          number of free 512-blocks in fs 
*          -1 on error
*
* OUTPUT:  name of fs
*
* ------------------------------------------------------------------------*/

long
get_free_blocks_in_filesystem ( char *path) 
{
   struct statfs sfs;  /* for statfs call */

   if (statfs (path, &sfs) == -1)
     {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                          CMN_CANT_OPEN_D), inu_cmdname, path);
       return (-1);
     }
                        /* Rounding up  num_of_free_blocks */
   return ( (long) ((((long long) sfs.f_bfree  * 
                     (long long) sfs.f_bsize) 
                                  + 
                     (long long) 511) / (long long) 512) ) ;
}

/* ------------------------------------------------------------------------
*
* NAME: expand_my_lv
* 
* FUNCTION: call chfs to expand logical volume
*
* NOTES:
*     Error Condition:  chfs failures
*
* RECOVERY OPERATION:
*          Bump up upper limit of logical partitions 
*
* DATA STRUCTURES:
*     parameters:  fs -- fs/lv to expand
*                  num_of_blocks -- Amount to expand lv by
*     global:      none
*
* RETURNS: (boolean)
*     TRUE on success
*     FALSE on failure
*
* OUTPUT:
*
* ------------------------------------------------------------------------*/

boolean
expand_my_lv (char *fs,     
              long num_of_512_blocks)
{
   char cmd [CMD_LINE_LT]; /* for chfs command */
  
   sprintf (cmd, "%s %s%d %s", CHFS, CHFS_FLAGS, num_of_512_blocks, fs);
 
   if ( system (cmd) != 0)
     {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_EXPAND_E,
                          CMN_CANT_EXPAND_D), inu_cmdname,fs,num_of_512_blocks);

       return (FALSE);
     }

   return (TRUE);
}

/* ------------------------------------------------------------------------
*
* NAME: get_size_of_vpd
* 
* FUNCTION: Get the size of the given vpd.
*
* DATA STRUCTURES:
*     parameters:  path -- path to vpd
*     global:
*
* RETURNS: (long)
*     > -1 on success 
*     -1 on failure   
*
* OUTPUT: expanded fs if necessary 
*
* ------------------------------------------------------------------------*/

long   
get_size_of_vpd   (char *path)        /* path vpd that needs to be saved */
                        
{
   long num_of_512_blocks_for_vpd = 0;     /* Original size of the vpd */ 

   struct stat statbuf;      /* for statx */
   int command = STX_NORMAL;


   /* does the file exist? */
   if (statx (path, &statbuf,0, command) != PASS )
     {
       if (strstr (path,"share") == NIL(char) )
           return (TRUE);
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                 CMN_CANT_OPEN_D), inu_cmdname, path);
       return (-1);
     }

   if (!S_ISREG (statbuf.st_mode)) /* is a regular file */
      {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                          CMN_CANT_OPEN_D), inu_cmdname, path);
        return (-1);
      }

   /* How many blox does it take? */
   if ( (num_of_512_blocks_for_vpd = get_size_of_file (statbuf.st_size)) 
                                   == -1)
      {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                       CMN_CANT_OPEN_D), inu_cmdname, path);
        return (-1);
      }

  return (num_of_512_blocks_for_vpd);

}


/* ------------------------------------------------------------------------
*
* NAME: get_size_of_file 
* 
* FUNCTION: Round up the filesize
*
* DATA STRUCTURES:
*     parameters: filesize -- Actual file size
*     globals:    None
*
* RETURNS: (long)
*          file size in number of 512-byte blocks
*
* OUTPUT:
*          file size in number of 512-byte blocks
*
* ------------------------------------------------------------------------*/

long
get_size_of_file (long filesize)  /* filesize to be converted to block size */
{
  return ((long) ( (filesize + 511) /512) );
}

/* ------------------------------------------------------------------------
*
* NAME: validate_command_line 
*
* FUNCTION:
*       checks if the command has two arguments
*
* DATA STRUCTURES:
*     parameters:  argc
*                  argv 
*     global:
*
* RETURNS: (void)
*
* OUTPUT: exit on error. 
*
* ------------------------------------------------------------------------*/

void
validate_command_line (int argc,
                       char **argv)

{
   if ((argc != 3) && (argc != 4))
     {
       inu_msg (FAIL_MSG, MSGSTR (MS_CONVERT_VPD, CONVERT_MIGRATE_USAGE_E, 
                        CONVERT_MIGRATE_USAGE_D), inu_cmdname);
       exit (1);
     }

   vpdsrc=argv[1];
   vpdtarget=argv[2];
   if (argc == 4) {
       target_lpp_count=atoi(argv[3]);
   }
  
}

/* ------------------------------------------------------------------------
*
* NAME: enough_space_for_vpd
*
* FUNCTION:
*           Expands target fs if necessary
*
* DATA STRUCTURES:
*     parameters:  src_path 
*                  target_path
*                  auto_expand -- Always set to TRUE 
*     global:
*
* RETURNS: (boolean)
*     TRUE on success
*     FALSE on failure
*
* OUTPUT: expanded fs if necessary
*
* ------------------------------------------------------------------------*/

boolean
enough_space_for_vpd ( char *src_path,         /* src vpd path */
                       char *target_path,
                       boolean auto_expand)
{

   long num_of_512_blocks_for_vpd = 0,     /* Original size of the vpd */
        num_of_free_512_blocks_in_fs = 0,
        num_of_512_blocks_for_target_vpd = 0,
        num_of_512_blocks_needed = 0;      /* to expand */

   short i;                                /* An index */

   char *vpdfs,                            /* fs name */
        *vpd  []= {"lpp",                  /* which db are we dealing with? */
                   "product",
                   "history",
                   "inventory",
                   "eol"
                  },

        db [MAX_PATH_LT]="";               /* for db name */

   /*---------------------------------------------------------------------
   * Get sizes of all each source vpd, and vpd.vc, and add 'em up 
   * and check if we have enough space in target fs to store it      
   *---------------------------------------------------------------------*/
   num_of_512_blocks_for_target_vpd =  0;
   for (i=0; strcmp (vpd[i], "eol") != 0 ; i++) {

       sprintf (db,"%s/%s", src_path, vpd[i]);
       if ((num_of_512_blocks_for_vpd = get_size_of_vpd (db)) == -1)
          return (FALSE);
       num_of_512_blocks_for_target_vpd += num_of_512_blocks_for_vpd; 

       /* add the .vc value */
       strcat (db,".vc");
       if ((num_of_512_blocks_for_vpd = get_size_of_vpd (db)) == -1)
          return (FALSE);
       num_of_512_blocks_for_target_vpd += num_of_512_blocks_for_vpd; 

   } /* of for */
 
   /* See how much free space we have in target fs */
   if ((num_of_free_512_blocks_in_fs=get_free_blocks_in_filesystem(target_path))
                                                                      == -1) {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                CMN_CANT_OPEN_D), inu_cmdname,target_path);
       return (FALSE);
   }

   /* Is there a need to expand? */
   if ((num_of_512_blocks_needed =(num_of_512_blocks_for_target_vpd -
                                  num_of_free_512_blocks_in_fs)) > (long)0 ){

       /* get fs name for chfs */
       if ((vpdfs=get_fsname (target_path)) == NIL(char))
            return (FALSE);

       if (auto_expand)   /* this is always true */
          if ( !expand_my_lv (vpdfs, num_of_512_blocks_needed) )
              return (FALSE);
          else
             return (TRUE);
       else /* won't be hit */
         {
           inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NOSPACE_E, 
                              CMN_NOSPACE_D), inu_cmdname, vpdfs, 
                              num_of_512_blocks_needed);
           return (FALSE);
         }
   }

}
