static char sccsid[] = "@(#)95	1.2  src/rspc/usr/lib/boot/softros/aixmon/esck.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:00";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: cmd_backspace
 *		cmd_deol
 *		cmd_down
 *		cmd_up
 *		cursor_left
 *		cursor_right
 *		esck
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef DEBUG

/* esck.c - Simple command line editor like ksh esc - k 	*/

#include	"esck.h"
#include	<string.h>


static int cmd_no = 0;		// Current command in loop
static char hist[HIST_SIZE][MAXCMDL];	// History storage
char	tstr[MAXCMDL+1];	// Temp String
char	blank[MAXCMDL];		// Blank Command line string
int	epos;			// Position of last char
int	cpos;			// Position of the cursor
char    *estr;			// Current temp edit string

char *esck()
{
static int esck_init = FALSE;	// Initialization flag
int	i,j;
int	insert;			// Insert Mode flag
char	mode;			// Current Edit Mode
unsigned char kee;		// Input Character
int	current_cmd;		// Current command being edited
				// This causes last edit to be on top
				// of the stack and leaves the old command

if(!esck_init) 			// Initialize hist if first time
	{
	for(i=0;i<HIST_SIZE;i++)
		hist[i][0] = 0;
	for(i=0;i<MAXCMDL;i++)
		blank[i] = ' ';
	blank[MAXCMDL-1] = 0;
	esck_init = TRUE;
	}

current_cmd = cmd_no;		// Save the current command number
estr = hist[cmd_no];		// Point to first edit string
tstr[0] = 0;			// Make this a new one
cpos = epos = 0;
mode = ENTRY;			// Beginning in Entry mode
insert = FALSE;			// Start out in replace mode

while(1)
	{
	kee = getch();

	switch(mode)
		{
		case	COMMAND:	// VI commands
		switch(kee)
			{
			case	ENTER:
			case	DOS_ENTER:
			cmd_no = current_cmd;
			cmd_no=(cmd_no<(HIST_SIZE-1))?(++cmd_no):0;
			tstr[epos] = 0;
			estr = hist[current_cmd];
			strcpy(estr,tstr);
			putchar(kee);
			return(estr);

			case	VI_UP:
			cmd_up();
			break;

			case	VI_DOWN:
			cmd_down();
			break;

			case	VI_APPEND:
			cursor_right(epos-cpos);
			mode = ENTRY;
			break;

			case	VI_INSERT:
			mode = ENTRY;
			insert = TRUE;
			break;

			case	VI_append:
			cursor_right(1);
			mode = ENTRY;
			insert = TRUE;
			break;

			case	VI_REPLACE:
			case	VI_replace:
			mode = ENTRY;
			break;

			case	VI_LEFT:
			case	BACKSPC:
			if(cpos)
				{
				cursor_left(1);
				cpos--;
				}
			break;

			case	VI_RIGHT:
			cursor_right(1);
			break;

			case	VI_DEL:
			cmd_backspace(0);
			break;

			case	VI_BACKW:
			if(cpos-2 >= 0)
				{
				cpos-=2;
				cursor_left(2);
				}
			else
				{
				cursor_left(cpos);
				cpos = 0;
				}
			while(cpos && tstr[cpos] != ' ') 
				{
				cursor_left(1);
				cpos--;
				}
			if(cpos)
				cursor_right(1);
			break;

			case	VI_FORWW:
			while((cpos < epos) && tstr[cpos] != ' ')
				cursor_right(1);
			if(cpos<epos)
				cursor_right(1);
			break;

			case	VI_HOME:
			cursor_left(cpos);
			cpos = 0;
			break;

			case	VI_DEOL:
			cmd_deol();
			break;

			default:
			break;
			}
		break;

		case	HALF_CMD:	// 2nd Char of 2 char VI commands
		switch(kee)
			{
			default:
			mode = COMMAND;
			break;
			}
		break;

		case	ENTRY:
		switch(kee)
			{
			case	ENTER:
			case	DOS_ENTER:
			cmd_no = current_cmd;
			cmd_no=(cmd_no<(HIST_SIZE-1))?(++cmd_no):0;
			tstr[epos] = 0;
			estr = hist[current_cmd];
			strcpy(estr,tstr);
			putchar(kee);
			return(estr);

			case	ESC:
			mode = COMMAND;
			insert = FALSE;
			break;

			case	BACKSPC:
			cmd_backspace(1);
			break;

			case	EMACS_DOWN:
			cmd_down();
			break;

			case	EMACS_UP:
			cmd_up();
			break;

			case	EMACS_LEFT:
			if(cpos)
				{
				cursor_left(1);
				cpos--;
				}
			break;

			case	EMACS_RIGHT:
			cursor_right(1);
			break;

			case	EMACS_HOME:
			cursor_left(cpos);
			cpos = 0;
			break;

			case	EMACS_END:
			cursor_right(epos-cpos);
			break;

			case	EMACS_DEOL:
			cmd_deol();
			break;	
			
			case	EMACS_DC:
			cmd_backspace(0);
			break;

			default:
			if(((cpos+1) < MAXCMDL) && ((epos+1)< MAXCMDL))
				{
				if(!insert)
					{
					tstr[cpos++] = kee;
					epos = (cpos>epos)?cpos:epos;
					putchar(kee);
					}
				else
					{
					putchar(kee);
					for(i=epos;i>=cpos;i--)
						tstr[i+1] = tstr[i];
					epos++;
					for(i=cpos+1;i<epos;i++)
						putchar(tstr[i]);	
					tstr[cpos++] = kee;
					cursor_left(epos-cpos);
					}
				}
			break;
			}
		break;
		}
	}
}

