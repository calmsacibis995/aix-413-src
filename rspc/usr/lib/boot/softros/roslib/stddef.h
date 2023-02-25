/* @(#)56	1.1  src/rspc/usr/lib/boot/softros/roslib/stddef.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:05  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: MAX
 *		MIN
 *		offsetof
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* @(#)78 1.1 rom/stddef.h, libipl, sf100 5/13/93 14:10:26 */

/*
 *
 * 78
 *
 * Maintenance history:
 *
 *	24-Feb-93  M. McLemore		Original version, derived from AIX
 *                                      3.2 stddef.h header.
 *	17-Mar-93  M. McLemore		Added MAX, MIN, TRUE, FALSE.
 */

#ifndef _STDDEF 
#define _STDDEF 

/*---------------------------------------------------------------------------
 *  Macros:
 */

#ifndef NULL
#define NULL		((void *)0)
#endif

#ifndef MAX
# define MAX( a, b ) 	((( a ) > ( b )) ? ( a ) : ( b ))
#endif
#ifndef MIN
# define MIN( a, b ) 	((( a ) < ( b )) ? ( a ) : ( b ))
#endif
#ifndef TRUE
# define TRUE	1
#endif
#ifndef FALSE
# define FALSE	0
#endif

#define offsetof(s_name, s_member) (size_t)&(((s_name *)0)->s_member)

/*---------------------------------------------------------------------------
 *  Types:
 */

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long	size_t;
#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef unsigned short	wchar_t;
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef	int		ptrdiff_t;
#endif

#endif 	/* _STDDEF */
