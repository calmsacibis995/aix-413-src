static char sccsid[] = "@(#)03	1.22.2.1  src/bos/usr/sbin/mkfs/mkfs.c, cmdfs, bos411, 9428A410j 1/10/94 17:59:49";
/*
 * COMPONENT_NAME: (CMDFS) commands that manipulate files
 *
 * FUNCTIONS: mkfs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
**
** AIXv3 mkfs
**
**   Create and format a filesystem using filesystem helper operation
**
*/

#include <stdio.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <errno.h>

#include <sys/vmount.h>
#include <sys/vfs.h>
#include <sys/syspest.h>

#include <ctype.h>
#include <sys/types.h>

#include <locale.h>
#include <langinfo.h>

#include <nl_types.h>
#include "mkfs_msg.h"

#include <fshelp.h>

/*
** Exit / Message codes are defined in mkfs_msg.h iff MSG defined
*/

#define	Exit_OK		  0

static nl_catd	catd;

#define ERR_MSG(num,str) catgets (catd, MS_MKFS_ERR, num, str)
#define MISC_MSG(num,str) catgets (catd, MS_MKFS_MISC, num, str)
    
#define NILSTR		""
#define NIL(type)	((type) 0)
#define NILPTR(type)	((type *) 0)

#define MAX(a,b)	(a > b? a: b)

#ifdef DEBUG
BUGVDEF (DebugLevel, 0)
#endif

static char *ErrMsgs[] =
{
  NILSTR,
  "Usage error: mkfs [-b boot] [-i inodes] [-l label] [-o options]\n\t\t  [-p proto] [-s size] [-v volabel] [-V vfs] <device> | <fsname>",
  "Illegal size was specified.",
  "Illegal number of inodes was specified.",
  "No vfs type was specified.",
  "Filesystem helper error.",
  "Malloc failed.",
  "String functions failed.",
  "Internal error.",
  "Unknown vfs type was specified.",
  "Filesystem is currently mounted.",
  "Both the \"copyboot\" and \"labelit\" options may not be specified.",
  "No boot program was specified.",
  "No label or volume label was specified."
};

static char *ProgName = NILPTR (char);
/*
** used by getopt()
*/
extern int   optind;
extern char *optarg;

/*
** old_fs's are an interesting subset of filesystems
** a working def'n of "interesting" is in found_any()
*/

struct old_fs
{
  char	boot[PATH_MAX];		/* boot program	*/
  char	dev[PATH_MAX];		/* device path	*/
  char	name[PATH_MAX];		/* mount point	*/
  char	size[32];		/* no. of UBSIZE byte blocks */
  char	vol[PATH_MAX];		/* volume label	*/ 
  char	vfs[32];		/* vfs type	*/
  char  inodes[32];		/* no. of inodes initially allocated */

  struct old_fs  *next;
};

static struct old_fs    *OldFsList;		/* head of old_fs list */

/*
** actual values used
** set by parse_args or if not set explicitly, then in add_defaults
*/
static char           *Boot     = NILPTR (char);	/* boot program	    */
static char           *Dev      = NILPTR (char);	/* device name	    */
static char           *Name     = NILPTR (char);	/* mount point      */
static char           *Label    = NILPTR (char);	/* fs. label	    */
static char           *Proto    = NILPTR (char);	/* prototype file   */
static char           *Vol      = NILPTR (char);	/* volume label	    */
static char           *Vfs      = NILPTR (char);	/* vfs type	    */
static long   	       Size     = 0;			/* # blocks	    */
static long	       Inodes   = 0;			/* # initial inodes */
static int             CopyBoot = 0;			/* just copy boot   */
static int             LabelIt  = 0;			/* just chg labels  */

static char           *Opts  = NILPTR (char);	/* options to helper */

typedef enum
{
  NIL_T = 0, LONG_T, STR_T
} arg_t;

#define	NAME_BIT	0x01
#define	LABEL_BIT	0x02
#define	BOOT_BIT	0x04
#define	PROTO_BIT	0x08
#define	SIZE_BIT	0x10
#define	VOL_BIT		0x20
#define DEV_BIT		0x40
#define INODE_BIT	0x80

