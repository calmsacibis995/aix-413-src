static char sccsid[] = "@(#)06        1.4.1.2  src/bos/usr/sbin/strchg/strchg.c, cmdpse, bos411, 9428A410j 5/10/94 19:11:27";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * strchg - change configuration of stream on stdin
 *
 * from man page in SVR4 SPG
 *
 * 11/17/90 dgb
 *
 * SYNOPSIS:
 *	strchg -h <module1> [,<module2> ...]
 *	strchg -p [-a | -u <module>]
 *	strchg -f <file>
 *
 *	options:
 *	-h <module>	pusH module(s)
 *	-p		Pop topmost module
 *	-p -a		Pop All modules
 *	-p -u <module>	Pop Until module <module>
 *	-f <file>	configure stream to resemble File <file>
 *	-?		print a usage message
 *
 * HISTORY:
 *	11/17/90 dgb	original version
 *	11/20/90 dgb	ported to AIX3
 *	91/04/27 dgb	removed config section (to strload)
 *	91/07/09 dgb	now silent if does nothing; "fixed" -f option
 *
 * TODO:
 *	- do not fail PUSHes and POPs with EINVAL; print a message
 *	- change -u option to use "more efficient" I_LIST
 */

#include <sys/types.h>
#include <sys/stropts.h>
#include <errno.h>
#include <stdio.h>

#include "strchg_msg.h"
#include <locale.h>
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_STRCHG,n,s)

void push(), pop(), file(), config();
void warn(), die(), usage();
extern char *strchr(), *strrchr(), *strcpy(), *memset(), *malloc();
extern void exit();

#define	Fprintf	(void)fprintf
#define	Sprintf	(void)sprintf
#define	Printf	(void)printf
#define	Memset	(void)memset
#define	Strcpy	(void)strcpy
#define	Strncat	(void)strncat

char *Prog;
extern int errno;

