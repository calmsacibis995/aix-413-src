static char sccsid[] = "@(#)10	1.1  src/bos/usr/sbin/adfutil/dosfirst.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:13";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: dosfirst 
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

#include "tables.h"
#include "doserrno.h"

char    *dosnext();

char    *dosfirst(srch,pattern,mode)
DOSFIND *srch;
char    *pattern;
int     mode;
{
	APATH a;
	int i;
	char *_getdir(),*s;

	doserrno = 0;
	if( _analyze( pattern, &a ) < 0) return( NULL );

	strncpy(srch->path,pattern,128);
	srch->index = 0;
	srch->mode  = mode;
	srch->base  = srch->path;
	srch->extn  = 0;
	for( s = srch->path + strlen(srch->path); s >= srch->path; s--)
	{
		switch(*s)
		{
			case '.' : if (srch->extn==0)
				   {
					srch->extn = s+1;
					 *s=0;
				   }
				   continue;
			case ':' :
			case '\\': srch->base = s+1;
				   s = srch->path;
				   break;
		}
	}
	if(srch->extn==0) srch->extn = srch->base+strlen(srch->base);

	TRACE(("dosfirst: pattern=(%s,%s)\n",srch->base,srch->extn));

	switch( pd(a.pdev).contents ) {

	    case is_unknown:
	    case is_io:
	    default:
		doserrno = DE_NOTDIR;
		return( NULL );

	    case is_dos:
		srch->is_dos = 1;
		TRACE(("dosfirst: %s %x\n", a.fpath, srch ));
		TRACE(("dosfirst: %x %s %x %x\n",pd(a.pdev).dcb,
		       _getdir(a.fpath), mode, &(srch->dos_srch) ));
		i = dfirst(pd(a.pdev).dcb, _getdir(a.fpath,'\\'),
			   mode, &(srch->dos_srch));
		if (i<0) return( NULL );
		_upcase(srch->base);
		_upcase(srch->extn);
		break;

	    case is_unix:

		srch->is_dos = 0;

		srch->handle = _devio(open, _getdir(a.upath,'/'), O_RDONLY );
		TRACE(("dosfirst: open `%s'=%d\n",a.upath,srch->handle));
		if ( srch->handle < 0 ) {
		     doserrno = errno;
		     return( NULL );
		}
	}
	return( dosnext(srch) );

}

static char *_getdir(s,c)
char *s,c;
{
	register int i;
	/*
	 *   Get directory part of pathname
	 *   y/x -> y  where x contains no / character
	 *   /x  -> /
	 *   x   -> .
	 */

	i = strlen(s);
	while( i>0 )
	     if( s[--i] == c) {
		    if (i==0) i++;
		    s[i] = 0;
		    return(s);
	     }
	return(".");
}
