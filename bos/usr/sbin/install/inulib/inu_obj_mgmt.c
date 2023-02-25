static char     sccsid[] = "@(#)69  1.26  src/bos/usr/sbin/install/inulib/inu_obj_mgmt.c, cmdinstl, bos411, 9428A410j 3/6/94 19:28:06";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:  voloffcmp, insert_bffref, insert_optionref, insert_option, 
 *             create_toc, create_bffref, copy_option, create_option, 
 *             create_optionref, insert_bffref, insertSop_toc, copy_option
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <inulib.h>

static int voloffcmp (BffRef_t * a, 
                      BffRef_t * b);

/*--------------------------------------------------------------------------*
** FUNCTION: voloffcmp
**
** DESCRIPTION: Compares two BffRef_t instances.  The criteria sorts the
**              instances by volume and then by offset.
**
** RETURNS:
**          -1  a < b
**           0   a = b
**           1   a > b
**
**--------------------------------------------------------------------------*/

static int voloffcmp (BffRef_t * a, 
                      BffRef_t * b)
{
   if (a->vol < b->vol)
      return -1;

   if (a->vol == b->vol)
   {
      if (a->off < b->off)
         return -1;
      if (a->off > b->off)
         return 1;
      return 0;
   }

   return 1;
} /* end voloffcmp */

/*--------------------------------------------------------------------------*
**
** FUNCTION:    insert_bffref
**
** DESCRIPTION: Inserts an instance of BffRef_t into the specified list.
**
**--------------------------------------------------------------------------*/

void insert_bffref (BffRef_t * L,   /* Ptr to BffRef_t anchor struct of the toc
                                     * link list */
                    BffRef_t * ref) /* Ptr to the new BffRef_t struct to be
                                     * added to list */
{
   BffRef_t       *p, *q;

   for (p = L->next, q = L;
        p != NIL (BffRef_t)  &&  (voloffcmp (p, ref) < 0);
        q = p, p = p->next)
   {
     ;
   } /* end for */

   ref->next = p;
   q->next = ref;

} /* end insert_bffref */

/*--------------------------------------------------------------------------*
**
** FUNCTION:   insert_optionref
**
** DESCRIPTION:Inserts an instance of OptionRef_t into the specified list.
**
**--------------------------------------------------------------------------*/

void insert_optionref (OptionRef_t * L, 
                       OptionRef_t * ref)
{
   OptionRef_t    *p;

   for (p = L;
        p->next != NIL (OptionRef_t);
        p = p->next)
   {
     ;
   }

   p->next = ref;
   ref->next = NIL (OptionRef_t);

} /* end insert_optionref */

/*--------------------------------------------------------------------------*
**
** FUNCTION:    insert_option
**
** DESCRIPTION: Inserts an instance of Option_t into the specified list.
**
**--------------------------------------------------------------------------*/

void insert_option (Option_t * L, 
                    Option_t * option)
{
   Option_t       *p;

   /*-----------------------------------------------------------------------*
    * Locate the first option in the list whose name is >= option->name
    *-----------------------------------------------------------------------*/

   for (p = L;
        p->next != NIL (Option_t);
        p = p->next)
   {
     ;
   }

   p->next = option;
   option->next = NIL (Option_t);

} /* end insert_option */

/*--------------------------------------------------------------------------*
**
** FUNCTION:    create_toc
**
** DESCRIPTION: Creates an instance of a table of contents (TOC).
**
** RETURNS:     A pointer to the newly created TOC.
**              NIL if there was a memory allocation failure.
**
**--------------------------------------------------------------------------*/

TOC_t * create_toc (BffRef_t * bff_list, /* First bff structure for the bff
                                          * link list. */
                    Option_t * op_list)  /* First option structure for the
                                          * option link list. */
{
   TOC_t          *toc;

   if ((toc = MALLOC (TOC_t, 1)) == NIL (TOC_t))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                   CMN_MALLOC_FAILED_D), inu_cmdname);
      return NIL (TOC_t);
   }

   memset (toc, '\0', sizeof (TOC_t));
   toc->content = bff_list;
   toc->options = op_list;

   return (toc);

} /* end create_toc */

/*--------------------------------------------------------------------------*
**
** FUNCTION:    create_bffref
**
** DESCRIPTION: Creates an instance of a BFF reference (BFF_t).
**
** RETURNS:     A pointer to the newly created BFF ref.
**              NIL if there was a memory allocation failure.
**
**--------------------------------------------------------------------------*/

BffRef_t * create_bffref (int    vol, 
                          int    off, 
                          int    size, 
                          char   fmt, 
                          char   platform, 
                          char * media, 
                          int    crc, 
                          int    action)
{
   BffRef_t       *bffref;

   if ((bffref = MALLOC (BffRef_t, 1)) == NIL (BffRef_t))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                   CMN_MALLOC_FAILED_D), inu_cmdname);
      return NIL (BffRef_t);
   }

   bffref->vol = vol;
   bffref->off = off;
   bffref->size = size;
   bffref->fmt = fmt;
   bffref->platform = platform;
   bffref->path = strdup (media);
   if (bffref->path == NIL (char))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                   CMN_MALLOC_FAILED_D), inu_cmdname);
      return NIL (BffRef_t);
   }
   bffref->crc = crc;
   bffref->action = action;
   bffref->action_string = NIL (char);
   bffref->flags = 0;
   bffref->options = NIL (OptionRef_t);
   bffref->next = NIL (BffRef_t);

   return (bffref);

} /* end create_bffref */

