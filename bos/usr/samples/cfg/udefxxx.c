static char sccsid[] = "@(#)81	1.2  src/bos/usr/samples/cfg/udefxxx.c, cfgsamp, bos411, 9428A410j 6/7/91 11:59:02";

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
 * FUNCTION: Undefine method for mca adapter of type xxx
 *
 * interface:
 * udefgyyy -l <logical_name>
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>


main(argc,argv)
int argc;
char *argv[];

{
	extern  int     optind;         /* for getopt function */
	extern  char    *optarg;        /* for getopt function */

	char *devname;			/* device name to undefine */
	char sstring[256];		/* search criteria pointer */

	struct CuDv  cusobj;	 	/* customized device object */
	struct CuDv  cuschildobj;       /* customized child object */

	struct Class *cusdev;		/* customized device class handle */

	int     rc;                     /* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */


  	/*
	 * Parse Command Line
	 */
	errflg = 0;
	devname = NULL;
	while ((c = getopt(argc,argv,"l:")) != EOF) {
		switch (c) {
		case 'l':
			if (devname != NULL)
				errflg++;
			devname = optarg;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		exit(E_ARGS);
	}

	/* logical name must be specified */
	if (devname == NULL)
		exit(E_LNAME);

	/* start up odm */
	if (odm_initialize() == -1)
		/* initialization failed */
		exit(E_ODMINIT);

	/* lock the database */
	if (odm_lock("/etc/objrepos/config_lock",0) == -1)
		/* failed to get lock */
		err_exit(E_ODMLOCK);

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1)
		/* error opening class */
		err_exit(E_ODMOPEN);

	/* Get Customized Device Object */
	sprintf(sstring,"name = '%s'",devname);	
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		err_exit(E_NOCuDv);
	} else if (rc==-1) {
		/* ODM error */
		err_exit(E_ODMGET);
	}

	/* Device state must be DEFINED in order to be undefined */
	if (cusobj.status != DEFINED)
		/* device is not defined */
		err_exit(E_DEVSTATE);

	/* See if this device has children.  If it does, it can  */
	/* not be undefined                                      */
	sprintf(sstring,"parent = '%s'",devname);
	rc = (int)odm_get_first(cusdev,sstring,&cuschildobj);
	if (rc==-1) {
		/* ODM error */
		err_exit(E_ODMGET);
	} else if (rc!=0) {
		/* Has children!! */
		err_exit(E_CHILDSTATE);
	}

	/* Everything is OK.  Begin deleting objects. */

	/* Delete all attributes for this device */
	sprintf(sstring,"name = '%s'",devname);
	if ((odm_rm_obj(CuAt_CLASS,sstring)) == -1)
		/* error deleting object */
		err_exit(E_ODMDELETE);

	/* reldevno() releases major and minor no and removes the */
	/* special files from /dev.                               */
	reldevno(devname,TRUE);

	/* Delete Cutomized Device Object for this device */
	sprintf(sstring,"name = '%s'",devname);
	if ((odm_rm_obj(cusdev,sstring)) == -1)
		/* error deleting object */
		err_exit(E_ODMDELETE);

	/* Close Customized Device Object Class */
	if (odm_close_class(CuDv_CLASS) == -1)
		/* ODM error */
		err_exit(E_ODMCLOSE);

	/* Terminate ODM */
	odm_terminate();

	/* Successful exit */
	exit(0);

}


/*
 * NAME: err_exit
 *                                                                    
 * FUNCTION: Exit with an error message. 
 *                                                                    
 */  
err_exit(exit_code)
int     exit_code;              /* Error exit code */
{
	odm_close_class(CuDv_CLASS);
	odm_terminate();
	exit(exit_code);
}

