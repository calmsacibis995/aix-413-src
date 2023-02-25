/*NOTUSED*/
/* @(#)51	1.4  src/tenplus/include/libobj.h, tenplus, tenplus411, GOLD410 3/23/93 12:02:05 */

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
/*file: libobj.h*/

extern void c_print(char *, int);
extern int c_read(char *, FILE *);
extern c_write(char *, FILE *);
extern void debug(char *, ...);
extern void e_free(void);
extern void e_print(char *, int);
extern void e_read(void);
extern void e_write(void);
extern void fatal(char *, ...);
extern int fbackup(char *);
extern int fexists(char *);
extern int fgetline(FILE *, char *, int);
extern int geti(FILE *);
extern long getl(FILE *);
extern void i_print(int *, int);
extern int i_read(int *, FILE *);
extern int i_write(int *, FILE *);
extern int isdelta(char *);
extern void l_addtype(void);
extern void l_print(long *, int);
extern int l_read(long *, FILE *);
extern int l_write(long *, FILE *);
extern void nputc(char, FILE *, int);
extern void p_free(POINTER *);
extern void p_print(POINTER *, int);
extern int p_read(POINTER *, FILE *);
extern int p_write(POINTER *, FILE *);
extern void pindent(int);
extern void puti(int, FILE *);
extern void putl(long, FILE *);
extern void r_print(RECORD *, int);
extern int r_read(RECORD *, FILE *);
extern int r_write(RECORD *, FILE *);
extern int s_addtype(char *, int, void(*)(void *, int), int(*)(void *, FILE *),            int (*) (void *, FILE *), void(*)(void *));
extern char *s_alloc(int, int, char *);
extern POINTER *s_append(POINTER *, char *);
extern char *s_cat(char *, char *);
extern char *s_delete(char *, int, int);
extern char *s_findnode(POINTER *, char *);
extern int s_fixtree(POINTER *, char *, char *, int);
extern void s_free(char *);
extern void s_freenode(char *);
extern char *s_getline(FILE *);
extern POINTER *s_getmulti(POINTER *, char *);
extern char *s_getsingle(POINTER *, char *);
extern void s_indexname(char *, POINTER *, char *);
extern int s_inlist(char *, POINTER *);
extern char *s_insert(char *, int, int);
extern char *s_mknode(POINTER *, char *, int);
extern void s_newname(char *, char *);
extern POINTER *s_path(POINTER *, char *);
extern char *s_pathname(POINTER *, char *);
extern POINTER * s_pointer(char *);
extern POINTER *s_prefix(POINTER *, char *);
extern char *s_read(FILE *);
extern void s_print(char *);
extern char *s_realloc(char *, unsigned);
extern POINTER *s_reorder(POINTER *, POINTER *);
extern int s_settype(char *, int, int, void(*)(void *, int),                               int(*)(void *, FILE *),int(*)(void *, FILE *), void(*)(void *));
extern SHEADER *s_sheader(FILE *);
extern char *s_string(char *);
extern int s_typecheck(char *, char *, int);
extern int s_write(char *, FILE *);
extern char *scopy(char *);
extern int seq(char *, char *);
extern int sf_close(SFILE *);
extern void sf_cookie(SFILE *, int, ...);
extern int sf_delete(SFILE *, int, int);
extern void sf_free(SFILE *);
extern POINTER *sf_getfields(SFILE *, int, POINTER *);
extern POINTER *sf_getnames(SFILE *);
extern int sf_insert(SFILE *, int, int);
extern void sf_print(SFILE *, int);
extern char *sf_read(SFILE *, int);
extern void sf_seek(SFILE *, long, char);
extern SHEADER *sf_sheader(SFILE *, int);
extern void sf_setindex(SFILE *, int);
extern int sf_write(SFILE *, int, char *);
extern SFILE * sf_open(char *, char *);
extern void noop(void);

 
