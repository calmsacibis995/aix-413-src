static char sccsid[] = "@(#)33  1.3  src/rspc/kernext/pm/pmmi/pmdump/pmdump.c, pwrsysx, rspc41J, 9521A_all 5/22/95 05:26:22";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: init_dump_mem, term_dump_mem
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
#include <sys/types.h>
#include <sys/device.h>
#include <sys/dump.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/intr.h>
#include <sys/xmem.h>
#include <sys/pm.h>
#include "cmdlvm.h"
#include "x41.h"
#include "hibernation.h"
#include "pmdump.h"
#include "pmdumpdebug.h"
#include "pmmi.h"

extern int	start_hibernation(int *restart);
extern int	check_pmhlv(pm_hibernation_t *pmhp);
extern int	get_x41head(dev_t devno, X41HEAD **x41head, int *x41headoffset);
extern int	memcmp(char *s1, char *s2, int l);
extern void	memcpy(char *t, char *f, int l);
extern void	memset(char *t, int i, int l);
extern int	get_residual_data(int *memory_size, int *check_sum);
extern int	read_wakeup_code(char *filename, char **code, int *length);
extern int	copy_pmhibernation(pm_hibernation_t *hiber,
						pm_hibernation_t **pmhiber);
extern int	get_area_info(char **dma_buf, int *dma_buf_length,
				char **wakeup_area, int *wakeup_area_length);

extern char		*outbuf;		/* output buffer */
extern pm_hibernation_t *pmh;			/* hibernation volume info */
extern X41HEAD		*x41head;		/* x41 header */
extern int		x41head_offset;		/* location of x41 header */
extern char		*wakeup_code;		/* wakeup code */
extern int		wakeup_code_length;	/* wakeup code length */
extern int		wakeup_entry;		/* wakeup code entry offset */
extern int		residual_sum;		/* residual data check-sum */
extern int		total_memory;		/* total memory in bytes */
extern char		*wakeup_area;		/* wakeup area address */
extern int		wakeup_area_length;	/* wakeup area length */
extern int		*restart_address;	/* restart address for resume */
extern int		dump_size;		/* data size for dumpdev */
extern char		*bitmap;		/* bitmap for wakeup code */
extern int		bitmap_length;		/* bitmap length */
extern int		dumpwrite_error;	/* DUMPWRITE error */
extern char		*comp_buf;		/* copy data from real addr to
						   here */
extern char		*comp_buf_real;		/* real address of comp_buf */
extern char		*dma_buf;		/* DMA buffer address */
extern int		dma_buf_length;		/* DMA buffer length */

/*
 * NAME:  init_hibernation
 *
 * FUNCTION:  initializes dump routine for hibernation
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      pm_hibernation_t *hiber	passes the hibernation volume information
 *
 * RETURN VALUE DESCRIPTION:
 *      RESUMEwSUCCESS    : success
 *      not RESUMEwSUCCESS: error
 */
