static char sccsid[] = "@(#)93  1.17  src/bos/usr/sbin/install/inulib/inu_get_lppname.c, cmdinstl, bos411, 9428A410j 9/20/93 17:48:59";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_get_lppname, inu_get_prodname,
 *            inu_get_optlevname_from_Option_t (Installp)
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

/*
#include <swvpd.h>
#include <inudef.h>
#include <inu_toc.h>
*/

#include <inulib.h>

/*--------------------------------------------------------------------------*
**
** NAME: inu_get_lppname()
**
** FUNCTION: Using the op_name parameter, find the first period and move
**           up to the first period into the lppname parameter.
**
**--------------------------------------------------------------------------*/
void inu_get_lppname(
char     *op_name,	/* Option where the info resides to determine the libdir */
char 	 *lppname)	/* Top level lppname for this option */

{
    char *p; /* generic char pointer */
    char *q; /* generic char pointer */
   /*----------------------------------------------------------------------*
    * Get the top level name of the LPP to put in the lppname string
    *----------------------------------------------------------------------*/
    p = op_name; 
    q = lppname;
    while ((*p != ' ') && (*p != '.') && (*p != '\0')) {
        *q = *p;
        p++; 
        q++;
    }
    *q = '\0';
}


/**------------------------------------------------------------------------*
 **
 **  Function:   inu_get_prodname
 ** 
 **  Purpose:    Determines what the product name of op is and stores
 **              it in the prodname parameter.  Three sets of rules
 **              currently exist, namely 3.1, 3.2, and 4.1 rules. 
 ** 
 **  Returns:    None
 **
 **------------------------------------------------------------------------*/

void inu_get_prodname 
(
  Option_t * op,       /* Option used to determine what libdir is */
  char     * prodname  /* Top level lppname for this option       */
)

{
   switch (op->op_type & OP_TYPE_VERSION_MASK)   /* mask with (4 | 8 | 16) */
   {
      case OP_TYPE_4_1:
          if(*op->prodname == '\0')
          {
             inu_get_lppname (op->name, op->prodname);
          }
          strcpy (prodname, op->prodname);
          break;

      case OP_TYPE_3_2:
          inu_get_lppname (op->name, prodname);
          break;

      case OP_TYPE_3_1:
          if (*op->prodname == '\0')
             inu_get_lppname (op->name, op->prodname);
          strcpy (prodname, op->prodname);
          break;

      default:
          inu_get_lppname (op->name, prodname);
          break;
   }
}
/**--------------------------------------------------------------------------*
 **
 **  NAME               inu_get_name_and_level
 **
 **  FUNCTION           Creates a string of the form "lppname level" 
 **               -- "bosnet 3.2.0.0.U400100" for example -- or of the
 **               form "lpp optionname level" -- "bosnet.tcpip.obj 
 **               3.2.0.0.U400100" and returns it through the name_lev 
 **               paramter.
 **
 **  PARAMETERS         
 **                     op        -  a ptr to an option structure
 **                     name_lev  -  is filled in and returned
 **                     fullname  -  flag, 1 ==> lppname vs lpp optionname
 **
 **  RETURNS            none  (void function)
 **
 **--------------------------------------------------------------------------*/

void inu_get_name_and_level
 ( 
   Option_t *op,        /* option structure containing name & level info */
   char name_lev  [],   /* returned as "name level" including any ptf    */
   int      fullname    /* 0 ==> optionname,  1 ==> lppname is desired   */ 
 )

 {
    char lppname [PATH_MAX+1];   /* temp working lpp name */

   /**
    **  If we have nothing to work with, then do nothing.
    **/

    if (op == NIL (Option_t))
    {
	name_lev [0] = '\0'; /* make sure nuttin is in the name_lev returned */
        return;
    }

   /**
    **  If fullname is nonzero, then we use just the lppname ("bosnet")
    **  else we use the entire option name ("bosnet.tcpip.obj")
    **/

    if (fullname)
	inu_get_prodname (op, lppname);
    else
	strcpy (lppname, op->name);

   /** ---------------------------------------------------------- *
    **  Make sure the level field has a value in it.  If not,
    **  then just crate a string w/o a level.  -1 ==> no level.
    ** ---------------------------------------------------------- */

    if (op->level.ver != -1)
    {
       if (op->level.ptf [0] == '\0')
       {
           sprintf (name_lev, "%s %d.%d.%d.%d", lppname, 
                               op->level.ver, op->level.rel, 
                               op->level.mod, op->level.fix);
       }

       else
          /**
           ** We must be dealing with a ptf (update).
           **/
       {
           sprintf (name_lev, "%s %d.%d.%d.%d.%s", lppname, 
                               op->level.ver, op->level.rel, 
                               op->level.mod, op->level.fix,
                               op->level.ptf);
       }
    }

    else
    {
       strcpy (name_lev, lppname);
    }
 }
/**--------------------------------------------------------------------------*
 **
 **  NAME               inu_get_optlevname_from_Option_t     
 **
 **  FUNCTION           Creates a string of the form "lpp optionname level" 
 **               -- "bosnet.tcpip.obj 3.2.0.0.U400100" for example --
 **               and returns it through the name_lev paramter.
 **
 **  PARAMETERS         
 **                     op       -  a ptr to an option structure
 **                     name_lev -  is filled in and returned
 **
 **  RETURNS            none  (void function)
 **
 **--------------------------------------------------------------------------*/


void inu_get_optlevname_from_Option_t
 ( 
   Option_t *op,        /* option structure containing name & level info */
   char name_lev  []   /* returned as "name level" including any ptf    */
 )

 {
     inu_get_name_and_level (op, name_lev, 0);
 }
/**--------------------------------------------------------------------------*
 **
 **  NAME               inu_get_lpplevname_from_Option_t     
 **
 **  FUNCTION           Creates a string of the form "lppname level" 
 **               -- "bosnet 3.2.0.0.U400100" for example --
 **               and returns it through the name_lev paramter.
 **
 **  PARAMETERS         
 **                     op       -  a ptr to an option structure
 **                     name_lev -  is filled in and returned
 **
 **  RETURNS            none  (void function)
 **
 **--------------------------------------------------------------------------*/

void inu_get_lpplevname_from_Option_t
 ( 
   Option_t *op,        /* option structure containing name & level info */
   char name_lev  []   /* returned as "name level" including any ptf    */
 )

 {
     inu_get_name_and_level (op, name_lev, 1);
 }
/**--------------------------------------------------------------------------*
 **
 **  NAME               inu_prepend_string
 **
 **  FUNCTION           prepends a string with another string
 **
 **  PARAMETERS
 **
 **  RETURNS
 **
 **--------------------------------------------------------------------------*/

void inu_prepend_str
 (
    char *prestr,          /* string to prepend to root string */
    char *rootstr,         /* root string to prepend to        */
    char *deststr          /* deststr = prestr + rootstr       */
 )

{
    char tmpstr [PATH_MAX+1];

    strcpy (tmpstr, prestr);
    strcat (tmpstr, rootstr);
    strcpy (deststr, tmpstr);
}