static struct opt
{
  char      *name;	/* attribute name */
  int        bit;	/* internal flag  */
  caddr_t    ptr;	/* ptr to value   */
  arg_t      type;	/* type of value  */
} opts[] =
{
     "name",       NAME_BIT,	NIL (caddr_t),	STR_T,
     "label",      LABEL_BIT,	NIL (caddr_t),	STR_T,
     "boot",       BOOT_BIT,	NIL (caddr_t),	STR_T,
     "proto",      PROTO_BIT,	NIL (caddr_t),	STR_T,
     "size",       SIZE_BIT,	NIL (caddr_t), 	LONG_T,
     "vol",        VOL_BIT,	NIL (caddr_t),	STR_T,
     "dev",	   DEV_BIT,	NIL (caddr_t),  STR_T,
     "inodes",	   INODE_BIT,	NIL (caddr_t),  LONG_T,
      NILPTR (char),  0,	NIL (caddr_t),	NIL_T
};
/* ** forward refs */
static void init_opt();
#ifdef DEBUG
static void dump_old_fs();
static void dump_opt_defaults();
#endif /* DEBUG */
static struct old_fs *get_old_fs_dev();
static struct old_fs *get_old_fs();
static void cleanup();
static int poschr();
static int check_for();
static int check_mounted();
static int parse_args();
static int add_defaults();
static int makeFStab();
static int call_helper();
static int yesno();
char *basename();

int catalog;	/* Boollean flag to specify if the catalog
		** exists or not */

main (ac, av)
     int ac;
     char **av;
{
  int             rc;
  char            ans[20];
  char            strbuf[BUFSIZ];
  char 		 	prompt[10];
  
  (void) setlocale (LC_ALL,"");

  catd = catopen (MF_MKFS, NL_CAT_LOCALE);
  
  ProgName = basename (av[0]);

  if (rc = makeFStab (FSfile))
      cleanup (rc, TRUE);

  if (rc = parse_args (ac, av))
      cleanup (rc, TRUE);

  if (rc = check_mounted())
      cleanup (rc, TRUE);

  if (rc = add_defaults())
      cleanup (rc, TRUE);

  if (isatty (0))
  {
    int devnull  = Dev == NILPTR (char) || strcmp (Dev, NILSTR) == 0; 
    int namenull = Name == NILPTR (char) || strcmp (Name, NILSTR) == 0; 

	/* Determine if the catalog is there or not by testing if
	** a default is returned when a message that exists in the
	** catalog is obtained via catgets().
	*/ 
	{
		char *p= "relabel";
		char *q ;
		q = catgets (catd, MS_MKFS_MISC,Misc_RELABEL, p);
		if ( q == p)
			catalog = FALSE;
		else
			catalog = TRUE ;

	}

    if (catalog == FALSE){ 
		/* use English if current message catalog does not exist*/
	*prompt = 'y';
	prompt[1]= '\0';
    }else
	strcpy(prompt, nl_langinfo(YESSTR));
      
    do
    {
      memset ((void *)strbuf, 0, (size_t)(sizeof( strbuf)));
      if (!namenull)
         sprintf (strbuf, "[%s] ", Name);
      fprintf (stderr, "%s: %s %s %s(%s)? ", ProgName,
	      LabelIt? MISC_MSG (Misc_RELABEL, "relabel"):
	      CopyBoot? MISC_MSG (Misc_REPLACEBOOT, "replace boot program on"):
	      		MISC_MSG (Misc_DESTROY, "destroy"),
	       devnull? MISC_MSG (Misc_IT, "it"): Dev,
	       strbuf, prompt);
      read(0, ans, sizeof(ans));
    }
    while (yesno (ans) == -1);
    if (yesno (ans) == FALSE)
	cleanup (Exit_OK, FALSE);	/* wimp out */
  }

  if (rc = call_helper())
      cleanup (rc, FALSE);

  cleanup (Exit_OK, FALSE);
#ifdef lint
  return Exit_OK;
#endif  
}

