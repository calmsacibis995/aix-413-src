static char sccsid[] = "@(#)73	1.11  src/tenplus/e2/common/menus.c, tenplus, tenplus411, GOLD410 11/4/93 10:15:24";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	Emenu, Escrnio, fixcommand, makelist, task, taskmenu,
 *		usepop, usescreen , Ecustommenu
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/***************************************************************************/
/* file:  menus.c                                                          */
/*                                                                         */
/* command handlers that use the menu facility                             */
/***************************************************************************/
extern int g_highlight;

#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"

void usescreen (char *, char *);
/***************************************************************************/
/* Emenu:  puts up menu and lets the cursor roam in it                     */
/* Returns the ending line (zero based) in the item list, or ERROR if the  */
/* user pressed the CANCEL key.                                            */
/* (Predefined menus in the message catalog)                               */
/*                                                                         */
/* arguments:              int menu_no      - Message catalog number        */
/*                         char * menu_pointer - Menu desription lines     */
/*                         int menu_item     - default choice              */
/*                                                                         */
/* return value:           int - zero-based line number of user's choice,  */
/*                               or ERROR if exited menu with CANCEL       */
/*                                                                         */
/* globals referenced:     g_popright, g_popleft, g_poptop, g_keyvalue, LI */
/*                                                                         */
/*                                                                         */
/* globals changed:        g_keyvalue                                      */
/*                                                                         */
/* synopsis:                                                               */
/*     Combine the header and items to produce the menu.  If it has        */
/*     too many lines to fit on the screen, discard excess menu items.     */
/*     Put the menu on the screen using newbox, with the cursor on the     */
/*     given default choice.  Loop on reading commands from the terminal.  */
/*                                                                         */
/*     Starting from the input default item, the UPARROW command moves     */
/*     the cursor to the previous choice (wrapping around to the last);    */
/*     DOWNARROW, RETURN, and LINEFEED all move to the next choice         */
/*     (wrapping around to the first); HOME moves to the first choice;     */
/*     RIGHTARROW and LEFTARROW do nothing whatever; REFRESH refreshes     */
/*     the screen and then acts like CANCEL; HELP puts up the helpbox      */
/*     and returns to the beginning of Emenu; CANCEL and EXECUTE leave     */
/*     leave the menu, returning either ERROR or the selection.            */
/*     Hitting any other key causes the terminal to beep.                  */
/*                                                                         */
/***************************************************************************/

int Emenu (int menu_no, char *menu_pointer,int menu_item, char *filename)
    {
    int olditem;            /* item to remove hilite from*/
    int top, width;
    int menu_size;
    int menu_count=0;
    int *menu_item_size;
    char buffer[PATH_MAX];

    POINTER *menu;

#ifdef DEBUG
    debug ("menu: menu_no %d, menu_pointer %s, menu_item %d",menu_no,menu_pointer, menu_item);  
#endif

    
    menu_pointer = Egetmessage(menu_no,menu_pointer,FALSE);

    menu_size = Ecountlines(menu_pointer,&menu_count);
    menu_item_size = (int *)malloc(sizeof(int)*menu_count);
    menu_size = Esizelines(menu_pointer,menu_item_size);

    if (menu_size > (LI -3))
    {
	Eerror (M_ESMALLSCREEN,
	    "CANCEL to continue.  Screen too small to display menu.");
	free((void *)menu_item_size);
	return(ERROR);
    }

    menu = s2pointer(menu_pointer);

    /* Put the filename into the menu message */
    sprintf(buffer,menu[1],filename);
    s_free(menu[1]);
    menu[1] = s_string(buffer);


restart:

    /* Put up menu in a pop up box */

    newbox(menu);
    width = g_popright -g_popleft -1;

    /* get commands from the terminal */
    olditem = -1;
    for (;;)
	{
	top = g_popbottom - menu_count;
	if (menu_item >= menu_count)
	    menu_item = 0;

	if (g_highlight)
	    {
	    if (olditem >= 0)
		complement_region (g_popleft + 2,
				   top + olditem,
				   g_popleft + menu_item_size[olditem] + 1,
				   top + olditem);

	    complement_region (g_popleft + 2,
			       top + menu_item,
			       g_popleft + menu_item_size[menu_item] + 1,
			       top + menu_item);
	    }

	if( Si_display (FALSE))
	    St_flush ();

	St_goto (top + menu_item, g_popleft + 2);
	St_flush ();
	olditem = menu_item;

	nextin ();
	switch (g_keyvalue)
	    {
	    case UP_ARROW:
		if (--menu_item < 0)
		    menu_item = menu_count - 1;
		break;

	    case DOWN_ARROW:
	    case RETURN:
	    case LINE_FEED:
		if (++menu_item >= menu_count)
		    menu_item = 0;
		break;

	    case LEFT_ARROW:
	    case RIGHT_ARROW:
		Ebell ();
		break;

	    case HOME:
		menu_item = 0;
		break;

	    case REFRESH:
		Refresh ();
		g_keyvalue = CANCEL;
 
		free((void *)menu_item_size);
		return (ERROR);

	    /* asumes U1-U8 are sequential */
	    case U1:
	    case U2:
	    case U3:
	    case U4:
	    case U5:
	    case U6:
	    case U7:
	    case U8:
		if (g_keyvalue-U1 >= menu_count)
		    {
		    Ebell ();
		    continue;
		    }

		killbox (FALSE);

		free((void *)menu_item_size);
		return (g_keyvalue-U1);

	    case DO:
		if (g_keyvalue == DO) {
		    Ebell ();
		    break;
		}

	    case CANCEL:
	    case EXECUTE:
	    case HELP:

		if (g_keyvalue == HELP)
		    {
		    helpmenu (menu_no, menu_item);
		    goto restart;
		    }
		killbox (FALSE);


		free((void *)menu_item_size);
		if (g_keyvalue == CANCEL)
		    return (ERROR);
		return (menu_item);

	    default:
		Ebell ();

	    } /* end switch */
	} /* end FOREVER */
    }

