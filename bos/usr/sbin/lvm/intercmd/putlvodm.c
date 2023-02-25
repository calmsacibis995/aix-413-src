static char sccsid[] = "@(#)38	1.25.2.6  src/bos/usr/sbin/lvm/intercmd/putlvodm.c, cmdlvm, bos41J, bai15 3/30/95 08:45:14";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	getflg,ck_required,addcuat,addcudv,addcudep,addlogvol,addvolgrp,
 *	rmlogvol,rmvolgrp,chng_attr,chng_lvatnm,chng_lvnm,
 *	rm_pv,check_lvid,get_lvinfo,get_dvname,usage_error,
 *	ck_field_ln,main
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


/*
 * FILE NAME: putlvodm  
 *
 * FILE DESCRIPTION: Source file for putlvodm command.  "putlvodm" provides an
 *	intermediate level of command interface.  "Putlvodm" puts volume group 
 *	and logical volume data into the Object Data Manager (ODM).
 *
 *	For additional information on this program and the other 
 *	lvm immediate level command programs see the specification 
 *	"Logical Volume Manager Intermediate Level  Command Interface"
 *	and also the interface documentation to the lvm library functions.
 *         
 * RETURN VALUE DESCRIPTION:
 *			0   Command Successfully completed
 *		        1   Illegal Syntax
 *			2   ODM Access Error
 *			3   Object not found in ODM 
 *
 * EXTERNAL PROCEDURES CALLED: setflg, odm_open_class, odm_add_obj, 
 * 			       getmem, odm_initialize, odm_rm_obj,
 * 			       odm_change_obj, odm_terminate.
 *
 * EXTERNAL VARIABLES: optind, optarg.
 *
 * 
 * FUNCTION_NAMES: getflg, ck_required, addcuat, addcust, addcudv,
 * 		   addcudep, main
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>
#include <sys/signal.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include "cmdlvm.h"
#include "cmdlvm_msg.h"		/* include files for message tests */

/*
 * EXTERNAL PROCEDURES CALLED:
 */
extern odm_initialize();
extern odm_add_obj();
extern odm_rm_obj();
extern char * odm_set_path();	/* odm rountine to set config path */
extern int * genminor();
extern int  genmajor();
extern int * getminor();
extern int putattr();
extern struct CuAt * getattr();
  
/* local library functions */
extern int setflg();
extern char * getmem();
extern odm_terminate();
extern odm_error();
extern get_pvname();
extern convt_pvid_32();

#define ODMERROR -1

/* external variables */
extern int odmerrno;
extern int optind;		/* index to next argv argument */
				/* - incremented by getopt      */
extern char *optarg;		/* pointer to command line argument string */
				/* - set by getopt  */
extern char *Prog;			/* points to name of the current Program */
extern nl_catd  scmc_catd;   		/* Cat descriptor for scmc conversion */

/*
 * GLOBAL VARIABLES:
 */

/* global structure */
struct uniqueid {
	unsigned int word1;
	unsigned int word2;
	}; 
struct sigaction action;		/* parameters for sigaction */
struct sigaction oldaction;		/* parameters for sigaction */

/* global variables */
char **idptr;			/* buffer for the file name */
				/* (non-option field in setflg) */
long *cuathdl;   		/* structure to maintain the return */
				/* value from an odm_open_class */ 
long *cudvhdl;			/* structure to maintain the return */
				/* value from an odm_open_class */ 
long *cudephdl;			/* structure to maintain the return */
				/* value from an odm_open_class */ 
int firstcuat;			/* boolean for opening of class CuAt */
int firstcudv;			/* boolean for opening of class CuDv */
int firstcudep;			/* boolean for opening of class CuDep */

/* Pointer to command line parameters associated */
/* with each command line option. These variables */
/* only contain values if corresponding flag is true */
/* - see below. */
static char *a_value = " ";
static char *B_value = " ";
static char *c_value = " ";
static char *e_value = " ";
static char *l_value = " ";
static char *m_value = " ";
static char *n_value = " ";
static char *O_value = " ";
static char *o_value = " ";
static char *p_value = " ";
static char *q_value = " ";
static char *r_value = " ";
static char *s_value = " ";
static char *S_value = " ";
static char *t_value = " ";
static char *u_value = " ";
static char *v_value = " ";
static char *y_value = " ";
static char *z_value = " ";
static char *Q_value = " ";
    

/* Each flag corresponds to a command option - */
/* flag true if option specified on command line */
static int a_flag = FALSE;
static int B_flag = FALSE;
static int c_flag = FALSE;
static int e_flag = FALSE;
static int k_flag = FALSE;
static int K_flag = FALSE;
static int l_flag = FALSE;
static int L_flag = FALSE;
static int m_flag = FALSE;
static int n_flag = FALSE;
static int O_flag = FALSE;
static int o_flag = FALSE;
static int p_flag = FALSE;
static int P_flag = FALSE;
static int q_flag = FALSE;
static int r_flag = FALSE;
static int s_flag = FALSE;
static int S_flag = FALSE;
static int t_flag = FALSE;
static int u_flag = FALSE;
static int v_flag = FALSE;
static int V_flag = FALSE;
static int y_flag = FALSE;
static int z_flag = FALSE;
static int Q_flag = FALSE;
int lock_word;

/*
 * NAME: usage_error
 *
 * DESCRIPTION: Prints out usage message for putlvodm command
 *
 * INPUT: None
 *
 * OUTPUT: Usage message printed to standard error.
 *
 */
