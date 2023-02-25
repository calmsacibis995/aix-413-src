/*NOTUSED*/
/* @(#)96	1.7  src/tenplus/include/window.h, tenplus, tenplus411, GOLD410 3/23/93 12:02:22 */

/*
 * COMPONENT_NAME: (INED) INed Editor
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
 
/*****************************************************************************
* File: window.h - defines the window structure.                             *
*****************************************************************************/

#define STANDARDFORM  "std"     /* standard text file form      */
#define INDEXFORM     "sindex"  /* form for non-text files      */
#define FORMEXTENSION ".ofm"    /* extension for compiled forms */
#define	WRONGSIZEFORM "*wrongsize*"	/* dummy form name used to */
					/* indicate that the window */
					/* has changed size, so that */
					/* the form in question is now */
					/* the wrong size. */

#define W_SZ            30      /* used in window structure     */
#define TVHEIGHT        24      /* number of lines on screen    */
#define TVWIDTH         80      /* number of columns on screen  */

/*** for adding additional elements to the window structure ***/
/*** see: lib/window/wn_read.c and lib/window/wn_write.c    ***/

#define STOP        (0375)      /* marks end of the data */
#define CONTINUE    (0374)      /* additional structure elements follow */

/***** window structure *****/
#ifndef WIN_DEF 
#define WIN_DEF 
typedef struct window WSTRUCT;
#endif


struct window
    {
    int      w__flags;           /* status bits for field                   */
    char    *w__name;            /* the window ID (used by helper)          */
    char    *w__pfx;             /* field prefix                            */
    char    *w__sfx;             /* field suffix                            */
    char    *w__zoom;            /* name of zoom form                       */
    char    *w__tvloc;           /* ulhc of the window in tv array          */
    int      w__line;            /* cursor line in 0 based window coords    */
    int      w__col;             /* cursor column in 0 based window coords  */
    int      w__lrline;          /* bottom line in 0 based window coords    */
    int      w__lrcol;           /* right column in 0 based window coords   */
    int      w__ftline;          /* ulhc line in 0 based field coords       */
    int      w__ftcol;           /* ulhc column in 0 based field coords     */
#ifdef VBLSCR
    char    *w__modflgs;         /* modified bits array for window          */
    int      w__CO;              /* CO value at time form was read in       */
    int      w__LI;              /* LI value at time form was read in       */
    char     dummy [W_SZ - sizeof (char *) - 2 * sizeof (int)];
#else
    char     w__modflgs [W_SZ];  /* modified bits array for window          */
#endif
    int      w__ttline;          /* ulhc line in 0 based tv coords          */
    int      w__ttcol;           /* ulhc column in 0 based tv coords        */
    int      w__tbline;          /* lrhc line in 0 based tv coords          */
    int      w__tbcol;           /* lrhc column in 0 based tv coords        */
    POINTER *w__cache;           /* line cache array for window             */
    RECORD  *w__reclocs;         /* file locations of the records in cache  */
    WSTRUCT *w__nxtgang;         /* next window to right in a gang          */
    char    *w__tabs;            /* tabs array                              */
    unsigned char w__lmar;       /* left margin in 0 based window coords    */
    unsigned char w__rmar;       /* right margin in 0 based window coords   */
    char    *w__filename;        /* saved g_filename                        */
    char    *w__pickname;        /* name for PICK/CLOSE                     */
    int      w__msgno;           /* Mesage number for a given window        */
    char    *w__deftxt;          /* Default text for a a given field        */
    };

#define w_flags(wp)    (wp)->w__flags
#define w_name(wp)     (wp)->w__name
#define w_pfx(wp)      (wp)->w__pfx
#define w_sfx(wp)      (wp)->w__sfx
#define w_zoom(wp)     (wp)->w__zoom
#define w_tvloc(wp)    (wp)->w__tvloc
#define w_line(wp)     (wp)->w__line
#define w_col(wp)      (wp)->w__col
#define w_lrline(wp)   (wp)->w__lrline
#define w_lrcol(wp)    (wp)->w__lrcol
#define w_ftline(wp)   (wp)->w__ftline
#define w_ftcol(wp)    (wp)->w__ftcol
#define w_modflgs(wp)  (wp)->w__modflgs
#define w_ttline(wp)   (wp)->w__ttline
#define w_ttcol(wp)    (wp)->w__ttcol
#define w_tbline(wp)   (wp)->w__tbline
#define w_tbcol(wp)    (wp)->w__tbcol
#define w_cache(wp)    (wp)->w__cache
#define w_reclocs(wp)  (wp)->w__reclocs
#define w_nxtgang(wp)  (wp)->w__nxtgang
#define w_istabs(wp)   (wp)->w__tabs
#define w_tabs(wp)     (wp)->w__tabs
#define w_ismar(wp)    (wp)->w__rmar
#define w_iswrap(wp)   (wp)->w__rmar
#define w_lmar(wp)     (wp)->w__lmar
#define w_rmar(wp)     (wp)->w__rmar
#define w_filename(wp) (wp)->w__filename
#define w_pickname(wp) (wp)->w__pickname
#define w_msgno(wp)    (wp)->w__msgno
#define w_deftxt(wp)   (wp)->w__deftxt         

#define w_text(wp,i)    ((wp)->w__cache) [i]
#define w_setmod(wp,i)  ((wp)->w__modflgs) [i] = 1;
#define w_testmod(wp,i) ((wp)->w__modflgs) [i]
#define w_tabchar(wp,i) ((wp)->w__tabs) [i]

#ifdef VBLSCR
/* window size macros */
#define w_CO(wp) ((wp)->w__CO)
#define w_LI(wp) ((wp)->w__LI)
#endif

/* w_flags bits */

#define INDEXED          1
#define MODIFIED         2
#define HGANGED          4
#define SHOWTABS       010
#define NAMEFIELD      020
#define DOTTED         040
#define INVISIBLE     0100
#define DOUMOD        0200
#define USERRO        0400
#define CRSCROLL     01000
#define CHANGEHELPER 02000
#define NOZOOM       04000
#define RIGHTARROW  010000
#define CATENTRY    020000
#define DISPLAYONLY 040000

/* define for window structure type */
#define T_WSTRUCT 5


extern void wn_init (void);
extern void wn_free (WSTRUCT *);
extern void wn_print (WSTRUCT *, int);
extern int  wn_read (WSTRUCT *, FILE *);
extern int  wn_write (WSTRUCT *, FILE *);
extern void _wnprint(WSTRUCT *, int);
