/* @(#)43	1.3  src/tenplus/filters/filters.h, tenplus, tenplus411, GOLD410 7/18/91 12:38:22 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
This header file contains prototypes of all functions defined
in the filters directory, except for those defined in rpl.c.
*/
 
void cmd_line( int , char ** );
void freetree( struct text * );
enum dirix det_cmd_type(wchar_t *);
int det_brack_type( wchar_t );
struct word * proc_word_node(void );
struct line * proc_non_dot(void);
struct line * proc_dot_cmd(void);
struct word * proc_bs_cmd(void );
boolean is_open_bracket(wchar_t);
extern void upd_apbo_flags( struct line * );
void output( struct text * );
void det_eos_sp(struct line *, struct line *);
void parse_drive(void);
void apply(boolean , FLAG_TYPE , struct text *);
void det_und(struct line * , struct line * ) ;
extern void do_default_action( struct text * );
FILE * filename(char *);
struct text * fill( struct text * );
struct text * just( struct text * );
void init(void );
void clean(void);
char * getline(void);
void ungetstr ( char *);
char * newstr(char * );
char * xalloc(int );
char * xrealloc(char *, int );