static int
parse_args (ac, av)
     int             ac;
     char          **av;
{
     struct old_fs   *ofspfs;
     struct old_fs   *ofspdev;
     int              opt;
     int              rc;

     while ((opt = getopt (ac, av, "b:i:l:o:p:s:v:D:V:")) != EOF)
     {
       switch (opt)
       {
       case 'b':		/* boot program */
	   Boot = optarg;
	   init_opt (BOOT_BIT, (caddr_t) Boot);
	   break;

       case 'i':
	   Inodes = atol (optarg);
           if (Inodes > 0)
	   	init_opt (INODE_BIT, &Inodes);
           else
		return ErrExit_BADINODES;
	   break;

	   
       case 'l':		/* filesystem label */
	   Label = optarg;
	   init_opt (LABEL_BIT, (caddr_t) Label);
	   break;
	   
       case 'o':		/* implementation-specific options */
	   Opts = optarg;
	   break;

       case 'p':		/* proto file */
	   Proto = optarg;
	   init_opt (PROTO_BIT, (caddr_t) Proto);
	   break;
	   
       case 's':		/* size (number of UBSIZE blocks) */
	   Size = atol (optarg);
           if (Size > 0)
	   	init_opt (SIZE_BIT, &Size);
           else
		return ErrExit_BADSIZE;
	   break;
	 
       case 'v':		/* volume label */
	   Vol = optarg;
	   init_opt (VOL_BIT, (caddr_t) Vol);
	   break;
	   
        case 'V':		/* vfs type */
	   if (getvfsbyname (optarg))
	     Vfs = optarg;
           else
	     return ErrExit_BADVFS;
	   break;
           
#ifdef DEBUG
       case 'D':		/* debug level */
           DebugLevel = atoi (optarg);
           break;
#endif /* DEBUG	*/
           
       case '?':
           return ErrExit_USAGE;
	   /*NOTREACHED*/
           break;
       }
     }
     
     /*
     ** device?
     */
     if (optind != ac-1)
	 return ErrExit_USAGE;
     Dev = av[optind];

     /*
     ** check for fs rather than device
     */
     if ((ofspfs = get_old_fs (Dev)) != NILPTR (struct old_fs) &&
	 strcmp (ofspfs->dev, NILSTR) != 0)
       Dev = ofspfs->dev;

     /*
     ** get its vfs type, if not yet specified
     */
     if (Vfs == NILPTR (char))
     {
       /*
       ** if can't find stanza for dev or fsname or
       ** can find it but can't find a vfs type for either one of them ==> lose
       ** else vfs is whichever was found
       */
       if ( (ofspdev = get_old_fs_dev (Dev)) == NILPTR (struct old_fs) ||
	    strcmp (ofspdev->vfs, NILSTR) == 0)
	 if ( (ofspfs = get_old_fs (Dev)) == NILPTR (struct old_fs) ||
	      strcmp (ofspfs->vfs, NILSTR) == 0)
		return ErrExit_NOVFS;
	 else
              Vfs = ofspfs->vfs;
       else
	   Vfs = ofspdev->vfs;
     }
  
     if ((rc = check_for ("copyboot", &CopyBoot)) ||
          (rc = check_for ("labelit", &LabelIt)))
	    return rc;

     if (CopyBoot && LabelIt)
	    return ErrExit_ARGCONF;

     if (CopyBoot && (Boot == NILPTR (char) || strcmp (Boot, NILSTR) == 0))
	    return ErrExit_NOBOOT;
  
     if (LabelIt && (Label == NILPTR (char) || strcmp (Label, NILSTR) == 0) &&
	            (Vol == NILPTR (char) || strcmp (Vol, NILSTR) == 0))
	 return ErrExit_NOLABEL;

     return Exit_OK;
}

static void
init_opt (bit, ptr)
     int      bit;
     caddr_t  ptr;
{
  struct opt *optr;

  for (optr = opts; optr->name; optr++)
    if (optr->bit & bit)
    {
      optr->ptr = ptr;
      return;
    }
  return;
}


/*
** check_mounted
**
*/

