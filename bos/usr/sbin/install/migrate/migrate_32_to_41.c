static char sccsid[] = "@(#)96  1.15  src/bos/usr/sbin/install/migrate/migrate_32_to_41.c, cmdinstl, bos411, 9428A410j 7/1/94 17:26:59";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: enough_space_for_vpd
 *		expand_my_lv
 *		get_free_blocks_in_filesystem
 *		get_history_database
 *		get_inventory_database
 *		get_lpp_database
 *		get_product_database
 *		get_size_of_file
 *		main
 *		print_history_stanza
 *		print_inventory_stanza
 *		print_lpp_stanza
 *		print_product_stanza
 *		create_exceptions_table
 *		hashname
 *		omit_from_vpd
 *              is_usr_root_update_lpp_entry
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

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/

#include <migrate.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

/* static nl_catd catd; */

char *vpd_path_32; 
char *vpd_path_41; 
int  target_lpp_count = 0; 
char *inu_cmdname;

#define HASHTABLESIZE 512
struct filehash {
    char *name;
    struct filehash *next;
} exceptions[HASHTABLESIZE];

/* We get the required space for the corresponding 4.1 vpd also */
long num_of_512_blocks_for_41_vpd = 0;

static void no_space (void);

/*--------------------------------------------------------------------------*
**
** NAME: main (migrate_32_to_41)
**
** FUNCTION:
**  Obtain the following vpds:
**      a. lpp
**      b. product
**      c. history
**      d. inventory
**  for USR, SHARE, and ROOT parts and perform the filtering action needed
**  to migrate them to AIX 4.1.  The filtered vpds will be placed in
**  the same directories as the original vpds. 
**
** EXECUTION ENVIRONMENT: 
**   Can be executed by root user only
**
** Usage:
**        migrate_32_to_41  <32_vpd_path> <41_vpd_path>
**  
** Arguments: 
**       32_vpd_path  -- path where 32 VPD is located
**       41_vpd_path  -- path where 41 VPD will be located
**
** RETURNS: 0 on success, non-zero on failure
**
**-------------------------------------------------------------------------*/

int
main (int    argc, 
      char   **argv)
{
   /* the three VPD paths */

   /*-----------------------------------------------------------------------*
   ** NOTE: DO NOT change the order of elements !  ! 
   ** vpd[], and get_database[] are parallel arrays
   **-----------------------------------------------------------------------*/
     
   char  *vpd []={LPP, 
                  PRODUCT, 
                  HISTORY, 
                  INVENTORY, 
                  LAST_ELEMENT
                 };

   /*-----------------------------------------------------------------------*
   ** NOTE: DO NOT change the order of elements !  ! 
   ** vpd[], and get_database[] are parallel arrays
   ** get_database is an array of pointers to function.
   **-----------------------------------------------------------------------*/

   typedef boolean (*gdb) (char *, char *);

   gdb get_database []  = { get_lpp_database, 
                            get_product_database, 
                            get_history_database, 
                            get_inventory_database
                          };


   char  vpdsrc    [MAX_PATH_LT],  /* the source filename */
         vpdtarget [MAX_PATH_LT];  /* the target filename (*.32.add) */

   int i, j;  /* generic counters */

   inu_cmdname=getbasename (argv[0]);      /* For error messages */

   validate_command_line (argc, argv);

   setlocale (LC_ALL, "");

   CATOPEN ();

  /*
   inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_PLEASE_WAIT_I, 
                              CMN_PLEASE_WAIT_D), inu_cmdname);

  */
   umask ((mode_t) (S_IWGRP | S_IWOTH));    /* set umask to 022     */
   
   /* -----------------------------------------------------------------------
   ** for vpd in "lpp product history inventory"
   ** do
   **    get_the_converted_database  
   ** done
   *-----------------------------------------------------------------------*/

   /* set vpd path */
   vpdlocalpath (vpd_path_32);
   vpdlocal ();

   for (j=0; strcmp (vpd[j], LAST_ELEMENT) != 0; j++) {

        sprintf (vpdsrc, "%s/%s", vpd_path_32, vpd[j]);    /* source name */
        /* Form the target name */
        sprintf (vpdtarget, "%s/%s%s", vpd_path_41, vpd[j], TARGET_SUFFIX); 
        if (((boolean) (*get_database[j])  (vpdsrc, vpdtarget)) != TRUE) {
              inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, 
                                         CMN_VPDERR_D), inu_cmdname);
              exit (2);
        }
    }

  /* now get the space for 4.1 vpds */
  if ( !enough_space_for_41_vpd (vpd_path_41,TRUE,num_of_512_blocks_for_41_vpd))
      exit (2);

  CATCLOSE ();

  return (0);  /* Success */

} /* end main */

