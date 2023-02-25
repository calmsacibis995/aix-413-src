static char sccsid[] = "@(#)78	1.2  src/bos/usr/samples/cfg/startzzz.c, cfgsamp, bos411, 9428A410j 6/7/91 11:56:30";

/*
 * COMPONENT_NAME: (BOSSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
*/


/*
 * Include files needed for this module follow
 */
#include <sys/types.h>
#include <sys/errno.h>
#include <stdio.h>

/*
 * odm interface include files
 */
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>

int	lock_id;			/* global odm lock variable */

/*
 * NAME:  main
 *
 * FUNCTION: 
 *
 *   The purpose of this program is to define a ZZZ pseudo device.  It
 *   will check to see if one has already been defined, and , if one
 *   doesn't exist, it will define them. In either case the logical name
 *   of the device will be returned to the config manager via stdout.
 *
 */

main(argc, argv)
int	argc;
char	*argv[];
{
	struct CuDv cudv;		/* customized dependency object */
	struct PdDv pddv;		/* customized dependency object */
	char	crit[256];		/* search criteria string */
	char	*odm_path;		/* path to current odm directory */
	int	rc;			/* return status */

	/*
	 * Start up the ODM
	 */
	if ( (rc = odm_initialize()) < 0 ) {
		/* odm initialize failed */
		exit(E_ODMINIT);
	}

	/*
	 * Lock the database so we get accurate data from it.
	 */

	odm_path = getenv( "ODMDIR" );
	if ((lock_id = odm_lock(odm_path, 2)) == -1) {
		/* failed to lock the data base */
		err_exit(E_ODMLOCK);
	}

	/*
	 * Obtain the predefined device object for the ZZZ type, this
	 * will give us the information needed to define a ZZZ device.
	 */

	if ((rc = (int)odm_get_first(PdDv_CLASS,"type='zzz'",&pddv)) == 0) {
		/* did not get predefined object for ZZZ device */
		err_exit(E_NOPdDv);
	} else if (rc == -1) {
		/* odm error while trying to get object */
		err_exit(E_ODMGET);
	}

	/*
	 * See if a ZZZ device has already been defined. We will
	 * need to run the define method if there isn't one defined
	 * already. In either case the logical name of the device
	 * gets returned to the config manager over stdout. The
	 * config manager is responsible for getting the device
	 * configured. Note: this example is for a device that allows
	 * only one instance of itself to be defined/configured.
	 */

	sprintf(crit, "name='%s0'", pddv.prefix);
	rc = (int)odm_get_first(CuDv_CLASS, crit, &cudv);
	if (rc == 0) {

		/*
		 * No customized device object exists. We need to
		 * run the define method for ZZZ devices to create
		 * one. First unlock the database so the define
		 * method can access it, then invoke the define
		 * method for the ZZZ device type.
		 */

		odm_unlock(lock_id);
		if ((rc = odm_run_method(pddv.Define,"-c zzz -s node -t zzz", 
		    (char **)NULL,(char **)NULL)) < 0 ) {
			err_exit(E_ODMRUNMETHOD);
		}

	} else if (rc == -1) {
		/* had an odm error trying to get object */
		err_exit(E_ODMGET);
	} else
		/* print the logical name of the device to stdout */
		fprintf(stdout,"%s",cudv.name);

	/*
	 * Terminate the ODM connection and exit with success.
	 */

	odm_terminate();
	exit(E_OK);

}


err_exit(exitcode)
char	exitcode;
{
	odm_unlock(lock_id);
	odm_terminate();
	exit(exitcode);
}


