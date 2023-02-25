static char sccsid[] = "@(#)30	1.36.2.8  src/bos/usr/sbin/lvm/intercmd/getlvodm.c, cmdlvm, bos41J, bai15 3/30/95 08:45:11";

/*
 * COMPONENT_NAME:(cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	getflg,subcnt,subcmp,check_desc,get_nm_id,
 *	get_id_nm,get_lvattr,get_attr,get_lvnm_lvdesc,get_vgnm_lvid,
 *	get_allochar_lvid,get_majnum_vgdesc,get_lvnm_lvid,get_pvnm_pvid,
 *	get_vgnms,get_vgid_pvdesc,,get_lvid_lvdesc,get_mtpt_lvid,
 *	get_pvid_pvdesc,get_reloc_lvid,get_vgst_vgdesc,
 *	get_vgnm_vgid,get_auto_on_vgdesc,get_vgid_vgdesc,get_pvids_vgdesc,
 *	get_type_lvid,get_lvs_vgdesc,get_label_lvdesc,get_confpvs,get_freepv,
 *	get_all_pvs,get_stripe_size,get_stripe_width,main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * FILE NAME: getlvodm  
 *
 * FILE DESCRIPTION: Source file for getlvodm command.  "getlvodm" provides an
 *	intermediate level of command interface.  "Getlvodm" gets volume group 
 *	and logical volume data from the Object Data Manager (ODM).
 *
 *	For additional information on this program and the other lvm immediate
 *	level command programs see the specification "Logical Volume Manager 
 *	Intermediate Level  Command Interface" and also the interface 
 *	documentation to the lvm library functions.
 *         
 * RETURN VALUE DESCRIPTION:
 *			0   Command Successfully completed
 *		        1   Illegal Syntax
 *			2   ODM Access Error
 *			3   Object not found in ODM 
 *
 * EXTERNAL PROCEDURES CALLED: setflg, getmem, odm_error, 
 *	odm_initialize, odm_get_list, get_config_pvs, 
 *      change_obj.
 *
 * EXTERNAL VARIABLES: optind, optarg, odmerrno.
 *
 * 
 * FUNCTION_NAMES: getflg, subcnt, subcmp, dvdesc_type, 
 * 	get_nm_id, get_id_nm, get_lvattr, get_attr, get_lvnm_lvdesc,
 *	get_vgnm_lvid, get_allochar_lvid, get_majnum_vgdesc,
 *	get_lvnm_lvid, get_pvnm_pvid, get_vgnms, get_vgid_pvdesc,
 *	get_km_id, get_lvid_lvdesc, get_mtpt_lvid, get_pvid_pvdesc,
 *	get_reloc_lvid, get_vgst_vgdesc, get_vgnm_vgid,
 *	get_auto_on_vgdesc, get_vgid_vgdesc, get_pvids_vgdesc, 
 *	get_type_lvid, get_lvs_vgdesc, get_all_pvs, get_stripe_size,
 *      get_stripe_width,main. 
 *
 */

#define _ILS_MACROS	/* performance enhancement for isxdigit(), etc calls */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <fstab.h>
#include "cmdlvm.h"
#include "cmdlvm_msg.h"		/* include files for message tests */
#include <ctype.h>

/*
 * EXTERNAL PROCEDURES CALLED:
 */
extern odm_error();
extern int odmerrno;
extern int setfsent();
extern int endfsent();
extern struct fstab *getfsent();
extern get_pvname();
extern convt_pvid_16();
extern convt_pvid_32();
extern int genmajor();
extern struct CuAt * getattr();

/* local library functions */
extern int setflg();
extern char * getmem();
void get_pvid_pvnm();

/*
 * GLOBAL VARIABLES:
 */

#define ODMERROR -1		/* error return code from cfg routines */
#define PVATTR "pvid"
#define ZEROPV "0000000000000000"
#define PVNONE "none"

/* external variables */
extern int optind;	/* index to next argv argument-incremented by getopt */
extern char *optarg;	/* pointer to command line argument string - */
			/* set by getopt  */
extern char *Prog;			/* points to name of the current Program */
extern nl_catd  scmc_catd;   		/* Cat descriptor for scmc conversion */

struct uniqueid {
	unsigned int word1;	/* the structure used to verify identifier */
	unsigned int word2;	/* is a valid 16 digit hex number */
	}; 

struct pv_info {
	char pvname[DBNAMESIZE];
	char pvid[LVMPVID];
	char vgname[DBNAMESIZE];
};

struct all_pvs {
	char pvinfo[80];
};

typedef enum { ident, name } dvdesc_type ;

/* Pointer to command line parameters associated  with each command line */
/* option.  These variables only contain values if corresponding flag is */
/* true - see below. */
static char *a_value;
static char *b_value;
static char *B_value;
static char *c_value;
static char *d_value;
static char *e_value;
static char *g_value;
static char *G_value;
static char *j_value;
static char *l_value;
static char *L_value;
static char *m_value;
static char *N_value;
static char *p_value;
static char *r_value;
static char *s_value;
static char *t_value;
static char *u_value;
static char *v_value;
static char *w_value;
static char *y_value;
static char *Q_value;
    

/* Each flag corresponds to a command option - */
/* flag true if option specified on command line */
static int a_flag;
static int b_flag;
static int B_flag;
static int c_flag;
static int C_flag;
static int d_flag;
static int e_flag;
static int F_flag;
static int g_flag;
static int G_flag;
static int h_flag;
static int j_flag;
static int l_flag;
static int L_flag;
static int m_flag;
static int N_flag;
static int p_flag;
static int P_flag;
static int r_flag;
static int R_flag;
static int s_flag;
static int S_flag;
static int t_flag;
static int u_flag;
static int v_flag;
static int w_flag;
static int y_flag;
static int Q_flag;
int lock_word;

/*
 * NAME: getflg
 *
 * FUNCTION: Parse command line options until the first non-option 
 *	is encountered.
 *
 * (NOTES:) For each option in the command line, a global flag 
 *	is set and a global
 *	value is loaded with any accompaning command argument.
 *
 * RETURN VALUE DESCRIPTION: Return negative value (-1) if illegal option found.
 *		Return positive value (1) if options parsed correctly.
 *		Return zero if no command options found.
 */

int getflg(argc,argv)