/* ------------------------------------------------------------------------
*
* NAME: get_product_database
* 
* FUNCTION:    1. Get 32 product database (U/S/R)
*              2. Do the required filtering to migrate to 4.x
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
*        Write the 4.x into a file in the same directory as 
*        the original with the suffix TARGET_SUFFIX (migrate.h).
*
* ------------------------------------------------------------------------*/

boolean
get_product_database  (char *vpd_file,  /* original 3.2.x vpd */
                       char * vpdadd_file) /* 4.x output .add file */
{
  int rc=FALSE;        /* rteurn code */ 
  prod_t prod;   /* product struct */
  FILE *fp;      /* For output file */
  char vc [MAX_PATH_LT]=""; 
  

  memset (&prod, NULL, sizeof (prod_t));  /* initialize prod struct */

  /* ------------------------------------------------------------
  ** See if we have enough disk space for the output file, 
  ** If not expand the fs
  ** ------------------------------------------------------------*/
  if ( ! enough_space_for_vpd (vpd_file, TRUE))    
     return (rc);

  /* ------------------------------------------------------------
  ** See if we have enough disk space for the output file.vc, 
  ** If not expand the fs
  ** ------------------------------------------------------------*/
  sprintf (vc, "%s.vc", vpd_file);
  if ( ! enough_space_for_vpd (vc, TRUE))    
     return (rc);

  rc = vpdget (PRODUCT_TABLE, VPD_ALL, &prod);  /* get the first record */
 

  if (rc != VPD_OK)
    {
      if (strstr (vpd_file, "share") != NIL (char)) 
           return (TRUE);       /* share vpd was empty */

      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                 inu_cmdname);
      exit (FALSE);
    }

  if ((fp = fopen (vpdadd_file, "w")) == (FILE *)0)   /* open the output file */
    {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, vpdadd_file);
       return (rc);
    }

  while (rc == VPD_OK)
   {
     print_product_stanza (prod, fp);
     vpd_free_vchars (PRODUCT_TABLE, &prod);
     rc = vpdgetnxt (PRODUCT_TABLE, &prod);
     if (rc == VPD_SYS  ||  rc == VPD_BADCNV)
       {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                   inu_cmdname);
        fclose (fp);
        exit (FALSE);
       }
   }

  fclose (fp);
  return (rc=TRUE);
}

/* ------------------------------------------------------------------------
*
* NAME: get_lpp_database
*
* FUNCTION:    1. Get 32 lpp database (U/S/R)
*              2. Do the required filtering to migrate to 4.x
*
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*    exits on vpd errors.
*
* RETURNS: (boolean)
*     TRUE   on success
*     FALSE  failure
*
* OUTPUT:
*        Write the 4.x into a file in the same directory as
*        the original with the suffix TARGET_SUFFIX (migrate.h).
*
* ------------------------------------------------------------------------*/

boolean
get_lpp_database  (char *vpd_file,     /* original 3.2.x vpd */ 
                   char * vpdadd_file) /* 4.x output .add file */
{
  int rc=FALSE; /* return code */
  lpp_t lpp;    /* lpp struct */
  FILE *fp;     /* For output file */
  char vc [MAX_PATH_LT]="";
  boolean doing_root_vpd;

  memset (&lpp, NULL, sizeof (lpp_t));    /* initialize */ 

  /* ------------------------------------------------------------
  ** See if we have enough disk space for the output file, 
  ** If not expand the fs
  ** ------------------------------------------------------------*/
  if ( ! enough_space_for_vpd (vpd_file, TRUE))
      return (rc); 

  /* ------------------------------------------------------------
  ** See if we have enough disk space for the output file.vc, 
  ** If not expand the fs
  ** ------------------------------------------------------------*/
  sprintf (vc, "%s.vc", vpd_file);
  if ( ! enough_space_for_vpd (vc, TRUE))    
     return (rc);

  rc = vpdget (LPP_TABLE, VPD_ALL, &lpp); /* get the first record */

  if (rc != VPD_OK)
    {
      if (strstr (vpd_file, "share") != NIL (char))
           return (TRUE);       /* share vpd was empty */

      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                 inu_cmdname);
      exit (1);
    }

  if ((fp = fopen (vpdadd_file, "w")) == (FILE *)0)  /* open the output file */
    {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, vpdadd_file);
       return (rc);
    }

  /*
   * Set a flag if the vpd_file indicates that we are working on the
   * root vpd.  (will be used in print_lpp_stanza)
   */
  doing_root_vpd = (strstr (vpd_file, ROOT_VPD_PATH) != NIL (char));

  while (rc == VPD_OK)
   {

     print_lpp_stanza (lpp, fp, doing_root_vpd);
     vpd_free_vchars (LPP_TABLE, &lpp);
     rc = vpdgetnxt (LPP_TABLE, &lpp);
     if (rc == VPD_SYS  ||  rc == VPD_BADCNV)
       {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                   inu_cmdname);
        fclose (fp);
        exit (FALSE);
       }
   }

  fclose (fp);

  return (rc=TRUE);
}

