static char sccsid[] = "@(#)01	1.10  src/tenplus/helpers/print/print.c, tenplus, tenplus411, GOLD410 3/3/94 16:21:48";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	Hinit, PTgetdir, PTgetstate, PTputstate, PTmenu,
 *		PTparsecmd, PTgettype, PTgetcmd, PTgetfile, PTckfile,
 *		PTfixvars, PTskipws, PTreadform, PTbasename, PTopen,
 *		PTclose, PTctu, PTsetfds, PTresetfds, PTrderr,
 *		PTprint, PTstdprint, PTprtit, PTrmtrlblks,
 *		PTformprint, PTwindows, PTlastline, PTemptyall,
 *		PTgetboxes, PTinvtext, PTgut, PTstuff, PTsystem, Hstop
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *                      Copyrighted as an unpublished work.                 
 *                         INTERACTIVE TEN/PLUS System                      
 *                            TEN/PLUS Print Helper                         
 *              (c) Copyright INTERACTIVE Systems Corp. 1983, YeAr          
 *                             All rights reserved.                         
 *                                                                          
 *   RESTRICTED RIGHTS                                                      
 *   These programs are supplied under a license.  They may be used and/or  
 *   copied only as permitted under such license agreement.  Any copy must  
 *   contain the above notice and this restricted rights notice.  Use,      
 *   copying, and/or disclosure of the programs is strictly prohibited      
 *   unless otherwise provided in the license agreement.                    
 *                                                                          
 *   TEN/PLUS is a registered trademark of INTERACTIVE Systems Corporation  
 *	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
 */
/****************************************************************************/
/* File: print.c - TENPLUS print helper                                     */
/****************************************************************************/

#if !defined(lint)
static char copyright[] =
	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
#endif /* lint */

#include    <stdio.h>
#include    <string.h>
#include    <signal.h>
#include    <ctype.h>
#include    <fcntl.h>
#include    <unistd.h>
#include    <stdlib.h>
#include    <sys/wait.h>
#include    <limits.h>

#include    "chardefs.h"
#include    "database.h"
#include    "edexterns.h"
#include    "window.h"
#include    "keynames.h"
#include    "print.h"
#include    "libobj.h"
#include    "libprf.h"
#include    "libutil.h"
#include    "libshort.h"
#include    "libhelp.h"
#include    "INeditor_msg.h"
#include    "INprint_msg.h"
nl_catd	     g_printcatd;

#define DEVTTY "/dev/tty"
#define SIZE     (250 * MB_LEN_MAX)

#define CMD      0  /* menu option is a command run via PTsystem()*/
#define PIPE     1  /* menu option is a pseudo pipe line run      */
		    /* via PTsystem() with stdin redirected       */
#define OFILE    2  /* menu option is overwrite a file            */
#define AFILE    3  /* menu option is append to a file            */

char     g_tv [1];  /* fake TV array         */
static int gr_out;  /* printing shouldn't translate graphics to ascii */

struct box {        /* structure gleaned from the window            */
    int start;      /* virtual line on which this window starts     */
    int end;        /* virtual line on which this window ends       */
    int curidx;     /* current readline-line #                      */
    int limit;      /* datalength () on this window                 */
    };

struct stinfo {     /* structure of print state information       */
    char *command;  /* last default command                       */
    char *pipe;     /* last default pipe command                  */
    char *ofile;    /* last default overwrite filename            */
    char *afile;    /* last default append filename               */
    int   lsel;     /* last selected printer                      */
    };

struct ptinfo {     /* structure of print information             */
    int   fd[3];    /* file descriptors for stdin, stdout, stderr */
    int   cmdtype;  /* type of command line                       */
    int   clear;    /* if TRUE clear screen and run command       */
    int   save;     /* if TRUE save all ascii files               */
    int   allout;   /* if TRUE display all command output         */
    int   gr_out;   /* if TRUE dont translate graphic characters  */
    char  *cmd;     /* command to perform                         */
    char  *popdesc; /* description of command to perform          */
    char  *printfile;    /* name of file to print on                   */
    char  *formname;     /* name of form to use                        */
    char  *errout;       /* file to use to hold stdout/stderr          */
    FILE  *fp;           /* file pointer                               */
    char  *curdir;       /* current UNIX directory                     */
    char  *statefile;    /* name of the state file $HOME/.pstate       */
    POINTER  *recstate;  /* the record with the state info             */
    WSTRUCT *warray;     /* the window array of the form               */
    struct stinfo st;    /* the last state of things                   */
    };

#ifdef VBLSCR
    int offset;         /* offset of upper left hand corner of the form  */
    int TVheight;       /* height of invariant text */
#endif

#ifdef VBLSCR
void PTwindows  (int , ATTR *, ATTR *, struct box *, struct ptinfo *, int );
#else
void PTwindows  (int , ATTR *, ATTR *, struct box *, struct ptinfo *);
#endif

int PTgetfile   (char *, struct ptinfo *);
void PTemptyall (int, ATTR *, struct box *, struct ptinfo *, int );
void PTgetboxes (struct box *, struct ptinfo *);
void PTgetstate (struct ptinfo *);
void PTputstate (struct ptinfo *);
int PTmenu      (struct ptinfo *);
int PTparsecmd  (char *, struct ptinfo * );
char *PTgettype (char *, struct ptinfo *);
int PTgetcmd    (struct ptinfo *);
int PTgetcmd    (struct ptinfo *);
int PTckfile    (char *, struct ptinfo *);
char *PTfixvars (char *, struct ptinfo *);
int PTreadform  (struct ptinfo *);
int PTopen      (struct ptinfo *);
int PTclose     (struct ptinfo *);
void PTprint    (struct ptinfo *);
void PTformprint(struct ptinfo *);
void PTlastline (int , int , ATTR *, ATTR *, struct box *, struct ptinfo *);

/****************************************************************************/
/* Hinit: The editor starts up the print helper and calls Hinit.            */
/*        Hinit handles the print request and then returns.                 */
/*        The editor kills the print helper and restarts the helper         */
/*        that was running prior the print request.                         */
/****************************************************************************/

