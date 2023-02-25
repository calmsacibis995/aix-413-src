/* @(#)66	1.24  src/bos/usr/sbin/backbyname/util.h, cmdarch, bos411, 9438C411a 9/23/94 18:01:35 */
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27, 9
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* utility routines common to backup and restore */
/* this file is #included by both backup.c (#define BR_BACKUP) and
   restore.c (#define BR_RESTORE) */

#include	<unistd.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<ctype.h>
#include	<sys/stat.h>
#include 	<sys/termio.h>

extern int  breakout();

#define WRDS_BLOCK (512 / BPW)
#define WRDS_CLUSTER (cluster * WRDS_BLOCK)

/*
 *      Re-order the bytes of data dependent on cpu type;
 *      the operation is self-inverting; i.e., it doesn't matter whether
 *      data is to be converted from host order to target order or v.v.
 *
 *      The types are:
 *
 *           0:  lo byte is first memory-order byte, hi byte is last
 *           1:  bytes are swapped relative to type 0
 *           2:  words are swapped relative to type 0
 *           3:  bytes and words are swapped relative to type 0
 *
 * RETURN VALUE DESCRIPTION:
 *
 * The B(), S(), L(), XXlong(), and XXshort() stuff was taken from
 * libIN/XX.c so that cmdarch could remove its dependencies on the
 * libIN library.
 */

#define B(n)        ((unsigned char *)&data)[n]
#define S(a,b)      ( (a)<<8 | (b) )
#define L(a,b,c,d)  S(S((long)S(a,b),c),d)

long
XXlong ( data, cpu )
        long data;
{
        switch( cpu & 3 )
        {   default: return L( B(3), B(2), B(1), B(0) );
            case 1:  return L( B(2), B(3), B(0), B(1) );
            case 2:  return L( B(1), B(0), B(3), B(2) );
            case 3:  return L( B(0), B(1), B(2), B(3) );
        }
}

short
XXshort( data, cpu )
        short data;
{
        if( cpu & 1 )
            return S( B(0), B(1) );
        else
            return S( B(1), B(0) );
}

/* data for buffering routines */

dword   *buf_start = NULL;     /* where the data goes */
dword   *buf_save  = NULL;      /* place to save the fragments */
dword   *buf_ptr   = NULL;       /* current position */
unsigned buf_len;       /* length of buffer in dwords */
unsigned fragleft;	/* dwords of fragments */
unsigned long buf_1stwd;      /* word number of beginning of buffer */

#ifdef BR_BACKUP
long    blocks_tot;     /* total number of blocks written to medium */
daddr_t high_wd;        /* highest valid word written */
#endif
long    tsize;          /* tape length in feet */
long    density;        /* tape density in bytes per inch */

#ifdef BR_RESTORE
dword   *buf_back;      /* point before or at buf_start where data is valid */
unsigned buf_actual;    /* actual amount of data read */
int  buf_newvol;        /* affects buffering around volume switches */
#endif

unsigned cluster;       /* cluster size in 512-byte blocks */
long    clus_vol;       /* number of clusters on output volume */
long    wrds_vol;       /* number of BPW-byte words on volume */
unsigned blks2use;      /* limit to blks2use sectors if non-zero */

struct devinfo devinfo;          /* medium device information */
struct devinfo FSstuff;          /* File System information */
unsigned char diskette;          /* is medium a diskette? */
unsigned char blk_limit;         /* user specified block limit */

/* tell where we are in the output buffer */
#define buf_tell() (buf_1stwd + (buf_ptr - buf_start))

#ifdef BR_BACKUP
#define buf_full() (buf_ptr >= buf_start + buf_len)
#define buf_empty() (buf_ptr == buf_start)
#define buf_append(dwp) (*buf_ptr++ = *(dwp))
#define buf_fits(ndw) (buf_tell() + (ndw) <= wrds_vol)
#endif

#ifdef BR_RESTORE
#define EXTRA (RDTMAX/sizeof(dword))
#else
#define EXTRA 0
#endif