/* ------------------------------------------------------------------------
*
* NAME: get_inventory_database
*
* FUNCTION:    1. Get 32 inventory database (U/S/R)
*              2. Do the required filtering to migrate to 4.x
*
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*    exits on vpd errors.
*
* RETURNS: (boolean)
*     TRUE   on success
*     FALSE  failure
*
* OUTPUT:
*        Write the 4.x into a file in the same directory as
*        the original with the suffix TARGET_SUFFIX (migrate.h).
*
* ------------------------------------------------------------------------*/

boolean
get_inventory_database  (char *vpd_file,     /* original 3.2.x vpd */
                       char * vpdadd_file) /* 4.x output .add file */
{
  int rc=FALSE;        /* return code */
  inv_t inv;     /* inv struct */
  FILE *fp;      /* For output file */
  FILE *exceptions = (FILE *) NULL;  /* List of entries to not add to target */
  char exceptfn[MAX_PATH_LT];
  char *vpdbase;
  char vc [MAX_PATH_LT]="";


  memset (&inv, NULL, sizeof (inv_t));  /* initialize */

  /* ------------------------------------------------------------
  ** See if we have enough disk space for the output file, 
  ** If not expand the fs
  ** ------------------------------------------------------------*/
  if ( ! enough_space_for_vpd (vpd_file, TRUE))
     return (rc);

  /* ------------------------------------------------------------
  ** See if we have enough disk space for the output file.vc, 
  ** If not expand the fs
  ** ------------------------------------------------------------*/
  sprintf (vc, "%s.vc", vpd_file);
  if ( ! enough_space_for_vpd (vc, TRUE))    
     return (rc);

  /* ------------------------------------------------------------
  ** Check for the existence of an exceptions list, things which
  ** we should not add to the target VPD.  This is necessary 
  ** because we cannot run sysck against the 3.2 VPD, and we cannot
  ** run the 3.2 sysck on 4.1.  Therefore, the files to be eliminated
  ** from bos.obj at BOS install time are simply listed in files,
  ** bos.rte.usr.rmlist and bos.rte.root.rmlist
  ** ------------------------------------------------------------*/

  strcpy(exceptfn,vpd_file);
  if ((vpdbase = strstr(exceptfn,"/usr/lib/objrepos")) != NULL) {
	*vpdbase = '\0';
	strcat(exceptfn,"/bos.rte.usr.rmlist");
  } else if ((vpdbase = strstr(exceptfn,"/etc/objrepos")) != NULL) {
	*vpdbase = '\0';
	strcat(exceptfn,"/bos.rte.root.rmlist");
  } else {
	exceptfn[0] = '\0';
  }

  /* Open the exceptions file, if it exists */
  if (exceptfn[0] != '\0') {
	exceptions = fopen(exceptfn,"r");
  }

  /*
   * Read the exceptions list into a hash table.
   */
  if (exceptions != (FILE *) NULL) {
      rc = create_exceptions_table(exceptions);
      unlink(exceptfn);
  }

  rc = vpdget (INVENTORY_TABLE, VPD_ALL, &inv);  /* get the first record */

  if (rc != VPD_OK)
    {
      if (strstr (vpd_file, "share") != NIL (char))
           return (TRUE);       /* share vpd was empty */

      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                 inu_cmdname);
      exit (FALSE);
    }

  if ((fp = fopen (vpdadd_file, "w")) == (FILE *)0) /* open the output file */
    {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, vpdadd_file);
       return (rc);
    }

  while (rc == VPD_OK)
   {
     if ((exceptions == (FILE *) NULL) || (omit_from_vpd(inv.loc0) == FALSE)) {
           print_inventory_stanza (inv, fp);
     }
     vpd_free_vchars (INVENTORY_TABLE, &inv);
     rc = vpdgetnxt (INVENTORY_TABLE, &inv);
     if (rc == VPD_SYS  ||  rc == VPD_BADCNV)
       {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                   inu_cmdname);
        fclose (fp);
        exit (FALSE);
       }
   }

  fclose (fp);
  return (rc=TRUE);
}

