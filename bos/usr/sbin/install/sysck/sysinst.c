static char sccsid[] = "@(#)16  1.2.1.13  src/bos/usr/sbin/install/sysck/sysinst.c, cmdinstl, bos411, 9428A410j 6/26/94 14:20:03";

/*
 * COMPONENT_NAME: cmdinstl
 *
 * FUNCTIONS: install_tcbents, compare_string, compare_list
 *            search_list, merge_classes, vpd_entry, 
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/audit.h>
#include <sys/mode.h>
#include <sys/fullstat.h>
#include <errno.h>
#include <stdio.h>
#include <swvpd.h>
#include <usersec.h>
#include "sysdbio.h"
#include "sysmsg.h"
#include "sysaudit.h"

extern	struct	tcbent	*gettcbstanza (AFILE_t);
extern	struct	tcbent	*gettcbent (void);
extern	struct  tcbent	*hash_lookup (char *);
extern	char	**null_list (char *);
extern	void	*xmalloc (unsigned);
extern char *deleteRootPath(char *);
extern char *addRootPath(char *);
extern char **deleteRootPathList(char **);
extern char **addRootPathList(char **);

/*
 * Global variables
 */

extern	int	verbose;
extern	int	tcb_cnt;
extern	int	Nflg;
extern	int	Rflg;
extern	int	rootflg;
extern	int	usrflg;
extern	int	shareflg;
extern  int     sflg;
extern	int	RootPath;
int	RootPathvpd[512];
int     force_size;
int     checksum_or_size_error;
extern	struct	tcbent	**tcb_table;
extern struct tcb_bucket        *tcb_ht[HASHSIZE]; /* hash table */

/*
 * This macro defines the permissions bits which cause a file to be
 * added to the database at installation time.  The purpose of this
 * is to have all files which are added with privilege be a member
 * of the SYSCK database so that tree checking will not turn off any
 * of the administrative permission bits.
 */

#define	ADD_MODE	(S_ITCB|S_ITP|S_ISUID|S_ISGID|S_ISVTX)

/*
 * NAME: compare_string
 *
 * FUNCTION: compare to string pointers for equivalence
 *
 * RETURNS: Zero if equivalent, non-zero otherwise
 */

int
compare_string (char *a, char *b)
{
	if (a && b && strcmp (a, b) == 0)
		return 0;

	if (a == (char *) 0 && b == (char *) 0)
		return 0;

	return -1;
}

/*
 * NAME: compare_list
 *
 * FUNCTION: compare two lists of string pointers for equivalence
 *
 * RETURNS: Zero if equivalent, non-zero otherwise
 */

int
compare_list (char **a, char **b)
{
	int	i;

	if (a == (char **) 0 && b == (char **) 0)
		return 0;

	if (a == (char **) 0 || b == (char **) 0)
		return -1;

	while (*a && *b) {
		if (compare_string (*a, *b))
			return -1;

		a++;
		b++;
	}
	if (*a == (char *) 0 && *b == (char *) 0)
		return 0;

	return -1;
}

/*
 * NAME: search_list
 *
 * FUNCTION: check if a string is in a list of strings
 *
 * RETURNS: Non-zero if the string is found, zero otherwise
 */

int
search_list (char **list, char *string)
{
	while (*list)
		if (strcmp (*list++, string) == 0)
			return 1;

	return 0;
}

/*
 * NAME: merge_classes
 *
 * FUNCTION: Take two lists of class names, and create a single list
 *
 * NOTES:
 *
 *	This routine takes two lists of classes and creates a third
 *	list which contains each entry at most once.  It will even
 *	clean house and not output duplicates which may have been
 *	present already.
 *
 * RETURNS:
 *	NONE.
 */

