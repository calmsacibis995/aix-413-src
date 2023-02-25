static char sccsid[] = "@(#)60	1.16.1.5  src/bos/usr/sbin/lsuser/lsuser.c, cmdsuser, bos41J, 9514A_all 3/28/95 15:39:38";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: getusrs
 *		getvalue
 *		list_def
 *		listall
 *		listattrs
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

#include	<stdio.h>	/* for printfs		*/
#include	<pwd.h>		/* for perror		*/
#include	<sys/param.h>	/* for MAXUID		*/
#include	<locale.h>	/* for setlocale() 	*/
#include	<userpw.h>	/* for PW_NAMELEN	*/
#include	<stdlib.h>
#include	<sys/access.h>
#include	"tcbauth.h"

struct user
	{
		char *usr;
		struct user *next_usr;
	};
		
static	void	getvalue(char *,char *,char **,char *);
static	int	getusrs(char *,struct user *);
static	void	listall(char *,char *,unsigned short);
static	void	listattrs(struct user *,char *,int,int,char **,unsigned short);
static	void	list_def();


/*
 *
 * NAME:     lsuser
 *                                                                    
 * FUNCTION: lists the attributes of a user
 *                                                                    
 * USAGE:    lsuser [-c|-f] [-a] [-D] "attr ..." { "ALL" | user1[,user2,...]}
 *	     where:
 *			-c : prints the output for smit 
 *				in colon separated fields.
 *			-f : prints the output in stanza file format.
 *			-a : lets you specify specific attributes to list.
 *			-D : lists the defaults in the /etc/security files for
 *			     smit.  If you don't have access to the files,
 *			     then you're not root or group "security" so just
 *			     send smit a "#" sign.
 *		"attr ... ": are the attributes to list.
 *		username   : is the user or users.
 *		"ALL"	   : is all system users.
 *                                                                   
 * RETURNS: 0 if successful else the return is as follows:
 *		
 *		EINVAL 	if the attribute specified is invalid
 *		ENOENT 	if the user doesn't exist
 *		EACCES 	if the attribute cannot be listed - i.e., the invoker
 *			does not have read_access to the user database.
 *		EPERM 	if the user identification and authentication fails
 *		errno	if system error.
 */  

main(int argc,char *argv[])
{
struct	 user	usrs[1];	/* structure for user link list		*/
struct	 user	*usrsp;		/* pointer to increment through list 	*/
unsigned short	flag = 0;	/* flag for colon format 		*/
register int	c;		/* counter for getopt			*/
char	 	*delim = SPACE;	/* the delimiter value			*/

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	if (argc < 2)
		usage(LSUSRUSAGE);

	/* Open the user database */
	setuserdb( S_READ );

	/* Get the flags */
	while ((c = getopt(argc,argv,"acfD")) != EOF)
	{
		switch (c)
		{
		case 'a':	flag |= AFLAG;
				break;

		case 'c':	if (flag & FFLAG)
					usage(LSUSRUSAGE);
				flag |= CFLAG;	
				delim = COLON;	
				break;

		case 'f':	if (flag & CFLAG)
					usage(LSUSRUSAGE);
				flag |= FFLAG;	
				delim = CARAT;
				break;

		case 'D':	list_def();

		default :	usage(LSUSRUSAGE);
		}
	}

	if (flag & AFLAG)
	{
		/* if no more arguments print usage */
		if (!argv[optind])
			usage(LSUSRUSAGE);

		/* get array of user names */
		getusrs((char *)argv[argc - 1],usrs);

		/*
		 * Print all the attributes listed.  This 
		 * routine exits, so no need to check return.
		 */
		listattrs(usrs,delim,optind,argc,argv,flag);
	}

	/* get array of user names */
	getusrs((char *)argv[argc - 1],usrs);

	/* set up pointer to user struct */
	usrsp = usrs;

	do
	{
		listall(usrsp->usr,delim,flag);
		usrsp = usrsp->next_usr;

	} while (usrsp != NULL);

	/* Close the user database */
	enduserdb();
}