void 
usage_error()
{
	fprintf(stderr,lvm_msg(PUTODM_USE),Prog);
}

/*
 * NAME: getflg
 *
 * DESCRIPTION: Parses command line options until the first non-option 
 *		is encountered.	For each option in the command line,
 *	 	a global flag is set and a global value is loaded with 
 * 		any accompaning command argument. (Called from setflg.)
 *
 * INPUT: The command line arguments and number of arguments.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			-1:  If illegal option found.
 *			 1:  If options parsed correctly.
 *			 0:  If no command options found.
 */
int getflg(argc,argv)
int argc;	 /* number of command line strings referenced by argv */
char **argv;	 /* array of command line strings */
{
	int rc; 		/* getopt return value */

	if (!argc) return(0);	/* if no command line options to parse */
				/* then return */

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv,
		 "a:B:c:e:kKl:Lm:n:o:O:p:Pq:r:s:S:t:u:v:Vx:y:z:Q:")) != EOF) {
		switch (rc) {
		case 'a':
			a_flag = TRUE;
			a_value = optarg;
			break;
		case 'B':
			B_flag = TRUE;
			B_value = optarg;
			break;
		case 'c':
			c_flag = TRUE;
			c_value = optarg;
			break;
		case 'e':
			e_flag = TRUE;
			e_value = optarg;
			break;
		case 'k':
			k_flag = TRUE;
			break;
		case 'K':
			K_flag = TRUE;
			break;
		case 'l':
			l_flag = TRUE;
			l_value = optarg;
			break;
		case 'L':
			L_flag = TRUE;
			break;
		case 'm':
			m_flag = TRUE;
			m_value = optarg;
			break;
		case 'n':
			n_flag = TRUE;
			n_value = optarg;
			break;
		case 'O':
			O_flag = TRUE;
			O_value = optarg;
			break;
		case 'o':
			o_flag = TRUE;
			o_value = optarg;
			break;
		case 'p':
			p_flag = TRUE;
			p_value = optarg;
			break;
		case 'P':
			P_flag = TRUE;
			break;
		case 'q':
			q_flag = TRUE;
			q_value = optarg;
			break;
		case 'r':
			r_flag = TRUE;
			r_value = optarg;
			break;
		case 's':
			s_flag = TRUE;
			s_value = optarg;
			break;
		case 'S':
			S_flag = TRUE;
			S_value = optarg;
			break;
		case 't':
			t_flag = TRUE;
			t_value = optarg;
			break;
		case 'u':
			u_flag = TRUE;
			u_value = optarg;
			break;
		case 'v':
			v_flag = TRUE;
			v_value = optarg;
			break;
		case 'V':
			V_flag = TRUE;
			break;
		case 'y':
			y_flag = TRUE;
			y_value = optarg;
			break;
		case 'z':
			z_flag = TRUE;
			z_value = optarg;
			break;
		case 'Q':
			Q_flag = TRUE;
			Q_value = optarg;
			break;
		default:
			usage_error();
			return(-1);
		case '?':
			usage_error();
			return(-1);
		}
	}
	return(1);
}

/*
 * NAME: ck_required
 *
 * DESCRIPTION: Check input flag indicators to determine if all required fields
 *	have been received and too many have not been added.
 *
 * RETURN VALUE DESCRIPTION: If zero then no error.  
 *		If not zero then error 
 *
 * INPUT: None.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			-1:  If illegal option found.
 *			 0:  No error found.               
 */
static int
ck_required()
{

	if (*idptr == NULL) { 
	/* If there was no argument on the command line then error */
		fprintf(stderr,lvm_msg(MISSING_ID),Prog);
		usage_error();
		return(ILLEGALSYNTAX);
	}
	if (L_flag && (a_flag || c_flag || e_flag || k_flag || 
		K_flag || l_flag || n_flag || o_flag || p_flag || 
		P_flag || q_flag || r_flag || s_flag || S_flag || 
		t_flag || u_flag || v_flag || V_flag || y_flag || 
		B_flag || O_flag)) {
		fprintf(stderr,lvm_msg(BAD_FLAG),Prog);
		usage_error();
		return(ILLEGALSYNTAX);
	}
	if (V_flag && (a_flag || c_flag || e_flag || k_flag || K_flag || 
		L_flag || l_flag || n_flag || o_flag || p_flag || 
		P_flag || q_flag || r_flag || s_flag || S_flag ||
		t_flag || u_flag || v_flag || y_flag || B_flag ||
		O_flag)) {
		fprintf(stderr,lvm_msg(BAD_FLAG2),Prog);
		usage_error();
		return(ILLEGALSYNTAX);
	}
	if (strlen(*idptr) < VGIDLENGTH) {
		fprintf(stderr,lvm_msg(BAD_VGID),Prog);
		usage_error();
		return(ILLEGALSYNTAX);
	}
	return(SUCCESS);
}
/*
 * NAME: addcuat     
 *
 * DESCRIPTION: Adds a logical volume or volume group attribute 
 *	to the CuAt class.
 *		
 * INPUT: The attribute to add, its type in ODM, its non-default value,
 *	its default value and the name of the device being added. 
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  No error found.               
 *			 2:  If ODM error.            
 *                       3:  Object not found.
 */
