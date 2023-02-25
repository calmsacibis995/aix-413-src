static char sccsid[] = "@(#)65	1.12.1.10  src/bos/usr/sbin/rmuser/rmuser.c, cmdsuser, bos41J, 9514A_all 3/28/95 15:40:59";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: main
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include 	<stdio.h>
#include	<ctype.h>
#include	<sys/audit.h>	/* for auditing		*/
#include	<sys/priv.h>	/* for privilege 	*/
#include	<userpw.h>	/* for PW_NAMELEN	*/
#include 	<locale.h>	/* for setlocale() 	*/
#include	<sys/wait.h>	/* for fork()		*/
#include	"tcbauth.h"	/* for getuserattr	*/

/*
 *
 * NAME:     rmuser
 *                                                                    
 * FUNCTION: removes an existing user account
 *                                                                    
 * USAGE:    rmuser [ -p ] username
 *	     where:
 *			-p : deletes the users authentication info from
 *			     /etc/security/passwd.
 *	       	 username  : is the user to be removed.
 *                                                                   
 * RETURNS: 0 if successful else the return is as follows:
 *		
 *		ENOENT 	if the username does not exist
 *		EACCES 	if the files cannot be written-i.e., the invoker
 *			doesn't have write access to the user database.
 *		EPERM 	if the user identification and authentication fails
 *			if the user is administrative and the invoker != root
 *		errno	if system error.
 */  

main(int argc,char *argv[])
{
int	flag = 0;
char	*user;
int	rc;
int	id;
DBM	*pwdbm;		/* hashed look-aside passwd database	  */
char	*emptylist;	/* empty list of groups for putuserattr() */
char	*groups;	/* set of current user's groups		  */
struct	passwd *pw;	/* passwd struct returned from getpwnam() */

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/*
	 * Suspend auditing and privilege
	 * for this process.
	 */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);
	privilege(PRIV_LAPSE);

	/*
	 * Check arguments from command line
	 */
	if (argc < 2 || argc > 3)
		usage(RMUSRUSAGE);

	if (argc == 3)
		if (strcmp("-p",argv[1]))
			usage(RMUSRUSAGE);
		else
			flag = 1;

	user = argv[argc - 1];

	/* Check for user's length < PW_NAMELEN */
	if(strlen(user) > PW_NAMELEN - 1)
	{
		fprintf(stderr,RMERR,user);
		fprintf(stderr,TOOLONG);
		exit(ENAMETOOLONG);
	}

	/* check for deletion of keywords */
	if (chkeyword(user))
	{
		fprintf(stderr,RMKEY,user);
		exit(EPERM);
	}

	/* Check if the user exists locally or in NIS	*/

	set_getpwent_remote(2);	/* compat lookups only	*/
	pw = getpwnam(user);
	set_getpwent_remote(1);	/* resume full lookups	*/
	if (pw == (struct passwd *)NULL)
	{
		fprintf (stderr,USRNONEX,user);
		exit(ENOENT);
	}

	/* Open the user database */
	setuserdb( S_READ | S_WRITE );

	/* open database */
	pwdbm = pwdbm_open();

	/* Check if the invoker has permission to remove the user */
	if (!gotuaccess(user))
	{
		fprintf(stderr,RMADMERR,user);
		exitax(RMUSRAUD,EPERM,user,NULL,PRINT);
	}

	/*
	 * Delete the user from all groups in /etc/group.
	 * The variable "emptylist" is only a holding
	 * place and will be discarded.
	 */
	if (getuserattr(user,S_GROUPS,&groups,SEC_LIST))
		if (errno != ENOATTR && errno != ENOENT)
		{
			fprintf(stderr,RMERR,user);
			exitax(RMUSRAUD,errno,user,NULL,PRINT);
		}

	if (groups && *groups)
	{
		listocom(groups);
		delmems((char *)NULL, groups, user, &emptylist);
	}

	/* 
	 * We must make sure we pass a NULL to the putuserattr()
	 * routines to keep the database correct.
	 */
	if (putuserattr(user,S_GROUPS,(char *)NULL,SEC_LIST))
	{
		fprintf(stderr,RMERR,user);
		exitax(RMUSRAUD,errno,user,NULL,PRINT);
	}

	/* 
	 * Delete the user from /etc/security/group.
	 */
	if (addtoadms(user,(char *)NULL))
	{
		fprintf(stderr,RMERR,user);
		exitax(RMUSRAUD,errno,user,NULL,PRINT);
	}

	/*
	 * Remove the user and audit
	 * the success.  Save the return
	 * code for dbm file updates.
	 */
	rc = rmufile(user,flag,USER_TABLE);

	/*
	 * commit the changes to the data base
	 */
	if (putuserattr(user,(char *)NULL,((void *) 0),SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,user);
		exitax(RMUSRAUD,errno,user,NULL,PRINT);
	}
	
	if(putgroupattr((char *)NULL,(char *)NULL,(void *)NULL,SEC_COMMIT))
	{
		fprintf(stderr,COMMIT,user);
		exitax(RMUSRAUD,errno,user,NULL,PRINT);
	}

	/* Close the user database */
	enduserdb();

	/*
	 * Update the dbm files.
	 */
	if (pwdbm != NULL)
	{
		if (rc == 0)
			 if (pwdbm_delete(pwdbm, user))
				fprintf (stderr, DBM_ADD_FAIL);
		(void) dbm_close(pwdbm);
	}

	exitax(RMUSRAUD,0,user,NULL,NOPRINT);

}
