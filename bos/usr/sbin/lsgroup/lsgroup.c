static char sccsid[] = "@(#)59	1.20  src/bos/usr/sbin/lsgroup/lsgroup.c, cmdsuser, bos41J, 9514A_all 3/28/95 15:40:23";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: addprim
 *		addprimaries
 *		getgrps
 *		getvalue
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

#include	<stdio.h>
#include	<grp.h>	
#include 	<pwd.h>
#include 	<locale.h>		/* for setlocale() */
#include	<userpw.h>		/* for PW_NAMELEN  */
#include	<stdlib.h>
#include	"tcbauth.h"

struct list {				/* 'cache' of users we've already */
		struct list	*next;	/* seen, in case addprimaries()   */
		char		*name;	/* is called more than once.	  */
		gid_t		 gid;
	    };

struct groups {
		char *grp;
		struct groups *next_grp;
	      };
		
struct list *head = NULL;		/* head of list			*/

static	void	getvalue(char *,char *,char **,char *);
static	int	getgrps(char *,struct groups *);
static	void	listall(char *,char *,int);
static	void	listattrs(struct groups	*,char *,int,int,char **,int);
static 	void 	*addprimaries(char *, void *);
static	char 	*addprim(char *, char *);

/*
 *
 * NAME: lsgroup
 *                                                                    
 * FUNCTION: lists the attributes of a group
 *                                                                    
 * USAGE: lsgroup [ -c | -f ] [ -a  "attr ..."] { "ALL"|group1[,group2,...]}
 *	       where:
 *			-c : prints the output for smit 
 *				in colon separated fields.
 *			-f : prints the output in stanza file format.
 *			-a : lets you specify specific attributes to list.
 *		"attr ... ": are the attributes to list.
 *		group      : is the group or groups.
 *		"ALL"	   : is all system users.
 *                                                                   
 * RETURNS: 0 if successful else the return is as follows:
 *		
 *		EINVAL 	if the specified attribute is invalid; usage.
 *		ENOENT 	if the group does not exist.
 *		EACCES 	if the attribute cannot be listed - ie, invoker doesn't
 *			have read_access to the user database.
 *		EPERM 	if the user identification and authentication fails
 *		errno	if system error.
 */  

main(int argc,char *argv[])
{
register int	flag = 0;	/* flag for format			*/
register int	c;		/* counter for getopt()			*/
struct	groups	grps[1];	/* structure for user link list		*/
struct	groups	*grpsp;		/* pointer to increment through list 	*/
char		*delim = SPACE; /* the delimiter value			*/

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	if (argc < 2)
		usage(LSGRPUSAGE);

	/* Open the user database */
	setuserdb( S_READ );

	/* Get the flags */
	while ((c = getopt(argc,argv,"acf")) != EOF)
	{
		switch (c)
		{
		case 'a':	flag |= AFLAG;
				break;

		case 'c':	if (flag & FFLAG)
					usage(LSGRPUSAGE);
				flag |= CFLAG;
				delim = COLON;
				break;

		case 'f':	if (flag & CFLAG)
					usage(LSGRPUSAGE);
				flag |= FFLAG;	
				delim = CARAT;
				break;

		default :	usage(LSGRPUSAGE);
		}
	}

	if (flag & AFLAG)
	{
		/* if no more arguments print usage */
		if (!argv[optind])
			usage(LSGRPUSAGE);

		/* get the group array */
		getgrps(argv[argc - 1],grps);

		/*
		 * Print each users attribute.  This
		 * routine exits.
		 */
		listattrs(grps,delim,optind,argc,argv,flag);
	}


	/* get the group array */
	getgrps(argv[argc - 1],grps);

	/* set up group linked list pointer */
	grpsp = grps;

	do
	{
		listall(grpsp->grp,delim,flag);
		grpsp = grpsp->next_grp;

	} while (grpsp != NULL);

	/* Close the user database */
	enduserdb();

	return(0);
}

/*
 * FUNCTION:	getvalue
 *
 * DESCRIPTION:	get the current attributes.
 *
 * PASSED:	group = the groupname, name = the attribute name, string = the 
 *		attribute value, delim = the default delimiter.
 *
 * RETURNS:	None.
 *
 */

