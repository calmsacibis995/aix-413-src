static char sccsid[] = "@(#)73	1.2  src/bos/usr/samples/cfg/chgyyy.c, cfgsamp, bos411, 9428A410j 6/7/91 11:53:50";

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
 * FUNCTION: Change method for tape device yyy
 *
 * Interface:
 *
 *	chgdevice -l <logical name> [-p <parent>] [-w <location>]
 *                      [-a attr=val [-a attr=val...]] [-P | -T]
 *
*/
 
 
/* header files needed for compilation */
#include <stdio.h>
#include <cf.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
 
 
/* external functions */
extern	char  *strtok();
	
 
/*****									*/
/***** Main Function							*/
/*****									*/
 
main(argc,argv)
int	argc;
char	*argv[];
{
extern	int	optind;		/* for getopt function */
extern	char	*optarg;	/* for getopt function */
 
struct	Class	*cusdev,	/* customized devices class pointer */
		*precon,	/* predefined connection class pointer */
		*predev;	/* predefined devices class pointer */
 
struct	CuDv	cusobj,		/* device customized device object  */
		parobj,		/* parent customized device object  */
		childobj;	/* child customized device object */
struct	PdDv	preobj;		/* predefined device object         */
struct  PdCn    pcnobj;         /* predefined connection object */
struct  CuAt    *attrobj;       /* Attribute object */
 
char    *lname,*parent;         /* logical name and parent name */
char    *connection;            /* parent connection */
char    *attr;                  /* attr=value string */
char    *attrname,*attr_val;     /* attribute name and value */
char	*badattr;
char	sstring[256];		/* searchstring space */
 
int	errflg,c;		/* used in parsing parameters */
int     Tflag;                  /* flag for -T parameter */
int	Pflag;			/* flag for -P param (i.e, "only change db")*/
int	new_location;		/* Flag used to tell if should generate new
				   location code. */
int	rc;			/* return code */
int     rc2;                    /* return code */
 
int	oldstatus;		/* Status of device at program commencement */
int	how_many;		/* number of attributes returned by getattr() */
char    old_val[ATTRVALSIZE];   /* Attribute value before change */
char    oldparent[NAMESIZE];    /* Parent name before change */
char    oldconnect[NAMESIZE];   /* Connection before change */
char    *out_p;                 /* Message from config method */
 
	/* set up initial variable values */
	lname = parent = connection = NULL;
	attr = NULL;
	errflg = 0;
	Pflag = 0;
	Tflag = 0;
 
	/* parse command line parameters */
	while ((c = getopt(argc,argv,"l:a:p:w:PT")) != EOF) {
		switch (c) {
		case 'l':
			if (lname != NULL)
				errflg++;
			lname = optarg;
			break;
		case 'a':
			if (attr != NULL);
				errflg++;
			attr = optarg;
			break;
		case 'p':
			if (parent != NULL)
				errflg++;
			parent = optarg;
			break;
		case 'w':
			if (connection != NULL)
				errflg++;
			connection = optarg;
			break;
		case 'P':
			Pflag++;
			break;
		case 'T':
			Tflag++;
			break;
		default:
			errflg++;
		}
	}
 
	if (errflg)
		/* error parsing parameters */
		exit(E_ARGS);
 
	if (lname==NULL)
		/* logical name must be specified */
		exit(E_LNAME);
 
	if (Tflag)
		/* The -T flag is not supported by device yyy */
		exit(E_TFLAG);
 
	/* Start up ODM */
	if (odm_initialize() == -1)
		/* Initialization failed */
		exit(E_ODMINIT);
 
	/* Lock the database */
	if (odm_lock("/etc/objrepos/config_lock",0) == -1)
		err_exit(E_ODMLOCK);
 
	/* open customized devices object class (CuDv) */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1)
		/* error: open failed */
		err_exit(E_ODMOPEN);
 
	/* find customized object */
	sprintf(sstring,"name = '%s'",lname);
	rc =(int)odm_get_first(cusdev,sstring,&cusobj);
	if (rc==0)
		/* No CuDv object with this name */
		err_exit(E_NOCuDv);
	else if (rc==-1)
		/* ODM failure */
		err_exit(E_ODMGET);

	/* get device's predefined object */
	sprintf(sstring,"uniquetype = '%s'",cusobj.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&preobj);
	if (rc==0)
		/* No PdDv object for this device */
		err_exit(E_NOPdDv);
	else if (rc==-1)
		/* ODM failure */
		err_exit(E_ODMGET);

	/******************************************
	  Validate attributes, if any were passed.
	 *******************************************/
	if (attr != NULL) {
		if (attrval(cusobj.PdDvLn_Lvalue,attr,&badattr) > 0) {
			/* Invalid attribute call attribute display routine */
			list_badattr(cusobj.PdDvLn_Lvalue,badattr);
			err_exit(E_ATTRVAL);
		}

		/* Parse attribute name and new value */
		attrname = strtok(attr,"=");
		attr_val = strtok(NULL," ");

		/* Get current atrribute information */
		attrobj = getattr(lname, attrname, FALSE, &how_many);
		if (attrobj == NULL)
			/* ODM error */
			err_exit(E_ODMGET);

		/* Save attribute value in case we need to restore it */
		strcpy(old_val, attrobj->value);
	}

	/* Need to find out if parent device or connection has changed. */
	/* This requires checking the parent and connection values passed */
	/* into the routine against the device's current parent and */
	/* connection as recorded in the database. */

	/* if parent or connection are NULL they are not being changed */
	if (parent == NULL)
		/* parent is same as before */
		parent = cusobj.parent;
 
	if (connection == NULL)
		/* connection is same as before */
		connection = cusobj.connwhere;
 
	/* Find out if either parent or connection have changed */
	if (strcmp(parent,cusobj.parent) || strcmp(connection,cusobj.connwhere)) {
		/* New parent and/or connection */
		/* Set Generate New Location Flag */
		new_location = 1;

		/* Save current parent and connection in case they need */
		/* to be restored */
		strcpy(oldparent,cusobj.parent);
		strcpy(oldconnect,cusobj.connwhere);

		/* Now validate the new parent/connection */
		/* Get the current/new customized parent object */
		sprintf(sstring,"name = '%s'",parent);
		rc =(int)odm_get_first(cusdev,sstring,&parobj);
		if (rc==0)
			/* Parent device not in CuDv */
			err_exit(E_NOCuDvPARENT);
		else if (rc==-1)
			/* ODM failure */
			err_exit(E_ODMGET);

		/* make sure that location exists on (new) parent */
		sprintf(sstring,"uniquetype = '%s' AND connkey = '%s' AND connwhere = '%s'",
				parobj.PdDvLn_Lvalue,preobj.subclass,connection);
		rc = (int)odm_get_first(PdCn_CLASS,sstring,&pcnobj);
		if (rc==0)
			/* Not a valid connection */
			err_exit(E_INVCONNECT);
		else if (rc==-1)
			/* ODM failure */
			err_exit(E_ODMGET);

		/* If device is AVAILABLE, then make sure that no other
		   devices are configured at the new location. */

		if ( cusobj.status!=DEFINED ) {
			sprintf(sstring,"parent = '%s' AND connwhere = '%s' AND status != %d",
						parent,connection,DEFINED);
			rc = (int)odm_get_first(cusdev,sstring,&childobj);
			if (rc==-1)
				/* ODM failure */
				err_exit(E_ODMGET);
			else if (rc!=0)
				/* There is another device AVAILABLE here */
				err_exit(E_AVAILCONNECT);
		}
	}
	else {
		/* No parent or connection change */
		new_location = 0;
		if (attr == NULL)
			/* Also no attribute change so just exit */
			exit (E_OK);
	}

	/* Save device's current status */
	oldstatus = cusobj.status;
 
	/* If the device is configured and the -P flag was not specified, */
	/* then need to unconfigure the device before updating database.  */
	if ( (cusobj.status != DEFINED) && (!Pflag) ) {
		/* In this case we WILL be unconfiguring,	*/
		/* & reconfiguring the device.                  */
 
		sprintf( sstring, "-l %s", cusobj.name );
		rc = odm_run_method(preobj.Unconfigure, sstring, &out_p,NULL);
		if (rc == -1)
			/* Error in running Unconfigure Method */
			err_exit(E_ODMRUNMETHOD);
		else if (rc != 0)
			/* Error returned from Unconfigure method */
			err_exit(rc);
	}
 
	/* Update parent, connection, and location if necessary */
	if (new_location) {
		rc = update_dev(lname,parent,connection);
		if (rc != 0)
			err_exit(rc);
	}
 
	/* update the device's attribute with the new value */
	if (attr != NULL) {
		strcpy(attrobj->value, attr_val);
		if (putattr(attrobj) == -1)
			/* ODM error */
			err_exit(E_ODMUPDATE);
	}
 
 
	/*****************************************************************
	 If device was configured when beginning the change and the Pflag
	 was not specified, then need to re-configure the device.
	 *****************************************************************/
	if ( (oldstatus != DEFINED) && (!Pflag) ) {
		sprintf( sstring, "-l %s", cusobj.name );
		rc = odm_run_method(preobj.Configure, sstring, &out_p, NULL);
		if (rc == -1)
			/* Error in running Configure Method */
			err_exit(E_ODMRUNMETHOD);
		else if (rc != 0) {
			/* Restore device info to previous state:*/
			if (attr != NULL) {
				/* Restore attribute value */
				strcpy(attrobj->value, old_val);
				if (putattr(attrobj) == -1)
					/* ODM error */
					err_exit(E_ODMUPDATE);
			}

			if (new_location) {
				/* Restore parent,connection,location */
				rc2 = update_dev(lname,oldparent,oldconnect);
				if (rc2 != 0)
					err_exit(rc2);
			}

			err_exit(rc);   /* Exit with configure method's error */
		}
	}   /* end if device is configured */
 
	/* clean up and return successful */
	if (odm_close_class(cusdev) == -1)
		/* Close ODM failure */
		err_exit(E_ODMCLOSE);
 
	odm_terminate();
	exit(0);
 
}
 
 
 