int Hinit (char *filename)
    {
    static int first_time = TRUE;
    static struct ptinfo pts;
    struct ptinfo *pt;

    pt = &pts; /* initialize pointer to ptinfo structure */
    if (first_time)
	{
	first_time = FALSE;

	/* this guarentees that stderr is set to /dev/null */
	(void)close(2);
	open("/dev/null", O_WRONLY);

#ifdef  DEBUG
	g_debugfp = fopen ("print.trc", "w");
	g_setflag(G_COREDUMP);
#endif

	wn_init ();                        /* set up structured allocator   */
	pt->formname = s_string("");       /* init lastform to empty string */
	pt->warray   = NULL;               /* init warray to null           */
	}

    /* PTgetdir() should be replaced by Efixvars("$CURDIR") */
    /* when it is implemented in the editor                 */

    pt->curdir = PTgetdir(filename);
    if (chdir (pt->curdir) == ERROR)
       {
       Eerror (P_CURDIR,
		 "Couldn't change to current directory \"%s\"", pt->curdir);
       (void)s_free (pt->curdir);
       return (ERROR);
       }

    if(Pinit("print") == ERROR)
	{
	Eerror(P_NOPROFILE, "Couldn't find the print helper profile");
	(void)s_free (pt->curdir);
	return(ERROR);
	}

    g_printcatd = catopen(MF_INEDITOR, NL_CAT_LOCALE);
    PTgetstate(pt);

    pt->cmd = NULL;  /* init command string */

    if (PTmenu (pt) == ERROR)  /* get the request */
	{
	PTputstate(pt);
	(void)s_free (pt->curdir);
	Pcleanup ();
	return(ERROR);
	}

    Pcleanup ();
    if(PTreadform (pt) == ERROR)  /* read the form */
	{
	PTputstate(pt);
	(void)s_free (pt->curdir);
	return(ERROR);
	}

    if(PTopen (pt) == ERROR)  /* open files for printing */
	{
	PTputstate(pt);
	(void)s_free (pt->curdir);
	return(ERROR);
	}

    PTprint (pt);   /* print the file through the form */
    PTclose (pt);   /* close files, run command, display output if any */
    PTputstate(pt); /* save the state info */
    (void)s_free (pt->curdir);

    return(ERROR);  /* tell the editor that we never restart */
    }

/****************************************************************************/
/* PTgetdir: given full filename returns directory file is in               */
/*           dir is a structured allocation and must be freed               */
/****************************************************************************/


char *PTgetdir(char *filename)

    {
    char *dir_end, *dir;

    dir = s_string(filename);
    dir_end = strrchr(dir, DIR_SEPARATOR);
    if(dir == dir_end)
	*(dir_end + 1) = 0;
    else
	*dir_end = 0;
    dir = s_realloc(dir, strlen(dir));
    return(dir);
    }

/****************************************************************************/
/* PTgetstate: read the pstate file to get the last print state info        */
/*             last default command                                         */
/*             last default pipe command                                    */
/*             last default filename to append to                           */
/*             last default filename to overwrite                           */
/*             last selected menu item number                               */
/****************************************************************************/

void PTgetstate(struct ptinfo *pt)

    {
    FILE *sfp;

    pt->statefile = Efixvars("$HOME/.pstate");
    sfp = fopen (pt->statefile, "r");
    if (sfp == NULL)  /* if no file set defaults */
	{
	 defaults:
	 pt->st.command = s_string("qprt -c -T $FILE $PRTFILE");
	 pt->st.pipe =  s_string("qprt -T $FILE -");
	 pt->st.ofile = s_string("print.tmp");
	 pt->st.afile = s_string("print.tmp");
	 pt->st.lsel =  0;
	 return;
	}

    pt->recstate = (POINTER *) s_read (sfp);
    if(pt->recstate == (POINTER *) ERROR)
	{
	(void)fclose (sfp);
	goto defaults;
	}

    pt->st.command = s_string(pt->recstate[0]);
    pt->st.pipe = s_string(pt->recstate[1]);
    pt->st.ofile = s_string(pt->recstate[2]);
    pt->st.afile = s_string(pt->recstate[3]);
    pt->st.lsel =  atoi(pt->recstate[4]);

    (void)fclose (sfp);
    (void)s_free((char *)pt->recstate);
    }

/****************************************************************************/
/* PTputstate: save the current print state in the file $HOME/.pstate       */
/****************************************************************************/

void PTputstate(struct ptinfo *pt)

    {
    FILE *sfp;
    char buf[10];
    POINTER *record;

    sfp = fopen (pt->statefile, "w");
    if (sfp == NULL)
       {
       Eerror(P_STATEFILE,
		"Can't create print statefile \"%s\"", pt->statefile);
       (void)s_free(pt->statefile);
       return;
       }

    record = (POINTER *) s_alloc(5, T_POINTER, (char *) NULL);
    record[0] = pt->st.command;
    record[1] = pt->st.pipe;
    record[2] = pt->st.ofile;
    record[3] = pt->st.afile;
    (void) sprintf(buf, "%d", pt->st.lsel);
    record[4] = s_string(buf);

    (void)s_write ((char *)record, sfp);
    (void)fclose(sfp);
    (void) chmod(pt->statefile, 0600);
    (void)s_free((char *)record);
    (void)s_free((char *)pt->statefile);
    }

/****************************************************************************/
/* PTmenu: Read the profile, display the menu, get the users selection,     */
/*         and set up the ptinfo structure                                  */
/****************************************************************************/

