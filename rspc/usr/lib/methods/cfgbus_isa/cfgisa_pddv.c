static char sccsid[] = "@(#)09 1.1 src/rspc/usr/lib/methods/cfgbus_isa/cfgisa_pddv.c, rspccfg, rspc41J, 9513A_all 3/28/95 15:58:20";
/*
 *   COMPONENT_NAME: rspccfg
 *
 *   FUNCTIONS:
 *              find_PdDv
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <sys/cfgodm.h>         /* odm database definitions */
#include <odmi.h>               /* odm_* subroutine definitions */
#include <cf.h>                 /* E_xxxx error codes */
#include "isa_info.h"           /* ISA_info node definition */
#include "cfgdebug.h"           /* DEBUG_x declarations */
#include "cfgresid.h"           /* definition for CFG_DEVICE */

extern int get_chipid_descriptor();     /* get chip id from residual data */
extern  int     allpkg;                 /* packaging flag */

/*
 *   NAME:      find_PdDv
 *
 *   FUNCTION:  Get PdDv information for a specific device
 *              that is listed in the residual data device table.
 *              Used for both integrated and non-integrated devices.
 *
 *   INPUT:     resid_dev_ptr : pointer to a ISA_info node to get PdDv for
 *              devnum : device number offset into residual data array
 *   OUTPUT:    pointer to a PdDv entry (malloc'd) to match resid device.
 *   RTN CODE:  pointer to a malloc'd pddv object
 *              NULL if no entry found
 *              E_ODMGET        error getting PdDv odm object
 *              E_NODETECT      cannot find info in residual heap
 *              E_*   for error                                 <<< list
 *
 */

