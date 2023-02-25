#if !defined(lint)
static char sccsid[] = "@(#)40	1.8  src/tenplus/e2/common/globals.c, tenplus, tenplus411, GOLD410 1/21/94 15:39:54";
#endif /* lint */

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
 
/****************************************************************************/
/* file:  globals.c                                                         */
/*                                                                          */
/* global variables for the e2 editor                                       */
/****************************************************************************/

#include "def.h"
#include "keynames.h"

POINTER *g_altstack;               /* alternate file zoom stack             */



int      g_brkflag = TRUE;         /* TRUE if last input char was a break   */
char     g_curdir [PATH_MAX];      /* current UNIX directory                */
int      g_cursorflag;             /* TRUE if cursor should be repositioned */
char    *g_dfltfile;               /* Place to go when all else fails       */
int      g_dirtflag;               /* TRUE if any line is dirty             */
int      g_diskfull;               /* disk full flag                        */
int      g_dispflag;               /* display flag                          */
char    *g_dispname;               /* display file name                     */
/* VOID   (*g_edfunction [])(); */ /* defined in tables.c                   */
int      g_fakefile;               /* TRUE if editing fake file             */

POINTER *g_fieldlist;              /* field names for partial record reading*/

char    *g_filename;               /* file name                             */
int      g_font;                   /* current input font (for window 0)     */
POINTER *g_formcache;              /* forms cache                           */
char    *g_formname;               /* name of current form                  */
POINTER *g_formpath;               /* forms search directories              */
POINTER *g_def_formpath;           /* default forms search directory        */
char    *g_helper;                 /* name of active helper                 */
POINTER *g_helpfiles;              /* HELP menu files                       */
char     g_helpstate=NOTHELPING;   /* wants help, getting help, more help   */
char     g_hooks [NUMKEYS];        /* Hbefore and Hafter control hook table */
POINTER *g_hlprpath;               /* helper search dirs                    */
char    *g_homedir;                /* login directory                       */
int      g_imodeflag = TRUE;       /* TRUE if insert mode is on             */

int      g_wmodeflag;              /* WORD or CHAR wrapping in effect       */
int      g_fmtmode = WRAPPUNCT;    /* wrap punctuation by default           */

int      g_inmenu = FALSE;         /* TRUE if doing usefile from task menu  */
int      g_intflag;                /* TRUE if interupt has occurred         */
int      g_invtext;                /* TRUE if cursor is in invarient text   */
int      g_keyvalue;               /* keyvalue code for current char        */
int      g_killhelper;             /* TRUE if helper has called Ekillhelper */
char    *g_lastarg;                /* last argument entered                 */


int      g_noindex;                /* TRUE if index helper is not around    */
int      g_objflag;                /* TRUE if objprint should be done       */
int      g_popbottom;              /* popbox bottom in zero based tv coords */
int      g_popflag;                /* TRUE if pop up box is displayed       */
int      g_popleft;                /* popbox left in zero based tv coords   */
int      g_popright;               /* popbox right in zero based tv coords  */
int      g_poptop;                 /* popbox top in zero based tv coords    */
int      g_readonly;               /* 0:write, 1:read, 2:dir not writeable  */
int      g_recdirty;               /* record dirty flag                     */
int      g_recindex = -1;          /* index of current record               */
char    *g_record;                 /* current record                        */

SFILE   *g_sfp;                    /* current sfile object                  */
char    *g_snode;                  /* "ulhc" of form in data                */
char    *g_spath;                  /* path name inside record               */
char    *g_syncname;               /* real name of file being sync'ed       */
char    *g_sysdir;                 /* editor installation directory         */
FILE    *g_textfp;                 /* file pointer for the text file        */

unsigned char     g_textvalue[MB_LEN_MAX+1];/* text char if keyvalue is TEXTKEY*/

int      g_ttyflag;                /* TRUE if terminal has been initialized */
int      g_usechar;                /* TRUE if nextin should reuse last char */
POINTER *g_usefiles;               /* USE menu files                        */
WSTRUCT *g_warray;                 /* array of all windows                  */
POINTER *g_watchfiles;             /* files to watch                        */
char    *g_wnode = (char *) ERROR; /* "ulhc" of window in data              */
WSTRUCT *g_wp;                     /* the current window                    */
POINTER *g_zoomstack;              /* form name zoom stack                  */
int	g_help_count;	           /* Number of help text catalog entries   */
nl_catd g_helpfilecatd;            /* Help file catalogue descriptor        */

/***** globals in files *****/

/* int   argbits [];       (tables.c)  command argument descriptor table    */
/* int   breaks;           (in.c)      break set                            */
/* int (*edfunction [])(); (tables.c)  command dispatch table               */
/* REMOTE g_remotes []    (eimports.c) table of helper functions            */
/* LOCAL  g_locals []     (eimports.c) table of editor functions            */
/* int    g_nlocals;      (eimports.c) number of locals (plus one)          */
/* g_searchstr;           (src.c)      address of static char srch string   */

/***** notes about initial values of globals *****/

/* dirtyleft [line] = -1 means line not dirty              */
/* brkflag is initialized to TRUE to force initial display */


