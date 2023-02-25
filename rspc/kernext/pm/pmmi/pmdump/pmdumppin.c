static char sccsid[] = "@(#)36  1.6  src/rspc/kernext/pm/pmmi/pmdump/pmdumppin.c, pwrsysx, rspc41J, 9521A_all 5/22/95 05:28:24";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: dump_compressed_image, save_data, dump_memory, dump_mem,
 *            term_devdump
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
#include <sys/pm.h>
#include "hibernation.h"
#include "x41.h"
#include "pmdump.h"
#include "pmdumpdebug.h"
#include "pmmi.h"

extern int	memcmp(char *s1, char *s2, int l);
extern void	memcpy(char *t, char *f, int l);
extern void	memset(char *t, int i, int l);
extern void	compress(char *s, int in, char *t, int *out);

char		*outbuf = NULL;		/* output buffer */
pm_hibernation_t *pmh = NULL;		/* hibernation volume info */
X41HEAD		*x41head = NULL;	/* x41 header */
int		x41head_offset = 0;	/* location of x41 header */
char		*wakeup_code = NULL;	/* wakeup code */
int		wakeup_code_length = 0;	/* wakeup code length */
int		wakeup_entry = 0;	/* wakeup code entry offset */
int		residual_sum = 0;	/* residual data check-sum */
int		total_memory = 0;	/* total memory in bytes */
char		*wakeup_area = NULL;	/* wakeup area address */
int		wakeup_area_length = 0;	/* wakeup area length */
uint		*restart_address;	/* restart address for resume */
int		dump_size = DUMP_SIZE;	/* data size for dumpdev */
char		*bitmap = NULL;		/* bitmap for wakeup code */
int		bitmap_length = 0;	/* bitmap length */
int		dumpwrite_error = 0;	/* DUMPWRITE error */
char		*comp_buf = NULL;	/* copy data from real addr to here */
char		*comp_buf_real = NULL;	/* real address of comp_buf */
char		start_devdump = 0;	/* if devdump is started, them 1 */
char		*dma_buf = NULL;	/* DMA buffer address */
int		dma_buf_length = 0;	/* DMA buffer length */

static		intpri;			/* previous interrupt priority */

/*
 * NAME:  dump_compressed_image
 *
 * FUNCTION:  saves compressed memory image via devdump()
 *
 * EXECUTION ENVIRONMENT:
 *      This routine must be called at INTMAX.
 *      It can not page fault.
 *
 * DATA STRUCTURES:
 *      char *s		input data PHYSICAL address
 *      int in_len	input data length
 *      int *sect	sector list number currently used
 *      int *location	data location on physical disk
 *      int *image_len	returns compressed image length
 *
 * RETURN VALUE DESCRIPTION:
 *      0    : success
 *      not 0: error
 */
static	struct iovec		dumpiov;
static	struct uio		dumpuio;

#define DUMP_WRITE(base, len, offset)					\
	/* set uio structure */						\
	dumpiov.iov_base = (caddr_t)(base);				\
	dumpiov.iov_len = (len);					\
	dumpuio.uio_iov = &dumpiov;					\
	dumpuio.uio_resid = (len);					\
	dumpuio.uio_segflg = UIO_SYSSPACE;				\
	dumpuio.uio_iovcnt = 1;						\
	dumpuio.uio_iovdcnt = 0;					\
	dumpuio.uio_fmode = 0;						\
	dumpuio.uio_offset = (offset);					\
	dumpuio.uio_xmem = NULL;					\
	DEBUG_2("devdump(offset = 0x%x,len = 0x%x)\n", offset, len);	\
	if(devdump(pmh->devno, &dumpuio, DUMPWRITE, 0, 0, PM_DUMP) != 0){\
		dumpwrite_error = 1;					\
		return HIBERNATION_GENERAL_ERROR;			\
	}								\

