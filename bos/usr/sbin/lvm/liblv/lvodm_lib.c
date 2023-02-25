static char sccsid[] = "@(#)40	1.10.1.4  src/bos/usr/sbin/lvm/liblv/lvodm_lib.c, cmdlvm, bos411, 9428A410j 3/22/94 23:02:49";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: odm_error, get_nm_id, get_vgnm_lvid, check_vg_lock, lock_vg,
 *            chng_attr, unlock_vg.
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

#define _ILS_MACROS	/* performance enhancement for isxdigit(), etc. calls */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include "cmdlvm.h"
#include "cmdlvm_msg.h"
#include <ctype.h>

extern odm_terminate();
extern struct CuAt *getattr();
extern char* lvm_msg();

extern nl_catd  scmc_catd;     /* Cat descriptor for scmc conversion */
extern char *Prog;             /* program name */

#define ODMERROR       -1      /* error return code from cfg routines */

/*
 * NAME: odm_error   
 *
 * DESCRIPTION: Prints out an odm error message using odm_err_msg or 
 *		if an unknown error number occurs it prints out a
 *		generic error message to standard error.
 *		
 * INPUT: The error message.
 *
 * OUTPUT: Error message to standard error.  
 *
 */
void
odm_error(err)
int err;
{
char *text; /* pointer to return the NLS message from odm_err_msg */

	if(odm_err_msg(err,&text) == -1) {
		fprintf(stderr, lvm_msg(LVODM_ERROR),Prog,err);
	}
	else {
		fprintf(stderr,text);
		fprintf(stderr, lvm_msg(ODM_DOWN),Prog);
	}
			
	odm_terminate();
	exit(ODMACCESSERROR);
}
/* Get the quorum attribute given the vgdesc */
int
get_Q_vgdesc(vgdesc)
char *vgdesc;
{
struct CuAt *cuatp;
int temp;
	
cuatp = getattr(vgdesc,"quorum",FALSE,&temp);

/* if getattr returns ODMERROR then and odm error has occured */
if ((int)cuatp == -1) odm_error(odmerrno);
if (cuatp != NULL) {
	if (!strcmp(cuatp->value, "n")) return(1);
	else return(0);
}
else return(0);
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
	
	odm_initialize();
	odm_set_path(CFGPATH);
   	sprintf(crit,"attribute = '%s' and value = '%s'",typeid,dvid);
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	if ((int)cuatp == ODMERROR)
		odm_error(odmerrno);
	if (cuatp == NULL) {
		fprintf(stderr,lvm_msg(ID_NOT_FOUND),Prog,dvid);
		odm_terminate();
		exit(OBJNOTFOUND);
	}
	/* if prtflag is set to TRUE then print to standard out */
	if (prtflag)
       		printf("%s\n",cuatp->name);           
       	return(cuatp);
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
get_vgnm_lvid(lvid,vgname)
char *lvid;
char *vgname;
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
		odm_error(odmerrno);
	if (cuatp != NULL) 
       		sprintf(lvname,"%s",cuatp->name);           
	else {
		fprintf(stderr,lvm_msg(LV_NOT_FOUND),Prog,lvid);
		odm_terminate();	
		exit(OBJNOTFOUND);
	}

	/* after we have the name from the id then get the parent the vgname */
     	sprintf(crit,"name = '%s'",lvname);
	cudvp = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,1,1);
	if ((int)cudvp == ODMERROR)
		odm_error(odmerrno);
	if (cudvp != NULL) 
			if (vgname == NULL) printf("%s\n",cudvp->parent);           
			else sprintf(vgname,"%s\n",cudvp->parent);           
	else {
		fprintf(stderr,lvm_msg(LV_NOT_FOUND2),Prog,lvname);
		odm_terminate();	
		exit(OBJNOTFOUND);
	}
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
		odm_error(odmerrno);
        
	if (cuatp != NULL) {
		if (cuatp->value[0] == 'n') { 
			strcpy(cuatp->value,"y");
			if (putattr(cuatp) == ODMERROR)
				odm_error(odmerrno);
			return(FALSE);
		}
		else
			return(TRUE);
	}      
	else {
		fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,LOCK);
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
int
lock_vg(cuatp,vgname)
struct CuAt *cuatp;
char *vgname;
{
int getlockid;
int i = 0;
int rc = 0;
int LOCKED = TRUE;

	while (LOCKED && (++i <= WAITLOOP)) {
		/* use the odm_lock to get the lvm_lock */
		/* the odm_lock returns an identifier to use in the unlock */

		/* wait for a free lvm_lock */
		getlockid = odm_lock("/etc/objrepos/lvm_lock",ODM_WAIT);

		/* if the lock was obtained then check the vg_lock */
		if (getlockid != -1) {
			/* if the volume is not locked, lock it and */
			/* return FALSE otherwise return TRUE */
			LOCKED = check_vg_lock(cuatp,vgname);

			/* unlock the lvm_lock using the identifier obtained */
			odm_unlock(getlockid);
		}
	}
	/* if the vg_lock was not obtained exit with 1 */
	if (LOCKED) {
		fprintf(stderr,lvm_msg(VG_LOCKED),Prog,vgname);
		rc = 1;
	}

	return(rc);
			
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
		odm_error(odmerrno);
        
	if (cuatp != NULL) {
		strcpy(cuatp->value,attrval);
		if (putattr(cuatp) == ODMERROR)
			odm_error(odmerrno);
	}      
	else {
		fprintf(stderr,lvm_msg(ATTRNOTFOUND),Prog,attribute);
		odm_terminate();
		exit(OBJNOTFOUND);     
	}
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
 *	lock_a_vg
 *
 *	- lock one vg
 *	- unlock_a_vg is this one's brother
 *
 *	- these two are just a quick set of functions for
 *	  locking vg's for the high level commands.  they can only keep
 *	  one vg locked at a time (and lock_a_vg enforces this by unlocking
 *	  the one currently locked).  and, they attempt to insure unlocking
 *	  at exit by catching some signals and using atexit()...
 */

static char *Locked_name = NULL;	/* name of the locked vg	*/
static int Lock_initted = 0;		/* did we setup cleanup yet?	*/

static void get_vg_name();		/* function we call below	*/
static void init_lock_a_vg();	/* setup cleanup		*/

void unlock_a_vg(void);

extern char *strdup(char *);

int
lock_a_vg(char *prog, char *vgname)
{
        struct CuAt *cuatp;
	int error, temp = 1;

	odm_initialize();
	odm_set_path(CFGPATH);

	Prog = prog;

	if (Locked_name != NULL)	/* vg already locked? */
		unlock_a_vg();/* already varied and/or locked, unlock */
	else /* not varied on, but locked, so unlock it*/
        {
                cuatp = getattr(vgname,LOCK,FALSE,&temp);

                if ((int)cuatp == -1)
                        odm_error(odmerrno);

                if (cuatp != NULL)
                        if (cuatp->value[0] == 'y')
                        {
                                strcpy(cuatp->value,"n");
                                if (putattr(cuatp) == ODMERROR)
                                        odm_error(odmerrno);
                        }
        }
	
	vgname = strdup(vgname);

	/*
	 * we can handle either a vg name or vg id...  if we're given
	 * a vgid, we get the name anyway...
	 */
	if (!isa_vgname(vgname))
		get_vg_name(&vgname);

	if (lock_vg(NULL, vgname) != 0) {
		error = 1;		/* already locked or lock failed */
		free((void *) vgname);
		}

	else {
		error = 0;		/* success!  the crowd goes wild... */
		Locked_name = vgname;
		/*
		 * be sure signals are caught, etc
		 */
		if (!Lock_initted)
			init_lock_a_vg();
		}

	return (error);
}

/*
 *	unlock_a_vg
 *
 *	- unlock our locked vg.
 */
void
unlock_a_vg(void)
{
	int temp = 1;

	if (Locked_name != NULL) {
		unlock_vg(NULL, Locked_name);
		free((void *) Locked_name);
		Locked_name = NULL;
		}
}

/*
 *	catchsigs
 *
 *	- generic signal catcher for this family of routines
 *
 *	- it just calls previous signal catching routines and exits
 */

#include <sys/signal.h>
static void (*Previous_sigs[NSIG])(int);	/* previous signal values */

static void
catchsigs(int sig)
{
	/*
	 * call previous catcher
	 */
	if (Previous_sigs[sig] != SIG_DFL && Previous_sigs[sig] != SIG_IGN &&
	    Previous_sigs[sig] != NULL)
		(*Previous_sigs[sig])(sig);

	exit(sig);
}

/*
 *	be sure to unlock at exit, etc.
 */
static void
init_lock_a_vg()
{
	int i;
	static int sigs[] = { SIGHUP, SIGINT, SIGQUIT, SIGTERM };

	/*
	 * if the caller exists, be sure to unlock the vg
	 */
	(void) atexit(unlock_a_vg);

	/*
	 * catch sigs.  remember previous catchers
	 */
	for (i = 0; i < sizeof(sigs) / sizeof(sigs[0]); i++) {
		Previous_sigs[sigs[i]] = signal(sigs[i], SIG_IGN);
		/*
		 * but only catch if we weren't ignoring this one...
		 */
		if (Previous_sigs[sigs[i]] != SIG_IGN)
			(void) signal(sigs[i], catchsigs);
		}

	Lock_initted = 1;
}

/* copied from get_nm_id */
static void
get_vg_name(dvid)
char **dvid;
{
struct CuAt *cuatp;
struct listinfo stat_info;
char crit[CRITSIZE];

	odm_initialize();
	odm_set_path(CFGPATH);

   	sprintf(crit,"attribute = '%s' and value = '%s'",VGSERIAL_ID, *dvid);
	cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);

	if ((int) cuatp == ODMERROR || cuatp == NULL) {
		  /* in case dvid is really a vgname */                             
   	      sprintf(crit,"attribute = '%s' and name = '%s'",VGSERIAL_ID, *dvid);
	      cuatp = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	      if ((int) cuatp == ODMERROR || cuatp == NULL) {
		          free((void *) *dvid);     /* NO match at all */
		          *dvid = strdup(*dvid);
		}
		}
	else {
		free((void *) *dvid);
		*dvid = strdup(cuatp->name);
		}
}

