static char sccsid[] = "@(#)16	1.1  src/bos/usr/sbin/adfutil/makeopen.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:30";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: _make_open 
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
#include   <fcntl.h>

int _make_open( dosfile )
DOSFILE dosfile;
{
struct open_file      *fp;
LD                    ldev;

     doserrno = 0;

     fp = &file(dosfile);
     ldev = fp->ldevice;

     if ( dosfile < 0 || dosfile >= NUM_OPENFILES )
	  err_return( DE_BADF );

     if (file(dosfile).status == is_ready)
	  if ( complete_open( dosfile ) == -1 ) return  ( -1 );

     if ((file(dosfile).status!=is_open) && (file(dosfile).status!=is_eof))
	  err_return( DE_BADF );

     return(0);
}

static
int complete_open( dosfile )
DOSFILE dosfile;
{
      /* reopen file as opened the first time       */
      /* position seek pointer to where we left off */

    LD            ldev;
    PD            pdev;
    register int  ufile,i;
    char          *upath;
    FCB           *fcb,*dopen();
    int           close();
    struct open_file      *fp;

    fp = &file(dosfile);
    ldev = fp->ldevice;
    TRACE(("complete_open: %d %d\n",dosfile,ldev));

    if (ldev == -1) {

	    /*
	     *  User is opening std-aux or std-prn file.
	     *  Try to open it on the right handle to begin with.
	     */

	    fp->status = is_closed;
	    switch(dosfile) {
		case 3:  i = dosopen("PRN:",DO_WRONLY); break;
		case 4:  i = dosopen("AUX:",DO_RDWR  ); break;
		default: err_return(9999); /* should not occur */
	    }
	    if ( i<0 ) err_return(i);

	    /* If a lower file number was open, have to rearrange handle */

	    if ( i<dosfile ) {
		*fp  = file(i);
		file(i).status = is_closed;
	    }
	    TRACE(("make_open: dosfile %d h=%d s=%d\n",
		   dosfile, fp->handle, fp->status ));
	    return(dosfile);
    }

    pdev = ld(ldev).pdevice;
    if ( _mount(ldev)<0 ) return(-1);   /* make sure ldev is mounted */

    TRACE(("complete_open: pd=%d contents=%d\n",pdev,pd(pdev).contents));

    switch( pd(pdev).contents ) {
    case is_dos:
	    fcb = dopen( pd(pdev).dcb,
			 fp->pathname,
			 fp->oflag & ~DO_EXCL);
	    if (fcb == NULL) return(-1);
	    i = dlseek(fcb,fp->lseek,0);
	    if (i<0) {
		    dclose(fcb);
		    return(-1);
	    }
	    ld(ldev).open_count++;
	    fp->status = is_open;
	    fp->fcb    = fcb;
	    TRACE(("reopen: dfile=%d fcb=0x%x %s\n",
		    dosfile, fcb, fp->pathname));
	    break;

    case is_unix:
	    i = _devio( open,
		      fp->pathname,
		      fp->oflag & 0x0000ffff & ~DO_EXCL,
		      fp->oflag >> 16 );
	    if (i < 0) err_return(errno);
	    i = lseek(i,fp->lseek,0);
	    if (i<0) {
		    _devio(close, i);
		    err_return(errno);
	    }
	    ld(ldev).open_count++;
	    fp->status = is_open;
	    file(dosfile).handle = i;
	    TRACE(( "reopen: dosfile=%d handle=%d %s\n",
		    dosfile, i, fp->pathname));
	    break;

    default: err_return(DE_INVAL);   /* panic... */
    }

    return(0);
}











