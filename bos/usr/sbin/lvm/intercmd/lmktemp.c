static char sccsid[] = "@(#)43	1.3  src/bos/usr/sbin/lvm/intercmd/lmktemp.c, cmdlvm, bos411, 9428A410j 1/18/94 10:06:43";
/*
 * COMPONENT_NAME: (CMDLVM) Logical Volume Commands
 *
 * FUNCTIONS: lmktemp
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*
 *	lmktemp template [size]	     - create a temporary file
 *
 *	- mktemp(3) is called with 'template' as its argument, so
 *	  lmktemp can be used to generate a mktemp-style file.
 *	- if 'size' is specified, fill the file with NULLS out to that size.
 *	- 'size' can be in dd-style units, eg:
 *		b,B	- 512 byte blocks
 *		k,K	- 1024 byte blocks
 *		m,M	- 1 meg blocks
 *	   so, 100M meets 100 megabytes.
 *
 *	- if successful, 0 is returned and the name of the temporary
 *	  file is printed on stdout
 *	- if unsuccessful, 1 is returned, the error is printed on stderr,
 *	  and no file is created
 *
 *	- note that if there's a file that currently exists with the
 *	  name indicated by template, that file is removed prior to
 *	  the call to mktemp
 *
 *	example usage:
 *
 *		tmpfile=`lmktemp /tmp/rmlvcopy.XXXXXX 100k`
 *		if [ $? != 0 ]
 *		then
 *			exit 1
 *		fi
 *
 *		lquerylv > $tmpfile
 *		rm $tmpfile
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/access.h>
#include <locale.h>
#include "cmdlvm_msg.h"

extern char *mktemp(char *);

extern char     *lvm_msg();
extern char     *Prog="lmktemp";    
 /* NLS Catalog descriptor for scmc conversion */
extern nl_catd  scmc_catd;        

off_t getsize(char *);
off_t units(char);

#define	min(x, y)	((x) < (y) ? (x) : (y))

/*
 * NAME:	main
 *
 * FUNCTION:	main - mainline code
 *
 * NOTES:	checks args, creates & optionally fills the file
 *
 * RETURN VALUE DESCRIPTION:	0 on success, 1 on failure
 */
int
main(int argc, char *argv[])
{
	int fd;
	char *prog;
	char *template;
	off_t size, bytes;
	static char buf[4096];		/* hardcode 4k buffer */

	/* set up for NLS */
	setlocale(LC_ALL, "");

	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	/*
	 * check command line
	 */
	if (argc < 2 || argc > 3) {
		/*
		 * stderr is bogus here, but old habits are hard to break...
		 */
		fprintf(stderr, lvm_msg(LMKTEMP_USAGE), Prog);
		return (1);
		}

	/*
	 * remove 'template' if it already exists
	 */
	if (access(argv[1], E_ACC) == 0)
		(void) unlink(argv[1]);

	/*
	 * yank useful things from the command line...
	 */
	template = mktemp(argv[1]);
	size = (argc == 3) ? getsize(argv[2]) : 0;

	/*
	 * did mktemp fail?
	 */
	if (template == NULL || *template == NULL) {
		perror("mktemp");
		return (1);
		}

	/*
	 * create the file
	 */
	if ((fd = creat(template, 0666)) < 0) {
		perror(template);
		return (1);
		}

	/*
	 * fill it in
	 */
	for ( ; size > 0; size -= bytes) {
		bytes = min(sizeof(buf), size);
		if (write(fd, buf, bytes) != bytes) {
			perror(template);
			(void) unlink(template);
			return (1);
			}
		}

	/*
	 * close it (check return code for NFS, etc)
	 */
	if (close(fd) < 0) {
		perror(template);
		(void) unlink(template);
		return (1);
		}

	/*
	 * it worked!  print out the name and get out...
	 */
	printf("%s\n", template);

	return (0);
}

/*
 * NAME:	getsize
 *
 * FUNCTION:	parse the size argument and return the size
 *
 * RETURN VALUE DESCRIPTION:	the size
 */
off_t
getsize(char arg[])
{
	off_t unit, size;
	int last = strlen(arg) - 1;

	size = atoi(arg);

	/*
	 * unit specified?
	 */
	if (last > 0 && (unit = units(arg[last])) > 0)
		size *= unit;

	return (size);
}

/*
 *	unit indicators
 */
struct {
	char	indicator;
	off_t	unit;
	} unitlist[] = {
		'b',	512,		/* blocks */
		'B',	512,		/* blocks */
		'k',	1024,		/* kbytes */
		'K',	1024,		/* kbytes */
		'm',	1024 * 1024,	/* megabytes */
		'M',	1024 * 1024,	/* megabytes */
		};

/*
 * NAME:	units
 *
 * FUNCTION:	return proper unit multiplier
 *
 * RETURN VALUE DESCRIPTION:	unit multiplier
 */
off_t
units(char indicator)
{
	int i;

	/* 
	 * search table
	 */
	for (i = 0; i < sizeof(unitlist) / sizeof(unitlist[0]); i++)
		if (indicator == unitlist[i].indicator)
			return (unitlist[i].unit);

	return (0);
}
