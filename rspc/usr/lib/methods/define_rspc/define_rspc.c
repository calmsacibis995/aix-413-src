static char sccsid[] = "@(#)42  1.4  src/rspc/usr/lib/methods/define_rspc/define_rspc.c, rspccfg, rspc41J, 9514A_all 3/31/95 17:10:58";
/*
 * COMPONENT_NAME: (RSPCCFG) define_rspc.c - Generic RSPC Define Method
 *
 * FUNCTIONS: main(), err_exit()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* interface:
   define -c <class> -s <subclass> -t <type> -p <parent> -w <connection>
	  -l <device name> -L <location> -u -n -o -k

   The -u flag indicates that USER specified names (via -l <device name>)
   is not allowed.
   The -n flag indicates that the device has NO parent or parent connection.
   The -o flag indicates that only ONE device of this type can be defined.
   NOTE: The -o flag also does not allow user specified names.
   The -k flag indicates that the device is only to be defined if the
   specified parent connection does not already have a device defined for it.
   The -L flag indicates that the location for this device is being
   passed in and should NOT be generated by the location routine.
   The -d flag indicates that the -w <connection> will not be validated
   against a PdCn.

   The -c, -s, -t, -p, -w, and -l flags along with their corresponding
   values are supplied by the invoker of the define method.  The -u, -n,
   -o, and -k flags are picked up along with the method name from the Define
   Method descriptor in PdDv.
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"


#define TRUE 1
#define FALSE 0

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


char	*class,*subclass,*type,		/* parameter pointers */
	*parent,*connect,*lname, *loc;
char	sstring[256];			/* search string */
char    parloc[LOCSIZE],                /* parents location */
	parut[UNIQUESIZE];              /* parents unique type */
char    tempname[NAMESIZE];     /* Used in generating device name */

struct  Class   *predev,        /* handle for predefined devices class    */
		*cusdev;        /* handle for customized devices class    */

struct	PdDv	PdDv;
struct	PdCn	PdCn;
struct	CuDv	CuDv;

