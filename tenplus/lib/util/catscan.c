#if !defined(lint)
static char sccsid[] = "@(#)49	1.2  src/tenplus/lib/util/catscan.c, tenplus, tenplus411, GOLD410 3/3/94 16:38:52";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:  catscan	
 * 
 * ORIGINS: 27  
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "database.h"
#include "libobj.h"
#include "libutil.h"

#define NOACTION 0
#define FLATTEN 1   /* Situation when a POINTER * needs replacing by a string */
#define RM_LEVEL 2  /* Situation when a level of indirection must be removed  */
#define RM_ELEMENT 3/* Situation where an element needs to be deleted         */
 
/****************************************************************************/
/* catscan:  Scan a message searching for nmemonics to translate            */
/*                                                                          */
/* arguments:    POINTER * obj_ptr   record from structured file            */
/*               nl_catd   catd      The message catalogue open for reading */
/*               int       set_no    The message catalogue set number       */
/*                                                                          */
/* return value:           POINTER *                                        */
/*                                                                          */
/* synopsis:                                                                */
/*           Catscan processes pointer arrays for the %+ mnemonic           */
/*           and extracts the message from the catalogue or uses            */
/*           the default message positioned recno+1 in the structured file. */
/*                                                                          */
/*           If a message requires underlining '\_' then the message        */
/*           needs to be formattted with underlining characters preceeding  */
/*           each character in the message.                                 */
/*                                                                          */
/*           catscan is a recursive procedure which moves down the          */
/*           levels in a structured alloaction searching for mnemonics.     */
/*           On finding a mnemonic,  the type of action must be             */
/*           passed back to the to the previous call to catscan.            */
/*                                                                          */
/*           This is done using the static variables action and msg_no.     */
/*           Because the use of these statics is confined to the previous   */
/*           level of recursion the contents are not over written.          */
/*                                                                          */
/*   Case FLATTEN:                                                          */
/*                                                                          */
/*     Type = pointer, Name = function:					    */
/*     		Type = character, Name = null:				    */
/*          		'%+MNEMONIC'					    */
/*     		Type = character, Name = null:				    */
/*          		'Menu functions (1) to (8).'			    */
/*									    */
/*     The above structure must be 'flattened'. 			    */ 
/*     The object function changed to be a character and contain the        */
/*     message from the catalogue or the default.			    */
/*								            */
/*     Result:          					            */
/*     Type = character, Name = function:				    */
/*          'Menu functions (1) to (8).'				    */
/*									    */
/*   Case RM_LEVEL:                                                         */
/*									    */
/*    This case is used for multiple line messages retreived from the       */
/*    catalogue. It must remove the default text and remove a level of      */
/*    indirection caused by expanding the mnemonic to a POINTER *           */ 
/*									    */
/*									    */
/*     Type = pointer, Name = description: 				    */
/*          Type = character, Name = (null):			            */
/*               '%+MNEMONIC'                               		    */
/*          Type = character, Name = (null):			            */
/*               'Functions (1) to (8) perform operations ...' (default)    */
/*          Type = character, Name = (null):				    */
/*               'or invoke options when any menu appears ...' (default)    */
/*    ....                                                                  */
/*    ....                                                                  */
/*                                                                          */
/*     Result:                                                              */
/*     Type = pointer, Name = description: 				    */
/*          Type = character, Name = (null):			            */
/*               'Functions (1) to (8) perform operations ...  		    */
/*          Type = character, Name = (null):				    */
/*               'or invoke options when any menu appears ...   	    */
/*    ....                                                                  */
/*    ....                                                                  */
/*                                                                          */
/*   Case RM_ELEMENT:                                                       */
/*                                                                          */
/*     In this case the message catalogue has failed to retrieve a          */
/*     multiple line message.  An element in the structure needs to         */
/*     be removed.							    */
/*                                                                          */
/*                                                                          */
/****************************************************************************/


