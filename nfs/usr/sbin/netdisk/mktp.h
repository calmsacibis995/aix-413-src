/* @(#)70	1.1  src/nfs/usr/sbin/netdisk/mktp.h, cmdnfs, nfs411, GOLD410 10/26/89 15:25:24 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: IS_TOC 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 * 
 */

/*
 * mktp.h - include file for mktp program
 */
#include "toc.h"

distribution_info dinfo;	/* distribution information             */
volume_info vinfo;		/* volume info - not used until actually*/
				/* making the distribution              */
#define	NENTRIES	100
toc_entry entries[NENTRIES];	/* pointer to first toc entry           */
toc_entry *ep;			/* pointer to one above last toc entry  */


/*
 * Progress of stuff. This is tracked thru dinfo.dstate.
 *
 */

#define	PARSED	0x01	/* input just been parsed	*/
#define	SIZED	0x02	/* sizes calculated		*/
#define	FITTED	0x04	/* files have had initial fitting into volumes*/
#define	EDITTED	0x08	/* files have been editted	*/

#define	READY_TO_GO	(PARSED|SIZED|FITTED)
#define	GONE	0	/* when it goes to the actual media,	*/
			/* dinfo.dstate is marked GONE		*/

/*
 * helpful defines
 *
 */


#define	IS_TOC(p) ((p)->Type == TOC && \
			(p)->Info.kind == CONTENTS \
			&& strcmp((p)->Name,"XDRTOC") == 0)

/*
 * Function declarations
 */

extern char *newstring();
extern void remove_toc_copies(), remove_entry(), destroy_entry(),
	destroy_all_entries(), dup_entry(), destroy_device();
extern void bell(), errprint(), infoprint();