int argc;	 /* number of command line string in referenced by argv */
char **argv;	 /* array of command line strings */
{
	int rc; 		/* getopt return value */

	if (!argc) return(0);	/* if no command line options to parse */
				/* then return */

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv, 
		"a:b:B:c:Cd:e:Fg:G:hj:l:L:m:N:p:Pr:Rs:St:u:v:w:y:Q:")) != EOF) {
		switch (rc) {
		case 'a':
			a_flag = 1;
			a_value = optarg;
			break;
		case 'b':
			b_flag = 1;
			b_value = optarg;
			break;
		case 'B':
			B_flag = 1;
			B_value = optarg;
			break;
		case 'c':
			c_flag = 1;
			c_value = optarg;
			break;
		case 'C':
			C_flag = 1;
			break;
		case 'd':
			d_flag = 1;
			d_value = optarg;
			break;
		case 'e':
			e_flag = 1;
			e_value = optarg;
			break;
		case 'F':
			F_flag = 1;
			break;
		case 'g':
			g_flag = 1;
			g_value = optarg;
			break;
		case 'G':
			G_flag = 1;
			G_value = optarg;
			break;
		case 'h':
			h_flag = 1;
			break;
		case 'j':
			j_flag = 1;
			j_value = optarg;
			break;
		case 'l':
			l_flag = 1;
			l_value = optarg;
			break;
		case 'L':
			L_flag = 1;
			L_value = optarg;
			break;
		case 'm':
			m_flag = 1;
			m_value = optarg;
			break;
		case 'N':
			N_flag = 1;
			N_value = optarg;
			break;
		case 'p':
			p_flag = 1;
			p_value = optarg;
			break;
		case 'P':
			P_flag = 1;
			break;
		case 'r':
			r_flag = 1;
			r_value = optarg;
			break;
		case 'R':
			R_flag = 1;
			break;
		case 's':
			s_flag = 1;
			s_value = optarg;
			break;
		case 'S':
			S_flag = 1;
			break;
		case 't':
			t_flag = 1;
			t_value = optarg;
			break;
		case 'u':
			u_flag = 1;
			u_value = optarg;
			break;
		case 'v':
			v_flag = 1;
			v_value = optarg;
			break;
		case 'w':
			w_flag = 1;
			w_value = optarg;
			break;
		case 'y':
			y_flag = 1;
			y_value = optarg;
			break;
		case 'Q':
			Q_flag = 1;
			Q_value = optarg;
			break;
		default:
		case '?':
			fprintf(stderr,lvm_msg(GETODM_USE),Prog);
			return(-1);
		}
	}
	return(1);
}
/*
 * NAME: check_desc
 *
 * FUNCTION: Checks the descriptor to decide if it is a name or identifier. 
 *	If there is a decimal then it will be an identifier.  And if
 *	there is a non-hex character then it will be a name.
 *
 * RETURN VALUE DESCRIPTION: Will return the type in dvdesc.
 *		Will print an error message to standard error
 *		if it is not a name or an identifier.
 */

dvdesc_type 
check_desc(dvdesc)
char *dvdesc;	/* returns the descriptor type (OUTPUT) */
{
char *tempptr;
int id;

	/* default value for the dvdesc_type */
	id = TRUE;
	tempptr = dvdesc;
	/* parse the descriptor until the end and */
	/* test if any non-hex characters are found */
	while ((*tempptr != NULL) && (*tempptr != '.')){
		if (!isxdigit((int)*tempptr)) 
			id = 0;		/* then not id */
		tempptr++;
	}
	/* if id is still possible test the length */
	if (id) {
		if ((strlen(dvdesc) == VGIDLENGTH) 
				|| (strlen(dvdesc) == CUPVPVID))
			return(ident);     
		else if (*tempptr == '.') {
			if ((strlen(dvdesc) <= DEVNAMESIZE))
				return(name);
			else 
				return(ident);
		}
		else {
			if (strlen(dvdesc) <= DEVNAMESIZE) 
				return(name);
			else {
				fprintf(stderr,lvm_msg(BAD_VGID),Prog);
				odm_unlock(lock_word);
				odm_terminate();
				exit(ILLEGALSYNTAX);
			}
		}
	}
	 else
		return(name);
}

/*
 * NAME: get_nm_id 
 *
 * FUNCTION: Gets a logical volume name or a volume group name
 *	from the CuAt class using the identifier.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *		will print error message to standard error.
 *		Otherwise it will return the pointer to the
 *		structure found and will print the name to
 *		standard out if the prtflag is on.
 */
struct CuAt *
get_nm_id(dvid,prtflag,typeid)
char *dvid;		/* search for this id (INPUT) */
int prtflag;		/* if the prtflag is TRUE then print name */
char *typeid;		/* the type of device searching for */
			/* either lv vg  (INPUT) */
{
struct CuAt *cuatp;
struct listinfo stat_info;
char crit[CRITSIZE];
	
     	sprintf(crit,"attribute = '%s' and value = '%s'",typeid,dvid);
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	if ((int)cuatp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (cuatp == NULL) {
		fprintf(stderr,lvm_msg(ID_NOT_FOUND),Prog,dvid);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}
	/* if prtflag is set to TRUE then print to standard out */
	if (prtflag)
       		printf("%s\n",cuatp->name);           
       	return(cuatp);
}
/*
 * NAME: get_id_nm
 *
 * FUNCTION: Gets the lv or vg identifier for the name in the CuAt class.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *			     error message to standard error.
 *			     Otherwise it will print the name to 
 *                           standard out.
 */
void
get_id_nm(dvname,typeid)
char *dvname;		/* lv or vg name (INPUT) */
char *typeid;		/* type of device */
{
struct CuAt *cuatp;
int temp;
	
	cuatp = getattr(dvname,typeid,FALSE,&temp);
	/* if getattr returns ODMERROR then and odm error has occured */
	if ((int)cuatp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	/* if the pointer is not NULL then a match was found */
	if (cuatp != NULL) {
		printf("%s\n",cuatp->value);
	}
	/* else the object was not found, print error and exit */
	else {
		if (strcmp(typeid,VGSERIAL_ID) == 0) 
			fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,
			"",dvname);
		else	
			fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,
			"",dvname);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}
}

/*
 * NAME: get_lvattr
 *
 * FUNCTION: Gets a logical volume attribute from the CuAt class.
 *	Appends the attribute value onto the variable 
 *	lvchartstics to save for output later.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Does not return a value.
 */
