static char sccsid[] = "@(#)67	1.23.2.2  src/bos/usr/sbin/fsck/fsck.c, cmdfs, bos411, 9428A410j 1/10/94 17:56:41";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: fsck
 *
 * ORIGINS: 3, 27
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
 * FSCKMAIN
 *
 *      This is the module of fsck that reads and deciphers arguments,
 *      deciding which file systems to check in which order.  This is
 *      the module that knows about /etc/filesystems, roots, block vs
 *      raw devices, parallel passes and user specifiable flags.  It
 *      knows next-to-nothing about file systems.  Having decided what
 *      needs to be done, it invokes "check" to do the actual file system
 *	checks/repairs
 */

#include <nl_types.h>
#include "fsck_msg.h"
#define MSGSTR(N,S)	catgets(catd,MS_FSCK,N,S)
nl_catd catd;

#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <fcntl.h>		/* for O_RDWR */
#include "fshelp.h"
#include <varargs.h>
#include <IN/FSdefs.h>
#include <IN/AFdefs.h>
#include <IN/CSdefs.h>
#include "fsop.h"
#include "fsck.h"

/*
 * Almost all output to the console is performed via the routine "pr".
 * The first argument to "pr" is a flag word describing the message
 * to be printed and appropriate actions to take.  The following constants
 * are for use in that flag word.
 */
#define ARGX    1       /* argument (question or buffer) provided       */
#define ASKX    2       /* Message asks a question of the operator      */
#define FATALX  4       /* If answered no, abort program immediately    */
#define PREENX  010     /* This question defaults to yes in PREEN mode  */
#define YNX     020     /* Question requires YES or NO answer           */
#define ERROR   0100    /* If answered no, set xcode to EXIT_FAIL       */
#define NOB     0200    /* NO Beginning of line leader before message   */
#define NOE     0400    /* NO End of line should be printed after msg   */
#define PINODE  01000   /* Print out info on the "current" I-node       */
#define QUIET   02000   /* no-op if in quiet (preen) mode               */
#define TRIVIA  04000   /* this message is normally suppressed, -v only */
#define DDEBUG  010000   /* this message is normally suppressed, -D only */
#define CONTIN  (ASKX|FATAL|YNX)/* ask YES/NO and die if no             */
#define FATAL   (ERROR|FATALX)  /* abort check if question answered no  */
#define GETLINE (ARGX|ASKX)     /* ask general question, return answer  */
#define ASK     (ARGX|ASKX|YNX) /* ask YES/NO question, return answer   */
#define PREEN   (ASK|PREENX)    /* confirm making an obvious fix        */
#define ASKN    (ASK|ERROR)     /* question is an important one         */
#define PREENN  (ASKN|PREENX)   /* confirm making an important fix      */

/*
 * 'boolean' constants
 */
#define	YES		(1)
#define	NO		(0)

/*
 * usage
 */
char    Usage[] = "\
Usage: fsck [-y|-n|-p] [-f] [-V Vfs] [-d #] [-i #]\n\
\t[-t File] [-o Options] Filesystem ...\n";

/*
 * 'constant' type globals
 */
char    fsprofile[] = FSYSname; /* name of filesystem profile database  */
char    Badroot[] = "Problems with root file system";
char    Contin[] =  "CONTINUE";
char    Reboot[]   ="REBOOT";

/*
 * The major activity in this module is the construction of fsname and
 *     fsgroup structures to describe the checks which must be performed.
 *     The file systems to be checked are divided into groups (based on
 *     what head they are on).  The check list is, therefore, a chain of
 *     groups - each group being a chain of file systems.
 */
typedef struct fsname   FSNAME;
struct fsname
{       char    *name;          /* name of this file system     */
	FSNAME  *next;          /* pointer to next fs in group  */
	char     mounted;       /* is this file system mounted  */
	int	 readonly;	/* is it mounted readonly?	*/
	char	*vfsname;	/* vfs name			*/
};

typedef struct fsgroup  FSGROUP;
struct fsgroup
{       FSNAME  *list;          /* list of filesystem names     */
	FSGROUP *next;          /* pointer to next fs group     */
	char    num;            /* number of this group         */
} *groups;

/*
 * miscellaneous global variables within this module
 */
