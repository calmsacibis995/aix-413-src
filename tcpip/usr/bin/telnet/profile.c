static char sccsid[] = "@(#)28	1.16  src/tcpip/usr/bin/telnet/profile.c, tcptelnet, tcpip411, 9432A411a 8/5/94 09:17:29";
/* 
 * COMPONENT_NAME: TCPIP profile.c
 * 
 * FUNCTIONS: IS_MAPPTR, MSGSTR, bindtoken, bindcolor, error, first_keymap, 
 *            free_keymap, fs_input, getcolor, input, keyname, load, 
 *            new_keymap, parse, pop_env, prof_init, push_env, 
 *            string_to_ident, tokenclass, unput , ident_to_string
 *
 * ORIGINS: 10  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<errno.h>
#include	<pwd.h>
#include	<termios.h>
#include	<stdio.h>
#ifndef AIX
#include	<sys/file.h>
#endif /* not AIX */
#include	<sys/types.h>
#include	"tokentype.h"
#include	"parse.h"
#include	"keyfunctions.h"

extern int tn3270flag;
extern int push_mode;
extern int HEBREW_ON;
extern int ARABIC_ON;

#define		USER_PROFILE	    	"~/.3270keys"
#ifdef AIX
#define		SYSTEM_PROFILE	        	"/etc/3270.keys"
#define		TN_SYSTEM_PROFILE      	"/etc/tn3270.keys"
#else
#define		SYSTEM_PROFILE 	"/usr/lib/3270.keys"
#endif /* AIX */
#define		NESTING_DEPTH	40

/* private functions */
#ifdef  DEBUG3270
#define TN_LOG(prspec)  fprintf prspec
#else   DEBUG3270
#define TN_LOG(prspec)
#endif  /* DEBUG3270 */

#include <nl_types.h>
#include "telnet_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TELNET,n,s) 

#define		FALSE		0
#define		TRUE		1

#include	<cur01.h>

#define		DEL		'\177'

/* global color variables */
int	lo_prot_color;		/* low intensity, protected    */
int	lo_unprot_color;	/* low intensity, unprotected  */
int	hi_prot_color;		/* high intensity, protected   */
int	hi_unprot_color;	/* high intensity, unprotected */
int     background_color;	/* global background color     */

/* possible colors for your viewing pleasure */
#define		S_BLUE		"blue"
#define		S_RED		"red"
#define		S_GREEN		"green"
#define		S_WHITE		"white"
#define		S_MAGENTA	"magenta"
#define		S_CYAN		"cyan"
#define		S_BLACK		"black"

#define		FOREGROUND	0
#define		BACKGROUND	1

/* variables global to the profile code */
static struct keymap	*firstmap;
static struct keymap	*thismap;
static char		*termtype;
static	int bindcolor (int *, char *, int);
static	int getcolor (int, char *);

extern int InitControl();

struct environ {
    FILE	*infile;
    int		last_condition;
    int		flags;
    int		linenumber;
    char	filename[1024];
};

static struct environ	envstack[NESTING_DEPTH];
static struct environ	*env;

#ifdef AIX
struct keymap {
    struct {
	int	tag;	/* 0 if function, 1 if nextmap */
	union {
	    int			(*entfunction) ();
	    struct keymap	*entnextmap;
	}	ent;
    }	entry[256];
};
#define	function	ent.entfunction
#define	nextmap		ent.entnextmap

#define	IS_MAPPTR(e)	((e).tag)
#else
struct keymap {
    union {
	int		(*function) ();
	struct keymap	*nextmap;
    }			entry[256];
};
/* the following line reflects silly machine-dependent assumptions */
#define	IS_MAPPTR(e)	((int (*)())((e).nextmap) >= etext)
#endif /* AIX */

/* exported functions */
void		prof_init ();
int		input ();
int		unput ();
void		scan_keys ();

/* imported functions */
void		fs_redraw ();

#ifdef BIDI  /* BIDI support */

void            fs_autopush ();
void            fs_scrrev ();    
void            fs_push ();       
void            fs_endpush ();    
void            fs_eng_lang ();   
void            fs_national_lang ();   
void            fs_fldrev ();     
void            fs_autorev ();   

#endif /* BIDI support */

