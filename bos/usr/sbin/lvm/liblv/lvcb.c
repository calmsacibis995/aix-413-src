static char sccsid[] = "@(#)83	1.9.1.1  src/bos/usr/sbin/lvm/liblv/lvcb.c, cmdlvm, bos411, 9428A410j 3/4/94 17:33:17";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: build_lvcb write_lvcb read_lvcb
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FILENAME: lvcb.c
 *
 * FILE DESCRIPTION: Contains functions to read, build,
 *  and write the logical volume control block.
 *
 *                   This file's C Functions: build_lvcb
 *                                            write_lvcb
 *                                            read_lvcb
 *
 */

#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include "cmdlvm.h"

#define LVCB_BLOCK_OFFSET      0
#define OPEN_FAILURE          -1
#define SEEK_FAILURE          -1
#define READ_FAILURE          -1
#define END_OF_FILE            0

/*
 * STRMAXCOPY
 * string copy macro to copy at most N characters and
 * insure there is a null terminator.
 */
#define STRMAXCOPY(a,b,n) strncpy(a,b,n);a[n-1]='\0'


/*
 * NAME: build_lvcb
 *
 * FUNCTION: Build a logical volume control block structure.
 *  If the real control block already exist in the logical
 *  volume, it must be read first so the structure built
 *  contains the correct info (e.g. creation time).
 *
 *
 * RETURN VALUE DESCRIPTION: If successful then a pointer to a built lvcb.
 *                           If failure then null pointer.
 */

struct LVCB *build_lvcb (type, lvname, vgname, lvid, num_lps,
			 interpolicy, intrapolicy, upperbound,
			 strict, copies, relocatable, auto_on,forced,
			 label,fs,striping_width,stripe_exp)
STRING
	*type,          /* logical volume type (INPUT) */
	*lvname,        /* logical volume name (INPUT) */
	*vgname,        /* volume group name (INPUT) */
	*lvid;          /* logical volume id (INPUT) */
int
	*num_lps,       /* number of logical partitions (INPUT) */
	*upperbound,    /* the upper bound on allocation PVs (INPUT) */
	*copies;        /* number of copies to be created for LPs (INPUT) */
char
	*interpolicy,   /* inter-physical volume allocation policy (INPUT) */
	*intrapolicy,   /* intra-physical volume allocation policy (INPUT) */
	*strict,        /* strictness flag for allocation (INPUT) */
	*relocatable,   /* relocation flag (INPUT) */
	*auto_on;       /* auto vary on flag (INPUT) */
int
        forced;         /* flag to force the build even if the copy of the */
			/* control block is invalid */
