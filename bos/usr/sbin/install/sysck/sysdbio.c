static char sccsid[] = "@(#)12  1.5.1.12  src/bos/usr/sbin/install/sysck/sysdbio.c, cmdinstl, bos41J, 9521B_all 5/25/95 18:43:57";
/*
 * COMPONENT_NAME: cmdinstl
 *
 * FUNCTIONS: gettcbstanza, init_tcbent, addRootPath, 
 *            deleteRootPath, addRootPathList, deleteRootPathList
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <memory.h>
#include <sys/access.h>
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

/*
 * A macro defining the security relevant mode bits
 */

#define	ALL_MODES (S_ITCB|S_ITP|S_ISUID|S_ISGID|S_ISVTX|0777)

extern	char	*xstrdup (char *);
extern	char	**null_list (char *);
extern	char	*list_null (char **, char *, int);
extern	char	*get_program (char *, char *);
extern	char	*mk_sum (struct tcbent *);
extern	struct	group	*getgrnam (char *);
extern	struct	group	*getgrgid (gid_t);
extern	struct	passwd	*getpwnam (char *);
extern	struct	passwd	*getpwuid (uid_t);
extern	char	*Aclget;
extern	char	*Pclget;
extern	char	*Checksum;

extern	int	iflg;
extern	int	uflg;
extern	int	Rflg;
extern	int	Nflg;
extern	char	*RootPath;
extern	int	RootPathLen;
char *deleteRootPath(char *);
char *addRootPath(char *);
char **deleteRootPathList(char **);
char **addRootPathList(char **);
void CheckLinks(char **symlinks, char *fname);

/*
 * NAME:	init_tcbent
 *
 * FUNCTION:	Initialize members of struct tcbent
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User Process
 *
 * RETURNS: NONE
 */

void
init_tcbent (struct tcbent *tcb)
{
	static	struct	tcbent	zero = {
		(char *) 0,	(char **) 0,	(tcb_type_t) -1, (uid_t) -1,
		(gid_t) -1,	(char *) 0,	(char *) 0,	(char *) 0,
		(char **) 0,	(char **) 0,	(char **) 0,	(char *) 0,
		(char *) 0, 	-1,		-1,		0,		
		0, 		0,		0,		0,		
		0, 		0
	};

	*tcb = zero;
}

/*
 * NAME:	gettcbstanza
 *
 * FUNCTION:	Get a tcbent structure from an ASCII stanza file
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	This routine differs from gettcbent() in that it knows it
 *	is reading from an attribute file.  gettcbent() only knows
 *	about the interface, this routine is required to know
 *	about the actual underlying file structure.  Attributes
 *	which have a NULL value are computed from the file itself.
 *
 * RETURNS:
 *	NULL on EOF or error, a pointer to the next structure in the
 *	file if there is one.
 */