int     seqno;
int	rc;
int     errflg,c;               /* used in parsing parameters */
int     uflag;                  /* User specified name not allowed */
int     nflag;                  /* Device does not have parent/connection */
int     oflag;                  /* Only one device of this type allowed */
int     kflag;                  /* Only one device per parent connection */
int	Lflag;			/* location field passed in */
int	dflag;			/* do not validate the connection field  */
char	criteria[256], slot_str[16];
struct	CuDv *p;
struct	listinfo info;	/* info on the CuDv list */
int 	i, slot;

	/*****								*/
	/***** Parse Parameters 					*/
	/*****								*/
	errflg = 0;
	class = subclass = type = parent = connect = lname = NULL;
	uflag = 0;
	nflag = 0;
	oflag = 0;
	kflag = 0;
	Lflag = 0;
	dflag = 0;

	while ((c = getopt(argc,argv,"c:s:t:p:w:l:L:unokd")) != EOF) {
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
		case 'l':                       /* User specified device */
			if (lname != NULL)      /* name */
				errflg++;
			lname = optarg;
			break;
		case 'u':               /* User specified names not allowed */
			uflag++;
			break;
		case 'n':               /* Device does not have a parent */
			nflag++;        /* or connection */
			break;
		case 'o':               /* Only one device of this type can */
			oflag++;        /* be defined */
			uflag++;        /* User name not allowed */
			break;
		case 'k':               /* Device can not be defined if a */
			kflag++;        /* device is already defined at */
			break;          /* specified parent connection */
		case 'L':		/* Location string */
			Lflag++;
			loc = optarg;
			break;
		case 'd':		/* validate connection ?	*/
			dflag++;
			break;
		default:
			errflg++;
		}
	}
	if (errflg) {
		/* error parsing parameters */
		DEBUG_0("define: command line error\n");
		exit(E_ARGS);
	}

	/*****								*/
	/***** Validate Parameters					*/
	/*****								*/
	/* class, subclass, and type must be specified */
	if (class==NULL || subclass==NULL || type==NULL) {
		DEBUG_0("define: class, subclass, and type names must be specified\n");
		exit(E_TYPE);
	}

	/* for isa adapters, only parent parameter is required */
	if (strcmp(subclass, "isa")==0) {
		DEBUG_0("ISA adapter being defined. \n");
		if (parent == NULL) {
			DEBUG_0("define:  for isa adapter, parent must be specified. \n");
			exit(E_PARENT3);
		}
	}
	

	/* for non-isa adapters, parent and connection must be 
	   specified except when -n is used */
	else if (nflag) {
		if (parent != NULL || connect != NULL) {
			DEBUG_0("define: -p and -w not to be used with this device type\n");
			exit(E_PARENT2);
		}
	} else {
		if (parent == NULL || connect == NULL) {
			DEBUG_0("define: parent and connection must to be specified\n");
			exit(E_PARENT);
		}
	}

	/* User specified name not allowed in some cases */
	if (lname!=NULL && uflag!=0) {
		DEBUG_0("define: Device name cannot be specified for this type\n");
	     /*   exit(E_UNAME);   */
		exit(E_LNAME);
	}

	DEBUG_0 ("Defining device: \n")
	DEBUG_1 ("     class      = %s\n",class)
	DEBUG_1 ("     subclass   = %s\n",subclass)
	DEBUG_1 ("     type       = %s\n",type)
	DEBUG_1 ("     parent     = %s\n",parent)
	DEBUG_1 ("     connection = %s\n",connect)
	DEBUG_1 ("     location   = %s\n",loc)

	/* start up odm */
	if (odm_initialize() == -1) {
		/* initialization failed */
		DEBUG_0("define: odm_initialize() failed\n")
		exit(E_ODMINIT);
	}
	DEBUG_0 ("ODM initialized\n")

	/* get predefined object for this device */
	sprintf(sstring,"type = '%s' AND class = '%s' AND subclass = '%s'",
						type,class,subclass);
	rc = (int)odm_get_first(PdDv_CLASS,sstring,&PdDv);
	if (rc==0) {
		/* failed to get an object */
		DEBUG_0("define: No PdDv object\n")
		err_exit(E_NOPdDv);
	}
	else if (rc==-1) {
		/* ODM error */
		DEBUG_0("define: ODM error getting PdDv object\n")
		err_exit(E_ODMGET);
	}

	/* open customized device object class */
	if ((int)(cusdev=odm_open_class(CuDv_CLASS)) == -1) {
		/* error opening class */
		DEBUG_0("define: open of CuDv failed\n")
		err_exit(E_ODMOPEN);
	}

	/* Verify user supplied device name else generate a name */
	if (lname == NULL) {
		/* generate logical name for device */
		if ((seqno=genseq(PdDv.prefix))<0) {
			/* error making logical name */
			DEBUG_0("define: error making logical name\n");
			err_exit(E_MAKENAME);
		}
		if (oflag!=0 && seqno>0) {
			/* only one of this type device is allowed */
			DEBUG_0("define: Not allowed to add another device of this type\n");
			err_exit(E_ALREADYDEF);
		}
		sprintf(tempname,"%s%d",PdDv.prefix,seqno);
		lname = tempname;
	} else {
		/* Verify that name is not already in use */
		sprintf(sstring,"name = '%s'",lname);
		rc =(int)odm_get_first(cusdev,sstring,&CuDv);
		if (rc==-1) {
			/* ODM error */
			DEBUG_0("define: ODM error getting CuDv object\n")
			err_exit(E_ODMGET);
		}
		else if (rc != 0) {
			/* Device name already in use */
			DEBUG_0("define: Specified name already in use\n")
		    /*    err_exit(E_NAMEUSED);   */
			err_exit(E_LNAME);
		}
		/* Otherwise, name is just fine! */
	}

	/* Check parent device if device has a parent */
	if (nflag == 0) {
		/* get parent customized device object */
		sprintf(sstring,"name = '%s'",parent);
		rc =(int)odm_get_first(cusdev,sstring,&CuDv);
		if (rc==0) {
			/* failed to get an object */
			DEBUG_0("define: No parent CuDv object\n")
			err_exit(E_NOCuDvPARENT);
		}
		else if (rc==-1) {
			/* ODM error */
			DEBUG_0("define: ODM error getting CuDv object\n")
			err_exit(E_ODMGET);
		}
		strcpy(parloc,CuDv.location);
		strcpy(parut,CuDv.PdDvLn_Lvalue);


		/* if subclass is not isa, verify connection if req'd */
		if (strcmp(subclass, "isa") != 0) {
			/* Non-ISA device */
			/* if connection validation is required */
			if (!dflag) {
				sprintf(sstring,"uniquetype = '%s' and connkey = '%s' and connwhere = '%s'",
				CuDv.PdDvLn_Lvalue,subclass,connect);
				rc = (int)odm_get_first(PdCn_CLASS,sstring,&PdCn);
				if (rc==0) {
					/* invalid connection */
					DEBUG_0("define: Invalid connection\n")
					err_exit(E_INVCONNECT);
				}
				else if (rc==-1) {
					/* ODM error */
					DEBUG_0("define: ODM error getting PdDv object\n")
					err_exit(E_ODMGET);
				}
			}

			if (kflag != 0) {
				/* see if a customized device already connected */
				/* at specified connection on specified parent */
				sprintf(sstring,"parent = '%s' AND connwhere = '%s'",
								parent,connect);
				rc =(int)odm_get_first(cusdev,sstring,&CuDv);
				if (rc==-1) {
					/* ODM error */
					DEBUG_0("define: ODM error getting CuDv object\n")
					err_exit(E_ODMGET);
				}
				if (rc) {
					/* already a device at connection */
					DEBUG_0("define: Already defined device at connection\n")
					err_exit(E_INVCONNECT);
				}
			}
		}

		else {  /* ISA adapter : generate the connwhere value */
				
		    /* if connection generation is required */
		    /* (if not required, a connection value must be supplied) */
		    if (!dflag) {
			/* In this code, connwhere ranges from 1 to x
			   and slot ranges from 0 to x - 1.  Connwhere
			   denotes the actual physical slot number and slot
			   is used to access the cudv array.
			*/

			 /* Get all the instances of isa adapters */
			 sprintf(criteria, "PdDvLn LIKE '*/isa/*' and parent=%s", parent);
			 p = odm_get_list(cusdev,criteria,&info,1,1 );

			 if ( (int)p == -1 )
			     err_exit(E_ODMGET);

			 if ( p==NULL )
			      /* there are no isa slots alotted.  Use
				 first one (1) */
			      slot = 0;

			 else  /* find the lower number un-used slot */
			 {
				 /* There were some isa adapters found..*/
				 for (i=0; i<info.num; i++) p[i]._scratch = FALSE;

				 for (i=0; i<info.num; i++)
				 {
				      /* set up slot-this may be the slot we want.*/
				      /* convert from "real" slots (1 - 3) to
					 indexed      */
				      /* slots (0 - 2)*/
				      slot = atol(p[i].connwhere) - 1;

				      if (slot < 0)
				      {
					  DEBUG_0("connwhere -1 is less than 0.  loop. \n");
					  continue;
				      }

				      /* If we're not through the list yet, then
					 this instance must be used - set scratch
					 to true */
				      if (slot < info.num)
					   p[slot]._scratch = TRUE;
				 }


				 /* If there are no holes in the slot numbers,then*/
				 /* info.num will be the value of the new slot num*/
				 slot = info.num;

				 for (i=0; i<info.num; i++)
				 {
				      if (p[i]._scratch == FALSE)
				      {
					   /* Found an unused number */
					   slot = i;
					   break;
				      }
				 }
			 }

			 /* up until this point, we've been numbering slots */
			 /* from 0 to x -1.  Convert that to range  */
			 /* from 1 to x adding 1 */
			 slot++;
			 sprintf(slot_str, "%d", slot);
			 connect = slot_str;

			 /* free the list from the odm_get call */
			 odm_free_list(p, &info);

		    }
		}  /* end of generating connwhere for isa adapter */
	}


	DEBUG_0 ("base objects found\n")

	/* create a new customized device object */
	DEBUG_1 ("creating customized object: %s\n",lname);
	strcpy(CuDv.name,lname);
	CuDv.status      = DEFINED;
	CuDv.chgstatus   = PdDv.chgstatus;
	strcpy(CuDv.ddins,PdDv.DvDr);
	strcpy(CuDv.parent,parent);
	strcpy(CuDv.connwhere,connect);
	strcpy(CuDv.PdDvLn_Lvalue,PdDv.uniquetype);
	CuDv.location[0] = '\0';

	/* If device has a parent, fill in the location code */
	if (nflag==0) {
		if (Lflag==0) {
			location(parut,parloc,&CuDv);
			DEBUG_1 ("location = %s\n",CuDv.location)
		}
		else {
			DEBUG_1("location passed in is %s\n", loc);
			strcpy(CuDv.location, loc);
		}
	}

	/* add customized object to customized devices object class */
	if (odm_add_obj(cusdev,&CuDv) == -1) {
		/* error: add failed */
		DEBUG_0("define: ODM error creating CuDv object\n")
		err_exit(E_ODMADD);
	}
	DEBUG_1 ("created customized device: %s\n",lname)

	if (odm_close_class(CuDv_CLASS) == -1) {
		/* ODM error */
		DEBUG_0("define: failure closing CuDv object class\n")
		err_exit(E_ODMCLOSE);
	}

	odm_terminate();

	/* device defined successfully */
	fprintf(stdout,"%s ",lname);
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
char    exitcode;
{
	odm_close_class(CuDv_CLASS);
	odm_close_class(PdDv_CLASS);
	odm_terminate();
	exit(exitcode);
}
