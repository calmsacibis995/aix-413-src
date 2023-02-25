static char sccsid[] = "@(#)53        1.2.1.2  src/bos/usr/sbin/strclean/strclean.c, cmdpse, bos411, 9428A410j 11/16/93 08:47:14";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   FUNCTIONS: MSGSTR
 *		main
 *		
 *
 *   ORIGINS: 27,63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/** Copyright (c) 1990  Mentat Inc.
 ** strclean.c 2.1, last change 5/2/91
 **/


#include <pse/common.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

#include "strclean_msg.h"
#include <locale.h>
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_STRCLEAN,n,s)

#ifndef DEFAULT_DIRECTORY
#define	DEFAULT_DIRECTORY	"/var/adm/streams"
#endif

#define	DAY_IN_SECONDS		(60 * 60 * 24)	/* seconds * minutes * hours */

#ifndef errno
extern	int	errno;
#endif

extern	char	noshare * optarg;
extern	int	noshare optind;

main (argc, argv)
	int	argc;
	char	** argv;
{
	long	age;
	char	buf[MAX_STACK_BUF];
	int	c;
	char	* dir_name;
	DIR	* dirp;
	struct dirent	* direntp;
	struct stat	st;
	struct tm	* tmp;
	time_t	cutoff_time;

	age = 3;
	dir_name = DEFAULT_DIRECTORY;

	setlocale(LC_ALL, "");
	catd = catopen(MF_STRCLEAN, NL_CAT_LOCALE);
	set_program_name(argv[0]);

	while ((c = getopt(argc, argv, "?d:a:")) != -1) {
		switch (c) {
		case 'a':	/* Age */
			age = atoi(optarg);
			break;
		case 'd':	/* Directory name */
			dir_name = optarg;
			break;
		case '?':
		default:
			fprintf(stderr, MSGSTR(USAGE1, "usage: strclean [-d logdir] [-a age]\n"));
			exit(1);
		}
	}
	/* Convert the age into seconds */
	age *= DAY_IN_SECONDS;

	/* Get current month and day */
	time(&cutoff_time);

	/*
	 * Convert to a cutoff time.
	 * All files older than this will be removed.
	 */
	cutoff_time -= age;

	dirp = opendir(dir_name);
	if (!dirp)
		return 0;

	/* Check all files in the directory. */
	while ( (direntp = readdir(dirp)) != NULL) {
		/*
		 * Ignore . and .. as well as all files that don't start
		 * with "error."
		 */
		if (strcmp(direntp->d_name, "..") == 0
		||  strcmp(direntp->d_name, ".") == 0
		||  strncmp(direntp->d_name, "error.", 6) != 0)
			continue;

		sprintf(buf, "%s/%s", dir_name, direntp->d_name);
		if (stat(buf, &st) != -1) {
			/*
			 * Compare the file modification time to the
			 * cutoff time.
			 */
			if (st.st_mtime < cutoff_time)
				unlink(buf);	/* Remove the file. */
		}
	}
	closedir(dirp);
	return 0;
}