POINTER *catscan (POINTER *obj_ptr,nl_catd catd,int set_no)
{
    int recno;        /* Record pointer                  */
    char *msg_ptr;
    char *message;    /* Message returned form catalogue */
    char *name;       /* Safe place for object name      */
    int flagbits;  /* Safe place for flagbits         */
    int number;
    char *new_message;
    char *underline_message;
    char *new_ptr;
    static int action; /* Has one of the following values NOACTION,
						          FLATTEN,
							  RM_LEVEL,
						      or  RM_ELEMENT.
             	       */
    static int msg_idx=0;
    POINTER *tmp_ptr;
    POINTER *newlist;
    POINTER *list_ptr;
    char *start;
    int length, i;

#ifdef DEBUG
    debug ("catscan: called");
#endif

    if (obj_ptr == NULL)
    {
        action = NOACTION;
        return (obj_ptr);
    }

    if (obj_type (obj_ptr) != T_POINTER)
    {
	return(obj_ptr);
    }

    action = NOACTION;


    for (recno = 0; recno < obj_count (obj_ptr); recno++)
    {
	switch (obj_type (obj_ptr [recno]))
	{
	  case T_POINTER :

	    obj_ptr[recno] = catscan (obj_ptr [recno],catd,set_no);
            if (obj_ptr[recno] == NULL)
            {
                break;
            }

            tmp_ptr = (POINTER *)obj_ptr[recno];
	    name = s_string (obj_name (obj_ptr [recno]));
	    flagbits = obj_flag (obj_ptr [recno]);

	    switch (action)
	    {
            case FLATTEN:

		    new_message = s_string(tmp_ptr[msg_idx]);
	            s_free(obj_ptr[recno]);

                    length = strlen(new_message);
                    /* Allow room for underlining */
                    underline_message = (char *)s_alloc(length*3,T_CHAR,NULL);
                    new_ptr = underline_message;
                    start = new_message;
                    while (*new_message)
                        {
                        if (*new_message == '\\' && *(new_message+1) == '_')
                            { /* Underlining to perform */

                            new_message += 2;
                            while (*new_message && !(*new_message == '\\' && *(new_message+1) == '+'))
                                {
                                *new_ptr++ = '_';
                                *new_ptr++ = '\b';
                                i = mblen(new_message, MB_CUR_MAX);
				/* copy at least 1 char */
				if (i < 1) i = 1;
                                while (i-- > 0)
                                    *new_ptr++ = *new_message++;
                                }
                            new_message += 2;
                            }
                        else {
			    i = mblen(new_message, MB_CUR_MAX);
			    /* copy at least 1 char */
			    if (i < 1) i = 1;
			    while (i-- > 0)
                                *new_ptr++ = *new_message++;
			    }
                        }

                    *new_ptr = '\0';
                    obj_ptr[recno] = underline_message;
                    s_free(start);

	            break;

            case RM_LEVEL:

		    tmp_ptr = (POINTER *)tmp_ptr[msg_idx];
		    newlist = treecopy(tmp_ptr);
		    s_free(obj_ptr[recno]);
		    obj_ptr[recno] = newlist;

		    break;

            case RM_ELEMENT:

		    length = obj_count(tmp_ptr);
		    newlist = s_alloc(length - 1, T_POINTER, NULL);
		    list_ptr = newlist;
		    for (i = 0; i < length; i++)
			if (i != msg_idx)
			    *list_ptr++ = s_string(tmp_ptr[i]);
		    s_free(obj_ptr[recno]);
		    obj_ptr[recno] = newlist;

		    break;
            } /* switch */

	    action = NOACTION;
	    msg_idx = -1;
            
            if (obj_ptr[recno] != NULL)
            {
	        s_newname (obj_ptr[recno], name);
	        s_free (name);
	        obj_setflag (obj_ptr[recno], flagbits);
            }
	    break;

	  case T_CHAR :

	    if ((msg_ptr = strstr(obj_ptr[recno],"%+")) != NULL)
	    {

	      while (*msg_ptr != '+')
	        msg_ptr++;

	      msg_ptr++; /* Increment message pointer to start of numeric */
             
	      if (isdigit(*msg_ptr))
	      {

	        number = atoi(msg_ptr);
		message = catgets(catd,set_no,number,NULL);
		msg_idx=recno;

		if (message == NULL)
                {
		   	if (obj_count(obj_ptr) > 2)
		   	    action = RM_ELEMENT ;
		   	else
			{
			    msg_idx = recno + 1;
		   	    action = FLATTEN;
			}
		        continue;
                }


		if (strstr(message,"\n") != NULL)
		{ /* Multiple lines */

		    newlist = s2pointer(message);
		    obj_ptr [recno] = newlist;
                    action = RM_LEVEL;
		}
		else
		{ /* Single line */
                    
		    s_free(obj_ptr[recno]);
		    obj_ptr[recno] = s_string(message);
                    action=FLATTEN;
		}
              }
	    }
	    break;

	  default :
	    break;
	}/* switch */
    }/* for */
    return(obj_ptr);
}
