static char sccsid[] = "@(#)04	1.63.1.17  src/bos/usr/sbin/mount/mount.c, cmdfs, bos41J, 9523A_all 6/2/95 09:48:44";
/*
 * COMPONENT_NAME: (CMDFS) commands that manipulate files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	This utility implements the mount system manager command
 *	which makes a new file system available.  This program must
 *	run setuid root to be able to do device mounts and assorted
 *	checks.  It is designed so that:
 *
 *		anyone can find out what is mounted
 *		anyone in system group (0) can do normal device mounts
 *		only super user can do an arbitrary device mount
 *		anyone who has write permission to the stub can
 *			mount a directory or file over the stub
 *
 *		additionally, anyone in group (0) can do arbitrary
 *			device mounts iff they have write permission on
 *			dev, and directory.
 *
 *	synopsis
 *		mount			(list mounted file systems)
 *
 *		mount fsname		(mount the named file system,
 *						or local or remote dir/file)
 *		mount all		(mount all normal file systems
 *						and all local and remote
 *						dirs/files)
 *		mount -a		(ditto)
 *		mount device file	(mount device over file)
 *		mount object stub	(mount object over stub)
 *		mount -n nodename mntfile dir
 *					(mount remote mntfile on node
 *					 nodename over local or remote
 *					 dir/file)
 *		mount -t tstring [-n nodename]
 *					(mount all entries in filesystem
 *					 database with attribute
 *					 type = tstring and not already
 *                                       mounted)
 *
 *					 If the optional -n flag is
 *					 specified, then all the mounts
 *					 with attribute type = tstring
 *					 will be remotely mounted from
 *					 the specified nodename.
 *		mount -i [-n nodename] fsname
 *					(mount named file system and
 *					 inherit all mounts underneath
 *					 mount point)
 *		mount -i [-n nodename] object stub
 *					(perform arbitrary mount and
 *					 inherit all mounts underneath
 *					 mount point)
 *
 *		If the optional "-r" flag is specified, the file
 *		system will be mounted readonly.  This flag may not
 *		be used on directory or file mounts.
 *
 *		If the optional "-s" flag is specified, the list of
 *              filesystems is produced in "short" form, which bears
 *	        an altogether uncanny resemblance to that of Sun's.
 *              It is only usable for reporting on the current node's
 *              mount table.
 *
 *		If the optional "-p" ("partial") flag is specified, the
 *		file system will be mounted removably.
 *
 *		If the optional "-n" ("node") flag is specified, a
 *		nodename will follow the flag.  If mntfile is not
 *		specified, the user is requesting the status of a
 *		remote node.  Otherwise the user specifies mntfile and
 *		dir.  Mntfile is the remote directory or file
 *		that will mounted over dir, a local or remote directory or
 *		file.
 *
 *
 *	SECURITY CONSIDERATION
 *		From early on, a setuid(0) sets ruid = euid = 0.
 *		This enables the program to determine if a fs is mounted ro.
 *		Be sure, then, that any normal accessibility checks are based
 *		on variable "ruid", the user's real user id.
 ******/

#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <ustat.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <grp.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>
#include <sys/syspest.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <jfs/filsys.h>
#include <jfs/log.h>
#include <fshelp.h>
#include <ctype.h>
#include <sys/types.h>
#include <arpa/nameser.h>		/* MAXDNAME */
#include <locale.h>
#include <libfs/libfs.h>
#include <nl_types.h>
#include "mount_msg.h"


static nl_catd catd;

#define BOTH_MSG(num,str)    catgets (catd, MS_BOTH, num, str)
#define MOUNT_MSG(num,str)   catgets (catd, MS_MOUNT, num, str)
#define UMOUNT_MSG(num,str)  catgets (catd, MS_UMOUNT, num, str)


#define NILSTR ""
#define NIL(type)	((type) 0)
#define NILPTR(type)	((type *) 0)
#define MNTQP	struct mntquery *

/* EXTERNS herein */
static int	helper_Mfunc();
static int	helper_UMfunc();
static int	builtin_Mfunc();
static struct mntquery *parse_mq();
static void	eprintf();
static char   *get_log();

/* Locals */
static int mnt_query();
static int mountmain();
static int mount_X();
static int local_query();
static int remote_query();
static int domount();
static int domount2();
static int vmountdata();
static int umountmain();
static int unmount();
static int dounmount();
static int get_vmount();
static int sysgrp();
static char *getmount_time();
static MNTQP vmt_to_mq();
static int makeFStab ();
static int found_any ();

static void	chk_super (char *dev);

/*
** old_fs's are used to table the stanza's in FSfile (or later ODM)
** The list head is OldFsList.
*/

struct old_fs
{
  struct old_fs  *next;

  char            dev[PATH_MAX];
  char            log[PATH_MAX];
  char            mount[BUFSIZ];
  char            name[PATH_MAX];
  char		  nodename[MAXDNAME];
  char            options[BUFSIZ];
  char            type[32];
  char            vfs[16];

};

struct old_fs    *OldFsList = NILPTR (struct old_fs); /* head of old_fs list */

struct cfg_load cfg_load;
struct cfg_kmod cfg_kmod;

#define LOGREDO			"/usr/sbin/logredo"

#define CDROM_KMOD_PATH		"/usr/lib/drivers/cfs.ext"

#ifdef DEBUG
BUGVDEF(mdebug, 0);	/* for debugging purposes only */
#endif

/* the standard format print string */
#define PRINT_FMT	"%-8.32s %-16s %-16s %-6s %-12s %-16s\n"
/* the short format print string */
#define SHORT_FMT	"%s%s%s on %s type %s %s\n"
/* the width of the options column in the standard format print string */
#define OPTWIDTH	16

/* the standard format sscanf string */
#define SCAN_FMT       "%9s %19s %19s %3s %12c %12s\n"

#define ROUNDUP(x)      (((x) + 3) & ~3)

/* These are the list of Key letters that the mount helpers use */
#define MNT_QUERY	"Q"
#define MNT_MOUNT	"M"
#define MNT_UMOUNT	"U"

/*
 * this structure defines each vfs mount knows about, either as
 * as builtin vfs or one known from /etc/vfs
 */
struct vfs_info {
	struct vfs_info	*vi_next;	/* next in list */
	int	vi_flags;		/* see below */
	int	vi_infd;		/* send to helper on this fd */
	int	vi_outfd;		/* read from helper on this fd */
	/* following read from VFSfile for non-builtin vfs's */
	char	*vi_name;		/* name of vfs as text */
	int	 vi_type;		/* MNT_XXX, see vmount.h */
	char	*vi_helper;		/* mnt helper program, if it exists */
	int    (*vi_builtin)();		/* ptr builtin Mfunc, if it exists */
};

#define VFSI_NEEDXLATE	0x0001	/* requires xlation helper use */
#define VFSI_BUILTIN	0x0002	/* is a builtin */
#define VFSI_LOCAL 	0x0004	/* this is the default local vfs */
#define VFSI_REMOTE	0x0008	/* this is the default remote vfs */
#define VFSI_RUNNING	0x1000	/* helper has been started */


/*
 * declare built-in vfs's
 */
struct vfs_info cdrom_vfsinfo = {
	NILPTR (struct vfs_info), VFSI_BUILTIN, 0, 0, "cdrom",
	MNT_CDROM, NILPTR (char), builtin_Mfunc
};

struct vfs_info aix3_vfsinfo = {
	&cdrom_vfsinfo, VFSI_BUILTIN | VFSI_LOCAL, 0, 0, "jfs",
	MNT_JFS, NILPTR (char), builtin_Mfunc
};

struct vfs_info aix_vfsinfo = {
	&aix3_vfsinfo, VFSI_BUILTIN, 0, 0, "oaix",
	MNT_AIX, NILPTR (char), builtin_Mfunc
};

/*
** head of list of known DEFAULT vfs's, initialized to first on that list
*/

struct vfs_info *default_vfs_info = &aix_vfsinfo;

/*
 * head of list of known vfs's
 */
struct vfs_info *vfs_info;

/*
 * Buffer to store the name of the program.
 */
char progname[BUFSIZ];

/*
 * define a structure that is the internal representation of the parsed
 * info from mount queries.  Since this comes from the query helpers as
 * text strings, it is much easier for mnt_query to parse them into arrays
 * of strings and put the pointers in an array of these structures
 */
struct mntquery {
	int	mq_mflag;	/* general mount flags RO, RMV */
	char	*mq_hostname;	/* ptr to hostname, NULL for local */
	char	*mq_object;	/* ptr to object name */
	char	*mq_stub;	/* ptr to stub name */
	char	*mq_type;	/* ptr to name of vfs */
	char	*mq_options;	/* ptr to comma separated options string */
};


/*
 * Useful.
 */
#define COMPARE(v, i, s)	(strcmp(vmt2dataptr((v), (i)), (s)) == 0)

/*
 * miscellaneous variables
 */
int	ruid;				/* real user id		 */
int	in_sysgrp;			/* TRUE if in system group	*/

int	doing_inherit;	/* doing inherited mounts, clear to stop */
int     fflag;		/* force flag, set to mount even if dirty */
int	iflag;		/* do inherited mounts, set by cmd line or stanza */
int	rflag;		/* readonly flag */
int	pflag;		/* removable mount flag */
int	vflag;		/* specify vfs type */
int	oflag;		/* specify filesystem-specific mount options */

/*
 * default vfs type to use, if one is not specified on command line or
 * in VFSfile.
 */
char default_local[16] = "jfs";
char default_remote[16] = "";

/* EXTERNS from elsewhere */
extern	int errno;
extern	void exit();
extern	char *malloc();
extern	int vmount();
extern	int uvmount();
extern char *basename();

static	char *getvfstypebynode();
static struct vfs_info *getvfsibytype();
static struct vfs_info *getvfsibyname();
static void	vfs_table_init();
static struct old_fs *get_old_fs();
static struct old_fs *get_old_fs_dev();
static char *get_vfstypefromhelper();
static	char *get_fullname();
static void mountusage(), umountusage();
static	void do_canon(), chkoptions();

/* flag defines for mount_X() */
#define MNTX_ALL	0	/* mount "all", arg ignored */
#define MNTX_TYPE	1	/* mount by type, arg is type string */
#define MNTX_FSNAME	2	/* mount named filesystem, arg is fsname */
                                /*                       or arg is fsdev */

main(argc, argv)
int	argc;
char	*argv[];
{
	char *cmd;
	extern char progname[];

        (void) setlocale (LC_ALL, NILSTR);

        catd = catopen (MF_MOUNT, NL_CAT_LOCALE);

	cmd = basename (argv[0]);
	strcpy(progname, cmd);

	/*
	** create table of known vfs's, both mount and umount need it
	*/
	vfs_table_init();

	/*
	** create list of interesting filesystems in FSfile
	*/
	if (makeFStab (FSfile))
	  perror (BOTH_MSG (Err_FSTABLING, "tabling filesystem data"));

	if (!strcmp(cmd, "mount"))
		mountmain(argc, argv);
	else if ((!strcmp(cmd, "umount")) || (!strcmp(cmd, "unmount")))
		umountmain(argc, argv);
	else {
		fprintf (stderr,
			 BOTH_MSG (Err_CALLEDWRONG, "?what am i?: This command must be named \"mount\" or \"umount,\" not \"%s.\"\n"),
			 cmd);
		exit(-1);
	}
	exit(0);
	/* NOTREACHED */
#ifdef lint
	return 0;
#endif
}

