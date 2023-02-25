/* @(#)60	1.1  src/rspc/usr/lib/boot/softros/roslib/string.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:12  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
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
/* @(#)84 1.3 rom/string.h, libipl, sf100 8/3/93 15:49:42 */

/*---------------------------------------------------------------------------
 *
 *
 * 84
 *
 * Maintenance history:
 *
 *      24-Feb-93  M. McLemore          Original version, derived from AIX
 *                                      3.2 string.h header.
 */

#ifndef _STRING
#define _STRING

/*---------------------------------------------------------------------------
 *  Macros:
 */

#ifndef NULL
# define NULL           ((void *)0)
#endif

/*---------------------------------------------------------------------------
 *  Types:
 */

#ifndef _SIZE_T
# define _SIZE_T
typedef unsigned long   size_t;
#endif

/*---------------------------------------------------------------------------
 *  Functions:
 */

extern void     *memchr(const void *s, int c, size_t n);
extern int      memcmp(const void *s1, const void *s2,size_t n);
extern void     *memcpy(void *s1, const void *s2, size_t n);
extern void     *memmove(void *s1, const void *s2, size_t n);
extern void     *memset(void *s, int c, size_t n);
extern char     *strcat(char *s1, const char *s2);
extern char     *strchr(const char *s, int c);
extern int      strcmp(const char *s1, const char *s2);
extern int      strcoll(const char *s1, const char *s2);
extern char     *strcpy(char *s1, const char *s2);
extern size_t   strcspn(const char *s1, const char *s2);
extern char     *strerror(int errnum);
extern size_t   strlen(const char *s);
extern char     *strncat(char *s1, const char *s2, size_t n);
extern int      strncmp(const char *s1,const char *s2,size_t n);
extern char     *strncpy(char *s1, const char *s2, size_t n);
extern char     *strpbrk(const char *s1, const char *s2);
extern char     *strrchr(const char *s, int c);
extern size_t   strspn(const char *s1, const char *s2);
extern char     *strstr(const char *s1, const char *s2);
extern char     *strtok(char *s1, const char *s2);
extern size_t   strxfrm(char *s1, const char *s2, size_t n);
extern char     *strdup(const char *s1);
extern char     *strupr(char *s1);


#endif                  /* _STRING */
