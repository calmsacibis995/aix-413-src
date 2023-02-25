/* @(#)34       1.1  src/rspc/kernext/pm/pmmi/pmdump/pmdump.h, pwrsysx, rspc41J, 9510A 3/8/95 11:50:22 */
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: Power Management Kernel Extension
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_PMDUMP
#define _H_PMDUMP

#define SECTOR_SIZE	(512)
#define	ONE_PAGE	(4096)
#define	DUMP_SIZE	(16*4096)
#define WAKEUP_CODE	"/usr/lib/drivers/pmwakeup"
#define SECTOR_ALIGN(x)	\
		((((x) + (SECTOR_SIZE-1)) / SECTOR_SIZE) * SECTOR_SIZE)

#define LEint(x)	(((x) & 0xff000000) >> 24) |    \
			(((x) & 0x00ff0000) >> 8)  |    \
			(((x) & 0x0000ff00) << 8)  |    \
			(((x) & 0x000000ff) << 24)

#define FREE_MEMORY(mem)			\
	if(mem != NULL){			\
		xmfree(mem, pinned_heap);	\
		mem = NULL;			\
	}

#define FREE_PAGE(addr)	\
		(((bitmap[addr/(ONE_PAGE*8)]) << ((addr/ONE_PAGE)%8)) & 0x80)

#define WAKEUP_AREA_LENGTH	(64 * 1024)			/* 64KB */

#define HEADER_DATA_SIZE	SECTOR_SIZE
#define CONFIG_DATA_SIZE	SECTOR_SIZE

#endif /* _H_PMDUMP */
