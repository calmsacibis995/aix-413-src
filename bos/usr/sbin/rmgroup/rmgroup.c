static char sccsid[] = "@(#)64	1.12.1.4  src/bos/usr/sbin/rmgroup/rmgroup.c, cmdsuser, bos41J, 9514A_all 3/28/95 15:41:27";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: chkrmgroup
 *		isprimary
 *		main
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
#include	<pwd.h>
#include	<ctype.h>
#include	<sys/audit.h>	/* for auditing		*/
#include	<sys/priv.h>	/* for privilege 	*/
#include	<userpw.h>	/* for PW_NAMELEN	*/
#include 	<locale.h>	/* for setlocale() 	*/
#include	<grp.h>		/* for getgrnam()	*/
#include	"tcbauth.h"	/* for stuff		*/

static	int	isprimary(char *,gid_t);
static	void	chkrmgroup(char *);

/*
 *
 * NAME:     rmgroup
 *                                                                    
 * FUNCTION: removes an existing group
 *                                                                    
 * USAGE:    rmgroup group
 *	     where:
 *	       	 group : is the group to be removed.
 *                                                                   
 * RETURNS: 0 if successful else the return is as follows:
 *		
 *		ENOENT 	if the group does not exist
 *		EACCES 	if the files cannot be written -i.e., the invoker
 *			doesn't have write access to the user database
 *		EPERM 	if the group identification and authentication fails
 *			if the group is administrative or has members that are
 *				administrative users & invoker != root.
 *			if the group is a primary group of a user.
 *		errno	if system error.
 */  

main(int argc,char *argv[])
{
gid_t	gid;		/* the group id from getgroupattr */
char	*group;		/* the group name from the command line */
struct	group *gr;	/* the group struct from getgrnam	*/

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/* check arguments */
	if (argc != 2 || argv[1][0] == '-')
		usage(RMGRPUSAGE);

	group = argv[1];

	/* suspend auditing for this process */
	auditproc(0,AUDIT_STATUS,AUDIT_SUSPEND,0);

	/* Check for user's length < PW_NAMELEN */
	if(strlen(group) > PW_NAMELEN - 1)
	{
		fprintf(stderr,RMERR,group);
		fprintf(stderr,TOOLONG);
		exit(ENAMETOOLONG);
	}

	/* suspend privilege */
	privilege(PRIV_LAPSE);

	/* check for deletion of keywords */
	if (chkeyword(group))
	{
		fprintf(stderr,RMKEY,group);
		exit(EPERM);
	}

	/* Check if the group exists locally or in NIS	*/

	set_getgrent_remote(2);	/* compat lookups only	*/
	gr = getgrnam(group);
	set_getgrent_remote(1);	/* resume full lookups	*/
	if (gr == (struct group *)NULL)
	{
		fprintf(stderr,GRPNONEX,group);
		exit(ENOENT);
	}
	gid = gr->gr_gid;

	/* Open the user database */
	setuserdb( S_READ | S_WRITE );

	/* Check if the group is a primary group for a user */
	if(isprimary(group,gid))
		exitax(RMGRPAUD,EPERM,group,NULL,NOPRINT);

	/*
	 * Check if the invoker has permission to remove
	 * this group.  There is no need for return check
	 * since this routine exits on error.
	 */
	chkrmgroup(group);

	/* Remove the group */
	rmufile(group,0,GROUP_TABLE);

	/* Close the user database */
	enduserdb();
	/*
	 * Audit the successful removal of group and exit.
	 */
	exitax(RMGRPAUD,0,group,NULL,NOPRINT);
}

/*
 * FUNCTION:	chkrmgroup
 *
 * DESCRIPTION:	checks to see if the invoker has access to remove an 
 *		administrative group or to remove a group that has members that 
 *		are administrative users.
 *
 * PASSED:	group = groupname
 *
 * RETURNS:	None.
 *
 */

static void
chkrmgroup(char *group)
{
int	i;		/* loop counter			      */
int	num = 0;	/* num of grps in the attribute value */
char	*members;	/* the group's members 		      */
char	**users;	/* array of group member users 	      */

	/* Check if the group is administrative & the invoker has access */
	if(!gotgaccess(group))
	{
		fprintf(stderr,RMADMERR,group);
		exitax(RMGRPAUD,EPERM,group,NULL,PRINT);
	}

	/*
	 * Check if the group contains administrative member users & if the 
	 * invoker has access 
	*/
	if (getgroupattr(group,S_USERS,&members,SEC_LIST))
	{
		if (errno != ENOATTR && errno != ENOENT)
			fprintf(stderr,RMADMERR,group);
			exitax(RMGRPAUD,EPERM,group,NULL,PRINT);
	}

	users = listoarray(members,&num);

	for (i = 0;i < num;i++)
	{
		if(!gotuaccess(users[i]))
		{
			fprintf(stderr,RMADMUSR,users[i],group);
			exitax(RMGRPAUD,EPERM,group,NULL,PRINT);
		}
	}
}


/*
 * FUNCTION:	isprimary
 *
 * DESCRIPTION:	checks for primary id in /etc/passwd
 *
 * RETURNS:	true or false.
 *
 */

static int
isprimary(char *group,gid_t gid)
{
struct	passwd	*pwd;	/* pointer to passwd structure	*/

	setpwent();
	while ((pwd = (struct passwd *)getpwent()) != NULL)
	{
		if (gid == pwd->pw_gid)
		{
			fprintf(stderr,RMPRIME,pwd->pw_name,group);
			return(1);
		}
	}
			
	endpwent();

	return(0);
}

