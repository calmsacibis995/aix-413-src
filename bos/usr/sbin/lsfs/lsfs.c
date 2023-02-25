static char sccsid[] = "@(#)69	1.73  src/bos/usr/sbin/lsfs/lsfs.c, cmdfs, bos411, 9431A411a 8/3/94 16:04:18";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: crfs, lsfs, chfs, rmfs, imfs
 *
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

/*
 * Announce to the preprocessor that we're in lsfs.c (see lsfs.h)
 */
#define IN_LSFS.C	1

#include "lsfs.h"
#include <libfs/fslimits.h>

extern  void		query_jfs (char*, int, int);
extern 	int		print_jfs_query (char*, int);
extern	void		cr_jfs (char**,	char*, 	mode_t,	struct jfs_attrs*);

/*
 * lsfs.c Module Globals
 */
static	char Getlvcb[] 		= "/usr/sbin/getlvcb";
static	char Putlvcb[] 		= "/usr/sbin/putlvcb";
static	char Chlv[]    		= "/usr/sbin/chlv";
static	char Attrsep[] 		= ":";		/* lv attribute seperator  */
static	char Dev_attr[] 	= "dev";
static	char Mount_attr[] 	= "mount";
static	char Check_attr[] 	= "check";
static	char Node_attr[] 	= "nodename";
static	char Options_attr[] 	= "options";
static	char Type_attr[] 	= "type";
static	char Vfs_attr[] 	= "vfs";
static	char Size_attr[] 	= "size";
static	char Log_attr[] 	= "log";
static	char Acct_attr[]	= "account";
static	char Whitespace[] 	= " \t\n";
static	char *stanzafile 	= FILESYSTEMS;
static	char Devdir[] 		= "/dev/";
static	int Devdirlen 		= sizeof(Devdir) - 1;

/*
 * Local functions
 */
static	void		init_fs (void);
static 	int		found_any (ATTR_t,  struct fs_stanza*);
static 	char 		*findpath (ushort, dev_t, dev_t);
static 	char 		*getvgid (char*);
static	int		putlvattrs (char*, char*);
static	char 		*basename (char*);
static  void		check_fsmax (char*, size_t);
static  int		read_super (int, struct superblock*, char*, int);


/*
 * lsfs specific functions
 */
static  int		do_ls (int,  char**);
static	void		ls_usage (void);
static	int		split_header (char*, char**, int);
static	void		prnt_header (int);
static	void		prnt_stanza (struct fs_stanza*, int, int);

#define DEFAULT_HEADER "Name\t\tNodename\tMount Pt\tVFS\tSize\tOptions\tAuto\tAccounting\n"
#define NUMWORDS 	8

/*
 * crfs specific functions
 */
static	int		do_cr (int,  char**);
static  void		cr_usage (void);
static	void		crfs_check (char*, char*, char*, struct jfs_attrs*, int);

/*
 * chfs specific functions
 */
static	int		do_ch (int,  char**);
static	void		ch_usage (void);
static 	void		repl_opt (char*,  char*);static	size_t		get_size (char*);
static	int		do_extendfs (char*, char*, char*, int);

/*
 * rmfs specific functions
 */
static	int		do_rm (int,  char**);
static	void		rm_usage (void);

/*
 * imfs specific functions
 */
static	int		do_im (int,  char**);
static	void		im_usage (void);
static 	int		exportfs (char*, int);
static 	int		importfs (char*, int);
static 	int		importlv (char*);
static	int		same_vg	(char*, char*);
static	int		exportlv (char*);

#define	GET_ATTR(fs, name)	((a = find_attribute(name, &fs->list))? \
				 a->value : "")

/*
 * function dispatch structure
 */
struct progs
{
    char *progname;		/* program name */
    int (*func)();		/* function to call */
    int errexit;		/* what code to exit on bad return from func */
} prog[] =
{ 
#define DEFPROG 0		/* default program: lsfs */
    { "lsfs", do_ls, 1 }, 
    { "crfs", do_cr, 1 },
    { "chfs", do_ch, 1 },
    { "rmfs", do_rm, 1 },
    { "imfs", do_im, 1 },
};

/*
 *	main
 *
 *	- call dispatch routine
 */

int
main (int argc,
      char **argv)
{
	int i;
	int rv;
	
	if (!(stanzafile = getenv("FILESYSTEMS")))
		stanzafile = FILESYSTEMS;
	
	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_LSFS, NL_CAT_LOCALE);
	
	/*
	 * get program name from argv[0] by stripping path
	 */
	progname = basename(argv[0]); 
	
	/*
	 * identify our name
	 */
	for ( i = 0 ; i < NELEM(prog); i++)
		if (EQ(progname,prog[i].progname)) break;
	
	/*
	 * did we find our name?
	 */
	if (i == NELEM(prog))
	{
		char buf[PROG_NAME_LEN];
		fprintf(stderr,
			MSGSTR(OOPS,"%s: I should be called "),progname);
		for (i = 0; i < NELEM(prog)-1; i++)
		{
			strcat(buf,prog[i].progname);
			strcat(buf,", ");
		}
		strcat (buf,MSGSTR(OR,"or ")); strcat(buf,prog[i].progname);
		printf(MSGSTR(DEFAULT,
			      ". Defaulting to %s.\n"),prog[DEFPROG].progname);
		i = DEFPROG;
	}
	
	init_fs();
	/*
	 * Initialize some shared global variables
	 */
	LPTOUB = 8192L;
	Debugflag = 0;	
	
	/*
	 * call the desired function
	 */
	rv = (prog[i].func)(argc, argv);

	exit (rv == 0 ? 0 : prog[i].errexit);
}

/*
 *	init_fs
 *
 *	- read in /etc/filesystems
 */

static void
init_fs (void)
{
	ATTR_t	 stanza;
	struct fs_stanza  *fs_ptr 	= NILPTR (struct fs_stanza);
	struct fs_stanza  *last_fsptr 	= NILPTR (struct fs_stanza);
	AFILE_t  af;

	af = open_stanzafile();

	while (stanza = AFnxtrec (af))
	{
		if (! (fs_ptr = (struct fs_stanza *)
		      malloc (sizeof (struct fs_stanza))))
		{
			fprintf(stderr, MSGSTR (NOMEMORY,
					"%s: Out of memory.\n"), progname);
			exit (1);
		}
		memset ((char *) fs_ptr, 0, sizeof (struct fs_stanza));
		if (! found_any(stanza, fs_ptr))
			free(fs_ptr);
		else
		{
			if (last_fsptr == NILPTR (struct fs_stanza))
				stanzalist = fs_ptr;
			else
				last_fsptr->next = fs_ptr;
			last_fsptr = fs_ptr;
		}
	}
	AFclose(af);
}
/*
 *	ls_usage
 *
 *	- print usage for lsfs
 */

static void
ls_usage (void)
{
	fprintf(stderr, MSGSTR(LS_USAGE, "Usage: lsfs [-q] [-c|-l] { -a | -v vfstype | -u mtgroup | fsname ... }\n"));
}

/*
 *	rm_usage
 *
 *	- print usage for rmfs
 */

static void
rm_usage (void)
{
	fprintf(stderr, MSGSTR(RM_USAGE, "Usage: rmfs [-r] fsname\n"));
}
/*
 *	cr_usage
 *
 *	- print usage for crfs
 */

static void
cr_usage (void)
{
	fprintf(stderr, MSGSTR(CR_USAGE, "Usage: crfs -v vfs -m mtpt {-g vgrp| -d dev} [-u mtgrp] [-A {yes|no}] [-t {yes|no}] [-p {ro|rw}] [-l log_partitions] [-n nodename] [-a attr1=val1]\n"));
}

/*
 *	ch_usage
 *
 *	- print usage for chfs
 */

static void
ch_usage (void)
{
	fprintf (stderr, MSGSTR(CH_USAGE,
	"Usage: chfs [-n nodename] [-m newmtpt] [-u mtgrp] [-A {yes|no}] [-t {yes|no}] [-p {ro|rw}] [-a attr1=val1] [-d attr] fsname\n"));
}

static void
im_usage (void)
{
	fprintf(stderr, MSGSTR(IM_USAGE, "Usage: imfs [-xl] vgname ...\n"));
}


static int
do_ls (int argc,
       char **argv)
{
	ATTR_t	ar;
	int   	c;
	int	gotone	= 0;	
	int	column	= FALSE;
	int	vfstype	= FALSE;
	int	all	= FALSE;
	int	uflag	= FALSE;
	int	query	= FALSE;
	char  	*fs;
	char  	*vfs;
	char 	*mtgrp;
	char	*fsname;
	struct fs_stanza  *fs_ptr;

	/*
	 * process command line arguments
	 */
	while ((c = getopt(argc, argv, "Dqv:u:acl")) != EOF)
		switch(c)
		{
			case 'D':
			Debugflag++;
			break;
			/*
			 * Query for lvsize, fssize, fragsize ...
			 */
			case 'q':
			query = TRUE;
			break;
		    
			case 'v':
			vfstype = TRUE;
			vfs = optarg;
			break;
			/*
			 * Specify a mount group
			 */
			case 'u':
			uflag = TRUE;
			mtgrp = optarg;
			break;
			/*
			 * All - same as no args
			 */
			case 'a':
			all = TRUE;
			break;
			/*
			 * Colon seperated output
			 */
			case 'c':
			column = TRUE;
			break;
			/*
			 * Default output (white space seperated)
			 */
			case 'l':
			column = FALSE;
			break;
		    
			default:
			ls_usage();
			exit(1);
		}

	/*
	 * vfstype given?
	 */
	if (vfstype)
	{
		/*
		 * known vfs type?
		 */
		if (getvfstype(vfs) == MNT_BADVFS)
		{
			fprintf(stderr, MSGSTR(NOSUCHVFS,
				"%s: \"%s\" is an unknown vfs type\n"),
				progname, vfs);
			exit(1);
		}
	    
		/*
		 * print out all filesystems in this vfs
		 */
		for (fs_ptr = stanzalist;
		     fs_ptr != NULL; fs_ptr = fs_ptr->next)
		{
			if (contains_attribute(fs_ptr, Vfs_attr, vfs))
			    
			{
				if (++gotone == 1)
					prnt_header(column);
				prnt_stanza(fs_ptr, column, query);
			}
		}
	}

	/*
	 * mount group given?
	 */
	else if (uflag)
	{
		for (fs_ptr = stanzalist;
		     fs_ptr != NULL; fs_ptr = fs_ptr->next)
		{
			if (contains_attribute(fs_ptr, Type_attr, mtgrp))
			{
				if (++gotone == 1)
					prnt_header(column);
				prnt_stanza(fs_ptr, column, query);
			}
		}
	}	
	/*
	 * either -a or no fsname arguments given, so print all filesystems
	 */
	else if (all || optind == argc)
	{
		prnt_header(column);
		
		for (fs_ptr = stanzalist;
		     fs_ptr != NULL; fs_ptr = fs_ptr->next)
		{
			prnt_stanza(fs_ptr, column, query);
		}
	}
	
	/*
	 * else, print out filesystems given
	 */
	else
	{
		gotone = 0;
		
		for (; optind < argc; optind++)
		{
			fsname = absolute_and_canonize(argv[optind]);
			if ((fs_ptr = stanza_lookup(fsname,
						    MATCH_EITHER)) == NULL)
			{
				fprintf(stderr, MSGSTR(NOMATCH2,
			"%s: No record matching '%s' was found in %s.\n"),
					progname, fsname, stanzafile);
				exit(1);
			}
			if (++gotone == 1)
				prnt_header(column);
			prnt_stanza(fs_ptr, column, query);
		}
	}

	return (0);
}

/*
 *	do_rm
 *
 *	- perform rmfs
 */