/* ------------------------------------------------------------------------
*
* NAME: get_history_database
*
* FUNCTION:    1. Get 32 history database (U/S/R)
*              2. Do the required filtering to migrate to 4.x
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*    exits on vpd errors.
*
* RETURNS: (boolean)
*     TRUE   on success
*     FALSE  failure
*
* OUTPUT:
*        Write the 4.x into a file in the same directory as
*        the original with the suffix TARGET_SUFFIX (migrate.h).
*
* ------------------------------------------------------------------------*/

boolean
get_history_database  (char *vpd_file,     /* original 3.2.x vpd */
                       char * vpdadd_file) /* 4.x output .add file */
{
  int rc=FALSE;        /* return code */
  hist_t hist;   /* hist struct */ 
  FILE *fp;      /* For output file */
  char vc [MAX_PATH_LT]="";

  memset (&hist, NULL, sizeof (hist_t));  /* initialize */

  /* ------------------------------------------------------------
  ** See if we have enough disk space for the output file, 
  ** If not expand the fs
  ** ------------------------------------------------------------*/
  if ( ! enough_space_for_vpd (vpd_file, TRUE))
     return (rc);

  /* ------------------------------------------------------------
  ** See if we have enough disk space for the output file.vc, 
  ** If not expand the fs
  ** ------------------------------------------------------------*/
  sprintf (vc, "%s.vc", vpd_file);
  if ( ! enough_space_for_vpd (vc, TRUE))    
     return (rc);

  rc = vpdget (HISTORY_TABLE, VPD_ALL, &hist); /* get the first record */
  
  if (rc != VPD_OK)
    {
      if (strstr (vpd_file, "share") != NIL (char))
           return (TRUE);       /* share vpd was empty */

      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                 inu_cmdname);
      exit (1);
    }

  if ((fp = fopen (vpdadd_file, "w")) == (FILE *)0)  /* open the output file */
    {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, vpdadd_file);
       return (rc);
    }

  while (rc == VPD_OK)
   {
     print_history_stanza (hist, fp);
     vpd_free_vchars (HISTORY_TABLE, &hist);
     rc = vpdgetnxt (HISTORY_TABLE, &hist);
     if (rc == VPD_SYS  ||  rc == VPD_BADCNV)
       {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_VPDERR_E, CMN_VPDERR_D), 
                                   inu_cmdname);
        fclose (fp);
        exit (FALSE);
       }
   }

  fclose (fp);
  return (rc=TRUE);
}

/* ------------------------------------------------------------------------
*
* NAME: print_product_stanza
*
* FUNCTION: Perform filtering on the 3.2 product record to 
*           generate a 4.x product record and write it to
*           a file. 
*
*
* NOTES:
*    ABnormal exit on malloc errors.
*
* RETURNS: (void)
*
* OUTPUT:
*        Write the a 4.x product record to the file pointed 
*        by fp.
* ------------------------------------------------------------------------*/

