static char sccsid[] = "@(#)09	1.1  src/bos/usr/sbin/adfutil/dosclose.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:10";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: dosclose 
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


#include "doserrno.h"
#include "tables.h"

int     dosclose(dosfile)
DOSFILE dosfile;
{
     register int i;
     LD ldev;
     PD pdev;
     int close();
     doserrno = 0;

     if ( dosfile < 0 || dosfile >= NUM_OPENFILES )
	  err_return( DE_BADF );

     if ( file(dosfile).status != is_ready    &&
	  file(dosfile).status != is_open     )
	  err_return( DE_BADF );


     if ( (_file_state(dosfile) == is_dos) &&
	  (file(dosfile).oflag & (DO_WRONLY|DO_RDWR)) &&
	  (i=dosseek(dosfile,0,1),i==dosseek(dosfile,0,2)) )
     {    if (i % 512)                /* no EOF if file ends on sector bdry */
	  {     i = doswrite( dosfile, "\032", 1 );                /* ctl-Z */
		if ( i == -1 )
		      err_return( errno );
		file(dosfile).fcb->size--;            /* don't count the ^Z */
	  }
     }

     if ( file(dosfile).status == is_open )
     switch( _file_state(dosfile) ) {
	 default:
	     err_return(DE_BADF);
	 case is_dos:
	     i = dclose( file(dosfile).fcb );
	     if ( i == -1 ) return( -1 );
	     break;
	 case is_io:
	 case is_unix:
	     i = _devio(close, file(dosfile).handle );
	     if ( i == -1 ) err_return( errno );
     }

     /* if file_type == dosfile && handle <= 2, still need to        */
     /* close corresponding Unix handle to maintain synchronization  */

     file(dosfile).status = is_closed;
     free(file(dosfile).pathname);

     TRACE(("dosclose: file %d, handle %d\n", dosfile, file(dosfile).handle));

     ldev = file(dosfile).ldevice;
     if (ldev >=0 ) {          /* if ldev == -1, this is an inherited file */
/*         if ( --(ld(ldev).open_count)<=0 )
	   {       TRACE(("DOSCLOSE: unmounting %d\n",pdev));
		   pdev = ld(ldev).pdevice;
		   _unmount( pdev );
	   }
*/     }
     return(0);
}

