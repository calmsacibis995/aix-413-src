static char sccsid[] = "@(#)15	1.1  src/bos/usr/sbin/adfutil/doswrite.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:28";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: doswrite _u2dos 
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

int     doswrite(dosfile,buf,n)
DOSFILE dosfile;
char    *buf;
int     n;
{
     register int i;
     static int bufsize;
     static char *newbuf = 0;
     char *malloc(), *bufptr;
     int write();

     if (_make_open(dosfile) == -1) return (-1);

     bufptr = buf;
     if (file(dosfile).format == dos_fmt) {
	     i = 2*n;
	     if (i > bufsize) {
		       if (newbuf) free(newbuf);
		       newbuf = malloc(i);
		       bufsize = i;
	     }
	     n = _u2dos( buf, newbuf, n );
	     bufptr = newbuf;
     }

     switch( _file_state(dosfile) ) {

	 default:
	     err_return(DE_BADF);

	 case is_dos:
	     i = dwrite( file(dosfile).fcb, bufptr, n );
	     break;

	 case is_io:
	 case is_unix:
	     i = _devio(write, file(dosfile).handle, bufptr, n );
	     if ( i == -1 ) err_return( errno );
     }
     return( i );

}


/*
 *       given inbuf, insize, global
 *       create outbuf, outsize
 */

int
_u2dos(inbuf,outbuf,insize)
char *inbuf,*outbuf;
register int insize;
{
	register char *fromptr,*toptr;
	register int outsize;

	fromptr = inbuf;
	toptr = outbuf;
	outsize = 0;
	while(insize)
	{       if (*fromptr == 0x0a)                                /* LF */
		{       *toptr++ = 0x0d;
			outsize++;
		}
		*toptr++ = *fromptr++;
		outsize++;
		insize--;
	}
	return(outsize);
}