void
print_product_stanza (prod_t prod,  /* 3.2.x product struct */
                      FILE  *fp)    /* File pointer for 4.x .add file */
{

   char *cptr,      /* a character pointer */
        *tempstr1,  /* to hold a string temporarily */
        *c, 
        *str;


   /*---------------------------------------------------------------------*
   ** If a record is AVAILABLE, and sceded_by is null, don't bother
   ** write it to the .add file
   **---------------------------------------------------------------------*/
   if (prod.state == ST_AVAILABLE  &&  (prod.sceded_by[0] == '\0'))
      return;

   if (fprintf (fp, "\n\
product:\n\
\t\tlpp_name=\"%s\"\n\
\t\tcomp_id=\"%s\"\n\
\t\tcp_flag=%d\n\
\t\tupdate=%d\n\
\t\tfesn=\"%s\"\n\
\t\tname=\"%s\"\n\
\t\tstate=%d\n\
\t\tver=%d\n\
\t\trel=%d\n\
\t\tmod=%d\n\
\t\tfix=%d\n\
\t\tptf=\"%s\"\n\
\t\tmedia=%d\n\
\t\tsceded_by=\"%s\"\n", prod.lpp_name, prod.comp_id, 
			(prod.cp_flag & LPP_INSTAL) ?
			  (prod.cp_flag | LPP_INSTALLED_ON_32) : prod.cp_flag, 
			 (prod.cp_flag & LPP_UPDATE) ? 1 : 0,
			 prod.fesn, prod.name, prod.state, prod.ver, prod.rel, 
			 prod.mod, prod.fix, prod.ptf, prod.media, 
			 prod.sceded_by) < 0)
				       no_space ();

   /*---------------------------------------------------------------------*
   ** Filtering for fixinfo field:
   **   3.2.x  fixinfo field may contain up to 2 actual fields
   **       |description|VPD_STRING_DELIMITER|fixinfo|
   ** 
   ** In 4.x since they are separate fields, we split the field in the
   ** following block of code:
   **---------------------------------------------------------------------*/
   if ((cptr = strstr (prod.fixinfo, VPD_STRING_DELIMITER)) != NIL (char))
      {         /* piggy back exists */
           /* malloc string to hold description */
        if ((tempstr1 = (char *) malloc ((size_t) (cptr-prod.fixinfo)+1))
           == NIL (char))
          {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                          CMN_MALLOC_FAILED_D), inu_cmdname);
            exit (1);
          }
        /* copy description */
        strncpy (tempstr1, prod.fixinfo, (size_t) (cptr - prod.fixinfo));
        tempstr1 [ (cptr - prod.fixinfo)] = '\0';
      }


   if (fprintf (fp, "\t\tfixinfo=\"") < 0)
       no_space ();

   free (prod.fixinfo);

   if (fprintf (fp, "\"\n\t\tdescription=\"") < 0)
       no_space ();

   str = (cptr == NIL (char)) ? "" : replace_quotes (tempstr1, QUOTE, fp);

   if (fprintf (fp, "\"\n") < 0)
       no_space ();

   free (str);
   free (tempstr1);

   cptr = NIL (char); /* Reinitialize */
 /*---------------------------------------------------------------------*
   ** Filtering for prereq field for MC type ptfs:
   **   3.2.x  prereq field may contain upto 2 actual fields
   **       |prereq|VPD_STRING_DELIMITER|supersedes|
   ** 
   ** In 4.x since they are separate fields, we split the field in the
   ** following block of code:
   **---------------------------------------------------------------------*/
  
   if (IF_LPP_PKG_PTF_TYPE_C (prod.cp_flag)  &&     /* if cume type */
      ((cptr = strstr (prod.prereq, VPD_STRING_DELIMITER)) != NIL (char)))
      {
           /* piggy back exists for this MC ptf */

        /* malloc string to hold prereq */
        if ((tempstr1 = (char *) malloc ((size_t) (cptr-prod.prereq)+1))
             == NIL (char))
          {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                          CMN_MALLOC_FAILED_D), inu_cmdname);
            exit (1);
          }
        strncpy (tempstr1, prod.prereq, (int) (cptr - prod.prereq)); /* prereq info */
        tempstr1 [ (cptr - prod.prereq) ] = '\0';

    }

   if (fprintf (fp, "\t\tprereq=\"%s\"\n", (cptr == NIL (char)) ? prod.prereq :
                                                                tempstr1) < 0)
       no_space ();

   if (fprintf (fp, "\t\tsupersedes=\"%s\"\n", (cptr == NIL (char)) ? "" :
                                                cptr+1) < 0)
       no_space ();

   free (tempstr1);

}


/* ------------------------------------------------------------------------
*
* NAME: print_lpp_stanza
*
* FUNCTION: Perform filtering on the 3.2 lpp record to
*           generate a 4.x product record and write it to
*           a file.
*
* NOTES:
*    Abnormal exit on malloc errors.
*
* RETURNS: (void)
*
* OUTPUT:
*        Write the a 4.x product record to the file pointed
*        by fp.
* ------------------------------------------------------------------------*/