#ifdef ARAB /* start ARABIC support */
void            fs_autoshape ();
void            fs_isolated ();
void            fs_initial ();
void            fs_middle ();
void            fs_final ();
void            fs_fieldshape ();
void            fs_fieldbase ();
extern unsigned char ARAB_KYB_MAP[256];
extern int ARABIC_ON;
extern FILE    *dumpfno;
#endif /* end ARABIC support */


/* lexical scanner interface */
extern int	yylex ();
extern char	yytext[];

/* private functions */
void		bindtoken ();
static void		error ();
static struct keymap	*first_keymap ();
static void		free_keymap ();
static char		*keyname ();
static int		load ();
static struct keymap	*new_keymap ();
static void		parse ();
static void		push_env ();
static void		pop_env ();
static void		string_to_ident ();
static int		tokenclass ();
static char		*ident_to_string ();
#ifdef ARAB
void                    Init_Arab_7bit_Table();    /* ARABIC support */
#endif /* ARAB */

extern int		errno;
extern			etext ();
char			*getenv ();
#ifdef AIX
char			*strpbrk ();
#else
char			*index ();
#endif /* AIX */

void prof_init ()
{
    if ((termtype = getenv ("TERM")) == (char *)NULL) {
	fputs (MSGSTR(NO_TERM, "No environment-specified terminal type.\n"), stderr); /*MSG*/
	exit (1);
    }

    thismap = firstmap = first_keymap ();

    env = envstack;
    if (tn3270flag)
	(void)InitControl (termtype,0);
    else if (! load (USER_PROFILE, FALSE))
	  (void)load (SYSTEM_PROFILE, TRUE);
#ifdef ARAB
  if (ARABIC_ON)
      Init_Arab_7bit_Table ();
#endif /* ARAB */
}
#ifdef ARAB      /* ARABIC support */

void Init_Arab_7bit_Table()
{
  int  i;
  char key_map_filename[131] = "/etc/3270_arab_kyb.map";
  char line[80], *lpoint;
  FILE *fid;

  /* init struct to default values */
  for (i = 0; i < 256; i++)
    ARAB_KYB_MAP[i] = i;

TN_LOG((dumpfno,"map_file_name %s \n",key_map_filename));
  if ( (fid = fopen(key_map_filename, "r")) == NULL)
     {TN_LOG((dumpfno,"ERROR: the keymap file was not found in /etc\n"));
      return;}

TN_LOG((dumpfno,"opened   \n"));
        while( !feof(fid) )
        {
                lpoint = fgets(line, 80, fid);
                lpoint = strtok(line, " ");
                if(*lpoint != '#')
                {

                        i      = atoi(lpoint);
                        lpoint = strtok(NULL, " ");
                        ARAB_KYB_MAP[i] = atoi(lpoint);
                }
        }

  fclose(fid);

}
#endif /* ARAB */    /*  end of ARABIC support */

/* create the first keymap and initialize it to something reasonable */
static struct keymap *first_keymap ()
{
    register struct keymap	*kp = new_keymap ();
    register int		i;
    struct termios		sg;

    if (tcgetattr(0,(struct termios *)&sg) != 0) {
	perror (MSGSTR(P_TIOCGETP, "TIOCGETP")); /*MSG*/
	exit (1);
    }
    kp->entry['\t'].function = fs_tab;
    kp->entry['\r'].function = fs_enter;
    kp->entry['\n'].function = fs_return;
    for (i = ' '; i < '\377'; i++) {
		if (isprint(i))
		kp->entry[i].function = fs_character;
    }
    if (sg.c_cc[VERASE] == DEL) {
	kp->entry[DEL].function = fs_backspace;
	kp->entry['\b'].function = fs_delete;
    }
    else {
	kp->entry[sg.c_cc[VERASE]].function = fs_backspace;
	kp->entry[DEL].function = fs_delete;
    }

    background_color = B_BLACK;		/* default background color    */
    lo_prot_color = F_BLUE;		/* low intensity, protected    */
    lo_unprot_color = F_GREEN;		/* low intensity, unprotected  */
    hi_prot_color = F_WHITE;		/* high intensity, protected   */
    hi_unprot_color = F_RED;		/* high intensity, unprotected */

    return kp;
}

