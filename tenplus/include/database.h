/*NOTUSED*/
/* @(#)87	1.6  src/tenplus/include/database.h, tenplus, tenplus411, GOLD410 3/23/93 12:01:41 */

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
* file:  database.h                                                          *
*                                                                            *
* include file for database operations in the e2 editor                      *
*                                                                            *
* 22 June 1981                                                               *
*****************************************************************************/

#ifndef _IOREAD
#include <stdio.h>
#endif

/* the definition of the ATTR type */
typedef unsigned short ATTR;

#ifndef _NFILE
#define _NFILE 20   /* if it's not defined, 20 is plenty */
#endif

/* settable parameters */
#define NAMESIZE (256)    /* maximum size of names in allocated objects */
#define OBJSIZE  (65500)  /* maximum size of a structured object, governed by */
			  /* the fact that the count is stored in two bytes.  */
#define MAX_TYPES 25      /* maximum number of types allowed */
#define RESERVED_TYPES 10 /* number of types reserved to s_settype and compile
			     time generated types */

/* data type descriptor */
typedef struct 
  {
  char *dt__name;         /* data type name                             */
  int   dt__size;         /* size of object                             */
  void (*dt__printer)(void *, int); /* debugging routine to print object contents */
  int (*dt__reader)(void *, FILE *);  /* routine for reading object from file       */
  int (*dt__writer)(void *, FILE *);  /* routine for writing object to file         */
  void (*dt__freer)(void *);   /* routine for freeing data in object         */
    } DTYPE;

#define dt_name(dp)         ((dp)->dt__name)
#define dt_size(dp)         ((dp)->dt__size)
#define dt_printer(dp)      ((dp)->dt__printer)
#define dt_reader(dp)       ((dp)->dt__reader)
#define dt_writer(dp)       ((dp)->dt__writer)
#define dt_freer(dp)        ((dp)->dt__freer)

/* Datatypes currently defined, indicies in the g_dtypes array */
#define T_ANY       (-1)    /* used as argument to s_typecheck */
#define T_FREED     (0)     /* freed structured allocation        */
#define T_POINTER   (1)     /* pointer array to other allocations */
#define T_RECORD    (2)     /* array of record file locations     */
#define T_CHAR      (3)     /* character array                    */
#define T_SFILE     (4)     /* delta file object                  */
#define T_ATTR 6

/* Structured allocation structure */
typedef struct 
    {
    char      sh__type;      /* type bits for this allocation    */
    char      sh__flag;      /* flag bits for this allocation    */
    int       sh__count;     /* number of items allocated        */
    char     *sh__name;      /* name of this item                */
    char     *sh__data;      /* start of data for this object    */
    } SHEADER;

#define sh_type(shp)      ((shp)->sh__type)
#define sh_flag(shp)      ((shp)->sh__flag)
#define sh_count(shp)     ((shp)->sh__count)
#define sh_name(shp)      ((shp)->sh__name)
#define sh_data(shp)      ((char *) &(shp)->sh__data)

#define sh_getflag(shp,bits)    ((shp)->sh__flag & (bits))
#define sh_setflag(shp,bits)    ((shp)->sh__flag |= (bits))
#define sh_clearflag(shp,bits)  ((shp)->sh__flag &= ~(bits))

#define HEADERSIZE (sizeof (SHEADER) - sizeof (char *))

/* sheader macros for data pointers to objects */
#define obj_shp(objp)             ((SHEADER *)((char *)(objp) - HEADERSIZE))
#define obj_type(objp)            (sh_type (obj_shp (objp)))
#define obj_flag(objp)            (sh_flag (obj_shp (objp)))
#define obj_count(objp)           (sh_count (obj_shp (objp)))
#define obj_name(objp)            (sh_name (obj_shp (objp)))
#define obj_getflag(objp,bits)    (obj_flag (objp) & (bits))
#define obj_setflag(objp,bits)    (obj_flag (objp) |= (bits))
#define obj_clearflag(objp,bits)  (obj_flag (objp) &= ~(bits))
#define obj_fixflag(objp,bits)    (obj_flag (objp) = (bits))