void
print_lpp_stanza (lpp_t   lpp,    /* 3.2.x product struct */
                  FILE   *fp,     /* File pointer for 4.x .add file */
                  boolean doing_root_vpd)
{
   char * str;

   /*------------------------------------------------------------------*
   ** If a record is AVAILABLE don't bother to write it to the .add file 
   ** unless the record is a special available root record added to the
   ** vpd when a usr-root update is installed on a usr-only base.
   **------------------------------------------------------------------*/
   if ((lpp.state == ST_AVAILABLE) 
                    && 
       ((! doing_root_vpd) ||
        (! is_usr_root_update_lpp_entry (lpp))))
      return ;

   /*------------------------------------------------------------------*
   ** Don't write an extra control record.
   **------------------------------------------------------------------*/
   if ( ! strcmp (lpp.name, "__SWVPD_CTL__"))
      return ;

   if (fprintf (fp, "\n\
lpp:\n\
\t\tname=\"%s\"\n\
\t\tsize=%ld\n\
\t\tstate=%d\n\
\t\tcp_flag=%d\n\
\t\tgroup=\"%s\"\n\
\t\tmagic_letter=\"%s\"\n\
\t\tver=%d\n\
\t\trel=%d\n\
\t\tmod=%d\n\
\t\tfix=%d\n", lpp.name, lpp.size, lpp.state, lpp.cp_flag, lpp.group, 
		lpp.magic_letter, lpp.ver, lpp.rel, lpp.mod, lpp.fix) < 0)
       no_space ();

   /* ptf field has been blasted from lpp struct for 4.1 */ 

   if (fprintf (fp, "\t\tdescription=\"") < 0)
       no_space ();

   str = replace_quotes (lpp.description, QUOTE, fp);

   free (str);

   if (fprintf (fp, "\"\n\t\tlpp_id=%d\n",  lpp.lpp_id + target_lpp_count) <0)
       no_space ();

}

/* ------------------------------------------------------------------------
*
* NAME: print_inventory_stanza
*
* FUNCTION: Perform filtering on the 3.2 inventory record to
*           generate a 4.x product record and write it to
*           a file.
*
* NOTES:
*
* RETURNS: (void)
*
* OUTPUT:
*        Write the a 4.x product record to the file pointed
*        by fp.
* ------------------------------------------------------------------------*/

void
print_inventory_stanza (inv_t inv,    /* 3.2.x product struct */
                        FILE *fp)     /* File pointer for 4.x .add file */
{

   if (fprintf (fp, "\n\
inventory:\n\
\t\tlpp_id=%d\n\
\t\tprivate=%d\n\
\t\tfile_type=%d\n\
\t\tformat=%d\n\
\t\tloc0=\"%s\"\n\
\t\tloc1=\"%s\"\n\
\t\tloc2=\"%s\"\n\
\t\tsize=%ld\n\
\t\tchecksum=%ld\n", inv.lpp_id + target_lpp_count, inv.private, 
		     inv.file_type, inv.format, inv.loc0, inv.loc1, inv.loc2, 
		     inv.size, inv.checksum) <0)   
       no_space ();

}

/* ------------------------------------------------------------------------
*
* NAME: print_history_stanza
*
* FUNCTION: Perform filtering on the 3.2 history record to
*           generate a 4.x product record and write it to
*           a file.
*
* NOTES:
*
* RETURNS: (void)
*
* OUTPUT:
*        Write the a 4.x product record to the file pointed
*        by fp.
* ------------------------------------------------------------------------*/

void
print_history_stanza (hist_t hist,  /* 3.2.x product struct */
                      FILE *fp)     /* File pointer for 4.x .add file */

{
   char *str;

   if (fprintf (fp, "\n\
history:\n\
\t\tlpp_id=%d\n\
\t\tevent=%d\n\
\t\tver=%d\n\
\t\trel=%d\n\
\t\tmod=%d\n\
\t\tfix=%d\n\
\t\tptf=\"%s\"\n\
\t\tcorr_svn=\"%s\"\n\
\t\tcp_mod=\"%s\"\n\
\t\tcp_fix=\"%s\"\n\
\t\tlogin_name=\"%s\"\n\
\t\tstate=%d\n\
\t\ttime=%ld\n", hist.lpp_id + target_lpp_count, hist.event, hist.ver, 
		   hist.rel, hist.mod, hist.fix, hist.ptf, hist.corr_svn, 
		   hist.cp_mod, hist.cp_fix, hist.login_name, hist.state, 
							 hist.time) <0)
       no_space ();

   if (fprintf (fp, "\t\tcomment=\"") < 0)
       no_space ();

   str = replace_quotes (hist.comment, QUOTE, fp);

   if (fprintf (fp, "\"\n") < 0)
       no_space ();

   free (str);

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
*                   fs   -- output parameter, will contain
*                           name of filesystem.
*      global:      none
*
* RETURNS: (long)
*          number of free 512-blocks in fs 
*          -1 on error
*
* OUTPUT:  
*
* ------------------------------------------------------------------------*/

long
get_free_blocks_in_filesystem (char *path)      /* vpd fs */
{
   struct statfs sfs;  /* for statfs call */

   if (statfs (path, &sfs) == -1)
     {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, path);
       return (-1);
     }

   /* Rounding up  num_of_free_blocks */
   return ((long) ((((long long) sfs.f_bfree  * (long long) sfs.f_bsize) + 
                    (long long) 511) / (long long) 512)) ;
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
 
   if (system (cmd) != 0)
     {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_EXPAND_E, 
                CMN_CANT_EXPAND_D), inu_cmdname, fs, num_of_512_blocks);

       return (FALSE);
     }

   return (TRUE);
}