static int
check_mounted()
{
  struct vmount  *vmtbuf   = NILPTR (struct vmount);
  int             nmounted = 0;
  int             size     = 0;
  struct vmount  *vmtp;
  register        i;
  
  do
  {
    size = size == 0? BUFSIZ: vmtbuf->vmt_revision;

    if (vmtbuf)
	free ((void *)vmtbuf);
    
    if ((vmtbuf = (struct vmount *) malloc (size)) == NILPTR (struct vmount))
	return ErrExit_NOMEM;
    
    nmounted = mntctl (MCTL_QUERY, size, vmtbuf);

    if (nmounted > 0 && vmtbuf->vmt_revision != VMT_REVISION ||
	nmounted < 0)
    {
      if (!errno)
	  errno = ESTALE;
      return ErrExit_INTERNAL;
    }
  }
  while (!nmounted);

  for (i = 0, vmtp = vmtbuf;
       i < nmounted;
       i++, vmtp = (struct vmount *) ((int) vmtp + vmtp->vmt_length))
  {
    if (strcmp (vmt2dataptr (vmtp, VMT_OBJECT), Dev) == 0)
    {
      if (vmtbuf)
	  free ((void *)vmtbuf);
      return ErrExit_MOUNTED;
    }
  }
  if (vmtbuf)
      free ((void *)vmtbuf);
  return Exit_OK;
}
    
  
/*
** add_defaults
**  check for defaults specified in FSfile
** "name", "vol", etc.
** build a string to be added to options, provided
** it isn't already in it.
**
** This is really just a nicety for the helpers so they
** don't have to parse a stanza file.  If any additional
** attributes are necessary, this is (at best) redundant.
*/