static int load (filename, complain)
char	*filename;
int	complain;
{
    char		*slash, *cp;
    struct passwd	*pw;
#ifdef AIX
    char		*strchr();
#define	index(a,b)	strchr(a,b)
     struct passwd	*getpwuid(uid_t);
     struct passwd	*getpwnam(char *);
#endif /* AIX */

    if (*filename == '~') {
	if (slash = index (++filename, '/'))
	    *slash = '\0';

	pw = NULL;
	if (*filename) {  /* there's a username after the ~ */
	    if (!(pw = getpwnam(filename))) {
		error(MSGSTR(INV_USER, "no such user: \"%s\"\n"), filename);
		return FALSE;
	    }
	} else {  /* try our $HOME first */
	    if (!(cp = getenv("HOME")) || !*cp)  /* no $HOME: get our pw dir */
		if (!(pw = getpwuid(getuid()))) {
		    perror("getpwuid");
		    return(FALSE);
		}
	}

	strcpy (env->filename, pw ? pw->pw_dir : cp);
	if (slash) {
	    *slash = '/';
	    strcat (env->filename, slash);
	}
    }
    else
	strcpy (env->filename, filename);

    if ((env->infile = fopen (env->filename, "r")) == (FILE *)NULL) {
	if (complain || errno != ENOENT)
	    perror (env->filename);
	return FALSE;
    }

    env->linenumber = 1;
    parse ();
    fclose (env->infile);
    env->infile = (FILE *)NULL;
    env->filename[0] = '\0';

    return TRUE;
}

static void parse ()
{
    int			token;
    register int	class;
    register int	state = S_STRT;
    register int	action;
    int			execute;
    int			last_function;

    env->last_condition = FALSE;
    env->flags = 0;
    
    while (state != S_STRT || ! (env->flags & (F_EXIT|F_QUIT))) {

	if (state == S_STRT)
	    execute = TRUE;

	class		= tokenclass (token = yylex ());
/*	fprintf (stderr, "state %d, token %d, class %d -> ",
		 state, token, class);	 * DEBUG */
	action		= actions[class][state];
	state		= states[class][state];
/*	fprintf (stderr, "state %d, action <%d>%d, cond %d, exec %d\n",
		 state, action >> 8, action & ~F_FLAGS,
		 env->last_condition, execute);  * DEBUG */

	if (! execute)
	    continue;		/* test failed - don't execute statement */

	env->flags	|= action & (F_EXIT|F_QUIT);
	if (action & F_S2I)
	    string_to_ident (yytext);

	switch (action & ~F_FLAGS) {
	    case A_____:	/* do nothing */
		break;
	    case A_ELSE:	/* invert last condition */
		execute = ! env->last_condition;
		break;
	    case A_IF:		/* clear last condition */
		env->last_condition = FALSE;
		break;
	    case A_FUNC:	/* store function key reference */
		last_function = token;
		break;
	    case A_LOAD:	/* load file */
		push_env ();
		load (yytext, TRUE);
		pop_env ();
		break;
	    case A_TERM:	/* match terminal type */
		env->last_condition |= ! strcmp (termtype, yytext);
		break;
	    case A_PRNT:	/* print on terminal */
		if (action & F_S2I)
			fputs(yytext, stderr);
		else
			fputs (ident_to_string(yytext), stderr);
		break;
	    case A_PRTN:	/* print newline */
		putc ('\n', stderr);
		break;
	    case A_BIND:	/* bind string to saved function key */
		bindtoken (last_function, yytext);
		break;
	    case A_COND:	/* set conditional execution flag */
		execute = env->last_condition;
		break;
	    case A_E001:	/* ERROR - misplaced identifier ignored */
		error (MSGSTR(MIS_IDENTIFIER,
			"misplaced identifier \"%s\" ignored\n"), yytext); /*MSG*/
		break;
	    case A_E002:	/* ERROR - misplaced string ignored */
		error (MSGSTR(MISP_STRING,
			"misplaced string <%s> ignored\n"), yytext); /*MSG*/
		break;
	    case A_E003:	/* ERROR - misplaced OR ignored */
		error (MSGSTR(MISP_OR, "misplaced OR ignored\n")); /*MSG*/
		break;
	    case A_E004:	/* ERROR - misplaced function key reference */
		error (MSGSTR(MISP_REF,
			"misplaced reference to %s ignored\n"), keyname(token)); /*MSG*/
		break;
	    case A_E005:	/* ERROR - incomplete BIND command */
		error (MSGSTR(INC_BIND, "incomplete BIND command\n")); /*MSG*/
		break;
	    case A_E006:	/* ERROR - missing file name */
		error (MSGSTR(MISS_FILE_NAME, "missing file name\n")); /*MSG*/
		break;
	    case A_E007:	/* ERROR - incomplete IF statement */
		error (MSGSTR(INC_IF, "incomplete IF statement\n")); /*MSG*/
		break;
	    case A_E008:	/* ERROR - incomplete ELSE statement */
		error (MSGSTR(EOF_IN_ELSE, "end of file in ELSE statement\n")); /*MSG*/
		break;
	    default:
		error (MSGSTR(ILL_ACT, "*** ILLEGAL ACTION %08x ***\n"), action); /*MSG*/
		exit (1);
	}
    }
}