/* ------------------------------------------------------------------------
*
* NAME: enough_space_for_vpd
* 
* FUNCTION: 
*     Checks to see if ODMDIR has enough space. If not, 
*     and expansion is requested, the function will expand the file 
*     system
*
* DATA STRUCTURES:
*     parameters:  path -- path to vpd
*                  auto_expand -- Always set to true
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
enough_space_for_vpd   (char *path,        /* path vpd that needs to be saved */
                        int auto_expand)   /* to expand or not? */
{
   long num_of_512_blocks_for_vpd = 0,     /* Original size of the vpd */ 
        num_of_free_512_blocks_in_fs = 0,   
        num_of_512_blocks_needed = 0;      /* to expand */

   char *vpdfs; 

   struct stat statbuf;      /* for statx */
   int command = STX_NORMAL;


   if (statx (path, &statbuf, 0, command) != PASS)
     {
       if (strstr (path, "share") == NIL (char))
           return (TRUE);
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                CMN_CANT_OPEN_D), inu_cmdname, path);
       return (FALSE);
     }

   if ( ! S_ISREG (statbuf.st_mode)) /* is a regular file */
      {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                CMN_CANT_OPEN_D), inu_cmdname, path);
        return (FALSE);
      }

   /* get fs name for chfs */
   if ((vpdfs=get_fsname (path)) == NIL (char)) {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, path);
        return (FALSE);
   }

   if ((num_of_free_512_blocks_in_fs = get_free_blocks_in_filesystem (path)) 
                                      == -1)
     {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, path);
       return (FALSE);
     }

   if ((num_of_512_blocks_for_vpd = get_size_of_file (statbuf.st_size)) 
                                   == -1)
      {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, path);
        return (FALSE);
      }

   /* increment 4.1 fs requirement */
   num_of_512_blocks_for_41_vpd += num_of_512_blocks_for_vpd;

   if ((num_of_512_blocks_needed = (num_of_512_blocks_for_vpd - 
                                  num_of_free_512_blocks_in_fs)) > (long) 0)
     {
       if (auto_expand)
         {
          if ( ! expand_my_lv (vpdfs, num_of_512_blocks_needed))
              return (FALSE);
         }
       else
         {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_EXPAND_E, 
                               CMN_CANT_EXPAND_D), inu_cmdname, vpdfs, 
                               num_of_512_blocks_needed);
           return (FALSE);
         }
     }
  return (TRUE);
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
  return ((long) ((filesize + 511) /512));
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
   if ((argc != 3)  &&  (argc != 4))
     {
       inu_msg (FAIL_MSG, MSGSTR (MS_CONVERT_VPD, CONVERT_MIGRATE_USAGE_E, 
                        CONVERT_MIGRATE_USAGE_D), argv[0]);
       exit (2);
     }

   vpd_path_32=argv[1];
   vpd_path_41=argv[2];
   if (argc == 4) {
       target_lpp_count=atoi (argv[3]);
   }
   
}

