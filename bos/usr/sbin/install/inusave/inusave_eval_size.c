static char sccsid[] = "@(#)16	1.7.1.1  src/bos/usr/sbin/install/inusave/inusave_eval_size.c, cmdinstl, bos411, 9435B411a 8/26/94 15:40:46";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inusave_eval_size
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <inu_eval_size.h>
#include <inuerr.h>
#include <sys/stat.h>
#include <time.h>


#define AR_MEMNM_MAX 255        /* Maximum length of an archive member name */

extern int archive_member_fs_size ();


int inusave_eval_size(char *savelist)
{
	FILE	*fp;				/* size file pointer */
	size_t total_size=0;
	size_t *totals;
	char *inusavedir;
	int i;
	int rc;
        struct stat acf_stat;               /* archive control file stats */

	if ((rc = mk_fs_list()) != INUGOOD)
		return(rc);
        totals = (size_t *)calloc(sizeof(size_t),nfs+1);

	if((fp = fopen(savelist,"r")) != NULL) {
		inu_fget_file_sizes(fp,totals,0);
		(void) fclose(fp);
	}
	
	for(i=0;i<=nfs;i++)
		total_size=total_size+totals[i];

        /* Add in any archive member requirements */
        /* If there are any archive members to be saved, account for those */
        if ((stat("lpp.acf",&acf_stat) != -1) && (acf_stat.st_size !=0)) {
            total_size += size_archive_members();
        }

	fs_total[2]=total_size;

        free(totals);
	return(enough_space("inusave"));
}


/*
 * Account for sizes of archive members.  These are not normally found
 * by inu_fget_file_sizes() so we must process them specially.
 */

int size_archive_members()
{
        FILE *member_check;              /* Stream for ar tv arch member cmd */
        FILE *acf_fp;                    /* Archive control file pointer */
        char apply_member [PATH_MAX+1];  /* Archive member as in apply list */
        char ar_name [PATH_MAX+1];       /* Archive path - maybe relative */
        char * ar_member;                /* Archive member without path */
        char * ar_path;                  /* Archive member with path */
        char mem_chk_cmd[PATH_MAX+AR_MEMNM_MAX+8];      /* Holds ar tv cmd */
        char mem_chk_out[32+NLTBMAX+AR_MEMNM_MAX];   /* Holds ar tv output */
        int member_length;               /* Length in bytes of archive member */
        int matched_items;               /* Number of entries on acf line */
        int ar_member_fs_total = 0;      /* Filesystem size for saved members */

        if ((acf_fp = fopen("lpp.acf", "r")) == NULL) {
            return(0);
        }

        /* Loop through the archive control file and gather size requirements */
        while ((matched_items = fscanf(acf_fp,"%s %s",
                                                apply_member,ar_name)) != EOF) {             /* Throw out garbage lines */
               if (matched_items != 2) {
                   continue;
               }

               /* Get name of member */
               ar_member = (char *) get_member (apply_member);

               /* Get a non-relative path to the archive */
               if (ar_name[0] == '.') {
                   ar_path = &ar_name[1];
               } else {
                   ar_path = &ar_name[0];
               }

               /* Determine if member is in archive on existing system */
               if (sprintf(mem_chk_cmd,"/usr/ccs/bin/ar tv %s %s 2>/dev/null",
                                                    ar_path,ar_member) > 0) {
                   if ((member_check = popen(mem_chk_cmd,"r")) != NULL) {
                        /* Grab the output of ar tv libname membername */
                        if (fgets(mem_chk_out,
                                sizeof(mem_chk_out),member_check) == NULL) {
                            continue;
                        }

                        /* Close the ar tv output stream */
                        (void) pclose (member_check);

                        /* Grab the member size out of the resulting output */
                        /* The member sizes SHOULD be based upon what the
                         * blocksize of the save directory is.  The ar output
                         * gives the member length in bytes.  This must be
                         * converted to blocks, then to the 512-byte blocksize
                         * used internally by installp.
                         *
                         * Given time, this code should be changed to use the
                         * block sizes from the save directory filesystem.
                         * Unfortunately at this point in the code, there is
                         * no guarantee that the save directory will exist or
                         * that INUSAVEDIR will be defined, at least until
                         * changes are made in inusave.c.
                         *
                         * Also, there has been no check put in to see that
                         * the member didn't actually already exist on the
                         * filesystem as it appears in the apply list.  This
                         * is an unlikely case, but if it does occur, then the

                        * space for that member will be counted twice.
                         */
                        member_length = 0;
                        if (sscanf(mem_chk_out,
                                      "%*s %*d/%*d %d",&member_length) == 1) {
                            ar_member_fs_total +=
                              ((((member_length+4095)/4096) * 4096) + 511)/512;
                        }
                   }
               }
        }

        fclose(acf_fp);
        return ar_member_fs_total;
}

