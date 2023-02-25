static char sccsid[] = "@(#)23	1.1.1.5  src/bos/usr/sbin/tsm/tsm_su_util.c, cmdsauth, bos41J, 9522A_all 5/30/95 15:47:32";
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS: tsm_authenticate, tsm_iscurr_console
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <login.h>
#include <unistd.h>
#include <userconf.h>
#include <userpw.h>
#include <usersec.h>
#include <sys/audit.h>
#include <sys/console.h>
#include <sys/errno.h>
#include <sys/id.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include "tsm.h"

int	login_restricted = 1;

/*
 * NAME: tsm_iscurr_console
 *
 * FUNCTION: Determines if the passed in port is the current console.
 *
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  NOTES:
 *	If the passed in port is the current console or NULL, then return 0 and
 *	copy the name of the current console (currently always /dev/console)
 *	into the passed in variable console if not NULL, otherwise return -1.
 *
 *  RETURNS:
 *	0 if port is the current console, -1 otherwise.
 */

int
tsm_iscurr_console( char * port, char * console)
{
	struct stat stat_buf;
	struct qry_devsw qdevsw;
	struct cons_config config;
	struct cfg_dd cfgdata;		/* Config DD			*/
	char console_port[256];		/* Name of console		*/
	int getport = FALSE;		/* Does the caller have match
					   or NULL			*/
	int return_value = -1;

	/*
	 * ensure console is there
	 */
	if ( stat(TSM_CONSOLE, &stat_buf) == 0) {
		if (S_ISCHR(stat_buf.st_mode))
		{
			/*
			 * If the console is there and a character special
			 * device, query the system for the physical name of
			 * the console.
		 	 */

			qdevsw.devno = stat_buf.st_rdev;
			sysconfig (SYS_QDVSW, &qdevsw, sizeof(qdevsw));
			if (qdevsw.status & DSW_CONSOLE)
			{
				cfgdata.kmid = 0;
				cfgdata.devno = stat_buf.st_rdev;
				cfgdata.cmd = CONSOLE_CFG;
				cfgdata.ddsptr = (char *) (&config);
				cfgdata.ddslen = sizeof(config);
				config.cmd = CONSGETCURR;
				config.path = console_port;
				if(sysconfig( SYS_CFGDD, &cfgdata,
				    sizeof(struct cfg_dd)) ==0 ) {
					/*
					 * If the passed in port is the same as
					 * the console or NULL return 0 and
					 * copy its logical name (which is
					 * always /dev/console now) into
					 * console, if not NULL.
					 */

					if (port != NULL)  {
						if (strcmp (port,
						    console_port) == 0) {
							getport = TRUE;
						}
					}
					if (port == NULL || getport) {
						return_value = 0;
						if (console)  {
							strcpy (console,
							        TSM_CONSOLE);
						}
					}
				}
			}
		}
	}
	return (return_value);
}

/*
 * NAME: sysauth()
 *
 * FUNCTION: Call authenticate() to provide system authentication of the user.
 *
 * EXECUTION ENVIRONMENT: static
 *
 * RETURNS:   1 - authentication not successful
 *            0 - authentication successful
 */
static int
sysauth(char *username, char *shortname, char **opasswd)
{
	int	rc;			 /* return code 		      */
	int 	reenter;		 /* recall authenticate()             */
	uid_t	uid;		 	 /* User id of target user	      */
	uid_t	curuid;			 /* Current user id		      */
	char 	*op	 = (char *)NULL; /* Pointer to user's old password    */
	char    *tp      = (char *)NULL; /* temporary pointer 		      */
	char	*message = (char *)NULL; /* return message from authenticate()*/
	char	passwd[MAX_PASS+1];	 /* user's cleartext password         */
	char    npasswd[MAX_PASS+1];	 /* user's new cleartext password     */
	
	/*
	 * Call authenticate.  If reenter is not set then this user
	 * is not required to enter a password.  Either the user is not
	 * a valid user, has no password, or has no authentication 
	 * requirements.  Otherwise display the challenge and gather
	 * the password.
	 */ 
	do
	{
		rc = authenticate(username, tp, &reenter, &message);
		if (message)
		{
			if (reenter)
			{
				readpasswd(message, passwd);
				tp = passwd;
			}
			else
			{
				/*
				 * If user passed authentication then it is
				 * ok to give him informative messages.  
				 */
				if (!rc)
					puts(message);
			}

			free(message);
			message = (char *)NULL;
		}
	} while (reenter);

	/* Check login_restricted before calling tsm_chpass to see if
	 * the user's password is expired or need's changing.  Cannot
	 * let the user change passwords if loginrestrictions fails.
	 */

	*opasswd = strdup(tp);	/* Point to password authenticated with */
	if (!(login_restricted || rc)) {
		rc = tsm_chpass(username, shortname, *opasswd);
	}

	if (**opasswd)
		memset(*opasswd, 0, strlen(*opasswd));

	return(rc);
}