/*
 * FUNCTION:	getvalue
 *
 * DESCRIPTION:	get the current attribute values from the user database.
 *
 * PASSED:	user = username, name=attribute name, string=attribute value
 *		delim= default delimiter
 *
 * RETURNS:	None.
 *
 */

static	void
getvalue(char *user,char *name,char **string,char *delim)
{
void		*val;		/* attribute value			*/
struct lsusattr	*ptr;		/* ptr to table				*/
void		*ret;		/* return value from check routines	*/
int		err = 1;	/* flag to indicate valid attribute	*/

	/* go through table looking for attribute name */
	for (ptr = lsusatab;ptr < &lsusatab[lsusatabsiz]; ptr++)
	{
		if (!strcmp(name, ptr->gattr))
		{
			err = 0;
			if (getuserattr(user,name,(void *)&val,ptr->type))
			{
			    if(errno != ENOATTR && errno != ENOENT)
				err = 1;
			}
			else
			{
				/* check if data needs formatting */
				if (ptr->format)
					ret=(void *)(*(ptr->format))(user,val);
				else
					ret = val;

				/* build the new string */
				*string = getnewstring(*string,ptr->gattr,
					delim,ptr->type,ret);
			}				
			break;
		}
	}

	/*
	 * If any attributes were invalid, print error and exit.
	 */
	if (err)
	{
		fprintf(stderr,LISTERR,name);
		exit (EINVAL);
	}
}

/*
 * FUNCTION:	getusrs
 *
 * DESCRIPTION:	parses the list of users passed, validates the string, and
 *		builds a linked list of struct user's.
 *
 * PASSED:	string = string of comma-separated users, llist = linked
 *		list of struct user.
 *
 * RETURNS:	the number of users found.
 *
 */

static	int
getusrs(char *string,struct user *llist)
{
struct	user	*llistp;	/* temporary pointer		*/
struct	user	*llistpn;	/* temporary pointer		*/
struct	passwd	*pwd;		/* return from getpwent		*/
int		i = 0;		/* the number of valid users 	*/
char		*ptr;		/* temporary pointer		*/
char		**userlist;	/* list of all valid users	*/
int		j = 0;		/* loop counter			*/
int		siz;
int		id;

	llistp = llist;

	/* a wildcard will return all users */
	if (!strcmp(string,ALL))
	{
		setpwent();
		
		while ((pwd = (struct passwd *)getpwent()) != NULL)
		{
			/* Check for username's length < PW_NAMELEN */
			if((siz = (strlen(pwd->pw_name) + 1)) > PW_NAMELEN)
			{
				fprintf(stderr,ERRLIST,pwd->pw_name);
				fprintf(stderr,TOOLONG);
				errno = ENAMETOOLONG;
				continue;
			}

			if ((ptr = (char *)malloc(siz)) == NULL)
			{
				fprintf (stderr,MALLOC);
				exitx (errno);
			}
			
			strcpy(ptr,pwd->pw_name);
			if (i == 0)
				llistp->usr = ptr;
			else 
			{
				siz = sizeof(struct user);
				if ((llistpn=(struct user *)malloc(siz))==NULL)
				{
					fprintf(stderr,MALLOC);
					exitx(errno);
				}
				llistp->next_usr = llistpn;
				llistp = llistpn;
				llistp->usr = ptr;
			}
			llistp->next_usr = NULL;
			i++;
		}

		endpwent();
	}
	else
	{

		/*
		 *  Drop all spaces and tabs from string, and
		 *  build a linked list of user names provided
		 *  from the command line.  Then check each name
		 *  for existence and build the list.
		 */

		userlist = comtoarray(string,&i);

		for (j=0;j<i;j++)
		{
			/* check if user exists locally or in NIS	*/
			set_getpwent_remote(2);	/* compat lookups only	*/
			pwd = getpwnam(userlist[j]);
			set_getpwent_remote(1);	/* resume full lookups	*/
			if (pwd == (struct passwd *)NULL)
			{
				fprintf(stderr,USRNONEX,userlist[j]);
				exit (ENOENT);
			}

			if (j == 0)
				llistp->usr = userlist[j];
			else 
			{
				siz = sizeof(struct user);
				if ((llistpn=(struct user *)malloc(siz))
						== NULL)
				{
					fprintf(stderr,MALLOC);
					exitx(errno);
				}
				llistp->next_usr = llistpn;
				llistp = llistpn;
				llistp->usr = userlist[j];
			}
		}
		llistp->next_usr = NULL;
	}

	return(i);
}