void 
addcuat(cuatp,dvname,attribute,uniquetype,attflag,attvalue)
struct CuAt *cuatp;	/* Customized Attribute class pointer */
char *dvname;		/* The device name to be added */
char *attribute;	/* The name of the attribute */
char *uniquetype;	/* Specific uniquetype for attribute */
int  attflag;		/* The attribute flag in the command line */
char *attvalue;		/* The attributes non-default value */
{
struct PdAt *pdatp;
struct listinfo stat_info;	/* returns values from odm_get_list */
char crit[CRITSIZE];			/* criteria for search of class in get list */
long *pdathdl;

	bzero(cuatp,sizeof(struct CuAt));

	/* Set the values for the CuAt struct */
	strncpy(cuatp->name,dvname,DEVNAMESIZE);
	strcpy(cuatp->attribute,attribute);
 
	/* get Predefined attribute value and flags */
     	sprintf(crit,"uniquetype = '%s' and attribute = '%s'",
		uniquetype,attribute);
	
	pdatp = (struct PdAt *)odm_get_list(PdAt_CLASS,crit,&stat_info,1,1);
	if ((int)pdatp == -1) 
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (pdatp != NULL) {
		/* If the flag was specified, use the value specified */
		if (attflag) 
			strcpy(cuatp->value,attvalue);
		else
			strcpy(cuatp->value,pdatp->deflt);
       		strcpy(cuatp->type,pdatp->type);
       		strcpy(cuatp->generic,pdatp->generic);
       		strcpy(cuatp->rep,pdatp->rep);
       		cuatp->nls_index = pdatp->nls_index;
	}
	else {
		fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,attribute);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}

	/* If first time though, open the CuAt class */
	if (firstcuat) { 
		cuathdl = (long *)odm_open_class(CuAt_CLASS);
		if ((int)cuathdl == -1) 
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
	}
	firstcuat = FALSE;
	if ( !strcmp(attribute,"pv"))
	{
		if (odm_add_obj(CuAt_CLASS,cuatp) == -1)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
	}
	else
	{
		if (putattr(cuatp) == ODMERROR)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
	}
}
/*
 * NAME: addcudv     
 *
 * DESCRIPTION: Adds a logical volume or volume group instance to the 
 *	Customized Device class.
 *		
 * INPUT: The device name.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 2:  If ODM error.            
 *                       3:  Object not found.
 *			 0:  No error found.               
 */
void 
addcudv(cudvp,dvname,lvflag,vgname)
struct CuDv *cudvp;	/* Customized Device class pointer */
char *dvname;		/* device name (lvname or vgname) to be added */
int lvflag;		/* flag to tell if logical volume or volume group */
char *vgname;		/* volume group name or parent device */
{

	bzero(cudvp,sizeof(struct CuDv));
	strncpy(cudvp->name,dvname,DEVNAMESIZE);
	cudvp->status = STATUS;
	cudvp->chgstatus = CHGSTATUS;
	if (lvflag) {
		strcpy(cudvp->parent,vgname);
		strcpy(cudvp->PdDvLn_Lvalue,LVUNIQUETYPE);
	}
	else {
		strcpy(cudvp->parent,NULL);
		strcpy(cudvp->PdDvLn_Lvalue,VGUNIQUETYPE);
	}
	/* Open the CuDv class and add the new device instance */	
	if (firstcudv) {
		cudvhdl = (long *)odm_open_class(CuDv_CLASS);
		if ((int)cudvhdl == -1) 
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
	}
	firstcudv = FALSE;
	if (odm_add_obj(CuDv_CLASS,cudvp) == -1) 
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);	
	}
}

/*
 * NAME: addcudep     
 *
 * DESCRIPTION: Adds a connection instance for a volume group for either a 
 *	physical volume or a logical volume with the appropriate 
 *	connection type. 
 *		
 * INPUT: The volume group name, the connection device name and the relation.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  No error found.               
 *			 2:  If ODM error.            
 */
void
addcudep(cudepp,dvname,depname)
struct CuDep *cudepp;	/* Customized Dependency class pointer */
char *dvname;		/* the volume group name to be added */
char *depname;		/* the connection device to be added */
{

	bzero(cudepp,sizeof(struct CuDep));
	strncpy(cudepp->name,dvname,DEVNAMESIZE);
	strcpy(cudepp->dependency,depname);
	if (firstcudep) {
		cudephdl = (long *)odm_open_class(CuDep_CLASS);
		if ((int)cudephdl == -1) 
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
	}
	firstcudep = FALSE;
	if (odm_add_obj(CuDep_CLASS,cudepp) == -1) 
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);	
	}
}
/*
 * NAME: ck_field_ln     
 *
 * DESCRIPTION: Checks the length of a field and concatenates
 *	if it is greater than FIELDLENGTH.
 *		
 * INPUT: The field string.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: None.
 */
void
ck_field_ln(field,fieldlength)
char *field;
int fieldlength;
{
int len;
char temp[LVCHARSIZE];

	len = strlen(field);
	if (len >= fieldlength) {
		strncpy(temp,field,fieldlength - 1);
		bzero(field,len);
		strncpy(field,temp,fieldlength - 1);
	}
}

/*
 * NAME: addlogvol     
 *
 * DESCRIPTION: Adds a logical volume to the CuDv class, adds the 
 *	logical volume attributes to the CuAt class and add
 *	the dependency information to the CuDep class.
 *		
 * INPUT: The logical volume name, the minor number, the volume group
 *	name, and the major number.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  No error found.               
 *			 2:  If ODM error.            
 */
