static char sccsid[] = "@(#)71  1.17  src/bos/usr/sbin/install/installp/inu_grep_option_toc.c, cmdinstl, bos41B, 412_41B_sync 1/6/95 15:20:10";
/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS: grep_option_toc, 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <inudef.h>
#include <inu_toc.h>
#include <toc0.h>
#include <instlmsg.h>
#include <installpmsg.h>
#include <inulib.h>

extern TOC_t *TocPtr;    /* The pointer to the Media toc */
extern Option_t  * toc_hashlist[TOC_HASH_SIZE];

extern int get_toc_hash_value (char *);

Level_t * get_max_level     (char *opname, Level_t *level);
int       does_level_match  (Level_t *level, Option_t *tm);

/*--------------------------------------------------------------------------*
** NAME: grep_option_toc
**
** FUNCTION:Starts at the top of the toc option list and trys to find a
**          match based on the compiled reg exp string. If one is found
**          then it checks to make sure it isn't already in the Selected
**          Options list and that we haven't searched on this one before.
**  L     - Pointer to top of the option list.
**  name_len   - length of name string.
**  name  - String containing a name to use in locating option.
**  level - Points to the level that is requested by the user.
**          (If level->ver -1, then find the LPP with the latest level.
**  found - Flag to indicate if option was found.
**
** RETURNS:
**  Pointer to located option entry
**     or,
**  NIL if no matching entry found.
**
**--------------------------------------------------------------------------*/

Option_t *grep_option_toc
   (Option_t *L,        /* Ptr to top of the toc option list.                 */
    char     *name,     /* Ptr to the name to search for.                     */
    int       name_len, /* length of name                                     */
    Level_t  *level,    /* Points to the level that is requested by the user. */
    int      *found,    /* Flag to indicate if an option was found or not.    */
    char     *svname)   /* name of lpp entered by the user on the cmd line    */

