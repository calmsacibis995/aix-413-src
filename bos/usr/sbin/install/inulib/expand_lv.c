static char sccsid[] = "@(#)54  1.11.1.10  src/bos/usr/sbin/install/inulib/expand_lv.c, cmdinstl, bos411, 9428A410j 6/3/94 10:44:13";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: CONVERGE_FACTOR
 *		expand_lv
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <inulib.h>
#include <sys/statfs.h>

#define CONVERGE_FACTOR(t) (size_t)(((size_t)(.05*t) + (size_t)1) / ((size_t)1))

extern char *inu_cmdname;
/*
 * NAME: expand_lv
 *
 * FUNCTION:
 *	Expand the specified logical volume (filesystem) by
 *	the appropriate amount.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 *
 * ON ENTRY:
 *	filesystem	string pointer to name of filesystem to be enlarged
 *	total_blox	integer number of 1024-byte blocks needed
 *
 * RETURNS:
 *	One is returned if chfs fails
 *	Zero is returned if enough space was acquired.
 */
int expand_lv(
char	*i_filesystem,		/* name of LV to enlarge		*/
size_t	total_blox_requested)	/* number of 1024-byte blocks needed	*/
{
    long   blks_per_partn;
    long   size_of_partition;
    int    num_of_partitions;
    char   command[255];
    char   filesystem[255];
    int    i, rc, err;
    size_t chfs_blox=0;            /* num blox converted into chfs blox      */
    size_t converge_factor=0;      /* in chfs blox                           */
    size_t orig_num_free_blox=0;   /* orig num of free blox before chfs call */
    size_t orig_total_blox=0;      /* orig num of total blox before chfs call*/
    size_t requested_blox=0;       /* orig num of total blox before chfs call*/
    size_t new_num_free_blox=0;    /* num free blox after chfs call          */
    struct statfs stat;

   /** ----------------------------------------------------------- *
    **  Get rid of trailing slashes "/" in the filesystem name.
    ** ----------------------------------------------------------- */
    strcpy(filesystem,i_filesystem);
    i=strlen(filesystem);
    i--;
    while(i>0 && filesystem[i]=='/')
    {
	filesystem[i]='\0';
    	i--;
    }

    /**********
    ** As of this moment, chfs expects 512 blocks.
    ***********/
    chfs_blox = total_blox_requested * 2;

    if ((err = statfs(filesystem, &stat)) != 0)
    {
        perror (inu_cmdname);
        return 1;
    }
    /*
     * Note on casting:  long long is only needed for intermediate result of
     * multiplication for large (gigabyte) filesystems.
     */
    orig_num_free_blox = (size_t)((((long long) stat.f_bfree  * 
                                   (long long) stat.f_bsize) 
                                            + 
                             (long long) 1023) / (long long) 1024) ;
    orig_total_blox    = (size_t)((((long long) stat.f_blocks * 
                                   (long long) stat.f_bsize) + 
                             (long long) 1023) / (long long) 1024) ;
    requested_blox     = total_blox_requested - orig_total_blox;

   /** ----------------------------------------------------------------- *
    **  while the amount free is less than what we asked for
    **      expand the filesystem
    **  The reason we have to expand in a loop is because the amount we 
    **  ask chfs to expand by is not converted into 100 % free space.  
    **  There is overhead associated with the filesystem (this ranges from
    **  as little as a 2 or 3 percent to as much as 7 or 8 percent).
    ** ----------------------------------------------------------------- */

    converge_factor += CONVERGE_FACTOR(requested_blox * 2);

    do
    {

       sprintf(command, "/usr/sbin/chfs -a size=%ld %s", 
             (chfs_blox + converge_factor), filesystem);
       rc = inu_do_exec (command);

       if (rc != 0)
	  return(1);

       if ((err = statfs(filesystem, &stat)) != 0)
       {
           perror (inu_cmdname);
           return (1);
       }
      /** ----------------------------------------------------------------- *
       ** bs = block size
       ** new number of free blox = (total BYTES free + bs - 1) / bs*
       ** ----------------------------------------------------------------- */
       new_num_free_blox = (size_t) ((((long long) stat.f_bsize * 
                                      (long long) stat.f_bfree) 
                                                     + 
                                      (long long) 1023) / (long long) 1024) ;

       converge_factor += CONVERGE_FACTOR(new_num_free_blox * 2);

    } while (new_num_free_blox  <  requested_blox); 

    return(0);
}
