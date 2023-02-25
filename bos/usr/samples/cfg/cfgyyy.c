static char sccsid[] = "@(#)71	1.2  src/bos/usr/samples/cfg/cfgyyy.c, cfgsamp, bos411, 9428A410j 6/7/91 11:52:23";

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
 * FUNCTION: Configure method for tape device of type yyy
 *
 * interface:
 * cfgyyy -l <logical_name> [-<1|2>]
*/


/* header files needed for compilation */
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>

/* external functions */
extern long	genmajor();


/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

	struct cfg_dd cfg;		/* sysconfig command structure */

	char	*logical_name;		/* logical name to configure */
	char	*phase1, *phase2;	/* ipl phase flags */
	char	sstring[256];		/* search criteria pointer */

	struct Class *cusdev;		/* customized devices class ptr */
	struct Class *predev;		/* predefined devices class ptr */
	struct Class *cusvpd;           /* customized vpd class ptr */

	struct CuDv cusobj;		/* customized device object storage */
	struct PdDv preobj;		/* predefined device object storage */
	struct CuDv parobj;		/* customized device object storage */
	struct CuDv dmyobj;		/* customized device object storage */

	int     majorno;                /* major number assigned to device */
	int     minorno;                /* minor number assigned to device */
	long    *minor_list;            /* list returned by getminor */
	int     how_many;               /* number of minors in list */

	int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
	int	rc;			/* return codes go here */
	int     errflg,c;               /* used in parsing parameters   */

	/*****                                                          */
	/***** Parse Parameters                                         */
	/*****                                                          */
	ipl_phase = RUNTIME_CFG;
	errflg = 0;
	logical_name = NULL;

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
	if (errflg)
		/* error parsing parameters */
		exit(E_ARGS);

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
		err_exit(E_ODMLOCK);

	/* open customized devices object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1)
		err_exit(E_ODMOPEN);

	/* search for customized object with this logical name */
	sprintf(sstring, "name = '%s'", logical_name);
	rc = (int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0) {
		/* No CuDv object with this name */
		err_exit(E_NOCuDv);
	} else if (rc==-1) {
		/* ODM failure */
		err_exit(E_ODMGET);
	}

	/* open predefined devices object class */
	if ((int)(predev = odm_open_class(PdDv_CLASS)) == -1)
		err_exit(E_ODMOPEN);

	/* get predefined device object for this logical name */
	sprintf(sstring, "uniquetype = '%s'", cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(predev, sstring, &preobj);
	if (rc==0) {
		/* No PdDv object for this device */
		err_exit(E_NOPdDv);
	} else if (rc==-1) {
		/* ODM failure */
		err_exit(E_ODMGET);
	}

	/* close predefined device object class */
	if (odm_close_class(predev) == -1)
		err_exit(E_ODMCLOSE);

	/****************************************************************
	  If tape yyy is being configured during an ipl phase, then
	  display its LED value on the system LEDs.
	  ****************************************************************/
	if (ipl_phase != RUNTIME_CFG)
		setleds(preobj.led);

	/******************************************************************
	  Check to see if the tape is already configured (AVAILABLE).
	  We actually go about the business of configuring the tape
	  only if it is not configured yet. Configuring the tape
	  refers to the process of checking parent status, loading the
	  device driver, allocating major and minor numbers, creating
	  special files, building DDS, etc....
	  ******************************************************************/
	if (cusobj.status == DEFINED) {

		/* The tape drive is not configured */

		/* get the device's parent object */
		sprintf(sstring, "name = '%s'", cusobj.parent);
		rc = (int)odm_get_first(cusdev,sstring,&parobj);
		if (rc==0) {
			/* Parent device not in CuDv */
			err_exit(E_NOCuDvPARENT);
		} else if (rc==-1) {
			/* ODM failure */
			err_exit(E_ODMGET);
		}

		/* Parent MUST be available to continue */
		if (parobj.status != AVAILABLE)
			/* parent is not AVAILABLE */
			err_exit(E_PARENTSTATE);

		/* make sure that no other devices are configured     */
		/* at this location                                   */
		sprintf(sstring, "parent = '%s' AND connwhere = '%s' AND status = %d",
			cusobj.parent, cusobj.connwhere, AVAILABLE);
		rc = (int)odm_get_first(cusdev,sstring,&dmyobj);
		if (rc == -1) {
			/* odm failure */
			err_exit(E_ODMGET);
		} else if (rc) {
			/* Error: device config'd at this location */
			err_exit(E_AVAILCONNECT);
		}


		/* call loadext to load the device driver */
		if ((cfg.kmid = loadext(preobj.DvDr, TRUE, FALSE)) == NULL)
			/* error loading device driver */
			err_exit(E_LOADEXT);

		/* get major number      */
		if ((majorno = genmajor(preobj.DvDr)) == -1) {
			(void)loadext(preobj.DvDr,FALSE,FALSE);
			err_exit(E_MAJORNO);
		}

		/* get minor number      */
		minor_list = getminor(majorno,&how_many,logical_name);
		if (minor_list == NULL || how_many == 0) {
			/* Need to allocate minor numbers */
			/* Use genminor to request eight minor numbers.  These */
			/* must be contiguous, starting on a multiple-of-eight */
			/* boundary. */
			minor_list = genminor(logical_name, majorno, -1, 8, 1, 8);
			if (minor_list == NULL) {
				/* the genminor service failed */
				(void)loadext(preobj.DvDr,FALSE,FALSE);
				err_exit(E_MINORNO);
			}
		}

		minorno = *minor_list;

		/* create devno for this device */
		cfg.devno = makedev(majorno, minorno);

		/* make special files      */
		rc = make_special_files(logical_name, cfg.devno);
		if (rc) {
			/* error making special files */
			(void)loadext(preobj.DvDr,FALSE,FALSE);
			err_exit(E_MKSPECIAL);
		}

		/* build the DDS  */
		rc = build_dds(logical_name, &cfg.ddsptr, &cfg.ddslen);
		if (rc) {
			/* error building dds */
			(void)loadext(preobj.DvDr,FALSE,FALSE);
			err_exit(rc);
		}

		/* call sysconfig to pass DDS to driver */
		cfg.cmd = CFG_INIT;
		if (sysconfig(SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
			/* error configuring device */
			(void)loadext(preobj.DvDr,FALSE,FALSE);
			err_exit(E_CFGINIT);
		}

		/* download microcode */
		rc = download_microcode(logical_name);
		if (rc) {
			/* error downloading microcode */
			err_undo(cfg.devno);
			(void)loadext(preobj.DvDr,FALSE,FALSE);
			err_exit(rc);
		}

		/* get VPD for this device */
		rc = query_vpd(logical_name);
		if (rc) {
			/* failed to get VPD */
			err_undo(cfg.devno);
			(void)loadext(preobj.DvDr,FALSE,FALSE);
			err_exit(rc);
		}

		/* update customized device object with a change operation */
		cusobj.status = AVAILABLE;
		if (odm_change_obj(cusdev, &cusobj) == -1)
			/* ODM failure */
			err_exit(E_ODMUPDATE);

	} /* end if (tape device is not AVAILABLE) then ... */

	/* close customized device object class */
	if (odm_close_class(cusdev) == -1)
		err_exit(E_ODMCLOSE);

	odm_terminate();
	exit(E_OK);
}

/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * void
 *   err_exit( exitcode )
 *      exitcode = The error exit code.
 *
 * RETURNS:     None
 */
err_exit(exitcode)
char    exitcode;
{
	/* Close any open object class */
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_close_class(CuAt_CLASS);

	/* Terminate the ODM */
	odm_terminate();
	exit(exitcode);
}


/*
 * NAME: err_undo
 *
 * FUNCTION: Terminates the device.  Used to back out on an error.
 *
 * void
 *   err_undo( devno )
 *      devno = The device's devno.
 *
 * RETURNS:     None
 */

err_undo(devno)
dev_t   devno;                  /* The device's devno */
{
	struct  cfg_dd cfg;             /* sysconfig command structure */

	/* terminate device */
	cfg.devno = devno;
	cfg.kmid = (mid_t)0;
	cfg.ddsptr = (caddr_t) NULL;
	cfg.ddslen = (int)0;
	cfg.cmd = CFG_TERM;

	(void)sysconfig(SYS_CFGDD,&cfg,sizeof(struct cfg_dd));
	return;
}


/*
 * NAME: query_vpd
 *
 * FUNCTION: Obtaining VPD data from the adapter
 *
 * RETURNS:
 *   0 - success
 */
int
query_vpd(logical_name)
	char    *logical_name;          /* logical name of adapter */
{
	char    vpd[VPDSIZE];           /* vpd data */
	struct Class *cusvpd;           /* customized vpd class ptr */
	struct CuVPD vpdobj;            /* customized vpd object */
	char    sstring[256];           /* search criteria string */
	int     rc;                     /* odm return codes */


	/* code to be added here to obtain VPD from the tape drive */
	/* remainder of code assumes VPD is in the string vpd.     */

	/* open customized vpd object class */
	if ((int)(cusvpd = odm_open_class(CuVPD_CLASS)) == -1)
		return(E_ODMOPEN);

	/* search for customized vpd object with this logical name */
	sprintf(sstring, "name = '%s' and vpd_type = '%d'",
			logical_name,HW_VPD);
	rc = (int)odm_get_first(cusvpd,sstring,&vpdobj);
	if (rc==-1) {
		/* ODM failure */
		return(E_ODMGET);
	}
	if (rc==0) {
		/* need to add vpd object */
		strcpy(vpdobj.name,logical_name);
		vpdobj.vpd_type = HW_VPD;
		strcpy(vpdobj.vpd,vpd);
		if (odm_add_obj(cusvpd,&vpdobj) == -1) {
			return(E_ODMADD);
		}
	} else {
		/* see if vpd object needs to be updated */
		if (strcmp(vpdobj.vpd,vpd)) {
			strcpy(vpdobj.vpd,vpd);
			if (odm_change_obj(cusvpd,&vpdobj) == -1) {
				return(E_ODMUPDATE);
			}
		}
	}

	/* close customized vpd object class */
	if (odm_close_class(CuVPD_CLASS) == -1)
		return(E_ODMCLOSE);

	return(0);
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
 */
build_dds(lname, dds_out, size)
char    *lname;                 /* logical name of device */
char    **dds_out;              /* pointer to dds structure for return */
int     *size;                  /* pointer to dds structure size */
{
	/*
	 * This function builds the dds for the yyy tape device
	 * by getting the values needed to complete the structure
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
 * NAME: make_special_files
 *
 * FUNCTION:
 *      This routine creates whatever special files are needed for
 *      the yyy tape device.
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
	 * to the yyy tape device. If an error occurs during this
	 * process, the function returns with the appropriate
	 * error code--probably E_MKSPECIAL--otherwise it re-
	 * turns with a good return code (E_OK).
	 */
}


/*
 * NAME: download_microcode
 *
 * FUNCTION:
 *      This routine passes adapter software to the device driver to
 *      be downloaded to the adapter.
 *
 * RETURNS:
 *   0 - success
 *   positive return code on failure
 */
int
download_microcode(lname)
char    *lname;         /* logical device name */
{
	/*
	 * This function performs whatever steps are required to
	 * pass adapter software or microcode to the device driver.
	 * The function returns with the appropriate error code or
	 * with a good return code (E_OK).
	 */
}

