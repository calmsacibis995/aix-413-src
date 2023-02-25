static char sccsid[] = "@(#)23  1.42  src/bos/usr/sbin/install/installp/inu_toc_vpd.c, cmdinstl, bos41J, 9510A_all 2/27/95 12:14:44";

/*----------------------------------------------------------------------------
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: free_bff_list
 *              free_toc_list 
 *              free_option_list
 *              free_optref
 *              inu_get_lpp_desc
 *              set_cp_flag
 *              set_media_field
 *              update_lpp_table
 *              create_product_record
 *              create_base_level_prod_rec
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *--------------------------------------------------------------------------*/

#include <installp.h>

#define LPP_LEN         144
#define TOC_HD_NULL     1
#define VPD_ACC_E       2

static void   free_bff_optref     (BffRef_t * bffptr);
static void   free_optref         (OptionRef_t * ptr);
static void   inu_get_lpp_desc    (lpp_t *, char);
static void   get_usr_entry       (Option_t *op, lpp_t *usr_lpp_rec);

extern int odmerrno;

/*==========================================================================*
** 
**  Function:     update_lpp_table ()
**
**  Description:  Queries the LPP table to see if there's a record matching
**                op's name, ver, rel and  mod.  If one already exists it
**                will be updated, otherwise a new one will be created.
**
**  Note 1:       This routine is only called for base levels AND for 
**                ROOT updates.  In the case of a ROOT update to a USR only
**                base level, there may not be an ROOT LPP record yet.
**
**  Note 2:       The case where a record already exists will happen when
**                re-applying a product option after a deinstall. 
**
**  Returns:      None.
** 
**==========================================================================*/


