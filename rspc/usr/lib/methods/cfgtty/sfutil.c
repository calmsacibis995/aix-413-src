#ifndef lint
static char sccsid[] = "@(#)87 1.2 src/rspc/usr/lib/methods/cfgtty/sfutil.c, isatty, rspc41J, 9520A_all 5/16/95 09:40:12";
#endif
/*
 * COMPONENT_NAME: (CFGTTY) Functions for isaload module
 *
 * FUNCTIONS: Run_Other_Method
 *            Generate_Minor
 *            Make_Special_Files
 *            Do_Other_Processing
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

#include <stdio.h>          /* standard I/O */
#include <cf.h>             /* error messages */
#include <errno.h>          /* standard error numbers */
#include <fcntl.h>          /* logical file system #define */

#include <sys/mode.h>
#include <sys/cfgdb.h>      /* config #define */
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/cfgodm.h>     /* config structures */

#include <sys/str_tty.h>

#include "ttycfg.h"
#include "cfgdebug.h"

/*
 * ========================================================================
 * defines and strutures
 * ========================================================================
 */

#define MKNOD_FLAGS   S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH

/*
 * ------------------------------------------------------------------------
 *                       Generate_Minor
 * ------------------------------------------------------------------------
 * This function generates the minor number(s) for the tty devices.
 *
 * First, we try to get existing (in ODM database) minors. If none are found,
 * we generate minor numbers and record them in the ODM CuDvDr database.
 *
 * Notes: Minor number is generated for the tty device being configured by
 * applying a special algorithm. The minor number is set in the database
 * by calling genminor() with the minor number to be used.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * ------------------------------------------------------------------------
 */
int Generate_Minor(struct CuDv *devcusobj, struct CuDv *parcusobj,
			struct CuDv *buscusobj, char *drvrname,
			 long maj_no, long **minorP)
{
	/* specific routines for getting and generating minor numbers */
	extern int gen_rspc_minor();

	long *minor_list;
	int  count;
	int  rc;

	DEBUG_0("Generate_Minor: entered\n");

	/* see if we already have a minor number */
	if ((minor_list = getminor(maj_no,&count,devcusobj->name)) == NULL) {

		/*
		 * no minor numbers found for this device
		 * call the appropriate routine to create
		 * a minor number
		 */

		DEBUG_1("gen_min: no minors found for %s\n",devcusobj->name);


		if (!strcmp(drvrname,RSPC_DRIVER)) {

			/* call rspc minor number generator */
			DEBUG_0("gen_min: calling gen_rspc_minor()\n");
			rc = gen_rspc_minor(devcusobj,parcusobj,
						 maj_no,&minor_list);

		} else if (!strcmp(drvrname,PCMCIA_DRIVER)) {
			
			/* call pcmcia minor number generator */
			DEBUG_0("gen_min: calling gen_pcmcia_minor()\n");
			rc = gen_pcmcia_minor(devcusobj,parcusobj,
                                                  maj_no,&minor_list);
                } else   /*not found*/
                        {
                        DEBUG_0(" not here");
                        return(ENXIO);
                        }

                if (rc > 0){
                        /* error generating minor number */
                        DEBUG_1("gen_rspc/pcmcia_minor failed, rc=%d\n",rc);
                        return(rc);
                }
                if (rc == -1){
                        DEBUG_0("gen_rspc/pcmcia_minor: return -1\n");
                        /* minor number  in use */
                        *minorP = minor_list;
                        return(rc);
                }
	}/*end: if ((minor_list = ... */

	DEBUG_1("Generate_Minor: return OK, minor=%d\n",*minor_list);

	*minorP = minor_list;
	return(0);

} /* end Generate_Minor(...) */