int PTmenu(struct ptinfo *pt)

    {
    /* Define the default print menu header */
    char *header = "      Print Helper\n\
Move the cursor and press:\n\
EXECUTE to select,\n\
CANCEL to leave the menu, or\n\
HELP for more information.\n#0";
    POINTER *prfdesc = NULL;  /* profile description field */
    POINTER *prfcmd = NULL;   /* profile command field */
    POINTER *prfpop = NULL;   /* profile popdesc field */
    char *cp1;
    char *cp2;
    char *cp3;
    POINTER *tp;
    int retval;               /* return value for menu display */
    char datapath[PATH_MAX];  /* buffer to build datapath name */
    int i;

    prfdesc = Pgetmulti("menu/*/desc");
    if (prfdesc == (POINTER *) ERROR || obj_count(prfdesc) == 0)
	{
	Eerror (P_NOMENU, "No menu of print options");
	return(ERROR);
	}

    /* make sure that the last selected item isn't
       greater than the current number of items    */

    prfdesc = catscan(prfdesc, g_printcatd, MS_PROFILES);

    if(pt->st.lsel > obj_count(prfdesc))
	pt->st.lsel = 0;

    retval = Ecustommenu(MN_P_MENU, header, prfdesc, pt->st.lsel);

    (void)s_free ((char *)prfdesc);

    if(retval == ERROR)
       return(ERROR);

    pt->st.lsel = retval; /* remember this selection */

    prfcmd = Pgetmulti ("menu/*/cmd");

    if (prfcmd == (POINTER *) ERROR || retval > obj_count(prfcmd)) 
	cp1 = s_string("");
    else
	{
        prfcmd = catscan(prfcmd, g_printcatd, MS_PROFILES);
	cp1 = s_string(prfcmd[retval]);
	}

    (void)s_free ((char *)prfcmd);

    cp2 = PTfixcommand (cp1);  /* get answers to prompts _F_i_l_e_ _n_a_m_e_: */
    (void)s_free (cp1);

    if(cp2 == (char *) ERROR) /* see if they hit cancel */
	return(ERROR);


    cp1 = Efixvars(cp2);      /* expand environment variables $HOME etc. */
    (void)s_free (cp2);

    if (PTparsecmd(cp1, pt) == ERROR)
	{
	(void)s_free (cp1);
	return(ERROR);
	}

    (void)s_free (cp1);

    /* get the clear, save and dispcmd flags */

    (void) sprintf(datapath, "menu/%d/clear", retval);
    pt->clear = Pcheckbox(datapath);

    (void) sprintf(datapath, "menu/%d/save", retval);
    pt->save = Pcheckbox(datapath);

    (void) sprintf(datapath, "menu/%d/allout", retval);
    pt->allout = Pcheckbox(datapath);

    (void) sprintf(datapath, "menu/%d/gr_out", retval);
    pt->gr_out = Pcheckbox(datapath);

    prfpop = Pgetmulti ("menu/*/popdesc");

    if (prfpop == (POINTER *) ERROR || retval > obj_count(prfpop)) 
	cp3 = s_string("");
    else
	{
        prfpop = catscan(prfpop, g_printcatd, MS_PROFILES);
	cp3 = s_string(prfpop[retval]);
	}

    (void)s_free ((char *)prfpop);

    if (cp3 == (char *) ERROR)
	cp3 = s_string("");

    cp1 = Efixvars(cp3);      /* expand environment variables $HOME etc. */
    (void)s_free (cp3);

    pt->popdesc = PTfixvars(cp1, pt); /* expand $PRTFILE and $PRTCMD */
    if(*(pt->popdesc) == '\0')
	{
	(void)s_free(pt->popdesc);
	pt->popdesc = s_string("Printing");
	}

    (void)s_free (cp1);

    return(RET_OK);
    }

/****************************************************************************/
/* PTparsecmd: parse the command string                                     */
/****************************************************************************/

int PTparsecmd (char *string, struct ptinfo * pt)

    {
    int rv;

    string = PTgettype(string, pt);
    if(pt->cmdtype == OFILE || pt->cmdtype == AFILE)
       {
	if (PTgetfile(string, pt) == ERROR)
	   return(ERROR);
       }
    else
       {
       pt->printfile = s_string (uniqname (TRUE));
       pt->cmd = PTfixvars(string, pt);

       if (pt->cmd == NULL || *(pt->cmd) == '\0')
	   {
	   s_free(pt->cmd);
	   rv = PTgetcmd(pt);
	   if (rv == ERROR)
	       (void)s_free(pt->printfile);
	   return(rv);
	   }
       }

    return(RET_OK);
    }

/****************************************************************************/
/* PTgettype: parse the string to get the type of command                   */
/****************************************************************************/

char *PTgettype(char *string, struct ptinfo *pt)
    {

    string = PTskipws(string);
    if(*string == '|') /* this is a pipe line command */
	{
	pt->cmdtype = PIPE;
	string++;
	}
    else if(*string == '>') /* this is a file command */
	{
	if(*(++string) == '>')
	    {
	    pt->cmdtype = AFILE;
	    string++;
	    }
	else
	    pt->cmdtype = OFILE;
	}
    else
	pt->cmdtype = CMD;

    return(string);
    }

/****************************************************************************/
/* PTgetcmd: if command is not specified ask for command                    */
/*           return ERROR if canceled or no command specified else return   */
/*           RET_OK                                                         */
/****************************************************************************/

int PTgetcmd (struct ptinfo *pt)
    {
    char *cp;
    char *answer;
    char *cmdname;

    answer = Eask (P_CMD, "Command to run (\"%s\"):              ",
		      (pt->cmdtype == CMD ? pt->st.command : pt->st.pipe));

    if (answer == (char *) ERROR) /* watch for CANCEL */
	return(ERROR);

    cp = PTskipws(answer);

    /* use default command */
    if (cp == NULL || *cp == '\0')
      cmdname = s_string(pt->cmdtype == CMD ? pt->st.command : pt->st.pipe);

    /* if user specified a command */
    else
      cmdname = s_string(cp);

    (void)s_free(answer);

    /* free the last command */
    (void)s_free(pt->cmdtype == CMD ? pt->st.command : pt->st.pipe);
    if(pt->cmdtype == CMD)
	pt->st.command = s_string(cmdname);
    else
	pt->st.pipe = s_string(cmdname);

    cp = Efixvars(cmdname);
    (void)s_free(cmdname);
    pt->cmd = PTfixvars(cp, pt);
    (void)s_free(cp);

    if (pt->cmd == NULL || *pt->cmd == '\0')
	{
	(void)s_free(pt->cmd);
	Eerror(P_NOCMD, "No command specified");
	return(ERROR);
	}

    return (RET_OK);
    }

