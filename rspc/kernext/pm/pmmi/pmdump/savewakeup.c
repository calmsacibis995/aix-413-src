static char sccsid[] = "@(#)40  1.1  src/rspc/kernext/pm/pmmi/pmdump/savewakeup.c, pwrsysx, rspc41J, 9510A 3/8/95 11:50:31";
/*
 * COMPONENT_NAME: PWRSYSX
 *
 * FUNCTIONS: read_wakeup_code, entry_point
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
#include <sys/stat.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <scnhdr.h>
#include <aouthdr.h>
#include <filehdr.h>
#include "pmdump.h"
#include "pmdumpdebug.h"

/*
 * NAME:  read_wakeup_code
 *
 * FUNCTION:  reads PM wakeup code into memory
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * DATA STRUCTURES:
 *      char *filename		file name of the wakeup code
 *      char **code		returns buffer allocated to hold wakeup code
 *      int *length		length of the wakeup code
 *
 * RETURN VALUE DESCRIPTION:
 *      0  : PM initialization success.
 *      -1 : error.
 */
int
read_wakeup_code(char *filename, char **code, int *length)
{
	struct file	*fp;
	struct stat	stat;
	int		size;

	DEBUG_3("Enter read_wakeup_code(%s, 0x%x, 0x%x)\n",
						filename, code, length);

	/* get file size of the wakeup code file */ 
	if(fp_open(filename, 0, 0, 0, SYS_ADSPACE, &fp) != 0){
		DEBUG_0("Exit read_wakeup_code(): fp_open error\n");
		return -1;
	}
	if(fp_fstat(fp, &stat, STATSIZE, SYS_ADSPACE) != 0){
		DEBUG_0("Exit read_wakeup_code(): fp_fstat error\n");
		fp_close(fp);
		return -2;
	}
	
	/* allocate word aligned area for the wakeup code */
	size = ((stat.st_size + (SECTOR_SIZE - 1)) / SECTOR_SIZE) * SECTOR_SIZE;
	if((*code = xmalloc(size, 2, pinned_heap)) == NULL){
		DEBUG_0("Exit read_wakeup_code(): xmalloc error\n");
		fp_close(fp);
		return -3;
	}

	/* read the wakeup code */
	if(fp_read(fp, *code, stat.st_size, 0, SYS_ADSPACE, &size) != 0){
		DEBUG_0("Exit read_wakeup_code(): fp_read error\n");
		fp_close(fp);
		return -4;
	}
	fp_close(fp);
	if(size != stat.st_size){
		DEBUG_2("Exit read_wakeup_code(): size(%d)!=stat.st_size(%d)\n",
							size, stat.st_size);
		return -5;
	}

	DEBUG_1("Exit read_wakeup_code(): wakeup code size = %d\n", size);
	DEBUG_0("Exit read_wakeup_code() successfully\n");
	*length = size;
	return 0;
}

/*
 * NAME:  entry_point
 *
 * FUNCTION:  gets entry point of an executable
 *
 * DATA STRUCTURES:
 *      char *buf	executable image
 *
 * RETURN VALUE DESCRIPTION:
 *      returns entry point offset
 */
typedef struct _Xtoc_Header
{
	struct filehdr  filehdr;
	struct aouthdr  aouthdr;
	struct scnhdr   scnhdr[10];
} Xtoc_Header;

int
entry_point(char *buf)
{
	Xtoc_Header     *xtocp;

	xtocp = (Xtoc_Header *)buf;

	return (xtocp->scnhdr[xtocp->aouthdr.o_sntext-1].s_scnptr);
}
