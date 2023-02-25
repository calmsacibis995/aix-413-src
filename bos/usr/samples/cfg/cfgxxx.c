static char sccsid[] = "@(#)70	1.2  src/bos/usr/samples/cfg/cfgxxx.c, cfgsamp, bos411, 9428A410j 6/7/91 11:51:37";

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
 * FUNCTION: Configure method for mca adapter of type xxx
 *
 * interface:
 * cfgxxx -l <logical_name> [-<1|2>]
*/


/* header files needed for compilation */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <fcntl.h>


/* main function code */
main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];

{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */

	char	*logical_name;		/* logical name to configure */
	char	*phase1, *phase2;	/* ipl phase flags */
	char    sstring[256];           /* search criteria string */
	char	conflist[1024];		/* busresolve() configured devices */
	char	not_resolved[1024];	/* busresolve() not resolved devices */

	struct Class *cusdev;		/* customized devices class ptr */
	struct Class *predev;		/* predefined devices class ptr */

	struct CuDv cusobj;		/* customized device object storage */
	struct PdDv preobj;		/* predefined device object storage */
	struct CuDv parobj;		/* customized device object storage */
	struct CuDv dmyobj;		/* customized device object storage */

	ushort	devid;			/* Device id - used at run-time */
	int	ipl_phase;		/* ipl phase: 0=run,1=phase1,2=phase2 */
	int	slot;			/* slot of adapters */
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
	}
	else if (rc==-1) {
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
	}
	else if (rc==-1) {
		/* ODM failure */
		err_exit(E_ODMGET);
	}

	/* close predefined device object class */
	if (odm_close_class(predev) == -1)
		err_exit(E_ODMCLOSE);

	/****************************************************************
	  If adapter xxx is being configured during an ipl phase, then
	  display its LED value on the system LEDs.
	  ****************************************************************/
	if (ipl_phase != RUNTIME_CFG)
		setleds(preobj.led);

	/******************************************************************
	  Check to see if the adapter is already configured (AVAILABLE).
	  We actually go about the business of configuring the adapter
	  only if it is not configured yet. Configuring the adapter
	  refers to the process of checking parent status, resolving
	  bus resources, and obtaining VPD.
	  ******************************************************************/
	if (cusobj.status == DEFINED) {

		/* The adapter is not configured */

		/* get the device's parent object */
		sprintf(sstring, "name = '%s'", cusobj.parent);
		rc = (int)odm_get_first(cusdev,sstring,&parobj);
		if (rc==0) {
			/* Parent device not in CuDv */
			err_exit(E_NOCuDvPARENT);
		}
		else if (rc==-1) {
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

		/***************************************************
		  If the xxx adapter is being configured at
		  RUN TIME, then we must resolve any bus attribute
		  conflicts before configuring.  If being configured
		  at boot time, the bus configurator will have already
		  resolved conflicts.
		 ***************************************************/
		if (ipl_phase == RUNTIME_CFG) {
			/* Make sure card is in specified slot */
			slot = atoi(cusobj.connwhere);
			devid = (ushort) strtol(preobj.devid,(char **) NULL,0);
			sprintf (sstring,"/dev/%s",cusobj.parent);
			rc = chkslot(sstring,slot,devid);
			if (rc != 0)
				err_exit(rc);

			/* Invoke Bus Resolve  */
			rc = busresolve(logical_name,(int)0,conflist,
				not_resolved, cusobj.parent);
			if (rc != 0)
				err_exit(rc);
		}

		/* get VPD for this device */
		rc = query_vpd(logical_name);
		if (rc)
			/* failed to get VPD */
			err_exit(rc);

		/* update customized device object with a change operation */
		cusobj.status = AVAILABLE;
		if (odm_change_obj(cusdev, &cusobj) == -1)
			/* ODM failure */
			return(E_ODMUPDATE);

	} /* end if (adapter is not AVAILABLE) then ... */

	/* call routine to detect/manage child devices */
	if (define_children(logical_name, ipl_phase) != 0) {
		/* error defining children */
		err_exit(E_FINDCHILD);
	}

	/* close customized device object class */
	if (odm_close_class(cusdev) == -1)
		err_exit(E_ODMCLOSE);

	odm_terminate();
	exit(0);

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


	/* This routine obtains the VPD from the adapter card and returns
	   it in the vpd string.  The actual method of obtaining the VPD
	   will depend on the hardware interface. */

	/* code to be added here to obtain VPD */

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
 * NAME: define_children
 *
 * FUNCTION: Routine for detecting and managing children of the
 *    adapter.  It defines each new device not already represented in
 *    the Customized database.  The names of all the defined children
 *    in the Customized database are then written to stdout so they
 *    can be configured.
 *
 * RETURNS:
 *   0 - success (E_OK)
 */
int
define_children(logical_name,phase)
	char	*logical_name;
	int	phase;
{
	/* code to be added here to detect child devices */

	return(E_OK);
}



#include <sys/mdio.h>

/*
 *  NAME: chkslot
 * 
 *  FUNCTION:
 *      Return zero value if cardid is in desired slot.
 * 
 *  INPUTS:
 *      bus     - The name of the bus device, for example, bus0.
 *      slot    - The slot number from the parent connection descriptor.
 *                It should be a value of 1 through 8, with 0 being used
 *                for the Standard I/O Planar.
 *      cardid  - The card Id composed as ((POS0<<8) || POS1).
 *
 * RETURNS: Returns  0 on success, >0 Error code.
 */

int
chkslot(bus, slot, cardid)
char *bus;
int slot;
ushort cardid;
{
	MACH_DD_IO mddRecord;
	uchar pos[2];
	int fd;
	int i;

	pos[0] = 0xff;
	pos[1] = 0xff;

	/* Decrement slot number found in database */
	slot == (slot--) & 0x0F;

	if (0 > (fd = open(bus, O_RDWR)))
		return E_NODETECT;

	mddRecord.md_size = 2; 
	mddRecord.md_incr = MV_BYTE;
	mddRecord.md_data = pos;      
	mddRecord.md_addr = POSREG(0, slot);

	if (0 > ioctl(fd, MIOCCGET, &mddRecord))
		return E_NODETECT;

	close(fd);

	if (cardid == ((pos[0] << 8) | pos[1]))
		return(0);
	else
		return E_NODETECT;
}
