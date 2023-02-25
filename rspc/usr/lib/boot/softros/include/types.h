/* @(#)32	1.1  src/rspc/usr/lib/boot/softros/include/types.h, rspc_softros, rspc411, GOLD410 4/18/94 15:53:42  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: major
 *		makedev
 *		minor
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

#ifndef _TYPES
#define _TYPES

typedef unsigned char 	uchar;
typedef unsigned short 	ushort;
typedef unsigned int   	uint;
typedef unsigned long  	ulong;

/* typedefs for BSD unsigned things */
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;


typedef long            daddr_t;        /* disk address */
typedef char *          caddr_t;        /* "core" (i.e. memory) address */
typedef long            paddr_t;
typedef ulong		dev_t;          /* device number (major+minor) */
typedef ulong		mode_t;         /* file mode */
typedef short           nlink_t;
typedef long            off_t;          /* file offset */
typedef long            chan_t;         /* channel number (minor's minor) */

/* structure and typedef for volume group IDs and physical volume IDs */
/* Required to use AIX boot record				      */
struct unique_id {
       unsigned long word1;
       unsigned long word2;
       unsigned long word3;
       unsigned long word4;
};

typedef struct unique_id unique_id_t;
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long	size_t;
#endif

#ifndef NULL
#define NULL    	((void *)0)
#endif

#ifndef TRUE
#define TRUE    	1
#endif
#ifndef FALSE
#define FALSE   	0
#endif

/* major part of a device */
#define major(x)        (int)((unsigned int)(x) >> 16)

/* minor part of a device */
#define minor(x)        (int)((x) & 0xFFFF)

/* make a device number */
#define makedev(x,y)    (dev_t)(((x) << 16) | (y))

#endif			/* _TYPES */
