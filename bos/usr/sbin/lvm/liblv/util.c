static char sccsid[] = "@(#)23  1.9  src/bos/usr/sbin/lvm/liblv/util.c, cmdlvm, bos411, 9428A410j 5/30/91 15:50:03";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: getmem, get_zeroed_mem, get_uniq_id
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
 * FILENAME: util.c
 *
 * FILE DESCRIPTION: Source file for common functions used by the
 *      low and intermediate Logical Volume Manager programs.
 */

#include <stdio.h>
#include <lvm.h>

/*
 * EXTERNAL PROCEDURES CALLED:
 */


/*
 * EXTERNAL VARIABLES
 */



/*
 * NAME: getmem
 *
 * FUNCTION: Common routine for allocating memory and detecting allocation error.
 *
 * RETURN VALUE DESCRIPTION: Pointer to the memory block allocated.  If the
 *     size requested could not be allocated then the program will exit with
 *     a -1 (i.e. the function will never return on error)..
 */

char *getmem(size)

int size;   /* the requested allocation size in bytes */
{
	char *ptr;   /* pointer to memory block allocated */

	if ((ptr = (char *) malloc(size)) != NULL) return(ptr);	 /* if success - return */
	perror("malloc:");                  /* print the system message for the failure */
	exit(-1);			    /* fatal error - exit the program */
}

/*
 * NAME: get_zeroed_mem
 *
 * FUNCTION: Common routine for allocating memory and detecting allocation error.  All memory allocated is zeroed.
 *
 * RETURN VALUE DESCRIPTION: Pointer to the memory block allocated.  If the size requested could not be allocated
 *     then the program will exit with a -1 (i.e. the function will never return on error)..
 */

char *get_zeroed_mem (amount, size)
int amount,size;     /* requested allocation size in bytes (INPUT) */
{
	char *ptr;   /* pointer to memory block allocated */

	if ((ptr = (char *) calloc(amount,size)) != NULL) return(ptr);      /* if success - return */
	perror("calloc:");                         /* print the system message for the failure */
	exit(-1);                                  /* fatal error - exit the program */
}


/*
 * NAME: get_uniq_id
 *
 * FUNCTION: Common routine for setting the values of stucture unique_id.
 *
 * RETURN VALUE DESCRIPTION: Zero is always returned.
 */

int get_uniq_id(value, uniq_id)

char  *value;		 /* convert value to uniq_id */
struct unique_id  *uniq_id;
{
	char *ptr;
        char temp[20];
        int count, strlen();

	
	if ((count = strlen(value)) > 8) {		/* more than 8 digits ? */
		strcpy(temp, value);	/* make a copy to be modified */
		ptr = temp + count - 8;	  /* get the 8 least significant digits first */
		sscanf(ptr, "%8x", &uniq_id->word2);
		*ptr = 'G';
		sscanf(temp,"%x", &uniq_id->word1); /* then the most significants */
	}
	else {						/* 8 digits or less */
		sscanf(value, "%x", &uniq_id->word2);
		uniq_id->word1 = 0;                     /* pad the most significants with 0 */
	}
	return(0);
}

/*
 * NAME: lvm_rc
 *
 * FUNCTION: Tranlate low-level error code to user return code.
 *
 * RETURN VALUE DESCRIPTION: The return code
 *
 */

int lvm_rc (error_code)
int error_code;   /* error code to be translated */
{
	int rc;

        switch (error_code) {
 
             case 0:        /* No error occurred */
		rc=0;
                break;
	     case LVM_VGDELETED:
             case LVM_CHKVVGOUT:
             case LVM_REMRET_INCOMP:
		rc=2;
                break;
             case LVM_VGMEMBER:
                rc=3;
                break;
             case LVM_MEMACTVVG:
                rc=4;
                break;
             case LVM_NOQUORUM:
                rc=5;
                break;
             case LVM_MISSPVNAME:
                rc=6;
                break;
             case LVM_MISSINGPV:
                rc=7;
                break;
             default:
		rc=1;
                break;
        }

	return(rc);
}