/****************************************************************************/
/* PTgetfile: get the file name from the command string                     */
/*            if no file name ask user using default file name              */
/*            confirm creation of filename                                  */
/*            return ERROR or RET_OK                                        */
/****************************************************************************/

int PTgetfile (char *string, struct ptinfo *pt)
    {
    char *file;
    char *answer;
    char *cp;
    char *fullname;
    int askflag = FALSE;

    string = PTskipws(string);
    if (string == NULL || *string == '\0')
	{
	askflag = TRUE;

	answer = Eask (P_FILE,
		       "File for printing (\"%s\"):              ",
		      (pt->cmdtype == AFILE ? pt->st.afile : pt->st.ofile));

	if (answer == (char *) ERROR) /* watch for CANCEL */
	    return(ERROR);

	file = s_string(PTskipws(answer));
	if (file == NULL || *file == '\0') /* use default file */
	    {
	    (void)s_free(file);
	    file =
	      s_string(pt->cmdtype == AFILE ? pt->st.afile : pt->st.ofile);
	    }

	cp = Efixvars(file);
	fullname = abspath (cp, pt->curdir);
	(void)s_free(cp);
	(void)s_free(answer);
	}
    else /* if there is a filename on command line */
	{
	 cp = Efixvars(string);
	 fullname = abspath (cp, pt->curdir);
	 (void)s_free(cp);
	}

    if(PTckfile(fullname, pt) == ERROR)
	{
	(void)s_free(fullname);
	return(ERROR);
	}

    pt->printfile = fullname;
    if(askflag)
	{
	(void) s_free(pt->cmdtype == AFILE ? pt->st.afile : pt->st.ofile);
	if(pt->cmdtype == AFILE)
	    pt->st.afile = s_string(file);
	else if(pt->cmdtype == OFILE)
	    pt->st.ofile = s_string(file);
	(void)s_free (file);
	}

    return (RET_OK);
    }

/****************************************************************************/
/* PTckfile: check to see if file exists put up creation/clobber menu       */
/****************************************************************************/

int PTckfile(char *fname, struct ptinfo *pt)
    {
    char *filename;

    if (access (fname, 0) == -1) /* File does not exist */
	{
	if (Econfirm (P_MAKEFILE,
"The file \"%s\" does not exist.\nTouch EXECUTE to create it.\nTouch CANCEL to return to editing, HELP for help.",
	    fname) == FALSE)
	    return(ERROR);
	}
    else if(pt->cmdtype == OFILE || pt->cmdtype == AFILE) /* if file exists */
	{
	filename = Efilename ();
	if (seq (fname, filename))
	    {
	    (void) s_free(filename);
	    if (pt->cmdtype == OFILE)
		Eerror (P_OVERWRFILE,
		    "Do not overwrite the file you are currently editing");
	    else
		Eerror (P_APPFILE,
		    "Do not append to the file you are currently editing");
	    return(ERROR);
	    }
	(void) s_free(filename);
	if(pt->cmdtype == OFILE)
	    {
	    if (Econfirm (P_CLOBBERFILE,
"The file \"%s\" already exists.\nTouch EXECUTE to overwrite it.\nTouch CANCEL to return to editing, HELP for help.",
	    fname) == FALSE)
	    return(ERROR);
	    }

	if(pt->cmdtype == AFILE)
	    {
	    if(isdelta(fname))
	       {
	       Eerror (P_ISDELTA, "You cannot append print output to a structured file");
	       return(ERROR);
	       }
	    }
	}

    return(RET_OK);
    }

/****************************************************************************/
/* PTfixvars: fix $PRTFILE and $PRTCMD in the cmd string                    */
/****************************************************************************/

char *PTfixvars (char *string, struct ptinfo *pt)

    {
    char  buffer [200 * MB_LEN_MAX];  /* Place to build the result */
    char *bp;                         /* Pointer into buffer       */

    bp = buffer;
    string = PTskipws(string); /* skip over initial whitespace */
    while (*string != '\0') {

	/* Copy anything that doesn't begin with a $ */
	if (*string != '$') {
	    *bp++ = *string++;
	    continue;
	}

	/* *bp must be '$'; pick off the succeeding variable */
	string++;

        /* We're only interested in $PRTFILE and $PRTCMD. So
           only check for those two.
        */
     
	if ((strncmp(string, "PRTFILE", 7) == 0) && 
                      /* 7 is length of PRTFILE */
            (*(string + 7) != '_') &&
            !ismbalnum(string + 7) )  {
           
	     (void) strcpy(bp, pt->printfile);
             bp += strlen(bp);
             string += 7; /* let string point at char after PRTFILE */
        }

	else if ((strncmp(string, "PRTCMD", 6) == 0) && 
                       /* 6 is length of PRTCMD */
                 (pt->cmd != NULL) &&
                 (*(string + 6) != '_') &&
                 !ismbalnum(string + 6) )  {

                    (void) strcpy(bp, pt->cmd);
                    bp += strlen(bp);
                    string += 6; /* let string point at char after PRTCMD */
              }
              else
	          /* If value cannot be expanded, put in the literal 
                     string $variable */
	          *bp++ = '$';

    }

    *bp = '\0';
    return (s_string (buffer));
    }

/****************************************************************************/
/* PTskipws: skip white space                                               */
/****************************************************************************/

char *PTskipws(char *string)

    {
    skipmbspaces(&string);
    return(string);
    }

/****************************************************************************/
/* PTreadform: read the current form for use in printing                    */
/****************************************************************************/