int
init_hibernation(pm_hibernation_t *hiber)
{
	struct xmem	x;

	PMDEBUG_1("Enter init_hibernation(0x%x)\n", hiber);

	/* if no hibernation volume, then return */
	if(hiber->snum == 0){
		DEBUG_0("Exit init_hibernation(): hiber->snum == 0\n");
		return NO_HIBERNATION_VOLUME;
	}

	/* get memory size and check-sum of residual data */
	if(get_residual_data(&total_memory, &residual_sum) != 0){
		DEBUG_0("Exit init_hibernation(): get_residual_data error\n");
		return HIBERNATION_GENERAL_ERROR;
	}

	/* allocate page aligned buffer (dump size + 2 pages) */
	if((outbuf = xmalloc((2*ONE_PAGE+DUMP_SIZE), 12, pinned_heap)) == NULL){
		DEBUG_0("Exit init_hibernation(): xmalloc(outbuf) error\n");
		return MEMORY_CANNOT_OBTAINED;
	}

	/* allocate page aligned buffer (ONE_PAGE) for compression */
	if((comp_buf = xmalloc(ONE_PAGE, 12, pinned_heap)) == NULL){
		DEBUG_0("Exit init_hibernation(): xmalloc(comp_buf) error\n");
		return MEMORY_CANNOT_OBTAINED;
	}
	/* get physical address of the comp_buf */
	x.aspace_id = XMEM_GLOBAL;
	comp_buf_real = (char *)xmemdma(&x, comp_buf, XMEM_HIDE);
	DEBUG_1("init_dump_mem(): comp_buf_real = 0x%x\n", comp_buf_real);

	/* allocate compression internal data area */
	if(init_compress() != 0){
		DEBUG_0("Exit init_hibernation(): init_compress error\n");
		term_hibernation();
		return MEMORY_CANNOT_OBTAINED;
	}

	/* allocate and copy pm_hibernation_t */
	if(copy_pmhibernation(hiber, &pmh) != 0){
		DEBUG_0("Exit init_hibernation(): copy_pmhibernation error\n");
		term_hibernation();
		return MEMORY_CANNOT_OBTAINED;
	}

	/* check hibernation logical volume */
	if(check_pmhlv(pmh) != 0){
		DEBUG_0("Exit init_hibernation(): check_pmhlv() error\n");
		term_hibernation();
		return INVALID_HIBERNATION_VOLUME;
	}

	/* read x41 partition header */
	if(get_x41head(pmh->devno, &x41head, &x41head_offset) != 0){
		DEBUG_0("Exit init_hibernation(): get_x41head() error\n");
		term_hibernation();
		return HIBERNATION_GENERAL_ERROR;
	}

	/* read wakeup code */
	if(read_wakeup_code(WAKEUP_CODE, &wakeup_code,
					&wakeup_code_length) != 0){
		DEBUG_0("Exit init_hibernation(): read_wakeup_code() error\n");
		term_hibernation();
		return NO_EXECUTABLE_WAKEUPCODE;
	}
	wakeup_entry = entry_point(wakeup_code);

	/* allocate bitmap area */
	bitmap_length = total_memory / (ONE_PAGE * 8);
        if((bitmap = xmalloc(SECTOR_ALIGN(bitmap_length), 2, pinned_heap))
								== NULL){
		DEBUG_0("Exit init_hibernation(): xmalloc(bitmap) error\n");
		term_hibernation();
		return MEMORY_CANNOT_OBTAINED;
	}
	memset(bitmap, 0x00, SECTOR_ALIGN(bitmap_length));

	/* get DMA buffer area and wakeup area information */
	if(get_area_info(&dma_buf, &dma_buf_length,
				&wakeup_area, &wakeup_area_length) != 0){
		DEBUG_0("Exit init_hibernation(): get_area_info() error\n");
		term_hibernation();
		return HIBERNATION_GENERAL_ERROR;
	}

	PMDEBUG_0("Exit init_hibernation() successfully\n");
	return RESUMEwSUCCESS;
}


/*
 * NAME:  write_over_pmhlv
 *
 * FUNCTION:  if a DUMPWRITE error occurs, then write dummy data over the
 *            entire lenght of the hibernation volume to use hardware
 *            relocation.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      none
 *
 * RETURN VALUE DESCRIPTION:
 *      none
 */
void
write_over_pmhlv(void)
{
	struct file	*fp;
	struct LVCB	*lvcb;
	int		i, j, size, end;

	DEBUG_0("Enter write_over_pmhlv()\n");

	if(fp_open("/dev/pmhibernation", 0, 0, 0, SYS_ADSPACE, &fp) != 0){
		DEBUG_0("Exit write_over_pmhlv(): fp_open error\n");
		return;
	}

	if(fp_read(fp, outbuf, SECTOR_SIZE, 0, SYS_ADSPACE, &size) != 0){
		DEBUG_0("Exit write_over_pmhlv(): fp_read error\n");
		fp_close(fp);
		return;
	}
	if(size != SECTOR_SIZE){
		DEBUG_1("Exit write_over_pmhlv(): size(%d) != SECTOR_SIZE\n",
							size);
		fp_close(fp);
		return;
	}

	lvcb = (struct LVCB *)outbuf;
	end = pmh->ppsize * lvcb->num_lps - SECTOR_SIZE;
	size = DUMP_SIZE;
	memset(outbuf, 0, size);
	for(i = 0; i < end; i += size){
		if(end - i < size){
			size = end - i;
		}
		if(fp_write(fp, outbuf, size, 0, SYS_ADSPACE, &j) != 0){
			DEBUG_0("Exit write_over_pmhlv(): fp_write error\n");
			fp_close(fp);
			return;
		}
	}
	fp_close(fp);

	DEBUG_0("Exit write_over_pmhlv() successfully\n");
	return;
}


/*
 * NAME:  term_hibernation
 *
 * FUNCTION:  terminates dump routine after resuming from hibernation
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      none
 *
 * RETURN VALUE DESCRIPTION:
 *      RESUMEwSUCCESS : success (always)
 */
int
term_hibernation(void)
{
	PMDEBUG_0("Enter term_hibernation()\n");

	/* if DUMPWRITE error, then write dummy data to hibernation volume */
	if(dumpwrite_error != 0){
		write_over_pmhlv();
		dumpwrite_error = 0;
	}

	term_compress();
	FREE_MEMORY(outbuf);
	FREE_MEMORY(comp_buf);
	FREE_MEMORY(pmh->slist);
	FREE_MEMORY(pmh);
	FREE_MEMORY(x41head);
	FREE_MEMORY(wakeup_code);
	wakeup_code_length = 0;
	FREE_MEMORY(bitmap);
	bitmap_length = 0;

	PMDEBUG_0("Exit term_hibernation() successfully\n");
	return RESUMEwSUCCESS;
}
