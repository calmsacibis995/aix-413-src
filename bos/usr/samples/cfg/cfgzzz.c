static char sccsid[] = "@(#)72	1.2  src/bos/usr/samples/cfg/cfgzzz.c, cfgsamp, bos411, 9428A410j 6/7/91 11:52:59";

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
 * FUNCTION: Configure method for pseudo device of type yyy
 *
 * interface:
 * cfgzzz -l <logical_name> [-<1|2>]
*/

/*
 * Include files needed for this module follow
 */
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>


/* external functions */
extern int	*genminor();

/*
 * NAME: main
 *
 * FUNCTION:
 *
 *   The purpose of cfgzzz is to configure the ZZZ pseudo device into
 *   the system and make it ready for use.  It is called with the name
 *   of the logical device representing the ZZZ and possibly a flag
 *   representing which phase the configuration is taking place in.
 *
 */

main(argc, argv)
int	argc;
char	*argv[];
{
	struct cfg_dd cfg;		/* sysconfig command structure */

	char	*logical_name;		/* logical name to configure */
	char	sstring[256];		/* search criteria pointer */

	struct	CuDv cusobj;		/* customized device object storage */
	struct	PdDv preobj;		/* predefined device object storage */

	mid_t	kmid;			/* module id from loader	*/
	dev_t	devno;			/* device number for config_dd	*/

	long	majorno, minorno;	/* major and minor numbers	*/
	long	*minor_list;		/* list returned by getminor	*/
	int	how_many;		/* number of minors in list	*/
	int	ipl_phase;		/* ipl phase			*/
	int	rc;			/* return codes go here		*/
	int	c;			/* flag char returned by getopt */
	int	errflg = 0;		/* error flag */
	char	*odm_path;		/* path to ODM directory	*/

	extern  int     optind;         /* for getopt function */
	extern  char    *optarg;        /* for getopt function */

	/* 
	 * Set up initial variable values 
	 */
	ipl_phase = RUNTIME_CFG;

	/* 
	 * Parse command line 
	 */
	while ((c = getopt(argc,argv,"l:12")) != EOF) {
		switch (c) {
		case 'l':
			if (logical_name != NULL)
				errflg++;
			logical_name = optarg;
			break;
		case '1':
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE1;
			break;
		case '2':
			if (ipl_phase != RUNTIME_CFG)
				errflg++;
			ipl_phase = PHASE2;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		exit(E_ARGS);
	}

	/*
	 * Check command-line parameters for validity
	 */
	if (logical_name == NULL) {
		exit(E_LNAME);
	}

	/* 
	 * Start up odm then lock the database until through.
	 */
	if ((rc = odm_initialize()) < 0) {
		exit(E_ODMINIT);
	}

	/* lock the database */
	if (odm_lock("/etc/objrepos/config_lock",0) == -1)
		err_exit(E_ODMLOCK);

	/* 
	 * Search CusDevices for customized object with this logical name 
	 */
	sprintf(sstring, "name = '%s'", logical_name);
	if ((rc = (int)odm_get_first(CuDv_CLASS, sstring, &cusobj)) == 0) {
		err_exit(E_NOCuDv);
	} else if (rc == -1) {
		err_exit(E_ODMGET);
	}

	/*
	 * Get the predefined object for this device. This object is located
	 * searching the predefined devices object class based on the unique
	 * type link descriptor in the customized device.
	 */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	if ((rc = (int)odm_get_first(PdDv_CLASS, sstring, &preobj)) == 0) {
		err_exit(E_NOPdDv);
	} else if (rc == -1) {
		err_exit(E_ODMGET);
	}

	/*
	 * If this device is being configured during an ipl phase, then
	 * display this device's LED value on the system LEDs.
	 */
	if (ipl_phase != RUNTIME_CFG)
		setleds(preobj.led);

	/*
	 * Check to see if the device is defined.
	 * We actually go about configuring the device only if the device
	 * is defined. Configuring the device in this case refers to the
	 * process of checking for attribute consistency, building a DDS,
	 * loading the driver, etc...
	 */
	if (cusobj.status == DEFINED) {

		if ((rc=build_dds(logical_name,&cfg.ddsptr,&cfg.ddslen))<0) {
			err_exit(rc);
		}

		/*
		 * Call loadext() to load device driver.
		 */
		if ((cfg.kmid=loadext(preobj.DvDr,TRUE,FALSE)) == NULL) {
			err_exit(E_LOADEXT);
		}

		/*
		 * genmajor() will create a major number for this device
		 * if one has not been created yet. If one already exists,
		 * it will return that one.
		 */
		if ((majorno = genmajor(preobj.DvDr)) == -1) {
			loadext(preobj.DvDr, FALSE, FALSE);
			err_exit(E_MAJORNO);
		}

		/*
		 * Get minor number.
		 */
		minor_list = getminor(majorno, &how_many, logical_name);
		if (minor_list == NULL || how_many == 0) {
			if ((rc = generate_minor(logical_name, majorno, &minorno))
			     != E_OK) {
				reldevno(logical_name, TRUE);
				loadext(preobj.DvDr, FALSE, FALSE);
				err_exit(rc);
			}
		} else
			minorno = *minor_list;


		/* 
		 * Create devno for this device 
		 */
		cfg.devno = makedev(majorno, minorno);

		/*
		 * Now call sysconfig() to configure the driver.
		 */
		cfg.cmd = CFG_INIT;
		if (sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd)) == -1) {
			loadext(preobj.DvDr, FALSE, FALSE);
			err_exit(E_CFGINIT);
		}

		/*
		 * Now make the special files that this device will
		 * be accessed through.
		 */
		if ((rc=make_special_files(logical_name,cfg.devno)) != E_OK) {
			err_undo(cfg.devno);
			loadext(preobj.DvDr, FALSE, FALSE);
			err_exit(rc);
		}

		/*
		 * Update the customized object to AVAILABLE status.
		 */
		cusobj.status = AVAILABLE;
		if ((rc = odm_change_obj(CuDv_CLASS, &cusobj)) < 0) {
			err_exit(E_ODMUPDATE);
		}

	}


	/*
	 * Terminate the ODM lock then terminate ODM and exit with success
	 */
	odm_terminate();
	exit(E_OK);

}