#define EXECUTE_DUMP(buf, cnt, ofst, end, sct, tmp)			\
	if(ofst + (cnt) <= end){		/* enough space */	\
		DUMP_WRITE(buf, (cnt), ofst);				\
		ofst += (cnt);						\
	}else{	/* not enough space. use the next sector, too */	\
		tmp = end - ofst;					\
		DUMP_WRITE(buf, tmp, ofst);				\
		if(++sct >= pmh->snum){		/* no more sector */	\
			return INVALID_HIBERNATION_VOLUME;		\
		}							\
		ofst = pmh->slist[sct].start * SECTOR_SIZE;		\
		end = ofst + pmh->slist[sct].size * SECTOR_SIZE; 	\
		DUMP_WRITE(buf + tmp, (cnt) - tmp, ofst);		\
		ofst += ((cnt) - tmp);					\
	}

static int
dump_compressed_image(char *s, int in_len, int *sect,
				int *location, int *image_len)
{
	register int	i, comp_cnt, offset, out_end, sector, outcnt, tmp;
	int		n;

	DEBUG_2("Enter dump_compress(0x%x, 0x%x, ...)\n", s, in_len);

	sector = *sect;
	offset = *location;
	out_end = (pmh->slist[sector].start + pmh->slist[sector].size)
								* SECTOR_SIZE;
	outcnt = 0;
	comp_cnt = 0;

	/* from end of wakeup area to end of memory */
	for(i = (int)(wakeup_area + wakeup_area_length); i < in_len;
								i += ONE_PAGE){

		tmp = (int)(s + i);
		if(FREE_PAGE(tmp)){
			DEBUG_1("dump_compress(): 0x%x is a free page\n", tmp);
			continue;
		}
		copy_real(comp_buf_real, tmp, (ONE_PAGE / 4));
		compress(comp_buf, ONE_PAGE, outbuf + comp_cnt, &n);
		comp_cnt += n;

		if(comp_cnt >= dump_size){
			EXECUTE_DUMP(outbuf, dump_size, offset,
							out_end, sector, tmp);
			comp_cnt -= dump_size;
			outcnt += dump_size;
			if(comp_cnt > 0){
				memcpy(outbuf, outbuf + dump_size, comp_cnt);
			}
		}
	}

	/* from s to beginning of wakeup area */
	for(i = 0; i < (int)(wakeup_area); i += ONE_PAGE){

		tmp = (int)(s + i);
		if(FREE_PAGE(tmp)){
			DEBUG_1("dump_compress(): 0x%x is a free page\n", tmp);
			continue;
		}
		copy_real(comp_buf_real, tmp, (ONE_PAGE / 4));
		compress(comp_buf, ONE_PAGE, outbuf + comp_cnt, &n);
		comp_cnt += n;

		if(comp_cnt >= dump_size){
			EXECUTE_DUMP(outbuf, dump_size, offset,
							out_end, sector, tmp);
			comp_cnt -= dump_size;
			outcnt += dump_size;
			if(comp_cnt > 0){
				memcpy(outbuf, outbuf + dump_size, comp_cnt);
			}
		}
	}
	if(comp_cnt > 0){
		outbuf[comp_cnt] = 0;		/* end mark (0x00000000) */
		outbuf[comp_cnt + 1] = 0;	/* end mark (0x00000000) */
		outbuf[comp_cnt + 2] = 0;	/* end mark (0x00000000) */
		outbuf[comp_cnt + 3] = 0;	/* end mark (0x00000000) */
		outcnt += comp_cnt;
		comp_cnt = SECTOR_ALIGN(comp_cnt);
		EXECUTE_DUMP(outbuf, comp_cnt, offset, out_end, sector, tmp);
	}

	*sect = sector;
	*location = offset;
	*image_len = outcnt;
	DEBUG_0("Exit dump_compress() successfully\n");
	return 0;
}


/*
 * NAME:  save_data
 *
 * FUNCTION:  saves data via devdump() without compression
 *
 * EXECUTION ENVIRONMENT:
 *      This routine must be called at INTMAX.
 *      It can not page fault.
 *
 * DATA STRUCTURES:
 *      char *data	input data address
 *      int len		input data length
 *      int *sec	sector list number currently used
 *      int *locatio	data location on physical disk
 *
 * RETURN VALUE DESCRIPTION:
 *      0    : success
 *      not 0: error
 */
