/* @(#)25	1.3  src/rspc/usr/lib/boot/softros/include/stand.h, rspc_softros, rspc411, GOLD410 6/13/94 16:04:32  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: SectorSize
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

#ifndef _STAND
#define _STAND

#include <types.h>
#include <termios.h>

/*
 *  Prog_t structure: passed to C runtime environment to indicate start
 *  and size of image being executed.
 */

typedef struct {
        long    start;                  /* where image starts */
        long    size;                   /* how big it is */
        long    execaddr;               /* where it is to be run */
        long    pad[ 3 ];
 } prog_t;

/*----------------------------------------------------------------------------
 *  Device Support:
 *      The standalone environment currently supports the console and floppy;
 *      later support can be added for hard disks, CD ROM, and network.
 *
 *  Device major numbers:
 */

# define CONSOLE        0               /* attached terminal */
# define FLOPPY         1               /* 1.44 MB floppy drive */
# define HARDDISK       2               /* hard disk drive */
# define CD_ROM         3               /* CD ROM drive */
# define NETWORK        4               /* network device */

/*
 *  Device Descriptor:
 */

typedef struct {                /* device descriptor */
        int     open;           /* open flag */
        caddr_t addr;           /* current address (for read/write/seek) */
        int     oflags;         /* open flags */
        dev_t   dev;            /* device number */
        struct termios t;       /* termios structure */
} DevDesc;

/*
 *  Macros for accessing devices:
 */

# define DDESC          (_iob[ fildes ])

# define stdin          0               /* all tied to console */
# define stdout         1
# define stderr         2

# define SectorSize( dev )      512     /* fixed at 512 bytes/sector for now */

/*
 *  Open flags:
 */

#define O_RDONLY        0
#define O_WRONLY        1
#define O_RDWR          2
#define O_NONBLOCK      4

/*
 *  lseek whence values:
 */

#define SEEK_SET        0
#define SEEK_CUR        1

/*
 *  Function prototypes for standalone facilities:
 */

int open( dev_t dev, int oflag, mode_t perms );
int read( int fildes, void *buf, unsigned int nbyte );
int write( int fildes, const void *buf, unsigned int nbyte );
int close( int fildes );
off_t lseek( int fildes, off_t offset, int whence );

char *bcopy( char *src, char *dest, int len );
void bzero( char *ptr, int len );
void delay( long milliseconds );
void soft_reboot( void );
char *sbrk( int n );
void display_memory( char *buffer, int length, int offset );

void prog_exec( dev_t dev, daddr_t da, size_t size, caddr_t memaddr, char **argv, char **envp );
int  prog_call( dev_t dev, daddr_t da, size_t size, caddr_t memaddr, char **argv, char **envp );
int  prog_load( dev_t dev, daddr_t da, size_t size, caddr_t memaddr );
int  prog_run( prog_t *progp, char **argv, char **envp );

/* Start of standalone definitions for routines in support of AIX            */
unsigned int read_msr();
void set_msr(unsigned int);
unsigned int read_segreg(int);
void set_segreg(int, unsigned int);
unsigned int read_pvr();
unsigned int read_pir();
void flush_dcache(unsigned int);
void flush_fullcache();
int read_icache_line_size();
int read_dcache_line_size();

#endif                  /* _STAND */