static  int     ngroups;        /* number of filesystem groups to check */
static  int     groupnum;       /* current group number we're checking  */
static  dev_t   rootdev = -1;   /* device designation of running root   */
static  char    root[PATH_MAX]; /* root fs name (if we're checking it)  */
static	int	xcode;		/* exit code value			*/
static	char	preen;		/* preen flag				*/
static	char	yflag;		/* -y flag				*/
static	char	nflag;		/* -n flag				*/
static	char	vflag;		/* -v flag				*/
static	char   *devname;	/* device name we are looking at	*/
static	char   *Vfsname;	/* command line specified vfs ...	*/

static	int	fsh_mode_flags;	/* file system helper mode flags	*/
static	int	fsh_debug_level; /* file system helper debug level	*/
static	char	fsh_opflags[2048]; /* file system helper fs flags	*/

/*
 * declarations for return types system calls and other routines
 */
extern  char   *FSdskname();
extern	int	getopt(), optind, opterr;	/* for getopt... */
extern	char	*optarg;			/* for getopt... */
extern	char	*sys_errlist[];

/*
 * local function types
 */
FSNAME *getname();
char   *myalloc();
char   *rawname();
char   *unraw();
void    parse_args();
void    check();
void    checkall();
int     pr();
int     dofork();
char   *getvfsname();
void    cmdargs(), autoargs();
char   *strsave();
extern void *malloc(), *realloc();

/*
 * NAME:	main
 *
 * FUNCTION:	main - entry point of fsck
 *
 * NOTES:	Main initializes globals, calls parse_args to
 *		handle the command line, calls autoargs or cmdargs
 *		to determine file systems to be checked, calls
 *		checkall to check them, possibly reboots, then
 *		returns a status.
 *
 * RETURN VALUE DESCRIPTION:	returns status to calling program
 */

/*
 * don't want no stinkin dumpargs!
 */
#ifdef DEBUG
# undef DEBUG
#endif

void checkfds();

int
main(ac,av)
 int    ac;
 char   **av;
{
	struct stat statarea;

	checkfds();

	(void) setlocale (LC_ALL,"");

	catd = catopen(MF_FSCK, NL_CAT_LOCALE);

	/*
	 * init fshelper flags
	 */
	fsh_mode_flags = FSHMOD_INTERACT | FSHMOD_FORCE | FSHMOD_PERROR;
	fsh_debug_level = FSHBUG_NONE;
	fsh_opflags[0] = '\0';

#ifdef STANDALONE
	if (av[0][0] == '\0')
		ac = getargv ("fsck", &av, 0);
	fsh_mode_flags |= FSHMOD_STANDALONE;
#else
	sync();
#endif

	setbuf(stdout, myalloc(BUFSIZ));

	/*
	 * check command line...
	 */
	parse_args(ac, av);

	/*
	 * reset ac and av according to what getopt() found...
	 */
	ac -= optind;
	av += optind;

#ifndef STANDALONE
	/*
	 * If we are running under a normal system, we have to know where
	 *    the root is so we can treat it specially.
	 */
	if( stat("/", &statarea) )
		pr(FATAL, MSGSTR(CANTSTAT, "Cannot stat root: %s"), sys_errlist[errno]);
	rootdev = statarea.st_dev;
#endif

	xcode = EXIT_OK;

	if( ac > 0 )
		cmdargs(ac, av);	/* process command line arguments */
	else
		autoargs();	/* do checks according to /etc/filesystems */

#if DEBUG
	dumpargs();
#endif
	checkall();

#if REBOOT
	if (xcode&EXIT_BOOT)
	{       if (pr(PREEN, Reboot, MSGSTR(CHANGM, "Changes to mounted file systems") ))
			reboot( (char *) 0 );
	}
#endif

	exit(xcode);
	/* NOTREACHED */
}


