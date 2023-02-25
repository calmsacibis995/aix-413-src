static char sccsid[] = "@(#)49	1.12.1.6  src/bos/usr/sbin/mkpasswd/mkpasswd.c, cmdsuser, bos41B, 412_41B_sync 12/19/94 16:49:05";
/*
 *   COMPONENT_NAME: CMDSUSER
 *
 *   FUNCTIONS: MSGSTR
 *		catch
 *		del_file
 *		fgetpwent
 *		get_lock
 *		interpret
 *		main
 *		pwskip
 *
 *   ORIGINS: 26,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";

#include <sys/file.h>
#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <locale.h>
#include <fcntl.h>
#include <signal.h>
#include "tcbauth.h"

static	int	parse_error = 0;
char	buf[BUFSIZ];
char	*lock_name = "/etc/locks/LCK.mkpasswd";

struct	passwd	*fgetpwent();
nl_catd catd;
#define MSGSTR(Num, Str) catgets (catd, MS_TCBAUTH, Num, Str)

/*
 *
 * NAME:     mkpasswd
 *                                                                    
 * FUNCTION: Creates a hashed look-aside version of passwd file
 *                                                                    
 * USAGE:    mkpasswd [ -v ] PasswordFile
 * 	     where:
 *			-v	     : indicates verbose.
 *			PasswordFile : is a file in the format of 
 *					   /etc/passwd file
 *
 *	the invoker must be root or a member of the security group.
 *                                                                   
 * 	RETURNS: 0 if successful, or error.
 *		
 */  