void
addlogvol(cudvp,cuatp,cudepp,lvname,vgname)
struct CuDv *cudvp;	/* Customized Device class pointer */
struct CuAt *cuatp;	/* Customized Attribute class pointer */
struct CuDep *cudepp;	/* Customized Dependency class pointer */
char *lvname;
char *vgname;
{

		/* add an instance to the Customized Device class */
		addcudv(cudvp,lvname,TRUE,vgname);

		/* add Customized Dependency instance */	
		addcudep(cudepp,vgname,lvname);
        
		/* add the serial id information for the logical volume */
		addcuat(cuatp,lvname,LVSERIAL_ID,LVUNIQUETYPE,TRUE,*idptr); 

		/* add each attribute for a logical volume to the CuAt class */ 
		/* and set the global flag to FALSE so it will not be */
		/* done twice */
		addcuat(cuatp,lvname,INTRA,LVUNIQUETYPE,a_flag,a_value); 
		a_flag = FALSE;
       
		addcuat(cuatp,lvname,COPIES,LVUNIQUETYPE,c_flag,c_value); 
		c_flag = FALSE;
      	 
		addcuat(cuatp,lvname,INTER,LVUNIQUETYPE,e_flag,e_value); 
		e_flag = FALSE;
       	 
		addcuat(cuatp,lvname,RELOCATABLE,LVUNIQUETYPE,r_flag,r_value);
		r_flag = FALSE;
      	 
		addcuat(cuatp,lvname,STRIPE_WIDTH,LVUNIQUETYPE,O_flag,O_value); 
		O_flag = FALSE;
      
		addcuat(cuatp,lvname,STRICTNESS,LVUNIQUETYPE,s_flag,s_value); 
		s_flag = FALSE;

		addcuat(cuatp,lvname,STRIPE_SIZE,LVUNIQUETYPE,S_flag,S_value); 
		S_flag = FALSE;
      
		ck_field_ln(t_value,TYPELENGTH);
		addcuat(cuatp,lvname,TYPEA,LVUNIQUETYPE,t_flag,t_value); 
		t_flag = FALSE;
       	 
		addcuat(cuatp,lvname,UPPERBOUND,LVUNIQUETYPE,u_flag,u_value); 
		u_flag = FALSE;
	 
		addcuat(cuatp,lvname,COPYFLAG,LVUNIQUETYPE,y_flag,y_value); 
		y_flag = FALSE;
       	 
		addcuat(cuatp,lvname,SIZE,LVUNIQUETYPE,z_flag,z_value); 
		z_flag = FALSE;
       	
		ck_field_ln(B_value,LABELLENGTH);
		addcuat(cuatp,lvname,LABEL,LVUNIQUETYPE,B_flag,B_value); 
		B_flag = FALSE;
       	
}
/*
 * NAME: addvolgrp     
 *
 * DESCRIPTION: Adds a volume group to the ODM config database.  Adds one
 *	instance to the CuDv class, and adds volume group attributes
 *	to the CuAt class.  
 *		
 * INPUT: The volume group name and the minor number.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  No error found.               
 *			 2:  If ODM error.            
 */
void
addvolgrp(cudvp,cuatp,vgname)
struct CuDv *cudvp;	/* Customized Device class pointer */
struct CuAt *cuatp;	/* Customized Attribute class pointer */
char *vgname;
{

		/* add a customized device instance */
		addcudv(cudvp,vgname,FALSE,vgname);

		addcuat(cuatp,vgname,VGSERIAL_ID,VGUNIQUETYPE,TRUE,*idptr);

		/* add the attributes for a volume group to the CuAt class */
		addcuat(cuatp,vgname,LOCK,VGUNIQUETYPE,k_flag,"y");
		k_flag = FALSE;

		addcuat(cuatp,vgname,STATE,VGUNIQUETYPE,q_flag,q_value);
		q_flag = FALSE;

		addcuat(cuatp,vgname,AUTO_ON,VGUNIQUETYPE,o_flag,o_value);
		o_flag = FALSE;

}

/*
 * NAME: rmlogvol     
 *
 * DESCRIPTION: Removes logical volume information from ODM including
 *	information in CuDv class, CuAt class and CuDep class.
 *		
 * INPUT: The logical volume name.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 2:  If ODM error.            
 *			 0:  No error found.               
 */
void
rmlogvol(lvname)
char *lvname;
{
char crit[CRITSIZE];			/* criteria for search of class in get list */
int status;

		/* remove each attribute from the CuAt class */ 
       		sprintf(crit,"name = '%s'",lvname);
		status = odm_rm_obj(CuAt_CLASS,crit);
		if((int)status == -1)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
		/* remove the instance from the Customized Device class */
       		sprintf(crit,"name = '%s'",lvname);
		status = odm_rm_obj(CuDv_CLASS,crit);
		if((int)status == -1)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
		/* remove the instance from the System Connection class */
       		sprintf(crit,"dependency = '%s'",lvname);
		status = odm_rm_obj(CuDep_CLASS,crit);
		if((int)status == -1)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
}
/*
 * NAME: rmvolgrp     
 *
 * DESCRIPTION: Removes a volume group from ODM.  Removes information
 *	from the CuDv class, and CuAt class.
 *		
 * INPUT: The volume group name.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 2:  If ODM error.            
 *			 0:  No error found.               
 */