void
get_lvattr(attr,lvchartstics,lvname)
char *attr;		/* attribute searching for (INPUT) */
char *lvchartstics;	/* (INPUT & OUTPUT) */
char *lvname;		/* logical volume name */
{
struct CuAt *cuatp;
int temp;

	cuatp = getattr(lvname,attr,FALSE,&temp);
	if ((int)cuatp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (cuatp != NULL) 
		sprintf(lvchartstics,"%s %s",lvchartstics,cuatp->value);
	else {
		fprintf(stderr,lvm_msg(CH_NOT_FOUND),Prog,lvname);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}
}

/*
 * NAME: get_attr
 *
 * FUNCTION: Gets an attribute from the CuAt class.
 *	Prints the attribute if found to standard out.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Does not return a value.
 */
void
get_attr(dvname,attribute)
char *dvname;
char *attribute;
{
struct CuAt *cuatp;
int temp;

	cuatp = getattr(dvname,attribute,FALSE,&temp);
	if ((int)cuatp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (cuatp != NULL) 
		printf("%s\n",cuatp->value);
	else {
		fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,attribute);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}		
}

/*
 * NAME: get_lvnm_lvdesc
 *
 * FUNCTION: Given a logical volume descriptor, this function will return a
 *	logical volume name.  It will determine if the descriptor is
 *	an identifier or a name, and act accordingly.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints name to standard out.
 */
void
get_lvnm_lvdesc(lvdesc)
char *lvdesc;
{
dvdesc_type lv_desc;	/* structure lvname or lvid */
char *tempptr;		/* temporary pointer */

	lv_desc = check_desc(lvdesc);

	switch (lv_desc) {

		case name:

       			printf("%s\n",lvdesc);           
			break;

		default:         /* this is a lvid */

			/* Get the logical volume name */
			/* for the logical volume id */
			get_nm_id(lvdesc,TRUE,LVSERIAL_ID);
		}
				
}

/*
 * NAME: get_vgnm_lvid
 *
 * FUNCTION: Gets the volume group name (or the parent) of the logical volume.
 *	First is gets the name from the CuAt class then the vgname.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints the vgname to standard out.
 */
void
get_vgnm_lvid(lvid)
char *lvid;
{
struct CuAt *cuatp;
struct CuDv *cudvp;
struct listinfo stat_info;
char crit[CRITSIZE];
char lvname[DBNAMESIZE];
	
	/* Get the volume group name for this logical volume id */ 
     	sprintf(crit,"attribute = '%s' and value = '%s'",LVSERIAL_ID,lvid);
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	if ((int)cuatp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (cuatp != NULL) 
       		sprintf(lvname,"%s",cuatp->name);           
	else {
		fprintf(stderr,lvm_msg(LV_NOT_FOUND),Prog,lvid);
		odm_unlock(lock_word);
		odm_terminate();	
		exit(OBJNOTFOUND);
	}

	/* after we have the name from the id then get the parent the vgname */
     	sprintf(crit,"name = '%s'",lvname);
	cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,1,1);
	if ((int)cudvp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (cudvp != NULL) 
       		printf("%s\n",cudvp->parent);           
	else {
		fprintf(stderr,lvm_msg(LV_NOT_FOUND2),Prog,lvname);
		odm_unlock(lock_word);
		odm_terminate();	
		exit(OBJNOTFOUND);
	}
}       	

/*
 * NAME: get_allochar_lvid
 *
 * FUNCTION: Gets the specified allocation characteristics for the logical 
 *	volume idenifier.  Puts them in a variable lvchartstics then
 *	prints is to standard out.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints lvchartstics to standard out.
 */
void
get_allochar_lvid(lvid)
char *lvid;
{
struct CuAt *cuatp;
char lvchartstics[LVCHARSIZE];

	bzero(lvchartstics,LVCHARSIZE);

	/* Get the logical volume name for the logical volume id */
	cuatp = get_nm_id(lvid,FALSE,LVSERIAL_ID);

	sprintf(lvchartstics,"");

	get_lvattr(TYPEA,lvchartstics,cuatp->name);

	get_lvattr(INTRA,lvchartstics,cuatp->name);

	get_lvattr(INTER,lvchartstics,cuatp->name);
	
	get_lvattr(UPPERBOUND,lvchartstics,cuatp->name);
	
	get_lvattr(STRICTNESS,lvchartstics,cuatp->name);
	
	get_lvattr(COPIES,lvchartstics,cuatp->name);

	get_lvattr(RELOCATABLE,lvchartstics,cuatp->name);

	get_lvattr(STRIPE_WIDTH,lvchartstics,cuatp->name);

	get_lvattr(STRIPE_SIZE,lvchartstics,cuatp->name);

	printf("%s\n",lvchartstics);
}

/*
 * NAME: get_majnum_vgdesc
 *
 * FUNCTION: Gets the major number from the CuDv class for 
 *	the volume group descriptor.
 *	The descriptor can be a name or identifier. Prints the major number
 *	to standard out.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints major number to standard out.
 */
void
get_majnum_vgdesc(vgdesc)
char *vgdesc;
{
struct CuAt *cuatp;
struct CuDv *cudvp;
struct listinfo stat_info;
char crit[CRITSIZE];
char vgname[DBNAMESIZE];
dvdesc_type vg_desc;
int majornum;

	vg_desc = check_desc(vgdesc);
	switch (vg_desc) {
		case name:
			/* if name then continue */	
			strcpy(vgname,vgdesc);
			break;
		default:
			/* Get the volume group name for this volume group id */
     			sprintf(crit,"attribute = '%s' and value = '%s'",
			VGSERIAL_ID,vgdesc);
			cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,
			crit,&stat_info,1,1);
			if ((int)cuatp == ODMERROR)
			{
				odm_unlock(lock_word);
				odm_error(odmerrno);
			}
			if (cuatp != NULL) 
       				sprintf(vgname,"%s",cuatp->name);           
			else {
				fprintf(stderr,lvm_msg(VG_NOT_FOUND),
				    Prog,vgdesc);
				odm_unlock(lock_word);
				odm_terminate();	
				exit(OBJNOTFOUND);
			}
	}

	/* call genmajor to get the major number */
	/* if genmajor fails then an ODM error has occured and */
	/* a message will be printed and return code set to FAILURE */
	majornum = genmajor(vgname);

	if (majornum == ODMERROR) {
		fprintf(stderr,lvm_msg(GENMAJ_ERROR),Prog);
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	else
		printf("%d\n",majornum);


}

/*
 * NAME: get_lvnm_lvid
 *
 * FUNCTION: Gets the logical volume name for the logical volume identifier.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 */
void
get_lvnm_lvid(lvid)
char *lvid;
{
	/* print the name to standard out if found */
	/* send the lv type to search for */
	get_nm_id(lvid,TRUE,LVSERIAL_ID);
} 

/*
 * NAME: get_pvnm_pvid
 *
 * FUNCTION: Gets the physical volume name for the physical volume identifier.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		If no error will print name to standard out.
 */
void
get_pvnm_pvid(pvid)
char *pvid;
{
char pvname[DBNAMESIZE]; 

	bzero(pvname,DBNAMESIZE);
	/* print the name to standard out if found */
	/* send the pv type to search for */
	get_pvname(pvid,pvname);
	printf("%s\n",pvname);

}

/*
 * NAME: get_vgnms
 *
 * FUNCTION: Gets all the volume groups in the CuDv class.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		If not volume groups are found it will print
 *		an error message to standard error.
 */
void
get_vgnms()
{
struct CuDv *cudvp;
struct listinfo stat_info;
char crit[CRITSIZE];
int num = 0;
	
	sprintf(crit,"PdDvLn = '%s'",VGUNIQUETYPE);
	cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,
		&stat_info,MAXNUMVGS,1);
	if ((int)cudvp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (!stat_info.num) {
		fprintf(stderr,lvm_msg(NO_VGS_NOW),Prog);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}
	else 
		/* stat_info.num is the number of objects that were found */
		num = stat_info.num;
		while(num > 0) {
			printf("%s \n",cudvp->name);
			cudvp++;
			num--;
		}

}

/*
 * NAME: get_vgid_pvdesc
 *
 * FUNCTION: Gets the volume group idenifier that is dependent on
 *	the physical volume descriptor (name or identifier).
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints the vgid to standard out.
 */
void
get_vgid_pvdesc(pvdesc)
char *pvdesc;
{
struct CuAt cuat;
dvdesc_type pv_desc;
char crit[CRITSIZE];
char pvid[CUPVPVID];
char vgname[DBNAMESIZE];

	/* is it a name or identifier */
	pv_desc = check_desc(pvdesc);

	switch (pv_desc) {
		/* if a pvident then set search crit */
		case ident:         
			    convt_pvid_32(pvdesc,pvid); 
     			sprintf(crit,"attribute = '%s' and value = '%s'", "pv",pvid);
				break;
		default:         /* this is a name and get id and set search crit */
			get_pvid_pvnm(pvdesc,pvid);
   			sprintf(crit,"attribute = '%s' and value = '%s'", "pv",pvid);
			break;
	}
	if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) <= 0)
	{
		fprintf(stderr,lvm_msg(PV_NOT_IN_VG),Prog,pvdesc);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}
	else {
		bzero(vgname,DBNAMESIZE);
		strncpy(vgname,cuat.name,DEVNAMESIZE);
		get_id_nm(vgname,VGSERIAL_ID);
	}

}