/*
 * FUNCTION:	listall
 *
 * DESCRIPTION:	list all attributes
 *
 * PASSED:	user = username, delim = default delimiter
 *		flag format flag
 *
 * RETURNS:	None.
 *
 */

static	void
listall(char *user,char *delim,unsigned short flag)
{
struct lsusattr	*ptr;		/* ptr to table			*/
void		*val;		/* returned attribute value	*/
void		*ret;		/* returned attribute value	*/
char		*string;	/* pointer to the final string	*/
register int	siz;

	siz = strlen(user) + strlen(delim) + 1;
	if ((string = (char *)malloc(siz)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}
	strcpy(string,user);
	strcat(string,delim);
	
	/*
	 * go through table looking for attribute name and get value
	 */
	for (ptr = lsusatab;ptr < &lsusatab[lsusatabsiz]; ptr++)
	{
		if(getuserattr(user,ptr->gattr,(void *)&val,ptr->type))
			continue;
		else
		{
			/* check if data needs to be formatted */
			if (ptr->format)
				ret = (void *)(*(ptr->format))(user,val);
			else
				ret = val;

			string = getnewstring(string,ptr->gattr,
					delim,ptr->type,ret);
		}
	}

	if (flag & FFLAG)
		printstanza(delim,string);

	else if (flag & CFLAG)
		printcolon(delim,string);

	else
		printf("%s\n",string);

	free ((void *)string);
}


/*
 * FUNCTION:	listattrs
 *
 * DESCRIPTION:	main routine to parse the attributes and call the routines
 *		to print them.
 *
 * PASSED:	names = ptr to linked list of users, delim = default delimiter, 
 *		num = # of attributes, argc = # of parameters entered, argv = 
 * 		array of attributes,  flag = format flag
 *		format flag.
 *
 * RETURNS:	No Return.
 *
 * PASSED:
 *	struct	user	*names;		 ptr to the list of users
 *	char		*delim;		 the default delimiter
 *	int		num;		 the number of attributes
 *	int		argc;		 the number of parameters entered
 *	char		*argv[];	 the array of attributes
 *	int		flag;		 the format flag
 */

static	void
listattrs(struct user *names,char *delim, int num,int argc,
		char *argv[],unsigned short flag)
{
struct	user	*namesp;	/* pointer to step through list */
register int	c = 0;		/* counter */
char		*string;	/* the final string to print */
register int	siz;

	namesp = names;
	while(*(namesp->usr))
	{
		siz = strlen(namesp->usr) + strlen(delim) + 1;
		if ((string =(char *)malloc(siz)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}
		strcpy(string,namesp->usr);
		strcat(string,delim);

		/* get value of attribute from data base */
	   	for(c=num;c<(argc-1);c++)
	   	{
			getvalue(namesp->usr,argv[c],&string,delim);
		}

		if (flag & FFLAG)
			printstanza(delim,string);
		else if (flag & CFLAG)
			printcolon(delim,string);
		else
			printf("%s\n",string);

		if (namesp->next_usr == NULL)
			break;

		namesp = namesp->next_usr;
	}
	exit (0);
}

/*
 * FUNCTION:	list_def
 *
 * DESCRIPTION:	Lists the user defaults found in /etc/security/limits and
 *		/etc/security/user, except those with ring values in the
 *		SMIT data base.
 *
 * RETURNS:	No Return.
 *
 */

static void
list_def()
{
struct lsusattr	*valptr;	/* ptr to lsuser attribute table	   */
int	i = 0,			/* number of elements that we put into ptr */
	j = 0;			/* counter for total number of elements	   */
char	*val;			/* the value returned from getuserattr()   */
char	**ptr;			/* the list of attribute values		   */
int	size = BUFSIZ;		/* space to malloc			   */


	/*
	 *   Here's the place where an access check needs to be
	 *   made.  If we don't have access to the security
	 *   databases, then might as well break out now.
	 *   The printing of the "#" sign is for SMIT.
	 */
	printf("#");

	if (access(USERFILE,R_ACC) || access(LIMFILE,R_ACC))
	{
		printf("\n");
		exit(0);
	}

	/*
	 *   Make a table to hold any user attributes that
	 *   we may need.  It won't be more that the lsusatab
	 *   attribute list.
	 */

	if ((ptr = malloc(lsusatabsiz * sizeof (char *))) == NULL)
	{
		fprintf (stderr,MALLOC);
		exitx (errno);
	}

	/*
	 *   Loop through each attribute in the lsuser table looking
	 *   for the "default" value.  If getuserattr() fails, skip
	 *   that value, otherwise check for ring value and process
	 *   based on the type.  SEC_BOOL will be "true" or "false",
	 *   with size = 5 and 6 respectively.  Since we don't know
	 *   the size of the list, we'll take a limit of BUFSIZ.  For
	 *   SEC_INT, we take a size of 20, since we won't get an 
	 *   integer out over 20 digits.
	 */
	for (j = 0,valptr = lsusatab; j < lsusatabsiz; j++, valptr++)
	{
		if (getuserattr(DEFAULT,valptr->gattr,(void *)&val,
							valptr->type))
			continue;

		/*
		 * Go ahead and get all the default values
		 * and display them.  There is no longer
		 * a need to check ring values for SMIT
		 * since the ring attributes have handlers
		 * built into the SMIT code.
		 */
		if (i != 0)
			printf(COLON);
		printf("%s",valptr->gattr);
		switch (valptr->type)
		{
			case SEC_CHAR:
				size = strlen(val)+1;
				break;
			case SEC_INT:
				size = 20;
				break;
			case SEC_BOOL:
				if ((int)val)
				{
					size = 5;
					val = chgbool(NULL,val);
				}
				else
				{
					size = 6;
					val = chgbool(NULL,val);
				}
				break;
			case SEC_LIST:
				size = BUFSIZ;
				break;
		}
		if ((ptr[i] = malloc(size)) == NULL)
		{
			fprintf (stderr,MALLOC);
			exitx (errno);
		}

		/*
		 *  Since we want to display our "umask" in three
		 *  digits to SMIT, we'll play with the output
		 *  string.  If you don't special-case this attr,
		 *  you'll get "22" for "022".  This is OK and
		 *  won't break anything, but formatting the output
		 *  could help with any confusion.
		 */
		switch (valptr->type)
		{
			case SEC_BOOL:
			case SEC_CHAR:
				strcpy(ptr[i],val);
				break;
			case SEC_INT:
				if (!strcmp(valptr->gattr,S_UMASK))
					sprintf(ptr[i],"%3.3d",
							(int)val);
				else
					sprintf(ptr[i],"%d",(int)val);
				break;
			case SEC_LIST:
				listocom(val);
				strcpy(ptr[i],val);
				break;
		}
		i++;
	}
	printf("\n");

	/*
	 *  Print up to last one but don't put COLON on end
	 */
	for (j = 0; j < (i - 1); j++)
	{
		printf("%s",ptr[j]);
		printf(COLON);
	}
	printf("%s",ptr[j]);
	exit (0);
}