/*****                                                                  */
/***** Subroutines and Functions                                        */
/*****                                                                  */
 
/* error exit routine */
err_exit(exit_code)
int     exit_code;              /* Error exit code */
{
	odm_close_class(CuDv_CLASS);
	odm_close_class(CuAt_CLASS);
	odm_close_class(PdAt_CLASS);
	odm_terminate();
	exit(exit_code);
}

/* update parent, connection, and location */
update_dev(lname,parent,connection)
char    *lname;
char    *parent;
char    *connection;
{
	struct  CuDv    cusobj;         /* device customized device object */
	char    sstring[256];           /* search string space */
	int     rc;                     /* odm return code */
	char    parloc[5];              /* parent location */

	/* First need to get device's CuDv object */
	sprintf(sstring,"name = '%s'",lname);
	rc =(int)odm_get_first(CuDv_CLASS,sstring,&cusobj);
	if (rc==0)
		/* No CuDv object for this name */
		return(E_NOCuDv);
	else if (rc==-1)
		/* ODM failure */
		return(E_ODMGET);

	strcpy(cusobj.parent,parent);
	strcpy(cusobj.connwhere,connection);

	strncpy(parloc,cusobj.location,5);
	sprintf(cusobj.location,"%s-%02d",parloc,connection);

	if (odm_change_obj(CuDv_CLASS,&cusobj) == -1)
		/* Change ODM failure */
		return(E_ODMUPDATE);

	return(E_OK);
}



