/* @(#)38       1.9  src/tenplus/e2/include/def.h, tenplus, tenplus411, GOLD410 1/21/94 15:40:31 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10, 27
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
 
/*****************************************************************************
* File: def.h - defines for e2                                               *
*****************************************************************************/
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <nl_types.h>

#include "database.h"
#include "window.h"
#include "chardefs.h"
#include "keys.h"
#include "libobj.h"
#include "libshort.h"
#include "libutil.h"
#include "edexterns.h"

extern int LI, CO;

#define R_FONT  0              /* input font states */
#define W_FONT  1
#define C_FONT  2
#define G_FONT  3

#define UAR '^'                /* direction values for text off screen */
#define DAR 'v'
#define LAR '<'
#define RAR '>'

#define WORDWRAP     1         /* word & character wrap mode values */
#define CHARWRAP     0
#define WRAPPUNCT    1
#define NOWRAPPUNCT  0

/* g_usefiles object flags which indicate file type */
#define USETYPE   (017) /* this should be &'ed with flag to get type */
#define USEFILE   (1)
#define USEPOPBOX (2)
#define USESCREEN (3)
#define USEHELPER (4)
#define USEFORM   (5)
#define USEQUIT   (6)
#define USESYNC   (020)
#define USESAVE   (040)

#define MAXL (300 * MB_LEN_MAX) /* maximum number of bytes in a line */
#define LOCAL                   /* If null, then adb works, else static */

#define MAXFLINE 20000          /* maximum field line number */
#define MAXFCOL  200            /* maximum field column number */

#define DIRHELPER  "index"      /* directory helper */

#define getstat(status,bits)  ((status) & (bits))
#define setstat(status,bits)  ((status) |= (bits))

#define objprint(name,obj) if (g_objflag) { debug ("%s (0%o):\n", name, obj); \
					 if (obj) s_print (obj); }

#define thrugngs {struct window *_t = g_wp; do {

#define endthrugngs g_wnode = (char *) ERROR; } \
       while ((w_nxtgang (g_wp) != (WSTRUCT *)NULL) && ((g_wp = w_nxtgang (g_wp)) != _t)); }

/* extra error codes for g_errno */
#define E_NOFILE  100              /* findfile error          */
#define E_NOFORM  101              /* no records in form file */
#define E_BADFORM 102              /* form file not window    */

/* offset for helper-specific error codes
   NOTE:  this is also defined in mkdefs.c */

#define HELPER_MSG_OFFSET  5000

/* stat mode bits (modebits return values) */
#define READ  (4)
#define WRITE (2)
#define EXEC  (1)

/* g_readonly value: TRUE, FALSE, DIR_NOREAD */
#define DIR_NOREAD (2)

/* dispflag values */
#define DISPINIT (0) /* initial value:  redisplay everything            */
#define DISPNORM (1) /* normal value:  display anything that's changed  */

/* window flag contents */
#define W_FILENAME   2
#define W_ABSPATHS   4

/* g_helpstate values (used in controlling zoomstack during HELP)
   NOTE: the values must be consecutive integers in this order */

#define NOTHELPING  (0)   /* normal state, user has not asked for help  */
#define INHELPMENU  (1)   /* user looking at help menu from normal file */
#define INHELPFILE  (2)   /* looking at a help file selected from help menu */
#define IN2NDHMENU  (3)   /* looking at help menu again from help file */

/* g_hooks bits (used by Esethook) */
#define BEFORE (1) /* means Hbefore should be called for command */
#define AFTER  (2) /* means Hafter should be called for command  */

/* editor globals */
extern POINTER *g_altstack;        /* alternate file zoom stack             */
extern int      g_argbits [];      /* command argument descriptor table     */
extern int      g_breakset;        /* break set                             */
extern int      g_brkflag;         /* TRUE if last input char was a break   */
extern ATTR   **g_curscreen;       /* current screen                        */
extern char     g_curdir [];       /* current UNIX directory                */
extern int     *g_curcrc;          /* current crc                           */
extern int      g_cursorflag;      /* TRUE if cursor should be repositioned */
extern char    *g_dfltfile;        /* Place to go when all else fails       */
extern int      g_diskfull;        /* disk full flag                        */
extern int      g_dispflag;        /* display flag                          */
extern char    *g_dispname;        /* display file name                     */