struct tcbent *
gettcbstanza (AFILE_t fp)
{
	char	*cp;
	char	*cp2;
	char	*file;
	char	*xacl = 0;
	int	i;
	int	exists = 0;
	int	symlink_exists = 0;
	int	needed = 0;
	int	rc = 0;
	ATTR_t	attr;
	struct	tcbent	tcbent;
	struct	tcbent	*ntcbent;
        struct  stat statbuf;
	struct	passwd	*pw;
	struct	group	*gr;
	static	char	last[BUFSIZ];

	/*
	 * Read the next stanza and setup up the default values.
	 * The filename in the stanza is checked for existance and
	 * the attributes fetched.  These attributes will be used
	 * as defaults if the user specifies an attribute with no
	 * value.
	 */

	if (! afread (fp)) {
		if (errno == 0)
			return (struct tcbent *)EOF;

	        MSG_S(MSG_Input_File_Error,DEF_Input_File_Error,0,0,0,0);
		if (last[0])
	         	MSG_S(MSG_Last_Stanza,DEF_Last_Stanza,last,0,0,0);
		else
                        MSG_S(MSG_No_Last_Stanza,DEF_No_Last_Stanza,0,0,0,0);

		return 0;
	}
	attr = fp->AF_catr;

	strcpy (last, attr->AT_value);

	/*
	 * File name must begin with a slash
	 */

	if (attr->AT_value[0] != '/') {
		MSG_S(MSG_Absolute_File,DEF_Absolute_File,
                      attr->AT_value,0,0,0);
                return 0;
	}

	/*
	 * See if the name is valid
	 */

	if (validate_name (attr->AT_value)) {
		MSG_S(MSG_Illegal_Attribute,DEF_Illegal_Attribute,
                      "name", attr->AT_value,0,0);
                return 0;
	}

	/*
	 * Initialize the structure to just a filename and all
	 * of the attributes set to values indicating no value
	 * for that attribute.
	 */

	init_tcbent (&tcbent);

	if (Rflg)
		tcbent.tcb_name = file = addRootPath(attr->AT_value);
	else
		tcbent.tcb_name = file = attr->AT_value;
	if (validate_name (tcbent.tcb_name)) {
		MSG_S(MSG_Illegal_Entry,DEF_Illegal_Entry,tcbent.tcb_name,
                      0,0,0);
		return 0;
	}

        /*
         * See if the file exists.
         */

         if (stat (tcbent.tcb_name,&statbuf) == 0)
       		exists = 1; /* File exists */

	/*
	 * Initialize the fields in the structure from the attributes
	 * in the stanza.
	 */

	for (attr++;attr->AT_name;attr++) {
		cp = attr->AT_value;

		/*
		 * Get the "class" attribute.  It is an error if this
		 * attribute is specified without a value.
		 */
		 
		if (! strcmp (attr->AT_name, "class")) {
			if (cp && *cp) {
				tcbent.tcb_class = null_list (cp);
			} else {
				MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
				return 0;
			}
                        continue;
		}

		/*
		 * Get the "type" attribute.  This attribute must be
		 * specified for all entries.
		 */

		if (! strcmp (attr->AT_name, "type")) {
			if (! *cp) {
				MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
			}
			if (add_type (&tcbent, cp, &statbuf)) {
                                MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
				return 0;
                        }
			if (!uflg && !exists && 
				(tcbent.tcb_type != TCB_SYMLINK) && 
				(tcbent.tcb_type != TCB_LINK) &&
				(tcbent.tcb_type != TCB_DIR)) {
                        		MSG_S(MSG_No_Such_File,DEF_No_Such_File,
                             				tcbent.tcb_name,0,0,0);
		 			return 0;
			}

			continue;
		}

		/*
		 * Get the "owner" attribute.  Not specified by default.
		 * Value will be computed from the file if the attribute
		 * value is missing.
		 */

		if (! strcmp (attr->AT_name, "owner")) {
			if (add_owner (&tcbent,cp,&statbuf)) {
                                MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
				return 0;
                        }
			continue;
		}

		/*
		 * Get the "group" attribute.  No default value.
		 * The value of the group attribute will be computed
		 * from the file if the attribute value is blank.
		 */

		if (! strcmp (attr->AT_name, "group")) {
			if (add_group (&tcbent, cp, &statbuf)) {
                                MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
				return 0;
                        }
			continue;
		}

		/*
		 * Get the "acl" attribute.  No default value.
		 * The value of the acl attribute will be computed
		 * from the file if the attribute value is blank.
		 *
		 * Here we have to invoke the aclget program to
		 * fetch the acl.
		 */

		if (! strcmp (attr->AT_name, "acl")) {
                        if (! *cp)
                                continue;
			cp = xstrdup (cp);
			if (add_acl (&tcbent,*cp ? cp:0, &statbuf)) {
                               MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
                               return 0;
                        }

			continue;
		}

		/*
		 * Get the "mode" attribute.  Convert character
		 * strings to a full file mode word.  If the attribute
		 * value is blank the mode on the file will be used.
		 */

		if (! strcmp (attr->AT_name, "mode")) {
			if (*cp == '\0') {
				tcbent.tcb_mode = statbuf.st_mode & ALL_MODES;
				continue;
			}
			if (string2mode (cp, &tcbent.tcb_mode, tcbent.tcb_name))
                          {
                                MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
				return 0;
                          }
			continue;
		}

		/*
		 * Get the "links" attribute.  "links" is a comma
		 * separated list of hard links.  This attribute
		 * requires a value.
		 */

		if (! strcmp (attr->AT_name, "links")) {
			if (! *cp) {
				MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
				return 0;
			}
			tcbent.tcb_links = addRootPathList(null_list (cp));
			continue;
		}

		/*
		 * Get the "symlinks" attribute.  "symlinks" is a comma
		 * separated list of symbolic links.  This attribute requires
		 * a value.
		 */

		if (! strcmp (attr->AT_name, "symlinks")) {
			if (! *cp) {
				MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
				return 0;
			}
			tcbent.tcb_symlinks = addRootPathList(null_list (cp));

			CheckLinks(tcbent.tcb_symlinks, tcbent.tcb_name);

			if (tcbent.tcb_symlinks[0] == (char *) NULL)
				tcbent.tcb_symlinks = (char **) NULL;

			continue;
		}

		/*
		 * Get the "checksum" attribute.  No default value.
		 * The checksum attribute will be computed from the
		 * file if the value is blank.
		 *
		 * The system checksum utility is invoked to compute
		 * the file's checksum.
		 *
		 * After installation it may be required to have the
		 * checksum ignored, for example, for files which
		 * change after install but need to be validated at
		 * install.  These files have ",VOLATILE" after the
		 * checksum to indicate this.
		 */

		if (! strcmp (attr->AT_name, "checksum")) {

			/*
			 * Checksum is only defined for regular files -
			 * return if this is not a file
			 */

			if (! S_ISREG (statbuf.st_mode)) {
				return 0;
			}

			/*
			 * If the checksum attribute has no value, compute
			 * the checksum from the file.  Set tcb_vsum to 1,
                         * this will keep the sum from being checked against
                         * the file later in ck_tcbent. 
			 */

			if (! *cp) {
				tcbent.tcb_checksum = xstrdup (mk_sum(&tcbent));
                                tcbent.tcb_vsum = 1;
				continue;
			}

			/*
			 * The checksum attribute is a list of attributes,
			 * including "NOSIZE", "VOLATILE", and a string
			 * valued checksum.  The entire list must be
			 * walked through to find the parts since there
			 * is no ordering of the members.
			 */

			while (*cp) {
				if (! strcmp (cp, "NOSIZE"))
					tcbent.tcb_checksum = (char *) 0;
				else if (! strcmp (cp, "VOLATILE"))
					tcbent.tcb_vchecksum = 1;
				else {

					/*
					** Modified by DM Geise on 7/17/91 to convert
					** the stanza's checksum value from a string to
					** an unsigned integer. This value will be subsequently
					** stored in the SWVPD rather than running mk_checksum
					** to calculate this value. This should significantly
					** speed up installp.
					*/

					tcbent.tcb_checksum = xstrdup (cp);
					tcbent.tcb_sum = (unsigned) atoi(cp);
					tcbent.tcb_vsum = 1;
				} 

				cp += strlen (cp) + 1;
			}
		}

		/*
		 * Get the "program" attribute.  The value for this
		 * attribute must be specified.
		 */

		if (! strcmp (attr->AT_name, "program")) {
			if (! *cp) {
				MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
				return 0;
			}
			tcbent.tcb_program = null_list (cp);
			continue;
		}

		/*
		 * Get the "source" attribute.  No default value.
		 * This attribute is unique in that if the value is
		 * NULL it means the file is created from scratch.
		 */

		if (! strcmp (attr->AT_name, "source")) {
			if (cp)
				tcbent.tcb_source = xstrdup(addRootPath(cp));
			continue;
		}

		/*
		 * Get the "size" attribute.  The attribute value
		 * is computed directly from the file if one is not
		 * given.  This attribute is valid for regular files
		 * only.
		 *
		 * After installation it may be required to have the
		 * size ignored, for example, for files which
		 * change after install but need to be validated at
		 * install.  These files have ",VOLATILE" after the
		 * size to indicate this.
		 */

		if (! strcmp (attr->AT_name, "size")) {

			/*
			 * The size is only defined for regular files -
			 * return if this is not a file 
			 */

			if (exists && ! S_ISREG (statbuf.st_mode)) {
				return 0;
			}

			/*
			 * If the size attribute has no value, compute
			 * the size from the file.  It is an error if
			 * the file does not exist.
			 */

			if (! *cp) {
				tcbent.tcb_size = statbuf.st_size;
				continue;
			}

			/*
			 * The size attribute is a list of attributes,
			 * including "NOSIZE", "VOLATILE", and a decimal
			 * value in bytes.  The entire list must be
			 * walked through to find the parts since there
			 * is no ordering of the members.
			 */

			while (*cp) {
				if (! strcmp (cp, "NOSIZE"))
					tcbent.tcb_size = -1;
				else if (! strcmp (cp, "VOLATILE")) {
					tcbent.tcb_vsize = 1;
					tcbent.tcb_size = 0;
                                }
				else {
					tcbent.tcb_size = strtol (cp, &cp2, 10);
					if (cp2 == cp || *cp2 != '\0') {
						MSG_S(MSG_Invalid_Value,
                                                      DEF_Invalid_Value,
							"size", cp,0,0);
						return 0;
					}
				}
				cp += strlen (cp) + 1;
			}
			continue;
		}

		/*
		 * See if there is an extended ACL entry which
		 * needs to be applied.  This is only valid
		 * in 'install' mode.
		 */

		if (iflg && ! strcmp (attr->AT_name, "xacl")) {
			if (cp && *cp) {
				if (add_xacl (&tcbent, cp))
					return 0;
			}
			continue;
		}

		/*
		 * Get the "target" attribute.  "target" is a fullpath name
		 * for a symbolic link or hard link target.  This attribute 
		 * requires a value.
		 * ex. /tmp/foo -> /tmp/bar  (bar is the target)
		 */

		if (! strcmp (attr->AT_name, "target")) {
			if (! *cp) {
				MSG_S(MSG_Needs_A_Value,DEF_Needs_A_Value,
                                      attr->AT_name,tcbent.tcb_name,0,0);
				return 0;
			}
			tcbent.tcb_target = xstrdup(addRootPath(cp));
			continue;
		}

	}

	/*
	 * If this is INSTALL mode the file must have a type = attribute
	 * or it is an error.
	 */

	if (iflg && tcbent.tcb_type == -1) {
		MSG_S(MSG_Missing_Type,DEF_Missing_Type,tcbent.tcb_name,0,0,0);
		return 0;
	}

	if (!uflg && !exists && (tcbent.tcb_type == TCB_DIR)) 
		xmkdir(tcbent.tcb_name,tcbent.tcb_mode);
				
	/*
	 * Flag the entry and return it in a newly malloc()d buffer
	 * Any errors during initialization will cause the entry to
	 * not be marked valid.  A valid tag is required for the
	 * entry to be added to the configuration file.
	 */

	tcbent.tcb_valid = 1;
	tcbent.tcb_changed = 1;
	ntcbent = (struct tcbent *) malloc (sizeof (struct tcbent));
	*ntcbent = tcbent;
	ntcbent->tcb_name = xstrdup(tcbent.tcb_name);

	return ntcbent;
}

