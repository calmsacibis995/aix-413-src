static char sccsid[] = "@(#)31	1.22  src/bos/usr/sbin/lvm/liblv/util8.c, cmdlvm, bos411, 9428A410j 3/22/94 23:02:58";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: get_prefix,checkname,convt_pvid_32,
 *	      get_pvname,convt_pvid_16. 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS	/* performance enhancement for isdigit(), etc. calls */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include "cmdlvm.h"
#include "cmdlvm_msg.h"
#include <ctype.h>


extern odm_error();
extern int odmerrno;
extern nl_catd  scmc_catd;     /* Cat descriptor for scmc conversion */
extern char *Prog;             /* program name */

#define ODMERROR -1	/* cfg error return code */
#define PVATTR "pvid"
#define ZEROPV "0000000000000000"

/*
 *
 * NAME: get_prefix
 *	
 * DESCRIPTION: Accesses the Predefined Attribute class in ODM to 
 *	find a prefix for the specified type of logical volume.     
 *			
 * RETURN CODE DESCRIPTION: Returns TRUE if a prefix was found for 
 *	the specified type. Returns FALSE if a prefix was not found 
 *	to match the type.
 */ 
int
get_prefix(lvtype,prefix)
char *lvtype;		/* Type specified by user or getlvname (INPUT) */
char *prefix;		/* return prefix (OUTPUT) */
{
struct PdAt *pdatp;          	/* Predefined Attribute class pointer */
struct listinfo stat_info; 	/* return structure for odm_get_list */
char crit[CRITSIZE];		/* search criteria string */

	/* specify logical volume uniquetype and logical volume type */
	/* to search for */
       	sprintf(crit,"uniquetype = '%s' and attribute = '%s'",
		LVUNQPREFIX,lvtype);

	/* call odm_get_list to search for prefix */
	pdatp = (struct PdAt *)odm_get_list(PdAt_CLASS,crit,&stat_info,1,1);

	/* if pointer returns a negative one then an ODM error has occurred */
	if ((int)pdatp == ODMERROR)
		/* Print ODM error message and exit */
		odm_error(odmerrno);

	else {
		/* if a prefix was found, set prefix and return a TRUE value */
		if (pdatp != NULL) {
			sprintf(prefix,"%s",pdatp->deflt);
			return(TRUE);
		}
	
		/* else set prefix to NULL and return a FALSE value */
		else {
			prefix = NULL;
			return(FALSE);
		}
	}
}
/*
 * NAME: checkname   
 *
 * DESCRIPTION: Checks a device name to see if it already exists 
 *	in the CuDv class or if ends in a digit.
 *		
 * INPUT: The device name. 
 *
 * OUTPUT: Error message if criteria is not met.  
 *
 * RETURN VALUE DESCRIPTION: None.
 *
 */
void
checkname(devname,Prog)
char *devname;
char *Prog;
{
struct CuDv *cudvp;		/* pointer to CuDv class */
struct PdDv *pddvp;		/* pointer to PdDv class */
struct listinfo stat_info;	/* values from getlist returned struct */
char crit[CRITSIZE];		/* criteria for search of class in get list */
char prefix[STRSIZE];		/* array of characters to hold search prefix */
char *tempptr;			/* temp ptr to check last character of name */
int count;			/* prefix size */

	/* check if last character is a number */
	tempptr = devname;		/* temporary hold */
	strcpy(prefix," ");

	while ((*tempptr != '\0') && (*tempptr != prefix[0])) {
		++tempptr;
	}
	--tempptr;
	while (isdigit(*tempptr) != 0) {
		tempptr--;
	}
	count = tempptr - devname + 1;
	strncpy(prefix,devname,count);

	/* check PdDv class to see if prefix exists in database */
	sprintf(crit,"prefix = '%s'",prefix);
	pddvp = (struct PdDv *)odm_get_list(PdDv_CLASS,crit,&stat_info,1,1);
	/* if ODM error then print error message and exit */
	if ((int)pddvp == ODMERROR) 
		odm_error(odmerrno);
	/* else if a NULL pointer was NOT returned then the name exists */
	/* print message and exit */
	if (pddvp != NULL) {
		if ((strcmp(pddvp->prefix,"lv") != 0) && 
			(strcmp(pddvp->prefix,"vg") != 0)) {
		fprintf(stderr,lvm_msg(PREFIX_EXISTS),Prog);
		exit(FAILURE);
		}
	}

	/* check CuDv class to see if name exists in database */
	sprintf(crit,"name = '%s'",devname);
	cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,1,1);
	/* if ODM error then print error message and exit */
	if ((int)cudvp == ODMERROR) 
		odm_error(odmerrno);
	/* else if a NULL pointer was NOT returned then the name exists */
	/* print message and exit */
	if (cudvp != NULL) {
		fprintf(stderr,lvm_msg(NAME_EXISTS),Prog);
		exit(FAILURE);
	}

}
/*
 * NAME: convt_pvid_32     
 *
 * DESCRIPTION: Converts a pvid from a 16 char array into a
 *	32 char array.
 *		
 * INPUT: The 16 char array.
 *
 * OUTPUT: The 32 char array.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  No error found.               
 */
void
convt_pvid_32(pvid16,pvid32)
char *pvid16;		/* INPUT */
char *pvid32;		/* OUTPUT */
{
	bzero(pvid32,CUPVPVID);
	strncpy(pvid32,pvid16,16);
	strcat(pvid32,"0000000000000000");
	return;
}

/*
 * NAME: get_pvname     
 *
 * DESCRIPTION: Gets a specified device name given the device identifier
 *	and device type (either logical volume, volume group,
 *	or physical volume.  Gets the name from the CuAt class.
 *		
 * INPUT: The physical volume identifier.
 *
 * OUTPUT: The physical volume name in pvname.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 2:  If ODM error.            
 *			 3:  Identifier not found and errflag set.
 */
void
get_pvname(pvid,pvname)
char *pvid;		/* physical volume identifier (INPUT) */
char *pvname;		/* the device name (OUTPUT) */
{
struct CuAt *cuatp;
struct listinfo stat_info;	/* returns values from odm_get_list */
char crit[CRITSIZE];		/* criteria for search of class in get list */
char longpvid[CUPVPVID];	/* string to hold a 32 pvid */

	convt_pvid_32(pvid,longpvid);
	sprintf(crit,"attribute = '%s' and value = '%s'",PVATTR,longpvid);
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	if ((int)cuatp == -1) 
		odm_error(odmerrno);
	if (cuatp == NULL) {
		fprintf(stderr,lvm_msg(ID_NOT_FOUND),Prog,pvid);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}
	else
		sprintf(pvname,"%s",cuatp->name);
}

/*
 * NAME: convt_pvid_16     
 *
 * DESCRIPTION: Converts a pvid from a 32 char array into a
 *	16 char array.
 *		
 * INPUT: The 32 char array.
 *
 * OUTPUT: The 16 char array.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  No error found.               
 */
void
convt_pvid_16(pvid32,pvid16)
char *pvid32;		/* INPUT */
char *pvid16;		/* OUTPUT */
{
	strncpy(pvid16,pvid32,16);
	*(pvid16+16) = '\0';
	return;
}
