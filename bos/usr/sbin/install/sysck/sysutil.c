static char sccsid[] = "@(#)22  1.9.1.9  src/bos/usr/sbin/install/sysck/sysutil.c, cmdinstl, bos411, 9432A411a 8/6/94 17:48:36";
/*
 * COMPONENT_NAME: cmdinstl
 *
 * FUNCTIONS: sig_ignore, sig_reset, isnumeric, xperror, xmalloc, xrealloc,
 *		xstrdup, xchown, xlchown, xlink, xmkdir, xmknod,
 *		xsymlink, xunlink, xremove, comma_list, null_list,
 *		get_program, put_program, validate_name,
 *		mk_checksum, fuzzy_compare, free_list, write_inv
 *              write_tcbent
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS
#include <sys/types.h>
#include <sys/access.h>
#include <sys/fullstat.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <sys/sysmacros.h>
#include <sys/priv.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <varargs.h>
#include <swvpd.h>
#include "sysdbio.h"
#include "sysmsg.h"
#include <ctype.h>

#define ZERO	'\0'

void	*signals[NSIG];
int	once;

extern	char	*malloc(unsigned);
extern	char	*realloc(void *, unsigned);
extern	int	vflg;

/*
 * NAME: sig_ignore
 *                                                                    
 * FUNCTION: Set user creatable signals to SIG_IGN.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: NONE
 */  

void
sig_ignore (void)
{
	once++;

	signals[SIGHUP] = (void *) signal (SIGHUP, SIG_IGN);
	signals[SIGINT] = (void *) signal (SIGINT, SIG_IGN);
	signals[SIGQUIT] = (void *) signal (SIGQUIT, SIG_IGN);
	signals[SIGTERM] = (void *) signal (SIGTERM, SIG_IGN);
	signals[SIGTSTP] = (void *) signal (SIGTSTP, SIG_IGN);
}

/*
 * NAME: sig_reset
 *                                                                    
 * FUNCTION: Reset user createable signals to their previous values
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: NONE
 */  

void
sig_reset (void)
{
	if (! once)
		return;

	signal (SIGHUP, (void (*) (int)) signals[SIGHUP]);
	signal (SIGINT, (void (*) (int)) signals[SIGINT]);
	signal (SIGQUIT, (void (*) (int)) signals[SIGQUIT]);
	signal (SIGTERM, (void (*) (int)) signals[SIGTERM]);
	signal (SIGTSTP, (void (*) (int)) signals[SIGTSTP]);

	once = 0;
}

/*
 * NAME: isnumeric
 *                                                                    
 * FUNCTION: Determine if a string is all digits
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero if there are any non-numeric characters, non-zero otherwise
 */  

int
isnumeric (char *str)
{
	char	*cp;

	if (str == 0 || *str == '\0')
		return 0;

	for (cp = str;*cp;cp++)
		if (! isdigit (*cp))
			return 0;

	return 1;
}

/*
 * NAME: xperror
 *                                                                    
 * FUNCTION: variable argument list perror()
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: NONE
 */  

void
xperror (va_alist)
va_dcl
{
	va_list	ap;
	char	buf[BUFSIZ];
	char	*cp;

	va_start (ap);

	strcpy (buf, va_arg (ap, char *));
	strcat (buf, "(");

	if (cp = va_arg (ap, char *)) {
		strcat (buf, cp);
		while (cp = va_arg (ap, char *)) {
			strcat (buf, ", ");
			strcat (buf, cp);
		}
	}
	strcat (buf, ")");

	perror (buf);
}

/*
 * NAME: xmalloc
 *                                                                    
 * FUNCTION: malloc() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Pointer to region
 */  

char *
xmalloc (int len)
{
	char	*cp;

	if (len == 0)
		len = 4;

	if (cp = (char *)malloc ((unsigned) len)) {
		memset ((void *) cp, 0, len);
		return (cp);
	}
        MSG_S(MSG_Out_Of_Memory,DEF_Out_Of_Memory,0,0,0,0);
        exit(ENOMEM);
	/*NOTREACHED*/
}

/*
 * NAME: xrealloc
 *                                                                    
 * FUNCTION: realloc() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Pointer to new region
 */  

