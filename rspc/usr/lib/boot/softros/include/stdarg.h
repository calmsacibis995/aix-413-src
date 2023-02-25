/* @(#)26	1.1  src/rspc/usr/lib/boot/softros/include/stdarg.h, rspc_softros, rspc411, GOLD410 4/18/94 15:53:24  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: va_arg
 *		va_end
 *		va_start
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

#ifndef _STDARG
#define _STDARG

/* Argument list convention: Each argument is allocated a multiple of 4 bytes.
 * Each argument occupies the high-order bytes within its allocated multiple
 * of 4 bytes.  This implementation is restricted to one-quantity arguments.
 * The implementation does not support argument descriptors, which immediately
 * follow their respective argument value word(s) if they are used.
 */

#define va_arg(list, mode) ((mode *)( (((list)+=(((sizeof(mode)+3)/4)*4))-sizeof(mode)) ))[0]

#define va_end(list) (list)=(char *)0

#ifndef	_VA_LIST
#define _VA_LIST
typedef char *	va_list;
#endif

#define va_start(list,parmN) list = (char *)((unsigned int)&(parmN) + (((sizeof(parmN)+3)/4)*4))


#endif			/* _STDARG */
