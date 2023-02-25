/* @(#)73	1.5  src/tenplus/filters/fexterns.h, tenplus, tenplus411, GOLD410 7/18/91 12:38:08 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
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
 
/* =======================================================================
 *
 *  fexterns.h      -   Declarations and externs for format
 *
 * =======================================================================
 */

extern FILE * input ;                   /* input file pointer */
extern int  r_marg,                     /* left margin */
	    l_marg ;                    /* right margin */
extern char * bigblock ;                /* pointer to big-block if non-null */

    /*
     *  Keep this type definition consistent with the array cmd_array
     */
enum dirix
{
    D_NODIR,  /* (error) */
    D_1f,  D_1h,  D_bp,  D_br,  D_c,   D_ce,  D_cu,  D_cui,
    D_d,   D_db,  D_ds,  D_ef,  D_eh,  D_eo,  D_ep,  D_f,
    D_fb,  D_fe,  D_fi,  D_fk,  D_fm,  D_fp,  D_fr,  D_fs,
    D_ft,  D_hd,  D_hf,  D_hn,  D_hu,  D_hy,  D_j,   D_ke,
    D_kf,  D_ks,  D_lm,  D_ne,  D_nf,  D_nh,  D_nj,  D_opt,
    D_p1f, D_p1h, D_pb,  D_pf,  D_pl,  D_pn,  D_po,  D_rj,
    D_rm,  D_so,  D_sp,  D_ss,  D_u,   D_ue,  D_wu
} ;


	/*              Nodes in the tree ...
	 *      Major text structures:  text, line, word.
	 */
    /* The tree top:  one of these nodes is allocated for each "chunk" of text */
struct text
{
    struct line *t_head     ;   /* pointer to first line of this chunk     */
    struct line *t_tail     ;   /* -> last line of it                      */
    int          t_nrlines  ;   /* how many lines there are                */
 } ;

struct line
{   /* One of these nodes is allocated for each input line     */
    struct word *t_next     ;   /* pointer to first word on the line       */
    short        t_padding  ;   /* display length of the leading spaces    */
    short        t_length   ;   /* display length of this line             */
    struct line *t_nextline ;   /* pointer to next line in the list        */
    FLAGTYPE     t_flags    ;   /* status bits describing this line        */
    short        t_wordcount;   /* how many distinct words in line         */
    struct word *t_lastword ;   /* pointer to last word on the line        */
 } ;

struct word
{   /* One of these nodes is allocated for each input word     */
    struct word *t_next     ;   /* pointer to the next word on the line    */
    short        t_padding  ;   /* display width of blanks preceding this word */
    short        t_length   ;   /* true display width of this word         */
    char        *t_offset   ;   /* where can this word be found in core    */
    FLAGTYPE     t_flags    ;   /* status bits describing this word        */
    FLAGTYPE     t_cflags   ;   /* if a cmd, these flags are relevant      */
    enum   dirix t_cmdix    ;   /* command; relevant only if cflags has    *
				 *  either command flag set. `int' on PDP. *
				 */
 } ;



struct dir
{
    char       *dir_p ;            /* _pointer to the directive string */
    enum dirix  dir_e ;            /* the value from the _enum */
} ;