/***************************************************************************/
/* Escrnio:  editor/helper interface routine stub                          */
/*                                                                         */
/* arguments:              char *command                                   */
/*                                                                         */
/* return value:           int - ERROR                                     */
/*                                                                         */
/* globals referenced:     none                                            */
/*                                                                         */
/* globals changed:        none                                            */
/*                                                                         */
/* synopsis:                                                               */
/*     stub routine                                                        */
/***************************************************************************/

int Escrnio (void)
    {
    return (ERROR);
    }

/***************************************************************************/
/* fixcommand:  prompt for underlined args in string                       */
/* Returns an allocated copy with the prompts replaced by the user response*/
/* or ERROR if user hit CANCEL                                             */
/*                                                                         */
/* arguments:              char *command - to be fixed                     */
/*                                                                         */
/* return value:           char *  - the fixed versions                    */
/*                                                                         */
/* globals referenced:     none                                            */
/*                                                                         */
/* globals changed:        none                                            */
/*                                                                         */
/* synopsis:                                                               */
/*     Copy the input command into an output buffer until encountering     */
/*     an underlined character.  Copy the underlined section into another  */
/*     buffer, concatenate a field of spaces to it, and use Eask to get    */
/*     the user to provide a string to be substituted for the underlined   */
/*     section in the output buffer.  If the user hit CANCEL, return       */
/*     ERROR.  Otherwise continue this loop until end of command string,   */
/*     null-terminate the output buffer, and return an allocated copy.     */
/*                                                                         */
/***************************************************************************/

char *fixcommand (char *command)
    {
    ATTR *string;            /* converted command    */
    ATTR *strp;              /* pointer to string    */
    char *bufp;              /* pointer to buffer    */
    char *cp;                /* pointer to prompt    */
    char *answer;            /* answer to prompt     */
    char *buffer;            /* new command string   */
    char *prompt;
    int   growth, offset;

#ifdef DEBUG
    debug ("fixcommand:  command = '%s'", command);
#endif

    string = unpackup (command, 0);
    strp = string;
    buffer = s_nzalloc (obj_count (string) - 1, T_CHAR, "");
    bufp = buffer;

    while (*strp)
	{
	if (! is_underlined(*strp))
	    {
	    *bufp++ = *strp;    /* this avoids a compiler bug       */
	    strp++;            /* that increments strp incorrectly */
	    continue;
	    }

	/* pick out the prompt from the command string */
	prompt = s_nzalloc (obj_count (string) - 1, T_CHAR, "");
	cp = prompt;

	while (*strp && is_underlined(*strp))
	    {
	    *cp++ = de_attr (*strp);  /* this avoids a compiler bug */
	    strp++;                   /* that increments strp incorrectly */
	    }

	*cp = '\0';

	/* ask question and put answer into the command string */
	answer = Eask (M_EPROMPT, "%s                       ", prompt);
	if (answer == (char *) ERROR)
	    {
	    s_free (prompt);
	    s_free (buffer);
	    s_free ((char *)string);
	    return ((char *) ERROR);
	    }

	growth = obj_count (answer) - (cp - prompt);
	if (growth > 0)
	    {
	    offset = bufp - buffer;
	    buffer = s_realloc (buffer, obj_count (buffer) + growth);
	    bufp = buffer + offset;
	    }

	for (cp = answer; *cp; )
	    *bufp++ = *cp++;

	s_free (prompt);
	s_free (answer);
	}

    s_free ((char *)string);
    *bufp = '\0';
    return (buffer);
    }

