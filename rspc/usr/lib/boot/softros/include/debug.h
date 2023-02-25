/* @(#)23	1.3  src/rspc/usr/lib/boot/softros/include/debug.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:16  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: dbgmsg
 *		dbgtst
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

#ifndef _DEBUG_H
#define _DEBUG_H


/**************************************************************************/
/****************** Define Debug Stuff  ***********************************/
/**************************************************************************/


#define AMON_D1		(1<<0)
#define AMON_D2  	(1<<1)
#define AMON_D3  	(1<<2)
#define AMON_D4  	(1<<3)
#define AMON_D5  	(1<<4)
#define AMON_D6		(1<<5)
#define AMON_D7		(1<<6)
#define AMON_D8		(1<<7)
#define AMON_D9		(1<<8)
#define AMON_D10	(1<<9)
#define AMON_D11	(1<<10)
#define AMON_D12	(1<<11)
#define AMON_D13	(1<<12)
#define AMON_D14	(1<<13)
#define AMON_D15	(1<<14)
#define AMON_D16	(1<<15)
#define AMON_D17	(1<<16)
#define AMON_D18	(1<<17)
#define AMON_D19	(1<<18)
#define AMON_D20	(1<<19)
#define AMON_D21	(1<<20)
#define AMON_D22	(1<<21)
#define AMON_D23	(1<<22)
#define AMON_D24	(1<<23)
#define AMON_D25	(1<<24)
#define AMON_D26	(1<<25)
#define AMON_D27	(1<<26)
#define AMON_D28	(1<<27)
#define AMON_D29	(1<<28)
#define AMON_D30	(1<<29)
#define AMON_D31	(1<<30)
#define AMON_D32	(1<<31)

#define dbgmsg(m,t)     if(aixmon_debug & m) printf("%s ====> %s\n",__FILE__,t)
#define dbgtst(m)       if(aixmon_debug & m)

#ifdef IN_MAIN

char*	dl_desc[33] = {
    "",					/* 0 - blank so we don't need offset */
    "Top level debugging - function names", 	/* 1 */
    "Main line debugging messages",		/* 2 */
    "Subroutine internal messages",		/* 3 */
    "Disassembler debugging", 			/* 4 */
    "Debug messages in hdf",			/* 5 */
    "Loadi debugging",				/* 6 */
    "AIXMON Install debugging",			/* 7 */
    "Keyboard debugging stuff",			/* 8 */
    "SCSI Exerciser",				/* 9 */
    "SCSI Initialization debug",		/* 10 */
    "Residual Data Debugging",			/* 11 */
    "",						/* 12 */
    "",						/* 13 */
    "",						/* 14 */
    "Boot header information debug",		/* 15 */
    "IPL control block offsets",		/* 16 */
    "",						/* 17 */
    "",						/* 18 */
    "",						/* 19 */
    "Debug only mode for JUMP command",		/* 20 */
    "",						/* 21 */
    "",						/* 22 */
    "",						/* 23 */
    "",						/* 24 */
    "",						/* 25 */
    "",						/* 26 */
    "",						/* 27 */
    "",						/* 28 */
    "",						/* 29 */
    "",						/* 30 */
    "",						/* 31 */
    ""						/* 32 */
    };

#ifdef DEBUG
int		aixmon_debug = 0xFFFFFFFF;
#else
int		aixmon_debug = 0;
#endif

#else /* IN_MAIN */

extern char*	dl_desc[33];
extern int	aixmon_debug;

#endif /* IN_MAIN */
#endif /*_DEBUG_H*/


