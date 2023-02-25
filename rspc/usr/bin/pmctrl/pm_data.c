static char sccsid[] = "@(#)27  1.6  src/rspc/usr/bin/pmctrl/pm_data.c, pwrcmd, rspc41J, 9519A_all 5/9/95 22:52:13";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: change_dev_odm
 *            : get_dev_odm
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>

#include <errno.h>

extern int errno;

#ifdef PM_DEBUG
#define Debug0(format)                fprintf(stderr,format)
#define Debug1(format,arg1)           fprintf(stderr,format,arg1)
#define Debug2(format,arg1,arg2)      fprintf(stderr,format,arg1,arg2)
#define Debug3(format,arg1,arg2,arg3) fprintf(stderr,format,arg1,arg2,arg3)
#else 
#define Debug0(format)                
#define Debug1(format,arg1)           
#define Debug2(format,arg1,arg2)      
#define Debug3(format,arg1,arg2,arg3) 
#endif /* PM_DEBUG */

#define Free(pointer) if (pointer !=NULL){free(pointer);pointer=NULL;}

int change_dev_odm(char *, char *, int);
int get_dev_odm(char *,char *, char *);
int check_dev_odm(char *, char *, int);
int parse_range(char *, unsigned long[]);

/*
 *  NAME:       change_dev_odm()
 *
 *  FUNCTION:   Change CuAt ODM data for each device
 *
 *  INPUTS:     lname     -- device logical name
 *              attribute -- attribute to be changed
 *              value     -- new value
 *
 *  OUTPUTS:     0 :  success
 *              -1 :  failure
 *
 *  NOTES:
 *
 */
int
change_dev_odm(char *lname, char *attr, int value)
{
    struct CuAt *cuat;
    int         howmany = 1;
    int         rc = -1;

    /* get an attribute value of specified device */
    if( (cuat = getattr(lname, attr, 0, &howmany)) == NULL ){
        rc = -1;
    }
    else {
	Debug2("previous value of %s is %s\n",attr,cuat->value);
        sprintf(cuat->value, "%d", value);
	Debug1("now set to %d\n",value);
	/* update the attribute value to new value */
        rc = putattr(cuat);      

	/* if successefully changed, invoke savebase */
	if (rc == 0){
	    system("/usr/sbin/savebase");
	}
    }

    return(rc);
}

/*
 *  NAME:       get_dev_odm()
 *
 *  FUNCTION:   get the value of the specified attribute if it
 *              exists in ODM data for each device
 *
 *  INPUTS:     lname     -- device logical name
 *              attribute -- attribute to be checked
 *              pvalue    -- pointer to the value
 *
 *  OUTPUTS:    0 : success
 *             -1 : failure
 *
 *  NOTES:
 *
 */
int
get_dev_odm(char *lname, char *attr, char *pvalue)
{
    struct CuAt *cuat;
    int         howmany = 1;
    int         rc = -1;

    /* get an attribute value of specified device */
    if( (cuat = getattr(lname, attr, 0, &howmany)) == NULL ){
        rc = -1;
    }
    else {
        if (pvalue!=NULL) strcpy(pvalue,cuat->value);
        rc = 0;
    }
    return(rc);
}

/*
 *  NAME:       check_dev_odm
 *
 *  FUNCTION:   search for the odm data of the specified device 'lname'
 *              of the specified attribute 'att' and check if the 
 *              specified value 'val' is within the range described
 *              in 'values' in odm data.
 *
 *  NOTES:      first, search CuDv for the 'uniquetype' of the specified 
 *              devices by means of logical name 'lname' as a key.
 *              second, search PdAt for the 'values' of the specified 
 *              attribute 'att' by means of the uniquetype 'uniquetype' 
 *              and the attrigute 'att' as a key. 
 *     
 *  RETURN VALUE:
 *              0 : specified value is within the range.
 *             -1 : processed unsuccessfully.
 */