/***************************************************************************/
/* makelist:  converts array of strings to a POINTER array.                */
/* The input array should end in a NULL pointer                            */
/* The pointer list has pointers to the original strings, NOT copies       */
/*                                                                         */
/* arguments:              char **array - the strings to point to,         */
/*                                        terminated by NULL element       */
/*                                                                         */
/* return value:           POINTER *    - pointers to actual strings       */
/*                                        (not copies! beware s_free)      */
/*                                                                         */
/* globals referenced:     none                                            */
/*                                                                         */
/* globals changed:        none                                            */
/*                                                                         */
/* synopsis:                                                               */
/*     First walk down the array looking for the NULL terminator,          */
/*     then allocate a pointer array of the right number of elements.      */
/*     Finally, just make each pointer element point to the                */
/*     corresponding array element.                                        */
/*                                                                         */
/***************************************************************************/

POINTER *makelist (char **array)
    {
    int i;
    POINTER *list;

#ifdef DEBUG
    debug ("makelist:  array = 0%o", array);
#endif

    for (i = 0; array [i]; i++) /* get length of list */
	;

    list = (POINTER *) s_alloc (i, T_POINTER, NULL);

    for (i = 0; array [i]; i++) /* get length of list */
	list [i] = array [i];

    return (list);
    }

/***************************************************************************/
/* task:  handler for the TASK button                                      */
/*                                                                         */
/* arguments:              int   argtype  - whether NOARGS, EMPTYARG, ...  */
/*                         char *strvalue - argument if STRINGARG          */
/*                         int   numvalue - argument if NUMBERARG          */
/*                                                                         */
/* return value:           void                                            */
/*                                                                         */
/* globals referenced:     g_usefiles                                      */
/*                                                                         */
/* globals changed:        none                                            */
/*                                                                         */
/* synopsis:                                                               */
/*     If the argtype is STRINGARG, execute the string argument in         */
/*     a popbox, using the argument as its own description.                */
/*     Otherwise put up the task menu with a standard task menu            */
/*     header and menu options from the global g_usefiles.                 */
/*                                                                         */
/***************************************************************************/

void task (int argtype, char *strvalue, int numvalue)
    {

#ifdef DEBUG
    debug ("task:  type = %d, strvalue = '%s', numvalue = %D",
	argtype, strvalue, numvalue);
#endif

    if (argtype == STRINGARG)
	usepop (strvalue, strvalue, FALSE, FALSE);
    else
	taskmenu (MN_EUSEMENU, "             New Task Menu\n\n\
Select a menu option (for example, move the\n\
cursor to an item and EXECUTE).  Otherwise,\n\
press CANCEL to remove the menu, or\n\
press HELP to display help information.\n#0", g_usefiles);
    }

/***************************************************************************/
/* taskmenu:  handler for the TASK and HELP menus                          */
/*                                                                         */
/* arguments:              int helpcode - extended help message number     */
/*                         POINTER *hdrlist - menu header lines            */
/*                         POINTER *itemlist - menu choices lines          */
/*                                                                         */
/* return value:           void                                            */
/*                                                                         */
/* globals referenced:     none                                            */
/*                                                                         */
/* globals changed:        none                                            */
/*                                                                         */
/* synopsis:                                                               */
/*     Invoke Efixvars to instantiate any variables in the item list       */
/*     (for example, to resolve current value of $HELPER).  Use Emenu      */
/*     to put up the menu.  If user hit CANCEL, return.                    */
/*                                                                         */
/*     Invoke Efixvars again, and fixcommand, to fill in the details       */
/*     of the user's choice.  (For example, to specify a new helper).      */
/*     If user hits CANCEL to a fixcommand prompt, return.                 */
/*                                                                         */
/*     Finally, depending on the flags, either go to some other file,      */
/*     use some other form, exit from the editor, use a popbox or          */
/*     a screen, use some other helper, or load new macro glossary.        */
/*     If a popbox or a screen, then if specified by the flags do an Esync(*/
/*     and possibly a savefakes(), to get the current file to disk, and bac*/
/*     up any open non-structured files.                                   */
/*                                                                         */
/*                                                                         */
/**************************************************************************/

