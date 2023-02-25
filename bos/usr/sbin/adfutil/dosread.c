static char sccsid[] = "@(#)13	1.1  src/bos/usr/sbin/adfutil/dosread.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:22";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: dosread _dos2u _noctlZ get_next_ch 
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

int     dosread(dosfile,buf,n)
DOSFILE dosfile;
char    *buf;
int     n;
{
     register int i;
     int read();

     if ( _make_open(dosfile) == -1 ) return (-1);

     switch( _file_state(dosfile) ) {

	 default:
	     err_return(DE_BADF);

	 case is_dos:
	     i = dread( file(dosfile).fcb, buf, n );
	     if ( i == -1 ) return( i );
	     break;

	 case is_io:
	     if (file(dosfile).status == is_eof)
		   return(0);
	     i = _devio(read, file(dosfile).handle, buf, n );
	     if ( i == -1 ) err_return( errno );
	     i = _noctlZ( buf, i, dosfile );
	     break;

	 case is_unix:
	     i = _devio(read, file(dosfile).handle, buf, n );
	     if ( i == -1 ) err_return( errno );
	     return( i );
     }
     if (file(dosfile).format == dos_fmt) {
	     i = _dos2u( buf, i, dosfile );
     }
     return( i );

}


/*
 *          translate inbuf and return its size:
 *          Ctl-Z marks EOF - sets global _deof
 *          translate cr-lf to Unix nl
 */

int _dos2u( inbuf, size, dosfile )
char *inbuf;
register int size;
DOSFILE dosfile;
{
	register char  *fromptr, *toptr, lastchar;
	register int   newsize;

	newsize = 0;
	lastchar = '\0';
	fromptr = inbuf;
	toptr = inbuf;
	while(size)
	{       if ( *fromptr == 0x1a)                          /* CTL-Z ? */
			break;                               /* done if so */
		if ((*fromptr == 0xa)                              /* LF ? */
		    && (lastchar == 0xd))
			{       *(--toptr) = 0xa;
				lastchar = 0;
				fromptr++;
				toptr++;
			}
		else
		{       lastchar = *fromptr;
			newsize++;
			*toptr++ = *fromptr++;
		}
		size--;
		if ((size==0)&&(lastchar==0xd)) /* was last char in blk <CR>*/
		{       if (get_next_ch(dosfile))
			{       *(--toptr) = 0xa;
				toptr++;
				lastchar = 0;
			}
		}
	}
	return(newsize);
}

int _noctlZ( inbuf, size , dosfile )
char *inbuf;
register int size;
DOSFILE dosfile;
{
	register char  *fromptr, *toptr;
	register int   newsize;

	newsize = 0;
	fromptr = inbuf;
	while(size--)
	{       if ( *fromptr++ == 0x1a)                         /* CTL-Z ? */
		{       file(dosfile).status = is_eof;
			break;                                /* done if so */
		}
		newsize++;
	}
	return(newsize);
}

get_next_ch(dosfile)
DOSFILE dosfile;
{
char chbuf[1];
int i;
int read();

     chbuf[1] = '\0';
     switch( _file_state(dosfile) )
     {   default:
	 case is_io:
	     return(0);                        /* no next char if I/O */

	 case is_dos:
	     i = dread( file(dosfile).fcb, chbuf, 1 );
	     if ( i == -1 )
		 return(0);
	     if ( *chbuf == 0xa )
		 return(1);
	     else
	     {   dlseek( file(dosfile).fcb, -1, 1 ); /* seek back 1 if not LF*/
		 return(0);
	     }

	 case is_unix:
	     i = _devio(read, file(dosfile).handle, chbuf, 1 );
	     if ( i == -1 )
		 return(0);
	     if ( *chbuf == 0xa )
		 return(1);
	     else
	     {   lseek( file(dosfile).handle, -1, 1 );
		 return(0);
	     }
     }
}
