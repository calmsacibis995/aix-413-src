static char sccsid[] = "@(#)08	1.1  src/bos/usr/sbin/adfutil/dfstat.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:07";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: dfstat 
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

#include "pcdos.h"
#include "doserrno.h"

/*
 *      DFSTAT returns 0 or -1 on error
 */

dfstat(fcb,buf)
register FCB *fcb;
DOSSTAT *buf;
{
int read();

	if (fcb->magic != FCBMAGIC)
	{       doserrno = DE_INVAL;
		return (-1);
	}
	_DFsetlock(fcb->disk);
	if (lseek(fcb->disk->fd, fcb->d_seek,0) < 0)
	{       doserrno = errno;
		_DFunlock(fcb->disk);
		return(-1);
	}
	if (_devio(read,fcb->disk->fd,dir,32) < 0)
	{       doserrno = errno;
		_DFunlock(fcb->disk);
		return(-1);
	}
	buf->st_dev      = 0 ;
	buf->st_ino      = dir->df_lcl + (dir->df_hcl << 8);
	buf->st_mode     = (dir->df_attr & FIL_RO ) ?    0444 : 0777 ;
	buf->st_mode    += (dir->df_attr & FIL_HDD) ? 0040000 :    0 ;
	buf->st_mode    += (dir->df_attr & FIL_HDF) ? 0400000 :    0 ;
	buf->st_rdev     = dir->df_attr;
	buf->st_nlink    = 0 ;
	buf->st_uid      = 0 ;
	buf->st_gid      = 0 ;
	buf->st_size     =  dir->df_siz0      + (dir->df_siz1<< 8) +
			   (dir->df_siz2<<16) + (dir->df_siz3<<24) ;
	buf->st_atime    = 0 ;
	buf->st_mtime    = unixtime(
			   dir->df_time[0]  + (dir->df_time[1]<<8),
			   dir->df_time[2]  + (dir->df_time[3]<<8) );
	buf->st_ctime    = 0 ;
	_DFunlock(fcb->disk);
	return(0);
}

static
unixtime(time,date)
register unsigned time,date;
{

static     int dmsize[12] = { 0,31,59,90,120,151,181,212,243,273,304,334};
register   int i,t;
int        y,d,h,m,s;
register long timbuf;
long timbuf2;
#define    dysize(i) ((i)%4?365:366)
	/* pull components out of DOS date and time */

	if (time==0 && date==0)
		return(0);
	y = ((date>>9)  & 0x1f) + 1980;
	t =  (date>>5)  & 0x0f;
	d =  (date)     & 0x1f;
	h =  (time>>11) & 0x1f;
	m =  (time>>5)  & 0x3f;
	s = ((time<<1)  & 0x3f) + 1;

	/* stick them into UNIX date       */
	/* This code borrowed from date(1) */

	timbuf = 0;
	for (i=1970; i<y; i++) timbuf += dysize(i);
	if  (dysize(y)==366 && t>2)                           /* Leap Year */
		timbuf += 1;
	if (t>0)
		timbuf += dmsize[t-1];
	timbuf += (d-1);
	timbuf *= 24;
	timbuf += h;
	timbuf *= 60;
	timbuf += m;
	timbuf *= 60;
	timbuf += s;

	/* convert to Greenwich time, on assumption of Standard time. */
	timbuf += timezone;

	/* Now correct for local daylight time. */
	timbuf2 = timbuf;
	if (localtime(&timbuf2)->tm_isdst) timbuf += -1*60*60;

	return(timbuf);
};