void
rmvolgrp(vgname)
char *vgname;
{
char crit[CRITSIZE];			/* criteria for search of class in get list */
int status;

	/* remove volume group instance attributes from CuAt class */
       	sprintf(crit,"name = '%s'",vgname);
	status = odm_rm_obj(CuAt_CLASS,crit);
	if((int)status == -1)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	/* remove volume group instance from CuDv class */
       	sprintf(crit,"name = '%s'",vgname);
	status = odm_rm_obj(CuDv_CLASS,crit);
	if((int)status == -1)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
}		

/*
 * NAME: chng_attr     
 *
 * DESCRIPTION: Changes a specified attribute value to the value
 *	specified.  The CuAt class is accessed and updated.
 *		
 * INPUT: The device name (logical volume or volume group), the 
 *	attribute name to update, and the value to change to.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 2:  If ODM error.            
 *			 3:  If attribute not found.
 */
void
chng_attr(cuatp,dvname,attribute,attrval) 
struct CuAt *cuatp;	/* Customized Attribute class pointer */
char *dvname;
char *attribute;
char *attrval;
{
struct listinfo stat_info;	/* returns values from odm_get_list */
char crit[CRITSIZE];			/* criteria for search of class in get list */
int temp = 1;

	cuatp = getattr(dvname,attribute,FALSE,&temp);

	if ((int)cuatp == ODMERROR) 
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
        
	if (cuatp != NULL) {
		strcpy(cuatp->value,attrval);
		if (putattr(cuatp) == ODMERROR)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
	}      
	else {
		fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,attribute);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}
}

/*
 * NAME: chng_lvatnm     
 *
 * DESCRIPTION: Changes the logical volume name in the CuAt class for a
 *	specified attribute.  The CuAt class is accessed and 
 *	updated. If the errflag is set to TRUE then an error
 *	message will be printed to standard error if the
 *	attribute is not found, otherwise nothing will happen.
 *		
 * INPUT: The old logical volume name, the attribute name, the new
 *	logical volume name and the error flag for printing an error message.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 2:  If ODM error.            
 *			 3:  Attribute not found and errflag set.
 */
void
chng_lvatnm(cuatp,lvname,attribute,newlvnm,errflag)
struct CuAt *cuatp;	/* Customized Attribute class pointer */
char *lvname;
char *attribute;
char *newlvnm;
int errflag;
{
char crit[CRITSIZE];			/* criteria for search of class in get list */
char newlvname[DBNAMESIZE];	/* string to hold logical volume name */
int status;
int temp = 1;

	bzero(newlvname,DBNAMESIZE);
	strncpy(newlvname,newlvnm,DEVNAMESIZE);

	cuatp = getattr(lvname,attribute,FALSE,&temp);
	if ((int)cuatp == -1) 
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (cuatp != NULL) {
		sprintf(crit,"name = '%s' and attribute = '%s'",lvname,attribute);
		status = odm_rm_obj(CuAt_CLASS,crit);
		if((int)status == -1)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
		strcpy(cuatp->name,newlvname);
		if (putattr(cuatp) == ODMERROR)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
	}      
	else if (errflag) {
		fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,attribute);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}
}

/*
 * NAME: chng_lvnm     
 *
 * DESCRIPTION: Changes the logical volume name in ODM.  Changes the name in
 *	the CuDv class, the CuDep class and CuAt class.
 *	Also releases the sequence number for the prefix of the
 *	old logical volume name.
 *		
 * INPUT: The old logical volume name and the new logical volume name.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 2:  If ODM error.            
 *			 3:  Attribute not found.
 */
void
chng_lvnm(cudvp,cuatp,cudepp,lvname,newlvnm) 
struct CuDv *cudvp;	/* Customized Device class pointer */
struct CuAt *cuatp;	/* Customized Attribute class pointer */
struct CuDep *cudepp;	/* Customized Dependency class pointer */
char *lvname;
char *newlvnm;
{
struct listinfo stat_info;	/* returns values from odm_get_list */
char crit[CRITSIZE];			/* criteria for search of class in get list */
int *rc;
int rel = 0;
char newlvname[DBNAMESIZE];	/* string to hold new logical volume name */
int *minor_num;
int major_num;
int status;
int temp;

	bzero(newlvname,DBNAMESIZE);
	strncpy(newlvname,newlvnm,DEVNAMESIZE);
       	sprintf(crit,"name = '%s' and PdDvLn = '%s'",lvname,LVUNIQUETYPE);
	cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,1,1);
	if ((int)cudvp == -1) 
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (cudvp != NULL) {
		/* add the new lvname to the CuDv class so it will be a */
		/* valid name for getattr and putattr to work with */
		strcpy(cudvp->name,newlvname);
		if (odm_add_obj(CuDv_CLASS,cudvp) == -1) 
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);	
		}

		/* change each attribute to the new logical volume name */
		chng_lvatnm(cuatp,lvname,LVSERIAL_ID,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,INTRA,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,COPIES,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,INTER,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,RELOCATABLE,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,TYPEA,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,COPYFLAG,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,STRICTNESS,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,STRIPE_WIDTH,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,STRIPE_SIZE,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,UPPERBOUND,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,SIZE,newlvname,TRUE);
		chng_lvatnm(cuatp,lvname,LABEL,newlvname,TRUE);

		/* call genmajor to get the major number */
		/* if genmajor fails then an ODM error has occured and */
		/* a message will be printed and return code set to FAILURE */
		major_num = genmajor(cudvp->parent);

		if (major_num == ODMERROR) {
			fprintf(stderr,lvm_msg(GENMAJ_ERROR),Prog);
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
		else {
			temp = 1;
			minor_num = getminor(major_num,&temp,lvname);
			if (major_num == ODMERROR) {
				fprintf(stderr,lvm_msg(GEN_MIN_FAIL),Prog);
				odm_unlock(lock_word);
				odm_error(odmerrno);
			}
			rel = reldevno(lvname,FALSE);
			if (rel == ODMERROR) {
				fprintf(stderr,lvm_msg(REL_MIN_FAIL),
					Prog,lvname);
				odm_unlock(lock_word);
				exit(ODMACCESSERROR);
			}
			else {
				rc = genminor(newlvname,major_num,
					*minor_num,1,1,1);
				if (rc == NULL) {
				fprintf(stderr,lvm_msg(GEN_MIN_FAIL),
				    Prog,newlvname);
				odm_unlock(lock_word);
				exit(ODMACCESSERROR);
				}
			}
		}
		/* remove the old name from the CuDv class */
       		sprintf(crit,"name = '%s' and PdDvLn = '%s'",lvname,LVUNIQUETYPE);
		status = odm_rm_obj(CuDv_CLASS,crit);
		if((int)status == -1)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
       		sprintf(crit,"dependency = '%s'",lvname);
		cudepp = (struct CuDep *)odm_get_list(CuDep_CLASS,crit,&stat_info,1,1);
		if ((int)cudepp == -1) 
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
		if (cudepp != NULL) {
			strcpy(cudepp->dependency,newlvname);
			odm_change_obj(CuDep_CLASS,cudepp);
		}      
		else {
			fprintf(stderr,lvm_msg(LV_NOT_FOUND),Prog,lvname);
			odm_unlock(lock_word);
			odm_terminate();
			exit(OBJNOTFOUND);     
		}
	}      
	else {
		fprintf(stderr,lvm_msg(LV_NOT_FOUND2),Prog,lvname);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}
}

