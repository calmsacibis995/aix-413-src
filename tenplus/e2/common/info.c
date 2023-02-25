#if !defined(lint)
static char sccsid[] = "@(#)51	1.6  src/tenplus/e2/common/info.c, tenplus, tenplus411, GOLD410 3/23/93 11:51:51";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	help, helpbox,
 *		helpmenu, 
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* file:  info.c                                                            */
/*                                                                          */
/* routines to handle the HELP command                                      */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"

extern nl_catd g_e2catd;
extern nl_catd g_helpcatd;

/****************************************************************************/
/* help:  hander for the HELP command                                       */
/*                                                                          */
/* arguments:              int   type   - how called: NOARGS or ...         */
/*                         char *string - STRINGARG value                   */
/*                         int   number - NUMBERARG value                   */
/*                                                                          */
/* return value:           VOID                                             */
/*                                                                          */
/* globals referenced:     g_helpfiles                                      */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     help is the handler for the HELP command.  It finds the appropriate  */
/*     menu header and item list and then invokes taskmenu to handle the    */
/*     menu processing.  In the multinational version of the editor,        */
/*     taskmenu is called with a null header list; otherwise help builds a  */
/*     constant header list which it passes to taskmenu.                    */
/****************************************************************************/

void help (int type)
    /*type;      how called      */
    /*string;    STRINGARG value */
    /*number;    NUMBERARG value */
    {
    extern char g_helpstate;

#ifdef DEBUG
    debug ("help called");
#endif

    switch (type)
	{
	case LINEREGION:
	case BOXREGION:
	    helpbox (H_EBOXREGION, TRUE);
	    break;

	case TEXTREGION:
	    helpbox (H_ETEXTREGION, TRUE);
	    break;

	default:
	    /* make sure user has specified items in the help menu */

	    if ((g_helpfiles == NULL) || (obj_count (g_helpfiles) == 0))
		{
		Eerror (M_EHELPFILES,
		    "There are no HELP options specified in your editor profile.");
		return;
		}
	    /* help state changes from NOTHELPING to INHELPMENU,        */
	    /*                 or from INHELPFILE to IN2NDHMENU         */

	    g_helpstate ++;

	    taskmenu (MN_EHELPMENU, "               Help Menu\n\n\
Select a menu option (for example, move the\n\
cursor to an item and press EXECUTE) or\n\
press CANCEL to remove the menu, or\n\
press HELP to display help information.\n#0", g_helpfiles);
	    break;
	}
    }

/****************************************************************************/
/* helpbox:  puts up the help code information in a box                     */
/*                                                                          */
/* arguments:              int helpcode - for which to see help information */
/*                         int helpflag - >0 if help is available           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     helpbox is used to put up a popbox containing whatever help is       */
/*     available for the given helpcode.  It calls Egetmessage to get the   */
/*     help message, and pulls out its message subfield.  It then invokes   */
/*     newbox to put up the message and calls onechar to wait for the       */
/*     user to cancel out of the popbox.                                    */
/****************************************************************************/

void helpbox (int helpcode, int helpflag)
{
    register char *message;    /* message field from msgcat    */
    register POINTER *pointer; /* used for char array messages */

#ifdef DEBUG
    debug ("helpbox:  helpcode = %d", helpcode);
#endif


    if (helpflag  <= 0)
    {
	Eerror(M_ENOHELP,"There is no help available for current item.");
	return;
    }

    /* Set TRUE flag for both Egetmessage calls so g_help_count isn't altered */
    message = Egetmessage(helpcode,Egetmessage(M_ENOHELP, "There is no help available for current item.", TRUE),TRUE);

    pointer = s2pointer (message); /* Convert to pointer array */

    newbox (pointer);
    onechar(FALSE);

    s_freenode ((char *)pointer);
   
}

/****************************************************************************/
/* helpmenu:  puts up the help box for a menu entry                         */
/*                                                                          */
/* arguments:              int menu_no - defining which menu                */
/*                         int menu_entry - item to see help for            */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_help_count                                     */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     helpmenu is used to get help for a particular menu_entry             */ 
/*     (rather than for the menu as a whole).                               */  
/*     It uses Egetmessage to get the menu's help                           */
/*     message, and then calls s_findnode to find the help record for the   */
/*     given item, if it exists. It then uses newbox and onechar to put up  */
/*     the help message. If no help record exists for the given item it     */
/*     uses the help for the menu as a whole by calling Egetmessage with    */
/*     given menu_no+1. If there is no help here, then display the message  */
/*     associated with msg_no M_ENOHELP.                                    */
/****************************************************************************/

void helpmenu (int menu_no, int menu_entry)
    {
    register POINTER *message;
    int msg_no;
    char *help_msg;

#ifdef DEBUG
    debug ("helpmenu:  menu_no = %d, menu_entry = %d", menu_no, menu_entry);
#endif

    if (g_help_count <= 0)
    {
        Eerror(M_ENOHELP,"There is no help available for current item.");  
	return;
    }

    if (g_help_count == 1)
        msg_no = menu_no + 1;

    if (g_help_count > 1) {
	if (menu_entry + 1 > g_help_count)
        {
       	     Eerror(M_ENOHELP,"There is no help available for current item.");  
	     return;
        }
	msg_no = menu_no + menu_entry + 1; 
    }

    /* Set TRUE flag for both Egetmessage calls so g_help_count isn't altered */
    help_msg = Egetmessage(msg_no,
	       Egetmessage(M_ENOHELP,"There is no help available for current item.", TRUE),TRUE);

    message = s2pointer(help_msg);

    newbox (message);

    onechar (FALSE);

    }