static int
add_defaults ()
{
  struct old_fs *ofsp;
  struct opt    *optr; 
  char          *tokp;
  char          *optbuf  = NILPTR (char);
  char          *newopts = NILPTR (char);
  short          add     = 0;
  int            len     = 0;
  char           intbuf[32];

  /*
  ** Set up defaults  
  */
  init_opt (DEV_BIT, (caddr_t) Dev);
  if ((ofsp = get_old_fs_dev (Dev)) != NILPTR (struct old_fs))
  {
    if (Name == NILPTR (char) && strcmp (ofsp->name, NILSTR) != 0)
    {
      Name = ofsp->name;
      init_opt (NAME_BIT, (caddr_t) Name);
    }
    if ((Label == NILPTR (char) || strcmp (Label, NILSTR) == 0) &&
		strcmp (Name, NILSTR) != 0)
    {
      Label = Name;
      init_opt (LABEL_BIT, (caddr_t) Label);
    }
    if (Boot == NILPTR (char) && strcmp (ofsp->boot, NILSTR) != 0)
    {
      Boot = ofsp->boot;
      init_opt (BOOT_BIT, (caddr_t) Boot);
    }
    if (Size == 0 && strcmp (ofsp->size, NILSTR) != 0)
    {
      Size = atol (ofsp->size);
      if (Size > 0)
      	init_opt (SIZE_BIT, &Size);
      else
	Size = 0;
    }
    if (Vol == NILPTR (char) && strcmp (ofsp->vol, NILSTR) != 0)
    {
      Vol = ofsp->vol;
      init_opt (VOL_BIT, (caddr_t) Vol);
    }
  }
  /*
  ** build mask of valid options
  **
  ** if ptr to stored area is unset or value is zero or <null>, ==> not valid
  */
  for (optr = opts; optr->name; optr++)
    if (optr->ptr != NIL (caddr_t))
	switch (optr->type)
	{
	case LONG_T:
	  if ( * (int *) (optr->ptr) != 0)
	    add |= optr->bit;
	  break;
	case STR_T:
	  if (strcmp ((char *) optr->ptr, NILSTR) != 0)
	    add |= optr->bit;
	  break;
	default:
	  return ErrExit_INTERNAL;
	}
  /*
  ** check for overriding command line specifications
  */
  if (Opts != NILPTR (char))
  {
    /*
    ** which don't need to be added (ie, are already specified)?
    */
    len = strlen (Opts) + 1;
    if ((optbuf = (char *) malloc (len)) == NILPTR (char))
	return ErrExit_NOMEM;
    
    /*
    ** mess with optbuf because strtok modifies its argument
    */
    (void) strcpy (optbuf, Opts);
    for (tokp = strtok (optbuf, ",");
	  tokp;
	   tokp = strtok (NILPTR (char), ","))
    {
      /*
      ** either use equal sign or string length as length of name
      */
      if ((len = poschr (tokp, '=')) == 0 && (len = strlen (tokp)) == 0)
	  continue;
      for (optr = opts; optr->name; optr++)
	  if (strncmp (tokp, optr->name, MAX (strlen (optr->name), len)) == 0)
	      add &= ~optr->bit;
    }
    free ((void *)optbuf);
    optbuf = NILPTR (char);
  }
  
  /*
  ** don't bother if everything specified explicitly
  */
  if (!add)
    return Exit_OK;
  /*
  ** build additional options in optbuf
  */
  for (optr = opts; optr->name; optr++)
  {
    if (!(add & optr->bit))
	continue;
    /*
    ** call strlen("=") for NLS/KJI support 
    ** strlen (name) + strlen ("=") + strlen (value) + terminating null
    */
    len = strlen (optr->name) + strlen ("=") + 1;
    switch (optr->type)
    {
    case STR_T:
      len += strlen (optr->ptr);
      break;
    case LONG_T:
      memset ((void *)intbuf, 0, (size_t)(sizeof(intbuf)));
      sprintf (intbuf, "%d", * (long *) optr->ptr);
      len += strlen (intbuf);
      break;
    default:
      return ErrExit_INTERNAL;
    }

    if (optbuf == NILPTR (char))
    {
      if ((optbuf = (char *) malloc (len)) == NILPTR (char))
	  return ErrExit_NOMEM;
      memset ((void *)optbuf, 0, (size_t)len);
    }
    else
    {
      len += strlen (optbuf) + 1;
      if ((optbuf = (char *) realloc (optbuf, len)) == NILPTR (char))
	  return ErrExit_NOMEM;
      if (sprintf (optbuf, "%s,", optbuf) <= 0)
	  return ErrExit_STRFAIL;
    }
    if (sprintf (optbuf, "%s%s=", optbuf, optr->name) <= 0)
	return ErrExit_STRFAIL;
    
    switch (optr->type)
    {
    case STR_T:
    	if (sprintf (optbuf, "%s%s", optbuf, optr->ptr) <= 0)
	   return ErrExit_STRFAIL;
	break;
    case LONG_T:
	if (sprintf (optbuf, "%s%d", optbuf, * (long *) optr->ptr) <= 0)
	  return ErrExit_STRFAIL;
        break;
    }
  }

  /*
  ** string = [Opts,]optbuf\0
  */
  if (Opts == NILPTR (char))
    Opts = optbuf;
  else
  {
    len = strlen (Opts) + strlen (optbuf) + 2;
    if ((newopts = (char *) malloc (len)) == NILPTR (char))
	return ErrExit_NOMEM;
    if (sprintf (newopts, "%s,%s", Opts, optbuf) <= 0)
	return ErrExit_STRFAIL;
    Opts = newopts;
  }
  return Exit_OK;
}

/*
** check_for
**
**  if matches on optname, sets found
*/
static int
check_for (optname, found)
     char *optname;
     int  *found;
{
  int    len;
  char  *optbuf;
  char  *tokp;
  
  if (Opts != NILPTR (char))
  {
    len = strlen (Opts) + 1;
    if ((optbuf = (char *) malloc (len)) == NILPTR (char))
	return ErrExit_NOMEM;
    memset ((void *)optbuf, 0, (size_t)len);
    /*
    ** mess with optbuf because strtok modifies its argument
    */
    (void) strcpy (optbuf, Opts);
    for (tokp = strtok (optbuf, ",");
	  tokp;
	   tokp = strtok (NILPTR (char), ","))
    {
      /*
      ** either use equal sign position or string length as length of name
      */
      if ((len = poschr (tokp, '=')) == 0 && (len = strlen (tokp)) == 0)
      {
	  continue;
      }
      if (strncmp (tokp, optname, MAX (strlen (optname), len)) == 0)
      {
	*found = 1;
	break;
      }
    }
    if (optbuf)
	free ((void *)optbuf);
  }
  return Exit_OK;
}
    
    
/*
** found_any()
**  
** hard coding of attributes here
** These are about as generic as you can get for
** creating a filesystem though.
**   "dev"     - raw device       (ie. "/dev/hd3")
**   "vol"     - volume id,       (ie. "Pack3")
**   "name"    - filesystem name  (ie. "/tmp")
**   "vfs"     - vfs type         (ie. "jfs")
*/

