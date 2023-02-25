/* @(#)55	1.1  src/rspc/usr/lib/boot/softros/roslib/stdarg.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:03  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: va_arg
 *		va_end
 *		va_start
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
/* @(#)77 1.1 rom/stdarg.h, libipl, sf100 5/13/93 14:09:10 */

/*---------------------------------------------------------------------------
 *
 *
 * 77
 *
 * Maintenance history:
 *
 *	24-Feb-93  M. McLemore		Original version, derived from AIX
 *                                      3.2 stdarg.h header.
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
