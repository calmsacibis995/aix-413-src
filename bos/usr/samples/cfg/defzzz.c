static char sccsid[] = "@(#)77	1.2  src/bos/usr/samples/cfg/defzzz.c, cfgsamp, bos411, 9428A410j 6/7/91 11:55:58";

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
 * defzzz -c <class> -s <subclass> -t <type>
*/

/*
 * header files needed for this program
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>

/* external odm manipulation functions */
extern int	genseq();

/*****									*/
/***** Main Function							*/
/*****									*/

main(argc, argv, envp)
int	argc;
char	*argv[];
char	*envp[];
{
extern int	optind;         /* for getopt function */
extern char	*optarg;        /* for getopt function */

char	*class, *subclass, *type;	/* parameter pointers */
char	*lname;				/* logical name */
char	sstring[256];			/* search string */

struct Class	*predev,        /* handle for predefined devices class */
		*cusdev;	/* handle for customized devices class */

struct PdDv PdDv;
struct CuDv CuDv;

int     seqno;                  /* Used in generating device name */
int     rc;                     /* ODM return code */
int	errflg, c;		/* used in parsing parameters */

	/*****								*/
	/***** Parse Parameters 					*/
	/*****								*/

	errflg = 0;
	class = subclass = type = lname = NULL;

	while ((c = getopt(argc, argv, "c:s:t:")) != EOF) {
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
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		exit(E_ARGS);
	}

	/*****								*/
	/***** Validate Parameters					*/
	/*****								*/
	/* class, subclass, and type must be for pseudo device zzz */
	if (strcmp(class,"zzz") || strcmp(subclass,"node") ||
							strcmp(type,"zzz"))
		/* called for wrong type of device */
		exit(E_TYPE);
	if (class == NULL || subclass == NULL || type == NULL) {
		exit(E_TYPE);
	}

	/* start up odm */
	if (odm_initialize() == -1) {
		/* odm initialization failed */
		exit(E_ODMINIT);
	}

	/* lock the database */
	if (odm_lock("/etc/objrepos/config_lock", 0) == -1) {
		/* failed to lock odm data base */
		err_exit(E_ODMLOCK);
	}

	/* get predefined object for this device */
	sprintf(sstring, "type = '%s' AND class = '%s' AND subclass = '%s'",
		type, class, subclass);
	rc = (int)odm_get_first(PdDv_CLASS, sstring, &PdDv);
	if (rc == 0) {
		/* failed to get the PdDv object */
		err_exit(E_NOPdDv);
	} else if (rc == -1) {
		/* ODM error */
		err_exit(E_ODMGET);
	}

	/* open customized device object class */
	if ((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		err_exit(E_ODMOPEN);
	}

	/* generate logical name for device */
	if ((seqno = genseq(PdDv.prefix)) < 0) {
		/* error making logical name */
		err_exit(E_MAKENAME);
	}

	/* create a new customized device object */
	sprintf(CuDv.name,"%s%d",PdDv.prefix,seqno);
	CuDv.status       = DEFINED;
	CuDv.chgstatus    = PdDv.chgstatus;
	CuDv.parent[0]    = '\0';
	CuDv.connwhere[0] = '\0';
	CuDv.location[0]  = '\0';
	strcpy(CuDv.ddins, PdDv.DvDr);
	strcpy(CuDv.PdDvLn_Lvalue, PdDv.uniquetype);

	/* add customized object to customized devices object class */
	if (odm_add_obj(cusdev, &CuDv) == -1) {
		/* error: add failed */
		err_exit(E_ODMADD);
	}

	if (odm_close_class(CuDv_CLASS) == -1) {
		/* ODM error */
		err_exit(E_ODMCLOSE);
	}

	/* terminate the odm */
	odm_terminate();

	/* Device defined successfully. Print device name
	   to stdout with a space appended */
	fprintf(stdout,"%s ",CuDv.name);
	exit(E_OK);

}


/*
 * NAME: err_exit
 *
 * FUNCTION: Closes any open object classes and terminates ODM.  Used to
 *           back out on an error.
 *
 * RETURNS:
 *               None
 */

err_exit(exitcode)
char	exitcode;
{
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_terminate();
	exit(exitcode);
}