/*
 * NAME: get_lvid_lvdesc
 *
 * FUNCTION: Gets the lvid or either a lvname or lvid.
 *	If it is determined that the descriptor is an
 *	identifier then odm is not checked.  This is
 *	in case odm is down and the high level commands 
 *	must still be run.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints identifier to standard out.
 */
void
get_lvid_lvdesc(lvdesc)
char *lvdesc;		/* descriptor (INPUT) */
{ 
dvdesc_type lv_desc;
char *tempptr;
	
	/* determine if name or identifier */
	lv_desc = check_desc(lvdesc);

	switch (lv_desc) {
		case name:
			get_id_nm(lvdesc,LVSERIAL_ID);
			break;
		default:         /* this is a lvid */
       			printf("%s\n",lvdesc);           
		}
}
/*
 * NAME: get_lvs_vgdesc
 *
 * FUNCTION: Gets all the lvids and lvnames for the volume
 *	group descriptor. The volume group is checked
 *	to determine if it is an identifier or a name.
 *	If it is an identifier then the name is obtained
 *	from the CuAt class.  The name is then used as 
 *	the value for the parent field in the CuDv class
 *	and all the logical volume with that parent are
 *	obtained.  Then an identifier for each lvname is
 *	obtained from the CuAt class.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints identifiers and names to standard out.
 */
void
get_lvs_vgdesc(vgdesc)
char *vgdesc;		/* descriptor (INPUT) */
{ 
struct CuAt *cuatp;
struct CuDv *cudvp;
dvdesc_type vg_desc;
struct listinfo stat_info;
char crit[CRITSIZE];
static char lvdesc[DBNAMESIZE] = "";
char lvname[DBNAMESIZE];
int num; 
int temp;

	/* is it a name or identifier */
	vg_desc = check_desc(vgdesc);

	switch (vg_desc) {
		case name:
     			sprintf(crit,"parent = '%s'",vgdesc);
			break;
		default:         /* this is a ident */
			cuatp = get_nm_id(vgdesc,FALSE,VGSERIAL_ID);
     			sprintf(crit,"parent = '%s'",cuatp->name);
			break;
	}
	/* get the logical volumes for the vgname */
	cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,
		&stat_info,MAXNUMPVS,1);
	if ((int)cudvp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	/* if there were no physical volumes then print an error message */
	if (!stat_info.num) {
		/* if no logical volumes were found then exit with success */
		odm_unlock(lock_word);
		odm_terminate();
		exit(SUCCESS);
	}
	else {
		num = stat_info.num;
		while(num > 0) {
			bzero(lvname,DBNAMESIZE);
			strcpy(lvname,cudvp->name);
			cuatp = getattr(lvname,LVSERIAL_ID,FALSE,&temp);
			if ((int)cuatp == ODMERROR)
			{
				odm_unlock(lock_word);
				odm_error(odmerrno);
			}
			if (cuatp != NULL) {
				/* print out the lvname and lvid */
				sprintf(lvdesc,"%s %s",cudvp->name,
					cuatp->value);
				printf("%s\n",lvdesc);
				cudvp++;
				num--;
			}
		}
	}
	
}

/*
 * NAME: get_mtpt_lvid
 *
 * FUNCTION: Gets the mount point for the filesystem logical volume 
 *	from the /etc/filesystems file.  Opens the file with
 *	'setfsent', get the structure associated with the /dev
 *	entry, and closes the file with 'endfsent'.
 *
 * RETURN VALUE DESCRIPTION: Prints the mount point to standard out.
 *
 */