void taskmenu (int helpcode, char *hdrlist, POINTER *itemlist)
    {
    int i;                      /* used to Efixvars items  */
    int line;                   /* menu return value       */
    int flags;                  /* USE menu item flags     */
    POINTER *desc;              /* g_usefiles item chosen  */
    char *header;               /* used to display menu    */
    POINTER *items;             /* fixvar'ed itemlist      */
    char *filename;             /* file name to edit       */
    char *cp;                   /* temporary pointer       */
    char Hfilename[PATH_MAX+1]; /* Help text msg cat name  */

    extern char g_helpstate;    /* asking for help?        */

#ifdef DEBUG
    debug ("taskmenu:  header = 0%o, items = 0%o", hdrlist, itemlist);
#endif

    header = hdrlist;
    items = (POINTER *) s_alloc (obj_count (itemlist), T_POINTER, NULL);

    for (i = 0; i < obj_count (itemlist); i++)
	items [i] = Efixvars (itemlist [i]);

    line = Ecustommenu(helpcode, header, items, 0);
    s_free ((char *)items);

    if (line == ERROR)
    {
	/* return from help menu into either the user's file or the */
	/* help file */
	if (g_helpstate == INHELPMENU || g_helpstate == IN2NDHMENU)
	    g_helpstate --;     /* = NOTHELPING or INHELPFILE, */
				/* respectively */
	return;
    }

    desc = (POINTER *) itemlist [line];
    cp = Efixvars (obj_name (desc));
    filename = fixcommand (cp);
    s_free (cp);

    if (filename == (char *) ERROR) /* user CANCEL'ed out of a prompt */
	return;

    flags = obj_flag (desc);

#ifdef DEBUG
    debug ("taskmenu:  flags = '%d', desc = '%s'", flags, desc);
#endif

    switch (flags & USETYPE)
	{
	case USEFILE:

	    if (g_helpstate)
	    {    /* create the Help text msg catalogue name :- */
                 /* from 'H<filename>.cat'                     */
                 (void)strcpy(Hfilename, "H");
                 if (strrchr(filename, '/') == NULL)
                     (void)strcat(Hfilename, filename);
                 else
                     (void)strcat(Hfilename, strrchr(filename, '/')+1);
                 (void)strcat(Hfilename, ".cat");
	         g_helpfilecatd = catopen(Hfilename, NL_CAT_LOCALE);
	    }

	    if (seq (obj_name (desc), "$ALTFILE"))
		{
		altfile ();
		break;
		}
	    g_inmenu = TRUE;    /* so makefile will create structured */
				/* files */
	    usefile (filename);
	    g_inmenu = FALSE;
	    break;

	case USEFORM:
	    if (Euseform (filename) == ERROR)
		Eerror (M_EUSEFORM, "Cannot open form %s.", filename);
	    break;

	case USEQUIT:
	    e2exit (0,"",0);

	case USEPOPBOX:
	case USESCREEN:
	    if (flags & (USESAVE | USESYNC)) {
		s_free (Esync ());
		if (flags & USESAVE)
		    savefakes (); /* save all the fake files */
		}

	    cp = Efixvars ((char *)desc);

	    if ((flags & USETYPE) == USEPOPBOX)
		usepop (filename, cp, FALSE, FALSE);
	    else
		usescreen (filename, cp);
	    s_free (cp);

	    break;

	case USEHELPER:

	    pushstate (TRUE);
	    openhelper (filename,
			seq (filename, DIRHELPER) ? g_curdir : g_filename,
			NULL);
	    break;


	}
    s_free (filename);
    }