int PTreadform(struct ptinfo *pt)
    {
    char *absform;  /* the full pathname of the form */
    char *form;     /* the basename of the absform   */
    SFILE *sfp;

    absform = Efixvars("$ABSFORM");
    if (seq(absform, "$ABSFORM") || *absform == '\0')
	{
	s_free(absform);
	Eerror (P_ABSFORM, "Can't get the name of the current form");
	return(ERROR);
	}

    if (seq (absform, "*win*"))
	{
	s_free(absform);
	Eerror (P_MULTIWIN, "Printing is unimplemented in multiple-windows");
	return(ERROR);
	}

    if(seq(absform, pt->formname))
       {
       s_free(absform);
       return(RET_OK);
       }

    /* cache one form */

    s_free(pt->formname);      /* free old formname   */
    pt->formname = absform;    /* set to new formname */

    /* the standard form and the c and h forms aren't printed */
    /* so there is no need to read them */

    if (seq ((form = PTbasename(pt->formname)), "std.ofm") ||
	seq (form, "c.ofm") ||
	seq (form, "h.ofm"))
	return(RET_OK);

    sfp = sf_open (pt->formname, "r");

    if (sfp == (SFILE *) ERROR)
	{
	Eerror (P_NOFORM, "Couldn't open form \"%s\"", pt->formname);
	return(ERROR);
	}

    s_free((char *)pt->warray);
    pt->warray = (WSTRUCT *) sf_read (sfp, 0);

    if (pt->warray == (WSTRUCT *) ERROR)
	{
	Eerror (P_BADFORM, "Error reading form \"%s\"", pt->formname);
	pt->warray = NULL;
	s_free(pt->formname);
	pt->formname = s_string("");
	return(ERROR);
	}

    (void) sf_close (sfp);
    return(RET_OK);
    }

/****************************************************************************/
/* PTbasename: return basename of name /dir/basename                        */
/****************************************************************************/

char *PTbasename (char *name)
    {
    char *cp;

    cp = name;
    while (*cp)
	cp++;

    while (cp >= name && *cp != DIR_SEPARATOR)
	cp--;

    cp++;
    return (cp);
    }

/****************************************************************************/
/* PTopen: open files/pipeline for processing                               */
/****************************************************************************/

int PTopen(struct ptinfo *pt)
    {
    char *tmpname;
    extern FILE *PTpopen();

    if(pt->save)          /* if user selected save all ascii files */
	Esaveascii("");

    switch (pt->cmdtype)
	{

	 /* open file for appending */
	 case AFILE:

	     if(!pt->save)
		 Esaveascii (pt->printfile);

	      pt->fp = fopen(pt->printfile, "a");
	      if (pt->fp == NULL)
		 {
		 Eerror (P_AFILE,
		   "Couldn't open file \"%s\" for appending",pt->printfile);                             s_free (pt->printfile);
		 return(ERROR);
		 }

	      break;

	 /* blast the "..." file. This guarentees that the editor will
	    start up cleanly on the new file. */

	 case OFILE:

	      tmpname = fakename (pt->printfile);
	      (void)unlink(tmpname);
	      s_free (tmpname);

	      /* fall through and open the file for writing */

	 /* open temporary file for command processing */
	 case PIPE:
	 case CMD:

	      pt->fp = fopen(pt->printfile, "w");
	      if (pt->fp == NULL)
		  {
		  Eerror (P_OFILE, "Couldn't open file \"%s\" for writing",
					 pt->printfile);
		  s_free (pt->printfile);
		  return(ERROR);
		  }

	      break;
	}

    return(RET_OK);
    }

/****************************************************************************/
/* PTclose: close files/pipeline                                            */
/****************************************************************************/

int PTclose(struct ptinfo *pt)

    {
    int status;         /* return status of system command */
    char *format;      /* multinational returned format string   */
    char *a_format;    /* multinational returned format string   */
    char *b_format;    /* multinational returned format string   */

    /* close the temp file and run command */
    if(pt->cmdtype == CMD || pt->cmdtype == PIPE)
	{
	(void)fclose (pt->fp);
	if(pt->clear && pt->cmdtype == CMD)
	    {
	    format = Egetmessage(P_EXEC,"Output of command %s:\n\n",FALSE);
	    a_format = PTctu(format);     /* convert trailing underbars */
	    s_free(format);
	    format = Egetmessage(P_CONTINUE,"\nPress RETURN to continue editing:",FALSE);
	    b_format = PTctu(format);
	    s_free(format);

	    Ettyfix ();

	    if(PTsetfds(pt->fd, DEVTTY, DEVTTY) != ERROR)
		{
		(void) printf (a_format, pt->cmd);
		(void) fflush (stdout);

		PTsystem (pt->cmd);

		(void) printf (b_format);
		(void) fflush (stdout);

		while(fgetc (stdin) != '\n')
		    ;
		PTresetfds(pt->fd);
		}

	    Ettyunfix ();

	    s_free(a_format);
	    s_free(b_format);
	    }
	else
	    {
	    pt->errout = s_string (uniqname (TRUE));
	    if(PTsetfds(pt->fd,
			(pt->cmdtype == PIPE ? pt->printfile : DEVTTY),
			pt->errout) != ERROR)
		{
		status = PTsystem (pt->cmd);
		PTresetfds(pt->fd);
		if(pt->allout || status != 0)
		    PTrderr(pt->errout, pt->cmd);
		(void)unlink(pt->errout);
		}
	    s_free(pt->errout);
	    }

	(void)unlink (pt->printfile);  /* remove the temporary file           */
	s_free (pt->printfile);  /* free the name of the temporary file */
	s_free (pt->cmd);        /* free the command                    */
	}
    else       /* else close file and we are done */
	{
	(void)fclose (pt->fp);
	s_free (pt->printfile);
	}

    s_free(pt->popdesc);  /* always free the description */
    }

/****************************************************************************/
/* PTctu: convert trailing underbars in format string to real blanks        */
/*        convert occurances of "\r" and "\n" to the characters '\r' and    */
/*        '\n'                                                              */
/****************************************************************************/

