/* @(#)22	1.1  src/rspc/usr/lib/boot/softros/include/ctype.h, rspc_softros, rspc411, GOLD410 4/18/94 15:53:13  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: none
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

#ifndef _H_TYPES
#define _H_TYPES
#include        <types.h>
#endif

#ifndef _CTYPE
#define _CTYPE

extern int	isalpha(int c);
extern int	isalnum(int c);
extern int	iscntrl(int c);
extern int	isdigit(int c);
extern int	isgraph(int c);
extern int	islower(int c);
extern int	isprint(int c);
extern int	ispunct(int c);
extern int	isspace(int c);
extern int	isupper(int c);
extern int	isxdigit(int c);
extern int	toupper(int c);
extern int	tolower(int c);

#endif			/* _CTYPE */