static int
save_data(char *data, int len, int *sect, int *location)
{
	register int	tmp, offset, out_end, sector;

	DEBUG_2("Enter save_data(0x%x, 0x%x)\n", data, len);

	sector = *sect;
	offset = *location;
	out_end = (pmh->slist[sector].start + pmh->slist[sector].size)
								* SECTOR_SIZE;
	for(;len > 0; len -= dump_size){
		if(len >= dump_size){
			EXECUTE_DUMP(data, dump_size, offset,
						out_end, sector, tmp);
			data += dump_size;
		}else{
			len = SECTOR_ALIGN(len);
			EXECUTE_DUMP(data, len, offset, out_end, sector, tmp);
		}
	}

	*sect = sector;
	*location = offset;
	DEBUG_0("Exit save_data() successfully\n");
	return 0;
}


/*
 * NAME:  dump_memory
 *
 * FUNCTION:  saves memory image via devdump()
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:
 *      RESUMEwSUCCESS    : success
 *      not RESUMEwSUCCESS: error
 *
 * NOTE:
 *      If an error occurs, non-RESUMEwSUCCESS value is returned.
 *      If an error occurs in devdump(DUMPWRITE), 1 is set to dumpwrite_error
 *      global variable, too. (see DUMP_WRITE macro)
 */
