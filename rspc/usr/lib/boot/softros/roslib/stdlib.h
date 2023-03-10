/* @(#)59	1.1  src/rspc/usr/lib/boot/softros/roslib/stdlib.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:11  */

#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27,71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

#ifndef _H_STDLIB
#define _H_STDLIB

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 * The ANSI standard requires that certain values be in stdlib.h.
 * It also requires if _ANSI_C_SOURCE is defined then ONLY these
 * values are present. This header includes all the ANSI required entries.
 * In addition other entries for the XIX system are included.
 */

#ifdef _ANSI_C_SOURCE

/*
 * The following 3 definitions are included in <sys/types.h>.  They are
 * also included here to comply with ANSI standards.
 */

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long   size_t;
#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef unsigned short  wchar_t;
#endif

typedef struct div_t  {         /* struct returned from div     */
        int quot;               /* quotient                     */
        int rem;                /* remainder                    */
} div_t;

typedef struct ldiv_t  {        /* struct returned from ldiv    */
        long int quot;          /* quotient                     */
        long int rem;           /* remainder                    */
} ldiv_t;

#define EXIT_FAILURE   (1)              /* exit function failure        */
#define EXIT_SUCCESS    0               /* exit function success        */

#define RAND_MAX        32767           /* max value returned by rand   */

extern int __getmbcurmax (void);
extern int __getmaxdispwidth (void);

#define MB_CUR_MAX              (__getmbcurmax())
#define __max_disp_width        (__getmaxdispwidth())

/*
 * Functions that are methods
 */
#ifdef   _NO_PROTO
        extern int              mblen();
        extern size_t   mbstowcs();
        extern int              mbtowc();
        extern size_t   wcstombs();
        extern int              wctomb();
#else /* _NO_PROTO */
        extern int              mblen(const char *, size_t);
        extern size_t   mbstowcs(wchar_t *, const char *, size_t);
        extern int              mbtowc(wchar_t *, const char *, size_t);
        extern size_t   wcstombs(char *, const wchar_t *, size_t);
        extern int              wctomb(char *, const wchar_t);
#endif /* _NO_PROTO */

/*
 * Regular Functions no prototyping
 */
#ifdef _NO_PROTO
        extern double   atof();
        extern int      atoi();
        extern long int atol();
        extern double   strtod();
        extern long int strtol();
        extern unsigned long int strtoul();
        extern int      rand();
        extern void     srand();
        extern void     *calloc();
        extern void     free();
        extern void     *malloc();
        extern void     *realloc();
        extern void     abort();
        extern int      atexit();
        extern void     exit();
        extern char     *getenv();
        extern int      system();
        extern void     *bsearch();
        extern void     qsort();

/*
 * Undefine conflicting definition in  macros.h
 */
#ifdef _ALL_SOURCE
#ifdef abs
#undef abs
#endif
#endif /* _ALL_SOURCE */
//        extern int      abs();
        extern struct div_t     div();
        extern long int labs();
        extern struct ldiv_t    ldiv();

/*
 * Regular Functions with prototyping
 */
#else  /* _NO_PROTO */
/*
 * Any changes to the atof() prototype must also be made to the atof()
 * prototype in math.h.
 */
        extern double   atof(const char *);
        extern int      atoi(const char *);
        extern long int atol(const char *);
        extern double   strtod(const char *, char * *);
        extern long int strtol(const char *, char * *, int);
        extern unsigned long int strtoul(const char *, char * *, int);
        extern int      rand(void);
        extern void     srand(unsigned int);
        extern void     *calloc(size_t, size_t);
        extern void     free(void *);
        extern void     *malloc(size_t);
        extern void     *realloc(void *, size_t);
        extern void     abort(void);
        extern int      atexit(void (*)(void));
        extern void     exit(int);
        extern char     *getenv(const char *);
        extern int      system(const char *);
        extern void     *bsearch(const void *, const void *, size_t, size_t, int(*)(const void *,const void *));
        extern void     qsort(void *, size_t, size_t, int(*)(const void *,const void *));

/**********
 * Undefine conflicting definition in  macros.h
 */
#ifdef _ALL_SOURCE
#ifdef abs
#undef abs
#endif
#endif /* _ALL_SOURCE */

        extern int      abs(int);
        extern struct div_t     div(int, int);
        extern long int labs(long int);
        extern struct ldiv_t    ldiv(long int, long int);
#  endif /* _NO_PROTO */
#endif /* _ANSI_C_SOURCE */


/*
 * functions required by XOPEN
 */
#ifdef _XOPEN_SOURCE

#ifndef _H_WAIT
#include <sys/wait.h>   /* WNOHANG, WUNTRACED, WEXITSTATUS(), etc. */
#endif

#ifdef _THREAD_SAFE
/*
 * The following structure is used by the different thread_safe
 * *rand48_r() functions and must be passed in as an argument.
 */
 typedef struct drand48_data {
        unsigned x[3];                  /* 48 bit integer value */
        unsigned a[3];                  /* mutiplier value */
        unsigned c;                     /* addend value */
        unsigned short lastx[3];        /* previous value of Xi */
        int init;                       /* initialize ? */
} DRAND48D;
#endif  /* _THREAD_SAFE */

#ifdef _NO_PROTO
        extern double           drand48();
        extern double           erand48();
        extern long             jrand48();
        extern void             lcong48();
        extern long             lrand48();
        extern long             mrand48();
        extern long             nrand48();
        extern unsigned short   *seed48();
        extern void             setkey();
        extern void             srand48();
        extern int              putenv();
