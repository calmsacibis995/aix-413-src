#if !defined(lint)
static char sccsid[] = "@(#)80	1.6  src/tenplus/helpers/history/u9.c, tenplus, tenplus411, GOLD410 3/23/93 11:58:34";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	H_u9, H_ubefore
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
/* file:  u9.c                                                              */
/*                                                                          */
/* u9 (menu) function for the history helper                                */
/****************************************************************************/

#include <stdio.h>
#include <database.h>
#include "keynames.h"
#include <keys.h>
#include <edexterns.h>
#include "history.h"
#include "INhistory_msg.h"
#include <libhelp.h>

extern int Hg_numzooms;         /* number of levels of zooming */

/****************************************************************************/
/* H_u9: puts up a menu of commands                                         */
/****************************************************************************/

void H_u9 (void)
    {
    register retval;                /* menu return value       */
    static POINTER *items = NULL;  /* used to display menu    */

    if (!Hg_numzooms)         /* we're at the top level */
	{
	Eerror (HNOMENU, "There is no LOCAL-MENU for this data");
	return;
	}

    retval = Emenu (MN_HMENU, 
"\t       History Display\n\
\n\
Select a menu option by moving the cursor to\n\
an item and pressing EXECUTE.  Press CANCEL\n\
to remove the menu, or press HELP to display\n\
help information.\n\n\
@(1) Show Time of this Version of the File\n\
@(2) Show Next Time\n\
@(3) Show Previous Time\n\
@(4) Redisplay History\n\
@(5) Save Current Version of File#0", 0, NULL);

    if (retval == ERROR)
	return;

    switch (retval)
	{
	case 0:
	    H_u1 ();
	    break;

	case 1:
	    H_u2 ();
	    break;

	case 2:
	    H_u3 ();
	    break;

	case 3:
	    H_u4 ();
	    break;

	case 4:
	    H_u5 (0, "");
	    break;
	}
    }

/****************************************************************************/
/* H_ubefore: distribute button pushes to the rest of the helper            */
/****************************************************************************/

int H_ubefore (register int key, int flag, char *string)
    {
    if ( (flag == LINEREGION) || (flag == BOXREGION) || (flag == TEXTREGION) )
	{
	Eerror (HREGION, "This helper does not understand regions");
	return (1);
	}
    switch (key)
	{
    case U1:
	H_u1 ();
	break;

    case U2:
	H_u2 ();
	break;

    case U3:
	H_u3 ();
	break;

    case U4:
	H_u4 ();
	break;

    case U5:
	H_u5 (flag, string);
	break;

    case LOCAL_MENU:
	H_u9 ();
	break;

    case EXIT:
	H_exit ();
	return (0);

    case ZOOM_IN:
	return (H_zoomin ());

    case ZOOM_OUT:
	return (H_zoomout ());
	}
    return (1);
    }


