static char sccsid[] = "@(#)19	1.11.1.8  src/bos/usr/sbin/strload/strload.c, cmdpse, bos41J, 9509A_all 1/4/95 17:23:12";
/*
 *   COMPONENT_NAME: cmdpse
 *
 *   FUNCTIONS: MSGSTR
 *		config
 *		die
 *		foreach
 *		ldstatus
 *		load_pse
 *		main
 *		mutex
 *		query
 *		sigoff
 *		sigon
 *		sthpresent
 *		unload_pse
 *		usage
 *		warn
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * strload - load streams and streams extensions
 *
 * 91/04/28 dgb
 *
 * SYNOPSIS:
 *	strload [-u|q] [-h <size>] [-f <file>] [-d <list>] [-m <list>] [<pse>]
 *
 *	options:
 *	<pse>		name of streamhead to load (default: PSE)
 *	-h <size>	size (in k) of heap to allocate
 *	-H <size>	maximum size (in k) of heap
 *	-u 		unload instead of load
 *	-q		query load status instead of load
 *	-f <file>	(un)configure devices/modules in <filename>
 *	-d <list>	(un)load devices in <list>
 *	-m <list>	(un)load modules in <list>
 *	-?		print a usage message
 *
 * EXIT CODE:
 *	number of errors detected
 *
 * HISTORY:
 *	91/04/28 dgb	created
 *	91/05/17 dgb	added file configuration, simplified, bugfixes
 *	91/06/08 dgb	changed types into attributes, added explicit minors
 *	91/06/26 dgb	reworked device creation code - now "failsafe"
 *	91/08/02 dgb	unload now conditional, with override (-F)
 *	93/02/26 dgb	APAR ix34492 Defect  82071 - sth reload foobar
 *	93/03/02 dgb	APAR         Defect  82072 - bogus device removal
 *	93/03/02 dgb	APAR         Defect  82073 - critical section protect
 *	93/03/03 dgb	APAR         Defect  82075 - mknod fail, cannot unload
 *	93/03/04 dgb	APAR         Defect  82078 - smarter -q
 *	93/03/10 dgb	APAR ix34511 Defect  82080 - mutex
 *	93/12/05 dgb	APAR         Defect 118842 - multiple clone not found
 *	94/08/05 dgb	APAR         Defect 151742 - extract library routines
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <sys/mode.h>
#include <sys/stat.h>
#include <nlist.h>

#include <sys/strinfo.h>
#include <sys/strconf.h>

#include "strload_msg.h"
#include <locale.h>
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_STRLOAD,n,s)

char PSE[] =		"pse";			/* default streamhead */
char CFGFILE[] =	"/etc/pse.conf";	/* default config file */
char LOCKFILE[] =	"/tmp/.strload.mutex";	/* mutex lock file */

#define	NUMMINORS	5
#define	DRIVER		0
#define	MODULE		1

/* predeclarations */
int load_pse(), unload_pse();
char *basename();
void warn(), die(), usage(), ldstatus();
void sigoff(), sigon();
extern int atoi();
extern void exit();

/* lint hacks */
#define	Fprintf	(void)fprintf
#define	Sprintf	(void)sprintf
#define	Printf	(void)printf
#define	Strcpy	(void)strcpy
#define	Strcat	(void)strcat
#define	Strncat	(void)strncat

char *Prog;
int Heapsize = 0;		/* heap increment size (0=default) */
int Maxheap = 0;		/* heap maximum size (0=default) */
int Verbose = 0;

/*
 * mutex - force strloads to run one-at-a-time
 */

void
mutex()
{
	static struct flock flock = { F_WRLCK, SEEK_END, 0, 1, };
	int fd;

	if ((fd = open(LOCKFILE, O_CREAT|O_RDWR, 0600)) < 0)
		die(errno, MSGSTR(MSG1, "cannot open %s"), LOCKFILE);
	if (fcntl(fd, F_SETLKW, &flock) < 0)
		die(errno, MSGSTR(MSG4, "cannot initialize %s"), LOCKFILE);
}