char *PTctu(char *format)

    {
    char *cp;
    char *buf;
    char *pbuf;
    int nbytes, count;

    buf = s_alloc (strlen(format), T_CHAR, (char *) NULL);
    pbuf = buf;

    for (cp = format; *cp; )
	{           
	if ((*cp == '\\') &&  (* (cp + 1) == 'r'))
	    {
	    *pbuf++ = '\r';
	    cp += 2;
	    }
	 else 
            {
            if ((*cp == '\\') && (* (cp + 1) == 'n'))
	        {
	        *pbuf++ = '\n';
	        cp += 2;
	        }
	     else
                {
                if (MultibyteCodeSet)
                   {
                   nbytes = mblen(cp, MB_CUR_MAX); 
		   /* copy at least 1 char */
		   if (nbytes < 1) nbytes = 1;
                   for (count = 0; count < nbytes; count++)
                       *pbuf++ = *cp++;
                   }
                else
                   *pbuf++ = *cp++;             
                }
              }
            }

     *pbuf = '\0';
     buf = s_realloc(buf, strlen(buf));
     blank_trailing_underbars(buf);

     return(buf);
     }

/****************************************************************************/
/* PTsetfds: set up fd's for stdin, stdout, stderr                          */
/*           set stdin to infile                                            */
/*           set stdout to outfile                                          */
/*           set stderr to outfile                                          */
/****************************************************************************/

int PTsetfds(int *fd, char *infile, char *outfile)
    {
    int rv;

    fd[0] = dup(0);
    (void)close(0);
    if ((rv = open (infile, O_RDONLY)) != 0)
	{
	if(rv != -1) /* if we have the wrong fd close it */
	    {
	    (void)close(rv);
	    Eerror(P_BADFD, "Wrong file descriptor fd = \"%d\"", rv);
	    return(ERROR);
	    }

	dup(fd[0]);
	(void)close(fd[0]);
	Eerror(P_RSTDIN, "Couldn't open file \"%s\" for reading standard input", infile);
	return(ERROR);
	}
    fd[1] = dup(1);
    (void)close(1);

    if((rv = open (outfile, O_WRONLY | O_CREAT, 0777)) != 1)
	{
	if(rv != -1) /* if we have the wrong fd close it */
	    {
	    (void)close(rv);
	    Eerror(P_BADFD, "Wrong file descriptor fd = \"%d\"", rv);
	    return(ERROR);
	    }

	dup(fd[1]);
	(void)close(fd[1]);
	(void)close(0);
	dup(fd[0]);
	(void)close(fd[0]);
	Eerror(P_WSTDOUT,
	  "Couldn't open file \"%s\" for writing standard output", outfile);
	return(ERROR);
	}

    fd[2] = dup(2);    /* send stderr to stdout */
    (void)close(2);
    dup(1);
    return(RET_OK);
    }

/****************************************************************************/
/* PTresetfds: reset the fd's for stdin, stout and sterr back to what they  */
/*             were                                                         */
/****************************************************************************/

int PTresetfds(int *fd)
    {
    (void)close(0);
    dup(fd[0]);
    (void)close(fd[0]);
    (void)close(1);
    dup(fd[1]);
    (void)close(fd[1]);
    (void)close(2);
    dup(fd[2]);
    (void)close(fd[2]);
    }

/****************************************************************************/
/* PTrderr: If there was output from the command, display up to 18 lines    */
/*          in a popbox                                                     */
/****************************************************************************/

int PTrderr(char *file, char *cmd)
    {
    char *format;
    char *box;                        /* error box                     */
    char buffer [SIZE];               /* line buffer for output        */
    FILE *fp;                         /* file pointer to error file    */
    int line;                         /* line number of box            */
    int length;                       /* length of the box             */

    extern char *fgets(char *s,int n, FILE *stream);

    fp = fopen (file, "r");
    if (fp == NULL)
	{
	Eerror (P_ERROUT,
    "Cannot open standard output/standard error file \"%s\" for reading",
		file);
	return(ERROR);
	}

    box = s_alloc (0, T_CHAR, (char *) NULL);
    format = Egetmessage(P_OUTPUT,"Output of command %s:\n\n",FALSE);

    (void) sprintf (buffer, format, cmd);
    length = strlen(buffer);
    box = s_realloc (box, length);
    (void) strcat(box, buffer);

    s_free(format);

    /* at most box is 20 lines long - (18 + 2 line header) */

    for (line = 0; line < 18; line++)
	{
	if (fgets (buffer, SIZE, fp) == NULL)
	    break;

	length += strlen(buffer);

	box = s_realloc (box, length);
	(void) strcat(box, buffer);
	}

    box[length - 1] = 0;  /* remove the last newline */

    if(line > 0) /* if the file wasn't empty display its contents */
	Eerror(P_BOX, "%s", box);

    s_free (box);
    (void)fclose(fp);   /* release the file descriptor */
    return(RET_OK);
    }

/****************************************************************************/
/* PTprint: do the printing                                                 */
/****************************************************************************/

void PTprint(struct ptinfo *pt)

    {
    char *form;

    Emessage(P_MESS, "%s...", pt->popdesc);

    /* set up for output translation */
    gr_out = pt->gr_out;

    /* at this point, see if the standard form or the c or h forms are
       involved. If so do a very simple version of printing the file */

    if (pt->formname &&
       (seq ((form = PTbasename(pt->formname)), "std.ofm") ||
	seq (form, "c.ofm")   ||
	seq (form, "h.ofm")))
	PTstdprint (pt->fp);
    else
	PTformprint(pt);
    }

/****************************************************************************/
/* PTstdprint: prints the data of the current field as if there were no     */
/* borders on the standard form.                                            */
/****************************************************************************/

void PTstdprint (FILE *fp)
    {
    int i;
    char *line;
    ATTR *tmp;
    int numlines;

    /* we must be in the right window...        */

    numlines = Enumlines ("");
    for (i = 0; i < numlines; i++)
	{
	line = (char *) Egettext ("", i);
	tmp = (ATTR *)unpackup (line, 0);
	s_free (line);
	PTprtit (tmp, fp);
	s_free ((char *)tmp);
	}
    }

/****************************************************************************/
/* PTprtit: prints the specified line on fp with graphics characters        */
/* escaped to such things as '|', etc                                       */
/****************************************************************************/