/***************************************************************************/
/* usepop:  run program and put output in pop-up box                       */
/*                                                                         */
/* arguments:              char *command     - to be executed in subshell  */
/*                         char *description - to show in box while running*/
/*                         int   bellflag    - true if bell should be rung */
/*                         int   watchflag   - true if from watchfiles     */
/*                                                                         */
/* return value:           void                                            */
/*                                                                         */
/* globals referenced:     g_syncname, LI, CO                              */
/*                                                                         */
/* globals changed:        none                                            */
/*                                                                         */
/* synopsis:                                                               */
/*     Invoke flush_window to ensure the current window is safe.           */
/*     If description was provided, use Emessage to show the               */
/*     "Executing <whatever>..." message.  Run the command in a            */
/*     subprocess and examine its output.                                  */
/*                                                                         */
/*     If the command failed, and the g_syncname global is set,            */
/*     use Ereopen to reopen the file and Eerror to complain               */
/*     before returning.                                                   */
/*                                                                         */
/*     Otherwise display the output of the command.  If it fits            */
/*     on the screen, put in in a box and kill the box when the            */
/*     user types anything.  If it is too big for the screen,              */
/*     do a pseudo-"more" operation to walk through the output.            */
/*     If there is no output at all, use Eerror to say so.                 */
/*     If g_syncname is set, invoke Ereopen before returning.              */
/*                                                                         */
/***************************************************************************/

void usepop (char *command, char *description, int bellflag, int watchflag)
    {
    FILE *fp;             /* fp from system process   */
    POINTER *list;        /* pop box message          */
    int toobig;           /* TRUE if message too big  */
    int i;                /* used to print list       */
    int count;            /* number of output lines   */
    char *line;
    char *buffer;         /* Holds extended popen command */
    char *outstring;      /* holds user instructions  */

#ifdef DEBUG
    debug ("usepop:  command = '%s', description = '%s'",
	   command, description);
#endif

    buffer = s_alloc (strlen (command) + 20, T_CHAR, NULL);
    flush_window ();

    if (description)
	Emessage (M_ERUNPOP, "Running %s.", description);
    else
	description = command;

    (void) sprintf (buffer, "sh -c \"%s\" 2>&1", command);

    fp = popen (buffer, "r");
    s_free (buffer);

    if (fp == NULL)
	{
	if (g_syncname) /* if file has been sync'ed */
	    Ereopen ();

	Eerror (M_EUSEPOP, "Cannot run program %s.", description);
	return;
	}
    list = (POINTER *) s_alloc (0, T_POINTER, NULL);
    toobig = FALSE;
    count = 0;
    line = s_getline (fp);

    while (line != NULL)
	if (toobig)             /* Continue oversize output */
	    {
	    if (count++ > (LI - 3))
		{
		count = 0;
		outstring = Egetmessage(M_EPOPMORE,"Touch RETURN to see more.",FALSE);
		(void) printf (outstring);
		s_free (outstring);
		(void) fflush (stdout);

		while ((getchar () != '\n') && (! g_intflag))
		    ;
		if (g_intflag)
		    break;
		}
	    count += obj_count (line) / CO;

	    (void) printf ("%s\n", line);
	    s_free (line);
	    line = s_getline (fp);
	    }

	else if (count >= (LI - 5))  /* Abandon popbox; start oversize */
				     /* output */
	    {
	    Ettyfix ();
	    /* Delay required here to give screen enough time to clear 
	       before any more output is sent to the screen. */
	    sleep(1);
	    outstring = Egetmessage (M_EPOPBIG, "The following output of\n%s\nis too large for a popup box:\n", FALSE);
	    (void) printf (outstring, description);
	    s_free (outstring);

	    for (i = 0; i < count; i++)
		(void) printf ("%s\n", list [i]);

	    count += 2;
	    (void) fflush (stdout);
	    s_free ((char *)list);
	    toobig = TRUE;
	    }

	else                            /* Continue adding to popbox */
	    {
	    char *cp = line;
	    int width;

	    while ((width = text_width (cp)) > CO - 2)
		{
		/* Convert string to attr to calculate the position */
		/* in line where column (CO - 2) is reached */
		/* and split the line at this position */
		ATTR *str = unpackup(cp, width);
		int split_pos = calc_line_position(str, CO - 2);
		char oldc = cp[split_pos];

		cp[split_pos] = '\0';
		list = s_append (list, s_string (cp));
		cp += split_pos;
		*cp = oldc;
		count++;
		}
	    list = s_append (list, s_string (cp));
	    count++;
	    s_free (line);
	    line = s_getline (fp);
	    }

    pclose (fp);

    if (g_syncname) /* if file has been sync'ed */
	Ereopen ();

    if (toobig)
	{
	outstring = Egetmessage (M_EPOPDONE,"Touch RETURN to continue editing.",FALSE);
	(void) printf (outstring);
	s_free (outstring);
	(void) fflush (stdout);
	(void) fgetc (stdin);
	Ettyunfix ();
	return;
	}

    if (g_intflag)
	Eerror (M_EPOPBREAK, "Command stopped by BREAK");

    else if (count > 0)
	{
	newbox (list);

	if (bellflag)
	    Ebell ();

	onechar (FALSE); /* wait for user to type character */
	killbox (FALSE);
	}

    else if (watchflag)
        killbox (FALSE);

    else
	Eerror (M_EPOPOUTPUT, "There is no output from command\n%s", description);

    s_free ((char *)list);
    }

