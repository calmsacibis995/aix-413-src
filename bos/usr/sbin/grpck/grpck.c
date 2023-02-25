static char sccsid[] = "@(#)14  1.17.1.2  src/bos/usr/sbin/grpck/grpck.c, cmdsadm, bos411, 9428A410j 2/23/94 09:43:19";
/*
 * COMPONENT_NAME: (CMDSADM) security: system administration
 *
 * FUNCTIONS: grpck
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/audit.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <usersec.h>
#include <userpw.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/flock.h>
#include <locale.h>
#include <string.h>
#include "grpck_msg.h"
#include "grpck.h"

#define	MAXGRP	200

struct table {
	char *name;
	char *id;
	int test;
};

struct table *groups;
int ngroups;

struct table *secgroups;
int nsecgroups;

struct stat orig_etc;
struct stat current_etc;
struct stat orig_sec;
struct stat current_sec;
struct stat pw_pag;

char *dup_string();

int verbose;
int fixit;
int query;
int all = 0;

int pflag = 0;         		/* argv flag settings:   -p             */
int yflag = 0;                	/*                       -y             */   
int nflag = 0;                	/*                       -n             */
int tflag = 0;                	/*                       -t             */
int errors = 0;

#ifdef _NO_PROTO
	char *grpckcatgets();
#else
	char *grpckcatgets(int, char *);
#endif