int
do_rm (int argc,
       char **argv)
{
	int   	c;
	int	rc;
	char  	*fsname;
	struct fs_stanza  *fs_ptr;
	struct attribute  *dev, *vfs;

	rflg = FALSE;
	while ((c = getopt(argc, argv, "Dr")) != EOF && c != '?')
    		switch(c)
		{
			case 'D':
			Debugflag++;
			break;

			case 'r':
			rflg = TRUE;
			break;
		}

	/*
	 * 0nly want 1 fsname
	 */
	if (c == '?' || optind != argc - 1)
	{
		rm_usage();
		exit(1);
	}

	/*
	 * see if we recognize this fsname
	 */
	fsname = absolute_and_canonize (argv[optind]);
	if ((fs_ptr = stanza_lookup(fsname, MATCH_EITHER)) ==
	    NILPTR(struct fs_stanza))
	{
		fprintf(stderr,MSGSTR(NOMATCH2,
		"%s: No record matching '%s' was found in %s.\n"),
		progname, fsname, stanzafile);
		exit(1);
	}

	/*
	 * get the device and vfs attributes
	 */
	dev = find_attribute(Dev_attr, &fs_ptr->list);
	vfs = find_attribute(Vfs_attr, &fs_ptr->list);

	if (dev != NULL)
	{
		/*
		 * sorry, can't remove a mounted filesystem
		 */
		if (ismounted(dev->value, NULLPTR, fs_ptr->name, NULLPTR))
		{
			fprintf(stderr, MSGSTR(STMNTED,
			"%s: %s is still mounted.  Please unmount first.\n"),
				progname, fsname);
			exit(1);
		}

		/*
		 * remove the logical volume if we're a journalled filesystem
		 */
		if (vfs != NULL && getvfstype(vfs->value) == MNT_JFS &&
		    file_type(makedevname(dev->value)) == S_IFBLK)
		{
			if (rc = run_cmd("/usr/sbin/rmlv -f %s",
					  getdevname(dev->value)))
			{
				exit (rc);
			}
		}
	}

	/*
	 * remove the stanza
	 */
	remove_stanza(fs_ptr->name);

	/*
	 * remove the mount point
	 */
	 if (rflg)
	      rmdir(fsname);

	return (0);
}

/*
 * NAME:	do_cr
 *
 * FUNCTION:	Create a filesystem. If the filesystem is JFS this function
 *		does somenumber of the following:
 *			1) Creates a log if need be
 *			2) Possibly makes a LV
 *			3) Always calls mkfs on the LV
 *			4) Always adds a stanza to /etc/filesystems
 *			5) Allways puts some filesystem specific data in
 *			block 0 of teh LV (see function putlv). This data is
 *			used by imfs (the evil jfs specific step
 *			child of importvg).
 *	
 *		In general filesystems not created by crfs will not have
 *		number 5. This means that imfs may not always retreive all
 *		the correct data when an importvg is performed. This is
 *		the case for all the base filesystems (hd[1,2,3,4...]).
 *
 */

int
do_cr (int argc,
       char **argv)
{
	int  c;
	int  rv;
	int  jfs	= 0;
	int  makedir 	= 1;	/* by default, make the directory */
	int  makefile 	= 0;

	char  *p;
	char  *dname	= NILPTR(char);
	char  *mntpt	= NILPTR(char);		
	char  *vfstype	= NILPTR(char);
	char  *value	= NILPTR(char);
	char  *type	= NILPTR(char);
	char  *perm	= NILPTR(char);
	char  *nodename	= NILPTR(char);
	char  *automnt	= "no";
	char  *accounting = "no";
	mode_t 	devtype, mnttype;
	struct jfs_attrs  jfs_info;
	struct attrlist   attr;
	struct fs_stanza  fs_ptr;


	jfs_info.size		= 0L;
	jfs_info.fragsize	= 4096;
	jfs_info.nbpi 		= 4096;
	jfs_info.logsiz		= 1;
	jfs_info.lvsize		= NILPTR(char);
	jfs_info.logdev		= NILPTR(char);
	jfs_info.vgrp		= NILPTR(char);
	jfs_info.compress	= "no";
	attr.count = 0;
	
	while ((c = getopt(argc, argv, "Dv:g:d:m:u:A:a:p:l:n:t:")) != EOF)
    		switch(c)
		{
			case 'n':
			nodename = optarg;
			break;

			case 'D':
			Debugflag++;
			break;

			case 'v':
			vfstype = optarg;
			if (getvfstype(vfstype) == MNT_JFS)
				jfs = 1;
			if (getvfstype(vfstype) == MNT_NFS)
				nfs = 1;
			break;

			case 'g':
			jfs_info.vgrp = getdevname(optarg);
			break;

			case 'd':
			dname = getdevname(optarg);
			break;

			case 'a':
			/*
			 * parse attribute
			 */
			switch (parse_attr(optarg, &value, 0))
			{
				case LV_SIZE:
				jfs_info.lvsize = value;
				break;

				case LOG_NAME:
				jfs_info.logdev = getdevname(value);
				break;

				case FRAG_SIZE:
				if ((jfs_info.fragsize = atoi (value)) <= 0)
				{
					fprintf (stderr, MSGSTR(FRAG_VALUE,
 			         "%s:\tInvalid fragment size: -a frag=%s\n"),
						 progname, value);
					cr_usage ();
					exit (1);
				}
				break;

				case NBPI:
				if ((jfs_info.nbpi = atoi (value)) <= 0)
				{
					fprintf (stderr, MSGSTR(NBPI_VALUE,
		   "%s:\tInvalid number of bytes per inode: -a nbpi=%s\n"),
						 progname, value);
					cr_usage ();
					exit (1);
				}				
				break;

				case COMP:
				jfs_info.compress = value;
				break;
				
				case OTHER:
				append_attribute(optarg, value, &attr);
				break;

				default:
				cr_usage ();
				exit (1);
			}
			break;
			
			case 'm':
			mntpt = absolute_and_canonize(optarg);
			break;

			case 'u':
			type = optarg;
			break;

			case 'A':
			automnt = optarg;
			break;

			case 'p':
			perm = optarg;
			if (strcmp(perm, "ro") && strcmp(perm, "rw"))
			{
				fprintf(stderr, MSGSTR(ROORRW,
				"%s: specify ro or rw for -p\n"), progname);
				cr_usage();
				exit(1);
			}
			break;

			case 't':
			accounting = optarg;
			break;

			case 'l':
			if ((jfs_info.logsiz = atoi(optarg)) > 0)
				break;
			/* fall thru */

			default:
			cr_usage();
			exit(1);
		}
	/*
	 * Check required parameters
	 */
	crfs_check (vfstype, mntpt, dname, &jfs_info, jfs);

	/*
	 * If this is to be a JFS then perform
	 * the JFS specific actions.
	 */
	if (jfs)
	{
		/*
		 * get mountpoint & device file type
		 */
		mnttype= file_type (mntpt);
		devtype = (dname == NILPTR(char)) ? (mode_t) 0 :
			file_type(makedevname(dname));

		DPRINTF("mnttype = 0%o, devtype = 0%o\n", mnttype, devtype);
		/*
		 * normal case:
		 * (1)	the mountpoint doesn't exist or it's a directory, and
		 * (2)  the device doesn't exist or it's a block special dev
		 */
		if ((! mnttype || mnttype == S_IFDIR) &&
		    (! devtype || devtype == S_IFBLK))
		{
			cr_jfs (&dname, mntpt, devtype, &jfs_info);
		}
		/*
		 * we also support directory over directory and file over file
		 */
		else if ((mnttype && mnttype != devtype) ||
			 (devtype != S_IFDIR && devtype != S_IFREG))
		{
			fprintf(stderr, MSGSTR(BADTYPES,
		"%s: this type of filesystem is not supported by jfs\n"),
				progname);
			exit(1);
		}
		/*
		 * don't make the mountpoint unless the device type is
		 * directory...
		 */
		else if (devtype != S_IFDIR)
		{
			makefile = 1;
			makedir = 0;
		}
	}
	/*
	 * Set up our fs_stanza to add to the stanza file.
	 * The mountpoint is always the name of the stanza.
	 */
	fs_ptr.name = mntpt;

	/*
	 * first, fill in attributes we didn't recognize (eg the ones
	 * we recognized override these...)
	 */
	fs_ptr.list = attr;

	/*
	 * now, fill in recognized attributes
	 */
	save_attribute(Dev_attr, makedevname(dname), &fs_ptr.list);
	save_attribute(Vfs_attr, vfstype, &fs_ptr.list);

	if (jfs && jfs_info.logdev != NILPTR(char))
		save_attribute(Log_attr, makedevname(jfs_info.logdev),
			       &fs_ptr.list);

	do_changeables (&fs_ptr, nodename, type, automnt,
			accounting, getvfstype(vfstype));

	if (jfs_info.size > 0)
	{
		char buf[BUFSIZ];

		if (!jfs)
		{
			sprintf(buf, "%ld", jfs_info.size);
			save_attribute(Size_attr, buf, &fs_ptr.list);
		}
		fprintf(stdout, MSGSTR(NEWSIZE,
				       "New File System size is %ld\n"),
			jfs_info.size);
	}

	if (perm != NULL)
		append_attribute(Options_attr, perm, &fs_ptr.list);

	append_stanza(&fs_ptr);

	/*
	 * save the label and attributes to the lv
	 *
	 *	What do we do if putlv fails ?
	 *	The filesystem is created; It'll work, but importfs
	 *	may not recreate the /etc/filesystems stanza 100% correctly.
	 *	It will create a default stanza which will be wrong if the
	 *	any of the options have non-default values.
	 *	This is not seen as major problem at this time. This problem
	 *	exists for all filesystems not created w/ crfs (ie all
	 *	all filesystems made by bosinst). 
	 */
	if (jfs)
		putlv(&fs_ptr);

	/*
	 * make our mountpoint directory if need be
	 */
	if (makedir)
		makedirectory(mntpt);

	if (makefile)
		make_file(mntpt);

	return (0);
}

/*
 *	crfs_check
 *
 *		Perform various commandline sanity checks.
 *		Exit upon any error.
 */

void
crfs_check (char  		*vfstype,
	    char 	 	*mntpt,
	    char  		*dname,
	    struct jfs_attrs	*jfs_info,
	    int	 		jfs)
{
	int 		fd;
	size_t		maxsz;
	struct devinfo  dinfo;
	
	/*
	 * vfstype MUST be specified
	 */
	if (vfstype == NILPTR(char))
	{
		fprintf(stderr, MSGSTR(NEEDVFS,
				"%s: vfstype must be specified.\n"), progname);
		cr_usage();
		exit(1);
	}
	/*
	 * and, we must recognize it...
	 */
	if (getvfstype(vfstype) == MNT_BADVFS)
	{
		fprintf(stderr,MSGSTR(NOSUCHVFS,
			"%s: \"%s\" is an unknown vfs type\n"), progname,
			vfstype);
		cr_usage();
		exit(1);
	}
	/*
	 * mount point MUST be given ...
	 */
	if (mntpt == NILPTR(char))
	{
		fprintf(stderr, MSGSTR(NEEDMT,
		"%s: mount point must be specified\n"), progname);
		cr_usage();
		exit(1);
	}
	/*
	 * and it must not already exist in /etc/filesystems...
	 */
	if (stanza_lookup(mntpt, MATCH_MOUNTPT) != NILPTR(struct fs_stanza))
	{
		fprintf(stderr, MSGSTR(FILEEXIST,
		"%s: %s file system already exists\n"), progname, mntpt);
		exit(1);
	}
	/*
	 * if device name not given....
	 */
	if (dname == NILPTR(char))
	{
		/*
		 * it's required if we're not doing a journalled filesystem
		 */
		if (!jfs)
		{
			fprintf(stderr,
				MSGSTR(NEEDDEV, "%s: device name missing\n"),
				progname);
			cr_usage();
			exit(1);
		}
		/*
		 * and, if we are doing a jfs, we need the volume group...
		 */
		if (jfs_info->vgrp == NILPTR(char))
		{
			fprintf(stderr, MSGSTR(NEEDVG,
			     "%s: specify one of volume group or device\n"),
			     progname);
			cr_usage();
			exit(1);
		}
	}
	/*
	 * if device name given check logical volume type if jfs
	 */
	else
	{
		if (jfs)
		{
			if (! jfstype(dname))
			{
				fprintf(stderr,
				MSGSTR(JFSTYPE, 
				"%s: logical volume must be type jfs\n"),
					progname);
				cr_usage();
				exit(1);
			}

			DPRINTF("-- checking out size of %s...\n",
				makedevname(dname));

			if ((fd = open (makedevname(dname), O_RDONLY)) >= 0)
			{
			        maxsz = MIN (FS_ADDR_LIM (jfs_info->fragsize), 
					     FS_NBPI_LIM (jfs_info->nbpi));

				DPRINTF("--   open of %s worked...\n",
					makedevname(dname));
				
				if (ioctl (fd, IOCINFO, &dinfo) == 0 &&
	    			    dinfo.un.dk.numblks > maxsz)
				{
					fprintf(stderr, MSGSTR(LV_TOO_BIG, 
	"%s: Warning: logical volume %s larger than filesystem max (%ld).\n"),
						progname, dname, maxsz);
				}
				DPRINTF("--     size is %ld (max is %ld)...\n",
					dinfo.un.dk.numblks, maxsz);
				close(fd);
			}
		}
	}
}