/*
 * NAME: err_exit
 *
 * FUNCTION: Terminates ODM.  Used to back out on an error.
 *
 * NOTES:
 *
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:
 *               None
 */

err_exit(exitcode)
char	exitcode;
{
	/*
	 * Terminate the ODM
	 */
	odm_terminate();
	exit(exitcode);
}


/*
 * NAME: err_undo
 *
 * FUNCTION: Terminates the device.  Used to back out on an error.
 *
 * NOTES:
 *
 * void
 *   err_undo( devno )
 *      devno = The device's devno.
 *
 * RETURNS:
 *               None
 */
err_undo(devno)
dev_t   devno;				/* The device's devno */
{
	struct cfg_dd cfg;		/* sysconfig command structure */

	/* set up structure to terminate the device */
	cfg.devno = devno;
	cfg.kmid = (mid_t)0;
	cfg.ddsptr = (caddr_t) NULL;
	cfg.ddslen = (int)0;
	cfg.cmd = CFG_TERM;

	/* call sysconfig to terminate the device */
	sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd ));
	return;
}


/*
 * NAME: build_dds
 *
 * FUNCTION:
 *   build_dds will allocate memory for the dds structure, reporting any
 *   errors, then open the Customized Attribute Class to get the attribute
 *   objects needed for filling the dds structure.
 *
 * RETURNS:
 *    0 on success
 *    positive return code on failure
 *
 */

build_dds(lname, dds_out, size)
char    *lname;                 /* logical name of device */
char    **dds_out;              /* pointer to dds structure for return */
int     *size;                  /* pointer to dds structure size */
{

	/*
	 * This function builds the dds for the ZZZ device by
	 * getting the values needed to complete the structure
	 * from the predefined and customized attribute object
	 * classes. If an attribute can't be found, or another
	 * error occurs, this function will return the appro-
	 * priate error code. If the dds gets built completely,
	 * then its address is returned to the caller using
	 * the dds_out pointer and its size is returned through
	 * the size pointer.
	 */

}


/*
 * NAME: generate_minor
 *
 * FUNCTION: Device dependent routine for generating the device minor number
 *
 * RETURNS:
 *   minor number success
 *   E_MINORNO on failure
 */

int
generate_minor(lname, majno, minorno)
char    *lname;     /* logical device name */
long    majno;      /* device major number */
long    *minorno;   /* device minor number */
{
	long    *minorptr;

	/*
	 * use genminor() to create and reserve the minor
	 * numbers used by this device.
	 */

	minorptr = genminor(lname, majno, -1, 1, 1, 1);

	if ( minorptr == (long *)NULL )
		/* error generating minor number */
		return(E_MINORNO);

	*minorno = *minorptr;
	return(E_OK);
}


/*
 * NAME: make_special_files
 *
 * FUNCTION:
 *      This routine creates whatever special files are needed for
 *      the ZZZ device.
 *
 * RETURNS:
 *   0 - success
 *   positive return code on failure
 */

int
make_special_files(lname, devno)
char    *lname;         /* logical device name */
dev_t devno;            /* major/minor number */
{
	struct stat buf;
	extern int      errno;

	/*
	 * This function creates the special files used to talk
	 * to the ZZZ device. If an error occurs during this
	 * process, the function returns with the appropriate
	 * error code--probably E_MKSPECIAL--otherwise it re-
	 * turns with a good return code (E_OK).
	 */

}