void PTprtit (ATTR *line, FILE *fp)

    {
    wchar_t wc;
    int nbytes, count;
    char *linech, *lineptr;

    PTrmtrlblks(line);  /* strip trailing blanks    */
    linech = a2string(line, i_strlen((short *)line));
    lineptr = linech;

    while (*line)
	  {
	  if (is_underlined (*line)) /* watch out for underlined chars */
             {
             (void) fputc ('_', fp);
             (void) fputc ('\b', fp);
             }

          /* print character (if printable character) */
          nbytes = mbtowc(&wc, lineptr, MB_CUR_MAX);
	  /* copy at least 1 char */
	  if (nbytes < 1) nbytes = 1;
          if (iswprint((wint_t)wc)) 
             (void)fputwc(wc, fp);
          else {
               /* not printable- must be a graphics character */
               if (gr_out)
                  (void) fputwc (wc, fp);
               else {
                    switch (*lineptr)
                           { 
                           case G_ULC :     /* ulhc           */
                           case G_URC :     /* urhc           */
                           case G_LLC :     /* llhc           */
                           case G_LRC :     /* lrhc           */
                           case G_X   :     /* cross          */
      
                               (void) fputc ('+', fp);
                           break;
      
                           case G_HLINE :     /* horizontal bar */
                           case G_TEE   :     /* T joint        */
                           case G_ITEE  :     /* inverse T      */
      
                               (void) fputc ('-', fp);
                               break;
      
                           case G_VLINE :     /* vertical bar   */
                           case G_TOL   :     /* left T         */
                           case G_TOR   :     /* right T        */
      
                               (void) fputc ('|', fp);
                               break;
      
                           default:
                               (void) fputc (SPACE, fp);
                            }                         /* end switch */
                     }                                /*end else    */
               }
	  line += nbytes;
	  lineptr += nbytes;
	  }           /*end while */

    (void) fputc ('\n', fp);
    s_free(linech);
    return;
    }

/****************************************************************************/
/* PTrmtrlblks: remove trailing blanks from line.                           */
/****************************************************************************/

int PTrmtrlblks(ATTR *line)

    {
    ATTR *cp;

    for (cp = line; *cp; cp++)
	;

    cp--; /* cp pointed at null char, which is always 1 long, so
             no backupattr needed here. */

    while (cp >= line && isattrspace(cp) )
	backupattr(&cp);
    skipattr(&cp);

    *cp = 0;
    }

/****************************************************************************/
/* PTformprint: print the text through the form                             */
/****************************************************************************/

void PTformprint(struct ptinfo *pt)
    {
    int virlin;                 /*"Virtual" line number being worked on */
    ATTR invtext [TVWIDTH + 1]; /* line being pasted up                 */
    ATTR gutted  [TVWIDTH + 1]; /* gutted version of invtext            */
    struct box *boxes;          /* info on all the fields of the form   */

    boxes = (struct box *)
		 calloc (obj_count (pt->warray), sizeof (struct box));

    PTgetboxes(boxes, pt);  /* first, collect a little information */

    /* across the virtual screen    */

#ifdef VBLSCR
    offset = Egetttline (0);
    TVheight = Egettbline (0) - offset + 1;

    for (virlin = 0; virlin < TVheight; virlin++)
#else
    for (virlin = 0; virlin < TVHEIGHT; virlin++)
#endif
	{
	PTinvtext(invtext, virlin);  /* get invariant text  */

	/* make copy for 2nd thru Nth use (nothing but border chars)    */
	i_strcpy ((short *)gutted, (short *)invtext);
	PTgut (gutted);

#ifdef VBLSCR
	PTwindows (virlin, invtext, gutted, boxes, pt, offset);
#else
	PTwindows(virlin, invtext, gutted, boxes, pt);
#endif
	}

    free (boxes);
    }

/****************************************************************************/
/* PTwindows: print the contents of the windows                             */
/****************************************************************************/

#ifdef VBLSCR
void PTwindows(int virlin, ATTR *invtext, ATTR *gutted, struct box *boxes, 
          struct ptinfo *pt, int offst)
#else
void PTwindows(int virlin, ATTR *invtext, ATTR *gutted, struct box *boxes, 
          struct ptinfo *pt)
#endif
    {
    int i;             /* loop poop                            */
    int printed;      /* flag says invtext has been printed   */

    printed = FALSE;  /* set flag at beginning of each line */

    /* search list of windows, installing field values for
       all active at this virtual screen line               */

    for (i = 1; i < obj_count (pt->warray); i++)
	{
#ifdef VBLSCR
	if ((virlin + offst) < boxes [i].start
	 || (virlin + offst) > boxes [i].end
	 || boxes [i].curidx > boxes [i].limit)
#else
	if (virlin < boxes [i].start
	 || virlin > boxes [i].end
	 || boxes [i].curidx > boxes [i].limit)
#endif
	    continue;

	/* normal case - install this field value in the invtext line */

#ifdef VBLSCR
	if ((virlin + offst) < boxes [i].end)
#else
	if (virlin < boxes [i].end)
#endif
	    {
	    PTstuff (invtext, &(pt->warray) [i], boxes [i].curidx++);
	    if (printed)
		printed = FALSE; /* be sure to print this new line */

	    continue;
	    }

	/* if on last screen line of a field, exhaust it    */

#ifdef VBLSCR
	if ((virlin + offst) == boxes [i].end)
#else
	if (virlin == boxes [i].end)
#endif
	    {
	    if (boxes [i].curidx <= boxes [i].limit)
		{
		PTlastline(i, virlin, invtext, gutted, boxes, pt);
		printed = TRUE; /* PTlastline always does a PTprtit */

		/*$$ keep from tripping on this window again    */
		boxes [i].end = ERROR;
#ifdef VBLSCR
		boxes [i].start = TVheight;
#else
		boxes [i].start = TVHEIGHT;
#endif
		continue;
		}
	    }
	}

     /* if the line has not yet been printed then do it */
     if(printed == FALSE)
	PTprtit (invtext, pt->fp);
     }

/****************************************************************************/
/* PTlastline: if we are on the last virtual line                           */
/*             continue to print as long as window has contents             */
/****************************************************************************/