/*
 * NAME:	parse_args
 *
 * FUNCTION:	parse_args - process command line
 *
 * NOTES:	Parse_args processes the command line, checking validity
 *		of the flags and building the op flags for the fs helper.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
parse_args(ac, av)
register int	  ac;
register char	**av;
{
	register int c;
	register char *p;

	/*
	 * check flags
	 */
	while ((c = getopt(ac, av, "pnyft:i:d:c:V:o:vD")) != EOF) {
		switch( c ) {

		case 'p':	/* preen: quietly and automatically */
			preen = YES;
			strcat(fsh_opflags, "preen");
			break;

		case 'n':       /* default no answer flag */
			nflag = TRUE;
			strcat(fsh_opflags, "no");
			break;

		case 'y':       /* default yes answer flag */
			yflag = YES;
			strcat(fsh_opflags, "yes");
			break;

		case 'f':       /* fast and gullible check */
			strcat(fsh_opflags, "fast");
			break;

		case 't':	/* specify name of a scratch file   */
			strcat(fsh_opflags, "scratch=");
			strcat(fsh_opflags, optarg);
			break;

		case 'i':	/* specify I-node number to search for */
			strcat(fsh_opflags, "inum=");
			strcat(fsh_opflags, optarg);
			break;

		case 'd':	/* specify disk block to search for    */
			strcat(fsh_opflags, "block=");
			strcat(fsh_opflags, optarg);
			break;

		case 'c':	/* SPECIAL WIERD FLAG: specify cache size */
			strcat(fsh_opflags, "cache=");
			strcat(fsh_opflags, optarg);
			break;

		case 'v':       /* verbose operation	*/
			vflag = YES;
			break;

		case 'D':	/* Debug operation	*/
			fsh_debug_level = FSHBUG_SUBDATA;
			continue;		/* don't append comma */

		case 'V':       /* Specify vfs			*/
			Vfsname = optarg;
			continue;		/* don't append comma */

		case 'o':	/* fs implementation-specific options	*/
			strcat(fsh_opflags, optarg);
			break;

		default:
			pr(FATAL, MSGSTR(USAGE, Usage));
			break;

		}
		strcat(fsh_opflags, ",");
	}

	/*
	 * remove trailing comma ...
	 */
	if ((p = strrchr(fsh_opflags, ',')) != NULL)
		*p = '\0';
}

/*
 * NAME:	cmdargs
 *
 * FUNCTION:	cmdargs - read command line for file systems to check
 *
 * NOTES:	Cmdargs generates the list of file systems to check from
 *		the command line arguments.  The remaining arguments are
 *		the names of file systems.  An argument of "-" indicates
 *		the start of a new group of file systems.
 *
 *		This ability to enter groups from the command line will
 *		probably never be used, but it sure is dandy for debugging
 *		purposes.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
cmdargs(ac, av)
register int ac;
char **av;
{
	register FSNAME *fp, *prevfp = 0;
	register FSGROUP *gp, *prevgp = 0;
	register char *p;

	/*
	 * walk thru arguments
	 */
	while( ac-- > 0 )
	{       p = *av++;

		/* see if we are supposed to start a new group  */
		if (CSequal( p, "-" ))
		{       prevfp = 0;
			continue;
		}

		/* ignore non-reasonable file system names      */
		if ( !(fp = getname( p ) ) )
			continue;

		/* If we have a group, chain this filesystem in */
		if (prevfp)
		{       prevfp->next = fp;
			prevfp = fp;
		} else
		/* if we don't, create a new group for this fs  */
		{       gp = (FSGROUP *) myalloc( sizeof( FSGROUP ) );
			if (prevgp)
				prevgp->next = gp;
			else
				groups = gp;

			gp->num = ++ngroups;
			gp->list = fp;
			gp->next = 0;
			prevfp = fp;
			prevgp = gp;
		}
	}
}