/* tokenclass maps TOKENTYPE -> TOKENCLASS */

static int tokenclass (token)
int	token;
{
    return token == 0 ? C_EOF : token > T_QUIT ? C_FUNC : token - T_IDENT;
}

/* input routine for yylex() */

int input ()
{
    int		ch;

    if ((ch = getc (env->infile)) == '\n')
	env->linenumber++;
    return ch == EOF ? 0 : ch;
}
int unput (ch)
int	ch;
{
    if (ch == '\n')
	env->linenumber--;
    return ungetc (ch, env->infile);
}

/* error() prints somewhat reasonable syntax error messages */
/*VARARGS*/
static void error (msg, arg1)
char	*msg;
int	arg1;
{
	char	*t;

	t = (char *)malloc(strlen(msg) + 1);	/* malloc space to store passed
						   message, since it probably
						   is in the catalog buffer */
	if (t == NULL) {		/* if malloc() failed, print in default
					   format.  Better default format than
					   losing the message. */
	    fprintf(stderr, "\"%s\", line %d: ", env->filename,env->linenumber);
	    return;
	}
		
	strcpy(t, msg);				/* copy message to space
						   returned by malloc */
	msg = t;				/* reset msg to point to
						   buffered message */

    fprintf (stderr, MSGSTR(ERROR_HEAD, "\"%s\", line %d: "),
	env->filename, env->linenumber); /*MSG*/
    fprintf (stderr, msg, arg1);

    free(t);
}

/* bind a string to a function, creating/deleting keymaps as necessary */

void bindtoken (token, string)
int	token;
char	*string;
{
    register char		*sp = string;
    register struct keymap	*kp = firstmap;
    static int	(*keyfunctions[])() = {
	fs_character, fs_insertmode, fs_delete, fs_backspace, fs_tab,
	fs_backtab, fs_return, fs_up, fs_down, fs_left, fs_right, fs_home,
	fs_eraseinput, fs_eraseeof, fs_dup, fs_fieldmark, fs_reset,
	fs_clear, fs_enter, fs_pa1, fs_pa2, fs_pa3, fs_penselect,
	fs_pf1, fs_pf2, fs_pf3, fs_pf4, fs_pf5, fs_pf6, fs_pf7, fs_pf8,
	fs_pf9, fs_pf10, fs_pf11, fs_pf12, fs_pf13, fs_pf14, fs_pf15,
	fs_pf16, fs_pf17, fs_pf18, fs_pf19, fs_pf20, fs_pf21, fs_pf22,
	fs_pf23, fs_pf24, fs_sysreq, fs_wordnext, fs_wordprev, fs_attention,
#ifdef BIDI
        fs_autopush, fs_scrrev, fs_eng_lang, fs_national_lang, fs_push,
        fs_endpush, fs_fldrev, fs_autorev,
#endif /* BIDI */
#ifdef ARAB
        fs_autoshape, fs_isolated, fs_initial, fs_middle, fs_final, 
        fs_fieldshape, fs_fieldbase
#endif /* ARAB */

    };
/*	fprintf(stderr,"token = %d\n",token);
	fprintf(stderr,"text = %s\n",string); */
/* check whether bind is for colors */
switch(token) {
	case T_LOWPROT :
			bindcolor(&lo_prot_color, string, FOREGROUND);
			return;
	case T_LOWUNPROT :
			bindcolor(&lo_unprot_color, string, FOREGROUND);
			return;
	case T_HIGHPROT :
			bindcolor(&hi_prot_color, string, FOREGROUND);
			return;
	case T_HIGHUNPROT :
			bindcolor(&hi_unprot_color, string, FOREGROUND);
			return;
	case T_BACKGROUND :
			bindcolor(&background_color, string, BACKGROUND);
			return;
}

    while (*sp) {
	if (*(sp+1) == '\0') {
	    if (*sp == '\033') {
		error(MSGSTR(ESC_NOT_VAL, "%s - \\e valid only in sequences : %s aborting ...\n") 			                        , keyname(token)); /*MSG*/
		exit(1);
		break;
	    }
	    if (IS_MAPPTR (kp->entry[*sp])) {
		free_keymap (kp->entry[*sp].nextmap);
                kp->entry[*sp].tag = 0;
            }
	    kp->entry[*sp].function =
		(token < T_CHARACTER || token >= T_ILLEGAL) ?
			(int (*)())NULL : keyfunctions[token - T_CHARACTER];
	}
	else {
	    if (! IS_MAPPTR (kp->entry[*sp])) {
		kp->entry[*sp].nextmap = new_keymap ();
#ifdef AIX
		kp->entry[*sp].tag = 1;
#endif /* AIX */
	    }
	    kp = kp->entry[*sp].nextmap;
	}
	++sp;
    }
}