void
get_mtpt_lvid(lvid)
char *lvid;
{
struct fstab *fsstruct;
char tempdev[DBNAMESIZE];
struct CuAt *cuatp;

	strcpy(tempdev,"/dev/");
	cuatp = get_nm_id(lvid,FALSE,LVSERIAL_ID);
	strcat(tempdev,cuatp->name);
	setfsent();
	fsstruct = getfsspec(tempdev);
	if (fsstruct != NULL)
		printf("%s\n",fsstruct->fs_file);
	else
		printf("N/A\n");
	endfsent();

}
/*
 * NAME: get_pvid_pvnm     
 *
 * DESCRIPTION: Gets a specified physical volume id given the pv name.
 *	Gets the identifier from the CuAt class.
 *		
 * INPUT: The physical volume name.
 *
 * OUTPUT: The physical volume identifier in pvid.
 *
 * RETURN VALUE DESCRIPTION: 
 *			 0:  Successful.
 *			 2:  If ODM error.            
 *			 3:  Identifier not found and errflag set.
 */
void
get_pvid_pvnm(pvname,pvid)
char *pvname;		/* the device name (INPUT) */
char *pvid;		/* physical volume identifier (OUTPUT) */
{
struct CuAt *cuatp;
int temp;

	
	cuatp = getattr(pvname,PVATTR,FALSE,&temp);
	/* if getattr returns ODMERROR then and odm error has occured */
	if ((int)cuatp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	/* if the pointer is not NULL then a match was found */
	if (cuatp != NULL) {
		if((strncmp(cuatp->value,PVNONE,4)) == 0) 
			sprintf(pvid,"%s",ZEROPV);
		else
			sprintf(pvid,"%s",cuatp->value);
	}
	/* else the object was not found, print error and exit */
	else {
		fprintf(stderr,lvm_msg(DEV_NOT_FOUND),Prog,
			lvm_msg(PVINTRO),pvname);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}

}

/*
 * NAME: get_pvid_pvdesc
 *
 * FUNCTION: Gets the physical volume identifier for either the pvname or pvid.
 *	If it is determined the it is an id then if will not verify with
 *	odm, it will just print the id back.  This is in case odm is down.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints the identifier to standard out.
 */
void
get_pvid_pvdesc(pvdesc)
char *pvdesc;
{
dvdesc_type pv_desc;
char pvid[CUPVPVID];
char shortpvid[LVMPVID];

	/* is it a name or id */
	pv_desc = check_desc(pvdesc);

	switch (pv_desc) {
		case name:
			bzero(pvid,CUPVPVID);
			get_pvid_pvnm(pvdesc,pvid);
			convt_pvid_16(pvid,shortpvid);
			printf("%s\n",shortpvid);
			break;
		default:         /* this is a ident */
			printf("%s\n",pvdesc);
			break;
	}

}

/*
 * NAME: get_reloc_lvid
 *
 * FUNCTION: Gets the relocatable attribute for the logical volume identifier.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints the reloc value to standard out.
 */
void
get_reloc_lvid(lvid)
char *lvid;
{
struct CuAt *cuatp;
char lvname[DBNAMESIZE];

	/* first get the logical volume name */
	cuatp = get_nm_id(lvid,FALSE,LVSERIAL_ID);
	/* then get the relocatable attribute value and print */
	strcpy(lvname,cuatp->name);
	get_attr(lvname,RELOCATABLE);
}
/*
 * NAME: get_runtime
 *
 * FUNCTION: Gets the runtime attribute from the PdAt class to 
 *		tell if in runtime.
 *
 * RETURN VALUE DESCRIPTION: Does not return. 
 *		Returns OBJNOTFOUND if the attribute was not found.
 *		Returns SUCCESS if it was found.
 */
void
get_runtime()
{
struct PdAt *pdatp;
struct listinfo stat_info;
char crit[CRITSIZE];

     	sprintf(crit,PHASE1_DISALLOWED);
	pdatp = (struct PdAt *)odm_get_list(PdAt_CLASS,crit,&stat_info,1,1);
	if ((int)pdatp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	if (pdatp == NULL) {
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}
	else
	{
		odm_unlock(lock_word);
		exit(SUCCESS);
	}
}

/*
 * NAME: get_vgst_vgdesc
 *
 * FUNCTION: Gets the volume group state attribute for the volume group
 *	descriptor (name or identifier) and prints to standard out.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints the state attribute value to standard out.
 */
void
get_vgst_vgdesc(vgdesc)
char *vgdesc;		/* (INPUT) */
{
struct CuAt *cuatp;
dvdesc_type vg_desc;

	/* is it a name or identifier */
	vg_desc = check_desc(vgdesc);

	switch (vg_desc) {
		case name: 
			/* if name then get the volume group state */
			get_attr(vgdesc,STATE); 
			break;
		default:         /* this is a ident */
			cuatp = get_nm_id(vgdesc,FALSE,VGSERIAL_ID);
			get_attr(cuatp->name,STATE);
			break;
	}
}

/*
 * NAME: get_rwopt_class
 *
 * FUNCTION: Gets the physical volume class from odm and prints
 *	     to standard out.
 *
 * RETURN VALUE DESCRIPTION: Does not return if invalid and will print an
 *		error message to standard error.
 *		Prints the device class  to standard out.
 */
void
get_rwopt_class()
{
struct CuDv *cudvp;
struct listinfo stat_info;
int num = 0;
	
	cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,
		"PdDvLn like 'rwoptical*' and status = '1'",
		&stat_info,MAXNUMPVS,1);
	if ((int)cudvp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
		exit(FAILURE);
	}
	else
	{
		/* stat_info.num is the number of objects that were found */
		num = stat_info.num;
		while(num > 0) {
			printf("%s \n",cudvp->name);
			cudvp++;
			num--;
		}
	}
}
/*
 * NAME: get_vgnm_vgid 
 *
 * FUNCTION: Gets a volume group name from the CuAt using the volume
 *	group identifier.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *		will print error message to standard error.
 *		Will print the volume group name to standard
 *		out if found. 
 *
 */
void
get_vgnm_vgid(vgid)
char *vgid;
{
 
	get_nm_id(vgid,TRUE,VGSERIAL_ID);
}

/*
 * NAME: get_auto_on_vgdesc 
 *
 * FUNCTION: Gets a volume group attribute (auto_on) from the CuAt
 *	class and prints out the value to standard out.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *		will print error message to standard error.
 *		Will print the value of the auto_on attribute 
 *		to standard out if found.
 */
void
get_auto_on_vgdesc(vgdesc)
char *vgdesc;
{
struct CuAt *cuatp;
dvdesc_type vg_desc;

	/* is it a name or identifier? */
	vg_desc = check_desc(vgdesc);

	switch (vg_desc) {
		case name:
			/* if name then get auto_on attribute from CuAt class */
			get_attr(vgdesc,AUTO_ON); 
			break;
		default:         /* this is a ident */
			/* else get name from CuAt class and proceed */	
			cuatp = get_nm_id(vgdesc,FALSE,VGSERIAL_ID);
			get_attr(cuatp->name,AUTO_ON);
			break;
	}
}

/*
 * NAME: get_vgid_vgdesc 
 *
 * FUNCTION: Gets the volume group identifier for a volume group
 *	descriptor (name or identifier).  If an identifer is
 *	given as the descriptor then it will be automatically
 *	returned and ODM will not be checked in case ODM is down.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *		will print error message to standard error.
 *		Will print the vg identifier to standard out.
 */
void   
get_vgid_vgdesc(vgdesc)
char *vgdesc;
{
dvdesc_type vg_desc;

	/* is it a name or identifier? */
	vg_desc = check_desc(vgdesc);

	switch (vg_desc) {
		case name:
			/* if name then get id from CuAt class */	
			get_id_nm(vgdesc,VGSERIAL_ID);
			break;
		default:         /* this is a ident */
			/* else print id to standard out */	
			printf("%s\n",vgdesc);
			break;
	}
}

/*
 * NAME: get_pvids_vgdesc 
 *
 * FUNCTION: Gets all the physical volumes contained in a volume group.
 *	Searches the CuAt class to get all physical volumes with
 *  attribute "pv" of specified volume group name.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *	will print error message to standard error.
 *	Will print a list of physical volume identifiers
 *	(one on each line) to standard out.
 */
void
get_pvids_vgdesc(vgdesc)
char *vgdesc;
{
struct CuAt *cuatp, *temp;
struct listinfo stat_info;
char crit[CRITSIZE];
char vgname[DBNAMESIZE];
char *pvid;
char shortpvid[LVMPVID];
dvdesc_type vg_desc;
static char pvdesc[DBNAMESIZE] = "";
int num; 

	/* is it a name or identifier? */
	vg_desc = check_desc(vgdesc);

	switch (vg_desc) {
		case name:
			strcpy(vgname,vgdesc);
			break;
		default:         /* this is a ident */
			/* else get name from CuAt class and proceed */	
			cuatp = get_nm_id(vgdesc,FALSE,VGSERIAL_ID);
			strcpy(vgname,cuatp->name);
			break;
	}

     	sprintf(crit,"name = '%s' and attribute = '%s'",vgname,"pv");
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,
		crit,&stat_info,MAXNUMPVS,1);
	if ((int)cuatp == ODMERROR)
	{
		odm_unlock(lock_word);
		odm_error(odmerrno);
	}
	/* if there were no physical volumes, print an error message */
	if (!stat_info.num) {
		fprintf(stderr,lvm_msg(1022),Prog);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}
	else {
		num = stat_info.num;
		while(num > 0) {
				convt_pvid_16(cuatp->value,shortpvid);
				temp = get_nm_id(cuatp->value,FALSE,PVATTR);
				printf("%s %s\n",shortpvid,temp->name);
				cuatp++;
				num--;
		}
	}
}
/*
 * NAME: get_type_lvid 
 *
 * FUNCTION: Gets the type attribute value for the logical volume identifier.
 *	Gets the name from the CuAt class using the identifier then
 *	gets the type value from the CuAt class using the name.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *	will print error message to standard error.
 *	Will print the value of the type attribute for
 *	the logical volume identifier to standard out.
 */
