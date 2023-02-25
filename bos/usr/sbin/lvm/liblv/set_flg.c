static char sccsid[] = "@(#)93	1.1  src/bos/usr/sbin/lvm/liblv/set_flg.c, cmdlvm, bos411, 9428A410j 2/1/90 16:31:01";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: setflg
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
 * FILENAME: set_flg.c
 *
 * FILE DESCRIPTION: Source file for common functions used by the
 *      low and intermediate Logical Volume Manager programs.
 */

#include <stdio.h>

/*
 * EXTERNAL PROCEDURES CALLED:
 */

extern int getflg();    /* getflg defined by the calling program */

/*
 * EXTERNAL VARIABLES
 */

extern int optind;     /* index used in command parameter parsing */


/*
 * NAME: setflg
 *
 * FUNCTION: Supports position independence of command line options. The
 * function "getflg" uses "getopt" to parse the command line options.
 * The function "getopt" stops parsing the command list when the first non-
 * option field is encountered. The function "setflg" saves each non-option
 * field in a seperate list (if list ptr is not NULL) and calls "getflag" with
 * the remaining elements in the old list. This continues until the old list
 * is completely parsed.  The new list contains all the non-option fields that
 * were on the command line.
 *
 * (NOTES:) The string "--" in the command line signals that no more command
 *     options will follow; therefore, only non-options remain to be parsed.
 *
 *     If a NULL pointer is input for the non-option list, no non-option
 *     fields will be saved.  This prevents the calling routines from
 *     being required to allocate a list buffer when the non-option fields
 *     are not used.
 *
 * RETURN VALUE DESCRIPTION: number of non-option fields saved
 *
 */

int setflg(argc,argv,save)

int argc;        /* the number of command line strings in argv */
char **argv;     /* the command line strings to be parsed */
char **save;     /* buffer to store list of non-option fields */
{
	int retval;   /* temperary also used for the return value */
	int cnt=0;    /* index into the save array - the next empty slot */

	/* getflg will set the command option flags and command option values.   */
	/* On return - argv will be pointing to a non-option field, if existing. */
	while (retval = getflg(argc,argv), argc -= optind) {   /* parse each cmd line string */

		if (retval < 0) return(retval);                /* if getflg error then exit */

		else if (!strcmp("--",argv[optind-1]))  {
			/* store the remaining command line strings as non-options */
			for (argv = &argv[optind]; *argv; ++argv)
			    if (save != NULL) save[cnt++] = *argv;
			break;
		}

		else {  if (save != NULL)
			    save[cnt++] = argv[optind]; /* save non-option field of cmd line */
			argv = &argv[optind];           /* increment to next cmd line string */
		}

		optind = 1;                   /* incremented by getopt so reset */
	}

	if (retval < 0) return(retval);       /* if getflg error on last call then exit */
	if (save != NULL) save[cnt] = NULL;   /* mark the end of the non-options list   */
	return(cnt);                          /* return the number of non-options saved */
}


