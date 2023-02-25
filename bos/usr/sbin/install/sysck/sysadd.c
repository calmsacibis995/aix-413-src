static char sccsid[] = "@(#)03  1.2.1.5  src/bos/usr/sbin/install/sysck/sysadd.c, cmdinstl, bos411, 9428A410j 1/19/94 11:21:20";
/*
 * COMPONENT_NAME: cmdinstl
 *
 * FUNCTIONS: add_type, add_owner, add_group, add_acl,
 *            add_audit_info
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/access.h>
#include <sys/stat.h>
#include <sys/audit.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <ctype.h>
#include <usersec.h>
#include "sysdbio.h"
#include "sysmsg.h"
#include "sysaudit.h"

/*
 * A macro defining the security relevant mode bits
 */

#define	ALL_MODES (S_ITCB|S_ITP|S_ISUID|S_ISGID|S_ISVTX|0777)

extern	char	*xstrdup ();
extern	char	*Aclget;
extern	char	*Aclput;
extern	char	*Pclget;
extern	char	*Checksum;
extern	char	*Attributes[];
extern	AFILE_t	afopen();

extern	int	vflg;
/*
 * Constant values representing each attribute.
 */

typedef	enum	{
	ATTR_CLASS,	ATTR_TYPE,	ATTR_OWNER,
	ATTR_GROUP,	ATTR_ACL,	ATTR_PCL,	ATTR_LINKS,
	ATTR_SYMLINKS,	ATTR_CHECKSUM,	ATTR_PROGRAM,	ATTR_SOURCE,
	ATTR_MODE,	ATTR_SIZE
} attr_t;

/*
 * Traits of each attribute.  The name, constant from above, and whether
 * the attribute is set on the command line or from the file appear here.
 */

struct	{
	char	*name;
	int	attr;
	int	arg;
} Attribute_Info[] = {
	{ "class",	ATTR_CLASS,	1},
	{ "type",	ATTR_TYPE,	0},
	{ "owner",	ATTR_OWNER,	0},
	{ "group",	ATTR_GROUP,	0},
	{ "acl",	ATTR_ACL,	0},
	{ "pcl",	ATTR_PCL,	0},
	{ "links",	ATTR_LINKS,	1},
	{ "symlinks",	ATTR_SYMLINKS,	1},
	{ "checksum",	ATTR_CHECKSUM,	0},
	{ "program",	ATTR_PROGRAM,	1},
	{ "source",	ATTR_SOURCE,	0},
	{ "mode",	ATTR_MODE,	0},
	{ "size",	ATTR_SIZE,	0},
	{ 0, 0, 0}
};

#define	N_CLASS		0
#define	N_TYPE		1
#define	N_OWNER		2
#define	N_GROUP		3
#define	N_ACL		4
#define	N_PCL		5
#define	N_LINKS		6
#define	N_SYMLINKS	7
#define	N_CHECKSUM	8
#define	N_PROGRAM	9
#define	N_SOURCE	10
#define	N_MODE		11
#define	N_SIZE		12
#define	N_ATTR	(sizeof Attribute_Info / sizeof Attribute_Info[0])

/*
 * NAME:	add_type
 *
 * FUNCTION:	Update the type value from the command line
 *
 * RETURN VALUE:
 *	Zero on success, non-zero otherwise
 */

int
add_type (
struct	tcbent	*tcbent,
char	*type,
struct	stat *sb)
{

	/*
	 * The type wasn't specified, so I get the type directly
	 * from the file itself.
	 */