/*
 * NAME: addRootPath
 * FUNCTION:  This function adds the RootPath to the string if
 * the -R flag was specified on the command line.
 */
char *addRootPath(char *string)
{
	char *newstring;

	if (!Rflg)
		return(string);
	else
	{
		newstring = (char *)xmalloc((strlen(string) + RootPathLen + 2));
		newstring = strcpy(newstring,RootPath);
		newstring = strcat(newstring,string);
		return(newstring);
	}
}

/*
 * NAME: deleteRootPath
 * FUNCTION:  This function deletes RootPath from string if the -R flag
 * was specified on the command line.
 */
char *deleteRootPath(char *string)
{
	if (!Rflg)
		return(string);
	else
	{
		if (strncmp(string,RootPath,RootPathLen) == 0)
			return(&(string[RootPathLen]));
		else
			return(string);
	}
}

/*
 * NAME: addRootPathList
 * FUNCTION:  This function adds the RootPath to each of the strings in the
 * list if the -R flag was specified on the command line.
 */
char **addRootPathList(char **string)
{
	int i;
	char *tmpstr;

	if (!Rflg)
		return(string);
	else
	{
		for(i=0;string[i];i++)
		{
			tmpstr = (char *)string;
			string[i] = addRootPath(string[i]);
/*			free(tmpstr);*/
		}
		string[i] = (char *) NULL;
		return(string);
	}
}