extern void   (*g_edfunction [])(int, char *, int);/* command dispatch table*/

extern int      g_fakefile;        /* TRUE if editing fake file             */
extern POINTER *g_fieldlist;       /* field names for partial record reading*/
extern char    *g_filename;        /* file name                             */
extern int      g_font;            /* current input font (for window 0)     */
extern POINTER *g_formcache;       /* forms cache                           */
extern char    *g_formname;        /* name of current form                  */
extern POINTER *g_formpath;        /* forms search directories              */
extern POINTER *g_def_formpath;    /* default forms search directory        */
extern int     *g_goalcrc;         /* goal crc                              */
extern ATTR   **g_goalscreen;      /* goal screen                           */
extern char    *g_helper;          /* name of active helper                 */
extern POINTER *g_helpfiles;       /* HELP menu files                       */
extern char     g_hooks [];        /* Hbefore and Uafter control hook table */
extern POINTER *g_hlprpath;        /* helper search dirs                    */
extern char    *g_homedir;         /* login directory                       */
extern int      g_imodeflag;       /* TRUE if insert mode is on             */

extern int      g_fmtmode;         /* WRAPPUNCT or NOWRAPPUNCT */

extern int      g_inmenu;          /* TRUE if doing usefile from task menu  */
extern int      g_intflag;         /* TRUE if interrupt has occurred        */
extern int      g_keyvalue;        /* keyvalue code for current char        */
extern int      g_killhelper;      /* TRUE if helper just did Ekillhelper   */
extern char    *g_lastarg;         /* last argument entered                 */
extern int      g_noindex;         /* TRUE if index helper is not around    */
extern int      g_objflag;         /* TRUE if objprint should be done       */
extern int      g_popbottom;       /* popbox bottom in zero based tv coords */
extern int      g_popflag;         /* TRUE if pop up box is displayed       */
extern int      g_popleft;         /* popbox left in zero based tv coords   */
extern int      g_popright;        /* popbox right in zero based tv coords  */
extern int      g_poptop;          /* popbox top in zero based tv coords    */
extern int      g_quietflag;       /* helper quiet flag                     */
extern int      g_readonly;        /* TRUE if file is readonly              */
extern int      g_recdirty;        /* record dirty flag                     */
extern int      g_recindex;        /* index of current record               */
extern char    *g_record;          /* current record                        */
extern SFILE   *g_sfp;             /* current sfile object                  */
extern char    *g_snode;           /* "ulhc" of form in data                */
extern char    *g_spath;           /* path name inside record               */
extern char    *g_syncname;        /* real name of file being sync'ed       */
extern char    *g_sysdir;          /* editor installation directory         */
extern FILE    *g_textfp;          /* file pointer for the text file        */

extern unsigned char g_textvalue[MB_LEN_MAX+1];

extern int      g_ttyflag;         /* TRUE if terminal has been initialized */
extern int      g_uniqnum;         /* unique number for crc                 */
extern int      g_usechar;         /* TRUE if nextin should reuse last char */
extern POINTER *g_usefiles;        /* USE menu files                        */
extern WSTRUCT *g_warray;          /* array of all windows                  */
extern POINTER *g_watchfiles;      /* files to watch                        */
extern int      g_wmodeflag;       /* WORDWRAP or CHARWRAP                  */
extern char    *g_wnode;           /* "ulhc" of window in data              */
extern WSTRUCT *g_wp;              /* the current window                    */
extern POINTER *g_zoomstack;       /* form name zoom stack                  */
extern ATTR     g_place_filler;    /* ATTR to hold place of 1/2 multi Attrs */
extern int      g_help_count;
extern nl_catd   g_helpfilecatd;

/* E2 extern function declarations */
extern void     Efatal (int , char *); /* This is the ed.c Efatal version */

/* Library routine extern declarations */
extern char     *s_mknode (POINTER * tree, char * path, int leaftype);
#define St_flush()  (void) fflush (stdout)

/*  Declarations for a lot of files */
extern char 	*altname(void);
extern int 	checkpop(void);
extern void 	cleanstack(void);
extern void 	popstate(int);
extern void 	pushstate(int);
extern void 	zoomin(void);
extern void     zoominit(void);
extern void     zoomout(void);
extern void     tmopen (void);
extern void     fixname (void);
extern char 	*s_findnode (POINTER *, char *);
extern void     sendbox (int, int, int, int);