void 
get_type_lvid(lvid)
char *lvid;
{
struct CuAt *cuatp;
char lvname[DBNAMESIZE];

	cuatp = get_nm_id(lvid,FALSE,LVSERIAL_ID);
	strcpy(lvname,cuatp->name);
	get_attr(lvname,TYPEA);
}
/*
 * NAME: get_label_lvdesc 
 *
 * FUNCTION: Gets the label attribute value for the logical volume descriptor.
 *	Gets the name from the CuAt class using the identifier then
 *	gets the type value from the CuAt class using the name.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *	will print error message to standard error.
 *	Will print the value of the type attribute for
 *	the logical volume identifier to standard out.
 */
void 
get_label_lvdesc(lvdesc)
char *lvdesc;		/* descriptor (INPUT) */
{ 
dvdesc_type lv_desc;
struct CuAt *cuatp;

	/* determine if name or identifier */
	lv_desc = check_desc(lvdesc);

	switch (lv_desc) {
		case name:
			get_attr(lvdesc,LABEL);
			break;
		default:         /* this is a lvid */
			cuatp = get_nm_id(lvdesc,FALSE,LVSERIAL_ID);
			get_attr(cuatp->name,LABEL);
		}

}


/*************************************************************************
*                                                                        *
* NAME: get_config_pvs                                                   *
*                                                                        *
* FUNCTION: Get a list of all configured physical volumes (names)        *
*           from ODM.                                                    *
*                                                                        *
* INPUT:    NONE                                                         *
*                                                                        *
* OUTPUT:                                                                *
*            pvinfo_ptr       A POINTER to a POINTER of character        *
*                             strings used to store the physical.        *
*                             volume names.                              *
*                                                                        *
*            num_of_pvs   An INTEGER used to store the number            *
*                             of configured physical volumes.            *
*                                                                        *
* RETURN VALUE DESCRIPTION:                                              *
*                          0 Function successfully completed             *
*                          2 ODM Access Error                            *
*                          3 Object not found in ODM                     *
*                          5 MALLOC Failure                              *
*                                                                        *
**************************************************************************/
int  
get_config_pvs (pvinfo_ptr,num_of_pvs)
struct pv_info **pvinfo_ptr;
int *num_of_pvs;
{
struct CuDv *cudvp, *curwopt;
struct CuAt *cuatp;
struct listinfo stat_info, rwoptstat_info;
char crit[CRITSIZE];
int index, rwindex;
int temp;
int num;

	/* Get a list of all configured physical volumes from the CuAt class */

	cudvp =  (struct CuDv *) odm_get_list(CuDv_CLASS,
					"PdDvLn like 'disk*' and status = '1'",
					 &stat_info, MAXNUMPVS, 1);
	if ((int)cudvp == -1) {
		odm_unlock(lock_word);
		odm_terminate();
		exit(FAILURE);
	}

	if (stat_info.num == 0) {
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}

	/* get read/write optical list */
	curwopt = (struct CuDv *)odm_get_list(CuDv_CLASS,
				"PdDvLn like 'rwoptical*' and status = '1'",
				&rwoptstat_info,MAXNUMPVS,1);
	if ((int)curwopt == ODMERROR) {
		odm_free_list (cudvp, &stat_info);
		odm_unlock(lock_word);
		odm_terminate();
		exit(FAILURE);
	}
	num = stat_info.num + rwoptstat_info.num;
	*pvinfo_ptr = (struct pv_info *) malloc
		(sizeof (struct pv_info) * num);

	if (*pvinfo_ptr == NULL) {
		odm_free_list (cudvp, &stat_info);
		odm_free_list (curwopt, &rwoptstat_info);
		return (MALLOC_FAILURE);
	}
	*num_of_pvs = 0;
	for (index = 0; index < stat_info.num ;index ++ ) {
		cuatp = getattr((cudvp+index)->name,
			PVATTR,FALSE,&temp);
		if ((int)cuatp == ODMERROR)
		{
			odm_unlock(lock_word);
			odm_error(odmerrno);
		}
		else if(cuatp != NULL) {
			bzero(*pvinfo_ptr+index,DBNAMESIZE);
			strcpy((*pvinfo_ptr+(*num_of_pvs))->pvname,
				(cudvp+index)->name);
			if((strncmp(cuatp->value,PVNONE,4)) == 0) 
				strncpy(
				(*pvinfo_ptr+(*num_of_pvs))->pvid,
				cuatp->value,4);
			else
				strncpy(
				(*pvinfo_ptr+(*num_of_pvs))->pvid,
				cuatp->value,32);
			++(*num_of_pvs);
		}
	} /* endfor */
	/* get read/write optical attribute */
	if (curwopt!=NULL)
	{
		for (rwindex = 0; rwindex < rwoptstat_info.num ;rwindex ++, index++ ) {
			cuatp = getattr((curwopt+rwindex)->name,
				PVATTR,FALSE,&temp);
			if ((int)cuatp == ODMERROR)
			{
				odm_unlock(lock_word);
				odm_error(odmerrno);
			}
			else if(cuatp != NULL) {
				bzero(*pvinfo_ptr+index,DBNAMESIZE);
				strcpy((*pvinfo_ptr+(*num_of_pvs))->pvname,
					(curwopt+rwindex)->name);
				if((strncmp(cuatp->value,PVNONE,4)) == 0) 
					strncpy(
					(*pvinfo_ptr+(*num_of_pvs))->pvid,
					cuatp->value,4);
				else
					strncpy(
					(*pvinfo_ptr+(*num_of_pvs))->pvid,
					cuatp->value,32);
					++(*num_of_pvs);
			}
		} /* endfor */
	}
	odm_free_list (cudvp, &stat_info);
	odm_free_list (curwopt, &rwoptstat_info);
}