/*
 * NAME:	autoargs
 *
 * FUNCTION:	autoargs - build list of file systems to check from
 *			   filesystems file
 *
 * NOTES:	Autoargs builds a list of file systems to be checked
 *		from the specifications in /etc/filesystems.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
autoargs()
{
	register char *atr;
	register int n;
	register FSNAME *fp, *pfp;
	register ATTR_t rec;
	register AFILE_t fsfile;
	register FSGROUP *gp, *pgp;

	/*
	 * open the filesystem information file so we can look things up
	 */
	if( !(fsfile = AFopen(fsprofile, MAXREC, MAXATR)) )
		pr(FATAL, "%s: %s", fsprofile, sys_errlist[errno]);

	/*
	 * search fsprofile for all file systems with "check=something"
	 */
	while( (rec = AFnxtrec( fsfile )) )
	{       if( !(atr = AFgetatr( rec, "check" )) )
			continue;	/* ignore fs without check attribute */

		/* figure out what group number to put it into  */
		n = atoi(atr);
		if( n <= 0 )
		{       /* see if true or false was specified   */
			if( CSequal(atr, "false") )
				continue;
			n = 1;
			if( !CSequal(atr, "true") )
				pr(0, MSGSTR(CHECKE, "%s: %s: check=%s? (using check=1)"),
					fsprofile, rec->AT_value, atr);
		}

		/* make sure we know what device it resides on  */
		if( !(atr = AFgetatr( rec, "dev" )) )
		{       pr(0, MSGSTR(NODEVIG, "%s: %s: no dev; ignored"),
				fsprofile, rec->AT_value, atr);
			continue;
		}

		if( !(fp = getname(atr)) )
			continue;

		if (Vfsname != NULL)
			fp->vfsname = Vfsname;
		else if ((fp->vfsname = AFgetatr(rec, "vfs")) != NULL)
			fp->vfsname = strsave(fp->vfsname);

		/* try to find the right group structue                 */
		for( gp = groups; gp && gp->num < n; gp = gp->next );

		if (gp == 0 || gp->num != n)
		{       /* we have to allocate a new group structure    */
			gp = (FSGROUP *)myalloc( sizeof( FSGROUP ) );
			gp->num = n;
			gp->list = 0;
			gp->next = 0;
			ngroups++;

			/* add it to the groups list                    */
			if (pgp = groups)
			{       if (pgp->num > n)
				{       gp->next = pgp;
					groups = gp;
				} else
				{       while(pgp->next && (pgp->next)->num<n)
						pgp = pgp->next;
					gp->next = pgp->next;
					pgp->next = gp;
				}
			} else
				groups = gp;
		}

		/* append this FSNAME to the end of the FSGROUP's list  */
		if (pfp = gp->list)
		{       while( pfp->next )
				pfp = pfp->next;
			pfp->next = fp;
		} else
			gp->list = fp;
	}

	AFclose( fsfile );
}

/*
 * NAME:	checkall
 *
 * FUNCTION:	checkall - check all of the specified file systems
 *
 * NOTES:	We have built up a list of file systems to be checked.
 *		The root should always be checked first and independently.
 *		Then we can check all of the other file systems.  If we have
 *		multiple groups and we are running automatically (preening),
 *		we will check each group in parallel.  If we are not preening
 *		we will not do any parallel checking.
 *
 *		The major reason for this restriction is the complexity of
 *		operator interaction.  If we are preening, fsck has been
 *		pre-authorized to make reasonable decisions.  Otherwise, it
 *		might have to ask questions of the operator - and managing
 *		communication between the operator twelve concurrent programs
 *		is a bit hairy.  Typically, when a huge number of file
 *		systems are checked it will be to preen them.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
checkall()
{
	int readonly;
	register FSNAME *np;
	register FSGROUP *gp;
	register int pid;
	register int nproc;

#ifndef	STANDALONE
	/*
	 * the front end ignores the signals - the backend can
	 * do what he wants with them...
	 */
#endif /* STANDALONE */

	/*
	 * always check the root first
	 */
	if( *root )
	{
		groupnum = -1;
		readonly = 0;
		(void) ismounted(root, &readonly);
		check(root, YES, readonly, (char *) NULL);
		*root = '\0';

		/*
		 * If root has been modified, continuing might undo the
		 *    repairs by causing the bad superblock to be rewritten.
		 */
		if (xcode & EXIT_BOOT)
			return;

		if (xcode) {
			/*
			 * if we had a failure on the root, fake the nflag so
			 * that pr will print "TERMINATED"
			 */
			if (xcode & EXIT_FAIL)
				nflag = TRUE;
			pr( CONTIN, MSGSTR(BADROOT, Badroot) );
			}
	}

	/*
	 * if there is nothing else to check, we are done
	 */
	if (ngroups == 0)
		return;

#ifndef STANDALONE
	/*
	 * if we are running automatically on a real system and we
	 *    have multiple groups to check, we can do them in parallel.
	 */
	if (preen  &&  ngroups > 1)
	{       /* spawn a sub-process for each file system group       */
		for( nproc = 0, gp= groups; gp; gp = gp->next )
		{       if ((pid = dofork()) == -1)
			{       pr( ERROR, MSGSTR(CANTFORK, "Cannot fork") );
				break;
			}

			if (pid == 0)
			{
				groupnum = gp->num;
				for( np = gp->list; np; np = np->next )
					check( np->name , np->mounted,
					       np->readonly, np->vfsname );
				exit( xcode );
			} else
				nproc++;
		}

		/* wait for all of the sub-processes to finish          */
		while( nproc )
		{       int status;

			if (wait( &status ) == -1 )
			{       if (errno == ECHILD)
					break;
			} else
			{       nproc--;
				if (status&0xff)
					xcode |= EXIT_FAIL;
				else
					xcode |= (status >> 8) & 0xff;
			}
		}
	} else
