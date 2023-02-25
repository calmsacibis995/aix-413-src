/* @(#)57	1.1  src/rspc/usr/lib/boot/softros/roslib/stdio.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:07  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: feof
 *		ferror
 *		fileno
 *		getc
 *		putc
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
/* @(#)83 1.8 rom/stdio.h, libipl, sf100 10/6/93 13:53:18 */

/*---------------------------------------------------------------------------
 *
 *
 * 83
 *
 * Maintenance history:
 *
 *      24-Feb-93  M. McLemore          Original version, derived from AIX
 *                                      3.2 stdio.h header.
 *      01-Aug-93  M. Turner            Modified version for Dakota I/O
 *
 */

#ifndef _STDIO
#define _STDIO

/*---------------------------------------------------------------------------
 *  Types:
 */

#ifndef _FPOS_T
#define _FPOS_T
typedef long    fpos_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long   size_t;
#endif

#define BUFSIZE 1024

typedef struct _iobuf {
   int  cnt;                            /* Characters left in buffer            */
   char *ptr;                           /* Next Character Position              */
   char *base;                          /* location of buffer                   */
   int  flag;                           /* mode of file access                  */
   int  fd;                             /* File Descriptor                      */
} FILE;


#define FILE_RDONLY 0x01
#define FILE_HIDDEN 0x02
#define FILE_SYSTEM 0x04
#define FILE_LABEL  0x08
#define FILE_DIR    0x10
#define FILE_ARCH   0x20


typedef struct FIND_T {                 /* Find First/Find Next information blk */
   int          next_place;             /* Offset into directory to start next  */
   int          this_place;             /* Directory being searched             */
   int          drive;
   int          FileDate;               /* Found File information: Date         */
   int          FileTime;               /*                         Time         */
   int          FileSize;               /*                         Size         */
   int          FileAlloc;              /*                         Start Cluster*/
   char         FileAttr;               /*                         Attribute    */
   char         FileNameExt[14];        /* "????????.???",0   File name found   */
   char        *SearchName;             /* Fully qualified path to search       */
   char         attribute;              /* Attributes to search for             */
} FIND_T;

/*---------------------------------------------------------------------------
 *  Macros:
 */

#define BUFSIZ          4096

#ifndef EOF
#define EOF             (-1)
#endif

#define FILENAME_MAX    255
#define FOPEN_MAX       20
#define _P_tmpdir       "\\tmp\\"
#define L_tmpnam        (sizeof(_P_tmpdir) + 15)

#ifndef NULL
#define NULL            ((void *)0)
#endif

#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

#ifndef TMP_MAX
#define TMP_MAX         16384
#endif

#define _NIOBRW         20

/*
 * _IOLBF means that a file's output will be buffered line by line
 * In addition to being flags, _IONBF, _IOLBF and _IOFBF are possible
 * values for "type" in setvbuf.
 */
#define _IOFBF          0000
#define _IOLBF          0100
#define _IONBF          0004

#ifdef _POSIX_SOURCE
# define L_ctermid      9
# define L_cuserid      9
#endif

extern FILE _iob[FOPEN_MAX];

#define stdin   (&_iob[0])
#define stdout  (&_iob[1])
#define stderr  (&_iob[2])

enum _file_flags {
   _READ   = 01,                /* File open for reading */
   _WRITE  = 02,                /* File open for writing  */
   _UNBUF  = 04,                /* File is unbuffered */
   _EOF    = 010,               /* EOF has occurred on this file */
   _ERR    = 020                /* An error occurred on this file */
};

int _fillbuf(FILE *);
int _flushbuf(int, FILE *);

#define feof(p)   (((p)->flag & _EOF) != 0)
#define ferror(p) (((p)->flag & _ERR) != 0)
#define fileno(p) ((p)->fd)

#define getc(p)   (--(p)->cnt >= 0 ? (unsigned char) *(p)->ptr++ : _fillbuf(p))
#define putc(x,p) (--(p)->cnt >= 0 ? *(p)->ptr++ = (x) : _flushbuf((x),p))

/*#define getchar()   getc(stdin) */
/*#define putchar(x)  putc((x),stdout) */

int  getchar(void);                     /* Read a character from console        */

int  putchar(unsigned char);        /* Write a character to console         */



/*---------------------------------------------------------------------------
 *  Functions:
 */

#include<stdarg.h>

extern int      getchar(void);
extern char     *gets(char *s);
extern void     perror(const char *s);
extern int      printf(const char *format, ...);
/* extern int   putchar(int c); */
extern int      puts(const char *s);
extern int      scanf(const char *format, ...);
extern int      sprintf(char *s, const char *format, ...);
extern int      fprintf(FILE *fp, const char *format, ...);
extern int      sscanf(const char *s, const char *format, ...);
extern int      vprintf(const char *format, va_list arg);
extern int      vsprintf(char *s, const char *format, va_list arg);



/*--------------------------------------------------------------------------------------*/
/*      File Access Routines                                                            */
/*--------------------------------------------------------------------------------------*/
FILE   *fopen(const char *filename,     /* Open this file                       */
              const char *mode);        /* this way                             */

int     fflush(FILE    *fp);            /* Flush unwritten data                 */

int     fclose(FILE    *fp);            /* Close this open file                 */

int     fseek(FILE     *fp,             /* Repostion the file pointer           */
              long      offset,         /* to this location from                */
              int       origin);        /* this part of the file                */

long    ftell(FILE     *fp);            /* Return current offset in file        */

void    rewind(FILE    *fp);            /* Reposition file pointer to beginning */

int     fgetc(FILE     *fp);            /* Get a character from file            */

int     fgets(char * s,
              int n,
              FILE     *fp);            /* Get a string from FILE               */

int     fputc(int       c,              /* Put this character into file         */
              FILE     *fp);

int     fputs(char     *s,              /* Put a string to a FILE               */
              FILE     *fp);

int     fread(void     *bfr,            /* Read into this buffer                */
              int       size,           /* This size item                       */
              int       n_obj,          /* This many items                      */
              FILE     *fp)    ;        /* from this file                       */

int     fwrite(void     *bfr,           /* Write from this buffer               */
               int       size,          /* This size item                       */
               int       n_items,       /* This many items                      */
               FILE     *fp);           /* to this file                         */

int     remove(const char *filename);   /* Erase a file                         */

int     rename(const char *old_filename,/* Erase a file                         */
               const char *new_filename);

void    clearerr(FILE *fp);             /* Clears EOF and ERROR indicators      */


/*--------------------------------------------------------------------------------------*/
/*      DOS Like Routines for searching directories                                     */
/*--------------------------------------------------------------------------------------*/
int     ffind_first(FIND_T *fileinfo);  /* Perform a directory lookup           */

int     ffind_next(FIND_T *fileinfo);   /* Perform a directory lookup           */

int     change_dir(const char *dirname);/* Change to the directory specified    */

int     mk_dir(const char *dirname);    /* Make the specified directory         */

int     rm_dir(const char *dirname);    /* Remove the specified directory       */

#endif          /* _STDIO */