/*
 *	do_ch
 *
 *	- perform chfs
 */

int
do_ch (int argc,
       char **argv)
{
	int 	c;
	int	rc;
	char 	*nodename 	= NILPTR(char);
	char 	*newsize 	= NILPTR(char);
	char 	*newmnt		= NILPTR(char);
	char 	*newtype 	= NILPTR(char);
	char 	*automnt	= NILPTR(char);
	char 	*accounting 	= NILPTR(char);
	char 	*perm		= NILPTR(char);
	char 	*fsname		= NILPTR(char);
	char 	*value 		= NILPTR(char);
	char 	*vfs		= NILPTR(char);
	char 	*dev 		= NILPTR(char);
	char 	opdata[16];	/* long enough to return code from op_extend */
	struct fs_stanza  *fs_ptr;
	struct attribute  *a, *d;
	struct attrlist   deletes, attr;

	/*
	 * process flags
	 */
	deletes.count = 0;
	attr.count = 0;
	
	while ((c = getopt(argc, argv, "Dn:a:m:u:A:p:d:t:")) != EOF)
    		switch(c)
		{

		    case 'D':
			Debugflag++;
			break;

			case 'n':
			nodename = optarg;
			break;

			case 'd':
			/*
			 * parse attribute
			 */
			parse_attr(optarg, &value, 1);
			save_attribute(optarg, value, &deletes);
			break;

			case 'a':
			switch (parse_attr(optarg, &value, 0))
			{
				case LV_SIZE:
				newsize = value;
				break;

				case OTHER:
				append_attribute(optarg, value, &attr);
				break;

				case FRAG_SIZE:
				case NBPI:
				default:
 				ch_usage ();
				exit (1);
			}
			break;

			case 'm':
			newmnt = absolute_and_canonize(optarg);
			break;

			case 'u':
			newtype = optarg;
			break;

			case 'A':
			automnt = optarg;
			break;

			case 't':
			accounting = optarg;
			break;
			
			case 'p':
			perm = optarg;
			if (strcmp(perm, "ro") && strcmp(perm, "rw"))
			{
				fprintf(stderr, MSGSTR(ROORRW,
				"%s: specify ro or rw for -p\n"), progname);
				ch_usage();
				exit(1);
			}
			break;
			default:
			ch_usage();
			exit(1);
		}

	/*
	 * filesystem given?
	 */
	if (optind != argc - 1)
	{
		ch_usage();
		exit (1);
	}
	/*
	 * get stanza
	 */
	fsname = absolute_and_canonize(argv[optind]);
	if ((fs_ptr = stanza_lookup (fsname, MATCH_EITHER)) ==
	    NILPTR(struct fs_stanza))
	{
		fprintf(stderr,MSGSTR(NOMATCH2,
		"%s: No record matching '%s' was found in %s.\n"),
		progname, fsname, stanzafile);
		exit(1);
	}

	/*
	 * first, fill in attributes we didn't recognize (eg the ones
	 * we recognized override these...)
	 */
	for (a = attr.attr; a < attr.attr + attr.count; a++)
		save_attribute(a->name, a->value, &fs_ptr->list);

	/*
	 * next, delete those attributes someone needed deleted
	 */
	for (d = deletes.attr; d < deletes.attr + deletes.count; d++)
		if (a = find_attribute(d->name, &fs_ptr->list))
			*a->value = '\0';

	vfs = GET_ATTR(fs_ptr, Vfs_attr);
	dev = GET_ATTR(fs_ptr, Dev_attr);

	/*
	 * it must not already exist in /etc/filesystems...
	 */
	if (newmnt != NILPTR(char) &&
	    stanza_lookup (newmnt, MATCH_MOUNTPT) !=
	    NILPTR(struct fs_stanza))
	{
		fprintf(stderr, MSGSTR(FILEEXIST,
		"%s: %s file system already exists\n"), progname, newmnt);
		exit(1);
	}

	/*
	 * process recognized attributes
	 */
	if (newsize != NILPTR(char))
	{
		/*
		 * This only works for journalled filesystems
		 */
		if (getvfstype(vfs) == MNT_JFS && file_type(dev) == S_IFBLK)
		{
			int 	rv;
			long 	num_part, old_part;
			size_t	oldfssize, maxsz;
			long 	vgpart;
			char 	*lvsize, *value;
			char 	sizebuf[BUFSIZ];
			long 	nsize;

			/*
			 * get the logical partition size of the lv &
			 * compute the logical to ubsize factor
			 */
			if (vgpart = getvgpart(NULLPTR, dev))
				LPTOUB = vgpart / UBSIZE;

			DPRINTF("LPTOUB = %ld\n", LPTOUB);

			/*
			 * get current filesystem size
			 */
			oldfssize = getfssize(fs_ptr->name, dev, 1);
			if (oldfssize == 0)
				exit(1);
		
			if (*newsize == '+')
				nsize = oldfssize + atol(newsize + 1);
			else
				nsize = atol(newsize);

			DPRINTF("old size = %ld,new = %ld\n",oldfssize,nsize);

			/*
			 * Shrinking is not supported
			 */
			if (oldfssize > nsize)
			{
				fprintf(stderr, MSGSTR(CANTREDU,
				"%s: Cannot reduce size of file system\n"),
					progname);
				exit(1);
			}

			/*
			 * filesystem already that size?
			 */
			if (oldfssize == nsize)
			{
				fprintf(stdout, MSGSTR(SIZEALREADY,
				"File System size already %ld\n"), oldfssize);
				return (0);
			}

			/*
			 * round newsize up to logical partition boundary
			 */
			nsize = ((nsize + LPTOUB-1) / LPTOUB) * LPTOUB;

			/*
			 * Can not allow the lv to be extended 
			 * past the max fs size.
			 */
			check_fsmax (makedevname(dev), nsize);

			/*
			 * get current lv size
			 */
			lvsize = getlvsiz(dev);

			/*
			 * compute old & new number of logical partitions
			 */
			old_part = (atol(lvsize) + LPTOUB - 1) / LPTOUB;
			num_part = nsize / LPTOUB;

			/*
			 * extend logical volume?
			 */
			if (num_part > old_part)
			{
				if (rc = run_cmd("/usr/sbin/extendlv %s %d",
					getdevname(dev), num_part - old_part))
				{
					exit (rc);
				}
			}
			/*
			 * format size and extend the filesystem
			 */
			sprintf(sizebuf, "%ld", nsize);	
			rv = extendfs(vfs, dev, sizebuf, opdata, 0);
			if (rv)
				exit(rv);

			fprintf(stdout, MSGSTR (SIZECH,
				"File System size changed to %s\n"), sizebuf);
		}
		else
			save_attribute(Size_attr, newsize, &fs_ptr->list);
	}

	do_changeables (fs_ptr, nodename, newtype,
			automnt, accounting, getvfstype(vfs));

	if (perm != NILPTR(char))
			append_attribute(Options_attr, perm, &fs_ptr->list);
	/*
	 * new mountpoint?
	 */
	if (newmnt != NILPTR(char))
	{
		/*
		 * if we're jfs, and the filesystem
		 * isn't mounted go ahead and change the label...
		 */
		if (getvfstype(vfs) == MNT_JFS &&
		    !ismounted(dev, NULLPTR, NULLPTR, NULLPTR))
		{
			if (rc =
		    run_cmd("echo y | /usr/sbin/mkfs -V %s -l %s -o labelit %s",
			    vfs, newmnt, dev))
			{
				exit (rc);
			}
		}
	}

	change_stanza(fs_ptr, newmnt);
	if (newmnt != NILPTR(char))
	{
		free(fs_ptr->name);
		fs_ptr->name = newmnt;
	}

	/*
	 * put the stanza onto the lv
	 */
	if (getvfstype(vfs) == MNT_JFS)
		putlv(fs_ptr);

	/*
	 * make the new mountpoint if given
	 */
	if (newmnt != NILPTR(char))
		makedirectory(newmnt);

	return(0);
}
/*
 *	do_im
 *
 *	- perform imfs
 */
do_im (int argc,
       char **argv)
{
	int c, rc, xflag, lflag;

	rc = lflag = xflag = 0;

	while ((c = getopt(argc, argv, "Dxl")) != EOF && c != '?')
	{
    		switch(c)
		{
			case 'D':
			Debugflag++;
			break;
			
			case 'x':
			xflag++;
			break;
			
			case 'l':
			lflag++;
			break;
		}

	}
	if (c == '?' || optind == argc)
	{
		im_usage();
		exit(1);
	}

	for (; optind < argc && rc == 0; optind++)
		rc = xflag ? exportfs(argv[optind], lflag) :
			importfs(argv[optind], lflag);
	
	return (rc);
}

/*
 *	makedirectory
 *
 *	- make a directory if it doesn't already exist
 *
 *	Defect 149483:	Create the mount point with search permissions
 *			for all users; this eliminates problems
 *			discussed in defects 80755, 126783, 149098, and
 *			feature 72704.
 *
 */

makedirectory (char 	*dir)
{
	int		rc;
	char 		*p;			/* generic ptr */
	char 		path[MAXPATHLEN + 1];	/* workspace */
	mode_t		omask;			/* current umask */
	struct stat 	statbuf;
	

	DPRINTF("makedirectory(\"%s\")\n", dir);

	/*
	 * walk through directory path and make parents if necessary
	 */
	strcpy(path, dir);

	for (p = path[0] == '/' ? &path[1] : &path[0];
	     (p = strchr(p, '/')) != NULL; p++)
	{
		*p = '\0';		/* zap slash */
		DPRINTF("\tchecking/making %s\n", path);

		if (stat(path, &statbuf) < 0 && errno == ENOENT &&
		    mkdir(path, 0777) < 0)
		{
			PERROR2(path, "mkpath");
			exit(1);
		}
		*p = '/';		/* restore slash */
	}

	if (stat(dir, &statbuf) < 0 && errno == ENOENT)
	{
		omask = umask (0);
		umask (omask & ~0111);
		if (rc = mkdir(dir, 0777))
			PERROR2(dir, "mkdir");

		umask (omask);
		if (rc)
			exit(1);
	}
}

/*
 *	make_file
 */

void
make_file (char *file)
{
	struct stat statbuf;

	if (stat(file, &statbuf) < 0 && errno == ENOENT &&
	    creat(file, 0644) < 0)
	{
		PERROR2(file, "creat");
		exit(1);
		}
}

/*
 * scanopt searches for the occurrence of ro or rw.
 */

int scanopt (char *options)
{
	char *p, *q;

	p = options;
	while (*p)
	{
		if ((q = strchr(p, ',')) == NULL)
		{
			if (!strcmp(p, "ro") || !strcmp(p, "rw"))
				return(1);
			else
				return(0);
		}
		else
		{
			if (!strncmp(p, "ro,", 3) || !strncmp(p, "rw,", 3))
				return(1);
			else
				p = q + 1;
		}
	}
	return(0);
}

/*
 *	mklv
 *
 *	- make a logical volume
 */
char *
mklv (char *vgrp, long size, char *format, ...)
{
	char *lv;
	va_list args;
	char optional[BUFSIZ];

	/*
	 * handle optional args to mklv...
	 */
	va_start(args, format);
	vsprintf(optional, format, args);
	va_end(args);
	
	lv = read_cmd("/usr/sbin/mklv %s %s %ld", optional, vgrp, size);

	if (lv == NULL)
	{
		fprintf(stderr, MSGSTR(CANCRLV,
			"%s: Cannot create logical volume.\n"), progname);
	}

	return(lv);
}

/*
 *	process_lvs
 *
 *	if (type == IS_VG) then
 *	- process_lvs walks thru the volume group 'name' and calls
 *	  the function 'func' for each lv we find there.
 *
 *	else  type == IS_PV
 *	- process_lvs walks thru the physical volume 'name' and calls
 *	  the function 'func' for each lv we find there.
 *
 *
 *	- keep calling 'func' until it returns non-zero.  when that
 *	  happens, stuff the return code into '*return_code' and return
 *	  the lv we were looking at
 */

