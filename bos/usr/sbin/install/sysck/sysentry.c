static char sccsid[] = "@(#)15  1.4.1.12  src/bos/usr/sbin/install/sysck/sysentry.c, cmdinstl, bos411, 9435D411a 9/2/94 16:43:35";
/*
 * COMPONENT_NAME: cmdinstl
 *
 * FUNCTIONS: ck_program, ck_checksum, ck_size,
 *            ck_type, ck_tcbent
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

#include <sys/audit.h>
#include <sys/acl.h>
#include <sys/priv.h>
#include <sys/limits.h>
#include <sys/fullstat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "sysdbio.h"
#include "sysmsg.h"
#include "sysaudit.h"

/*
 * All of the security relevant mode bits
 */

#define	ALL_MODES (S_ITP|S_ITCB|S_ISUID|S_ISGID|S_ISVTX|0777)

/*
 * External variables, flags, and function calls
 */

extern	int	iflg;
extern	int	vflg;

extern	char	*get_program();
extern	struct	tcbent	*hash_lookup();

extern	int	ck_acl();
extern	int	ck_mode();
extern	char	*mk_sum();

/*
 * Program names
 */

extern	char	*Aclget;
extern	char	*Aclput;
extern	char	*Pclget;
extern	char	*Pclput;
extern	char	*Checksum;
extern  int	Dont_Remove_ACL;
extern  int     checksum_or_size_error;
extern	int	BuiltInSum;

/*
 * NAME:	ck_program
 *
 * FUNCTION:	Validate a file in the TCB using the file-dependent program
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	TCB entries which fail the validation test will be disabled by
 *	having their modes set to 0.
 *
 * RETURNS:	Zero on success, non-zero otherwise
 */

int
ck_program (struct tcbent *tcbent)
{
	int	i;
	char	buf[BUFSIZ];
	char	*file = tcbent->tcb_name;
	char	*program;
	struct	tcbent	*program_ent;

	/* 
	 * Set up program name
	 */

	if (tcbent->tcb_program)
		program = tcbent->tcb_program[0];
	else
		return 0;

	/*
	 * Verify that the test program exists and is executable.
	 */

	if (! program || program[0] != '/') {
	        MSG_S(MSG_Absolute_Program,DEF_Absolute_Program,program,0,0,0);
		mk_vaudit (SYSCK_Check, AUDIT_FAIL,
			file, CantFix, DEF_Absolute_Program, program, NULL);
		return -1;
	}
	if (access (program, X_ACC)) {
	        MSG_S(MSG_No_Program,DEF_No_Program,program,0,0,0);
		return -1;
	}

	/*
	 * Create the command line buffer for the command to execute
	 */

	strcpy (buf, program);

	strcat (buf, " -t");

	/*
	 * Find any remaining arguments on the command line.
	 */

	if (tcbent->tcb_program[1]) {
		for (i = 1;tcbent->tcb_program[i];i++) {
			strcat (buf, " ");
			strcat (buf, tcbent->tcb_program[i]);
		}
	} else
		strcat (buf, " ALL");

	/*
	 * Execute the command and examine the return code.  A command
	 * which fails will cause a message to be put out, but no
	 * further action taken.
	 */

	if (system (buf) == 0)
		return 0;

	/*
	 * The file verify failed for some reason.  Complain about it.
	 */

        MSG_S(MSG_Verify_Failed,DEF_Verify_Failed,program,file,0,0);
	return ENOTRUST;
}

/*
 * NAME:	ck_checksum
 *
 * FUNCTION:	Test a checksum for correctness
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	This procedure calls the Checksum program to determine the
 *	file checksum, which it compares against the checksum stored
 *	for the file.
 *
 * RETURNS:	Zero for success, non-zero otherwise
 */