main(argc, argv)
	int argc;
	char **argv;
{
	int c;
	int rc = 0;			/* return code */
	int cmd = 0;			/* command to perform */
	char *devlist = NULL;
	char *modlist = NULL;
	char *path = NULL;
	char *pse_name = NULL;

	extern int optind;
	extern char *optarg;

	setlocale(LC_ALL, "");
	catd = catopen(MF_STRLOAD, NL_CAT_LOCALE);
	/*
	 * Wunderbar - if the message file doesn't exist,
	 * catgets hits errno, thus invalidating what we needed.
	 * Priming the pump here avoids such antisocial behavior.
	 */
	(void)catgets(catd,MS_STRLOAD,1,"foobar");

	if (Prog = strrchr(argv[0], '/'))
		argv[0] = ++Prog;	/* for getopt() */
	else
		Prog = argv[0];

	while ((c = getopt(argc, argv, "?uFqh:H:d:m:f:vS:")) != EOF) {
		switch (c) {
		case 'u':	if (cmd) usage(); cmd = STR_UNLOAD; break;
		case 'q':	if (cmd) usage(); cmd = STR_QUERY; break;
		case 'F':	/* obsolete (Force flag) */ break;
		case 'h':	Heapsize = atoi(optarg); break;
		case 'H':	Maxheap = atoi(optarg); break;
		case 'd':	devlist = optarg; break;
		case 'm':	modlist = optarg; break;
		case 'f':	path = optarg; break;
		case 'v':	Verbose = 1; break;
		case 'S':	pse_name = optarg; break;
		case '?':	/* fallthru */
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc)
		usage();

	mutex();	/* one strload at a time */

	if (!cmd)
		cmd = STR_LOAD;
	if (!(path || devlist || modlist))
		path = CFGFILE;
	
	/* if -q && -S, check for streamhead; exit if no other arguments */
	if (cmd == STR_QUERY && pse_name) {
		ustrconf_t conf;
		int rc;
		conf.version = STRCONF_V1;
		conf.extname = pse_name;
		rc = query(&conf);
		if (path == CFGFILE)
			exit(rc);
	}

	if (!pse_name)
		pse_name = PSE;

	/* if unloading and no streamhead, there can be no modules/drivers */
	if (cmd == STR_UNLOAD && !sthpresent())
		exit(0);

	odm_initialize();

	/*
	 * Note: single functions return success or fail,
	 *       iterative functions return number of failures
	 */

	if (cmd == STR_LOAD) {
		sigoff();
		if (load_pse(pse_name))
			exit(1);	/* no point in continuing */
		sigon();
	}

	if (path)
		rc += config(cmd, path);
	if (devlist)
		rc += foreach(cmd, devlist, 1);
	if (modlist)
		rc += foreach(cmd, modlist, 0);

	if (cmd == STR_UNLOAD) {
		sigoff();
		rc += unload_pse(pse_name) ? 1 : 0;
		sigon();
	}

	/*
	 * NB: We do *not* want to exec slibclean on just any unload
	 *     because that would make it difficult to detect and
	 *     correct failures in a scripted environment.  The best
	 *     we can do now is to exec slibclean only after a
	 *     successful unload_pse() - the retcode would be 0 anyway.
	 *     See unload_pse() for details.
	 */

	exit(rc);
	/* NOTREACHED */
}

void
usage()
{
	Fprintf(stderr, MSGSTR(USAGE,
	"usage: %s [-v][-u|-q][-H size][-h size][-f file][-d list][-m list]\n"),
									Prog);
	exit(1);
}

/* VARARGS */
void
warn(err, msg, a, b, c)
	int err;
	char *msg;
	int a,b,c;
{
	Fprintf(stderr, "%s: ", Prog);
	Fprintf(stderr, msg, a, b, c);
	if (err)
		Fprintf(stderr, ": %s", strerror(err));
	Fprintf(stderr, "\n");
}

/* VARARGS */
void
die(err, msg, a, b, c)
	int err;
	char *msg;
	int a,b,c;
{
	warn(err, msg, a, b, c);
	exit(1);
}

/*
 * ldstatus - report success/failure of operation
 */

void
ldstatus(cmd, name, arg)
	int cmd;
	char *name, *arg;
{
	static char *cmdtxt[] = { "", "loaded", "unloaded", "queried", 0 };

	Printf("%s %s", cmdtxt[cmd], name);
	if (arg) Printf(" (%s)", arg);
	Printf("\n");
}

/*
 * query - report existance of extension in kernel
 *
 * prints "$extension: yes" or "$extension: no" to stdout
 * returns -1 on error, 0 on no, 1 on yes
 */

int
query(ustrconf_t *conf)
{
	int rc;

	if ((rc = strconfig(STR_QUERY, conf)) < 0)
		strconferr(stderr, Prog, 1);
	else
		Printf("%s: %s\n", conf->extname,
			rc ? MSGSTR(MSG15, "yes") : MSGSTR(MSG16, "no"));
	return rc > 0 ? 1 : rc;
}

/*
 ***************************************************
 * Iterative Functions
 *
 * Each routine in this section should return the
 * number of failures as a positive integer.  Diagostics
 * are handled at the next lower level, fatality
 * decisions at the next highest.
 ***************************************************
 */

/*
 * foreach - foreach item in list, do the command
 *
 * returns number of failures
 *
 * list is a comma delimited list of strings (e.g. "one,two,three")
 */

int
foreach(int cmd, char *list, int isdev)
{
	ustrconf_t conf;
	int rc = 0;
	int err;

	for (list = strtok(list, ","); list; list = strtok(0, ",")) {
		if (*list) {	/* guard against ",mod" */
			bzero(&conf, sizeof(conf));
			conf.version = STRCONF_V1;
			conf.extname = list;
			conf.flags = isdev ? SF_DEVICE : SF_MODULE;
			conf.maj = -1;
			conf.dds = basename(list);
			conf.ddslen = strlen(list);
			if (cmd == STR_QUERY)
				err = query(&conf);
			else {
				sigoff();
				err = 0;
				if (strconfig(cmd, &conf)) {
					strconferr(stderr, Prog, Verbose);
					err = 1;
				}
				sigon();
			}
			rc += err;
			if (Verbose && !err)
				ldstatus(cmd, conf.extname, NULL);
		}
	}
	return rc;
}

/* 
 * config - (un)configure PSE from a file
 *
 * returns number of failures
 *
 * format: attributes pathname [argument [node [minor ...]]]
 * field#: 0          1        2         3      4     ...
 *
 * hash marks begin comments, whitespace is the field delimiter
 * attributes are case-sensitive, and may consist of the following:
 * (d and m are mutex, and one of them is mandatory)
 *	d	driver
 *	m	module
 *	s	standard driver (not clone)
 *	+	allow multiple configuration
 */

int
config(cmd, path)
	int cmd;
	char *path;
{
	FILE *fp, *fopen();
	char buf[512];
	char *cp, **cpp;
	char *whitespace = " \t";
	char *field[4+NUMMINORS+1];	/* +1 for nullterm of minor list */
	node_t nodes[NUMMINORS+1];
	ustrconf_t conf;
	int i;
	int rc = 0, err = 0;
	int type, dupload, std;
	int lineno = 0;

	extern char *mkdevname();

	if (!(fp = fopen(path, "r"))) {
		warn(errno, MSGSTR(MSG1, "cannot open %s"), path);
		return 1;
	}
	
	while (fgets(buf, sizeof buf, fp)) {
		++lineno;

		/* remove comments, null lines */
		if (cp = strpbrk(buf, "#\n"))
			*cp = 0;

		if (!(cp = strtok(buf, whitespace)))
			continue;

		/* parse fields - sets missing fields to null */
		for (i = 0; i < sizeof(field)/sizeof(field[0])-1; i++) {
			field[i] = cp;
			cp = strtok((char *)0, whitespace);
		}
		field[i] = (char *)0;
		if (cp)
			warn(0, MSGSTR(MSG30,
				"line %d: only first %d minors used"),
				lineno, NUMMINORS);

		type = -1;
		dupload = 0;
		std = 0;

		/* verify parsed fields */
		for (cp = field[0]; *cp; ++cp) {
			switch (*cp) {
			case 'd':	type = DRIVER; break;
			case 'm':	type = MODULE; break;
			case 's':	std = 1; break;
			case '+':	dupload = 1; break;
			default:
				warn(0, MSGSTR(MSG31,
					"line %d: unknown attribute '%c'"),
					lineno, *cp);
				goto badattr; /* boo hoo: no labeled loops */
			}
		}
		if (type == -1) {
			warn(0, MSGSTR(MSG32,"line %d: missing extension type"),
				lineno);
	badattr:
			continue;
		}
		if (!field[1]) {
			warn(0, MSGSTR(MSG33, "line %d: no filename present"),
				lineno);
			continue;
		}
		if (!field[2] || !strcmp(field[2], "-"))
			field[2] = basename(field[1]);
		if (!field[3] || !strcmp(field[3], "-"))
			field[3] = basename(field[1]);
		
		/* fields: 0=attr, 1=extname, 2=arg, 3=node, 4=minors */
		bzero(&conf, sizeof(conf));
		conf.version = STRCONF_V1;
		conf.extname = field[1];
		if (type == DRIVER)
			conf.flags |= SF_DEVICE;
		if (dupload)
			conf.flags |= SF_DUP;
		conf.dds = field[2];
		conf.ddslen = strlen(conf.dds)+1;

		if (type == DRIVER) {
			conf.maj = -1;
			conf.nodes = nodes;

			nodes[0].dev = std ? makedev(-1, 0) : -1;
			nodes[0].name = mkdevname(field[3], 0);
			for (i=1, cpp = &field[4]; *cpp; ++i, ++cpp) {
				nodes[i].dev = makedev(-1, atoi(*cpp));
				nodes[i].name = mkdevname(nodes[0].name, *cpp);
				if (!nodes[i].name)
					die(errno, "");
			}
			nodes[i].dev = 0;
			nodes[i].name = NULL;

			conf.odmkey = nodes[0].name;
		}

		/* do the deed */
		if (cmd == STR_QUERY)
			err = query(&conf);
		else {
			sigoff();
			err = 0;
			if (strconfig(cmd, &conf)) {
				strconferr(stderr, Prog, Verbose);
				err = 1;
			}
			sigon();
		}
		rc += err;
		if (Verbose && !err)
			ldstatus(cmd, conf.extname, conf.dds);
	}
	(void)fclose(fp);
	return rc;
}

/*
 * sthpresent - return true if a streamhead is present
 *
 * NB: doesn't check if sth is _active_, only present
 */

static int
sthpresent()
{
	static struct nlist nlist[1] = { { "str_config" } };
	return knlist(nlist, 1, sizeof(struct nlist)) == 0 ? 1 : 0;
}

/*
 * load_pse - load the stream head
 *
 * returns 0 or -1
 */

static node_t nodes[] = {
	{ -1, "clone" },
	{ -1, "sad" },
	{ -1, "slog" },
	0
};

int
load_pse(name)
	char *name;
{
	strmdi_t mdi;
	ustrconf_t conf;

	/* try *really* hard to avoid reloading active streamhead */
	if (sthpresent())
		return 0;
	
	mdi.heapsize = Heapsize;
	mdi.maxheap = Maxheap;
	if ((mdi.clonemaj = genmajor("/dev/clone")) == -1 ||
	    (mdi.sadmaj   = genmajor("/dev/sad"  )) == -1 ||
	    (mdi.logmaj   = genmajor("/dev/slog" )) == -1)
		return -1;

	conf.version = STRCONF_V1;
	conf.extname = name;
	conf.flags = SF_MODULE;
	conf.dds = &mdi;
	conf.ddslen = sizeof(mdi);

	if (strconfig(STR_LOAD, &conf)) {
		strconferr(stderr, Prog, Verbose);
		return -1;
	}

	nodes[0].dev = makedev(-1, 0);
	nodes[1].dev = makedev(-1, mdi.sadmaj);
	nodes[2].dev = makedev(-1, mdi.logmaj);

	(void)mkdevice(nodes, mdi.clonemaj);

	return 0;
}

/*
 * unload_pse - unloads stream head
 *
 * returns 0 or -1
 */

unload_pse(name)
	char *name;
{
	ustrconf_t conf;

	conf.version = STRCONF_V1;
	conf.extname = name;
	conf.flags = SF_MODULE;
	conf.dds = (char *)0;

	if (strconfig(STR_UNLOAD, &conf)) {
		if (errno == EEXIST)	/* still have loaded modules/drivers */
			return 0;
		strconferr(stderr, Prog, Verbose);
		return -1;
	}
	(void)rmdevice(nodes, 0);
	/* XXX should probably relmajor clone, sad and slog here */

	/*
	 * XXX this hack forces AIX to cleanup after itself.
	 * Unfortunately, slibclean *must* be exec'd, since
	 * strload still has a use-count attached to the
	 * kernel extension (it called the config routine).
	 * Sigh.
	 *
	 * Old code:
	 * return retcode;
	 */

	return execl("/etc/slibclean", "slibclean", (char *)0);
}

/*
 * sigoff - turn off (or defer) signals
 */

static sigset_t nset, oset;

void
sigoff()
{
	static int first = 1;
	if (first) {
		sigemptyset(&nset);
		sigaddset(&nset, SIGINT);
		sigaddset(&nset, SIGTERM);
		sigaddset(&nset, SIGQUIT);
		first = 0;
	}
	if (sigprocmask(SIG_BLOCK, &nset, &oset))
		warn(errno, "cannot block signals");
}

/*
 * sigon - turn on (or release) signals
 */

void
sigon()
{
	if (sigprocmask(SIG_SETMASK, &oset, NULL))
		warn(errno, "cannot unblock signals");
}