void update_lpp_table (Option_t *op, int new_state)
{
   int       vpdrc;        /* vpd return code */
   lpp_t     lpp_rec;      /* LPP record */
   lpp_t     usr_lpp_rec;  /* corresponding USR part LPP record */
   int       oem_specific = 0; /* OEM-specific fileset */

   (void) inu_set_vpdtree (op->vpd_tree);

   memset (&lpp_rec, NULL, sizeof(lpp_t));
   memset (&usr_lpp_rec, NULL, sizeof(lpp_t));

   /*--------------------------------------------------------------*
    * Check to see if this is an OEM-specific fileset
    *--------------------------------------------------------------*/
    if ((op->lang[0] == 'o') && (!strncmp(op->lang,"oem_",4))) {
        oem_specific = 1;
    }

  /*--------------------------*
   * Establish search criteria
   *--------------------------*/
   strcpy (lpp_rec.name, op->name);
   lpp_rec.ver  =  op->level.ver;
   lpp_rec.rel  =  op->level.rel;
   lpp_rec.mod  =  op->level.mod;

  /*--------------------*
   * Query the LPP table
   *--------------------*/
   vpdrc = vpdget (LPP_TABLE, LPP_NAME | LPP_VER | LPP_REL | LPP_MOD, &lpp_rec);
   inu_ck_vpd_sys_rc (vpdrc);

   lpp_rec.state  =  new_state;

  /*---------------------------------------------------------------*
   * Change the record that's already there if this is not the ROOT
   * update special case.
   *---------------------------------------------------------------*/
   if (vpdrc == VPD_OK)
   {
     /*---------------------------------------------------------------*
      * If OEM-specific, set the appropriate cp_flag bit.
      * If not and it USED to be, then unset the OEM_SPECIFIC bit.
      *---------------------------------------------------------------*/
      if (oem_specific) {
	      lpp_rec.cp_flag |= LPP_OEM_SPECIFIC;
      } else if (lpp_rec.cp_flag & LPP_OEM_SPECIFIC) {
	      lpp_rec.cp_flag &= ~LPP_OEM_SPECIFIC;
      }

      vpdrc = vpdchgget (LPP_TABLE, &lpp_rec);
      inu_ck_vpd_ok (vpdrc,
		"\tvpdchgget LPP_TABLE.name=%s vpderrno=%d odmerrno=%d\n",
		lpp_rec.name, vpdrc, odmerrno);
   }
   else if (vpdrc == VPD_NOMATCH)
   {
     /*----------------------------------------------*
      * Create an lpp record to add to the LPP table
      *----------------------------------------------*/
      lpp_rec.size            =  0;
      lpp_rec.group           =  strdup (NullString);
      lpp_rec.magic_letter[0] =  'I';
      lpp_rec.magic_letter[1] =  '\0';

      lpp_rec.ver             =  op->level.ver;
      lpp_rec.rel             =  op->level.rel;
      lpp_rec.mod             =  op->level.mod;
      lpp_rec.fix             =  op->level.fix;

     /*-----------------------------------------------------------*
      * If we're acting on a 4.1-or-later root part, get the level
      * of the record we're about to add from the usr lpp record.
      *-----------------------------------------------------------*/
      if (IF_UPDATE (op->op_type)  && 
          ! IF_3_X (op->op_type)   && 
          op->vpd_tree == VPDTREE_ROOT)
      {
         get_usr_entry (op, &usr_lpp_rec);
         lpp_rec.ver             =  usr_lpp_rec.ver;
         lpp_rec.rel             =  usr_lpp_rec.rel;
         lpp_rec.mod             =  usr_lpp_rec.mod;
         lpp_rec.fix             =  usr_lpp_rec.fix;

         vpd_free_vchars (LPP_TABLE, &usr_lpp_rec);
      }

      lpp_rec.lpp_id          =  0;
      lpp_rec.description     =  strdup (op->desc);

      if (op->vpd_tree == VPDTREE_ROOT)
         inu_get_lpp_desc (&lpp_rec, op->vpd_tree);

     /*--------------------------------------------------------------*
      * Set the cp_flag, but make sure the instal bit is on and the
      * update bit is off.  Also make sure update specific bits are
      * turned off.
      *--------------------------------------------------------------*/
      lpp_rec.cp_flag   =  (long) set_cp_flag (op);
      lpp_rec.cp_flag  |=  LPP_INSTAL;
      lpp_rec.cp_flag  &= ~LPP_UPDATE;
      lpp_rec.cp_flag  &= ~LPP_PKG_PTF_TYPE;
      lpp_rec.cp_flag  &= ~LPP_BOSBOOT;
      if (oem_specific) {
          lpp_rec.cp_flag  |= LPP_OEM_SPECIFIC;
      }


     /*--------------------------------*
      * Add the record to the LPP table 
      *--------------------------------*/
      vpdrc = vpdadd (LPP_TABLE, &lpp_rec);
      inu_ck_vpd_ok (vpdrc,
		"\tvpdadd LPP_TABLE.name=%s vpderrno=%d odmerrno=%d\n",
		lpp_rec.name, vpdrc, odmerrno);
   }

   inu_ck_vpd_ok (vpdrc,
	"\tvpdget LPP_TABLE.name=%s vpderrno=%d odmerrno=%d\n",
	lpp_rec.name, vpdrc, odmerrno);

   vpd_free_vchars (LPP_TABLE, &lpp_rec);

} /* update_lpp_table */


/*==========================================================================*
** 
**  Function:     create_product_record
**
**  Description:  Ensures that a product record exists for the option 
**                specified by ptr -- which can be either a base install
**                or an update.
**
**  Returns:      None.
** 
**==========================================================================*/

