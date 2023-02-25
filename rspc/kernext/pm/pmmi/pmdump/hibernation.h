/* @(#)32       1.1  src/rspc/kernext/pm/pmmi/pmdump/hibernation.h, pwrsysx, rspc41J, 9510A 3/8/95 11:50:19 */
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
#ifndef _H_HIBERNATION
#define _H_HIBERNATION

#include <sys/types.h>

/*
 * hibernation header
 *
 * If this structure is changed, the hibernation header definition in
 * src/rspc/pm/wakeup/hrstart.m4 must be maintained.
 */
typedef struct _pm_hibernation_header {
	char	signature[8];		/* "AIXPM001" */
	uint	*wakeup_area;		/* physical address of wakeup area */
	int	wakeup_area_length;	/* length of the wakeup area */
	int	wakeup_code_offset;	/* offset of the wakeup code */
	int	wakeup_code_length;	/* length of the wakeup code */
	int	config_data_offset;	/* offset of the config data */
	int	config_data_length;	/* length of the config data */
	uint	*restart;		/* physical restart address */
	int	image_offset;		/* offset of compressed memory image */
	int	image_length;		/* length of compressed memory image */
	int	bitmap_offset;		/* offset of the bitmap data */
	int	bitmap_length;		/* length of the bitmap data */
	char	reserve[12];
} pm_hibernation_header_t;


/* config data */
typedef struct _pm_config_data {
	int	memory_size;		/* system memory size (byte) */
	int	residual_sum;		/* check sum of residual data */
} pm_config_data_t;

#endif /* _H_HIBERNATION */
