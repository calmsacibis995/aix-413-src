/* @(#)28	1.1  src/rspc/usr/lib/boot/softros/include/stdio.h, rspc_softros, rspc411, GOLD410 4/18/94 15:53:30  */
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

#ifndef _STDIO
#define _STDIO

/*---------------------------------------------------------------------------
 *  Types:
 */

#ifndef _FPOS_T
#define	_FPOS_T
typedef long	fpos_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long	size_t;
#endif

typedef struct {
	unsigned char	*_ptr;
	int	_cnt;
	unsigned char	*_base;
	unsigned char   *_bufendp;
	short	_flag;
	short	_file;
	int	__stdioid;
	char	*__newbase;
	long	_unused[1];
} FILE;

/*---------------------------------------------------------------------------
 *  Macros:
 */

#define BUFSIZ		4096

#ifndef EOF
#define EOF		(-1)
#endif

#define FILENAME_MAX 	255
#define FOPEN_MAX 	2000
#define _P_tmpdir       "/tmp/"
#define L_tmpnam	(sizeof(_P_tmpdir) + 15)

#ifndef NULL
#define NULL		((void *)0)
#endif

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#ifndef TMP_MAX
#define	TMP_MAX		16384
#endif

#define _NIOBRW		20

/*
 * _IOLBF means that a file's output will be buffered line by line
 * In addition to being flags, _IONBF, _IOLBF and _IOFBF are possible
 * values for "type" in setvbuf.
 */
#define _IOFBF		0000
#define _IOLBF		0100
#define _IONBF		0004

#ifdef _POSIX_SOURCE
# define L_ctermid	9
# define L_cuserid	9
#endif

/*---------------------------------------------------------------------------
 *  Functions:
 */

#include<stdarg.h>

extern int 	getchar(void);
extern char 	*gets(char *s);
extern void 	perror(const char *s); 
extern int	printf(const char *format, ...); 
extern int 	putchar(int c);
extern int 	puts(const char *s);
extern int	scanf(const char *format, ...); 
extern int	sprintf(char *s, const char *format, ...); 
extern int	sscanf(const char *s, const char *format, ...); 

#endif		/* _STDIO */