void create_product_record 
   (Option_t * op,         /* ptr to the option to create the record for   */
    int        new_state)  /* which state to set the VPD record to         */
{
   prod_t    prod_rec;
   int       vpdrc;
   char      productname [MAX_LPP_FULLNAME_LEN];

   (void) inu_set_vpdtree (op->vpd_tree);

   memset (&prod_rec, NULL, sizeof(prod_t));

  /*--------------------------*
   * Establish search criteria
   *--------------------------*/
   strcpy (prod_rec.lpp_name, op->name);
   inu_get_prodname (op, productname);
   prod_rec.name        =  strdup (productname);
   prod_rec.ver         =  op->level.ver;
   prod_rec.rel         =  op->level.rel;
   prod_rec.mod         =  op->level.mod;
   prod_rec.fix         =  op->level.fix;
   prod_rec.description =  strdup (op->desc);
   strcpy (prod_rec.ptf, op->level.ptf);

   if (IF_C_UPDT (op->op_type))
      prod_rec.supersedes  =  strdup (op->supersedes);
   else
      prod_rec.supersedes  =  "";

  /*-------------------------*
   * Query the PRODUCT table
   *-------------------------*/
   vpdrc = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_VER | PROD_REL |
                                  PROD_MOD | PROD_FIX | PROD_PTF, &prod_rec);
   inu_ck_vpd_sys_rc (vpdrc);

  /*------------------*
   * Change the state
   *------------------*/
   prod_rec.state = new_state;

   if (vpdrc == VPD_NOMATCH)
   {
     /*----------------------------------------------------*
      * Create a product record to add to the PRODUCT table
      *----------------------------------------------------*/
      if (op->vpd_tree == VPDTREE_ROOT  &&  op->prereq == NIL(char))
         get_prereq_file (op, &prod_rec);
      else
         prod_rec.prereq     =  strdup (op->prereq);

      prod_rec.cp_flag       =  (long) set_cp_flag (op);

     /*--------------------------------------------------------------*
      * If 4.1 or later base level, add supersedes info ("" if none).
      *--------------------------------------------------------------*/
      if (IF_INSTALL (op->op_type)  &&  ! IF_3_X (op->op_type)) 
         prod_rec.supersedes = strdup (op->supersedes);

      prod_rec.sceded_by[0]  =  '\0';
      (void) get_productid_file (op, &prod_rec);

      if (op->vpd_tree != VPDTREE_ROOT)
         get_fixinfo_file (op, &prod_rec);
      else
         prod_rec.fixinfo    =  strdup (NullString);

     /*----------------------------------------------------------------*
      *  Just set the media field to be "on file".  lslpp is the only
      *  thing that uses this field, and then its only for AVAILABLE's.
      *----------------------------------------------------------------*/
      prod_rec.media        =  PROD_MED_FILE;

     /** -------------------------------------------------------------- *
      ** Set the update field (INU_TRUE if update, INU_FALSE if base).
      ** -------------------------------------------------------------- */

      if (IF_UPDATE (op->op_type))
         prod_rec.update = INU_TRUE;
      else
         prod_rec.update = INU_FALSE;

     /*------------------------------------*
      *  Add prod_rec to the PRODUCT table
      *------------------------------------*/
      vpdrc = vpdadd (PRODUCT_TABLE, &prod_rec);
      inu_ck_vpd_ok (vpdrc,
		"\tvpdadd PRODUCT_TABLE.lpp_name=%s vpderrno=%d odmerrno=%d\n",
		prod_rec.lpp_name, vpdrc, odmerrno);
   }

  /*----------------------------------------------*
   * We've found a match; a record already exists
   *----------------------------------------------*/
   else if (vpdrc == VPD_OK)
   {
      /*------------------------------------------------------------------*
       *  For AVAILABLE records (which are not dummy supersedes records):
       *     If the prereq info from the toc is non-empty               AND
       *        is different from that in the vpd
       *     update the requisite info in the vpd
       *
       *     Do the same for the supersedes info of  C_TYPE updates.
       *------------------------------------------------------------------*/
      if ((prod_rec.state == ST_AVAILABLE) &&
          (prod_rec.sceded_by[0] == '\0'))
      {
         if ((op->prereq != NULL)                        &&
             (strcmp (prod_rec.prereq, op->prereq) != 0))
         {
            if (prod_rec.prereq != NIL (char))
               free (prod_rec.prereq);

            prod_rec.prereq = strdup (op->prereq);
         }

         if (IF_C_UPDT (op->op_type)                    &&
             op->supersedes != NULL                     &&
             (strcmp (prod_rec.supersedes, op->supersedes) != 0))
         {
            if (prod_rec.supersedes != NIL (char))
               free (prod_rec.supersedes);

            prod_rec.supersedes = strdup (op->supersedes);
         }
      }

     /*---------------------------------------------------------*
      * Change the existing product record in the PRODUCT table.
      *---------------------------------------------------------*/
      vpdrc = vpdchgget (PRODUCT_TABLE, &prod_rec);
      inu_ck_vpd_ok (vpdrc,
		"\tvpdchgget PRODUCT_TABLE.lpp_name=%s vpderrno=%d odmerrno=%d\n",
		prod_rec.lpp_name, vpdrc, odmerrno);
   }

   inu_ck_vpd_ok (vpdrc,
	"\tvpdget PRODUCT_TABLE.lpp_name=%s vpderrno=%d odmerrno=%d\n",
	prod_rec.lpp_name, vpdrc, odmerrno);

   vpd_free_vchars (PRODUCT_TABLE, &prod_rec);

} /* create_product_record */


