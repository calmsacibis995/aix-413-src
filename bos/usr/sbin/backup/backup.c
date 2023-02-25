static char sccsid[] = "@(#)95	1.18.1.4  src/bos/usr/sbin/backup/backup.c, cmdarch, bos411, 9430C411a 7/14/94 15:00:55";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: backup
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* This is the backup front end.  This command parses command
 * line options and based on those options will execl() the
 * appropriate backend, backbyname for byname backups or
 * backbyinode for BSD style byinode backups.
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include <nl_types.h>
#include "backup_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_BACKUP,N,S)
nl_catd catd;

#define OPTIONLETTER 3
#define OPTIONS "?cb:orqwWu0123456789ivpe:f:l:S"
#define BYNAME_USE "By Name Usage: \n\
    backup -i[oqv] [-b number] [-f device] [-l number] [-p[e regexp]]\n"
#define BYINODE_USE "By Inode Usage: \n\
    backup [-level] [-u] [-c] [-w|-W] [-b number] [-f device] [filesystem]\n"

char ByNameProgram[12] = "backbyname";
char ByInodeProgram[13] = "backbyinode";

main(int argc, char *argv[])
{
	/* parse command line and construct a vector as argv for
	 * the backend.  If the byname backup is requested, then
	 * we exec the backbyname program, else the user is requesting
	 * to backup byinode and we exec backbyinode which is a port
	 * of the Berkeley based dump program.
	 */
	
	int i, j, byname, byinode;
	int c, byname_opt, byinode_opt;
	char **arg_vec;
	char Backend[PATH_MAX];
        char string[PATH_MAX];

	(void) setlocale(LC_ALL,"");
	catd = catopen("backup.cat", NL_CAT_LOCALE);
	byname = byinode = 0;
	c = byname_opt = byinode_opt = 0;
	arg_vec = (char **)calloc((size_t)(argc+20), (size_t)sizeof (char *));
	j = 1;

	while ((c = getopt(argc, argv, OPTIONS)) != EOF) {
		switch(c) {
		/* These options applies to both backends */
		case 'f':
		case 'b':
			arg_vec[j] = (char *)malloc((size_t)OPTIONLETTER);
			arg_vec[j][0] = '-';
			arg_vec[j][1] = c;
			arg_vec[j][2] = '\0';
			++j;
			arg_vec[j] = optarg;
			break;
		/* These options are only for backbyinode */
		case 'u':
		case 'w':
		case 'W':
		case 'c':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		      	if (byname) 
				conflict(c, byname_opt);
		      	byinode = 1;
			byinode_opt = c;
			arg_vec[j] = (char *)malloc((size_t)OPTIONLETTER);
			arg_vec[j][0] = '-';
			arg_vec[j][1] = c;
			arg_vec[j][2] = '\0';
			break;
		/* These options are only for backbyname */
		case 'i':
		case 'l':
		case 'v':
		case 'o':
		case 'p':
		case 'e':
		case 'r':
		case 'q':
		case 'S':
			if (byinode)
				conflict(c, byinode_opt);
		      	byname = 1;
			byname_opt = c;
			arg_vec[j] = (char *)malloc((size_t)OPTIONLETTER);
			arg_vec[j][0] = '-';
			arg_vec[j][1] = c;
			arg_vec[j][2] = '\0';
			if (c == 'e' || c == 'l') {
				++j;
				arg_vec[j] = optarg;
			}
			break;
		case '?':
		default:
		      	usage();
			break;
	       	}
		++j;
	}
	if (byinode && (optind < argc - 1))    /* command line garbled */
		usage();
	if (optind < argc)
		arg_vec[j++] = argv[optind];   /* filesystem specified */
	arg_vec[j] = NULL;
	if (byname) 
		arg_vec[0] = ByNameProgram;
	else 
		arg_vec[0] = ByInodeProgram;
	sprintf(Backend, "%s/%s", "/usr/sbin", arg_vec[0]);
	execv(Backend, arg_vec);
        sprintf(string, MSGSTR(EXECF, "Exec of %s failed"), Backend);
        perror(string);
	exit(1);
}

conflict(c, opt)
char c, opt;
{
	fprintf(stderr, MSGSTR(OPTBAD, 
		"Option %c and option %c are mutually exclusive.\n"), c, opt);
	usage();
}

usage()
{
	fprintf(stderr, MSGSTR(USAGE1,BYNAME_USE));
	fprintf(stderr, MSGSTR(USAGE2,BYINODE_USE));
	exit(1);
}