#endif
	{
		groupnum = -1;
		for( gp = groups; gp; gp = gp->next )
			for( np = gp->list; np; np = np->next )
				check( np->name, np->mounted, np->readonly,
				       np->vfsname );
	}
}

#ifndef STANDALONE
/*
 * NAME:	dofork
 *
 * FUNCTION:	dofork - try to fork a new process
 *
 * NOTES:	Dofork tries 5 times to fork a new process.
 *
 * RETURN VALUE DESCRIPTION:
 */

int
dofork()
{
	register int pid, n;

	for( n = 5; (pid = fork()) == -1; )
		if( --n >= 0 )
			sleep(2);
		else
			break;
	return pid;
}
#endif

/*
 * NAME:	myalloc
 *
 * FUNCTION:	myalloc - allocate memory
 *
 * NOTES:	Myalloc tries to allocate at least 'n' bytes of
 *		memory.  The memory is rounded up to the nearest
 *		4 byte boundary.
 *
 * RETURN VALUE DESCRIPTION:	the allocated memory
 */

char *
myalloc(n)
 int n;
{
	register char *p;

	if (n&1)    /* Round to nearest 2-byte boundary */
		n++;

	if (n&2)    /* Round to nearest 4-byte boundary */
		n += 2;

	if( (p = malloc((unsigned) n)) == NULL )
		pr( FATAL, MSGSTR(OUTSPAC, "Out of space") );

	return p;
}

/*
 * NAME:	getname
 *
 * FUNCTION:	getname - find out the name of the file system a person
 *			  wants to check
 *
 * NOTES:	getname
 *		the specified name might be:
 *			name of a block device
 *			name of a character device
 *			name of a known file system
 *
 *		depending on whether or not the file system is currently
 *		mounted, we might prefer to check the associated block or
 *		character device.
 *
 *		a special case is made of the root, which is always checked
 *		before anything else.
 *
 * RETURN VALUE DESCRIPTION:	NULL if this is the root device of
 *		an unknown file system, else a structure to the new
 *		FSNAME node
 */

FSNAME *
getname(fsname)
register char *fsname;
{
	register char *d = fsname;
	register FSNAME *fp;
	int      fsmounted;
	int	 readonly;
	dev_t    devid;
	struct stat statarea;

#define NOTBLOCK (stat(d, &statarea) || (statarea.st_mode&S_IFMT) != S_IFBLK)
#define NOTCHAR  (stat(d, &statarea) || (statarea.st_mode&S_IFMT) != S_IFCHR)

#ifndef STANDALONE
	/* normally, we want to find the block device   */
	if(     NOTBLOCK
	     && (!(d = unraw(d)) || NOTBLOCK)
	     && (!(d = FSdskname(fsname)) || NOTBLOCK) )
#else
	/* In saio, all devices are character devices   */
	if(     NOTCHAR
	     && (!(d = FSdskname(fsname)) || NOTCHAR) )
#endif
	{
		pr( ERROR, MSGSTR(UNKFILSYS, "%s is not a known file system"), fsname);
		return NULL;
	}
#undef NOTBLOCK
#undef NOTCHAR

#ifndef STANDALONE
	devid = statarea.st_rdev;

	/*
	 * if this is the root, save it so we can check it in a special pass
	 */
	if( devid == rootdev ) {
		CScpy(root, d);
		return NULL;
	}

	/*
	 * save off d in case we got it from FSdskname, which uses
	 * the AF routines to read /etc/filesystems.  these routines
	 * return memory which has been free()'d...  So we have to
	 * save it off before the next malloc...
	 */
	d = strsave(d);

	/*
	 * if the file system is mounted, note that fact for check so it
	 *    can take special precautions.
	 */

	if ((fsmounted = ismounted(d, &readonly)) == NO)
		/*
		 * if the file system is not mounted, we can safely use the
		 * raw device for improved performance.
		 */
		d = rawname( d );
#endif

	fp = (FSNAME *) myalloc( sizeof( FSNAME ) + CSlen(d) + 1 );
	fp->name = (char *)fp + sizeof( FSNAME );
	fp->next = 0;
	fp->mounted = fsmounted;
	fp->readonly = readonly;
	fp->vfsname = NULL;
	CScpy( fp->name, d );

	return( fp );
}