char *
process_lvs (char *name,
	     int (*func)(),
	     int *return_code,
	     lvmid_t type)
{
	FILE *fp;
	char *vgid;		/* volume group id */
	char *lvid;		/* logical volume id */
	char *lvname;		/* lv name */
	char *path;		/* lv path */
	char buf[BUFSIZ];

	DPRINTF("process_lvs(\"%s\", \"%s\"):", name,
		 type == IS_VG ? "vg" : type == IS_PV ? "pv" : "??");

	if (type == IS_VG)
	{
		/*
		 * get vgid from odm...
		 */
		vgid = read_cmd("/usr/sbin/getlvodm -v %s", name);

		DPRINTF(" vgid: %s\n", vgid ? vgid : "(null)");

		if (vgid == NULL)
			return (NULL);

		/*
		 * lqueryvg will list out all volumes within that vg
		 */
		sprintf(buf, "/usr/sbin/lqueryvg -g %s -L", vgid);
	}

	else /* type == IS_PV */
		/*
		 * lqueryvg will list out all volumes within that vg
		 */
		sprintf(buf, "/usr/sbin/lqueryvg -p %s -L", name);

	/*
	 * - read thru them and call 'func' for each
	 */
	if ((fp = popen(buf, "r")) != NULL)
	{
		while (fgets(buf, sizeof(buf), fp) != NULL)
		{
			/*
			 *	lvid	volume_name
			 */

			/* get first field */
			lvid = strtok(buf, Whitespace);

			/* get lv */
			lvname = strtok(NILPTR(char), Whitespace);

			/* NULL terminate it */
			(void) strtok(NILPTR(char), Whitespace);

			if (lvname)
			{
				path = makedevname(lvname);
				if (*return_code = (*func)(path))
				{
					pclose(fp);
					return (path);
				}
				free(path);
			}
		}
		
		pclose(fp);
	}

	return(NILPTR(char));
}


/*
 *	querylog
 *
 *	- attempt to figure out a log device for this volume group
 *	  by querying the lvm
 */

char *
querylog (char *vgrp)			/* volume group		*/
{
	int  rc;

	return (process_lvs(vgrp, isa_jfslog, &rc, IS_VG));
}

/*
 *	getvgrp
 *
 *	- given a logical volume name, return the volume group it's in
 */

char *
getvgrp (char *volume)
{
	char *vgrp;

	if ((vgrp = getlvfld(volume, "VOLUME GROUP:")) == NULL)
	{
		fprintf(stderr, MSGSTR(CANTFINDVG,
			"%s: can't find volume group for logical volume %s\n"),
			progname, volume);
		exit(1);
	}

	return (vgrp);
}

/*
 *	getlvfld
 *
 *	- given a logical volume name & a field name, return
 *	  the field value from the lv people
 */

char *
getlvfld (char *volume,
	  char *field)
{
	char *vgrp, *p;

	/*
	 * get the first line of "lslv vol | grep field"
	 */
	if ((vgrp = read_cmd("lslv %s | grep '%s'", volume, field)) != NULL &&
	    /*
	     * get to 'field' column
	     */
	    (p = strstr(vgrp, field)) != NULL &&
	    (p = strchr(p, ':')) != NULL)
	{
		/*
		 * skip leading white space
		 */
		for (++p; isspace(*p); p++)
			;

		/*
		 * null terminate it
		 */
		strtok(p, Whitespace);

		DPRINTF("getlvfld(%s,%s) = %s\n", volume, field, p);

		return (p);
	}

	return (NULL);
}

/*
 *	change_stanza
 *
 *	- change a stanza in /etc/filesystems
 */

change_stanza (struct fs_stanza *p,
	       char *newmnt)
{
	char line[BUFSIZ];
	FILE *fp1, *fp2;
	char **o;
	struct attribute *a;
	/*
	 * the order of recognized attributes
	 */
	static char *order[] =
	{
		Dev_attr, Vfs_attr, Node_attr, Log_attr, Mount_attr,
		Check_attr, Type_attr, Options_attr, Size_attr, NULL,
	};

	/*
	 * clear out marked flags
	 */
	for (a = p->list.attr; a < p->list.attr + p->list.count; a++)
		a->flag &= ~MARKIT;

	/*
	 * open a tmp file
	 */
	if ((fp2 = tmpfile()) == NULL)
	{
		PERROR("tmpfile()");
		exit(1);
	}

	/* 
	 * open and lock the stanza file
	 */
	if ((fp1 = fopen(stanzafile, "r+")) == NULL)
	{
		PERROR(stanzafile);
		exit(1);
	}

	lock_file(fp1);

	/*
	 * Read thru the stanza file, copying everything but the stanza
	 * we're looking for.  Replace that stanza with the new one.
	 */
	while (fgets(line, BUFSIZ, fp1))
	{
		if (match(line, p->name))
		{
			/*
			 * stick in the new stanza
			 */
			fprintf(fp2, "%s:\n", newmnt ? newmnt : p->name);

			for (o = order; *o != NULL; o++)
			{
				if ((a =
				     find_attribute (*o, &p->list)) != NULL)
				{
					if (*a->value)
					{
						fprintf(fp2, "\t%s%s\t= %s\n",
							a->name,
							strlen(a->name) < 8 ?
							"\t" : "",
							a->value);
					}
					a->flag |= MARKIT;
				}
			}

			for (a = p->list.attr;
			     a < p->list.attr + p->list.count; a++)
			{
				if (*a->value && !(a->flag & MARKIT))
					fprintf(fp2, "\t%s%s\t= %s\n", a->name,
						strlen(a->name) < 8 ?
						"\t" : "", a->value);
			}

			/*
			 * skip the old stanza
			 */
			while (fgets(line, BUFSIZ, fp1) &&
			       !isblank(line) &&
			       line[strlen(line) - 2] != ':')
				;
			/*
			 * if it was the last stanza we're done
			 */
			if (feof(fp1))
				break;
		}

		fwrite(line, strlen(line), 1, fp2);
	}

	/*
	 * did an error occur while writing the tmpfile?  if so
	 * do NOT copy the bad tmpfile over /etc/filesystems!
	 */
	if (fflush(fp2) == EOF)
	{
		fprintf(stderr, MSGSTR(UPDATFAIL,
			"%s: update of %s failed\n"), progname, FILESYSTEMS);
	    PERROR("tmpfile");
	    exit(1);
	}

	rewind(fp1);
	rewind(fp2);

	copy_file(fp2, fp1, stanzafile);

	fclose(fp1);
	fclose(fp2);
}

/*
 *	remove_stanza
 *
 *	- remove a stanza from /etc/filesystems
 */

void
remove_stanza (char *fsname)
{
	char line[BUFSIZ], saveline[BUFSIZ];
	FILE *fp1, *fp2;

	DPRINTF("remove_stanza(\"%s\")\n", fsname);

	/*
	 * open a tmp file
	 */
	if ((fp2 = tmpfile()) == NULL)
	{
		PERROR("tmpfile()");
		exit(1);
	}

	/* 
	 * open and lock the stanza file
	 */
	if ((fp1 = fopen(stanzafile, "r+")) == NULL)
	{
		PERROR(stanzafile);
		exit(1);
	}

	lock_file(fp1);

	/*
	 * read thru the stanza file, copying everything but the stanza
	 * we're looking for (and it's preceding blank line)
	 */
	saveline[0] = '\0';
	while (fgets(line, BUFSIZ, fp1) != NULL)
	{
		/*
		 * save 1 blank line
		 */
		if (isblank(line))
		{
			/*
			 * if we already have a blank line saved, write it
			 */
			if (saveline[0] != '\0')
				fwrite(saveline, strlen(saveline), 1, fp2);
			/*
			 * save the new one
			 */
			strcpy(saveline, line);
			continue;
		}

		if (match(line, fsname))
		{
			do
			{
				if (fgets(line, BUFSIZ, fp1) == NULL)
					goto out;
				
			} while (!isblank(line) &&
			       line[strlen(line) - 2] != ':');
			
			saveline[0] = '\0';
		}
		else if (saveline[0] != '\0')
		{
			fwrite(saveline, strlen(saveline), 1, fp2);
			saveline[0] = '\0';
		}

		fwrite(line, strlen(line), 1, fp2);
	}

	if (saveline[0] != '\0')
		fwrite(saveline, strlen(saveline), 1, fp2);
out:

	/*
	 * did the write to the tmpfile fail?  if so, we do not want to
	 * copy the bad tmpfile over /etc/filesystems!
	 */
	if (fflush(fp2) == EOF)
	{
	    fprintf(stderr, MSGSTR (UPDATFAIL,
			"%s: update of %s failed\n"), progname, FILESYSTEMS);
	    PERROR("tmpfile");
	    exit(1);
	}

	rewind(fp1);
	rewind(fp2);

	copy_file(fp2, fp1, stanzafile);

	fclose(fp1);
	fclose(fp2);
}

/*
 *	isblank
 *
 *	return 1 if 'line' contains nothing but white space
 */

int
isblank (char *line)
{
	for ( ; isspace(*line) && *line != '\n'; line++)
		;

	return (*line == '\n');
}

/*
 *	copy_file
 *
 *	- copies from 'from' to 'to'
 */

void
copy_file (FILE *from,
	   FILE *to,
	   char *tofile)
{
	char buf[BUFSIZ];
	int f, t, count;
	off_t len;

	f = fileno(from);
	t = fileno(to);
	while ((count = read(f, buf, BUFSIZ)) > 0)
	{
		if (write(t, buf, count) < 0)
		{
			fprintf(stderr, MSGSTR(UPDATFAIL,
			"%s: update of %s failed\n"), progname, tofile);
			return;
		}
	}
	len = lseek(t, 0, 1);
	ftruncate(t, len);
}

/*
 *	match
 *
 *	- see if 'fsname' is in 'line'
 */

int
match (char *line,
       char *fsname)
{
	char *name;

	if (!strncmp(fsname, line, strlen(fsname)))
	{
		name = line + strlen(fsname);
		if (*name == ':' && *(++name) == '\n')
				return(TRUE);
	}

	return(FALSE);
}

/*
 *	append_stanza
 *
 *	- append stanza 'p' to /etc/filesystems
 */
void
append_stanza (struct fs_stanza *p)
{
	FILE *fd;
	long size;
	char **o;
	struct attribute *a;
	/*
	 * the order of recognized attributes
	 */
	static char *order[] =
	{
		Dev_attr, Vfs_attr, Node_attr, Log_attr, Mount_attr,
		Check_attr, Type_attr, Options_attr, Size_attr, NULL,
	};
	char *vfs = GET_ATTR(p, Vfs_attr);
	char *dev = GET_ATTR(p, Dev_attr);

	/*
	 * clear out marked flags
	 */
	for (a = p->list.attr; a < p->list.attr + p->list.count; a++)
		a->flag &= ~MARKIT;

	/*
	 * open a lock the stanza file
	 */
	if ((fd = fopen(stanzafile, "a+")) == NULL)
	{
		PERROR(stanzafile);
		exit(1);
	}
	lock_file(fd);

	fprintf(fd, "\n%s:\n", p->name);

	for (o = order; *o != NULL; o++)
		if ((a = find_attribute(*o, &p->list)) != NULL)
		{
			if (*a->value)
				fprintf(fd, "\t%s%s\t= %s\n", a->name,
					strlen(a->name) < 8 ? "\t" : "",
					a->value);
			a->flag |= MARKIT;
		}

	for (a = p->list.attr; a < p->list.attr + p->list.count; a++)
		if (*a->value && !(a->flag & MARKIT))
			fprintf(fd, "\t%s%s\t= %s\n", a->name,
				strlen(a->name) < 8 ? "\t" : "", a->value);

	fclose(fd);
}

/*
 * check logical volume type for jfs
 */

int
jfstype(char *devname)
{
	char *type;

	/*
	 *	type = getlvodm -y `getlvodm -l lvid`;
	 */
	type = read_cmd("/usr/sbin/getlvodm -y `getlvodm -l %s`",
			 getdevname(devname));

	if (type == NULL)
	{
		fprintf(stderr, MSGSTR(CANTGETYPE,
			 "%s: Cannot get type from odm.\n"), progname);
		exit(1);
	}

	DPRINTF("type of %s is %s\n", devname, type);

	if (strncmp(type,"jfs",3) == 0)
		return (1);
	else		
		return (0);

}

/*
 *
 * return logical volume id given device name
 *
 */

char *
getlvid(char *devname)
{
	char *lvid;

	/*
	 *	lvid = getlvodm -l dname;
	 */
	lvid = read_cmd("/usr/sbin/getlvodm -l %s", getdevname(devname));

	if (lvid == NULL)
	{
		fprintf(stderr, MSGSTR(CANTGETLV,
		"%s: Cannot get lv id from odm.\n"), progname);
		exit(1);
	}

	DPRINTF("lvid of %s is %s\n", devname, lvid);

	return (lvid);
}

/*
 * return volume group id given group name
 */