/*
 * NAME: isa_vgname
 *
 * FUNCTION: Checks the descriptor to decide if it is a name or identifier. 
 *	If there is a decimal then it will be an identifier.  And if
 *	there is a non-hex character then it will be a name.
 *
 * RETURN VALUE DESCRIPTION: Will return 1 if it's a vg name, 0 if it's
 *		a vg id, else -1
 */

int
isa_vgname(desc)
char *desc;
{
char *tempptr;
int id;

	/* default value for the dvdesc_type */
	id = TRUE;
	tempptr = desc;
	/* parse the descriptor until the end and */
	/* test if any non-hex characters are found */
	if (tempptr == NULL)
	    return -1;

	while ((*tempptr != NULL) && (*tempptr != '.')){
		if (!isxdigit((int)*tempptr)) 
			id = 0;		/* then not id */
		tempptr++;
	}
	/* if id is still possible test the length */
	if (id) {
		if ((strlen(desc) == VGIDLENGTH) 
				|| (strlen(desc) == CUPVPVID))
			return (0);
		else if (*tempptr == '.') {
			if ((strlen(desc) <= DEVNAMESIZE))
				return (1);
			else 
				return (0);
		}
		else {
			if (strlen(desc) <= DEVNAMESIZE) 
				return (0);
			else
				return (-1);
		}
	}
	 else
		return (1);
}