/*
 * NAME:	ismounted
 *
 * FUNCTION:	ismounted - see if a device is mounted
 *
 * NOTES:	Ismounted looks at the currently mounted filesystems
 *		and sees if the device in question is mounted.
 *
 * RETURN VALUE:	YES if the device is mounted, else NO
 */

int
ismounted(dev, readonly)
char *dev;
int *readonly;
{
	int count, i, ct;
	struct vmount *vmt;
	static int mntsiz = 0;
	static char *mnttab = NULL;

	/*
	 * first time in, point to count
	 */
	if (mnttab == NULL) {
		mnttab = (char *) &count;
		mntsiz = sizeof(count);
		}

	/*
	 * loop till we have enuf mem to read it in ...
	 */
	while ((ct = mntctl(MCTL_QUERY, mntsiz, mnttab)) <= 0) {
		/*
		 * error?
		 */
		if (ct < 0) {
			pr(FATAL, MSGSTR(MNTCTLF, "mntctl failed: %s"), sys_errlist[errno]);
			/* pr should exit, so we should not reach here... */
			return (NO);
			}

		/*
		 * get the current size and either malloc or realloc
		 */
		mntsiz = *((int *) mnttab);
		mnttab = (mnttab == (char *) &count) ? malloc(mntsiz) :
			realloc(mnttab, mntsiz);
		if (mnttab == NULL)
			pr(FATAL, MSGSTR(OUTSPAC1, "out of space"));
		}

	for (vmt = (struct vmount *) mnttab;
	     ct > 0 && strcmp(vmt2dataptr(vmt, VMT_OBJECT), dev) != 0;
	     ct--, vmt = (struct vmount *) ((char *) vmt + vmt->vmt_length))
		;

	/*
	 * find it?
	 */
	if (ct > 0) {
		*readonly = vmt->vmt_flags & MNT_READONLY;
		return (YES);
		}

	*readonly = 0;

	return (NO);
}

/*
 * NAME:	unraw
 *
 * FUNCTION:	unraw - turn a raw device name into the corresponding
 *			block device name
 *
 * NOTES:	Unraw turns a raw device name to it's block device name
 *		(by removing the 'r').  The block device name is returned.
 *
 * RETURN VALUE DESCRIPTION:	0 if it was not a raw device, else the
 *		block device name
 */

char *
unraw( n )
register char *n;
{       static char nbuf[PATH_MAX];
	register char *s;

	CScpy( nbuf, n );

	s = CSsname( nbuf );
	if (*s != 'r')
		return( 0 );

	CScpy( s, s+1 );
	return( nbuf );
}

#ifndef STANDALONE
/*
 * NAME:	rawname
 *
 * FUNCTION:	rawname - turn a block device name into the corresonding
 *			  raw device name
 *
 * NOTES:	Rawname turns a block device name to it's raw device
 *		name by adding the 'r'.
 *
 *		This is not meaningful if we are running standalone (where
 *		there is no distinction bewteen block and character devices).
 *
 * RETURN VALUE DESCRIPTION:	the raw device name
 */

char *
rawname(n)
register char *n;
{
	static char rawbuf[PATH_MAX];
	struct stat statarea;

	CScpy( rawbuf, n );
	CScat( CSsname(rawbuf), "r" , CSsname(n), (char *)NULL );

	if( stat(rawbuf, &statarea) || (statarea.st_mode&S_IFMT) != S_IFCHR )
		return( n );

	return( rawbuf );
}
#endif

#if DEBUG
/*
 * DUMPARGS
 *
 *      debugging routine, lists out the group and file system parm struct
 */
dumpargs()
{       register FSNAME *fp;
	register FSGROUP *gp;

	if (*root)
		printf("ROOT: %s (mounted)\n", root );

	for( gp = groups; gp; gp = gp->next )
	{       printf("GROUP %d\n", gp->num );
		for( fp = gp->list; fp; fp = fp->next )
			printf("    DEVICE: %s (%smounted)\n",
				fp->name, fp->mounted ? "" : "not " );
	}

	fflush( stdout );
}
#endif