static char *
getvgid (char *groupname)
{
	char *vgid;

	/*
	 *	vgid = getlvodm -v groupname;
	 */
	vgid = read_cmd("/usr/sbin/getlvodm -v %s", getdevname(groupname));

	if (vgid == NULL)
	{
		fprintf(stderr, MSGSTR(CANTGETLG,
		"%s: Cannot get lg id from odm.\n"), progname);
		exit(1);
	}

	DPRINTF("lgid of %s is %s\n", groupname, vgid);

	return (vgid);
}

/*
 *	getlvsiz
 *
 *	- get logical volume 'dname's size, stuff it into 'lvsize'
 */

char *
getlvsiz (char *dname)
{
	char *part;
	char lvid[BUFSIZ];
	char volsize[BUFSIZ];

	/*
	 * partitions = lquerylv -c -L lvid
	 */
	strcpy(lvid, getlvid(getdevname(dname)));
	part = read_cmd("/usr/sbin/lquerylv -c -L %s", lvid);

	if (part == NULL)
	{
		fprintf(stderr, MSGSTR (CANTGLVSIZE,
		"%s: Cannot get logical volume size.\n"), progname);
		exit(1);
	}

	sprintf(volsize, "%ld", atol(part) * LPTOUB);

	DPRINTF("getlvsiz(%s): %s blocks\n", dname, volsize);

	return (strsave(volsize));
}


/*
 *	lock_file
 *
 *	- lock file 'fd'
 */

void
lock_file (FILE *fd)
{
	struct flock arg;

	arg.l_type = F_WRLCK;
	arg.l_whence = SEEK_SET;
	arg.l_start = 0;
	arg.l_len = 0;
	(void) fcntl(fileno(fd), F_SETLKW, &arg);
}

/* 
 *	stanza_lookup
 *
 *	- look thru the stanzafile for a stanza that matches 'fsname'
 *	- if (match & MATCH_MOUNTPT) match name/mount point
 *	  if (match & MATCH_DEVICE) match device
 *	  match a device only if there is no name/mount point that matches
 *	  (Of course, if we're matching only devices, that's guaranteed.)
 */

struct fs_stanza *
stanza_lookup (char *fsname,
	       int match)
{
	struct fs_stanza *fs_ptr;
	struct fs_stanza *dev_ptr = NILPTR(struct fs_stanza);

	for (fs_ptr = stanzalist; fs_ptr != NULL; fs_ptr = fs_ptr->next)
	{
		if ((match & MATCH_MOUNTPT) &&
		    strcmp(fs_ptr->name, fsname) == 0)
		{
			return(fs_ptr);
		}
		else if ((match & MATCH_DEVICE) &&
			 !dev_ptr &&
			 contains_attribute(fs_ptr,Dev_attr,fsname))
		{
			dev_ptr = fs_ptr;
		}
	}
	return(dev_ptr);
}


/*
 * found_any()
 *   tables stanzas conditionally 
 */

static int
found_any (ATTR_t stanza,
	   struct fs_stanza  *ofsp)
{
	int found;
	char value[BUFSIZ];
	static struct fs_stanza fsbuf;

	memset ((char *) &fsbuf, 0, sizeof (fsbuf));
 
	found = 1;		/* let's keep each and every stanza... */

	for ( ; stanza->AT_name != NULL; stanza++)
		if (strcmp(stanza->AT_name, "") == 0)
			fsbuf.name = strsave(stanza->AT_value);
		else
			save_attribute(stanza->AT_name,
				do_list(value,stanza->AT_value,sizeof(value)),
				&fsbuf.list);

	if (found)
		memcpy((char *) ofsp, (char *) &fsbuf, sizeof(fsbuf));

	return (found);
}

/*
 *	parse NULL terminated 'list' into 'dest'. use only 'len' chars
 */

char *
do_list (char *dest,
	 char *list,
	 int len)
{
	*dest = '\0';

	while (*list)
	{
		if (strlen(dest) + strlen(list) + 2 >= len)
			break;
		if (*dest) 
			strcat(dest, ",");
		strcat(dest, list);
		list += strlen(list) + 1;
	}

	return (dest);
}

/*
 *	split_header
 *
 *	Split a tab separated header into an array of individual words.
 *	This is to get around the fact that the only header available for
 *	translation is a single tab separated message when it should be a
 *	bunch of short (mostly one word) messages.
 *
 *	return 1 if successful
 *	return 0 if unsuccessful
 */

static int
split_header(char *s, char *sa[], int size)
{
	char *t;
	int i = 0;

	t = strtok(s, "\t");
	while (i < size && t)
	{
		sa[i++] = t;
		t = strtok((char *)0, "\t");
	}
	
	/* if we finished early return failure (i.e., false) */
	if (t || (i < size))
		return 0;

	/* return success (i.e., true) */
	return 1;
}

/*
 *	prnt_header
 *
 *	- prints header line for columnar output
 */

static void
prnt_header (int column)
{
	char *ha[NUMWORDS];	/* the header split into words */
	
	if (column)
	{
		/*
		 * note that this must not be messagized
		 */
		printf ("#MountPoint:Device:Vfs:Nodename:Type:Size:Options:AutoMount:Acct\n");
	}
	else if (split_header(MSGSTR (HEADERINFO, DEFAULT_HEADER),
			      ha, NUMWORDS))
	{
		printf("%-15s %-10s %-22s %-5s %-7s %-10s %-3s %-3s",
		ha[0], ha[1], ha[2], ha[3], ha[4], ha[5], ha[6], ha[7]);
	}
	else
	{
		/* If we can't split it use the English version.
		 * At least we'll be no worse off than when we weren't
		 * even translating it.
		 */
		printf("%-15s %-10s %-22s %-5s %-7s %-10s %-3s %-3s\n",
		       "Name", "Nodename", "Mount Pt", "VFS", "Size",
		       "Options", "Auto", "Accounting");
	}
}

/*
 *	prnt_stanza
 *
 *	- print out a stanza
 */

static void
prnt_stanza (struct fs_stanza *fs_ptr,
	     int column,
	     int query)
{
	size_t	size;
	struct attribute *a;
	int  vfstype;
	char *name 	= fs_ptr->name;
	char *dev 	= GET_ATTR(fs_ptr, Dev_attr);
	char *node 	= GET_ATTR(fs_ptr, Node_attr);
	char *vfs 	= GET_ATTR(fs_ptr, Vfs_attr);
	char *options 	= GET_ATTR(fs_ptr, Options_attr);
	char *mount 	= GET_ATTR(fs_ptr, Mount_attr);
	char *sizeattr 	= GET_ATTR(fs_ptr, Size_attr);
	char *type 	= GET_ATTR(fs_ptr, Type_attr);
	char *acct 	= GET_ATTR(fs_ptr, Acct_attr);
	char *yesstr	= MSGSTR (YES, "yes");
	char *nostr 	= MSGSTR (NO, "no");	
	

	if ((vfstype = getvfstype(vfs)) != MNT_JFS ||
	    (size = getfssize(name, dev, 0)) == 0)
	{
		size = atol(sizeattr);
	}

	if (column)
	{
		/*
		 * if options doesn't contain either "rw" or "ro", assume "rw
		 */
		if (!scanopt(options))
		{
			if (options && *options)
				options = strcatsave(strcatsave(options, ","),
							"rw");
			else
				options = "rw";
		}
		printf("%s:%s:%s:%s:%s:%ld:%s:%s:%s\n",
		       name, dev, vfs, node, type, size, options,
		       strncmp(mount, "true", 4) &&
		       strncmp(mount, "automatic", 9) ? nostr : yesstr,
		       strncmp(acct, "true", 4) ? nostr : yesstr);

	}
	else
	{
		fprintf(stdout, "%-15s %-10s %-22s %-5s ",
			dev, *node ? node : "--", name, *vfs ? vfs : "--");

		if (size > 0)
			fprintf(stdout, "%-7d", size);
		else
			fprintf(stdout, "%-7s", "--");

		fprintf (stdout,
			 " %-10s %-3s  %-3s\n", *options ? options : "--",
			 !strncmp(mount, "true", 4) ||
			 !strncmp(mount, "automatic", 9) ? yesstr : nostr,
			 !strncmp(acct, "true", 4) ? yesstr : nostr);
	}

	if (query && vfstype == MNT_JFS)
		query_jfs (dev, query, column);
}

/*
 *	open_stanzafile
 *
 */

AFILE_t
open_stanzafile (void)
{
    AFILE_t af;

    af = (AFILE_t) AFopen(stanzafile,MAXREC,MAXATTR);
    if (af == NULL)
    {
	PERROR(stanzafile);
	exit(1);
    }
    return(af);
}

/*
 *	extendfs globals
 */
static int	Increment = 0;		/* -i : increment fs size	*/
static int	Bytes = 0;		/* size is in bytes		*/
static int	Ublocks = 0;		/* size is in 1/2K blocks	*/
static int	Filesystem_blocks = 0;	/* size is in fs blocks		*/
static int	Kblocks = 0;		/* size is in Kilobytes		*/
static int	Meg = 0;		/* size is in Megabytes		*/
static int	Partition_size = 0;	/* extend fs to match partition	*/
static int	Query;
static int	Force = 0;		/* force the operation		*/
static int	fsh_mode_flags;		/* file system helper mode flags  */
static int	fsh_debug_level;	/* file system helper debug level */
static char	fsh_opflags[2048];	/* file system helper fs flag	*/
static char   	*Vfsname;		/* command line specified vfs ..*/

/*
 *	extendfs
 *
 *	Front end to the front end of extendfs. 
 *
 *
 */

int
extendfs (char *vfs,
	  char *dev,
	  char *newsize,
	  char *data,
	  int  query)
{
	/*
	 * init fshelper flags
	 */
	fsh_mode_flags = FSHMOD_INTERACT | FSHMOD_FORCE | FSHMOD_PERROR;
	fsh_debug_level = FSHBUG_NONE;
	fsh_opflags[0] = '\0';

	Vfsname = vfs;		/* set vfs type */

	/*
	 * init data
	 */
	data[0] = '\0';
	return (do_extendfs (dev, newsize, data, query));
}

/*
 *	do_extendfs(dev, size)
 *
 *	- do actual work of extending the filesystem 'dev'
 *	- (note that size is always NULL, but we keep it around in case
 *	   we wanna go back to the old ways)
 */

static int
do_extendfs (char *path,
	     char *size,
	     char *opdata,
	     int  query)
{
	size_t	newsize;
	int rc, mounted, readonly;
	char opflags[2048]; 	/* file system helper fs flags		*/
	char stub[PATH_MAX] = { '\0' };
	char *vfsname;
	int vfsnumber;

	/* 
	 * get the real device name, regardless of what they give us.
	 * see if this device is mounted (just to help the backends some),
	 * also convert size argument to real size
	 */
	if ((path = get_dev(path)) == NULL ||
	    (mounted = ismounted(path, &readonly, stub, &vfsnumber)) < 0 ||
	    (newsize = get_size(size)) <= 0)
		rc = 1;

	else
	{
		/*
		 * set up all options to the backend
		 */

		if (fsh_opflags[0])
			sprintf(opflags, "%s,device=%s", fsh_opflags, path);
		else
			sprintf(opflags, "device=%s", path);

		if (!Partition_size)
			sprintf(opflags, "%s,size=%ld", opflags, newsize);

		if (query)
		{
			strcat(opflags, ",");
			strcat(opflags, "query");
		}

		if (Increment)
		{
			strcat(opflags, ",");
			strcat(opflags, "increment");
		}

		if (Partition_size)
		{
			strcat(opflags, ",");
			strcat(opflags, "partition_size");
		}
		else if (Bytes)
		{
			strcat(opflags, ",");
			strcat(opflags, "bytes");
		}
		else if (Ublocks)
		{
			strcat(opflags, ",");
			strcat(opflags, "ublocks");
		}
		else if (Filesystem_blocks)
		{
			strcat(opflags, ",");
			strcat(opflags, "fsblocks");
		}
		else if (Kblocks)
		{
			strcat(opflags, ",");
			strcat(opflags, "kblocks");
		}
		else if (Meg)
		{
			strcat(opflags, ",");
			strcat(opflags, "meg");
		}

		/*
		 * tell the backend if this fs is mounted
		 */
		if (mounted)
		{
			sprintf(opflags, "%s,mounted,stub=%s,vfsnumber=%d",
				opflags, stub, vfsnumber);
			if (readonly)
			{
				strcat(opflags, ",");
				strcat(opflags, "readonly");
			}
		}

		if (Force)
			sprintf(opflags, "%s,force", opflags);

		/*
		 * get the vfs name
		 */
		vfsname = (Vfsname == NULL) ? getvfsname(path) : Vfsname;

		if (Debugflag)
			fsh_debug_level =  FSHBUG_BLOOP;

		/*
		 * call the file system helper
		 */
		rc = fshelp(path, vfsname, FSHOP_CHGSIZ, fsh_mode_flags,
			fsh_debug_level, opflags, opdata);	

		/*
		 * get xcode.  make it 1 if we get a file system
		 * helper error
		 */
		rc = (rc == FSHRC_GOOD) ? atoi(opdata) : 1;
	}

	return (rc);
}