/* associate display attribute with color */
static	int bindcolor (int *color_attr, char *string, int fore_or_back)
{
	int	curses_attr;		/* curses color display attribute */
	
	if (fore_or_back == FOREGROUND)
		curses_attr = getcolor(FOREGROUND, string);
	else
		curses_attr = getcolor(BACKGROUND, string);
	if (curses_attr != -1)
		*color_attr = curses_attr;
	else
		error(MSGSTR(INV_DSP_COLOR, "%s -  invalid display color : \n"), string); /*MSG*/
}

/* associate curses display attribute with color string */
/* returns curses color value suitable for use with curses colorout() */
static	int getcolor (int fore_or_back, char *string)
{
	if (fore_or_back == FOREGROUND) {
		if (!strcmp(string, S_BLUE))
			return(F_BLUE);
		if (!strcmp(string, S_RED))
			return(F_RED);
		if (!strcmp(string, S_GREEN))
			return(F_GREEN);
		if (!strcmp(string, S_WHITE))
			return(F_WHITE);
		if (!strcmp(string, S_MAGENTA))
			return(F_MAGENTA);
		if (!strcmp(string, S_CYAN))
			return(F_CYAN);
		if (!strcmp(string, S_BLACK))
			return(F_BLACK);
		return(-1);		/* not a supported color */
	}
	else {
		if (!strcmp(string, S_BLACK))
			return(B_BLACK);
		if (!strcmp(string, S_BLUE))
			return(B_BLUE);
		if (!strcmp(string, S_RED))
			return(B_RED);
		if (!strcmp(string, S_GREEN))
			return(B_GREEN);
		if (!strcmp(string, S_WHITE))
			return(B_WHITE);
		if (!strcmp(string, S_MAGENTA))
			return(B_MAGENTA);
		if (!strcmp(string, S_CYAN))
			return(B_CYAN);
		return(-1);		/* not a supported color */
	}
}




/* allocate a new keymap and return a pointer to it */

static struct keymap *new_keymap ()
{
    register struct keymap	*kp;

    if ((kp = (struct keymap *)malloc (sizeof (struct keymap))) ==
	(struct keymap *)NULL)
    {
	perror (MSGSTR(P_MALLOC_KEYMAP, "can't allocate keymap")); /*MSG*/
	exit (1);
    }

    bzero ((char *)kp, sizeof (struct keymap));

    return kp;
}

/* delete a keymap subtree */

static void free_keymap (kp)
register struct keymap	*kp;
{
    register int	i;

    for (i = 0; i < 256; i++) {
	if (IS_MAPPTR (kp->entry[i])) {
	    free_keymap (kp->entry[i].nextmap);
#ifdef AIX
	    kp->entry[i].tag = 0;
#endif /* AIX */
	}
    }
    /* BUG!! BUG!! BUG!! */
    free(kp);
}

static void push_env ()
{
    if (env + 1 >= &envstack[NESTING_DEPTH]) {
	error (MSGSTR(NESTED_FILES, "files nested too deeply (> %d)\n"), NESTING_DEPTH); /*MSG*/
	exit (1);
    }

    env++;
}

static void pop_env ()
{
    if (env > envstack)
	--env;
}

