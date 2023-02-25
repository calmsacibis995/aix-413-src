static char sccsid[] = "@(#)42  1.1  src/rspc/kernext/pm/pmmi/pmdump/x41head.c, pwrsysx, rspc41J, 9510A 3/8/95 11:50:35";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: get_x41head
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
#include <sys/device.h>
#include <sys/dump.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include "x41.h"
#include "pmdump.h"
#include "pmdumpdebug.h"

/*
 * NAME:  get_x41head
 *
 * FUNCTION:  gets x41 header data
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      dev_t devno		device number which has x41 header
 *      X41HEAD **x41head	returns newly allocated x41 header
 *      int *x41head_offset	returns location of the x41 header
 *
 * RETURN VALUE DESCRIPTION:
 *      0    : success
 *      not 0: error
 */
int
get_x41head(dev_t devno, X41HEAD **x41head, int *x41head_offset)
{
	struct file	*fp;
	char		buf[SECTOR_SIZE];
	int		i, n = 0;
	register char	*p = buf + 0x1be;
	X41HEAD		*head;
	int		offset;

	DEBUG_1("Enter get_x41head(0x%x)\n", devno);

	/* open the device and get the x41-type partition location */
	if(fp_opendev(devno, DREAD, 0, 0, &fp) != 0){
		DEBUG_0("Exit get_x41head(): fp_opendev error\n");
		return -1;
	}
	if(fp_read(fp, buf, SECTOR_SIZE, 0, SYS_ADSPACE, &i) != 0){
		DEBUG_0("Exit get_x41head(): fp_read error\n");
		fp_close(fp);
		return -2;
	}
	for(i = 0; i < 4; i++){
		if((p[4] == 0x41) &&	/* x41-type partition && size != 0 */
			((p[12] != 0) | (p[13] != 0) |
			 (p[14] != 0) | (p[15] != 0))){
			n = p[8] | (p[9] << 8) | (p[10] << 16) | (p[11] << 24);
			break;
		}
	}
	if(n <= 0){
		DEBUG_0("Exit get_x41head(): x41type partition search error\n");
		fp_close(fp);
		return -3;
	}

	/* allocate word aligned area for x41head */
	if((head = (X41HEAD *)xmalloc(SECTOR_SIZE, 2, pinned_heap)) == NULL){
		DEBUG_0("Exit get_x41head(): xmalloc error\n");
		fp_close(fp);
		return -4;
	}

	/* read x41head */
	offset = (n + 1) * SECTOR_SIZE;
	if(fp_lseek(fp, offset, SEEK_SET) != 0){
		DEBUG_0("Exit get_x41head(): fp_lseek error\n");
		fp_close(fp);
		return -5;
	}
	if(fp_read(fp, head, SECTOR_SIZE, 0, SYS_ADSPACE, &i) != 0){
		DEBUG_0("Exit get_x41head(): fp_read error\n");
		fp_close(fp);
		return -6;
	}

	fp_close(fp);
	*x41head = head;
	*x41head_offset = offset;
	DEBUG_0("Exit get_x41head() successfully\n");
	return 0;
}
