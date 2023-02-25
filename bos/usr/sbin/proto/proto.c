static char sccsid[] = "@(#)83	1.12  src/bos/usr/sbin/proto/proto.c, cmdfiles, bos411, 9428A410j 11/16/93 08:40:36";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: proto
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <IN/standard.h>
#include <IN/CSdefs.h>
#include <dirent.h>

#include <ctype.h>
    
#include <locale.h>

#include <nl_types.h>
#include "proto_msg.h"
    
static nl_catd	nls_catd;

#define ERR_MSG(num,str)    catgets (nls_catd, MS_PROTO_ERR, num, str)
#define MISC_MSG(num,str)   catgets (nls_catd, MS_PROTO_MISC, num, str)

#define indent(l) {register i=l; while(i--) printf( "  " );}

/*
** character which starts a comment in proto file
*/
#define COMMENT	'#'
#define NILSTR  ""    

char filename[MAXNAMLEN+1];
char *fileptr  = filename;
char *prepend  = NULL;          /* String to prepend for file sources */
char *startdir;
int level;
dev_t device;                   /* Used to prevent crossing fs boundaries */
char *enddir   = "$\n";         /* String to end directories */

main( argc, argv )
     int    argc;
     char **argv;
{
  struct stat statbuf;


        (void) setlocale (LC_ALL, NILSTR);
        nls_catd = catopen (MF_PROTO, NL_CAT_LOCALE);

  switch( argc )
  {
  default:
    fprintf (stderr,
	     ERR_MSG (Msg_USAGE, "usage: proto basedir [prepend-path]\n"));
    exit( EXITBAD );
  case 3:
    prepend = argv[2];  /* fall through */
  case 2:
    break;
  }

  if (chdir( startdir = argv[1] ) < 0)
  {
    fprintf(stderr, ERR_MSG (Msg_CDFAIL, "Can't cd to %s\n"), startdir );
    exit( EXITBAD );
  }

  if (lstat( ".", &statbuf ) < 0)
  {
    fprintf(stderr, ERR_MSG (Msg_STATFAIL, "Can't stat %s\n"), startdir );
    exit( EXITBAD );
  }

  if (prepend == NULL)
  {
    if (CScmp( startdir, "/" ) == 0)
	prepend = NILSTR;
    else prepend = startdir;
  }

  device = statbuf.st_dev;

  setbuf( stdout, SOBUF );
  level = 0;
  printf(MISC_MSG (Msg_BYLINE, "%c Prototype file for %s\n"),
	 COMMENT, startdir);
  listfile( NULL );
  printf( enddir );
  
  exit( 0 );
}

listfile( fname )
     char *fname;
{
  register int     f;
  register mode_t  n;
  dev_t            devid;
  struct stat      statbuf;
  char             slinkbuf[MAXNAMLEN+1];

  if (fname)
  {
    
    if (lstat( fname, &statbuf ) < 0)
    {
      fflush( stdout );
      fprintf(stderr, ERR_MSG (Msg_STATFAILF, "Can't stat %s/%s\n"),
	      startdir, filename, fname);
      return;
    }
    /* sockets are silently ignored */
    if ((statbuf.st_mode & S_IFMT) == S_IFSOCK)
	return;
    indent( level );
    printf( "%s\t", fname );
  }
  else
    if( lstat( ".", &statbuf) < 0)
    {
      fflush( stdout );
      fprintf(stderr, ERR_MSG (Msg_STATFAILF, "Can't stat %s/%s\n"),
	      startdir, filename, fname);
      return;
    }
		

  f = statbuf.st_mode;
  n = prtype( f );

  printf("  %d %d   ", statbuf.st_uid, statbuf.st_gid);
  
  switch( n )
  {
  case S_IFREG:      /* ordinary file */
    printf("%s%s/%s\n", prepend, filename, fname);
    break;
    
  case S_IFBLK:      /* character device */
  case S_IFCHR:      /* block device */
    devid = statbuf.st_rdev;
    printf("  %d %d\n", major( devid ), minor( devid ) );
    break;
    
  case S_IFDIR:      /* directory */
    printf("\n");
    if (statbuf.st_dev == device)
	listdir( fname, statbuf.st_size );
    else
    {
      indent( level+1 );
      printf( enddir );
    }
    break;
    
  case S_IFLNK:
    memset (slinkbuf, 0, sizeof slinkbuf);
    if (readlink (fname, slinkbuf, sizeof slinkbuf) < 0)
    {
      fprintf (stderr,
	       ERR_MSG (Msg_READLNKFAIL,
			"Can't read contents of symbolic link '%s'\n"),
	       fname);
      exit (EXITBAD);
    }
    printf("%s\n", slinkbuf);
    break;

  case S_IFIFO:      /* FIFO */
    printf("\n");
    break;
    
  default:
    break;
  }
}


listdir( name, dsize )
     register char *name;
     off_t dsize;
{
  register char *namep;
  struct dirent **dirp;
  int numdirs,i;
  int CScmp();
  int alphasort();		/* sort jobs by date of execution */
  int isinclude();		/* should a file be included in queue?*/

  namep = fileptr;

  if (name)
  {
    *fileptr++ = '/';
    fileptr = CScpy( fileptr, name );
    if (chdir( name ) < 0)
    {
      fflush(stdout);
      fprintf(stderr, ERR_MSG (Msg_CDFAILF,
			       "Can't cd to %s%s\n"), startdir, filename);
      fileptr = namep;
      *namep = 0;
      return;
    }
  }
  numdirs=scandir(".",&dirp,isinclude,alphasort);
  if (numdirs < 0)
  {
    fflush(stdout);
    fprintf(stderr, ERR_MSG (Msg_OPNDIRFAIL, "Can't opendir\n"));
    return;
  }

  level++;

  /* And list the files in order */
  for(i=0;i < numdirs;i++)
      listfile(dirp[i]->d_name);

  indent( level );
  printf( enddir );
  level--;
  chdir( ".." );
  
  fileptr = namep;
  *namep = 0;
}

prtype( flags )
     mode_t flags;
{
  register mode_t type = flags & S_IFMT;
  register char   c;

  /*
  ** Note that hard links (ie. 2nd occurences) aren't distinguished from
  ** regular files, although aix prototype files may have them, c = 'L'
  */
  switch (type)
  {
  default:
    fprintf(stderr,
	    ERR_MSG (Msg_INVALFILTYP, "Invalid file type: %o\n"), type);
    			c = '?'; break;
  case S_IFDIR:		c = 'd'; break;
  case S_IFCHR:		c = 'c'; break;
  case S_IFBLK:		c = 'b'; break;
  case S_IFIFO:		c = 'p'; break;
  case S_IFREG:		c = '-'; break;
  case S_IFLNK:		c = 'l'; break;
  }
  printf ("%c%c%c%c  %.3o", c,
	    (flags & S_ISUID)? 'u': '-',
	    (flags & S_ISGID)? 'g': '-',
	    (flags & S_ISVTX)? 't': '-',
	    (flags & S_IRWXU) | (flags & S_IRWXG) | (flags & S_IRWXO));
  return type;
}

/*
 * NAME; isinclude
 * FUNCTION:  include all directory entries but "." and ".."
 */
isinclude(dp)
struct dirent *dp;
{
	char *filename;

	filename = dp->d_name;
	if (!strcmp(filename, "."))
		return(0);
	if (!strcmp(filename, ".."))
		return(0);
	return(1);
}