#ifdef _THREAD_SAFE
        extern int              getopt_r();
        extern  int     drand48_r();
        extern  int     erand48_r();
        extern  int     lrand48_r();
        extern  int     mrand48_r();
        extern  int     srand48_r();
        extern  int     seed48_r();
        extern  int     lcong48_r();
        extern  int     nrand48_r();
        extern  int     jrand48_r();
        extern  int     rand_r();
#endif  /* _THREAD_SAFE */
#else
        extern double           drand48(void);
        extern double           erand48(unsigned short[]);
        extern long             jrand48(unsigned short[]);
        extern void             lcong48(unsigned short int *);
        extern long             lrand48(void);
        extern long             mrand48(void);
        extern long             nrand48(unsigned short[]);
        extern unsigned short   *seed48(unsigned short[]);
        extern void             setkey(const char *);
        extern void             srand48(long);
        extern int              putenv( char *);
#ifdef _THREAD_SAFE
        extern int              getopt_r( int, char *[], char *, int *,
                                          char **, int *, int , int *,
                                          int *);
        extern  int     drand48_r(DRAND48D *dp, double *randval);
        extern  int     erand48_r(unsigned short xsubi[3], DRAND48D *dp,
                                  double *randval);
        extern  int     lrand48_r(DRAND48D *dp, long *randval);
        extern  int     mrand48_r(DRAND48D *dp, long *randval);
        extern  int     srand48_r(long seedval, DRAND48D *dp);
        extern  int     seed48_r(unsigned short seed16v[3], DRAND48D *dp);
        extern  int     lcong48_r(unsigned short param[7], DRAND48D *dp);
        extern  int     nrand48_r(unsigned short xsubi[3], DRAND48D *dp,
                                  long *randval);
        extern  int     jrand48_r(unsigned short xsubi[3], DRAND48D *dp,
                                  long *randval);
        extern  int     rand_r(unsigned int *, int *);
#endif  /* _THREAD_SAFE */
#endif /* _NO_PROTO */

#endif /* _XOPEN_SOURCE */

/*
 * The following macro definitions cause the XLC compiler to inline
 * these functions whenever possible.
 */

/*
 * Undefine conflicting definition in  macros.h
 */

#ifdef _ALL_SOURCE
#ifdef abs
#undef abs
#endif
#endif /* _ALL_SOURCE */

#ifdef __MATH__
#define abs(__j)          __abs(__j)
#define labs(__j)         __labs(__j)
#endif /* __MATH__ */

/*
 * Definition of functions and structures used by the thread-safe
 * random_r() functions.
 */
#if defined(_ALL_SOURCE) && defined(_THREAD_SAFE)
typedef struct random_data {
        long    *fptr;
        long    *rptr;
        long    *state;
        int     rand_type;
        int     rand_deg;
        int     rand_sep;
        long    *end_ptr;
} RANDOMD;

/* functions */

#ifdef _NO_PROTO
extern  int     srandom_r();
extern  int     initstate_r();
extern  int     setstate_r();
extern  long    random_r();
#else
extern  int     srandom_r(unsigned, RANDOMD *);
extern  int     initstate_r(unsigned, char *, int, char **,RANDOMD *);
extern  int     setstate_r(char *, char **,  RANDOMD *);
extern  int     random_r(long *, RANDOMD *);
#endif /* _NO_PROTO */

#endif /* _ALL_SOURCE && _THREAD_SAFE */

/*
 * The following function prototypes are not defined for programs
 * that are adhering to strict ANSI conformance, but are included
 * for floating point support.
 */

#ifdef _ALL_SOURCE

#ifndef _H_TYPES
#include <sys/types.h>
#endif

#ifndef __H_LOCALEDEF
#include <sys/localedef.h>
#endif

/*
 * Redefine MB_CUR_MAX and __max_disp_width to not call the functions
 */
#undef MB_CUR_MAX
#undef __max_disp_width

#define MB_CUR_MAX              (__OBJ_DATA(__lc_charmap)->cm_mb_cur_max)
#define __max_disp_width        (__OBJ_DATA(__lc_charmap)->cm_max_disp_width)


extern char *optarg;
extern int optind;
extern int opterr;

float    atoff();
char     *ecvt();
char     *fcvt();
char     *gcvt();
float    strtof();
void     imul_dbl();
void     umul_dbl();

#if (defined (__MATH__) &&  defined (__XLC121__))
#define imul_dbl(__a, __b, __c)   __imul_dbl(__a, __b, __c)
#define umul_dbl(__a, __b, __c)   __umul_dbl(__a, __b, __c)
#endif

#ifdef _NO_PROTO
/*
 * functions that are methods
 */
        extern double   wcstod();
        extern long int wcstol();
        extern unsigned long int wcstoul();

        extern int              rpmatch();
        extern int              clearenv();
        extern int              getopt();
        extern char             *getpass();
#else
        extern double    wcstod(const wchar_t *, wchar_t **);
        extern long int  wcstol(const wchar_t *, wchar_t **, int);
        extern unsigned long int wcstoul(const wchar_t *, wchar_t **, int);

        extern int              rpmatch(const char *);
        extern int              clearenv(void);
        extern int              getopt(int, char *[], char *);
        extern char             *getpass(const char *);
#endif /* _NO_PROTO */

#endif /* _ALL_SOURCE */

/*
 * __XLC121__ is automatically defined by the XLC 1.2.1 compiler so that
 * the compiler can inline the following functions when possible.
 */

/*
 * The following macros are commented out because they cause problem when
 * assigned to a variable of type div_t.  The compiler would complain that
 * the types are incompatible.

#if (defined (__MATH__) &&  defined (__XLC121__))
#define div(__numer,__denom)        __div(__numer,__denom)
#define ldiv(__numer,__denom)       __ldiv(__numer,__denom)
#endif

*/

#endif /* _H_STDLIB */
