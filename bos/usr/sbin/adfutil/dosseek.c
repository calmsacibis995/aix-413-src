static char sccsid[] = "@(#)14	1.1  src/bos/usr/sbin/adfutil/dosseek.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:25";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: dosseek 
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

int     dosseek(dosfile,pos,whence)
DOSFILE dosfile;
long    pos;
int     whence;
{
     register int i;

     if ( _make_open(dosfile) == -1 ) return (-1);

     switch( _file_state(dosfile) ) {

	 default:
	     err_return(DE_BADF);

	 case is_dos:
	     i = dlseek( file(dosfile).fcb, pos, whence );
	     break;

	 case is_io:
	 case is_unix:
	     i = lseek( file(dosfile).handle, pos, whence );
	     if ( i == -1 ) err_return( errno );
     }


     return( i );
}