void
merge_classes (struct tcbent *tp, char *lppname)
{
	char	**old;
	char	**new;
	struct tcbent	*cp;
	int	i;
	int	total;
	char	file_name[128];

	/*
	 * Query the database for this entry and see what classes
	 * it has already.
	 */

	strcat (strcpy (file_name, "C:"), tp->tcb_name);

	if (cp = hash_lookup (file_name))
		old = null_list ((char *)cp);
	else
		old = (char **) 0;

	/*
	 * Add up the maximum number of elements which the output
	 * list can contain.  Don't forget to add one for the lppname
	 * entry.
	 */

	total = 0;

	if (old)
		for (i = 0;old[i];i++, total++)
			;

	if (tp->tcb_class)
		for (i = 0;tp->tcb_class[i];i++, total++)
			;

	total++;

	/*
	 * Allocate an arrary to hold the pointers from the various
	 * lists.
	 */

	new = (char **) xmalloc ((unsigned) (total + 1) * sizeof (char *));
	new[total = 0] = (char *) 0;

	/*
	 * Check each entry in the classes from the database and see
	 * if they are already in the output class.  Don't output
	 * duplicates.
	 */

	if (old) {
		for (i = 0;old[i];i++) {
			if (! search_list (new, old[i])) {
				new[total++] = old[i];
				new[total] = (char *) 0;
			}
		}
bzero(old,strlen(old)+1);
		free ((void *) old);
	}

	/*
	 * Check each entry in the classes from the installation file
	 * and see if they were in the set of classes from before.  This
	 * might happen on update or if the install is run more than
	 * once.  Don't output duplicates here either.
	 */

	if (tp->tcb_class) {
		for (i = 0;tp->tcb_class[i];i++) {
			if (! search_list (new, tp->tcb_class[i])) {
				new[total++] = tp->tcb_class[i];
				new[total] = (char *) 0;
			}
		}
bzero(tp->tcb_class,strlen(tp->tcb_class)+1);
		free ((void *) tp->tcb_class);
	}

	/*
	 * Check the lppname and see if someone already included it
	 * before now.  Same thing about dup's applies here as well.
	 * Make the final result the new class list for this entry.
	 */

	if (! search_list (new, lppname)) {
		new[total++] = lppname;
		new[total] = 0;
	}
	tp->tcb_class = new;
	tp->tcb_changed = 1;
}

/*
 * NAME: vpd_entry
 *
 * FUNCTION: Add or update a VPD entry to the database
 *
 * NOTES:
 *
 *	vpd_entry is called with a inv_t structure to be added or updated
 *	to the database.  A query is made, and if the item is returned
 *      an update is performed if the lpp id matched.  If item is not returned
 *      the entry is added.
 *
 * RETURNS:
 *	Zero for success, non-zero otherwise
 */