/*
 * NAME: get_confpvs
 *
 * FUNCTION: Gets a list of all the configured physical volumes 
 *      from the CuDv class.  Gets the pvid from the CuAt class.
 *
 * RETURN VALUE DESCRIPTION: 
 *
 *
 *
 */
void 
get_confpvs()
{ 
struct pv_info *pvinfo_ptr;
int num_of_pvs;

	get_config_pvs(&pvinfo_ptr,&num_of_pvs);

	while (num_of_pvs >= 1) {
		if((strncmp((pvinfo_ptr)->pvid,PVNONE,4)) != 0) 
			printf("%s\n",(pvinfo_ptr)->pvname);
		++pvinfo_ptr;
		--num_of_pvs;
	}
}

/*
 * NAME: get_freepvs
 *
 * FUNCTION: Gets a list of all the configured physical volumes 
 *      from the CuDv class and then check the CuAt class to
 *	see if they are already in a volume group.  If not print
 *	the pvname.
 *
 * RETURN VALUE DESCRIPTION: Prints a list of free physical 
 *	volume names to standard out.
 *
 *
 */
void 
get_freepvs()
{ 
char crit[CRITSIZE];
struct CuAt cuat;
struct pv_info *pvinfo_ptr;
int num_of_pvs;
int available=FALSE;

	get_config_pvs(&pvinfo_ptr,&num_of_pvs);

	while (num_of_pvs >= 1) {
        sprintf(crit,"attribute = '%s' and value = '%s'","pv",pvinfo_ptr->pvid);
        if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) <= 0)
		    /* if the pv was not found then it is free */
		{
			available=TRUE;
			printf("%s\n",pvinfo_ptr->pvname);
		}
		++pvinfo_ptr;
		--num_of_pvs;
	}
	
	if (!available) {
		fprintf(stderr,lvm_msg(NO_FREE_PVS),Prog);
		odm_unlock(lock_word);
		odm_terminate();
		exit(OBJNOTFOUND);
	}
}

/*
 * NAME: get_all_pvs
 *
 * FUNCTION: Gets a list of all the physical volumes 
 *      from the CuDv class and lists them and 
 *	the PVID and the volume group it belongs to.
 *
 * RETURN VALUE DESCRIPTION: 
 *
 *
 *
 */
void 
get_all_pvs()
{ 
struct pv_info *pvinfo_ptr;
int num_of_pvs;
struct all_pvs *pv;
char crit[CRITSIZE];
struct CuAt cuat;
int index;
char pvid_16[LVMPVID];
char vgname[DBNAMESIZE];



	get_config_pvs(&pvinfo_ptr,&num_of_pvs);

	if (num_of_pvs == 0) {
		fprintf(stderr,lvm_msg(PVS_NOT_FOUND),Prog);
		odm_unlock(lock_word);
		odm_terminate();
		exit(FAILURE);
	}
	pv = (struct all_pvs *) malloc
		(sizeof (struct all_pvs) * num_of_pvs);

	if (pv == NULL) {
		fprintf(stderr,lvm_msg(ALLOCERR_LVM),Prog);
		odm_unlock(lock_word);
		odm_terminate();
		exit(MALLOC_FAILURE);
	}
	for (index = 0; index < num_of_pvs ;index++ ) {
   		sprintf(crit,"attribute = '%s' and value = '%s'","pv",(pvinfo_ptr+index)->pvid); 
		bzero(vgname,DBNAMESIZE);
		if ((int)odm_get_first(CuAt_CLASS,crit,&cuat) <= 0)
			strcpy(vgname,"None");
		else 
			strncpy(vgname,cuat.name,(int)DEVNAMESIZE);

		sprintf(pv+index,"%-15s%-20s%-15s",
			(pvinfo_ptr+index)->pvname,
			strncpy(pvid_16,(pvinfo_ptr+index)->pvid,16),
			vgname);

		printf("%s\n",pv+index);
	} 
}