static int
found_any (stanza, ofsp)
     ATTR_t  stanza;
     struct old_fs *ofsp;
{
  char    *attrval;
  int      found = 0;

  if (attrval = AFgetatr (stanza, "dev"))
  {
    strncpy (ofsp->dev, attrval, strlen (attrval));
    found++;
  }
  if (attrval = AFgetatr (stanza, "vol"))
  {
    strncpy (ofsp->vol, attrval, strlen (attrval));
    found++;
  }
  if (attrval = AFgetatr (stanza, "vfs"))
  {
    strncpy (ofsp->vfs, attrval, strlen (attrval));
    found++;
  }
  if (attrval = AFgetatr (stanza, "size"))
  {
    strncpy (ofsp->size, attrval, strlen (attrval));
    found++;
  }
  if (attrval = AFgetatr (stanza, "boot"))
  {
    strncpy (ofsp->boot, attrval, strlen (attrval));
    found++;
  }

  /*
  ** only worth tabling if it has relevant keywords in it
  */
  if (found)
      strncpy (ofsp->name, stanza->AT_value, strlen (stanza->AT_value));
  
  return found;
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
      return Exit_OK;

  if (!(attrfile = AFopen (filesystems, MAXREC, MAXATR)))
      return ErrExit_NOMEM;