/*
 * NAME:	check
 *
 * FUNCTION:	check - check a filesystem
 *
 * NOTES:	Check calls the filesystem helper to check a filesystem.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
check( name, mounted, readonly, vfsname )
char	*name;
int	 mounted;
int	 readonly;
char	*vfsname;
{
	int rc;			/* return code from fshelp()		*/
	char opdata[1024];  	/* where the data comes back thru	*/
	char opflags[2048]; 	/* file system helper fs flags		*/

	devname = name;		/* for pr()	*/

	/*
	 * format device name flag and group number flag (if needed)
	 */
	if (groupnum < 0)
		sprintf(opflags, "device=%s", name);
	else
		sprintf(opflags, "device=%s,group=%d", name, groupnum);

	/*
	 * append command line flags onto our local copy
	 */
	if (fsh_opflags[0] != '\0') {
		strcat(opflags, ",");
		strcat(opflags, fsh_opflags);
		}

	/*
	 * tell the backend if this fs is mounted
	 */
	if (mounted == YES) {
		strcat(opflags, ",");
		strcat(opflags, "mounted");
		if (readonly) {
			strcat(opflags, ",");
			strcat(opflags, "readonly");
			}
		}

	/*
	 * add verbose
	 */
	if (vflag) {
		strcat(opflags, ",");
		strcat(opflags, "verbose");
		}

	/*
	 * get the vfs name if we already don't have it
	 */

	if (vfsname == NULL)
		vfsname = (Vfsname == NULL) ? getvfsname(name) : Vfsname;

	/*
	 * init opdata
	 */
	opdata[0] = '\0';

	/*
	 * call the file system helper
	 */
	rc = fshelp(name, vfsname, FSHOP_CHECK, fsh_mode_flags,
			fsh_debug_level, opflags, opdata);	

	/*
	 * get xcode.  make it EXIT_FAIL if we get a file system
	 * helper error
	 */
	xcode |= (rc == FSHRC_GOOD) ? atoi(opdata) : EXIT_FAIL;

	devname = NULL;		/* no longer processing this one */
}

/*
 * NAME:	getvfsname
 *
 * FUNCTION:	getvfsname - the the file system type for a device
 *
 * NOTES:	getvfsname looks in /etc/filesystems and /etc/vfs to
 *		find the file system type for a specific device.
 *
 * RETURN VALUE DESCRIPTION:	the file system type
 */

char *
getvfsname(dev)
char *dev;
{
	char *name;
	ATTR_t rec;
	char *unraw();
	AFILE_t fsfile;
	char *atr, *block_dev;

	/*
	 * be sure we have the block device since that's what's always
	 * in /etc/filesystems...
	 */
	if ((block_dev = unraw(dev)) != NULL)
		dev = block_dev;

	/*
	 * try to open the filesystems file
	 */
	if( !(fsfile = AFopen(fsprofile, MAXREC, MAXATR)) )
		pr(FATAL, "%s: %s", fsprofile, sys_errlist[errno]);

	/*
	 * search for this device
	 */
	while( (rec = AFnxtrec( fsfile ))  &&
	      (!(atr = AFgetatr( rec, "dev" )) || strcmp(atr, dev) != 0 ))
		;

	/*
	 * if we found the entry in the filesystems file, and it
	 * has a "vfs" attribute, use it
	 */
	if (rec && atr && (atr = AFgetatr(rec, "vfs")))
		name = strsave(atr);
	else
		name = NULL;

	AFclose( fsfile );

	return (name);
}

/*
 * NAME:	pr
 *
 * FUNCTION:	pr - print out a diagnostic message
 *
 * NOTES:
 *      This routine is used throughout fsck to handle the output of
 *      diagnostic messages and interaction with the operator.  It
 *      prints out messages (like printf), asks questions, handles
 *      defaults, acknowleges answers and knows how to handle errors of
 *      various severities.  In previous versions of fsck, all of this
 *      functionality was spread around dozens of places - and as a result
 *      operator message handling was non-uniform and hard to work on.
 *
 *      The first argument to pr is a word of flags which describe
 *      the significance of the message and how it should be handled.
 *
 *      The second argument may be (if specified in the flags) a buffer
 *      into which an answer is to be returned.
 *
 *      The subsequent arguments are a format string and data items which
 *      are bound for printf.
 *
 * RETURN VALUE DESCRIPTION:	YES if the user answered yes to a question,
 *		else no
 */