{
    Option_t    *p;
    Option_t    *first_u_p;   /* points to the option struct with the */
                              /* first valid update with this name    */
    Option_t    *latest_i_p; /* points to the option struct with the  */
                             /* latest level for the lpp option       */
    char        latest_i_name[MAX_LPP_FULLNAME_LEN]; /* name of latest option */
    char        buf[MAX_LPP_FULLNAME_LEN];           /* temp buffer */
    char        *s;
    int         rc;
    short       fix_id_save;
    register int  comp;
    char        levname [MAX_LPP_FULLNAME_LEN];
    int		exact_match_bos_rte = 0;

    first_u_p        = NIL(Option_t);
    latest_i_p       = NIL(Option_t);
    latest_i_name[0] = '\0';

    exact_match_bos_rte = (!strcmp(svname, "bos.rte"));

    for (p=L->next; 
         p!=NIL(Option_t);  
         p=p->next) 
    {

       /*------------------------------------------------------------------*
        * If the this option isn't a backup-format update or install image 
        * continue...NOTE: tar formatted bos.obj images would be an example.
        * Since we don't deal with anything other than bff images, ignore it.
        *------------------------------------------------------------------*/
	if (p->bff->action == ACT_UNKNOWN  ||  p->bff->action == ACT_OTHER) 
	    continue;

       /** ------------------------------------------- *
        **  The 2ndary call ==> use svname
        ** ------------------------------------------- */
        
	if(name == NIL(char))
           comp = strcmp(svname, p->name);

       /** ------------------------------------------- *
        **  "all" or "ALL" given ==> force a match
        ** ------------------------------------------- */

        else if(name_len == 0)
           comp = 0;

       /** ---------------------------------------------------------- *
        **  Check n chars worth, where n = strlen (name to match)
        ** ---------------------------------------------------------- */

        else if (exact_match_bos_rte || ((comp=strncmp(name, p->name, name_len))!=0))
           comp = strcmp(svname, p->name);

        if (comp == 0) {

           /*--------------------------------------------------------------*
            * If this is an installation and no level was specified on
            * the command line, then we want to return the latest option
            * that was on the media.
            * We want to return the installations first then the updates:
            * so if we find a valid update, save it (first_u_p) and continue;
            * When we are out of this loop, then if no installations have
            * been found then return the saved update (first_u_p)...
            *--------------------------------------------------------------*/
            if (level->ver == -1) { /* -1 means user didn't specify level */
	        *found = 1;
		/* have we already picked this one? */
                if (p->op_checked)
                    continue;
                if (IF_INSTALL(p->op_type)) {
                    if (latest_i_p == NIL(Option_t)) {
                        latest_i_p = p;
			strcpy(latest_i_name, p->name);
		    }
                    else
                       /*--------------------------------------------------*
                        * If the name of this option is = to name of the latest
			* found so far (latest_i_name) and this option's level
			* is > (or =) to the latest_i_p option then make this 
			* option the latest.
                        * (inu_level_compare() returns a 1  if (arg1 > arg2))
                        *--------------------------------------------------*/
			if ((strcmp(latest_i_name, p->name) == 0) &&
                         (inu_level_compare (&(p->level), &(latest_i_p->level) )
			  >=   0)) {
                            latest_i_p = p;
                        }
                }
                else {
                   /*-----------------------------------------------------*
                    * else it's an update, if we haven't found any
                    * installs then save this update pointer.
                    *-----------------------------------------------------*/
                    if (first_u_p == NIL(Option_t))
                        first_u_p = p;
                }
            }
            else 
            {
              /** ---------------------------------------------------------*
               **  Level criteria was given.  So, see if the toc entry's 
               **  level matches the criteria.
               ** ---------------------------------------------------------*/

               if ((strcmp (level->ptf, "all") == 0) ||
                   (strcmp (level->ptf, "ALL") == 0))
               {
                  rc = does_level_match (level, p);
               }
               else
                  rc = inu_level_compare (&(p->level), level);

               if (rc == LEVEL_MATCH)    
               {
                  *found = 1;

                  /* have we already picked this one? */

                  if (p->op_checked)
                     continue;
                  p->op_checked = 1;
                  return (p);
               }
            } /* else */
        } /* if */
    } /* for */

    if (latest_i_p != NIL(Option_t)) {
        latest_i_p->op_checked = 1;
        return (latest_i_p);
    }
    else
        if (first_u_p != NIL(Option_t)) {
            first_u_p->op_checked = 1;
            return (first_u_p);
	}
        else
            return NIL(Option_t);
}

/* ------------------------------------------------------------------------- *
**
**   Function  does_level_match
**
**   Purpose   Determines if the toc member's level matches the level
**             criteria given from the command line (v.r.m.f.all).
**
**             The algorithm here is to match toc entries w/ level:
**               <  max level (v.r.m.f), where max level is the lowest
**                  v.r.m.f for a base level greater than the v.r.m.f 
**                  specified, AND
**               >= given level (v.r.m.f).
**             Example:
**             A.obj 1.1.1.0.all is specified
**               If A.obj is a 3.2 product, our expression should match with 
**               A.obj 1.1.1.0, A.obj 1.1.1.0.U1, A.obj 1.1.1.0.U2, etc.
**               If there is another version of A.obj on the media, say
**               A.obj 1.1.1.3, then we'll match with A.obj 1.1.1.1.U3,
**               and A.obj 1.1.2.U4 if they are on the media but not with
**               A.obj 1.1.1.4 or any of its updates.  If A.obj 1.1.1.3
**               base level is not on the media, then we'll match with 
**               A.obj 1.1.1.4.U5, A.obj 1.1.1.4.U6, etc.  (ie. we don't
**               make any assumptions about the base level for A.obj 1.1.1.4.UX)
** 
**               If A.obj is a 4.1 product, our expression should match with 
**               A.obj 1.1.1.0, A.obj 1.1.1.1, A.obj 1.1.1.2, etc up to, but
**               not including, the next base level for this product whether 
**               or not it has the same version, release and mod.  It will
**               not match with any updates for the "next base level", as
**               in the example above with a 3.2 product.
**
**   Returns   LEVEL_MATCH    -  toc entry's level matches criteria
**             GREATER_LEVEL  -  toc entry's level does NOT match criteria
**             LESSER_LEVEL   -  toc entry's level does NOT match criteria
** 
** ------------------------------------------------------------------------- */