void PTlastline(int i, int virlin, ATTR *invtext,
				   ATTR *gutted, struct box *boxes,
				   struct ptinfo *pt)
    {
    int firsttime = TRUE;

    /* as long as this window has contents  */
    while (boxes [i].curidx <= boxes [i].limit)
	{

	/* go thru all windows and empty them
	   watchout for windows already written  */

	PTemptyall(virlin, invtext, boxes, pt, firsttime ? i : 1);
	firsttime = FALSE;

	PTprtit (invtext, pt->fp);
	i_strcpy ((short *)invtext, (short *)gutted);
	}
    }

/****************************************************************************/
/* PTemptyall:  go thru all windows and empty them                          */
/****************************************************************************/

void PTemptyall(int virlin, ATTR *invtext, struct box *boxes,
					   struct ptinfo *pt, int i)
    {
    int j;

    /* go thru all windows and empty them       */
    for (j = i; j < obj_count (pt->warray); j++)
	{
#ifdef VBLSCR
	if ((virlin + offset) < boxes [j].start
	 || (virlin + offset) > boxes [j].end
	 || boxes [j].curidx > boxes [j].limit)
#else
	if (virlin < boxes [j].start
	 || virlin > boxes [j].end
	 || boxes [j].curidx > boxes [j].limit)
#endif
	    continue;

	PTstuff (invtext, &(pt->warray) [j], boxes [j].curidx++);
	}
    }

/****************************************************************************/
/* PTgetboxes: initialize the boxes array                                   */
/*             all of this code is 0 origined (boxes [0] is left undefined, */
/*             since window 0 is special for our purposes)                  */
/****************************************************************************/

void PTgetboxes(struct box *boxes, struct ptinfo *pt)

    {
    int i;
    WSTRUCT *pw;

    for (i = 1; i < obj_count (pt->warray); i++)
	{
	pw = &(pt->warray) [i];

	if (w_flags (pw) & INVISIBLE) /* starting line */
#ifdef VBLSCR
	{
	    boxes [i].start = TVheight;
	    boxes [i].limit = ERROR;
	}
	else
	{
	    boxes [i].start = Egetttline (i);
	    boxes [i].limit = Enumlines (w_name (pw)) - 1;
	}
	boxes [i].curidx = 0;                /* current place in field  */
	boxes [i].end = Egettbline (i);
#else
	    boxes [i].start = TVHEIGHT;
	else
	    boxes [i].start = w_ttline (pw);

	boxes [i].end   = w_tbline (pw);     /* ending line             */
	boxes [i].curidx= 0;                 /* current place in field  */
	boxes [i].limit = Enumlines (w_name (pw)) - 1;
#endif

#ifdef DEBUG
debug ("window %d, start %d, end %d, limit %d\n",
	 i, boxes [i].start, boxes [i].end, boxes [i].limit);
#endif
	}
    }

/****************************************************************************/
/* PTinvtext: read invariant text of window into buffer                     */
/****************************************************************************/

void PTinvtext(ATTR *buffer, int line)

    {
    char *invcp;                /* used to generate invariant text      */
    ATTR *uinvcp;

    invcp = Egettext ("InvTxt", line);
    uinvcp = (ATTR *)unpackup (invcp, TVWIDTH);
    (void)s_free (invcp);
    (void) i_strncpy ((short *)buffer, (short *)uinvcp, TVWIDTH);
    buffer [TVWIDTH] = '\0';
    (void)s_free ((char *)uinvcp);
    }

/****************************************************************************/
/* PTgut: remove all but graphics characters from the supplied line of      */
/*      invariant text                                                      */
/****************************************************************************/

void PTgut (ATTR *cp)
    {

  if (MultibyteCodeSet)
     {
     ATTR *cpcpy = malloc(i_strlen((short *)cp) + 1);
     ATTR *cpcpy_buf = cpcpy;

     (void) i_strcpy((short *)cpcpy, (short *)cp);

     while (*cpcpy)
	{
	if (iswgraph(attr_to_wc(cpcpy)))
	    {
	    *cp++ = SPACE | FIRST;
	    skipattr(&cpcpy);
	    }
        else attrcpy(&cp, &cpcpy);
	}

     *cp = (ATTR)0;
     free(cpcpy_buf);  
     }
  else
    while (*cp)
	{
	if (isgraph(de_attr(*cp)))
	    *cp = SPACE | FIRST;
	cp++;
	}
  }

/****************************************************************************/
/* PTstuff: put contents of given window into the invariant text (similar   */
/* to displaying it)                                                        */
/****************************************************************************/

void PTstuff (ATTR *invtext, WSTRUCT *pw, int line)
    {
    char *fvalue;      /* value of current field        */
    int i;
    ATTR *tmp;         /* internal format for fvalue    */

    fvalue = (char *) Egettext (w_name (pw), line);
    tmp = unpackup (fvalue, 0);
    (void)s_free (fvalue);

    /* trim field value to fit form (horizontally)              */
		    /* width of pw */

    for (i = 0;
	 i < min ((w_lrcol (pw) + 1), i_strlen ((short *)tmp));
	 i++)
	invtext [w_ttcol (pw) + i] = tmp [i];

    (void)s_free ((char *)tmp);
    }

/****************************************************************************/
/* PTsystem:  executes the command in a sub-process.                        */
/*            interrupts are enabled in the sub-process                     */
/****************************************************************************/

int PTsystem(char *s)
{
    int status, pid, w;

#ifdef DEBUG
    debug ("PTsystem:  command = '%s'", s);
#endif

    if((pid = fork()) == 0)   /* the child process */
	{
	(void) signal (SIGINT, SIG_DFL);  /* enable interrupts */
	execl("/bin/sh", "sh", "-c", s, (char *)0);
	_exit(127);
	}

    while((w = wait(&status)) != pid && w != -1)
	    ;

    if(w == -1)
	status = -1;

#ifdef DEBUG
    debug ("PTsystem:  status = 0%o", status);
#endif

    return (status);
}

#ifdef LEAK_FINDING

/****************************************************************************/
/* Hstop: free all the allocations for the leak finder.                     */
/****************************************************************************/

void Hstop (void)
    {
    catclose(g_printcatd);
    Pcleanup();
    _cleanup();
    (void)abort ();
    }

#endif