int
vpd_entry (inv_t *entry)
{
	inv_t	search;
        int     rc;
	char other_owner[MAX_LPP_NAME+1];
        memset (&search,0,sizeof(search));
        strcpy (search.loc0,entry->loc0);

	if ((rc = vpdget (INVENTORY_TABLE, INV_LOC0, &search)) == VPD_NOMATCH) {
		if (vpdadd (INVENTORY_TABLE, entry) != VPD_OK) {
			MSG_S(MSG_VPD_Add_Failed,DEF_VPD_Add_Failed,
                               entry->loc0,0,0,0);
		        return -1;
                }
        } else if (rc == VPD_OK) {
                /* Check for Two Owners */
                if (search.lpp_id == entry->lpp_id) {
                    /* _id is the ODM record number for this record */
                    entry->_id = search._id;

		    if (vpdchgget (INVENTORY_TABLE, entry) != VPD_OK) {
			MSG_S(MSG_VPD_Add_Failed,DEF_VPD_Add_Failed,entry->loc0,
                              0,0,0);
			return -1;
                    }
		
                }
                else {

		/*
		 * if the type is not a directory, then it's not good to have
                 * it owned by different lpp_id's, so we give a WARNING
		 * and continue 
                 */

	            if (entry->file_type != 1) {


			if (vpdreslpp_id(search.lpp_id,&other_owner) != VPD_OK) 
                        {
                           /*
                            * No match for this lpp id in the lpp database.
                            * Give a warning and delete the inventory entry.
                            */
			   MSG_S(MSG_No_LPP_for_Inv_Rec,DEF_No_LPP_for_Inv_Rec,
			 	entry->loc0,0,0,0);  
                           (void) vpddel (INVENTORY_TABLE, INV_LOC0, &search);
			}
                        else
                        {
			   MSG_S(MSG_VPD_Two_Owners,DEF_VPD_Two_Owners,
			 	search.loc0,&other_owner,0,0);  
                        }
				
		    }
			/*
			 * search again using the lpp_id 
			 * to get an exact match if there is one
			 */

        		memset (&search,0,sizeof(search));
        		strcpy (search.loc0,entry->loc0);
			search.lpp_id = entry->lpp_id;

        		if ((rc = vpdget (INVENTORY_TABLE, INV_LOC0|INV_LPP_ID, &search)) 
							== VPD_NOMATCH) {

			      if (vpdadd (INVENTORY_TABLE, entry) != VPD_OK) {
                        		MSG_S(MSG_VPD_Add_Failed,DEF_VPD_Add_Failed,
                               				entry->loc0,0,0,0);
                        		return -1;
                              }
                        }
			else if (rc == VPD_OK) {
                    		if (vpdchgget (INVENTORY_TABLE, entry) != VPD_OK) {
                        		MSG_S(MSG_VPD_Add_Failed,DEF_VPD_Add_Failed,entry->loc0,
                              				0,0,0);
                        		return -1;
				}
			}
			else {
               			MSG_S(MSG_VPD_Open_Failed2,DEF_VPD_Open_Failed2,
                                      "inventory",0,0,0);
               			return -1;
			}				

		}
	} else {
               MSG_S(MSG_VPD_Open_Failed2,DEF_VPD_Open_Failed2,
                     "inventory",0,0,0);
               return -1;
        }
               
	return 0;
}

/*
 * NAME: bld_inv
 *
 * FUNCTION: Builds up an inventory to add to ODM (vpd class)
 *
 * NOTES:
 *      The tcbent structure is used to create the vpd structure
 *      to add to the vpd class.
 *
 * RETURNS:
 *	NONE
 */