/*==========================================================================*
** 
**  Function:     create_root_base_level_prod_rec
**
**  Description:  Given a product option record, op, which represents a ROOT
**                PART update, make sure a ROOT part base level PRODUCT record
**                exists.  If not then create one with the proper attributes.
**                If one already exists then make sure it has the proper 
**                attributes by doing a vpdchgget.  The cp_flag should indicate
**                that there is also a USR part to this product option.
**
**  Notes:        It is assumed upon entry that we are already dealing with
**                the ROOT vpd; hence, no call to inu_set_vpdtree.
**
**  Returns:      TRUE  - if a PRODUCT record was created
**                FALSE - if a record was not created (but may have been 
**                        modified)
** 
**==========================================================================*/

boolean create_root_base_level_prod_rec (Option_t * op)
{
   prod_t    prod_rec;
   lpp_t     usr_lpp_rec;
   char      productname [MAX_LPP_FULLNAME_LEN];
   int       vpdrc;
   boolean   CREATED_OR_CHANGED = FALSE;  /* indicated whether the PRODUCT
                                             record was created or changed */

   memset (&prod_rec, NULL, sizeof(prod_t));
   memset (&usr_lpp_rec, NULL, sizeof(lpp_t));

  /*-------------------------------------------------------------------*
   * Set the fields for searching for a ROOT base level PRODUCT record 
   *-------------------------------------------------------------------*/
   strcpy (prod_rec.lpp_name, op->name);
   prod_rec.ver    =  op->level.ver;
   prod_rec.rel    =  op->level.rel;
   prod_rec.mod    =  op->level.mod;
   prod_rec.fix    =  op->level.fix;
   prod_rec.update =  INU_FALSE;

  /*-----------------------------------------------------------*
   * See if there's already a ROOT base level PRODUCT record
   *-----------------------------------------------------------*/
   vpdrc = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_VER | PROD_REL |
                                  PROD_MOD | PROD_FIX | PROD_UPDATE, &prod_rec);

   if (vpdrc == VPD_NOMATCH)
   {
     /*---------------------------------------------------------------*
      * Create a PRODUCT record based on the information in the op 
      * structure.  If we found a record, we will replace it; if not 
      * we will add this one.
      *---------------------------------------------------------------*/

     /*-----------------------------------------------------------*
      * Copy cp_flag and change it from LPP_UPDATE to LPP_INSTAL
      *-----------------------------------------------------------*/
      prod_rec.cp_flag     =  (long) set_cp_flag (op);
      prod_rec.cp_flag    ^=  LPP_UPDATE;   /* turn off UPDATE bit */
      prod_rec.cp_flag    |=  LPP_INSTAL;   /* turn on INSTALL bit */

      inu_get_prodname (op, productname);
      prod_rec.name        =  strdup (productname);

     /*--------------------------------------------------------------*
      *  3.1, and 3.2 pkgs, use level of option we're dealing with,
      *  4.1 or later pkgs, get level from usr VPD lpp record.
      *--------------------------------------------------------------*/

      prod_rec.ver         =  op->level.ver;
      prod_rec.rel         =  op->level.rel;
      prod_rec.mod         =  op->level.mod;
      prod_rec.fix         =  op->level.fix;

     /*-----------------------------------------------------------*
      * If we're acting on a 4.1-or-later root part, get the level
      * of the record we're about to add from the usr lpp record.
      *-----------------------------------------------------------*/
      if (IF_UPDATE (op->op_type)  &&
          ! IF_3_X (op->op_type)   &&
          op->vpd_tree == VPDTREE_ROOT)
      {
         get_usr_entry (op, &usr_lpp_rec);
         prod_rec.ver      =  usr_lpp_rec.ver;
         prod_rec.rel      =  usr_lpp_rec.rel;
         prod_rec.mod      =  usr_lpp_rec.mod;
         prod_rec.fix      =  usr_lpp_rec.fix;

         vpd_free_vchars (LPP_TABLE, &usr_lpp_rec);
      }

      prod_rec.state       =  ST_AVAILABLE;
      prod_rec.media       =  PROD_MED_FILE;
      prod_rec.fixinfo     =  strdup (NullString);
      prod_rec.prereq      =  strdup (NullString);
      prod_rec.description =  strdup (op->desc);
      prod_rec.supersedes  =  strdup (NullString);
      prod_rec.fesn[0]     =  '\0';
      prod_rec.comp_id[0]  =  '\0';
      prod_rec.ptf[0]      =  '\0';
      prod_rec.sceded_by[0]=  '\0';
      prod_rec.update      =  INU_FALSE;

     /*------------------------------------*
      * Add the record to the PRODUCT table
      *------------------------------------*/
      vpdrc = vpdadd (PRODUCT_TABLE, &prod_rec);
      inu_ck_vpd_ok (vpdrc,
		"\tvpdadd root PRODUCT_TABLE.lpp_name=%s vpderrno=%d odmerrno=%d\n",
		prod_rec.lpp_name, vpdrc, odmerrno);

      CREATED_OR_CHANGED = TRUE;
   }

   inu_ck_vpd_ok (vpdrc,
	"\tvpdget root PRODUCT_TABLE.lpp_name=%s vpderrno=%d odmerrno=%d\n",
	prod_rec.lpp_name, vpdrc, odmerrno);

   vpd_free_vchars (PRODUCT_TABLE, &prod_rec);

   return (CREATED_OR_CHANGED);

} /* create_root_base_level_prod_rec */


