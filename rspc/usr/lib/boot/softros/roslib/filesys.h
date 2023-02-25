/* @(#)31	1.1  src/rspc/usr/lib/boot/softros/roslib/filesys.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:15  */
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

/*------------------------------------------------------------------------------*/
/*      Limited File System                                                     */
/*------------------------------------------------------------------------------*/

#define MAX_FILES   20                  /* Allow for internal opens of subdirs  */
#define NUM_BUFFERS 30
#define MAX_DRIVES  26

#define MAX_FILENAME_LEN 64

typedef struct FBUFFER {                /* File buffer                          */
   struct FBUFFER *next;                /* Next buffer Pointer                  */
   int          drive;                  /* Drive this block belongs on          */
   int          rba;                    /* Where on the disk this block belongs */
   int          dirty;                  /* Block needs to be written to disk    */
   char         bfr[512];
} FBUFFER;

typedef struct FNAME {
   char         drive;                  /* Drive Letter - defatult to curr drive*/
   char         pathname[MAX_FILENAME_LEN+2];/* Path, suitable for use in change_dir */
   char         FileName[10];           /* File name and ext of name found      */
   char         FileExt[4];             /*                                      */
} FNAME;


#define O_WRONLY 0x01
#define O_RDONLY 0x02

/*------------------------------------------------------------------------------*/
/*      Variables Used                                                          */
/*------------------------------------------------------------------------------*/
extern int  current_drive;
extern char current_dir[MAX_DRIVES][MAX_FILENAME_LEN+2];

/*------------------------------------------------------------------------------*/
/*      Functions Supported                                                     */
/*------------------------------------------------------------------------------*/
int     f_init(void);                   /* Initialize the file system           */

void    f_shutdown(void);               /* Close file system                    */

int     open(char * name,int flags, int perms) ;

int     creat(char * name, int perms) ;

long    lseek(int fd, long offset, int origin) ;

int     read(int fd, char *buf, int n) ;

int     write(int fd, char *buf, int n) ;

int     fs_flush(int fd) ;

int     close(int fd) ;

int     unlink(char *name) ;

int     _rename(const char *old_filename,/* Rename                              */
                const char *new_filename);

int     _mk_dir(const char *dirname);   /* Make the specified directory         */

int     _rm_dir(const char *dirname);   /* Remove the specified directory       */

int     parse_filename(const char *filename,FNAME *fn);

int     freespace(const char drive);    /* Return freespace on drive drive      */