  while (stanza = AFnxtrec (attrfile))
  {
    if (!(ofsp = (struct old_fs *) malloc (sizeof (struct old_fs))))
	return ErrExit_NOMEM;
    memset ((void *)ofsp, 0, (size_t)(sizeof (struct old_fs)));
    
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
  return Exit_OK;
}

/*
** get_old_fs_dev
**
** gets old_fs entry from table, given device
*/

static struct old_fs *
get_old_fs_dev (fsdev)
     char *fsdev;
{
  register struct old_fs *ofsp;
  
  if (makeFStab (FSfile))
    return NILPTR (struct old_fs);

  for (ofsp = OldFsList; ofsp; ofsp = ofsp->next)
      if (!strcmp (ofsp->dev, fsdev))
	  return ofsp;
  return NILPTR (struct old_fs);
}
/*
** get_old_fs
**
** gets old_fs entry from table, given fs name
*/

static struct old_fs *
get_old_fs (fsname)
     char *fsname;
{
  register struct old_fs *ofsp;
  
  if (makeFStab (FSfile))
     return NILPTR (struct old_fs);

  for (ofsp = OldFsList; ofsp; ofsp = ofsp->next)
      if (!strcmp (ofsp->name, fsname))
	  return ofsp;
  return NILPTR (struct old_fs);
}
    
/*
** call_helper
**
** returns: non-zero on error
*/
static int
call_helper()
{
  int    rc;
  int    mode;
  char  noise[BUFSIZ];
  struct stat statbuf;
  char *blockname();
  int errnum;
  
  /*
  ** set up & do the helper call
  */

  if (Dev == NILPTR (char) || Vfs == NILPTR (char) || Opts == NILPTR (char))
      return -1;
  
  mode =  FSHMOD_PERROR | (isatty (0)? FSHMOD_INTERACT: 0);

  if (stat(Dev, &statbuf) < 0) {
	errnum = errno;
	fprintf(stderr, ERR_MSG(ErrExit_BADDEV, "mkfs: "));
	errno = errnum;
	perror(Dev);
	return -1;
  }
  if ((statbuf.st_mode & IFMT) == IFCHR)
	  Dev = blockname(Dev);

#ifdef DEBUG
  if (DebugLevel)
    memset ((void *)noise, 0, (size_t)(sizeof(noise)));

  rc = fshelp (Dev, Vfs, FSHOP_MAKE, mode,
		DebugLevel? FSHBUG_BLOOP+1: FSHBUG_NONE, Opts, noise);

  if (DebugLevel && noise[0] != NIL (char))
    write (2, noise, sizeof noise);
#else

  rc = fshelp (Dev, Vfs, FSHOP_MAKE, mode,
		FSHBUG_NONE, Opts, noise);

#endif
  return rc == FSHRC_GOOD? Exit_OK: ErrExit_HELPER;

}

#ifdef DEBUG
static void
dump_old_fs()
{
  register struct old_fs *ofsp;

  BUGLPR (DebugLevel, BUGGID, ("-- old_fs list: -- \n"));
  for (ofsp = OldFsList; ofsp; ofsp = ofsp->next)
  {
    BUGLPR (DebugLevel, BUGGID, ("name: %s, dev: %s, vfs: %s\n",
				 ofsp->name, ofsp->dev, ofsp->vfs));
    BUGLPR (DebugLevel, BUGGID, ("boot: %s, vol: %s, size: %s\n",
				 ofsp->boot, ofsp->vol, ofsp->size));
  }
  BUGLPR (DebugLevel, BUGGID, ("-- old_fs list --\n"));
  return;
}



static void
dump_opt_defaults()
{
  register struct opt *optr;

  BUGLPR (DebugLevel, BUGGID, ("-- opt default list: -- \n"));
  for (optr = opts; optr->name; optr++)
  {
    BUGLPR (DebugLevel, BUGGID, ("option: '%s', bit: 0x%x, ",
			optr->name, optr->bit));
    switch (optr->type)
    {
    case STR_T:
    	BUGLPR (DebugLevel, BUGGID, ("optr->ptr: '%s'\n",
					(char *) optr->ptr));
	break;
    case LONG_T:
    	BUGLPR (DebugLevel, BUGGID, ("optr->ptr: '%d'\n",
					* (long *) optr->ptr));
        break;
    }
  }
  BUGLPR (DebugLevel, BUGGID, ("-- opt default list --\n"));
  return;
}
#endif DEBUG

static void
cleanup (rc, print_msg)
     int rc;
     int print_msg;
{
  if (print_msg)
  {
    if (rc != ErrExit_USAGE)
	fprintf (stderr, "%s: ", ProgName);
    fprintf (stderr, "%s\n", ERR_MSG (rc, ErrMsgs[rc]));
  }
  fflush (stderr);	/* exit is slow, do output now */
  exit (rc);
}
    
/*
** returns position of c in str
**
**  [XXX requires any changes for NLS/KJI support?]
*/

static int
poschr (str, c)
     char *str;
     char  c;
{
  int    pos;
  char  *cp;

  pos = 0;
  for (cp = str; *cp != NIL (char); cp++, pos++)
     if (*cp == c)
	break;
  return (*cp == NIL (char)? 0: pos);
}

static int
yesno (c)
     char *c;
{
  
  if (*c == '\n') return TRUE;
  if (catalog == FALSE ){ /* use default English */
  	if ((*c == 'y') || (*c == 'Y') )
     		return TRUE;

	if ((*c == 'n') || (*c == 'N'))
   	 	return FALSE;
	else
		return (-1);
   }else
  	return rpmatch (c);
}


/*
 * blockname:  Generates a block device name from the raw device name.
 * 	       Assume that the name is in one of the following formats:
 *	       /dev/rxxx or rxxx.
 */
char *
blockname(dev)
char *dev;
{
	char *p, *q, *r;

	r = p = (char *)malloc(strlen(dev) + 1);
	if ((q = strrchr(dev, '/')) == NULL) {
		if (*dev == 'r')
			strcpy(p, dev+1);
		else
			return(dev);
	}
	else {
		while(q >= dev) *p++ = *dev++;
		if (*dev == 'r')
			dev++;
		*p = '\0';
		strcat(p, dev);
	}
	return(r);
}    