/* Some prototypes for eimports */
extern int  Hinit(char *);
extern void Hafter(int);
extern int  Hbefore(int, int, char *, int);
extern void Hdel(char *, int, int);
extern int  Hmod(char *, int, char *, char *);
extern void Hins(char *, int, int);
extern int  Hrestart(char *);
extern char *Hsavestate(void);
extern void Halarm(void);
extern void smear(char, int, char *);
extern char *pathcat (char *, char *);

extern int copyfile(FILE *, FILE *);
extern int builtin_helper(char *);
extern int seq (char *, char *);

extern void usepop (char *, char *, int, int);
extern void fixtabs(void);
extern void logfatal(char *, int);
extern void tmclose (void);
extern void Eflushform (char *);
extern void save_reclocs (void);
extern void bagfakes (void);
extern FILE *efopen (char *, char *);
extern FILE *ffopen (char *, char *, char *);
extern SFILE *esf_open (char *, char *);
extern SFILE *fsf_open (char *, char *, char *);
SFILE *openfake (char *);
extern char *readfake (SFILE *, FILE *, int);
extern char *backname (char *);
extern int fbackup (char *);
extern void saveascii (char *);
extern void savefakes (void);
extern void Eclrhook (char *, char *);
extern void action (void);
extern void backspace (void);
extern void callfunction (void (*) (int, char *, int), int, char *, int);
extern void clrhooks (void);
extern void enter (void);
extern void St_init (void);
extern int getflag (char *);
extern int getnum (char *);
extern int parse_tc_entry (char *, char *);
extern void St_exitmodes (void);
extern void St_quit (void);
extern void St_outchar (ATTR *, int);
extern void St_getcap (void);
extern char *getstring (char *);
extern void Si_init (void);
extern int Si_display (int);
extern void dodots (int, WSTRUCT *);
extern void repaint (int);
extern int scr_seq (ATTR *, ATTR *);
extern void fixmode (int, int);
extern int setattr (ATTR *, int, int, ATTR);
extern void Si_redisplay (void);
extern void Si_quit (void);
extern void term_enter (void);
extern void term_exit (void);
extern int winsize(void);
extern Si_dirty (int, int);
extern void Si_ivector (ATTR *, int, int, int);
extern void Si_ovector (ATTR *, int, int, int);
extern void Si_osvector (char *, int, int, int);
extern void boxopen (int, int, int, int);
extern void textregion (void);
extern void textopen (int, int, int, int);
extern void boxregion (void);
extern void markregion (int, int, int, int, int);
extern void complement_region (int, int, int, int);
extern void invert_text_region (int, int, int, int, int, int);
extern void setcursor (void);
extern void bol (void);
extern void insert (int);
extern void left (int, char *, int);
extern void linefeed (void);
extern void margin (int, char *, int);
extern void mnline (int, char *, int);
extern void mnpage (int, char *, int);
extern void newline (void);
extern void nextitem (int, char *, int);
extern void nospecials (void);
extern void numtoobig (void);
extern void chfont (int, char *, int);
extern void pick (int, char *, int);
extern void plline (int, char *, int);
extern void plpage (int, char *, int);
extern void print (void);
extern void Refresh (void);
extern void right (int, char *, int);
extern void save (int, char *, int);
extern void tabset (int, char *, int);
extern void use (int, char *, int);
extern void delch (int, char *, int);
extern void e2close (int, char *, int);
extern void e2do (int, char *, int);
extern void e2open (int, char *, int);
extern void e2return (void);
extern void eol (void);
extern void fbutton (void);