/*
 *	get_size(arg)
 *
 *	- look at the 'size' argument and figure out what they want!
 */

static size_t
get_size (char	*arg)
{
	size_t size;
	int lastchar;

	/*
	 * if no size given, just extend out to the partition size...
	 */
	if (arg == NULL)
	{
		if (Increment)
		{
			fprintf(stderr, MSGSTR(INEGNOSENSE, 
			"-i doesn't make sense unless you specify a delta\n"));
			size = 0;
		}
		else
		{
			Partition_size++;
			size = 1;		/* dummy size */

		}
	}
	else
	{
		/*
		 * else figure out what the user wants...
		 */
		size = atol(arg);
		if ((lastchar = strlen(arg) - 1) > 0)
		{
			/*
			 * note that UBSIZE is default!
			 */
			switch (isdigit (arg[lastchar]) ? 'u' :
				arg[lastchar])
			{
				case 'b':	/* bytes - no conversion    */
				Bytes++;
				break;

				case 'u':	/* UBSIZE blocks	    */
				Ublocks++;
				break;

				case 'f':	/* filesystem blocks	    */
				Filesystem_blocks++;
				break;

				case 'k':	/* 1K blocks		    */
				Kblocks++;
				break;

				case 'm':	/* 1M blocks		    */
				Meg++;
				break;

				default:
				fprintf (stderr,MSGSTR (USEONEONLY,
			       "%s: can only use one of b,u,f,k,m for size\n"),
					 progname);
				return (0);
			}
		}

		if (size <= 0)
		{
			fprintf(stderr, MSGSTR(INVALSIZE,
			"%s: \"%s\": invalid size\n"),	progname, arg);
			size = 0;
		}

		else if (lastchar < 0)
		{
			fprintf(stderr, MSGSTR(EMPTYARG,
					"%s: size arg is empty\n"), progname);
			size = 0;
		}
	}

	return (size);
}

/*
 *	get_dev(path)
 *
 *	- get the real device name
 */

char *
get_dev(char	*path)
{
	mode_t type;
	char *newpath;
	struct stat stbuf;
	char tmp[PATH_MAX];
	register struct stat *st = &stbuf;

	/*
	 * get file mode (this also checks existence and perms)
	 */
	if (stat(path, st) < 0)
	{
		PERROR(path);
		return (NULL);
	}
	type = st->st_mode & S_IFMT;

	/*
	 * if we're given a directory, convert it to a device by looking at
	 * /etc/filesystems...
	 */
	if (type == S_IFDIR)
	{
		/* 
		 * get device name
		 */
		if ((newpath = FSdskname(path)) == NULL)
		{
			fprintf (stderr, MSGSTR(NOTFS,
				"%s: \"%s\": not a filesystem\n"),
				 progname, path);
			return (NULL);
		}
		/*
		 * FSdskname, in all its wisdom, returns memory that's
		 * been free()'d.  so we copy it out, then strsave it
		 * ourselves...
		 */
		strcpy(tmp, newpath);
		path = strsave(tmp);

		/*
		 * re-stat
		 */
		if (stat(path, st) < 0)
		{
			PERROR(path);
			return (NULL);
		}
		type = st->st_mode & S_IFMT;
	}

	/*
	 * we only work on block or char special devs...
	 */
	if (type != S_IFCHR && type != S_IFBLK)
	{
		fprintf (stderr, MSGSTR(NOTFS,
		"%s: \"%s\": not a filesystem.\n"), progname, path);
		return (NULL);
	}

	/*
	 * convert to block device (for now?)
	 */
	if (type == S_IFCHR && (newpath = findpath(S_IFBLK, major(st->st_rdev),
	     minor(st->st_rdev))) != NULL)
		path = newpath;

	return (path);
}

/* 
 *	ismounted(dev, readonly, stub, vfsnumber)
 *
 *	- returns true (TRUE) if 'dev' is mounted, false (FALSE) if not mounted
 *	  If 'stub' isn't null, 'dev' must be mounted on 'stub'
 *	  for true to be returned.
 *	- sets readonly to 1 if this fs is mounted readonly
 *	- also copies in the mounted over stub
 *	- also fills in 'vfsnumber'
 */

int
ismounted (char	*dev,
	   int	*readonly,
	   char	*stub,
	   int	*vfsnumber)
{
	int count, ct, mntsiz;
	struct vmount *vmt;
	char *mnttab;

	mnttab = (char *) &count;
	mntsiz = sizeof(count);

	/*
	 * loop till we have enuf mem to read it in ...
	 */
	while ((ct = mntctl(MCTL_QUERY, mntsiz, mnttab)) <= 0)
	{
		/*
		 * error?
		 */
		if (ct < 0)
		{
			PERROR("mntctl");
			return (FALSE);
		}

		/*
		 * get the current size and either malloc or realloc
		 */
		mntsiz = *((int *) mnttab);
		mnttab = (mnttab == (char *) &count) ?
			(char *) malloc(mntsiz) :
				(char *) realloc(mnttab, mntsiz);
		if (mnttab == NULL)
		{
			PERROR("{m,re}alloc");
			return (FALSE);
		}
	}

	/*
	 * walk thru the mount table, see if this device is mounted
	 */
	for (vmt = (struct vmount *) mnttab; ct > 0;
	     ct--, vmt = (struct vmount *) ((char *) vmt + vmt->vmt_length))
		if (strcmp(vmt2dataptr(vmt, VMT_OBJECT), dev) == 0)
		{
			/*
			 * If stub was passed in, compare further
			 * to see if this is the mount.
			 */
			if (stub != NULL && *stub != '\0'
			    && strcmp(vmt2dataptr(vmt, VMT_STUB), stub) != 0)
					continue;
			if (readonly != NULL)
				*readonly = vmt->vmt_flags & MNT_READONLY;
			if (vfsnumber != NULL)
				*vfsnumber = vmt->vmt_vfsnumber;
			if (stub != NULL && *stub == '\0')
				strcpy(stub, vmt2dataptr(vmt, VMT_STUB));
			free((char *) mnttab);
			return (TRUE);
		}

	free((char *) mnttab);

	return (FALSE);
}

/*
 *	find a device in /dev given its type, major device number and
 *	minor device number.  path returned is static.
 */