/*==========================================================================*
**
**  Function:     set_media_field
**
**  Description:  Sets the media field in the prod_rec passed in.
**
**  Returns:      None
**
**==========================================================================*/

void set_media_field (prod_t * prod_rec, TOC_t  * head)
{
   switch (head->type)
   {
      case TYPE_DISK:
         prod_rec->media = PROD_MED_FILE;
         break;
      case TYPE_TAPE_SKD:
      case TYPE_TAPE_BFF:
         prod_rec->media = PROD_MED_TAPE;
         break;
      case TYPE_FLOP_SKD:
      case TYPE_FLOP_BFF:
         prod_rec->media = PROD_MED_DISK;
         break;
      default:
         prod_rec->media = PROD_MED_FILE;
         break;
   }

} /* set_media_field */


/*==========================================================================*
**
**  Function:     set_cp_flag
**
**  Description:  Calculate the value the cp_flag field should have based on
**                the values in the option ptr passed in. 
**
**  Returns:      cp_flag
**
**==========================================================================*/

long set_cp_flag (Option_t * optptr)
{
   long     rt_val = 0;

  /*------------------------------------------- *
   * Determine which parts we have ...
   *------------------------------------------- */

   if ( (optptr->content == CONTENT_MRI)
                      ||
        (optptr->content == CONTENT_OBJECT)
                      ||
        (optptr->content == CONTENT_MCODE)
                      ||
        (optptr->content == CONTENT_PUBS)
                      ||
        (optptr->content == CONTENT_USR) )
   {
      rt_val |= LPP_USER;
   } 

   if (optptr->content == CONTENT_BOTH)
   {
      rt_val |= LPP_USER;
      rt_val |= LPP_ROOT;
   }

   if (optptr->content == CONTENT_SHARE)
      rt_val |= LPP_SHARE;

  /*-------------------------------------------- *
   * Determine if base install or update ... 
   *-------------------------------------------- */

   if (IF_INSTALL (optptr->op_type))
      rt_val |= LPP_INSTAL;

   if (IF_UPDATE (optptr->op_type))
      rt_val |= LPP_UPDATE;

  /*-------------------------------------------- *
   * Determine what package version we have ... 
   *-------------------------------------------- */

   if (IF_3_1 (optptr->op_type))
      rt_val |= LPP_31_FMT;
   else if (IF_3_2 (optptr->op_type))
      rt_val |= LPP_32_FMT;
   else if (IF_4_1 (optptr->op_type))
      rt_val |= LPP_41_FMT;

  /*-------------------------------------------- *
   * Determine what type of update we have ... 
   *-------------------------------------------- */

   if (optptr->bff != NIL (BffRef_t))
    {
      switch (optptr->bff->action)
      {
         case (ACT_CUM_UPDT):
            rt_val |= (LPP_PKG_PTF_TYPE  &  LPP_PKG_PTF_TYPE_C);
            break;

         case (ACT_EN_PKG_UPDT):
            rt_val |= (LPP_PKG_PTF_TYPE  &  LPP_PKG_PTF_TYPE_E);
            break;

         case (ACT_MAINT_LEV_UPDT):
            rt_val |= (LPP_PKG_PTF_TYPE  &  LPP_PKG_PTF_TYPE_ML);
            break;

         case (ACT_MULT_UPDT):
            rt_val |= (LPP_PKG_PTF_TYPE  &  LPP_PKG_PTF_TYPE_M);
            break;
      }
    }
   else
    {  /* if bff there is no media, get it from op_type */ 

       if (IF_C_UPDT (optptr->op_type))
           rt_val |= (LPP_PKG_PTF_TYPE  &  LPP_PKG_PTF_TYPE_C);

       else if (IF_E_UPDT (optptr->op_type))
           rt_val |= (LPP_PKG_PTF_TYPE  &  LPP_PKG_PTF_TYPE_E);

       else if (IF_ML_UPDT (optptr->op_type))
           rt_val |= (LPP_PKG_PTF_TYPE  &  LPP_PKG_PTF_TYPE_ML);

       else if (IF_M_UPDT (optptr->op_type))
           rt_val |= (LPP_PKG_PTF_TYPE  &  LPP_PKG_PTF_TYPE_M);
     }

   /*---------------------------------------------------------------------- *
    * AIX 3.2.4 Overloads the "quiesce" field with the values
    * ('B' Quiesce and perform bosboot; 'b' No quiesce but perform bosboot).
    * I perform the mapping between the character to a bit representation
    * here and in inu_toc.c set_op_type().
    *---------------------------------------------------------------------- */

    if (QUIESCE_TO_BOSBOOT (optptr->quiesce))
        rt_val |= LPP_BOSBOOT;

   return ((long) rt_val);

} /* set_cp_flag */