int
ck_checksum (struct tcbent *tcbent)
{
	char	*checksum;
	int	 calculated_checksum_int_value;
	int	 correct_checksum_int_value;
	char	*file = tcbent->tcb_name;
	int	okay;
	int	rc;

	/*
 	 * if the -v flag was not specified then suppress checking the sums
         */
	if (!vflg)
		return(0);

	/*
	 * See if there is a checksum - return OK if there is none.
	 */

	if (tcbent->tcb_checksum == 0)
		return 0;

	/*
	 * Run the checksum program and save the output.  The output
	 * of the checksum command must equal the value which was
	 * present before.
	 */

	if (! BuiltInSum) {
		if (! (checksum = get_program (Checksum, file))) {
			rc = -1;
			goto error;
		}
	} else {
		if (! (checksum = mk_sum (tcbent))) {
			rc = -1;
			goto error;
		}
	}
	sscanf (checksum, "%d", &calculated_checksum_int_value);
	sscanf (tcbent -> tcb_checksum, "%d", &correct_checksum_int_value);
	if (calculated_checksum_int_value == correct_checksum_int_value)
	   okay = TRUE;
	 else
	   okay = FALSE;

	if (! BuiltInSum)
		free (checksum);

	/*
	 * Free the checksum output and see if there was a match.
	 * Report an error and disable the object if the two values
	 * are different.
	 */

	if (okay)
		return 0;

        MSG_S(MSG_Wrong_Checksum,DEF_Wrong_Checksum,file,0,0,0);

	rc = ENOTRUST;
	if (Dont_Remove_ACL)
	 {
	   checksum_or_size_error = TRUE;

	   /*
	    * Use the original 'correct' checksum, not the calculated version.
	    */

	   tcbent -> tcb_sum = (unsigned) atoi (tcbent -> tcb_checksum);
	 }

error:
	mk_audit (SYSCK_Check, AUDIT_FAIL, file, NotFixed, "checksum");
	errno = rc;
	return -1;
}

/*
 * NAME:	ck_size
 *
 * FUNCTION:	verify the correct file size
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUE:
 *	Zero if the size is correct, non-zero otherwise
 */

int
ck_size (struct	tcbent	*tcbent, struct	fullstat *stat)
{
	extern int force_size;

	char	*file = tcbent->tcb_name;
	int	rc;

	/*
	 * File type must be REGULAR FILE.  Sizes on other
	 * stuff is pretty meaningless.
	 */

	if (tcbent->tcb_type != TCB_FILE || tcbent->tcb_size == -1 ||
                     tcbent->tcb_size == 0)
		return 0;

	/*
	 * Compare size against the value stored in the
	 * database.  The object is disabled if there is a
	 * discrepancy.
	 */

	if (tcbent->tcb_size == stat->st_size)
		return 0;

        MSG_S(MSG_Wrong_Size,DEF_Wrong_Size,file,0,0,0);

	rc = ENOTRUST;
	if (Dont_Remove_ACL)
	 {
	   checksum_or_size_error = TRUE;
	   force_size = TRUE;  /* Forces tcbent -> tcb_size to be stored into */
	                       /* sysck database instead of actual size. */
	 }

error:
	mk_audit (SYSCK_Check, AUDIT_FAIL, file, NotFixed, "size");
	errno = rc;
	return -1;
}

/*
 * NAME:	ck_type
 *
 * FUNCTION:	verify a file has the correct type
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURN VALUE:
 *	Zero if the type exists and is correct, non-zero otherwise
 */

int
ck_type (struct	tcbent	*tcbent, struct fullstat *stat)
{
	mode_t	mode = stat->st_mode;
	char	*file = tcbent->tcb_name;
	int	rc;

	/*
	 * Test for the proper file type according to the type= attribute
	 * in the TCB file.
	 */
	switch (tcbent->tcb_type) {
		case TCB_FILE:
			if (S_ISREG (mode))
				return 0;
			break;
		case TCB_DIR:
			if (S_ISDIR (mode))
				return 0;
			break;
		case TCB_FIFO:
			if (S_ISFIFO (mode))
				return 0;
			break;
		case TCB_BLK:
			if (S_ISBLK (mode))
				return 0;
			break;
		case TCB_CHAR:
			if (S_ISCHR (mode) && ! S_ISMPX (mode))
				return 0;
			break;
		case TCB_MPX:
			if (S_ISMPX (mode))
				return 0;
			break;
		case TCB_SYMLINK:
			return 0;
		case TCB_LINK:
			return 0;
	}

	/*
	 * If the file had the wrong type, report the error and disable
	 * the object by setting the mode to 0.
	 */

        MSG_S(MSG_Wrong_File_Type,DEF_Wrong_File_Type,file,0,0,0);
	rc = ENOTRUST;

error:
	mk_audit (SYSCK_Check, AUDIT_FAIL, file, NotFixed, "type");
	errno = ENOTRUST;
	return -1;
}

