static char sccsid[] = "@(#)27  1.3  src/rspc/kernext/pm/pmmi/pmdump/bitmap.c, pwrsysx, rspc41J, 9521A_all 5/18/95 12:54:49";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: get_bitmap
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
#include "iplcb_init.h"
#include "pmdump.h"
#include "pmdumpdebug.h"

extern char		*wakeup_area;		/* wakeup area address */
extern int		wakeup_area_length;	/* wakeup area length */
extern char		*dma_buf;		/* DMA buffer address */
extern int		dma_buf_length;		/* DMA buffer length */

/*
 * NAME:  get_bitmap
 *
 * FUNCTION: gets bitmap from vmm and mark wakeup area as free.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called from the interrupt environment.
 *
 * DATA STRUCTURES:
 *      char *bitmap		returns bitmap
 *      int bitmap_length	bitmap area length
 *
 * RETURN VALUE DESCRIPTION:
 *      0 : success
 *      -1: error
 *
 * NOTE:
 *     DMA buffer is marked as busy.
 *     wakeuap area is marked as free.
 */
int
get_bitmap(char *bitmap, int bitmap_length)
{
	int	page, byte, bit, last;

	DEBUG_0("Enter get_bitmap()\n");

	if(pm_set_bitmap(bitmap, bitmap_length * 8) != 0){
		DEBUG_0("get_bitmap(): pm_set_bitmap() failed\n");
		return -1;
	}

	/* make sure DMA buffer is busy (not free) */
	last = (int)(dma_buf + dma_buf_length) / ONE_PAGE;
	for(page = (int)dma_buf / ONE_PAGE; page < last; page++){
		byte = page / 8;
		bit = page % 8;
		if((bitmap[byte] & (0x80 >> bit)) != 0){
			DEBUG_2("get_bitmap(): dma_buf bitmap[0x%x] = 0x%x\n",
							byte, bitmap[byte]);
			return -1;
		}
	}

	/* mark wakeup area as free not to save it */
	last = (int)(wakeup_area + wakeup_area_length) / ONE_PAGE;
	for(page = (int)wakeup_area / ONE_PAGE; page < last; page++){
		byte = page / 8;
		bit = page % 8;
		if((bitmap[byte] & (0x80 >> bit)) != 0){
			DEBUG_2("get_bitmap(): wakeup bitmap[0x%x] = 0x%x\n",
							byte, bitmap[byte]);
			return -1;
		}
		bitmap[byte] |= (0x80 >> bit);
	}

#ifdef PM_DEBUG
	printf("bitmap\n");
	for(byte = 0; byte < bitmap_length; byte++){
		if((byte % 32) == 0){
			printf("%3d ", (byte / 32));
		}else if((byte % 4) == 0){
			printf(" ");
		}
		printf("%02x", bitmap[byte]);
		if(((byte+1) % 32) == 0){
			printf("\n");
		}
	}
#endif

	DEBUG_0("Exit get_bitmap() successfully\n");
	return 0;
}