/*==========================================================================*
**
**  Function:     free_option_list
**
**  Description:  The parameter is the first node of options link list. This
**                function will free the size, prereq, supersedes, PrereqPtr 
**                and then free the node itself. It will go through whole list 
**                to free whole options nodes.
**
**  Returns:      None
**
**==========================================================================*/

int free_option_list (Option_t * optptr)
{
   Option_t       *tmpptr;

   for (tmpptr = optptr;
        tmpptr != NIL (Option_t);
        tmpptr = optptr)
   {
      optptr = tmpptr->next;
      inu_free_op (tmpptr);
   }

} /* free_option_list */


/*==========================================================================*
**
**  Function:       free_bff_list
**
**  Description:    The parameter to this function is the beginning node of 
**                  bff link list. This function just free up whole nodes in
**                  the link list.
**
**  Returns:        None
**
**==========================================================================*/

static void free_bff_optref (BffRef_t * bffptr)
{
   BffRef_t       *tmpptr;

   for (tmpptr = bffptr;
        tmpptr != NIL (BffRef_t);
        tmpptr = tmpptr->next)
   {
      if (tmpptr->options != NIL (OptionRef_t) )
         free_optref (tmpptr->options);
      tmpptr->options = NIL (OptionRef_t);
   }

} /* free_bff_optref */


