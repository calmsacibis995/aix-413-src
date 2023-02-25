/*NOTUSED*/
/* @(#)54	1.4  src/tenplus/include/libutil.h, tenplus, tenplus411, GOLD410 3/23/93 12:02:11 */

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

/*file: libutil.h*/
#include <nl_types.h>

extern char    *abspath (char *, char *);
extern char    *a2string (ATTR *, int);
extern char    *backname (char *);
extern char    *fakename (char *);
extern char    *findfile (char *, char *, POINTER *);
extern char    *Lputregion (int, int, int, int);
extern char    *makestring (char *);
extern char    *p2string (POINTER *);
extern char    *packup (ATTR *);
extern char    *pathcat (char *, char *);
extern char    *uniqname (int);

extern int     ghost(char *);
extern int     gmatch(char *, char *);
extern int     ingroup(int);
extern int     Llock (char *);
extern int     Lunlock (char *);

extern void    Lgetregion (char *, int *, int *, int *, int *);
extern void    smear (char, int, char *);

extern ATTR    *unpackup (char *, int);
extern POINTER *s2pointer (char *);
extern POINTER *treecopy( POINTER *);

int isattrspace(ATTR *);
void skipattrspaces(ATTR **);
void skipattr(ATTR **);
void skipattr_idx(ATTR *, int *);
void backupattr(ATTR **);
void backupattr_idx(ATTR *, int *);

int ismbspace(char *);
void skipmbspaces(char **);
void skipmb(char **);
void blank_trailing_underbars(char *);
void attrcpy(ATTR **, ATTR **);
wchar_t attr_to_wc(ATTR *);
int mb_to_attr(ATTR **, char **);
void wc_to_mbs(char **, wchar_t);
int ismbalnum(char *);
void mbcpy(char **, char **);
int is_wspc(wchar_t);
int is_wspc_or_wtab(wchar_t);
int attr_text_width(ATTR *);
int calc_column (ATTR *, int );
int char_width (ATTR *);
int text_width (char *);
int calc_line_position (ATTR *, int );
int calc_line_position2 (ATTR *, int , int *);
int attr_text_width(ATTR *);
int attrlen(ATTR *);
int calc_line_position3 (ATTR *, int );
ATTR *overwrite_char(ATTR *, int , ATTR , ATTR );
POINTER *catscan(POINTER *, nl_catd, int);
int copyprofile( char *, char *);
extern	char *nls_path(char *);