/*
 * NAME: getauthprog
 *
 * FUNCTION: gets the authentication program from /etc/security/login.cfg
 *           and converts it into a form which execvp will take.
 *
 * EXECUTION ENVIRONMENT: static
 *
 * RETURNS: the arguments or NULL.
 *
 * PASSED:
 *	char 	*methname; stanza name of the authentication method to run
 *	char	*username; user whom we are authenticating
 */  
static char **
getauthprog(char *methname, char *username)
{
	char	*tp;
	int	argc = 3;
	int	siz;
	char	**args = NULL;
	char	*prog;
	char	*pp;
	register int	i;

	/*
	 * get the attribute from /etc/security/login.cfg
	 * methname:
	 *	program = "something"
	 */

	if (getconfattr (methname,SC_AUTHPROGRAM, (void *) &tp, SEC_CHAR))
		return ((char **)NULL);

	if (*tp)
	{
		if ((prog = malloc(strlen(tp) + 2)) != NULL)
		{
			pp = prog;
		
			/* convert string into double-null terminated buffer */
			while (*tp)
			{
				/* skip the string indicators */
				if (*tp == '"')
					tp++;

				/* this is another argument */
				if (*tp == ' ')
				{
					*pp++ = '\0';
					argc++;
				}
				else
					*pp++ = *tp;
				tp++;
			}
			*pp++ = '\0';
			*pp = '\0';
			siz = argc * sizeof(char *);
			if ((args = (char **)malloc(siz)) != NULL)
			{
				/*
				 * build arguments for program
				 * username is last argument
				 */
				for (i = 0;; i++)
				{
					if (!(*prog))
					{
						args[i++] = username;
						args[i] = NULL;
						break;
					}
					args[i] = prog;
					while (*prog++)
						;
				}
			}
		}
	}
	return (args);
}

/*
 * NAME: authmethods
 *                                                                    
 * FUNCTION:
 *	The methods parameter is a DNL string of the form 'method;name'.
 *	Each method is 'invoked' with the associated user.
 *		"method;user\0method;user\0 ... \0\0"
 *
 *	If the 'primary' parameter is set then the first failed
 *	method will fail the authentication. If 'primary' is not
 *	set (secondary) then each method will be 'invoked' regardless
 *	of the value of the previous method.
 *                                                                   
 *	Methods are one of:
 *		SYSTEM   - the typical UNIX authentication method with 
 *		           passwords.  This method has been expanded to
 *			   include load modules and login rules.  Reference
 *			   the authenticate routine.
 *		NONE     - no authentication specified
 *		token	 - an administrator defined token which is described
 *			   in login.cfg 
 *
 * EXECUTION ENVIRONMENT: static 
 *				 
 * RETURNS: 
 *		0 if authentication successful and 1 if not 
 *
 * PASSED:
 *	char	*authuser;	 the user being authenticated
 *	char    *shortname;	 User's shortened (max 8 bytes) name
 *	char	*methods;	 authentication methods for above user
 *	int	primary; 	 indicating primary or secondary auth method
 */  
static int
authmethods (char *authuser, char *shortname, char *methods, int primary, char **passwd)
{
	int	rc;		/* return code */
	char	*user;		/* user whose method is being used */
	char	*meth;		/* a method of authentication from 'methods'*/
	char	*mp;		/* temporary pointer for auth methods */
	int	donesysauth = 0;/* Signal variable for system authentication */

	/*
	 * parse the methods list
	 * method;user\0method2;user2...\0\0
	 */

	while (*methods)
	{
		/* get some space and copy in the methods string */
		if ((meth = malloc (strlen (methods) + 1)) == NULL)
		{
			rc = 1;
			break;
		}
		strcpy (meth, methods);
		mp = meth;

		/* look for the end of the string or a ";" */
		while (*mp && (*mp != ';'))
			mp++;

		/*
		 * Get the user out of the methods list.  The string 
		 * should be: method;user.  Otherwise set the user to
		 * the name being authenticated.
		 */
		if (*mp == ';')
		{
			*mp++ = '\0';
			user = mp;
		}
		else
			user = authuser;

		/*
		 * increment methods pointer to catch up to where 
		 * we are now in the meth string.
		 */
		while (*methods++);

		/* 
		 * If the method is "SYSTEM" do password authentication.
		 * If the method is "NONE" then no authentication required.
		 * Otherwise the administrator has his/her own method and
		 * we have to exec it.
		 */
		if (strcmp (meth, "SYSTEM") == 0)
		{
			char	sname[PW_NAMELEN];
			_normalize_username(sname,user);
			rc = sysauth(user, sname, passwd);
			donesysauth = 1;	/* sysauth() complete */
		}
		else if (strcmp (meth, "NONE") == 0)
			rc = 0;
		else
		{
			char	**authprog;

			authprog = getauthprog (meth, user);
			if (authprog)
			{
			int	child;
			int	pid;
			int	status;

				if ((child = kfork()) == 0)
				{
					execvp (authprog[0], &authprog[0]);
					_exit (-1);
				}
				while ((pid = wait (&status)))	
				{
					if (child == pid)
					{
						if (status & 0x00FF)
							/* due to signal */
							rc = status & 0x00FF;
						else
							/* return exit code */
							rc = status & 0xFF00;
						break;
					}
				}
			}
			else
			{
				/* methods not found but expected */
				rc = EINVAL;
			}
		}

		/* free up space from earlier malloc */
		free ((void *)meth);

		/* if we are checking primary and have failed return */
		if (primary && rc)
			break;

	}

	/*
	 * sysauth() implements the system password authentication.  This
	 * implementation is based on the new SYSTEM grammar used by 
	 * authenticate().  If we have arrived here and have yet to process
	 * sysauth(), then the user has explicity removed "SYSTEM" from the
	 * auth1 line or stated "NONE".  Since the SYSTEM grammar is the 
	 * preferred authentication means (and obsoletes auth1/auth2),
	 * the user must explicitly cut off the SYSTEM grammar as well
	 * in order to bypass sysauth().  DON'T CHANGE THIS CODE POLICY.
	 */
	if (!donesysauth && primary && !rc)
		rc = sysauth(authuser, shortname, passwd);
		
	return (rc);
}

