/* @(#)29	1.1  src/rspc/usr/lib/boot/softros/include/stdlib.h, rspc_softros, rspc411, GOLD410 4/18/94 15:53:33  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: atexit
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _STDLIB
#define _STDLIB

/*---------------------------------------------------------------------------
 *  Macros:
 */

#define EXIT_FAILURE	1		/* exit function failure	*/
#define EXIT_SUCCESS	0		/* exit function success	*/
#define MB_CUR_MAX	1		/* max one byte per multibyte char */
#define RAND_MAX	32767		/* max value returned by rand	*/

#ifndef NULL
#define NULL		((void *)0)
#endif

#ifndef TRUE
# define TRUE		1
#endif

#ifndef FALSE
# define FALSE		0
#endif


/*---------------------------------------------------------------------------
 *  Types:
 */

typedef struct div_t  {		/* struct returned from div	*/
    int quot;			/* quotient			*/
    int rem;			/* remainder			*/ 
} div_t;

typedef struct ldiv_t  {	/* struct returned from ldiv	*/
    long int quot;		/* quotient			*/
    long int rem;		/* remainder			*/ 
} ldiv_t;

#ifndef _SIZE_T
# define _SIZE_T
    typedef unsigned long	size_t;
#endif /* ifndef _SIZE_T */

#ifndef	_WCHAR_T
# define _WCHAR_T
    typedef unsigned short	wchar_t;
#endif /* ifndef _WCHAR_T */


/*---------------------------------------------------------------------------
 *  Functions:
 */

extern int		atexit(void (*func)(void));
extern int 		atoi(const char *nptr);
extern long int 	atol(const char *nptr);
extern void 		*calloc(size_t nmemb, size_t size);
extern void		exit(int status);
extern void		free(void *ptr);
extern void		*malloc(size_t size);
extern int 		rand(void);
extern void 		*realloc(void *ptr, size_t size);
extern void		srand(unsigned int seed);

#endif			/* _STDLIB */
