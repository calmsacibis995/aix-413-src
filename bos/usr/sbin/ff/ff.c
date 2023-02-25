static char sccsid [] = "@(#)56	1.12.2.1  src/bos/usr/sbin/ff/ff.c, cmdfs, bos411, 9428A410j 1/10/94 17:58:51";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: ff
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
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <IN/AFdefs.h>
#include <IN/CSdefs.h>
#include <fshelp.h>
#include <sys/vfs.h>

#include <ctype.h>
#include <sys/types.h>
#include <locale.h>

#include <nl_types.h>
#include "ff_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_FF,Num,Str)

#include "fsop.h"

/*
	Formatting options for printout
*/
#define	F_INODE		(1<<0)
#define	F_SIZE		(1<<1)
#define	F_UID		(1<<2)
#define	F_PREPATH	(1<<3)

/*
 *	Attribute file gibberish
 */
#define FILESYSTEMS	"/etc/filesystems"
#define MAXATR		4096
#define MAXREC		200

/*
	Run options
*/
#define	F_REQUEST	(1<<0)
#define	F_LINKSTOO	(1<<1)
#define	F_DEBUG		(1<<2)
#define F_MALLOC	(1<<3) /* NOT USED */
#define F_IGSPECIAL	(1<<4)

/*
 * global variable declarations 
 */
int fsh_debug_level;		/* fshelper debug level */
int fsh_mode_flags;		/* fshelper mode flags */
char fsh_opflags[4096];		/* option flags for helper */
int prtopt, runopt;		/* program's run-options and print-options */
char *g_cmd, *g_dev, *g_errf;	/* various and sundry strings */

/* 
 * external variable declarations 
 */
extern	int	optind, opterr;	/* for getopt... */
extern	char	*optarg;			/* for getopt... */

/* 
 * function declarations 
 */
char *resolve_device();		/* change filesystem name to device name */
char *inodes();			/* change comma delimited str to vector str */
extern int getopt();		/* parse command line options */

/*
 * NAME:	main
 *
 * FUNCTION:	main - entry point of ff
 *
 * NOTES:	Main parses arguments on the command line,
 *		makes an attempt to resolve the device name of the filesystem
 *		we want to do a find on, and then dispatches to the 
 *		helper routine FSHOP_NAMEI.
 *
 *		This should not change from helper-version to helper-version
 */