list_badattr( uniquetype, badattr )
char    *uniquetype;
char    *badattr;
{
	char    *attrname;
	char    sstring[100];
	struct  PdDv    PdDvobj;
	struct  PdAt    PdAtobj;
	int             rc;

	sprintf( sstring, "uniquetype = %s", uniquetype );
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&PdDvobj);
	if (rc==0) {
		err_exit(E_NOCuDv);
	}
	else if (rc == -1) {
		err_exit(E_ODMGET);
	}

	/* Loop through bad attribute names: */

	for( attrname = strtok(badattr,", ");
		/* Check not at end */
		attrname != (char *)NULL;
		/* Find next */
		attrname = strtok( (char *)NULL, ", ") ) {

		/* Find the PdAt entry for the attribute */
		sprintf( sstring, "uniquetype = %s AND attribute = %s",
			uniquetype, attrname );
		rc = (int)odm_get_first(PdAt_CLASS,sstring,&PdAtobj);
		if (rc==0) {
			fprintf(stderr,"     %s\n");
			err_exit(E_INVATTR);
		}
		else if (rc == -1) {
			err_exit(E_ODMGET);
		}
		else {
			NLfprintf( stderr, "     %s     %s\n", attrname,
				NLgetamsg( PdDvobj.catalog,
				PdDvobj.setno,PdAtobj.nls_index,"" ));
		}
	}
}