/******************************/
/* Cursor Left		      */
/******************************/

static void cursor_left(int num)
{
int	i,j;
for(i=0;i<num;i++)
	{
		putchar('\b');
	}
}

/******************************/
/* Cursor Right		      */
/******************************/

static void cursor_right(int num)
{
int	i;
i=cpos+num;
for(;cpos<i;cpos++)
	if(cpos < epos)
		putchar(tstr[cpos]);
}

/********************************/
/* cmd_up - Move up one command */
/********************************/

static void cmd_up(void)
{
cmd_no=(cmd_no>0)?(--cmd_no):(HIST_SIZE-1);
estr = hist[cmd_no];
strcpy(tstr,estr);
cursor_left(cpos);
printf("%s",blank);
cursor_left(strlen(blank));
printf("%s",tstr);
epos = strlen(tstr);
cpos = epos;
}

/********************************/
/* cmd_down - Mode down one cmd */
/********************************/

static void cmd_down(void)
{
cmd_no=(cmd_no<(HIST_SIZE-1))?(++cmd_no):0;
estr = hist[cmd_no];
strcpy(tstr,estr);
cursor_left(cpos);
printf("%s",blank);
cursor_left(strlen(blank));
printf("%s",tstr);
epos = strlen(tstr);
cpos = epos;
}

/********************************/
/* cmd_backspace - Backspace    */
/* type - 0 gives delete func   */
/* type - 1 gives backspace func*/
/********************************/

static void cmd_backspace(int type)
{
int	i;

if(cpos)
	{
	if(cpos == epos)
		{
		tstr[epos] = 0;
		printf("\b \b");
		cpos--;
		epos--;
		}
	else
		{
		cursor_left(type);
		for(i=cpos-type;i<epos;i++)
			{
			tstr[i] = tstr[i+1];
			putchar(tstr[i]);
			}
		cpos-=type;
		epos--;
		cursor_left(epos-(cpos-1));
		}	
	}
}

/********************************/
/* cmd_deol - Delete to end of l*/
/********************************/

static void cmd_deol(void)
{
int	i;
for(i=cpos;i<epos;i++)
	{
	tstr[i] = 0;
	putchar(' ');
	}
cursor_left(epos-cpos);
}
#endif /*DEBUG*/
