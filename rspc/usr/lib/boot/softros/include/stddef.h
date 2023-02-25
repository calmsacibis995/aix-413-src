/* @(#)27	1.1  src/rspc/usr/lib/boot/softros/include/stddef.h, rspc_softros, rspc411, GOLD410 4/18/94 15:53:27  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: MAX
 *		MIN
 *		offsetof
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