/*
 * NAME: rm_pv     
 *
 * DESCRIPTION: Removes a physical volume from the CuAt class.
 *		
 * INPUT: The physical volume name.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 2:  If ODM error.            
 */
void
rm_pv(pvname)
char *pvname;
{	
	char		crit[CRITSIZE];
	struct CuAt	cuat;

	sprintf(crit,"name = '%s' and attribute = '%s'",pvname,"pvid");
	if((int)odm_get_first(CuAt_CLASS,crit,&cuat) > 0)
	{
    		sprintf(crit,"attribute = '%s' and value = '%s'","pv",cuat.value);
		if (odm_rm_obj(CuAt_CLASS,crit) == -1)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
    	}
	else
	{
		fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,pvname);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}
}

/*
 * NAME: check_lvid     
 *
 * DESCRIPTION: Checks a logical volume identifier to insure it is
 *	in the correct form. 
 *		
 * INPUT: Uses the global varible idptr for the logical volume id.
 *
 * OUTPUT: Puts the vgid into the variable vgid.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 1:  If illegal identifier.
 *			 2:  If ODM error.            
 */
void
check_lvid(vgid)
char *vgid;
{
struct uniqueid vg_id;
int min_num;

/* parse volume group id and minor number of id. They must be seperated    */
/* by a period - so key off the period for parsing the values into tokens  */
	if (parsepair(*idptr,DECIMAL,0,32,&vg_id,&min_num) != 0) {
		fprintf(stderr,lvm_msg(BADLVID),Prog);
		odm_unlock(lock_word);
		odm_terminate();
		exit(ILLEGALSYNTAX);
	}
	sprintf(vgid,"%.8x%.8x",vg_id.word1,vg_id.word2);
}

/*
 * NAME: get_lvinfo     
 *
 * DESCRIPTION: Gets vgid and vgname for the logical volume. 
 *		
 * INPUT: Uses the global variable idptr for the logical volume identifier.
 *
 * OUTPUT: The volume group name (vgname).
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 2:  If ODM error.            
 *			 3:  Object not found.
 */
void
get_lvinfo(vgname)
char *vgname;	/* Storage for lv parent, the vg (OUTPUT) */
{
struct CuDv *cudvp; 
struct CuAt *cuatp;
struct listinfo stat_info;	/* returns values from odm_get_list */
char crit[CRITSIZE];			/* criteria for search of class in get list */
char vgid[17];			/* string to hold serial_id */


	check_lvid(vgid);

	/* Get the volume group name for this */
	/* logical volume to add to CuDep Class */ 
     	sprintf(crit,"attribute = '%s' and value = '%s'",VGSERIAL_ID,vgid);
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	if ((int)cuatp == -1) 
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (cuatp != NULL) 
       		strcpy(vgname,cuatp->name);
	else {
		fprintf(stderr,lvm_msg(VG_NOT_FOUND),Prog,vgid);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}
}

/*
 * NAME: get_dvname     
 *
 * DESCRIPTION: Gets a specified device name given the device identifier
 *	and device type (either logical volume, volume group,
 *	or physical volume.  Gets the name from the CuAt class.
 *		
 * INPUT: The device identifier and the type.
 *
 * OUTPUT: The device name in dvname.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 2:  If ODM error.            
 *			 3:  Attribute not found and errflag set.
 */