/* pointer object */
typedef char *POINTER;

/* record structure */
typedef struct 
    {
    long    r__fileloc; /* file location for record (plus one) */
    } RECORD;

#define r_fileloc(rp)   ((rp)->r__fileloc)
#define NORECORD (0L)   /* zero file location means no record */
#define RC_NONTEXT (1)  /* record flag bit:  nontext record in file */

/* cookie types */
#define COOKIE      (0376)
#define C_INSERT    (1)     /* insert records        */
#define C_DELETE    (2)     /* delete records        */
#define C_INDEX     (3)     /* set current index     */
#define C_START     (4)     /* start information     */
#define C_END       (5)     /* end information       */
#define C_COMMENT   (6)     /* user comment          */
#define C_ARRAY     (7)     /* start of record array */
#define ERROR       (-1)
#define RET_OK 	    (1)

/* structured file structure */
typedef struct 
    {
    FILE   *sf__fp;         /* file pointer                     */
    RECORD *sf__records;    /* records array                    */
    int     sf__index;      /* current index in records array   */
    long    sf__eofloc;     /* current file location for writes */
    char    sf__uid;        /* user's user id                   */
    char    sf__gid;        /* user's group id                  */
    long    sf__time;       /* time the file was opened         */
    
    } SFILE;

#define sf_fp(sfp)          ((sfp)->sf__fp)
#define sf_records(sfp)     ((sfp)->sf__records)
#define sf_index(sfp)       ((sfp)->sf__index)
#define sf_eofloc(sfp)      ((sfp)->sf__eofloc)
#define sf_uid(sfp)         (((sfp)->sf__uid) & 0xff)
#define sf_gid(sfp)         (((sfp)->sf__gid) & 0xff)
#define sf_time(sfp)        ((sfp)->sf__time)

/* This is the value of sf_eofloc (sfp) when sfp points to an empty
   instance of a structured file */

#define EMPTYEOF 8L

/* sfile flag bits */
#define SF_READ     (1)     /* set if last i/o operation was a read */
#define SF_MODIFIED (2)     /* set if start cookie has been written */
#define SF_RONLY    (4)     /* set if file is read-only             */

/* global declarations */
extern DTYPE  g_dtypes [];  /* data type descriptor array  */
extern FILE  *g_debugfp;    /* debugging file pointer      */
extern int    g_errno;      /* error code for last error   */
extern int    g_flags;      /* global flag bits            */
extern void  (*g_allocfcn)(void);/* allocation failure function */
extern void  (*g_typefcn)(void); /* type check failure function */
extern int  (*g_diskfcn)(char *, char *);  /*full disk error handler     */

/* global flag bits */
#define g_getflag(bits)    (g_flags & (bits))
#define g_setflag(bits)    (g_flags |= (bits))
#define g_clearflag(bits)  (g_flags &= ~(bits))

#define G_COREDUMP  (1)     /* causes fatal to do an abort exit      */
#define G_REVERSE   (2)     /* do all tree searches in reverse order */

/* error code numbers */
#define E_NOERR     (0)     /* no error                                 */
#define E_IO        (1)     /* i/o error                                */
#define E_ALLOC     (2)     /* allocation failure                       */
#define E_MISC      (3)     /* miscellaneous error                      */
#define E_FOPEN     (4)     /* fopen failure                            */
#define E_FILE      (5)     /* file is not delta file                   */
#define E_ARRAY     (6)     /* file has broken record array             */
#define E_TYPECHECK (7)     /* non-fatal type check error               */
#define E_NOFIELD   (8)     /* field does not exist (for libprf)        */
#define E_BADFIELD  (9)     /* field contains invalid data (for libprf) */

#ifdef min
#undef min
#endif
#define min(x,y) (((x) < (y)) ? (x) : (y))
#ifdef max
#undef max
#endif
#define max(x,y) (((x) > (y)) ? (x) : (y))