char    *label;         /* label string */
char    *fs;            /* fs (/etc/filesystem entry (INPUT) */
int     *striping_width;  /* stripe width */
int     *stripe_exp;  /* stripe block in exponent */
{
	long int
		create_time,    /* time the orginal lvcb was created  */
		modify_time;    /* time the lvcb was modified  */
	struct LVCB
		*lvcb;          /* structure for the lvcb */
	struct utsname
		utsname;        /* structure which contains the machine id */
	RETURN_CODE
		rc=SUCCESS;     /* error flag */
	struct tm
		*create_time_struct,  /* time the orginal lvcb created */
		*modify_time_struct;  /* time the lvcb was modified  */

	int good_copy;           /* true if copy of control block read ok. */
	

	lvcb = (struct LVCB *) get_zeroed_mem(1, sizeof(struct LVCB));
	rc = read_lvcb(lvname, lvcb);
	if (rc != SUCCESS) {
		free (lvcb);
		lvcb = (struct LVCB *) NULL;
	}
	else {
	        /* check to see if the copy of the control block that
		 * was read is valid.  If the copy is not valid, ie
		 * it doesn't have the LVCB_TAG in it, then an error
		 * is returned unless the force flag is set.
		 */
	        good_copy = strcmp(lvcb->validity_tag, LVCB_TAG);
	        if ( (good_copy != 0)  && (forced == 0) )
	        {
		  /*
		   * The condition is as follows:
		   *    the copy of the control block is invalid
		   *    the forced flag is not set
		   * therefore this is an error condition and must
		   * be reported.  
		   */
		  free(lvcb);
		  lvcb = (struct LVCB *) NULL;
		  return lvcb;
		}
	
	        if (forced != 0)
	        {
		  /*
		   * The forced flag is set.
		   * So the control block structure gets
		   * initilized before it gets written out.
		   */
		  bzero (lvcb, sizeof(struct LVCB));
		  create_time = time((long *) 0);
		  create_time_struct = localtime(&create_time);
		  strcpy(lvcb->created_time, asctime(create_time_struct));
		  strcpy(lvcb->validity_tag, LVCB_TAG);

		  STRMAXCOPY(lvcb->type,  LVDF_TYPE ,LVCB_TYPE_SIZE);
		  lvcb->relocatable = LVDF_RELOCAT;
		  lvcb->interpolicy = LVDF_INTER;
		  lvcb->intrapolicy = LVDF_INTRA;
		  lvcb->strict      = LVDF_STRICT;
		  lvcb->upperbound  = LVDF_UPPERBOUND;
		  lvcb->copies      = LVDF_COPIES;
		  lvcb->striping_width = LVDF_STRIPE_WIDTH;
		  lvcb->stripe_exp = LVDF_STRIPE_BLK_EXP;
		}
		if (uname(&utsname) != FAILURE)
		{
			bzero(lvcb->mod_machine_id,LVCB_MACHINE_ID_SIZE);
		/* left terminate the machine id because it is usually 12 */
			strncpy (lvcb->mod_machine_id, utsname.machine+3,
				LVCB_MACHINE_ID_SIZE - 1);
		}
		else
			*lvcb->mod_machine_id = (char) NULL;

		modify_time = time((long *) 0);
		modify_time_struct = localtime(&modify_time);
		strcpy(lvcb->modified_time, asctime(modify_time_struct));

		if (type != (char *) NULL)
			STRMAXCOPY(lvcb->type, type,LVCB_TYPE_SIZE);
		if (lvid != (char *) NULL)
			STRMAXCOPY(lvcb->lvid, lvid,LVCB_LVID_SIZE);
		if (lvname != (char *) NULL)
			STRMAXCOPY(lvcb->lvname, lvname,LVCB_NAME_SIZE);
		if (relocatable != (char *) NULL)
			lvcb->relocatable = *relocatable;
		if (interpolicy != (char *) NULL)
			lvcb->interpolicy = *interpolicy;
		if (intrapolicy != (char *) NULL)
			lvcb->intrapolicy = *intrapolicy;
		if (auto_on != (char *) NULL)
			lvcb->auto_on = *auto_on;
		if (strict != (char *) NULL)
			lvcb->strict = *strict;
		if (upperbound != (int *) NULL)
			lvcb->upperbound = (short int) *upperbound;
		if (num_lps != (int *) NULL)
			lvcb->num_lps = (short int) *num_lps;
		if (copies != (int *) NULL)
			lvcb->copies = (short int) *copies;
		if (label != (char *) NULL)
		    STRMAXCOPY(lvcb->label,label,LVCB_LABEL_SIZE);
		if (fs != (char *) NULL)
		    STRMAXCOPY(lvcb->fs,fs,LVCB_FS_SIZE);
		if (striping_width != (int *) NULL)
			lvcb->striping_width = (short int) *striping_width;
		if (stripe_exp != (int *) NULL)
			lvcb->stripe_exp = (short int) *stripe_exp;
	}
	return lvcb;
}





/*
 * NAME:
 *
 * FUNCTION:
 *
 * (NOTES:)
 *
 *
 * RETURN VALUE DESCRIPTION:
 *
 *
 */
RETURN_CODE write_lvcb (lvname, lvcb)
STRING *lvname;
struct LVCB *lvcb;
{
	int
		bytes_written,
		lv_handle,
		rc;
	off_t
		offset;
	STRING
		devname[STRSIZE];


	strcpy (devname, DEVICE_DIRECTORY);
	strcat (devname, lvname);

	lv_handle = open (devname,  O_RDWR | O_DELAY );
	if (lv_handle == OPEN_FAILURE)
		rc = FAILURE;
	else {
		offset = (off_t) LVCB_BLOCK_OFFSET * UBSIZE;
		offset = lseek (lv_handle, offset, SEEK_SET);
		if (offset == (off_t) SEEK_FAILURE)
			rc = FAILURE;
		else {
			bytes_written =
			    write (lv_handle,
				   (char *) lvcb,
				   sizeof (struct LVCB));
			if (bytes_written != sizeof (struct LVCB))
				rc = FAILURE;
			else
				rc = SUCCESS;
		}
	}
	close(lv_handle);
	return rc;
}





/*
 * NAME:
 *
 * FUNCTION:
 *
 * (NOTES:)
 *
 *
 * RETURN VALUE DESCRIPTION:
 *
 *
 */
RETURN_CODE read_lvcb (lvname, lvcb)
STRING *lvname;
struct LVCB *lvcb;
{
	int
		bytes_read,
		lv_handle,
		rc;
	off_t
		offset;
	STRING
		devname[STRSIZE];


	strcpy (devname, DEVICE_DIRECTORY);
	strcat (devname, lvname);

	lv_handle = open (devname, O_RDWR  | O_DELAY );
	if (lv_handle == OPEN_FAILURE)
		rc = FAILURE;
	else {
		offset = (off_t) LVCB_BLOCK_OFFSET * UBSIZE;
		offset = lseek (lv_handle, offset, SEEK_SET);
		if (offset == (off_t) SEEK_FAILURE)
			rc = FAILURE;
		else {
			bytes_read =
			    read (lv_handle,
				  (char *) lvcb,
				  sizeof (struct LVCB));
			if (bytes_read != sizeof (struct LVCB))
				rc = FAILURE;
			else
				rc = SUCCESS;
		}
	}
	close(lv_handle);
	return rc;
}