static void string_to_ident (string)
char	*string;
{
    register char	*sp = string;
    register char	*np = string;

    while (*sp) {
	switch (*sp) {
	    case '^':
		sp++;
		/* Suport for mapping the ASCII del key */
		if ( *sp == '?' ) {
                        *np++ = *sp | 0x40;
                        *sp++;
                }
		else
		   *np++ = *sp++ & 037;
		break;
	    case '~':
		sp++;
		*np++ = *sp++ + 0200;
		break;
	    case '"':
		sp++;
		break;
	    case '\\':
		switch (*++sp) {
		    case 'b':
			*np++ = '\b';
			break;
		    case 'e':
			*np++ = '\033';
			break;
		    case 's':
			*np++ = ' ';
			break;
		    case 't':
			*np++ = '\t';
			break;
		    case 'n':
			*np++ = '\n';
			break;
		    case 'r':
			*np++ = '\r';
			break;
		    default:
			*np++ = *sp;
			break;
		}
		sp++;
		break;
	    default:
		*np++ = *sp++;
		break;
	}
    }
    *np = '\0';
}

static char	*keyname (token)
int	token;
{
    static char	*names[] = {
	"CHARACTER", "INSERTMODE", "DELETE", "BACKSPACE", "TAB",
	"BACKTAB", "RETURN", "UP", "DOWN", "LEFT", "RIGHT", "HOME",
	"ERASEINPUT", "ERASEEOF", "DUP", "FIELDMARK", "RESET",
	"CLEAR", "ENTER", "PA1", "PA2", "PA3", "PENSELECT", "PF1",
	"PF2", "PF3", "PF4", "PF5", "PF6", "PF7", "PF8", "PF9",
	"PF10", "PF11", "PF12", "PF13", "PF14", "PF15", "PF16",
	"PF17", "PF18", "PF19", "PF20", "PF21", "PF22", "PF23",
	"PF24", "SYSREQ", "WORDNEXT", "WORDPREV", "ATTENTION", 
#ifdef BIDI
        "AUTOPUSH", "SCRREV", "ENG_LANG", "NLS_LANG", "PUSH",   
        "ENDPUSH", "FLDREV", "AUTOREV",
#endif /* BIDI */
#ifdef ARAB
        "AUTOSHAPE", "ISOLATED", "INITIAL", "MIDDLE", "FINAL",
        "FIELDSHAPE", "FIELDBASE",
#endif /* ARAB */
        "ILLEGAL"
    };

    return token < T_CHARACTER ? "?" : names[token - T_CHARACTER];
}

/* process keyboard input according to the keymaps constructed above */

void fs_input (ch)
register int	ch;
{
#ifdef AIX
    /* oops! the define messes this up ... */
    register int	(*entfunction)();
#else
    register int	(*function)();
#endif /* AIX */

#ifdef AIX
    ch &= 0xff;
#endif /* AIX */
    if (IS_MAPPTR (thismap->entry[ch]))
	thismap = thismap->entry[ch].nextmap;
    else {
	if (thismap->entry[ch].nextmap == (struct keymap *)NULL)
	    fs_alarm ();
	else {
#ifdef AIX
	    entfunction = thismap->entry[ch].function;
#else
	    function = thismap->entry[ch].function;
#endif /* AIX */
#ifdef AIX
	    if (fs_keyboard_locked && entfunction != fs_reset
		&& entfunction != fs_attention)
#else
	    if (fs_keyboard_locked && function != fs_reset
		&& function != fs_attention)
#endif
		fs_alarm ();
	    else
#ifdef AIX
            {
		(void)(*entfunction) (ch);
#ifdef BIDI 
                if (push_mode)
                   fs_check_to_end_push(*entfunction);
#endif /* BIDI */
            }
#else
		(void)(*function) (ch);
#endif
	}
	thismap = firstmap;
	fs_redraw ();
    }
}

/************************************************************************/
/* Function Name: ident_to_string					*/
/* Purpose: to take a msg identifier and convert it into either the	*/
/*	    appropriate catalog string -or- the default string.    	*/
/* NOTE: all messages to be printed in the 3270 keys file (via the      */
/*       parser must be defined here.					*/
/************************************************************************/

static char *ident_to_string(sp)
char *sp;
{
	if (!strcmp(sp, "NOBINDINGS")) 
		return(MSGSTR(P_NOBIND, "No 3270 key bindings for this terminal type - using defaults.\n"));
	return(MSGSTR(P_UNKNOWNMSG, "Unknown message id from input file\n"));
}