int gen_rspc_minor(struct CuDv *cusobj, struct CuDv *parobj,
		   long major, long **minP)
{
	long  port;
	long *minor;
	int   rc;

	DEBUG_3("gen_rspc_minor: name=%s parent=%s major=%d\n",
		cusobj->name,parobj->name,major);

	/*
	 * the minor numbers for devices attached to the native
	 * ports on an rspc machine are the same as the port number
	 * they are attached to.
	 */

	/* calculate the port number this device is attached to */
	port = atoi(&cusobj->connwhere[strcspn(cusobj->connwhere,"0123456789")]);
	port--;

	/* register this minor number */
	DEBUG_0("gen_rspc_minor: Trying to set in database:\n");
	DEBUG_3("gen_rspc_minor: deviceName = %s, major= %ld, minor = %ld\n",
		cusobj->name, major, port);
	if ((minor = genminor(cusobj->name, major, port, 1, 1, 1)) == NULL) {

            /* Failed to set minor number requested in CuDvDr Class */
	    DEBUG_1("gen_rspc_minor: genminor failed: %ld\n", port);

	    /* see if we can steal this minor number */
            *minP = minor;
            return(-1);
	}

	/* minor number generated OK; return OK */
	DEBUG_1("gen_rspc_minor: returning minor=%d\n",*minor);
    	*minP = minor;
	return(0);
}

int gen_pcmcia_minor(struct CuDv *cusobj, struct CuDv *parobj,
                   long major, long **minP)
{
	long  socket;
	long *minor;
	int   rc;

	DEBUG_4("gen_pcmcia_minor: name=%s parent=%s major=%d socket=%s\n",
		cusobj->name,parobj->name,major,cusobj->connwhere);

	/*
	 * the minor numbers for devices attached to the PCMCIA 
	 * socket on an rspc machine are the same as the PCMCIA socket number
	 * they are attached to.
	 */

	/* calculate the socket number this device is attached to */
	socket = atoi(parobj->connwhere) - 1;

	/* register this minor number */
	DEBUG_0("gen_pcmcia_minor: Trying to set in database:\n");
	DEBUG_3("gen_pcmcia_minor: deviceName = %s, major= %ld, minor = %ld\n",
			cusobj->name, major, socket);
	if ((minor = genminor(cusobj->name, major, socket, 1, 1, 1)) == NULL) {

	    /* Failed to set minor number requested in CuDvDr Class */
	    DEBUG_1("gen_pcmcia_minor: genminor failed: %ld\n", socket);
            /* see if we can steal this minor number */
            *minP = minor;
            return(-1);

	}

	/* minor number generated OK; return OK */
	DEBUG_1("gen_pcmcia_minor: returning minor=%d\n",*minor);
    	*minP = minor;
	return(0);
}

int Run_Other_Method(cusdev,parcusdev,buscusdev,parpredev,attrList, ipl_stat)
struct CuDv      *cusdev;     /* customized device data  */
struct CuDv      *parcusdev;  /* parent device customized data  */
struct CuDv      *buscusdev;  /* bus customized data  */
struct PdDv      *parpredev;  /* parent predefined data  */
struct attr_list *attrList;   /* list of attributes  */
int              *ipl_stat;   /* phase of ipl  */

{ return(0); }



int Do_Other_Processing(cusdev,parcusdev,buscusdev,parpredev,attrList)
struct CuDv      *cusdev;     /* customized device data  */
struct CuDv      *parcusdev;  /* parent device customized data  */
struct CuDv      *buscusdev;  /* bus customized data  */
struct PdDv      *parpredev;  /* parent predefined data  */
struct attr_list *attrList;   /* list of attributes  */

{ return(0); }

/*
 * -----------------------------------------------------------------------------
 *                       MAKE_SPECIAL_FILES
 * -----------------------------------------------------------------------------
 * This function makes the special files for the tty devices.
 *
 * This function operates as a device dependent subroutine called
 * by the configure method for all devices.
 *
 * Return code: Exits with 0 on success, ODM error code otherwise.
 * -----------------------------------------------------------------------------
 */
int Make_Special_Files(logicalName, driverName, majorNo, minorPtr)
char * logicalName;
char * driverName;
long   majorNo;
long * minorPtr;
{
    int    rc;
    dev_t  devno;
    char   file_name[ATTRVALSIZE];

    DEBUG_4("make_special_files: lname=%s drvr=%s major=%d minor=%d\n",
            logicalName,driverName,majorNo,*minorPtr);


    devno = makedev(majorNo, *minorPtr);
    if ((rc = mk_sp_file(devno, logicalName, (long) MKNOD_FLAGS)) != 0) {
         DEBUG_1("make_special_files: Error creating %s\n", logicalName);
         return (E_MKSPECIAL);
    }

    /* That's OK */
    return(0);

}