/* imports.c others */
extern ATTR *strip (ATTR *);
extern void puttext (int , ATTR *);
extern int  gowin (char *);
extern int  clipcursor(WSTRUCT *);
extern void dofilter (char *);
extern int  filter (char *, char *, char *);
extern void format (int, char *, int);
extern int  cache_line (int, ATTR *);
extern void putli (int, ATTR *);
extern int  checkprot (void);
extern void flush_window (void);
extern void getall (void);
extern void getgngs (int, int);
extern ATTR *getli (int);
extern void movdn (int);
extern int  movup (int);
extern void putgng (int, int);
extern void closehelper (void);
extern void errx (void);
extern void killhelpers (void);
extern void openhelper (char *, char *, char *);
extern void Ebell (void);
extern char *Egetprefix (char *);
extern char *Egetsuffix (char *);
extern void Ecallfun (int, int, char *, int);
extern void Eoldhelper (void);
extern void Eputformat (char *, char *);
extern int  Eputlist (char *, int, POINTER *);
extern void Echangepfx (int, char *);
extern void Edisplay(void);
extern int  Eemptyrecord(void);
extern char *inbox (int, int, int, int, int);
extern int  input_waiting (void);
extern void onechar (int);
extern void quote (void);
extern void help (int);
extern void helpbox (int, int);
extern void helpmenu (int, int);
extern int  nextchar (unsigned char *);
extern void nextin (void);
extern void St_goto(int, int);
extern int  Escrnio (void);
extern char *fixcommand (char *);
extern POINTER *makelist (char **);
extern void task (int, char *, int);
extern void taskmenu (int, char *, POINTER *);
extern void nextsibling (int);
extern void Remove (char *);
extern char *restofpath (char *);
extern char *trimpath (char *);
extern int  Esetalarm (int);
extern int  Eclearalarm (void);
extern void Ediskfull(char *);
extern char *findfile (char *, char *, POINTER *);
extern char *firstnode (char *);
extern char *formprefix (char *);
extern int  gotosibling (char *);
extern int  inlist (POINTER *, char *);
extern void make_vector(int, char **, char **);
extern void allocerr (void);
extern void suspend(void);
extern void winchsz2(int);
extern void winchsz(void);
extern int  readprofile (char *);
extern void watchfiles (void);
extern void e2exit (int, char *, int);
extern int  datalength (void);
extern void pfxnode (int);
extern ATTR *readline (int, int);
extern void writeline (int, ATTR *);
extern int  emptyrecord (POINTER *);
extern void delline (int, int);
extern void digin (void);
extern char *findrecord (char *);
extern char *fixtree (char *, char *, char *);
extern void flushrecord (int);
extern POINTER *idxnode (int, int);
extern void insline (int, int);
extern void dosetup (FILE *, FILE *);
extern sizenewbox (int, int, int, int, int *, int *, int *, int *);
extern int newbox (POINTER *);
extern ATTR **doformat (ATTR **, int);
extern void Eerror (int, char *, ...);
extern void Emessage (int, char *, ...);
extern char *ask (int, int, ...);
extern void killbox (int);
extern void boxpick (int, int, int, int, int);
extern void opendelfile (void);
extern void openput (void);
extern void putback (int, char *, int);
extern void savelines (int, int);
extern void textpick (int, int, int, int, int);
extern char *s_read (FILE *);
extern char *s_realloc (char *, unsigned);
extern char *s_string (char *);
extern int  s_write (char *, FILE *);
extern char *scopy (char *);
extern int  setfile (char *, int, int);
extern void usefile (char *);
extern void altfile (void);
extern void closefile (void);
extern char *defaultform (void);
extern void getword (char *);
extern void go (int, char *, unsigned);
extern void keyreplace (int, char *, int);
extern void keysearch (int, char *, int);
extern void putcurs (int, int);
extern void search (char *, int);
extern void text(void);
extern void plop(void);
extern int  isclosepunct (ATTR *);
extern void dowrap(void);
extern void lrscroll(void);
extern int  nblen(ATTR *);
extern int  coverup (ATTR *);
extern int  is_long (ATTR *, int);
extern void tvdn (int, int, int);
extern void tvlf (int, int, int, int, int);
extern void tvrt (int, int, int, int, int);
extern void tvup (int, int, int);
extern void backtab (void);
extern void chwin (int, char *, int);
extern void tab (void);
extern void winbtab (void);
extern int  winlf (void);
extern void wintab (void);
extern void leavewin (WSTRUCT *);
extern void switchwin (void);
extern void window (int, char *, int);
extern void center (int, char *, unsigned);
extern void split (void);
extern void delli (int, int, int);
extern void delsp (int, int, int);
extern void insli (int, int);
extern void inssp (int, int, int);
extern void join (void);
extern int  movlf (int);
extern int  movrt (int);
extern void cursor (void);
extern int  calc_column(ATTR *, int);
extern int  char_width(ATTR *);
extern int  text_width(char *);
extern int  calc_line_position(ATTR *, int);
extern void drawline(ATTR , ATTR, ATTR ,
                int , int , int , ATTR **, ATTR *, ATTR **);