static int
mountmain(argc, argv)
int	argc;
char	*argv[];
{
	extern char	*optarg;
	extern int	optind;
	register int	i;
	register char	*name1 = NILPTR (char), *name2 = NILPTR (char);
	register char   *cp;
	char	options[BUFSIZ];
	register char	*type = NILPTR (char);
	register char	*vfsname = NILPTR (char);
	register char	*node = NILPTR (char);
	int		mflags = 0;		/* mount flags */
	int		errors = 0;		/* count of errors, ret stat */
	int		aflag = 0;		/* mount -a */
	struct vfs_info  *vip;

	*options = '\0';
	/*
	 * make a pass over the arguments and figure out what user wants
	 */
	while ((i = getopt(argc, argv, "afrpit:o:V:v:n:D:")) != EOF)
		switch (i) {
		case 'a':
		    aflag++;
		    break;
		case 'f':
		    fflag = TRUE;
		    break;
		case 'r':
			if (*options == '\0')
				strcpy(options, "ro");
			else
				strcat(options, ",ro");
			mflags |= MNT_READONLY;
			break;
		case 'p':
			if (*options == '\0')
				strcpy(options, "rmv");
			else
				strcat(options, ",rmv");
			mflags |= MNT_REMOVABLE;
			break;
		case 'i':
			iflag = TRUE;
			break;
		case 't':
			type = optarg;
			break;
		case 'o':
			if (*options == '\0')
				strcpy(options, optarg);
			else {
				strcat(options, ",");
				strcat(options, optarg);
			}
			oflag = TRUE;
			break;
		case 'v':
		case 'V':
			vfsname = optarg;
			vflag = TRUE;
			break;
		case 'n':
			node = optarg;
			break;
#ifdef DEBUG
		case 'D':
		        mdebug = atoi(optarg);
			break;
#endif /* DEBUG */
		case '?':
		default:
			mountusage();
		}

	if (aflag) {
		if (argc > optind)
			mountusage();
		name1 = "all";
	}
	else {
	if (argc > optind)
		name1 = argv[optind++];
	if (argc > optind)
		name2 = argv[optind++];
	if (argc > optind)
		mountusage();
	}

	/*
	** if BSD-style name (node:/device) is specified for remote device, use
	** it, provided an AIX-style name (-n node /device) was not supplied
	*/
	if (name1 != NILPTR (char) && node == NILPTR (char) &&
	    (cp = strchr (name1, ':')) != NILPTR (char))
	{
	  node  = name1;
	  *cp   = '\0';         /* ':' becomes terminator */
	  name1 = cp + 1;	/* the device name after the colon */
	}

	/*
	 * now check for invalid combinations of parameters and options
	 */

	/*
	 * if type string specified, then only nodename, protocol are allowed.
	 * (otherwise the options processing is complicated, "type" mounts
	 * are meant for simple mounts, using stuff in FSfile
	 */
	if ( type != NILPTR (char) ) {
	  if ( rflag || pflag || oflag || vflag || iflag || name1 || name2 ) {
	    eprintf( MOUNT_MSG (Err_INVALTOPT1, "\t%s%s%s%s%s%s%s.\n"),
		    rflag ? "\'-r\' " : NILSTR,
		    pflag ? "\'-p\' " : NILSTR,
		    oflag ? "\'-o\' " : NILSTR,
		    vflag ? "\'-v\' " : NILSTR,
		    iflag ? "\'-i\' " : NILSTR,
		    (name1 || name2) ? "\'Directory/File Name\' " : NILSTR,
	  MOUNT_MSG (Err_INVALTOPT2, "invalid with the \'-t\' option"));
	    exit(-1);
	  }
	}

	/*
	 * check for inherited and "all"
	 */
	if (iflag == TRUE) {
		if (!strcmp(name1,"all")) {
			eprintf(MOUNT_MSG (Err_INHERITMLT,
					 "Cannot inherit multiple mounts\n"));
			exit(-1);
		}
	}

	/* if remote mounts... disallow removable and "all" */
	if ( node != NILPTR (char) ) {
		if ( pflag ) {
			eprintf(MOUNT_MSG (Err_RMTANDRMV,
		 "Remote mounts on removable media are not allowed\n"));
			exit( -1 );
		}
		if (!strcmp(name1,"all")) {
			eprintf( MOUNT_MSG (Err_NODEANDALL,
	  "\"Nodename\" and \"all\" are mutually exclusive options\n"));
			exit(-1);
		}
	}

	/*
	 * set up for checking if a file system is mounted readonly
	 *	first capture the invoker's real user id.
	 *	set ruid=0 since access() checks perms for the real uid.
	 *	if root doesn't have write access to the root dir of the fs,
	 *		it's mounted read-only.
	 *	mark mtab for ro open if request is for printing of
	 *		mount table situated on read-only root.
	 */
	ruid = getuid();

	/*
	 * Before we go setuid, we might need to check permissions.
	 */
	if (name2 && ruid)
	{
		if (access(name2, W_OK) != 0)
		{
			if ( errno == ENOENT )
				perror (name2);
			else
			{
				eprintf( MOUNT_MSG (Err_STUBPERM,
			"Write permission is required to mount over %s.\n"),
					name2);
			}
			exit(-1);
		}
	}

	in_sysgrp = sysgrp();	/* is user in sys grp? */

	if ( ruid && !in_sysgrp && (strcmp(name1,"all") == 0) )
	{
	  eprintf(MOUNT_MSG (Err_NOTPRIV,
			   "Only root and system group members may perform \"all\" mounts\n"));
	  exit(-1);
	}

	if (ruid)
		setuid((uid_t)0);

	/*
	 * now do the mount action as requested
	 */
	if (type != NILPTR (char))
		/* if given type, then mount by type */
		errors = mount_X (MNTX_TYPE, type, vfsname,
						node, mflags, options);
	else if (name1 == NILPTR (char)) {
		/* no name means do query */
		if (mnt_query (TRUE, node, vfsname, NIL (struct mntquery ***)) < 0)
			errors = 1;
	} else if (name2) {
		/*
		 * if more than one name is specified, the user is trying
		 * to mount something in a non-default manner, so we call
		 * domount() directly.  Check to see if there is a node
		 * specified.  If not, then this is a local operation and
		 * we need to fill in the default local vfsname.
		 * XXX This is fine for file and directory mounts, but
		 * if it's a device mount we should try harder to find the
		 * right vfs type.
		 */
		if (node == NILPTR (char) && vfsname == NILPTR (char)) {
			for (vip = vfs_info;
			     vip && !(vip->vi_flags & VFSI_LOCAL);
			     vip = vip->vi_next)
				;
			if (vip)
				vfsname = vip->vi_name;
		}
		errors = domount(name1, name2, vfsname, node, mflags, options);
	} else if (strcmp(name1, "all") == 0)
		/* only one name and it is "all", so mount all */
		errors = mount_X( MNTX_ALL, "all", vfsname,
						node, mflags, options);
	else
	{
	  /* one name which ought to be a file system name or device */
	  errors = mount_X( MNTX_FSNAME, name1,vfsname, node, mflags, options);
	}

	exit(errors);
} /* mountmain */

static void
mountusage()
{
	fprintf(stderr, MOUNT_MSG (Err_MOUNTUSAGE,
				   "Usage: mount [-fipr] [-n node] [-o options] [-t type] [-{v|V} vfs] [-a | all | [[node:]device] [dir]]\n"));
	exit(-1);
}

/*
 * do a mount or mounts, based on information in FSfile
 * if cmd is:
 *	MNTX_ALL - mount all with "mount = true"
 *	MNTX_TYPE - mount all with "type = type_given_in_arg"
 *	MNTX_FSNAME - mount the thing described by the stanza "arg"
 *
 */
static int
mount_X( cmd, arg, vfsname, hostname, flags, options)
int	cmd;	/* MNTX_... */
char	*arg;
char	*vfsname;
char	*hostname;
int	flags;
char	*options;
{
  char	*dev_attr;		 /* ptr to "dev" attributes */
  char	*mount_attr;		 /* ptr to "mount" attributes */
  char	*vfs_attr;		 /* ptr to "vfs" attribute */
  char	*fsname = NILPTR (char); /* name of object being mounted */
  char	optbuf[BUFSIZ];
  struct old_fs *ofsp;
  int	tmpflags;
  register char *nodename;
  int	mntcount = 0;		/* for err checking for MNTX_TYPE */
  int	errors = 0;
  int	match_dev = 0;		/* for MNTX_FSNAME */


  tmpflags = 0;
  /*
   * scan the stanza's looking for a match
   * if cmd == MNTX_FSNAME scan twice and don't match devices the first time
   */
  for (ofsp = OldFsList;
       ofsp;
       (ofsp = ofsp->next) || (cmd != MNTX_FSNAME) || (match_dev)
       || (match_dev = 1, ofsp = OldFsList))
  {
    /* everybody eventually needs the mount attributes */
    mount_attr = strcmp (ofsp->mount, NILSTR)? ofsp->mount:NILPTR (char);

    switch ( cmd ) {
    case MNTX_ALL:
      /* try to find "mount = true" */
      if ( mount_attr == NILPTR (char) )
	  continue;
      if ( strcmp( mount_attr, "true" ) != 0)
	  continue;	/* no "mount = true" */
      fsname = ofsp->name;
      break;
    case MNTX_TYPE:
      /* try to find type keyword and matching attribute */
      if (strcmp (ofsp->type, NILSTR) == 0)
	  continue;
      if (strcmp(ofsp->type, arg) != 0)
	  continue;	/* type mis-compares */
      fsname = ofsp->name;
      mntcount++;	/* keep track of any found */
      break;
    case MNTX_FSNAME:
      /* look for a particular filesystem name or device */
      if (strcmp (arg, ofsp->name) != 0 && strcmp (arg, ofsp->dev) != 0)
	  continue;
      /*
       ** which matched?
       */
      if (strcmp (arg, ofsp->name) == 0)
	  fsname = arg;		/* object */
      /*
       ** local or remote match?
       */
      else if (strcmp (arg, ofsp->dev) == 0 && match_dev &&
	       ((hostname == NILPTR (char) || strcmp (hostname, NILSTR) == 0) ||
		(hostname != NILPTR (char) && strcmp (hostname, ofsp->nodename) == 0)))
	  fsname = ofsp->name;	/* device */
      else
	  continue;
      break;
    }

    /*
     * we found the stanza
     *	get the other information and do the mount
     */
    while ( mount_attr && *mount_attr ) {
      /* XXX this doesn't work -- it only gets the first option */
#ifdef DEBUG
      fprintf(stderr, "mount_attr: '%s'\n", mount_attr);
#endif /* DEBUG */
      if (strcmp(mount_attr, "readonly") == 0)
	  tmpflags = flags | MNT_READONLY;
      else
	  if (strcmp(mount_attr, "removable") == 0)
	      tmpflags = flags | MNT_REMOVABLE;
	  else
	      if (strcmp(mount_attr, "inherit") == 0)
		  iflag = TRUE;
      mount_attr += strlen( mount_attr ) + 1;
    }

    /* get options attribute */
    if (strcmp (ofsp->options, NILSTR) != 0)
	if (*options != '\0')
	    sprintf(optbuf,"%s,%s", ofsp->options, options);
	else
	    strcpy(optbuf, ofsp->options);
    else
    {
      if (*options != '\0')
	  strcpy(optbuf, options);
      else
	  *optbuf = '\0';
    }

    if (*optbuf == '\0')
    	strcpy(optbuf, "rw");
    else {
	    if (!scan_ro(optbuf) && !scan_rw(optbuf))
	    	strcat(optbuf, ",rw");
    }

    /* nodename is a confusing story */
    switch ( cmd ) {
    case MNTX_ALL:
      /* "all" only allows nodename from filesystems */
      nodename = strcmp (ofsp->nodename, NILSTR)? ofsp->nodename: NILPTR (char);
      break;
    case MNTX_TYPE:
      /* type will accept override */
      if ( (nodename = hostname) == NILPTR (char) )
	  nodename = strcmp (ofsp->nodename, NILSTR)?
	      ofsp->nodename: NILPTR (char);
      break;
    case MNTX_FSNAME:
      /* by name takes stanza first, then override */
      nodename = strcmp (ofsp->nodename, NILSTR)?
	  ofsp->nodename: NILPTR (char);
      if ( nodename == NILPTR (char) )
	  nodename = hostname;
      break;
    }

    /* now we get the name of object to mount */
    dev_attr = strcmp (ofsp->dev, NILSTR)? ofsp->dev: NILPTR (char);
    if ( dev_attr == NILPTR (char) ) {
      eprintf( MOUNT_MSG (Err_UNKOBJ, "No %s specified for stanza %s\n"),
	      nodename == NILPTR (char)?
	      MOUNT_MSG (Err_UNKOBJLOCAL, "mount object"):
	      MOUNT_MSG (Err_UNKOBJREMOT, "remote path"),
	      fsname );
      errors++;
      goto endloop;
    }

    /* EVERYBODY needs vfs attribute */
    /*
     ** if specified explicitly, that is what they get
     ** otherwise let filesystem helper determine it for us
     ** if that fails too, get default for this filesystem
     ** or if that fails too, get default for this node
     */
    if (vfsname != NILPTR (char) && strcmp (vfsname, NILSTR) != 0)
	vfs_attr = vfsname;
    else
    {
      if ((vfs_attr = get_vfstypefromhelper (dev_attr)) == NILPTR (char))
      {
	vfs_attr = strcmp (ofsp->vfs, NILSTR)? ofsp->vfs: NILPTR (char);
	if ( vfs_attr == NILPTR (char) )
	    vfs_attr = nodename == NILPTR (char)? default_local: default_remote;
      }
    }

    /* check to see if doing remote & removable mount */
    if ( (nodename != NILPTR (char)) && (flags & MNT_REMOVABLE) ) {
      eprintf( MOUNT_MSG (Err_RMVNOTDEV,
			  "Non-device mounts may not be removable\n"));
      errors++;
      goto endloop;
    }

    errors += domount(dev_attr, fsname, vfs_attr, nodename, tmpflags, optbuf);

    tmpflags = flags;
endloop:
    if ( cmd == MNTX_FSNAME )
	goto done;
  }

  switch ( cmd ) {
  case MNTX_ALL:
    break;
  case MNTX_TYPE:
    if ( mntcount == 0 )
	eprintf(BOTH_MSG (Err_NOTYPES, "There are not 'type=%s' stanzas\n"), arg );
    break;
  case MNTX_FSNAME:
    eprintf(MOUNT_MSG (Err_UNKFS,  "%s is not a known file system\n"), arg );
    errors++;
    break;
  }

done:
  return(errors);
} /* mount_X */