static char *
findpath (ushort devtype,
	  dev_t	devmajor,
	  dev_t	devminor)
{
	struct stat stbuf;
	register DIR *dir;
	register struct dirent *d;
	static char path[MAXPATHLEN + 1];
	register struct stat *st = &stbuf;

	if ((dir = opendir(Devdir)) == NULL)
	{
		PERROR(Devdir);
		return (NULL);
	}

	while ((d = readdir(dir)) != NULL)
	{
		sprintf(path, "%s/%s", Devdir, d->d_name);

		if (stat(path, st) < 0)
			PERROR(path);

		else if (devtype == (st->st_mode & S_IFMT) &&
			 devmajor == major(st->st_rdev) &&
			 devminor == minor(st->st_rdev))
				break;
	}

	closedir(dir);

	return (d == NULL ? NULL : path);
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
getvfsname (char *dev)
{
	char *name;
	ATTR_t rec;
	AFILE_t fsfile;
	char *atr;

	/*
	 * try to open the filesystems file
	 */
	if( !(fsfile = AFopen(FSYSname, MAXREC, MAXATR)) ) {
		PERROR(FSYSname);
		return (NULL);
		}

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
		/* strsave it out since AFclose frees everything */
		name = strsave(atr);
	else
		name = NULL;

	AFclose( fsfile );

	return (name);
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
	register char *new;

	if ((new = (char *) malloc(strlen(string) + 1)) == NULL)
	{
		PERROR("malloc");
		exit(1);
	}

	return ((char *)strcpy(new, string));
}

/*
 *	getvfstype
 *
 *	- walk thru /etc/vfs, looking for the vfs type of `vfsent_name'.
 */

int
getvfstype (char *vfsent_name)
{
	int vfstype;
	struct vfs_ent   *vp;
	extern struct vfs_ent  *getvfsbyname();

	if (vfsent_name && *vfsent_name &&
	    (vp = getvfsbyname(vfsent_name)) != NULL)
		vfstype = vp->vfsent_type;
	else
		vfstype = MNT_BADVFS;

	return (vfstype);
}

/*
 *	getjfsname
 *
 *	- walk thru /etc/vfs, looking for the vfs that has type MNT_JFS
 */

char *
getjfsname()
{
	struct vfs_ent   *vp;
	extern struct vfs_ent  *getvfsbytype();

	return ((vp=getvfsbytype(MNT_JFS)) ? strsave(vp->vfsent_name) : "jfs");
}

/*
 * get volume group partition size
 */

long
getvgpart (char *vgrp, char *devname)
{
	char *part;
	int exp;
	long vgpart;

	/*
	 * if volume group is NULL, use device name
	 */
	if (vgrp == NULL)
		part = read_cmd("/usr/sbin/lquerylv -L %s -s", getlvid(devname));
	/*
	 * else use volume group
	 */
	else
		part = read_cmd("/usr/sbin/lqueryvg -g %s -s", getvgid(vgrp));

	if (part == NULL)
	{
		fprintf(stderr, MSGSTR (CANTGVGSIZE,
		"%s: Cannot get volume group partition size.\n"), progname);
		exit(1);
	}

	exp = atoi(part);

	DPRINTF("exp of vg = %s, dev = %s is %d\n", vgrp ? vgrp : "(null)",
		devname ? devname : "(null)", exp);

	vgpart = (long) pow((double) 2, (double) exp);

	DPRINTF("partition size is %ld\n", vgpart);

	return (vgpart);
}

/*
 * get filesystem size - jfs version
 */

size_t
getfssize(char 	*stub, 
	char 	*dev,
	int	verbose)
{
	int	fd;
	size_t	size;
	struct statfs  	sf;
	struct superblock   super;

	/*
	 * only worry about the size of block device mounts
	 */
	if (file_type(dev) != S_IFBLK)
		size = 0;

	/*
	 * filesystem is mounted - just use statfs
	 */
	else if (ismounted (dev, NULLPTR, NULLPTR, NULLPTR))
	{
		if (statfs (stub, &sf) < 0)
			size = 0;
		else
			size = sf.f_bsize / UBSIZE * sf.f_blocks;
	}
	else
	{
		/*
		 * not mounted - read superblock
		 */
		if ((fd = open(dev, O_RDONLY)) < 0)
			size = 0;

		if (read_super (fd, &super, dev, verbose) != 0)
			size = 0;
		else
			size = super.s_fsize;

		close(fd);
	}
	DPRINTF("getfssize(%s, %s) = %ld\n", stub, dev, size);
	return (size);
}

/*
 *	pars_attr
 *
 *	- parse attribute - check format too
 *	- attributes look like:
 *
 *		name[white space]=[white space]value
 *
 *	- 'value' will point to the value
 *	- if 'ignore_equal' is non-zero, don't require the '='
 */

fsattr_t
parse_attr (char *attr,
	    char **value,
	    int ignore_equal)
{
	char	*p, *equal;
	char	size_plus[] = "size+";

	/*
	 * get '='
	 */
	if ((equal = strchr(attr, '=')) == NULL && !ignore_equal)
	{
		fprintf (stderr, MSGSTR(BADATTR,
		"%s: attribute format is \"attr=value\"\n"), progname);
		exit(1);
	}

	/*
	 * blast trailing white space
	 */
	for (p = attr + (strlen(attr) - 1); isspace(*p); p--)
		;
	*(p + 1) = '\0';

	/*
	 * no equal sign = null value
	 */
	if (equal == NULL)
		*value = "";
	else
	{
		/*
		 * complain if there is no attribute
		 */
		if (equal == attr)
		{
			fprintf (stderr, MSGSTR(BADATTR,
			"%s: attribute format is \"attr=value\"\n"), progname);
			exit(1);
		}

		/*
		 * skip optional white space after '=' and assign value
		 */
		for (p = equal + 1; isspace(*p); p++)
			;
		*value = p;

		/*
		 * back up thru optional white space in front of the '='
		 */
		for (p = equal - 1; isspace(*p); p--)
			;
		*(p + 1) = '\0';
	}

	/*
	 * skip white space at beginning
	 */
	while (isspace(*attr))
		attr++;

	/*
	 * complain if there is no attribute (i.e., there was only
	 * whitespace before the '='
	 */
	if ((equal == attr) || (*attr == '\0'))
	{
		fprintf (stderr,
			 MSGSTR (BADATTR,
		"%s: attribute format is \"attr=value\"\n"), progname);
		exit(1);
	}

	if (strcmp (attr, SIZE_str) == 0)
		return (LV_SIZE);
	else if (strcmp (attr, LOG_str) == 0)
		return (LOG_NAME);
	else if (strcmp (attr, FRAG_str) == 0)
		return (FRAG_SIZE);
	else if (strcmp (attr, NBPI_str) == 0)
		return (NBPI);
	else if (strcmp (attr, COMPRESS_str) == 0)
		return (COMP);
	else
	{
		/*
		 * Check for one final pathalogical common error. That is
		 * using the followinf invalid 'C'-like syntax:
		 * 		-a size+=512
		 *
		 */
		if (strcmp (attr, size_plus) == 0)
		{
			fprintf (stderr,  MSGSTR (BADATTR,
			 "%s: attribute format is \"attr=value\"\n"), progname);
			exit(1);
		}
		return (OTHER);
	}
}

/*
 *	basename
 *
 *	- return basename of path (eg basename("/bin/sh") returns "sh")
 */

static char *
basename (char *path)
{
	char *base;

	if ((base = rindex(path, '/')) == NULL)
		base = path;
	else
		base++;

	return (base);
}


/*
 *	run_cmd
 *
 *	- run a command, and return the return code
 */

int
run_cmd (char *format, ...)
{
	int rv;
	va_list args;
	char cmd[BUFSIZ];

	/*
	 * handle variable arguments...
	 */
	va_start(args, format);
	vsprintf(cmd, format, args);
	va_end(args);

	DPRINTF("run_cmd<: \"%s\"\n", cmd);

	if (rv = system(cmd))
	{
		/*
		 * fork or exec failure
		 */
		if (rv == -1 || rv == 127)
		{
			fprintf(stderr, "%s: \"%s\": ", progname, cmd);
			perror("");
		}
		else
		{
			DPRINTF("\texiting because of code %d", rv);

			/*
			 * feeble attempt at deciphering status into
			 * something usable...
			 */
			if (WIFEXITED(rv))
				rv = WEXITSTATUS(rv);
			else if (WIFSIGNALED(rv))
				rv = WTERMSIG(rv);
			else
				rv = 1;		/* punt */

			DPRINTF(" (return %d)\n", rv);
		}
		return(rv);
	}
}

/*
 *	getdevname
 *
 *	- return path of device minus the "/dev/" (if it exists)
 */

char *
getdevname (char *path)
{
	if (strncmp(path, Devdir, Devdirlen) == 0)
		path += Devdirlen;

	return (path);
}

/*
 *	makedevname
 */

char *
makedevname (char *dev)
{
	if (*dev == '/')
		return (strsave(dev));

	if(nfs)
	     return (strsave(dev));
	else
	     return (strcatsave(Devdir, dev));
}

/*
 *
 *	append_attribute
 *
 *	If attribute is already in the list, append it to the existing
 *	list.  If it is not create it.
 *
 *	There is a special case of appending permissions.  If a permissions
 *	string already exists in the attribute, substitute in the new
 *	permissions string.
 *
 */

void
append_attribute (char  *name,
		  char  *value,
		  struct attrlist  *list)
{
	struct attribute *attr;
	char *p;
	char *avalue;

	/*
	 * See if you can find the attribute
	 */
	if (attr = find_attribute(name, list))
	{
		avalue = attr->value;
		/*
		 * Check for special case of appending the permissions
		 * (rw or ro) options,
		 * remember the option looks like the regexp 
		 * [,\0]r[ow][,\0].  The procedure is if value == ro or rw
		 * and you find the regexp in avalue, just switch the second
		 * character and return
		 */
		if (*value == 'r' && 
		   (*(value + 1) == 'o' || *(value + 1) == 'w') &&
		    *(value + 2) == '\0')
		{
			for (p = strchr(avalue, 'r'); p;
			     p = strchr(p + 1, 'r'))
			{
				/*
				 * match [\0,] and  match [ow] and
				 * match [\0,]
				 */
				if ((p == avalue       || *(p - 1) == ',') &&
				    (*(p + 1) == 'o'   || *(p + 1) == 'w') &&
				    (*(p + 2) == ','   || *(p + 2) == '\0'))
				{
					*(p + 1) = *(value + 1);
					return;
				}
			}
		}
		/*
		 * Not a special case so append
		 */
		attr->value = strcatsave(strcatsave(avalue, ","), value);
	}
	else
	{
		/*
		 * No such attribute so add
		 */
		save_attribute(name, value, list);
	}
	return;
}

/*
 *	save_attribute
 *
 *	- save the attribute 'name=value' onto the attribute list 'list'
 *
 *	- first time in, 'list->ct' should be 0
 *
 *	- if a malloc/realloc error occurs, we just exit...
 */

void
save_attribute (char *name,
		char *value,
		struct attrlist *list)
{
	struct attribute *new;

	/*
	 * attribute already exist on list?
	 */
	if ((new = find_attribute(name, list)) != NULL)
		/*
		 * yep - just free the old value
		 */
		free((char *) new->value);

	else
	{
		unsigned size = (unsigned) ((list->count + 1) *
						sizeof(*list->attr));

		/*
		 * nope - create or grow the list...
		 */
		list->attr = (list->count==0) ?
			(struct attribute *) malloc(size):
			(struct attribute *) realloc((char *) list->attr,size);
		if (list->attr == NULL)
		{
			PERROR("malloc");
			exit(1);
		}

		/*
		 * no null pointers allowed...
		 */
		if (value == NULL)
			value = "";

		new = list->attr + list->count++;	/* new entry in list */
		new->name = strsave(name);
	}

	/*
	 * save name and value
	 */
	new->value = strsave(value);
}

/*
 *	find_attribute
 *
 *	- find attribute with name 'name' in 'list' and
 *	  return it.  if it doesn't exist, return NULL
 */

struct attribute *
find_attribute(char *name,
	       struct attrlist *list)
{
	struct attribute *attr;

	for (attr = list->attr; attr < list->attr + list->count; attr++)
		if (strcmp(attr->name, name) == 0)
			return (attr);
	
	return (NULL);
}

/*
 *
 *	contains_attribute
 *
 *	- returns 1 if the stanza 'fs_ptr' contains a 'name = value'
 *	  else returns 0
 */
int
contains_attribute (struct fs_stanza *fs_ptr,
		    char *name,
		    char *value)
{
	struct attribute *attr;

	return ((attr = find_attribute(name, &fs_ptr->list)) &&
		 strcmp(attr->value, value) == 0);
}

/*
 *	strcatsave
 *
 *	- strcat 'str1' and 'str2' together, and strsave
 */

char *
strcatsave(char *str1,
	   char *str2)
{
	register char *new;

	if ((new = (char *) malloc(strlen(str1) + strlen(str2) + 1)) == NULL)
	{
		PERROR("malloc");
		exit(1);
	}

	strcpy(new, str1);

	return (strcat(new, str2));
}

/*
 *	do_changeables
 *
 *	- fill in the changeable (ala do_ch) attributes
 */

void
do_changeables (struct fs_stanza *fs_ptr,
		char *nodename,
		char *newtype,
		char *automnt,
		char *accounting,
		int vfstype)
{
	char	True[]	= "true";
	char	False[]	= "false";		

	if (nodename != NILPTR(char))
		save_attribute(Node_attr, nodename, &fs_ptr->list);

	if (newtype != NILPTR(char))
		save_attribute(Type_attr, newtype, &fs_ptr->list);


	switch (rpmatch(accounting))
	{
		/* An affirmative response was received */
		case 1 :
		save_attribute(Acct_attr, True, &fs_ptr->list);
		break;

		/* A negative response was received */
		case 0 :
		save_attribute(Acct_attr, False, &fs_ptr->list);
		break;
	}

	switch (rpmatch(automnt))
	{
		/* An affirmative response was received */
		case 1 :
		save_attribute(Mount_attr, True, &fs_ptr->list);			
		break;

		/* A negative response was received */
		case 0 :
		save_attribute(Mount_attr, False, &fs_ptr->list);			
		break;
	}
}

/*
 *	file_type
 *
 *	- return type of file
 */

mode_t
file_type(char *device)
{
	struct stat stbuf;

	if (stat(device, &stbuf) < 0)
		return (0);

	return (stbuf.st_mode & S_IFMT);
}

/*
 *	isa_jfslog
 *
 *	- returns 1 if this device looks like a jfs log
 */

int
isa_jfslog(dev)
char *dev;
{
	int fd;
	char *type;
	struct logsuper sup;

	DPRINTF("isa_jfslog(\"%s\")", dev);

	/*
	 * see if lvm thinks this is a log
	 */
	if ((type = getlvfld(getdevname(dev), "TYPE:")) == NULL ||
	    !EQ(type, Logtype))
	{
		DPRINTF(" lvm doesn't think it's a log (%s)\n",
		 	 type ? type : "(null)");
		return (0);
	}

	/*
	 * try to open the device file
	 */
	if ((fd = open(dev, O_RDONLY)) >= 0)
	{
		lseek(fd, (off_t) ctob(LOGSUPER_B), SEEK_SET); 
		if (read(fd, (char *) &sup, (unsigned) sizeof(sup)) ==
			(int) sizeof(sup) && 
			(sup.magic == LOGMAGIC || sup.magic == LOGMAGICV4))
		{
			DPRINTF(" is a log...\n");
			close(fd);
			return (1);
		}
		close(fd);
	}

	DPRINTF("\n");

	return (0);
}

/*
 *	isa_jfs
 *
 *	- returns 1 if this device looks like a jfs
 */

int
isa_jfs (char *dev)
{
	int fd;
	int rc = 0;
	struct superblock sup;

	/*
	 * use libfs to open the device and check the super
	 */
	if ((fd = fsopen (dev, O_RDONLY)) >= 0)
	{
		if (read_super (fd, &sup, dev, 0) == 0)
			rc = 1;
		close (fd);
	}
	return (rc);
}

/*
 *	read_cmd
 *
 *	- format a command line and read a line of input from that
 *	  command
 */

char *
read_cmd(char *format, ...)
{
	FILE *fp;
	va_list args;
	char *p, cmd[BUFSIZ], buf[BUFSIZ];

	/*
	 * handle variable arguments...
	 */
	va_start(args, format);
	vsprintf(cmd, format, args);
	va_end(args);

	DPRINTF("read_cmd: \"%s\"\n", cmd);

	/*
	 * try to open a pipe to the command
	 */
	if ((fp = popen(cmd, "r")) != NULL)
	{
		p = fgets(buf, sizeof(buf), fp);
		pclose(fp);

		if (p != NULL)
		{
			/*
			 * zap trailing whitespace
			 */
			for (p = buf; *p != '\n' && *p != '\0'; p++)
				;
			for (--p; isspace(*p); p--)
				;
			*(p + 1) = '\0';

			return (strsave(buf));
		}
	}

	return (NULL);
}

/*
 *	being_used
 *
 *	return 1 if 'device' is already being used in a filesystem
 *	(either as the filesystem logical volume or log logical volume)
 */

int
being_used (char *device)
{
	struct fs_stanza *fs_ptr;

	for (fs_ptr = stanzalist; fs_ptr != NULL; fs_ptr = fs_ptr->next)
		if (contains_attribute(fs_ptr, Dev_attr, device) ||
		    contains_attribute(fs_ptr, Log_attr, device))
			break;

	if (fs_ptr == NULL)
		return (0);

	fprintf(stderr, MSGSTR(EXISTS,
		"%s: lv %s already being for filesystem %s\n"), progname,
		device, fs_ptr->name);

	return (1);
}


/*
 *	exportfs
 *
 *	- if 'lv' is non-zero, "pvname" is actually an lv.
 *	- do the exportfs stuff for physical volume "pvname"
 */

static int
exportfs (char 	*pvname,
	  int	lv)
{
	int	rc;

	DPRINTF("exportfs(\"%s\", %s)\n", pvname, lv ? "lv" : "pv");

	if (lv)
		rc = exportlv(makedevname (pvname));

	else
	{
		(void) process_lvs(pvname, exportlv, &rc, IS_PV);
		rc = 0;
	}
	return (rc);
}

/*
 *	exportlv
 *
 *	- do the export stuff for the logical volume 'lv'
 */

static int
exportlv (char *lv)
{
	struct fs_stanza *fs;

	DPRINTF("exportlv(\"%s\")\n", lv);

	if ((fs = stanza_lookup(lv, MATCH_DEVICE)) != NULL)
		remove_stanza(fs->name);

	return (0);
}

/*
 *	importfs
 *
 *	- do the importfs stuff for volume group 'vgname'
 *	- if lv is non-zero, vgname is actually a logical volume
 */

char *Vgname;

static int
importfs (char *vgname,
	  int lv)
{
	int importlv(), rc;

	DPRINTF("importfs(\"%s\")\n", vgname);

	if (lv)
	{
		Vgname = NULL;
		rc = importlv(makedevname(vgname));
	}

	else
	{
		Vgname = vgname;
		rc = 0;

		(void) process_lvs(vgname, importlv, &rc, IS_VG);
	}

	return (rc);
}


/*
 *	importlv
 *
 *	- do the import stuff for the logical volume 'lv'
 */
static int
importlv (char *lv)
{
	char *attrs;
	char *lvname = getdevname(lv);
	char *mountpoint = getlvlabel(lv);
	struct fs_stanza new, *fs = NULL;
	struct attribute *attr;
	struct attribute *log;
	char *logdev;

	DPRINTF("importlv(\"%s\"): mountpoint is %s\n", lvname,
		mountpoint ? mountpoint : "(null)");

	/*
	 * only import jfs logical volumes
	 */
	if (!isa_jfs(lv))
	{
		DPRINTF("\tskipped - not a jfs\n");
	}

	/*
	 * was there a filesystem label?
	 */
	else if (!mountpoint || !*mountpoint || EQ(mountpoint, "None"))
	{
		DPRINTF("\tskipped - no mountpoint\n");
	}
	
	/*
	 * does a stanza already exist?
	 */
	else if ((fs = stanza_lookup(mountpoint, MATCH_MOUNTPT)) ||
		 (fs = stanza_lookup(lv, MATCH_DEVICE)))
	{
		/* 
		 * does the mountpoint match & lv name match thea
		 * device name in the stanza?
		 */
		if (EQ(fs->name, mountpoint) &&
		    contains_attribute(fs, Dev_attr, lv))
		{
			DPRINTF("\tskipped - device/mountpoint matches\n");
		}
		else
			fprintf(stderr, MSGSTR(WRONGSTANZA,
		"%s: logical volume \"%s\" already exists in %s\n"),
			progname, lvname, FILESYSTEMS);
	}
	else
	{
		int save_lvcb = 0;

		attrs = getlvattrs(lv);

		DPRINTF("\tadding stanza for lv = %s / mnt = %s / attrs = %s\n",
			lv, mountpoint, attrs ? attrs : "(null)");

		new.name = mountpoint;
		new.list = lvattrs_to_attrlist(attrs);

		save_attribute (Dev_attr, makedevname(lv), &new.list);
		
		if (find_attribute(Vfs_attr, &new.list) == NULL)
			save_attribute(Vfs_attr, getjfsname(), &new.list);

		if ((log = find_attribute(Log_attr, &new.list)) == NULL ||
		    ! same_vg (lv, log->value))
		{
			DPRINTF("%s and %s arent in the same vg\n",
				log ? log->value : "(null log)", lv);

			if (Vgname == NULL)
				Vgname = getvgrp(lvname);
			if ((logdev = querylog(Vgname)) == NULL)
			{
				fprintf(stderr, MSGSTR(NOLOG,
				"%s: can't find log for volume group %s\n"),
					progname, Vgname);
				return (0);
			}
			save_attribute(Log_attr,makedevname(logdev),&new.list);
			save_lvcb++;
		}

		if (find_attribute(Check_attr, &new.list) == NULL)
			save_attribute(Check_attr, "false", &new.list);

		if (find_attribute(Mount_attr, &new.list) == NULL)
			save_attribute(Mount_attr, "false", &new.list);

		append_stanza(&new);

		/*
		 * make sure mount point (stub) exists
		 */
		makedirectory(mountpoint);
		
		if (save_lvcb)
			putlv(&new);

		free_attrlist(new.list);
	}

	return (0);
}

/*
 *	getlvlabel
 *
 *	- get the logical volume label (eg: the mountpoint)
 */

char *
getlvlabel (char *lv)
{
	return (read_cmd("%s -L %s", Getlvcb, getdevname(lv)));
}

/*
 *	putlvlabel
 *
 *	- set the logical volume label (eg: the mountpoint)
 */

int
putlvlabel (char *lv,
	    char *label)
{
	return (run_cmd("%s -L %s %s", Chlv, label, getdevname(lv)));
}

/*
 *	getlvattrs
 *
 *	- get the logical volume attrs
 */

char *
getlvattrs (char *lv)
{
	return (read_cmd("%s -f %s", Getlvcb, getdevname(lv)));
}

/*
 *	putlvattrs
 *
 *	- set the logical volume attrs
 */

static int
putlvattrs (char *lv,
	    char *attrs)
{
	return (run_cmd("%s -f %s %s", Putlvcb, attrs, getdevname(lv)));
}

/*
 *	putlv
 *
 *	used by: chfs, crfs
 *
 *	- put a stanza onto the lv
 */

int
putlv (struct fs_stanza *fs_ptr)
{
	struct attribute *a;
	char *dev = GET_ATTR(fs_ptr, Dev_attr);

	putlvlabel(dev, fs_ptr->name);
	putlvattrs(dev, attrlist_to_lvattrs(&fs_ptr->list)); 
}

/*
 *	lvattrs_to_attrlist
 *
 *	- take a list of attributes we got from an lv and convert
 *	  it to an attrlist
 */

struct attrlist
lvattrs_to_attrlist (char *attrs)
{
	char *attr;
	char *value;
	char *sep;
	struct attrlist attrlist;

	attrlist.count = 0;

	while (attr = strtok(attrs, "="))
	{
		attrs = NULL;
		if (value = strtok(attrs, Attrsep))
			save_attribute(attr, value, &attrlist);
	}

	return (attrlist);
}

/*
 *	attrlist_to_lvattrs
 *
 *	- take an attrlist and format a list of attributes for an lv
 */

char *
attrlist_to_lvattrs (struct attrlist *attrlist)
{
	char **o;
	char buf[BUFSIZ];
	static char lvattrs[LVATTRLEN + 1];
	struct attribute *a;
	/*
	 * the order of important attributes
	 */
	static char *order[] =
	{
		Vfs_attr, Log_attr, Mount_attr, Check_attr, Type_attr,
		Options_attr, NULL,
	};
	
	lvattrs[0] = '\0';

	/*
	 * clear out marked flags
	 */
	for (a = attrlist->attr; a < attrlist->attr + attrlist->count; a++)
		a->flag &= ~MARKIT;

	for (o = order; *o != NULL; o++)
		if ((a = find_attribute(*o, attrlist)) != NULL)
		{
			if (*a->value)
				SAVEATTR();
			a->flag |= MARKIT;
		}

	for (a = attrlist->attr; a < attrlist->attr + attrlist->count; a++)
		if (*a->value && !(a->flag & MARKIT))
			SAVEATTR();

	return (lvattrs);
}

/*
 *	free_attrlist
 *
 *	- free an attribute list
 */

void
free_attrlist (struct attrlist list)
{
	if (list.count > 0)
	{
		struct attribute *a;
	
		for (a = list.attr; a < list.attr + list.count; a++)
		{
			free(a->name);
			free(a->value);
		}
			
		free((char *) list.attr);
	}
}

/*
 *	same_vg
 *
 *	- returns non-zero if 'fslv' and 'loglv' are in the same volume group
 */
static int
same_vg	(char *fslv, char *loglv)
{
	struct stat fsstat, logstat;
	char *fspath = makedevname(fslv);
	char *logpath = makedevname(loglv);

	DPRINTF("are %s and %s in the same vg?\n", fspath, logpath);

	return (stat(fspath, &fsstat) == 0 && stat(logpath, &logstat) == 0 &&
		major(fsstat.st_rdev) == major(logstat.st_rdev));
}


/*
 *	absolute_and_canonize
 *
 *	- turn 'original' (which may be a relative pathname)
 *	  into an absolute pathname
 *	- canonize it as well
 */

char *
absolute_and_canonize (char *original)
{
	char *newpath, cwd[PATH_MAX + 1], *p, *c;

	/*
	 * build the absolute pathname...
	 */
	if (*original != '/')
	{
		if (getwd(cwd) == NULL)
			return (original);	/* punt */
		if ((newpath = malloc(strlen(cwd) + strlen(original) + 2)))
		{
			sprintf(newpath, "%s/%s", cwd, original);
			original = newpath;
		}
	}

	/*
	 * and canonize, one path component at a time.
	 *
	 * we'll use 'p' to walk ahead and look at the
	 * component, and we'll use 'c' to copy chars back.
	 */
	for (c = p = original; *p; )
	{
		/*
		 * skip redundant '/'
		 */
		while (*p == '/')
			p++;
		*c++ = '/';

		/*
		 * handle ".." and "./"
		 */
		if (*p == '.')
		{
			/*
			 * ..
			 */
			if (*++p == '.' && (*(p+1) == '/' || *(p+1) == '\0'))
			{
				/*
				 * back 'c' up over the last component
				 */
				if (--c > original)
					while (*--c != '/')
						;
				/*
				 * save the slash if we're at the end
				 */
				if (*++p == '\0')
					c++;
			}

			/*
			 * ./
			 */
			else if (*p == '/')
				c--;	/* back up over current '/' */

			else if (*p != '\0')
				p--;    /* no dot processing. 	*/
		}

		/*
		 * copy rest of component
		 */
		while (*p && *p != '/')
			*c++ = *p++;
	}

	/*
	 * don't end in '/', but allow "/"
	 */
	if (c > original + 1 && *(c - 1) == '/')
		c--;

	*c = '\0';

	DPRINTF("\tjust canonized to \"%s\"\n", original);

	return (original);
}


/*
 * NAME:        check_fsmax
 *
 * FUNCTION:	Read the filesystems superblock to calculate the
 *		maximum filesystem size in 512 byte blocks.
 *
 * PARAMETERS:	
 *	char	*fsname		- device name of the filesystem
 *
 * RETURNS: void
 *
 *	exit if size > maxfs size
 *
 */

static void
check_fsmax (char	*fs,
	     size_t	sz)
{
	struct superblock	sb;
	int			maxsz;
	int			fd;
	int			nbpi;
	    

	if ((fd = fsopen (fs, O_RDONLY)) < 0)
	{
		perror (fs);
		exit (10);
	}

	if (read_super (fd, &sb, fs, 1) != 0)
		exit (10);

        close (fd);

	nbpi = (sb.s_fragsize * sb.s_agsize) / sb.s_iagsize;
	maxsz = MIN (FS_ADDR_LIM (sb.s_fragsize), 
		     FS_NBPI_LIM (nbpi));

	if (sz > maxsz)
	{

		fprintf(stderr, MSGSTR (MAXFS_SIZE,
"%s:  Can not extend a JFS file system with nbpi = %d\nand frag = %d past %u (512 byte blocks).\n"),
			progname, nbpi, sb.s_fragsize, maxsz);
		exit (10);
	}
}


/*
 *
 *	NAME:	read_super
 *
 *	FUNCTION:
 *		Read the super block using the fslib routines.
 *		Process the return codes of get_super.
 *
 *	RETURN:
 *		-1 if couldn't read the super block
 *		0  if superblock is ok
 *
 */

static int
read_super (int 		fd,
	    struct superblock 	*sb,
	    char		*fsname,
	    int			verbose)
{
        int	rc;
	char	*msg;

	msg 	= NULL;
	rc 	= - 1;

	switch (get_super (fd, sb))
	{
		case LIBFS_SUCCESS:
	        rc = 0;
		break;

		case LIBFS_BADMAGIC:
		msg = MSGSTR (NOTJFS,
		        "%s:  %s is not recognized as a JFS filesystem.\n");
		break;

		case LIBFS_BADVERSION:
		msg = MSGSTR (NOTSUPJFS,
	  		"%s:  %s is not a supported JFS filesystem version.\n");
		break;

		case LIBFS_CORRUPTSUPER:
		msg = MSGSTR (CORRUPTJFS,
	        "%s:  The %s JFS filesystem superblock is corrupted.\n");
		break;		

		
		default:
		msg = MSGSTR(CANTRESB, 
			"%s:  Cannot read superblock on %s.\n");
		break;
	}

	if (verbose && msg != NULL)
	    fprintf (stderr, msg, progname, fsname);


	return (rc);
}

