static char sccsid[] = "@(#)12	1.1  src/bos/usr/sbin/adfutil/dosopen.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:19";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: dosopen 
 *
 * ORIGINS: 10,27
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


#include   "tables.h"
#include   "doserrno.h"

#define OPEN_MODES   0x0070f            /* all Unix flags defined by DOS  */

FCB *dopen();

DOSFILE    dosopen(path,oflag,mode)
char       *path;
long       oflag;
DOSMODE    mode;
{
    APATH   a;
    DOSFILE dfile;
    register char *q;
    char    *copystr();

    doserrno = 0;

    /*
     *  Parse filename, find physical device & form real filename
     */

    if( _analyze( path, &a ) < 0 ) return(-1);


    /*
     *   Find a free slot in open file table
     */

    for( dfile=0; dfile<NUM_OPENFILES; dfile++ )
       if (file(dfile).status == is_closed) break;
    if (dfile==NUM_OPENFILES) err_return(DE_NFILE);

    /*
     *  Open in appropriate filesystem
     */

    switch( pd(a.pdev).contents ) {

	case is_dos: {


	    /* if dfile <= 2, need to open Unix file /dev/null    */
	    /* to maintain synchronization of stdin, stdout       */

	    /*
	     *   Create a new open file entry
	     */

	    DOSSTAT stat[1];
	    FCB *fcb;
	    int dosmode;

	    if (oflag & (DO_WRONLY|DO_RDWR))
	    {   stat->st_mode = 0;      /* can't open directories for write */
		fcb = dopen( pd(a.pdev).dcb, a.fpath, DO_RDONLY );
		if (fcb != NULL)        /* not dir if it doesn't exist */
		{    dfstat( fcb, stat );
		     dclose( fcb );
		     if (stat->st_mode & 0040000)     /* is it a directory? */
		     {   doserrno = DE_ISDIR;
			 return(-1);
		     }
		}
	    }
	    dosmode = 0;
	    if ((mode  & 0200)==0) dosmode |= FIL_RO;
	    if (oflag & DO_HIDDEN) dosmode |= FIL_HDF;
	    if (oflag & DO_SYSTEM) dosmode |= FIL_SYS;

	    fcb = dopen( pd(a.pdev).dcb, a.fpath, oflag, dosmode );
	    if (fcb == NULL) return(-1);

	    ld(a.ldev).open_count++;
	    file(dfile).status         = is_open;
	    file(dfile).ldevice        = a.ldev;
	    file(dfile).lseek          = 0;
	    file(dfile).oflag          = oflag;
	    file(dfile).pathname       = copystr( a.fpath );
	    if ((oflag & DO_ASCII)==0)
		    file(dfile).format = unix_fmt;
	    else if (_e.format == tba_fmt )
		    file(dfile).format = dos_fmt;     /* translate cr-lf's */
	    else    file(dfile).format = _e.format;
	    file(dfile).fcb            = fcb;
	    TRACE(( "dosopen: %d fcb=%x format=%d\n",
		    dfile, file(dfile).fcb, file(dfile).format ));
	    break;
	}

	case is_unknown:
	default: err_return( DE_NODEV );

	case is_io:
	case is_unix: {
		char c, *p, temp[128];
	    /*
	     *   Create a new open file entry
	     */

	    int     ufile;

	    if (oflag & DO_HIDDEN)
	    {   p = a.basename;
		c = *p;
		*p = '\0';
		strcpy(temp, a.upath);
		strcat(temp, ".");
		*p = c;
		strcat(temp, p);
	    }
	    else
		strcpy(temp,a.upath);
	    oflag &= OPEN_MODES;
	    ufile = _devio(open, temp, oflag, mode );
	    if (ufile < 0) err_return(errno);

	    ld(a.ldev).open_count++;
	    file(dfile).status         = is_open;
	    file(dfile).ldevice        = a.ldev;
	    file(dfile).pathname       = copystr( temp );
	    file(dfile).oflag          = (oflag & 0xffff) | (mode<<16);
	    file(dfile).lseek          = 0;
	    if (_e.format == tba_fmt )
		    file(dfile).format = unix_fmt;
	    else    file(dfile).format = _e.format;
	    file(dfile).handle         = ufile;
	    TRACE(( "dosopen: %d handle=%d\n",
		    dfile, file(dfile).handle ));
	}
    }

    return( dfile );
}

static char *copystr(s)
char *s;
{
	register char *t;
	char *malloc();
	t = malloc(strlen(s)+1);
	strcpy(t,s);
	return(t);
}