int does_level_match 
   (Level_t *level,             /*  level criteria from command line */
    Option_t *tm)               /*  toc member to check              */
{
   int rc;
   Level_t * max_level;

  /** ------------------------------------------------------------------- *
   **  Compare v.r.m.f's (don't wanna compare the "all" in ptf field).
   **  Levels < level criteria do not match.
   ** ------------------------------------------------------------------- */

   level->ptf[0] = '\0';
   rc = inu_level_compare (&(tm->level), level);

   if (rc != LESSER_LEVEL)
   {
      /** ------------------------------------------------------------------- *
       **  Get the max level which corresponds to the 1st base level for this 
       **  product on the toc which is greater than the level specified with 
       **  the .all extension.  We match everything  >= the level specified, 
       **  but only < the max level.  If there was no max level or this toc 
       **  member falls in our range, we have a match.
       ** ------------------------------------------------------------------- */

      max_level = get_max_level (tm->name, level);

      if (max_level != NIL(Level_t))
      {
         rc = inu_level_compare (&(tm->level),max_level);
         if (rc == GREATER_LEVEL || rc == LEVEL_MATCH)
            rc = GREATER_LEVEL;
         else
            rc = LEVEL_MATCH;
      }
      else
      {
         rc = LEVEL_MATCH;
      }
   }

   strcpy (level->ptf, "all");
   return (rc);
}

/* ------------------------------------------------------------------------- *
**
**   Function     get_max_level
**
**   Purpose      Determines the base level with the lowest v.r.m.f level
**                on the media that's higher than the level passed in.
**                Such a level may not exist.
**
**   Returns      NULL       - if there is no such level
**                Level_t *  - if there is such a level
** 
** ------------------------------------------------------------------------- */

Level_t * get_max_level 
 (char     *opname,       /* Option name that corresponds to min_level    */
  Level_t  *min_level)    /* The min level that our max level must exceed */
{
  int hv;               /*  Hash value                                      */
  Option_t *op;         /*  Traversal option ptr                            */
  Level_t *max_level;   /*  Ptr to level we'll malloc and return on success */

  max_level = NIL(Level_t);
  
 /** ----------------------------------------------------------------------- *
  **  Hash to this option name's location, and search the bucket for the 
  **  install image with the lowest higher level than our min_level.
  ** ----------------------------------------------------------------------- */

  hv = get_toc_hash_value (opname);

  for (op=toc_hashlist[hv];
       op != NIL(Option_t);
       op=op->hash_next)
  {
     if (IF_INSTALL(op->op_type))    /* BASE LEVEL */
     {
        if ((inu_level_compare (&(op->level), min_level)) == GREATER_LEVEL) 
        {
           if (max_level == NIL(Level_t)  || 
               (inu_level_compare (&(op->level), max_level)) == LESSER_LEVEL) 
           {
              if (max_level==NIL(Level_t) &&
                 (max_level=(Level_t *)malloc(sizeof(Level_t))) ==NIL(Level_t))
              {
                 instl_msg(FAIL_MSG, MSGSTR(MS_INUCOMMON, CMN_MALLOC_FAILED_E,
                                            CMN_MALLOC_FAILED_D), inu_cmdname);
                 inu_quit (INUMALLOC);
              }

             /** ----------------------------------------------------------- *
              **  This option meets the criteria, so call it the max level.
              ** ----------------------------------------------------------- */

              max_level->ver = op->level.ver;
              max_level->rel = op->level.rel;
              max_level->mod = op->level.mod;
              max_level->fix = op->level.fix;
              max_level->ptf[0] = '\0';
           } 
        }
     }
  }

  return max_level;
}