	 if (! type) {
		if (S_ISFIFO (sb->st_mode))
			tcbent->tcb_type = TCB_FIFO;
		else if (S_ISDIR (sb->st_mode))
			tcbent->tcb_type = TCB_DIR;
		else if (S_ISCHR (sb->st_mode)) {

			/* see if character device is multiplexed */

			if (S_ISMPX (sb->st_mode))
				tcbent->tcb_type = TCB_MPX;
			else
				tcbent->tcb_type = TCB_CHAR;
		} else if (S_ISBLK (sb->st_mode))
			tcbent->tcb_type = TCB_BLK;
		else if (S_ISREG (sb->st_mode))
			tcbent->tcb_type = TCB_FILE;
		else {

                        MSG_S(MSG_Unknown_Type,DEF_Unknown_Type,
                              tcbent->tcb_name,0,0,0);
			tcbent->tcb_type = -1;
			return -1;
		}
	} else {

	/*
	 * There is a type string.  It overrides the file value.
	 */

		if (! strcmp (type, "FILE"))
			tcbent->tcb_type = TCB_FILE;
		else if (! strcmp (type, "DIRECTORY"))
			tcbent->tcb_type = TCB_DIR;
		else if (! strcmp (type, "BLK_DEV"))
			tcbent->tcb_type = TCB_BLK;
		else if (! strcmp (type, "CHAR_DEV"))
			tcbent->tcb_type = TCB_CHAR;
		else if (! strcmp (type, "MPX_DEV"))
			tcbent->tcb_type = TCB_MPX;
		else if (! strcmp (type, "FIFO"))
			tcbent->tcb_type = TCB_FIFO;
		else if (! strcmp (type, "SYMLINK"))
			tcbent->tcb_type = TCB_SYMLINK;
		else if (! strcmp (type, "LINK"))
			tcbent->tcb_type = TCB_LINK;
		else {
			tcbent->tcb_type = -1;
                        MSG_S(MSG_Illegal_Type,DEF_Illegal_Type,
                              tcbent->tcb_name, type,0,0);
			return -1;
		}
	}
	return 0;
}

/*
 * NAME:	add_owner
 *
 * FUNCTION:	Update TCB entry from command line argument for owner
 *
 * RETURN VALUE:
 *	-1 on error, 0 otherwise
 */

uid_t
add_owner (
struct	tcbent	*tcbent,
char	*owner,
struct	stat *sb)
{
	long	l;
	char	*cp;
	struct	passwd	*pw;

	/*
	 * No value means to compute it from the file
	 */

	if (owner == 0) {
		if (sb) {
			tcbent->tcb_owner = sb->st_uid;
			return 0;
		} else {
                        MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,"owner",
                              tcbent->tcb_name,0,0);
			return -1;
		}
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (*owner == '\0') {
		tcbent->tcb_owner = -1;
		return 0;
	}

	/*
	 * Try to convert the value.  Numbers are converted directly
	 * to user IDs, non-numbers are looked up in the database.
	 */

	if (! isnumeric (owner)) {
		if (pw = getpwnam (owner)) {
			tcbent->tcb_owner = pw->pw_uid;
		} else {
                        MSG_S(MSG_Unknown_User,DEF_Unknown_User,owner,
                              tcbent->tcb_name,0,0);
			return -1;
		}
	} else {
		tcbent->tcb_owner = strtol (owner, &cp, 10);
		if (cp == (char *) 0 || *cp != '\0') {
                        MSG_S(MSG_Invalid_Value,DEF_Invalid_Value,"owner",
                              owner,0,0);
			return -1;
		}
	}
	return 0;
}

/*
 * NAME:	add_group
 *
 * FUNCTION:	Update TCB entry from command line argument for group
 *
 * RETURN VALUE:
 *	-1 on error, 0 otherwise
 */