/* ------------------------------------------------------------------------
*
* NAME: enough_space_for_41_vpd
*
* FUNCTION:
*     Checks to see if 4_1 ODMDIR has enough space. If not, 
*     and expansion is requested, the function will expand the file
*     system
*
* DATA STRUCTURES:
*     parameters:  path -- path to 4.1 vpd
*                  auto_expand -- Always set to true
*                  num_of_512_blocks_for_41_vpd
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
enough_space_for_41_vpd (char *path,         /* 4.1 vpd path */
                          boolean auto_expand, 
                          long num_of_512_blocks_for_41_vpd)  /* how many */
{

  long num_of_512_blocks_for_vpd = 0,     /* Original size of the vpd */
       num_of_free_512_blocks_in_fs = 0, 
       num_of_512_blocks_needed = 0;      /* to expand */

  char *vpdfs; 


   /* get fs name for chfs */
   if ((vpdfs=get_fsname (path)) == NIL (char)) {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, path);
        return (FALSE);
   }

   if ((num_of_free_512_blocks_in_fs =get_free_blocks_in_filesystem (path))
                                                                        == -1) 
   {
       inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                  CMN_CANT_OPEN_D), inu_cmdname, path);
       return (FALSE);
   }

   if ((num_of_512_blocks_needed = (num_of_512_blocks_for_41_vpd -
                                  num_of_free_512_blocks_in_fs)) > (long)0)
   {
       if (auto_expand) 
         {
           if ( ! expand_my_lv (vpdfs, num_of_512_blocks_needed))
              return (FALSE);
         }
       else
         {
           inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_EXPAND_E, 
                                      CMN_CANT_EXPAND_D), inu_cmdname, vpdfs, 
                                      num_of_512_blocks_needed);
           return (FALSE);
         }
   }

}

/* ------------------------------------------------------------------------
*
* NAME: is_usr_root_update_lpp_entry
*
* FUNCTION:
*     Checks to see if the AVAILABLE lpp entry passed as an argument has
*     any corresponding non-AVAILABLE entries in the product database.
*     Uses name and v.r.m.f as search criteria.
*
* DATA STRUCTURES:
*     parameters:  lpp - structure containing AVAILABLE info for this vpd rec.
*
* RETURNS: (boolean)
*     TRUE if non-AVAILABLE record exists for this lpp in product database
*     FALSE otherwise
*
* ------------------------------------------------------------------------*/
boolean
is_usr_root_update_lpp_entry (lpp_t lpp)
{

   prod_t prod;
   int rc;
   int i;

   memset (&prod, NULL, sizeof (prod_t));  /* initialize prod struct */

   strcpy (prod.lpp_name, lpp.name);
   prod.ver = lpp.ver;
   prod.rel = lpp.rel;
   prod.mod = lpp.mod;
   prod.fix = lpp.fix;
   for (i=ST_APPLYING; i <= ST_DEINSTALLING; i++)
   {
       prod.state = i;
       rc = vpdget (PRODUCT_TABLE, 
                    (PROD_LPP_NAME | PROD_VER | PROD_REL | PROD_MOD | 
                     PROD_FIX | PROD_STATE),
                    &prod); 
       if (rc == VPD_OK)
       {
          vpd_free_vchars (PRODUCT_TABLE, &prod);
          return (TRUE);
       }
   }
   return (FALSE);
}

void
no_space ()
{
  inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_SPACE_E, CMN_NO_SPACE_D), 
                             inu_cmdname);
fflush (stdout);
  exit (2);
}

unsigned int
hashname ( char *name )
{
   char *c = name;
   unsigned int hash = 0;

   if (name == NULL) {
       return 0;
   }
   while (*c != '\0') {
       hash += (unsigned int) *c++;
   }
   hash = hash % HASHTABLESIZE;

   return hash;
}

int
create_exceptions_table ( FILE *exceptions_file )
{
   char name [PATH_MAX+1];
   struct filehash *newentry;
   unsigned int hashval;
   char *newline;
   char *slash;

   if (exceptions_file == (FILE *) NULL) {
       return -1;
   }
   while (fgets (name, PATH_MAX+1, exceptions_file) != NULL) {
       newentry = (struct filehash *) malloc (sizeof (struct filehash));
       if (newentry != (struct filehash *) NULL) {
	  if ((newline = strrchr(name,'\n')) != NULL) { /* Kill the \n */
	      *newline = '\0';
	  }
	  /* Kill a trailing "/" - directory entry designation */
	  /* Check for one-character entry ('/'), then check for 
	   * last character == "/"
	   */
	  if ((name[1] != '\0') &&
	      ((slash = strrchr(name,'/')) != NULL) && (*(slash+1) == '\0')) {
	       *slash = '\0';
	  }
	  newentry->name = strdup(name);
	  newentry->next = exceptions[hashval = hashname(name)].next;
	  exceptions[hashval].next = newentry;
       } else {
	  return -1;
       }
   }
   return 0;
}

boolean
omit_from_vpd ( char *name )
{
   struct filehash *namesearch;
   unsigned int hashval;
   
   for (namesearch = exceptions[hashval = hashname(name)].next;
        namesearch != (struct filehash *) NULL;
	namesearch = namesearch->next) {
           if (!strcmp(name,namesearch->name)) {
		   return TRUE;
	   }
   }
   return FALSE; /* No match */
}