/*
 * NAME:     tsm_authenticate
 *
 * FUNCTION: Invokes both the Primary and Secondary authentication code.
 *
 * EXECUTION ENVIRONMENT: static
 *
 * RETURNS:   0 - Success.
 *           -1 - Failure with errno set.
 *
 */  
int
tsm_authenticate(char *username, int mode, char **passwd)
{
	char *methods;
	char shortname[PW_NAMELEN];

	char krb_blank[12] = "KRB5CCNAME=";
	char *krb_old = NULL;
	char *krb_new = NULL;
	int  env_rc = 0;

	/* Clear out KRB5CCNAME variable to make sure it is not inherited
	 * from any previous process or authentication attempt.
	 */

	krb_old = getenv("KRB5CCNAME");
	if (krb_old)
	{
		krb_new = strdup(krb_blank);
		if (krb_new)
		{
			env_rc = putenv(krb_new);
			if (env_rc)
				free(krb_new);
		}
	}

	if(!username || !((mode & S_PRIMARY) || (mode & S_SECONDARY)))
	{
		errno = EINVAL;
		return(-1);
	}

	_normalize_username(shortname, username);

	if(mode & S_PRIMARY)
	{
		if(getuserattr(shortname, S_AUTH1, (void *)&methods, SEC_LIST))
			methods = "SYSTEM";

		if(authmethods(username, shortname, methods, 1, passwd))
		{
			errno = ESAD;
			return(-1);
		}
	}

	if(mode & S_SECONDARY)
	{
		if(getuserattr(shortname, S_AUTH2, (void *)&methods, SEC_LIST))
			return(0);

		if(methods && *methods)
			authmethods(username, shortname, methods, 0, passwd);
	}

	return(0);
}


/*
 * NAME:     tsm_chpass
 *
 * FUNCTION: Requires users to change their passwords (if necessary)
 *           at login time.
 *
 * EXECUTION ENVIRONMENT: static
 *
 * RETURNS:     0     - Success.
 *           non-zero - Failure.
 *
 */  
int
tsm_chpass(char *lname, char *sname, char *opasswd)
{
	int curuid, uid, ret = 0, reenter;
	char passwd[MAX_PASS+1], *tpasswd, *msg = (char *) NULL;

	curuid = getuid();
	if(!getuserattr(sname, S_ID, &uid, SEC_INT))
	{
		setuidx(ID_REAL|ID_EFFECTIVE, uid);
		setuidx(ID_EFFECTIVE, curuid);
	}

	/*
	 * If the SYSTEM authentication is not being used (i.e.,
	 * "SYSTEM = NONE"), then the passwords should not 
         * be checked.
	 */
	if (!SYSTEM_is_NONE(sname))
		ret = passwdexpired(lname, &msg);

	if(msg)
	{
		puts(msg);
		free(msg);
		msg = (char *) NULL;
	}

	if((ret == 1) || (ret == 2 && uid == 0))
	{
		do
		{
			tpasswd = opasswd;
			do
			{
				ret = chpass(lname, tpasswd, &reenter, &msg);

				if(msg)
				{
					if(reenter)
					{
						readpasswd(msg, passwd);
						tpasswd = passwd;
					}
					else
						puts(msg);

					free(msg);
					msg = (char *) NULL;
				}
			} while(reenter);
		} while(ret == 1);

		auditwrite("PASSWORD_Change", ret ? AUDIT_FAIL : AUDIT_OK,
			   sname, strlen(sname) + 1, NULL);
	}
	/*
	 * Let root onto the system when there's an internal error
	 * in passwdexpired().  (NOTE: root has already authenticated.)
	 */
	else if(ret == -1 && uid == 0)
		ret = 0;

	memset(passwd, 0, sizeof(passwd));
	setuidx(ID_REAL|ID_EFFECTIVE, curuid);

	return(ret);
}
