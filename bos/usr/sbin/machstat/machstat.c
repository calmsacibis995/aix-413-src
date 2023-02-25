static char sccsid[] = "@(#)03  1.3  src/bos/usr/sbin/machstat/machstat.c, cmdoper, bos411, 9436B411a 8/30/94 14:02:49";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: machstat
 *
 * ORIGINS: 27, 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * NAME: machstat
 * FUNCTION: returns status of selected system registers
 *
 *   -p  Power Status Register: 0-9 bits
 *   -f  Power Status Register: 10-13 bits
 *
 * REQUIREMENTS:
 *	This command will have no output to stderr or stdout.
 *	The exit code is either the value of the register checked
 *		or the 255 if there is an error.
 */

#include <sys/file.h>
#include <sys/mdio.h>
#include <sys/iocc.h>
#include <stdio.h>

#define ERROR	255	/* error exit code */
#define EPOW	0	/* check epow bits in pksr */
#define FAN	1	/* check 10-13 bits in pksr */

#define FAN_MASK	0x003C0000 /* mask all bits but 10-13 */

void psr_check(int);

main(int argc, char **argv)
{

	int opt;

	/* 
	 * Verify that there is at least ONE option and that there is no
	 * more than ONE option to the command line
	 */
	if (argc != 2)
		exit(ERROR);

	/*
	 * Redirect stderr to /dev/null to hide any getopt errors
	 */

	freopen("/dev/null", "w", stderr);

	/*
	 * Validate the option on the command line and call the 
	 * appropriate function
	 */
	while ((opt = getopt(argc, argv, "pf")) != EOF) {
		switch (opt) {

			case 'p':	/* Power Status Register */
				psr_check(EPOW);
				break;
			case 'f':	/* Power Status Register: 10-13 bits */
				psr_check(FAN);
				break;
			default:	/* Error if it is an invalid option */
				exit(ERROR);

		} /* end switch statement */
	} /* end while(getopt) */
} /* end main */

/*
 * NAME: psr_check
 * FUNCTION: Returns the value of the Power Status Register
 */

void psr_check(int position)
{
	int 		fd;		/* file descriptor for /dev/bus0 */
	unsigned int	ev = ERROR;	/* exit value */
	MACH_DD_IO 	mdd;		/* structure for register output
					 * for more info see sys/mdio.h */

	/* Structure as defined in sys/mdio.h
	 * typedef struct mdio {
	 *	ulong md_addr;      specified address
	 *	ulong md_size;      size of md_data
	 *	int md_incr;        increment type MV_BYTE or MV_WORD
	 *	char *md_data;      pointer to space of size md_size
	 *	int md_sla;         entry buid value, exit error code
	 * } MACH_DD_IO;
	 */

	/*
	 * Open /dev/bus0 for reading the psr
	 */

	if ((fd = open("/dev/bus0", O_RDONLY)) == -1)
		exit(ERROR);

	/*
	 * Setup the register structure for read
	 */

	mdd.md_data = (char *)&ev;
	mdd.md_size = sizeof(ev);

	/*
	 * Read the data from /dev/bus0. ERROR out if there is an error.
	 */

	if (ioctl(fd, MIOGETPS, &mdd) != 0)
		exit(ERROR);

	if (position == FAN) {
        	/* Mask off all bits but  10-13 */

		ev &= FAN_MASK;

		/*
	 	* shifting to convert the int to the 0 to 15 exit code range
	 	*/

		exit((char)(ev>>14));
	}
	else {
		/*
	 	* Mask off the 4 MSB as defined in sys/iocc.h as "EPOW_MASK 0xF0000000"
	 	*/

		ev &= EPOW_MASK;

		/*
	 	* shifting to convert the int to the 0 to 15 exit code range
	 	*/

		exit((char)(ev>>24));
	}

} /* exit psr_check */