/***************************************************************************/
/* usescreen:  run program and put output on tv screen                     */
/*                                                                         */
/* arguments:              char *command     - to be executed in subshell  */
/*                         char *description - to put on screen first      */
/*                                                                         */
/* return value:           void                                            */
/*                                                                         */
/* globals referenced:     g_syncname                                      */
/*                                                                         */
/* globals changed:        none                                            */
/*                                                                         */
/* synopsis:                                                               */
/*     Flush the current window, write out the description, and execute    */
/*     the command as a filter onto the screen.  When finished, wait for   */
/*     the user to type anything.  If g_syncname is set, invoke Ereopen    */
/*     before returning.                                                   */
/*                                                                         */
/***************************************************************************/

void usescreen (char *command, char *description)
    /* command to execute     */
  /* description of command */
    {
    char *outstring;    /* for printf output */

#ifdef DEBUG
    debug ("usescreen:  command = '%s', description = '%s'",
	   command, description);
#endif


    flush_window ();

    /* this avoids some funny-looking repainting        */
    killbox (FALSE);
    Ettyfix ();
    (void) printf ("%s\n\n", description);
    (void) fflush (stdout);
    filter (command, NULL, NULL);
    outstring = Egetmessage (M_ESCREENDONE,"\nTouch RETURN to continue editing.", FALSE);
    (void) printf (outstring);
    s_free (outstring);
    (void) fflush (stdout);
    (void) fgetc (stdin);

    Ettyunfix ();

    if (g_syncname) /* if file has been sync'ed */
	{
	Ereopen ();
	Refresh ();
	}
    }

/***************************************************************************/
/* Ecustommenu: puts up menu and lets the cursor roam in it                */
/* Returns the ending line (zero based) in the item list, or ERROR if the  */
/* user pressed the CANCEL key.                                            */
/* User defined menus.                                                     */
/*                                                                         */
/* arguments:              int msg_no    - extended help message number    */
/*                         char *tmp_hdr   - menu description lines        */
/*                         POINTER *items  - menu choice lines             */
/*                         int curitem     - default choice                */
/*                                                                         */
/* return value:           int - zero-based line number of user's choice,  */
/*                               or ERROR if exited menu with CANCEL       */
/*                                                                         */
/* globals referenced:     g_popright, g_popleft, g_poptop, g_keyvalue, LI */
/*                                                                         */
/*                                                                         */
/* globals changed:        g_keyvalue                                      */
/*                                                                         */
/* synopsis:                                                               */
/*     Combine the header and items to produce the menu.  If it has        */
/*     too many lines to fit on the screen, discard excess menu items.     */
/*     Put the menu on the screen using newbox, with the cursor on the     */
/*     given default choice.  Loop on reading commands from the terminal.  */
/*                                                                         */
/*     Starting from the input default item, the UPARROW command moves     */
/*     the cursor to the previous choice (wrapping around to the last);    */
/*     DOWNARROW, RETURN, and LINEFEED all move to the next choice         */
/*     (wrapping around to the first); HOME moves to the first choice;     */
/*     RIGHTARROW and LEFTARROW do nothing whatever; REFRESH refreshes     */
/*     the screen and then acts like CANCEL; HELP puts up the helpbox      */
/*     and returns to the beginning of Emenu; CANCEL and EXECUTE leave     */
/*     leave the menu, returning either ERROR or the selection.            */
/*     Hitting any other key causes the terminal to beep.                  */
/*                                                                         */
/***************************************************************************/