void
get_dvname(dvident,idtype,dvname)
char *dvident;		/* device identifier (INPUT) */
char *idtype;		/* the type (lv, pv or vg) (INPUT) */
char *dvname;		/* the device name (OUTPUT) */
{
struct CuAt *cuatp;
struct listinfo stat_info;	/* returns values from odm_get_list */
char crit[CRITSIZE];			/* criteria for search of class in get list */

	sprintf(crit,"attribute = '%s' and value = '%s'",idtype,dvident);
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	if ((int)cuatp == -1) 
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (cuatp == NULL) {
		fprintf(stderr,lvm_msg(ID_NOT_FOUND),Prog,dvident);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}
	sprintf(dvname,"%s",cuatp->name);
}
/*
 * NAME: check_vg_lock     
 *
 * DESCRIPTION: Checks the volume group lock in the CuAt class.  If the
 *	lock is set to 'n' (unlocked) then it sets it to 'y' (locked).
 *	Otherwise it returns a TRUE value to indicate the volume
 *	group is still locked. 
 *		
 * INPUT: A pointer to the CuAt structure to get the lock with, and the
 *	volume group name.
 *
 * OUTPUT: An ODM error message from odm_error if an error occurs.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  The volume group has been locked by the calling
 *			     process.
 *			 1:  The lock is still set.
 *			 2:  If ODM error.            
 *			 3:  Attribute not found and errflag set.
 */
int
check_vg_lock(cuatp,vgname)
struct CuAt *cuatp;
char *vgname;
{
int temp = 1;

	cuatp = getattr(vgname,LOCK,FALSE,&temp);

	if ((int)cuatp == -1) 
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
        
	if (cuatp != NULL) {
		if (cuatp->value[0] == 'n') { 
			strcpy(cuatp->value,"y");
			if (putattr(cuatp) == ODMERROR)
			{
				odm_unlock(lock_word);
				odm_error(odmerrno);
			}
			return(FALSE);
		}
		else
			return(TRUE);
	}      
	else {
		fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,LOCK);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}

}
/*
 * NAME: lock_vg     
 *
 * DESCRIPTION: Locks a volume group using an ODM lock.  The process calls
 *	odm_lock and tries to get the lvm_lock for the database.
 *	If the process succeeds then it will hold the lock and look
 * 	at the LOCK attribute for the specified volume group.  If it
 *	is set to 'n' then it will set it to 'y' and release 
 *	the lvm_lock.
 *	Otherwise it will wait a specified time and check the vg_lock
 *	a specified number of times before failing.  
 *	The lvm_lock will always be released at the end of the routine.
 *		
 * INPUT: A pointer to the structure for the lock attribute, and 
 *	  the volume group 
 *	  name to lock.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 1:  If lock is not obtained.  
 *			 2:  If ODM error.            
 *			 3:  Attribute not found and errflag set.
 */
void
lock_vg(cuatp,vgname)
struct CuAt *cuatp;
char *vgname;
{
int getlockid;
int i = 0;
int rc = 0;
int LOCKED = TRUE;

	LOCKED = check_vg_lock(cuatp,vgname);
	/* if the vg_lock was not obtained exit with 1 */
	if (LOCKED) {
		fprintf(stderr,lvm_msg(VG_LOCKED),Prog,vgname);
		rc = 1;
	}

	odm_unlock(lock_word);
	exit(rc);
			
}
/*
 * NAME: unlock_vg     
 *
 * DESCRIPTION: Unlocks a volume group by changing the value of the LOCK
 *	attribute in the CuAt class to 'n'.  The lvm_lock is not 
 *	called because another process might be holding it waiting 
 *	for the lock and the unlocking process would not be able
 *	to obtain it.
 *		
 * INPUT:A pointer to the structure for the lock attribute,
 *	 and the volume group 
 *	 name to unlock.
 *
 * OUTPUT: None.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 2:  If ODM error.            
 *			 3:  Attribute not found and errflag set.
 */
void
unlock_vg(cuatp,vgname)
struct CuAt *cuatp;
char *vgname;
{
	chng_attr(cuatp,vgname,LOCK,"n");
} 

/*
 * NAME: main         
 *
 * DESCRIPTION: Reads command line data and writes it to the appropriate 
 *	ODM data class fields.  The command line options specify 
 *	what information is written to ODM.
 *		
 * INPUT: The command line.                                                  
 *
 * OUTPUT: Error messages to stderr. 
 *
 * RETURN VALUE DESCRIPTION: 
 *			 3:  I/O Error.
 *			 2:  If ODM error.            
 *			 1:  Illegal syntax.
 *			 0:  No error found.               
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
struct CuDv *cudvp;
struct CuAt *cuatp;		/* customized device pointer */
struct CuDep *cudepp;