/*
 * NAME:	ck_tcbent
 *
 * FUNCTION:	Test the attributes of a single file in the TCB
 *
 * NOTES:
 *	ck_tcbent calls a variety of functions to test the different
 *	attributes of a file.  ck_tcbent itself and these other functions
 *	may change the attributes of a file and create new files and
 *	directories in the course of doing their work.
 *
 *      If the file is a symlink, ck_tcbent needs to test the different
 *      attributes of the symlink's destination file.
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * RETURNS: Count of errors encountered correcting file.
 */

int
ck_tcbent (struct tcbent *tcbent)
{
	char	*file;		/* Name of file being tested                */
	char	buf[BUFSIZ];	/* Scratch character buffer                 */
	int	just_created;	/* This file had to be copied or created    */
	int	i;		/* Scratch counter                          */
	int	errors = 0;	/* Count of errors testing current file     */
	int	severe = 0;	/* Count of severe errors on current file   */
	int	rc;		/* Return code from subroutines             */
	struct  fullstat linkbuf; /* Information about linked file          */
	struct  fullstat *ck_buf; /* Pointer to file to check               */
        struct  fullstat statbuf;
        struct  tcbent  *tp;
	char    link[PATH_MAX]; /* buffer to read symbolic link into */
	int     length;         /* length of symbolic link name */
	FILE	*fp;		/* File pointer used for piping commands    */

	/*
	 * Initialize local variables and such
	 */

	file = tcbent->tcb_name;
	just_created = 0;

        if (tp = hash_lookup (tcbent->tcb_name)) {
        	MSG_S(MSG_Duplicate_Stanza,DEF_Duplicate_Stanza,
                      tp->tcb_name,0,0,0);
                errors++;
        }
        fullstat (tcbent->tcb_name, FL_STAT|FL_NOFOLLOW, &statbuf);

	/*
	 * See if this file has a source attribute.  It must be copied
	 * afresh if there is one.  Also, the source attribute must be
	 * an absolute path name =and= exist, even if it is not to be
	 * copied.
	 */

	if (! ck_source (tcbent) || tcbent->tcb_type != TCB_FILE) {
		if (tcbent->tcb_source) {
				if (cp_source (tcbent)) {
					mk_audit (SYSCK_Check, AUDIT_FAIL,
						file, CantFix, "source");
					if (tcbent->tcb_source[0])
					MSG_S(MSG_Copy_Failed,DEF_Copy_Failed,
							tcbent->tcb_source,
							file,0,0);
					else
					MSG_S(MSG_Create_Failed,DEF_Create_Failed, file,0,0,0);

					severe++;
				} else
					mk_audit (SYSCK_Check, AUDIT_OK,
						file, Fixed, "source");
			errors++;
		}
	} else {
		if (tcbent->tcb_type == TCB_DIR)
			xmkdir(tcbent->tcb_name,tcbent->tcb_mode);
		errors++;
	}

	ck_buf = &statbuf;

       /*
	* See if this file/directory is a symlink.  If so, get the
	* destination file, and check it.
	*/

	if ( S_ISLNK(statbuf.st_mode) ) {
		if ((length = readlink (file, link, BUFSIZ)) == -1) {
			errors++;
		}
		else {
			link[length] = '\0';
			if ( (fullstat (link, FL_STAT|FL_NOFOLLOW,
                              &linkbuf))==-1 ) {
        			if (tcbent->tcb_type != TCB_SYMLINK) {
					MSG_S(MSG_No_Such_File,DEF_No_Such_File,file,0,0,0);
					errors++;
				}
			}
			else
				ck_buf = &linkbuf;
		}
	}

        /*
         * If the type is SYMLINK then check if the symlink exists.
         */

        if (tcbent->tcb_type == TCB_SYMLINK) {
                if (rc = ck_symlink_type (tcbent))
                        return -1;
        }

        /*
         * If the type is LINK then check if the hard link exists.
         */

        if (tcbent->tcb_type == TCB_LINK) {
                if (rc = ck_link_type (tcbent))
                        errors++;
        }

	/*
	 * Begin by checking the file type for correctness.  Give
	 * up immediately if it is wrong.
	 */

	if (ck_type (tcbent, ck_buf))
		return -1;

	/*
	 * Check for the correct file owner.
	 */

	if (tcbent->tcb_owner != (uid_t) -1 &&
			tcbent->tcb_owner != ck_buf->st_uid) {
		errors++;
                if (tcbent->tcb_type == TCB_SYMLINK) {
                        if (xlchown (file, tcbent->tcb_owner, ck_buf->st_gid)) {
                                severe++;
                                mk_audit (SYSCK_Check, AUDIT_FAIL,
                                        file, CantFix, "owner");
                        } else {
                                ck_buf->st_uid = tcbent->tcb_owner;
                                mk_audit (SYSCK_Check, AUDIT_OK,
                                        file, Fixed, "owner");
                        }
                }

                else {
                        if (xchown (file, tcbent->tcb_owner, ck_buf->st_gid)) {
                                severe++;
                                mk_audit (SYSCK_Check, AUDIT_FAIL,
                                        file, CantFix, "owner");
                        } else {
                                ck_buf->st_uid = tcbent->tcb_owner;
                                mk_audit (SYSCK_Check, AUDIT_OK,
                                        file, Fixed, "owner");
                        }
                }


	}

	/*
	 * Check for the correct file group.
	 */

	if (tcbent->tcb_group != (gid_t) -1 &&
			tcbent->tcb_group != ck_buf->st_gid) {
		errors++;

                if (tcbent->tcb_type == TCB_SYMLINK) {
                        if (xlchown (file, ck_buf->st_uid, tcbent->tcb_group)) {
                                mk_audit (SYSCK_Check, AUDIT_FAIL,
                                        file, CantFix, "group");
                                severe++;
                        } else {
                                ck_buf->st_gid = tcbent->tcb_group;
                                mk_audit (SYSCK_Check, AUDIT_OK,
                                        file, Fixed, "group");
                        }
                }

                else {

			if (xchown (file, ck_buf->st_uid, tcbent->tcb_group)) {
				mk_audit (SYSCK_Check, AUDIT_FAIL,
					file, CantFix, "group");
				severe++;
			} else {
				ck_buf->st_gid = tcbent->tcb_group;
				mk_audit (SYSCK_Check, AUDIT_OK,
					file, Fixed, "group");
			}
		}
	}

	/*
	 * Check for the correct file modes
	 */

	if (tcbent->tcb_mode != -1) {
		if (rc = ck_mode (tcbent, ck_buf)) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	 * Check for the correct ACL.
	 */

	if (tcbent->tcb_acl) {
		if (rc = ck_acl (tcbent)) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	 * Check for the correct number of links and each link having
	 * the proper destination.
	 */

	if (tcbent->tcb_links) {
		if (rc = ck_links (tcbent, ck_buf)) {
			if (rc == -1)
				severe++;

			errors++;
		}
	} else if (! S_ISDIR (ck_buf->st_mode) && ck_buf->st_nlink != 1) {
		errors++;
	}

	/*
	 * Check for the correct symbolic links.
	 */

	if (tcbent->tcb_symlinks) {
		if (rc = ck_symlinks (tcbent)) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	 * Validate the file using the specified program.  It is
	 * presumed the checking program has made any required
	 * modifications to the file if an error is detected.
	 */

	if (tcbent->tcb_program) {
		if (rc = ck_program (tcbent)) {
			if (rc == -1)
				severe++;

			errors++;
		}
	}

	/*
	 * Validate the file using the checksum.  This is an uncorrectable
	 * error.
	 */

	if (tcbent->tcb_checksum) {
		if (rc = ck_checksum (tcbent)) {
			if (! Dont_Remove_ACL)
		   	   severe++;
			errors++;
		}
	}

	/*
	 * Validate the file's size.  This is an uncorrectable error.
	 */

	if ((tcbent->tcb_size != -1) && (tcbent->tcb_size != 0)) {
		if (rc = ck_size (tcbent, ck_buf)) {
			if (! Dont_Remove_ACL)
		   	   severe++;
			errors++;
		}
	}

	/*
	 * Return error result.  If all of errors could be fixed I return
	 * ENOTRUST and set errno == ENOTRUST.  If any of the problems could
	 * =not= be fixed I return -1 and leave errno alone.
	 */

	if (severe)
		return -1;

	if (errors)
		return (errno = ENOTRUST);

	return 0;
}