void bld_inv (inv_t *inv, struct tcbent *tcbent,short lpp_id)
{

        struct fullstat statbuf;
	/*
         * Update the vital product data.  We must register the
         * name, size, and checksum.  The macro ADD_MODE gives
         * the bits which force the file to be added to the
         * database.
         */

         tcbent->tcb_changed = 0;

         /*
          * Set up the entry for this file in the VPD.
          */

         strncpy (inv->loc0, deleteRootPath(tcbent->tcb_name),sizeof inv->loc0);

         /*
          * Initialize the inventory record.  Store the size,
          * the location, up to two links, and the checksum.
          */

         inv->private = 0;
         inv->format = INV_FILE;
         inv->lpp_id = lpp_id;

         fullstat(tcbent->tcb_name,FL_STAT|FL_NOFOLLOW,&statbuf);
         /*
          * The size will only be computed for regular files, and
          * then only if it isn't subject to change.
          */

         if (S_ISREG (statbuf.st_mode) && (!tcbent->tcb_vsize) && (tcbent->tcb_size != 0) )
         	inv->size = statbuf.st_size;
         else
                inv->size = 0;

         switch (tcbent->tcb_type) {
                case TCB_FILE:
			inv->file_type = 0;
                        break;
                case TCB_DIR:
			inv->file_type = 1;
                        break;
                case TCB_FIFO:
			inv->file_type = 2;
                        break;
                case TCB_BLK:
			inv->file_type = 3;
                        break;
                case TCB_CHAR:
			inv->file_type = 4;
                        break;
                case TCB_MPX:
			inv->file_type = 5;
                        break;
                case TCB_SYMLINK:
			inv->file_type = 6;
                        break;
                case TCB_LINK:
			inv->file_type = 7;
			break;
		default:
			inv->file_type = 0;
        }


         /*
          * If we are doing an installp -r (reject) operation,
          * then we will be passed the -F flag.  This means that we
          * should gripe about checksum and size errors, but to go
          * ahead and allow the file in (we don't clobber the ACL).
          * We still cut an audit record to say that the size and
          * checksum tests failed.  At the same time, we want to
          * store the 'correct' value of checksum and size into the
          * sysck database so that we can continue to gripe about
          * this in the future.
          */

          if (force_size) 
              inv->size = tcbent -> tcb_size;

          /*
           *  Add links to the vpd entry as a comma separated list
           */

          if (tcbent->tcb_links)
          	if (tcbent->tcb_links[0])
                	inv->loc1 = (char *)array_comma(deleteRootPathList(tcbent->tcb_links));

          /*
           *  Add symlinks to the vpd entry as a comma separated list
           */

          if (tcbent->tcb_symlinks)
                  if(tcbent->tcb_symlinks[0])
                         inv->loc2 = (char *)array_comma(deleteRootPathList
                                       (tcbent->tcb_symlinks));

          /*
           *  Add the target entry to the loc2 entry (loc2 is overloaded)
           */

          if (tcbent->tcb_target)
                         inv->loc2 = tcbent->tcb_target;

          /*
           * A checksum will only be computed for regular files, and
           * then only if it isn't subject to change.  The sum may
           * have been computed earlier, in which case I use that
           * value.
           */

          if (S_ISREG (statbuf.st_mode) && ! tcbent->tcb_vchecksum) {
                 if (tcbent->tcb_vsum)
                                inv->checksum = tcbent->tcb_sum;
                 else
                                inv->checksum = mk_checksum (tcbent->tcb_name);
          } else
                 inv->checksum = 0;
}

/*
 * NAME: install_tcbents
 *                                                                    
 * FUNCTION: Install a file of entries, updating the configuration files
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Reads a file full of stanzas and adds entries which do not
 *	exist already, or updates entries which already existed.  Only
 *	members of the TCB are actually added to the configuration file.
 *	All file are added to the Vital Products Manager.
 *
 * RETURNS: Zero on success, non-zero otherwise
 */  