#define LINSIZE 80      /* maximum length for user input line           */

/*VARARGS*/
pr(va_alist) va_dcl
{
	va_list ap;
	char line[LINSIZE];
	register int flags;
	register char *p;
	register int retyes = NO;
	register char *fmt;

	va_start(ap);
	flags = va_arg(ap, int);

	/*
	 * Running automatically normally suppresses the printing of
	 * simple progress messages.  Trivia messages are always
	 * suppressed unless we were asked for them.
	 */
	if (!vflag && (flags&TRIVIA))
		return retyes;
	if( preen && !vflag && (flags&QUIET) )
		return retyes;

	/*
	 * The default question is whether or not to continue, but the
	 * caller may have specified something else.
	 */
	p = Contin;
	if( flags&ARGX )
		p = va_arg(ap, char *);

	/*
	 * each new line of output should be prefixed with the name of
	 * the file system with which the error is associated (unless
	 * there is no possible doubt about which file system was involved).
	 */

	if (!(flags&NOB)  &&  preen) {
		if( devname )
			printf("%s", devname);
#if 0
		if( fslabel )
			printf(" (%s)", fslabel);
#endif
		printf(": ");
	}

	/* call printf to print the actual message      */
	fmt = va_arg(ap, char *);
	vprintf(fmt, ap);
	va_end(ap);

	/* if we are supposed to ask a question ...     */
	if( flags&ASKX ) {
		/* if we're authorized to do this, default yes  */
		if( yflag || preen && (flags&PREENX) )
			retyes = YES;

		/* if we have an answer, print out our default  */
		if( (retyes || nflag) && flags&YNX )
			printf(" (%s%s%s)", retyes? "" : "NOT ", p,
				p[CSlen(p)-1] == 'E'? "D" : "ED");
		else if (!preen)
		{       /* ask the question     */
			if( flags&YNX )
				printf("; %s? ", p);
			fflush(stdout);

			/* get the answer       */
			if( getline(flags&YNX? line : p) == EOF )
				flags |= FATAL;
			else if ( flags&YNX )
				if( *line == 'q' || *line == 'Q' )
					flags |= FATAL;
				else {
					if( *line == 'y' || *line == 'Y' )
						retyes = YES;
				}

			/* user has supplied the NL at end of line      */
			flags |= NOE;
		}
	}

	/* see if a serious error condition has come about              */
	if( flags&ERROR && !retyes ) {
		xcode |= EXIT_FAIL;
		if( flags&FATALX ) {
			pr(flags&NOE? 0 : NOB, MSGSTR(TERMDONE, " (TERMINATED)"));
			exit(xcode);
		}
	}

	/* print a newline at the end of line (unless it's suppressed)  */
	if( !(flags&NOE) ) {
		putchar('\n');
		fflush(stdout);
	}

	return retyes;
}

/*
 * getline - get a line of input from the user into a provided buffer,
 *      eating all blanks and assuming buffer to be LINSIZE bytes long.
 *
 *   returns
 *      number of bytes read
 *      EOF
 */
getline(loc)
char *loc;
{
	register n;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[LINSIZE-1];
	while((n = getchar()) != '\n') {
		if(n == EOF)
			return(EOF);
		if(!isspace(n) && p < lastloc)
			*p++ = n;
	}
}

/*
 *	strsave
 *
 *	save a string somewhere
 */

char *
strsave(string)
register char *string;
{
	return ((char *)strcpy(myalloc(strlen(string) + 1), string));
}






/*
 *	checkfds
 *
 *	- check important filedescriptors for validity...
 *
 *	- why?  because if stdout is closed, the first call
 *	  to open returns it.  then, the next printf goes
 *	  to that file regardless of where it's at (eg a filesystem)
 */


static void
checkfds()
{
	int fd, newfd;
	struct stat stbuf;

	/*
	 * walk thru the "fab 3" filedescriptors (0,1,2) and be
	 * sure they point somewhere.
	 *
	 * be sure to do these in order so that open will return
	 * the lowest one...
	 *
	 * try to open /dev/tty first, then punt and open /dev/null
	 */
	for (fd = 0; fd <= 2; fd++)
		if (fstat(fd, &stbuf) < 0 && errno == EBADF &&
		   (newfd = open("/dev/tty", O_RDWR)) != fd)
			newfd = open("/dev/null", O_RDWR);
}