static	void
getvalue(char *group,char *name,char **string,char *delim)
{
char			*val;		/* attribute value		*/
struct	lsgrattr	*ptr;		/* ptr to table			*/
void			*ret;		/* return from check routines	*/
int			err = 1;	/* invalid attribute 		*/

	/*
	 * go through table looking for attribute name
	 */
	for (ptr = lsgratab;ptr < &lsgratab[lsgratabsiz]; ptr++)
	{
		if (!strcmp(name, ptr->gattr))
		{
			err = 0;
			/* get attribute value from data base */
			if (getgroupattr(group,name,&val,ptr->type))
			{
				if (errno != ENOATTR && errno != ENOENT)
					err = 1;
			}
			else
			{
				/* see if value is correct */
				if (ptr->format)
					ret=(void *)(*(ptr->format))(group,val);
				else
					ret = val;

				/*
				 * if we're looking at the users attribute,
				 * add all users that have this group as
				 * their primary group
				 */
				if (strcmp(name, "users") == 0)
					ret = addprimaries(group, ret);

				/* build the new string */
				*string = getnewstring(*string,ptr->gattr,
						delim,ptr->type,ret);

			}
			break;
		}
	}

	/*
	 * if this flag is still set the attribute entered wasn't valid
	 */
	if (err)
	{
		fprintf(stderr,LISTERR,name);
		exit (EINVAL);
	}
}

/*
 * FUNCTION:	getgrps
 *
 * DESCRIPTION:	parses the list of groups passed in, validates the string,
 *		and builds a linked list of struct groups.
 *
 * PASSED:	string = string of comma-separated groups, llist = linked
 *		list of struct groups. 
 *
 * RETURNS:	the number of groups found.
 *
 */

static	int
getgrps(char *string,struct groups *llist)
{
struct	groups	*llistpn;	/* temporary pointer		*/
struct	groups	*llistp;	/* temporary pointer		*/
struct	group	*grp;		/* used for getgrent		*/
int		i = 0;		/* the number of valid groups	*/
char		*ptr;		/* temporary pointer		*/
char		**grplist;	/* list of valid groups		*/
int		j = 0;		/* loop counter			*/
int		siz;
int		id;

	llistp = llist;

	/* if "ALL" get all the groups */
	if (!strcmp(string,ALL))
	{
		setgrent();
		
		while ((grp = (struct group *)getgrent()) != NULL)
		{
			/* Check for group's length < PW_NAMELEN */
			if((siz = (strlen(grp->gr_name) + 1)) > PW_NAMELEN)
			{
				fprintf(stderr,ERRLIST,grp->gr_name);
				fprintf (stderr,TOOLONG);
				errno = ENAMETOOLONG;
				continue;
			}

			if ((ptr = (char *)malloc(siz)) == NULL)
			{
				fprintf(stderr,MALLOC);
				exitx(errno);
			}
			strcpy(ptr,grp->gr_name);
			if (i == 0)
				llistp->grp = ptr;
			else 
			{
				siz = sizeof(struct groups);
				if((llistpn=(struct groups *)malloc(siz))==NULL)
				{
					fprintf(stderr,MALLOC);
					exitx(errno);
				}
				llistp->next_grp = llistpn;
				llistp = llistpn;
				llistp->grp = ptr;
			}
			llistp->next_grp = NULL;
			i++;
		}
		endgrent();
	}

	/* else get all the groupnames passed in */
	else
	{

		/*
		 * Drop all spaces and tabs from string, and 
		 * build a linked list of group names provided
		 * from the command line.  Then check each group
		 * for existence and build the list.
		 */

		grplist = comtoarray(string,&i);


		for (j=0;j<i;j++)
		{
			/* check if group exists locally or in NIS	*/

			set_getgrent_remote(2);	/* compat lookups only	*/
			grp = getgrnam(grplist[j]);
			set_getgrent_remote(1);	/* resume full lookups	*/
			if (grp == (struct group *)NULL)
			{
				fprintf(stderr,GRPNONEX,grplist[j]);
				exit (ENOENT);
			}

			if (j == 0)
				llistp->grp = grplist[j];
			else 
			{
				siz = sizeof(struct groups);
				if ((llistpn=(struct groups *)malloc(siz))
						== NULL)
				{
					fprintf(stderr,MALLOC);
					exitx(errno);
				}
				llistp->next_grp = llistpn;
				llistp = llistpn;
				llistp->grp = grplist[j];
			}
		}
		llistp->next_grp = NULL;
	}

	return(i);
}

/*
 * FUNCTION:	listall
 *
 * DESCRIPTION:	list all attributes found.
 *
 * PASSED:	group = groupname, delim = the default delimiter,
 *			flag = format specifier
 *
 * RETURNS:	None.
 *
 */