char dvname[DBNAMESIZE];		/* string to hold device name */
char lvname[DBNAMESIZE];		/* string to hold logical volume name */
char vgname[DBNAMESIZE];		/* string to hold volume group name */
char pvname[DBNAMESIZE];		/* string to hold volume group name */
char pvid_32[CUPVPVID];
int retval = 0;			/* temporary return variable */

	/* Initialize Prog */
	Prog = argv[0];

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	/* ignore these signals */
	memset(&action, '\0', sizeof (struct sigaction));
	action.sa_handler = SIG_IGN;
	sigaction (SIGQUIT, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGINT, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGTERM, &action, &oldaction);
	action.sa_handler = SIG_IGN;
	sigaction (SIGHUP, &action, &oldaction);

	/* idptr will be pointer to command line argument */ 
        idptr = (char **) getmem(argc * sizeof(char *));
  
	/* setflg sets command option flags and command option values. */
	/* If error then exit. */
        /* Get lvid, pvid, or vgid from standard input */
        /* and store them in input structure */     
 
	if ((retval = setflg(argc,argv,idptr)) < 0) exit(ILLEGALSYNTAX);

	/* ck_required confirms required parameters were supplied */
	if (ck_required()) {	

			exit(ILLEGALSYNTAX);
	}


	cudvp = (struct CuDv *) getmem(sizeof(struct CuDv));
	cuatp = (struct CuAt *) getmem(sizeof(struct CuAt));
	cudepp = (struct CuDep *) getmem(sizeof(struct CuDep));

	bzero(dvname,DBNAMESIZE);
	bzero(lvname,DBNAMESIZE);
	bzero(vgname,DBNAMESIZE);
	bzero(pvname,DBNAMESIZE);
	firstcuat = TRUE;
	firstcudv = TRUE;
	firstcudep = TRUE;

	if (odm_initialize() == -1) 
		odm_error(odmerrno);

	/* set the config database path for the classes */
	odm_set_path(CFGPATH);

	lock_word = odm_lock("/etc/objrepos/lvm_lock",ODM_WAIT);
	if ((a_flag || B_flag || c_flag || e_flag || L_flag || n_flag 
		|| r_flag || O_flag || s_flag || S_flag || t_flag || 
		u_flag || y_flag || z_flag) && !l_flag) {
 			/* get the logical volume name for the lvid */ 
			get_dvname(*idptr,LVSERIAL_ID,lvname);
		}

	if ((Q_flag || V_flag || k_flag || K_flag || o_flag || q_flag) && !(v_flag)) {
		/* the volume group name for these flags */
		get_dvname(*idptr,VGSERIAL_ID,vgname);
	}
	
	/* If 'add a new logical volume' flag */ 
	if (l_flag) {

		get_lvinfo(vgname);
		strncpy(lvname,l_value,DEVNAMESIZE);
		addlogvol(cudvp,cuatp,cudepp,lvname,vgname);
       	
	}
	
	/* If 'add a new volume group' flag */ 
	if (v_flag) {	
		strncpy(vgname,v_value,DEVNAMESIZE);
		addvolgrp(cudvp,cuatp,vgname);

	}

	/* add a new physical volume to volume group idptr */
	if (p_flag) {
		get_dvname(p_value,VGSERIAL_ID,vgname);
		convt_pvid_32(*idptr,pvid_32);
		addcuat(cuatp,vgname,"pv",VGUNIQUETYPE,TRUE,pvid_32);
	}

	/* remove a logical volume from ODM */
	if (L_flag) {
		rmlogvol(lvname);		
	}

	/* this flag will also change with access to the */
	/* physical volume object classes */
	/* remove a physical volume from ODM */
	if (P_flag) {
		get_pvname(*idptr,pvname);
		rm_pv(pvname);
	}

	/* remove a volume group from ODM */
	if (V_flag) {	
		rmvolgrp(vgname);
	}
         
	/* update the intra-policy attribute */
	if (a_flag) {		
		chng_attr(cuatp,lvname,INTRA,a_value);
	}

	/* update the label attribute */
	if (B_flag) {		
		ck_field_ln(B_value,LABELLENGTH);
		chng_attr(cuatp,lvname,LABEL,B_value);
	}

	/* update the inter-policy attribute */
	if (e_flag) {		
		chng_attr(cuatp,lvname,INTER,e_value);
	}

	/* update the copies attribute */
	if (c_flag) {		
		chng_attr(cuatp,lvname,COPIES,c_value);
	}

	/* lock the volume group */
	if (k_flag) {		
		lock_vg(cuatp,vgname);
	}

	/* unlock the volume group */
	if (K_flag) {		
		unlock_vg(cuatp,vgname);
	}

	/* change the lvname to newlvname */
	if (n_flag) {     
		chng_lvnm(cudvp,cuatp,cudepp,lvname,n_value);
	}

	/* update the relocatable attribute */
	if (r_flag) {		
		chng_attr(cuatp,lvname,RELOCATABLE,r_value);
	}

	/* update the stripe width */
	if (O_flag) {		
		chng_attr(cuatp,lvname,STRIPE_WIDTH,O_value);
	}
	/* update the strictness policy attribute */
	if (s_flag) {		
		chng_attr(cuatp,lvname,STRICTNESS,s_value);
	}

	/* update the stripe block in exponent */
	if (S_flag) {		
		chng_attr(cuatp,lvname,STRIPE_SIZE,S_value);
	}

	/* update the copyflag attribute */
	if (y_flag) {		
		chng_attr(cuatp,lvname,COPYFLAG,y_value);
	}

	/* update the type attribute */
	if (t_flag) {		
		ck_field_ln(t_value,TYPELENGTH);
		chng_attr(cuatp,lvname,TYPEA,t_value);
	}

	/* update the upperbound attribute */
	if (u_flag) {		
		chng_attr(cuatp,lvname,UPPERBOUND,u_value);
	}

	/* update the size attribute */
	if (z_flag) {		
		chng_attr(cuatp,lvname,SIZE,z_value);
	}

	/* update the auto-on flag */
	if (o_flag) {		
		chng_attr(cuatp,vgname,AUTO_ON,o_value);
	}

	/* update the state flag */
	if (q_flag) {		
		chng_attr(cuatp,vgname,STATE,q_value);
	}
	
	if (Q_flag) {
		addcuat(cuatp,vgname,"quorum",VGUNIQUETYPE,Q_flag,Q_value);
	}

      	/* exit gracefully from ODM */ 
	odm_unlock(lock_word);
	odm_terminate();
	
	exit(SUCCESS);
}