/*--------------------------------------------------------------------------*
**
** FUNCTION:    create_option
**
** DESCRIPTION: Creates an instance of an LPP Option (Option_t).
**
** RETURNS:     A pointer to the newly created entry.
**              NIL if there was a memory allocation failure.
**
**--------------------------------------------------------------------------*/

Option_t * create_option (char     * name, 
                          int        op_checked, 
                          int        quiesce, 
                          char       content, 
                          char     * lang, 
                          Level_t  * level, 
                          char     * desc, 
                          BffRef_t * bff)
{
   Option_t       *option;

   if ((option = MALLOC (Option_t, 1)) == NIL (Option_t))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                   CMN_MALLOC_FAILED_D), inu_cmdname);
      return (NIL (Option_t));
   }

   memset (option, '\0', sizeof (Option_t));
   strncpy (option->name, name, MAX_LPP_FULLNAME_LEN);
   option->name[MAX_LPP_FULLNAME_LEN - 1] = '\0';
   option->op_checked = op_checked;
   option->quiesce = quiesce;
   option->content = content;
   option->op_type = 0;
   option->vpd_tree = VPDTREE_USR;    /* default to USR */
   option->operation = OP_APPLY;      /* default to apply */
   option->Status = STAT_SUCCESS;     /* default to SUCCESS */
   strncpy (option->lang, lang, MAX_LANG_LEN);
   option->lang[MAX_LANG_LEN] = '\0';
   if ((desc == NIL (char))
               || 
        (desc == NullString))
   {
      option->desc = NullString;  /* Use no memory */
   }
   else
   {
      option->desc = strdup (desc);
      if (option->desc == NIL (char))
      {
         inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                      CMN_MALLOC_FAILED_D), inu_cmdname);
         return (NIL (Option_t));
      }
   }
   if (level != NIL (Level_t))
   {
      option->level.sys_ver = level->sys_ver;
      option->level.sys_rel = level->sys_rel;
      option->level.sys_mod = level->sys_mod;
      option->level.sys_fix = level->sys_fix;
      option->level.ver = level->ver;
      option->level.rel = level->rel;
      option->level.mod = level->mod;
      option->level.fix = level->fix;
      strcpy (option->level.ptf, level->ptf);
   }
   option->flag = 0;  /* not a dupe of another option */
   option->bff = bff;
   option->netls = NIL (Netls_t);

   return (option);

} /* end create_option */

/*--------------------------------------------------------------------------*
**
** FUNCTION:    create_optionref
**
** DESCRIPTION: Creates an instance of an LPP Option reference (OptionRef_t).
**
** RETURNS:     A pointer to the newly created entry.
**              NIL if there was a memory allocation failure.
**
**--------------------------------------------------------------------------*/

OptionRef_t * create_optionref (Option_t * option)
{
   OptionRef_t    *opref;

   if ((opref = MALLOC (OptionRef_t, 1)) == NIL (OptionRef_t))
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                   CMN_MALLOC_FAILED_D), inu_cmdname);
      return NIL (OptionRef_t);
   }

   opref->option = option;
   opref->next = NIL (OptionRef_t);

   return (opref);

} /* end create_optionref */

/*--------------------------------------------------------------------------*
**
** FUNCTION:    InsertSop_toc
**
** DESCRIPTION: Inserts an instance of Option_t into the specified list. The
**              criteria used is the vol and offset contained in the bff field
**              of the specified option.
**
**--------------------------------------------------------------------------*/

Option_t * InsertSop_toc (Option_t * L,  /* Ptr to the top of the Selected
                                          * Option List */
                          Option_t * op) /* Ptr to option that needs to be
                                          * inserted in the Sop */
{
   Option_t       *p, *q;
   Option_t       *sop;
   int             vol_cmp;

   /*----------------------------------------------------------------------*
    * Make a copy of the Option_t structure...
    * Then scan the selected option list and insert this option
    * into the Sop based on the order on the media...
    *----------------------------------------------------------------------*/

   if ((sop = copy_option (op)) == NIL (Option_t))
      return (NIL (Option_t));
   for (p = L->next, q = L;
        p != NIL (Option_t);
        q = p, p = p->next)
   {
      if ((vol_cmp = voloffcmp (p->bff, sop->bff)) > 0)
         break;
   }

   sop->next = p;
   q->next = sop;
   return (sop);

} /* end Insertsop_toc */

/*--------------------------------------------------------------------------*
**
** FUNCTION:    copy_option
**
** DESCRIPTION: Copies an instance of an Option_t.
**
** RETURNS:     A pointer to the newly created entry.
**              NIL if there was a memory allocation failure.
**
**--------------------------------------------------------------------------*/

Option_t * copy_option (Option_t * op)  /* Ptr to option that needs to be
                                         * copied. */
{
   Option_t       *nu_op;

   /*----------------------------------------------------------------------*
    * Create another instance of the Option_t structure...
    * Set/Initialize members that we're set by create_option ()...
    *----------------------------------------------------------------------*/

   if ((nu_op = create_option (op->name, 
                               op->op_checked, 
                               op->quiesce, 
                               op->content, 
                               op->lang, 
                               &op->level, 
                               op->desc, 
                               op->bff)) == NIL (Option_t))
      return (NIL (Option_t));

   nu_op->vpd_tree = op->vpd_tree;
   nu_op->operation = op->operation;
   nu_op->op_type = op->op_type;
   nu_op->size = op->size;
   nu_op->prereq = op->prereq;
   nu_op->supersedes = op->supersedes;
   nu_op->next = NIL (Option_t);
   nu_op->SelectedList = NIL (Option_t);
   strcpy (nu_op->prodname, op->prodname);
   nu_op->netls = op->netls;

   nu_op->flag = op->flag;

   return (nu_op);

} /* end copy_option */