static int
dump_memory()
{
	int			sector_list;	/* #Block for sector list */
	int			i, sector, image_len, offset, rc;
	pm_sector_t		*pms_ptr;
	pm_hibernation_header_t	header;
	pm_config_data_t	config;
	struct dmp_query	query;

	DEBUG_1("Enter dump_memory(0x%x)\n", restart_address);

	/* initialize dump device */
	DEBUG_0("dump_memory(): devdump(DUMPINIT)\n");
	if(devdump(pmh->devno, NULL, DUMPINIT, 0, 0, PM_DUMP) != 0){
		DEBUG_0("Exit dump_memory(): DUMPINIT error\n");
		devdump(pmh->devno, NULL, DUMPTERM, 0, 0, PM_DUMP);
		return HIBERNATION_GENERAL_ERROR;
	}
	query.min_tsize = 0;
	query.max_tsize = 0;
	DEBUG_0("dump_memory(): devdump(DUMPQUERY)\n");
	if(devdump(pmh->devno, NULL, DUMPQUERY, &query, 0, PM_DUMP) != 0){
		DEBUG_0("Exit dump_memory(): DUMPQUERY error\n");
		devdump(pmh->devno, NULL, DUMPTERM, 0, 0, PM_DUMP);
		return HIBERNATION_GENERAL_ERROR;
        }
	DEBUG_1("query.min_tsize = 0x%x\n", query.min_tsize);
	DEBUG_1("query.max_tsize = 0x%x\n", query.max_tsize);
	if(dump_size > query.max_tsize){
		dump_size = query.max_tsize;
		DEBUG_1("dump_size is changed to %x\n", dump_size);
	}
	DEBUG_0("dump_memory(): devdump(DUMPSTART)\n");
	if(devdump(pmh->devno, NULL, DUMPSTART, 0, 0, PM_DUMP) != 0){
		DEBUG_0("Exit dump_memory(): DUMPSTART error\n");
		devdump(pmh->devno, NULL, DUMPEND, 0, 0, PM_DUMP);
		devdump(pmh->devno, NULL, DUMPTERM, 0, 0, PM_DUMP);
		return HIBERNATION_GENERAL_ERROR;
	}
	start_devdump = 1;

	/* initialize global data */
	sector = 0;	/* sector list item number */
	offset = 0;	/* byte offset from the beginning of disk */

	/* skip LVCB (Logical volume Control Block) and sector list area */
	sector_list = (pmh->snum + 1) * sizeof(pm_sector_t) / SECTOR_SIZE + 1;
	pmh->slist[0].start += (sector_list + 1);
	pmh->slist[0].size -= (sector_list + 1);
	offset = pmh->slist[0].start * SECTOR_SIZE;

	/* skip area for hibernation header and config data */
	offset += HEADER_DATA_SIZE;		/* skip header area */
	offset += CONFIG_DATA_SIZE;		/* skip config data area */

	/*
	 * save wakeup code
	 */
	rc = save_data(wakeup_code + wakeup_entry,
			wakeup_code_length - wakeup_entry, &sector, &offset);
	if(rc != 0){
		return rc;
	}

	/*
	 * get memory bitmap
	 */
	if(get_bitmap(bitmap, bitmap_length) != 0){
		DEBUG_0("dump_memory(): get_bitmap() failed\n");
		return HIBERNATION_GENERAL_ERROR;
	}

	/*
	 * save memory bitmap
	 */
	rc = save_data(bitmap, bitmap_length, &sector, &offset);
	if(rc != 0){
		return rc;
	}

	/*
	 * save compressed memory image
	 */
	rc = dump_compressed_image(0, total_memory,
					&sector, &offset, &image_len);
	if(rc != 0){
		return rc;
	}

	/*
	 * save sector list
	 */
	pmh->slist[sector].size		/* offset is already sector aligned */
		= (offset / SECTOR_SIZE) - pmh->slist[sector].start;
	pmh->snum = ++sector;
	pmh->slist[sector].start = 0;
	pmh->slist[sector++].size = 0;
	memset(outbuf, 0, sector_list * SECTOR_SIZE);
	pms_ptr = (pm_sector_t *)outbuf;
	for(i = 0; i < sector; i++){	/* make sector list in little endian */
		pms_ptr[i].start = LEint(pmh->slist[i].start);
		pms_ptr[i].size = LEint(pmh->slist[i].size);
	}
	/* skip PC compatibility block */
	offset = (pmh->slist[0].start - sector_list) * SECTOR_SIZE;
	DEBUG_0("dump_memory(): dump_write(sector list)\n");
	DUMP_WRITE(outbuf, (SECTOR_SIZE * sector_list), offset);

	/*
	 * save header area
	 */
	memcpy(header.signature, "AIXPM001", (sizeof("AIXPM001") - 1));
	header.wakeup_area = (uint *)wakeup_area;
	header.wakeup_area_length = wakeup_area_length;
	header.config_data_offset = HEADER_DATA_SIZE;
	header.config_data_length = sizeof(pm_config_data_t);
	header.wakeup_code_offset = HEADER_DATA_SIZE + CONFIG_DATA_SIZE;
	header.wakeup_code_length = wakeup_code_length - wakeup_entry;
	header.restart = restart_address;
	header.bitmap_offset = header.wakeup_code_offset +
				SECTOR_ALIGN(header.wakeup_code_length);
	header.bitmap_length = SECTOR_ALIGN(bitmap_length);
	header.image_offset = header.bitmap_offset +
				SECTOR_ALIGN(header.bitmap_length);
	header.image_length = image_len;
	DEBUG_HEADER(&header);
	memset(header.reserve, 0, sizeof(header.reserve));
	memset(outbuf, 0, SECTOR_SIZE);
	memcpy(outbuf, (char *)&header, sizeof(header));
	offset = pmh->slist[0].start * SECTOR_SIZE;
	DEBUG_0("dump_memory(): dump_write(header)\n");
	DUMP_WRITE(outbuf, HEADER_DATA_SIZE, offset);

	/*
	 * check image length
	 */
	if(header.image_offset + header.image_length + 0x400000 >
							(int)wakeup_area){
		DEBUG_0("dump_memory(): image too big\n");
		return HIBERNATION_GENERAL_ERROR;
	}

	/*
	 * save config data area
	 */
	config.memory_size = total_memory;
	config.residual_sum = residual_sum;
	DEBUG_CONFIG_DATA(&config);
	memset(outbuf, 0, SECTOR_SIZE);
	memcpy(outbuf, (char *)&config, sizeof(pm_config_data_t));
	offset = (pmh->slist[0].start + 1) * SECTOR_SIZE;
	DEBUG_0("dump_memory(): dump_write(config data)\n");
	DUMP_WRITE(outbuf, CONFIG_DATA_SIZE, offset);

	/*
	 * save x41 header
	 */
	x41head->X41PMMode = Hibernate;
	i = pmh->slist[0].start - sector_list;
	x41head->HibResumeSectorListRBA = LEint(i);
	i = (sector * sizeof(pm_sector_t) + SECTOR_SIZE) / SECTOR_SIZE;
	x41head->HibResumeSectorListCount = LEint(i);
	i = header.wakeup_code_offset;
	x41head->HibResumeCodeEntryOffset = LEint(i);
	x41head->HibResumeDevice[0] = '\0';
	DEBUG_0("dump_memory(): dump_write(x41head)\n");
	DUMP_WRITE((char *)x41head, SECTOR_SIZE, x41head_offset);

	/* check page table overflow while saving memory */
	if(pm_check_overflow() != 0){
		DEBUG_0("dump_memory(): pm_check_overflow() != 0\n");
		/* invalidate hibernation image */
		x41head->X41PMMode = Normal;
		x41head->HibResumeSectorListRBA = LEint(0);
		x41head->HibResumeSectorListCount = LEint(0);
		x41head->HibResumeCodeEntryOffset = LEint(0);
		DEBUG_0("dump_memory(): dump_write(invalidate x41head)\n");
		DUMP_WRITE((char *)x41head, SECTOR_SIZE, x41head_offset);
		return HIBERNATION_GENERAL_ERROR;
	}
	DEBUG_0("dump_memory(): pm_check_overflow OK\n");
		
	DEBUG_0("Exit dump_memory() successfully\n");
	return RESUMEwSUCCESS;
}