/*
 * NAME: get_stripe_size 
 *
 * FUNCTION: Gets the stripe block in exponent  attribute value for 
 *      the logical volume descriptor.
 *	Gets the name from the CuAt class using the identifier then
 *	gets the type value from the CuAt class using the name.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *	will print error message to standard error.
 *	Will print the value of the type attribute for
 *	the logical volume identifier to standard out.
 */
void 
get_stripe_size(lvdesc)
char *lvdesc;		/* descriptor (INPUT) */
{ 
dvdesc_type lv_desc;
struct CuAt *cuatp;

	/* determine if name or identifier */
	lv_desc = check_desc(lvdesc);

	switch (lv_desc) {
		case name:
			get_attr(lvdesc,STRIPE_SIZE);
			break;
		default:         /* this is a lvid */
			cuatp = get_nm_id(lvdesc,FALSE,LVSERIAL_ID);
			get_attr(cuatp->name,STRIPE_SIZE);
		}

}


/*
 * NAME: get_stripe_width 
 *
 * FUNCTION: Gets the stripe width attribute value for 
 *      the logical volume descriptor.
 *	Gets the name from the CuAt class using the identifier then
 *	gets the type value from the CuAt class using the name.
 *
 * RETURN VALUE DESCRIPTION: Does not return if a match is not found, and
 *	will print error message to standard error.
 *	Will print the value of the type attribute for
 *	the logical volume identifier to standard out.
 */
void 
get_stripe_width(lvdesc)
char *lvdesc;		/* descriptor (INPUT) */
{ 
dvdesc_type lv_desc;
struct CuAt *cuatp;

	/* determine if name or identifier */
	lv_desc = check_desc(lvdesc);

	switch (lv_desc) {
		case name:
			get_attr(lvdesc,STRIPE_WIDTH);
			break;
		default:         /* this is a lvid */
			cuatp = get_nm_id(lvdesc,FALSE,LVSERIAL_ID);
			get_attr(cuatp->name,STRIPE_WIDTH);
		}

}

/*
 * NAME: main
 *
 * FUNCTION: getlvodm command reads data from the command line and
 *	gets the appropriate information from the ODM data class 
 *	fields.  The command line options specify what information 
 *	is retreivied to ODM.  
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. 
 *		One if illegal syntax.
 *		Two if ODM access error.
 *		Three if object not found in ODM.
 */

main(argc,argv) 
int argc;	   /* number of command line strings in argc */ 
char **argv;	   /* array of command line strings - from user */ 
{
int retval;			/* return code */ /* Initialize Prog */

	Prog = argv[0];
 
	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	/* setflg sets command option flags and command option values. */
	/* If error then exit. */
 
	if ((setflg(argc,argv,NULL)) < 0) exit(ILLEGALSYNTAX);

       	odm_initialize();

	/* set the config database path for the classes */
	odm_set_path(CFGPATH);

	lock_word = odm_lock("/etc/objrepos/lvm_lock",ODM_WAIT);
	/* each flag is done individually because only one flag is possible */

	if (a_flag) {
		/* Get the logical volume name given the lvdescript */
		/* lvdescript can be a lvname, or lvid */
		get_lvnm_lvdesc(a_value);
	}
	if (b_flag) {
		/* Get the volume group name for */
		/* this logical volume identifier */ 
		get_vgnm_lvid(b_value);
	}
	if (B_flag) {
		/* Get the label attribute for the */
		/* this logical volume descriptor */ 
		get_label_lvdesc(B_value);
	}
	if (c_flag) {
		/* Get the logical volume allocation characteristics */
		/* for this lvid */
		get_allochar_lvid(c_value);
	}
	if (C_flag) {
		/* Get all the configured physical volumes */
		get_confpvs();
	}
	if (d_flag) {
		/* Get the major number for this vgid */
		get_majnum_vgdesc(d_value);
	}
	if (e_flag) {
		/* Get the lvname for this lvid */
		get_lvnm_lvid(e_value);
	}
	if (F_flag) {
		/* Get the free configured physical volumes */
		get_freepvs();
	}
	if (g_flag) {
		get_pvnm_pvid(g_value);
	}
	if (G_flag) {
		/* Get stripe block size for this lvid */
		get_stripe_size(G_value);
	}
	if (h_flag) {
		/* Get all volume group names known to the system */
		get_vgnms();
	}
	if (j_flag) {
		/* Get the vgid for this physical volume identifier */
		get_vgid_pvdesc(j_value);
	}
	if (l_flag) {
		get_lvid_lvdesc(l_value);
	}
	if (L_flag) {
		get_lvs_vgdesc(L_value);
	}
	if (m_flag) {
		get_mtpt_lvid(m_value);
	}
	if (N_flag) {
		/* Get stripe width for this lvid */
		get_stripe_width(N_value);
	}
	if (p_flag) {
		get_pvid_pvdesc(p_value);
	}
	if (P_flag) {
		/* Get all the configured physical volumes */
		get_all_pvs();
	}
	if (r_flag) {
		get_reloc_lvid(r_value);
	}
	if (R_flag) {
		get_runtime();
	}
	if (s_flag) {
		/* Get the volume group state for this volume group name */
		get_vgst_vgdesc(s_value);
	}
	if (S_flag) {
		/* List all read/write optical device */
		get_rwopt_class();
	}
	if (t_flag) {
		/* Get the volume group id for this volume group name */
		get_vgnm_vgid(t_value);
	}
	if (u_flag) {
		/* Get the volume group state for this volume group name */
		get_auto_on_vgdesc(u_value);
	}
	if (v_flag) {
		/* Get the vgid for this volume group name */
		get_vgid_vgdesc(v_value);
	}
	if (Q_flag) {
		/* Get the quorum attribute for this vg name */
		retval = get_Q_vgdesc(Q_value);
                if ( retval ) printf("n");
                else printf("y");
	}
	if (w_flag) {
		/* Get the physical volume identifiers for this vgid */
		get_pvids_vgdesc(w_value);
	}
	if (y_flag) {
		get_type_lvid(y_value);
	}

	odm_unlock(lock_word);
	odm_terminate();

	exit(SUCCESS);
}