main(argc,argv)
int    argc;
char   **argv;
{
    int rc;
    int c;
    struct stat statb;
    char *cp;
    char *vfsname = NULL;
    char opdata[1024];  /* where the data comes back thru */
    char opflags[2048]; /* file system helper fs flags	*/

    (void) setlocale (LC_ALL,"");
    catd = catopen((char *)MF_FF, NL_CAT_LOCALE);
    g_cmd = argv[0];
    g_dev = "";

    /*
     * init fshelper flags
     */
    fsh_mode_flags = FSHMOD_INTERACT | FSHMOD_FORCE | FSHMOD_PERROR;
    fsh_debug_level = FSHBUG_NONE;
    fsh_opflags[0] = '\0';

    /*
     * check command line...
     */
    prtopt |= F_INODE;
    runopt = 0;
    runopt |= F_IGSPECIAL;	/* always assert this, since special files
				 * no longer have entries in the directory
				 */

    while((c=getopt(argc,argv,"3IsulV:a:m:c:n:i:p:dD:o:z:Z:")) != EOF) {
	switch(c) {
	case '3':	/* ignore special . files on v3 filesystems */
	    runopt |= F_IGSPECIAL; continue;
	case 'I':	/* don't print inode #'s */
	    prtopt &= ~F_INODE; continue;
	case 's':	/* do print size of file */
	    prtopt |= F_SIZE; continue;
	case 'u':	/* print uids */
	    prtopt |= F_UID; continue;
	case 'l':	/* print links also */
	    runopt |= F_LINKSTOO; continue;
	case 'V':	/* specify type of filesystem */
	    vfsname = optarg; continue;	

	case 'a':			/* access time */
	    strcat(fsh_opflags, "atime=");
	    if (atoi(optarg) == 0) 
		errorx(MSGSTR(BADVALUE,"bad value %s for option %c\n"),
			optarg,'a');
	    strcat(fsh_opflags, optarg);
	    break;

	case 'm':			/* modification time */
	    strcat(fsh_opflags, "mtime=");
	    if (atoi(optarg) == 0) 
		errorx(MSGSTR(BADVALUE,"bad value %s for option %c\n"),
			optarg,'m');
	    strcat(fsh_opflags, optarg);
	    break;

	case 'c':			/* creation time */
	    strcat(fsh_opflags, "crtime=");
	    if (atoi(optarg) == 0) 
		errorx(MSGSTR(BADVALUE,"bad value %s for option %c\n"),
			optarg,'c');
	    strcat(fsh_opflags, optarg);
	    break;

	case 'n':			/* device time */
	    strcat(fsh_opflags, "ntime=");
	    if (stat(optarg,&statb) < 0) 
		errorx(MSGSTR(STATFAIL,"n option, %s stat failed\n"),optarg);

	    strcat(fsh_opflags, optarg);
	    break;

	case 'i':			/* only certain inodes */
	    runopt |= F_REQUEST;
	    strcat(fsh_opflags,"inodes=");
	    if ((cp = (char *)inodes(optarg)) == 0)
		errorx(MSGSTR(BADVALUE,"bad value %s for option %c\n"),
			optarg,'i');
	    strcat(fsh_opflags,cp);
	    break;
	case 'p':			/* prepend a path name */
	    prtopt |= F_PREPATH;
	    strcat(fsh_opflags,"prefix=");
	    strcat(fsh_opflags,optarg);
	    break;

	case 'd':       /* debug operation       */
	    fsh_debug_level++;
	    runopt |= F_DEBUG;
	    break;
	case 'D':       /* set debug level */
	    fsh_debug_level = atoi(optarg);
	    runopt |= F_DEBUG;
	    break;

	case 'o':	/* fs implementation-specific options	*/
	    strcat(fsh_opflags, optarg);
	    break;

	case '?':	/* error */
	default:
	    usage();
	}
	strcat(fsh_opflags, ",");
    }

    /*
     * remove trailing comma ...
     */
    if ((cp = strrchr(fsh_opflags, (int)',')) != NULL)
	*cp = '\0';

    /*
     * if we don't have any arguments left, we're in trouble. 
     * tell the user he's a dolt and exit.
     */
    if (argv[optind] == 0) {
	usage();

    }

    /*
     * try to make sense out of the filesystem name that was on the command
     * line.  
     */
    g_dev = resolve_device(argv[optind],(vfsname)?((char **)0):(&vfsname));

    /*
     * setup to call the helper...
     */
    sprintf(opflags,"device=%s,prtopt=%d,runopt=%d",g_dev,prtopt,runopt);

    if (fsh_opflags[0] != '\0') {
	    strcat(opflags, ",");
	    strcat(opflags, fsh_opflags);
    }

    opdata[0] = '\0';

    /* 
     * call the fshelper.  do some nasty things with fork'in and exec'in
     */
    /* fprintf(stderr,"calling fshelp(\"%s\",\"%s\",%d,%x,%x,\"%s\",\"%s\")\n",
     * g_dev,vfsname,FSHOP_NAMEI,fsh_mode_flags,fsh_debug_level,opflags,opdata);
     */
    rc = fshelp(g_dev, vfsname, FSHOP_NAMEI, fsh_mode_flags,
		    fsh_debug_level, opflags, opdata);	

    /* 
     * if fshelp returns good, exit happy else, exit sad. 
     */
    exit((rc == FSHRC_GOOD) ? atoi(opdata) : 1);
}

/* 
 * resolve device checks to see if dev is a block or character device.
 * if it is, return dev.  if not, try to see what filesystem we are really
 * talking about.
 */