uid_t
add_group (struct tcbent *tcbent, char *group, struct stat *sb)
{
	long	l;
	char	*cp;
	struct	group	*gr;

	/*
	 * No value means to compute it from the file
	 */

	if (group == 0) {
		if (sb) {
			tcbent->tcb_group = sb->st_gid;
			return 0;
		} else {
                        MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,"group",
                              tcbent->tcb_name,0,0);
			return -1;
		}
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (*group == '\0') {
		tcbent->tcb_group = -1;
		return 0;
	}

	/*
	 * Try to conver the value.  Numbers are converted directly
	 * to user IDs, non-numbers are looked up in the database.
	 */

	if (! isnumeric (group)) {
		if (gr = getgrnam (group)) {
			tcbent->tcb_group = gr->gr_gid;
		} else {
                        MSG_S(MSG_Unknown_Group,DEF_Unknown_Group,group,
                              tcbent->tcb_name,0,0);
			return -1;
		}
	} else {
		tcbent->tcb_group = strtol (group, &cp, 10);
		if (cp == (char *) 0 || *cp != '\0') {
                        MSG_S(MSG_Invalid_Value,DEF_Invalid_Value,"group",
                              group,0,0);
			return -1;
		}
	}
	return 0;
}

/*
 * NAME:	add_acl
 *
 * FUNCTION:	Update the ACL value from the command line argument
 *
 * RETURN VALUE:
 *	0 for success, non-zero otherwise
 */

int
add_acl (
struct  tcbent  *tcbent,
char	*acl,
struct	stat *sb)
{
	/*
	 * No value means to compute it from the file.  Give up if
	 * the file doesn't exist and there is no value.
	 */

	if (acl == 0 && sb == 0) {
                MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,"acl",
                      tcbent->tcb_name,0,0);
		return -1;
	}

	/*
	 * Blank value means to remove the attribute
	 */

	if (acl && *acl == '\0') {
		tcbent->tcb_acl = 0;
		return 0;
	}

	/*
	 * Either a value was specified and we use it, or no value
	 * was specified and we compute it directly
	 */

	if (acl)
		tcbent->tcb_acl = acl;
	else
		tcbent->tcb_acl = (char *)get_program (Aclget,tcbent->tcb_name);

	return tcbent->tcb_acl == 0;
}

/*
 * NAME: add_audit_info
 *
 * FUNCTION: create audit records for each field in a tcbent struct
 *
 * ENVIRONMENT:
 *
 *	User Process
 *
 * NOTES:
 *
 *	This routine creates a "SYSCK_Update" audit record for
 *	each field in a tcbent structure.  It is assumed that the
 *	passed in structure is going to be added to the database
 *	and so this information will be changed.
 *
 * RETURNS: NONE
 */

void
add_audit_info (struct tcbent *tcbent, int status)
{
	char	*file = tcbent->tcb_name;

	if (tcbent->tcb_class)
		mk_audit_rec (SYSCK_Update, status, file, "class", NULL);

	if (tcbent->tcb_type != -1)
		mk_audit_rec (SYSCK_Update, status, file, "type", NULL);

	if (tcbent->tcb_owner != (uid_t) -1)
		mk_audit_rec (SYSCK_Update, status, file, "owner", NULL);

	if (tcbent->tcb_group != (gid_t) -1)
		mk_audit_rec (SYSCK_Update, status, file, "group", NULL);

	if (tcbent->tcb_mode != -1)
		mk_audit_rec (SYSCK_Update, status, file, "mode", NULL);

	if (tcbent->tcb_acl)
		mk_audit_rec (SYSCK_Update, status, file, "acl", NULL);

	if (tcbent->tcb_links)
		mk_audit_rec (SYSCK_Update, status, file, "links", NULL);

	if (tcbent->tcb_symlinks)
		mk_audit_rec (SYSCK_Update, status, file, "symlinks", NULL);

	if (tcbent->tcb_checksum)
		mk_audit_rec (SYSCK_Update, status, file, "checksum", NULL);

	if (tcbent->tcb_program)
		mk_audit_rec (SYSCK_Update, status, file, "program", NULL);

	if (tcbent->tcb_source)
		mk_audit_rec (SYSCK_Update, status, file, "source", NULL);

	if (tcbent->tcb_size != -1)
		mk_audit_rec (SYSCK_Update, status, file, "size", NULL);
}