/*
 * NAME:  start_hibernation
 *
 * FUNCTION:  saves memory image via devdump()
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called from process environment.
 *      In this routine, interrupt is disabled (INTMAX).
 *
 * DATA STRUCTURES:
 *      int *restart	restart PHYSICAL address
 *
 * RETURN VALUE DESCRIPTION:
 *      RESUMEwSUCCESS    : success
 *      not RESUMEwSUCCESS: error
 */
int
start_hibernation(uint *restart)
{
	int	rc;

	PMDEBUG_1("Enter start_hibernation(0x%x)\n", restart);

	/* check buffers */
	if(outbuf == NULL || pmh == NULL ||
				x41head == NULL || wakeup_code == NULL){
		DEBUG_0("Exit start_hibernation(): buffer is NULL\n");
		return HIBERNATION_GENERAL_ERROR;
	}

	/* disable interrupt (devdump needs INTMAX) */
	/* i_enable() is done in term_devdump() after resuming */
	intpri = i_disable(INTMAX);

	/* save compressed memory image in offlevel handler */
	restart_address = restart;
	rc = dump_memory();

	/* if dump_memory failes, term_devdump and i_enable are done here */
	if(rc != RESUMEwSUCCESS){
		if(start_devdump == 0){
			i_enable(intpri);
		}else{
			term_devdump();
		}
	}

	PMDEBUG_1("Exit start_hibernation(): return code = %d\n", rc);
	return rc;
}


/*
 * NAME:  term_devdump
 *
 * FUNCTION:  terminates devdump()
 *
 * DATA STRUCTURES:
 *      none
 *
 * RETURN VALUE DESCRIPTION:
 *      RESUMEwSUCCESS           : success
 *      HIBERNATION_GENERAL_ERROR: error
 */
int
term_devdump(void)
{
	PMDEBUG_0("Enter term_devdump()\n");

	/* this term_devdump is effective only when the devdump() */
	/* already started */
	if(start_devdump == 0){
		DEBUG_0("Exit term_devdump(): start_devdump = 0\n");
		DEBUG_0("Exit term_devdump(): return code = 0\n");
		return RESUMEwSUCCESS;
	}

	/* terminate dump device */
	if(devdump(pmh->devno, NULL, DUMPEND, 0, 0, PM_DUMP) != 0){
		DEBUG_0("Exit dump_mem(): DUMPEND error\n");
		return HIBERNATION_GENERAL_ERROR;
	}
	if(devdump(pmh->devno, NULL, DUMPTERM, 0, 0, PM_DUMP) != 0){
		DEBUG_0("Exit dump_mem(): DUMPTERM error\n");
		return HIBERNATION_GENERAL_ERROR;
	}

	/* enable interrupt */
	i_enable(intpri);

	start_devdump = 0;
	PMDEBUG_0("Exit term_devdump(): return code = 0\n");
	return RESUMEwSUCCESS;
}
