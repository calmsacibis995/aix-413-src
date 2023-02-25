static char sccsid[] = "@(#)31  1.3  src/rspc/kernext/pm/pmmi/pmdump/getresidual.c, pwrsysx, rspc41J, 9517A_all 4/25/95 15:32:02";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: iplcb_get, get_residual, get_residual_data, get_area_info
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
#include <sys/adspace.h>
#include <sys/mdio.h>
#include <sys/residual.h>
#include <sys/iplcb.h>
#include <sys/malloc.h>
#include "iplcb_init.h"
#include "pmdump.h"
#include "pmdumpdebug.h"

/*
 * NAME:  iplcb_get
 *
 * FUNCTION:  gets data from IPLCB (IPL control block)
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      void *dest	points a buffer
 *      int address	offset in IPLCB
 *      int num_bytes	data length
 *      int iocall	command to pass fp_ioctl()
 *
 * RETURN VALUE DESCRIPTION:
 *      0 : success
 *      -1: access error
 */
static int
iplcb_get(void *dest, int address, int num_bytes, int iocall)
{
	MACH_DD_IO	mdd;
	struct file	*fp;

	if(fp_open("/dev/nvram", 0, 0, 0, SYS_ADSPACE, &fp) != 0){
		return -1;
	}

	mdd.md_addr = (long)address;
	mdd.md_data = dest;
	mdd.md_size = (long)num_bytes;
	mdd.md_incr = MV_BYTE;

	if(fp_ioctl(fp, iocall, &mdd, 0) != 0){
		return -1;
	}

	fp_close(fp);
	return 0;
}


/*
 * NAME:  get_residual
 *
 * FUNCTION:  gets residual data
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      RESIDUAL *rp	points a residual structure
 *      int *size	returns the size of the residual data
 *
 * RETURN VALUE DESCRIPTION:
 *      0 : success
 *      -1: access error
 */
int
get_residual(RESIDUAL *rp, int *size)
{
	IPL_DIRECTORY	iplcb_dir;

	if(iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB) != 0){
		return -1;
	}

	if(iplcb_get(rp, iplcb_dir.residual_offset,
				iplcb_dir.residual_size, MIOIPLCB) != 0){
		return -1;
	}
	*size = iplcb_dir.residual_size;
	return 0;
}


/*
 * NAME:  get_residual_data
 *
 * FUNCTION:  gets system memory size and residual data check-sum
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      int *memory_size	returns the system memory size
 *      int *check_sum		returns the residual data check-sum
 *
 * RETURN VALUE DESCRIPTION:
 *      0 : success
 *      -1: access error
 */
int
get_residual_data(int *memory_size, int *check_sum)
{
	RESIDUAL	residual;
	int		size;
	register int	sum;
	register char	*p;

	DEBUG_2("Enter get_residual_data(0x%x, 0x%x)\n",
					memory_size, check_sum);

	if(get_residual(&residual, &size) != 0){
		DEBUG_0("Exit get_residual_data(): get_residual error\n");
		return -1;
	}
	DEBUG_1("get_residual_data(): residual data length = 0x%x\n", size);

	/* Segs[] array must be excluded from check-sum */
	sum = 0;
	for(p = (char *)&residual; p < (char *)&(residual.Segs[0]); p++){
		sum += *p;
	}
	for(p = (char *)&(residual.ActualNumMemories);
					p < (((char *)&residual) + size); p++){
		sum += *p;
	}

	*memory_size = residual.TotalMemory;
	*check_sum = sum;

	DEBUG_1("get_residual_data(): check sum = 0x%x\n", *check_sum);
	DEBUG_0("Exit get_residual_data() successfully\n");
	return 0;
}


/*
 * NAME:  get_area_info
 *
 * FUNCTION:  gets DMA buffer and wakeup area information
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process environment.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      char **dma_buf          returns DMA buffer address
 *      int *dma_buf_length     returns DMA buffer length
 *      char **wakeup_area      returns wakeup area address
 *      int *wakeup_area        returns wakeup area length
 *
 * RETURN VALUE DESCRIPTION:
 *      0 : success
 *      -1: access error
 */
int
get_area_info(char **dma_buf, int *dma_buf_length,
		char **wakeup_area, int *wakeup_area_length)
{
	IPL_DIRECTORY	iplcb_dir;
	BUC_DATA	buc[MAX_BUC_SLOTS];
	int		i, ds, ws;
	char		*dp, *wp;

	DEBUG_0("Enter get_area_info()\n");

	/* get BUC table */
	if(iplcb_get(&iplcb_dir, 128, sizeof(iplcb_dir), MIOIPLCB)){
                DEBUG_0("Exit get_area_info(): iplcb_get(iplcb_dir) error\n");
		return -1;
	}
	if(iplcb_get(buc, iplcb_dir.buc_info_offset, iplcb_dir.buc_info_size,
								MIOIPLCB)){
                DEBUG_0("Exit get_area_info(): iplcb_get(buc) error\n");
		return -1;
	}

	ds = ws = 0;
	for(i = 0; i < buc[0].num_of_structs; i++){
		/* DMA buffer information */
		if(buc[i].device_id_reg == DMA_BUC_ID){
			dp = buc[i].mem_addr1;
			ds = buc[i].mem_alloc1;
		/* wakeup area information */
		}else if(buc[i].device_id_reg == HIBERNATION_BUC_ID){
			wp = buc[i].mem_addr1;
			ws = buc[i].mem_alloc1;
		}
	}
			
	if(ds == 0 || ws == 0){
                DEBUG_0("Exit get_area_info(): ds == 0 || ws == 0\n");
		return -1;
	}

	DEBUG_1("get_area_info(): dma_buf = %x\n", dp);
	DEBUG_1("get_area_info(): dma_buf_length = %x\n", ds);
	DEBUG_1("get_area_info(): wakeup_area = %x\n", wp);
	DEBUG_1("get_area_info(): wakeup_area_length = %x\n", ws);

	*dma_buf = dp;
	*dma_buf_length = ds;
	*wakeup_area = wp;
	*wakeup_area_length = ws;
	DEBUG_0("Exit get_area_info() successfully\n");
	return 0;
}