main(argc, argv)
	int argc;
	char **argv;
{
	int c;
	int fd = fileno(stdin);
	int popt = 0;
	int popall = 0;
	int flag = 0;
#define	H_FLAG	0x01
#define	P_FLAG	0x02
#define	F_FLAG	0x04
	char *arg = NULL;

	extern int optind;
	extern char *optarg;

	setlocale(LC_ALL, "");
	catd = catopen(MF_STRCHG, NL_CAT_LOCALE);

	if (Prog = strrchr(argv[0], '/'))
		argv[0] = ++Prog;	/* for getopt() */
	else
		Prog = argv[0];

	while ((c = getopt(argc, argv, "?h:pau:f:")) != EOF) {
		switch (c) {
		/* invocation form 1 */
		case 'h':	flag |= H_FLAG; arg = optarg; break;

		/* invocation form 2 */
		case 'p':	flag |= P_FLAG; break;
		case 'a':	++popt; popall = 1; break;
		case 'u':	++popt; arg = optarg; break;

		/* invocation form 3 */
		case 'f':	flag |= F_FLAG; arg = optarg; break;

		/* all cases */
		case '?':	/* fallthru */
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (!flag || (popt && flag != P_FLAG))
		usage();
	
	switch (isastream(fd)) {
	case 0:		errno = ENOSTR;	/* fallthrough */
	case -1:	die(MSGSTR(MSG1, "stdin"));
	}

	switch (flag) {
	case H_FLAG:
		push(fd, arg);
		break;
	case P_FLAG:
		if (popall && arg)
			die(MSGSTR(MSG2, "-a and -u are mutually exclusive"));
		pop(fd, popall, arg);
		break;
	case F_FLAG:
		file(fd, arg);
		break;
	default:
		usage();
	}

	exit(0);
	/* NOTREACHED */
}

/*
 * push - push modules onto stream
 *
 * modlist is a comma separated list of module names
 */

void
push(fd, modlist)
	int fd;
	char *modlist;
{
	char *cp;

	while (modlist) {
		if (cp = strchr(modlist, ','))
			*cp++ = '\0';
		if (*modlist) {	/* guard against ",mod" */
			if (ioctl(fd, I_PUSH, modlist) < 0)
				die(MSGSTR(MSG3, "ioctl I_PUSH"));
		}
		modlist = cp;
	}
}

/*
 * pop - pop modules from stream
 *
 * if all is set, pop all modules
 * if until is set, pop modules until a module named until is found
 * if neither are set, pop only the topmost module
 */

void
pop(fd, all, until)
	int fd;
	int all;
	char *until;
{
	int count = 0;
	char modname[FMNAMESZ+1];

	if (all) {
		while (ioctl(fd, I_POP, 0) != -1)
			++count;
		if (errno != EINVAL)
			die(MSGSTR(MSG5, "ioctl I_POP"));
	} else if (until) {
		for (count = 0; ; ++count) {
			if (ioctl(fd, I_LOOK, modname) < 0) {
				if (errno != EINVAL)
					die(MSGSTR(MSG6, "ioctl I_LOOK"));
				exit(0);
			}
			if (!strcmp(modname, until))
				break;
			if (ioctl(fd, I_POP, 0) < 0)
				die(MSGSTR(MSG9, "ioctl I_POP"));
		}
	} else {
		if (ioctl(fd, I_POP, 0) < 0 && errno != EINVAL)
			die(MSGSTR(MSG11, "ioctl I_POP"));
	}
}

/*
 * file - make stream look like file
 *
 * file is newline separated list of names in the order they
 * should appear on the stream (top to bottom)
 */

void
file(fd, fname)
	int fd;
	char *fname;
{
	FILE *fp;
	char buf[100];
	int nmods;
	struct node {
		char *mod;
		struct node *next;
	} *np, head;
	struct str_list *list, *getlist();

	head.next = (struct node *)0;

	if (!(fp = fopen(fname, "r")))
		die(MSGSTR(MSG12, "cannot open %s"), fname);

	while (fgets(buf, sizeof buf, fp) != NULL) {
		if (!(np = (struct node *)malloc(sizeof(struct node))))
			die(MSGSTR(MSG13, "out of memory"));
		if (!(np->mod = malloc((unsigned)strlen(buf))))
			die(MSGSTR(MSG14, "out of memory"));
		buf[strlen(buf)-1] = '\0';	/* toast newline */
		Strcpy(np->mod, buf);
		np->next = head.next;
		head.next = np;
	}
	(void) fclose(fp);

	if (!(list = getlist(fd)))
		die("cannot get module list");

	/*
	 * compare file list (arranged bottom to top) with current list
	 * (arranged top to bottom); at first mismatch, pop remaining
	 * modules from stream, and push remainder of file list.
	 */

	for (np = head.next, nmods = list->sl_nmods; np && nmods; --nmods) {
		if (strcmp(list->sl_modlist[nmods-1].l_name, np->mod) != 0)
			break;
		np = np->next;
	}
	while (nmods-- > 0)
		if (ioctl(fd, I_POP, 0) < 0)
			die(MSGSTR(MSG5, "ioctl I_POP"));
	while (np) {
		if (ioctl(fd, I_PUSH, np->mod) < 0)
			die(MSGSTR(MSG15, "ioctl I_PUSH"));
		np = np->next;
	}
}

/*
 * getlist - initialize struct str_list with current module list
 *
 * returns a null pointer, or a pointer to a static struct str_list
 *
 * Note: the driver is "removed" from the list by decrementing sl_nmods.
 */

struct str_list *
getlist(fd)
	int fd;
{
	static struct str_list strlist;
	unsigned size;

	if ((strlist.sl_nmods = ioctl(fd, I_LIST, (char *)0)) < 0)
		return (struct str_list *)0;
	size = strlist.sl_nmods * sizeof(struct str_mlist);
	strlist.sl_modlist = (struct str_mlist *)malloc(size);
	if (ioctl(fd, I_LIST, (char *)&strlist) < 0)
		return (struct str_list *)0;
	strlist.sl_nmods--;	/* "remove" driver name from list */
	return &strlist;
}

void
usage()
{
	Fprintf(stderr, MSGSTR(USAGE1, "usage: %s -h <module1>[,<module2>,...]\n"), Prog);
	Fprintf(stderr, MSGSTR(USAGE2, "       %s -p [-a | -u <module1>]\n"), Prog);
	Fprintf(stderr, MSGSTR(USAGE3, "       %s -f <file>\n"), Prog);
	exit(1);
}

/* VARARGS */
void
warn(msg, a, b, c)
	char *msg;
	int a,b,c;
{
	char buf[100];

	Sprintf(buf, "%s: ", Prog);
	Sprintf(buf+strlen(buf), msg, a, b, c);
	if (errno != 0)
		perror(buf);
	else
		Fprintf(stderr, "%s\n", buf);
}

/* VARARGS */
void
die(msg, a, b, c)
	char *msg;
	int a,b,c;
{
	warn(msg, a, b, c);
	exit(1);
}
