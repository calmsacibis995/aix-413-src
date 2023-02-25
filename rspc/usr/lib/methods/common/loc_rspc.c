static char sccsid[] = "@(#)47  1.8  src/rspc/usr/lib/methods/common/loc_rspc.c, rspccfg, rspc41J, 9523C_all 6/2/95 16:19:24";
/*
 * COMPONENT_NAME: (RSPCCFG) location.c - Build location code
 *
 * FUNCTIONS: location()
 *
 * ORIGINS: 27
 *
 *  This module contains IBM CONFIDENTIAL code. -- (IBM
 *  Confidential Restricted when combined with the aggregated
 *  modules for this product)
 *                   SOURCE MATERIALS
 *  (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 *  All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#include "cfgdebug.h"

/*
 * NAME: location
 * 
 * FUNCTION: This function constructs the location code for a device.
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This function is called by both the generic define method and
 *	the generic change method.
 *
 * RETURNS: Returns with 0 on completion.
 */
int
location (par_utype,par_loc,cusobj)
char *par_utype, *par_loc;
struct CuDv *cusobj;
{
	char    devcst[UNIQUESIZE],     /* space for class,subclass & type */
	        Parcst[UNIQUESIZE],     /* space for class,subclass & type */
		*devclass,		/* pionter to device class */
		*devsubc,		/* pointer to device subclass */
		*devtype,               /* pointer to device type */
	        *ParClass,		/* ptr to parent's class    */
	        *ParSubClass,           /* ptr to parent's subclass */
	        *ParType;		/* ptr to parent's types    */
	int	connection;		/* connection as an integer */

	DEBUG_0 ("Entering location(): Build location value\n")

       /*------------------------------------------------------------ 
	*  Break out the parts of the parent's unique type for easier
	*  reference later.
	*------------------------------------------------------------*/

	strcpy(Parcst,par_utype);
	ParClass   = strtok(Parcst,"/");
	ParSubClass= strtok((char *)0,"/");
	ParType    = strtok((char *)0,"/");

	DEBUG_1("location: par_utype = %s\n", par_utype) ;
	DEBUG_3("location: ParClass= %s   ParSubClass= %s   ParType= %s\n",
		ParClass, ParSubClass, ParType) ;

       /*------------------------------------------------------- 
	*  Break out the parts of the device's predefined unique 
	*  type for easier reference later.
	*-------------------------------------------------------*/

	strcpy(devcst,cusobj->PdDvLn_Lvalue);
	devclass=strtok(devcst,"/");
	devsubc=strtok((char *)0,"/");
	devtype=strtok((char *)0,"/");

	connection=atoi(&cusobj->connwhere[strcspn(cusobj->connwhere,
					           "0123456789")]);
	DEBUG_1 ("location: connection = %s\n",cusobj->connwhere)
	DEBUG_1 ("location: parent location = %s\n",par_loc)

       /*--------------------------------------------------------------
	* Now "case" through the various special cases so that the
	* location code can be set correctly.  If no special case 
	* exists, the default is to use the connwhere passed in the 
	* CuDv.
	* 
	* Order is somewhat important.  You must be careful that a
	* generic specification high in the list does not catch a 
	* more specific specification that appears later.
	* 
	* The current order attempts to follow the heirarchy of the
	* device trees (planar devices first, adapters next (including 
	* subset adapters (ie. sio)), and finally end devices 
	* (ie. scsi and tty type devices). 
	*--------------------------------------------------------------*/
	    
	if (!strcmp(devsubc, "isa"))
	{
	   /*--------------------------------------------------- 
	    * Device is an isa adapter
	    *---------------------------------------------------*/
	   sprintf(cusobj->location, "01-%02d", connection);
	}

	else if (!strcmp(devsubc, "pcmcia"))
	{
	   /*---------------------------------------------------
	    * Device is a pcmcia adapter
	    *---------------------------------------------------*/
	    sprintf(cusobj->location, "05-%02d", connection);
	}

	else /* no special processing required */ 
        {
	    sprintf(cusobj->location,"%s-%02d",par_loc,connection);
	}

	return(0);
}