struct PdDv *find_PdDv(resid_dev_ptr, devnum )
CFG_DEVICE   *resid_dev_ptr;      /* DEVICES - residual data */
int devnum;                     /* offset into residual device array */
{

    struct PdDv   *pddv;
    char query_str[256];        /* temp string to store query criteria */
    int rc;                     /* return code from function calls */
    int num_packets;            /* number of packets returned on heap call */
    uchar *buf;                 /* buffer for data         */
    int i;                      /* loop counter */
    char subclass[16];          /* subclass string flag */
    char pkg_str[512];          /* string for package names to output */

    pddv = NULL;
    DEBUG_0("cfgbus_isa: find_PdDv\n");

    /* set subclass for integrated or non-integrated */
    if ( ( resid_dev_ptr->deviceid.flags & INTEGRATED ) == INTEGRATED ) {
	strcpy ( subclass, INTG_SUBCLASS );
    }
    else {
	strcpy ( subclass, NINTG_SUBCLASS );
    }

    /* null out package string */
    pkg_str[0] = '\0';

    /*******************************************************************
     *** Find PdDv entry based on devid alone
     *******************************************************************/

    DEBUG_1("cfgbus_isa: find_PdDv, match on id = %s\n",
			 resid_dev_ptr->pnpid);
    sprintf(query_str, "devid=%s and subclass=%s",
			resid_dev_ptr->pnpid,
			subclass);

    DEBUG_1("cfgbus_isa: find_PdDv, PdDv query string = %s\n",
			 query_str);
    pddv = odm_get_first (PdDv_CLASS, query_str, NULL);

    if ( (int)pddv == -1 ) {
	/* error occured */
	DEBUG_0("cfgbus_isa: find_PdDv, error getting pddv objects\n");
	return ((struct PdDv *) E_ODMGET);
    }

    /* entry found or not, set name to output for install software */
    /* ":" indicates to config manager this is a package name, */
    /*     not a logical name */
    sprintf(pkg_str, ":devices.%s.%s\n",
			subclass,resid_dev_ptr->pnpid);

    /* check if pddv was found, if not, do some other checks */
    if ( pddv == NULL ) {

	if (resid_dev_ptr->deviceid.basetype != SystemPeripheral) {

	    /*******************************************************************
	     *** Find PdDv entry based on compatable device ids
	     *******************************************************************/
	    DEBUG_0("cfgbus_isa: find_PdDv, calling get_deviceid\n");
	    /* rc = get_chipid_descriptor(devnum, 'c', &num_packets, &buf); */
	    num_packets=0;                  /* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	    rc=0;                           /* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

	    DEBUG_2("cfgbus_isa: find_PdDv, get_devid_descriptor rc = %d, returned %d packets\n",
					    rc,num_packets);

	    if ( rc != 0 ) {
		/* return error from subroutine */
		return ((struct PdDv *) rc);
	    }

	    for (i=0; i<num_packets; i++) {
		sprintf(query_str, "devid=%s.%s and subclass=%s",
				    resid_dev_ptr->pnpid,
				    buf+(i*8),
				    subclass);
		pddv = odm_get_first (PdDv_CLASS, query_str, NULL);

		if ( (int)pddv == -1 ) {
		    /* error occured */
		    DEBUG_0("cfgbus_isa: find_PdDv, error getting pddv objects\n");
		    return ((struct PdDv *) E_ODMGET);
		}
		if ( pddv != NULL ) {
		    /* entry found, use it */
		    DEBUG_0("cfgbus_isa: find_PdDv, pddv objects matched this query\n");
		    break;
		}
	    }

	    /*******************************************************************
	     *** Find PdDv entry based on devid alone + serial number
	     *******************************************************************/
	    if ( pddv == NULL &&
		 resid_dev_ptr->deviceid.serialnum != 0xffffffff ) {

		sprintf(query_str, "devid=%s%x and subclass=%s",
				    resid_dev_ptr->pnpid,
				    resid_dev_ptr->deviceid.serialnum,
				    subclass);

		DEBUG_1("cfgbus_isa: find_PdDv, PdDv query string = %s\n",
				     query_str);
		pddv = odm_get_first (PdDv_CLASS, query_str, NULL);

		if ( (int)pddv == -1 ) {
		    /* error occured */
		    DEBUG_0("cfgbus_isa: find_PdDv, error getting pddv objects\n");
		    return ((struct PdDv *) E_ODMGET);
		}
	    }

	    /*******************************************************************
	     *** Find PdDv entry based on compatable device ids + serial number
	     *******************************************************************/
	    DEBUG_0("cfgbus_isa: find_PdDv, calling get_deviceid\n");
	    /* rc = get_chipid_descriptor(devnum, 'c', &num_packets, &buf); */
	    num_packets=0;                  /* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
	    rc=0;                           /* <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

	    DEBUG_2("cfgbus_isa: find_PdDv, get_devid_descriptor rc = %d, returned %d packets\n",
					    rc,num_packets);

	    if ( rc != 0 ) {
		/* return error from subroutine */
		return ((struct PdDv *) rc);
	    }

	    for (i=0; i<num_packets; i++) {

		sprintf(query_str, "devid=%s%x and subclass=%s",
				    buf+(i*8),
				    resid_dev_ptr->deviceid.serialnum,
				    subclass);
		pddv = odm_get_first (PdDv_CLASS, query_str, NULL);

		if ( (int)pddv == -1 ) {
		    /* error occured */
		    DEBUG_0("cfgbus_isa: find_PdDv, error getting pddv objects\n");
		    return ((struct PdDv *) E_ODMGET);
		}
		if ( pddv != NULL ) {
		    /* entry found, use it */
		    DEBUG_0("cfgbus_isa: find_PdDv, pddv objects matched this query\n");
		    break;
		}
	    }

	}
	else {          /* system peripheral and power management */

	    /*******************************************************************
	     *** Find PdDv entry based on device id + "." + chip ids
	     *******************************************************************/
	    DEBUG_0("cfgbus_isa: find_PdDv, calling get_chipid\n");
	    rc = get_chipid_descriptor(devnum, 'a', &num_packets, &buf);
	    DEBUG_2("cfgbus_isa: find_PdDv, get_chipid_descriptor rc = %d, returned %d packets\n",
					    rc,num_packets);
	    if ( rc != 0 ) {
		/* return error from subroutine */
		return ((struct PdDv *) rc);
	    }

	    for (i=0; i<num_packets; i++) {
		sprintf(query_str, "devid=%s.%s and subclass=%s",
				    resid_dev_ptr->pnpid,
				    buf+(i*8),
				    subclass);
		pddv = odm_get_first (PdDv_CLASS, query_str, NULL);

		if ( (int)pddv == -1 ) {
		    /* error occured */
		    DEBUG_0("cfgbus_isa: find_PdDv, error getting pddv objects\n");
		    return ((struct PdDv *) E_ODMGET);
		}
		if ( pddv != NULL ) {
		    /* entry found, use it */
		    DEBUG_0("cfgbus_isa: find_PdDv, pddv objects matched this query\n");
		    break;
		}
	    }

	    /*******************************************************************
	     *** Find PdDv entry based on devid + compatable chip id
	     *******************************************************************/
	    if ( pddv == NULL ) {
		/* pddv was not found above, try matching on compatible ids */
		DEBUG_0("cfgbus_isa: find_PdDv, calling get_chipid\n");
		rc = get_chipid_descriptor(devnum, 'c', &num_packets, &buf);
		DEBUG_2("cfgbus_isa: find_PdDv, get_chipid_descriptor rc = %d, returned %d packets\n",
						rc,num_packets);
		if ( rc != 0 ) {
		    /* return error from subroutine */
		    return ((struct PdDv *) rc);
		}

		for (i=0; i<num_packets; i++) {
		    sprintf(query_str, "devid=%s.%s and subclass=%s",
					resid_dev_ptr->pnpid,
					buf+(i*8),
					subclass);
		    pddv = odm_get_first (PdDv_CLASS, query_str, NULL);

		    if ( (int)pddv == -1 ) {
			/* error occured */
			DEBUG_0("cfgbus_isa: find_PdDv, error getting pddv objects\n");
			return ((struct PdDv *) E_ODMGET);
		    }
		    if ( pddv != NULL ) {
			/* entry found, use it */
			DEBUG_0("cfgbus_isa: find_PdDv, pddv objects matched this query\n");
			break;
		    }
		}


	    }
	}

    }

    if ( pddv == NULL || allpkg == 1 ) {
	/* entry not found, output name to install software */
	fprintf(stdout, "%s", pkg_str);

    }

    return (pddv);

}
