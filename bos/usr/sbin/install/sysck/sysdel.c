static char     sccsid[] = "@(#)14  1.3.1.11  src/bos/usr/sbin/install/sysck/sysdel.c, cmdinstl, bos41J, 9512A_all 3/15/95 14:26:15";

/*
 * COMPONENT_NAME: cmdinstl
 *
 * FUNCTIONS: rm_links, del_tcbents
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/audit.h>
#include <sys/fullstat.h>
#include <sys/stat.h>
#include <sys/mode.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <swvpd.h>
#include <usersec.h>
#include "sysdbio.h"
#include "sysmsg.h"
#include "sysaudit.h"

extern  struct  tcbent  *gettcbent();
extern  struct  tcbent  *gettcbstanza();
extern  struct  tcbent  *hash_lookup();
extern  char    *xstrdup();
extern  char    *deleteRootPath(char *);
extern  char    *addRootPath(char *);
extern  char    **deleteRootPathList(char **);
extern  char    **addRootPathList(char **);
extern  char    *strchr (char *,int);

extern  int     verbose;

extern  struct  tcbent  **tcb_table;
extern  int     tcb_cnt;

extern  int     uflg;
extern  int     Nflg;
extern  int     Rflg;
extern  int     sflg;
extern  int     rootflg;
extern  int     usrflg;
extern  int     shareflg;
extern  int     RootPath;
extern  int     RootPathvpd[512];
extern  struct  tcb_bucket        *tcb_ht[HASHSIZE]; /* hash table */

#define ADD_MODE        (S_ITCB|S_ITP|S_ISUID|S_ISGID|S_ISVTX)

/*
 * NAME: rm_links
 *
 * FUNCTION: Calls unlink to remove symbolic or hard links 
 *           for list given.
 *
 * RETURNS: NONE.  
 *          Calls xperror on error.
 */

void rm_links (struct tcbent *tp)
{
        int i;
                for (i = 0;tp->tcb_links && tp->tcb_links[i];i++){
                     unlink (tp->tcb_links[i]);
                }

                for (i = 0;tp->tcb_symlinks && tp->tcb_symlinks[i];i++){
                        unlink (tp->tcb_symlinks[i]);
                }

		if (tp->tcb_type == TCB_LINK) 
                     unlink (tp->tcb_name);

		if (tp->tcb_type == TCB_SYMLINK) 
                       unlink (tp->tcb_name);
	
}

/*
 * NAME: rm_inv_links
 *
 * FUNCTION: Calls unlink to remove symbolic or hard links
 *           for given list of comma separated links.
 *
 * RETURNS: NONE.
 */

void rm_inv_links (char *list)
{

char *p0;
char *p1;
char *p2;

    p0 = p1 = list;  

    while (p0 != NULL) {

       p1 = strchr(p0,',');

       if (p1 != NULL) {
        p2 = p1;     
        p2++;
       }
       else
        p2 = NULL;

       if (p1 != NULL)    
          *p1 = '\0';   

       unlink(p0);

       p0 = p2;

     }   

}


/*
 * NAME: del_tcbents
 *
 * FUNCTION: Delete TCB entries named in a file.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      User process
 *
 * NOTES:
 *      Scans the a stanza file removing entries matched in the TCB
 *      file.  Also optionally removes inventory file data.
 *
 *      When uninstalling things we remove the entry
 *      from the swvpd.  We also unlink any hard or
 *      soft links SYSCK created.  This is relatively
 *      safe as you can not have a hard link specified to a
 *      directory.
 *
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */

int
del_tcbents (file, lppname)
char    *file;
char    *lppname;
{
        AFILE_t fp;
        FILE    *sfp;
        struct  tcbent  *tp;
        struct  tcb_bucket *bp;
        struct  fullstat sb;
        int     i,j;
        int     rc;
        int     once = 0;
        int     errors = 0;
        int     lastrc;
        short   lpp_id = -1;
        inv_t   inv;
        lpp_t   lpp;

        /*
         * It is expected that there may be some errors from the
         * input file.  It would be best to specify exactly which
         * error messages we want output.  For DELETE mode we
         * assume the invoker knows what they are doing and wants
         * to see all error messages.  For UNINSTALL mode we
         * assume the invoker is INSTALLP or some such and is only
         * calling SYSCK to cleanup a database which may or may
         * not be complete.
         */

        verbose = 0;

        /*
         * Open the named stanza file and read in the stanzas.  The
         * file name in each stanza is then used to delete the named
         * stanza from the TCB file.
         */

        if (! (fp = afopen (file))) {
                MSG_S(MSG_No_Such_File,DEF_No_Such_File,file,0,0,0);
                return ENOENT;
        }
        /* Open the swvpd database */
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

        /*
         * Get the LPP id
         */
	if ((lppname != NULL) || (!uflg)) {
            if (vpdreslpp_name(lppname,&lpp_id) != VPD_OK) {
                    MSG_S(MSG_VPD_Open_Failed,DEF_VPD_Open_Failed,
                          lppname,0,0,0);
                    exit(1);
            }
	}

        /*
         * Open inventory save file
         */
        if (sflg)
           if (! (sfp = fopen (save_file,"w"))) {
              MSG_S(MSG_Can_Not_Open,DEF_Can_Not_Open,save_file,0,0,0);
              return -1;
           }


       /*
        * Read all of the stanzas to be removed from the
        * swvpd inventory database.
        */

        while ((tp = gettcbstanza (fp)) != (struct tcbent *)EOF) {
           if (tp == 0) { /* Bad entry, don't process */
             errors++;
             continue;
           } /* end if */
           if (hash_lookup (tp->tcb_name) == NULL)  /* OK entry */
                    hash_install(tp);
           else {
                    MSG_S(MSG_Duplicate_Stanza,DEF_Duplicate_Stanza,
                          tp->tcb_name,0,0,0);
           }
        }
        for (i=0;i<HASHSIZE;i++) {  /* walk through the hash table */
         for (bp=tcb_ht[i];bp!=NULL;bp=bp->next) {

           memset (&inv, 0, sizeof inv);
           strcpy (inv.loc0,bp->tcbp->tcb_name);

	   /*
	    * Since we are going to remove an entry, we should search
	    * on the name and lpp_id.  If there are duplicates, it's
	    * too late to complain.
            */

	   if (lpp_id != -1) {
	       inv.lpp_id = lpp_id;

               if (vpdget (INVENTORY_TABLE, INV_LOC0|INV_LPP_ID, &inv) == VPD_OK){
                        if (sflg) 
                           write_tcbent(sfp,bp->tcbp,inv);
		        else {
                           if (uflg) {
                              bp->tcbp->tcb_type = inv.file_type;
                              rm_links(bp->tcbp);
                              if (inv.loc1[0] != '\0')
                                 rm_inv_links(inv.loc1);
                              if ((inv.loc2[0] != '\0') && \
                                   (bp->tcbp->tcb_type != TCB_SYMLINK))
                                 rm_inv_links(inv.loc2);
                           }
                           if (!Nflg)
                              (void) vpddel(INVENTORY_TABLE,INV_LOC0|INV_LPP_ID,&inv);
		        }
               }
	   } else {
               lastrc = vpdget (INVENTORY_TABLE, INV_LOC0, &inv);
	       while ( lastrc == VPD_OK) {
                        if (sflg) 
                           write_tcbent(sfp,bp->tcbp,inv);
		        else {
                           if (uflg) {
                              bp->tcbp->tcb_type = inv.file_type;
                              rm_links(bp->tcbp);
                              if (inv.loc1[0] != '\0')
                                 rm_inv_links(inv.loc1);
                              if ((inv.loc2[0] != '\0') && \
                                   (bp->tcbp->tcb_type != TCB_SYMLINK))
                                 rm_inv_links(inv.loc2);
                           }
                           if (!Nflg)
                              (void) vpddel(INVENTORY_TABLE,INV_LOC0,&inv);
		        }
			lastrc = vpdgetnxt (INVENTORY_TABLE, &inv);
		}
	   }
         }
        }
        /* Close save file */
        if (sflg)
           fclose (sfp);

        if (errors)
           return ENOTRUST;

        /*
         * Close the SWVPD.
         */

        (void) vpdterm ();

        return 0;
}
