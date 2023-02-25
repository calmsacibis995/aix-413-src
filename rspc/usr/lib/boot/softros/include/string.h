/* @(#)30	1.1  src/rspc/usr/lib/boot/softros/include/string.h, rspc_softros, rspc411, GOLD410 4/18/94 15:53:36  */
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

#ifndef _STRING
#define _STRING

/*---------------------------------------------------------------------------
 *  Macros:
 */

#ifndef NULL
# define NULL		((void *)0)
#endif

/*---------------------------------------------------------------------------
 *  Types:
 */

#ifndef _SIZE_T
# define _SIZE_T
typedef unsigned long	size_t;
#endif

/*---------------------------------------------------------------------------
 *  Functions:
 */

extern void	*memchr(const void *s, int c, size_t n);
extern int 	memcmp(const void *s1, const void *s2,size_t n);
extern void	*memcpy(void *s1, const void *s2, size_t n);
extern void 	*memmove(void *s1, const void *s2, size_t n);
extern void	*memset(void *s, int c, size_t n);
extern char 	*strcat(char *s1, const char *s2);
extern char	*strchr(const char *s, int c);
extern int	strcmp(const char *s1, const char *s2);
extern int	strcoll(const char *s1, const char *s2);
extern char 	*strcpy(char *s1, const char *s2);
extern size_t 	strcspn(const char *s1, const char *s2);
extern char 	*strerror(int errnum);
extern size_t	strlen(const char *s);
extern char	*strncat(char *s1, const char *s2, size_t n);
extern int	strncmp(char *s1,const char *s2,size_t n);
extern char 	*strncpy(char *s1, const char *s2, size_t n);
extern char	*strpbrk(const char *s1, const char *s2);
extern char	*strrchr(const char *s, int c);
extern size_t	strspn(const char *s1, const char *s2);
extern char	*strstr(const char *s1, const char *s2);

#endif			/* _STRING */