/*==========================================================================*
**
**  Function:       free_optref
**
**  Description:    Frees a list of OptionRef_t's.
**
**  Returns:        None
**
**==========================================================================*/

static void free_optref (OptionRef_t * ptr)
{
   OptionRef_t    *tmp;

   for (tmp = ptr;
        tmp != NIL (OptionRef_t);
        tmp = ptr)
   {
      ptr = tmp->next;
      free (tmp);
   }

} /* free_optref */


/*==========================================================================*
**
**  Function:       inu_get_lpp_desc
**
**  Description:    If the given description is empty, attempt to get one 
**                  stored in the vpd record.
**
**  Returns:        None
**
**==========================================================================*/

static void inu_get_lpp_desc (lpp_t * lpp, char vpd_tree)
{
   char      * desc;
   lpp_t       lpp_tmp;

   /* do we have a description? */

   if ((lpp->description != NULL)  &&  (*(lpp->description) != NUL))
      return;

   inu_set_vpdtree (VPDTREE_USR);

   /* does the puppy have a description in the user part? */

   bzero (&lpp_tmp, sizeof (lpp_tmp));
   strcpy (lpp_tmp.name, lpp->name);
   if (vpdget (LPP_TABLE, LPP_NAME, &lpp_tmp) == VPD_NOMATCH)
   {
      (void) inu_set_vpdtree (vpd_tree);
      return;
   }

   desc = strdup (lpp_tmp.description);

   vpd_free_vchars (LPP_TABLE, &lpp_tmp);

   (void) inu_set_vpdtree (vpd_tree);

} /* inu_get_lpp_desc */


/*==========================================================================*
**
**  Function:       free_toc_list 
**
**  Description:    Frees up the 2 lists that hang off the toc anchor. 
**
**  Parameters:     tp  -  Ptr to the toc linked list
**
**  Returns:        None
**
**==========================================================================*/

void free_toc_list (TOC_t * tp)
{
   if (tp == NIL(TOC_t))
      return;

   /* clean up and free memory */
   /* start with cleaning up options list */

   free_option_list (tp->options);
   tp->options = (Option_t *) NULL;

   /* Then free BffRet_t list */

   free_bff_optref (tp->content);
   tp->content = (BffRef_t *) NULL;

} /* free_toc_list */

/*==========================================================================*
**
**  Function:       get_usr_entry
**
**  Description:    Gets ths usr lpp VPD record. 
**
**  Returns:        None
**
**                  Call inu_quit() if cannot get record.  It should be
**                  there.
**
**==========================================================================*/

static void get_usr_entry (Option_t *op, lpp_t *usr_lpp_rec)
{
   int rc;

   strcpy (usr_lpp_rec->name, op->name);

   if (op->vpd_tree != VPDTREE_USR)
      (void) inu_set_vpdtree (VPDTREE_USR);

   rc = vpdget (LPP_TABLE, LPP_NAME, usr_lpp_rec);

   inu_ck_vpd_ok (rc,
	"\tvpdget get_usr_entry LPP_TABLE.lpp_name=%s vpderrno=%d odmerrno=%d\n",
	op->name, rc, odmerrno);

   if (op->vpd_tree != VPDTREE_USR)
      (void) inu_set_vpdtree (op->vpd_tree);
}