/*
 * NAME: deleteRootPathList
 * FUNCTION:  This function deletes RootPath from each sting in the list 
 * if the -R flag was specified on the command line.
 */
char **deleteRootPathList(char **string)
{
	int i;

	if (!Rflg)
		return(string);
	else
	{
		for(i=0;string[i];i++)
			string[i] = deleteRootPath(string[i]);
		return(string);
	}
}

/* 
** Name: CheckLinks
**
** Function: Input is an array of strings. Each string specifies the name of 
**           a symbolic link.  If a string corresponds to a real file or 
**	     directory, it is deleted from the array (so the symbolic link
**	     is not created).
*/

void CheckLinks(char ** symlinks, char *fname)

{

	int		i, j;
	int		rc;
	struct stat	stat_buf;

	ino_t		symlink_inode;  /* inode number of a symbolic link */

	dev_t		symlink_dev;    /* inode number of the file the    */
					/* symbolic link will point at     */

	for (i=0; symlinks[i]; i++) {
		
		rc = lstat(symlinks[i], &stat_buf);

		if (rc == 0) {

			/* The symlink[i] file exists. Save its inode and device numbers. */

			symlink_inode = stat_buf.st_ino;
			symlink_dev   = stat_buf.st_dev;

			/* If its a regular file or a directory,   */
			/* then determine if the symbolic link     */
			/* will point at itself.         	   */

			if ( S_ISREG(stat_buf.st_mode) || S_ISDIR(stat_buf.st_mode) ) {

				rc = stat(fname, &stat_buf);

				if (rc == 0) {

					/* The file the symbolic link will point at exists.  */

					/* If its device and inode numbers equal the device  */
					/* and inode numbers of the symbolic link, then I    */
					/* won't create the symbolic link because it would   */
					/* point at itself (and wipe out the existing file)! */

					if ( (symlink_inode == stat_buf.st_ino) && 
					     (symlink_dev   == stat_buf.st_dev) ) {
			

						for (j=i; symlinks[j]; j++) {
							symlinks[j] = symlinks[j+1];
						}

						i--;  /* must reprocess symlinks[j] */

					} /* end if symbolic link would point at itself */

				} /* end if the file the symbolic link will point at exists */

			} /* the file is a regular file or a directory */

		} /* if we can successfully lstat the symlink file */

        } /* for each string in the input array */

} /* end CheckLinks */

