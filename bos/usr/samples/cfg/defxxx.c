static char sccsid[] = "@(#)74	1.2  src/bos/usr/samples/cfg/defxxx.c, cfgsamp, bos411, 9428A410j 6/7/91 11:54:21";

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
 * FUNCTIONS: Define method for mca adapter xxx
 *
 *
 * interface:
 * defxxx -c <class> -s <subclass> -t <type> -p <parent> -w <connection>
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>

/* external odm manipulation functions */
extern  int genseq();

/*****									*/
/***** Main Function							*/
/*****									*/

main(argc,argv,envp)
int	argc;
char	*argv[];
char	*envp[];

{
extern  int     optind;         /* for getopt function */
extern  char    *optarg;        /* for getopt function */


char    *class,*subclass,*type, /* parameter pointers */
	*parent,*connect;
char    sstring[256];           /* ODM search string */
char    parloc[LOCSIZE],        /* parents location */
	parut[UNIQUESIZE];      /* parents unique type */

struct  Class   *predev,        /* handle for predefined devices class    */
		*cusdev;        /* handle for customized devices class    */

struct  PdDv    PdDv;           /* Structure for retrieving PdDv object */
struct  PdCn    PdCn;           /* Structure for retrieving PdCn object */
struct  CuDv    CuDv;           /* Structure for retrieving CuDv object */

int     seqno;                  /* Used in generating device name */
int     rc;                     /* ODM return code */
int     errflg,c;               /* used in parsing parameters */

	/*****								*/
	/***** Parse Parameters 					*/
	/*****								*/
	errflg = 0;
	class = subclass = type = parent = connect = NULL;

	while ((c = getopt(argc,argv,"c:s:t:p:w:")) != EOF) {
		switch (c) {
		case 'c':                       /* Device class */
			if (class != NULL)
				errflg++;
			class = optarg;
			break;
		case 's':                       /* Device subclass */
			if (subclass != NULL)
				errflg++;
			subclass = optarg;
			break;
		case 't':                       /* Device type */
			if (type != NULL)
				errflg++;
			type = optarg;
			break;
		case 'p':                       /* Parent device name */
			if (parent != NULL)
				errflg++;
			parent = optarg;
			break;
		case 'w':                       /* Connection location on */
			if (connect != NULL)    /* parent */
				errflg++;
			connect = optarg;
			break;
		default:
			errflg++;
		}
	}
	if (errflg)
		/* error parsing parameters */
		exit(E_ARGS);

	/*****								*/
	/***** Validate Parameters					*/
	/*****								*/
	/* class, subclass, and type must be for adapter xxx */
	if (strcmp(class,"adapter") || strcmp(subclass,"mca") ||
							strcmp(type,"xxx"))
		/* called for wrong type of device */
		exit(E_TYPE);

	/* parent and connection must be specified */
	if (parent == NULL || connect == NULL)
		/* parent or connection not specified */
		exit(E_PARENT);

	/* start up odm */
	if (odm_initialize() == -1)
		/* initialization failed */
		exit(E_ODMINIT);

	/* lock the database */
	if (odm_lock("/etc/objrepos/config_lock",0) == -1)
		/* failed to lock odm data base */
		err_exit(E_ODMLOCK);

	/* get predefined object for this device */
	sprintf(sstring,"class = '%s' AND subclass = '%s' AND type = '%s'",
						class,subclass,type);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&PdDv);
	if (rc==0) {
		/* failed to get the PdDv object */
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM error */
		err_exit(E_ODMGET);
	}

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1)
		/* error opening class */
		err_exit(E_ODMOPEN);

	/* Check parent device */
	/* get parent customized device object */
	sprintf(sstring,"name = '%s'",parent);
	rc =(int)odm_get_first(cusdev,sstring,&CuDv);
	if (rc==0) {
		/* failed to get the parent CuDv object */
		err_exit(E_NOCuDvPARENT);
	}
	else if (rc==-1) {
		/* ODM error */
		err_exit(E_ODMGET);
	}
	strcpy(parloc,CuDv.location);
	strcpy(parut,CuDv.PdDvLn_Lvalue);

	sprintf(sstring,"uniquetype = '%s' and connkey = '%s' and connwhere = '%s'",
			CuDv.PdDvLn_Lvalue,subclass,connect);
	rc = (int)odm_get_first(PdCn_CLASS,sstring,&PdCn);
	if (rc==0) {
		/* invalid connection */
		err_exit(E_INVCONNECT);
	}
	else if (rc==-1) {
		/* ODM error */
		err_exit(E_ODMGET);
	}


	/* generate logical name for device */
	if ((seqno=genseq(PdDv.prefix))<0)
		/* error making logical name */
		err_exit(E_MAKENAME);

	sprintf(CuDv.name,"%s%d",PdDv.prefix,seqno);

	/* Set state to defined */
	CuDv.status      = DEFINED;

	/* Copy information from PdDv object */
	CuDv.chgstatus   = PdDv.chgstatus;
	strcpy(CuDv.ddins,PdDv.DvDr);
	strcpy(CuDv.PdDvLn_Lvalue,PdDv.uniquetype);

	/* Parent name and connection are as input */
	strcpy(CuDv.parent,parent);
	strcpy(CuDv.connwhere,connect);

	/* Location code for an adapter card consists of two pairs of     */
	/* digits separated by a hyphen.  The first two digits are always */
	/* 00 and the last two identify the cards slot number.            */
	sprintf(CuDv.location,"00-%02d",connect);

	/* add customized object to customized devices object class */
	if (odm_add_obj(cusdev,&CuDv) == -1)
		/* error: add failed */
		err_exit(E_ODMADD);

	if (odm_close_class(CuDv_CLASS) == -1)
		/* ODM error */
		err_exit(E_ODMCLOSE);

	/* terminate the odm */
	odm_terminate();

	/* device defined successfully */
	/* Write name to stdout with a space appended */
	fprintf(stdout,"%s ",CuDv.name);
	exit(E_OK);
}


/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * RETURNS:     None
 */

err_exit(exitcode)
char    exitcode;
{
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_terminate();
	exit(exitcode);
}