int
install_tcbents (
	char	*file,	/* Name of the file to be read for new entries */
	char	*lppname) /* Name of the LPP being installed */
{
	int	i;
	int	rc;
	int	errors = 0;
	int	exists;
	int	save;
	struct	tcbent	*tcbent, *tp;
        struct tcb_bucket *bp;
	inv_t	inv;
	lpp_t	lpp;
	struct  fullstat  statbuf;
	time_t	last_time = time ((time_t *) 0);
	int	file_cnt = 0;
	AFILE_t	fp;
        FILE    *sfp;
	char	**classes;
	char	file_name[128];
	char	*class_list;
        short   lpp_id;

	/*
	 * Set the flags how we want them
	 */

	verbose = 0;	/* No warning messages */

	/*
	 * Initialize the LPP table.  The LPP table record should
	 * already exist for INSTALLP installs, but we add it here
	 * in case the user is adding their own inventory data.
	 */

		if (Rflg)
			strcpy(RootPathvpd,RootPath);
		else
			RootPathvpd[0] = '\0';
		if (usrflg)
			strcat(RootPathvpd,VPD_USR_PATH);
		else if (shareflg)
			strcat(RootPathvpd,VPD_SHARE_PATH);
		else if (rootflg)
			strcat(RootPathvpd,VPD_ROOT_PATH);
		else
			strcat(RootPathvpd,VPD_LOCAL_PATH);

		if (Rflg || usrflg || shareflg || rootflg)
			if (vpdremotepath((char *)RootPathvpd) != VPD_OK) {
                           MSG_S(MSG_VPD_Open_Failed,DEF_VPD_Open_Failed,
                                 RootPathvpd,0,0,0);
                           exit(1);
                        }
			else
				vpdremote();

		memset (&lpp, 0, sizeof lpp);
		strncpy (lpp.name, lppname, sizeof lpp.name);

		if ((rc = vpdget (LPP_TABLE, LPP_NAME, &lpp)) != VPD_OK &&
				(rc = vpdadd (LPP_TABLE, &lpp)) != VPD_OK) {
                    MSG_S(MSG_VPD_Open_Failed,DEF_VPD_Open_Failed,0,0,0,0);
                           exit(EACCES);
                }

	/*
	 * Open the stanza file.  The stanza's to be installed are
	 * read from this file.
	 */

	if ((fp = afopen (file)) == 0) {
           MSG_S(MSG_No_Install_File,DEF_No_Install_File,file,0,0,0);
           exit(ENOENT);
        }

	/*
	 * Read the entire stanza file, one stanza at a time.  Each
	 * stanza is tested for validity and duplication.  The file is
	 * then checked according to the usual rules for file checking.
	 * Additionally, if the file is in the TCB it must be added to
	 * the vital product data.
	 */
	tp = NULL;
	while ((tcbent = gettcbstanza (fp)) != (struct tcbent *)EOF) {
		if (tcbent == 0) { /* Bad entry, don't process */
                   errors++;
                   continue;
                }


		file_cnt++;

		/*
		 * The class name comes from the command line `lppname'
		 * argument, unless there were user-defined classes on
		 * this entry.  It is also possible that there were
		 * already classes for this entry, in which case we
		 * very carefully merge the two together.
		 */

		merge_classes (tcbent, lppname);

		force_size = FALSE;
		checksum_or_size_error = FALSE;
		if ( ( (rc = ck_tcbent (tcbent)) == -1)
				||
		     (checksum_or_size_error) )
		{
			MSG_S(MSG_Install_Failed,DEF_Install_Failed,
                              tcbent->tcb_name,0,0,0);
			last_time = time ((time_t *) 0);
		        errors++;
		}
                else
                        /* Add tcbent entry to the hash table */
                        /* hash lookup was done in ck_tcbent */
                 	hash_install (tcbent);
		/*
		 * Audit the status of the installation of this file
		 */

		mk_audit_rec (SYSCK_Install, rc ? AUDIT_FAIL:AUDIT_OK,
			tcbent->tcb_name, lppname, NULL);
	}

        /*
         * Since we have a good tcbent table, setup the inventory 
         * entries.
         */

        if (vpdreslpp_name(lppname,&lpp_id) != VPD_OK) {
           MSG_S(MSG_VPD_Open_Failed,DEF_VPD_Open_Failed,lppname,0,0,0);
           exit(1);
        }

        if (sflg)
           if (! (sfp = fopen (save_file,"w"))) {
              MSG_S(MSG_Can_Not_Open,DEF_Can_Not_Open,save_file,0,0,0);
              return -1;
        }


        for (i=0;i<HASHSIZE;i++) {  /* walk through the hash table */
		for (bp=tcb_ht[i];bp!=NULL;bp=bp->next) {
                        if (!Nflg && !sflg) {
                           memset (&inv, 0, sizeof inv);
			   bld_inv (&inv,bp->tcbp,lpp_id);

                 	   /* Update the SWVPD */
                	   if (vpd_entry (&inv) == -1)
                                   errors++;
                    	   free(inv.loc1);
                    	   free(inv.loc2);
			}
                        if(sflg)
                           write_tcbent (sfp,bp->tcbp,inv);       
                }
        } 
        if (sflg)
           fclose (sfp); 
        /*
         * Report the status of the installation.  Returning with
         * an error will prevent the SYSCK database from being
         * updated with the new records.
         */
        if (errors) {
                MSG_S(MSG_Installp_Failed,DEF_Installp_Failed,lppname,0,0,0);
                return ENOTRUST;
        }

        /*
         * close the vpd 
         */
        (void) vpdterm ();

        return 0;
}