int Ecustommenu (int msg_no, char *tmp_hdr, POINTER *items, int curitem)
    {
    int i;                  /* loop counter              */
    int index1;             /* message index             */
    int hdrsize;            /* number of lines in header */
    int itemsize;           /* number of items           */
    int olditem;            /* item to remove hilite from*/
    int freehdr=FALSE;      /* TRUE if header should be freed */
    int top, width;
    int menu_opt;

    char *line;             /* line in menu              */
    POINTER *message;       /* message to put in box     */
    POINTER *header;        /* header text               */

#ifdef DEBUG
    debug ("Ecustommenu:  msg_no %d,header = %d, items = 0%o, curitem = %d",msg_no,tmp_hdr,items,curitem);
#endif

	tmp_hdr = Egetmessage (msg_no,tmp_hdr,FALSE);

	header = s2pointer(tmp_hdr);
	if (header != NULL)
	   freehdr = TRUE;

	hdrsize = Ecountlines (tmp_hdr, &menu_opt);

        itemsize = obj_count (items);

        if (itemsize + hdrsize > (LI -3))
	{
	  Eerror (M_ESMALLSCREEN,
	    "CANCEL to continue.  Screen too small to display menu.");
	  return(ERROR);
	}


restart:

    /* make message from header and items */

    message = (POINTER *) s_alloc (itemsize + hdrsize,T_POINTER, (char *) NULL);
    index1 = 0;

    for (i = 0; i < hdrsize; i++)
 	{
	message[index1]=header[index1];
 	if (obj_count (message[index1]) > CO - 2)
 	    {
 	    message[index1] = s_realloc (message[index1], CO - 2);
 	    message[index1][CO - 3] = '\0';
 	    }
 	index1++;
 	}

    for (i = 0; i < itemsize; i++)
	{
	if (obj_count (items[i]) > CO - 3)
	    {
	    items[i] = s_realloc (items[i], CO - 3);
	    items[i][CO - 4] = '\0';
	    }
	line = s_string (items [i]);
	line = s_insert (line, 0, 1);
	*line = SPACE;
	message [index1++] = line;
	}

    /* put up message in a pop up box */
    newbox (message);

    /* get commands from the terminal */
    olditem = -1;
    for (;;)
	{
	top = g_popbottom - itemsize;
	if (curitem >= itemsize)
	    curitem = 0;

	if (g_highlight)
	    {
	    if (olditem >= 0)
		complement_region (g_popleft + 2,
				   top + olditem,
				   g_popleft + text_width(items[olditem]) + 1,
				   top + olditem);

	    complement_region (g_popleft + 2,
			       top + curitem,
			       g_popleft + text_width(items[curitem]) + 1,
			       top + curitem);
	    }

	if( Si_display (FALSE))
	    St_flush ();

	St_goto (top + curitem, g_popleft + 2);
	St_flush ();
	olditem = curitem;

	nextin ();
	switch (g_keyvalue)
	    {
	    case UP_ARROW:
		if (--curitem < 0)
		    curitem = itemsize - 1;
		break;

	    case DOWN_ARROW:
	    case RETURN:
	    case LINE_FEED:
		if (++curitem >= itemsize)
		    curitem = 0;
		break;

	    case LEFT_ARROW:
	    case RIGHT_ARROW:
		Ebell ();
		break;

	    case HOME:
		curitem = 0;
		break;

	    case REFRESH:
		Refresh ();
		g_keyvalue = CANCEL;

		if (freehdr)
		    s_free ((char *)header);

		return (ERROR);

	    /* asumes U1-U8 are sequential */
	    case U1:
	    case U2:
	    case U3:
	    case U4:
	    case U5:
	    case U6:
	    case U7:
	    case U8:
		if (g_keyvalue-U1 >= itemsize)
		    {
		    Ebell ();
		    continue;
		    }

		 killbox (FALSE);

		 if (freehdr)
		    s_free ((char *)header);

		return (g_keyvalue-U1);

	    case DO:
		if (g_keyvalue == DO)
		    Ebell ();
		break;

	    case CANCEL:
	    case EXECUTE:
	    case HELP:

		if (g_keyvalue == HELP)
		    {
		    helpmenu (msg_no, curitem);
		    goto restart;
		    }
		killbox (FALSE);

		if (freehdr)
		    s_free ((char *)header);

		if (g_keyvalue == CANCEL)
		    return (ERROR);
		return (curitem);

	    default:
		Ebell ();

	    } /* end switch */
	} /* end FOREVER */
    }
