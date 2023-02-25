static char sccsid[] = "@(#)80	1.2  src/bos/usr/samples/cfg/ucfgyyy.c, cfgsamp, bos411, 9428A410j 6/7/91 11:58:28";

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
 * FUNCTION: Unconfigure method for tape device of type yyy
 *
 * interface:
 * ucfgyyy -l <logical_name>
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>


extern	int errno;		/* System Error Number */

main(argc,argv)
int argc;
char *argv[];
{

	struct	cfg_dd cfg;		/* sysconfig command structure */
	char    *logical_name;          /* logical name to unconfigure */
	char	sstring[256];		/* search criteria */

	struct	CuDv	cusobj;		/* customized devices object */
	struct	PdDv	preobj;		/* predefined devices object */
	struct  CuDv    childcusobj;    /* child cust. devices object */

	struct	Class	*cusdev;	/* customized devices class handle */

	long	majorno,minorno;	/* major and minor numbers */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	extern  int optind;             /* flag parsed by getopt() */
	extern  char *optarg;           /* argument parsed by getopt() */


	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	errflg = 0;
	logical_name = NULL;

	while ((c = getopt(argc,argv,"l:")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		exit(E_ARGS);
	}

	/*****                                                          */
	/***** Validate Parameters                                      */
	/*****                                                          */

	/* logical name must be specified */
	if (logical_name == NULL)
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
	sprintf(sstring,"name = '%s'",logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* failed to get an object */
		err_exit(E_NOCuDv);
	} else if (rc==-1) {
		/* ODM error */
		err_exit(E_ODMGET);
	}

	/*
	 * If device state is DEFINED, then already unconfigured 
	 * so just exit.
	 */
	if (cusobj.status == (short) DEFINED)
		/* Device is already unconfigured */
		err_exit(E_OK);

	/* get device's predefined object */
	sprintf(sstring,"uniquetype = '%s'",cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&preobj);
	if (rc==0) {
		/* failed to get an object */
		err_exit(E_NOPdDv);
	} else if (rc==-1) {
		/* ODM error */
		err_exit(E_ODMGET);
	}

	/*
	 * Call sysconfig() to "terminate" the device.
	 * If fails with EBUSY, then device instance is "open",
	 * and device cannot be "unconfigured".  Any other errno
	 * returned will be ignored since we MUST unconfigure the
	 * device even if it reports some other error.
	 */

	/* get major number of device */
	majorno = genmajor(preobj.DvDr);
	if (majorno == -1)
		/* failed to get major number */
		err_exit(E_MAJORNO);

	/* get minor number */
	minor_list = getminor(majorno, &how_many, logical_name);
	if (minor_list == NULL || how_many == 0)
		/* failed to get minor numbers */
		err_exit(E_MINORNO);

	minorno = *minor_list;

	/* create devno for this device */
	cfg.devno = makedev(majorno,minorno);
	cfg.kmid  = (mid_t)0;
	cfg.ddsptr = (caddr_t) NULL;
	cfg.ddslen = (int)0;
	cfg.cmd = CFG_TERM;	

	if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
		if (errno == EBUSY)
			/* device can not be unconfigured */
			err_exit(E_SYSCONFIG);
	}

	/*
	 * Call loadext() to unload device driver.
	 * loadext() will unload the driver only if 
	 * device is last instance of driver configured.
	 */
	cfg.kmid = loadext(preobj.DvDr,FALSE,FALSE);
	if (cfg.kmid == NULL)
		/* error unloading device driver */
		err_exit(E_UNLOADEXT);

	/*
	 * Change the status field of device to "DEFINED"
	 */
	cusobj.status = (short)DEFINED;
	if (odm_change_obj(cusdev,&cusobj) == -1)
		/* Could not change customized device object */
		err_exit(E_ODMUPDATE);

	/* close the CuDv object class */
	if (odm_close_class(CuDv_CLASS) == -1)
		/* Error closing CuDv object class */
		err_exit(E_ODMCLOSE);

	/* shut down the odm and return successful */
	odm_terminate();
	exit(E_OK);

}

/*
 * NAME: err_exit
 *                                                                    
 * FUNCTION: Error exit routine.
 *                                                                    
 */  
err_exit(exit_code)
int     exit_code;              /* Error exit code */
{
	odm_close_class(CuDv_CLASS);
	odm_terminate();
	exit(exit_code);
}