char *
xrealloc (char *buf, int len)
{
	char	*cp;

	if (cp = realloc (buf, len))
		return (cp);

        MSG_S(MSG_Out_Of_Memory,DEF_Out_Of_Memory,0,0,0,0);
        exit(ENOMEM);
	/*NOTREACHED*/
}

/*
 * NAME: xstrdup
 *                                                                    
 * FUNCTION: Duplicate a string into a new buffer with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Pointer to string
 */  

char *
xstrdup (char *s)
{
	char	*cp;

	cp = xmalloc (strlen (s) + 1);
	return (strcpy (cp, s), cp);
}

/*
 * NAME: xchown
 *                                                                    
 * FUNCTION: xchown() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xchown (char *file, uid_t owner, gid_t group)
{
	char	sown[16];
	char	sgrp[16];

	if (chown (file, owner, group)) {
		sprintf (sown, "%d", owner);
		sprintf (sgrp, "%d", group);

		xperror ("chown", file, sown, sgrp, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xlink
 *                                                                    
 * FUNCTION: xlink() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xlink (char *from, char *to)
{
	char dir[PATH_MAX];
	char *strip;

	/* strip directory down one level */
	strip = (char *)strrchr(to,'/');	/* set strip to the last '/' */
	strip[0] = '\0';		/* set the last '/' to '\0'  */
	strcpy(dir,to); 	/* set dir to dir minus the last '/' */
	strip[0] = '/';		/* return to to its original state */
	mkpath(dir);

	if (link (from, to)) {
		xperror ("link", from, to, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xmkdir
 *                                                                    
 * FUNCTION: mkdir() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xmkdir (char *dir, int mode)
{
	char	buf[BUFSIZ];

	if (mkdir (dir, mode)) {
		sprintf (buf, "0%o", mode);
		xperror ("mkdir", dir, buf, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xmknod
 *                                                                    
 * FUNCTION: xmknod() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xmknod (char *file, int mode, dev_t dev)
{
	char	smode[16];
	char	sdev[16];

	if (mknod (file, mode, dev)) {
		sprintf (smode, "0%o", mode);
		sprintf (sdev, "<%d,%d>", major (dev), minor (dev));
		xperror ("mknod", file, smode,
			(S_ISBLK (mode) || S_ISCHR (mode)) ? sdev:0, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xsymlink
 *                                                                    
 * FUNCTION: symlink() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xsymlink (char *file, char *link)
{
	char dir[PATH_MAX];
	char *strip;

	/* strip directory down one level */
	strip = (char *)strrchr(link,'/');	/* set strip to the last '/' */
	strip[0] = '\0';		/* set the last '/' to '\0'  */
	strcpy(dir,link); 	/* set dir to dir minus the last '/' */
	strip[0] = '/';		/* return link to its original state */
	mkpath(dir);

	if (symlink (file, link)) {
		xperror ("symlink", file, link, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xunlink
 *                                                                    
 * FUNCTION: unlink() call with error checking
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xunlink (char *file)
{
	if (unlink (file)) {
		xperror ("unlink", file, 0);
		return -1;
	}
	return 0;
}

/*
 * NAME: xremove
 *                                                                    
 * FUNCTION: Remove various types of files, devices and directories
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
xremove (char *name, struct fullstat *stat)
{
	if (S_ISDIR (stat->st_mode)) {
		if (rmdir (name)) {
			xperror ("rmdir", name, 0);
			return -1;
		}
	} else {
		if (unlink (name)) {
			xperror ("unlink", name, 0);
			return -1;
		}
	}
	return 0;
}

/*
 * NAME: comma2null
 *
 * FUNCTION: Convert comma separated list into null separated list
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Pointer to double-null terminated list
 */

char *
comma2null (char *arg)
{
	char	*cp, *cp2;

	/*
	 * Duplicate the string so it can be modified in place.  The
	 * commas are replaced with NUL characters.
	 */

	cp = xmalloc (strlen (arg) + 2);
	memset (cp, 0, strlen (arg) + 2);
	strcpy (cp, arg);

	for (cp2 = cp;*cp2;cp2++)
		if (*cp2 == ',')
			*cp2 = '\0';

	return cp;
}

/*
 * NAME: comma_list
 *
 * FUNCTION: Convert comma separated list into an array of strings
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Pointer to array of allocated characters strings
 */

char **
comma_list (char *arg)
{
	char	*cp, *cp2;
	int	i;
	char	**list;

	/*
	 * Duplicate the string so it can be modified in place.  The
	 * commas are replaced with NUL characters.  Allocate a list
	 * of pointers for each [ plus the terminator ] pointer.
	 */

	cp = xstrdup (arg);

	for (cp2 = cp, i = 1;*cp2;cp2++)
		if (*cp2 == ',')
			i++;

	list = (char **) calloc (i + 1, sizeof (char *));

	/*
	 * Find each element in the string, point to it in the table
	 * and then NUL terminate.  The list itself must then be
	 * terminated with a (char *) 0.
	 */

	for (cp2 = cp, i = 0;*cp2;i++) {
		list[i] = cp2;

		while (*cp2 && *cp2 != ',')
			cp2++;

		if (*cp2)
			*cp2++ = '\0';
	}
	list[i] = 0;

	return list;
}

/*
 * NAME: null_list
 *
 * FUNCTION: Convert NUL separated list into an array of strings
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Pointer to array of allocated characters strings
 */

char **
null_list (char	*arg)
{
	char	*cp, *cp2;
	int	i;
	char	**list;

	/*
	 * Duplicate the string so it can be modified in place.  The
	 * commas are replaced with NUL characters.  Allocate a list
	 * of pointers for each [ plus the terminator ] pointer.
	 */

	for (i = 0, cp = arg;cp[0] || cp[1];cp++, i++)
		;

	cp = xmalloc (i + 2);
	memset (cp, 0, i + 2);
	memcpy (cp, arg, i);

	/*
	 * Count the number of strings in the list so an array of
	 * pointers can be allocated for it.
	 */

	for (cp2 = cp, i = 0;*cp2;) {
		i++;

		while (*cp2++)
			;
	}

	list = (char **) calloc (i + 1, sizeof (char *));

	/*
	 * Find each element in the string and point to the beginning
	 * of it.  NULL terminate the entire list when done.
	 */

	for (cp2 = cp, i = 0;*cp2;i++) {
		list[i] = cp2;

		while (*cp2++)
			;
	}
	list[i] = 0;

	return list;
}

/*
 * NAME: get_program
 *
 * FUNCTION: Execute a program and return its output
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Pointer to output string or NULL on error.
 */

char *
get_program (char *prog, char *arg)
{
	char	buf[MAXTCBSIZ];
	FILE	*fp;
	int	i;
	int	rc;

	/*
	 * Create the command line by appending the argument to
	 * the pathname which was provided in prog.
	 */

	if (! arg)
		strcpy (buf, prog);
	else
		sprintf (buf, "%s %s", prog, arg);

	/*
	 * Execute the program, and pipe the results back in to
	 * the buffer.  Report read or execution errors.
	 */

	if (! (fp = popen (buf, "r"))) {
	        MSG_S(MSG_Program_Error,DEF_Program_Error,prog,0,0,0);
		return 0;
	}
	i = fread (buf, 1, sizeof buf - 1, fp);
	if (i < 0 || ferror (fp)) {
	        MSG_S(MSG_Program_Error,DEF_Program_Error,prog,0,0,0);
		return 0;
	}

	/*
	 * Close the pipeline and NUL terminate the string.  Pass
	 * back a malloc'd version to the caller.
	 */

	if (pclose (fp))
	      MSG_S(MSG_Program_Error,DEF_Program_Error,prog,0,0,0);

	if (buf[i - 1] == '\n')
		buf[i - 1] = '\0';
	else
		buf[i] = '\0';

	return xstrdup (buf);
}

/*
 * NAME: put_program
 *
 * FUNCTION: Execute a program piping it's input from a buffer
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: Exit status from executed command.
 */

int
put_program (char *prog, char *arg, char *input)
{
	FILE	*fp;
	int	i;
	int	rc;
	char	buf[BUFSIZ];

	/*
	 * Create the command line by appending the argument to
	 * the pathname which was provided in prog.
	 */

	if (! arg)
		strcpy (buf, prog);
	else
		sprintf (buf, "%s %s", prog, arg);

	/*
	 * Execute the program, and pipe the input from the user's
	 * buffer.  Report write or execution errors.
	 */

	if (! (fp = popen (buf, "w"))) {
	        MSG_S(MSG_Program_Error,DEF_Program_Error,prog,0,0,0);
		return -1;
	}
	while (*input && ! ferror (fp))
		putc (*input++, fp);

	if (! ferror (fp)) {
		putc ('\n', fp);
		fflush (fp);
		if (rc = pclose (fp))
		MSG_S(MSG_Program_Error,DEF_Program_Error,prog,0,0,0);

		return rc;
	} else {
	        MSG_S(MSG_Program_Error,DEF_Program_Error,prog,0,0,0);
		(void) pclose (fp);
		return -1;
	}
}

/*
 * NAME: validate_name
 *                                                                    
 * FUNCTION: Test a name for containing special character
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process.
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */  

int
validate_name (char *name)
{
	return strpbrk (name, "~`'\"$^&()\\|{}[]<>?") != 0;
}

/*
 * NAME: mk_checksum
 *                                                                    
 * FUNCTION: Sum the bytes in a file
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Reads a file byte-by-byte summing each byte as it goes.
 *
 * RETURNS: Zero on success, non-zero otherwise
 */  

unsigned long
mk_checksum (char *file)
{
	int	c;
	unsigned tmp = 0;
	FILE	*fp;

	/* 
	** Removed by Dave Geise 7/15/91. Defect 26885.
	**
	** if (!vflg)
	** 	return(-1);
	*/

	if (! (fp = fopen (file, "r")))
		return (unsigned) -1;

	while ((c = getc (fp)) != EOF) {
		if (tmp & 01)
			tmp = ((tmp >> 1) + 0x8000);
		else
			tmp >>= 1;

		tmp += c;
		tmp &= 0xffff;
	}
	fclose (fp);
	return tmp;
}

/*
 * NAME: mk_sum
 *
 * FUNCTION: perform processing similar to "/bin/sum" command
 *
 * NOTE:
 *
 *	mk_sum() is called if BuiltInSum is TRUE and the user needs
 *	to checksum a file.  It will invoke mk_checksum, and save
 *	away the checksum value.
 *
 * RETURNS: pointer to static character array with string, or NULL
 *	on error.
 */

char *
mk_sum (struct tcbent *tcb)
{
	static	char	checksum[20];
	struct	stat	sb;

	/*
	 * Compute the checksum, save it away, and set the flag
	 * that say's we've been here before.
	 */

	tcb->tcb_sum = mk_checksum (tcb->tcb_name);
	if (tcb->tcb_sum == (unsigned) -1) {
	        MSG_S(MSG_Program_Error,DEF_Program_Error,"sum",0,0,0);
		return 0;
	}
	tcb->tcb_vsum = 1;

	sprintf (checksum, "%u", tcb -> tcb_sum);

	return checksum;
}

/*
 * NAME: fuzzy_compare
 *                                                                    
 * FUNCTION: Compare two strings, ignore differences in whitespace
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * NOTES:
 *	Similiar to strcmp, except differences in the amount and
 *	type of whitespace are ignored.  Strings must match
 *	byte-for-byte otherwise.
 *
 * RETURNS: Zero on match, non-zero otherwise
 */  

int
fuzzy_compare (char *a, char *b)
{
	int	newline = 1;

	/*
	 * Scan both strings, looking for a difference, or the
	 * end of both strings ...
	 */

	while (*a && *b) {

		/*
		 * At the beginning of a line, eat whitespace in both
		 * strings.
		 */

		if (newline) {
			while (*a == ' ' || *a == '\t')
				a++;

			while (*b == ' ' || *b == '\t')
				b++;

			newline = 0;
		}

		/*
		 * Whitespace matches if both strings have whitespace
		 * at the same general location in the string.  Newlines
		 * are considered to be whitespace, to some extent -
		 * but you can't have more consecutive newlines in one
		 * string than another.  The intent is simply to gobble
		 * up trailing blanks.
		 */

		if ((*a == ' ' || *a == '\t' || *a == '\n')
				&& (*b == ' ' || *b == '\t' || *b == '\n')) {
			while (*a == ' ' || *a == '\t')
				a++;

			while (*b == ' ' || *b == '\t')
				b++;

			if (*a != '\n' && *b != '\n')
				continue;

			newline = 1;
			a++, b++;
			continue;
		}

		/*
		 * Now each byte must match.  This is just a regular
		 * strings compare at this time
		 */

		if (*a++ != *b++)
			return -1;
	}

	/*
	 * One or both of the strings have terminated.  If "a" is done,
	 * strip any whitespace in this position off of "b".  Do the
	 * same for "b".  This makes trailing whitespace insignificant.
	 *
	 * Once that's over and done with, return 0 if both strings have
	 * been scanned completely.
	 */


	if (*a == '\0' && (*b == ' ' || *b == '\t')) {
		while (*b == ' ' || *b == '\t')
			b++;
	}
	if (*b == '\0' && (*a == ' ' || *a == '\t')) {
		while (*a == ' ' || *a == '\t')
			a++;
	}
	return *a != '\0' || *b != '\0';
}

/*
 * NAME: free_list
 *
 * FUNCTION: free a NULL-terminated list of strings
 *
 * RETURNS: NONE
 */

void
free_list (char **list)
{
	char	**org = list;

	while (*list)
		free (list++);

	free (org);
}

/*
 * NAME: list_null
 *
 * FUNCTION: convert a list of strings to NUL-separate format
 *
 * RETURNS: Address of buffer, or NULL on error.
 */

char *
list_null (char **list, char *string, int i)
{
	char	*org = string;
	char	*cp = string;
	int	len;

	while (*list) {
		if ((len = strlen (*list)) > i + 2)
			return 0;

		strcpy (cp, *list++);
		cp += len;
		*cp++ = '\0';
		i -= (len + 1);
	}
	*cp = '\0';

	return org;
}


/*
 * NAME: mkpath 
 *
 * FUNCTION: verifies the existance of a directory.  If the directory 
 *           does not exist and the parent directory is writable, the
 *           original directory will be created.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS:
 *    0 if the directory is writable and exists
 *    -1 if the directory is unwritable 
 */

int mkpath(
char *dir)		/* path to the file that needs created	*/
{
	char *strip;                 /* pointer to the last "/" in dir */
	char new_dir[PATH_MAX];               /* the striped directory */
	int  rc;                     /* return code from mkdir */

	if(*dir == '\0')
		return(0);

	if (access(dir,E_ACC) == 0){  	/* the file DOES exist */
		return(0);
	}
	else {	/* the directory DOES NOT exist */
		if(mkdir(dir,0755) == 0)
			return(0);

		/* strip directory down one level */
		strip = (char *)strrchr(dir,'/');/* set strip to the last '/' */
		strip[0] = '\0';		/* set the last '/' to '\0'  */
		strcpy(new_dir,dir); /* set new_dir to dir minus the last '/' */
		strip[0] = '/';		/* return dir to its original state */

		/* call mkpath with the striped down directory */
		if (strip != dir) {
			if (mkpath(new_dir) == 0) {
				mkdir(dir,0755);
				return(0);
			}
		}

		/* everything below the directory exists so make this directory */
		return(mkdir(dir,0755));

	} /* end else(not there) */

}

/*
 * NAME: array_comma
 *
 * FUNCTION: converts an arrar of strings into a comma separate list.
 *
 * RETURNS: Address of buffer, or NULL on error.
 */

char *
array_comma (char **array)
{
	char	*new;
	int	len=0,i;

	for (i=0;array[i] != (char *) NULL && array[i][0] != '\0';i++)
		len += strlen(array[i]) + 1;

	new = (char *) malloc(len);

	new[0] = '\0';
	for (i=0;array[i] != (char *) NULL && array[i][0] != '\0'; i++) {
		if (i != 0)
			strcat(new,",");
		strcat(new,array[i]);
	}

	return new;
}

/*
 * NAME: write_tcbent
 *
 * FUNCTION: Writes tcbent structure to savefile.
 *
 * RETURNS: None
 */
void write_tcbent (FILE *fp,struct tcbent *tcb,inv_t inv)
{
        int i;
	/* write tcbent data structure */

        /* print file name first */
        fprintf (fp,"%s:\n",tcb->tcb_name);
        if (tcb->tcb_class) {
           fprintf (fp,"\tclass = %s",tcb->tcb_class[0]);
           for (i=1;tcb->tcb_class[i];i++)
               fprintf(fp,",%s",tcb->tcb_class[i]);
           fprintf (fp,"\n");
        }
	switch (inv.file_type) {
               case 0:
                       fprintf (fp,"\ttype = FILE\n");
                       break;
               case 1:
                       fprintf (fp,"\ttype = DIRECTORY\n");
                       break;
               case 2:
                       fprintf (fp,"\ttype = FIFO\n");
                       break;
               case 3:
                       fprintf (fp,"\ttype = BLK_DEV\n");
                       break;
               case 4:
                       fprintf (fp,"\ttype = CHAR_DEV\n");
                       break;
               case 5:
                       fprintf (fp,"\ttype = MPX_DEV\n");
                       break;
               case 6:
                       fprintf (fp,"\ttype = SYMLINK\n");
                       break;
               case 7:
                       fprintf (fp,"\ttype = LINK\n");
                       break;
               default:
                       fprintf (fp,"\ttype = FILE\n"); 
        }
        if (tcb->tcb_owner && tcb->tcb_owner != -1)
           fprintf (fp,"\towner = %i\n",tcb->tcb_owner);
        if (tcb->tcb_group && tcb->tcb_group != -1)
           fprintf (fp,"\tgroup = %i\n",tcb->tcb_group);
        if (tcb->tcb_acl)
           fprintf (fp,"\tacl = %s\n",tcb->tcb_acl);
        if (inv.checksum && (inv.file_type == TCB_FILE))
           fprintf (fp,"\tchecksum = %i\n",inv.checksum);
        if (tcb->tcb_links) {
           fprintf (fp,"\tlinks = %s",tcb->tcb_links[0]);
           for (i=1;tcb->tcb_links[i];i++)
               fprintf(fp,",%s",tcb->tcb_links[i]);
           fprintf (fp,"\n");
        }
        if (tcb->tcb_symlinks) {
           fprintf (fp,"\tsymlinks = %s",tcb->tcb_symlinks[0]);
           for (i=1;tcb->tcb_symlinks[i];i++)
               fprintf(fp,",%s",tcb->tcb_symlinks[i]);
           fprintf (fp,"\n");
        }
        if (tcb->tcb_program) {
           fprintf (fp,"\tprogram = %s",tcb->tcb_program[0]);
           for (i=1;tcb->tcb_program[i];i++)
               fprintf(fp,",%s",tcb->tcb_program[i]);
           fprintf (fp,"\n");
        }
        if (tcb->tcb_source)
           fprintf (fp,"\tsource = %s\n",(char *)tcb->tcb_source);
        if (tcb->tcb_target)
           fprintf (fp,"\ttarget = %s\n",(char *)tcb->tcb_target);
 
        /*
         * mode not written because it will be recreated if needed
         * based on the mode of the real file.
         */
 
        if ((inv.size != -1) && (inv.file_type == TCB_FILE))
           fprintf (fp,"\tsize = %i\n",inv.size);
        fprintf (fp,"\n");
}

/*
 * NAME: xlchown
 *
 * FUNCTION: lchown() call with error checking
 *
 * EXECUTION ENVIRONMENT:
 *
 *      User process
 *
 * RETURNS: Zero on success, non-zero otherwise.
 */

int
xlchown (char *file, uid_t owner, gid_t group)
{
        char    sown[16];
        char    sgrp[16];

        if (lchown (file, owner, group)) {
                sprintf (sown, "%d", owner);
                sprintf (sgrp, "%d", group);

                xperror ("lchown", file, sown, sgrp, 0);
                return -1;
        }
        return 0;
}