int
check_dev_odm(char *lname, char *att, int val)
{
    char criteria[256];
    struct CuDv cudv;
    struct PdAt pdat;
    char buffer[256];
    char *ebuffer;
    int rc = 0;
    unsigned long range[3] = {0,0,0};/* for checking range */
    long match = 0;/* for checking numeral */
    int argc = 0; /* for checking list */
    char *argv[ATTRVALSIZE];  /* for checking list */

    int rFlag = 0;/* for rep flag (range)*/
    int lFlag = 0;/* for rep flag (list)*/
    int nFlag = 0;/* for rep flag (numeral)*/

    /* initialization */
    memset(buffer,0,256);
    memset(criteria,0,256);
    memset(&cudv,0,sizeof(cudv));
    memset(&pdat,0,sizeof(pdat));

    /* get CuDv data for specified device by means of logical name as a key */

    memset(criteria,0,256);
    sprintf(criteria, "name = %s", lname);
    if (odm_get_obj(CuDv_CLASS, criteria, &cudv, ODM_FIRST) <= 0) {
	return -1;
    }

    /* get PdAt data for specified device by means of uniquetype */
    /* and attrigute as a key */

    memset(criteria,0,256);
    sprintf(criteria,"uniquetype = %s AND attribute = %s",
	    cudv.PdDvLn_Lvalue,
	    att);
    if (odm_get_obj(PdAt_CLASS, criteria, &pdat, ODM_FIRST) <= 0) {
	return -1;
    }

    Debug3("lname= %s attribute= %s values = %s \n",
	   cudv.name,
	   att,
	   pdat.values); 
    Debug3("lname= %s attribute= %s reps = %s \n",
	   cudv.name,
	   att,
	   pdat.rep); 
    Debug1("specified value is %d\n",val);

    /* set rep flag */
    if (strchr(pdat.rep,'r') != 0){
	rFlag = 1;
	Debug0("rFlag on\n");
    }
    if (strchr(pdat.rep,'l') != 0){
	lFlag = 1;
	Debug0("lFlag on\n");
    }
    if (strchr(pdat.rep,'n') != 0){
	nFlag = 1;
	Debug0("nFlag on\n");
    }
    /* check the specified value in the specified attribute */

    /* parsing */

    if (rFlag && lFlag ){
	return -1;
    }
    else if (nFlag && !rFlag && !lFlag){

	Debug0("checking match\n");

	/* if one numeral data is specified as 'values' */
        errno = 0;
	match = strtol(pdat.values,(char **)NULL,0);
	if (errno) return -1;

        /* checking if it is matched */         
	if ( val != match ){
	    return -1;
	}

    }else if (rFlag && nFlag){

	Debug0("checking range\n");

	/* if range is specified as 'values' */

	if (parse_range(pdat.values,range) != 0){
	    return -1;
	}
	Debug0("parsing range OK\n");
	Debug3("range %d < %d , %d \n",range[0],range[1],range[2]);

	/* checking range */

	if ( val < range[0] || val > range[1] ||
	    (val - range[0]) % range[2] != 0 ){
	    return -1;
	}

    }else if (lFlag && nFlag){

	Debug0("checking list\n");

	/* if list is specified as 'values' */

	if (parse_list(pdat.values,&argc,argv) != 0){
	    return -1;
	}
	Debug0("parsing list OK\n");
	Debug2("argc : %d argv[0]=%s\n",argc,argv[0]);

	/* checking list */

	while(argc--){
	    errno = 0;
	    if ( strtol(argv[argc],(char **)NULL,0) == val &&
		 errno == 0) break;
	}
	if (argc<0) {
	    return -1;
	}
    }

    Debug0("OK: specified value is within the range\n");
    return 0;	
}

/*
 * FUNCTION: parse_range
 *       Decode input for range value.
 *
 * INPUT PARAMETERS:
 *       str   -- PdAt values field
 *       range -- 3 elem array to store lower, upper, increment
 *
 * RETURN VALUE DESCRIPTION:
 *              0 -- successful
 *              -1 -- failed
 */
int 
parse_range(char *str, unsigned long range[])
{
    int errflg = 0;

    if (!strchr(str, '-'))
	return -1;

    /* Convert lower-upper,increment string to ulong values */
    errno = 0;
    range[0] = strtoul(str, (char **)NULL, 0);
    errflg += errno;
    range[1] = strtoul(strchr(str, '-') + 1, (char **)NULL, 0);
    errflg += errno;
    range[2] = (str = strchr(str, ',')) ? 
	strtoul(str + 1, (char **)NULL, 0) : 1;
    errflg += errno;
    
    if (errflg) return -1;
    
    if ((range[1] <= range[0]) || range[2] == 0)
	return -1;
    return 0;
}

/*
 * NAME : parse_list
 *
 * FUNCTION : seperates the specified string into seperate arguments and
 *            returns pointers to each in the argv array. Commas and white
 *            space (as defined by isspace()) delimit each argument.
 *
 *            Note the input string is modified by this function.
 *
 * PARAMETERS :
 *      cp        - string to chop up
 *      argc      - # of args in argv
 *      argv      - array of pointers to each seperated argument (returned)
 *
 * RETURN VALUE:
 *      0 - ALWAYS successful
 *
 */
int 
parse_list(char *cp,int *argc,char *argv[])
{
    char *substr;
    char *tokstr = ", \t\v\r\n"; 
    /* blank, tab, vert tab, carr ret, new line */
    
    *argc = 0;
    
    for(substr = strtok(cp, tokstr);substr;substr = strtok(NULL, tokstr)){
	argv[(*argc)++] = substr;
    }
    return 0;
}