static	void
listall(char *group,char *delim,int flag)
{
struct	lsgrattr	*ptr;	/* ptr to table			*/
void		*val;		/* returned attribute value	*/
void		*ret;		/* returned attribute value	*/
char		*string;
register int	siz;

	/* copy in group name and delimiter */
	siz = strlen(group) + strlen(delim) + 1;
	if ((string = (char *)malloc(siz)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}
	strcpy(string,group);
	strcat(string,delim);

	/* go through table looking for attribute name */
	for (ptr = lsgratab;ptr < &lsgratab[lsgratabsiz]; ptr++)
	{
		/* get attribute value out of data base */
		if (!getgroupattr(group,ptr->gattr,&val,ptr->type))
		{
			/* see if it is in the correct form */
			if (ptr->format)
				ret = (void *)(*(ptr->format))(group,val);
			else
				ret = val;

			/*
			 * if we're looking at the users attribute,
			 * add all users that have this group as
			 * their primary group
			 */
			if (strcmp(ptr->gattr, "users") == 0)
				ret = addprimaries(group, ret);

			/* build the new string */
			string = getnewstring(string,ptr->gattr,
					delim,ptr->type,ret);

		}
	}

	/* print in stanza format */
	if (flag & FFLAG)
		printstanza(delim,string);

	/* print in colon format */
	else if (flag & CFLAG)
		printcolon(delim,string);

	/* print as a string */
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
 * PASSED:
 *	struct	groups	*names;  the array of names 
 *	char	*delim;		 the delimiter 
 *	int	num;		 optind from arguments 
 *	int	argc;		 number of arguments from command line
 *	char	*argv[];	 the array of attribute names
 *	int	flag;		 the format flag
 *
 * RETURNS:	No return.
 *
 */

static	void
listattrs(struct groups	*names,char *delim, 
		int num,int argc,char **argv,int flag)
{
struct	groups	*namesp;
register	int	c = 0;
char		*string;
register int	siz;

	namesp = names;
	while (*(namesp->grp))
	{
		siz = strlen(namesp->grp) + strlen(delim) + 1;
		if ((string = (char *)malloc(siz)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}
		strcpy(string,namesp->grp);
		strcat(string,delim);

	   	for(c=num; c<(argc-1); c++)
			getvalue(namesp->grp,argv[c],&string,delim);

		if (flag & FFLAG)
			printstanza(delim,string);
		else if (flag & CFLAG)
			printcolon(delim,string);
		else
			printf("%s\n",string);

		if (namesp->next_grp == (struct groups *)NULL)
			break;

		namesp = namesp->next_grp;
	}
	exit (0);
}

/*
 * FUNCTION:	addprimaries
 *
 * DESCRIPTION:	add users whose primary group is 'group' to the comma
 *		separated list 'list'
 *
 * RETURNS:	The new list.
 *
 */

static void *
addprimaries(char *group, void *list)
{
char *newlist;
struct list *l;
struct group *gr;
gid_t gid;

	/*
	 * get the gid of this group
	 */
	if ((gr = getgrnam(group)) == NULL)
		return (list);
	gid = gr->gr_gid;

	newlist = (char *) list;

	if (head != NULL)
	{
		/*
		 * if list already exists, just use it
		 */
		for (l = head; l != NULL; l = l->next)
			if (l->gid == gid)
				newlist = addprim(newlist, l->name);
	}
	else
	{
		/*
		 * else read all passwd entries, create cached list and
		 * add to primary list
		 */
		struct list **tail;
		struct passwd *pw;

		setpwent();

		tail = &head;
		while ((pw = getpwent()) != NULL)
		{
			/*
			 * new node in cache
			 */
			if ((l = (struct list *)malloc(sizeof(*l))) == NULL)
			{
				fprintf(stderr,MALLOC);
				exitx(errno);
			}
			l->name = strdup(pw->pw_name);
			l->gid = pw->pw_gid;

			/*
			 * link it in
			 */
			*tail = l;
			tail = &l->next;

			/*
			 * add to primary list
			 */
			if (l->gid == gid)
				newlist = addprim(newlist, l->name);
		}
		*tail = NULL;

		endpwent();
	}

	return (newlist);
}

/*
 * FUNCTION:	addprim
 *
 * DESCRIOTION: add 'name' to primary list 'oldlist' and return new list
 *
 * RETURNS:	return the new list
 *
 */
static char *
addprim(char *oldlist, char *name)
{
char *newlist;
char *beg, *end;

	if (oldlist == NULL || *oldlist == '\0')
		/*
		 * oldlist appears to be empty.  create it
		 */
		newlist = strdup(name);

	else 
	{
		/*
		 * extend it
		 */
		size_t newsize = strlen(oldlist) + strlen(name) + 2;

		/*
		 * search the current list.  make sure this user
		 * isn't already in the list...
		 */
		for (beg = oldlist, end = strchr(beg, ','); beg != NULL;
		     beg = end ? ++end : NULL, end = strchr(end, ','))
		{
			if (end)
				*end = '\0';		/* zap comma */
			if (strcmp(beg, name) == 0)
			{
				if (end)
					*end = ',';	/* restore */
				break;
			}
			if (end)
				*end = ',';		/* restore */
		}

		if ((newlist = (char *)malloc(newsize)) == NULL)
		{
			fprintf(stderr,MALLOC);
			exitx(errno);
		}
		if (beg == NULL)
		{
			strcpy(newlist,oldlist);
			strcat(newlist,COMMA);
			strcat(newlist,name);
		}
		else
			newlist = oldlist;
	}

	return (newlist);
}
