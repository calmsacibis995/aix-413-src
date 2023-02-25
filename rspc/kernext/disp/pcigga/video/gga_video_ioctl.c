static char sccsid[] = "@(#)65  1.3  src/rspc/kernext/disp/pcigga/video/gga_video_ioctl.c, pcigga, rspc41B, 9504A 1/12/95 17:14:19";
/*
 *
 * COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: video_ioctl
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/errno.h>
#include <sys/types.h>

video_mpx(dev_t dev, chan_t *chan, caddr_t chan_str)
{
	*chan = 0;
	return(0);
}

video_open(dev_t dev, long flag, long chan, long ext, struct phys_displays *pd)
{
	return(ESRCH);
}

video_close(dev_t dev, long chan, long ext, struct phys_displays *pd)
{
	return(ESRCH);
}

video_ioctl(dev_t dev, int cmd, void *argp, ulong devflag, long chan, 
			long ext, struct phys_displays *pd)
{
	return(ESRCH);
}