main (argc,argv)
int argc;
char **argv;
{
	FILE etcgrp, secgrp;
	int  flag;
	int  i,j;
	char *gname;
	int  group_errors = 0;
	extern	int	optind;		/* tabindex of argument after flags */

	auditproc (0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
	setlocale (LC_ALL, "");

	/*
	 * Begin by checking the arguments and setting up a pointer to
	 * the list of groups to check.
	 */

	while ((flag = getopt (argc, argv, "pynt")) != EOF) {
		switch (flag) {
			case 'p':
				verbose = 0;
				fixit = 1;
				pflag++;
				break;
			case 'y':
				verbose = 1;
				fixit = 1;
				yflag++;
				break;
			case 'n':
				verbose = 1;
				fixit = 0;
				nflag++;
				break;
			case 't':
				verbose = 1;
				query = 1;
				tflag++;
				break;
			default:
				usage();
		}
	}

	/* 
	 * Validate the values for pflag, yflag, nflag, and tflag.
	 * Also check that at least one user argument is supplied.
	 */

	if (pflag + yflag + nflag + tflag != 1 ||  optind >= argc)
		usage ();

	/* 
	 * Open user database for reading; if fixit, then write also
	 */

	if (setuserdb (fixit || query ? (S_READ|S_WRITE):(S_READ)) != 0)
		exit (EACCES);	

	/* 
	 * Read the entire /etc/group file worth of groups into a table for 
	 * testing.  The groups to check are flagged for later reference. 
	 * Then read in the entire /etc/security/group file for administrator
	 * checking.
	 */

	init_groups (&argv[optind]);

	/* 
	 * Every group to be tested has been flagged.  Loop through the
	 * entire table and perform all of the required tests on the group.
	 */

	for (i=0;i<ngroups;i++) {

		gname = groups[i].name;

		/*
		 * Remove this group's entry from the /etc/security/group
		 * table.  The only entries which will remain in the
		 * /etc/security/group table will be those which do not
		 * have a corresponding entry in /etc/group.
		 */

		for (j = 0; j < nsecgroups; j++) {
			if (strcmp(secgroups[j].name,gname) == 0) {
				secgroups[j].test = 0;
				break;
			}
		}

		/*
		 * See if we are testing this group.
		 */

		if (! groups[i].test || *groups[i].name == '+'
		   || *groups[i].name == '-' )
			continue;

		/*
		 * Check the validity and uniqueness of group name .  Print
		 * diagnostic message if a duplicate is found, but nothing more.
		 */

		if (j = ck_name (gname, i + 1)) {
			if (j==ENAMETOOLONG) groups[i].test = 0;
			group_errors++;
			errors++;
		}

		/*
		 * Check the validity and uniqueness of group id. Print 
		 * diagnostic message if a duplicate is found, but nothing more.
		 */

		if (ck_gid (groups[i].id, gname)) {
			group_errors++;
			errors++;
		}

		/*
		 * Check validity of remaining data for non-null group names
		 */

		if (*gname != '\0') {

			/*
			 * Check regular users for existence.  Delete any
			 * non-existent users.
			 */

			if (ck_users (gname)) {
 				group_errors++;
 				if (ck_query (MSGSTR (M_FIXUSERS, DEF_FIXUSERS),
					gname)) 
 					fix_users (gname);
 				errors++;
 			}

	 		/*
 			 * Check admin status for group.
 			 */

			if (ck_admin (gname)) {
 				group_errors++;
 				errors++;
 		  	}
 
 			/*
 			 * Check administrative users for existence.
			 * Delete any non-existent users.
 			 */

			if (ck_adms (gname)) {
 				group_errors++;
 				if (ck_query (MSGSTR (M_FIXAGRPS, DEF_FIXAGRPS),
					gname)) 
 					fix_adms (gname);
 				errors++;
 		  	}
		}
	}

	/*
	 * Any groups in the /etc/security/group table
	 * that still have test = 1, must not have a
	 * corresponding stanza in /etc/group.
	 */

	for (j = 0; j < nsecgroups; j++) {
		if (secgroups[j].test == 1) {
			errors++;
			if (verbose)
				fprintf (stderr,
					MSGSTR (M_NOETCGRP, DEF_NOETCGRP),
					secgroups[j].name);
		}
	}

	if ((fixit || query)  && group_errors) {
		/*
		 * Check current status of file
		 */
		stat("/etc/group",&current_etc);
		stat("/etc/security/group",&current_sec);
		if (current_sec.st_mtime != orig_sec.st_mtime ||
		    current_etc.st_mtime != orig_etc.st_mtime) {
 			if (!ck_query (MSGSTR(M_OVERWRITE,DEF_OVERWRITE)))
				exit(-1);
		}

		/*
		 * Commit changes to database
		 */

		for (i=0;i<ngroups;i++) {
			if (*groups[i].name != 0 && groups[i].test == 1) {
 				if (putgroupattr
				   (groups[i].name,NULL,NULL,SEC_COMMIT))
					if (ck_query
					   (MSGSTR (M_BADPUT,DEF_BADPUT),
						groups[i].name)) {
						perror("  ");
						exit(errno);
					}
			}
		}
 	}

	/*
	 * Now see if the password file is newer than the look-aside
	 * files.  This will cause a warning message with no action
	 * taken.
	 */

	if (stat ("/etc/passwd", &current_etc) == 0 &&
	    stat ("/etc/security/passwd", &current_sec) == 0 &&
	    stat ("/etc/passwd.pag", &pw_pag) == 0) {

		if (pw_pag.st_mtime < current_sec.st_mtime ||
		    pw_pag.st_mtime < current_etc.st_mtime) {
			errors++;
			fprintf (stderr, MSGSTR (M_OLDDBM, DEF_OLDDBM));
		}
	}

	enduserdb();

	/*
	 * Exit with EINVAL if any errors were seen.
	 */

	if (errors)
		exit(ENOTRUST);
	exit(0);
} 

/*
 * NAME: ck_name
 *
 * FUNCTION: Scan for duplicate instances of ' group name'
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Called once for each user selected for testing.
 *
 * RETURNS: Zero if name occurs exactly once, non-zero otherwise.
 */

int
ck_name (name, line)
char	*name;
int	line;
{
	int	i,rc;
	int	count = 0;

	/*
	 * Validate the group name.  It must begin with an alphabetic
	 * character and contain nothing but alphanumerics afterwards.
	 */

	if (strcmp (name, "ALL") == 0 || strcmp (name, "default") == 0) {
		if (verbose)
			fprintf (stderr,
				MSGSTR (M_BADNAME, DEF_BADNAME), name);
		return -1;
	}

	if (*name == '\0') {
		if (verbose)
			fprintf (stderr,
				MSGSTR (M_NULLNAME, DEF_NULLNAME), line);
		return -1;
	}

	if (rc = ck_char(name, line))
		return rc;

	/*
	 * Scan for the name, incrementing a counter as you go.
	 * If the counter reaches two bail out and return an
	 * error.
	 */

	for (i = 0;i < ngroups;i++) {
		if (strcmp (name, groups[i].name) == 0) {
			if (++count > 1)
				break;
		}
	}

	if (count > 1) {
		if (verbose)
			fprintf (stderr,
				MSGSTR (M_NONUNIQNAM, DEF_NONUNIQNAM), name);
		return -1;
	}
	return 0;
}

/*
 * NAME: rm_user
 *
 * FUNCTION: Remove a user from a double-null terminated list
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	Removes a user from a double-null terminate list of groups
 *	by copying all of the groups afterwards forward to this
 *	point.
 *
 * RETURNS: NONE
 */

void
rm_user (users)
char	*users;
{
	char	*old,
		*new;

	/*
	 * Point old at where the current user is and point
	 * new at the start of the next user in the list.
	 */

	new = old = users;
	while (*new++)
		;

	/*
	 * Copy from the new location backwards until a double
	 * null is seen.  Then paste an extra null on to double
	 * null terminate the new string.
	 */

	while (*new)
		while (*old++ = *new++)
			;

	*old = '\0';
}

/*
 * NAME: ck_users
 *
 * FUNCTION: Check a groups concurrent user set for validity
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the group's concurrent user set is checked
 *	for existence.  
 *
 * RETURNS: Zero if all groups exist, non-zero otherwise.
 */

int
ck_users (name)
char	*name;
{
	char	*users;
	int	errors = 0;
	int	rc;

	/*
	 * If name is null, return without checking users.
	 */

	if (*name == '\0') {
		return(0);
	}

	/*
	 * See if the group has a concurrent user set
	 */

	if (rc = getgroupattr (name, S_USERS, &users, 0) || ! users) {
		return 0;
	}

	/*
	 * Check each member of the concurrent user set for existence
	 * and count how many don't exist.  This will be returned as
	 * the error code.
	 */

	while (*users) {
		if (getpwnam(users) == NULL) {
			errors++;
			if (verbose)
				fprintf (stderr,
					MSGSTR (M_BADUSER, DEF_BADUSER), users);
		}
		while (*users++)
			;
	}
	return errors;
}

/*
 * NAME: ck_admin
 *
 * FUNCTION: Check a group's admin status
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	admin can be "true" or "false"
 *
 * RETURNS: Zero if "true" or "false", non-zero otherwise.
 */

int
ck_admin (name)
char	*name;
{
	int	admin;
	int	errors = 0;
	int	rc;

	/*
	 * See if the group has an administrative user set
	 */

	rc = getgroupattr (name, S_ADMIN, &admin, 0);
	if (admin != 1 && admin != 0) {
		if (verbose)
			fprintf (stderr, MSGSTR (M_BADADMIN, DEF_BADADMIN),
				name);
		errors++;
		return -1;
	}
	return 0;
}

/*
 * NAME: ck_adms
 *
 * FUNCTION: Check a groups administrative users for validity
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the group's administrative user set is checked
 *	for existence.
 *
 * RETURNS: Zero if all groups exist, non-zero otherwise.
 */

int
ck_adms (name)
char	*name;
{
	char	*adms;
	int	rc;

	/*
	 * See if the group has an administrative user set
	 */

	if (rc = getgroupattr (name, S_ADMS, &adms, 0) || ! adms) {
		if (rc && errno == ENOENT && strcmp(name, "default") != 0) {
			errors++;
			if (verbose)
				fprintf (stderr,
					MSGSTR (M_NOSECGRP, DEF_NOSECGRP),
					name);
		}
		return 0;
	}

	/*
	 * Check each member of the administrative user set for existence
	 * and count how many don't exist.  This will be returned as
	 * the error code.
	 */

	while (*adms) {
		if (getpwnam(adms) == NULL) {
			errors++;
			if (verbose)
				fprintf (stderr,
					MSGSTR (M_BADADMS, DEF_BADADMS), adms);
		}
		while (*adms++)
			;
	}
	return errors;
}

/*
 * NAME: fix_users
 *
 * FUNCTION: Fix a groups concurrent user set
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the group's concurrent user set is checked
 *	for existence and non-existent users are removed.
 *
 * RETURNS: NONE
 */


fix_users (name)
char	*name;
{
	char	*users;
	char	*cp;

	/*
	 * Get the groups concurrent user set
	 */

	if (getgroupattr (name, S_USERS, &users, 0) || ! users) {
		perror(" ");
		exit(errno);
	}

	cp = users;

	/*
	 * Check each member of the concurrent user set for existence
	 * and remove those that don't exist.
	 */

	while (*users) {
		if (getpwnam(users) == NULL) {
			auditwrite ("GROUP_User", AUDIT_OK,
					users, strlen(users) +1,
					name, strlen(name) + 1, 0);
			rm_user (users);
		} else
			while (*users++)
				;
	}
	if (putgroupattr (name, S_USERS, cp, 0)) {
		fprintf (stderr, MSGSTR (M_BADPUT, DEF_BADPUT),
			 name);
		perror("  ");
		exit(errno);
 	}
}

/*
 * NAME: fix_adms
 *
 * FUNCTION: Fix a groups administrative users 
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Each member in the group's administrative user set is checked
 *	for existence and non-existent users are removed.
 *
 * RETURNS: Zero if all groups exist, non-zero otherwise.
 */

fix_adms (name)
char	*name;
{
	char	*adms;
	char	*cp;

	/*
	 * Get the group's administrative user set
	 */

	if (getgroupattr (name, S_ADMS, &adms, 0) || ! adms) {
		perror(" ");
		exit(errno);
	}

	cp = adms;

	/*
	 * Check each member of the administrative user set for existence
	 * and remove those that don't exist.
	 */

	while (*adms) {
		if (getpwnam(adms) == NULL) {
			auditwrite("GROUP_Adms", AUDIT_OK,
					adms, strlen(adms) +1,
					name, strlen(name) + 1, 0);
			rm_user (adms);
		} else
			while (*adms++)
				;
	}
	if (putgroupattr (name, S_ADMS, cp, 0)) {
		fprintf (stderr, MSGSTR (M_BADPUT, DEF_BADPUT),
			 name);
		perror("  ");
		exit(errno);
	}
}

/*
 * NAME: ck_gid
 *
 * FUNCTION: Scan for duplicate instances of 'gid'
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * NOTES:
 *	Called once for each user selected for testing.
 *
 * RETURNS: Zero if gid occurs exactly once, non-zero otherwise.
 */

int
ck_gid (gid, name)
char	*gid, *name;
{
	int	i;
	int	count = 0;

	/*
	 * Scan for the GID, incrementing a counter as you go.
	 * If the counter reaches two bail out and return an
	 * error.
	 */

	for (i = 0;i < ngroups;i++) {
		if ( strcmp(gid, groups[i].id) == 0 ) {
			if (++count > 1)
				break;
		}
	}

	if (count > 1) {
		if (verbose)
			fprintf (stderr,
				MSGSTR (M_NONUNIQNUM, DEF_NONUNIQNUM),
				gid, name);
		return -1;
	}
	return 0;
}

/*
 * NAME: expand_grouptable
 *
 * FUNCTION: Increase the size of the /etc/group information table.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	This function exits on memory allocation failure.
 *
 * RETURNS: NONE
 */

void
expand_grouptable ()
{

	/*
	 * If the table does not already exist we create it with
	 * space for 32 groups, otherwise double its size.
	 */

	if (ngroups == 0) {
		ngroups = 32;
		groups = (struct table *)
			malloc (ngroups * sizeof *groups);
	} else {
		ngroups = ngroups * 2;
		groups = (struct table *)
			realloc (groups, ngroups * sizeof *groups);
	}

	/*
	 * Panic if we have run out of core.
	 */

	if (groups == 0) {
		fprintf (stderr, MSGSTR (M_NOMEM, DEF_NOMEM));
		exit (ENOMEM);
	}
}

/*
 * NAME: expand_secgrouptable
 *
 * FUNCTION: Increase the size of the /etc/security/group information table.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	This function exits on memory allocation failure.
 *
 * RETURNS: NONE
 */

void
expand_secgrouptable ()
{

	/*
	 * If the table does not already exist we create it with
	 * space for ngroup groups, otherwise add 5 to its size.
	 */

	if (nsecgroups == 0) {
		nsecgroups = ngroups;
		secgroups = (struct table *)
			malloc (nsecgroups * sizeof *secgroups);
	} else {
		nsecgroups = nsecgroups + 5;
		secgroups = (struct table *)
			realloc (secgroups, nsecgroups * sizeof *secgroups);
	}

	/*
	 * Panic if we have run out of core.
	 */

	if (secgroups == 0) {
		fprintf (stderr, MSGSTR (M_NOMEM, DEF_NOMEM));
		exit (ENOMEM);
	}
}

/*
 * NAME: add_group
 *
 * FUNCTION: Add a group to the group table, initializing the flags.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * RETURNS: NONE
 */

void
add_group (group, grp)
struct	table	*group;
struct	table	*grp;
{

	/*
	 * Copy the grp's login name and GID.
	 */

	group->name = dup_string (grp->name);
	group->id = dup_string (grp->id);

	/*
	 * Initialize the test flag.
	 */

	group->test = 0;
}

/*
 * NAME: add_secgroup
 *
 * FUNCTION: Add a group to the security group table, initializing the flag.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * RETURNS: NONE
 */

void
add_secgroup (group, buf)
struct	table	*group;
char	*buf;
{

	/*
	 * Copy the group name.
	 */

	group->name = dup_string (buf);

	/*
	 * Initialize flag.
	 */

	group->test = 0;
}

/*
 * NAME: dup_string
 *
 * FUNCTION: Allocate memory for and copy a string
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * NOTES:
 *	Exits on memory allocation errors.
 *
 * RETURNS: Pointer to an allocate version of the string
 */

char *
dup_string (s)
char	*s;
{
	char	*cp;

	cp = (char *)malloc (strlen (s) + 1);

	if (cp)
		return (char *)strcpy (cp, s);

	fprintf (stderr, MSGSTR (M_NOMEM, DEF_NOMEM));
	exit (ENOMEM);

}

/*
 * NAME: mark_groups
 *
 * FUNCTION: Tag groups to be tested using list of names.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Local to this file.
 *
 * RETURNS: NONE
 *	This function exits with ENOENT if an unknown group is requested
 */

void
mark_groups (grnames)
char	**grnames;
{
	int	i;		/* index into list of groups */
	int	gr_errors = 0;	/* Count of nonexistent users */
	struct	table	*group;	/* index into table of group */

	/*
	 * Scan for "ALL" as a group name.  Set the 'ALL' flag if
	 * it is found anywhere in the list.  If all is set, go
	 * ahead and mark the entire list of group.  Don't return
	 * just yet.  Scan the entire list for nonexistent groups
	 * that someone added for fun.
	 */

	for (i = 0;! all && grnames[i];i++)
		if (strcmp (grnames[i], "ALL") == 0)
			all = 1;

	if (all) {
		for (group = groups;group != &groups[ngroups];group++)
			group->test = 1;
		for (group = secgroups;group != &secgroups[nsecgroups];group++)
			group->test = 1;
	}

	/*
	 * Check each group in the table for a match against a name
	 * in the list.  Turn on the 'test' flag if a match is
	 * found.  Don't try looking for "ALL" if it exists.
	 */

	for (i = 0;grnames[i];i++) {

		if ( strcmp (grnames[i], "ALL") == 0 )
			continue;

		/*
		 * Check each name in the list for a match
		 * in /etc/group and set its group 'test' flag.
		 */

		for (group = groups;group != &groups[ngroups];group++) {
			if (strcmp (grnames[i], group->name) == 0) {
				group->test = 1;
				break;
			}
		}

		/*
		 * Now see if a match was made.  If not, report the error.
		 */

		if (group == &groups[ngroups]) {
			if (verbose)
				fprintf (stderr,
					MSGSTR (M_BADETCREQ, DEF_BADETCREQ),
					grnames[i]);
			gr_errors++;
		}

		/*
		 *  At this point I've tested the validity of the search
		 *  criteria and made sure that I have a group match.  If
		 *  this is a NIS group I will not force a complete
		 *  database match since these are often only found in the
		 *  system standard /etc/group file.
		 */
		if ( *grnames[i] == '+' || *grnames[i] == '-' )
			continue;

		/*
		 * Check each name in the list for a match in
		 * /etc/security/group and set its group 'test' flag.
		 */

		for (group = secgroups;group != &secgroups[nsecgroups];group++){
			if (strcmp (grnames[i], group->name) == 0) {
				group->test = 1;
				break;
			}
		}

		/*
		 * Now see if a match was made.  If not, report the error.
		 */

		if (group == &secgroups[nsecgroups]) {
			if (verbose)
				fprintf (stderr,
					MSGSTR (M_BADSECREQ, DEF_BADSECREQ),
					grnames[i]);
			gr_errors++;
		}
	}

	/*
	 * See if any nonexistent groups were requested.  All
	 * of them were found and reported to the user.
	 */

	if (gr_errors)
		exit (ENOENT);
}

/*
 * NAME: grskip
 *
 * FUNCTION: Skip to the first character after the next occurance of c in p.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * RETURNS: Returns a pointer to the first character
 *          after the next occurance of c in p.
 */

static char *
grskip(p,c)
register char *p;
register c;
{
	while( *p && *p != c ) ++p;
	if( *p ) *p++ = 0;
	return( p );
}

/*
 * NAME: init_groups
 *
 * FUNCTION: Read in the entire group file saving group names and UIDs.
 *
 * NOTES:
 *	This is handled in two steps.  The first part involves reading
 *	the entire group file into a table of group names and GIDs.
 *	This is required to test for unique names and GIDs later.
 *
 *	The second step involves flagging the groups to actually be
 *	tested.
 *
 *	The table of users is grown by doubling the size each time it
 *	is expanded.  After all groups have been entered the table is
 *	truncated to its correct size.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Called once at command startup.
 *
 * RETURNS: NONE
 *	This function exits if it is unable to open the system group file.
 */

init_groups (group_names)
char	**group_names;
{
	char	buf[BUFSIZ];	/* Place to put read in group lines */
	char	*colon;		/* Position of colon */
	int	colons;		/* Number of colons in /etc/group entry */
	int	current = 0;	/* Current number of groups in the etc table */
	int	currentsec = 0;	/* Current number of groups in security table */
	struct	table	grp;	/* A group file entry */
	FILE	*gpfp, *sgpfp;	/* handles for read group file */
	int	i;
	char	*p, *q;

	/* Save original status of /etc/group */
	stat("/etc/group", &orig_etc);
	stat("/etc/security/group", &orig_sec);

	if (! (gpfp = fopen ("/etc/group", "r"))) {
		fprintf (stderr, MSGSTR (M_BADOPENEG, DEF_BADOPENEG),errno);
		exit (EACCES);
	}

	if (! (sgpfp = fopen ("/etc/security/group", "r"))) {
		fprintf (stderr, MSGSTR (M_BADOPENESG, DEF_BADOPENESG),errno);
		exit (EACCES);
	}

	/*
	 * Read the entire group file, building our table as we go.
	 */

	while (fgets(buf, BUFSIZ, gpfp) != NULL) {
		p = buf;
		grp.name = p;
		p = grskip(p,':');
		q = p = grskip(p,':');
		while (*q != ':' && *q != '\n')
			q++;
		*q = '\0';
		grp.id = p;

		/*
		 * See if there is any room in the table for the
		 * new entry.  Expand the table as needed.
		 */

		if (current == ngroups)
			expand_grouptable ();

		/*
		 * Add the new entry, incrementing the pointer into
		 * the table.  current is always the first free space
		 * in the table.
		 */

		add_group (&groups[current++], &grp);
	}

	/*
	 * Trim the list down, saving the current size as
	 * 'ngroups'.
	 */

	groups = (struct table *) realloc (groups, sizeof *groups * current);
	ngroups = current;

	/*
	 * Read /etc/security/group group names in order to be
	 * sure that both group files have corresponding stanzas
	 */

	while (fgets(buf, BUFSIZ, sgpfp) != NULL) {

		/* If this line is a stanza header,
		 * remove the ':' from the string and
		 * add an entry to the table for this
		 * group.
		 */

		if ((colon = strrchr (buf, ':')) != NULL) {
			*colon = '\0';
			/*
			 * See if there is any room in the table for the
			 * new entry.  Expand the table as needed.
			 */

			if (currentsec == nsecgroups)
				expand_secgrouptable ();

			/*
			 * Add the new entry, incrementing the pointer into
			 * the table.  current is always the first free space
			 * in the table.
			 */

			add_secgroup (&secgroups[currentsec++], buf);
		}
	}
	fclose (sgpfp);

	/*
	 * Trim the list down
	 */

	secgroups = (struct table *)
		realloc (secgroups, currentsec * sizeof *secgroups);
	nsecgroups = currentsec;

	/*
	 * Mark all groups to be tested.
	 */

	mark_groups (group_names);

	/*
	 * Scan /etc/group to be sure each line
	 * has the right format and a valid id.
	 * getgrent has a problem with these two.
	 */

	i = 0;
	rewind(gpfp);
	while (fgets(buf, BUFSIZ, gpfp) != NULL) {
		if ( groups[i].test == 1 ) {
			if (*groups[i].name == '+' || *groups[i].name == '-') {
				q = buf;
				while ( *q != '\n' && *q != '\0' )
					*q++;
				*q = '\0';
				if (verbose)
					fprintf (stderr,
					MSGSTR (M_YPGRP, DEF_YPGRP), buf);
			}
			else {
				q = groups[i].id;
				while ( *q && isdigit(*q) )
					q++;
				if ( *q || strlen(groups[i].id) == 0) {
						errors++;
						if (verbose)
							fprintf (stderr,
							MSGSTR (M_BADGROUP,
							DEF_BADGROUP),
							groups[i].id,
							groups[i].name);
				}
				colons = 0;
				colon = buf;
				while ((colon = strchr(colon,':')) != NULL) {
					colons++;
					colon++;
				}

				if (colons != 3 && buf[0] != '\n' &&
				    (groups[i].test == 1 || all)) {
					buf[strlen(buf)-1] = 0;
					errors++;
					if (verbose)
						fprintf (stderr,
						MSGSTR (M_BADFORMAT,
						DEF_BADFORMAT), buf, i + 1);
				}
			}
		}
		i++;
	}
	fclose (gpfp);
}

/*
 * NAME: usage
 *
 * FUNCTION: Print usage message and exit
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * RETURNS: This routine does not return.
 */

usage ()
{
	fprintf (stderr, "%s\n",MSGSTR (M_USAGE, DEF_USAGE));
	exit (EINVAL);
}

/*
 * NAME: grpckcatgets
 *
 * FUNCTION: Get a string from the GRPCK message catalog
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	This routine returns a pointer to a message string.  If
 *	the catalog is unopened, an attempt is made to open it.
 *	If the open fails all future messages will be from the
 *	default built-in values.
 *
 * RETURNS: Pointer to message text or NULL on error.
 */

char *
grpckcatgets (int Num, char *Str)
{
	static	int	once;	/* counter for NLS initialization */
	static  nl_catd	catd;	/* catalogue file descriptor */

	if (! once) {
		catd = catopen (MF_GRPCK, NL_CAT_LOCALE);
		once++;
	}
	if (catd != (nl_catd)-1)
		return catgets (catd, MS_GRPCK, Num, Str);
	else
		return Str;
}

/*
 * FUNCTION:	ck_char().
 *
 * DESCRIPTION:	checks to see if the name contains invalid characters
 *		(non-alphanumeric) and begins w/ an alpha character.
 *		NLS characters are allowed.
 *
 * RETURNS:	errno or 0.
 *
*/

int
ck_char(name, line)
char	*name;
int	line;
{
	char	*namep;		/* ptr to the name */
	static char  illegal1stch[] = {'-', '+', ':', '~', 0};

	/* Check for NULL */
	if (!name)
		return(EINVAL);

	/* Check for username's length < PW_NAMELEN */
	if(strlen(name)+1 > PW_NAMELEN) {
		if (verbose)
			fprintf (stderr, MSGSTR (M_BADNAME, DEF_BADNAME), name);
		return(ENAMETOOLONG);
	}

	namep = name;	

	/* Check if the name is null */
	if (namep[0] == 0) {
		if (verbose)
			fprintf (stderr, MSGSTR (M_NULLNAME, DEF_NULLNAME),
				 line);
		return(EINVAL);
	}


	/* Check for usage -- can't begin with the illegalchar */
        if (namep && (strchr (illegal1stch, *namep)))
	{	if (verbose)
			fprintf (stderr, MSGSTR (M_BADCHAR, DEF_BADCHAR), name);
		return(EINVAL);
	}

	if (!strcmp(namep, "ALL") || !strcmp(namep, "default"))
	{	if (verbose)
			fprintf (stderr, MSGSTR (M_BADNAME, DEF_BADNAME), name);
		return(EINVAL);
	}
		
	namep++;

	/* Name can't contain a colon (:) */
	while (*namep)
	{
		if(*namep == ':') {
			if (verbose)
				fprintf (stderr,
					MSGSTR (M_BADCHAR, DEF_BADCHAR), name);
			return(EINVAL);
		}
		namep++;
	}

	return(0);
}