main(argc, argv)
int	argc;
char	*argv[];
{
	DBM	*dp;
	FILE	*fp;
	struct stat	stat;
	volatile datum	key;
	volatile datum	content;
	char	*cp;
	char 	*acl, *acl_fget();
	struct	passwd	*pwd;
	int	verbose = 0;
	int	entries = 0;
	int	maxlen = 0;
	char 	*tmpname, *dirfile, *pagfile, *newdirfile, *newpagfile;
	int	arg_len;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_TCBAUTH, NL_CAT_LOCALE);

	/*
	 * The only valid option is `verbose'.  Test for it and
	 * shift the remaining arguments if it is present.
	 */

	if (argc > 1 && strcmp(argv[1], "-v") == 0) {
		verbose++;
		argv++, argc--;
	}

	/*
	 * There can be no remaining arguments.  The only option
	 * left is the name of the look-aside file.
	 */

	if (argc != 2) {
		fprintf (stderr, MSGSTR (M_MKPASSWD, DEF_MKPASSWD));
		exit (EINVAL);
	}

	/*
	 * See if the file exists already.  If the file is non-existent
	 * or can't be read, tell the user what's wrong.
	 */

	if (access (argv[1], R_OK) < 0) {
		fprintf (stderr, MSGSTR (M_NOPASSWD, DEF_NOPASSWD), argv[1]);
		exit (ENOENT);
	}

	/*
	 * Create the database files afresh.  Set the umask to 0
	 * to override any funny business the user might be up to.
	 * Use a temporary database file, so that ls and other programs
	 * could still use the old database while this program is running.
	 * When updating the new database is done, we rename the new temporary
	 * files.
	 */

	umask(0);
	arg_len = strlen(argv[1])+1;

	/* allocate space for tempory file */
	if ((tmpname = (char *)malloc (arg_len+5)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}
	sprintf(tmpname, "%s%05d", argv[1], (int)getpid());

	/* allocate space for new .dir file */
	if ((newdirfile = (char *)malloc (arg_len+9)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}
	strcpy(newdirfile,tmpname);
	strcat(newdirfile,".dir");

	/* allocate space for new .pag file */
	if ((newpagfile = (char *)malloc (arg_len+9)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}
	strcpy(newpagfile,tmpname);
	strcat(newpagfile,".pag");

	/* allocate space for .dir file */
	if ((dirfile = (char *)malloc (arg_len+4)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}
	strcpy(dirfile,argv[1]);
	strcat(dirfile,".dir");

	/* allocate space for .pag file */
	if ((pagfile = (char *)malloc (arg_len+4)) == NULL)
	{
		fprintf(stderr,MALLOC);
		exitx(errno);
	}
	strcpy(pagfile,argv[1]);
	strcat(pagfile,".pag");

	dp = dbm_open(tmpname, O_RDWR|O_CREAT|O_TRUNC, 0600);
	if (dp == NULL) {
		fprintf (stderr, MSGSTR (M_NOPWDDBM, DEF_NOPWDDBM), argv[1]);
		exit(ENOENT);
	}

	/*
	 * Open the user's local password file
	 */

	if ((fp = fopen (argv[1], "r")) == NULL) {
		del_file(newdirfile, newpagfile);
		fprintf (stderr, MSGSTR (M_NOPASSWD, DEF_NOPASSWD), argv[1]);
		exit (ENOENT);
	}

	/*
	 * Now set ACLs on the two database files.
	 * we set them the same as the flat file when it's possible
	 * to set ownership too.
	 */
	if ((acl = acl_fget(fileno(fp))) == NULL 
	|| fstat(fileno(fp), &stat) != 0
	|| acl_fput(dp->dbm_dirf, acl, 0) != 0
	|| acl_fput(dp->dbm_pagf, acl, 1) != 0
	|| fchown(dp->dbm_dirf, stat.st_uid, stat.st_gid) != 0
	|| fchown(dp->dbm_pagf, stat.st_uid, stat.st_gid) != 0 ) {
	    acl_fset(dp->dbm_dirf, 6, 4, 4);
	    acl_fset(dp->dbm_pagf, 6, 4, 4);
	}


	/*
	 * Read the entire file and cram all of the fields into a
	 * single DBM block.  Strings are NUL separated from each
	 * other.  Binary objects, such as uids and gids, are
	 * stored directly in an unaligned fashion.
	 */

	while (pwd = fgetpwent (fp)) {
		cp = buf;

		strcpy (cp, pwd->pw_name);
		cp = (char *)strchr (cp, '\0') + 1;

		strcpy (cp, pwd->pw_passwd);
		cp = (char *)strchr (cp, '\0') + 1;

		bcopy ((char *)&pwd->pw_uid, cp, sizeof (uid_t));
		cp += sizeof (uid_t);

		bcopy ((char *)&pwd->pw_gid, cp, sizeof (gid_t));
		cp += sizeof (gid_t);

		strcpy (cp, pwd->pw_gecos);
		cp = (char *)strchr (cp, '\0') + 1;

		strcpy (cp, pwd->pw_dir);
		cp = (char *)strchr (cp, '\0') + 1;

		strcpy (cp, pwd->pw_shell);
		cp = (char *)strchr (cp, '\0') + 1;

		/*
		 * Tell the user what we are up to ...
		 */

		if (verbose)
			printf (MSGSTR (M_DSTORE, DEF_DSTORE),
				pwd->pw_name, pwd->pw_uid);

		/*
		 * Build the values for the key and content structures
		 * to be handed to DBM.  Each record is stored twice -
		 * once by its name and again by its UID.
		 */

		content.dptr = buf;
		content.dsize = cp - buf;

		/*
		 * Create a name to entry mapping
		 */

		key.dptr = pwd->pw_name;
		key.dsize = strlen (pwd->pw_name);

		if (dbm_store (dp, key, content, DBM_REPLACE) < 0) {
		        del_file(newdirfile, newpagfile);
			fprintf (stderr,
				MSGSTR (M_PWDBMFAIL, DEF_PWDBMFAIL), argv[1]);
			exit (ENOENT);
		}

		/*
		 * Create another entry for the UID mapping in the file.
		 */

		key.dptr = (char *) &pwd->pw_uid;
		key.dsize = sizeof (int);

		if (dbm_store (dp, key, content, DBM_INSERT) < 0) {
		        del_file(newdirfile, newpagfile);
			fprintf (stderr,
				MSGSTR (M_PWDBMFAIL, DEF_PWDBMFAIL), argv[1]);
			exit (ENOENT);
		}

		/*
		 * Tally up the entries and update the long entry content.
		 */

		entries++;
		if (cp - buf > maxlen)
			maxlen = cp - buf;
	}

	/*
	 * Close the password file and exit.
	 */

	dbm_close (dp);

	if (parse_error) {
		fprintf (stderr, MSGSTR(M_PWDBADFORM, DEF_PWDBADFORM),
				argv[1], parse_error);
		exit(EIO);
	}

	/*
	 * Make certain that the rename of the files appears to be
	 * atomic, more or less.  If there are multiple instances of
	 * mkpasswd running, one could rename a file at the same time
	 * another command is renaming the other.
	 */

	if (get_lock ()) {
		fprintf (stderr, MSGSTR (M_NOPWDDBM, DEF_NOPWDDBM), argv[1]);
		del_file (newdirfile, newpagfile);
		exit (1);
	}

	/*
	 * Rename both files to their new names.  Exiting will release
	 * the lock.
	 */

	rename(newdirfile, dirfile);
	rename(newpagfile, pagfile);
	printf (MSGSTR (M_MKPWDFINI, DEF_MKPWDFINI), entries, maxlen);
	exit (0);
}

/*
 * FUNCTION:	del_file
 *
 * DESCRIPTION:	deletes the .dir file and the .pag file
 *
 * RETURNS: 	No return.
 *
 */

del_file(dir, pag)
char *dir, *pag;
{
    unlink(dir);
    unlink(pag);
}

/*
 * BEGIN BSD CODE
 */

/*
 * FUNCTION:	pwskip
 *
 * DESCRIPTION:	skips past one field in an entry of the /etc/passwd file
 *
 * RETURNS: 	returns pointer to next field
 *
 */

char *
pwskip(p)
register char *p;
{
	while(*p && *p != ':' && *p != '\n')
		++p;
	if (*p == '\n')
		*p = '\0';
	else if (*p != '\0')
		*p++ = '\0';
	return(p);
}

/*
 * FUNCTION:	interpret
 *
 * DESCRIPTION:	turns a line of /etc/passwd into a passwd structure.
 *
 * RETURNS: 	address of new passwd structure
 *
 */

struct passwd *
interpret (val)
char	*val;
{
	char	*p;
	char	*end;
	long	x;
	static	struct	passwd	interppasswd;
	static	char	interpline[BUFSIZ];
	static	int	lineno = 0;

	parse_error = ++lineno;

	(void) strncpy(interpline, val, BUFSIZ);
	p = interpline;

	interppasswd.pw_name = p;
	p = pwskip(p);

	interppasswd.pw_passwd = p;
	p = pwskip(p);

	if (*p == ':')
		return (NULL);
	else
		x = strtoul(p, &end, 10);       
	p = end;

	if (*p++ != ':')
		return (NULL);
	else
		interppasswd.pw_uid = x;

	if (*p == ':')
		return (NULL);
	else
		x = strtoul(p, &end, 10);	
	p = end;
	if (*p++ != ':')
		return (NULL);
	interppasswd.pw_gid = x;

	interppasswd.pw_gecos = p;
	p = pwskip(p);

	interppasswd.pw_dir = p;
	p = pwskip(p);

	interppasswd.pw_shell = p;
	while(*p && *p != '\n')
		p++;

	*p = '\0';
	parse_error = 0;
	return &interppasswd;
}

/*
 * END BSD CODE
 */

/*
 * FUNCTION:	fgetpwent
 *
 * DESCRIPTION:	gets a line from /etc/passwd
 *
 * RETURNS: 	A passwd structure of the line in /etc/passwd
 *
 */

struct passwd *
fgetpwent (fp)
FILE	*fp;
{
	char	buf[BUFSIZ];
	char	*cp;

	if (! fp)
		return 0;

	if (! fgets (buf, BUFSIZ, fp))
		return 0;

	if (cp = (char *)strrchr (buf, '\n'))
		*cp = '\0';
	else
		buf[BUFSIZ-1] = '\0';

	return interpret (buf);
}

/*
 * FUNCTION:	catch
 *
 * DESCRIPTION:	handles the SIGALRM signal
 *
 * RETURNS: 	None.
 *
 */

void
catch (int sig)
{
}

/*
 * FUNCTION:	get_lock
 *
 * DESCRIPTION:	acquires a lock on a file.  When this lock is acquired,
 *		the process holding the lock has exclusive permission
 *		to move the .dir and .pag files into their new positions.
 *
 * RETURNS: 	0 if successful, or error.
 *
 */

int
get_lock ()
{
	int	fd;
	pid_t	pid;
	struct	flock	lock;

	if ((fd = open (lock_name, O_RDWR)) == -1)
		if ((fd = open (lock_name,
				O_RDWR|O_CREAT|O_EXCL, 0666)) ==-1)
			return -1;

	lock.l_type = F_WRLCK;
	lock.l_len = 0;
	lock.l_start = 0;
	lock.l_whence = 0;

	signal (SIGALRM, catch);
	alarm (60);

	if (fcntl (fd, F_SETLKW, &lock) == 0)
	{
		pid = getpid ();
		write (fd, &pid, sizeof pid);
		return 0;
	}
	return -1;
}