char *resolve_device(stanza_name,vfs)
char *stanza_name;
char **vfs;
{
    AFILE_t af;
    ATTR_t  ar;
    char *retval, *rawdev, *rawname();
    char *atr;
    struct stat statb;
    char *unraw();
    char *cp;

    if (runopt &F_DEBUG) error("resolve_device: stanza_name = `%s'\n",
	stanza_name);
    if(!(af = (AFILE_t) AFopen(FILESYSTEMS,MAXREC,MAXATR))
	||!(ar = (ATTR_t) AFgetrec(af,stanza_name))
	||!(retval = (char *)AFgetatr(ar,"dev"))) {
	    if (vfs) {

		AFclose(af);		/* we'll re-open this in a minute */
		/* 
		 * try to get the vfs type of a device by looking in the 
		 * /etc/filesystems file.
		 *
		 * be sure we have the block device since that's what's always
		 * in /etc/filesystems...
		 */
		if ((retval = unraw(stanza_name)) == NULL)
			retval = stanza_name;

		/*
		 * try to open the filesystems file
		 */
		if( (af = AFopen(FILESYSTEMS, MAXREC, MAXATR)) ) {
		    /*
		     * search for this device
		     */
		    while((ar = AFnxtrec(af)) &&
			  (!(atr = AFgetatr(ar,"dev"))||strcmp(atr,retval)!=0))
			    ;

		    /*
		     * if we found the entry in the filesystems file, and it
		     * has a "vfs" attribute, use it
		     */
		    if (ar && atr && (atr = AFgetatr(ar, "vfs")))
			    *vfs = atr;
		    else
			    *vfs= NULL;

	    }
	} else retval = stanza_name;
    } else if (ar && vfs) {
	cp = (char *)AFgetatr(ar,"vfs");
	if (runopt &F_DEBUG) fprintf(stderr,"vfsname = %s\n",cp);
	*vfs = (char *)strcpy((char *)malloc((unsigned)strlen(cp)+1),cp);
    } 
    cp = (char *)strcpy((char *)malloc((unsigned)strlen(retval)+1),retval);
    retval = cp;
    if (runopt &F_DEBUG) error("resolve_device: retval= `%s'\n",retval);
    (void) sync();
    
    if (stat(retval,&statb) < 0)
	errorx(MSGSTR(NOINPUT,"Input %s is non-existent\n"),retval);

    if((((statb.st_mode&S_IFMT)==S_IFBLK)|((statb.st_mode&S_IFMT)==S_IFCHR))==0)
	errorx(MSGSTR(NOTSPECIAL,"Input device %s is not a special file\n"), retval);

    if ((statb.st_mode&S_IFMT) == S_IFBLK)
    	rawdev = rawname(retval);
    AFclose(af);
    return(rawdev);
}

char *
rawname(cp)
char *cp;
{
	static char rawbuf[32];
	char *dp = rindex(cp, '/');

	if (dp == NULL)
		return (NULL);
	*dp = '\0';
	strcpy(rawbuf, cp);
	*dp = '/';
	strcat(rawbuf, "/r");
	strcat(rawbuf, dp+1);
	return(rawbuf);
}


/* 
 * NAME: inodes;  turn a comma demilited list of numbers into a FSH_VECTORDELIM
 * 		  seperated list.
 *
 */
char *inodes(args)
char *args;
{
    char *p;
    int	n, atoi();
    int	first = 0;
    char *cp;
    static char buf[] = { FSH_VECTORDELIM, '\0' };
    unsigned int len;

    cp = NULL;

    while ((p=strtok((first++==0?args:0)," ,")) != NULL) {
	if ((n=atoi(p)) <= 0)
	    error(MSGSTR(BADIVAL,"-i value, %d, ignored\n"),n);

	else {
	    len = strlen(p) + strlen(buf) + 1 + ((cp) ? strlen(cp) : 0);

	    /* fprintf(stderr,"tacking on '%s' onto string = '%s' (%d)\n",p, cp?cp:"(null)",len); */

	    if (cp == NULL) {
	    	cp = (char *)malloc(len);
		*cp = 0;
	    }
	    else
	    	cp = (char *)realloc(cp, len);
	    strcat(cp,p); strcat(cp,buf);
	}
    }
    if (p = rindex(cp,FSH_VECTORDELIM)) *p = 0;
    /* fprintf(stderr,"returning '%s'\n",cp); -DBG */
    return(cp);
}

/* 
 * error, errorx, usage:  error reporting routines, 
 */
error(s,  e,z,  g,o,l,d,m,a,n)
{
    fprintf(stderr,"%s: %s: ", g_cmd, g_dev);
    fprintf(stderr, s,  e,z,  g,o,l,d,m,a,n);
}

errorx(s,  e,z,  g,o,l,d,m,a,n)
{
    error(s,  e,z,  g,o,l,d,m,a,n);
    exit(1);
}

usage()
{
    errorx(MSGSTR(USAGE,"Usage: %s [-3MIldsu -V vfstype -i ilist -p path -n file -a # -m # -c #] /inputdev\n"),g_cmd);
}

/*
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