/*
 * vfs_table_init - create the table of known vfs's
 *	it reads VFSfile, then adds on the known built-in vfs's
 *      default flags are set appropriately
 */
static void
vfs_table_init()
{
	struct vfs_info  *vip;
	struct vfs_info  *vip2;
	struct vfs_info  *defvip;
	struct vfs_ent   *vfsp;
	struct vfs_info  *lastvfs = NILPTR (struct vfs_info);
	int		  found;
	int		  len;

	setvfsent();

	/* for each entry in /etc/vfs, read it and parse into table */
	while ((vfsp = getvfsent()) != NILPTR (struct vfs_ent))
	{
	  vip = (struct vfs_info *) malloc( sizeof( struct vfs_info ) );
	  if ( vip == NILPTR (struct vfs_info))
	  {
	    eprintf( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
	    exit(1);
	  }
	  /* initialize vip */
	  memset ((void *)vip, 0, sizeof(struct vfs_info));
	  vip->vi_next = NULL;
	  vip->vi_name = NULL;
	  vip->vi_helper = NULL;
	  vip->vi_builtin = NULL;

	  if (vfsp->vfsent_flags & VFS_DFLT_LOCAL)
	  {
	    strcpy( default_local, vfsp->vfsent_name );
	    vip->vi_flags |= VFSI_LOCAL;
	    /*
            ** clear builtin local flag
            */
	    for (defvip = default_vfs_info; defvip; defvip = defvip->vi_next)
		if (defvip->vi_flags & VFSI_LOCAL)
			defvip->vi_flags &= ~VFSI_LOCAL;

	    /*
	    ** last one on the list wins
	    */
	    for (vip2 = vfs_info; vip2; vip2 = vip2->vi_next)
		if (vip2->vi_flags & VFSI_LOCAL)
		    vip2->vi_flags &= ~VFSI_LOCAL;
	  }
	  else if (vfsp->vfsent_flags & VFS_DFLT_REMOTE)
	  {
	    strcpy( default_remote, vfsp->vfsent_name );
	    vip->vi_flags |= VFSI_REMOTE;
	    /*
            ** clear builtin remote flag
            */
	    for (defvip = default_vfs_info; defvip; defvip = defvip->vi_next)
		if (defvip->vi_flags & VFSI_REMOTE)
			defvip->vi_flags &= ~VFSI_REMOTE;
	    for (vip2 = vfs_info; vip2; vip2 = vip2->vi_next)
		if (vip2->vi_flags & VFSI_REMOTE)
		    vip2->vi_flags &= ~VFSI_REMOTE;
	  }
	  /*
	   * save vfs name
	   */

	  if ((len = strlen (vfsp->vfsent_name)) <= 0)
	  {
	    eprintf (MOUNT_MSG( Err_STRLENFAIL, "Internal error, strlen failed\n"));
	    exit (1);
	  }

	  if ( (vip->vi_name = malloc (len + 1)) == NILPTR (char) ) {
	    eprintf( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
	    exit(1);
	  }
	  if (strcpy( vip->vi_name, vfsp->vfsent_name ) != vip->vi_name)
	  {
	    eprintf (MOUNT_MSG( Err_STRCPYFAIL, "Internal error, strcpy failed\n"));
	    exit (1);
	  }

	  /*
	   * vfs type
	   */
	  vip->vi_type = vfsp->vfsent_type;

	  /*
	   * save mount helper if one exists
	   * otherwise mark builtin flag, if plausible
	   */
	  if (vfsp->vfsent_mnt_hlpr == NILPTR (char))
	  {
	    vip->vi_helper = NILPTR (char);
	    /*
	    ** mark flags
	    */
	    for (defvip = default_vfs_info; defvip; defvip = defvip->vi_next)
		if (defvip->vi_type == vip->vi_type)
		{
		  vip->vi_flags |= defvip->vi_flags;
		  /*
		  ** only really builtin if we know the function
		  ** (but don't complain because the user may not
		  ** be requesting a mount of this vfs type)
		  ** even if /etc/vfs or our builtin list is scragged
		  */
		    if (vip->vi_flags & VFSI_BUILTIN)
			if (defvip->vi_builtin != (int (*)()) 0)
			    vip->vi_builtin = defvip->vi_builtin;
			else
			  vip->vi_flags |= ~VFSI_BUILTIN; /* sorry */
		}
	  }
	  else {
	    vip->vi_helper = (char *)malloc(strlen(vfsp->vfsent_mnt_hlpr)+1);
	    if ( vip->vi_helper == NILPTR (char) ) {
	      eprintf( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
	      exit(1);
	    }
	    strcpy( vip->vi_helper, vfsp->vfsent_mnt_hlpr );
	  }

	  /* add to list of vfs's */
	  if ( lastvfs == NILPTR (struct vfs_info) )
	      vfs_info = vip;
	  else
	      lastvfs->vi_next = vip;

	  lastvfs = vip;
	}

	endvfsent();

	/*
	 * link the built-ins onto table last, so that /etc/vfs can
	 * override the defaults, provided they are not yet on our list
	 */
	for ( defvip = default_vfs_info; defvip; defvip = defvip->vi_next )
	{
		found = 0;
		for ( vip = vfs_info; vip && !found; vip = vip->vi_next )
			if (defvip->vi_type == vip->vi_type)
				found++;
		if (!found)
		{
			/* add to list of vfs's */
			if ( lastvfs == NILPTR (struct vfs_info) )
				vfs_info = defvip;
			else
				lastvfs->vi_next = defvip;
			lastvfs = defvip;
		}
	}
	return;
}

/*
 * getvfsibytype - get the vfs info by type
 *	it searches the global vfs info table and returns a pointer
 * to the entry for the gfs of a particular type (or NULL it it can't find
 * it.)
 */
static struct vfs_info *
getvfsibytype( gfstype )
int	gfstype;
{
	struct vfs_info *vip;

	for ( vip = vfs_info; vip; vip = vip->vi_next ) {
		if ( gfstype == vip->vi_type )
			return( vip );
	}
	return( NILPTR (struct vfs_info));
}

/*
 * getvfsibyname - get the vfs info by its name
 *	it searches the global vfs info table and returns a pointer
 * to the entry for the gfs of a particular name (or NULL it it can't find
 * it.)
 */
static struct vfs_info *
getvfsibyname( gfsname )
char	*gfsname;
{
	struct vfs_info *vip;

	for ( vip = vfs_info; vip ; vip = vip->vi_next ) {
		if ( strcmp( gfsname, vip->vi_name ) == 0 )
			return vip;
	}
	return( NILPTR (struct vfs_info) );
}

/*
 * mnt_query is used to invoke ALL query operations.
 *	If just printing, it prints the stuff on stdout,
 * possibly calling a helper to translate.
 *	If not printing, the info is gathered into
 * a list of mount entries herein, in the form of an array of pointers
 * to struct mntquery and the associated mntquery structures and their strings.
 *
 * INPUTS: see comments
 * OUTPUTS: number of mounted things, or 0 for not ok; if told to
 *	NOT print, it fills in ptr at mqbufp with ptr to array of
 *	pointers to struct mntquery structs.
 */
static int
mnt_query(print, nodename, vfstype, mqbufp)
int	print;			/* TRUE or FALSE */
char	*nodename;		/* ptr to nodename, or NULL for local */
char	*vfstype;		/* ptr to vfs type */
struct mntquery	***mqbufp;	/* place to put pointer to info gathered
				 * for internal use */
{
	/* print the header */
	if (print) {
	  printf (PRINT_FMT, "  node", "    mounted",
		"  mounted over", " vfs", "    date", "   options");
	  printf (PRINT_FMT, "--------", "---------------",
		    "---------------", "------", "------------",
		    "---------------");
	}
	if (nodename != NILPTR (char) && *nodename != '\0')
		return( remote_query( print, nodename, vfstype, mqbufp ) );
	else
		return( local_query( print, mqbufp ) );
}

/*
 * local_query is used to invoke a local query operation
 *	If just printing, it prints the stuff on stdout,
 * If not printing, the info is gathered into
 * a list of mount entries herein. (in standard format).
 * INPUTS: see comments
 * OUTPUTS: returns no of vfs's for OK, -error for not ok. If told to
 *	NOT print, it fills in data area at mq_bufp with std fmt...
 */
static int
local_query(print, mqbufp)
int	print;			/* TRUE or FALSE */
struct mntquery	***mqbufp;	/* place to put pointer to info gathered
				 * for internal use */
{
	register struct vmount *vm;
	struct vmount *bufp = NILPTR (struct vmount);	/* ptr to 1st vmount */
	int	nmount;			/* number of mounts (vmount structs) */
	struct vfs_info *vip;
	char	*mnttime = NILPTR (char), *getmount_time();
	struct mntquery	**mq, *vmt_to_mq();
	int	rv;
	time_t	tmptime;

	/* get kernel's idea of mnt info. If error, just return */
	if ((nmount = get_vmount( &bufp )) <= 0)
		return(nmount);

	rv = 0;

	if (!print) {
		/* if not printing, then setup first block of mntquery ptrs */
		mq = (struct mntquery **)malloc(sizeof(struct mntquery *) * nmount);
		*mqbufp = mq;
	}
	/*
	 * go thru all the structures in the vmount buffer
	 */
	for (vm = bufp; nmount; nmount--,
			vm = (struct vmount *)((char *)vm + vm->vmt_length)) {
		if ((vip = getvfsibytype(vm->vmt_gfstype)) ==
		    NILPTR (struct vfs_info)) {
		  eprintf( BOTH_MSG (Err_UNKGFS,
				     "%s has an unknown gfstype (%d)\n"),
			  vmt2dataptr(vm, VMT_STUB), vm->vmt_gfstype);
			continue;
		}

		tmptime = vm->vmt_time;
		if ((mnttime = getmount_time(tmptime)) == NILPTR (char))
			mnttime = "?";

		if (print)
		{
		  int   remote;
		  char  arg_fmt[32];
		  int   arg_size;
		  char  *args = NILPTR (char);

		  remote = vm->vmt_flags & MNT_REMOTE;
		  arg_size = vmt2datasize (vm, VMT_ARGS);
		  sprintf (arg_fmt, "%%.%ds", arg_size);
		  if (arg_size)
		    if ((args = malloc (arg_size+1)) == NILPTR (char))
		    {
		      eprintf( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
		      exit (-1);
		    }
		  if (args)
		  	sprintf (args, arg_fmt, vmt2dataptr (vm, VMT_ARGS));
		  printf (PRINT_FMT, remote?
				vmt2dataptr(vm, VMT_HOST): NILSTR,
				vmt2dataptr(vm, VMT_OBJECT),
				vmt2dataptr(vm, VMT_STUB),
				vip->vi_name, mnttime,
				args? args: NILSTR);
	        }
		else
			*mq++ = vmt_to_mq(vm, vip->vi_name);
		rv++;	/* another one printed */
	}
	if (bufp != NILPTR (struct vmount))
		free((void *)bufp);
	return(rv);
}

/*
 * remote_query is used to invoke a remote query operation
 *	It checks to see if the vfs type is specified, and if not,
 * it tries to find a stanza in /etc/filesystems where nodename matches
 * the given nodename and that has a "vfs =" attribute in it.
 * If none is found, it punts.
 *	Otherwise it invokes the query.  If just printing, it just
 * calls the helper.  If not printing, the info is gathered into
 * a list of mount entries herein. (in standard format).
 * INPUTS: see comments
 * OUTPUTS: number of mounted things, or 0 for not ok; if told to
 *	NOT print, it fills in data area at mqbufp with std fmt...
 */
static int
remote_query( print, nodename, vfstype, mqbufp )
int		print;		/* TRUE or FALSE */
char		*nodename;	/* ptr to host name for remote query */
char		*vfstype;	/* ptr to name of vfs to use for query */
struct mntquery	***mqbufp;	/* ptr to info gathered for internal use */
{
	char		buffer[BUFSIZ];
	struct vfs_info	*vip;
	struct mntquery	**mq;		/* temp pointer for gathering info */
	int		mqcnt;		/* count of info structs made */
	int		pid, pfd[2];
	int		status;

	/* If vfstype not specified, get one */
	if (vfstype == NILPTR (char))
		vfstype = getvfstypebynode (nodename);

	/* convert type into a pointer to info about it */
	if ((vip = getvfsibyname(vfstype)) == NILPTR (struct vfs_info)) {
	  eprintf (MOUNT_MSG (Err_UNKVFS, "Unknown vfs name \"%s\"\n"), vfstype);
	  return(0);
	}

	/*
	 * If not printing, then setup first block of mntquery ptrs
	 * Also setup a pipe for helpers's output
	 */
	if (!print) {
		mq = (struct mntquery **)malloc(sizeof(struct mntquery *) * 64);
		mqcnt = 0;
		*mqbufp = mq;
		if (pipe(pfd) < 0) {
			eprintf( MOUNT_MSG (Err_PIPEFAIL, "pipe to mount helper failed\n"));
			return(0);
		}
	}

	/* fork & exec the mount helper */
	pid = fork();
	if (pid == 0) {	/* child to call helper */
		if (!print) {
			close(pfd[0]);
			dup2(pfd[1], 1);
		}
		execlp(vip->vi_helper, vip->vi_helper, MNT_QUERY, "0",
						nodename, NILPTR (char));
		perror(vip->vi_helper == NILPTR (char)?
				"Mount helper": vip->vi_helper);
		exit(1);

	} else if (pid > 0) {	/* parent will wait for child */
	  /* while not EOF on pipe, read it, a line at a time */
	  if (! print) {
	    FILE	*pip;

	    close(pfd[1]);
	    if ((pip = fdopen(pfd[0], "r")) == NILPTR (FILE)) {
	      ;
	    }
	    while (fgets(buffer, BUFSIZ, pip) != NILPTR (char)) {
	      /*
	       * add buffer info into parse data
	       */
	      *mq = parse_mq(buffer);
	      mqcnt++;
	      if (mqcnt % 64 == 0) {
		*mqbufp = (struct mntquery **)
		    realloc(*mqbufp, sizeof(struct mntquery *) *(mqcnt + 64));
		mq = mqbufp[mqcnt];
	      }
	      mq++;
	    }
	    fclose(pip);
	  }
	  wait(&status);
	  if (status) {
	    eprintf(MOUNT_MSG (Err_RMTBADSTAT, "Bad status (%d) returned from mount helper\n"), status);
	    return(-1);
	  }
	} else {
	  /* bad news, can't fork! */
	  perror(BOTH_MSG (Err_FORKFAIL, "fork of mount helper failed"));
	  exit(1);
	}
	return(mqcnt);
}

/*
 * domount is the central function that is called to do mounts.
 *	It does the single original mount, and then does any inherited
 * processing.  It actually uses domount2() to perform the mount itself.
 */
static int
domount(object, stub, vfstype, node, mflags, options)
char	*object;
char	*stub;
char	*vfstype;	/* vfs name as a character string */
char	*node;
int	mflags;		/* bit mask of mount flags */
char	*options;
{
	int	nmount;
	struct mntquery **mqdata;
	register struct mntquery **mq;
	struct mntquery	invalid;
	char	istub[MAXPATH];
	char	*mqstub, *tmpnode;
	char	*tmpopt;
	int	tmpflag;
	int	mqstublen;
	int	root_case = FALSE;
	int	objlen = 0;
	int	rc = 0, saverr, i;
	struct	stat	statbuf;
	struct old_fs  *ofsp;
	extern 	int	errno;

	/*
	** If we don't have a vfstype, get one.
	**  order is: from filesystem database (FSfile/ODM/etc.)
	**             1. for the device; 2. for the mount point
	**            default for this node (from VFSfile/ODM/etc.)
	*/
	if (vfstype == NILPTR (char))
	{
	  if (((ofsp = get_old_fs_dev (object)) == NILPTR (struct old_fs) &&
	      (ofsp = get_old_fs (stub)) == NILPTR (struct old_fs)) ||
	      strcmp (ofsp->vfs, NILSTR) == 0 || node != NILPTR (char))
	  {
	    if ((vfstype = getvfstypebynode (node)) == NILPTR (char))
	    {
	      eprintf (MOUNT_MSG (Err_NODFLTVFS, "Default vfs for %s is indeterminate\n"),
		       node == NILPTR (char)? "this node": node);
	      return 1;
	    }
	  }
	  if (vfstype == NILPTR (char))
	      vfstype = ofsp->vfs;
	}
	/* do the original mount */
	rc = domount2(object, stub, vfstype, node, mflags, options);

	/* if error or NOT doing inherited mounts, return */
	if (rc || !iflag)
		return(rc);

	/* INHERITED MOUNTS only in the following code */

	iflag = 0;		/* clear "inherit" for future fs mounts */
	doing_inherit = 1;	/* but tell domount2 we're inheriting */

	if (*object != '/') {
		eprintf(MOUNT_MSG (Err_INHERITQUAL,
		   "Object %s for inherited mount is not fully qualified\n"),
			object);
		return(1);
	}
	/* canonicalize the object path */
	do_canon(object);

	/* get the mount query buffer from the requested node.  */
	if ((nmount = mnt_query(FALSE, node, vfstype, &mqdata)) <= 0) {
		eprintf(MOUNT_MSG (Err_INHERITSTAT, "Failed getting mount status buffer for inherit\n"));
		return(1);
	}

	/*
	 * NOTE: all pathnames as returned from the mnt_query are:
	 * (1) fully qualified - ie begin with a "/"
	 * (2) in canonical form - ie  ".."'s
	 */

	/* if local node, remove last entry from mount status.
	 * because we just mounted that guy (we hope!).
	 */
	if (node == NILPTR (char)) {
		free((void *)mqdata[--nmount]);
		mqdata[nmount] = NILPTR (struct mntquery);
	}

	/* if object is "/" we 'do all mounts but root' */
	if (strcmp(object, "/") == 0) {
		free((void *)mqdata[0]);
		mqdata[0] = &invalid;
		root_case = TRUE;
		goto doit;
	}

	objlen = strlen(object);
	/* traverse the list and see if this mount qualifies for inheritance */
	for (mq = mqdata, i = 0; i < nmount; mq++, i++){

		if (*mq == &invalid)
			continue;

		mqstub = (*mq)->mq_stub;
		mqstublen = strlen((*mq)->mq_stub);

		if (objlen >= mqstublen ||
				(strncmp(object, mqstub, objlen) != 0)) {
			free((void *)*mq);
			*mq = &invalid;
			continue;
		}
	}

doit:
	/* for (all in mount status that are not disqualified) */
	for (mq = mqdata, i = 0; i < nmount; mq++, i++){
		if (*mq == &invalid)
			continue;
		mqstub = (*mq)->mq_stub;
		tmpnode = node;

		/* Set up the stub name, special case for "/" mount */
		if (root_case)
		    sprintf(istub, "%s%s", stub, mqstub);
		else
		    sprintf(istub, "%s%s", stub, (char *)(mqstub + objlen));

		/* Special case where the local host is a "-" or NIL */
		if (strcmp((*mq)->mq_hostname, "-") == 0 ||
		    strcmp((*mq)->mq_hostname, NILSTR) == 0)
		    (*mq)->mq_hostname = NILPTR (char);

		/* If we are local, stat the object so we know if its a disk */
		if (node == NILPTR (char)) {
		  if (stat((*mq)->mq_object, &statbuf) < 0) {
		    saverr = errno;
		    eprintf(MOUNT_MSG (Err_STATFAIL, "Stat failed for %s: "),
			    (*mq)->mq_object);
		    errno = saverr;
		    perror(NILSTR);
		    continue;
		  }
		  /* If our object is not a disk then mount the object */
		  if ((statbuf.st_mode & S_IFMT) != S_IFBLK)
		      mqstub = (*mq)->mq_object;
		}

		/* If this mq is a mount on a remote machine, then use the
		 * object as the stub.
		 */
		if ((*mq)->mq_hostname != NILPTR (char)) {
		  mqstub = (*mq)->mq_object;
		  tmpnode = (*mq)->mq_hostname;
		}

		/*
		 * if we have passed any options into this routine then
		 * use it, else use the mq_options.
		 */
		if (mflags) {
		  tmpopt = options;
		  tmpflag = mflags;
		}
		else {
		  tmpopt = (*mq)->mq_options;
		  tmpflag = (*mq)->mq_mflag;
		}

		rc = domount2(mqstub, istub, vfstype, tmpnode, tmpflag,
			      tmpopt);
		if (rc != 0)
		    return(rc);

		printf( MOUNT_MSG (Misc_INHERITMNT,
				   "inherited mount of %s on %s\n"),
		       mqstub, istub);
	}
	return(0);
}

/*
 * domount2 is the central function that does ALL mounts.
 *	It does one mount of one thing at a time.
 */
static int
domount2(object, stub, vfsname, node, mflags, options)
char	*object;
char	*stub;
char	*vfsname;	/* vfs name as a character string */
char	*node;
int	mflags;		/* bit mask of mount flags */
char	*options;	/* (vfs specific) options, as string */
{
	register struct vfs_info *vip;	/* for finding vfs_info */
	int	rc = 0;

	/*
	** can't do anything over root filesystem
	**
	** unless in system group and root and force flag *HACK*
	** (This will only be allowed temporarily, until the real boot
	** process is implemented, that is, when we can re-mount a different
	** root filesystem. )
	*/
	if ( (ruid || !in_sysgrp) && !fflag && strcmp (stub, "/") == 0 )
        {
	  eprintf(MOUNT_MSG (Err_OVEROOT, "The root filesystem may not be overmounted\n"));
	  return(1);
	}

	/* get the full pathname of object if user didn't input it */
	if (node == NILPTR (char) && *object != '/') {
		if ((object = get_fullname(object)) == NILPTR (char)) {
			eprintf(BOTH_MSG (Err_FULNAM,
		"Cannot form full name of \"%s\"\n"), object);
			return(1);
		}
	}

	/* get the full pathname of stub if user didn't input it	*/
	if ((stub = get_fullname(stub)) == NILPTR (char)) {
	  eprintf( BOTH_MSG (Err_FULNAM,
		 "Cannot form full pathname of \"%s\"\n"), stub );
	  return(1);
	}

	/* find vfs info for this vfs */
	vip = getvfsibyname(vfsname);
	if (vip == NILPTR (struct vfs_info)) {
	  eprintf( MOUNT_MSG (Err_UNKVFSOBJ,
			    "Unknown vfs name \"%s\" for object %s\n"),
		  vfsname, object);
	  return(1);
	}

	/*
	** do a single mount, either builtin or via helper program
	** if helper specified, he overrides the builtin
	*/
	if (vip->vi_helper != NILPTR (char))
		rc = helper_Mfunc (vip, object, stub,
			(node != NULL) ? node:"", options);
	else if (vip->vi_flags & VFSI_BUILTIN && vip->vi_builtin != (int (*)()) 0)
		rc = (*vip->vi_builtin) (object, stub, vip->vi_type, node,
						mflags, options);
	else {
	  eprintf( MOUNT_MSG (Err_NOHELPER,
			    "No mount helper is defined for %s\n"), object);
	  return(1);
	}
	/*
	 * if an error has occurred, DON't print it out in detail!
	 *	People used to have an elaborate error printout scheme,
	 * BUT: 1. it wasn't the same as the standard perror msgs - so
	 * 	it was A confusing and B incompatible with NLS.
	 *	2. since we have mount helpers, they have to print their
	 *	own error messages.
	 */

	return(rc);
	/*=======*/
}

/*
 * Mount functions: the functions that actually do a mount, comes
 * in two flavors, for builtins and for helpers:
 *
 *	Mount_func(vip, object, stub, vfstype, node, flags, options)
 *
 * RETURNS: 0 ok, error count not ok
 */

/*
 * for builtin mounts of MNT_JFS and MNT_AIX types
 */
static int
builtin_Mfunc(object, stub, vfs, node, flags, options)
char	*object;
char	*stub;
int      vfs;
char	*node;
int	flags;
char	*options;
{
	register int	result;		/* result of vmount system call */
	char		device = FALSE;	/* flags if object is a device */
	struct stat	sb;
	int		vsize;		/* size of info in vmount struct */
	struct vmount	*vmt;
	int 		etmp;
	extern int 	errno;
	dev_t		logdevt;
	struct stat     stbf;
	char           *log = NILPTR (char);
	char	       *info = NILPTR (char);
	int             info_size;
        char           *opts;
	int		opt_len;
	struct stat	stub_sb;		/* stat struct from stub */
	int		stub_statted_ok;	/* did the stat succeed? */
	struct stat	root_sb;		/* stat struct from root */

	/* check mflags and options */
	chkoptions(vfs, &flags,
			&options, vfs == MNT_JFS? &log: NILPTR (char *));
	/*
	 * determine if the mount is of a device
	 */
	if (stat(object, &sb) == 0)
		device = ((sb.st_mode&S_IFMT) == S_IFBLK);

	if (!device && ((flags & MNT_REMOVABLE) != 0)) {
	  eprintf( MOUNT_MSG (Err_RMVNOTDEV,
			    "Non-device mounts may not be removable\n"));
	  return(1);
	}

	opts      = options;
	info_size = 0;

	if (vfs == MNT_JFS)
	{
	  /*
	  ** no logging for file-over-file, dir-over-dir or read-only mounts
	  */
	  if (device && !(flags & MNT_READONLY))
	  {
	    /*
	    ** if no log is yet specified, try to find one
	    */
	    if ((log == NILPTR (char) || strcmp (log, NILSTR) == 0) &&
		(log = get_log (object)) == NILPTR (char))
	    {
	      eprintf (MOUNT_MSG (Err_NOLOG,
				  "There is no plausible log device for %s (\"%s\")\n"),
		       object, stub);
	      return (1);
	    }

	    if (stat (log, &stbf) < 0)
	    {
	      eprintf (MOUNT_MSG (Err_STATLOG,
				  "Stat of log device (\"%s\") failed\n"),
		       log);
	      return (1);
	    }
	    logdevt = stbf.st_rdev;

	    /*
	    ** store logdevt and log in MNT_INFO and MNT_ARGS, resp.
	    */
	    info_size = sizeof (dev_t);
	    if ((info = malloc ( ROUNDUP (info_size + 1))) == NILPTR (char))
	    {
	      eprintf ( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
	      exit (-1);
	    }
	    memset ((void *)info, 0, (size_t)(ROUNDUP(info_size + 1)));
	    memcpy ((void *)info, (void *)&logdevt, (size_t)info_size);
	    if (log != NILPTR (char) && strcmp (log, NILSTR) != 0 &&
		strlen (log) > 0)
	    {
	      /*
	      ** space for options + comma + "log=" + logdev + null
	      */
	      opt_len = strlen (options) + strlen (log) + 6;
	      if ((opts = malloc (opt_len)) == NILPTR (char))
	      {
		eprintf ( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
		exit (-1);
	      }
	      memset ((void *)opts, 0, (size_t)opt_len);
	      sprintf (opts, "%s,log=%s", options, log);
	    }
	  }
	  /*
	   * stat the mountpoint to check its modes later
	   */
	  stub_statted_ok = stat(stub, &stub_sb) == 0;
	}
	/*
	 * build vmount structure
	 */

        /*
         * size is the size of the vmount structure and of the following
         * data items, each of which starts on a long word boundary.
         */
        vsize = sizeof (struct vmount) +
	        ROUNDUP (strlen (object) + 1) +
                ROUNDUP (strlen (stub) + 1) +
		ROUNDUP (strlen (node? node: "-") + 1) +
                ROUNDUP (strlen (node? node: "-") + 1) +
		ROUNDUP (info_size + 1) +
                ROUNDUP (strlen (opts) + 1);
	vmt = (struct vmount *) malloc (vsize);
	if (vmt == NILPTR (struct vmount))
	{
	  eprintf ( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
	  exit(-1);
	}
	memset ((void *)vmt, 0, (size_t)sizeof(struct vmount));
	vmt->vmt_revision = VMT_REVISION;
	vmt->vmt_length   = vsize;
	vmt->vmt_flags    = flags;
	vmt->vmt_gfstype  = vfs;

	/*
	 * now put the data strings into the vmount structure
	 */
	vmountdata(vmt, object, stub, (node)? node: "-",
		(node)? node: "-", info, info_size, opts);

	/*
	 * reset the real and effective uid to ruid if required,
	 * system group can do anything in /etc/filesystems
	 */
	if (in_sysgrp == FALSE)
		seteuid((uid_t)ruid);

	errno = 0;
	result = vmount(vmt, vsize);
	/*
	 * If this was a jfs mount and it failed because the log hasn'y
	 * been replayed, let's ask logredo to play it.
	 */
	if (result != 0 && vmt->vmt_gfstype == MNT_JFS && errno == EFORMAT)
	{
		char command[BUFSIZ];
		/*
		 *  This is really a mount message; however, Misc_REPLAY_LOG
		 *  was mistakenly added to the wrong place in mount.msg
		 *  (the umount only section).  I can't fix mount.msg now
		 *  because idd has all msg files locked out, so we're 
		 *  stuck with this kludge until the next release.
		 */
		fprintf(stdout,
			UMOUNT_MSG(Misc_REPLAY_LOG, "Replaying log for %s.\n"),
			object);
		fflush(stdout);
		sprintf(command, "%s %s", LOGREDO, log);

#ifndef DEBUG
		strcat(command, " > /dev/null");
#endif /* DEBUG */

		/*
		 * If that worked, try the vmount() again.
		 */
		if (system(command) == 0)
		{
			errno = 0;
			result = vmount(vmt, vsize);
		}
		else
			errno = EFORMAT;
	}
	if (result != 0 && vmt->vmt_gfstype == MNT_CDROM && errno == ENOSYS)
	{
		cfg_load.path = CDROM_KMOD_PATH;
		cfg_load.libpath = NULL;
		cfg_load.kmid = (void *)-1;
		errno = 0;
		if (sysconfig(SYS_KLOAD, &cfg_load, sizeof cfg_load)
				== CONF_SUCC)
		{
			cfg_kmod.kmid = cfg_load.kmid;
			cfg_kmod.cmd = CFG_INIT;
			cfg_kmod.mdiptr = NULL;
			cfg_kmod.mdilen = 0;
			if (sysconfig(SYS_CFGKMOD, &cfg_kmod, sizeof cfg_kmod)
					!= CONF_SUCC)
				(void)sysconfig(SYS_KULOAD,&cfg_load, sizeof cfg_load);
			else
			{
				result = vmount(vmt, vsize);
				if (result != 0)
				{
					cfg_kmod.cmd = CFG_TERM;
					(void)sysconfig(SYS_CFGKMOD,
							&cfg_kmod,
							sizeof cfg_kmod);
					(void)sysconfig(SYS_KULOAD,
							&cfg_load,
							sizeof cfg_load);
				}
			}
		}
	}
	if (result != 0 && vmt->vmt_gfstype == MNT_JFS && errno == ENOSYS)
        {
                cfg_load.path = COMP_KMOD_PATH;
                cfg_load.libpath = NULL;
                cfg_load.kmid = (void *)-1;
                errno = 0;
                if (sysconfig(SYS_KLOAD,&cfg_load,sizeof cfg_load) == CONF_SUCC)
                {
                        cfg_kmod.kmid = cfg_load.kmid;
                        cfg_kmod.cmd = CFG_INIT;
                        cfg_kmod.mdiptr = NULL;
                        cfg_kmod.mdilen = 0;
                        if (sysconfig(SYS_CFGKMOD, &cfg_kmod, 
			   	      sizeof cfg_kmod) == CONF_SUCC)
                                		result = vmount(vmt, vsize);
			else
	      			eprintf(MOUNT_MSG(Err_COMPNOTLOADED, 
"The compression algorithm is either not present or not loaded\n\t  into the kernel.\n"));
                }
		else
	      		eprintf(MOUNT_MSG(Err_COMPNOTLOADED, 
"The compression algorithm is either not present or not loaded\n\t  into the kernel.\n"));
        }

	if (result != 0)
	{
		/* Save errno; eprintf could change it */
		etmp = errno;
                eprintf ( MOUNT_MSG (Err_MNTFAIL, "%s on %s: "),
			 vmt2dataptr (vmt, VMT_OBJECT),
			 vmt2dataptr (vmt, VMT_STUB));
		errno = etmp;
		perror (NILSTR);
	}
	setuid((uid_t)0);	/* reset uid to superuser */

	/*
	 * If this was a jfs mount and it failed EFORMAT, check to see
	 * if the superblock is dirty and inform the user if so
	 */
	if (result != 0 && vmt->vmt_gfstype == MNT_JFS && etmp == EFORMAT)
		chk_super(vmt2dataptr(vmt, VMT_OBJECT));

	/***
	 *** WARNING: notes concerning defect 149438
	 ***
	 ***	The code that checked for 'compatible modes' and
	 ***	printed the following bogus message has generated 2 APARS:
	 ***
	 *** "warning: the permissions on %s are incompatible with the
	 *** permissions in the root directory of %s.\n"
	 ***
	 *** 	This code has been removed. If you feel inclined
	 ***	or pressured to add it again, read defects
	 ***	126783 and 149098, talk to all people involved, and
	 ***	then decide to leave well enough alone.
	 ***/

	return result;
}

/*
** log is copied back if successful and non-zero is returned
** none found returns NULL
*/
static char *
get_log (object)
     char *object;
{
  struct old_fs *ofsp;

  /*
  ** sanity check
  */
  if (object == NILPTR (char))
      return NILPTR (char);

  /*
  ** This needs to be architected.  When it is, this stub will become code.
  **
  ** Here is one possible scheme:
  **   1- find name of device which matches logdevt in superblock
  **   2- else, find stanza for this filesystem in FSfile (ODM?!?)
  **      and if there's a 'log=...' attribute, use this name
  **   3- else, find all possible log's on this device
  **              - lvm_queryvg for the volume group this device is on
  **              - find all of these name which may be a log
  **              - (by naming convention) /dev/LOG_PREFIX...
  **              - use name of 1st one of these which has a valid
  **                log superblock in it
  **
  ** Until this is decided, pass back the attribute value in FSfile
  ** if it is specified
  */
  if ((ofsp = get_old_fs_dev (object)) == NILPTR (struct old_fs))
      return NILPTR (char);
  if (strcmp (ofsp->log, NILSTR) == 0)
      return NILPTR (char);
  return ofsp->log;
}

static int
vmountdata(vmtp, obj, stub, host, hostname, info, info_size, args)
register struct vmount  *vmtp;
char                    *obj, *stub, *host, *hostname, *info, *args;
int                      info_size;
{
        register struct data {
                                short   vmt_off;
                                short   vmt_size;
                        } *vdp, *vdprev;
        register int    size;

#ifdef DEBUG
        BUGLPR (mdebug, BUGACT,
     ("vmountdata: 0x%x \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" %d\n",
                vmtp, obj, stub, host, hostname, args, info_size));
#endif
        vdp = (struct data *) vmtp->vmt_data;

        vdp->vmt_off = sizeof(struct vmount);
        size = ROUNDUP(strlen(obj) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_OBJECT), obj);

        vdprev = vdp;
        vdp++;
        vdp->vmt_off = vdprev->vmt_off + size;
        size = ROUNDUP(strlen(stub) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_STUB), stub);

        vdprev = vdp;
        vdp++;
        vdp->vmt_off = vdprev->vmt_off + size;
        size = ROUNDUP(strlen(host) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_HOST), host);

        vdprev = vdp;
        vdp++;
        vdp->vmt_off = vdprev->vmt_off + size;
        size = ROUNDUP(strlen(hostname) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_HOSTNAME), hostname);

        vdprev = vdp;
        vdp++;
        vdp->vmt_off = vdprev->vmt_off + size;
        size = ROUNDUP(info_size + 1);
        vdp->vmt_size = size;
        memcpy((void *)vmt2dataptr(vmtp, VMT_INFO), (void *)info, (size_t)size);

        vdprev = vdp;
        vdp++;
        vdp->vmt_off = vdprev->vmt_off + size;
	size = ROUNDUP(strlen (args) + 1);
        vdp->vmt_size = size;
        strcpy(vmt2dataptr(vmtp, VMT_ARGS), args);
	return(0);
}

/* ========== */
/*
 * MOUNT / UMOUNT library
 */

/*
 * parse_mq - is used to turn a text string in the format of the canonical
 *	mount status output line into a set of strings pointed to by
 *      various pointers in a mntquery structure
 */
static struct mntquery *
parse_mq( buffer )
char	*buffer;
{
	struct mntquery *mqp;
	char	hostname[32];
	char	object[PATH_MAX];
	char	stub[PATH_MAX];
	char	type[32];
	char	options[BUFSIZ];
	char	date[BUFSIZ];
	int	len, num;
	register char *cp;
	register char *cp2;

	mqp = (struct mntquery *)malloc( sizeof(struct mntquery) );
	if ( mqp == (struct mntquery *)NULL ) {
	  eprintf ( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
	  exit( -1 );
	}

	num = sscanf( buffer, SCAN_FMT, hostname, object, stub, type,
		 date, options);
	if (num < 6)
	{
	  eprintf( MOUNT_MSG (Err_PARSEMNT, "Can not parse the mount buffer\n"));
	  exit (-1);
	}

	len = strlen( hostname ) + strlen( object ) + strlen( stub ) +
		 strlen( type ) + strlen( options ) + 5; /* 5 for 5 nulls */

	if ( (cp = (char *)malloc( len )) == (char *)NULL ) {
		eprintf( BOTH_MSG (Err_NOMEM, "Out of memory\n") );
		exit( -1 );
	}

	/* strcpy these strings into malloced area, saving ptrs in mntquery */
	mqp->mq_object = strcpy( cp, object );
	cp += strlen( object ) + 1;
	mqp->mq_stub = strcpy( cp, stub );
	cp += strlen( stub ) + 1;
	mqp->mq_type = strcpy( cp, type );
	cp += strlen( type ) + 1;
	mqp->mq_options = strcpy( cp, options );
	cp += strlen( options ) + 1;
	mqp->mq_hostname = strcpy( cp, hostname );

	/*
	 * parse out general mount flags
	 */
	mqp->mq_mflag = 0;

	cp = options;
	while ( *cp ) {
		/* skip until next comma or eol, then clear that byte */
		cp2 = cp;
		while ( *cp2 ) {
			if ( (*cp2 == ',') || (*cp2 == '\n') ) {
				*cp2 = '\0';
				cp2++;
				break;
			}
			cp2++;
		}
		if ( strcmp( cp, "ro" ) == 0 ) {
			mqp->mq_mflag |= MNT_READONLY;
		} else
		if ( strcmp( cp, "rmv" ) == 0 ) {
			mqp->mq_mflag |= MNT_REMOVABLE;
		}
		cp = cp2;
	}

	return( mqp );
}

/*
 *	This utility implements the unmount system manager command
 *	which makes a file system unavailable.  It is designed so that:
 *
 *		it can unmount a file system by fs name
 *		it can unmount a file system by device name
 *		it can unmount a local or remote dir/file by stub name
 *		it can unmount all mounted file systems and dirs/files
 *		it can unmount all remotely mounted dirs/files
 *		it can unmount all remotely mounted dirs/files from a
 *							specified nid
 *		it can unmount all mounted dirs/files specified by type
 *		it can forcibly unmount all mounted dirs/files
 *
 *	synopsis
 *		umount [-f] all		(unmount all mounted file systems
 *						and dir/file mounts)
 *		umount [-f] -a		(ditto)
 *		umount [-f] allr	(unmount all remotely mounted
 *						directories and files)
 *		umount [-f] object	(unmount the named file system
 *						or dir/file mount)
 *		umount -s [-f] devname	(unmount device and ignore mount
 *						table) (Superuser only)
 *		umount [-f] -n node	(unmount all dirs/files mounted
 *						from node)
 *		umount [-f] -t type	(unmount all dirs/files specified
 *						by type (= a string) in
 *						/etc/filesystems)
 */
static int
umountmain(argc, argv)
int	argc;
char	*argv[];
{
	extern char	*optarg;
	extern int	optind;
	char		*node = NULL;
	char		*type = NULL;
	char		*fsname = NULL;
	struct vmount	*vmountp;
	int		nmounts, i, flags = 0, count = 0;
	int		errors = 0;	/* number of failures */
					/*   - becomes the exit status  */
	int		aflag = 0;	/* -a on command line?	*/
					/* (umounts all)	*/

	in_sysgrp = sysgrp();	/* is user in sys grp? */

	/* since we didn't used to be a setuid program, set ourselves
	 * back.
	 */
	if ((ruid = getuid()) < 0) {
		eprintf( UMOUNT_MSG (Err_GUIDFAIL, "getuid failed\n"));
		exit(1);
	}

	/*
	 * only let system group people run this as root (so they can
	 * unmount anything)
	 */
	if (in_sysgrp == FALSE)
		setuid((uid_t) ruid);

	/* validate the input parameters */
	while ((i = getopt(argc, argv, "afn:st:D:")) != EOF)
		switch (i) {
		case 'a':
			aflag++;
			break;
		case 'f':
			flags = UVMNT_FORCE;
			break;
		case 'n':
			node = optarg;
			break;
		case 's':
			break;
		case 't':
			type = optarg;
			break;
#ifdef DEBUG
		case 'D':
			mdebug = atoi(optarg);
			break;
#endif /* DEBUG */
		case '?':
		default:
			umountusage();
		}

	if ((argc <= optind && node == NULL && type == NULL && !aflag))
		umountusage();

	/* get the mntctl information */
	if ((nmounts = get_vmount(&vmountp)) <= 0)
		exit(1);

	/* unmount the universe */
	if (aflag) {
		count = unmount("all", nmounts, vmountp, type, node, flags);
	}
	else
	/* now go through all names and flag the ones to unmount */
	if (argc <= optind)
		count = unmount(NILPTR (char), nmounts, vmountp, type, node, flags);
	else {
		if (strcmp(argv[optind], "all") == 0 ||
				strcmp(argv[optind], "allr") == 0)
			fsname = argv[optind];
		else
			fsname = get_fullname(argv[optind]);
		count = unmount(fsname, nmounts, vmountp, type, node, flags);
		if (fsname != argv[optind])
			free((void *)fsname);
	}

	if (count <= 0) {
		eprintf( UMOUNT_MSG (Err_UNOP,
				     "Could not find anything to unmount\n"));
		exit(1);
	}

	/* finally do all the unmounts */
	errors = dounmount(count, nmounts, vmountp, flags);
	exit(errors);
}

static void
umountusage()
{
	fprintf(stderr, UMOUNT_MSG (Err_USAGEUMNT,
		   "Usage: umount [-sf] {-a|-n Node|-t Type|all|allr|Device|File|Directory|Filesystem}\n"));
	exit(-1);
}


/* Some bit flags used below */
#define UM_ALL		0x001		/* the all argument */
#define UM_ALLR		0x002		/* the allr argument */
#define UM_TYPE 	0x004		/* the -t argument */
#define UM_NODE		0x008		/* the -n argument */
#define UM_FSNAME	0x010		/* a fsname is an arg */
#define UM_FORCE	0x020		/* the -f argument */
/*
 * unmount finds the filesystems to be unmounted and sets a flag
 *      so dounmount knows what to do.
 * RETURNS:
 *      number of things to unmount
 */
static int
unmount(fsname, nmounts, vmtp, type, node, flags)
char		*fsname;
int		nmounts;
struct vmount	*vmtp;
char		*type, *node;
int		flags;
{
	int			count = 0;
	register int		mounts;
	register struct vmount	*vmt;
	register int		umtype = 0;
	int			fsmatch, nodematch;

	/*
	 * The only reason to look up the /etc/filesystems file
	 * is if the user specified a -t option (type). In that
	 * case open the file and look up all entries searching
	 * for the type attribute. If we get a match turn on the
	 * UNMOUNTING flag for them.
	 */
	if (type != NULL) {

	  register struct old_fs  *ofsp;
	  char			  *atype;
	  char                    *dev;

	  for (ofsp = OldFsList; ofsp; ofsp = ofsp->next)
	  {
	    atype = ofsp->type;
	    dev   = ofsp->dev;
	    /*
	    ** Look for someone that matches this type
	    */
	    for (mounts = nmounts, vmt = vmtp; mounts--; vmt =
		 (struct vmount *)((int)vmt + vmt->vmt_length)) {
	      if (COMPARE(vmt, VMT_OBJECT, dev) &&
		  COMPARE(vmt, VMT_STUB, ofsp->name) &&
		  strcmp(atype, type) == 0) {
		/* Found him, set MNT_UNMOUNTING */
		vmt->vmt_flags |= MNT_UNMOUNTING;
		count++;
	      }
	    }
	  }
	  if (count == 0)
	  {
		  eprintf (UMOUNT_MSG (Err_UNOP,
			       "Could not find anything to unmount\n"));
		  eprintf (BOTH_MSG (Err_NOTYPES,
			     "There are no 'type=%s' stanzas\n"), type);
		  return(1);
	  }
	  umtype |= UM_TYPE;
	}

	if (fsname != NULL)
		if (strcmp(fsname, "all") == 0)
			umtype |= UM_ALL;
		else if (strcmp(fsname, "allr") == 0)
			umtype |= UM_ALLR;
		else
			umtype |= UM_FSNAME;

	if (node != NULL)
		umtype |= UM_NODE;

	if (flags == UVMNT_FORCE)
		umtype |= UM_FORCE;

	/* search through vmount list and try to find something to unmount */
	for (mounts = nmounts, vmt = vmtp; mounts--; vmt =
	    (struct vmount *)((int)vmt + vmt->vmt_length)) {

		/* CASE: umount all */
		if (umtype & UM_ALL)
			goto endloop;

		/* CASE: umount allr */
		if ((umtype & UM_ALLR) && !COMPARE(vmt, VMT_HOST, "-") &&
		                          !COMPARE(vmt, VMT_HOST, NILSTR))
			goto endloop;

		/* CASE: umount -f x */
		if ((umtype & UM_FORCE) && (umtype & UM_FSNAME)) {
		if (COMPARE(vmt, VMT_STUB, fsname))
				goto endloop;
			else
				continue;
		}

		nodematch = fsmatch = FALSE;
		if ((umtype & UM_NODE) && (COMPARE(vmt, VMT_HOSTNAME, node) ||
		    		        COMPARE(vmt, VMT_HOST, node)))
			nodematch = TRUE;

		if ((umtype & UM_FSNAME) && COMPARE(vmt, VMT_STUB, fsname))
			fsmatch = TRUE;

		/* CASE: umount -n x OR umount -n x fsys */
		if (umtype & UM_NODE) {
			if (nodematch && ((fsname == NULL) || fsmatch))
				goto endloop;
			continue;
		}

		/* CASE: umount -t x is handled above */
		if (umtype & UM_TYPE) {
			/* CASE: umount -t x fsys, is handled here */
			if ((umtype & UM_FSNAME) && !fsmatch) {
				/*
				 * Turn off the flag because it was
				 * turned on above. Decrement the count.
				 */
				vmt->vmt_flags &= ~MNT_UNMOUNTING;
				count--;
			}
			continue;
		}

		/* CASE: umount dev */
		if ((umtype & UM_FSNAME) && (vmt->vmt_flags & MNT_DEVICE) &&
		     COMPARE(vmt, VMT_OBJECT, fsname))
			goto endloop;

		/* CASE: umount fsys */
		if (!fsmatch)
		{
			vmt->vmt_flags &= ~MNT_UNMOUNTING;
			continue;
		}
endloop:
		vmt->vmt_flags |= MNT_UNMOUNTING;
		count++;
	}
	/*
	 * If we have just a filename and no -f then unmount just one
	 * thing. Since we go through the list backwards when we
	 * actually unmount things, we will only unmount the last
	 * thing mounted with that name. Which is what we want!
	 */
	if (count > 0)
		if ((umtype & UM_FSNAME) && ((umtype & UM_FORCE) == 0))
			return(1);
	return(count);
}

/*
 * dounmount goes through the vmount list and unmounts everything
 *	with the UNMOUNTING flag on.
 * RETURNS:
 *      0 for OK or n for the number of errors found.
 */
static int
dounmount(count, nmounts, vmountp, flags)
int		count;		/* number of unmounts to perform */
int		nmounts;
struct vmount	*vmountp;
int		flags;
{
	register struct vmount	*tmp = NULL;
	int			errors = 0;

	/*
	 * first go through the vmount list and make it into a linked list
	 * in reverse order (from latest to oldest). This way we will not
	 * try to unmount a fs while some other fs is still mounted on top.
	 */
	for (; nmounts; nmounts--, vmountp = (struct vmount *)((int)vmountp +
			vmountp->vmt_length)) {
		vmountp->vmt_timepad = (ulong)tmp;
		tmp = vmountp;
	}

	/* finally unmount everything that needs unmounting */
	for (;(count > 0) && (tmp != NULL);
				tmp = (struct vmount *)tmp->vmt_timepad)
		if (tmp->vmt_flags & MNT_UNMOUNTING) {
			if (helper_UMfunc(tmp, flags) != 0)
				errors++;
			else if ((count > 0) && (flags == UVMNT_FORCE))
				printf( UMOUNT_MSG (Misc_UFORCE,
						    "forced unmount of %s\n"),
						vmt2dataptr(tmp, VMT_STUB));
			count--;
		}

	return(errors);
}

/*
 * get_vmount gathers up the mount status for this local machine
 *      using the funky mntctl bizness.
 * RETURNS:
 *      < 0 for -error or > 0 for number of struct vmounts in buffer
 *               which is pointed to by pointer left at *vmountpp.
 */
static int
get_vmount(vmountpp)
struct vmount	**vmountpp;
{
	struct vmount	*vmountp;
	int		 size;
	int		 nmounts;

	size = BUFSIZ;			/* Initial buffer size */
	while (1) {
	  if ((vmountp = (struct vmount *)malloc(size)) == NULL) {
	    eprintf(BOTH_MSG (Err_NOMEM, "Out of memory\n"));
	    return(-errno);
	  }

	  nmounts = mntctl(MCTL_QUERY, size, vmountp);
	  if (nmounts > 0 && vmountp->vmt_revision != VMT_REVISION) {
	    eprintf(BOTH_MSG (Err_STALEFS,
			      "Stale filesystem? revision %d != %d\n"),
		    vmountp->vmt_revision, VMT_REVISION);
	    return(-ESTALE);
	  }
		if (nmounts > 0) {
			*vmountpp = vmountp;
			return(nmounts);
		} else if (nmounts == 0) {
			/* the buffer wasn't big enough ... */
			size = vmountp->vmt_revision;
			free((void *)vmountp);
		} else {
			/* some other kind of error occurred */
			eprintf( BOTH_MSG (Err_MNTCTL, "mntctl error"));
			free((void *)vmountp);
			return(-errno);
		}
	}
	/*NOTREACHED*/
	return 0;
}

/*
 * get_fullname finds the full pathname in case the supplied name was
 * relative.
 * RETURNS:
 *      pointer to full pathname.
 */
static char *
get_fullname(name)
register char	*name;
{
	register char	*fullpath;
	/* get the full pathname if user didn't input it */
	if (name == NULL)
		return(name);
	else {

		if (name[0] == '/') {
		  if ((fullpath = malloc(MAXPATHLEN)) == NULL) {
		    eprintf(BOTH_MSG (Err_NOMEM, "Out of memory\n"));
		    return(NULL);
		  }
		  strcpy(fullpath, name);
		}
		else {
		  if ((fullpath = getcwd((char *)NULL, MAXPATHLEN)) == NULL) {
		    eprintf(BOTH_MSG (Err_FULNAM,
				      "Cannot form full pathname of %s\n"),
			    name);
		    return(NULL);
		  }
		  strcat(fullpath, "/");
			strcat(fullpath, name);
		}
		do_canon(fullpath);
		return(fullpath);
	}
}

/*
 * This routine takes an input string, canonicalizes it,
 *	and then copies the canonicalized string to the
 *	address where the input parameter was.
 */
#define	skipslash(s)	while (*(s) == '/') (s)++;

static void
do_canon(s)
char	*s;				/* the string to be canonicalized */
{
	register char	*sp, *newsp, *nextsp = NULL;
	char		clean[MAXPATH];	/* the canonicalized string */

	 for (sp = s, newsp = clean; sp != NULL; sp = nextsp) {
		skipslash(sp);
		*newsp++ = '/';
		nextsp = strchr(sp, '/');
		if (nextsp != NULL)
			*nextsp = '\0';
		if (strcmp(sp, ".") == 0) {
			newsp--;
			*newsp = '\0';
		} else if (strcmp(sp, "..") == 0) {
			newsp--;
			*newsp = '\0';
			/* at root level ".." is same as "." */
			if ((newsp = strrchr(clean, '/')) == NULL)
				newsp = clean;
			*(newsp + 1) = '\0';

		} else {			/* real component */
			strcpy(newsp, sp);
			newsp += strlen(sp);
		}
		if (nextsp != NULL)
			*nextsp = '/';
	}
	*newsp = '\0';				/* terminate path */
	if (*(--newsp) == '/')			/* path ends with "/" */
		*newsp = '\0';
	if (clean[0] == '\0')			/* path is "/" */
		clean[0] = '/';
	strcpy(s, clean);
	return;
}

/* Tmp bit flags used below */
#define 	TMP_RO 		001	/* Read only */
#define		TMP_RW		002	/* Read Write */
#define		TMP_RM		004	/* Removable */
#define		TMP_ND		010	/* NODEV */
#define		TMP_NS		020	/* NOSUID */
#define		TMP_BS		040	/* SYSV_MOUNT */
#define		NUM_FLGS	6	/* Number of flags */
#define 	MAX_BUF		32	/* size of all strings plus commas
					   doesn't include log=... */


/*
 * This is a confusing story. Mount options and flags may not be
 * consistent. The "readonly" and "removable" options may be set
 * in flags or options or both.
 * chkoptions takes pointers to flags and options and returns
 * consistent versions of both. It assumes that options is NOT
 * internally inconsistent (e.g. both ro and rw flags). Note
 * that we need either "ro" or "rw" in options. We assume that
 * options is a comma separated string of options.
 *
 * Note that this where flag bits get set from options.  Even though
 * nodev, nosuid, and nobsy can only be set via -o options (and thus, can't be
 * inconsistent with the flags) they have to be set here.
 */

static void
chkoptions(vfs, flags, options, logp)
int      vfs;
int	*flags;
char	**options;
char    **logp;
{
	int	tflags = 0;		/* temporary flags word */
	char	*tmpcp, *sp;		/* ptrs for string stuff */
	int 	i, l, mask;
	int	log_strlen;

	if (vfs == MNT_JFS)
		log_strlen = strlen ("log=");

	/*
	 * First make tflags agree with the options string.
	 */
	for (tmpcp = strchr(*options, ','), sp = *options;
	     sp != NULL && *sp != '\0';
	     sp = tmpcp, tmpcp = strchr(tmpcp, ',')) {
		if (tmpcp != NULL)
			*tmpcp++ = '\0';
		if (strcmp(sp, "ro") == 0)
			tflags |= TMP_RO;
		else if (strcmp(sp, "rmv") == 0)
			tflags |= TMP_RM;
		else if (strcmp(sp, "rw") == 0)
			tflags |= TMP_RW;
		else if (strcmp(sp, "bsy") == 0)
			tflags |= TMP_BS;		
		else if (strcmp(sp, "nosuid") == 0)
			tflags |= TMP_NS;
		else if (strcmp(sp, "nodev") == 0)
			tflags |= TMP_ND;
		else if (vfs==MNT_JFS && strncmp(sp,"log=", log_strlen) == 0)
		 	  if ((l = strlen (sp)) > log_strlen)
				if (*logp = malloc (l + 1))
				{
				   memset((void *)*logp, 0, (size_t)(l+1));
				   strncpy(*logp, sp + log_strlen, l);
				}
				else
				{
				   eprintf ( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
				   exit (1);
				}
			else
			   eprintf (MOUNT_MSG (Err_INVALOPT,
				       "invalid mount option: \"%s\"\n"), sp);
		else if (sp != NILPTR (char) && strcmp (sp, NILSTR))
			   eprintf (MOUNT_MSG (Err_INVALOPT,
				       "invalid mount option: \"%s\"\n"), sp);
	}

	/* allocate space, include space for null and commas */
	if ((tmpcp = malloc(MAX_BUF)) == NULL) {
	  eprintf ( BOTH_MSG (Err_NOMEM, "Out of memory\n"));
	  exit(1);
	}
	*tmpcp = '\0';

	/*
	 * Now settle the readonliness by looking at the flags and
	 * the options and picking our favorite mode.
	 */
	if (*flags & MNT_READONLY)
		tflags |= TMP_RO;
	if (*flags & MNT_REMOVABLE)
		tflags |= TMP_RM;
	if ((tflags & TMP_RO) == 0) /* if we're not RO we're RW, right? */
		tflags |= TMP_RW;
	/* If we have both RO and RW, choose RO */
	if ((tflags & (TMP_RO | TMP_RW)) == (TMP_RO | TMP_RW))
		tflags &= ~TMP_RW;

	for (i = 0, mask = 0; i < NUM_FLGS; i++) {
		mask = 1 << i;
		switch (tflags & mask) {
		case TMP_RO:	strcat(tmpcp, "ro");
				*flags |= MNT_READONLY;
				if (tflags & ~mask)
					strcat(tmpcp, ",");
				break;
		case TMP_RW:	strcat(tmpcp, "rw");
				if (tflags & ~mask)
					strcat(tmpcp, ",");
				break;
		case TMP_RM:	strcat(tmpcp, "rmv");
				*flags |= MNT_REMOVABLE;
				if (tflags & ~mask)
					strcat(tmpcp, ",");
				break;
		case TMP_BS:	strcat(tmpcp, "bsy");
				*flags |= MNT_SYSV_MOUNT;
				if (tflags & ~mask)
					strcat(tmpcp, ",");
				break;		
		case TMP_ND:	strcat(tmpcp, "nodev");
				*flags |= MNT_NODEV;
				if (tflags & ~mask)
					strcat(tmpcp, ",");
				break;
		case TMP_NS:	strcat(tmpcp, "nosuid");
				*flags |= MNT_NOSUID;
				if (tflags & ~mask)
					strcat(tmpcp, ",");
				break;
		}
		/* Turn off the bit so we'll know when there are no
		 * more flags;
		 */
		tflags &= ~mask;
	}
	*options = tmpcp;
}
/*
 * sysgrp
 *      return TRUE if (one of) our groups is the system group (0).
 *      On machines where each process has exactly one group,
 *      return !getgid();
 */
static int
sysgrp()
{
	int *gidset = (int *)malloc(NGROUPS * sizeof(int));
	int ngr;

	if (gidset == NULL) {
		ngr = getgid();
		return(!ngr);
	}
	ngr = getgroups((int)NGROUPS, (gid_t *)gidset);

	while (ngr-- > 0)
		if (gidset[ngr] == 0)
		{
			free((void *) gidset);
			return(TRUE);
		}

	free((void *) gidset);
	return(FALSE);
}

/*
 * eprintf is used to print errors, note that it flushes stdout,
 * then prints the error msg, then flushes it.
 */

#include <varargs.h>

/*VARARGS*/
static void
eprintf(va_alist) va_dcl
{
	va_list arglist;
	register char *fmt;
	extern char progname[];

	va_start(arglist);
	fmt = va_arg(arglist, char *);
	fflush(stdout);
	fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, fmt, arglist);
	fflush(stderr);
	va_end(arglist);
}

/*
 * This is here so we can get an independent NL date string.
 */
static char *
getmount_time(thetime)
time_t thetime;
{
	struct tm *tmp;
	static char buf[BUFSIZ];

	if ((tmp = localtime(&thetime)) == NULL)
		return((char *) NULL);

	/* HEY!, this never fails!! */
	strftime(buf, BUFSIZ, BOTH_MSG (Misc_DATE_FMT, "%sD %sT"), tmp);
	return(buf);
}

/*
 * Helper for unmounts
 */
static helper_UMfunc(vmt, flags)
struct vmount *vmt;
int flags;
{
	struct vfs_info *vip;
	int	pid;
	int	status;
	int	etmp;
	char	*helper, *host, *object;
	char    vfsno[BUFSIZ], fbuf[BUFSIZ], dbgbuf[BUFSIZ];

	if ((vip = getvfsibytype(vmt->vmt_gfstype)) == NULL)
	{
		eprintf(BOTH_MSG(Err_UNKGFS,
				 "%s has an unknown gfstype (%d)\n"),
			vmt2dataptr(vmt, VMT_STUB), vmt->vmt_gfstype);
		return(1);
	}
	helper = vip->vi_helper;
	host   = vmt2dataptr(vmt, VMT_HOST);
	object = vmt2dataptr(vmt, VMT_OBJECT);

	/* if no mount helper exists, umount it here
	 */
	if (helper == NULL)
	{
		if (uvmount(vmt->vmt_vfsnumber, flags) < 0)
		{
			etmp = errno;
			eprintf(UMOUNT_MSG(Err_UMNTFAIL, 
					   "error unmounting %s: "),
				strcmp(object, NILSTR) ? object : "?");
			errno = etmp;
			perror(NILSTR);
			return(1);
		}
#if 0
		if (vmt->vmt_vfsnumber == MNT_CDROM && cfg_load.kmid >= 0)
		{
			cfg_load.path = NULL;
			cfg_load.libpath = NULL;
			errno = 0;
			if (sysconfig(SYS_KULOAD, &cfg_load, sizeof cfg_load) <
			    0)
			{
				perror(NILSTR);
				return 1;
			}
		}
#endif
		return(0);
	}


	sprintf(vfsno, "%d", vmt->vmt_vfsnumber);
	sprintf(fbuf, "%d", flags);
#ifdef DEBUG
        sprintf(dbgbuf, "%d", mdebug);
#else
	sprintf(dbgbuf, "%d", 0);
#endif

	if ((pid = fork()) == 0) {	/* child to call helper */
#ifdef DEBUG
		BUGLPR (mdebug, BUGGID, ("%s MNT_UMOUNT %s %s %s %s %s\n",
				helper, dbgbuf, host, object, vfsno, fbuf));
#endif
		execlp(helper, helper, MNT_UMOUNT, dbgbuf, host, object,
							vfsno, fbuf, NULL);
		perror(helper == NILPTR (char)? "Unmount helper": helper);
		exit(1);
	}
	else if (pid > 0) {	/* parent will wait for child */
		wait(&status);
		if (status)
			return(1);
	}
	else {
		perror( MOUNT_MSG (Err_FORKFAIL, "fork of helper failed"));
		exit(1);
	}
	return(0);
}

/*
 * Helper for mounts.
 */
static helper_Mfunc( vip, object, stub, node, options )
struct vfs_info	*vip;
char		*object;
char		*stub;
char		*node;
char		*options;
{
  int	pid;
  int	status;
  char	*helper = vip->vi_helper;
  char   dbgbuf[BUFSIZ];

#ifdef DEBUG
  sprintf (dbgbuf, "%d", mdebug);
#else
  sprintf (dbgbuf, "%d", 0);
#endif

  /*
   * The helper is helpless (ironic, huh?) if we call
   * it as root.  Let it know who we really are.
   */
  if (ruid && !in_sysgrp)
	  setuid((uid_t)ruid);

  if ((pid = fork()) == 0)	/* child to call helper */
  {
    execlp(helper, helper, MNT_MOUNT, dbgbuf, node, object, stub,options,NULL);
    perror(helper == NILPTR (char)? "no mount helper": helper);
    exit(1);
  }
  else if (pid > 0)	/* parent will wait for child */
  {
    wait(&status);
    if (status) {
      if (iflag) {
	eprintf( MOUNT_MSG (Err_TAXES, "Cannot continue inheriting.\n"));
	exit(1);
      } else
	  return(1);
    }
  }
  else
  {
    /* bad news, can't fork! */
    perror(BOTH_MSG (Err_FORKFAIL, "fork of mount helper failed"));
    exit(1);
  }
  return(0);
}

/*
 * vmt_to_mq: Convert a vmount struct into a mntquery struct.
 */

static MNTQP
vmt_to_mq(vmt, vfsname)
struct vmount *vmt;
char *vfsname;
{

	register MNTQP mqp;
	register char *tmp;

	if ((mqp = (MNTQP)malloc(sizeof(struct mntquery))) == NULL)
		return((MNTQP)NULL);

	mqp->mq_mflag = vmt->vmt_flags;

	tmp = vmt2dataptr(vmt, VMT_HOSTNAME);
	if ((mqp->mq_hostname = malloc(strlen(tmp) + 1)) == NULL)
		return((MNTQP)NULL);
	strcpy(mqp->mq_hostname, tmp);

	tmp = vmt2dataptr(vmt, VMT_OBJECT);
	if ((mqp->mq_object = malloc(strlen(tmp) + 1)) == NULL)
		return((MNTQP)NULL);
	strcpy(mqp->mq_object, tmp);

	tmp = vmt2dataptr(vmt, VMT_STUB);
	if ((mqp->mq_stub = malloc(strlen(tmp) + 1)) == NULL)
		return((MNTQP)NULL);
	strcpy(mqp->mq_stub, tmp);

	if ((mqp->mq_type = malloc(strlen(vfsname) + 1)) == NULL)
		return((MNTQP)NULL);
	strcpy(mqp->mq_type, vfsname);

	tmp = vmt2dataptr(vmt, VMT_ARGS);
	if ((mqp->mq_options = malloc(strlen(tmp) + 1)) == NULL)
		return((MNTQP)NULL);
	strcpy(mqp->mq_options, tmp);

	return (mqp);

}

/*
** loop through known vfs's, calling helper to find match on filesystem type
** this is done because individual helpers can recognize their own
** but the mount command shouldn't have to care
*/
static char *
get_vfstypefromhelper (object)
     char *object;
{
  struct vfs_info  *vip;
  char              grab_rc[64];
  int               rc;
  int               real_rc;
  int               mode;
  struct stat       sb;

#ifdef DEBUG
  mode = (mdebug? FSHMOD_PERROR: 0) | (isatty(0)? FSHMOD_INTERACT: 0);
#else
  mode = (isatty(0)? FSHMOD_INTERACT: 0);
#endif

 /*
  * only do this mytype stuff for block devices...
  */
 if (stat(object, &sb) == 0 && (sb.st_mode&S_IFMT) == S_IFBLK)

  for ( vip = vfs_info; vip; vip = vip->vi_next )
  {
    memset ((void *)grab_rc, 0, (size_t)sizeof(grab_rc));
    rc = fshelp (object, vip->vi_name, FSHOP_CHECK, mode,
#ifdef DEBUG
	 mdebug > BUGGID? FSHBUG_BLOOP: FSHBUG_NONE, "mytype", grab_rc);
#else
	 FSHBUG_NONE, "mytype", grab_rc);
#endif
    real_rc = *grab_rc == '\0'? -1: atoi (grab_rc);
    if (rc == FSHRC_GOOD && real_rc == 0)
	return vip->vi_name;
  }
  return NILPTR (char);
}





/*
 * use default vfs in /etc/vfs or if none specified,
 * check thru /etc/filesystems for a nodename and use it's vfs
 */
static char *
getvfstypebynode (node)
char *node;
{
	int                      local;
	register struct old_fs  *ofsp;

	local = node == NILPTR (char) || strcmp (node, "-") == 0 ||
					 strcmp (node, NILSTR) == 0;

	if (local)
		if (strcmp (default_local, NILSTR) != 0)
		    return default_local;
		else
		   /* null */;
	else
		if (strcmp (default_remote, NILSTR) != 0)
		    return default_remote;

	for (ofsp = OldFsList; ofsp; ofsp = ofsp->next)
	    if (strcmp (node, ofsp->nodename) == 0 ||
		(local && strcmp (ofsp->nodename, NILSTR) == 0))
			if (strcmp (ofsp->vfs, NILSTR) != 0)
			  return ofsp->vfs;
	return NILPTR (char);
}

/*
** get_old_fs
**
** given a filesystem name, finds old_fs
*/
static struct old_fs *
get_old_fs (fsname)
     char *fsname;
{
  struct old_fs *ofsp = NILPTR (struct old_fs);

  if (makeFStab (FSfile))
  {
    eprintf ("%s\n", BOTH_MSG (Err_FSTABLING,
			       "Could not table filesystem data"));
    return NILPTR (struct old_fs);
  }

  for (ofsp = OldFsList; ofsp; ofsp = ofsp->next)
  {
      if (strcmp (fsname, ofsp->name) == 0)
	return ofsp;
  }
  return NILPTR (struct old_fs);
}

/*
** get_old_fs_dev
**
** given a filesystem device, finds old_fs
*/
static struct old_fs *
get_old_fs_dev (fsdev)
     char *fsdev;
{
  struct old_fs *ofsp = NILPTR (struct old_fs);

  if (makeFStab (FSfile))
  {
    eprintf ("%s\n", BOTH_MSG (Err_FSTABLING,
			       "Could not table filesystem data"));
    return NILPTR (struct old_fs);
  }

  for (ofsp = OldFsList; ofsp; ofsp = ofsp->next)
  {
      if (strcmp (fsdev, ofsp->dev) == 0)
	return ofsp;
  }
  return NILPTR (struct old_fs);
}

/*
** makeFStab
**
**   tables information in FSfile into handy list of old_fs's
**
**
** Warnings:
**  - uses libIN/AF... routines
**  - will eventually use the ODM
*/

static int
makeFStab (filesystems)
    char *filesystems;
{
  AFILE_t          attrfile;
  ATTR_t           stanza;
  struct old_fs   *ofsp     = NILPTR (struct old_fs);
  struct old_fs   *lastofs  = NILPTR (struct old_fs);

  /*
  ** no use redoing
  */
  if (OldFsList != NILPTR (struct old_fs))
      return 0;

  /*
  ** errno is set if this fails
  */
  if (!(attrfile = AFopen (filesystems, MAXREC, MAXATR)))
  {
      perror (BOTH_MSG (Err_AFOPENFAIL, "AFopen failed"));
      return -1;
  }

  while (stanza = AFnxtrec (attrfile))
  {
    if (!(ofsp = (struct old_fs *) malloc (sizeof (struct old_fs))))
    {
      eprintf (BOTH_MSG (Err_NOMEM, "Out of memory\n"));
      exit (-1);
    }
    memset ((void *) ofsp, 0, (size_t)sizeof(struct old_fs));

    /*
    ** add to the old_fs list
    */
    if (!found_any (stanza, ofsp))
	free ((void *)ofsp);
    else
    {
      if ( lastofs == NILPTR (struct old_fs) )
	OldFsList = ofsp;
      else
	lastofs->next = ofsp;
      lastofs = ofsp;
    }
  }
  AFclose (attrfile);
  return 0;
}

/*
** found_any()
**   tables stanzas conditionally
*/
static int
found_any (stanza, ofsp)
     ATTR_t          stanza;
     struct old_fs  *ofsp;
{
  char    *attrval = NILPTR (char);
  int      found   = 0;

  if (attrval = AFgetatr (stanza, "mount"))
  {
    strncpy (ofsp->mount, attrval, sizeof (ofsp->mount) - 1);
    found++;
  }

  if (attrval = AFgetatr (stanza, "vfs"))
  {
    strncpy (ofsp->vfs, attrval, sizeof (ofsp->vfs) - 1);
    found++;
  }

  if (attrval = AFgetatr (stanza, "dev"))
  {
    while (*attrval) {
	    strcat(ofsp->dev, attrval);
	    attrval += strlen(attrval) + 1;
	    if (*attrval)
	    	strcat(ofsp->dev, ",");
    }
    found++;
  }

  if (attrval = AFgetatr (stanza, "log"))
  {
    strncpy (ofsp->log, attrval, sizeof (ofsp->log) - 1);
    found++;
  }

  if (attrval = AFgetatr (stanza, "nodename"))
  {
    strncpy (ofsp->nodename, attrval, sizeof (ofsp->nodename) - 1);
    found++;
  }

  if (attrval = AFgetatr (stanza, "type"))
  {
    strncpy (ofsp->type, attrval, sizeof (ofsp->type) - 1);
    found++;
  }

  if (attrval = AFgetatr (stanza, "options"))
  {
    while (*attrval) {
	    strcat(ofsp->options, attrval);
	    attrval += strlen(attrval) + 1;
	    if (*attrval)
	    	strcat(ofsp->options, ",");
    }
    found++;
  }

  /*
  ** only worth tabling if it has relevant keywords in it
  */
  if (found)
  {
    strncpy (ofsp->name, stanza->AT_value, sizeof (ofsp->name) - 1);
  }
  return found;
}

/*
 * scanopt searches for the occurrence of ro.
 */
scan_ro(options)
char *options;
{
	char *p, *q;

	p = options;
	while (*p) {
		if ((q = strchr(p, ',')) == NULL) {
			if (!strcmp(p, "ro"))
				return(1);
			else
				return(0);
		}
		else {
			if (!strncmp(p, "ro,", 3))
				return(1);
			else
				p = q + 1;
		}
	}
	return(0);
}

/*
 * scanopt searches for the occurrence of rw.
 */
scan_rw(options)
char *options;
{
	char *p, *q;

	p = options;
	while (*p) {
		if ((q = strchr(p, ',')) == NULL) {
			if (!strcmp(p, "rw"))
				return(1);
			else
				return(0);
		}
		else {
			if (!strncmp(p, "rw,", 3))
				return(1);
			else
				p = q + 1;
		}
	}
	return(0);
}


/*
 * NAME:	chk_super
 *
 * FUNCTION:	the mount failed.  check to see if it's because
 *		the superblock is dirty.
 *
 * PARAMETERS:
 *	char	*dev	- path to device name
 *
 * RETURN:	void
 */

static void
chk_super (char *dev)
{
	int fd;
	struct superblock sb;

	if ((fd = fsopen (dev, O_RDONLY)) < 0)
		return;

	if (get_super (fd, &sb) == 0)
	{
		if (sb.s_fmod != FM_CLEAN)
		{
			fprintf(stderr,	MOUNT_MSG (Err_SBDIRTY,
	   "The superblock on %s is dirty.  Run a full fsck to fix.\n"), dev);
		}
	}
	close(fd);
	return;
}
